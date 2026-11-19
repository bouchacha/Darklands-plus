/* Tomb completion regression checks, using the real fix with emulated state.
 * From the repository root:
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_tomb port/tools/test_tomb.c && /tmp/test_tomb
 * The combat-loop call is isolated here. Exercise victory and quest-item award
 * in the full game separately, starting from a copy of a normal pre-battle save.
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active;
static int original_calls, finish, giving, awards, pending;

void hook_call_original(void)
{
    original_calls++;
    if (giving) {
        assert((s16)g_rw(G_CUR_LOC) == 200 && rw(SS, (u16)(SP + 4)) == 200);
        if (pending) { awards++; pending = 0; }
        hook_return_far(0);
        return;
    }
    if (finish && g_rb(GV_place_won) && g_rb(GV_real_combat)) {
        g_ww(GV_battle_result, 0);
        AX = 1;
    } else AX = 0;
}
void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
static void reset(void)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    original_calls = 0; finish = 1; unw_active = 0; giving = 0; awards = 0; pending = 1;
    g_ww(GV_battlefield_kind, 0x5f);
    g_ww(GV_battle_result, (u16)-1);
    g_wb(CBF(0, CB_STATUS), 1);
    g_ww(GV_activations_seg, 0x6000);
}
static void unchanged(void)
{
    int before = (s16)g_rw(GV_battle_result);
    tomb_over_hook();
    assert(!g_rb(GV_place_won) && !g_rb(GV_real_combat));
    assert((s16)g_rw(GV_battle_result) == before && original_calls == 1);
}
int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    for (int foe = 5; foe < 13; foe++) {
        reset(); g_wb(CBF(foe, CB_STATUS), 1); unchanged(); /* any standing skeleton */
    }
    for (int kind = 0; kind < 3; kind++) {
        reset(); g_wb(GV_activation_count, 2);
        wb(0x6000, (u16)(ACTIVATIONS_OFF + 0x1e + 0x0b + kind), 1);
        unchanged();                              /* an unexplored group of any kind */
    }
    reset(); g_wb(GV_activation_count, 31); unchanged(); /* malformed state is not a win */
    reset(); g_wb(GV_activation_count, 30);
    wb(0x6000, ACTIVATIONS_OFF + 29 * 0x1e + 0x0d, 1); unchanged(); /* last slot of last group */
    reset(); g_wb(GV_activation_count, 1); g_ww(GV_activations_seg, 0); unchanged();
    reset(); g_wb(CBF(0, CB_STATUS), 2); unchanged(); /* nobody conscious */
    reset(); g_wb(GV_cbt_floor, (u8)-99); unchanged(); /* party already left */
    reset(); g_wb(GV_floor_shown, 1); unchanged();
    reset(); g_ww(GV_battlefield_kind, 0x5e); unchanged();       /* another dungeon */
    reset(); g_ww(GV_battlefield_kind, 0); unchanged();          /* generic battle resume is outside this fix */
    for (int result = 1; result <= 8; result++) {
        reset(); g_ww(GV_battle_result, (u16)result); unchanged(); /* fled/lost/other exit */
    }
    reset(); unw_active = 1; unchanged();
    reset(); finish = 0;                           /* game's loot/delay has not finished */
    tomb_over_hook(); assert(!AX && g_rb(GV_place_won));
    reset(); g_wb(CBF(5, CB_STATUS), 2);            /* last skeleton unconscious */
    tomb_over_hook(); assert(AX == 1 && g_rw(GV_battle_result) == 0 && original_calls == 1);
    reset(); g_ww(GV_battle_result, 0);            /* victory delay already running */
    tomb_over_hook(); assert(g_rb(GV_place_won) && !g_rb(GV_real_combat) && original_calls == 1);
    for (int member = 1; member < 5; member++) {
        reset(); g_wb(CBF(0, CB_STATUS), 2);       /* a surviving follower can finish it */
        g_wb(CBF(member, CB_STATUS), 1);
        tomb_over_hook(); assert(AX == 1 && g_rw(GV_battle_result) == 0);
    }
    for (int result = -1; result <= 8; result++) {
        reset(); giving = 1; tomb_loc = 200; SS = G_DGROUP; SP = 0x8000;
        g_ww(G_CUR_LOC, (u16)-1); g_ww(GV_battle_result, (u16)result);
        ww(SS, SP + 4, (u16)-1); AX = (u16)-1; /* an unreliable native return must not veto victory */
        tomb_give_hook();
        int kept = result >= 0 && result <= 2;      /* won, or got out by the way out (1, 2) */
        assert(awards == kept && original_calls == kept);
        assert((s16)g_rw(G_CUR_LOC) == -1 && SP == 0x8004);
        if (result == 0) {
            SP = 0x8000; tomb_give_hook(); assert(awards == 1); /* fetching event already resolved */
        }
    }
    reset();                                     /* already returned to peaceful exploration */
    assert(!g_rb(GV_real_combat));
    tomb_over_hook(); assert(AX == 1 && g_rw(GV_battle_result) == 0 && original_calls == 1);
    free(M);
    puts("tomb regression checks: PASS");
    return 0;
}
