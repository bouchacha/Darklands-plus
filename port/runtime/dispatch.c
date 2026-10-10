/* dispatch.c - calls and jumps to computed addresses */
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "dispatch.h"
#include "hle.h"

void interp_run(u16 seg, u16 off);

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
    for (int k = first; k < FUNC_COUNT && FUNC_TABLE[k].lin == lin; k++) {
        int s = FUNC_TABLE[k].sect;
        if (s < 0 || ovl_loaded[s]) return FUNC_TABLE[k].f;
        if (!any) any = FUNC_TABLE[k].f;
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
int native_target(u16 seg, u16 off)
{
    if (seg == HLE_SEG || seg == EMS_SEG) return 1;
    if (driver_find(seg, off)) return 1;
    if (!in_program(seg, off)) return 0;
    u32 lin = LIN(seg, off) - ((u32)LOADSEG << 4);
    return find_vector(lin) || find_func(lin);
}

static void go(u16 seg, u16 off, int is_call)
{
    static int trace = -1;
    if (trace < 0) trace = getenv("DL_TRACE_DISPATCH") != 0;
    if (trace) {
        fprintf(stderr, "dispatch %s %04x:%04x SP=%04x\n", is_call ? "call" : "jmp", seg, off, SP);
        static int ss = -2; if (ss == -2) { const char *e = getenv("DL_STACK_ON_SEG"); ss = e ? (int)strtol(e, 0, 16) : -1; }
        if (ss == seg) ftrace_dump();
    }
    if (seg == HLE_SEG || seg == EMS_SEG) { hle_stub(seg, off); return; }
    void (*df)(void) = driver_find(seg, off);
    if (df) { df(); return; }
    if (in_program(seg, off)) {
        u32 lin = LIN(seg, off) - ((u32)LOADSEG << 4);
        const VecEntry *v = find_vector(lin);
        if (v) { ovl_enter(v->sect); v->f(); return; }
        void (*f)(void) = find_func(lin);
        if (f) { f(); return; }
        if (!is_call && is_retsite(lin)) {
            /* a jump to the instruction after a call: code that popped its own return
             * address and jumps back through it (MSC __setargv, ...). Returning from this
             * C function resumes that caller right there. */
            return;
        }
    }
    if (tiny_thunk(seg, off)) { tiny_interp(seg, off); return; }
    interp_run(seg, off);
}

void native_go(u16 seg, u16 off) { go(seg, off, 1); }
void dispatch_far(u16 seg, u16 off) { go(seg, off, 1); }
void dispatch_near(u16 cs, u16 off) { go(cs, off, 1); }
void dispatch_jmp_far(u16 seg, u16 off) { go(seg, off, 0); }
void dispatch_jmp_near(u16 cs, u16 off) { go(cs, off, 0); }
