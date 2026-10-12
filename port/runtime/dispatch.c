/* dispatch.c - calls and jumps to computed addresses */
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "dispatch.h"
#include "hle.h"

void interp_run(u16 seg, u16 off);
int driver_retsite(u16 seg, u16 off);

static void (*find_func(u32 lin))(void)
{
    int lo = 0, hi = FUNC_COUNT - 1, first = -1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (FUNC_TABLE[mid].lin < lin) lo = mid + 1;
        else { if (FUNC_TABLE[mid].lin == lin) first = mid; hi = mid - 1; }
    }
    if (first < 0) return 0;
    /* overlay sections share addresses: take the function of a loaded section */
    void (*any)(void) = 0;
    int n = 0;
    for (int k = first; k < FUNC_COUNT && FUNC_TABLE[k].lin == lin; k++) {
        int s = FUNC_TABLE[k].sect;
        if (s < 0 || ovl_loaded[s]) return FUNC_TABLE[k].f;
        if (!any) any = FUNC_TABLE[k].f;
        n++;
    }
    if (n > 1) {            /* none of the sections with a function here is present: a guess (should not happen) */
        static u32 seen[64]; static int nseen;
        int k2 = 0; while (k2 < nseen && seen[k2] != lin) k2++;
        if (k2 == nseen && nseen < 64) {
            seen[nseen++] = lin;
            fprintf(stderr, "dispatch: %05x is in %d overlay sections, none of them present (took s%03d)\n", lin, n, FUNC_TABLE[first].sect);
        }
    }
    return any;
}

static int is_retsite(u32 lin)
{
    int lo = 0, hi = RET_COUNT - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (RET_TABLE[mid].lin == lin) return 1;
        if (RET_TABLE[mid].lin < lin) lo = mid + 1; else hi = mid - 1;
    }
    return 0;
}

static const VecEntry *find_vector(u32 lin)
{
    if (lin < 0x9c00 + 0x17e3 || lin >= 0x9c00 + 0x3550) return 0;
    u32 o = lin - 0x9c00;
    int lo = 0, hi = VEC_COUNT - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (VEC_TABLE[mid].off == o) return &VEC_TABLE[mid];
        if (VEC_TABLE[mid].off < o) lo = mid + 1; else hi = mid - 1;
    }
    return 0;
}

/* code generated at run time - MSC int86() builds "int nn; retf" on the stack */
static int tiny_thunk(u16 seg, u16 off)
{
    if (rb(seg, off) != 0xcd) return 0;
    u8 n = rb(seg, (u16)(off + 2));
    return n == 0xcb || n == 0xca || n == 0xc3 || n == 0x44;
}
static void tiny_interp(u16 seg, u16 off)
{
    for (int steps = 0; steps < 8; steps++) {
        u8 b = rb(seg, off);
        switch (b) {
        case 0xcd: hle_int(rb(seg, (u16)(off + 1))); off += 2; break;
        case 0x44: SP++; off++; break;
        case 0x4c: SP--; off++; break;
        case 0xcb: SP = (u16)(SP + 4); return;
        case 0xc3: SP = (u16)(SP + 2); return;
        case 0xca: SP = (u16)(SP + 4 + rw(seg, (u16)(off + 1))); return;
        default: fatal("tiny_interp: opcode %02x at %04x:%04x", b, seg, off);
        }
    }
    fatal("tiny_interp: too long at %04x:%04x", seg, off);
}

static int in_program(u16 seg, u16 off) { return LIN(seg, off) >= ((u32)LOADSEG << 4); }

/* is there native code (recompiled or emulated service) at seg:off? */
#ifndef DL_RTSPLIT
extern const unsigned INTERP_ALWAYS_TABLE[]; extern const int INTERP_ALWAYS_COUNT;
#endif
static int interp_always(u32 lin)
{
    for (int i = 0; i < INTERP_ALWAYS_COUNT; i++) if (INTERP_ALWAYS_TABLE[i] == lin) return 1;
    return 0;
}

int native_target(u16 seg, u16 off)
{
    if (seg == HLE_SEG || seg == EMS_SEG) return 1;
    if (driver_find(seg, off)) return 1;
    if (tiny_thunk(seg, off)) return 1;
    if (!in_program(seg, off)) return 0;
    u32 lin = LIN(seg, off) - ((u32)LOADSEG << 4);
    if (interp_always(lin)) return 0;        /* stays in the calling interpreter */
    return find_vector(lin) || find_func(lin);
}

static void go(u16 seg, u16 off, int is_call);
static void go(u16 seg, u16 off, int is_call)
{
    static int trace = -1;
    if (trace < 0) trace = getenv("DL_TRACE_DISPATCH") != 0;
    if (trace) {
        fprintf(stderr, "dispatch %s %04x:%04x SP=%04x\n", is_call ? "call" : "jmp", seg, off, SP);
        static int ss = -2; if (ss == -2) { const char *e = getenv("DL_STACK_ON_SEG"); ss = e ? (int)strtol(e, 0, 16) : -1; }
        if (ss == seg) ftrace_dump();
    }
    if (seg == HLE_SEG || seg == EMS_SEG) { tail_pending = 0; hle_stub(seg, off); return; }
    void (*df)(void) = driver_find(seg, off);
    if (df) { df(); return; }
    if (!is_call && driver_retsite(seg, off)) { void ftrace_check_retsite(u16, u16); ftrace_check_retsite(seg, off); tail_pending = 0; return; }
    if (in_program(seg, off)) {
        u32 lin = LIN(seg, off) - ((u32)LOADSEG << 4);
        const VecEntry *v = find_vector(lin);
        if (v) {
            if (!is_call) { ovl_enter(v->sect); v->f(); return; }
            OvlSet o; ovl_save(&o); ovl_enter(v->sect); v->f(); ovl_restore(&o);     /* as RTLink (ovl.c) */
            return;
        }
        void (*f)(void) = find_func(lin);
        if (f) { f(); return; }
        if (!is_call && is_retsite(lin)) {
            { void ftrace_check_retsite(u16, u16); ftrace_check_retsite(seg, off); }
            tail_pending = 0;
            /* a jump to the instruction after a call: code that popped its own return
             * address and jumps back through it (MSC __setargv, ...). Returning from this
             * C function resumes that caller right there. */
            return;
        }
    }
    if (tiny_thunk(seg, off)) { tail_pending = 0; tiny_interp(seg, off); return; }
    interp_run(seg, off);
}

void native_go(u16 seg, u16 off) { go(seg, off, 1); }

/* is seg:off the instruction after a call in recompiled code (main program or driver)? */
int any_retsite(u16 seg, u16 off)
{
    if (driver_retsite(seg, off)) return 1;
    if (!in_program(seg, off)) return 0;
    return is_retsite(LIN(seg, off) - ((u32)LOADSEG << 4));
}
void dispatch_far(u16 seg, u16 off) { go(seg, off, 1); }
void dispatch_near(u16 cs, u16 off) { go(cs, off, 1); }
void dispatch_jmp_far(u16 seg, u16 off) { go(seg, off, 0); }
void dispatch_jmp_near(u16 cs, u16 off) { go(cs, off, 0); }

int unw_active; u16 unw_sp, unw_ip, unw_cs; int unw_far;
int tail_pending; u16 tail_sp;

void ret_mismatch(u16 my_sp, int kind, u16 n, u16 cs)
{
    u16 slot = SP;
    u16 ip = pop16();
    u16 seg = kind ? pop16() : cs;
    if (kind == 2) set_flags(pop16());
    SP = (u16)(SP + n);
    if ((u16)(slot - my_sp) < 0x8000) {
        /* return from an outer frame */
        unw_active = 1; unw_sp = slot; unw_ip = ip; unw_cs = seg; unw_far = kind != 0;
        static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_UNW") != 0;
        if (tr) {
            fprintf(stderr, "unwind: ret from slot %04x (frame %04x) to %04x:%04x, BP %04x\n", slot, my_sp, seg, ip, BP); ftrace_dump();
#ifdef FTRACE
            if ((u16)(slot - my_sp) > 0x2000) {      /* far outside the frame: the stack pointer is lost */
                extern u32 ret_ring[64]; extern u16 ret_ring_sp[64]; extern int ret_ring_i;
                fprintf(stderr, "  SP LOST; last returns (function: SP after):");
                for (int k = 0; k < 64; k++) { int j = (ret_ring_i + k) & 63; if (ret_ring[j]) fprintf(stderr, "%s%x:%04x", k % 6 ? "  " : "\n    ", ret_ring[j], ret_ring_sp[j]); }
                fprintf(stderr, "\n");
            }
#endif
        }
        return;
    }
    /* "push address; ret": a computed jump */
    go(seg, ip, 0);
}
