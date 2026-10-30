/* hooks.c - see hooks.h */
#include <stdio.h>
#include <stdlib.h>
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

/* hooks chain: several mods may hook one function. The hook installed last runs first; its
 * hook_call_original() runs the one installed before it, and the last of them the game's own
 * function. hook_table[i] is the first of the chain (the generated code tests it). */
#define CHAIN_MAX 6
static struct { void (*fn[CHAIN_MAX])(void); int n; } *chains;
static int nchains;
int hook_depth;

static int chain_ok(int i)
{
    if (!chains) {
        for (int k = 0; k < HOOK_COUNT; k++) if (HOOK_POINTS[k].idx >= nchains) nchains = HOOK_POINTS[k].idx + 1;
        chains = calloc((size_t)nchains + 1, sizeof *chains);
    }
    return i >= 0 && i < nchains;
}

int hook_point(int sect, unsigned seg, unsigned off) { return find(sect, seg, off); }   /* (not in hooks.h: a translator file) */

int hook_install(int sect, unsigned seg, unsigned off, void (*fn)(void))
{
    int i = find(sect, seg, off);
    if (i < 0 || !chain_ok(i)) { fprintf(stderr, "hook: no function at s%d %04x:%04x\n", sect, seg, off); return 0; }
    for (int k = 0; k < chains[i].n; k++) if (chains[i].fn[k] == fn) return 1;      /* (already in) */
    if (chains[i].n >= CHAIN_MAX) { fprintf(stderr, "hook: s%d %04x:%04x has too many hooks\n", sect, seg, off); return 0; }
    for (int k = chains[i].n; k > 0; k--) chains[i].fn[k] = chains[i].fn[k - 1];
    chains[i].fn[0] = fn;
    chains[i].n++;
    hook_table[i] = chains[i].fn[0];
    return 1;
}

void hook_remove(int sect, unsigned seg, unsigned off, void (*fn)(void))
{
    int i = find(sect, seg, off);
    if (i < 0 || !chain_ok(i)) return;
    for (int k = 0; k < chains[i].n; k++)
        if (chains[i].fn[k] == fn) {
            for (int j = k; j + 1 < chains[i].n; j++) chains[i].fn[j] = chains[i].fn[j + 1];
            chains[i].n--;
            break;
        }
    hook_table[i] = chains[i].n ? chains[i].fn[0] : 0;
}

/* the function each hook index belongs to, for hook_call_original (looked up once per index:
 * the two searches through ten thousand entries on every call of a hooked function that the
 * game calls in its idle loops, the card menu's above all, cost most of a CPU core) */
static void (*func_of(int idx))(void)
{
    static void (**cache)(void); static unsigned char *known; static int n;
    if (!cache) {
        for (int i = 0; i < HOOK_COUNT; i++) if (HOOK_POINTS[i].idx >= n) n = HOOK_POINTS[i].idx + 1;
        cache = calloc((size_t)n + 1, sizeof *cache); known = calloc((size_t)n + 1, 1);
    }
    if (idx >= 0 && idx < n && known[idx]) return cache[idx];
    unsigned lin = 0; int sect = -9;
    void (*f)(void) = 0;
    for (int i = 0; i < HOOK_COUNT; i++) if (HOOK_POINTS[i].idx == idx) { lin = HOOK_POINTS[i].lin; sect = HOOK_POINTS[i].sect; break; }
    for (int k = 0; k < FUNC_COUNT; k++) if (FUNC_TABLE[k].lin == lin && FUNC_TABLE[k].sect == sect) { f = FUNC_TABLE[k].f; break; }
    if (idx >= 0 && idx < n) { cache[idx] = f; known[idx] = 1; }
    return f;
}

void hook_run(int idx)
{
    int prev = hook_current, pd = hook_depth;
    hook_current = idx; hook_depth = 0;
    hook_table[idx]();
    hook_current = prev; hook_depth = pd;
}

/* the next hook of the chain, or the game's function */
static void unw_note(int idx, int d)            /* (debugging, DL_UNW_LOG: the hooks a game unwinding - a load, Quit - passes back through) */
{
    static int on = -1; if (on < 0) on = getenv("DL_UNW_LOG") != 0;
    if (!on || !unw_active) return;
    unsigned lin = 0; int sect = -9;
    for (int i = 0; i < HOOK_COUNT; i++) if (HOOK_POINTS[i].idx == idx) { lin = HOOK_POINTS[i].lin; sect = HOOK_POINTS[i].sect; break; }
    void (*h)(void) = chain_ok(idx) && d < chains[idx].n ? chains[idx].fn[d] : 0;
    fprintf(stderr, "unwind: back through the hook of s%d %05x (chain %d, hook %p)\n", sect, lin, d, (void *)h);
}
void hook_call_original(void)
{
    int idx = hook_current, d = hook_depth;
    if (chain_ok(idx) && d + 1 < chains[idx].n) {
        hook_depth = d + 1;
        chains[idx].fn[d + 1]();
        hook_current = idx; hook_depth = d;
        unw_note(idx, d);
        return;
    }
    void (*f)(void) = func_of(idx);
    if (!f) fatal("hook_call_original: no function");
    hook_bypass = idx;
    f();
    hook_current = idx; hook_depth = d;
    unw_note(idx, d);
}

void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
void hook_return_near(unsigned n) { SP = (u16)(SP + 2 + n); }

