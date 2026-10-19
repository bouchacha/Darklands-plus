/* orders.c - in a battle, a member who is not yet fighting can be sent at an enemy with a click
 * on it (F12 > Settings > Helpers; "Play the original game" leaves the game's way).
 *
 * The game's way: click a member (order mode), choose an attack in the order menu (Std Attack,
 * Vulnerable, Berserk, Parry), then click the enemy. A member whose standing order is only to walk
 * (or nothing) takes a click on an enemy as a click on the ground under it, which is taken: "can't
 * go there" (s014 19cd:0dcc / 0ad4 skip the enemies' hot spots unless an attack is chosen). Here,
 * that click (and the pointer over the enemy) is taken as Std Attack (0x12, what the menu's first
 * attack sets) on that enemy, or as Use Missile (0x80) for a member better with the missile weapon
 * (battle.click_best); the rest is the game's own order (walk there and fight, or shoot). */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"

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

/* a foe whose hot spot (head to waist, 8 pixels wide) is near (x, y): a little beside it or at its
   feet - where a click meant for the foe lands as often as not, and the game took it for the ground
   beside it (the member walked there). The centre of the nearest such spot, or 0. */
static int near_foe(int x, int y, int *cx, int *cy)
{
    int n = (s8)g_rb(D_HOT_N), best = -1; long bd = 1L << 30;
    for (int k = 0; k < n; k++) {
        int c = g_rb((u16)(D_HOT_CBT + k));
        if (c < 5 || c >= 13 || g_rb((u16)(D_CBT + c * 0x80 + 0x14)) != 1) continue;
        int x0 = (s16)g_rw((u16)(D_HOT_X0 + 2 * k)), x1 = (s16)g_rw((u16)(D_HOT_X1 + 2 * k));
        int y0 = (s16)g_rw((u16)(D_HOT_Y0 + 2 * k)), y1 = (s16)g_rw((u16)(D_HOT_Y1 + 2 * k));
        if (x < x0 - 5 || x > x1 + 5 || y < y0 - 3 || y > y1 + 10) continue;
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

/* the attack a click sends member i at foe m with: Use Missile (0x80) when the member is better
   with the missile weapon in hand (+0x22, its type) than with the default one (+0x51) - by the
   skill each wants (far 20a5:75ac) - can loose it (can_fire, s014 18bf:0c02) and the foe is not
   already within reach of a blow (far 20a5:7900, ai 1db7:09ae); else Std Attack */
static int best_attack(int i, int m)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    int wm = (s8)g_rb((u16)(c + 0x22)), wd = (s8)g_rb((u16)(c + 0x51));
    if (wm < 0 || wm > 62 || !config_get_int("battle.click_best", 1)) return STD_ATTACK;
    if (rb(G_FAR20A5, (u16)(0x7900 + i * 13 + m))) return STD_ATTACK;          /* beside it: hand to hand */
    int skm = rb(G_FAR20A5, (u16)(0x75ac + wm)), skd = wd >= 0 && wd <= 62 ? rb(G_FAR20A5, (u16)(0x75ac + wd)) : -1;
    int vm = skm < 19 ? (s8)g_rb((u16)(G_MEMBER(i) + G_M_SKILLS + skm)) : 0, vd = skd >= 0 && skd < 19 ? (s8)g_rb((u16)(G_MEMBER(i) + G_M_SKILLS + skd)) : 0;
    if (vm <= vd) return STD_ATTACK;
    if (!ns_call(GF_can_fire, 1, i)) return STD_ATTACK;
    /* a clear line from where the member stands (line_blocked, s014 1d69:000c, as the order's pointer
       tests it): else the member closes in instead */
    u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
    g_ww(GV_los_x0, g_rw((u16)(c + 4))); g_ww(GV_los_y0, g_rw((u16)(c + 6)));
    g_ww(GV_los_x1, g_rw((u16)(D_CBT + m * 0x80 + 4))); g_ww(GV_los_y1, g_rw((u16)(D_CBT + m * 0x80 + 6)));
    int blocked = ns_call(GF_line_blocked, 3, i, m, 0x80);
    g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
    return blocked ? STD_ATTACK : MISSILE;
}

/* the order the click now being given should take on what is under (x, y): 0 the game's own */
static int wants_attack(int x, int y)
{
    if (!orders_click_attack || config_get_int("original", 0)) return 0;
    if (g_rb(D_ORDER_MODE) != 1 || (s16)g_rw(D_ORDER_ACTION) != -1) return 0;
    int i = (s8)g_rb(D_SELECTED);
    if (i < 0 || i > 4) return 0;
    int f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    if (f != 0 && f != 1) return 0;                     /* already fighting: the game's own way */
    int m = topmost(x, y);
    if (m < 5 || m >= 13 || g_rb((u16)(D_CBT + m * 0x80 + 0x14)) != 1) return 0;
    return best_attack(i, m);
}

static void hover_hook(void)                    /* s014 19cd:0ad4 (x, y): the pointer in order mode */
{
    snap_to_foe();
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int set = wants_attack(x, y);
    if (set) g_ww(D_ORDER_ACTION, (u16)set);
    hook_call_original();
    if (set) g_ww(D_ORDER_ACTION, 0xffff);              /* only the look of the pointer */
}

static void click_hook(void)                    /* s014 19cd:0dcc (x, y): a click in order mode */
{
    snap_to_foe();
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int set = wants_attack(x, y), m = topmost(x, y);
    if (set) {
        g_ww(D_ORDER_ACTION, (u16)set);                 /* as if the attack had been chosen first */
        fprintf(stderr, "orders: member %d %s %d (clicked at %d,%d)\n", (s8)g_rb(D_SELECTED), set == MISSILE ? "shoots at" : "attacks", m, x, y);
    }
    /* a foe clicked: a spot on the ground clicked before in this order is no longer the way there
       (order_mode_leave, s014 19cd:0738, makes it the member's goal while the path goes to the foe:
       "walking" to beside the foe) */
    if (m >= 5 && orders_click_attack && !config_get_int("original", 0) && g_rw(GV_order_dest_x)) { g_ww(GV_order_dest_x, 0); g_ww(GV_order_dest_y, 0); }
    hook_call_original();
}

void orders_init(void)
{
    orders_click_attack = config_get_int("battle.click_attack", 1);
    hook_install(14, 0x19cd, 0x0ad4, hover_hook);
    hook_install(14, 0x19cd, 0x0dcc, click_hook);
}
