/* ammo.c - arrows, quarrels and balls that never run out (assist.infinite_ammo, on by default;
 * "Play the original game" turns it off).
 *
 * In a battle a shot (s014 0e76, the loose) records an arrow or a bolt to be found after the
 * battle 7 times in 10 (missile_spent, s018 1ef8:03ae; balls never), then use_ammo takes one
 * from the shooter's pack (consume_item, s014 18bf:0db4 (i, item type, quality 25)). Keeping
 * every member's quiver filled meant picking the spent ones out of the loot after each fight and
 * buying more. Here a party member's shot takes nothing from the pack (one arrow, quarrel or
 * ball is still needed to shoot at all), and leaves nothing behind for the loot; enemies shoot
 * and lose theirs as before. Javelins, throwing axes and darts are not ammunition: they go back
 * to their throwers after the battle (mods/loot.c). */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

int ammo_infinite = 1;
static int shooter = -1;                     /* who loosed the last missile */

static int is_ammo(int type) { return type == 0x40 || type == 0x41 || type == 0x42; }   /* arrow, quarrel, ball */
static int on(void) { return ammo_infinite && !config_get_int("original", 0); }

static void launch_hook(void)                /* missile_launch (i, target, ...) */
{
    shooter = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
}

static void spent_hook(void)                 /* missile_spent (kind, quality) */
{
    int kind = (s16)rw(SS, (u16)(SP + 4));
    if (on() && is_ammo(kind) && shooter >= 0 && shooter < 5) { hook_return_far(0); return; }   /* (nothing to pick up: none was used) */
    hook_call_original();
}

static void consume_hook(void)               /* consume_item (i, item type, quality): the count left, -1 none */
{
    int i = (s16)rw(SS, (u16)(SP + 4)), type = rb(SS, (u16)(SP + 6)), q = rb(SS, (u16)(SP + 8));
    if (on() && i >= 0 && i < 5 && is_ammo(type)) {
        int n = g_rb((u16)(G_MEMBER(i) + G_M_NITEMS));
        for (int k = 0; k < n && k < 64; k++) {
            u16 o = G_ITEM(i, k);
            if (rb(G_FAR2E38, (u16)(o + 2)) == type && rb(G_FAR2E38, (u16)(o + 3)) == q) {
                AX = rb(G_FAR2E38, (u16)(o + 4));
                hook_return_far(0);
                return;
            }
        }
    }
    hook_call_original();
}

void ammo_set(int v) { ammo_infinite = v; config_set_int("assist.infinite_ammo", v); }

void ammo_init(void)
{
    ammo_infinite = config_get_int("assist.infinite_ammo", 1);
    hook_install(GF_missile_launch, launch_hook);
    hook_install(GF_missile_spent, spent_hook);
    hook_install(GF_consume_item, consume_hook);
}
