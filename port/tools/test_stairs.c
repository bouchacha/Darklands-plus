/* test_stairs.c - synthetic stair records exercise the real transition and query hooks.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o build/test_stairs port/tools/test_stairs.c && build/test_stairs
 * The game calls are stubbed. Copied-game tests separately exercise the real placement,
 * floor loading and exploration group helper with the fix enabled and disabled. */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../mods/stairsfix.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active, ns_unwinding, dos_no_writes;
static u8 maps[2][0x33fc];
static int enabled, original_mode, original_calls, placing, reject_all, notices, canceled, unwind_transition;
enum { RNG = 0x5000 };

int fix_is_on(const char *id) { assert(!strcmp(id, "stairsreturn")); return enabled && !original_mode; }
void ui_notice(const char *message) { assert(strstr(message, "stairs")); notices++; }
void hook_return_far(unsigned count) { SP = (u16)(SP + 4 + count); }
int hook_install(int sect, unsigned seg, unsigned off, void (*fn)(void)) { return 1; }
typedef struct { CPU cpu; u8 memory[MEM_SIZE]; } Snapshot;
void *eval_snapshot(void)
{
    Snapshot *copy = malloc(sizeof *copy); assert(copy);
    copy->cpu = R; memcpy(copy->memory, M, MEM_SIZE); return copy;
}
void eval_restore(void *p) { Snapshot *copy = p; R = copy->cpu; memcpy(M, copy->memory, MEM_SIZE); }
void eval_free(void *p) { free(p); }

static void show(int floor)
{
    assert(floor >= 0 && floor < 2);
    memcpy(M + LIN(G_FAR20A5, 0), maps[floor], sizeof maps[floor]);
    g_wb(GV_floor_shown, (u8)floor);
}
static int place(int who, int x, int y)
{
    g_ww(RNG, (u16)(g_rw(RNG) + 1));
    if (reject_all || x != 116 || y != 308) return 0;
    g_ww(CBF(who, CB_X), 118); g_ww(CBF(who, CB_Y), 310);
    /* The original search can clear furniture after its first bounded search. */
    wb(G_FAR20A5, 4 * 100, rb(G_FAR20A5, 4 * 100) & 0x0f);
    return 1;
}
int game_call_far(int sect, u16 seg, u16 off, int n, const u16 *args)
{
    assert(dos_no_writes);
    if (sect == 24 && off == 0x059a) { show(args[0]); return 1; }
    assert(sect == 24 && off == 0x08f2 && n == 2);
    return place(args[0], (s16)rw(G_DGROUP, args[1] + 8), (s16)rw(G_DGROUP, args[1] + 10));
}
int ns_call(int sect, u16 seg, u16 off, int n, ...)
{
    int args[4]; va_list ap; va_start(ap, n);
    for (int i = 0; i < n; i++) args[i] = va_arg(ap, int);
    va_end(ap);
    if (sect == 24 && off == 0x059a) show(args[0]);
    else if (sect == 14 && off == 0x27ac) { canceled++; g_ww(CBF(args[0], CB_ORDER), 0); }
    return 0;
}
void hook_call_original(void)
{
    if (placing) { AX = 0; g_ww(RNG, (u16)(g_rw(RNG) + 1)); hook_return_far(0); return; }
    original_calls++;
    if (unwind_transition) { unw_active = ns_unwinding = 1; return; }
    int n = (s16)rw(SS, SP + 4), who = (s16)rw(SS, SP + 6);
    int before = g_rb(GV_floor_shown);
    int target = rb(G_FAR20A5, STAIRS_RECORDS + 14 * n + 6);
    g_wb(GV_cbt_floor + who, (u8)target); show(target);
    u16 stack = SP;
    SP -= 32; ww(SS, SP + 4, (u16)who);
    placing = 1; placement_hook(); placing = 0; SP = stack;
    if (target != g_rb(GV_party_floor)) show(before);
    g_wb(STAIRS_BACK + who, 0xff); /* the original may read this from the wrong image */
    hook_return_far(0);
}
static void record(int floor, int target, int arrival_x, int arrival_y, int back)
{
    u8 *at = maps[floor] + STAIRS_RECORDS;
    at[0] = 100; at[2] = 300 & 255; at[3] = 300 >> 8; at[4] = 32; at[5] = 16;
    at[6] = (u8)target; at[7] = 255;
    at[8] = (u8)arrival_x; at[9] = (u8)(arrival_x >> 8);
    at[10] = (u8)arrival_y; at[11] = (u8)(arrival_y >> 8);
    at[12] = 0; at[13] = (u8)back;
    maps[floor][STAIRS_COUNT] = 1;
    maps[floor][4 * 100] = 0x51;
}
static void setup(void)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R); memset(maps, 0, sizeof maps);
    SS = DS = G_DGROUP; SP = 0xf000;
    enabled = 1; original_mode = original_calls = placing = reject_all = notices = canceled = 0;
    unwind_transition = 0;
    dos_no_writes = ns_unwinding = unw_active = 0; landing = NULL;
    record(0, 1, 176, 72, 0); record(1, 0, 176, 72, 3);
    show(0); g_wb(LEVEL_COUNT, 2); g_wb(GV_cbt_floor + 1, 1);
    g_wb(CBF(1, CB_STATUS), 1); g_ww(CBF(1, CB_X), 176); g_ww(CBF(1, CB_Y), 72);
    g_ww(RNG, 1234); ww(SS, SP + 4, 0); ww(SS, SP + 6, 1);
}
int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    setup(); take_exit_hook();
    assert(original_calls == 1 && !notices && !landing && !dos_no_writes);
    assert(g_rb(GV_cbt_floor + 1) == 0 && g_rb(GV_floor_shown) == 0);
    assert(g_rw(CBF(1, CB_X)) == 118 && g_rw(CBF(1, CB_Y)) == 310);
    assert(g_rw(LAST_X + 2) == 118 && g_rw(LAST_Y + 2) == 310 && g_rb(STAIRS_BACK + 1) == 3);
    assert(rb(G_FAR20A5, 4 * 100) == 1 && g_rb(GV_floor_dirty));
    assert(g_rw(RNG) == 1235); /* only the actual placement draws, the query was restored */
    assert(SP == 0xf004);

    setup(); reject_all = 1; take_exit_hook();
    assert(!original_calls && notices == 1 && canceled == 1 && !dos_no_writes);
    assert(g_rb(GV_cbt_floor + 1) == 1 && g_rb(GV_floor_shown) == 0);
    assert(g_rw(CBF(1, CB_X)) == 176 && g_rw(CBF(1, CB_Y)) == 72 && g_rw(RNG) == 1234);
    assert(!memcmp(M + LIN(G_FAR20A5, 0), maps[0], sizeof maps[0]));

    setup(); ww(SS, SP + 4, 6); take_exit_hook();
    assert(!original_calls && notices == 1 && g_rb(GV_floor_shown) == 0 && g_rw(RNG) == 1234);
    setup(); maps[1][STAIRS_RECORDS + 6] = 16; take_exit_hook();
    assert(!original_calls && notices == 1 && g_rb(GV_cbt_floor + 1) == 1);
    for (int actor = -1; actor <= 5; actor += 6) {
        setup(); ww(SS, SP + 6, (u16)actor); take_exit_hook(); assert(!original_calls && SP == 0xf004);
    }
    setup(); original_mode = 1; take_exit_hook(); assert(original_calls == 1 && !notices);
    setup(); enabled = 0; take_exit_hook(); assert(original_calls == 1 && !notices);
    setup(); unwind_transition = 1; take_exit_hook();
    assert(unw_active && ns_unwinding && !landing && !dos_no_writes);
    free(M);
    puts("stairs regression checks: PASS");
    return 0;
}
