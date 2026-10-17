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
    f();
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
    native_go(seg, off);
    if (unw_active) { ns_unwinding = 1; return 0; }
    SP = sp0; DS = ds; ES = es;
    last_dx = DX;
    return (s16)AX;
}

/* ---------------------------------------------------------------- input */
#define A_MOUSE_BTN GV_mouse_buttons
static int btn_prev, rbtn_prev;
void ns_input_begin(void)
{
    int b = g_rb(A_MOUSE_BTN);
    btn_prev = b & 1; rbtn_prev = (b & 2) != 0;
    ns_in.click = ns_in.rclick = ns_in.pressed = 0; ns_in.down = b & 1;
}
void ns_input_poll(void)
{
    plat_mouse_game(&ns_in.x, &ns_in.y);
    int b = g_rb(A_MOUSE_BTN);
    if (btn_prev && !(b & 1)) { ns_in.click = 1; ns_in.cx = ns_in.x; ns_in.cy = ns_in.y; }
    if (!btn_prev && (b & 1)) { ns_in.pressed = 1; ns_in.px = ns_in.x; ns_in.py = ns_in.y; }
    ns_in.down = b & 1;
    if (rbtn_prev && !(b & 2)) ns_in.rclick = 1;
    btn_prev = b & 1; rbtn_prev = (b & 2) != 0;
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
        if (getenv("DL_KEYS_LOG")) fprintf(stderr, "keys: %04x to the game's hot keys (save %d, load %d)\n", k, g_rb(0x9920), g_rb(0x9934));
        bios_kbd_inject((u16)k);
        ns_call(GF_hot_keys, 0);
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------- actions */
#define NQ 32
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
static struct { NsDeferFn fn; int a, b, c, d; } dq[ND]; static int dn;
void ns_defer(NsDeferFn fn, int a, int b, int c, int d)
{
    if (dn >= ND) return;
    dq[dn].fn = fn; dq[dn].a = a; dq[dn].b = b; dq[dn].c = c; dq[dn].d = d; dn++;
}
static void run_deferred(void)
{
    int n = dn; dn = 0;
    for (int i = 0; i < n && !unw_active; i++) dq[i].fn(dq[i].a, dq[i].b, dq[i].c, dq[i].d);
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
void nscreen_init(void) { hook_install(GF_menu_bar_run, safe_point_hook); }

/* ---------------------------------------------------------------- the stack of screens */
static struct { int screen, nested; unsigned since; } stk[8];
static int depth;
void ns_enter(int screen)
{
    if (depth < 8) { stk[depth].screen = ns_screen; stk[depth].nested = ns_nested; stk[depth].since = ns_nested_since; }
    depth++;
    ns_screen = screen; ns_nested = 0;
}
void ns_leave(void)
{
    if (depth <= 0) return;
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
