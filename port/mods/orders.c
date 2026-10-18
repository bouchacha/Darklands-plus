/* orders.c - in a battle, a member who is not yet fighting can be sent at an enemy with a click
 * on it (F12 > Settings > Helpers; "Play the original game" leaves the game's way).
 *
 * The game's way: click a member (order mode), choose an attack in the order menu (Std Attack,
 * Vulnerable, Berserk, Parry), then click the enemy. A member whose standing order is only to walk
 * (or nothing) takes a click on an enemy as a click on the ground under it, which is taken: "can't
 * go there" (s014 19cd:0dcc / 0ad4 skip the enemies' hot spots unless an attack is chosen). Here,
 * that click (and the pointer over the enemy) is taken as Std Attack (0x12, what the menu's first
 * attack sets) on that enemy; the rest is the game's own order (walk there, then fight). */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

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

/* 1 when the order now being given should be Std Attack on what is under (x, y) */
static int wants_attack(int x, int y)
{
    if (!orders_click_attack || config_get_int("original", 0)) return 0;
    if (g_rb(D_ORDER_MODE) != 1 || (s16)g_rw(D_ORDER_ACTION) != -1) return 0;
    int i = (s8)g_rb(D_SELECTED);
    if (i < 0 || i > 4) return 0;
    int f = (s16)g_rw((u16)(D_CBT + i * 0x80 + 0x10));
    if (f != 0 && f != 1) return 0;                     /* already fighting: the game's own way */
    int m = topmost(x, y);
    return m >= 5 && m < 13 && g_rb((u16)(D_CBT + m * 0x80 + 0x14)) == 1;
}

static void hover_hook(void)                    /* s014 19cd:0ad4 (x, y): the pointer in order mode */
{
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    int set = wants_attack(x, y);
    if (set) g_ww(D_ORDER_ACTION, STD_ATTACK);
    hook_call_original();
    if (set) g_ww(D_ORDER_ACTION, 0xffff);              /* only the look of the pointer */
}

static void click_hook(void)                    /* s014 19cd:0dcc (x, y): a click in order mode */
{
    int x = (s16)rw(SS, (u16)(SP + 4)), y = (s16)rw(SS, (u16)(SP + 6));
    if (wants_attack(x, y)) {
        g_ww(D_ORDER_ACTION, STD_ATTACK);               /* as if Std Attack had been chosen first */
        fprintf(stderr, "orders: member %d attacks what was clicked\n", (s8)g_rb(D_SELECTED));
    }
    hook_call_original();
}

void orders_init(void)
{
    orders_click_attack = config_get_int("battle.click_attack", 1);
    hook_install(14, 0x19cd, 0x0ad4, hover_hook);
    hook_install(14, 0x19cd, 0x0dcc, click_hook);
}
