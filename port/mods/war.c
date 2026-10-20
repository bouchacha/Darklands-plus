/* war.c - cities preparing for war, and at war: coded by MicroProse and never switched on
 * (rules.war, experimental, off in every preset; docs/cut_content.md, "Cities at war").
 *
 * Byte +0x14 of a city's location record has two states nothing in the game ever sets (and
 * DARKLAND.LOC has 0 for every city): 1, the city prepares for war, and 2, it is at war. The game
 * reads them all over: prices up a quarter or a half (s039 1462:0000), the lodging dearer at war,
 * the news ($Situa01: grumbling over prices; rats in the pot), Soldier's Road and its soldiers'
 * market open only then, the docks under guard or barricaded with no passage for strong arms
 * ($Docks00), no political season at war, and a ship's passenger lands at the war's own city gate
 * (card 0x73, from $Enter00): guards who turn wayfarers away ($CityG04, 19 texts), walls watched
 * by patrols that take the party for spies ($CityW03, $CityW04, $CityW05: chases, a sally port,
 * a sewer grating). This module:
 *  - gives a city such a time now and then: in a quarter of the year, one city in twelve prepares
 *    for war for two to four weeks, and half of those then stand a siege of one to three weeks
 *    (worked out from the date, the city and the party's seed: nothing more is saved; a city in a
 *    political season is left alone);
 *  - lets the party reach the war's city gate by land too (the location's arrival card, +0x0c,
 *    is that card while it lasts, as the game itself rewrites it for a village burnt down);
 *  - gives that card its text: it names $CityE00 and loads no card at all, so its menu was the
 *    last card's and its texts 7 to 9 did not exist; its code follows $CityE01 block for block
 *    (the city "seethes like an anthill", the night watch, "Spies trying to enter!", the dungeon),
 *    the text MicroProse wrote for it and never used;
 *  - puts right what the war's cards get wrong: a picture never drawn, option lines that ran the
 *    next one's choice, the patrols' enemy, and Soldiers' Road, which its quarter never showed.
 * Switched off, every city is put back at peace (+0x14 bits 1 and 2 cleared, the arrival card 4). */
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

int politics_unrest_at(int loc);

#define NCITY      0x5c
#define CARD_CITY  4                /* a city's arrival card (DARKLAND.LOC) */
#define CARD_WAR   0x73             /* the war's arrival card (s072 1981) */

static int war_on(void) { return config_get_int("rules.war", 0) && !config_get_int("original", 0); }

static int day_of_year(void)
{
    int m = (s16)g_rw(GV_cur_month), d = (s16)g_rw(GV_cur_day), n = 0;
    for (int k = 0; k < m && k < 12; k++) n += (s16)g_rw((u16)(GV_days_per_month + 2 * k));
    return n + d;
}
static u32 mix(u32 h)
{
    h ^= h >> 16; h *= 0x7feb352du; h ^= h >> 15; h *= 0x846ca68bu; h ^= h >> 16;
    return h;
}

/* the state of city loc today: 0 peace, 1 preparing for war, 2 at war; *left: days to go */
static signed char forced[NCITY];         /* test (uwar:loc,state): 1 + the state */
int war_state(int loc, int *left)
{
    if (left) *left = 0;
    if (!war_on() || loc < 0 || loc >= NCITY) return 0;
    if (forced[loc]) { if (left) *left = 7; return forced[loc] - 1; }
    int doy = day_of_year(), q = doy / 92, d = doy % 92;
    u32 h = mix((u32)loc * 0x9e3779b1u ^ (u32)(s16)g_rw(GV_cur_year) * 0x85ebca6bu ^ (u32)q * 0xc2b2ae35u ^ (u32)g_rw(GV_party_seed));
    if (h % 12) return 0;                                     /* one city in twelve, each quarter */
    int start = (int)(h >> 8) % 50, prep = 14 + (int)(h >> 14) % 15, siege = (h >> 20) & 1 ? 7 + (int)(h >> 21) % 15 : 0;
    if (d < start) return 0;
    if (d < start + prep) { if (left) *left = start + prep - d; return 1; }
    if (d < start + prep + siege) { if (left) *left = start + prep + siege - d; return 2; }
    return 0;
}

/* the cities' records as their states are today */
static void apply(void)
{
    int on = war_on();                                        /* (off: put back at peace, a save made with it on too) */
    for (int loc = 0; loc < NCITY; loc++) {
        u32 r = g_location(loc);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        if ((s16)rw(s, o) != 0) continue;                     /* (a city) */
        int st = on ? war_state(loc, 0) : 0;
        if (st && politics_unrest_at(loc)) st = 0;
        u8 f = rb(s, (u16)(o + 0x14)), nf = (u8)((f & ~3) | st);
        if (nf != f) {
            wb(s, (u16)(o + 0x14), nf);
            if (getenv("DL_WAR_LOG")) fprintf(stderr, "war: %d now %d\n", loc, st);
        }
        u16 card = rw(s, (u16)(o + 0x0c));
        if (st && card == CARD_CITY) ww(s, (u16)(o + 0x0c), CARD_WAR);
        else if (!st && card == CARD_WAR) ww(s, (u16)(o + 0x0c), CARD_CITY);
    }
}

static void overland_hook(void) { apply(); hook_call_original(); }        /* the map: arrival cards */
/* every card: the town's prices, news, streets. And the war's gate ($CityG04) asks for a picture
   that was never drawn ("gate4": the game stopped, "pics\gate4.pic: No such file"): the
   city gate's own (the one $CityG01 shows, pic_names +0xa57) instead */
static void card_load_hook(void)
{
    apply();
    char pic[16];
    u16 po = rw(SS, (u16)(SP + 6)), ps = rw(SS, (u16)(SP + 8));     /* (the picture: a far pointer after the name) */
    g_str(ps, po, pic, sizeof pic);
    if (!strcasecmp(pic, "gate4")) {
        u16 t = (u16)(LOADSEG + 0x2caf), o = rw(t, 0x0a57), sg = rw(t, 0x0a59);
        char gate[16] = "";
        if (sg) g_str(sg, o, gate, sizeof gate);
        if (gate[0]) for (int i = 0; i < 16; i++) { wb(ps, (u16)(po + i), (u8)gate[i]); if (!gate[i]) break; }
    }
    hook_call_original();
}

/* the walls: $CityW03 was given "... leaving your horses behind" twins of two of its options after
   its code was written (7 option lines for 5 handlers), so every line after the first twin ran
   the next one's handler (praying attacked, attacking ran away). The twins are taken out of the
   text as it is laid out: each line runs its own handler again. ($CityW04 has the same fault,
   three twins: the "cardopts" fix puts it right, mods/fixes.c) */
static int card_is(const char *n) { char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm); return !strcasecmp(nm, n); }
static void text_expand_hook(void)          /* text_expand (idx, out) */
{
    u16 out = rw(SS, (u16)(SP + 6));
    int idx = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    if (!war_on() || !card_is("$CityW03") || idx > 1) return;
    char t[2048]; int n = 0; u8 c;
    while (n < (int)sizeof t - 1 && (c = g_rb((u16)(out + n)))) t[n++] = (char)c;
    t[n] = 0;
    int changed = 0;
    for (char *p = t; (p = strchr(p, 0x1d)); ) {
        char *b = p; while (b > t && *b != 0x14) b--;           /* (the line: \x14, its mark, "...", \x1d, the words, \n) */
        char *e = strchr(p, '\n');
        char line[160]; int ln = (int)((e ? e : p + strlen(p)) - p); if (ln > 159) ln = 159;
        memcpy(line, p, (size_t)ln); line[ln] = 0;
        if (*b == 0x14 && strstr(line, "horses")) {
            if (e) memmove(b, e + 1, strlen(e + 1) + 1);
            else { if (b > t && b[-1] == '\n') b--; *b = 0; }
            changed = 1; p = b;
        } else p++;
    }
    if (changed) for (int i = 0; ; i++) { g_wb((u16)(out + i), (u8)t[i]); if (!t[i]) break; }
}

/* $CityW03 with no sally port in sight (its text 0) clears that option's switch only after the
   menu's switches were copied: "...hidden option" stood among the choices the first time */
/* $MilCr00, the arms-makers' quarter, switches Soldiers' Road off, then switches it off again
   unless the city prepares for war or is at war: it never came on */
static int war_here(void)
{
    int loc = (s16)g_rw(GV_cur_loc);
    u32 r = loc >= 0 && loc < NCITY ? g_location(loc) : 0;
    int f = r ? rb((u16)(r >> 16), (u16)((u16)r + 0x14)) : 0;
    return f == 1 || f == 2;                                  /* (as the card tests it: loc_query 0x20) */
}
/* ...and its Soldiers' Road, once open, shows the quarter's own menu for the night or the fair
   (its texts 1 and 2) as if it were a description before the soldiers' market: not shown */
static void card_text_hook(void)            /* card_text (block, how) */
{
    int b = (s16)rw(SS, (u16)(SP + 4));
    if (war_on() && (b == 1 || b == 2) && card_is("$MilCr00")) { hook_return_far(0); return; }
    hook_call_original();
}
static void card_options_hook(void)         /* card_options (block) */
{
    if (war_on() && (s16)rw(SS, (u16)(SP + 4)) == 0) {
        if (card_is("$CityW03")) g_wb((u16)(GV_opt_on + 1), 0);
        else if (card_is("$MilCr00") && war_here()) g_wb(GV_opt_on, 1);
    }
    hook_call_original();
}

/* the walls' patrols are "soldiers" and "guards" in every text, but their code fights the game's
   "Brigand" (8): the city's "Guard" (3) instead, as at the gate (an enemy of the same strength) */
static void battle_hook(void)               /* run_battle (field, seed, enemy, ...) */
{
    if (war_on() && rw(SS, (u16)(SP + 8)) == 8 && (card_is("$CityW03") || card_is("$CityW04") || card_is("$CityW05")))
        ww(SS, (u16)(SP + 8), 3);
    hook_call_original();
}

static void menu_log_hook(void)             /* (DL_WAR_LOG: the menu's choice, the texts shown) */
{
    hook_call_original();
    char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
    fprintf(stderr, "war: %s menu -> %d\n", nm, (s16)AX);
}
static void text_log_hook(void)
{
    char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
    fprintf(stderr, "war: %s text %d\n", nm, (s16)rw(SS, (u16)(SP + 4)));
    hook_call_original();
}

/* $CityW05, forcing the sally port before the soldiers come up: the chance of the strongest
   (s055 1932:034c) reads "member" D_8dfc, the highest Strength's value, for the member found
   with it (var_chosen1); and a potion thrown at the door (1932:0196) sets menu_slot = 2 and its
   chance to 100 before the roll (left from testing): always the fire potion's text, never the
   Thunderbolt's or the acid's, never a miss. Both as the card meant them */
static void w05_strength_hook(void)         /* s055 1932:034c () -> (chance, who) */
{
    if (!war_on()) { hook_call_original(); return; }
    int who = (s16)g_rw(GV_var_chosen1);
    int v = ns_call(GF_member_attr, 2, who, 1) / 3;
    AX = (u16)v; DX = (u16)who;
    hook_return_far(0);
}
static void w05_blast_hook(void)            /* s055 1932:0196 (chance) */
{
    if (!war_on()) { hook_call_original(); return; }
    int chance = (s16)rw(SS, (u16)(SP + 4)), slot = (s16)g_rw(GV_menu_slot), loc = (s16)g_rw(GV_cur_loc);
    if (slot < 0 || slot > 2) slot = 2;
    ns_call(GF_take_item_impl, 2, (s16)g_rw(GV_menu_member), (s16)g_rw((u16)(GV_card_items + 2 * slot)));
    ns_call(GF_event_renew, 10, -2, loc, 0, 0x13, 1, 0x63, 0, 0x18, 0, 0);
    if (ns_unwinding) return;
    int next;
    if (ns_call(GF_rand_mod, 1, 100) > chance) {
        ns_call(GF_card_text, 2, 4, 1);
        if (ns_unwinding) return;
        ns_call(GF_advance_hours, 1, 0);
        next = 0x76;
    } else {
        ns_call(GF_loc_reputation_adj, 3, loc, 0, -3);
        static const int TEXT[3] = { 1, 2, 3 }, RA[3] = { -1, 0, -1 }, RB[3] = { -6, -3, -4 };
        ns_call(GF_card_text, 2, TEXT[slot], 1);
        if (ns_unwinding) return;
        if (slot == 2) ns_call(GF_injure_impl, 4, -2, 0, 0, 5);
        ns_call(GF_loc_reputation_adj, 3, loc, RA[slot], RB[slot]);
        ns_call(GF_advance_hours, 1, 0);
        next = ns_call(GF_is_daylight, 0) ? 9 : 0xa;
    }
    if (ns_unwinding) return;
    g_ww(GV_next_card, (u16)next);
    AX = (u16)next;
    hook_return_far(0);
}

/* loc_hostile (s039 1462:00ba): hostile is an event 0x11 here, or a standing of -75 or less, or of
   -10 or less while the town is on alert (event 0x13: a fight at the gate, $Selec00; a rising,
   conflict_outcome; here a blasted sally port, $CityW05); the alert's test passes its arguments the
   wrong way round (event_at(loc, 0x13)), so it never held (found by Arkana Mechanika, Darklands
   Restoration). The fix cityalert (mods/fixes.c) does the same for every town; this keeps it for
   the war's cards when that fix is off. */
static void hostile_hook(void)
{
    hook_call_original();
    if (AX || !war_on() || ns_unwinding) return;
    u16 ax = AX, dx = DX;
    int loc = (s16)g_rw(GV_cur_loc);
    int alert = ns_call(GF_event_at, 2, 0x13, loc), rep = alert ? ns_call(GF_loc_reputation, 1, loc) : 0;
    if (ns_unwinding) return;
    AX = ax; DX = dx;
    if (alert && rep <= -10) AX = 1;
}

/* card 0x73's empty helper, run just before its menu: its card, $CityE01 */
static void war_gate_hook(void)             /* s072 1981:057e */
{
    if (war_on()) {
        static const char N[] = "$CityE01";
        for (unsigned i = 0; i < sizeof N; i++) g_wb((u16)(GV_card_name + i), (u8)N[i]);
        ns_call(GF_card_load, 3, GV_card_name, GV_pic_buf, G_DGROUP);
    }
    hook_return_far(0);
}

/* test: list the cities at war today; or force city loc into state st (-1: as the dates say) */
void war_test(int loc, int st)
{
    if (loc >= 0 && loc < NCITY) forced[loc] = (signed char)(st < 0 ? 0 : st + 1);
    apply();
    for (int l = 0; l < NCITY; l++) {
        int left, w = war_state(l, &left);
        u32 r = g_location(l);
        if (w && r) fprintf(stderr, "war: city %d state %d, %d days to go (+0x14 %02x, arrival card %#x)\n", l, w, left, rb((u16)(r >> 16), (u16)((u16)r + 0x14)), rw((u16)(r >> 16), (u16)((u16)r + 0x0c)));
    }
}

void war_init(void)
{
    hook_install(GF_overland_loop, overland_hook);
    hook_install(GF_card_load, card_load_hook);
    hook_install(GF_text_expand, text_expand_hook);
    hook_install(GF_card_options, card_options_hook);
    hook_install(GF_card_text, card_text_hook);
    hook_install(GF_run_battle, battle_hook);
    hook_install(55, 0x1932, 0x034c, w05_strength_hook);
    hook_install(55, 0x1932, 0x0196, w05_blast_hook);
    hook_install(GF_loc_hostile_impl, hostile_hook);
    if (getenv("DL_WAR_LOG")) { hook_install(GF_card_menu, menu_log_hook); hook_install(GF_card_text, text_log_hook); }
    if (!hook_install(72, 0x1981, 0x057e, war_gate_hook)) fprintf(stderr, "war: no hook for card 0x73\n");
}
