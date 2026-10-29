/* orders.c - in a battle, a member who is not yet fighting can be sent at an enemy with a click
 * on it (F12 > Settings; the Full Original preset leaves the game's way).
 *
 * The game's way: click a member (order mode), choose an attack in the order menu (Std Attack,
 * Vulnerable, Berserk, Parry), then click the enemy. A member whose standing order is only to walk
 * (or nothing) takes a click on an enemy as a click on the ground under it, which is taken: "can't
 * go there" (s014 19cd:0dcc / 0ad4 skip the enemies' hot spots unless an attack is chosen). Here,
 * that click (and the pointer over the enemy) is taken as Std Attack (0x12, what the menu's first
 * attack sets) on that enemy, or as Use Missile (0x80) for a member whose way is to shoot (mods/ways.c);
 * the rest is the game's own order (walk there and fight, or shoot). What the members do between orders:
 * mods/ways.c. */
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
#include "ways.h"

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

/* the attack a click sends member i at foe m with: Use Missile (0x80) when the member opens by
   shooting (orders_member_mode), can loose the missile weapon (can_fire, s014 18bf:0c02) and the
   foe is not already within reach of a blow (far 20a5:7900, ai 1db7:09ae); else Std Attack */
static int best_attack(int i, int m)
{
    u16 c = (u16)(D_CBT + i * 0x80);
    int melee = ways_melee_kind(i, m);
    int mode = orders_member_mode(i);
    if (mode < 0) return melee;
    if (rb(G_FAR20A5, (u16)(0x7900 + i * 13 + m))) return melee;   /* beside it: hand to hand */
    if (mode == 0 || (!combat_improved() && ways_near_foe(i, m))) {   /* (improved combat: a shooter shoots until the foe is beside it) */
        /* hand to hand, unless there is no way to the foe at all (hemmed in by others): then a shot,
           if the member has a missile weapon and something to loose */
        u16 tx = g_rw((u16)(c + CB_TO_X)), ty = g_rw((u16)(c + CB_TO_Y));
        g_ww((u16)(c + CB_TO_X), g_rw(CBF(m, CB_X))); g_ww((u16)(c + CB_TO_Y), g_rw(CBF(m, CB_Y)));
        int way = ns_call(GF_path_find, 1, i) == 1;
        g_ww((u16)(c + CB_TO_X), tx); g_ww((u16)(c + CB_TO_Y), ty);
        if (way) return melee;
        if (!ways_line_clear(i, m)) return melee;                         /* (no way found, nor a line: the game's own attack, which walks round as it can) */
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
        else if (orders_click_attack && orders_member_mode(i) == 1 && !rb(G_FAR20A5, (u16)(0x7900 + i * 13 + m)) && !ways_near_foe(i, m)) mode = MISSILE;
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
int ui_pointer_on_cover(void);                  /* (ui/margins.cpp: the pointer on the order strip or another panel of ours) */
int orders_pointer_target(void)
{
    if (ui_pointer_on_cover()) return -1;
    if (!config_get_int("battle.click_hint", 1) || config_get_int("original", 0) || g_rb(D_ORDER_MODE) != 1 || (s8)g_rb(D_SELECTED) != hint_sel) return -1;
    return hint_ct;
}
const char *orders_hint(void)
{
    if (ui_pointer_on_cover()) return 0;
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
    ways_untold();
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
        ways_keep_kind(sel, m >= 5 && (act == 0x22 || act == 6 || act == 0x0a) ? act : -1);
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
        ways_told((s8)g_rb(D_SELECTED));               /* (an order of the player's: the member's way gives way to it, mods/ways.c) */
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
    ways_init();                                    /* (the members' ways between orders: mods/ways.c) */
}
