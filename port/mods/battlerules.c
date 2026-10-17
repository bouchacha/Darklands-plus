/* battlerules.c - two rules of the battles, each a switch (F12 > Helpers > Battles).
 *
 * battle.shoot_past_allies (on): a member with a bow, crossbow, gun or thrown weapon will not
 *   shoot while anyone stands in the line of fire, friend or foe; they wait ('b' on the panel)
 *   and try again. The test is line_blocked (s014 1d69:000c, (a shooter, b target, mode 0x80 at
 *   firing and pointing, 0x100 on a click)): walls, furniture, then every active body on the floor
 *   shown but the shooter and the target, its box shrunk by the shooter's skill / 90 (none at 90).
 *   Yet a missile never strikes anyone but its target: a hit is decided before it flies and flies
 *   to the target, a miss passes through everyone (s018 1ef8 missile_launch, s014 1d47). So for
 *   the party's shooters the party's other members are left out of the test: they stand on no
 *   floor while it runs. The enemies, and the enemies' shots, are as the game has them.
 *
 * battle.potions_spare_party (off): a potion the party throws lands where it is aimed and acts on
 *   everyone there (effect_spell, s019 1ef8:0000 (x, y, thrower, kind, quality, target, floor)):
 *   the clouds (Noxious Aroma, Eyeburn, Black Cloud, Fleadust, Eater-Water, Breath of Death,
 *   kinds 0-5) on all inside and in sight of the middle, Thunderbolt and Arabian Fire (7, 8) on all
 *   in reach, the party too; Sunburst (6) only on the other side. A cloud acts only as it lands. With
 *   this on, the party's members are immune to their own potions then (the immunity mask of each
 *   fighter, int[13] at 0xc927, a bit per kind, as some foes have). Stone-tar (9) slows whoever
 *   walks in it, a step at a time, with no thrower: it stays as it is. */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

#define D_IMMUNE 0xc927                       /* int[13]: kinds of potion a fighter is immune to */

static int on_past(void) { return config_get_int("battle.shoot_past_allies", 1) && !config_get_int("original", 0); }
static int on_spare(void) { return config_get_int("battle.potions_spare_party", 0) && !config_get_int("original", 0); }

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

/* (effect_spell is also how a trapped floor or chest goes off, s023 1ef8:0d88 / 153e, with the member who set it
   off as the "thrower": only a thrown potion landing, in missiles_advance (s014 1d47:0000), counts) */
static int landing;
static void missiles_hook(void) { landing++; hook_call_original(); if (landing > 0) landing--; }
static void spell_hook(void)                  /* effect_spell (x, y, thrower, kind, quality, target, floor) */
{
    int thrower = (s16)rw(SS, (u16)(SP + 8)), kind = (s16)rw(SS, (u16)(SP + 10));
    if (!landing || thrower < 0 || thrower >= 5 || kind < 0 || kind > 8 || kind == 6 || !on_spare()) { hook_call_original(); return; }
    u16 im[5];
    for (int k = 0; k < 5; k++) { im[k] = g_rw((u16)(D_IMMUNE + 2 * k)); g_ww((u16)(D_IMMUNE + 2 * k), (u16)(im[k] | (1u << kind))); }
    hook_call_original();
    for (int k = 0; k < 5; k++) g_ww((u16)(D_IMMUNE + 2 * k), im[k]);
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "battlerules: potion kind %d from %d spares the party\n", kind, thrower);
}

void battlerules_init(void)
{
    hook_install(GF_line_blocked, blocked_hook);
    hook_install(GF_effect_spell, spell_hook);
    hook_install(14, 0x1d47, 0x0000, missiles_hook);
}
