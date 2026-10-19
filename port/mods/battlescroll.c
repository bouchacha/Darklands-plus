/* battlescroll.c - scrolling the battle map at a pace a person can follow.
 *
 * The original scrolls the battle view one step each time combat_input (s014 0e76:3166) runs
 * while Shift is held and the pointer is on an edge of the map window (the pointer turns into
 * an arrow there). On a 1992 PC every step took a visible moment to redraw; here the loop runs
 * hundreds of times a second and the view flies to the edge of the field at the lightest
 * touch of Shift. Now the steps are paced (a few a second at first, faster while held), and,
 * optionally, resting the pointer on the edge scrolls without Shift.
 * The pacing works by hiding Shift (BIOS 0040:0017) from combat_input on the calls that may
 * not scroll, and only while the pointer is on an edge, where Shift does nothing else. */
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include <stdio.h>

u32 plat_ticks_ms(void);
void plat_mouse_game(int *x, int *y);

int battlescroll_on = 1;          /* paced steps */
int battlescroll_rest = 1;        /* the pointer resting on the edge scrolls (no Shift needed) */

static int on_edge(int mx, int my)  /* the original's arrow zones (combat_input, cursor 33a2) */
{
    if (mx < 0x3c) return 0;
    return my < 8 || my > 0xbf || mx < 0x44 || mx >= 0x137;
}

/* the view moved by whole steps of its origin (D_c866: one step moves the picture 32 pixels
 * sideways; D_c886: 16 aside and 16 up), asked from outside (a drag of the field, the platform's
 * mouse) and done here, at the start of combat_input, where the game scrolls itself: vec_1d3d
 * (s016 1ef8:0b92) clamps the origin to 1..0x27 and draws the view anew */
static int pan_h, pan_v;
void battlescroll_pan(int h, int v) { pan_h += h; pan_v += v; }
static void pan_apply(void)
{
    if (!pan_h && !pan_v) return;
    int h = pan_h, v = pan_v; pan_h = pan_v = 0;
    if (game_mode() != GM_BATTLE) return;
    u8 c = g_rb(GV_battle_view_col), r = g_rb(GV_battle_view_row);
    g_wb(GV_battle_view_col, (u8)((s8)c + h)); g_wb(GV_battle_view_row, (u8)((s8)r + v));
    ns_call(16, 0x1ef8, 0x0b92, 0);
    if (getenv("DL_SCROLL_LOG")) fprintf(stderr, "battlescroll: pan %d across, %d down: origin %d,%d\n", h, v, (s8)g_rb(GV_battle_view_col), (s8)g_rb(GV_battle_view_row));
}

/* 1..5 in a battle (s016 1ef8:0a2c (i): the member chosen to be given orders) sets the view to follow
 * that member (D_11b1), which jumps whenever the member stands near an edge. With battle.select_stays
 * the view stays where it is when the member is in sight (it has a hot spot: the order then needs
 * no scrolling); pressed twice within a moment, the view goes to the member (floor_view_draw, s017
 * 1ef8:0000, around who) and follows. */
static int center_req = -1;
static void select_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    static u32 last_t; static int last_i = -1;
    u32 now = plat_ticks_ms();
    int twice = i == last_i && now - last_t < 450;
    last_i = i; last_t = now;
    hook_call_original();
    if (!config_get_int("battle.select_stays", 1) || config_get_int("original", 0) || i < 0 || i >= 5) return;
    if (twice) { center_req = i; return; }
    int n = (s8)g_rb(GV_hotspot_count), seen = 0;
    for (int k = 0; k < n && k < 40; k++) if (g_rb((u16)(GV_hotspot_cbt + k)) == i) seen = 1;
    if (seen) g_wb(GV_battle_follow, 0);                               /* (in sight: no following, no jump) */
}

static void input_hook(void)        /* s014 0e76:3166 combat_input(): one tick of player input */
{
    pan_apply();
    if (center_req >= 0) { int c = center_req; center_req = -1; if (game_mode() == GM_BATTLE) { ns_call(GF_floor_view_draw, 1, c); g_wb(GV_battle_follow, 1); } }
    if (!battlescroll_on || config_get_int("original", 0)) { hook_call_original(); return; }
    static u32 last_step, edge_since, held_since;
    static int was_edge;
    int mx, my; plat_mouse_game(&mx, &my);
    u32 now = plat_ticks_ms();
    int plat_pointer_outside(void);
    int edge = on_edge(mx, my) && !plat_pointer_outside();     /* (beside the picture, over the margins: no) */
    if (edge && !was_edge) edge_since = now;
    was_edge = edge;
    if (!edge) { held_since = 0; hook_call_original(); return; }
    u8 f = rb(0x40, 0x17);
    int shift = (f & 3) != 0;
    int want = shift || (battlescroll_rest && now - edge_since >= 300);
    if (want) {
        if (!held_since) { held_since = now; last_step = 0; }
        u32 held = now - held_since;
        u32 every = held < 700 ? 150 : held < 1800 ? 100 : 60;   /* a few steps a second, then faster */
        if (last_step && now - last_step < every) want = 0;
        else last_step = now;
    } else held_since = 0;
    u8 g = (u8)(want ? (f | 1) : (f & ~3));
    wb(0x40, 0x17, g);
    hook_call_original();
    if (rb(0x40, 0x17) == g) wb(0x40, 0x17, f);         /* (unless a key changed it meanwhile) */
}

void battlescroll_set(int on, int rest)
{
    battlescroll_on = on; battlescroll_rest = rest;
    config_set_int("battle.scroll_paced", on); config_set_int("battle.scroll_rest", rest);
    hook_install(GF_combat_input, input_hook);
    hook_install(16, 0x1ef8, 0x0a2c, select_hook);
}

void battlescroll_init(void)
{
    battlescroll_set(config_get_int("battle.scroll_paced", 1), config_get_int("battle.scroll_rest", 1));
}
