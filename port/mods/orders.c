/* orders.c - in a battle, a member who is not yet fighting can be sent at an enemy with a click
 * on it (F12 > Settings; "Play the original game" leaves the game's way).
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
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "battlerules.h"
#include "losaim.h"

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
        int cy = (s16)g_rw((u16)(D_CBT + c * 0x80 + 6));
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
        if (c < 5 || c >= 13 || g_rb((u16)(D_CBT + c * 0x80 + 0x14)) != 1) continue;
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
    int f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    int attack = act == -1 ? (f == 0 || f == 1 || (f & 2) || f == 0x80) : ((act & 2) || act == 0x80);
    int cx, cy;
    if (attack && near_foe(x, y, &cx, &cy)) { ww(SS, (u16)(SP + 4), (u16)cx); ww(SS, (u16)(SP + 6), (u16)cy); }
}

/* ---------------------------------------------------------------- battle.volley
 * Shoot or melee: each member with a bow, crossbow, gun or thrown weapon in hand (+0x22, its type)
 * opens a fight one way or the other, shown on their box in the party panel and switched with a
 * click there (ui/partypanel.cpp; kept by the character's name, battle.shoot.<name>). Until it is
 * switched, a member better with the missile weapon than with the default one (+0x51), by the
 * skill each wants (far 20a5:75ac), shoots (battle.click_best). */
static void member_key(int m, char *key, int n)
{
    char nm[26]; int k = 0;
    for (; k < 24; k++) { char ch = (char)g_rb((u16)(G_MEMBER(m) + G_M_FULLNAME + k)); if (!ch) break; nm[k] = ch == ' ' || ch == '=' ? '_' : ch; }
    nm[k] = 0;
    snprintf(key, n, "battle.shoot.%s", nm);
}
int orders_member_mode(int m)                   /* 1 shoot, 0 melee, -1 no missile weapon in hand */
{
    if (m < 0 || m > 4) return -1;
    u16 c = (u16)(D_CBT + m * 0x80);
    int wm = (s8)g_rb((u16)(c + 0x22)), wd = (s8)g_rb((u16)(c + 0x51));
    if (wm < 0 || wm > 62) return -1;
    char key[48]; member_key(m, key, sizeof key);
    int v = config_get_int(key, -1);
    if (v == 0 || v == 1) return v;
    (void)wd;
    return combat_improved();   /* (until switched: with improved combat, everyone with a missile weapon shoots first) */
}

/* the melee attack member i opens with against foe t: Vulnerable (0x0a: 1d4 more penetration, fewer
   and less guarded blows: s014 18bf hit_damage, attack_rate) when the default weapon's penetration
   (far 20a5:76a8, low nibble) does not get through t's body armour (its hardness, far 20a5:781e by
   +0x4b) but would with the extra 1d4 (battle.auto_vulnerable); else Std Attack */
static int melee_kind(int i, int t)
{
    if (!combat_improved() || t < 5 || t >= 13) return STD_ATTACK;
    int w = (s8)g_rb((u16)(D_CBT + i * 0x80 + 0x51)), at = g_rb((u16)(D_CBT + t * 0x80 + 0x4b));
    if (w < 0 || w > 62 || at == 0xff) return STD_ATTACK;
    int pen = rb(G_FAR20A5, (u16)(0x76a8 + w)) & 15, h = rb(G_FAR20A5, (u16)(0x781e + at));
    return pen <= h && pen + 4 >= h ? 0x0a : STD_ATTACK;
}
/* a foe within about five paces (a fighter is some 8 pixels wide; the field's y is foreshortened by
   half): a shooter fights it hand to hand rather than loose at it */
#define NEAR_PX 56
static int near_foe_of(int i, int m)
{
    int dx = (s16)g_rw((u16)(D_CBT + m * 0x80 + 4)) - (s16)g_rw((u16)(D_CBT + i * 0x80 + 4));
    int dy = 2 * ((s16)g_rw((u16)(D_CBT + m * 0x80 + 6)) - (s16)g_rw((u16)(D_CBT + i * 0x80 + 6)));
    return (long)dx * dx + (long)dy * dy <= (long)NEAR_PX * NEAR_PX;
}
void orders_member_mode_set(int m, int shoot)
{
    if (orders_member_mode(m) < 0) return;
    char key[48]; member_key(m, key, sizeof key);
    config_set_int(key, shoot ? 1 : 0); config_save();
    fprintf(stderr, "orders: member %d opens with %s\n", m, shoot ? "the missile weapon" : "the melee weapon");
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
        u16 tx = g_rw((u16)(c + 8)), ty = g_rw((u16)(c + 0x0a));
        g_ww((u16)(c + 8), g_rw((u16)(D_CBT + m * 0x80 + 4))); g_ww((u16)(c + 0x0a), g_rw((u16)(D_CBT + m * 0x80 + 6)));
        int way = ns_call(GF_path_find, 1, i) == 1;
        g_ww((u16)(c + 8), tx); g_ww((u16)(c + 0x0a), ty);
        if (way) return melee;
        if (!line_clear(i, m)) return melee;                         /* (no way found, nor a line: the game's own attack, which walks round as it can) */
        if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d has no way to %d: a shot instead\n", i, m);
    }
    if (!ns_call(GF_can_fire, 1, i)) { if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d cannot shoot (nothing to loose)\n", i); return melee; }
    /* a clear line from where the member stands (line_blocked, s014 1d69:000c, as the order's pointer
       tests it): else the member closes in instead */
    u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
    g_ww(GV_los_x0, g_rw((u16)(c + 4))); g_ww(GV_los_y0, g_rw((u16)(c + 6)));
    g_ww(GV_los_x1, g_rw((u16)(D_CBT + m * 0x80 + 4))); g_ww(GV_los_y1, g_rw((u16)(D_CBT + m * 0x80 + 6)));
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
static signed char by_volley[5];
static u16 bow_given[5], bow_hold[5];   /* back_to_bow: when the bow was last given back; the game's way let be since */
static unsigned char bow_flips[5];
static signed char player_melee[5];   /* the player told this member to fight hand to hand (order_mode_leave): the bow is not taken up again for it */
static void leave_hook(void)                    /* s014 19cd:0738 order_mode_leave(): the order chosen applied */
{
    int i = (s8)g_rb(D_SELECTED);
    hook_call_original();
    if (ns_unwinding || i < 0 || i > 4) return;
    int f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    player_melee[i] = f != MISSILE && f != 0x100 && f != 0 && f != 1;
    by_volley[i] = 0; bow_given[i] = bow_hold[i] = 0; bow_flips[i] = 0;
}     /* put the missile weapon away for a foe beside it (improved combat): the bow again once none is */
static void to_melee(int i, int t, int beside)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    by_volley[i] = 1;
    ns_call(GF_weapon_ready, 3, i, (s8)g_rb((u16)(c + 0x51)), 0);
    g_wb((u16)(c + 0x4a), (u8)t);
    int kind = melee_kind(i, t);
    if (beside) { g_ww((u16)(c + 0x10), (u16)kind); g_wb((u16)(GV_cbt_action + i), 7); }
    else {
        g_ww((u16)(c + 8), g_rw((u16)(D_CBT + t * 0x80 + 4))); g_ww((u16)(c + 0x0a), g_rw((u16)(D_CBT + t * 0x80 + 6)));
        if (ns_call(GF_path_find, 1, i) != 1) { g_ww((u16)(c + 0x10), (u16)kind); g_wb((u16)(GV_cbt_action + i), 7); return; }   /* (no way: it fights when the foe comes) */
        ns_call(GF_path_give, 1, i);
        g_wb((u16)(GV_cbt_moving + i), 1); g_wb((u16)(GV_cbt_path_step + i), 0);
        g_ww((u16)(c + 0x10), 1);
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
    g_ww(GV_los_x0, g_rw((u16)(c + 4))); g_ww(GV_los_y0, g_rw((u16)(c + 6)));
    g_ww(GV_los_x1, g_rw((u16)(D_CBT + j * 0x80 + 4))); g_ww(GV_los_y1, g_rw((u16)(D_CBT + j * 0x80 + 6)));
    int blocked = ns_call(GF_line_blocked, 3, i, j, 0x80);
    g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
    return !blocked;
}
static signed char shot_at[5] = { -1, -1, -1, -1, -1 };
static int shot_target(int i, int *clear_out);
static void retarget_shooters(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int ord = g_rw((u16)(c + 0x10)), t = (s8)g_rb((u16)(c + 0x4a));
        if (g_rb((u16)(c + 0x14)) != 1) { shot_at[i] = -1; continue; }
        if (getenv("DL_SHOT_LOG") && ord != 0) fprintf(stderr, "shots: %d order %#x target %d\n", i, ord, t);
        if (ord == MISSILE && t >= 5 && t < 13 && g_rb((u16)(D_CBT + t * 0x80 + 0x14)) == 1) { shot_at[i] = (signed char)t; continue; }
        if (ord == MISSILE && shot_at[i] < 5) shot_at[i] = 5;                 /* (the order kept, its mark gone (-1, or fallen): a new mark now, below) */
        if (shot_at[i] < 5) continue;
        if (ord != MISSILE && g_rb((u16)(D_CBT + shot_at[i] * 0x80 + 0x14)) == 1) { shot_at[i] = -1; continue; }   /* (its mark stands: the order changed for another reason) */
        /* its mark has just fallen: whatever the game's AI gave it instead (to walk at someone, to
           fight hand to hand), it shoots again */
        { int wm = (s8)g_rb((u16)(c + 0x22)); if (wm < 0 || wm > 62) { shot_at[i] = -1; continue; } }   /* (no missile weapon now) */
        { int beside = 0; for (int j = 5; j < 13; j++) if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) beside = 1;
          if (beside) { shot_at[i] = -1; continue; } }                       /* (a foe beside it: hand to hand first, the bow after: back_to_bow) */
        /* the nearest foe on its floor with a clear line from where it stands; none: the nearest (the
           stepping aside finds a spot to shoot it from, mods/battlerules.c) */
        int best_clear = 0, best = shot_target(i, &best_clear);
        if (ns_unwinding) return;
        shot_at[i] = -1;
        if (best < 0) continue;
        g_ww((u16)(c + 0x10), MISSILE); g_wb((u16)(c + 0x4a), (u8)best); g_wb((u16)(GV_cbt_state + i), 0);
        if (g_rb((u16)(GV_cbt_action + i)) == 0) g_wb((u16)(GV_cbt_action + i), 7);   /* (idle: "ready", so the shooting starts) */
        shot_at[i] = (signed char)best;
        fprintf(stderr, "orders: member %d's mark fell: now shoots at %d (%s)\n", i, best, best_clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* the foe a shooter turns to: the nearest on its floor with a clear line; none: the nearest */
static int shot_target(int i, int *clear_out)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    int best = -1, best_clear = 0; long bd = 0;
    int x = (s16)g_rw((u16)(c + 4)), y = (s16)g_rw((u16)(c + 6));
    for (int j = 5; j < 13; j++) {
        u16 d = (u16)(D_CBT + j * 0x80);
        if (g_rb((u16)(d + 0x14)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != g_rb((u16)(GV_cbt_floor + i))) continue;
        long dx = (s16)g_rw((u16)(d + 4)) - x, dy = ((s16)g_rw((u16)(d + 6)) - y) * 2, dd = dx * dx + dy * dy;
        int clear = line_clear(i, j);
        if (ns_unwinding) return -1;
        if (best < 0 || clear > best_clear || (clear == best_clear && dd < bd)) { best = j; bd = dd; best_clear = clear; }
    }
    if (clear_out) *clear_out = best_clear;
    return best;
}
/* a foe within a few paces (near_foe_of) fighting one of the party hand to hand, where that friend
   needs the help - two or more foes on it, or its Endurance below 60%, or the fight right beside the
   shooter (some three paces) - and the foe has fewer than two of the party on it already: the
   shooter puts the bow away and joins in rather than loose into the melee from close by. Of
   several, the one beside the friend most set upon (the most foes, then the least Endurance), then
   the nearest. -1 none */
static int rescue_target(int i)
{
    int fl = g_rb((u16)(GV_cbt_floor + i)), best = -1; long bs = 0;
    for (int j = 5; j < 13; j++) {
        if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) != 1 || g_rb((u16)(GV_cbt_floor + j)) != fl || !near_foe_of(i, j)) continue;
        long dx = (s16)g_rw((u16)(D_CBT + j * 0x80 + 4)) - (s16)g_rw((u16)(D_CBT + i * 0x80 + 4));
        long dy = 2 * ((s16)g_rw((u16)(D_CBT + j * 0x80 + 6)) - (s16)g_rw((u16)(D_CBT + i * 0x80 + 6)));
        long dd = dx * dx + dy * dy;
        int close = dd <= (long)(NEAR_PX * 3 / 5) * (NEAR_PX * 3 / 5), on_it = 0;
        long sc = -1;
        for (int p = 0; p < 5; p++) {
            if (p == i || g_rb((u16)(D_CBT + p * 0x80 + 0x14)) != 1 || !rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))) continue;
            on_it++;
            int n = 0;
            for (int k = 5; k < 13; k++) if (g_rb((u16)(D_CBT + k * 0x80 + 0x14)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + p * 13 + k))) n++;
            int cur = 0, mx = 0; gm_member_attr(p, 0, &cur, &mx);           /* (Endurance: the share left) */
            int low = mx > 0 && cur * 10 < mx * 6;
            if (n < 2 && !low && !close) continue;                           /* (holding its own: the shooter shoots) */
            long v = (long)n * 1000 + (mx > 0 ? 999 - 999L * (cur < 0 ? 0 : cur > mx ? mx : cur) / mx : 0);
            if (v > sc) sc = v;
        }
        for (int p = 0; p < 5; p++)                                           /* (and those on their way to it) */
            if (p != i && g_rb((u16)(D_CBT + p * 0x80 + 0x14)) == 1 && !rb(G_FAR20A5, (u16)(0x7900 + p * 13 + j))
                && (s8)g_rb((u16)(D_CBT + p * 0x80 + 0x4a)) == j && g_rb((u16)(GV_cbt_moving + p)) && g_rw((u16)(D_CBT + p * 0x80 + 0x10)) != MISSILE) on_it++;
        if (sc < 0 || on_it >= 2) continue;
        sc = sc * 100000 - dd / 16;
        if (best < 0 || sc > bs) { best = j; bs = sc; }
    }
    return best;
}
/* a member with a missile weapon left without orders (the battle let run before the player gave any,
   or an order carried out): it opens fire at the nearest foe it can see, rather than stand with the
   bow until a foe reaches it - the volley before the lines meet. Not one told to hold, walking, set
   to fight hand to hand by its own switch (orders_member_mode 0), with a foe beside it, or with
   nothing to loose */
static void idle_shooters(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        if (g_rb((u16)(c + 0x14)) != 1 || g_rw((u16)(c + 0x10)) != 0 || g_rb((u16)(GV_cbt_moving + i)) || g_rb((u16)(GV_cbt_action + i)) != 0) continue;
        if (g_rb((u16)(GV_cbt_mods + 0x12 * i + 9)) == 1 || orders_member_mode(i) != 1) continue;
        int wm = (s8)g_rb((u16)(c + 0x22)), beside = 0;
        if (wm < 0 || wm > 62) continue;
        for (int j = 5; j < 13; j++) if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) beside = 1;
        if (beside) continue;
        if (!ns_call(GF_can_fire, 1, i)) { if (ns_unwinding) return; continue; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        if (t < 0) continue;
        ns_call(GF_weapon_ready, 3, i, wm, 0x80);
        g_wb((u16)(c + 0x4a), (u8)t); g_ww((u16)(c + 0x10), MISSILE);
        g_wb((u16)(GV_cbt_state + i), 0);
        g_ww((u16)(GV_cbt_flags + 2 * i), (u16)(g_rw((u16)(GV_cbt_flags + 2 * i)) | 2));
        g_ww((u16)(GV_cbt_engage_time + 2 * i), g_rw(0xc867));
        g_wb((u16)(GV_cbt_action + i), 7);
        player_melee[i] = 0;
        fprintf(stderr, "orders: member %d had no orders: opens fire at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* a shooter who fought a foe beside it: with none beside it now, the missile weapon again, at the
   next foe (as the game readies it for Use Missile, s014 19cd order_mode_leave) */
static void back_to_bow(void)
{
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        int f = (s16)g_rw((u16)(c + 0x10)), wm = (s8)g_rb((u16)(c + 0x22));
        /* hand to hand by our doing (by_volley) or the game's (its AI turns a shooter set upon to Parry or a
           Std Attack), not the player's */
        int melee = f == STD_ATTACK || f == 0x0a || f == 0x22 || f == 6;
        if (f == MISSILE) player_melee[i] = 0;
        if (!by_volley[i] && !(melee && !player_melee[i])) continue;
        if (g_rb((u16)(c + 0x14)) != 1 || !melee || wm < 0 || wm > 62 || orders_member_mode(i) != 1 || g_rb((u16)(GV_cbt_moving + i))) { if (!melee) by_volley[i] = 0; continue; }   /* (fallen, shooting, walking where sent, set to fight, on its way) */
        int beside = 0;
        for (int j = 5; j < 13; j++) if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) beside = 1;
        if (beside) continue;
        /* a foe within a few paces coming at it: the fight is about to reach it, the bow would only be
           put away again (each change of weapon starts the reloading over: a shooter switched back and
           forth never shoots) */
        { int coming = 0;
          for (int j = 5; j < 13; j++) if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) == 1 && near_foe_of(i, j) && (s8)g_rb((u16)(D_CBT + j * 0x80 + 0x4a)) == i) coming = 1;
          if (coming) continue; }
        if (rescue_target(i) >= 0) continue;              /* (a friend set upon close by: stays to help, rescue_target) */
        /* the game took the bow away again just after it was given back: let the game have its way
           for a while (its AI sees a fight the member is in) */
        { u16 now = g_rw(GV_combat_clock);
          if (bow_given[i] && (u16)(now - bow_given[i]) < 90) { if (++bow_flips[i] >= 2) { bow_hold[i] = now ? now : 1; bow_flips[i] = 0; } continue; }
          if (bow_hold[i] && (u16)(now - bow_hold[i]) < 600) continue;
          bow_hold[i] = 0; }
        int clear = 0, t = shot_target(i, &clear);
        if (ns_unwinding) return;
        by_volley[i] = 0;
        if (t < 0) continue;
        ns_call(GF_weapon_ready, 3, i, wm, 0x80);
        g_wb((u16)(c + 0x4a), (u8)t); g_ww((u16)(c + 0x10), MISSILE);
        g_wb((u16)(GV_cbt_moving + i), 0); g_wb((u16)(GV_cbt_state + i), 0);
        g_ww((u16)(GV_cbt_flags + 2 * i), (u16)(g_rw((u16)(GV_cbt_flags + 2 * i)) | 2));
        g_ww((u16)(GV_cbt_engage_time + 2 * i), g_rw(0xc867));
        g_wb((u16)(GV_cbt_action + i), 7);              /* "ready", as a click's order leaves it: the game then reloads, aims and looses (from 0 it never starts) */
        bow_given[i] = g_rw(GV_combat_clock); if (!bow_given[i]) bow_given[i] = 1;
        fprintf(stderr, "orders: member %d has no foe beside it now: the missile weapon again, at %d (%s)\n", i, t, clear ? "a clear line" : "no clear line: steps aside for one");
    }
}
/* test (uattack:i,t): member i told to fight t hand to hand, as the player's order gives it (Std Attack;
   the bow is not taken up again for it) */
void orders_test_attack(int i, int t)
{
    if (i < 0 || i > 4 || t < 5 || t > 12) return;
    to_melee(i, t, rb(G_FAR20A5, (u16)(0x7900 + i * 13 + t)) != 0);
    by_volley[i] = 0; player_melee[i] = 1;
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
    u16 c = (u16)(D_CBT + i * 0x80);
    int wm = (s8)g_rb((u16)(c + 0x22));
    if (wm < 0 || wm > 62) { fprintf(stderr, "orders: test: %d has no missile weapon\n", i); return; }
    ns_call(GF_weapon_ready, 3, i, wm, 0x80);
    g_wb((u16)(c + 0x4a), (u8)t); g_ww((u16)(c + 0x10), MISSILE);
    g_wb((u16)(GV_cbt_moving + i), 0); g_wb((u16)(GV_cbt_state + i), 0);
    g_ww((u16)(GV_cbt_flags + 2 * i), (u16)(g_rw((u16)(GV_cbt_flags + 2 * i)) | 2));
    g_ww((u16)(GV_cbt_engage_time + 2 * i), g_rw(0xc867));
    g_wb((u16)(GV_cbt_action + i), 7);
    fprintf(stderr, "orders: test: %d shoots at %d\n", i, t);
}
/* a Parry, Berserk or Vulnerable order kept: the game turns a member who is set upon on the way
   to its foe (or whose foe falls) to a Std Attack on whoever it fights; the kind the player chose
   is kept for that fight too, until the member is given another order */
static int kept_kind[5] = { -1, -1, -1, -1, -1 };
static void keep_kinds(void)
{
    if (!combat_improved()) return;
    for (int i = 0; i < 5; i++) {
        if (kept_kind[i] < 0) continue;
        u16 c = (u16)(D_CBT + i * 0x80);
        if (g_rb((u16)(c + 0x14)) != 1) { kept_kind[i] = -1; continue; }
        int f = (s16)g_rw((u16)(c + 0x10));
        if (f == STD_ATTACK) { g_ww((u16)(c + 0x10), (u16)kept_kind[i]); if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: member %d keeps its %#x\n", i, kept_kind[i]); }
        else if (f == MISSILE || f == 0) kept_kind[i] = -1;                 /* (another kind of order now) */
    }
}
static void melee_hook(void)                    /* s014 0e76:2a4a melee_exchanges() */
{
    hook_call_original();
    if (ns_unwinding || g_rb(GV_ai_paused)) return;
    { extern int arena_log_every; void arena_tick(void); if (arena_log_every) arena_tick(); }   /* (the test arena's log: runtime/testcmds.c) */
    keep_kinds();
    retarget_shooters();
    if (combat_improved()) {                    /* a shooter standing idle with its order (Use Missile, action 0, not walking): made ready */
        static u16 idle_since[5];
        u16 now = g_rw(GV_combat_clock);
        for (int i = 0; i < 5; i++) {
            u16 c = (u16)(D_CBT + i * 0x80);
            int t = (s8)g_rb((u16)(c + 0x4a));
            int idle = g_rb((u16)(c + 0x14)) == 1 && g_rw((u16)(c + 0x10)) == MISSILE && g_rb((u16)(GV_cbt_action + i)) == 0
                        && !g_rb((u16)(GV_cbt_moving + i)) && g_rb((u16)(GV_cbt_state + i)) == 0 && t >= 5 && t < 13 && g_rb((u16)(D_CBT + t * 0x80 + 0x14)) == 1;
            if (!idle) { idle_since[i] = 0; continue; }
            if (!idle_since[i]) { idle_since[i] = now ? now : 1; continue; }
            if ((u16)(now - idle_since[i]) < 40) continue;
            g_wb((u16)(GV_cbt_action + i), 7); idle_since[i] = 0;
            fprintf(stderr, "orders: member %d stood idle with its bow: ready again, at %d\n", i, t);
        }
    }
    if (combat_improved()) back_to_bow();
    if (ns_unwinding) return;
    idle_shooters();
    if (ns_unwinding) return;
    if (!volley_on()) return;
    for (int i = 0; i < 5; i++) {
        u16 c = (u16)(D_CBT + i * 0x80);
        if (g_rb((u16)(c + 0x14)) != 1 || g_rw((u16)(c + 0x10)) != MISSILE || g_rb((u16)(GV_cbt_moving + i))) continue;
        if (g_rb((u16)(GV_cbt_mods + 0x12 * i + 9)) == 1) continue;           /* (told to hold) */
        int t = (s8)g_rb((u16)(c + 0x4a)), near = -1;
        for (int j = 5; j < 13; j++)
            if (g_rb((u16)(D_CBT + j * 0x80 + 0x14)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) { if (near < 0 || j == t) near = j; }
        if (near >= 0) { to_melee(i, near, 1); continue; }
        { int r = rescue_target(i);                                       /* a friend fighting a few paces off: help it */
          if (ns_unwinding) return;
          if (r >= 0) { fprintf(stderr, "orders: member %d goes to help: %d fights one of the party a few paces off\n", i, r); to_melee(i, r, 0); continue; } }
        /* (a foe some paces off, coming: the shooter goes on shooting it, and fights hand to hand once it is beside) */
        if (t < 5 || t >= 13 || g_rb((u16)(D_CBT + t * 0x80 + 0x14)) != 1) continue;
        int st = g_rb((u16)(GV_cbt_state + i)), engaged = 0;
        for (int p = 0; p < 5; p++) if (p != i && rb(G_FAR20A5, (u16)(0x7900 + p * 13 + t))) engaged = 1;
        if (st == 2) to_melee(i, t, 0);                                  /* (nothing left to shoot; a line blocked is the stepping aside's: mods/battlerules.c) */
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
    if (m < 5 || m >= 13 || g_rb((u16)(D_CBT + m * 0x80 + 0x14)) != 1) return 0;
    int act = (s16)g_rw(D_ORDER_ACTION), f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10)), mode = 0;
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
    int f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    int m = topmost(x, y);
    if (m < 5 || m >= 13 || g_rb((u16)(D_CBT + m * 0x80 + 0x14)) != 1) return 0;
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
    if (act == -1) act = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    if (ct >= 5 && g_rw(D_BOX_COLOUR) == g_rw(D_REFUSED)) ct = -4;      /* (a foe it cannot reach or shoot at) */
    hint_sel = i; hint_act = act; hint_ct = ct;
    if (getenv("DL_ORDERS_LOG")) fprintf(stderr, "orders: pointer test: member %d, order %#x, target %d\n", i, act, ct);
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
    int t = (s8)g_rb((u16)(c + 0x4a)), f = (s16)g_rw((u16)(c + 0x10));
    if (t >= 5 && t < 13 && ((f & 2) || f == 1) && g_rb((u16)(D_CBT + t * 0x80 + 0x14)) == 1) foe_pending = t;   /* (fighting it, or walking to it to fight) */
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
        if (g_rb((u16)(c + 0x14)) != 1 || g_rw((u16)(c + 0x10)) != 0 || g_rb((u16)(GV_cbt_state + m)) || g_rb((u16)(GV_cbt_floor + m)) != g_rb(GV_floor_shown)) continue;
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
    if (!quick || !selected_ok()) return;
    int ct = (s8)g_rb(GV_cursor_target);
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
