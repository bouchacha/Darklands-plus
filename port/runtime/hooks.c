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
    /* ns_unwinding (mods/nscreen.c) tells a hook that a game call it made was cut short by the game
       unwinding (a load, Quit), so it must return at once and leave the stack to the unwinding. Once that
       unwinding has landed it means nothing, but nothing cleared it until the next ns_call: after a save
       loaded from the menu (a native screen's game call unwinds) a hook that made no game call of its own
       read it, returned without returning (levelwalk's on combat_input, every tick of the battle), the
       stack crept down 4 bytes a tick into the game's data, and the battle stopped ("interp: opcode d9 at
       6675:426f", build 1130). A hook is only ever entered with no unwinding under way: clear it here. */
    { extern int ns_unwinding; if (!unw_active) ns_unwinding = 0; }
    u16 sp0 = SP, bp0 = BP;
    hook_table[idx]();
    hook_current = prev; hook_depth = pd;
    /* what any game function leaves as it was, a hook too: it returns (SP above where it was entered: the
       return address popped, a near or far one, and the arguments of a function that pops its own) and BP is
       the caller's. Said once a hook point, in the log (release_check.py looks for it). */
    if (!unw_active && ((u16)(SP - sp0) < 2 || (u16)(SP - sp0) > 0x40 || BP != bp0)) {
        static unsigned char said[8192];
        if (idx >= 0 && idx < (int)sizeof said * 8 && !(said[idx >> 3] & (1 << (idx & 7)))) {
            said[idx >> 3] |= (u8)(1 << (idx & 7));
            unsigned lin = 0; int sect = -9;
            for (int i = 0; i < HOOK_COUNT; i++) if (HOOK_POINTS[i].idx == idx) { lin = HOOK_POINTS[i].lin; sect = HOOK_POINTS[i].sect; break; }
            fprintf(stderr, "hook: the hook of s%d %05x left the stack wrong (SP %+d, BP %04x, was %04x)\n", sect, lin, (s16)(u16)(SP - sp0), BP, bp0);
        }
    }
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

