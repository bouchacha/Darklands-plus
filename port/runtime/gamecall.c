/* gamecall.c - calling the game's own functions from native code (the new interface uses
 * the game's inventory routines so equipment, weights and stacks stay consistent).
 * The call runs on the game's stack below its current top; afterwards the machine state and
 * the overlay area are put back exactly as they were, so the paused game continues unaware. */
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include "cpu.h"
void ovl_changed(void);
#include "hle.h"
#include "dispatch.h"
#include "game.h"
#include "hooks.h"

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
    memcpy(ovl_loaded, loaded, sizeof loaded); ovl_changed();
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

/* ---- side-effect-free evaluation: run game code, read the result, then put the whole
 * machine back (memory, registers, resident overlays). Used to ask the card scripts for
 * their chances of success without disturbing the game (random numbers included). */
struct EvalSnap { u8 *mem; CPU r; int loaded[256]; int tail, unw; void *ems, *dos, *bios; };
void *bios_snapshot(void); void bios_restore(void *); void bios_snapshot_free(void *);
void *ems_snapshot(void); void ems_restore(void *); void ems_snapshot_free(void *);
void *dos_snapshot(void); void dos_restore(void *); void dos_snapshot_free(void *);

void *eval_snapshot(void)
{
    struct EvalSnap *s = malloc(sizeof *s);
    s->mem = malloc(MEM_SIZE);
    memcpy(s->mem, M, MEM_SIZE);
    s->r = R;
    memcpy(s->loaded, ovl_loaded, sizeof s->loaded);
    s->tail = tail_pending; s->unw = unw_active;
    s->ems = ems_snapshot();             /* expanded memory (its mapping and its pages) and the open files too */
    s->dos = dos_snapshot();
    s->bios = bios_snapshot();           /* the mouse driver's clicks, the palette, the timer */
    return s;
}

void eval_restore(void *p)
{
    struct EvalSnap *s = p;
    memcpy(M, s->mem, MEM_SIZE);
    R = s->r;
    memcpy(ovl_loaded, s->loaded, sizeof s->loaded); ovl_changed();
    tail_pending = s->tail; unw_active = s->unw;
    ems_restore(s->ems);
    dos_restore(s->dos);
    bios_restore(s->bios);
}

void eval_free(void *p)
{
    struct EvalSnap *s = p;
    ems_snapshot_free(s->ems); dos_snapshot_free(s->dos); bios_snapshot_free(s->bios);
    free(s->mem); free(s);
}

/* far call of the run-time address seg:off with cdecl word args; returns DX:AX. The caller
 * restores the machine afterwards (eval_restore). The free stack the callee will use is
 * filled with `fill` first: calling twice with different fills shows whether a result
 * depends on uninitialised stack (some card handlers return garbage). */
static jmp_buf *call_jb;
static u32 call_deadline;
int game_call_timed_out;          /* the last harness call was abandoned */
u32 plat_ticks_ms(void);

/* dl_poll() during an evaluation: game code that waits (for a key, say) would never return
 * while the game is held, so after a quarter of a second the evaluation is abandoned */
void call_watchdog(void)
{
    if (call_jb && plat_ticks_ms() > call_deadline) longjmp(*call_jb, 1);
}

static u32 harness(void (*f)(void), u16 seg, u16 off, int nargs, const u16 *args, int fill)
{
    jmp_buf jb, *prev = call_jb;
    int hc = hook_current, hb = hook_bypass, hd = hook_depth, depth = in_game_call;
    if (setjmp(jb)) {
        call_jb = prev; hook_current = hc; hook_bypass = hb; hook_depth = hd; in_game_call = depth;
        game_call_timed_out = 1;
        fprintf(stderr, "game_call_ptr %04x:%04x: no return after 100 ms, abandoned\n", seg, off);
        return 0xffffffffu;
    }
    call_jb = &jb; call_deadline = plat_ticks_ms() + 100;
    game_call_timed_out = 0;
    in_game_call++;
    tail_pending = 0; unw_active = 0;
    DS = ES = G_DGROUP;
    SP = (u16)(SP - 64);
    if (SP > 0x900) for (u16 o = (u16)(SP - 0x800); o < SP; o++) M[LIN(SS, o)] = (u8)fill;
    for (int i = nargs - 1; i >= 0; i--) push16(args[i]);
    push16(HLE_SEG); push16(0xfff0);
    if (f) f(); else dispatch_far(seg, off);
    u32 r = ((u32)DX << 16) | AX;
    in_game_call--;
    call_jb = prev;
    return r;
}

u32 game_call_ptr(u16 seg, u16 off, int nargs, const u16 *args, int fill)
{
    return harness(0, seg, off, nargs, args, fill);
}

/* the same for a function given by its link address (section sect, which must be resident) */
u32 game_call_link(int sect, u16 seg, u16 off, int nargs, const u16 *args)
{
    void (*f)(void) = func_in(sect, (u32)seg * 16 + off);
    if (!f) return 0xffffffffu;
    return harness(f, seg, off, nargs, args, 0);
}

/* ---- fast calls for native code that runs inside the game (hooks): the recompiled
 * function is called directly on the game's stack; only the registers are put back. For
 * functions whose memory effects are wanted or harmless (collision tests ...). */
void (*game_func(int sect, u16 seg, u16 off))(void) { return func_in(sect, (u32)seg * 16 + off); }

/* live: the game keeps running normally inside (input, screen, sound), for long calls */
static u32 call_direct(void (*f)(void), int nargs, const u16 *args, int live)
{
    CPU save = R;
    int tp = tail_pending, ua = unw_active;
    tail_pending = 0; unw_active = 0;
    if (!live) in_game_call++;
    DS = ES = G_DGROUP;
    SP = (u16)(SP - 64);
    for (int i = nargs - 1; i >= 0; i--) push16(args[i]);
    push16(HLE_SEG); push16(0xfff0);
    OvlSet o; ovl_save(&o);
    f();
    ovl_restore(&o);                    /* sections the call displaced come back, as after an RTLink call */
    u32 r = ((u32)DX << 16) | AX;
    if (!live) in_game_call--;
    R = save; tail_pending = tp; unw_active = ua;
    return r;
}
u32 game_call_native(void (*f)(void), int nargs, const u16 *args) { return call_direct(f, nargs, args, 0); }
u32 game_call_live(void (*f)(void), int nargs, const u16 *args) { return call_direct(f, nargs, args, 1); }
