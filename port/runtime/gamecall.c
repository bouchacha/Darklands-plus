/* gamecall.c - calling the game's own functions from native code (the new interface uses
 * the game's inventory routines so equipment, weights and stacks stay consistent).
 * The call runs on the game's stack below its current top; afterwards the machine state and
 * the overlay area are put back exactly as they were, so the paused game continues unaware. */
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#include "dispatch.h"
#include "game.h"

void ovl_area(u32 *lin_start, u32 *lin_end);
extern int in_game_call;

static void (*func_in(int sect, u32 lin))(void)
{
    for (int k = 0; k < FUNC_COUNT; k++) if (FUNC_TABLE[k].lin == lin && FUNC_TABLE[k].sect == sect) return FUNC_TABLE[k].f;
    return 0;
}

/* far cdecl call of sect seg:off with args (pushed right to left); returns AX, or -1 if
 * there is no such function */
int game_call_far(int sect, u16 seg, u16 off, int nargs, const u16 *args)
{
    void (*f)(void) = func_in(sect, (u32)seg * 16 + off);
    if (!f) return -1;
    u32 a0, a1;
    ovl_area(&a0, &a1);
    a0 += (u32)LOADSEG << 4; a1 += (u32)LOADSEG << 4;
    u8 *snap = malloc(a1 - a0);
    int loaded[256];
    memcpy(snap, M + a0, a1 - a0);
    memcpy(loaded, ovl_loaded, sizeof loaded);
    CPU save = R;
    int save_tail = tail_pending, save_unw = unw_active;
    in_game_call++;

    tail_pending = 0; unw_active = 0;
    DS = ES = G_DGROUP;
    SP = (u16)(SP - 64);                         /* leave the interrupted code's red zone alone */
    for (int i = nargs - 1; i >= 0; i--) push16(args[i]);
    push16(HLE_SEG); push16(0xfff0);             /* far return address (never jumped to) */
    if (sect >= 0) ovl_enter(sect);
    f();
    u16 ax = AX;

    in_game_call--;
    R = save;
    tail_pending = save_tail; unw_active = save_unw;
    memcpy(M + a0, snap, a1 - a0);
    memcpy(ovl_loaded, loaded, sizeof loaded);
    free(snap);
    return ax;
}

int game_mode(void)
{
    if (ovl_loaded[14] || ovl_loaded[2] || ovl_loaded[4]) return GM_BATTLE;
    if (!ovl_loaded[29]) return GM_OTHER;
    if (ovl_loaded[43]) return GM_CARD;
    if (ovl_loaded[30]) return GM_MAP;
    return GM_SCREEN;
}
