/* cutcards.c - encounters MicroProse wrote and never used, put back (rules.fehde, rules.hospice:
 * F12 > Reconstructed Content, experimental, off by default; the catalogue of what the game holds
 * unused is docs/cut_content.md).
 *
 * "Fehde!" ($MeetF00): soldiers of a private war lie in ambush on the road. The text is the
 * ambush of the mercenaries turned bandits ($MeetB02, card 0x103) written again block for block
 * (its menus, its outcomes, its empty block 16 and all), so it was meant for that card's code:
 * here, half the times that card comes up, it loads $MeetF00 instead, and everything else is the
 * card's own (the chances, the saints and potions, the battle, the losses). Only the name it
 * loads changes, and in the battle the enemies' names: the mercenaries are the game's "Soldier"
 * (the same kind of enemy, the same numbers, another name) and the robber captain who leads them
 * a knight (a private war was a knight's to declare; the "Captain" of a party with more lore
 * stays). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "travel.h"

static int fehde_on(void) { return config_get_int("rules.fehde", 0) && !config_get_int("original", 0); }
static int hospice_on(void) { return config_get_int("rules.hospice", 0) && !config_get_int("original", 0); }

static int fehde;                           /* this ambush is the private war's */
static void meetb02_hook(void)              /* the mercenaries' ambush ($MeetB02), s108 1b3a:0008 */
{
    fehde = fehde_on() && (ns_call(GF_rand_mod, 1, 2) == 0 || getenv("DL_FEHDE"));   /* (DL_FEHDE: every time, for tests) */
    if (fehde) fprintf(stderr, "cutcards: Fehde! ($MeetF00 for $MeetB02)\n");
    hook_call_original();
    fehde = 0;
}

static void card_load_hook(void)            /* card_load (name, pic): the card's text by its name */
{
    if (fehde) {
        char nm[16];
        g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
        if (!strcasecmp(nm, "$MeetB02")) {
            static const char F[] = "$MeetF00";
            for (unsigned i = 0; i < sizeof F; i++) g_wb((u16)(GV_card_name + i), (u8)F[i]);
        }
    }
    hook_call_original();
}

static void battle_hook(void)               /* run_battle (field, seed, enemy, quality, count, leader, ...) */
{
    if (fehde) {
        if (rw(SS, (u16)(SP + 8)) == 0x0f) ww(SS, (u16)(SP + 8), 0x15);       /* Mercenary: Soldier */
        if (rw(SS, (u16)(SP + 14)) == 0x16) ww(SS, (u16)(SP + 14), 0x21);     /* Robber Capt: Knight */
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- the hospice
 * $CityH00 (7 texts) is a city's hospice by day, written and never coded: give alms, work a day
 * beside the nuns (Healing), have the party's wounds tended for a fee, or beg to be tended for
 * nothing; $CityH01 is a one-line placeholder for the night. The church district ($Churc00, s048
 * 1a88) has an empty third option line, switched off, whose handler does nothing (an early version
 * of the card, $Churc01, has "the $hospital, where injuries and the ill are tended" there). Here,
 * by day, that line leads to the hospice, run as a script (encounters.c) with the card's own texts. */
static const char HOSPICE[] =
    "name     The hospice\n"
    "card     $CityH00\n"
    "picture  XDAMONK\n"
    "terrain  any\n"
    "weight   0\n"
    "vars     cost, wage\n"
    "[start]\n"
    "block    0\n"
    "chosen   bestskill(Healing)\n"
    "do       set cost = max(12, wounds(End) * 2 + wounds(Str) * 8); money2 cost; money1 240\n"
    "option 0\n"                                            /* alms: a florin */
    "  need   money >= 240\n"
    "  chance none\n"
    "  ok     pay 240; text 1; attr party, DF, 1; reputation 1, 2; hours 1; disable 0\n"
    "option 1\n"                                            /* a day's work beside the nuns */
    "  who    chosen\n"
    "  chance none\n"
    "  ok     if skill(chosen, Healing) < 20 goto refused else goto work\n"
    "option 2\n"                                            /* the wounds tended, for a fee */
    "  need   wounds(End) + wounds(Str) > 0 and money >= cost\n"
    "  chance none\n"
    "  ok     pay cost; hours 24; heal party, End, 100; heal party, Str, 100; text 4;\n"
    "         set cost = 12; money2 cost\n"
    "option 3\n"                                            /* ... or for nothing, as paupers */
    "  need   wounds(End) + wounds(Str) > 0\n"
    "  who    bestattr(Cha)\n"
    "  chance clamp(5, 95, 40 + standing / 2 + maxattr(Cha) / 4 - money / 2400)\n"
    "  ok     hours 24; heal party, End, 100; heal party, Str, 50; text 5; disable 3;\n"
    "         set cost = max(12, wounds(End) * 2 + wounds(Str) * 8); money2 cost\n"
    "  fail   text 6; hours 1; disable 3\n"
    "option 4\n"                                            /* back to the church district */
    "  chance none\n"
    "  ok     card 0x13\n"
    "[refused]\n"                                           /* (no novices: the best healer has under 20) */
    "do       text 2; hours 1; disable 1\n"
    "[work]\n"
    "do       set wage = 24 + skill(chosen, Healing); money1 wage; text 3; money1 240; gain wage\n"
    "         practice chosen, Healing, 1, 40, 0; practice party, Healing, 1, 10, 0; hours 24; disable 1\n";

int cutcards_restored_on(void) { return hospice_on(); }   /* (the scripts of places written and never coded: the hospice) */
int cutcards_hospice_on(void) { return hospice_on(); }
int cutcards_scripts(const char **id, const char **text, int i)
{
    if (i != 0) return 0;
    *id = "HOSPICE"; *text = HOSPICE;
    return 1;
}
int encounters_card_of(const char *id);
static int card_is(const char *n) { char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm); return !strcasecmp(nm, n); }
static void churches_text_hook(void)        /* text_expand (idx, out): $Churc00's empty line by day: the hospice */
{
    int idx = (s16)rw(SS, (u16)(SP + 4));
    u16 out = rw(SS, (u16)(SP + 6));
    hook_call_original();
    if (ns_unwinding || idx != 0 || !hospice_on() || !card_is("$Churc00")) return;
    char t[1000]; int n = 0; u8 c;
    while (n < (int)sizeof t - 1 && (c = g_rb((u16)(out + n)))) t[n++] = (char)c;
    t[n] = 0;
    char *q = 0;
    for (char *r = t; (r = strchr(r, 0x15)); r++) if (r[1] == 0x14 || r[1] == '\r' || r[1] == '\n' || !r[1]) { q = r + 1; break; }
    if (!q) return;
    /* ($Churc01's words; its $hospital has no name in DARKLAND.CTY: the hospital of the Holy Spirit,
       as most German towns had one) */
    static const char line[] = "...\x1dvisit the Heilig-Geist-Spital, where injuries and the ill are tended.";
    if (n + (int)strlen(line) >= 880) return;
    memmove(q + strlen(line), q, strlen(q) + 1); memcpy(q, line, strlen(line));
    for (int i = 0; ; i++) { g_wb((u16)(out + i), (u8)t[i]); if (!t[i]) break; }
}
static void churches_options_hook(void)     /* card_options (block): the hospice's line on, by day */
{
    if (hospice_on() && (s16)rw(SS, (u16)(SP + 4)) == 0 && card_is("$Churc00") && encounters_card_of("HOSPICE") > 0) g_wb((u16)(GV_opt_on + 2), 1);
    hook_call_original();
}
static void churches_hospice_hook(void)     /* $Churc00 option 2 (s048 1a88:04c2): an empty handler */
{
    int card = hospice_on() ? encounters_card_of("HOSPICE") : -1;
    if (card <= 0) { hook_call_original(); return; }
    ns_call(GF_advance_hours, 1, 0);
    if (ns_unwinding) return;
    g_ww(GV_next_card, (u16)card);
    hook_return_far(0);
}

/* test: the next random encounter on the map is the game's card n (ugamecard:N) */
static int force_card = -1;
void cutcards_force(int n) { force_card = n; fprintf(stderr, "cutcards: the next random encounter is card %#x\n", n); }
static void overland_hook(void)             /* s030 1462:02c0 overland_loop(): the next card */
{
    hook_call_original();
    if (unw_active || force_card < 0 || (travel_step_result != 1000 && !getenv("DL_FORCE_NOW"))) return;   /* (DL_FORCE_NOW: at the next step, encounter or not) */
    AX = (u16)force_card; force_card = -1;
}

/* test: in a town, the next time its main street (card 6) or a district's street menu (cards 9, 0x0a) comes up, card n
   runs instead (ujumpcard:N) */
static int jump_card = -1;
void cutcards_jump(int n) { jump_card = n; fprintf(stderr, "cutcards: the town's next street menu is card %#x\n", n); }
static void street_hook(void)               /* s045 18fc:0000 (card 6), s046 18aa:0004 (card 9), 194b:0002 (card 0x0a) */
{
    if (jump_card < 0) { hook_call_original(); return; }
    int n = jump_card; jump_card = -1;
    g_ww(GV_next_card, (u16)n); AX = (u16)n;
    hook_return_far(0);
}

void cutcards_init(void)
{
    hook_install(45, 0x18fc, 0x0000, street_hook);
    hook_install(46, 0x18aa, 0x0004, street_hook);
    hook_install(46, 0x194b, 0x0002, street_hook);
    hook_install(GF_overland_loop, overland_hook);
    hook_install(GF_text_expand, churches_text_hook);
    hook_install(GF_card_options, churches_options_hook);
    hook_install(48, 0x1a88, 0x04c2, churches_hospice_hook);
    hook_install(108, 0x1b3a, 0x0008, meetb02_hook);
    hook_install(GF_card_load, card_load_hook);
    hook_install(GF_run_battle, battle_hook);
}
