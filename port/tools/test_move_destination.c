/* Walk/Flee before a destination, through the port's order_mode_leave hook.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_move_destination port/tools/test_move_destination.c && /tmp/test_move_destination
 */
#include <assert.h>
#include <string.h>
#include "../mods/ways.c"

CPU R;
u8 *M;
u16 LOADSEG;
int ns_unwinding;
static int enabled, delivered;

int fix_is_on(const char *id)
{
    assert(!strcmp(id, "movedestination"));
    return enabled;
}

void hook_call_original(void)
{
    delivered = (s16)g_rw(GV_order_action);
}

static void order(int action, int prior_flee)
{
    memset(M, 0, MEM_SIZE);
    g_wb(GV_selected_member, 3);
    g_ww(GV_order_action, (u16)action);
    g_ww(CBF(3, CB_ORDER), MISSILE);
    g_wb(CBF(3, CB_TARGET), 6);
    g_wb(GV_order_prior_flee, (u8)prior_flee);
    g_ww(GV_cbt_flags + 6, (u16)(prior_flee ? 0x0022 : 0x1022));
    ps[3] = 6;
    enabled = 1;
}

int main(void)
{
    M = calloc(1, MEM_SIZE);
    assert(M);
    for (int was_fleeing = 0; was_fleeing <= 1; was_fleeing++) {
        order(1, was_fleeing);
        leave_hook();
        assert(delivered == -1 && ps[3] == 6);
        assert(g_rw(GV_cbt_flags + 6) == (was_fleeing ? 0x1022 : 0x0022));
    }
    order(1, 0);
    g_ww(GV_order_dest_x, 340); /* a ground click */
    leave_hook();
    assert(delivered == 1 && g_rw(GV_cbt_flags + 6) == 0x1022);

    order(1, 0);
    g_wb(GV_order_foe_clicked, 1); /* previous click in this selection is still pending */
    leave_hook();
    assert(delivered == 1);

    order(1, 0);
    enabled = 0;
    leave_hook();
    assert(delivered == 1 && g_rw(GV_cbt_flags + 6) == 0x1022);

    order(MISSILE, 0);
    leave_hook();
    assert(delivered == MISSILE && ps[3] == 6);
    free(M);
    return 0;
}
