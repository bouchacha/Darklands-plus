/* battlerules.c - a rule of the battles, a switch (F12 > Helpers > Battles).
 *
 * battle.shoot_past_allies (off by default): a member with a bow, crossbow, gun or thrown weapon will not
 *   shoot while anyone stands in the line of fire, friend or foe; they wait ('b' on the panel)
 *   and try again. The test is line_blocked (s014 1d69:000c, (a shooter, b target, mode 0x80 at
 *   firing and pointing, 0x100 on a click)): walls, furniture, then every active body on the floor
 *   shown but the shooter and the target, its box shrunk by the shooter's skill / 90 (none at 90).
 *   Yet a missile never strikes anyone but its target: a hit is decided before it flies and flies
 *   to the target, a miss passes through everyone (s018 1ef8 missile_launch, s014 1d47). So for
 *   the party's shooters the party's other members are left out of the test: they stand on no
 *   floor while it runs. The enemies, and the enemies' shots, are as the game has them.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"


static int on_past(void) { return config_get_int("battle.shoot_past_allies", 0) && !config_get_int("original", 0); }

static void blocked_hook(void)                /* line_blocked (a, b, mode) */
{
    int a = (s16)rw(SS, (u16)(SP + 4)), b = (s16)rw(SS, (u16)(SP + 6)), mode = rw(SS, (u16)(SP + 8));
    if (a < 0 || a >= 5 || (mode != 0x80 && mode != 0x100) || !on_past()) { hook_call_original(); return; }
    u8 fl[5];
    for (int k = 0; k < 5; k++) {
        fl[k] = g_rb((u16)(GV_cbt_floor + k));
        if (k != a && k != b) g_wb((u16)(GV_cbt_floor + k), 0xff);
    }
    hook_call_original();
    for (int k = 0; k < 5; k++) g_wb((u16)(GV_cbt_floor + k), fl[k]);
    if (getenv("DL_BATTLE_LOG") && AX) fprintf(stderr, "battlerules: %d's line to %d blocked (mode %#x)\n", a, b, mode);
}

void battlerules_init(void)
{
    hook_install(GF_line_blocked, blocked_hook);
}
