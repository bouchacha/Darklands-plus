/* battlerules.c - rules of the battles, switches (F12 > Helpers > Battles).
 *
 * battle.shoot_past_allies (off by default): a member with a bow, crossbow, gun or thrown weapon will not
 *   shoot while anyone stands in the line of fire, friend or foe; they wait ('b' on the panel)
 *   and try again. The test is line_blocked (s014 1d69:000c, (a shooter, b target, mode 0x80 at
 *   firing and pointing, 0x100 on a click)): walls, furniture, then every active body on the floor
 *   shown but the shooter and the target, its box shrunk by the shooter's skill / 90 (none at 90).
 *   Yet a missile never strikes anyone but its target: a hit is decided before it flies and flies
 *   to the target, a miss passes through everyone (s018 1ef8 missile_launch, s014 1d47). So for
 *   the party's shooters the party's other members are left out of the test: they stand on no
 *   floor while it runs. The enemies, and the enemies' shots, are as the game has them.
 *
 * battle.reposition_shooters (on by default): a member who would wait ('b') for a clear line
 *   steps aside to the nearest spot, within a few paces, from which the line to the target is
 *   clear, and shoots from there. melee_exchanges (s014 0e76:2a4a) is where a ready shooter
 *   finds the line blocked (cbt_state 'b', the next try 30..90 ticks on); after it, such a member
 *   (not held, not moving) gets the walk a click on the floor gives (destination, path_find,
 *   path_give, action "walk", as mods/levelwalk.c), the spot tried as the game tries one
 *   (cbt_collisions at it, line_blocked from it), nearest first, ties to the one farther from the
 *   target. On arriving (s014 0e76:2fa2, "ready") the missile order and its target are back and
 *   the shot is taken at once. Each member at most once in 120 ticks; a walk the player
 *   overrides with another order (or that ends elsewhere, or long after) is forgotten. With
 *   battle.shoot_past_allies on, only walls, furniture and enemies block, as there.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"


static int on_past(void) { return config_get_int("battle.shoot_past_allies", 0) && !config_get_int("original", 0); }

static void blocked_hook(void)                /* line_blocked (a, b, mode) */
{
    int a = (s16)rw(SS, (u16)(SP + 4)), b = (s16)rw(SS, (u16)(SP + 6)), mode = rw(SS, (u16)(SP + 8));
    if (a < 0 || a >= 5 || (mode != 0x80 && mode != 0x100) || !on_past()) { hook_call_original(); return; }
    u8 fl[5];
    for (int k = 0; k < 5; k++) {
        fl[k] = g_rb((u16)(GV_cbt_floor + k));
        if (k != a && k != b) g_wb((u16)(GV_cbt_floor + k), 0xff);
    }
    hook_call_original();
    for (int k = 0; k < 5; k++) g_wb((u16)(GV_cbt_floor + k), fl[k]);
    if (getenv("DL_BATTLE_LOG") && AX) fprintf(stderr, "battlerules: %d's line to %d blocked (mode %#x)\n", a, b, mode);
}

/* ---------------------------------------------------------------- stepping aside to shoot */
#define CBT(i) ((u16)(GV_combatants + (i) * 0x80))
#define HELD(i) ((u16)(GV_cbt_mods + (i) * 0x12 + 9))      /* cbt_mods[i].held */
#define PATH_STEP(i) ((u16)(GV_cbt_path_step + (i)))

static int on_reposition(void) { return config_get_int("battle.reposition_shooters", 1) && !config_get_int("original", 0); }
static signed char walk_for[5] = { -1, -1, -1, -1, -1 };  /* the target a member walks aside to shoot */
static int walk_x[5], walk_y[5];                          /* to where */
static u16 last_try[5], seen_engage[5], walk_since[5];

static int clear_from(int i, int t, int x, int y)       /* a free spot, and a clear line from it to t */
{
    u16 c = CBT(i);
    int ox = (s16)g_rw((u16)(c + 4)), oy = (s16)g_rw((u16)(c + 6));
    g_ww((u16)(c + 4), (u16)x); g_ww((u16)(c + 6), (u16)y);
    int ok = ns_call(GF_cbt_collisions, 1, c) == 0;
    if (ok && !ns_unwinding) {
        g_ww(GV_los_x0, (u16)x); g_ww(GV_los_y0, (u16)y);
        g_ww(GV_los_x1, g_rw((u16)(CBT(t) + 4))); g_ww(GV_los_y1, g_rw((u16)(CBT(t) + 6)));
        ok = ns_call(GF_line_blocked, 3, i, t, 0x80) == 0;
    }
    g_ww((u16)(c + 4), (u16)ox); g_ww((u16)(c + 6), (u16)oy);
    return ok && !ns_unwinding;
}

static void step_aside(int i)
{
    u16 c = CBT(i);
    int t = (s8)g_rb((u16)(c + 0x4a));
    if (t < 0 || t >= 13 || g_rb((u16)(CBT(t) + 0x14)) != 1) return;
    int x0 = (s16)g_rw((u16)(c + 4)), y0 = (s16)g_rw((u16)(c + 6));
    int tx = (s16)g_rw((u16)(CBT(t) + 4)), ty = (s16)g_rw((u16)(CBT(t) + 6));
    int otx = (s16)g_rw((u16)(c + 8)), oty = (s16)g_rw((u16)(c + 0xa));
    /* rings of half cells (16 x 8 pixels; a cell is 32 x 16) out to three cells */
    for (int r = 1; r <= 6; r++) {
        int bx = 0, by = 0; long bd = -1;
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++) {
                if (dx > -r && dx < r && dy > -r && dy < r) continue;
                int x = x0 + dx * 16, y = y0 + dy * 8;
                if (x < 0 || y < 0 || x >= 40 * 32 || y >= 40 * 16) continue;
                if (!clear_from(i, t, x, y)) { if (ns_unwinding) return; continue; }
                long d = (long)(x - tx) * (x - tx) + 4L * (y - ty) * (y - ty);   /* (rows half as tall) */
                if (d > bd) { bd = d; bx = x; by = y; }
            }
        if (bd < 0) continue;
        g_ww((u16)(c + 8), (u16)bx); g_ww((u16)(c + 0xa), (u16)by);
        int found = ns_call(GF_path_find, 1, i) == 1;
        if (ns_unwinding) return;
        if (!found) { g_ww((u16)(c + 8), (u16)otx); g_ww((u16)(c + 0xa), (u16)oty); continue; }
        ns_call(GF_path_give, 1, i);
        if (ns_unwinding) return;
        g_wb((u16)(GV_cbt_moving + i), 1);
        g_ww((u16)(c + 0x10), 1);                             /* walk */
        g_wb((u16)(c + 0x4a), 0xff);
        g_wb(PATH_STEP(i), 0);
        g_wb((u16)(GV_cbt_state + i), 0);
        walk_for[i] = (signed char)t; walk_x[i] = bx; walk_y[i] = by; walk_since[i] = g_rw(GV_combat_clock);
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d steps aside from %d,%d to %d,%d to shoot at %d\n", i, x0, y0, bx, by, t);
        return;
    }
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d finds no spot near %d,%d with a line to %d\n", i, x0, y0, t);
}

static void exchanges_hook(void)              /* melee_exchanges (), s014 0e76:2a4a */
{
    if (!on_reposition()) { hook_call_original(); return; }
    for (int i = 0; i < 5; i++) seen_engage[i] = g_rw((u16)(GV_cbt_engage_time + 2 * i));
    hook_call_original();
    if (ns_unwinding) return;
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < 5; i++) {
        u16 c = CBT(i);
        if (g_rb((u16)(GV_cbt_state + i)) != 1 || g_rw((u16)(GV_cbt_engage_time + 2 * i)) == seen_engage[i]) continue;   /* (blocked just now) */
        if (g_rb((u16)(c + 0x14)) != 1 || !(g_rw((u16)(c + 0x10)) & 0x80) || g_rb((u16)(GV_cbt_moving + i)) || g_rb(HELD(i)) == 1) continue;
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
    int still = was >= 0 && g_rw((u16)(c + 0x10)) == 1 && (s8)g_rb((u16)(c + 0x4a)) == -1        /* (no other order since: */
                && (s16)g_rw((u16)(c + 8)) == walk_x[i] && (s16)g_rw((u16)(c + 0xa)) == walk_y[i]       /*  still that walk, */
                && (u16)(g_rw(GV_combat_clock) - walk_since[i]) < 600;                                  /*  not long ago) */
    if (i >= 0 && i < 5) walk_for[i] = -1;
    if (still && g_rb((u16)(CBT(was) + 0x14)) == 1 && g_rb((u16)(c + 0x14)) == 1) {
        g_ww((u16)(c + 0x10), 0x80);                          /* the missile order again: 2fa2 sees it and engages now */
        g_wb((u16)(c + 0x4a), (u8)was);
        if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: %d in place, shoots at %d\n", i, was);
    }
    hook_call_original();
}

void battlerules_init(void)
{
    hook_install(GF_line_blocked, blocked_hook);
    hook_install(GF_melee_exchanges, exchanges_hook);
    hook_install(GF_cbt_ready, arrive_hook);
}
