/* nscreen.c - the frame for native screens (see nscreen.h). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "nscreen.h"

int plat_headless(void);
u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void plat_mouse_game(int *x, int *y);
void dl_poll(void);

int ns_unwinding, ns_nested, ns_screen;
unsigned ns_nested_since;
NsInput ns_in;

int ns_active(void) { return !plat_headless() && !config_get_int("original", 0); }

/* ---------------------------------------------------------------- calling the game */
static u16 last_dx;
int ns_call_dx(void) { return (s16)last_dx; }

/* a string the game reads (a file name for its routines): put on its own stack below what is in
 * use, until ns_stack_free(the SP before); its offset in SS (= DGROUP, so a near pointer too) */
u16 ns_stack_str(const char *s)
{
    int n = (int)strlen(s) + 1;
    SP = (u16)((SP - n - 2) & ~1);
    for (int i = 0; i < n; i++) wb(SS, (u16)(SP + i), (u8)s[i]);
    return SP;
}
void ns_stack_free(u16 sp) { SP = sp; }
static void input_resync(void);
u32 plat_ticks_ms(void);
int ns_call(int sect, u16 seg, u16 off, int n, ...)
{
    if (unw_active) { ns_unwinding = 1; return 0; }          /* (the game unwinds: nothing more) */
    ns_unwinding = 0;
    void (*f)(void) = game_func(sect, seg, off);
    if (!f) { fprintf(stderr, "nscreen: no game function s%d %04x:%04x\n", sect, seg, off); return 0; }
    int a[12]; va_list ap; va_start(ap, n);
    for (int i = 0; i < n && i < 12; i++) a[i] = va_arg(ap, int);
    va_end(ap);
    u16 sp0 = SP, ds = DS, es = ES;
    DS = ES = G_DGROUP;
    tail_pending = 0;
    for (int i = n - 1; i >= 0; i--) push16((u16)a[i]);
    push16(HLE_SEG); push16(0xfff0);
    OvlSet o; ovl_save(&o);
    if (sect >= 0) ovl_enter(sect);
    u32 t0 = plat_ticks_ms();
    f();
    if (plat_ticks_ms() - t0 > 150) input_resync();        /* (a game screen ran: its clicks were its own) */
    ovl_restore(&o);
    if (unw_active) { ns_unwinding = 1; return 0; }
    SP = sp0; DS = ds; ES = es;
    last_dx = DX;
    return (s16)AX;
}

/* a far pointer the game keeps (a handler: an overlay's vector or a root function) */
void native_go(u16 seg, u16 off);
int ns_call_ptr(u16 seg, u16 off, int n, ...)
{
    if (unw_active) { ns_unwinding = 1; return 0; }
    ns_unwinding = 0;
    if (!seg && !off) return 0;
    int a[12]; va_list ap; va_start(ap, n);
    for (int i = 0; i < n && i < 12; i++) a[i] = va_arg(ap, int);
    va_end(ap);
    u16 sp0 = SP, ds = DS, es = ES;
    DS = ES = G_DGROUP;
    tail_pending = 0;
    for (int i = n - 1; i >= 0; i--) push16((u16)a[i]);
    push16(HLE_SEG); push16(0xfff0);
    u32 t0 = plat_ticks_ms();
    native_go(seg, off);
    if (plat_ticks_ms() - t0 > 150) input_resync();
    if (unw_active) { ns_unwinding = 1; return 0; }
    SP = sp0; DS = ds; ES = es;
    last_dx = DX;
    return (s16)AX;
}

/* ---------------------------------------------------------------- input */
#define A_MOUSE_BTN GV_mouse_buttons
static int btn_prev, rbtn_prev;
static int inherited_left;
static unsigned press_seen, rpress_seen;
unsigned mouse_press_count(int b);
void ns_input_begin(void)
{
    int b = g_rb(A_MOUSE_BTN);
    btn_prev = b & 1; rbtn_prev = (b & 2) != 0;
    inherited_left = btn_prev;
    press_seen = mouse_press_count(0); rpress_seen = mouse_press_count(1);
    ns_in.click = ns_in.rclick = ns_in.pressed = 0; ns_in.down = b & 1;
}
void ns_input_poll(void)
{
    plat_mouse_game(&ns_in.x, &ns_in.y);
    int b = g_rb(A_MOUSE_BTN);
    /* a press and its release both between two rounds (a quick click, a slow round) still count */
    unsigned pc = mouse_press_count(0), rpc = mouse_press_count(1);
    int fresh = pc != press_seen, rfresh = rpc != rpress_seen;
    press_seen = pc; rpress_seen = rpc;
    /* A town portrait opens its sheet on mouse-down. Its release belongs to
       the town, even if the pointer has moved. A new press is the sheet's. */
    if (inherited_left) {
        if (fresh || !(b & 1)) inherited_left = 0;
        btn_prev = fresh ? 0 : (b & 1);
    }
    if ((btn_prev || fresh) && !(b & 1)) { ns_in.click = 1; ns_in.cx = ns_in.x; ns_in.cy = ns_in.y; }
    if ((!btn_prev || fresh) && (b & 1)) { ns_in.pressed = 1; ns_in.px = ns_in.x; ns_in.py = ns_in.y; }
    if (fresh && !btn_prev && !(b & 1)) { ns_in.pressed = 1; ns_in.px = ns_in.x; ns_in.py = ns_in.y; }
    ns_in.down = b & 1;
    if ((rbtn_prev || rfresh) && !(b & 2)) ns_in.rclick = 1;
    btn_prev = b & 1; rbtn_prev = (b & 2) != 0;
}
/* after a game screen ran inside: the presses it took are not this screen's clicks */
static void input_resync(void)
{
    int b = g_rb(A_MOUSE_BTN);
    btn_prev = b & 1; rbtn_prev = (b & 2) != 0;
    press_seen = mouse_press_count(0); rpress_seen = mouse_press_count(1);
}
int ns_take_click(int *x, int *y)
{
    if (!ns_in.click) return 0;
    ns_in.click = 0;
    if (x) *x = ns_in.cx;
    if (y) *y = ns_in.cy;
    return 1;
}

int ns_hot_key(int k);
/* a key for a native screen's loop; F10 opens the menus (as hot_keys does: the loop took the key
   before menu_bar_run saw it) and is not passed on (0) */
int ns_get_key(void)
{
    int k = (u16)ns_call(GF_get_key, 0);
    if (k == 0x4400) { g_wb(0x9c26, 1); g_wb(0x9c2f, 0); g_wb(0x9c3c, 0xff); g_wb(0x9c41, 0xff); return 0; }
    if (ns_hot_key(k)) return 0;
    return k;
}

/* the game's hot keys (hot_keys, root 0000:0caa: Alt-S save, Alt-L load, Alt-D difficulty, Alt-M
   music, Alt-F sound, Alt-C saintly changes, Alt-P pause, Alt-O marching order, Alt-Q quit) when a
   native loop took the key before menu_bar_run looked: put back and handed to hot_keys */
void bios_kbd_inject(u16 w);
int ns_hot_key(int k)
{
    switch (k) {
    case 0x1000: case 0x1800: case 0x1900: case 0x1f00: case 0x2000: case 0x2100: case 0x2600: case 0x2e00: case 0x3200:
        if (getenv("DL_KEYS_LOG")) fprintf(stderr, "keys: %04x to the game's hot keys (save %d, load %d)\n", k, g_rb(GV_font_flag_a), g_rb(GV_font_flag_b));
        bios_kbd_inject((u16)k);
        ns_call(GF_hot_keys, 0);
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------- actions */
#define NQ 160          /* (the shop's "Sell spare gear" pushes a sale for each of up to 64 kinds of things) */
static NsAction q[NQ]; static int qh, qt;
void ns_push(int type, int a, int b, int c) { int n = (qt + 1) % NQ; if (n == qh) return; q[qt].type = type; q[qt].a = a; q[qt].b = b; q[qt].c = c; qt = n; }
int ns_pop(NsAction *out) { if (qh == qt) return 0; *out = q[qh]; qh = (qh + 1) % NQ; return 1; }
void ns_clear(void) { qh = qt = 0; }

/* ---------------------------------------------------------------- waiting */
int ns_hidden(void)
{
    return ns_nested == 1 || (ns_nested == 2 && (g_rw(GV_menu_bar_active) || plat_ticks_ms() - ns_nested_since > 60));
}

/* ---------------------------------------------------------------- deferred requests */
#define ND 16
/* (ns_defer_here: a request that is run in the state it was asked in, or not at all: which battle, or none. One
   asked from the party panel in a battle's last tick ran at the first menu bar after it, a card's: the battle's
   sheet opened over a card, with the battle's memory gone. The audit, 2026-11-22. ns_defer: wherever the game
   is by then, the sound's and the music's) */
int fixes_battle_now(void);
static struct { NsDeferFn fn; int a, b, c, d, battle; } dq[ND]; static int dn;
void ns_defer(NsDeferFn fn, int a, int b, int c, int d)
{
    if (dn >= ND) return;
    dq[dn].fn = fn; dq[dn].a = a; dq[dn].b = b; dq[dn].c = c; dq[dn].d = d; dq[dn].battle = -1; dn++;
}
void ns_defer_here(NsDeferFn fn, int a, int b, int c, int d) { int n = dn; ns_defer(fn, a, b, c, d); if (dn > n) dq[n].battle = fixes_battle_now(); }
static void run_deferred(void)
{
    int n = dn; dn = 0;
    for (int i = 0; i < n && !unw_active; i++) {
        if (dq[i].battle != -1 && dq[i].battle != fixes_battle_now()) { fprintf(stderr, "nscreen: a request asked in another state dropped\n"); continue; }
        dq[i].fn(dq[i].a, dq[i].b, dq[i].c, dq[i].d);
    }
}
static void safe_point_hook(void)           /* menu_bar_run(menus, once): a game screen's round begins */
{
    if (dn && (s16)rw(SS, (u16)(SP + 6)) == 0) {
        run_deferred();
        if (unw_active) return;
    }
    hook_call_original();
}
/* the end of an interface frame while the game is held (F12's window open: no rounds run) */
void ns_defer_frame_end(int game_held) { if (dn && game_held) run_deferred(); }
/* ---------------------------------------------------------------- the world's tables, kept
 * world_load (s043 1462:0100) reads three files into blocks of DOS memory and keeps far pointers into them:
 * DARKLAND.LST (each thing's 46 bytes, 2e38:0890; the saints' and the formulae's names, 2e38:183e, 3bc4, 1736,
 * 36ec), the places (LOCS.TMP: 0x19e records, 2e38:10be) and the city's record (DARKLAND.CTY: city_rec, DS:9785).
 * cards_shutdown (1462:054e) frees all three for a battle; the battle's sheet (s031 1462:0000) reads them and
 * frees them again, the loot (s040 1462:0000) the list; each time the pointers stay, into memory the battle then
 * takes for itself. The game's own battle code reads none of them (s014-s028 call no function that does but
 * the sheet and the loot, which read the files again). Darklands+ does: the game's item functions it calls in a
 * battle (the potion tray's drink, the pack's hand-over) reach equip_weight, which summed whatever lay there (a
 * load of 198, 1.5.1-RC18), and its own code that reads the places every frame (the fix offmap wrote through
 * them, in every battle). So when the game frees one of the three, its bytes are kept in memory the game never
 * uses (KEPT_*_SEG, runtime/hle.h) and the pointers into it moved there, until the game reads the file again and
 * sets them anew. What is written into a kept block is lost then, as it was in the freed one. Watched: CK_ITEMLIST
 * and CK_PLACES (mods/checks.c). */
static const struct { u16 seg; unsigned room; } KEPT[3] = { { KEPT_LST_SEG, 0x5800 }, { KEPT_LOC_SEG, 0x5e00 }, { KEPT_CITY_SEG, 0x400 } };
static void moved(u16 table, int n, u16 from, u16 to)       /* the far pointers of a table that led into from */
{
    for (int i = 0; i < n; i++) if (rw(G_FAR2E38, (u16)(table + 4 * i + 2)) == from) ww(G_FAR2E38, (u16)(table + 4 * i + 2), to);
}
static void mem_free_hook(void)            /* mem_free (seg, name), root 05ba:00c2 */
{
    u16 seg = rw(SS, (u16)(SP + 4));
    if (seg == KEPT_LST_SEG || seg == KEPT_LOC_SEG || seg == KEPT_CITY_SEG || (seg >= EV_MIRROR_SEG && seg < EV_OURS_END)) {
        hook_return_far(0); return;                    /* (Darklands+'s own, not DOS's to free: a kept block; an event of the battle's copy, mods/fixes.c) */
    }
    int which = !seg ? -1 : rw(G_FAR2E38, (u16)(G_ITEMDEFS + 2)) == seg ? 0 : rw(G_FAR2E38, 0x10be + 2) == seg ? 1 : g_rw(GV_city_rec + 2) == seg ? 2 : -1;
    unsigned bytes = which >= 0 ? (unsigned)rw((u16)(seg - 1), 3) * 16 : 0;     /* (the block's size, from DOS's record before it) */
    if (which >= 0 && bytes > KEPT[which].room) {
        fprintf(stderr, "kept: no room for a block of %u bytes (%d), left as the game leaves it\n", bytes, which);
        which = -1;
    }
    int n = which == 0 ? rb(seg, 0) : 0, na = which == 0 ? rb(seg, 1) : 0, nb = which == 0 ? rb(seg, 2) : 0;
    if (which >= 0) memcpy(M + (u32)KEPT[which].seg * 16, M + (u32)seg * 16, bytes);
    hook_call_original();
    if (unw_active || which < 0) return;
    u16 to = KEPT[which].seg;
    if (which == 0) { moved(G_ITEMDEFS, n, seg, to); moved(0x183e, na, seg, to); moved(0x3bc4, na, seg, to); moved(0x1736, nb, seg, to); moved(0x36ec, nb, seg, to); }
    else if (which == 1) moved(0x10be, G_NLOC, seg, to);
    else g_ww(GV_city_rec + 2, to);
}

void nscreen_init(void) { hook_install(GF_menu_bar_run, safe_point_hook); hook_install(GF_mem_free, mem_free_hook); }

/* ---------------------------------------------------------------- the stack of screens */
static struct { int screen, nested; unsigned since; } stk[8];
static int depth;
void ns_enter(int screen)
{
    if (depth < 8) { stk[depth].screen = ns_screen; stk[depth].nested = ns_nested; stk[depth].since = ns_nested_since; }
    depth++;
    ns_screen = screen; ns_nested = 0;
    { void plat_hold(int ms); plat_hold(0); }                 /* (its own view draws from now: shown again) */
}
void ns_leave(void)
{
    if (depth <= 0) return;
    /* a screen of Darklands+'s closed (a shop, the sheet ...): the game redraws its own screen behind
       before the next card or screen of ours sets up; held, up to half a second, so the old
       interface never flashes (the next ns_enter shows again at once) */
    if (ns_screen != NS_CARD && ns_screen != NS_NONE && !config_get_int("original", 0)) { void plat_hold(int ms); plat_hold(500); }
    depth--;
    if (depth < 8) { ns_screen = stk[depth].screen; ns_nested = stk[depth].nested; ns_nested_since = stk[depth].since; }
}

int ns_showing(void) { return ns_screen != NS_NONE && ns_screen != NS_CARD && !ns_hidden(); }

int ns_round(void)
{
    ns_nested = 2; ns_nested_since = plat_ticks_ms();
    ns_call(GF_menu_bar_run, 2, 0x9419, 0);                 /* menu_bar_run */
    ns_nested = 0;
    if (ns_unwinding) return 0;
    ns_call(GF_exit_if_requested, 0);                            /* exit_if_requested */
    if (ns_unwinding) return 0;
    dl_poll();
    plat_sleep_ms(3);
    ns_input_poll();
    return 1;
}
