/* hooks.c - see hooks.h */
#include <stdio.h>
#include "cpu.h"
#include "hooks.h"
#include "dispatch.h"

int hook_bypass = -1, hook_current = -1;


static int find(int sect, unsigned seg, unsigned off)
{
    unsigned lin = seg * 16 + off;
    for (int i = 0; i < HOOK_COUNT; i++)
        if (HOOK_POINTS[i].lin == lin && HOOK_POINTS[i].sect == sect) return HOOK_POINTS[i].idx;
    return -1;
}

int hook_install(int sect, unsigned seg, unsigned off, void (*fn)(void))
{
    int i = find(sect, seg, off);
    if (i < 0) { fprintf(stderr, "hook: no function at s%d %04x:%04x\n", sect, seg, off); return 0; }
    hook_table[i] = fn;
    return 1;
}

void hook_remove(int sect, unsigned seg, unsigned off)
{
    int i = find(sect, seg, off);
    if (i >= 0) hook_table[i] = 0;
}

/* the function each hook index belongs to, for hook_call_original */
static void (*func_of(int idx))(void)
{
    unsigned lin = 0; int sect = -9;
    for (int i = 0; i < HOOK_COUNT; i++) if (HOOK_POINTS[i].idx == idx) { lin = HOOK_POINTS[i].lin; sect = HOOK_POINTS[i].sect; break; }
    for (int k = 0; k < FUNC_COUNT; k++) if (FUNC_TABLE[k].lin == lin && FUNC_TABLE[k].sect == sect) return FUNC_TABLE[k].f;
    return 0;
}

void hook_run(int idx)
{
    int prev = hook_current;
    hook_current = idx;
    hook_table[idx]();
    hook_current = prev;
}

void hook_call_original(void)
{
    void (*f)(void) = func_of(hook_current);
    if (!f) fatal("hook_call_original: no function");
    int prev = hook_current;
    hook_bypass = hook_current;
    f();
    hook_current = prev;
}

void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
void hook_return_near(unsigned n) { SP = (u16)(SP + 2 + n); }

