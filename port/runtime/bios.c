/* bios.c - interrupts, BIOS services and the PC hardware the game talks to (VGA DAC, PIT,
 * keyboard controller, PIC), plus the safe-point dl_poll() that delivers timer and keyboard
 * interrupts to the game's own handlers and refreshes the screen. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "cpu.h"
#include "config.h"
#include "hle.h"

void dos_int21(void);
int plat_menu_open(void);
void opl_out(u16 port, u8 v); u8 opl_in(u16 port); void sound_init(void);
void mpu_out(u16 port, u8 v); u8 mpu_in(u16 port);
void ems_int67(void); void ems_init(void);
int bios_trace;

volatile int poll_flag = 1;
extern int in_game_call;  /* (below) an evaluation is running (gamecall.c) */
int idle_hits;          /* busy-wait polls (keyboard status, retrace) since the last dl_poll() */
int spin_polls;         /* the game asked for a click (int 33h, 5 and 6), a key (int 16h, 1) or the time (int 21h, 2Ah / 2Ch) */
static u32 screen_changed;  /* when the picture or the palette last changed */
static int spin_limit(void) { static int v = -1; if (v < 0) { const char *e = getenv("DL_SPIN_LIMIT"); v = e ? atoi(e) : 150; } return v; }
int idle_rest = 1;      /* rest while the game only waits (always; DL_NO_SPIN_REST for tests) */
static int getenv_spin_off(void) { static int v = -1; if (v < 0) v = getenv("DL_NO_SPIN_REST") != 0; return v || !idle_rest; }
u8 vga_dac[768];
int video_mode = 3;

/* ---------------------------------------------------------------- helpers */
#ifdef FTRACE
u32 ftrace_stack[4096]; int ftrace_depth;
u16 ftrace_bp[4096], ftrace_sp[4096], ftrace_ss[4096], ftrace_ret[4096];
void ftrace_check_retsite(u16 seg, u16 off)
{
    static int on = -1; if (on < 0) on = getenv("DL_CHECK_RETSITE") != 0;
    if (!on || ftrace_depth <= 0 || ftrace_depth > 4096) return;
    int d = ftrace_depth - 1;
    if (ftrace_ret[d] == off) return;
    static u32 seen[256]; static int n;
    u32 id = ftrace_stack[d];
    for (int i = 0; i < n; i++) if (seen[i] == id) return;
    if (n < 256) seen[n++] = id;
    fprintf(stderr, "retsite jump from %x to %04x:%04x, but its return address was %04x (SP %04x, entry SP %04x)\n",
            id, seg, off, ftrace_ret[d], SP, ftrace_sp[d]);
    ftrace_dump();
}
void ftrace_sp_odd(int d)
{
    static u32 seen[256]; static int n;
    static int on = -1; if (on < 0) on = getenv("DL_CHECK_SP") != 0;
    if (!on) return;
    u32 id = ftrace_stack[d];
    for (int i = 0; i < n; i++) if (seen[i] == id) return;
    if (n < 256) seen[n++] = id;
    fprintf(stderr, "SP odd after function %x: entry %04x, now %04x\n", id, ftrace_sp[d], R.sp);
    ftrace_dump();
}
u16 sp_low;
u32 ret_ring[64]; u16 ret_ring_sp[64]; int ret_ring_i;
void sp_low_hit(void)
{
    static int n; if (n++ > 2) return;
    fprintf(stderr, "SP low: %04x:%04x\n", SS, SP);
    ftrace_dump();
    fprintf(stderr, "  last returns (function: SP after):");
    for (int k = 0; k < 64; k++) { int j = (ret_ring_i + k) & 63; if (ret_ring[j]) fprintf(stderr, "%s%x:%04x", k % 6 ? "  " : "\n    ", ret_ring[j], ret_ring_sp[j]); }
    fprintf(stderr, "\n");
}
void ftrace_bp_changed(int d)
{
    static u32 seen[256]; static int n;
    u32 id = ftrace_stack[d];
    for (int i = 0; i < n; i++) if (seen[i] == id) return;
    if (n < 256) seen[n++] = id;
    static int on = -1; if (on < 0) on = getenv("DL_CHECK_BP") != 0;
    if (on) fprintf(stderr, "BP changed by function %x: %04x -> %04x\n", id, ftrace_bp[d], R.bp);
}
void ftrace_dump(void)
{
    fprintf(stderr, "  call stack (innermost last):\n");
    int d = ftrace_depth < 4096 ? ftrace_depth : 4096;
    for (int i = d > 40 ? d - 40 : 0; i < d; i++) {
        u32 id = ftrace_stack[i]; int s = (int)(id >> 20) - 2;
        if (s == -1) fprintf(stderr, "    root_%05x\n", id & 0xfffff);
        else if (s >= 0) fprintf(stderr, "    s%03d_%05x\n", s, id & 0xfffff);
        else fprintf(stderr, "    drv_%05x\n", id & 0xfffff);
    }
}
u32 watch_lo = 0xffffffff, watch_len;
int watch_on;
static u8 watch_prev[64]; static int watch_init;
void watch_check(const char *where)
{
    if (watch_lo == 0xffffffff) return;
    u32 n = watch_len > 64 ? 64 : watch_len;
    if (!watch_init) { memcpy(watch_prev, M + watch_lo, n); watch_init = 1; watch_on = 1; return; }
    if (memcmp(watch_prev, M + watch_lo, n)) {
        fprintf(stderr, "watch: %05x changed by %s (now %02x)\n", watch_lo, where, M[watch_lo]);
        ftrace_dump();
        memcpy(watch_prev, M + watch_lo, n);
    }
}
void watch_hit(u32 l, u16 v)
{
    fprintf(stderr, "watch: write %05x <- %04x  CS? DS=%04x ES=%04x SI=%04x DI=%04x\n", l, v, DS, ES, SI, DI);
    ftrace_dump();
}
#else
void ftrace_check_retsite(u16 seg, u16 off) { (void)seg; (void)off; }
void ftrace_dump(void) {}
void watch_check(const char *where) { (void)where; }
#endif
void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "FATAL: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n  CS=%04x DS=%04x ES=%04x SS=%04x SP=%04x BP=%04x AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x LOADSEG=%04x\n",
            CS, DS, ES, SS, SP, BP, AX, BX, CX, DX, SI, DI, LOADSEG);
    va_end(ap);
    ftrace_dump();                                   /* (into the log before the box, which waits for a click) */
    { void interp_ring_dump(void); interp_ring_dump(); void ovl_dump(void); ovl_dump(); }
    fflush(stderr);
    {   char msg[512]; va_start(ap, fmt); vsnprintf(msg, sizeof msg, fmt, ap); va_end(ap);
        void plat_error_box(const char *); plat_error_box(msg); }
    plat_quit(3);
}

#define SETCF(c) do { u16 f_ = get_flags(); set_flags((u16)((c) ? (f_ | 1) : (f_ & ~1))); } while (0)
#define SETZF(c) do { u16 f_ = get_flags(); set_flags((u16)((c) ? (f_ | 0x40) : (f_ & ~0x40))); } while (0)

static u16 def_seg(int n) { return n == 0x67 ? EMS_SEG : HLE_SEG; }
static u16 def_off(int n) { return n == 0x67 ? 0x0100 : (u16)(n * 4); }
static int ivt_hooked(int n)
{
    return rw(0, (u16)(n * 4 + 2)) != def_seg(n) || rw(0, (u16)(n * 4)) != def_off(n);
}

/* run the game's handler for interrupt n as the CPU would */
/* an interrupt arrives between two instructions of whatever runs: the dispatcher's pending
 * tail jump and unwinding state belong to that code, not to the handler */
typedef struct { int tail; u16 tsp; int unw; u16 usp, uip, ucs; int ufar; } IrqSave;
void ovl_irq_begin(void); void ovl_irq_end(void);
static void irq_enter(IrqSave *s)
{
    { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_IRQSTACK") != 0;
      if (tr && SS == (u16)(LOADSEG + 0x321a) && SP > 0x8d00 && SP <= 0x9051) fprintf(stderr, "irq at SP %04x (LZW stack)\n", SP); }
    ovl_irq_begin();
    s->tail = tail_pending; s->tsp = tail_sp; s->unw = unw_active; s->usp = unw_sp; s->uip = unw_ip; s->ucs = unw_cs; s->ufar = unw_far;
    tail_pending = 0; unw_active = 0;
}
static void irq_leave(IrqSave *s, int n)
{
    if (unw_active || tail_pending) { static int w; if (w++ < 10) fprintf(stderr, "warning: int %02x handler left %s%s pending\n", n, unw_active ? "an unwinding return" : "", tail_pending ? " a tail jump" : ""); }
    tail_pending = s->tail; tail_sp = s->tsp; unw_active = s->unw; unw_sp = s->usp; unw_ip = s->uip; unw_cs = s->ucs; unw_far = s->ufar;
    ovl_irq_end();
}

/* debug (DL_IRQ_DIFF): which DGROUP bytes an interrupt handler changes, while the PIC
 * decoder's LZW stack is active (SS:SP inside DGROUP 8e00..9051) */
static u8 irqd_before[65536], irqd_e000[65536]; static int irqd_on = -1, irqd_armed;
static void irqd_begin(void)
{
    if (irqd_on < 0) irqd_on = getenv("DL_IRQ_DIFF") != 0;
    irqd_armed = irqd_on && SS == (u16)(LOADSEG + 0x321a) && SP > 0x8e00 && SP <= 0x9051;
    if (irqd_armed) { memcpy(irqd_before, M + ((u32)(LOADSEG + 0x321a) << 4), 65536); memcpy(irqd_e000, M + 0xe0000, 65536); }
}
static void irqd_end(int n)
{
    if (!irqd_armed) return;
    { const u8 *e = M + 0xe0000; int f = -1, cnt = 0;
      for (int i = 0; i < 65536; i++) if (e[i] != irqd_e000[i]) { if (f < 0) f = i; cnt++; }
      if (cnt) fprintf(stderr, "irqdiff int %02x SP %04x: page 1 (E000) %d bytes changed from %04x\n", n, SP, cnt, f); }
    const u8 *now = M + ((u32)(LOADSEG + 0x321a) << 4);
    int first = -1;
    for (int i = 0; i <= 65536; i++) {
        int d = i < 65536 && now[i] != irqd_before[i];
        if (d && first < 0) first = i;
        if (!d && first >= 0) { if (!(first >= 0x83b8 && first < 0x83c0) && !(first >= 0x9000 && first < 0x9052 && i - 1 < SP)) fprintf(stderr, "irqdiff int %02x SP %04x: %04x..%04x changed\n", n, SP, first, i - 1); first = -1; }
    }
}

static void call_handler(int n)
{
    irqd_begin();
    IrqSave sv; irq_enter(&sv);
    u16 off = rw(0, (u16)(n * 4)), seg = rw(0, (u16)(n * 4 + 2));
    push16(get_flags());
    R.iflag = 0; R.tf = 0;
    push16(HLE_SEG);
    push16((u16)(0xff00 | n));
    u16 ss0 = SS, sp0 = (u16)(SP + 6), bp0 = BP, si0 = SI, di0 = DI, ds0 = DS, es0 = ES;
    u16 ax0 = AX, bx0 = BX, cx0 = CX, dx0 = DX;
    u16 fl0 = rw(SS, (u16)(SP + 4));          /* the flags pushed above */
    dispatch_far(seg, off);
    { u16 fl1 = get_flags(); if ((fl1 ^ fl0) & 0x0cd5) { static int wf; if (wf++ < 10) fprintf(stderr, "warning: int %02x handler changed flags %04x -> %04x\n", n, fl0, fl1); } }
    if (AX != ax0 || BX != bx0 || CX != cx0 || DX != dx0) {
        static int w3;
        if (w3++ < 10) fprintf(stderr, "warning: int %02x handler %04x:%04x changed registers: AX %04x->%04x BX %04x->%04x CX %04x->%04x DX %04x->%04x\n",
                               n, seg, off, ax0, AX, bx0, BX, cx0, CX, dx0, DX);
    }
    if (BP != bp0 || SI != si0 || DI != di0 || DS != ds0 || ES != es0) {
        static int w2;
        if (w2++ < 5) fprintf(stderr, "warning: int %02x handler changed registers: BP %04x->%04x SI %04x->%04x DI %04x->%04x DS %04x->%04x ES %04x->%04x\n",
                              n, bp0, BP, si0, SI, di0, DI, ds0, DS, es0, ES);
    }
    if (SS != ss0 || SP != sp0) {
        static int warned;
        if (!warned++) fprintf(stderr, "warning: int %02x handler %04x:%04x returned with SS:SP %04x:%04x, expected %04x:%04x\n",
                               n, seg, off, SS, SP, ss0, sp0);
    }
    irq_leave(&sv, n);
    irqd_end(n);
}

/* ---------------------------------------------------------------- keyboard */
static u8 kq[64]; static int kq_head, kq_tail;
static u8 port60;
static int kbd_e0;
static u8 shift_state;          /* bit0 rshift bit1 lshift bit2 ctrl bit3 alt */

void kbd_push_scancode(u8 sc, int release)
{
    int n = (kq_tail + 1) % 64;
    if (n == kq_head) return;
    kq[kq_tail] = (u8)(sc | (release ? 0x80 : 0));
    kq_tail = n;
}

static const char sc_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0 };
static const char sc_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8, 9,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 13, 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0 };

static void bios_kbd_put(u16 w)
{
    u16 head = rw(0x40, 0x1a), tail = rw(0x40, 0x1c);
    u16 n = (u16)(tail + 2); if (n >= 0x3e) n = 0x1e;
    if (n == head) return;
    ww(0x40, tail, w);
    ww(0x40, 0x1c, n);
}
void bios_kbd_inject(u16 w) { bios_kbd_put(w); }
/* the keys pressed but not yet handed to the game (they go one every 10 ms): dropped, their releases
   kept (a held Alt or Shift must still be let go). For a page of Darklands+ that a key closed: the
   same key could reach the game after it (an Enter that saved then chose a card's first option) */
void bios_kbd_drop_pending(void)
{
    u8 keep[64]; int n = 0;
    for (int i = kq_head; i != kq_tail; i = (i + 1) % 64) {
        u8 c = kq[i];
        if (c == 0xe0) { int j = (i + 1) % 64; if (j != kq_tail && (kq[j] & 0x80)) { keep[n++] = c; keep[n++] = kq[j]; } if (j != kq_tail) i = j; continue; }
        if (c & 0x80) keep[n++] = c;
    }
    kq_head = kq_tail = 0;
    for (int i = 0; i < n; i++) { kq[kq_tail] = keep[i]; kq_tail = (kq_tail + 1) % 64; }
}
int bios_kbd_avail(void) { return rw(0x40, 0x1a) != rw(0x40, 0x1c); }
u16 bios_kbd_get(int remove)
{
    u16 head = rw(0x40, 0x1a);
    if (head == rw(0x40, 0x1c)) return 0;
    u16 w = rw(0x40, head);
    if (remove) { head = (u16)(head + 2); if (head >= 0x3e) head = 0x1e; ww(0x40, 0x1a, head); }
    return w;
}

/* BIOS int 9: port 60h -> BIOS keyboard buffer */
static void bios_int9(void)
{
    u8 sc = port60;
    if (sc == 0xe0) { kbd_e0 = 1; return; }
    int rel = sc & 0x80; u8 k = sc & 0x7f;
    int ext = kbd_e0; kbd_e0 = 0;
    u8 bit = k == 0x36 ? 1 : k == 0x2a ? 2 : k == 0x1d ? 4 : k == 0x38 ? 8 : 0;
    if (bit) { if (rel) shift_state &= (u8)~bit; else shift_state |= bit; wb(0x40, 0x17, shift_state); return; }
    if (rel) return;
    u8 a = 0;
    if (!ext || k == 0x1c || k == 0x35) a = (u8)((shift_state & 3) ? sc_shift[k] : sc_ascii[k]);
    if (shift_state & 4 && a >= 'a' && a <= 'z') a = (u8)(a - 'a' + 1);
    if (shift_state & 8) a = 0;
    bios_kbd_put((u16)((k << 8) | a));
}

/* ---------------------------------------------------------------- mouse (int 33h) */
static int ms_x = 320, ms_y = 100, ms_b, ms_shown;
static int ms_minx = 0, ms_maxx = 639, ms_miny = 0, ms_maxy = 199;
static int ms_mickx, ms_micky;
static u16 ms_hmask, ms_hseg, ms_hoff;
static int ms_pending;          /* event mask for the user handler */
/* The game learns its coordinate scale from where the pointer is right after a reset: the
 * high byte of x (DARKLAND.EXE 0598:0046, the x a driver reports, 320 in mode 13h, gives a
 * shift of 1). A real driver centres the pointer on reset; without that, a pointer in the left
 * part of the window at that moment (after switching windows during the opening) gave a
 * shift of 0: the game's pointer moved twice as fast and left the picture halfway across.
 * So a reset centres it, and the real position follows once the game has read the centre. */
static int ms_hostx = 320, ms_hosty = 100, ms_resync;
static int ms_press[3], ms_release[3], ms_lpx[3], ms_lpy[3], ms_lrx[3], ms_lry[3];
static unsigned ms_press_total[3];      /* presses ever (for Darklands+: a click the game's loop polled too late to see) */
unsigned mouse_press_count(int b) { return b >= 0 && b < 3 ? ms_press_total[b] : 0; }

void mouse_update(int x, int y, int buttons)
{
    ms_hostx = x; ms_hosty = y;
    if (x < ms_minx) x = ms_minx; if (x > ms_maxx) x = ms_maxx;
    if (y < ms_miny) y = ms_miny; if (y > ms_maxy) y = ms_maxy;
    if (x != ms_x || y != ms_y) {
        ms_mickx += (x - ms_x); ms_micky += (y - ms_y) * 2;
        ms_x = x; ms_y = y; ms_pending |= 1;
    }
    for (int i = 0; i < 3; i++) {
        int was = (ms_b >> i) & 1, now = (buttons >> i) & 1;
        if (now && !was) { ms_press[i]++; ms_press_total[i]++; ms_lpx[i] = ms_x; ms_lpy[i] = ms_y; ms_pending |= 2 << (2 * i); }
        if (!now && was) { ms_release[i]++; ms_lrx[i] = ms_x; ms_lry[i] = ms_y; ms_pending |= 4 << (2 * i); }
    }
    ms_b = buttons;
}

static void int33(void)
{
    { static int tr = -1; static u32 lt, n[16]; if (tr < 0) tr = getenv("DL_TRACE_MPOLL") != 0;
      if (tr) { if (AX < 16) n[AX]++; u32 t = plat_ticks_ms(); if (t - lt >= 1000) { fprintf(stderr, "int33/s:"); for (int i = 0; i < 16; i++) if (n[i]) fprintf(stderr, " fn%d %u", i, n[i]); fprintf(stderr, "\n"); memset(n, 0, sizeof n); lt = t; } } }
    switch (AX) {
    case 0x00: AX = 0xffff; BX = 2; ms_shown = 0; ms_hmask = 0;
        ms_minx = 0; ms_maxx = 639; ms_miny = 0; ms_maxy = 199; ms_x = 320; ms_y = 100; ms_mickx = ms_micky = 0; ms_resync = 1; break;
    case 0x01: ms_shown++; break;
    case 0x02: ms_shown--; break;
    case 0x03: BX = (u16)ms_b; CX = (u16)ms_x; DX = (u16)ms_y; if (ms_resync == 1) ms_resync = 2; break;
    case 0x04: { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_MOUSE") != 0; if (tr) fprintf(stderr, "mouse: set %d,%d\n", (s16)CX, (s16)DX); }
        ms_x = CX; ms_y = DX;
        if (!in_game_call) { void plat_mouse_warp(int, int); plat_mouse_warp((s16)CX / 2, (s16)DX); }   /* the system pointer follows (not for an evaluation) */
        break;
    case 0x05: spin_polls++; { int b = BX & 3; AX = (u16)ms_b; BX = (u16)ms_press[b]; CX = (u16)ms_lpx[b]; DX = (u16)ms_lpy[b]; ms_press[b] = 0; break; }
    case 0x06: spin_polls++; { int b = BX & 3; AX = (u16)ms_b; BX = (u16)ms_release[b]; CX = (u16)ms_lrx[b]; DX = (u16)ms_lry[b]; ms_release[b] = 0; break; }
    case 0x07: { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_MOUSE") != 0; if (tr) fprintf(stderr, "mouse: x range %d..%d\n", (s16)CX, (s16)DX); }
        ms_minx = (s16)CX < (s16)DX ? (s16)CX : (s16)DX; ms_maxx = (s16)CX < (s16)DX ? (s16)DX : (s16)CX; break;
    case 0x08: { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_MOUSE") != 0; if (tr) fprintf(stderr, "mouse: y range %d..%d\n", (s16)CX, (s16)DX); }
        ms_miny = (s16)CX < (s16)DX ? (s16)CX : (s16)DX; ms_maxy = (s16)CX < (s16)DX ? (s16)DX : (s16)CX; break;
    case 0x0b: CX = (u16)ms_mickx; DX = (u16)ms_micky; ms_mickx = ms_micky = 0; break;
    case 0x0c: ms_hmask = CX; ms_hseg = ES; ms_hoff = DX; break;
    case 0x14: { u16 m = ms_hmask, s = ms_hseg, o = ms_hoff; ms_hmask = CX; ms_hseg = ES; ms_hoff = DX; CX = m; ES = s; DX = o; break; }
    case 0x15: BX = 64; break;
    case 0x21: AX = 0xffff; BX = 2; break;
    case 0x24: BX = 0x0626; CH = 2; CL = 0; break;
    default: break;   /* 0f, 10, 1a, 1d ...: accept silently */
    }
}

static void mouse_deliver(void)
{
    int ev = ms_pending & ms_hmask;
    ms_pending = 0;
    if (!ev || !ms_hseg) return;
    CPU save = R;
    irqd_begin();
    IrqSave sv; irq_enter(&sv);
    AX = (u16)ev; BX = (u16)ms_b; CX = (u16)ms_x; DX = (u16)ms_y; SI = (u16)ms_mickx; DI = (u16)ms_micky;
    push16(HLE_SEG); push16(0xfe00);
    dispatch_far(ms_hseg, ms_hoff);
    u16 ds = R.ds; (void)ds;
    irq_leave(&sv, 0x33);
    R = save;
    irqd_end(0x33);
}

/* ---------------------------------------------------------------- PIT / timer */
static u16 pit_div = 0;         /* channel 0 divisor (0 = 65536) */
static int pit_write_hi, pit_latch_hi; static u16 pit_latch, pit_new;
static double tick_us_acc;
static u32 last_ms;
static double pit_period_us(void) { return (pit_div ? pit_div : 65536) * 1e6 / 1193182.0; }
double bios_tick_us(void) { return pit_period_us(); }

/* channel 0 counter, from real time; every latch also advances a virtual clock a little so
 * that code timing a short loop between two latches (sound driver CPU-speed calibration)
 * sees time pass, as it would on a real machine */
static uint64_t pit_virtual_us;
static u16 pit_count(void)
{
    u32 period = pit_div ? pit_div : 65536;
    pit_virtual_us += 40;
    uint64_t ticks = (plat_ticks_us() + pit_virtual_us) * 1193182ull / 1000000ull;
    return (u16)(period - (ticks % period));
}

/* BIOS int 8 */
static void bios_int8(void)
{
    u32 t = (u32)rw(0x40, 0x6c) | ((u32)rw(0x40, 0x6e) << 16);
    t++;
    if (t >= 0x1800b0) { t = 0; wb(0x40, 0x70, 1); }
    ww(0x40, 0x6c, (u16)t); ww(0x40, 0x6e, (u16)(t >> 16));
    if (ivt_hooked(0x1c)) call_handler(0x1c);
}

/* ---------------------------------------------------------------- VGA */
static int dac_w, dac_r, dac_wc, dac_rc;
static u8 vga_misc_idx[4];

static void set_mode(int m)
{
    video_mode = m & 0x7f;
    wb(0x40, 0x49, (u8)video_mode);
    if (video_mode == 0x13) {
        if (!(m & 0x80)) memset(M + 0xa0000, 0, 0x10000);
        /* default palette: first 16 EGA colours, a grey ramp; the game sets its own */
        static const u8 ega[16][3] = {{0,0,0},{0,0,42},{0,42,0},{0,42,42},{42,0,0},{42,0,42},{42,21,0},{42,42,42},
            {21,21,21},{21,21,63},{21,63,21},{21,63,63},{63,21,21},{63,21,63},{63,63,21},{63,63,63}};
        for (int i = 0; i < 256; i++) for (int c = 0; c < 3; c++)
            vga_dac[i * 3 + c] = i < 16 ? ega[i][c] : (u8)((i & 63));
        wb(0x40, 0x4a, 40); wb(0x40, 0x84, 24);
    } else {
        wb(0x40, 0x4a, 80); wb(0x40, 0x84, 24);
    }
}

static void int10(void)
{
    if (bios_trace) fprintf(stderr, "int10 ax=%04x bx=%04x cx=%04x dx=%04x\n", AX, BX, CX, DX);
    switch (AH) {
    case 0x00: set_mode(AL); break;
    case 0x01: case 0x02: case 0x05: case 0x06: case 0x07: case 0x09: case 0x0b: break;
    case 0x03: DX = 0; CX = 0x0607; break;
    case 0x08: AX = 0x0720; break;
    case 0x0e: if (video_mode != 0x13) { fputc(AL, stdout); fflush(stdout); } break;
    case 0x0f: AL = (u8)video_mode; AH = rb(0x40, 0x4a); BH = 0; break;
    case 0x10:
        switch (AL) {
        case 0x10: vga_dac[(BX & 255) * 3] = DH & 63; vga_dac[(BX & 255) * 3 + 1] = CH & 63; vga_dac[(BX & 255) * 3 + 2] = CL & 63; break;
        case 0x12: for (int i = 0; i < CX; i++) for (int c = 0; c < 3; c++)
                       vga_dac[((BX + i) & 255) * 3 + c] = rb(ES, (u16)(DX + i * 3 + c)) & 63; break;
        case 0x15: DH = vga_dac[(BX & 255) * 3]; CH = vga_dac[(BX & 255) * 3 + 1]; CL = vga_dac[(BX & 255) * 3 + 2]; break;
        case 0x17: for (int i = 0; i < CX; i++) for (int c = 0; c < 3; c++)
                       wb(ES, (u16)(DX + i * 3 + c), vga_dac[((BX + i) & 255) * 3 + c]); break;
        default: break;
        }
        break;
    case 0x11: if (AL == 0x30) { CX = 8; DL = 24; ES = 0xf000; BP = 0xfa6e; } break;
    case 0x12: if (BL == 0x10) { BH = 0; BL = 3; CX = 0x0009; } break;
    case 0x1a: if (AL == 0) { AL = 0x1a; BX = 0x0008; } break;
    case 0x1b: AL = 0x1b; break;
    default: if (bios_trace) fprintf(stderr, "int 10h: AH=%02x\n", AH); break;
    }
}

u8 hle_in8(u16 port)
{
    switch (port) {
    case 0x3da: {               /* input status: vertical retrace at 70 Hz, ~1.4 ms long */
        uint64_t us = plat_ticks_us();
        static int toggle;
        toggle ^= 1;
        idle_hits++;
        u32 ph = (u32)(us % 14286);
        int vr = ph < 1400;
        poll_flag = 1;
        return (u8)((vr ? 8 : 0) | (vr || toggle ? 1 : 0));
    }
    case 0x3c9: {
        u8 v = vga_dac[(dac_r * 3 + dac_rc) % 768];
        if (++dac_rc == 3) { dac_rc = 0; dac_r = (dac_r + 1) & 255; }
        return v;
    }
    case 0x3c7: return 3;
    case 0x3c8: return (u8)dac_w;
    case 0x60: return port60;
    case 0x61: { u8 spk_in61(void); return spk_in61(); }   /* (the speaker's gate bits as written: runtime/speaker.c) */
    case 0x64: return 0x1c;     /* input buffer empty, output buffer empty */
    case 0x40:
        if (!pit_latch_hi) { pit_latch_hi = 1; return (u8)pit_latch; }
        pit_latch_hi = 0; return (u8)(pit_latch >> 8);
    case 0x21: return 0;
    case 0x388: case 0x389: case 0x38a: case 0x38b: case 0x228: case 0x229: return opl_in(port);
    case 0x220: case 0x221: case 0x222: case 0x223: return opl_in((u16)(port - 0x220 + 0x388));   /* Sound Blaster Pro FM */
    case 0x330: case 0x331: return mpu_in(port);
    case 0x201: return 0xff;    /* no joystick */
    default:
        { int sb_in(u16, u8 *); u8 v; if (sb_in(port, &v)) return v; }
        if (bios_trace) { fprintf(stderr, "in %04x\n", port); static int once; if (port == 0x331 && !once++) ftrace_dump(); }
        return 0xff;
    }
}

void hle_out8(u16 port, u8 v)
{
    /* an evaluation (gamecall.c) is thrown away with the memory it changed: the sound chips
       must not change either (a note started there would hang, the driver's idea of the chip
       and the chip would differ) */
    { extern int game_call_sound_live;               /* (mods/soundpreview.c: a sound driver changed for a preview: its setup must be heard) */
      if (in_game_call && !game_call_sound_live && ((port >= 0x220 && port <= 0x22f) || (port >= 0x388 && port <= 0x38b) || port == 0x330 || port == 0x331 || port == 0x42 || port == 0x61)) return; }
    void spk_out(u16, u8);
    switch (port) {
    case 0x3c8: dac_w = v; dac_wc = 0; break;
    case 0x3c7: dac_r = v; dac_rc = 0; break;
    case 0x3c9:
        { static int tr = -1; static u32 lt, n; if (tr < 0) tr = getenv("DL_TRACE_DAC") != 0;
          if (tr) { n++; u32 t = plat_ticks_ms(); if (t - lt >= 100) { fprintf(stderr, "dac %u: %u writes, sum %d\n", t, n, (int)vga_dac[0] + vga_dac[300] + vga_dac[600]); n = 0; lt = t; } } }
        vga_dac[(dac_w * 3 + dac_wc) % 768] = v & 63;
        if (++dac_wc == 3) { dac_wc = 0; dac_w = (dac_w + 1) & 255; }
        break;
    case 0x43:
        if ((v >> 6) == 2) { extern int game_call_sound_live; if (!in_game_call || game_call_sound_live) spk_out(port, v); break; }    /* (channel 2: the speaker's note) */
        if ((v & 0x30) == 0) { pit_latch = pit_count(); pit_latch_hi = 0; }
        else if ((v >> 6) == 0) pit_write_hi = 0;
        break;
    case 0x42: spk_out(port, v); break;
    case 0x40:
        if (!pit_write_hi) { pit_new = v; pit_write_hi = 1; }
        else { pit_new |= (u16)(v << 8); pit_div = pit_new; pit_write_hi = 0;
               if (bios_trace) fprintf(stderr, "PIT divisor %u\n", pit_div); }
        break;
    case 0x61: spk_out(port, v); break;
    case 0x20: case 0x21: case 0x60: case 0x64: break;
    case 0x388: case 0x389: case 0x38a: case 0x38b: opl_out(port, v); break;
    case 0x330: case 0x331: mpu_out(port, v); break;
    case 0x228: case 0x229: opl_out((u16)(port - 0x228 + 0x388), v); break;
    case 0x220: case 0x221: case 0x222: case 0x223: opl_out((u16)(port - 0x220 + 0x388), v); break;
    default:
        { int sb_out(u16, u8); if (sb_out(port, v)) break; }
        if (bios_trace) fprintf(stderr, "out %04x\n", port);
        vga_misc_idx[0] = v;
        break;
    }
}

/* ---------------------------------------------------------------- interrupt services */
static void service(int n)
{
    switch (n) {
    case 0x00: fatal("divide error (no handler)"); break;
    case 0x08: bios_int8(); break;
    case 0x09: bios_int9(); break;
    case 0x10: int10(); break;
    case 0x11: AX = 0x0063; break;
    case 0x12: AX = 640; break;
    case 0x15: if (AH == 0x88) { AX = 0; SETCF(0); } else { AH = 0x86; SETCF(1); } break;
    case 0x16:
        switch (AH & 0xef) {
        case 0x00: { u16 w; while (!(w = bios_kbd_get(1))) { poll_flag = 1; dl_poll(); plat_sleep_ms(5); } AX = w; break; }
        case 0x01: { u16 w = bios_kbd_get(0); SETZF(w == 0); if (w) AX = w; else spin_polls++; poll_flag = 1; break; }
        case 0x02: AL = shift_state; break;
        default: break;
        }
        break;
    case 0x1a: if (AH == 0) { CX = rw(0x40, 0x6e); DX = rw(0x40, 0x6c); AL = rb(0x40, 0x70); wb(0x40, 0x70, 0); } break;
    case 0x1c: break;
    case 0x21: dos_int21(); break;
    case 0x23: case 0x24: break;
    case 0x2f: if (AX == 0x4300) AL = 0; else if (AX == 0x1600) AL = 0; break;
    case 0x33: int33(); break;
    case 0x67: ems_int67(); break;
    default:
        if (bios_trace) fprintf(stderr, "int %02x: unhandled AX=%04x\n", n, AX);
        break;
    }
}

void watch_check(const char *where);
void hle_int(int n)
{
#ifdef FTRACE
    watch_check("code before int");
    if (n == 0) { fprintf(stderr, "divide error AX=%04x BX=%04x CX=%04x DX=%04x BP=%04x [BP+6]=%04x\n", AX, BX, CX, DX, BP, rw(SS, (u16)(BP + 6))); ftrace_dump(); }
#endif
    if (ivt_hooked(n)) { call_handler(n); return; }
    service(n);
#ifdef FTRACE
    { char b[32]; snprintf(b, sizeof b, "int %02x service", n); watch_check(b); }
#endif
}

/* a far call / jump reached F000:off - an original interrupt vector the game saved and chains to */
void hle_stub(u16 seg, u16 off)
{
    if (off >= 0xfe00) fatal("hle_stub: return sentinel %04x reached", off);
    int n = seg == EMS_SEG ? 0x67 : off / 4;
    service(n);
    /* iret: IF/TF/DF come back from the frame; the arithmetic flags of the service result
     * (CF etc.) are kept, as DOS/BIOS services return them with retf 2 */
    u16 fl = rw(SS, (u16)(SP + 4));
    SP = (u16)(SP + 6);
    /* a hardware interrupt the game's handler chained to (timer, keyboard, other IRQs, and
     * the timer's int 1Ch): a real iret, all the interrupted code's flags come back. Keeping
     * the "service result" flags here let a timer tick land between a compare and its jump
     * and flip it; the interpreted picture decoder then mis-decoded now and then (white
     * blocks on the travel map, grey blocks and garbage in card pictures). */
    if (n == 8 || n == 9 || n == 0x1c || (n >= 0x0a && n <= 0x0f) || (n >= 0x70 && n <= 0x77)) set_flags(fl);
    else set_flags((u16)((get_flags() & 0x08d5) | (fl & 0x0700)));
}

/* ---- snapshots for side-effect-free evaluations (gamecall.c): what the BIOS, the mouse
 * driver and the VGA keep outside the game's memory. An evaluated handler that asked the mouse
 * driver for clicks (int 33h 5/6) took the player's click away; one that set the palette left
 * it changed. */
typedef struct {
    int ms_x, ms_y, ms_b, ms_shown, ms_minx, ms_maxx, ms_miny, ms_maxy, ms_mickx, ms_micky;
    u16 ms_hmask, ms_hseg, ms_hoff; int ms_pending;
    int ms_press[3], ms_release[3], ms_lpx[3], ms_lpy[3], ms_lrx[3], ms_lry[3];
    u8 kq[64]; int kq_head, kq_tail; u8 port60; int kbd_e0; u8 shift_state;
    u8 vga_dac[768]; int video_mode, dac_w, dac_r, dac_wc, dac_rc; u8 vga_misc_idx[4];
    u16 pit_div; int pit_write_hi, pit_latch_hi; u16 pit_latch, pit_new;
} BiosSnap;
#define BS(f) do { memcpy(&b->f, &f, sizeof b->f); } while (0)
#define BR(f) do { memcpy(&f, &b->f, sizeof b->f); } while (0)
#define BIOS_FIELDS(X) X(ms_x); X(ms_y); X(ms_b); X(ms_shown); X(ms_minx); X(ms_maxx); X(ms_miny); X(ms_maxy); \
    X(ms_mickx); X(ms_micky); X(ms_hmask); X(ms_hseg); X(ms_hoff); X(ms_pending); X(ms_press); X(ms_release); \
    X(ms_lpx); X(ms_lpy); X(ms_lrx); X(ms_lry); X(kq); X(kq_head); X(kq_tail); X(port60); X(kbd_e0); X(shift_state); \
    X(vga_dac); X(video_mode); X(dac_w); X(dac_r); X(dac_wc); X(dac_rc); X(vga_misc_idx); \
    X(pit_div); X(pit_write_hi); X(pit_latch_hi); X(pit_latch); X(pit_new)
void *bios_snapshot(void) { BiosSnap *b = malloc(sizeof *b); BIOS_FIELDS(BS); return b; }
void bios_restore(void *p) { BiosSnap *b = p; BIOS_FIELDS(BR); }
void bios_snapshot_free(void *p) { free(p); }

/* ---------------------------------------------------------------- poll */
static int in_irq;
int dl_in_irq(void) { return in_irq; }
/* the soundtrack's player (mods/playlist.c): the timer interrupts that drive the music driver,
   counted while a piece plays; held back while it is paused; a burst of them at once to move
   through a piece (the notes held back meanwhile: music_ff, runtime/mpu.c, runtime/sound.c) */
volatile u32 music_ticks; int music_hold;
volatile u32 sound_tick_late_us;          /* the tick being delivered fell due so long ago (the sound's timestamps go back by it) */
double bios_tick_us(void);
void bios_timer_burst(int n)
{
    if (!R.iflag || !ivt_hooked(8)) return;
    int was = in_irq; in_irq = 1;
    for (int k = 0; k < n; k++) { call_handler(8); music_ticks++; }
    in_irq = was;
}
static u32 last_present;
extern int poll_count_events;

int in_game_call;
void dl_poll(void)
{
    poll_flag = 0;
    { static int tr = -1; static u32 lt, np; if (tr < 0) tr = getenv("DL_TRACE_POLL") != 0;
      if (tr) { np++; u32 t = plat_ticks_ms(); if (t - lt >= 1000) { lt = t; fprintf(stderr, "poll %u: %u polls, game call %d, irq %d, if %d, menu %d, cs %04x\n", t, np, in_game_call, in_irq, R.iflag, plat_menu_open(), CS); np = 0; } } }
    if (in_game_call) { void call_watchdog(void); call_watchdog(); return; }
    if (in_irq) return;
    if (idle_hits > 300) plat_sleep_ms(1);     /* the game is busy-waiting: give the CPU back */
    idle_hits = 0;
    /* the card menus, screens and the travel map wait for a click or a key by asking the mouse
       driver for button releases and the keyboard for a key, up to a million times a second:
       after 150 such questions, a millisecond's rest (a card sat at 75 % of a core, a battle at
       a whole core). Battles too: their work is done once per clock tick (combat_tick, s014
       0e76:0f58, counts the same ticks a second with and without the rest). */
    { static int tr = -1; static u32 lt, rests, polls_seen; if (tr < 0) tr = getenv("DL_TRACE_SPIN") != 0;
      if (tr) { polls_seen += (u32)spin_polls; u32 t = plat_ticks_ms(); if (t - lt >= 1000) { int game_mode(void); fprintf(stderr, "spin: mode %d, %u polls/s, still for %u ms\n", game_mode(), polls_seen, t - screen_changed); polls_seen = 0; lt = t; } } }
    if (spin_polls > spin_limit()) {
        int ms = spin_polls / spin_limit(); if (ms > 8) ms = 8;     /* a millisecond per 150 questions */
        spin_polls = 0;
        int game_mode(void);
        if (!getenv_spin_off()) plat_sleep_ms((u32)ms);
    }
    u32 now = plat_ticks_ms();
    if (now != last_ms) {
        u32 d = now - last_ms; last_ms = now;
        { static u32 mx, lt2; static int on = -1; if (on < 0) on = getenv("DL_TRACE_PIT") != 0;
          if (on) { if (d > mx) mx = d; if (now - lt2 >= 1000) { fprintf(stderr, "pit: longest gap %u ms\n", mx); mx = 0; lt2 = now; } } }
        if (d > 500) d = 500;
        tick_us_acc += d * 1000.0;
    }
    /* a frame every 14 ms, but the game always gets at least as long to run as the last frame
     * took to draw: where drawing is slow (software rendering, a driver forcing vsync) the
     * frame rate drops instead of the game crawling (it stalled mid-picture before) */
    static u32 present_end, present_cost;
    if (now - last_present >= 14 && now - present_end >= present_cost) {
        last_present = now;
        plat_events();
        plat_present(M + 0xa0000, vga_dac);
        {   /* when the picture or the palette last changed (for the rest above) */
            static uint64_t last_hash;
            uint64_t h = 1469598103934665603ull; const uint64_t *v = (const uint64_t *)(M + 0xa0000);
            for (int i = 0; i < 8000; i++) h = (h ^ v[i]) * 1099511628211ull;
            const uint64_t *pd = (const uint64_t *)vga_dac;
            for (int i = 0; i < 96; i++) h = (h ^ pd[i]) * 1099511628211ull;
            if (h != last_hash) { last_hash = h; screen_changed = now; }
        }
        present_end = plat_ticks_ms();
        present_cost = present_end - now; if (present_cost > 100) present_cost = 100;
        if (plat_menu_open()) {               /* the game is paused while the menu is open */
            /* the music plays on (sound.music_in_menus) while the world map, F12 or another window of
               Darklands+ is open, outside battles: the game's timer interrupt (which drives the music
               driver) keeps coming while the game itself waits; in a battle (its clock counts those
               ticks) and with the window in the background, the sound stops as before */
            int game_mode(void), plat_in_background(void);
            int soundpreview_active(void);
            /* (asked again as the window stays open: a preview or the soundtrack's player started in
               it, with the music in menus off, was silent - the sound held and no tick delivered - for
               as long as the window stayed open) */
            #define KEEP_MUSIC() ((config_get_int("sound.music_in_menus", 1) || soundpreview_active()) && game_mode() != 4 /* GM_BATTLE */ && R.iflag && ivt_hooked(8))
            int keep = KEEP_MUSIC();
            void sound_pause(int);
            if (!keep) sound_pause(1);
            /* while music plays (a preview, the soundtrack's player, the music in menus) the timer's
               ticks go to the driver as they fall due, a millisecond at a time, and each is dated to
               when it fell due (sound_tick_late_us: the MT-32's messages and the FM chip's writes are
               placed by it), with the window drawn every 14 ms between: the ticks came in bursts
               after each frame and its 15 ms rest before, at most 32 at once (a fast timer lost
               ticks: the music slowed and its notes bunched) */
            u32 lt = plat_ticks_ms(), drawn = 0; double acc = 0;
            int paused_bg = 0, first = 1;
            while (plat_menu_open()) {
                int playing = keep && !paused_bg && !music_hold;
                u32 now2 = plat_ticks_ms();
                if (first || !playing || now2 - drawn >= 14) { plat_events(); plat_present(M + 0xa0000, vga_dac); drawn = plat_ticks_ms(); first = 0; }
                int bg = plat_in_background();
                { int k2 = KEEP_MUSIC(); if (k2 != keep) { keep = k2; sound_pause(!keep || bg); paused_bg = bg; lt = plat_ticks_ms(); acc = 0; } }
                if (keep && bg != paused_bg) { sound_pause(bg); paused_bg = bg; }
                if (keep && !bg && !music_hold) {
                    u32 t = plat_ticks_ms(); u32 d = t - lt; lt = t; if (d > 500) d = 500;
                    acc += d * 1000.0;
                    double per = pit_period_us();
                    in_irq = 1;
                    while (acc >= per) { acc -= per; sound_tick_late_us = (u32)acc; call_handler(8); music_ticks++; }   /* (all that fell due: the music keeps its tempo) */
                    sound_tick_late_us = 0;
                    { int sb_poll(void); int v = sb_poll(); if (v && ivt_hooked(v)) call_handler(v); }
                    in_irq = 0;
                    plat_sleep_ms(1);
                } else { int plat_bg_sleep_ms(void); plat_sleep_ms((u32)plat_bg_sleep_ms()); lt = plat_ticks_ms(); }
            }
            #undef KEEP_MUSIC
            sound_pause(0);
            last_ms = now = present_end = plat_ticks_ms();
            tick_us_acc = 0;
        }
    }
    if (!R.iflag) return;
    in_irq = 1;
    double per = pit_period_us();
    int delivered = 0;
    /* the timer's ticks as they fall due: a slow frame (a big window, a heavy screen drawn) leaves
       several due at once, and they are all delivered (the music driver counts its tempo in them:
       ticks dropped played the music slower); only a stall of half a second or more (loading, the
       window dragged) is let go */
    while (tick_us_acc >= per && delivered < 32) {
        tick_us_acc -= per;
        delivered++;
        sound_tick_late_us = tick_us_acc < 200000.0 ? (u32)tick_us_acc : 0;   /* (each dated to when it fell due) */
        if (ivt_hooked(8)) call_handler(8); else bios_int8();
        music_ticks++;
    }
    sound_tick_late_us = 0;
    static u32 st_deliv, st_drop, st_t; static int st_on = -1;
    if (st_on < 0) st_on = getenv("DL_TRACE_PIT") != 0;
    if (st_on) { st_deliv += (u32)delivered; if (tick_us_acc > 500000.0) st_drop += (u32)(tick_us_acc / per) - 1;
                 if (now - st_t >= 1000) { fprintf(stderr, "pit %u: %.1f Hz, %u ticks delivered, %u dropped\n", now, 1e6 / per, st_deliv, st_drop); st_deliv = st_drop = 0; st_t = now; } }
    if (tick_us_acc > 500000.0) tick_us_acc = per;   /* fell far behind (a stall): drop ticks */
    { int sb_poll(void); int v = sb_poll(); if (v && ivt_hooked(v)) call_handler(v); }   /* Sound Blaster transfer done */
    static u32 last_key_ms;
    while (kq_head != kq_tail && now - last_key_ms >= 10) {   /* one key event per 10 ms, as a keyboard would */
        port60 = kq[kq_head]; kq_head = (kq_head + 1) % 64;
        if (port60 != 0xe0) last_key_ms = now;
        { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_INPUT") != 0;
          if (tr) fprintf(stderr, "kbd irq: %02x (int 9 %s)\n", port60, ivt_hooked(9) ? "game" : "bios"); }
        if (ivt_hooked(9)) call_handler(9); else bios_int9();
    }
    { static int st = -1; if (st < 0) { const char *e = getenv("DL_STRESS"); st = !e ? 0 : !strcmp(e, "timer") ? 1 : !strcmp(e, "mouse") ? 2 : 0; }
      if (st == 1 && ivt_hooked(8)) { static u32 lastv; u32 v = rw(0, 0x22) << 16 | rw(0, 0x20); if (v != lastv) { fprintf(stderr, "stress: int 8 -> %04x:%04x\n", v >> 16, v & 0xffff); lastv = v; }
          { static u32 l1, l2; u16 cs = (u16)(LOADSEG + 0x47c); u32 a = (u32)rw(cs, 0x215) << 16 | rw(cs, 0x213), b = (u32)rw(cs, 0x221) << 16 | rw(cs, 0x21f);
            if (a != l1 || b != l2) { fprintf(stderr, "stress: timer calls %04x:%04x and %04x:%04x (LOADSEG %04x)\n", a >> 16, a & 0xffff, b >> 16, b & 0xffff, LOADSEG); l1 = a; l2 = b;
                static const u16 P[] = { 0x12, 0x2c, 0x51, 0x5d, 0x62, 0x7a0 };
                for (int k = 0; k < 6; k++) fprintf(stderr, "  047c:%04x -> %04x:%04x\n", P[k], rw(cs, (u16)(P[k] + 2)), rw(cs, P[k])); } } call_handler(8); }   /* test: a timer interrupt at every poll */
      if (st == 2) ms_pending |= 1; }                                 /* test: a mouse event at every poll */
    if (ms_resync == 2) { ms_resync = 0; int b = ms_b; mouse_update(ms_hostx, ms_hosty, b); }   /* back to where the mouse is */
    if (ms_pending) mouse_deliver();
    in_irq = 0;
}

void bios_init(void)
{
    for (int n = 0; n < 256; n++) {
        ww(0, (u16)(n * 4), (u16)(n * 4));
        ww(0, (u16)(n * 4 + 2), HLE_SEG);
        M[HLE_SEG * 16 + n * 4] = 0xcf;     /* iret, for code that peeks */
    }
    ww(0, 0x67 * 4, 0x0100); ww(0, 0x67 * 4 + 2, EMS_SEG);
    memcpy(M + EMS_SEG * 16 + 0x0a, "EMMXXXX0", 8);
    M[EMS_SEG * 16 + 0x100] = 0xcf;
    ems_init();
    ww(0x40, 0x10, 0x0063);
    ww(0x40, 0x13, 640);
    ww(0x40, 0x1a, 0x1e); ww(0x40, 0x1c, 0x1e);
    ww(0x40, 0x80, 0x1e); ww(0x40, 0x82, 0x3e);
    wb(0x40, 0x49, 3); wb(0x40, 0x4a, 80); wb(0x40, 0x84, 24);
    ww(0x40, 0x63, 0x3d4);
    memcpy(M + 0xffff5, "01/01/92", 8);
    M[0xffffe] = 0xfc;
    set_mode(3);
    sound_init();
    last_ms = plat_ticks_ms();
}
