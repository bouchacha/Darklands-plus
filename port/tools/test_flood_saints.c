/* Tidal-flood navigation after the card sets up its own saint choices.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_flood_saints port/tools/test_flood_saints.c && /tmp/test_flood_saints
 */
#include <assert.h>
#include "../mods/fixes.c"

CPU R;
u8 *M;
u16 LOADSEG;
int ns_unwinding;
static int original_mode, second_page, calls;

int config_get_int(const char *key, int def)
{
    return !strcmp(key, "original") ? original_mode : def;
}

void hook_call_original(void)
{
    calls++;
    if (second_page) g_ww(GV_next_card, 0x11c); /* the original chooses its own page again */
}

int main(void)
{
    M = calloc(1, MEM_SIZE);
    assert(M);
    for (original_mode = 0; original_mode <= 1; original_mode++) {
        for (ns_unwinding = 0; ns_unwinding <= 1; ns_unwinding++) {
            for (int can_pray = 1; can_pray <= 2; can_pray++) {
                int fixed = !original_mode && !ns_unwinding;
                memset(M, 0, MEM_SIZE);
                g_ww(GV_opt_flag_1, 2); /* no rope */
                g_ww(GV_opt_flag_2, can_pray);
                g_ww(GV_card_saints, 44);
                second_page = calls = 0;
                flood_lists_hook();
                assert(calls == 1 && g_rw(GV_opt_flag_3) == fixed && g_rw(GV_opt_flag_4) == fixed);
                assert(g_rw(GV_opt_flag_1) == 2 && g_rw(GV_opt_flag_2) == can_pray);
                assert(g_rw(GV_card_saints) == 44 && g_rw(GV_opt_flag_5) == 0);
                second_page = 1;
                flood_third_hook();
                assert(calls == 2 && g_rw(GV_next_card) == (fixed ? 0x11d : 0x11c));
            }
        }
    }
    free(M);
    return 0;
}
