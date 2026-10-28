/* orders.c - in a battle, a member who is not yet fighting can be sent at an enemy with a click
 * on it (F12 > Settings; the Full Original preset leaves the game's way).
 *
 * The game's way: click a member (order mode), choose an attack in the order menu (Std Attack,
 * Vulnerable, Berserk, Parry), then click the enemy. A member whose standing order is only to walk
 * (or nothing) takes a click on an enemy as a click on the ground under it, which is taken: "can't
 * go there" (s014 19cd:0dcc / 0ad4 skip the enemies' hot spots unless an attack is chosen). Here,
 * that click (and the pointer over the enemy) is taken as Std Attack (0x12, what the menu's first
 * attack sets) on that enemy, or as Use Missile (0x80) for a member better with the missile weapon
 * (battle.click_best); the rest is the game's own order (walk there and fight, or shoot). */
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
#include "losaim.h"
#include "styles.h"

#define D_ORDER_ACTION GV_order_action   /* int: the attack chosen in order mode (-1 none, 1 walk ...) */
#define D_SELECTED     GV_selected_member   /* char: the member given orders */
#define D_ORDER_MODE   GV_order_mode   /* char: 1 while a member is selected */
#define D_CBT          GV_combatants   /* struct cbt[13], 0x80 bytes: +4 x, +6 y, +0x10 order, +0x14 status */
#define D_HOT_CBT      GV_hotspot_cbt   /* char[]: hot spot -> combatant */
#define D_HOT_X0       0xd8f1   /* int[]: hot spot rectangles */
#define D_HOT_X1       0xd8d7
#define D_HOT_Y0       0xd8bd
#define D_HOT_Y1       0xd8a3
#define D_HOT_N        GV_hotspot_count   /* char: hot spots */
#define D_FOE_CLICKED  0xc7cc   /* char: a foe was taken in this order */
#define D_BOX_COLOUR   0xe1bc   /* int: the colour of the box around the order's target (D_c2bd: refused) */
#define D_REFUSED      0xc2bd
#define STD_ATTACK     0x12
#define MISSILE        0x80

int orders_click_attack = 1;

/* the combatant whose hot spot is topmost at (x, y) (the game's TOPMOST), or -1 */
static int topmost(int x, int y)
{
    int n = (s8)g_rb(D_HOT_N), m = -1, by = 0;
    for (int k = n - 1; k >= 0; k--) {
        int x0 = (s16)g_rw((u16)(D_HOT_X0 + 2 * k)), x1 = (s16)g_rw((u16)(D_HOT_X1 + 2 * k));
        int y0 = (s16)g_rw((u16)(D_HOT_Y0 + 2 * k)), y1 = (s16)g_rw((u16)(D_HOT_Y1 + 2 * k));
        if (x < x0 || x > x1 || y < y0 || y > y1) continue;
        int c = g_rb((u16)(D_HOT_CBT + k));
        if (c >= 13) continue;
        int cy = (s16)g_rw(CBF(c, CB_Y));
        if (by < cy) { m = c; by = cy; }
    }
    return m;
}

/* a foe whose hot spot is near (x, y): for a human figure 8 pixels wide and 25 high, from the
   feet up (s014 14f7; a figure is about 27 high, its arms and weapon reach beyond), so a click a
   little beside the figure, on the top of its head or at its feet - where a click meant for the
   foe lands as often as not - was taken for the ground beside it (the member walked there). 1
   and the centre of the nearest such spot, or 0. */
static int near_foe(int x, int y, int *cx, int *cy)
{
    int n = (s8)g_rb(D_HOT_N), best = -1; long bd = 1L << 30;
    for (int k = 0; k < n; k++) {
        int c = g_rb((u16)(D_HOT_CBT + k));
        if (c < 5 || c >= 13 || g_rb(CBF(c, CB_STATUS)) != 1) continue;
        int x0 = (s16)g_rw((u16)(D_HOT_X0 + 2 * k)), x1 = (s16)g_rw((u16)(D_HOT_X1 + 2 * k));
        int y0 = (s16)g_rw((u16)(D_HOT_Y0 + 2 * k)), y1 = (s16)g_rw((u16)(D_HOT_Y1 + 2 * k));
        if (x < x0 - 5 || x > x1 + 5 || y < y0 - 5 || y > y1 + 6) continue;    /* (beside, the head, the feet) */
        int mx = (x0 + x1) / 2, my = (y0 + y1) / 2;
        long d = (long)(x - mx) * (x - mx) + (long)(y - my) * (y - my);
        if (d < bd) { bd = d; best = k; *cx = mx; *cy = my; }
    }
    return best >= 0;
}
/* a click or the pointer in order mode just beside a foe, while the order is (or will be) an attack:
   moved onto the foe (the call's x, y), so the game takes the foe */
static void snap_to_foe(void)
{
    if (!orders_click_attack || config_get_int("original", 0) || g_rb(D_ORDER_MODE) != 1) return;
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int m = topmost(x, y);
    if (m >= 0) return;                                  /* (on someone already) */
    int act = (s16)g_rw(D_ORDER_ACTION), i = (s8)g_rb(D_SELECTED);
    if (i < 0 || i > 4) return;
    int f = (s16)g_rw(CBF(i, CB_ORDER));
    int attack = act == -1 ? (f == 0 || f == 1 || (f & 2) || f == 0x80) : ((act & 2) || act == 0x80);
    int cx, cy;
    if (attack && near_foe(x, y, &cx, &cy)) { ww(SS, (u16)(SP + 4), (u16)cx); ww(SS, (u16)(SP + 6), (u16)cy); }
}

/* ---------------------------------------------------------------- the members' ways (mods/styles.c)
 * Each member fights in a way of their own until the player gives them an order (style_of: Skirmisher,
 * Archer, Holdfast, Footman, Charger, Berserker, Bodyguard, Off). An order of the player's is theirs for
 * the rest of the fight in its manner (ov[]): sent hand to hand they go on hand to hand, sent to shoot
 * they go on shooting, sent somewhere or halted they hold there; Off is the original's way for that
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
    if (ov[i] == OV_HOLD) return ST_HOLDFAST;
    if (s0 == ST_SKIRMISHER && committed[i]) return ST_FOOTMAN;
    return s0;
}
static int automated(int i) { return combat_improved() && st(i) != ST_OFF; }
/* a member who shoots while it can: an Archer, a Holdfast, a Skirmisher before the enemies are close */
static int shooter(int i) { int s0 = st(i); return s0 == ST_ARCHER || s0 == ST_HOLDFAST || s0 == ST_SKIRMISHER; }
/* 1 shoot, 0 melee, -1 no missile weapon in hand: what a click on an enemy makes member m do
   (battle.click_attack), as the member's way has it (Off: as the game's improved click, a shot) */
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
/* a member may walk on its own (step aside for a line, go to help): not a Holdfast; one whose way is Off
   only to carry out the player's order to shoot (a spot with a line to the foe it was told to shoot) */
int orders_may_move(int i)
{
    int s0 = st(i);
    if (s0 == ST_OFF) return i >= 0 && i < 5 && ov[i] == OV_SHOOT;
    return s0 != ST_HOLDFAST;
}

/* the melee attack member i opens with against foe t: Vulnerable (0x0a: 1d4 more penetration, fewer
   and less guarded blows: s014 18bf hit_damage, attack_rate) when the default weapon's penetration
   (far 20a5:76a8, low nibble) does not get through t's body armour (its hardness, far 20a5:781e by
   +0x4b) but would with the extra 1d4 (battle.auto_vulnerable); else Std Attack */
static int melee_kind(int i, int t)
{
    if (!combat_improved() || t < 5 || t >= 13) return STD_ATTACK;
    int w = (s8)g_rb(CBF(i, CB_MELEE)), at = g_rb(CBF(t, CB_BODY_ARMOR));
    if (w < 0 || w > 62 || at == 0xff) return STD_ATTACK;
    int pen = rb(G_FAR20A5, (u16)(0x76a8 + w)) & 15, h = rb(G_FAR20A5, (u16)(0x781e + at));
    return pen <= h && pen + 4 >= h ? 0x0a : STD_ATTACK;
}
/* a foe within about five paces (a fighter is some 8 pixels wide; the field's y is foreshortened by
   half): a shooter fights it hand to hand rather than loose at it */
#define NEAR_PX 56
static int near_foe_of(int i, int m)
{
    int dx = (s16)g_rw(CBF(m, CB_X)) - (s16)g_rw(CBF(i, CB_X));
    int dy = 2 * ((s16)g_rw(CBF(m, CB_Y)) - (s16)g_rw(CBF(i, CB_Y)));
    return (long)dx * dx + (long)dy * dy <= (long)NEAR_PX * NEAR_PX;
}
/* the attack a click sends member i at foe m with: Use Missile (0x80) when the member opens by
   shooting (orders_member_mode), can loose the missile weapon (can_fire, s014 18bf:0c02) and the
   foe is not already within reach of a blow (far 20a5:7900, ai 1db7:09ae); else Std Attack */
static int line_clear(int i, int j);
static int best_attack(int i, int m)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    int melee = melee_kind(i, m);
    int mode = orders_member_mode(i);
    if (mode < 0) return melee;
    if (rb(G_FAR20A5, (u16)(0x7900 + i * 13 + m))) return melee;   /* beside it: hand to hand */
    if (mode == 0 || (!combat_improved() && near_foe_of(i, m))) {   /* (improved combat: a shooter shoots until the foe is beside it) */
        /* hand to hand, unless there is no way to the foe at all (hemmed in by others): then a shot,
           if the member has a missile weapon and something to loose */
        u16 tx = g_rw((u16)(c + CB_TO_X)), ty = g_rw((u16)(c + CB_TO_Y));
        g_ww((u16)(c + CB_TO_X), g_rw(CBF(m, CB_X))); g_ww((u16)(c + CB_TO_Y), g_rw(CBF(m, CB_Y)));
        int way = ns_call(GF_path_find, 1, i) == 1;
        g_ww((u16)(c + CB_TO_X), tx); g_ww((u16)(c + CB_TO_Y), ty);
        if (way) return melee;
        if (!line_clear(i, m)) return melee;                         /* (no way found, nor a line: the game's own attack, which walks round as it can) */
        if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d has no way to %d: a shot instead\n", i, m);
    }
    if (!ns_call(GF_can_fire, 1, i)) { if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d cannot shoot (nothing to loose)\n", i); return melee; }
    /* a clear line from where the member stands (line_blocked, s014 1d69:000c, as the order's pointer
       tests it): else the member closes in instead */
    u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
    g_ww(GV_los_x0, g_rw((u16)(c + CB_X))); g_ww(GV_los_y0, g_rw((u16)(c + CB_Y)));
    g_ww(GV_los_x1, g_rw(CBF(m, CB_X))); g_ww(GV_los_y1, g_rw(CBF(m, CB_Y)));
    int blocked = ns_call(GF_line_blocked, 3, i, m, 0x80);
    g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
    if (blocked && getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d has no clear line to %d\n", i, m);
    if (blocked && combat_improved()) return MISSILE;                   /* (it steps aside for a line: mods/battlerules.c) */
    return blocked ? melee : MISSILE;
}

/* battle.volley_melee: a member shooting (order Use Missile, 0x80) goes on shooting whatever comes:
   a foe beside it, a line of fire blocked by the melee, no arrows left - it stands and waits ('b',
   'e' on the panel) or shoots at point blank until given another order. With this on, after
   melee_exchanges (s014 0e76:2a4a) each tick: a shooter with a foe within reach of a blow (far
   20a5:7900), or within some five paces (it walks to it), draws its melee weapon and fights it (its own target if that one is beside it), as
   Std Attack on a foe beside the member gives it (order_mode_leave, s014 19cd:0738: weapon_ready
   with the default weapon, the target, order 0x12, action 7); one with nothing left to shoot ('e'),
   or whose line is blocked ('b') while its target is already fighting one of the party, walks to
   its target and fights it (path_find, path_give, order_queue: the leave's way for a foe further
   off). The volley before the lines meet is the shooters'; the melee after it is everyone's. */
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
}     /* put the missile weapon away for a foe beside it (improved combat): the bow again once none is */
static void to_melee(int i, int t, int beside)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    if (st(i) == ST_SKIRMISHER && !committed[i]) { committed[i] = 1; fprintf(stderr, "orders: member %d (Skirmisher) fights hand to hand from now on\n", i); }
    ns_call(GF_weapon_ready, 3, i, (s8)g_rb((u16)(c + CB_MELEE)), 0);
    g_wb((u16)(c + CB_TARGET), (u8)t);
    int kind = melee_kind(i, t);
    if (st(i) == ST_BERSERKER && kind != 0x0a) {     /* (Berserk blows while it can take a few - not against armor only Vulnerable gets through - Standard once badly hurt) */
        int e = (s8)g_rb((u16)(c + CB_END)), em = (s8)g_rb((u16)(c + CB_END_MAX));
        if (em > 0 && e * 3 > em) kind = 6;
    }
    if (beside) { g_ww((u16)(c + CB_ORDER), (u16)kind); g_wb((u16)(GV_cbt_action + i), 7); }
    else {
        g_ww((u16)(c + CB_TO_X), g_rw(CBF(t, CB_X))); g_ww((u16)(c + CB_TO_Y), g_rw(CBF(t, CB_Y)));
        if (ns_call(GF_path_find, 1, i) != 1) { g_ww((u16)(c + CB_ORDER), (u16)kind); g_wb((u16)(GV_cbt_action + i), 7); return; }   /* (no way: it fights when the foe comes) */
        ns_call(GF_path_give, 1, i);
        g_wb((u16)(GV_cbt_moving + i), 1); g_wb((u16)(GV_cbt_path_step + i), 0);
        g_ww((u16)(c + CB_ORDER), 1);
        ns_call(GF_order_queue, 2, i, kind);
    }
    g_wb((u16)(GV_cbt_state + i), 0);
    fprintf(stderr, "orders: member %d puts the missile weapon away for %d (%s)\n", i, t, beside ? "beside it" : "walking to it");
}
/* battle.retarget_shots: the game drops a shooter's order when its mark falls (the order back to
   0); the member then stands with the bow until told again. Here it shoots at the nearest foe still
   standing on its floor */
/* a clear line of fire from member i to foe j, where i stands (line_blocked, s014 1d69:000c, as the shot is tested) */
static int line_clear(int i, int j)
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
                fprintf(stderr, "orders: member %d has nothing left to shoot: %s\n", i, st(i) == ST_HOLDFAST ? "holds where it stands" : "the melee weapon");
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
        if (best >= 0 && !best_clear && st(i) == ST_HOLDFAST) best = -1;   /* (a Holdfast does not step aside: none it can see, it waits for one) */
        if (best < 0) {
            if (st(i) == ST_HOLDFAST) { g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_moving + i), 0); g_wb((u16)(GV_cbt_action + i), 7); }   /* (not the game's walk at someone) */
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
        int clear = line_clear(i, j);
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
        if (g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl || !near_foe_of(i, j)) continue;
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
        if (!engaged && line_clear(i, j)) return j;
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
/* the one a Bodyguard keeps beside: the most exposed of the others on its floor - a shooter first (it
   stands back and cannot fend off a foe while loosing), then the lightest body armor (far 20a5:781e, its
   hardness), then the nearest. -1 none */
static int protege_of(int i)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), best = -1; float bs = 0;
    for (int p = 0; p < 5; p++) {
        if (p == i || g_rb(CBF(p, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + p)) != fl) continue;
        int wm = (s8)g_rb(CBF(p, CB_MISSILE)), at = g_rb(CBF(p, CB_BODY_ARMOR));
        int shoots = g_rw(CBF(p, CB_ORDER)) == MISSILE || (wm >= 0 && wm <= 62 && (st(p) == ST_OFF || shooter(p)));
        int h = at == 0xff ? 0 : rb(G_FAR20A5, (u16)(0x781e + at));
        float s = (shoots ? 0.0f : 10000.0f) + (st(p) == ST_BODYGUARD ? 5000.0f : 0.0f) + h * 100.0f + dist(i, p) / 100.0f;
        if (best < 0 || s < bs) { best = p; bs = s; }
    }
    return best;
}
/* the foe member i goes at hand to hand, as its way has it; -1 none. A foe beside it always first (its
   own target first of those). Then:
     PICK_BESIDE (Archer, Holdfast, a shooter with something to loose): none.
     PICK_FOOT (Man-at-arms): once the lines have met (or no one of the party shoots), a foe fighting a
       friend that fewer than two of the party fight (two on one), else a foe no one fights, the nearest;
       else the nearest of the rest, the least crowded.
     PICK_CHARGE (Charger, Berserker): from the start, the enemy's shooters no one goes at yet, then the
       nearest foe no one fights, then the least crowded.
     PICK_GUARD (Bodyguard): a foe beside the one it guards, or within a few paces of them; no one to
       guard: as a Man-at-arms. */
enum { PICK_BESIDE, PICK_FOOT, PICK_CHARGE, PICK_GUARD };
static int melee_pick(int i, int how, int *beside_out)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), cur = (s8)g_rb(CBF(i, CB_TARGET)), best = -1, best_beside = 0;
    int g = how == PICK_GUARD ? protege_of(i) : -1;
    if (how == PICK_GUARD && g < 0) how = PICK_FOOT;
    int open = how != PICK_FOOT || lines_met || !party_shoots();
    float bs = 0;
    for (int j = 5; j < 13; j++) {
        if (g_rb(CBF(j, CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl) continue;
        int beside = adj(i, j);
        float d = dist(i, j), s;
        if (beside) s = 100000.0f + (j == cur ? 50.0f : 0.0f) - d;
        else if (how == PICK_BESIDE) continue;
        else if (how == PICK_GUARD) {
            if (adj(g, j)) s = 50000.0f - d;
            else if (near_foe_of(g, j)) s = 40000.0f - d;
            else continue;
        } else {
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
/* a Bodyguard with no foe to fight: it walks to a spot beside the one it guards, on the side of the
   nearest foe (between them), when more than a few paces off; not more often than every few seconds
   (the one it guards may be walking) */
static u16 guard_at[5];
static void guard_walk(int i)
{
    int g = protege_of(i);
    if (g < 0) return;
    u16 now = g_rw(GV_combat_clock);
    if (guard_at[i] && (u16)(now - guard_at[i]) < 60) return;
    if (dist(i, g) <= 28.0f) return;
    guard_at[i] = now ? now : 1;
    u16 c = (u16)(D_CBT + i * 0x80);
    int gx = (s16)g_rw(CBF(g, CB_X)), gy = (s16)g_rw(CBF(g, CB_Y)), nf = -1; float nd = 0;
    for (int j = 5; j < 13; j++)
        if (g_rb(CBF(j, CB_STATUS)) == 1 && g_rb((u16)(GV_cbt_floor + j)) == g_rb((u16)(GV_cbt_floor + g)) && (nf < 0 || dist(g, j) < nd)) { nf = j; nd = dist(g, j); }
    float ux = 1.0f, uy = 0.0f;                     /* (towards the foe, on the field's scale) */
    if (nf >= 0 && nd > 1.0f) { ux = ((s16)g_rw(CBF(nf, CB_X)) - gx) / nd; uy = 2.0f * ((s16)g_rw(CBF(nf, CB_Y)) - gy) / nd; }
    const float spots[4][2] = { { ux, uy }, { -uy, ux }, { uy, -ux }, { -ux, -uy } };
    u16 tx = g_rw((u16)(c + CB_TO_X)), ty = g_rw((u16)(c + CB_TO_Y));
    for (int k = 0; k < 4; k++) {
        int x = gx + (int)(16.0f * spots[k][0]), y = gy + (int)(8.0f * spots[k][1]);
        if (x < 0 || y < 0) continue;
        g_ww((u16)(c + CB_TO_X), (u16)x); g_ww((u16)(c + CB_TO_Y), (u16)y);
        int found = ns_call(GF_path_find, 1, i) == 1;
        if (ns_unwinding) return;
        if (!found) continue;
        ns_call(GF_path_give, 1, i);
        if (ns_unwinding) return;
        g_wb((u16)(GV_cbt_moving + i), 1); g_wb((u16)(GV_cbt_path_step + i), 0);
        g_ww((u16)(c + CB_ORDER), 1); g_wb((u16)(c + CB_TARGET), 0xff); g_wb((u16)(GV_cbt_state + i), 0);
        fprintf(stderr, "orders: member %d (Bodyguard) keeps beside %d\n", i, g);
        return;
    }
    g_ww((u16)(c + CB_TO_X), tx); g_ww((u16)(c + CB_TO_Y), ty);
}

/* a member with a missile weapon left without orders (the battle let run before the player gave any,
   or an order carried out), whose way is to shoot (Archer, Holdfast, Skirmisher): it opens fire at a foe
   it can see (shot_target) rather than stand with the bow until a foe reaches it - the volley before the
   lines meet. A Skirmisher with a friend set upon a few paces off goes to help instead (and fights hand
   to hand from then on); a Holdfast shoots only what it sees from where it stands. Not one told to hold
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
        if (s0 == ST_HOLDFAST && hold_try[i] && (u16)(now - hold_try[i]) < 20) continue;
        if (!ns_call(GF_can_fire, 1, i)) { if (ns_unwinding) return; continue; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        if (t < 0) continue;
        if (s0 == ST_HOLDFAST && !clear) { hold_try[i] = now ? now : 1; continue; }   /* (no line from where it stands: it waits for one) */
        ready_shot(i, t);
        fprintf(stderr, "orders: member %d had no orders: opens fire at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* a member left without orders who does not shoot now: it fights as its way has it (melee_pick) - a foe
   beside it; a friend's fight or a free foe once the lines meet (Man-at-arms); the enemy's shooters and
   free foes from the start (Charger, Berserker); whoever comes at the one it guards (Bodyguard, who
   otherwise keeps beside them). Not one told to hold (the game's Hold); Halt keeps its own order (0x400)
   and is let be */
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
        case ST_HOLDFAST: how = PICK_BESIDE; break;
        case ST_ARCHER: case ST_SKIRMISHER: how = armed ? PICK_BESIDE : PICK_FOOT; break;
        case ST_CHARGER: case ST_BERSERKER: how = PICK_CHARGE; break;
        case ST_BODYGUARD: how = PICK_GUARD; break;
        }
        int beside = 0, t = melee_pick(i, how, &beside);
        if (ns_unwinding) return;
        if (t < 0) { if (s0 == ST_BODYGUARD) { guard_walk(i); if (ns_unwinding) return; } continue; }
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
        if (st(i) == ST_HOLDFAST && (!alive || !adj(i, t))) {             /* (a Holdfast does not go after a foe that stepped back: the game would walk it there) */
            int b = -1;
            for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && adj(i, j)) { b = j; break; }
            g_wb((u16)(GV_cbt_moving + i), 0);
            if (b >= 0) { to_melee(i, b, 1); if (ns_unwinding) return; continue; }
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            fprintf(stderr, "orders: member %d (Holdfast) has no foe beside it: stands\n", i);
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
   a shooter set upon to Parry or a Std Attack): an Archer or a Holdfast (and one the player sent to
   shoot) with no foe beside it now takes up the missile weapon again, at the next foe; a Skirmisher fights
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
        if (s0 != ST_ARCHER && s0 != ST_HOLDFAST) continue;
        if (!has_ammo(i, wm) || foes_beside(i)) continue;
        /* a foe within a few paces coming at it: the fight is about to reach it, the bow would only be
           put away again (each change of weapon starts the reloading over: a shooter switched back and
           forth never shoots) */
        { int coming = 0;
          for (int j = 5; j < 13; j++) if (g_rb(CBF(j, CB_STATUS)) == 1 && near_foe_of(i, j) && (s8)g_rb(CBF(j, CB_TARGET)) == i) coming = 1;
          if (coming) continue; }
        /* the game took the bow away again just after it was given back: let the game have its way
           for a while (its AI sees a fight the member is in) */
        { u16 now = g_rw(GV_combat_clock);
          if (bow_given[i] && (u16)(now - bow_given[i]) < 90) { if (++bow_flips[i] >= 2) { bow_hold[i] = now ? now : 1; bow_flips[i] = 0; } continue; }
          if (bow_hold[i] && (u16)(now - bow_hold[i]) < 600) continue;
          bow_hold[i] = 0; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        if (t < 0 || (s0 == ST_HOLDFAST && !clear)) {                    /* (nothing it can shoot: the order ends; it shoots when it can, idle_shooters) */
            g_ww((u16)(c + CB_ORDER), 0); g_wb((u16)(GV_cbt_action + i), 7);
            continue;
        }
        ready_shot(i, t);
        bow_given[i] = g_rw(GV_combat_clock); if (!bow_given[i]) bow_given[i] = 1;
        fprintf(stderr, "orders: member %d has no foe beside it now: the missile weapon again, at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* a Holdfast or a Bodyguard fighting hand to hand parries (0x22) while two foes or more are beside it,
   and goes back to its attack (Std Attack, or Vulnerable as the armor asks) once only one is */
static signed char parried[5];
static void stances(void)
{
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int f = (s16)g_rw((u16)(c + CB_ORDER)), s0 = st(i);
        if (!automated(i) || g_rb((u16)(c + CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_moving + i)) || (s0 != ST_HOLDFAST && s0 != ST_BODYGUARD)
            || !(f == STD_ATTACK || f == 0x0a || f == 0x22)) { parried[i] = 0; continue; }
        int n = foes_beside(i);
        if (n >= 2 && f != 0x22) {
            g_ww((u16)(c + CB_ORDER), 0x22); parried[i] = 1;
            fprintf(stderr, "orders: member %d (%s) parries: %d foes beside it\n", i, style_name(s0), n);
        } else if (n <= 1 && f == 0x22 && parried[i]) {
            int t = (s8)g_rb((u16)(c + CB_TARGET)), k = melee_kind(i, t);
            g_ww((u16)(c + CB_ORDER), (u16)k); parried[i] = 0;
            fprintf(stderr, "orders: member %d (%s) attacks again: %d foe beside it\n", i, style_name(s0), n);
        }
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
/* member m's way chosen anew (the party panel): what the player's orders made of it in this fight is
   forgotten, the new way from now on */
void orders_style_changed(int m)
{
    if (m < 0 || m > 4) return;
    ov[m] = OV_NONE; committed[m] = 0; pt[m] = -1; parried[m] = 0; guard_at[m] = 0;
}
/* a Parry, Berserk or Vulnerable order kept: the game turns a member who is set upon on the way
   to its foe (or whose foe falls) to a Std Attack on whoever it fights; the kind the player chose
   is kept for that fight too, until the member is given another order */
static int kept_kind[5] = { -1, -1, -1, -1, -1 };
/* every frame (runtime/platform_sdl.c): a battle begun, what this file remembers of the members is
   cleared - the last battle's orders, kinds kept, marks, bow switches and ways given way to orders are
   not this one's (a member sent hand to hand there opened fire here no more) */
void battlerules_battle_begins(void), pathfix_battle_begins(void);
void orders_frame(void)
{
    static int was;
    int in = game_mode() == GM_BATTLE;
    if (in && !was) {
        for (int i = 0; i < 5; i++) {
            bow_given[i] = bow_hold[i] = 0; bow_flips[i] = 0;
            idle_since[i] = 0; shot_at[i] = -1; kept_kind[i] = -1; told[i] = 0;
            ov[i] = OV_NONE; committed[i] = 0; pt[i] = -1; parried[i] = 0; guard_at[i] = 0; hold_try[i] = 0;
        }
        lines_met = 0;
        battlerules_battle_begins(); pathfix_battle_begins();   /* (every file's memory of a battle: forgotten at the next) */
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
            if (g_rb(CBF(j, CB_STATUS)) == 1 && g_rb((u16)(GV_cbt_floor + j)) == g_rb((u16)(GV_cbt_floor + i)) && (adj(i, j) || near_foe_of(i, j))) {
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
    stances();
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
        } else if (stt == 1 && s0 == ST_HOLDFAST && (!hold_try[i] || (u16)(now - hold_try[i]) >= 30)) {   /* (a line blocked: a Holdfast does not step aside, it takes a foe it can see) */
            hold_try[i] = now ? now : 1;
            int clear = 0, b = shot_target(i, &clear);
            if (ns_unwinding) return;
            if (b >= 0 && clear && b != t) { g_wb((u16)(c + CB_TARGET), (u8)b); g_wb((u16)(GV_cbt_state + i), 0); fprintf(stderr, "orders: member %d (Holdfast) has no line to %d: shoots at %d\n", i, t, b); }
        }
    }
}

void plat_mouse_game(int *x, int *y);
static int selected_ok(void);
/* battle.los_line (mods/losaim.c): the member chosen and the foe under the pointer, when a click
   there would shoot (0x80) or throw (0x100) at it: the order chosen in the menu, or the order the
   member follows, or (none chosen) the shot a click on a foe gives one who opens by shooting */
static int aim_of(int *who, int *foe)
{
    if (config_get_int("original", 0) || g_rb(D_ORDER_MODE) != 1 || !selected_ok()) return 0;
    int gx, gy; plat_mouse_game(&gx, &gy);
    int mx = gx - 60, my = gy, i = (s8)g_rb(D_SELECTED);
    if (mx < 0 || mx >= 0x104 || i < 0 || i > 4) return 0;
    int m = topmost(mx, my), cx, cy;
    if ((m < 5 || m >= 13) && near_foe(mx, my, &cx, &cy)) m = topmost(cx, cy);
    if (m < 5 || m >= 13 || g_rb(CBF(m, CB_STATUS)) != 1) return 0;
    int act = (s16)g_rw(D_ORDER_ACTION), f = (s16)g_rw(CBF(i, CB_ORDER)), mode = 0;
    if (act == MISSILE || act == 0x100) mode = act;
    else if (act == -1) {
        if (f == MISSILE || f == 0x100) mode = f;
        else if (orders_click_attack && orders_member_mode(i) == 1 && !rb(G_FAR20A5, (u16)(0x7900 + i * 13 + m)) && !near_foe_of(i, m)) mode = MISSILE;
    }
    if (!mode) return 0;
    *who = i; *foe = m;
    return mode;
}
int orders_aim(int *who, int *foe) { return config_get_int("battle.los_line", 1) ? aim_of(who, foe) : 0; }

/* the order the click now being given should take on what is under (x, y): 0 the game's own */
static int wants_attack(int x, int y)
{
    if (!orders_click_attack || config_get_int("original", 0)) return 0;
    if (g_rb(D_ORDER_MODE) != 1 || (s16)g_rw(D_ORDER_ACTION) != -1) return 0;
    int i = (s8)g_rb(D_SELECTED);
    if (i < 0 || i > 4) return 0;
    int f = (s16)g_rw(CBF(i, CB_ORDER));
    int m = topmost(x, y);
    if (m < 5 || m >= 13 || g_rb(CBF(m, CB_STATUS)) != 1) return 0;
    if (f == 0 || f == 1) return best_attack(i, m);
    /* already fighting: the game's own way (the same kind of attack, at the foe clicked), unless the
       weapon changes - a shooter sent at a foe beside it (or who opens with the melee weapon), or a
       member fighting hand to hand who opens by shooting, at a foe it can shoot */
    int b = best_attack(i, m);
    if (orders_member_mode(i) >= 0 && ((f == MISSILE && b != MISSILE) || ((f & 2) && b == MISSILE))) return b;
    if ((f == STD_ATTACK || f == 0x0a) && (b == STD_ATTACK || b == 0x0a) && b != f) return b;   /* (Std Attack or Vulnerable, as the armour asks; Berserk and Parry stay) */
    return 0;
}

/* battle.click_hint: what a click would do, in words beside the pointer (ui/battleoverlay.cpp), from
   the game's own test of the pointer in order mode (s014 19cd:0ad4: cursor_target >= 5 a foe, -2 a
   spot to walk to, -3 none; the box's colour D_c2bd, refused) and the order it tested for */
static int hint_sel = -1, hint_act, hint_ct;
static void hint_note(int act)
{
    int i = (s8)g_rb(D_SELECTED), ct = (s8)g_rb(GV_cursor_target);
    if (act == -1) act = (s16)g_rw(CBF(i, CB_ORDER));
    if (ct >= 5 && g_rw(D_BOX_COLOUR) == g_rw(D_REFUSED)) ct = -4;      /* (a foe it cannot reach or shoot at) */
    hint_sel = i; hint_act = act; hint_ct = ct;
    if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: pointer test: member %d, order %#x, target %d\n", i, act, ct);
}
/* what a click would aim at now, as the pointer's test found it (a foe 5..12; -2 a spot, -3 none, -4 a foe out of
   reach), with a member chosen in order mode; -1 none (the drawn pointer's crossed blades: ui/overlay_text.cpp) */
int orders_pointer_target(void)
{
    if (!config_get_int("battle.click_hint", 1) || config_get_int("original", 0) || g_rb(D_ORDER_MODE) != 1 || (s8)g_rb(D_SELECTED) != hint_sel) return -1;
    return hint_ct;
}
const char *orders_hint(void)
{
    if (!config_get_int("battle.click_hint", 1) || config_get_int("original", 0) || g_rb(D_ORDER_MODE) != 1 || (s8)g_rb(D_SELECTED) != hint_sel) return 0;
    int a = hint_act, flee = (g_rw((u16)(GV_cbt_flags + 2 * hint_sel)) & 0x1000) != 0;
    switch (hint_ct) {
    case -2: return a == 0x100 ? "Click: throw here" : flee && a == 1 ? "Click: flee here" : 0;   /* (walking there goes without saying) */
    case -3: return a == 0x100 ? "No clear throw" : "Can't walk there";
    case -4: return a == MISSILE ? "No clear shot" : a == 0x100 ? "No clear throw" : "No way there";
    case -1: return 0;
    }
    if (hint_ct < 5 || hint_ct >= 13) return 0;
    switch (a) {
    case MISSILE: {
        int w, f;                                                  /* (improved combat: a shooter with no line moves for one) */
        if (combat_improved() && aim_of(&w, &f) == MISSILE && f == hint_ct) { LosBox bx[4]; if (los_trace(w, f, MISSILE, bx, 4)) return "Click: shoot (moves for a clear line)"; }
        return "Click: shoot";
    }
    case 0x100: return "Click: throw";
    case 0x0a: return "Click: attack (Vulnerable)";
    case 6: return "Click: attack (Berserk)";
    case 0x22: return "Click: parry its blows";
    }
    if ((a == STD_ATTACK || a == 0x0a) && hint_sel >= 0) {       /* one who opens by shooting, sent in instead: no clear shot */
        int w, f;
        if (aim_of(&w, &f) == MISSILE && f == hint_ct) return "No clear shot: walks up to fight";
    }
    return 0;                                                    /* (a plain attack goes without saying) */
}
/* the game tests the pointer only while the button is held (or the cursor keys move it: combat_mouse,
   s014 19cd:0006, D_d728); for the words beside it, the same test is run whenever the pointer moves
   over the field (or the order chosen changes), and what it changes is put back: the box stays on
   the order's target as the game has it, and Enter gives the order the game had */
void plat_mouse_game(int *x, int *y);
static int selected_ok(void);
static int probe_x = -999, probe_y, probe_sel, probe_act;
static void probe_pointer(void)
{
    if (!config_get_int("battle.click_hint", 1) || config_get_int("original", 0) || !selected_ok() || (rb(0x40, 0x17) & 4)) { probe_x = -999; return; }
    int gx, gy; plat_mouse_game(&gx, &gy);
    int mx = gx - 60, my = gy, i = (s8)g_rb(D_SELECTED), act = (s16)g_rw(D_ORDER_ACTION);
    if (mx < 0 || mx >= 0x104) { hint_sel = -1; probe_x = -999; return; }
    if (mx == probe_x && my == probe_y && i == probe_sel && act == probe_act) return;
    probe_x = mx; probe_y = my; probe_sel = i; probe_act = act;
    static const u16 W[] = { 0xe1bc, 0xe1c2, 0xe1be, 0xe1b8, 0xe1ba, 0xe1c0, 0xe1b4, 0xe1b6, GV_los_x0, GV_los_y0, GV_los_x1, GV_los_y1 };
    u16 keep[12]; u8 ct = g_rb(GV_cursor_target), c600 = g_rb(0xc600);
    for (int k = 0; k < 12; k++) keep[k] = g_rw(W[k]);
    ns_call(14, 0x19cd, 0x0ad4, 2, mx, my);             /* (through hover_hook: as a click there would be taken) */
    for (int k = 0; k < 12; k++) g_ww(W[k], keep[k]);
    g_wb(GV_cursor_target, ct); g_wb(0xc600, c600);
}
static void hover_hook(void)                    /* s014 19cd:0ad4 (x, y): the pointer in order mode */
{
    snap_to_foe();
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int set = wants_attack(x, y);
    if (set) g_ww(D_ORDER_ACTION, (u16)set);
    hook_call_original();
    hint_note((s16)g_rw(D_ORDER_ACTION));
    { int m = topmost(x, y); if (m >= 0 && m < 5) hint_ct = -1; }   /* (on a member: the game tests nothing; a click chooses them) */
    if (set) g_ww(D_ORDER_ACTION, 0xffff);              /* only the look of the pointer */
}

/* ---------------------------------------------------------------- battle.quick_orders
 * The game's way: a click on a foe or on the ground only marks the order's target (the box around
 * it); the order is given by Enter ("<name> finished"), by Space (which also resumes the battle)
 * or by a click on another member, and choosing a member pauses the battle (order_mode_enter, s014
 * 19cd:069c) until Space. With this on, the click that the game takes (a foe it can reach or shoot
 * at, a spot it can walk to) gives the order at once, and so does a choice of the Orders menu that
 * needs no target (Halt, a door, stairs, a lock: order_action 0x400, s016 1ef8:1590); an attack
 * chosen (Std Attack, Vulnerable, Berserk, Parry: s016 1ef8:156e) for a member who already fights
 * a foe in sight (or walks to it to fight) is given against that foe. If choosing the member is what paused the battle, it
 * runs on as the order is given (the game's Space: s016 1ef8:1436); while paused otherwise (Space,
 * or the battle stopping for orders), battle.next_member then chooses the next member who has no
 * order (in the party's order, one in sight), so a battle's first orders are a click on a foe each.
 * The order itself is the game's (order_mode_leave, s014 19cd:0738), given at the next input tick
 * (combat_input, s014 0e76:3166), outside the click's and the menu's own code. */
static int quick_on(void) { return config_get_int("battle.quick_orders", 1) && !config_get_int("original", 0); }
static int resume_after;            /* choosing a member paused the battle: it runs on with the order */
static int give_pending, foe_pending = -1;

/* the battle has been running (not paused, no one given orders) since run_since: a member chosen
   then paused it. (A battle starts with its leader chosen, s014 0e76 combat_main: vec_1c39 and
   D_d6f1, entering order mode at its first tick; that pause is the battle's, so the leader's order
   does not end it.) */
u32 plat_ticks_ms(void);
static int running; static u32 run_since; static u16 run_clock;
/* a member chosen by the player (a key 1..5, a click on them or on their box) as opposed to the
   battle's own choosing (its start, a pause for orders): only the player's choice, made while the
   battle runs, lets the order resume it */
static u32 user_pick_t; static int own_select;
static void pick_key_hook(void)                 /* s016 1ef8:0a2c (i): a member chosen */
{
    if (!own_select) user_pick_t = plat_ticks_ms();
    hook_call_original();
}
static void pick_mouse_hook(void)               /* s014 19cd:0006 combat_mouse () */
{
    if (g_rb(GV_mouse_buttons) & 1) user_pick_t = plat_ticks_ms();
    hook_call_original();
}
static void enter_hook(void)                    /* s014 19cd:069c order_mode_enter() */
{
    u32 now = plat_ticks_ms();
    /* (the battle must really have run: its clock gone on some ticks, not only the moments before its
       first pause, when the view still scrolls to the leader or a member chosen off the screen) */
    if (g_rb(GV_ai_paused) == 0 && running && now - run_since >= 300 && (u16)(g_rw(GV_combat_clock) - run_clock) >= 12 && now - user_pick_t < 600) resume_after = 1;
    hint_sel = -1;
    hook_call_original();
}
static void cancel_hook(void)                   /* s016 1ef8:13d4: Esc, Cancel Giving Order (the battle stays paused) */
{
    resume_after = 0; give_pending = 0; foe_pending = -1;
    for (int k = 0; k < 5; k++) told[k] = 0;
    hook_call_original();
}
static int selected_ok(void)
{
    int i = (s8)g_rb(D_SELECTED);
    return g_rb(D_ORDER_MODE) == 1 && i >= 0 && i <= 4;
}
static void place_hook(void)                    /* s016 1ef8:1590 (v): an order at a place (order_action 0x400) */
{
    hook_call_original();
    if (quick_on() && selected_ok() && (s16)g_rw(D_ORDER_ACTION) == 0x400) give_pending = 1;
}
static void attack_hook(void)                   /* s016 1ef8:156e (kind): an attack chosen */
{
    hook_call_original();
    int a = (s16)g_rw(D_ORDER_ACTION);
    if (!quick_on() || !selected_ok() || (a != 6 && a != 0x0a && a != STD_ATTACK && a != 0x22)) return;
    u16 c = (u16)(D_CBT + (s8)g_rb(D_SELECTED) * 0x80);
    int t = (s8)g_rb((u16)(c + CB_TARGET)), f = (s16)g_rw((u16)(c + CB_ORDER));
    if (t >= 5 && t < 13 && ((f & 2) || f == 1) && g_rb(CBF(t, CB_STATUS)) == 1) foe_pending = t;   /* (fighting it, or walking to it to fight) */
}
/* a point of foe t's hot spot where t is the topmost (a click there takes t), or 0 */
static int point_on(int t, int *px, int *py)
{
    int n = (s8)g_rb(D_HOT_N);
    for (int k = 0; k < n; k++) {
        if (g_rb((u16)(D_HOT_CBT + k)) != t) continue;
        int x0 = (s16)g_rw((u16)(D_HOT_X0 + 2 * k)), x1 = (s16)g_rw((u16)(D_HOT_X1 + 2 * k));
        int y0 = (s16)g_rw((u16)(D_HOT_Y0 + 2 * k)), y1 = (s16)g_rw((u16)(D_HOT_Y1 + 2 * k));
        for (int d = 0; d <= (y1 - y0) / 2 + 1; d++)          /* (rows from the middle out) */
            for (int s = -1; s <= 1; s += 2) {
                int y = (y0 + y1) / 2 + s * d;
                if (y < y0 || y > y1) continue;
                for (int x = x0; x <= x1; x++)
                    if (x >= 0 && x < 0x104 && topmost(x, y) == t) { *px = x; *py = y; return 1; }
            }
    }
    return 0;
}
/* the next member, after i, who has no order: active, on the floor shown (in sight or not: one off the
   screen is brought into view, mods/battlescroll.c); -1 none */
static int next_waiting(int i)
{
    for (int d = 1; d < 5; d++) {
        int m = (i + d) % 5;
        u16 c = (u16)(D_CBT + m * 0x80);
        if (g_rb((u16)(c + CB_STATUS)) != 1 || g_rw((u16)(c + CB_ORDER)) != 0 || g_rb((u16)(GV_cbt_state + m)) || g_rb((u16)(GV_cbt_floor + m)) != g_rb(GV_floor_shown)) continue;
        return m;
    }
    return -1;
}
static void give_now(void)
{
    int i = (s8)g_rb(D_SELECTED);
    hint_sel = -1;
    if (resume_after) {
        resume_after = 0;
        fprintf(stderr, "orders: member %d's order given, the battle runs on\n", i);
        ns_call(16, 0x1ef8, 0x1436, 0);                 /* (Space: order_mode_leave, then the AI runs) */
        return;
    }
    ns_call(GF_order_mode_leave, 0);
    int n = config_get_int("battle.next_member", 1) ? next_waiting(i) : -1;
    if (n >= 0) fprintf(stderr, "orders: member %d's order given, the next to choose for: %d\n", i, n);
    else fprintf(stderr, "orders: member %d's order given\n", i);
    if (n >= 0) { own_select = 1; ns_call(16, 0x1ef8, 0x0a2c, 1, n); own_select = 0; }   /* (as the key 1..5: chosen at the next tick) */
}
static void input_hook(void)                    /* s014 0e76:3166 combat_input() */
{
    if (g_rb(GV_ai_paused) == 0 && g_rb(D_ORDER_MODE) != 1) { resume_after = 0; if (!running) { running = 1; run_since = plat_ticks_ms(); run_clock = g_rw(GV_combat_clock); } }
    else running = 0;
    if (foe_pending >= 0) {
        int t = foe_pending, x, y; foe_pending = -1;
        if (quick_on() && selected_ok() && point_on(t, &x, &y)) ns_call(14, 0x19cd, 0x0dcc, 2, x, y);   /* (as a click on it: through click_hook) */
    }
    if (give_pending) { give_pending = 0; if (quick_on() && selected_ok()) give_now(); }
    probe_pointer();
    hook_call_original();
}

static void click_hook(void)                    /* s014 19cd:0dcc (x, y): a click in order mode */
{
    snap_to_foe();
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int set = wants_attack(x, y), m = topmost(x, y);
    {   /* the kind of attack chosen for this order (Parry, Berserk, Vulnerable), kept (below) */
        int sel = (s8)g_rb(D_SELECTED), act = set ? set : (s16)g_rw(D_ORDER_ACTION);
        if (sel >= 0 && sel < 5) kept_kind[sel] = m >= 5 && (act == 0x22 || act == 6 || act == 0x0a) ? act : -1;
    }
    if (set) {
        g_ww(D_ORDER_ACTION, (u16)set);                 /* as if the attack had been chosen first */
        fprintf(stderr, "orders: member %d %s %d (clicked at %d,%d)\n", (s8)g_rb(D_SELECTED), set == MISSILE ? "shoots at" : "attacks", m, x, y);
    }
    /* a foe clicked: a spot on the ground clicked before in this order is no longer the way there
       (order_mode_leave, s014 19cd:0738, makes it the member's goal while the path goes to the foe:
       "walking" to beside the foe) */
    if (m >= 5 && orders_click_attack && !config_get_int("original", 0) && g_rw(GV_order_dest_x)) { g_ww(GV_order_dest_x, 0); g_ww(GV_order_dest_y, 0); }
    int quick = quick_on() && m >= 5 ? 1 : quick_on() && m < 0 ? 2 : 0;   /* (on a member: the game leaves the order and chooses them) */
    if (quick) g_wb(D_FOE_CLICKED, 0);
    hook_call_original();
    if (!selected_ok()) return;
    int ct = (s8)g_rb(GV_cursor_target);
    if ((ct >= 5 && g_rw(D_BOX_COLOUR) != g_rw(D_REFUSED)) || (ct == -2 && g_rw(GV_order_dest_x) != 0))
        told[(s8)g_rb(D_SELECTED)] = 1;                 /* (an order of the player's: the member's way gives way to it, leave_hook) */
    if (!quick) return;
    if ((ct >= 5 && g_rb(D_FOE_CLICKED) == 1 && g_rw(D_BOX_COLOUR) != g_rw(D_REFUSED)) || (ct == -2 && g_rw(GV_order_dest_x) != 0))
        give_pending = 1;                               /* (taken: a foe within reach or in the line of fire, or a spot to walk to) */
}

void orders_init(void)
{
    orders_click_attack = config_get_int("battle.click_attack", 1);
    hook_install(14, 0x19cd, 0x0ad4, hover_hook);
    hook_install(14, 0x19cd, 0x0dcc, click_hook);
    hook_install(GF_order_mode_enter, enter_hook);
    hook_install(16, 0x1ef8, 0x0a2c, pick_key_hook);
    hook_install(GF_combat_mouse, pick_mouse_hook);
    hook_install(16, 0x1ef8, 0x13d4, cancel_hook);
    hook_install(16, 0x1ef8, 0x1590, place_hook);
    hook_install(16, 0x1ef8, 0x156e, attack_hook);
    hook_install(GF_combat_input, input_hook);
    hook_install(GF_melee_exchanges, melee_hook);
    hook_install(GF_order_mode_leave, leave_hook);
}
