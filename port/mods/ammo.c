/* ammo.c - what a shot or a throw uses up (assist.endless, a choice: 1 endless, by default; 2 spent ones
 * given back after a battle; 0 the original's; the Full Original preset sets 0).
 *
 * In a battle a shot (s014 0e76, the loose) records an arrow or a bolt to be found after the
 * battle 7 times in 10 (missile_spent, s018 1ef8:03ae; balls never), then use_ammo takes one
 * from the shooter's pack (consume_item, s014 18bf:0db4 (i, item type, quality 25)). Keeping
 * every member's quiver filled meant picking the spent ones out of the loot after each fight and
 * buying more. Endless: a party member's shot takes nothing from the pack (one arrow, quarrel or
 * ball is still needed to shoot at all), and leaves nothing behind for the loot; enemies shoot
 * and lose theirs as before. Javelins, throwing axes, throwing knives and darts likewise
 * (use_ammo takes a thrown weapon itself, consume_item (i, its type, its quality)). Given back:
 * the original's rules, and what each member shot or threw is handed back to them from the loot's
 * pile when the loot screen opens (mods/loot.c). */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

int ammo_infinite = 1, thrown_infinite = 1;
static int shooter = -1;                     /* who loosed the last missile */

static int is_shot(int type) { return type == 0x40 || type == 0x41 || type == 0x42; }   /* arrow, quarrel, ball */
static int is_thrown(int type) { return type == 0x17 || type == 0x18 || type == 0x19 || type == 0x1a; }   /* throwing axe, javelin, dart, throwing knife (LST flag 0x10) */
static int is_ammo(int type)
{
    if (config_get_int("original", 0)) return 0;
    return (ammo_infinite && is_shot(type)) || (thrown_infinite && is_thrown(type));
}
static int on(void) { return (ammo_infinite || thrown_infinite) && !config_get_int("original", 0); }

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

extern int loot_return_thrown;
void ammo_mode_set(int v) { ammo_infinite = thrown_infinite = v == 1; loot_return_thrown = v == 2; }

void ammo_init(void)
{
    /* before 1.4.6 the choice was a switch, endless or not, and a switch of its own gave thrown weapons back:
       endless off with thrown weapons given back is now the middle way (which gives arrows and quarrels back too) */
    if (!config_get_int("migrate.ammo_choice", 0)) {
        if (config_get_int("assist.endless", 1) == 0 && config_get_int("assist.return_thrown", 1)) config_set_int("assist.endless", 2);
        config_unset("assist.infinite_ammo"); config_unset("assist.infinite_thrown"); config_unset("assist.return_thrown");
        config_set_int("migrate.ammo_choice", 1);
    }
    ammo_mode_set(config_get_int("assist.endless", 1));
    hook_install(GF_missile_launch, launch_hook);
    hook_install(GF_missile_spent, spent_hook);
    hook_install(GF_consume_item, consume_hook);
}
