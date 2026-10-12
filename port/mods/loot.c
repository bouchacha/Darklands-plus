/* loot.c - the loot screen and the shops, one at a time no more.
 * The loot screen (s040, after a battle or a find): the game hands out one item at
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

/* ---------------------------------------------------------------- shops
 * The barter screen (s032) also sells and buys one at a time: A sells the whole stack under
 * the cursor (as S on each), T buys ten of the merchant's item under the cursor (as P, while
 * the purse lasts). s032 state (DGROUP): 884e the person bartering, 8842[5] 1 = the cursor in
 * the merchant's list, 885e[5] + 8852[5] the cursor in the person's list, 8838[5] the
 * person's number of items. Barter key handler: s032 1462:0f60. */
#define B_WHO  0x884e
#define B_SIDE 0x8842
#define B_TOP  0x885e
#define B_SEL  0x8852

int barter_active;
static u32 purse(void) { return ((u32)g_rw(G_MONEY) << 20) ^ ((u32)g_rw(G_MONEY + 2) << 10) ^ g_rw(G_MONEY + 4); }

static void barter_many(int buy)
{
    void (*key_fn)(void) = game_func(32, 0x1462, 0x0f60);
    if (!key_fn) return;
    int m = (s16)g_rw(B_WHO);
    if (m < 0 || m >= 5) return;
    char msg[120];
    int n = 0;
    if (buy) {
        if (!g_rw((u16)(B_SIDE + 2 * m))) { ui_notice("Point at one of the merchant's goods first (the upper list)."); return; }
        for (int i = 0; i < 10; i++) {
            u32 before = purse();
            u16 a[1] = { 0x1970 };
            game_call_live(key_fn, 1, a);
            if (purse() == before) break;          /* no money left, or no room */
            n++;
        }
        snprintf(msg, sizeof msg, n ? "Bought %d." : "Bought none: not enough money, or no room.", n);
    } else {
        if (g_rw((u16)(B_SIDE + 2 * m))) { ui_notice("Point at one of your own things first (the lower list)."); return; }
        int idx = (s16)g_rw((u16)(B_TOP + 2 * m)) + (s16)g_rw((u16)(B_SEL + 2 * m));
        if (idx < 0 || idx >= g_rb((u16)(G_MEMBER(m) + G_M_NITEMS))) return;
        u16 o = G_ITEM(m, idx), id = rw(G_FAR2E38, o);
        int q = rb(G_FAR2E38, (u16)(o + 3)), cnt = rb(G_FAR2E38, (u16)(o + 4));
        for (int k = 0; k < cnt && k < 255; k++) {
            int i2 = (s16)g_rw((u16)(B_TOP + 2 * m)) + (s16)g_rw((u16)(B_SEL + 2 * m));
            if (i2 != idx || i2 >= g_rb((u16)(G_MEMBER(m) + G_M_NITEMS))) break;
            u16 o2 = G_ITEM(m, i2);
            if (rw(G_FAR2E38, o2) != id || rb(G_FAR2E38, (u16)(o2 + 3)) != q) break;    /* the stack is gone */
            u32 before = purse();
            u16 a[1] = { 0x1f73 };
            game_call_live(key_fn, 1, a);
            if (purse() == before) break;          /* the merchant will not take it */
            n++;
        }
        snprintf(msg, sizeof msg, n ? "Sold %d." : "This merchant does not buy that.", n);
    }
    ui_notice(msg);
    if (getenv("DL_LOOT_LOG")) fprintf(stderr, "shop: %s\n", msg);
}

static void barter_key_hook(void)          /* s032 1462:0f60 barter_key(key): the key back, 0x11b to leave */
{
    int key = rw(SS, (u16)(SP + 4)) & 0xff;
    if (loot_enabled() && (key == 'a' || key == 'A' || key == 't' || key == 'T')) {
        barter_many(key == 't' || key == 'T');
        AX = 0;
        hook_return_far(0);
        return;
    }
    hook_call_original();
}

static void barter_hook(void)              /* s032 1462:0000 barter_screen(shop, name, cats) */
{
    barter_active++;
    hook_call_original();
    barter_active--;
}

/* the buttons beside the game's menus (ui/overlay.cpp): as the keys */
void loot_press(int i)
{
    static const u16 LOOT[2] = { 0x1e61, 0x1f73 }, SHOP[2] = { 0x1e61, 0x1474 };
    if (i < 0 || i > 1) return;
    bios_kbd_inject(loot_active ? LOOT[i] : SHOP[i]);
}

void loot_init(void)
{
    hook_install(40, 0x1462, 0x0c22, key_hook);
    hook_install(40, 0x1462, 0x01c8, screen_hook);
    hook_install(32, 0x1462, 0x0f60, barter_key_hook);
    hook_install(32, 0x1462, 0x0000, barter_hook);
}
