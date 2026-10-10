/* overrides.c - hand-written replacements for the few original routines whose tricks the
 * static recompiler cannot express (listed in port/recomp/overrides.txt). */
#include "cpu.h"
#include "hle.h"
#include "funcs.h"

/* root 05f8:0012 - first call through the game's int 67h far pointer. Detects the EMS
 * driver, stores the real int 67h vector in the pointer and then moves its caller's return
 * address back 8 bytes so that "mov ah,nn / pushf / call far [ptr]" runs again. Here: do
 * the same, then perform that repeated call directly (the pushf/call frame is already on
 * the stack). */
void F_root_05f92(void)
{
    u16 cs5 = (u16)(LOADSEG + 0x05f8);
    push16(cs5); push16(0x0017); F_root_05fe4();
    if (AX) {
        u16 ds = DS; AH = 9; DS = cs5; DX = 70; hle_int(0x21); DS = ds;
        SP = (u16)(SP + 4); set_flags(pop16());
        return;
    }
    u16 off = rw(0, 0x67 * 4), seg = rw(0, 0x67 * 4 + 2);
    ww(cs5, 14, off); ww(cs5, 16, seg);
    u16 ip = rw(SS, SP), cs = rw(SS, (u16)(SP + 2));
    u16 at = (u16)(ip - 8);
    if (rb(cs, at) != 0xb4 || rb(cs, (u16)(at + 2)) != 0x9c)
        fatal("EMS init: unexpected caller code at %04x:%04x", cs, at);
    AH = rb(cs, (u16)(at + 1));
    dispatch_jmp_far(seg, off);
}

/* root 047c:048a - dispatcher of the graphics driver thunks "call 048a; dd target".
 * It pops the thunk's return address (= the address of the far target), patches it into
 * the operand of its own jmp/call far cs:[...] and transfers there. While the mouse pointer
 * is shown (byte [6a81]) the driver is called and the pointer is redrawn afterwards. */
void F_root_04c4a(void)
{
    u16 cs = (u16)(LOADSEG + 0x047c);
    if (rb(DS, 27265) == 0) {
        u16 p = pop16();
        ww(cs, 1179, p);
        dispatch_jmp_far(rw(cs, (u16)(p + 2)), rw(cs, p));
        return;
    }
    wb(DS, 27264, 1);
    u16 p = pop16();
    ww(cs, 1204, p);
    ww(DS, 27266, pop16());
    ww(DS, 27268, pop16());
    push16(cs); push16(0x04b6);
    dispatch_far(rw(cs, (u16)(p + 2)), rw(cs, p));
    for (;;) {
        wb(DS, 27264, (u8)f_dec(rb(DS, 27264), 8));
        if (get_zf()) break;
        push16(0x04bf); F_root_05b14();
    }
    /* jmp far [6a82]: back to the thunk's caller, whose return address was popped above */
    dispatch_jmp_far(rw(DS, 27268), rw(DS, 27266));
}
