/* battlerules.c - rules of the battles, switches (F12 > Settings > Battles).
 *
 * People in the line of fire block a shot, friend or foe, as the game has it (line_blocked, s014
 *   1d69:000c: walls, furniture, then every active body on the floor shown but the shooter and the
 *   target, its box shrunk by the shooter's skill / 90). (Shooting past them was tried and left out:
 *   the player wants the game's rule.)
 *
 * Improved combat (battle.improved): a member who would wait ('b') for a clear line
 *   steps aside to the nearest spot, within a few paces, from which the line to the target is
 *   clear, and shoots from there. melee_exchanges (s014 0e76:2a4a) is where a ready shooter
 *   finds the line blocked (cbt_state 'b', the next try 30..90 ticks on); after it, such a member
 *   (not held, not moving) gets the walk a click on the floor gives (destination, path_find,
 *   path_give, action "walk", as mods/levelwalk.c), the spot tried as the game tries one
 *   (cbt_collisions at it, line_blocked from it), nearest first, ties to the one farther from the
 *   target. On arriving (s014 0e76:2fa2, "ready") the missile order and its target are back and
 *   the shot is taken at once. Each member at most once in 120 ticks; a walk the player
 *   overrides with another order (or that ends elsewhere, or long after) is forgotten.
 *   No spot near: the member walks towards the foe (path_find to it), and at each step
 *   (cbt_step, s014 0e76:1624) the line from there is tested; when it is clear the walk stops
 *   and the member shoots (the missile order back, cbt_ready).
 *   The game refuses a target with no clear line when it is pointed at or clicked (line_blocked
 *   with mode 0x80 from s014 19cd:17f6, 0x100 from 19cd:0dcc: the barred circle and a beep), so a
 *   shooter could never be sent at a foe behind a friend, a foe or a corner. With this switch on,
 *   any foe on the member's floor can be chosen; the shot itself is still tested as the game does.
 *
 * battle.retarget_near (on): when a member's foe falls, the next one is a foe within reach, not one
 *   across the melee. ai_rethink (s014 1db7:0e22) picks the new foe in three ways: one attacking the
 *   member hand to hand; else, when anyone on the field is down to half endurance, a foe attacking the
 *   most wounded friend - whatever the way to it, so a member walks around the whole melee to get
 *   there; else the foe nearest on foot. Its walking costs (s014 1db7:0028, steps of the game's own
 *   route trackers, 32000 for none) are taken as they are, but for a party member every foe much
 *   farther than the nearest (more than 24 steps, or half again, beyond) is marked out of reach
 *   (9999), which the second way passes over: the member helps a friend only when that friend's
 *   attacker is about as near as anyone. The enemies choose as the game has them.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "battlerules.h"
int orders_may_move(int i);                     /* (mods/ways.c: not a Holdfast; an Off member only when sent to shoot) */


/* battle.improved: the rules of a fight in one switch (the shooters' ways, the next foe, the kinds of attack kept) */
int combat_improved(void) { return config_get_int("battle.improved", 1) && !config_get_int("original", 0); }
#define CBT(i) ((u16)(GV_combatants + (i) * 0x80))
static int on_reposition(void);

/* asked by the choice of a target: the pointer over a foe while a member's target is chosen (s014
   19cd:17f6, mode 0x80: the barred circle) or the click choosing it (s014 19cd:0dcc, mode 0x100: the
   beep). Every other test - the shot itself, the game's own and Darklands+'s - gets the true answer. */
static int asked_for_target(void)
{
    u16 ip = rw(SS, SP), cs = (u16)(rw(SS, (u16)(SP + 2)) - LOADSEG);
    return cs == 0x19cd && ((ip > 0x17f6 && ip < 0x1965) || (ip > 0x0dcc && ip < 0x11ec));
}

static void blocked_hook(void)                /* line_blocked (a, b, mode) */
{
    int a = (s16)rw(SS, (u16)(SP + 4)), b = (s16)rw(SS, (u16)(SP + 6)), mode = rw(SS, (u16)(SP + 8));
    if (a < 0 || a >= 5 || (mode != 0x80 && mode != 0x100)) { hook_call_original(); return; }
    int choosing = asked_for_target();
    hook_call_original();
    if (ns_unwinding) return;
    if (getenv("DL_BATTLE_LOG") && AX && mode == 0x100) fprintf(stderr, "battlerules: %d's line to %d blocked (mode %#x)\n", a, b, mode);
    /* a foe chosen as a target with no line from where the member stands: taken (the member will
       step aside, or walk towards it, until a line opens) */
    if (AX && choosing && on_reposition() && b >= 5 && b < 13 && g_rb(CBF(b, CB_STATUS)) == 1
        && (g_rw(GV_order_action) & 0x180) == 0x80                                           /* (a missile, not a potion thrown or a spell) */
        && g_rb((u16)(GV_cbt_floor + a)) == g_rb((u16)(GV_cbt_floor + b))) {
        AX = 0;
        if (mode == 0x100 && getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d may target %d (no line yet)\n", a, b);
    }
}

/* ---------------------------------------------------------------- stepping aside to shoot */
#define HELD(i) ((u16)(GV_cbt_mods + (i) * 0x12 + 9))      /* cbt_mods[i].held */
#define PATH_STEP(i) ((u16)(GV_cbt_path_step + (i)))

static int on_reposition(void) { return combat_improved(); }
static signed char walk_for[5] = { -1, -1, -1, -1, -1 };  /* the target a member walks aside to shoot */
static int walk_x[5], walk_y[5];                          /* to where */
static signed char approach[5];                           /* the walk is towards the foe, until a line opens */
static u16 last_try[5], seen_engage[5], walk_since[5];
void battlerules_battle_begins(void)                         /* (mods/ways.c orders_frame: a new battle) */
{
    for (int i = 0; i < 5; i++) { walk_for[i] = -1; approach[i] = 0; last_try[i] = seen_engage[i] = walk_since[i] = 0; }
}

static int strict_spot;                                 /* the line clear to where the mark is going too (a first pass) */
static int spots_taken, spots_noline, spots_noway, spots_crossed;   /* (DL_BATTLE_LOG: why no spot was found) */
static int clear_from(int i, int t, int x, int y)       /* a free spot, and a clear line from it to t */
{
    u16 c = CBT(i);
    int ox = (s16)g_rw((u16)(c + CB_X)), oy = (s16)g_rw((u16)(c + CB_Y));
    g_ww((u16)(c + CB_X), (u16)x); g_ww((u16)(c + CB_Y), (u16)y);
    int ok = ns_call(GF_cbt_collisions, 1, c) == 0;
    if (!ok) spots_taken++;
    if (ok && !ns_unwinding) {
        g_ww(GV_los_x0, (u16)x); g_ww(GV_los_y0, (u16)y);
        g_ww(GV_los_x1, g_rw(CBF(t, CB_X))); g_ww(GV_los_y1, g_rw(CBF(t, CB_Y)));
        ok = ns_call(GF_line_blocked, 3, i, t, 0x80) == 0;
        if (!ok) spots_noline++;
    }
    if (ok && !ns_unwinding && strict_spot && g_rb((u16)(GV_cbt_moving + t))) {   /* (and to where the mark walks, so the line holds as it comes) */
        u16 tc = CBT(t); int tx = (s16)g_rw((u16)(tc + CB_X)), ty = (s16)g_rw((u16)(tc + CB_Y));
        g_ww((u16)(tc + CB_X), g_rw((u16)(tc + CB_TO_X))); g_ww((u16)(tc + CB_Y), g_rw((u16)(tc + CB_TO_Y)));
        g_ww(GV_los_x0, (u16)x); g_ww(GV_los_y0, (u16)y);
        g_ww(GV_los_x1, g_rw((u16)(tc + CB_TO_X))); g_ww(GV_los_y1, g_rw((u16)(tc + CB_TO_Y)));
        ok = ns_call(GF_line_blocked, 3, i, t, 0x80) == 0;
        g_ww((u16)(tc + CB_X), (u16)tx); g_ww((u16)(tc + CB_Y), (u16)ty);
    }
    g_ww((u16)(c + CB_X), (u16)ox); g_ww((u16)(c + CB_Y), (u16)oy);
    return ok && !ns_unwinding;
}

static void step_aside_at(int i);
/* friends on their way somewhere are taken as there already while a spot is chosen (three shooters in
   a column each picked a spot the others then walked into: they crept aside, sixteen pixels at a
   time, and never shot); the line to where the mark walks is wanted too, then only to where it is.
   And no spot whose way there crosses a walking friend's way (two in a column each took the spot
   behind the other: they met halfway and stood, each in the other's way, for good) */
static int fw_n, fw[5][4];                               /* the walking friends' ways: from x, y to x, y */
static int crosses(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy)   /* segment ab meets segment cd */
{
    long d1 = (long)(bx - ax) * (cy - ay) - (long)(by - ay) * (cx - ax), d2 = (long)(bx - ax) * (dy - ay) - (long)(by - ay) * (dx - ax);
    long d3 = (long)(dx - cx) * (ay - cy) - (long)(dy - cy) * (ax - cx), d4 = (long)(dx - cx) * (by - cy) - (long)(dy - cy) * (bx - cx);
    return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
}
static void step_aside(int i)
{
    int sx[5], sy[5], moved[5] = { 0 };
    fw_n = 0;
    for (int j = 0; j < 5; j++) {
        u16 f = CBT(j);
        if (j == i || g_rb((u16)(f + CB_STATUS)) != 1 || !g_rb((u16)(GV_cbt_moving + j))) continue;
        sx[j] = (s16)g_rw((u16)(f + CB_X)); sy[j] = (s16)g_rw((u16)(f + CB_Y)); moved[j] = 1;
        fw[fw_n][0] = sx[j]; fw[fw_n][1] = sy[j]; fw[fw_n][2] = (s16)g_rw((u16)(f + CB_TO_X)); fw[fw_n][3] = (s16)g_rw((u16)(f + CB_TO_Y)); fw_n++;
        g_ww((u16)(f + CB_X), g_rw((u16)(f + CB_TO_X))); g_ww((u16)(f + CB_Y), g_rw((u16)(f + CB_TO_Y)));
    }
    step_aside_at(i);
    fw_n = 0;
    for (int j = 0; j < 5; j++) if (moved[j]) { g_ww(CBF(j, CB_X), (u16)sx[j]); g_ww(CBF(j, CB_Y), (u16)sy[j]); }
}
static void step_aside_at(int i)
{
    u16 c = CBT(i);
    int t = (s8)g_rb((u16)(c + CB_TARGET));
    if (t < 0 || t >= 13 || g_rb(CBF(t, CB_STATUS)) != 1) return;
    int x0 = (s16)g_rw((u16)(c + CB_X)), y0 = (s16)g_rw((u16)(c + CB_Y));
    int tx = (s16)g_rw(CBF(t, CB_X)), ty = (s16)g_rw(CBF(t, CB_Y));
    int otx = (s16)g_rw((u16)(c + CB_TO_X)), oty = (s16)g_rw((u16)(c + CB_TO_Y));
    /* another foe in plain sight from here: shoot it rather than walk (the mark has gone behind
       someone; the nearest with a clear line) */
    {   int best = -1; long bd = 0;
        u16 l0 = g_rw(GV_los_x0), l1 = g_rw(GV_los_y0), l2 = g_rw(GV_los_x1), l3 = g_rw(GV_los_y1);
        for (int j = 5; j < 13; j++) {
            if (j == t || g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != g_rb((u16)(GV_cbt_floor + i))) continue;
            g_ww(GV_los_x0, (u16)x0); g_ww(GV_los_y0, (u16)y0);
            g_ww(GV_los_x1, g_rw(CBF(j, CB_X))); g_ww(GV_los_y1, g_rw(CBF(j, CB_Y)));
            int bl = ns_call(GF_line_blocked, 3, i, j, 0x80);
            if (ns_unwinding) return;
            if (bl) continue;
            long dx = (s16)g_rw(CBF(j, CB_X)) - x0, dy = 2L * ((s16)g_rw(CBF(j, CB_Y)) - y0), d = dx * dx + dy * dy;
            if (best < 0 || d < bd) { best = j; bd = d; }
        }
        g_ww(GV_los_x0, l0); g_ww(GV_los_y0, l1); g_ww(GV_los_x1, l2); g_ww(GV_los_y1, l3);
        if (best >= 0) {
            g_wb((u16)(c + CB_TARGET), (u8)best); g_wb((u16)(GV_cbt_state + i), 0);
            fprintf(stderr, "battlerules: %d has no line to %d: shoots at %d instead\n", i, t, best);
            return;
        }
    }
    spots_taken = spots_noline = spots_noway = spots_crossed = 0;
    /* rings of half cells (16 x 8 pixels; a cell is 32 x 16) out to three cells; first with the line
       holding to where the mark walks, then to where it is */
    for (int pass = 0; pass < 2; pass++)
    for (int r = 1; r <= 6 && !getenv("DL_TEST_APPROACH"); r++) {   /* (debugging: DL_TEST_APPROACH tries the walk towards the foe at once) */
        strict_spot = pass == 0;
        int bx = 0, by = 0; long bd = -1;
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++) {
                if (dx > -r && dx < r && dy > -r && dy < r) continue;
                int x = x0 + dx * 16, y = y0 + dy * 8;
                if (x < 0 || y < 0 || x >= 40 * 32 || y >= 40 * 16) continue;
                { int cross = 0; for (int k = 0; k < fw_n && !cross; k++) cross = crosses(x0, y0, x, y, fw[k][0], fw[k][1], fw[k][2], fw[k][3]); if (cross) { spots_crossed++; continue; } }
                if (!clear_from(i, t, x, y)) { if (ns_unwinding) { strict_spot = 0; return; } continue; }
                long d = (long)(x - tx) * (x - tx) + 4L * (y - ty) * (y - ty);   /* (rows half as tall) */
                if (d > bd) { bd = d; bx = x; by = y; }
            }
        strict_spot = 0;
        if (bd < 0) continue;
        g_ww((u16)(c + CB_TO_X), (u16)bx); g_ww((u16)(c + CB_TO_Y), (u16)by);
        int found = ns_call(GF_path_find, 1, i) == 1;
        if (ns_unwinding) return;
        if (!found) { spots_noway++; g_ww((u16)(c + CB_TO_X), (u16)otx); g_ww((u16)(c + CB_TO_Y), (u16)oty); continue; }
        ns_call(GF_path_give, 1, i);
        if (ns_unwinding) return;
        g_wb((u16)(GV_cbt_moving + i), 1);
        g_ww((u16)(c + CB_ORDER), 1);                             /* walk */
        g_wb((u16)(c + CB_TARGET), 0xff);
        g_wb(PATH_STEP(i), 0);
        g_wb((u16)(GV_cbt_state + i), 0);
        walk_for[i] = (signed char)t; walk_x[i] = bx; walk_y[i] = by; walk_since[i] = g_rw(GV_combat_clock);
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d steps aside from %d,%d to %d,%d to shoot at %d\n", i, x0, y0, bx, by, t);
        return;
    }
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d's spots near %d,%d: %d taken, %d with no line, %d with no way to them, %d across a friend's way\n", i, x0, y0, spots_taken, spots_noline, spots_noway, spots_crossed);
    /* no spot near: walk towards the foe until a line opens (step_hook) */
    g_ww((u16)(c + CB_TO_X), (u16)tx); g_ww((u16)(c + CB_TO_Y), (u16)ty);
    int found = ns_call(GF_path_find, 1, i) == 1;
    if (ns_unwinding) return;
    if (!found) {
        g_ww((u16)(c + CB_TO_X), (u16)otx); g_ww((u16)(c + CB_TO_Y), (u16)oty);
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d finds no spot near %d,%d with a line to %d, nor a way to it\n", i, x0, y0, t);
        return;
    }
    ns_call(GF_path_give, 1, i);
    if (ns_unwinding) return;
    g_wb((u16)(GV_cbt_moving + i), 1);
    g_ww((u16)(c + CB_ORDER), 1);                                 /* walk */
    g_wb((u16)(c + CB_TARGET), 0xff);
    g_wb(PATH_STEP(i), 0);
    g_wb((u16)(GV_cbt_state + i), 0);
    walk_for[i] = (signed char)t; walk_x[i] = tx; walk_y[i] = ty; walk_since[i] = g_rw(GV_combat_clock); approach[i] = 1;
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d walks from %d,%d towards %d until a line opens\n", i, x0, y0, t);
}

/* the missile order at t, engaged now: after cbt_ready, which takes the next queued order in
   place of the one set (s014 3cc4: the queue empty, the order became none and the shooter stood
   idle, its mark still set); as orders.c idle_shooters gives it */
static void shoot_now(int i, int t)
{
    u16 c = CBT(i);
    int wm = (s8)g_rb((u16)(c + CB_MISSILE));
    if (wm < 0 || wm > 62) return;
    ns_call(GF_weapon_ready, 3, i, wm, 0x80);
    if (ns_unwinding) return;
    g_wb((u16)(c + CB_TARGET), (u8)t); g_ww((u16)(c + CB_ORDER), 0x80);
    g_wb((u16)(GV_cbt_state + i), 0);
    g_ww((u16)(GV_cbt_flags + 2 * i), (u16)(g_rw((u16)(GV_cbt_flags + 2 * i)) | 2));
    g_ww((u16)(GV_cbt_engage_time + 2 * i), g_rw(GV_combat_clock));
    g_wb((u16)(GV_cbt_action + i), 7);
}

static void step_hook(void)                   /* cbt_step (i, set), s014 0e76:1624: one step along the path (a near call) */
{
    int i = (s16)rw(SS, (u16)(SP + 2));
    hook_call_original();
    if (ns_unwinding || i < 0 || i >= 5 || !approach[i]) return;
    u16 c = CBT(i);
    int t = walk_for[i];
    if (t < 0 || g_rw((u16)(c + CB_ORDER)) != 1 || (s8)g_rb((u16)(c + CB_TARGET)) != -1 || !g_rb((u16)(GV_cbt_moving + i))   /* (arrived, or another order) */
        || (s16)g_rw((u16)(c + CB_TO_X)) != walk_x[i] || (s16)g_rw((u16)(c + CB_TO_Y)) != walk_y[i] || g_rb(CBF(t, CB_STATUS)) != 1) { approach[i] = 0; return; }
    u16 l0 = g_rw(GV_los_x0), l1 = g_rw(GV_los_y0), l2 = g_rw(GV_los_x1), l3 = g_rw(GV_los_y1);
    g_ww(GV_los_x0, g_rw((u16)(c + CB_X))); g_ww(GV_los_y0, g_rw((u16)(c + CB_Y)));
    g_ww(GV_los_x1, g_rw(CBF(t, CB_X))); g_ww(GV_los_y1, g_rw(CBF(t, CB_Y)));
    int blocked = ns_call(GF_line_blocked, 3, i, t, 0x80) != 0;
    g_ww(GV_los_x0, l0); g_ww(GV_los_y0, l1); g_ww(GV_los_x1, l2); g_ww(GV_los_y1, l3);
    if (ns_unwinding || blocked) return;
    approach[i] = 0; walk_for[i] = -1;                        /* a line: stop here and shoot */
    g_wb((u16)(GV_cbt_moving + i), 0);
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d has a line to %d at %d,%d: shoots\n", i, t, (s16)g_rw((u16)(c + CB_X)), (s16)g_rw((u16)(c + CB_Y)));
    ns_call(GF_cbt_ready, 1, i);
    if (!ns_unwinding) shoot_now(i, t);
}

static void exchanges_hook(void)              /* melee_exchanges (), s014 0e76:2a4a */
{
    if (!on_reposition()) { hook_call_original(); return; }
    for (int i = 0; i < 5; i++) seen_engage[i] = g_rw((u16)(GV_cbt_engage_time + 2 * i));
    hook_call_original();
    if (ns_unwinding) return;
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < 5; i++) {                             /* a walk aside that has gone nowhere for a second: planned again from here */
        static int px[5], py[5]; static u16 pt[5];
        u16 c = CBT(i);
        if (walk_for[i] < 0 || !g_rb((u16)(GV_cbt_moving + i)) || g_rb((u16)(c + CB_STATUS)) != 1) { pt[i] = 0; continue; }
        int x = (s16)g_rw((u16)(c + CB_X)), y = (s16)g_rw((u16)(c + CB_Y));
        if (!pt[i] || abs(x - px[i]) > 1 || abs(y - py[i]) > 1) { px[i] = x; py[i] = y; pt[i] = now ? now : 1; continue; }
        if ((u16)(now - pt[i]) < 100) continue;
        int t = walk_for[i];
        pt[i] = 0; walk_for[i] = -1; approach[i] = 0;
        if (g_rb(CBF(t, CB_STATUS)) != 1) continue;
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d's walk aside to shoot at %d went nowhere from %d,%d: planned again\n", i, t, x, y);
        g_wb((u16)(GV_cbt_moving + i), 0);
        u16 l0 = g_rw(GV_los_x0), l1 = g_rw(GV_los_y0), l2 = g_rw(GV_los_x1), l3 = g_rw(GV_los_y1);
        g_ww(GV_los_x0, (u16)x); g_ww(GV_los_y0, (u16)y); g_ww(GV_los_x1, g_rw(CBF(t, CB_X))); g_ww(GV_los_y1, g_rw(CBF(t, CB_Y)));
        int blocked = ns_call(GF_line_blocked, 3, i, t, 0x80) != 0;
        g_ww(GV_los_x0, l0); g_ww(GV_los_y0, l1); g_ww(GV_los_x1, l2); g_ww(GV_los_y1, l3);
        if (ns_unwinding) return;
        if (!blocked) {                                       /* (a line from where it got to: shoot from there) */
            ns_call(GF_cbt_ready, 1, i);
            if (!ns_unwinding) shoot_now(i, t);
            if (ns_unwinding) return;
            continue;
        }
        g_ww((u16)(c + CB_ORDER), 0x80); g_wb((u16)(c + CB_TARGET), (u8)t);   /* (the shot it was walking for, then a spot from here) */
        last_try[i] = now ? now : 1;
        step_aside(i);
        if (ns_unwinding) return;
    }
    for (int i = 0; i < 5; i++) {
        u16 c = CBT(i);
        if (g_rb((u16)(GV_cbt_state + i)) != 1 || g_rw((u16)(GV_cbt_engage_time + 2 * i)) == seen_engage[i]) continue;   /* (blocked just now) */
        if (g_rb((u16)(c + CB_STATUS)) != 1 || !(g_rw((u16)(c + CB_ORDER)) & 0x80) || g_rb((u16)(GV_cbt_moving + i)) || g_rb(HELD(i)) == 1) continue;
        if (!orders_may_move(i)) continue;                      /* (a Holdfast stays: another mark with a line, mods/ways.c; Off: the game's way) */
        if ((u16)(now - last_try[i]) < 120 && last_try[i]) continue;
        last_try[i] = now ? now : 1;
        step_aside(i);
        if (ns_unwinding) return;
    }
}

static void arrive_hook(void)                 /* cbt_ready (i), s014 0e76:2fa2: arrived or stopped, "ready" */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    int was = i >= 0 && i < 5 ? walk_for[i] : -1;
    u16 c = i >= 0 && i < 5 ? CBT(i) : 0;
    int still = was >= 0 && g_rw((u16)(c + CB_ORDER)) == 1 && (s8)g_rb((u16)(c + CB_TARGET)) == -1        /* (no other order since: */
                && abs((s16)g_rw((u16)(c + CB_TO_X)) - walk_x[i]) <= 8 && abs((s16)g_rw((u16)(c + CB_TO_Y)) - walk_y[i]) <= 4   /*  still that walk (its end a pixel or two off), */
                && (u16)(g_rw(GV_combat_clock) - walk_since[i]) < 600;                                  /*  not long ago) */
    if (i >= 0 && i < 5) walk_for[i] = -1, approach[i] = 0;
    hook_call_original();
    if (ns_unwinding) return;
    if (still && g_rb(CBF(was, CB_STATUS)) == 1 && g_rb((u16)(c + CB_STATUS)) == 1) {
        shoot_now(i, was);                                    /* the missile order again, at the mark it walked for */
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d in place, shoots at %d\n", i, was);
    }
}

/* test (umissile:i,t): member i, told to shoot at t, finds the line blocked (step_aside at once) */
void battlerules_test_missile(int i, int t)
{
    if (i < 0 || i >= 5 || t < 5 || t >= 13) return;
    u16 c = CBT(i);
    g_ww((u16)(c + CB_ORDER), 0x80); g_wb((u16)(c + CB_TARGET), (u8)t);
    fprintf(stderr, "battlerules: test: %d shoots at %d, the line taken as blocked\n", i, t);
    step_aside(i);
}

/* ---------------------------------------------------------------- the next foe within reach */
#define AISEG ((u16)(LOADSEG + 0x20a5))
static int on_near(void) { return combat_improved(); }
static void costs_hook(void)                  /* s014 1db7:0028 (i, lo, hi), called by ai_rethink only */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    if (ns_unwinding || i < 0 || i >= 5 || !on_near()) return;
    int c[13], min = 32000;
    for (int j = 5; j < 13; j++) { c[j] = (s16)rw(AISEG, (u16)(0x787e + 2 * (i * 5 + j))); if (c[j] < min) min = c[j]; }
    if (min >= 9999) return;
    int lim = min + (min / 2 > 24 ? min / 2 : 24);
    for (int j = 5; j < 13; j++)
        if (c[j] < 9999 && c[j] > lim) {
            ww(AISEG, (u16)(0x787e + 2 * (i * 5 + j)), 9999);
            if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d's next foe: %d (%d steps) out of reach, the nearest %d steps\n", i, j, c[j], min);
        }
    if (getenv("DL_BATTLE_LOG")) {
        fprintf(stderr, "battlerules: %d's foes on foot:", i);
        for (int j = 5; j < 13; j++) if (c[j] < 32000) fprintf(stderr, " %d:%d", j, c[j]);
        fprintf(stderr, "\n");
    }
}

/* ---------------------------------------------------------------- battle.black_cloud
 * The party's potions to throw in a battle are listed by s019 1ef8:0c80 (i): every potion of
 * 0x5f..0x68 of quality 20..49 the member carries, three grades a potion, at most 20 (the middle
 * grades dropped first), into the menu's items (DS:9a47, 22 bytes: +0 the name's pointer from
 * DS:114e[potion], +3 "q45", +7 how many, +8 the quality, +9 the potion; DS:114a after the last),
 * DS:97dd 1 (something) or 2 (nothing). It leaves Black Cloud (0x61) out: the manual says it
 * "cannot be used in battle because the screen takes too long" (1992's machines), though its
 * effect is in the battle's code (enemies throw it). With this on, the list is made the same way
 * with Black Cloud in it. */
static int on_cloud(void) { return config_get_int("battle.black_cloud", 0) && !config_get_int("original", 0); }   /* (restored: experimental, off by default) */
static void throwlist_hook(void)                /* s019 1ef8:0c80 (i) */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    if (!on_cloud() || i < 0 || i > 4) { hook_call_original(); return; }
    u16 c = (u16)(GV_combatants + 0x80 * i);
    if ((g_rw((u16)(c + CB_ORDER)) & 2) == 2) { g_wb(0x97dd, 2); hook_return_far(0); return; }
    struct { int id, q, n; } list[30];
    for (int k = 0; k < 30; k++) list[k].id = list[k].q = list[k].n = -1;
    int cnt = 0, nit = g_rb((u16)(c + CB_NITEMS));
    for (int k = 0; k < nit && k < 64; k++) {
        u16 o = G_ITEM(i, k);
        int id = rw(G_FAR2E38, o), q = rb(G_FAR2E38, (u16)(o + 3));
        if (id < 0x5f || id > 0x68 || q < 0x14 || q >= 0x32) continue;
        int a = id - 0x5f, x = a * 3 + q / 10 - 2;
        list[x].id = a; list[x].q = q; list[x].n = rb(G_FAR2E38, (u16)(o + 4));
        cnt++;
    }
    for (int k = 1; cnt > 20 && k < 30; k += 3) if (list[k].id != -1) { list[k].id = -1; cnt--; }
    int m = 0;
    for (int k = 0; k < 30; k++) {
        if (list[k].id == -1) continue;
        u16 e = (u16)(0x9a47 + 22 * m++);
        char nm[8]; snprintf(nm, sizeof nm, "q%d", list[k].q);
        g_ww(e, g_rw((u16)(0x114e + 2 * list[k].id)));
        for (int j = 0; j < 4; j++) g_wb((u16)(e + 3 + j), (u8)nm[j]);
        g_wb((u16)(e + 7), (u8)list[k].n); g_wb((u16)(e + 8), (u8)list[k].q); g_wb((u16)(e + 9), (u8)list[k].id);
    }
    g_ww((u16)(0x9a47 + 22 * m), g_rw(0x114a));
    g_wb(0x97dd, (u8)(m ? 1 : 2));
    hook_return_far(0);
}

void battlerules_init(void)
{
    hook_install(19, 0x1ef8, 0x0c80, throwlist_hook);
    hook_install(GF_ai_costs, costs_hook);
    hook_install(GF_line_blocked, blocked_hook);
    hook_install(GF_melee_exchanges, exchanges_hook);
    hook_install(GF_cbt_ready, arrive_hook);
    hook_install(GF_cbt_step, step_hook);
}
