/* cpu.h - machine state for the statically recompiled Darklands.
 *
 * Every original x86 instruction is translated to C operating on this state (tools in
 * port/recomp). Memory is the real-mode address space: linear = seg*16 + off (16-bit
 * offset wrap, no A20 wrap). Flags are evaluated lazily from the last flag-setting
 * operation. */
#ifndef DL_CPU_H
#define DL_CPU_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32;
typedef int8_t s8; typedef int16_t s16; typedef int32_t s32;
#include "split.h"

#define MEM_SIZE (0x110000 + 16)
extern u8 *M;                       /* MEM_SIZE bytes */

typedef struct {
    union { u16 x; struct { u8 l, h; } b; } a, b, c, d;
    u16 si, di, bp, sp;
    u16 cs, ds, es, ss;
    /* lazy flags */
    int fop;                        /* FOP_* of the last flag-setting operation */
    u32 fa, fb, fr;                 /* operands and result (masked to width) */
    int fw;                         /* width: 8 or 16 */
    int fcf;                        /* carry for FOP_INC/DEC/EXPLICIT, materialised */
    int fof, fzf, fsf, fpf, faf;    /* for FOP_EXPLICIT */
    int df, iflag, tf;
} CPU;
extern CPU R;

#define AX R.a.x
#define BX R.b.x
#define CX R.c.x
#define DX R.d.x
#define AL R.a.b.l
#define AH R.a.b.h
#define BL R.b.b.l
#define BH R.b.b.h
#define CL R.c.b.l
#define CH R.c.b.h
#define DL R.d.b.l
#define DH R.d.b.h
#define SI R.si
#define DI R.di
#define BP R.bp
#define SP R.sp
#define CS R.cs
#define DS R.ds
#define ES R.es
#define SS R.ss

#define LIN(s, o) ((((u32)(u16)(s)) << 4) + (u16)(o))
static inline u8  rb(u16 s, u16 o) { return M[LIN(s, o)]; }
static inline u16 rw(u16 s, u16 o) { u32 l = LIN(s, o); return (u16)(M[l] | (M[l + 1] << 8)); }
#ifdef FTRACE
extern u32 watch_lo, watch_len; void watch_hit(u32 l, u16 v);
#define WATCH(l, v) do { if ((u32)((l) - watch_lo) < watch_len) watch_hit(l, v); } while (0)
#else
#define WATCH(l, v)
#endif
static inline void wb(u16 s, u16 o, u8 v) { u32 l = LIN(s, o); WATCH(l, v); M[l] = v; }
static inline void ww(u16 s, u16 o, u16 v) { u32 l = LIN(s, o); WATCH(l, v); WATCH(l + 1, v); M[l] = (u8)v; M[l + 1] = (u8)(v >> 8); }
static inline void push16(u16 v) { SP -= 2; ww(SS, SP, v); }
static inline u16 pop16(void) { u16 v = rw(SS, SP); SP += 2; return v; }

enum { FOP_ADD, FOP_ADC, FOP_SUB, FOP_SBB, FOP_LOGIC, FOP_INC, FOP_DEC, FOP_NEG, FOP_EXPLICIT };

static inline u32 msk(int w) { return w == 8 ? 0xffu : 0xffffu; }
static inline u32 sgn(int w) { return w == 8 ? 0x80u : 0x8000u; }

int get_cf(void); int get_zf(void); int get_sf(void); int get_of(void); int get_pf(void); int get_af(void);
u16 get_flags(void); void set_flags(u16 f);
void flags_explicit(int cf, int of, u32 res, int w);   /* res drives ZF/SF/PF */

/* arithmetic helpers: return the result, record lazy flags */
static inline u32 f_add(u32 a, u32 b, int w) { u32 r = (a + b) & msk(w); R.fop = FOP_ADD; R.fa = a; R.fb = b; R.fr = r; R.fw = w; return r; }
static inline u32 f_adc(u32 a, u32 b, int w) { int c = get_cf(); u32 r = (a + b + c) & msk(w); R.fop = FOP_ADC; R.fa = a; R.fb = b; R.fr = r; R.fw = w; R.fcf = c; return r; }
static inline u32 f_sub(u32 a, u32 b, int w) { u32 r = (a - b) & msk(w); R.fop = FOP_SUB; R.fa = a; R.fb = b; R.fr = r; R.fw = w; return r; }
static inline u32 f_sbb(u32 a, u32 b, int w) { int c = get_cf(); u32 r = (a - b - c) & msk(w); R.fop = FOP_SBB; R.fa = a; R.fb = b; R.fr = r; R.fw = w; R.fcf = c; return r; }
static inline u32 f_logic(u32 r, int w) { r &= msk(w); R.fop = FOP_LOGIC; R.fr = r; R.fw = w; return r; }
static inline u32 f_inc(u32 a, int w) { int c = get_cf(); u32 r = (a + 1) & msk(w); R.fop = FOP_INC; R.fa = a; R.fr = r; R.fw = w; R.fcf = c; return r; }
static inline u32 f_dec(u32 a, int w) { int c = get_cf(); u32 r = (a - 1) & msk(w); R.fop = FOP_DEC; R.fa = a; R.fr = r; R.fw = w; R.fcf = c; return r; }
static inline u32 f_neg(u32 a, int w) { u32 r = (0 - a) & msk(w); R.fop = FOP_NEG; R.fa = 0; R.fb = a; R.fr = r; R.fw = w; return r; }
u32 f_shl(u32 a, int n, int w); u32 f_shr(u32 a, int n, int w); u32 f_sar(u32 a, int n, int w);
u32 f_rol(u32 a, int n, int w); u32 f_ror(u32 a, int n, int w);
u32 f_rcl(u32 a, int n, int w); u32 f_rcr(u32 a, int n, int w);
void f_mul8(u8 v); void f_mul16(u16 v); void f_imul8(s8 v); void f_imul16(s16 v);
void f_div8(u8 v); void f_div16(u16 v); void f_idiv8(s8 v); void f_idiv16(s16 v);
u16 f_imul3(s16 a, s16 b);          /* imul r16, r/m16, imm */
void f_daa(void); void f_das(void);

/* conditions */
#define C_E   (get_zf())
#define C_NE  (!get_zf())
#define C_B   (get_cf())
#define C_AE  (!get_cf())
#define C_BE  (get_cf() || get_zf())
#define C_A   (!get_cf() && !get_zf())
#define C_L   (get_sf() != get_of())
#define C_GE  (get_sf() == get_of())
#define C_LE  (get_zf() || get_sf() != get_of())
#define C_G   (!get_zf() && get_sf() == get_of())
#define C_S   (get_sf())
#define C_NS  (!get_sf())
#define C_O   (get_of())
#define C_NO  (!get_of())

/* frames. Every recompiled function knows the stack slot of its own return address
 * (my_sp, SP at entry). A ret popping a slot above it returns from an outer frame: the
 * native C frames in between unwind (UNW_CHECK after every call) until the frame that owns
 * the slot, which then returns too. A ret popping a slot below it jumps to an address the
 * function pushed itself. */
extern int unw_active; extern u16 unw_sp, unw_ip, unw_cs; extern int unw_far;
/* a jump into another function (tail call) keeps the frame: the callee takes my_sp over */
extern int tail_pending; extern u16 tail_sp;
#define TAIL_SET() (tail_sp = my_sp, tail_pending = 1)
static inline u16 frame_sp(void) { if (tail_pending) { tail_pending = 0; return tail_sp; } return SP; }
#define FRAME_SP() frame_sp()
void ret_mismatch(u16 my_sp, int kind, u16 n, u16 cs);   /* kind 0 near, 1 far, 2 iret */
#define UNW_CHECK() do { if (unw_active) { if (unw_sp == my_sp) unw_active = 0; return; } } while (0)
#define RET_NEAR(n, cs) do { if (SP != my_sp) { ret_mismatch(my_sp, 0, (n), (cs)); return; } SP = (u16)(SP + 2 + (n)); return; } while (0)
#define RET_FAR(n) do { if (SP != my_sp) { ret_mismatch(my_sp, 1, (n), 0); return; } SP = (u16)(SP + 4 + (n)); return; } while (0)
#define IRET() do { if (SP != my_sp) { ret_mismatch(my_sp, 2, 0, 0); return; } SP = (u16)(SP + 4); set_flags(pop16()); return; } while (0)

/* services (runtime) */
void hle_int(int n);
u8 hle_in8(u16 port); void hle_out8(u16 port, u8 v);
void dispatch_far(u16 seg, u16 off);    /* call far to a computed address (return addr already pushed) */
void dispatch_near(u16 cs, u16 off);    /* call near to a computed address in segment cs */
void dispatch_jmp_far(u16 seg, u16 off);/* jmp far: like call but the callee's return is ours */
void dispatch_jmp_near(u16 cs, u16 off);
void ovl_enter(int section);            /* RTLink: make sure an overlay section is loaded */
void dl_poll(void);                        /* safe point: timer interrupts, input, screen refresh */
extern volatile int poll_flag;
#define POLL() do { if (poll_flag) dl_poll(); } while (0)
void fatal(const char *fmt, ...);
void interp_step(u16 cs, u16 ip);
void interp_run(u16 seg, u16 off);     /* execute one (patched, non-branch) instruction */
extern u16 LOADSEG;                     /* paragraph where the program was loaded (PSP + 0x10) */
/* debugging: shadow stack of recompiled function entries (build with -DFTRACE) */
extern int tail_pending; extern u16 tail_sp;
#ifdef FTRACE
extern u32 ftrace_stack[4096]; extern int ftrace_depth;
extern int watch_on; void watch_check(const char *where);
extern u16 ftrace_bp[4096], ftrace_sp[4096], ftrace_ss[4096], ftrace_ret[4096]; void ftrace_sp_odd(int d);
void ftrace_bp_changed(int d);
extern u16 sp_low; void sp_low_hit(void);
static inline int ftrace_push(u32 id) { if (watch_on) watch_check("function entry"); if (R.sp < sp_low && R.ss == 0x341a) sp_low_hit(); if (ftrace_depth < 4096) { ftrace_stack[ftrace_depth] = id; ftrace_bp[ftrace_depth] = R.bp; ftrace_sp[ftrace_depth] = R.sp; ftrace_ss[ftrace_depth] = R.ss; ftrace_ret[ftrace_depth] = rw(R.ss, R.sp); if (tail_pending) ftrace_ss[ftrace_depth] = 0xdead; } return ftrace_depth++; }
extern u32 ret_ring[64]; extern u16 ret_ring_sp[64]; extern int ret_ring_i;
static inline void ftrace_pop(int *d) { if (*d < 4096) { ret_ring[ret_ring_i & 63] = ftrace_stack[*d]; ret_ring_sp[ret_ring_i++ & 63] = R.sp; } if (*d < 4096 && ftrace_bp[*d] != R.bp) ftrace_bp_changed(*d); if (*d < 4096 && R.ss == ftrace_ss[*d] && (u16)(R.sp - ftrace_sp[*d]) > 4 && (u16)(R.sp - ftrace_sp[*d]) < 0x8000) ftrace_sp_odd(*d); ftrace_depth = *d; }
#define FENTER(id) int ft_ __attribute__((cleanup(ftrace_pop))) = ftrace_push(id)
/* differential debugging: functions whose link address is in [interp_lo, interp_hi) run in
 * the interpreter instead (DL_INTERP=lo-hi, hex) */
extern u32 interp_lo, interp_hi; extern int interp_sect;
void interp_run(u16 seg, u16 off);
#define FCHECK(lin, frame, sect) do { if ((u32)(lin) - interp_lo < interp_hi - interp_lo && (interp_sect == -9 || interp_sect == (sect))) \
    { interp_run((u16)(LOADSEG + (frame)), (u16)((lin) - (frame) * 16)); return; } } while (0)
#else
#define FENTER(id)
#define FCHECK(lin, frame, sect)
#endif
void ftrace_dump(void);
#endif
