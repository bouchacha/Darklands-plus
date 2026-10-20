/* cutcards.c - encounters MicroProse wrote and never used, put back (rules.cut_encounters;
 * the catalogue of what the game holds unused is docs/cut_content.md).
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

static int cut_on(void) { return config_get_int("rules.cut_encounters", 1) && !config_get_int("original", 0); }

static int fehde;                           /* this ambush is the private war's */
static void meetb02_hook(void)              /* the mercenaries' ambush ($MeetB02), s108 1b3a:0008 */
{
    fehde = cut_on() && (ns_call(GF_rand_mod, 1, 2) == 0 || getenv("DL_FEHDE"));   /* (DL_FEHDE: every time, for tests) */
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

/* test: the next random encounter on the map is the game's card n (ugamecard:N) */
static int force_card = -1;
void cutcards_force(int n) { force_card = n; fprintf(stderr, "cutcards: the next random encounter is card %#x\n", n); }
static void overland_hook(void)             /* s030 1462:02c0 overland_loop(): the next card */
{
    hook_call_original();
    if (unw_active || force_card < 0 || travel_step_result != 1000) return;
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
    hook_install(108, 0x1b3a, 0x0008, meetb02_hook);
    hook_install(GF_card_load, card_load_hook);
    hook_install(GF_run_battle, battle_hook);
}
