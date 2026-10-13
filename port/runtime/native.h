/* native.h - native functions the game can call through a far pointer.
 *
 * Mods give the game addresses it calls like its own far functions: a card in the card table,
 * an option's chance handler in opt_fn[]. Such an address is NATIVE_SEG:off; a far call there
 * runs the registered C function with the machine state of the call (the far return address at
 * SS:SP, the arguments above it) and then returns as retf would. The function leaves its result
 * in AX (and DX). */
#ifndef DL_NATIVE_H
#define DL_NATIVE_H
#include "cpu.h"
#define NATIVE_SEG 0xF200
typedef void (*NativeFn)(int arg);
u16 native_register(NativeFn fn, int arg);       /* the offset of a new entry (the same fn/arg: the same one) */
u32 native_far_ptr(NativeFn fn, int arg);        /* NATIVE_SEG << 16 | offset, as a far pointer in memory */
void native_call(u16 off);                       /* dispatch.c: a far call landed at NATIVE_SEG:off */
#endif
