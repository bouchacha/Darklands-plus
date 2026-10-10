/* bios.c - interrupts, BIOS services and the PC hardware the game talks to (VGA DAC, PIT,
 * keyboard controller, PIC), plus the safe-point poll() that delivers timer and keyboard
 * interrupts to the game's own handlers and refreshes the screen. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"

void dos_int21(void);
void opl_out(u16 port, u8 v); u8 opl_in(u16 port); void sound_init(void);
void mpu_out(u16 port, u8 v); u8 mpu_in(u16 port);
void ems_int67(void); void ems_init(void);
int bios_trace;

volatile int poll_flag = 1;
int idle_hits;          /* busy-wait polls (keyboard status, retrace) since the last poll() */
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
    {   char msg[512]; va_start(ap, fmt); vsnprintf(msg, sizeof msg, fmt, ap); va_end(ap);
        void plat_error_box(const char *); plat_error_box(msg); }
    ftrace_dump();
    { void interp_ring_dump(void); interp_ring_dump(); void ovl_dump(void); ovl_dump(); }
    fflush(stderr);
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
static void call_handler(int n)
{
    u16 off = rw(0, (u16)(n * 4)), seg = rw(0, (u16)(n * 4 + 2));
    push16(get_flags());
    R.iflag = 0; R.tf = 0;
    push16(HLE_SEG);
    push16((u16)(0xff00 | n));
    u16 ss0 = SS, sp0 = (u16)(SP + 6), bp0 = BP, si0 = SI, di0 = DI, ds0 = DS, es0 = ES;
    dispatch_far(seg, off);
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
static int ms_press[3], ms_release[3], ms_lpx[3], ms_lpy[3], ms_lrx[3], ms_lry[3];

void mouse_update(int x, int y, int buttons)
{
    if (x < ms_minx) x = ms_minx; if (x > ms_maxx) x = ms_maxx;
    if (y < ms_miny) y = ms_miny; if (y > ms_maxy) y = ms_maxy;
    if (x != ms_x || y != ms_y) {
        ms_mickx += (x - ms_x); ms_micky += (y - ms_y) * 2;
        ms_x = x; ms_y = y; ms_pending |= 1;
    }
    for (int i = 0; i < 3; i++) {
        int was = (ms_b >> i) & 1, now = (buttons >> i) & 1;
        if (now && !was) { ms_press[i]++; ms_lpx[i] = ms_x; ms_lpy[i] = ms_y; ms_pending |= 2 << (2 * i); }
        if (!now && was) { ms_release[i]++; ms_lrx[i] = ms_x; ms_lry[i] = ms_y; ms_pending |= 4 << (2 * i); }
    }
    ms_b = buttons;
}

static void int33(void)
{
    switch (AX) {
    case 0x00: AX = 0xffff; BX = 2; ms_shown = 0; ms_hmask = 0; break;
    case 0x01: ms_shown++; break;
    case 0x02: ms_shown--; break;
    case 0x03: BX = (u16)ms_b; CX = (u16)ms_x; DX = (u16)ms_y; break;
    case 0x04: ms_x = CX; ms_y = DX; break;
    case 0x05: { int b = BX & 3; AX = (u16)ms_b; BX = (u16)ms_press[b]; CX = (u16)ms_lpx[b]; DX = (u16)ms_lpy[b]; ms_press[b] = 0; break; }
    case 0x06: { int b = BX & 3; AX = (u16)ms_b; BX = (u16)ms_release[b]; CX = (u16)ms_lrx[b]; DX = (u16)ms_lry[b]; ms_release[b] = 0; break; }
    case 0x07: ms_minx = (s16)CX < (s16)DX ? (s16)CX : (s16)DX; ms_maxx = (s16)CX < (s16)DX ? (s16)DX : (s16)CX; break;
    case 0x08: ms_miny = (s16)CX < (s16)DX ? (s16)CX : (s16)DX; ms_maxy = (s16)CX < (s16)DX ? (s16)DX : (s16)CX; break;
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
    AX = (u16)ev; BX = (u16)ms_b; CX = (u16)ms_x; DX = (u16)ms_y; SI = (u16)ms_mickx; DI = (u16)ms_micky;
    push16(HLE_SEG); push16(0xfe00);
    dispatch_far(ms_hseg, ms_hoff);
    u16 ds = R.ds; (void)ds;
    R = save;
}

/* ---------------------------------------------------------------- PIT / timer */
static u16 pit_div = 0;         /* channel 0 divisor (0 = 65536) */
static int pit_write_hi, pit_latch_hi; static u16 pit_latch, pit_new;
static double tick_us_acc;
static u32 last_ms;
static double pit_period_us(void) { return (pit_div ? pit_div : 65536) * 1e6 / 1193182.0; }

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
    case 0x61: return 0x20;
    case 0x64: return 0x1c;     /* input buffer empty, output buffer empty */
    case 0x40:
        if (!pit_latch_hi) { pit_latch_hi = 1; return (u8)pit_latch; }
        pit_latch_hi = 0; return (u8)(pit_latch >> 8);
    case 0x21: return 0;
    case 0x388: case 0x389: case 0x38a: case 0x38b: case 0x228: case 0x229: return opl_in(port);
    case 0x330: case 0x331: return mpu_in(port);
    case 0x201: return 0xff;    /* no joystick */
    default:
        if (bios_trace) { fprintf(stderr, "in %04x\n", port); static int once; if (port == 0x331 && !once++) ftrace_dump(); }
        return 0xff;
    }
}

void hle_out8(u16 port, u8 v)
{
    switch (port) {
    case 0x3c8: dac_w = v; dac_wc = 0; break;
    case 0x3c7: dac_r = v; dac_rc = 0; break;
    case 0x3c9:
        vga_dac[(dac_w * 3 + dac_wc) % 768] = v & 63;
        if (++dac_wc == 3) { dac_wc = 0; dac_w = (dac_w + 1) & 255; }
        break;
    case 0x43:
        if ((v & 0x30) == 0) { pit_latch = pit_count(); pit_latch_hi = 0; }
        else if ((v >> 6) == 0) pit_write_hi = 0;
        break;
    case 0x40:
        if (!pit_write_hi) { pit_new = v; pit_write_hi = 1; }
        else { pit_new |= (u16)(v << 8); pit_div = pit_new; pit_write_hi = 0;
               if (bios_trace) fprintf(stderr, "PIT divisor %u\n", pit_div); }
        break;
    case 0x20: case 0x21: case 0x61: case 0x60: case 0x64: break;
    case 0x388: case 0x389: case 0x38a: case 0x38b: opl_out(port, v); break;
    case 0x330: case 0x331: mpu_out(port, v); break;
    case 0x228: case 0x229: opl_out((u16)(port - 0x228 + 0x388), v); break;
    default:
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
        case 0x00: { u16 w; while (!(w = bios_kbd_get(1))) { poll_flag = 1; poll(); plat_sleep_ms(5); } AX = w; break; }
        case 0x01: { u16 w = bios_kbd_get(0); SETZF(w == 0); if (w) AX = w; poll_flag = 1; break; }
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
    set_flags((u16)((get_flags() & 0x08d5) | (fl & 0x0700)));
}

/* ---------------------------------------------------------------- poll */
static int in_irq;
static u32 last_present;
extern int poll_count_events;

void poll(void)
{
    poll_flag = 0;
    if (in_irq) return;
    if (idle_hits > 300) plat_sleep_ms(1);     /* the game is busy-waiting: give the CPU back */
    idle_hits = 0;
    u32 now = plat_ticks_ms();
    if (now != last_ms) {
        u32 d = now - last_ms; last_ms = now;
        if (d > 200) d = 200;
        tick_us_acc += d * 1000.0;
    }
    if (now - last_present >= 14) {
        last_present = now;
        plat_events();
        plat_present(M + 0xa0000, vga_dac);
    }
    if (!R.iflag) return;
    in_irq = 1;
    double per = pit_period_us();
    int delivered = 0;
    while (tick_us_acc >= per && delivered < 4) {
        tick_us_acc -= per;
        delivered++;
        if (ivt_hooked(8)) call_handler(8); else bios_int8();
    }
    if (tick_us_acc > per * 8) tick_us_acc = per;   /* fell behind: drop ticks */
    static u32 last_key_ms;
    while (kq_head != kq_tail && now - last_key_ms >= 10) {   /* one key event per 10 ms, as a keyboard would */
        port60 = kq[kq_head]; kq_head = (kq_head + 1) % 64;
        if (port60 != 0xe0) last_key_ms = now;
        { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_INPUT") != 0;
          if (tr) fprintf(stderr, "kbd irq: %02x (int 9 %s)\n", port60, ivt_hooked(9) ? "game" : "bios"); }
        if (ivt_hooked(9)) call_handler(9); else bios_int9();
    }
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
