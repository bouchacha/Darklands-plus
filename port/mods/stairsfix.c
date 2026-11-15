/* stairsfix.c - use a member's own stairs and find a safe landing before changing floors.
 * The original take_exit reads the displayed floor, then keeps the old coordinates when
 * place_around fails. The query below uses the game's placement search in an evaluation
 * snapshot, before take_exit can move the party or its enemies. Restoration's findings
 * B-bdd7dd, B-5e9887 and B-fcc034 identified these original-game paths.
 */
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/hooks.h"
#include "fixes.h"
#include "nscreen.h"

#define STAIRS_COUNT 0x2fb6
#define STAIRS_RECORDS 0x2fb8
#define STAIRS_BACK 0xc8b2
#define LEVEL_COUNT 0xd661
#define LAST_X 0xd645
#define LAST_Y 0xd664

void ui_notice(const char *);
extern int dos_no_writes;

typedef struct {
    int who, destination, x, y, used;
    unsigned cleared[9], ncleared;
} StairLanding;
static StairLanding *landing;

static int stairs_count(void)
{
    int count = (s8)rb(G_FAR20A5, STAIRS_COUNT);
    return count >= 0 && count <= 6 ? count : 0;
}

/* take_exit uncovers this wall group on the destination before placing the member. */
static int reveal_arrival(int wall)
{
    if (wall == -1) return 1;
    if (wall < 0 || wall >= (s8)rb(G_FAR20A5, 0x2488) || wall >= 64) return 0;
    unsigned at = rw(G_FAR20A5, (u16)(0x248a + 2 * wall));
    for (int i = 0; i < 1600 && at + 1 < 0x33fc; i++, at += 2) {
        if (rw(G_FAR20A5, (u16)at) == 0xffff) return 1;
        unsigned y = rb(G_FAR20A5, (u16)at), x = rb(G_FAR20A5, (u16)(at + 1));
        if (x >= 40 || y >= 40) return 0;
        u16 cell = (u16)((40 * x + y) * 4 + 3);
        wb(G_FAR20A5, cell, rb(G_FAR20A5, cell) & 0x7f);
    }
    return 0;
}

static int try_landing(StairLanding *plan, const u8 record[14], const u8 cells[6400])
{
    int x = record[8] | record[9] << 8, y = record[10] | record[11] << 8;
    if (x < 32 || x >= 1248 || y < 16 || y >= 624 || (record[12] & 1) || record[12] > 6) return 0;
    u16 stack = SP;
    SP = (u16)((SP - 16) & ~1);
    memcpy(M + LIN(SS, SP), record, 14);
    u16 args[2] = { (u16)plan->who, SP };
    int found = game_call_far(GF_place_around, 2, args) == 1;
    SP = stack;
    plan->x = (s16)g_rw(CBF(plan->who, CB_X));
    plan->y = (s16)g_rw(CBF(plan->who, CB_Y));
    found = found && plan->x >= 32 && plan->x < 1248 && plan->y >= 16 && plan->y < 624;
    plan->ncleared = 0;
    /* The original search may clear furniture in the arrival's 3x3 cells. Keep
     * only those removals if this position is needed by the actual transition. */
    for (unsigned cell = 0; found && cell < 1600; cell++) {
        if ((cells[cell * 4] & 0xf0) && !(rb(G_FAR20A5, (u16)(cell * 4)) & 0xf0)) {
            if (plan->ncleared == 9) { found = 0; break; }
            plan->cleared[plan->ncleared++] = cell;
        }
    }
    memcpy(M + LIN(G_FAR20A5, 0), cells, 6400);
    return found;
}

static int find_landing(StairLanding *plan, int from, const u8 record[14])
{
    void *snapshot = eval_snapshot();
    int writes = dos_no_writes;
    StairLanding *previous = landing;
    landing = NULL;
    dos_no_writes = 1;
    /* The source image is in the snapshot. No query needs to write it to a swap file. */
    g_wb(GV_floor_dirty, 0);
    u16 destination = (u16)plan->destination;
    if ((s8)g_rb(GV_floor_shown) != plan->destination) game_call_far(GF_level_switch, 1, &destination);
    g_wb((u16)(GV_cbt_floor + plan->who), (u8)plan->destination);
    int found = 0;
    if ((s8)g_rb(GV_floor_shown) == plan->destination && reveal_arrival((s8)record[7])) {
        u8 cells[6400];
        memcpy(cells, M + LIN(G_FAR20A5, 0), sizeof cells);
        found = try_landing(plan, record, cells);
        for (int n = 0; !found && n < stairs_count(); n++) {
            u16 at = (u16)(STAIRS_RECORDS + 14 * n);
            if ((s8)rb(G_FAR20A5, (u16)(at + 6)) != from) continue;
            u8 back[14]; memcpy(back, M + LIN(G_FAR20A5, at), sizeof back);
            int x = (s16)rw(G_FAR20A5, at) + back[4] / 2;
            int y = (s16)rw(G_FAR20A5, (u16)(at + 2)) + back[5] / 2;
            back[8] = (u8)x; back[9] = (u8)(x >> 8);
            back[10] = (u8)y; back[11] = (u8)(y >> 8);
            found = try_landing(plan, back, cells);
        }
    }
    eval_restore(snapshot); eval_free(snapshot);
    dos_no_writes = writes;
    landing = previous;
    return found;
}

static void placement_hook(void)             /* place_around(who, near stairs record) */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    if (ns_unwinding || !landing || landing->who != who
        || (s8)g_rb(GV_floor_shown) != landing->destination) return;
    landing->used = 1;
    if (AX) return;
    for (unsigned i = 0; i < landing->ncleared; i++) {
        u16 at = (u16)(landing->cleared[i] * 4);
        wb(G_FAR20A5, at, rb(G_FAR20A5, at) & 0x0f);
        g_wb(GV_floor_dirty, 1);
    }
    g_ww(CBF(who, CB_X), (u16)landing->x); g_ww(CBF(who, CB_Y), (u16)landing->y);
    g_ww((u16)(LAST_X + 2 * who), (u16)landing->x);
    g_ww((u16)(LAST_Y + 2 * who), (u16)landing->y);
    AX = 1;
}

static void take_exit_hook(void)             /* take_exit(record, who), outside levelwalk's group helper */
{
    if (!fix_is_on("stairsreturn")) { hook_call_original(); return; }
    int n = (s16)rw(SS, (u16)(SP + 4)), who = (s16)rw(SS, (u16)(SP + 6));
    int levels = g_rb(LEVEL_COUNT), shown = (s8)g_rb(GV_floor_shown);
    if (who < 0 || who >= 5 || g_rb(CBF(who, CB_STATUS)) != 1) { hook_return_far(0); return; }
    int from = (s8)g_rb((u16)(GV_cbt_floor + who));
    if (levels < 1 || levels > 16 || from < 0 || from >= levels || shown < 0 || shown >= levels) {
        hook_return_far(0); return;
    }
    if (shown != from) ns_call(GF_level_switch, 1, from);
    if (ns_unwinding) return;
    u8 record[14];
    int valid = n >= 0 && n < stairs_count();
    StairLanding plan = { 0 }; plan.who = who;
    if (valid) {
        memcpy(record, M + LIN(G_FAR20A5, STAIRS_RECORDS + 14 * n), sizeof record);
        plan.destination = (s8)record[6];
        valid = plan.destination >= 0 && plan.destination < levels && find_landing(&plan, from, record);
    }
    if (!valid) {
        if (shown != (s8)g_rb(GV_floor_shown)) ns_call(GF_level_switch, 1, shown);
        if (ns_unwinding) return;
        ui_notice("These stairs have no safe arrival. The member stays on this floor.");
        ns_call(GF_cbt_cancel_action, 1, who);
        if (!ns_unwinding) hook_return_far(0);
        return;
    }
    StairLanding *previous = landing;
    landing = &plan;
    hook_call_original();
    landing = previous;
    if (ns_unwinding) return;
    u16 ax = AX, dx = DX;
    if (plan.used) g_wb((u16)(STAIRS_BACK + who), record[13]);
    if ((!plan.used || (s8)g_rb(GV_party_floor) != (s8)g_rb((u16)(GV_cbt_floor + who)))
        && shown != (s8)g_rb(GV_floor_shown)) {
        ns_call(GF_level_switch, 1, shown);
        if (ns_unwinding) return;
        ns_call(GF_floor_view_draw, 1, -1);
        if (ns_unwinding) return;
    }
    AX = ax; DX = dx;
}

void stairsfix_init(void)
{
    hook_install(GF_take_exit, take_exit_hook);
    hook_install(GF_place_around, placement_hook);
}
