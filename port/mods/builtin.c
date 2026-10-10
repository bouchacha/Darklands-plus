/* builtin.c - the built-in mods. The same changes as the source mods in mods/ (built into a
 * DOS executable there), done here as run-time hooks on the recompiled game. */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "mods.h"
#include "assist.h"

/* ---- heavy blows: every hit on an enemy does double damage (s014 18bf:08ca rolls the
 * damage of one hit on combatant t and queues it in D_d70c[t]; enemies are 5..12) */
static void heavy_blows_hook(void)
{
    u16 t = rw(SS, (u16)(SP + 6));                  /* far call: ret ip, cs, d, t, ... */
    u16 slot = (u16)(0xd70c + 2 * t);
    u16 before = t < 16 ? g_rw(slot) : 0;
    hook_call_original();
    if (t >= 5 && t < 16) {
        u16 after = g_rw(slot);
        if (after > before) g_ww(slot, (u16)(after + (after - before)));
    }
}
static void heavy_blows(int on) { if (on) hook_install(14, 0x18bf, 0x08ca, heavy_blows_hook); else hook_remove(14, 0x18bf, 0x08ca); }

/* ---- swift travel: overland steps cost a third of the time (s030 1462:2ac8 returns the
 * minutes of one step on a terrain) */
static void swift_travel_hook(void)
{
    hook_call_original();
    AX = (u16)((AX + 2) / 3);
    if (!AX) AX = 1;
}
static void swift_travel(int on) { if (on) hook_install(30, 0x1462, 0x2ac8, swift_travel_hook); else hook_remove(30, 0x1462, 0x2ac8); }

/* ---- half-price inns: the lodging price rule (vec_2099 = s039 1462:1d2c) */
static void cheap_inn_hook(void)
{
    hook_call_original();
    AX = (u16)((AX + 1) / 2);
}
static void cheap_inn(int on) { if (on) hook_install(39, 0x1462, 0x1d2c, cheap_inn_hook); else hook_remove(39, 0x1462, 0x1d2c); }

/* ---- strong party: the Quickstart party gets 60 in every attribute, 75 in every skill
 * (after s001 0e76:0000 has built the default party) */
static void strong_party_hook(void)
{
    hook_call_original();
    for (int m = 0; m < 4; m++) {
        u16 r = G_MEMBER(m);
        for (int a = 0; a < 6; a++) { g_wb((u16)(r + G_M_ATTR_MAX + a), 60); g_wb((u16)(r + G_M_ATTR_CUR + a), 60); }
        for (int k = 0; k < 19; k++) g_wb((u16)(r + G_M_SKILLS + k), 75);
    }
}
static void strong_party(int on) { if (on) hook_install(1, 0x0e76, 0x0000, strong_party_hook); else hook_remove(1, 0x0e76, 0x0000); }

/* ---- rich start: 5000 florins after the opening card script (s044 1838:0000) */
static void rich_start_hook(void)
{
    hook_call_original();
    g_ww(G_MONEY, 5000); g_ww(G_MONEY + 2, 0); g_ww(G_MONEY + 4, 0);
}
static void rich_start(int on) { if (on) hook_install(44, 0x1838, 0x0000, rich_start_hook); else hook_remove(44, 0x1838, 0x0000); }

Mod MODS[] = {
    { "swift_travel", "Swift travel", "Travel", "Overland travel takes a third of the time.", swift_travel, 0, 0 },
    { "cheap_inn", "Half-price inns", "Towns", "Lodging costs half as much.", cheap_inn, 0, 0 },
    { "heavy_blows", "Heavy blows", "Combat", "Every hit on an enemy does double damage. Hits on the party are unchanged.", heavy_blows, 0, 0 },
    { "strong_party", "Strong Quickstart party", "New game", "The four Quickstart characters start with 60 in every attribute and 75 in every skill.", strong_party, 0, 1 },
    { "rich_start", "Rich start", "New game", "A party made with Create a New World starts with 5000 florins instead of 200.", rich_start, 0, 1 },
};
int MOD_COUNT = sizeof MODS / sizeof MODS[0];

void mod_enable(int i, int on)
{
    if (i < 0 || i >= MOD_COUNT) return;
    MODS[i].set(on);
    MODS[i].enabled = on;
    char key[80]; snprintf(key, sizeof key, "mod.%s", MODS[i].id);
    config_set_int(key, on);
}

void mods_init(void)
{
    assist_init();
    for (int i = 0; i < MOD_COUNT; i++) {
        char key[80]; snprintf(key, sizeof key, "mod.%s", MODS[i].id);
        if (config_get_int(key, 0)) { MODS[i].set(1); MODS[i].enabled = 1; }
    }
}
