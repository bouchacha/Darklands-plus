/* interp.c - fallback x86 real-mode interpreter.
 *
 * The static recompiler only knows code that the analysis could reach. Code reached only
 * through data (interrupt handlers installed via pointers, callbacks, patched jumps) runs
 * here instead. Calls from interpreted code into recompiled code go native; the interpreter
 * returns when the code returns past the stack level it was entered with.
 * Every entry address is logged once (DL_LOG_INTERP=1) so it can be added to the
 * recompiler's extra_entries.txt. */
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "hle.h"
#include "strops.h"

int native_target(u16 seg, u16 off);      /* dispatch.c */
void native_go(u16 seg, u16 off);         /* dispatch.c: call native code (return address already pushed) */

static u16 ip, cs_;
static u32 ring[64]; static int ring_i;
void interp_ring_dump(void)
{
    fprintf(stderr, "  interpreter, last instructions:");
    for (int k = 0; k < 64; k++) { u32 v = ring[(ring_i + k) & 63]; if (v) fprintf(stderr, "%s%04x:%04x", k % 8 ? " " : "\n    ", v >> 16, v & 0xffff); }
    fprintf(stderr, "\n");
}
#ifdef FTRACE
u32 interp_lo, interp_hi; int interp_sect = -9;
#endif
static int seg_ovr;     /* -1 none, else segment value */
static u16 *seg_ovr_reg;

static u8 fetch8(void) { return rb(cs_, ip++); }
static u16 fetch16(void) { u16 v = rw(cs_, ip); ip += 2; return v; }

typedef struct { int mod, reg, rm; u16 seg, off; } ModRM;

static u16 *r16(int r)
{
    switch (r & 7) { case 0: return &R.a.x; case 1: return &R.c.x; case 2: return &R.d.x; case 3: return &R.b.x;
    case 4: return &R.sp; case 5: return &R.bp; case 6: return &R.si; default: return &R.di; }
}
static u8 *r8(int r)
{
    switch (r & 7) { case 0: return &R.a.b.l; case 1: return &R.c.b.l; case 2: return &R.d.b.l; case 3: return &R.b.b.l;
    case 4: return &R.a.b.h; case 5: return &R.c.b.h; case 6: return &R.d.b.h; default: return &R.b.b.h; }
}
static u16 *sreg(int r)
{
    switch (r & 3) { case 0: return &R.es; case 1: return &R.cs; case 2: return &R.ss; default: return &R.ds; }
}

static void decode(ModRM *m)
{
    u8 b = fetch8();
    m->mod = b >> 6; m->reg = (b >> 3) & 7; m->rm = b & 7;
    if (m->mod == 3) return;
    u16 off = 0, seg = DS;
    switch (m->rm) {
    case 0: off = BX + SI; break;
    case 1: off = BX + DI; break;
    case 2: off = BP + SI; seg = SS; break;
    case 3: off = BP + DI; seg = SS; break;
    case 4: off = SI; break;
    case 5: off = DI; break;
    case 6: if (m->mod == 0) off = fetch16(); else { off = BP; seg = SS; } break;
    case 7: off = BX; break;
    }
    if (m->mod == 1) off += (u16)(s16)(s8)fetch8();
    else if (m->mod == 2) off += fetch16();
    if (seg_ovr >= 0) seg = (u16)seg_ovr;
    m->seg = seg; m->off = off;
}
static u8 get8(ModRM *m) { return m->mod == 3 ? *r8(m->rm) : rb(m->seg, m->off); }
static u16 get16(ModRM *m) { return m->mod == 3 ? *r16(m->rm) : rw(m->seg, m->off); }
static void put8(ModRM *m, u8 v) { if (m->mod == 3) *r8(m->rm) = v; else wb(m->seg, m->off, v); }
static void put16(ModRM *m, u16 v) { if (m->mod == 3) *r16(m->rm) = v; else ww(m->seg, m->off, v); }

static u32 alu(int op, u32 a, u32 b, int w)
{
    switch (op) {
    case 0: return f_add(a, b, w);
    case 1: return f_logic(a | b, w);
    case 2: return f_adc(a, b, w);
    case 3: return f_sbb(a, b, w);
    case 4: return f_logic(a & b, w);
    case 5: return f_sub(a, b, w);
    case 6: return f_logic(a ^ b, w);
    default: f_sub(a, b, w); return a;   /* cmp */
    }
}
static u32 shift(int op, u32 a, int n, int w)
{
    n &= 31;
    switch (op) {
    case 0: return f_rol(a, n, w); case 1: return f_ror(a, n, w);
    case 2: return f_rcl(a, n, w); case 3: return f_rcr(a, n, w);
    case 4: case 6: return f_shl(a, n, w); case 5: return f_shr(a, n, w);
    default: return f_sar(a, n, w);
    }
}
static int cond(int c)
{
    switch (c & 15) {
    case 0: return C_O; case 1: return C_NO; case 2: return C_B; case 3: return C_AE;
    case 4: return C_E; case 5: return C_NE; case 6: return C_BE; case 7: return C_A;
    case 8: return C_S; case 9: return C_NS; case 10: return get_pf(); case 11: return !get_pf();
    case 12: return C_L; case 13: return C_GE; case 14: return C_LE; default: return C_G;
    }
}

static int logged(u32 lin)
{
    static u32 seen[512]; static int n;
    for (int i = 0; i < n; i++) if (seen[i] == lin) return 1;
    if (n < 512) seen[n++] = lin;
    return 0;
}

/* far/near transfer from interpreted code: native if possible */
int ovl_section_at(u32 lin);
/* the overlay section the interpreted caller runs in must be present again after the call
 * (RTLink reloads it on return) */
static int caller_section(void)
{
    u32 l = LIN(cs_, ip), b = (u32)LOADSEG << 4;
    return l >= b ? ovl_section_at(l - b) : -1;
}
/* after native code returned to the interpreter: handle an unwinding return (a ret that
 * popped some outer frame's return address). 0: continue, 1: leave interp_run */
static int after_native(u16 entry_sp)
{
    if (!unw_active) return 0;
    if (unw_sp == entry_sp) { unw_active = 0; return 1; }          /* our own entry frame returned */
    if ((u16)(unw_sp - entry_sp) < 0x8000) return 1;                /* beyond us: keep unwinding */
    ip = unw_ip; if (unw_far) cs_ = unw_cs;                         /* an interpreted frame returned */
    unw_active = 0;
    return 0;
}

/* an interpreted ret popped `slot`: our entry frame returns (1), an outer native frame
 * returns (1, unwinding), or an interpreted frame returns (0) */
static int ret_exit(u16 slot, u16 entry_ss, u16 entry_sp, int far)
{
    if (SS != entry_ss) return 0;
    if (slot == entry_sp) return 1;
    if ((u16)(slot - entry_sp) < 0x8000) {
        unw_active = 1; unw_sp = slot; unw_ip = ip; unw_cs = cs_; unw_far = far;
        return 1;
    }
    return 0;
}

/* 0: not native (caller transfers), 1: native call done, continue, 2: leave interp_run */
static int call_to(u16 seg, u16 off, u16 entry_sp)
{
    if (native_target(seg, off)) {
        int s = caller_section();
        native_go(seg, off);
        if (s >= 0) ovl_enter(s);
        return after_native(entry_sp) ? 2 : 1;
    }
    return 0;
}

/* jump from interpreted code. Interpreted targets: continue there (returns 0). Native
 * targets run to their return; if that return leaves the interpreter's entry frame we are
 * done (returns 1), else it returned to an address an interpreted call pushed: continue
 * there (near or far by how much it popped). */
static int jump_to(u16 seg, u16 off, u16 entry_ss, u16 entry_sp)
{
    if (!native_target(seg, off)) { cs_ = seg; ip = off; return 0; }
    u16 ss0 = SS, sp0 = SP;
    u16 w0 = rw(SS, SP), w1 = rw(SS, (u16)(SP + 2));
    int sec = caller_section();
    native_go(seg, off);
    if (sec >= 0) ovl_enter(sec);
    if (unw_active) return after_native(entry_sp);
    if (SS == entry_ss && SP > entry_sp) return 1;
    if (SS != ss0) fatal("interp: native jump target %04x:%04x switched stacks", seg, off);
    u16 delta = (u16)(SP - sp0);
    if (delta == 2) ip = w0; else { ip = w0; cs_ = w1; }
    return 0;
}

static int step_only;
void interp_run(u16 seg, u16 off);
void interp_step(u16 seg, u16 off)
{
    step_only = 1;
    interp_run(seg, off);
    step_only = 0;
}

void interp_run(u16 seg, u16 off)
{
    static int log = -1;
    if (log < 0) log = getenv("DL_LOG_INTERP") != 0;
    int one = step_only; step_only = 0;
    if (!one && log && !logged(LIN(seg, off)))
        { fprintf(stderr, "interp: entering %04x:%04x (link %05x)\n", seg, off, LIN(seg, off) - ((u32)LOADSEG << 4)); ftrace_dump(); }
    u16 save_ip = ip, save_cs = cs_;
    u16 entry_sp = SP, entry_ss = SS;
    if (tail_pending) { entry_sp = tail_sp; tail_pending = 0; }
    cs_ = seg; ip = off;
    u32 count = 0;
    for (;;) {
        if (((++count) & 255) == 0) POLL();
        seg_ovr = -1;
        int rep = 0;
        u16 start = ip;
        ring[ring_i++ & 63] = ((u32)cs_ << 16) | ip;
        u8 op;
    again:
        op = fetch8();
        switch (op) {
        case 0x26: seg_ovr = ES; goto again;
        case 0x2e: seg_ovr = cs_; goto again;
        case 0x36: seg_ovr = SS; goto again;
        case 0x3e: seg_ovr = DS; goto again;
        case 0xf0: goto again;
        case 0xf2: rep = 2; goto again;
        case 0xf3: rep = 1; goto again;
        default: break;
        }
        if (op < 0x40 && (op & 7) < 6) {     /* ALU r/m, reg, imm forms */
            int aop = op >> 3; ModRM m;
            switch (op & 7) {
            case 0: decode(&m); { u8 r = (u8)alu(aop, get8(&m), *r8(m.reg), 8); if (aop != 7) put8(&m, r); } break;
            case 1: decode(&m); { u16 r = (u16)alu(aop, get16(&m), *r16(m.reg), 16); if (aop != 7) put16(&m, r); } break;
            case 2: decode(&m); { u8 r = (u8)alu(aop, *r8(m.reg), get8(&m), 8); if (aop != 7) *r8(m.reg) = r; } break;
            case 3: decode(&m); { u16 r = (u16)alu(aop, *r16(m.reg), get16(&m), 16); if (aop != 7) *r16(m.reg) = r; } break;
            case 4: { u8 i = fetch8(); u8 r = (u8)alu(aop, AL, i, 8); if (aop != 7) AL = r; } break;
            case 5: { u16 i = fetch16(); u16 r = (u16)alu(aop, AX, i, 16); if (aop != 7) AX = r; } break;
            }
            continue;
        }
        switch (op) {
        case 0x06: push16(ES); break;
        case 0x07: ES = pop16(); break;
        case 0x0e: push16(cs_); break;
        case 0x16: push16(SS); break;
        case 0x17: SS = pop16(); break;
        case 0x1e: push16(DS); break;
        case 0x1f: DS = pop16(); break;
        case 0x27: f_daa(); break;
        case 0x2f: f_das(); break;
        case 0x37: case 0x3f: fatal("interp: aaa/aas at %04x:%04x", cs_, start); break;
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
            *r16(op) = (u16)f_inc(*r16(op), 16); break;
        case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
            *r16(op) = (u16)f_dec(*r16(op), 16); break;
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x55: case 0x56: case 0x57:
            push16(*r16(op)); break;
        case 0x54: { u16 t = SP; push16(t); break; }
        case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
            *r16(op) = pop16(); break;
        case 0x60: { u16 t = SP; push16(AX); push16(CX); push16(DX); push16(BX); push16(t); push16(BP); push16(SI); push16(DI); break; }
        case 0x61: DI = pop16(); SI = pop16(); BP = pop16(); pop16(); BX = pop16(); DX = pop16(); CX = pop16(); AX = pop16(); break;
        case 0x68: push16(fetch16()); break;
        case 0x6a: push16((u16)(s16)(s8)fetch8()); break;
        case 0x69: case 0x6b: { ModRM m; decode(&m); s16 a = (s16)get16(&m);
            s16 i = op == 0x69 ? (s16)fetch16() : (s16)(s8)fetch8(); *r16(m.reg) = f_imul3(a, i); break; }
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: {
            s8 d = (s8)fetch8(); if (cond(op)) ip = (u16)(ip + d); break; }
        case 0x80: case 0x82: { ModRM m; decode(&m); u8 i = fetch8(); u8 r = (u8)alu(m.reg, get8(&m), i, 8); if (m.reg != 7) put8(&m, r); break; }
        case 0x81: { ModRM m; decode(&m); u16 i = fetch16(); u16 r = (u16)alu(m.reg, get16(&m), i, 16); if (m.reg != 7) put16(&m, r); break; }
        case 0x83: { ModRM m; decode(&m); u16 i = (u16)(s16)(s8)fetch8(); u16 r = (u16)alu(m.reg, get16(&m), i, 16); if (m.reg != 7) put16(&m, r); break; }
        case 0x84: { ModRM m; decode(&m); f_logic(get8(&m) & *r8(m.reg), 8); break; }
        case 0x85: { ModRM m; decode(&m); f_logic(get16(&m) & *r16(m.reg), 16); break; }
        case 0x86: { ModRM m; decode(&m); u8 t = get8(&m); put8(&m, *r8(m.reg)); *r8(m.reg) = t; break; }
        case 0x87: { ModRM m; decode(&m); u16 t = get16(&m); put16(&m, *r16(m.reg)); *r16(m.reg) = t; break; }
        case 0x88: { ModRM m; decode(&m); put8(&m, *r8(m.reg)); break; }
        case 0x89: { ModRM m; decode(&m); put16(&m, *r16(m.reg)); break; }
        case 0x8a: { ModRM m; decode(&m); *r8(m.reg) = get8(&m); break; }
        case 0x8b: { ModRM m; decode(&m); *r16(m.reg) = get16(&m); break; }
        case 0x8c: { ModRM m; decode(&m); put16(&m, (m.reg & 3) == 1 ? cs_ : *sreg(m.reg)); break; }
        case 0x8d: { ModRM m; decode(&m); *r16(m.reg) = m.off; break; }
        case 0x8e: { ModRM m; decode(&m); *sreg(m.reg) = get16(&m); break; }
        case 0x8f: { ModRM m; decode(&m); u16 v = pop16(); put16(&m, v); break; }
        case 0x90: break;
        case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: { u16 t = AX; AX = *r16(op); *r16(op) = t; break; }
        case 0x98: AX = (u16)(s16)(s8)AL; break;
        case 0x99: DX = (AX & 0x8000) ? 0xffff : 0; break;
        case 0x9a: { u16 o = fetch16(), s = fetch16(); push16(cs_); push16(ip);
            { int r = call_to(s, o, entry_sp); if (r == 2) goto done_native; if (!r) { cs_ = s; ip = o; } } break; }
        case 0x9b: break;
        case 0x9c: push16(get_flags()); break;
        case 0x9d: set_flags(pop16()); break;
        case 0x9e: set_flags((u16)((get_flags() & 0xff00) | AH)); break;
        case 0x9f: AH = (u8)get_flags(); break;
        case 0xa0: { u16 o = fetch16(); AL = rb(seg_ovr >= 0 ? (u16)seg_ovr : DS, o); break; }
        case 0xa1: { u16 o = fetch16(); AX = rw(seg_ovr >= 0 ? (u16)seg_ovr : DS, o); break; }
        case 0xa2: { u16 o = fetch16(); wb(seg_ovr >= 0 ? (u16)seg_ovr : DS, o, AL); break; }
        case 0xa3: { u16 o = fetch16(); ww(seg_ovr >= 0 ? (u16)seg_ovr : DS, o, AX); break; }
        case 0xa4: str_movs8(rep ? 1 : 0, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xa5: str_movs16(rep ? 1 : 0, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xa6: str_cmps8(rep, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xa7: str_cmps16(rep, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xa8: f_logic(AL & fetch8(), 8); break;
        case 0xa9: f_logic(AX & fetch16(), 16); break;
        case 0xaa: str_stos8(rep ? 1 : 0, 0); break;
        case 0xab: str_stos16(rep ? 1 : 0, 0); break;
        case 0xac: str_lods8(rep ? 1 : 0, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xad: str_lods16(rep ? 1 : 0, seg_ovr >= 0 ? (u16)seg_ovr : DS); break;
        case 0xae: str_scas8(rep, 0); break;
        case 0xaf: str_scas16(rep, 0); break;
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: *r8(op) = fetch8(); break;
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: *r16(op) = fetch16(); break;
        case 0xc0: case 0xc1: case 0xd0: case 0xd1: case 0xd2: case 0xd3: {
            ModRM m; decode(&m);
            int n = (op == 0xc0 || op == 0xc1) ? fetch8() : (op <= 0xd1 ? 1 : CL);
            if (op & 1) put16(&m, (u16)shift(m.reg, get16(&m), n, 16)); else put8(&m, (u8)shift(m.reg, get8(&m), n, 8));
            break; }
        case 0xc2: case 0xc3: { u16 n = op == 0xc2 ? fetch16() : 0; u16 slot = SP; ip = pop16(); SP = (u16)(SP + n);
            if (ret_exit(slot, entry_ss, entry_sp, 0)) goto done; break; }
        case 0xca: case 0xcb: { u16 n = op == 0xca ? fetch16() : 0; u16 slot = SP; ip = pop16(); cs_ = pop16(); SP = (u16)(SP + n);
            if (ret_exit(slot, entry_ss, entry_sp, 1)) goto done; break; }
        case 0xcf: { u16 slot = SP; ip = pop16(); cs_ = pop16(); set_flags(pop16());
            if (ret_exit(slot, entry_ss, entry_sp, 1)) goto done; break; }
        case 0xc4: case 0xc5: { ModRM m; decode(&m); u16 v = rw(m.seg, m.off), s = rw(m.seg, (u16)(m.off + 2));
            *r16(m.reg) = v; if (op == 0xc4) ES = s; else DS = s; break; }
        case 0xc6: { ModRM m; decode(&m); put8(&m, fetch8()); break; }
        case 0xc7: { ModRM m; decode(&m); put16(&m, fetch16()); break; }
        case 0xc8: { u16 n = fetch16(); u8 l = fetch8(); if (l) fatal("interp: enter level");
            push16(BP); BP = SP; SP = (u16)(SP - n); break; }
        case 0xc9: SP = BP; BP = pop16(); break;
        case 0xcc: break;
        case 0xcd: hle_int(fetch8()); break;
        case 0xd4: { u8 b = fetch8(); if (!b) { hle_int(0); break; } AH = AL / b; AL = AL % b; f_logic(AL, 8); break; }
        case 0xd5: { u8 b = fetch8(); AL = (u8)(AL + AH * b); AH = 0; f_logic(AL, 8); break; }
        case 0xd7: AL = rb(seg_ovr >= 0 ? (u16)seg_ovr : DS, (u16)(BX + AL)); break;
        case 0xe0: { s8 d = (s8)fetch8(); CX--; if (CX && !get_zf()) ip = (u16)(ip + d); break; }
        case 0xe1: { s8 d = (s8)fetch8(); CX--; if (CX && get_zf()) ip = (u16)(ip + d); break; }
        case 0xe2: { s8 d = (s8)fetch8(); if (--CX) ip = (u16)(ip + d); break; }
        case 0xe3: { s8 d = (s8)fetch8(); if (!CX) ip = (u16)(ip + d); break; }
        case 0xe4: AL = hle_in8(fetch8()); break;
        case 0xe5: { u8 p = fetch8(); AL = hle_in8(p); AH = hle_in8((u16)(p + 1)); break; }
        case 0xe6: hle_out8(fetch8(), AL); break;
        case 0xe7: { u8 p = fetch8(); hle_out8(p, AL); hle_out8((u16)(p + 1), AH); break; }
        case 0xec: AL = hle_in8(DX); break;
        case 0xed: AL = hle_in8(DX); AH = hle_in8((u16)(DX + 1)); break;
        case 0xee: hle_out8(DX, AL); break;
        case 0xef: hle_out8(DX, AL); hle_out8((u16)(DX + 1), AH); break;
        case 0xe8: { s16 d = (s16)fetch16(); u16 t = (u16)(ip + d); push16(ip);
            { int r = call_to(cs_, t, entry_sp); if (r == 2) goto done_native; if (!r) ip = t; } break; }
        case 0xe9: { s16 d = (s16)fetch16(); u16 t = (u16)(ip + d);
            if (jump_to(cs_, t, entry_ss, entry_sp)) goto done_native; break; }
        case 0xea: { u16 o = fetch16(), s = fetch16(); if (jump_to(s, o, entry_ss, entry_sp)) goto done_native; break; }
        case 0xeb: { s8 d = (s8)fetch8(); ip = (u16)(ip + d); break; }
        case 0xf4: fatal("interp: hlt at %04x:%04x", cs_, start); break;
        case 0xf5: set_flags((u16)(get_flags() ^ 1)); break;
        case 0xf6: case 0xf7: {
            ModRM m; decode(&m); int w = op & 1;
            switch (m.reg) {
            case 0: case 1: if (w) f_logic(get16(&m) & fetch16(), 16); else f_logic(get8(&m) & fetch8(), 8); break;
            case 2: if (w) put16(&m, (u16)~get16(&m)); else put8(&m, (u8)~get8(&m)); break;
            case 3: if (w) put16(&m, (u16)f_neg(get16(&m), 16)); else put8(&m, (u8)f_neg(get8(&m), 8)); break;
            case 4: if (w) f_mul16(get16(&m)); else f_mul8(get8(&m)); break;
            case 5: if (w) f_imul16((s16)get16(&m)); else f_imul8((s8)get8(&m)); break;
            case 6: if (w) f_div16(get16(&m)); else f_div8(get8(&m)); break;
            case 7: if (w) f_idiv16((s16)get16(&m)); else f_idiv8((s8)get8(&m)); break;
            }
            break; }
        case 0xf8: set_flags((u16)(get_flags() & ~1)); break;
        case 0xf9: set_flags((u16)(get_flags() | 1)); break;
        case 0xfa: R.iflag = 0; break;
        case 0xfb: R.iflag = 1; break;
        case 0xfc: R.df = 0; break;
        case 0xfd: R.df = 1; break;
        case 0xfe: { ModRM m; decode(&m);
            if (m.reg == 0) put8(&m, (u8)f_inc(get8(&m), 8)); else if (m.reg == 1) put8(&m, (u8)f_dec(get8(&m), 8));
            else fatal("interp: fe /%d at %04x:%04x", m.reg, cs_, start);
            break; }
        case 0xff: { ModRM m; decode(&m);
            switch (m.reg) {
            case 0: put16(&m, (u16)f_inc(get16(&m), 16)); break;
            case 1: put16(&m, (u16)f_dec(get16(&m), 16)); break;
            case 2: { u16 t = get16(&m); push16(ip); int r = call_to(cs_, t, entry_sp); if (r == 2) goto done_native; if (!r) ip = t; break; }
            case 3: { u16 o = rw(m.seg, m.off), s = rw(m.seg, (u16)(m.off + 2)); push16(cs_); push16(ip);
                int r = call_to(s, o, entry_sp); if (r == 2) goto done_native; if (!r) { cs_ = s; ip = o; } break; }
            case 4: { u16 t = get16(&m); if (jump_to(cs_, t, entry_ss, entry_sp)) goto done_native; break; }
            case 5: { u16 o = rw(m.seg, m.off), s = rw(m.seg, (u16)(m.off + 2));
                if (jump_to(s, o, entry_ss, entry_sp)) goto done_native; break; }
            case 6: push16(get16(&m)); break;
            default: fatal("interp: ff /7 at %04x:%04x", cs_, start);
            }
            break; }
        default:
            fatal("interp: opcode %02x at %04x:%04x", op, cs_, start);
        }
        /* a near/far call or jump target may be native, checked at the transfer; a plain
         * fall-through into native code is not possible to detect cheaply and is rare */
        (void)start;
        if (one) break;
    }
done:
done_native:
    if (log && (SS != entry_ss)) fprintf(stderr, "interp: left %04x:%04x at %04x:%04x on another stack %04x:%04x\n", seg, off, cs_, ip, SS, SP);
    ip = save_ip; cs_ = save_cs;
}
