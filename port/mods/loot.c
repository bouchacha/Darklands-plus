/* loot.c - the loot screen (s040, after a battle or a find): the game hands out one item at
 * a time, and a stack of 30 arrows is 30 presses of G. Two keys (and buttons, ui/overlay.cpp)
 * take the whole pile:
 *   A  everything to the person shown (as G on every item), and
 *   S  shared out: each item to whoever already carries some of it, else to whoever has the
 *      most free places (a member holds at most 64 kinds of things; only what is worn or
 *      held in hand counts towards the load, so weight does not matter here).
 * Both work through the game's own G (s040 1462:0c22, key 0x2247), so the game does the
 * adding, stacking and drawing; a member with 64 kinds of things only gets what stacks.
 * s040 state (DGROUP): 8dc4 the person shown, 8dbc entries in the pile, 8db6 far pointer to
 * them (6 bytes: item, type, quality, count, weight), 4c10 side[5] (1 = the pile's cursor),
 * 4c24 pile_top[5], 4c38 pile_sel[5], a6b4 cbt_floor[] (-99: on another floor). */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/hooks.h"
#include "../runtime/hle.h"
#include "../runtime/config.h"

#define L_CUR   0x8dc4
#define L_N     0x8dbc
#define L_PILE  0x8db6
#define L_SIDE  0x4c10
#define L_PTOP  0x4c24
#define L_PSEL  0x4c38
#define L_FLOOR 0xa6b4

int loot_active;                         /* the loot screen's loop is running */
void ui_notice(const char *msg);

int loot_enabled(void) { return config_get_int("assist.loot_all", 1) && !config_get_int("original", 0); }

static u16 pile_seg(void) { return g_rw(L_PILE + 2); }
static u16 pile_off(int i) { return (u16)(g_rw(L_PILE) + 6 * i); }
static int pile_units(void)
{
    int n = (s16)g_rw(L_N), t = 0;
    for (int i = 0; i < n && i < 200; i++) t += rb(pile_seg(), (u16)(pile_off(i) + 4));
    return t;
}
static int item_type(int id)
{
    u32 d = g_itemdef(id & 0xfff);
    return d ? rb((u16)(d >> 16), (u16)((u16)d + 30)) : 0;
}
/* does member m carry this item already (it stacks onto that record) */
static int stacks(int m, u16 code, int qual)
{
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
    u8 type = (u8)item_type(code);
    for (int j = 0; j < n && j < 64; j++) {
        u16 o = G_ITEM(m, j);
        if (rw(G_FAR2E38, o) == code && rb(G_FAR2E38, (u16)(o + 2)) == type && rb(G_FAR2E38, (u16)(o + 3)) == qual && rb(G_FAR2E38, (u16)(o + 4)) < 0xff) return 1;
    }
    return 0;
}
/* can member m take one more of this item: a free record, or one it stacks onto */
static int room_for(int m, u16 code, int qual) { return g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)) < 64 || stacks(m, code, qual); }
static int able(int m)
{
    int st = g_rb(G_MEMBER(m));
    return st != 0 && st != 3 && (s16)g_rw((u16)(L_FLOOR + 2 * m)) != -99;
}

static void take_all(int share)
{
    void (*key_fn)(void) = game_func(40, 0x1462, 0x0c22), (*redraw)(void) = game_func(40, 0x1462, 0x0952);
    if (!key_fn) return;
    int size = (s16)g_rw(G_PARTY_SIZE); if (size < 1 || size > 5) return;
    int cur = (s16)g_rw(L_CUR); if (cur < 0 || cur >= size) return;
    int taken = 0, j = 0, guard = 0;
    while (j < (s16)g_rw(L_N) && guard++ < 4000) {
        u16 o = pile_off(j);
        u16 code = rw(pile_seg(), o); int qual = rb(pile_seg(), (u16)(o + 3));
        int m = -1;
        if (!share) m = room_for(cur, code, qual) ? cur : -1;
        else {
            int best = -1;
            for (int k = 0; k < size; k++) {
                if (!able(k) || !room_for(k, code, qual)) continue;
                int score = stacks(k, code, qual) ? 1000 : 2 * (64 - g_rb((u16)(G_MEMBER(k) + G_M_NITEMS))) + (k == cur);   /* (the person shown first, on a tie) */
                if (score > best) { best = score; m = k; }
            }
        }
        if (m < 0) { j++; continue; }                                 /* this one stays: on to the next */
        g_ww(L_CUR, (u16)m); g_ww((u16)(L_SIDE + 2 * m), 1); g_ww((u16)(L_PTOP + 2 * m), (u16)j); g_ww((u16)(L_PSEL + 2 * m), 0);
        int before = pile_units();
        u16 a[1] = { 0x2247 };
        game_call_live(key_fn, 1, a);
        if (pile_units() >= before) break;                            /* the game took nothing: stop */
        taken++;
    }
    g_ww(L_CUR, (u16)cur);
    for (int m = 0; m < size; m++) { g_ww((u16)(L_SIDE + 2 * m), 1); g_ww((u16)(L_PTOP + 2 * m), 0); g_ww((u16)(L_PSEL + 2 * m), 0); }
    if (redraw) { u16 a[1] = { (u16)cur }; game_call_live(redraw, 1, a); }
    int left = pile_units();
    char msg[160];
    if (!taken && left) snprintf(msg, sizeof msg, share ? "Nobody has room: everyone carries 64 kinds of things already."
                                                        : "No room: this person carries 64 kinds of things already (S shares it out).");
    else if (left) snprintf(msg, sizeof msg, "%s %d; %d left in the pile (no room for more kinds of things).", share ? "Shared out" : "Took", taken, left);
    else snprintf(msg, sizeof msg, share ? "Shared out all %d." : "Took all %d.", taken);
    ui_notice(msg);
    if (getenv("DL_LOOT_LOG")) fprintf(stderr, "loot: %s\n", msg);
}

static void key_hook(void)                 /* s040 1462:0c22 loot_key(key) */
{
    int key = rw(SS, (u16)(SP + 4)) & 0xff;
    if (loot_enabled() && (key == 'a' || key == 'A' || key == 's' || key == 'S')) {
        take_all(key == 's' || key == 'S');
        hook_return_far(0);
        return;
    }
    hook_call_original();
}

static void screen_hook(void)              /* s040 1462:01c8 the loot screen's loop */
{
    loot_active++;
    hook_call_original();
    loot_active--;
}

void loot_press(int share) { bios_kbd_inject(share ? 0x1f73 : 0x1e61); }   /* the buttons: as the keys */

void loot_init(void)
{
    hook_install(40, 0x1462, 0x0c22, key_hook);
    hook_install(40, 0x1462, 0x01c8, screen_hook);
}
