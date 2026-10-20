/* builtin.c - the built-in mods. The same changes as the source mods in mods/ (built into a
 * DOS executable there), done here as run-time hooks on the recompiled game. */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
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
static void heavy_blows(int on) { if (on) hook_install(GF_damage_queue, heavy_blows_hook); else hook_remove(GF_damage_queue, heavy_blows_hook); }

/* ---- swift travel: overland steps cost a third of the time (s030 1462:2ac8 returns the
 * minutes of one step on a terrain) */
static void swift_travel_hook(void)
{
    hook_call_original();
    AX = (u16)((AX + 2) / 3);
    if (!AX) AX = 1;
}
static void swift_travel(int on) { if (on) hook_install(30, 0x1462, 0x2ac8, swift_travel_hook); else hook_remove(30, 0x1462, 0x2ac8, swift_travel_hook); }

/* ---- half-price inns: the lodging price rule (vec_2099 = s039 1462:1d2c) */
static void cheap_inn_hook(void)
{
    hook_call_original();
    AX = (u16)((AX + 1) / 2);
}
static void cheap_inn(int on) { if (on) hook_install(39, 0x1462, 0x1d2c, cheap_inn_hook); else hook_remove(39, 0x1462, 0x1d2c, cheap_inn_hook); }

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
static void strong_party(int on) { if (on) hook_install(GF_party_default, strong_party_hook); else hook_remove(GF_party_default, strong_party_hook); }

/* ---- rich start: 5000 florins after the opening card script (s044 1838:0000) */
static void rich_start_hook(void)
{
    hook_call_original();
    g_ww(G_MONEY, 5000); g_ww(G_MONEY + 2, 0); g_ww(G_MONEY + 4, 0);
}
static void rich_start(int on) { if (on) hook_install(44, 0x1838, 0x0000, rich_start_hook); else hook_remove(44, 0x1838, 0x0000, rich_start_hook); }

/* ---- quick learning: skills improve about twice as often through use. Cards: practice_skill
 * (s039 1462:0132, args who, k, mode, pct, lvl) gets twice the percentage; battles:
 * skill_try_improve (s021 1ef8:0000, args m, sk, mode) gets a second try when the first fails */
static void quick_learn_cards_hook(void)
{
    u16 a = (u16)(SP + 10), pct = rw(SS, a);
    { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_LEARN") != 0;
      if (tr) fprintf(stderr, "quick learning: practice skill %d of %d, %d%% -> %d%%\n", rw(SS, (u16)(SP + 6)), (s16)rw(SS, (u16)(SP + 4)), pct, pct * 2); }
    ww(SS, a, (u16)(pct * 2));
    hook_call_original();
    ww(SS, a, pct);
}
static void quick_learn_battle_hook(void)
{
    u16 m = rw(SS, (u16)(SP + 4)), sk = rw(SS, (u16)(SP + 6));
    if (m >= 5 || sk >= 19) { hook_call_original(); return; }
    u16 at = (u16)(G_MEMBER(m) + G_M_SKILLS + sk);
    u8 before = g_rb(at);
    u16 sp0 = SP, ip = rw(SS, sp0), cs = rw(SS, (u16)(sp0 + 2));
    hook_call_original();                           /* (its far ret pops the return address) */
    { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_LEARN") != 0;
      if (tr) fprintf(stderr, "quick learning: battle skill %d of %d: %s\n", sk, m, g_rb(at) == before ? "second try" : "rose"); }
    if (g_rb(at) == before) {                       /* the second try: the same call again */
        u16 sp1 = SP;
        SP = sp0; ww(SS, sp0, ip); ww(SS, (u16)(sp0 + 2), cs);
        hook_call_original();
        SP = sp1;
    }
}
static void quick_learning(int on)
{
    if (on) { hook_install(GF_practice_skill_impl, quick_learn_cards_hook); hook_install(GF_skill_try_improve, quick_learn_battle_hook); }
    else { hook_remove(GF_practice_skill_impl, quick_learn_cards_hook); hook_remove(GF_skill_try_improve, quick_learn_battle_hook); }
}

Mod MODS[] = {
    { "swift_travel", "Swift travel", "Travel", "Overland travel takes a third of the time.", swift_travel, 0, 0 },
    { "cheap_inn", "Half-price inns", "Towns", "Lodging costs half as much.", cheap_inn, 0, 0 },
    { "quick_learning", "Quick learning", "Skills", "Skills improve about twice as often through use, on cards and in battles (Streetwise, Latin and the rest no longer take an age).", quick_learning, 0, 0 },
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

void pathfix_init(void);
void travel_init(void);
void fixes_init(void);
void hdtext_init(void);
void hdart_init(void);
void sfx_init(void);
void saves_init(void);
void mods_init(void)
{
    /* first of the hooks on menu_bar_run, so that it runs last, after the others' (the saves', the
       native screens' safe point): when on, it does the menu itself and does not call the original */
    { void menuview_init(void); menuview_init(); }
    hdtext_init();
    hdart_init();
    sfx_init();
    { void musicpack_init(void); musicpack_init(); }
    { void looks_init(void); looks_init(); }
    fixes_init();
    { void aging_init(void); aging_init(); void losses_init(void); losses_init(); }
    saves_init();
    assist_init();
    { void cardview_init(void); cardview_init(); }
    { void nscreen_init(void); nscreen_init(); }
    { void sheetview_init(void); sheetview_init(); }
    { void alchemyview_init(void); alchemyview_init(); }
    { void loot_init(void); loot_init(); }
    { void cacheview_init(void); cacheview_init(); }
    { void sharedstore_init(void); sharedstore_init(); }
    { void prayview_init(void); prayview_init(); }
    { void createview_init(void); createview_init(); }
    { void rosterview_init(void); rosterview_init(); }
    { void infoview_init(void); infoview_init(); }
    { void titleview_init(void); titleview_init(); }
    { void marchview_init(void); marchview_init(); }
    { void lodgeview_init(void); lodgeview_init(); }
    { void enemyview_init(void); enemyview_init(); }
    { void mapmsg_init(void); mapmsg_init(); }
    { void memberlooks_init(void); memberlooks_init(); }
    { void ammo_init(void); ammo_init(); }
    autopause_init();
    pathfix_init();
    travel_init();
    { void encounters_init(void); encounters_init(); }
    { void opening_init(void); opening_init(); }
    { void orders_init(void); orders_init(); }
    { void battlescroll_init(void); battlescroll_init(); }
    { void levelwalk_init(void); levelwalk_init(); }
    { void pack_init(void); pack_init(); }
    { void battlerules_init(void); battlerules_init(); }
    { void battleend_init(void); battleend_init(); }
    { void foesleft_init(void); foesleft_init(); }
    { void politics_init(void); politics_init(); }
    { void cutcards_init(void); cutcards_init(); }
    { void war_init(void); war_init(); }
    for (int i = 0; i < MOD_COUNT; i++) {
        char key[80]; snprintf(key, sizeof key, "mod.%s", MODS[i].id);
        if (config_get_int(key, 0)) { MODS[i].set(1); MODS[i].enabled = 1; }
    }
}
