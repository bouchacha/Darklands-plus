/* ways.c - Improved combat (battle.improved): each party member fights in their way (mods/styles.c) between
 * the player's orders, after each round of blows (melee_exchanges, s014 0e76:2a4a: melee_hook). mods/orders.c
 * is the player's side (a click gives an order); this file is what the members do without one:
 *   the shooters (Skirmisher, Archer) open fire by themselves (idle_shooters), each at a foe of its own
 *   (shot_target), take a new mark when theirs falls (retarget_shooters), step aside for a clear line
 *   (mods/battlerules.c), fight hand to hand a foe that reaches them (the volley at the end of melee_hook)
 *   and take up the missile weapon again after it (back_to_bow; a Skirmisher fights hand to hand to the
 *   end once the enemies are close);
 *   the fighters (Man-at-arms, Charger, and the shooters with nothing left to loose) go at a foe as their
 *   way has it (idle_fighters, melee_pick) and turn on a foe beside them when theirs falls (stale_melee);
 *   an order of the player's is the member's way for the rest of that fight (leave_hook): sent somewhere or
 *   halted, a member holds there (ST_HELD: it shoots what it sees from there, fights what reaches it, never
 *   walks on its own); Manual (ST_OFF) is the original's way for that member. The next battle starts afresh
 *   (orders_frame). */
#include "gamemodel.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "battlerules.h"
#include "styles.h"
#include "ways.h"

#define D_ORDER_ACTION GV_order_action   /* int: the attack chosen in order mode (-1 none, 1 walk ...) */
#define D_SELECTED     GV_selected_member   /* char: the member given orders */
#define D_CBT          GV_combatants   /* struct cbt[13], 0x80 bytes: +4 x, +6 y, +0x10 order, +0x14 status */
#define NEAR_PX 56                      /* (ways_near_foe) */

/* Each member fights in a way of their own until the player gives them an order (style_of: Skirmisher,
 * Archer, Man-at-arms (ST_FOOTMAN), Charger, Manual (ST_OFF)). An order of the player's is theirs for
 * the rest of the fight in its manner (ov[]): sent hand to hand they go on hand to hand, sent to shoot
 * they go on shooting, sent somewhere or halted they hold there; Manual is the original's way for that
 * member (nothing of ours). A Skirmisher shoots until the enemies are close, then fights hand to hand
 * for the rest of the fight (committed[]). The next battle starts afresh. */
enum { OV_NONE, OV_MELEE, OV_SHOOT, OV_HOLD };
static signed char ov[5], committed[5];
static int st(int i)                            /* the way member i fights now */
{
    if (i < 0 || i > 4) return ST_OFF;
    int s0 = style_of(i);
    if (s0 == ST_OFF) return ST_OFF;
    if (ov[i] == OV_MELEE) return ST_FOOTMAN;
    if (ov[i] == OV_SHOOT) return ST_ARCHER;
    if (ov[i] == OV_HOLD) return ST_HELD;
    if (s0 == ST_SKIRMISHER && committed[i]) return ST_FOOTMAN;
    return s0;
}
static int automated(int i) { return combat_improved() && st(i) != ST_OFF; }
/* a member who shoots while it can: an Archer, one holding where sent, a Skirmisher before the enemies are close */
static int shooter(int i) { int s0 = st(i); return s0 == ST_ARCHER || s0 == ST_HELD || s0 == ST_SKIRMISHER; }
/* 1 shoot, 0 melee, -1 no missile weapon in hand: what a click on an enemy makes member m do
   (battle.click_attack), as the member's way has it (Manual: as the game's improved click, a shot) */
int orders_member_mode(int m)
{
    if (m < 0 || m > 4) return -1;
    int wm = (s8)g_rb(CBF(m, CB_MISSILE));
    if (wm < 0 || wm > 62) return -1;
    if (st(m) == ST_OFF) return combat_improved();
    return shooter(m);
}
/* the way member m fights now, for the party panel (ui/partypanel.cpp): with the player's order of this
   fight in it; *by_order 1 when an order of the player's set it */
int orders_member_style(int m, int *by_order)
{
    if (by_order) *by_order = m >= 0 && m < 5 && ov[m] != OV_NONE && style_of(m) != ST_OFF;
    return st(m);
}
/* a member may walk on its own (step aside for a line, go to help): not one holding where sent; one whose way is Manual
   only to carry out the player's order to shoot (a spot with a line to the foe it was told to shoot) */
int orders_may_move(int i)
{
    int s0 = st(i);
    if (s0 == ST_OFF) return i >= 0 && i < 5 && ov[i] == OV_SHOOT;
    return s0 != ST_HELD;
}

/* the melee attack member i opens with against foe t: Vulnerable (0x0a: 1d4 more penetration, fewer
   and less guarded blows: s014 18bf hit_damage, attack_rate) when the default weapon's penetration
   (far 20a5:76a8, low nibble) does not get through t's body armour (its hardness, far 20a5:781e by
   +0x4b) but would with the extra 1d4 (battle.auto_vulnerable); else Std Attack */
int ways_melee_kind(int i, int t)
{
    if (!combat_improved() || t < 5 || t >= 13) return STD_ATTACK;
    int w = (s8)g_rb(CBF(i, CB_MELEE)), at = g_rb(CBF(t, CB_BODY_ARMOR));
    if (w < 0 || w > 62 || at == 0xff) return STD_ATTACK;
    int pen = rb(G_FAR20A5, (u16)(0x76a8 + w)) & 15, h = rb(G_FAR20A5, (u16)(0x781e + at));
    return pen <= h && pen + 4 >= h ? 0x0a : STD_ATTACK;
}
/* a foe within about five paces (a fighter is some 8 pixels wide; the field's y is foreshortened by
   half): the lines have met (note_lines), a friend set upon close by (rescue_target), a foe coming at a
   shooter (back_to_bow waits) */
int ways_near_foe(int i, int m)
{
    int dx = (s16)g_rw(CBF(m, CB_X)) - (s16)g_rw(CBF(i, CB_X));
    int dy = 2 * ((s16)g_rw(CBF(m, CB_Y)) - (s16)g_rw(CBF(i, CB_Y)));
    return (long)dx * dx + (long)dy * dy <= (long)NEAR_PX * NEAR_PX;
}
static int volley_on(void) { return combat_improved(); }
static u16 bow_given[5], bow_hold[5];   /* back_to_bow: when the bow was last given back; the game's way let be since */
static unsigned char bow_flips[5];
static u16 idle_since[5];             /* melee_hook: since when a shooter stood idle with its order */
static signed char told[5];           /* a click of the player's taken in this order (a foe, a spot): click_hook, place_hook */
static signed char pt[5] = { -1, -1, -1, -1, -1 };   /* the foe the player sent the member at hand to hand */
static void leave_hook(void)                    /* s014 19cd:0738 order_mode_leave(): the order chosen applied */
{
    int i = (s8)g_rb(D_SELECTED), act = (s16)g_rw(D_ORDER_ACTION);
    int was_told = i >= 0 && i < 5 && told[i];
    for (int k = 0; k < 5; k++) told[k] = 0;
    hook_call_original();
    if (ns_unwinding || i < 0 || i > 4) return;
    /* an order given (an action chosen in the menu, or a click on a foe or a spot): the member's own way
       gives way to it for the rest of the fight. (Choosing a member and going on without an order - Space,
       another member - is no order: the order it has from its way is not the player's) */
    if (act == -1 && !was_told) return;
    int f = (s16)g_rw(CBF(i, CB_ORDER)), k = act != -1 ? act : f;   /* (an attack on a foe far off: walking to it, the attack queued) */
    int was = ov[i];
    if (k == MISSILE) ov[i] = OV_SHOOT;
    else if (k == STD_ATTACK || k == 0x0a || k == 0x22 || k == 6) ov[i] = OV_MELEE;
    else if (k == 1 || k == 0x400) ov[i] = OV_HOLD;                 /* (walk, flee, halt, a door ...: held there after) */
    pt[i] = ov[i] == OV_MELEE ? (s8)g_rb(CBF(i, CB_TARGET)) : -1;
    if (ov[i] != was && getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d's order (%#x): %s for the rest of the fight\n", i, k, ov[i] == OV_SHOOT ? "shooting" : ov[i] == OV_MELEE ? "hand to hand" : "holding");
    bow_given[i] = bow_hold[i] = 0; bow_flips[i] = 0;
}     /* member i at foe t hand to hand (beside it, or walking to it), its melee weapon drawn; the kind of attack as its way has it */
static signed char noway[5] = { -1, -1, -1, -1, -1 };   /* a foe member i found no way to, and when: left alone a while */
static u16 noway_at[5];
static int no_way_to(int i, int j) { return noway[i] == j && noway_at[i] && (u16)(g_rw(GV_combat_clock) - noway_at[i]) < 90; }
static void to_melee(int i, int t, int beside)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    if (st(i) == ST_SKIRMISHER && !committed[i]) { committed[i] = 1; fprintf(stderr, "orders: member %d (Skirmisher) fights hand to hand from now on\n", i); }
    ns_call(GF_weapon_ready, 3, i, (s8)g_rb((u16)(c + CB_MELEE)), 0);
    g_wb((u16)(c + CB_TARGET), (u8)t);
    int kind = ways_melee_kind(i, t);
    if (beside) { g_ww((u16)(c + CB_ORDER), (u16)kind); g_wb((u16)(GV_cbt_action + i), 7); }
    else {
        g_ww((u16)(c + CB_TO_X), g_rw(CBF(t, CB_X))); g_ww((u16)(c + CB_TO_Y), g_rw(CBF(t, CB_Y)));
        if (ns_call(GF_path_find, 1, i) != 1) {     /* (no way to it now: the member is free, and goes at another, or at this one when it comes) */
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            noway[i] = (signed char)t; noway_at[i] = g_rw(GV_combat_clock); if (!noway_at[i]) noway_at[i] = 1;
            fprintf(stderr, "orders: member %d finds no way to %d: another for now\n", i, t);
            return;
        }
        ns_call(GF_path_give, 1, i);
        g_wb((u16)(GV_cbt_moving + i), 1); g_wb((u16)(GV_cbt_path_step + i), 0);
        g_ww((u16)(c + CB_ORDER), 1);
        ns_call(GF_order_queue, 2, i, kind);
    }
    g_wb((u16)(GV_cbt_state + i), 0);
    fprintf(stderr, "orders: member %d fights %d hand to hand (%s)\n", i, t, beside ? "beside it" : "walking to it");
}
/* a clear line of fire from member i to foe j, where i stands (line_blocked, s014 1d69:000c, as the shot is tested) */
int ways_line_clear(int i, int j)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
    g_ww(GV_los_x0, g_rw((u16)(c + CB_X))); g_ww(GV_los_y0, g_rw((u16)(c + CB_Y)));
    g_ww(GV_los_x1, g_rw(CBF(j, CB_X))); g_ww(GV_los_y1, g_rw(CBF(j, CB_Y)));
    int blocked = ns_call(GF_line_blocked, 3, i, j, 0x80);
    g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
    return !blocked;
}
static signed char shot_at[5] = { -1, -1, -1, -1, -1 };
static int shot_target(int i, int *clear_out);
static int has_ammo(int i, int wm);
static void to_melee(int i, int t, int beside);
static void retarget_shooters(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int ord = g_rw((u16)(c + CB_ORDER)), t = (s8)g_rb((u16)(c + CB_TARGET));
        if (g_rb((u16)(c + CB_STATUS)) != 1 || !automated(i)) { shot_at[i] = -1; continue; }
        if (getenv("DL_SHOT_LOG") && ord != 0) fprintf(stderr, "shots: %d order %#x target %d\n", i, ord, t);
        if (ord == MISSILE && t >= 5 && t < 13 && g_rb(CBF(t, CB_STATUS)) == 1) { shot_at[i] = (signed char)t; continue; }
        if (ord == MISSILE && shot_at[i] < 5) shot_at[i] = 5;                 /* (the order kept, its mark gone (-1, or fallen): a new mark now, below) */
        if (shot_at[i] < 5) continue;
        {   /* its last loosed (the game drops the order, its mark still standing): the melee weapon, at the nearest foe,
               once the shot is done (not for an order of the player's since) */
            int wm = (s8)g_rb((u16)(c + CB_MISSILE)), act = g_rb((u16)(GV_cbt_action + i));
            if ((ord == 0 || ord == MISSILE) && wm >= 0 && wm <= 62 && !has_ammo(i, wm)) {
                if (act != 0 && act != 7) continue;                          /* (still loosing it: the next tick) */
                shot_at[i] = -1;
                g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);   /* (the melee weapon as its way has it: idle_fighters) */
                if (st(i) == ST_SKIRMISHER) committed[i] = 1;
                fprintf(stderr, "orders: member %d has nothing left to shoot: %s\n", i, st(i) == ST_HELD ? "holds where it stands" : "the melee weapon");
                continue;
            }
        }
        if (ord != MISSILE && g_rb((u16)(D_CBT + shot_at[i] * 0x80 + 0x14)) == 1) { shot_at[i] = -1; continue; }   /* (its mark stands: the order changed for another reason) */
        /* its mark has just fallen: whatever the game's AI gave it instead (to walk at someone, to
           fight hand to hand), it shoots again */
        { int wm = (s8)g_rb((u16)(c + CB_MISSILE)); if (wm < 0 || wm > 62 || !shooter(i)) { shot_at[i] = -1; continue; } }   /* (no missile weapon now; or hand to hand from now on) */
        { int beside = 0; for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) beside = 1;
          if (beside) { shot_at[i] = -1; continue; } }                       /* (a foe beside it: hand to hand first, the bow after: back_to_bow) */
        /* the nearest foe on its floor with a clear line from where it stands; none: the nearest (the
           stepping aside finds a spot to shoot it from, mods/battlerules.c) */
        int best_clear = 0, best = shot_target(i, &best_clear);
        if (ns_unwinding) return;
        shot_at[i] = -1;
        if (best >= 0 && !best_clear && st(i) == ST_HELD) best = -1;   /* (one holding where sent does not step aside: none it can see, it waits for one) */
        if (best < 0) {
            if (st(i) == ST_HELD) { g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_moving + i), 0); g_wb((u16)(GV_cbt_action + i), 7); }   /* (not the game's walk at someone) */
            continue;
        }
        g_ww((u16)(c + CB_ORDER), MISSILE); g_wb((u16)(c + CB_TARGET), (u8)best); g_wb((u16)(GV_cbt_state + i), 0);
        if (g_rb((u16)(GV_cbt_action + i)) == 0) g_wb((u16)(GV_cbt_action + i), 7);   /* (idle: "ready", so the shooting starts) */
        shot_at[i] = (signed char)best;
        fprintf(stderr, "orders: member %d's mark fell: now shoots at %d (%s)\n", i, best, best_clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* the foe a shooter turns to: one with a clear line from where it stands, first; of those, the nearest, the
   enemy's own shooters counted nearer (their bows and guns hurt the party most from afar), a foe another of
   the party already shoots at farther (each shooter a foe of its own), and one fighting the party hand to
   hand farther too (the melee is in the way); none with a line: the same of all */
static int shot_target(int i, int *clear_out)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    int best = -1, best_clear = 0; long bd = 0;
    int x = (s16)g_rw((u16)(c + CB_X)), y = (s16)g_rw((u16)(c + CB_Y));
    for (int j = 5; j < 13; j++) {
        u16 d = (u16)(D_CBT + j * 0x80);
        if (g_rb((u16)(d + CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != g_rb((u16)(GV_cbt_floor + i))) continue;
        long dx = (s16)g_rw((u16)(d + CB_X)) - x, dy = ((s16)g_rw((u16)(d + CB_Y)) - y) * 2;
        float dd = sqrtf((float)(dx * dx + dy * dy));
        int wm = (s8)g_rb((u16)(d + CB_MISSILE));
        if (wm >= 0 && wm <= 62) dd -= 60;
        for (int p = 0; p < 5; p++) {
            if (p == i || g_rb(CBF(p, CB_STATUS)) != 1) continue;
            if (g_rw(CBF(p, CB_ORDER)) == MISSILE && (s8)g_rb(CBF(p, CB_TARGET)) == j) dd += 70;
            if (rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))) dd += 45;
        }
        int clear = ways_line_clear(i, j);
        if (ns_unwinding) return -1;
        if (best < 0 || clear > best_clear || (clear == best_clear && dd < bd)) { best = j; bd = (long)dd; best_clear = clear; }
    }
    if (clear_out) *clear_out = best_clear;
    return best;
}
/* a foe within a few paces (near_foe_of) fighting one of the party hand to hand: the shooter puts
   the bow away and joins in rather than stand by and loose into the melee from close by. Of
   several, the one beside the friend most set upon (the most foes, then the least Endurance), then
   the one fewest of the party fight, then the nearest. Not a foe two of the party already fight or
   walk to, while the shooter has another foe to shoot at: a third and fourth helper only crowded
   round it, one walking round the crowd for a free side and ending beside another foe with its
   order still on this one (a player's battle, RC32) - then it goes on shooting. -1 none */
static int other_mark(int i, int skip);
static int rescue_target(int i)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), best = -1, best_on = 0; long bs = 0;
    for (int j = 5; j < 13; j++) {
        if (g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl || !ways_near_foe(i, j) || no_way_to(i, j)) continue;
        long dx = (s16)g_rw(CBF(j, CB_X)) - (s16)g_rw(CBF(i, CB_X));
        long dy = 2 * ((s16)g_rw(CBF(j, CB_Y)) - (s16)g_rw(CBF(i, CB_Y)));
        long dd = dx * dx + dy * dy;
        int on_it = 0;
        long sc = -1;
        for (int p = 0; p < 5; p++) {
            if (p == i || g_rb(CBF(p, CB_STATUS)) != 1 || !rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))) continue;
            on_it++;
            int n = 0;
            for (int k = 5; k < 13; k++) if (g_rb(CBF(k, CB_STATUS)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + p * 13 + k))) n++;
            int cur = 0, mx = 0; gm_member_attr(p, 0, &cur, &mx);           /* (Endurance: the share left) */
            long v = (long)n * 1000 + (mx > 0 ? 999 - 999L * (cur < 0 ? 0 : cur > mx ? mx : cur) / mx : 0);
            if (v > sc) sc = v;
        }
        for (int p = 0; p < 5; p++)                                           /* (and those on their way to it) */
            if (p != i && g_rb(CBF(p, CB_STATUS)) == 1 && !rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))
                && (s8)g_rb(CBF(p, CB_TARGET)) == j && g_rb((u16)(GV_cbt_moving + p)) && g_rw(CBF(p, CB_ORDER)) != MISSILE) on_it++;
        if (sc < 0) continue;
        int crowded = on_it >= 2;
        sc = (sc * 10 - on_it) * 10000 - dd / 32;   /* (fits a 32-bit long: sc under 5000, dd / 32 under 10000) */
        if (best < 0 || (best_on >= 2 && !crowded) || ((best_on >= 2) == crowded && sc > bs)) { best = j; bs = sc; best_on = on_it; }
    }
    if (best >= 0 && best_on >= 2 && other_mark(i, best) >= 0) {             /* (crowded: the bow, at another foe) */
        static signed char said[5] = { -1, -1, -1, -1, -1 };
        if (said[i] != best) { said[i] = (signed char)best; fprintf(stderr, "orders: member %d leaves %d to the %d of the party on it: shoots at another\n", i, best, best_on); }
        return -1;
    }
    return best;
}
/* another foe member i could shoot at instead: standing on its floor, fighting none of the party, a
   clear line to it (the volley's own test). -1 none */
static int other_mark(int i, int skip)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), wm = (s8)g_rb(CBF(i, CB_MISSILE));
    if (wm < 0 || wm > 62 || !has_ammo(i, wm)) return -1;
    for (int j = 5; j < 13; j++) {
        if (j == skip || g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl) continue;
        int engaged = 0;
        for (int p = 0; p < 5; p++) if (g_rb(CBF(p, CB_STATUS)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))) engaged = 1;
        if (!engaged && ways_line_clear(i, j)) return j;
    }
    return -1;
}
static int has_ammo(int i, int wm)              /* member i carries what its missile weapon looses (a thrown weapon: itself) */
{
    int t = rb(G_FAR20A5, (u16)(0x7822 + wm)), n = g_rb((u16)(G_MEMBER(i) + G_M_NITEMS));
    if (!t) return 1;
    for (int k = 0; k < n && k < 64; k++) { u16 o = G_ITEM(i, k); if (rb(G_FAR2E38, (u16)(o + 2)) == t && rb(G_FAR2E38, (u16)(o + 4)) > 0) return 1; }
    return 0;
}
static int adj(int i, int j) { return rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j)) != 0; }   /* (within reach of a blow: ai 1db7:09ae) */
static int foes_beside(int i)
{
    int n = 0;
    for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && adj(i, j)) n++;
    return n;
}
static float dist(int a, int b)                 /* (pixels; the field's rows are half as tall) */
{
    float dx = (float)((s16)g_rw(CBF(b, CB_X)) - (s16)g_rw(CBF(a, CB_X)));
    float dy = 2.0f * (float)((s16)g_rw(CBF(b, CB_Y)) - (s16)g_rw(CBF(a, CB_Y)));
    return sqrtf(dx * dx + dy * dy);
}
static void ready_shot(int i, int t)              /* as the game readies it for Use Missile (s014 19cd order_mode_leave) */
{
    u16 c = (u16)(D_CBT + i * 0x80);
    ns_call(GF_weapon_ready, 3, i, (s8)g_rb((u16)(c + CB_MISSILE)), 0x80);
    g_wb((u16)(c + CB_TARGET), (u8)t); g_ww((u16)(c + CB_ORDER), MISSILE);
    g_wb((u16)(GV_cbt_moving + i), 0); g_wb((u16)(GV_cbt_state + i), 0);
    g_ww((u16)(GV_cbt_flags + 2 * i), (u16)(g_rw((u16)(GV_cbt_flags + 2 * i)) | 2));
    g_ww((u16)(GV_cbt_engage_time + 2 * i), g_rw(GV_combat_clock));
    g_wb((u16)(GV_cbt_action + i), 7);              /* "ready", as a click's order leaves it: the game then reloads, aims and looses (from 0 it never starts) */
}

/* the lines have met: a foe beside one of the party, or within a few paces of one, at some time in this
   fight. Until then a Man-at-arms stays with the party while the others shoot */
static int lines_met;
/* one of the party shoots, or will: Use Missile, or a shooter's way with something to loose. None: the
   Men-at-arms do not wait for the enemy */
static int party_shoots(void)
{
    for (int p = 0; p < 5; p++) {
        if (g_rb(CBF(p, CB_STATUS)) != 1) continue;
        if (g_rw(CBF(p, CB_ORDER)) == MISSILE) return 1;
        int wm = (s8)g_rb(CBF(p, CB_MISSILE));
        if (automated(p) && shooter(p) && wm >= 0 && wm <= 62 && has_ammo(p, wm)) return 1;
    }
    return 0;
}
/* how many of the party (not skip) fight foe j: beside it, or on their way to it to fight */
static int party_on(int j, int skip)
{
    int n = 0;
    for (int p = 0; p < 5; p++) {
        if (p == skip || g_rb(CBF(p, CB_STATUS)) != 1) continue;
        if (adj(p, j)) n++;
        else if ((s8)g_rb(CBF(p, CB_TARGET)) == j && g_rb((u16)(GV_cbt_moving + p)) && g_rw(CBF(p, CB_ORDER)) != MISSILE) n++;
    }
    return n;
}
/* the foe member i goes at hand to hand, as its way has it; -1 none. A foe beside it always first (its
   own target first of those). Then:
     PICK_BESIDE (Archer, one holding where sent, a shooter with something to loose): none.
     PICK_FOOT (Man-at-arms): once the lines have met (or no one of the party shoots), a foe fighting a
       friend that fewer than two of the party fight (two on one), else a foe no one fights, the nearest;
       else the nearest of the rest, the least crowded.
     PICK_CHARGE (Charger): from the start, the enemy's shooters no one goes at yet, then the nearest
       foe no one fights, then the least crowded. */
enum { PICK_BESIDE, PICK_FOOT, PICK_CHARGE };
static int melee_pick(int i, int how, int *beside_out)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), cur = (s8)g_rb(CBF(i, CB_TARGET)), best = -1, best_beside = 0;
    int open = how != PICK_FOOT || lines_met || !party_shoots();
    float bs = 0;
    for (int j = 5; j < 13; j++) {
        if (g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl) continue;
        int beside = adj(i, j);
        float d = dist(i, j), s;
        if (beside) s = 100000.0f + (j == cur ? 50.0f : 0.0f) - d;
        else if (how == PICK_BESIDE || no_way_to(i, j)) continue;
        else {
            if (!open) continue;
            int n = party_on(j, i), on_friend = 0;
            for (int p = 0; p < 5; p++) if (p != i && g_rb(CBF(p, CB_STATUS)) == 1 && adj(p, j)) on_friend = 1;
            int wm = (s8)g_rb(CBF(j, CB_MISSILE)), shoots = wm >= 0 && wm <= 62;
            if (how == PICK_FOOT) s = on_friend && n < 2 ? 30000.0f : n == 0 ? 20000.0f : 10000.0f - n * 500.0f;
            else s = n == 0 && shoots ? 30000.0f : n == 0 ? 20000.0f : 10000.0f - n * 500.0f;
            s -= d;
        }
        if (best < 0 || s > bs) { best = j; bs = s; best_beside = beside; }
    }
    if (beside_out) *beside_out = best_beside;
    return best;
}
/* a member with a missile weapon left without orders (the battle let run before the player gave any,
   or an order carried out), whose way is to shoot (Archer, Skirmisher, one holding where sent): it opens fire at a foe
   it can see (shot_target) rather than stand with the bow until a foe reaches it - the volley before the
   lines meet. A Skirmisher with a friend set upon a few paces off goes to help instead (and fights hand
   to hand from then on); one holding where sent shoots only what it sees from there. Not one told to hold
   (the game's Hold), walking, with a foe beside it (idle_fighters), or with nothing to loose */
static u16 hold_try[5];
static void idle_shooters(void)
{
    if (!combat_improved()) return;
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int act = g_rb((u16)(GV_cbt_action + i));             /* (0: the battle's start; 7: "ready", an order carried out. Halt keeps its order, 0x400) */
        if (g_rb((u16)(c + CB_STATUS)) != 1 || g_rw((u16)(c + CB_ORDER)) != 0 || g_rb((u16)(GV_cbt_moving + i)) || (act != 0 && act != 7)) continue;
        if (!automated(i) || !shooter(i) || g_rb((u16)(GV_cbt_mods + 0x12 * i + 9)) == 1) continue;
        int wm = (s8)g_rb((u16)(c + CB_MISSILE)), s0 = st(i);
        if (wm < 0 || wm > 62 || foes_beside(i)) continue;
        if (s0 == ST_SKIRMISHER) {
            int r = rescue_target(i);                                       /* (a friend fighting a few paces off: help it, not the bow) */
            if (ns_unwinding) return;
            if (r >= 0) { to_melee(i, r, 0); if (ns_unwinding) return; fprintf(stderr, "orders: member %d had no orders: goes to help: %d fights one of the party a few paces off\n", i, r); continue; }
        }
        if (s0 == ST_HELD && hold_try[i] && (u16)(now - hold_try[i]) < 20) continue;
        if (!ns_call(GF_can_fire, 1, i)) { if (ns_unwinding) return; continue; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        if (t < 0) continue;
        if (s0 == ST_HELD && !clear) { hold_try[i] = now ? now : 1; continue; }   /* (no line from where it stands: it waits for one) */
        ready_shot(i, t);
        fprintf(stderr, "orders: member %d had no orders: opens fire at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* a member left without orders who does not shoot now: it fights as its way has it (melee_pick) - a foe
   beside it; a friend's fight or a free foe once the lines meet (Man-at-arms); the enemy's shooters and
   free foes from the start (Charger); only a foe beside it (Archer, one holding where sent). Not one told
   to hold (the game's Hold); Halt keeps its own order (0x400) and is let be */
static void idle_fighters(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int act = g_rb((u16)(GV_cbt_action + i));
        if (g_rb((u16)(c + CB_STATUS)) != 1 || g_rw((u16)(c + CB_ORDER)) != 0 || g_rb((u16)(GV_cbt_moving + i)) || (act != 0 && act != 7)) continue;
        if (!automated(i) || g_rb((u16)(GV_cbt_mods + 0x12 * i + 9)) == 1) continue;                  /* (told to hold) */
        int wm = (s8)g_rb((u16)(c + CB_MISSILE)), s0 = st(i);
        int armed = wm >= 0 && wm <= 62 && has_ammo(i, wm);
        int how = PICK_FOOT;
        switch (s0) {
        case ST_HELD: how = PICK_BESIDE; break;
        case ST_ARCHER: case ST_SKIRMISHER: how = armed ? PICK_BESIDE : PICK_FOOT; break;
        case ST_CHARGER: how = PICK_CHARGE; break;
        }
        int beside = 0, t = melee_pick(i, how, &beside);
        if (ns_unwinding) return;
        if (t < 0) continue;
        to_melee(i, t, beside);
        if (ns_unwinding) return;
        fprintf(stderr, "orders: member %d had no orders: joins the fight against %d (%s, as %s)\n", i, t, beside ? "beside it" : "walking to it", style_name(s0));
    }
}
/* a member ordered to fight a foe that has fallen, or that is not beside it while another is (it walked
   to its foe and found the place taken: a player's battle, RC32, a member standing before a foe that
   fought a friend, its order still on one that fell): the foe beside it; with none, the order ends,
   and the member is free for the next (idle_fighters, idle_shooters, back_to_bow). The foe the player
   sent it at, still standing: the game's way to it */
static void stale_melee(void)
{
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int f = (s16)g_rw((u16)(c + CB_ORDER));
        if (!(f == STD_ATTACK || f == 0x0a || f == 0x22 || f == 6)) continue;
        if (g_rb((u16)(c + CB_STATUS)) != 1 || !automated(i)) continue;
        int t = (s8)g_rb((u16)(c + CB_TARGET));
        int alive = t >= 5 && t < 13 && g_rb(CBF(t, CB_STATUS)) == 1;
        if (st(i) == ST_HELD && (!alive || !adj(i, t))) {             /* (one holding where sent does not go after a foe that stepped back: the game would walk it there) */
            int b = -1;
            for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && adj(i, j)) { b = j; break; }
            g_wb((u16)(GV_cbt_moving + i), 0);
            if (b >= 0) { to_melee(i, b, 1); if (ns_unwinding) return; continue; }
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            fprintf(stderr, "orders: member %d (held where sent) has no foe beside it: stands\n", i);
            continue;
        }
        if (g_rb((u16)(GV_cbt_moving + i))) continue;
        if (alive && adj(i, t)) continue;                                           /* (beside its foe: fighting) */
        if (alive && t == pt[i]) continue;                                          /* (the player's own foe, still standing: the game's way to it) */
        int b = -1;
        for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && adj(i, j)) { b = j; break; }
        if (b >= 0) {
            to_melee(i, b, 1);
            if (ns_unwinding) return;
            fprintf(stderr, "orders: member %d's foe %d %s: fights %d beside it\n", i, t, alive ? "is not beside it" : "fell", b);
        } else if (!alive) {
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            fprintf(stderr, "orders: member %d's foe %d fell: free for the next\n", i, t);
        }
    }
}
/* hand to hand with the missile weapon in hand to go back to (by our doing, or the game's: its AI turns
   a shooter set upon to Parry or a Std Attack): an Archer (and one the player sent to shoot) or one
   holding where sent, with no foe beside it now, takes up the missile weapon again, at the next foe; a Skirmisher fights
   hand to hand from then on (the foes are close) */
static void back_to_bow(void)
{
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int f = (s16)g_rw((u16)(c + CB_ORDER)), wm = (s8)g_rb((u16)(c + CB_MISSILE));
        int melee = f == STD_ATTACK || f == 0x0a || f == 0x22 || f == 6;
        if (!melee || !automated(i) || g_rb((u16)(c + CB_STATUS)) != 1 || wm < 0 || wm > 62 || g_rb((u16)(GV_cbt_moving + i))) continue;
        int s0 = st(i);
        if (s0 == ST_SKIRMISHER) { committed[i] = 1; fprintf(stderr, "orders: member %d (Skirmisher) fights hand to hand from now on\n", i); continue; }
        if (s0 != ST_ARCHER && s0 != ST_HELD) continue;
        if (!has_ammo(i, wm) || foes_beside(i)) continue;
        /* a foe within a few paces coming at it: the fight is about to reach it, the bow would only be
           put away again (each change of weapon starts the reloading over: a shooter switched back and
           forth never shoots) */
        { int coming = 0;
          for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && ways_near_foe(i, j) && (s8)g_rb(CBF(j, CB_TARGET)) == i) coming = 1;
          if (coming) continue; }
        /* the game took the bow away again just after it was given back: let the game have its way
           for a while (its AI sees a fight the member is in) */
        { u16 now = g_rw(GV_combat_clock);
          if (bow_given[i] && (u16)(now - bow_given[i]) < 90) { if (++bow_flips[i] >= 2) { bow_hold[i] = now ? now : 1; bow_flips[i] = 0; } continue; }
          if (bow_hold[i] && (u16)(now - bow_hold[i]) < 600) continue;
          bow_hold[i] = 0; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        if (t < 0 || (s0 == ST_HELD && !clear)) {                    /* (nothing it can shoot: the order ends; it shoots when it can, idle_shooters) */
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            continue;
        }
        ready_shot(i, t);
        bow_given[i] = g_rw(GV_combat_clock); if (!bow_given[i]) bow_given[i] = 1;
        fprintf(stderr, "orders: member %d has no foe beside it now: the missile weapon again, at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* test (uattack:i,t): member i told to fight t hand to hand, as the player's order gives it (Std Attack;
   the bow is not taken up again for it) */
void orders_test_attack(int i, int t)
{
    if (i < 0 || i > 4 || t < 5 || t > 12) return;
    to_melee(i, t, adj(i, t));
    ov[i] = OV_MELEE; pt[i] = (signed char)t;
    fprintf(stderr, "orders: test: %d attacks %d hand to hand\n", i, t);
}
/* test (uhold:i,on): member i told to hold its ground (as the game's Hold modifier) */
void orders_test_hold(int i, int on)
{
    if (i < 0 || i > 4) return;
    g_wb((u16)(GV_cbt_mods + 0x12 * i + 9), (u8)(on ? 1 : 0));
    fprintf(stderr, "orders: test: %d %s\n", i, on ? "holds" : "no longer holds");
}
/* test (ushoot:i,t): member i told to shoot at t, as the order's click gives it */
void orders_test_shoot(int i, int t)
{
    if (i < 0 || i > 4 || t < 5 || t > 12) return;
    int wm = (s8)g_rb(CBF(i, CB_MISSILE));
    if (wm < 0 || wm > 62) { fprintf(stderr, "orders: test: %d has no missile weapon\n", i); return; }
    ready_shot(i, t);
    ov[i] = OV_SHOOT; pt[i] = -1;
    fprintf(stderr, "orders: test: %d shoots at %d\n", i, t);
}
/* test (uheld:i): member i holds where it stands for the rest of the fight, as after the player's walk or
   Halt order (leave_hook) */
void orders_test_held(int i)
{
    if (i < 0 || i > 4) return;
    ov[i] = OV_HOLD; pt[i] = -1;
    fprintf(stderr, "orders: test: %d holds where it was sent\n", i);
}
/* member m's way chosen anew (the party panel): what the player's orders made of it in this fight is
   forgotten, the new way from now on */
void orders_style_changed(int m)
{
    if (m < 0 || m > 4) return;
    ov[m] = OV_NONE; committed[m] = 0; pt[m] = -1;
}
/* a Parry, Berserk or Vulnerable order kept: the game turns a member who is set upon on the way
   to its foe (or whose foe falls) to a Std Attack on whoever it fights; the kind the player chose
   is kept for that fight too, until the member is given another order */
static int kept_kind[5] = { -1, -1, -1, -1, -1 };
/* every frame (runtime/platform_sdl.c): a battle begun (the battle's screen, not back from a sheet or
   Enemy Info opened in it: the game's in_battle flag stays set), what this file remembers of the members is
   cleared - the last battle's orders, kinds kept, marks, bow switches and ways given way to orders are
   not this one's (a member sent hand to hand there opened fire here no more) */
void battlerules_battle_begins(void), pathfix_battle_begins(void);
void orders_frame(void)
{
    static int was;
    int in = game_mode() == GM_BATTLE || (was && gm_in_battle());   /* (a sheet or Enemy Info opened in the battle is not a new battle) */
    if (in && !was) {
        for (int i = 0; i < 5; i++) {
            bow_given[i] = bow_hold[i] = 0; bow_flips[i] = 0;
            idle_since[i] = 0; shot_at[i] = -1; kept_kind[i] = -1; told[i] = 0;
            ov[i] = OV_NONE; committed[i] = 0; pt[i] = -1; hold_try[i] = 0; noway[i] = -1;
        }
        lines_met = 0;
        battlerules_battle_begins(); pathfix_battle_begins();   /* (every file's memory of a battle: forgotten at the next) */
        if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: a battle begins\n");
    }
    was = in;
}
static void keep_kinds(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        if (kept_kind[i] < 0) continue;
        u16 c = (u16)(D_CBT + i * 0x80);
        if (g_rb((u16)(c + CB_STATUS)) != 1) { kept_kind[i] = -1; continue; }
        int f = (s16)g_rw((u16)(c + CB_ORDER));
        if (f == STD_ATTACK) { g_ww((u16)(c + CB_ORDER), (u16)kept_kind[i]); if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d keeps its %#x\n", i, kept_kind[i]); }
        else if (f == MISSILE || f == 0) kept_kind[i] = -1;                 /* (another kind of order now) */
    }
}
static void note_lines(void)
{
    if (lines_met) return;
    for (int i = 0; i < 5; i++) {
        if (g_rb(CBF(i, CB_STATUS)) != 1) continue;
        for (int j = 5; j < 13; j++)
            if (g_rb(CBF(j, CB_STATUS)) == 1 && g_rb((u16)(GV_cbt_floor + j)) == g_rb((u16)(GV_cbt_floor + i)) && (adj(i, j) || ways_near_foe(i, j))) {
                lines_met = 1;
                fprintf(stderr, "orders: the lines meet: %d is a few paces from %d\n", j, i);
                return;
            }
    }
}
static void melee_hook(void)                    /* s014 0e76:2a4a melee_exchanges() */
{
    hook_call_original();
    if (ns_unwinding || g_rb(GV_ai_paused)) return;
    { extern int arena_log_every; void arena_tick(void); if (arena_log_every) arena_tick(); }   /* (the test arena's log: runtime/testcmds.c) */
    keep_kinds();
    if (!combat_improved()) return;
    note_lines();
    retarget_shooters();
    if (ns_unwinding) return;
    {   /* a shooter standing idle with its order (Use Missile, action 0, not walking): made ready */
        u16 now = g_rw(GV_combat_clock);
        for (int i = 0; i < 5; i++) {
            u16 c = (u16)(D_CBT + i * 0x80);
            int t = (s8)g_rb((u16)(c + CB_TARGET));
            int idle = g_rb((u16)(c + CB_STATUS)) == 1 && g_rw((u16)(c + CB_ORDER)) == MISSILE && g_rb((u16)(GV_cbt_action + i)) == 0
                        && !g_rb((u16)(GV_cbt_moving + i)) && g_rb((u16)(GV_cbt_state + i)) == 0 && t >= 5 && t < 13 && g_rb(CBF(t, CB_STATUS)) == 1;
            if (!idle) { idle_since[i] = 0; continue; }
            if (!idle_since[i]) { idle_since[i] = now ? now : 1; continue; }
            if ((u16)(now - idle_since[i]) < 40) continue;
            g_wb((u16)(GV_cbt_action + i), 7); idle_since[i] = 0;
            fprintf(stderr, "orders: member %d stood idle with its bow: ready again, at %d\n", i, t);
        }
    }
    back_to_bow();
    if (ns_unwinding) return;
    stale_melee();
    if (ns_unwinding) return;
    idle_shooters();
    if (ns_unwinding) return;
    idle_fighters();
    if (ns_unwinding) return;
    if (!volley_on()) return;
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < 5; i++) {                   /* the shooters: a foe beside, a friend set upon, nothing left */
        u16 c = (u16)(D_CBT + i * 0x80);
        if (g_rb((u16)(c + CB_STATUS)) != 1 || g_rw((u16)(c + CB_ORDER)) != MISSILE || g_rb((u16)(GV_cbt_moving + i)) || !automated(i)) continue;
        if (g_rb((u16)(GV_cbt_mods + 0x12 * i + 9)) == 1) continue;           /* (told to hold) */
        int t = (s8)g_rb((u16)(c + CB_TARGET)), near = -1, s0 = st(i);
        for (int j = 5; j < 13; j++)
            if (g_rb(CBF(j, CB_STATUS)) == 1 && adj(i, j)) { if (near < 0 || j == t) near = j; }
        if (near >= 0) { to_melee(i, near, 1); if (ns_unwinding) return; continue; }   /* (a foe that reaches it: hand to hand, whatever its way) */
        if (s0 == ST_SKIRMISHER) {
            int r = rescue_target(i);                                       /* a friend fighting a few paces off: help it */
            if (ns_unwinding) return;
            if (r >= 0) { fprintf(stderr, "orders: member %d goes to help: %d fights one of the party a few paces off\n", i, r); to_melee(i, r, 0); if (ns_unwinding) return; continue; }
        }
        /* (a foe some paces off, coming: the shooter goes on shooting it, and fights hand to hand once it is beside) */
        if (t < 5 || t >= 13 || g_rb(CBF(t, CB_STATUS)) != 1) continue;
        int stt = g_rb((u16)(GV_cbt_state + i));
        if (stt == 2) {                                                     /* (nothing left to shoot) */
            int wm = (s8)g_rb((u16)(c + CB_MISSILE));
            if (s0 == ST_SKIRMISHER) { to_melee(i, t, 0); if (ns_unwinding) return; }
            else if (wm < 0 || wm > 62 || !has_ammo(i, wm)) { g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7); }   /* (its way's melee: idle_fighters) */
        } else if (stt == 1 && s0 == ST_HELD && (!hold_try[i] || (u16)(now - hold_try[i]) >= 30)) {   /* (a line blocked: one holding where sent does not step aside, it takes a foe it can see) */
            hold_try[i] = now ? now : 1;
            int clear = 0, b = shot_target(i, &clear);
            if (ns_unwinding) return;
            if (b >= 0 && clear && b != t) { g_wb((u16)(c + CB_TARGET), (u8)b); g_wb((u16)(GV_cbt_state + i), 0); fprintf(stderr, "orders: member %d (held where sent) has no line to %d: shoots at %d\n", i, t, b); }
        }
    }
}

/* what mods/orders.c tells of the player's clicks: the kind of attack chosen for an order (Parry, Berserk,
   Vulnerable: kept, keep_kinds), a click taken in the order being given (an order of the player's) */
void ways_keep_kind(int i, int kind) { if (i >= 0 && i < 5) kept_kind[i] = kind; }
void ways_told(int i) { if (i >= 0 && i < 5) told[i] = 1; }
void ways_untold(void) { for (int k = 0; k < 5; k++) told[k] = 0; }
void ways_init(void)
{
    hook_install(GF_melee_exchanges, melee_hook);
    hook_install(GF_order_mode_leave, leave_hook);
}
