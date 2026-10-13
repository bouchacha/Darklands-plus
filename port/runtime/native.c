/* native.c - native functions the game calls through far pointers (native.h) */
#include <stdio.h>
#include "cpu.h"
#include "native.h"

#define NATIVE_MAX 512
static struct { NativeFn fn; int arg; } tab[NATIVE_MAX];
static int ntab;

u16 native_register(NativeFn fn, int arg)
{
    for (int i = 0; i < ntab; i++) if (tab[i].fn == fn && tab[i].arg == arg) return (u16)(i * 2);
    if (ntab >= NATIVE_MAX) fatal("native_register: table full");
    tab[ntab].fn = fn; tab[ntab].arg = arg;
    u16 off = (u16)(ntab * 2);
    M[NATIVE_SEG * 16 + off] = 0xcb;              /* retf, for code that peeks */
    ntab++;
    return off;
}

u32 native_far_ptr(NativeFn fn, int arg) { return (u32)NATIVE_SEG << 16 | native_register(fn, arg); }

void native_call(u16 off)
{
    int i = off / 2;
    if ((off & 1) || i >= ntab) fatal("native_call: no function at %04x:%04x", NATIVE_SEG, off);
    u16 sp0 = SP;
    tail_pending = 0;
    tab[i].fn(tab[i].arg);
    if (unw_active) {                 /* a longjmp or a return from an outer frame passes through */
        if (unw_sp == sp0) unw_active = 0;
        return;
    }
    SP = (u16)(sp0 + 4);              /* retf */
}
