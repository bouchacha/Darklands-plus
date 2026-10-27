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
    ns_call(16, 0x1ef8, 0x0b92, 0);                     /* (the last whole frame stays shown: view_redraw_hook) */
    if (getenv("DL_SCROLL_LOG")) fprintf(stderr, "battlescroll: pan %d across, %d down: origin %d,%d\n", h, v, (s8)g_rb(GV_battle_view_col), (s8)g_rb(GV_battle_view_row));
}

/* 1..5 in a battle (s016 1ef8:0a2c (i): the member chosen to be given orders) sets the view to follow
 * that member (D_11b1), which jumps whenever the member stands near an edge. With battle.select_stays
 * the view stays where it is when the member is in sight (it has a hot spot: the order then needs
 * no scrolling); pressed twice within a moment, the view goes to the member (floor_view_draw, s017
 * 1ef8:0000, around who) and follows. */
static int center_req = -1, look_req = -1;
/* the view to combatant i, without following it (the arrow to the enemies: ui/battleoverlay.cpp) */
void battlescroll_look(int i) { look_req = i; }
#define D_PENDING     0xd6f1      /* char: a member's choice waiting for their spot on the screen (s016 1ef8:0a2c) */
#define D_PENDING_WHO 0xd6ed      /* who */
static int has_spot(int i)
{
    int n = (s8)g_rb(GV_hotspot_count);
    for (int k = 0; k < n && k < 40; k++) if (g_rb((u16)(GV_hotspot_cbt + k)) == i) return 1;
    return 0;
}
static void select_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    static u32 last_t; static int last_i = -1;
    u32 now = plat_ticks_ms();
    int twice = i == last_i && now - last_t < 450;
    last_i = i; last_t = now;
    int stays = config_get_int("battle.select_stays", 1) && !config_get_int("original", 0) && i >= 0 && i < 5 && game_mode() == GM_BATTLE;
    /* off the screen on this floor: the game's choice waits (D_d6f1, s014 combat_main) until the
       member has a spot on the screen, and makes the view follow them there. Chosen here at once
       instead, as the game does it (order_mode_enter), the view left where it is - choosing shooters
       far from their marks without the view swinging to and fro */
    if (stays && !twice && g_rb((u16)(GV_cbt_floor + i)) == g_rb(GV_floor_shown) && !has_spot(i)) {
        hook_call_original();
        if (g_rb(D_PENDING) == 1 && (s8)g_rb(D_PENDING_WHO) == i) {
            g_wb(GV_battle_follow, 0);
            g_wb(GV_selected_member, (u8)i);
            ns_call(GF_order_mode_enter, 0);
            g_wb(D_PENDING, 0);
            if (getenv("DL_SCROLL_LOG")) fprintf(stderr, "battlescroll: %d chosen off the screen, the view kept\n", i);
        }
        return;
    }
    hook_call_original();
    if (!stays) return;
    if (twice) { center_req = i; return; }
    /* only a member on another floor is followed there, the view having nothing of them to show */
    if (g_rb((u16)(GV_cbt_floor + i)) != g_rb(GV_floor_shown)) return;
    g_wb(GV_battle_follow, 0);
}

/* s016 1ef8:0b92 (vec_1d3d): the view's origin clamped to the field and its floor drawn anew, by the game's
   own scrolling (Shift or the pointer resting on an edge: one step each time combat_input runs) and by a
   drag (pan_apply). The people come with the next combat frame (combat_draw); until then the window keeps
   showing the last whole frame. Else each step showed the field empty for a moment - at the field's edge,
   where the view no longer moves, at every step: the people and the floor flickered */
static void view_redraw_hook(void) { hook_call_original(); void plat_hold_until_drawn(void); plat_hold_until_drawn(); }
static void draw_hook(void)         /* s014 14f7:0008 combat_draw(): a whole combat frame drawn */
{
    hook_call_original();
    void plat_drawn(void); plat_drawn();
}

static void input_hook(void)        /* s014 0e76:3166 combat_input(): one tick of player input */
{
    pan_apply();
    if (look_req >= 0) { int c = look_req; look_req = -1; if (game_mode() == GM_BATTLE) { ns_call(GF_floor_view_draw, 1, c); g_wb(GV_battle_follow, 0); } }
    if (center_req >= 0) { int c = center_req; center_req = -1; if (game_mode() == GM_BATTLE) { ns_call(GF_floor_view_draw, 1, c); g_wb(GV_battle_follow, 1); } }
    if (!battlescroll_on || config_get_int("original", 0)) { hook_call_original(); return; }
    static u32 last_step, edge_since, held_since, last_call;
    static int was_edge, stale;
    int mx, my; plat_mouse_game(&mx, &my);
    u32 now = plat_ticks_ms();
    int plat_pointer_outside(void), plat_dragging(void);
    int edge = on_edge(mx, my) && !plat_pointer_outside();     /* (beside the picture, over the margins: no) */
    /* back from another screen (a member's sheet, enemy info, the menus: no input ticks meanwhile)
       with the pointer on an edge, where that screen's button was (the sheet's Close is at the top):
       it rests there by chance, so no scrolling until it has left the edge once (Shift still does).
       The same when a drag of the field took it there: the drag moves the view, not the edge, and
       after it the pointer must leave the edge before resting there scrolls */
    if (now - last_call > 250) stale = edge;
    last_call = now;
    int dragging = plat_dragging();
    if (dragging && edge) stale = 1;
    if (!edge) stale = 0;
    if (dragging) edge = 0;
    if (edge && !was_edge) edge_since = now;
    was_edge = edge;
    if (!edge) { held_since = 0; hook_call_original(); return; }
    u8 f = rb(0x40, 0x17);
    int shift = (f & 3) != 0;
    int want = shift || (battlescroll_rest && !stale && now - edge_since >= 300);
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
    hook_install(GF_combat_draw, draw_hook);
    hook_install(16, 0x1ef8, 0x0b92, view_redraw_hook);
    hook_install(16, 0x1ef8, 0x0a2c, select_hook);
}

void battlescroll_init(void)
{
    battlescroll_set(config_get_int("battle.scroll_paced", 1), config_get_int("battle.scroll_rest", 1));
}
