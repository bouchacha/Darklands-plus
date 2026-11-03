/* relics.c - relics as the manual has them (rules.relics, Darklands+), as the maintainer wants them:
 *
 *  - The manual (About Religion): "Possession of a relic may reduce the DF cost of praying to that
 *    saint. However, relics require the bearer to have sufficient virtue to gain this advantage."
 *    The game never did either. DARKLAND.LST keeps two numbers only relics have, which nothing
 *    read: +0x28, 6 to 80, read here as the Virtue the bearer needs, and +0x2a, 5 to 20, the Divine
 *    Favor the relic takes off a prayer. Off every prayer its bearer makes, to any saint (to its own
 *    saint alone was too narrow: his call, the third 1.4.5 test build), never below the least favor of 5 any saint
 *    asks; of several relics, the best. The bearer: of a weapon relic, the member wielding it; of
 *    any other, the member carrying it (with the shared pack, those are each member's own, as the
 *    potions: gm_member_own). Done in each saint's function (s039 165c, the table saint_fns, far
 *    290e:2937): asked its least favor (what 2) it answers that much less, asked the chance (what 7)
 *    it counts the favor spent as that much more, so every caller (the prayer screens, the Saints
 *    page, the cards' saint options, the margins) sees it, and the favor taken is what is spent.
 *  - A weapon relic is readied (item_ready, s031 18e7:11b2: the sheets, the game's own too) only by
 *    a member with the Virtue it asks.
 *  - Relics are immune from any quality degradation: the corroding water of a dragon's cursed land ($Drago01) lowers
 *    each weapon's quality, a weapon relic's too; acid and fire wear armour in a battle (s019
 *    1ef8:0968). A relic below its own quality (DARKLAND.LST +0x26) is made whole again (with the
 *    member's record of it in hand), and battle wear passes a relic by.
 *  - A cathedral's "donate one of your relics" (s056 19c4:0c6e, $Cathe00 and $Cathe01) gave the
 *    first relic the party's records held, without saying what giving it does. The player now picks
 *    which one (or none: back to the cathedral) in a box, shown for a single relic too, that says
 *    what each gives and what giving does (mods/choose.c): the town's opinion +30 (loc_reputation_adj(
 *    here, 30, 30): by a chance of 100 less its opinion's size), every member's Divine Favor in full
 *    (add_attr(-2, DF, 99)), three hours. The game's own routine then gives that one: its search for the first
 *    relic (s031 18e7:0fac) answers the one picked, and its taking of one (18e7:0e14) takes that
 *    one, from whoever carries it, who is then the $ChosenOneName of the text. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "gamemodel.h"
#include "nscreen.h"
#include "choose.h"
#include "relics.h"

int hook_point(int sect, unsigned seg, unsigned off);

static int on = 1;
int relics_on(void) { return on && !config_get_int("original", 0); }

#define RELIC_FIRST 0x96
#define RELIC_LAST  0xab
/* each saint's function in s039 165c (the vector saint_fns[i] leads to), and the least favor it asks (its table's t[2]) */
#define NSAINT 136
static const u16 SAINT_OFF[NSAINT] = {
    0x0034, 0x0190, 0x024c, 0x030c, 0x038e, 0x047e, 0x0536, 0x0604, 0x06a2, 0x0792, 0x0898, 0x09a2, 0x0b82,
    0x0c20, 0x0cbe, 0x0d5c, 0x0eb6, 0x0f6e, 0x1024, 0x10ca, 0x125a, 0x133a, 0x13d4, 0x14a8, 0x152a, 0x15e2,
    0x16b6, 0x17a4, 0x1868, 0x18ea, 0x1988, 0x1a18, 0x1b02, 0x1bac, 0x1c54, 0x1cf2, 0x1de2, 0x1eb6, 0x1f8c,
    0x2052, 0x2142, 0x2208, 0x23fe, 0x249a, 0x2538, 0x2628, 0x26d6, 0x27c2, 0x28b0, 0x296a, 0x2a24, 0x2b30,
    0x76d4, 0x2c2a, 0x2cc8, 0x2d58, 0x2ece, 0x2f82, 0x307e, 0x3186, 0x3274, 0x3320, 0x33d6, 0x353e, 0x3612,
    0x36d6, 0x3816, 0x38b4, 0x3934, 0x39b4, 0x3abc, 0x3c32, 0x3cec, 0x74ea, 0x3da4, 0x3e0a, 0x3eec, 0x3fc2,
    0x41fa, 0x4320, 0x43d8, 0x4608, 0x46c2, 0x47b0, 0x489c, 0x4962, 0x49e4, 0x4b0a, 0x4ba8, 0x4c46, 0x4d1a,
    0x4f76, 0x502e, 0x50cc, 0x51ba, 0x5296, 0x5334, 0x540a, 0x54c4, 0x563a, 0x5724, 0x57de, 0x5952, 0x5a84,
    0x5b82, 0x5cca, 0x5d6c, 0x5e5c, 0x5ede, 0x6008, 0x6122, 0x61aa, 0x627e, 0x6338, 0x642c, 0x64f4, 0x65ac,
    0x664a, 0x671e, 0x67f2, 0x68f4, 0x69ac, 0x6a4a, 0x6ae8, 0x78ca, 0x6bb8, 0x6ca6, 0x6d92, 0x6e62, 0x6f0c,
    0x7038, 0x710a, 0x71ce, 0x7288, 0x7342, 0x7432 };
static const unsigned char SAINT_LEAST[NSAINT] = {
    15, 20, 10, 15, 25, 15, 10, 5, 20, 20, 10, 15, 20, 20, 15, 10, 30, 10, 25, 15, 10, 50, 15, 30, 10, 15,
    20, 15, 25, 25, 25, 10, 25, 20, 20, 30, 20, 20, 15, 30, 20, 20, 10, 20, 15, 15, 20, 30, 25, 30, 40, 25,
    10, 30, 40, 40, 40, 35, 35, 15, 15, 25, 25, 30, 15, 30, 20, 15, 25, 50, 20, 10, 10, 12, 35, 10, 10, 40,
    15, 35, 20, 40, 20, 25, 30, 50, 30, 40, 20, 35, 25, 25, 25, 25, 20, 30, 25, 35, 10, 25, 20, 30, 15, 25,
    25, 27, 20, 40, 25, 25, 10, 40, 10, 10, 25, 40, 25, 15, 40, 20, 30, 40, 10, 25, 10, 15, 10, 30, 40, 35,
    15, 20, 25, 15, 20, 10 };
#define FLOOR_FAVOR 5                         /* (the least favor any saint asks: no relic takes a prayer lower) */

static int def_word(int code, int off) { int v = gm_itemdef_u16(code, off); return v < 0 ? 0 : (s16)v; }   /* (DARKLAND.LST's: gamemodel.h) */
int relic_is(int code)
{
    if (code < RELIC_FIRST || code > RELIC_LAST) return 0;
    return (def_word(code, 0x20) & 0x1000) != 0;                          /* (DARKLAND.LST +0x21 bit 4: a relic) */
}
int relic_is_weapon(int code) { int t = def_word(code, 0x1e); return relic_is(code) && t >= 0 && t <= 62; }   /* (its item type a weapon's) */
int relic_virtue(int code) { return relic_is(code) ? def_word(code, 0x28) : 0; }
int relic_favor(int code) { return relic_is(code) ? def_word(code, 0x2a) : 0; }
static int relic_quality(int code) { int q = gm_itemdef_u8(code, 0x26); return q < 0 ? 0 : q; }
static int virtue_of(int m) { return (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + 9)); }
int relic_member_virtue(int m) { return m >= 0 && m < 5 ? virtue_of(m) : 0; }
int relic_bearer_ok(int m, int code) { return relic_is(code) && m >= 0 && m < 5 && virtue_of(m) >= relic_virtue(code); }

int relic_discount(int who, int saint, int *code)
{
    if (code) *code = 0;
    if (!relics_on() || who < 0 || who > 4 || saint < 0 || saint >= NSAINT || !gm_member_present(who)) return 0;
    int best = 0, bc = 0, room = SAINT_LEAST[saint] - FLOOR_FAVOR;
    for (int i = 0; i < gm_item_count(who); i++) {
        GmItem g;
        if (!gm_item(who, i, &g) || !relic_bearer_ok(who, g.id)) continue;
        if (relic_is_weapon(g.id) && !gm_item_equipped(who, i)) continue;   /* (a weapon relic helps the one who wields it) */
        int d = relic_favor(g.id);
        if (d > best) { best = d; bc = g.id; }
    }
    if (best > room) best = room;
    if (best <= 0) return 0;
    if (code) *code = bc;
    return best;
}

/* ---------------------------------------------------------------- the saints' functions */
static int saint_hook_at[NSAINT];
static void saint_hook(void)                  /* a saint's function (who, favor, what), s039 165c */
{
    int who = (s16)rw(SS, (u16)(SP + 4)), x = (s16)rw(SS, (u16)(SP + 6)), what = (s16)rw(SS, (u16)(SP + 8)), k = -1;
    if (what != 2 && what != 7) { hook_call_original(); return; }
    for (int i = 0; i < NSAINT; i++) if (saint_hook_at[i] == hook_current) { k = i; break; }
    int d = k >= 0 ? relic_discount(who, k, 0) : 0;
    if (!d) { hook_call_original(); return; }
    if (what == 7) {                                         /* the chance: the favor spent counts that much more */
        u16 at = (u16)(SP + 6);                              /* (taken before: the call's return pops 4 bytes, and SP + 6 after it
                                                                is the caller's saved BP - a prayer by a relic's bearer came back
                                                                to saint_pray with BP the favor, and the game crashed) */
        ww(SS, at, (u16)(x + d));
        hook_call_original();
        if (unw_active) return;
        ww(SS, at, (u16)x);
    } else {                                                 /* the least favor: that much less */
        hook_call_original();
        if (unw_active) return;
        int least = (s16)AX - d;
        AX = (u16)(least < 0 ? 0 : least);
    }
}

/* ---------------------------------------------------------------- a weapon relic asks its Virtue */
static void ready_hook(void)                  /* item_ready (who, idx), s031 18e7:11b2 */
{
    int who = (s16)rw(SS, (u16)(SP + 4)), idx = (s16)rw(SS, (u16)(SP + 6));
    GmItem g;
    if (relics_on() && who >= 0 && who < 5 && gm_item(who, idx, &g) && relic_is_weapon(g.id) && !relic_bearer_ok(who, g.id)) {
        fprintf(stderr, "relics: member %d's Virtue %d is below the %d relic %#x asks: not readied\n", who, virtue_of(who), relic_virtue(g.id), g.id);
        hook_return_far(0);
        return;
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- immune from quality degradation */
static const u8 USE_T[5] = { 0x3d, 0x0e, 0x37, 0x38, 0x48 }, USE_Q[5] = { 0x44, 0x46, 0x3b, 0x3c, 0x47 };   /* (the member's things in use: type, quality; gamemodel.c) */
void relics_frame(void)
{
    if (!relics_on() || game_mode() == GM_BATTLE || game_mode() == GM_OTHER || (ns_screen != NS_NONE && ns_screen != NS_CARD)) return;
    for (int m = 0; m < 5; m++) {
        if (!gm_member_present(m)) continue;
        for (int i = 0; i < gm_item_count(m); i++) {
            GmItem g;
            if (!gm_item(m, i, &g) || !relic_is(g.id)) continue;
            int q = relic_quality(g.id);
            if (g.qual >= q || q <= 0 || q > 99) continue;
            u16 b = G_MEMBER(m);
            for (int k = 0; k < 5; k++)                      /* (in hand: the member's record of it follows, or it would no longer count as in hand) */
                if (g_rb((u16)(b + USE_T[k])) == (u8)g.type && g_rb((u16)(b + USE_Q[k])) == (u8)g.qual) g_wb((u16)(b + USE_Q[k]), (u8)q);
            wb(G_FAR2E38, (u16)(G_ITEM(m, i) + 3), (u8)q);
            fprintf(stderr, "relics: member %d's relic %#x made whole (quality %d, was %d)\n", m, g.id, q, g.qual);
        }
    }
}
static void wear_hook(void)                   /* s019 1ef8:0968 (combatant, item, quality, amount): battle wear */
{
    int code = (s16)rw(SS, (u16)(SP + 6));
    if (relics_on() && relic_is(code)) { hook_return_far(0); return; }
    hook_call_original();
}

/* ---------------------------------------------------------------- the cathedral's gift */
static int give_m = -1, give_i = -1, give_code;    /* the relic picked, while the game's routine gives it */
static void find_hook(void)                   /* s031 18e7:0fac (who, long mask): the first thing with these flags */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    long mask = (long)rw(SS, (u16)(SP + 6)) | ((long)rw(SS, (u16)(SP + 8)) << 16);
    if (give_m < 0 || who != -2 || mask != 0x1000) { hook_call_original(); return; }
    g_ww(GV_var_chosen1, (u16)give_m);                      /* (the one who carries it lays it on the altar) */
    AX = (u16)give_code; DX = 0;
    hook_return_far(0);
}
static void take_hook(void)                   /* s031 18e7:0e14 (who, long mask): take one thing with these flags */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    long mask = (long)rw(SS, (u16)(SP + 6)) | ((long)rw(SS, (u16)(SP + 8)) << 16);
    if (give_m < 0 || who != -2 || mask != 0x1000) { hook_call_original(); return; }
    int m = give_m, i = give_i;
    give_m = give_i = -1;
    ns_call(31, 0x18e7, 0x0474, 2, m, i);                   /* (one of that record used up: put away and gone when it was the last) */
    if (ns_unwinding) return;
    AX = (u16)gm_item_count(m); DX = 0;
    hook_return_far(0);
}
static void gift_hook(void)                   /* s056 19c4:0c6e: "donate one of your relics" */
{
    if (!relics_on() || !ns_active()) { hook_call_original(); return; }
    ChooseRow rows[CHOOSE_MAX]; int n = 0;
    for (int m = 0; m < 5 && n < CHOOSE_MAX; m++) {
        if (!gm_member_present(m)) continue;
        for (int i = 0; i < gm_item_count(m) && n < CHOOSE_MAX; i++) {
            GmItem g;
            if (!gm_item(m, i, &g) || !relic_is(g.id)) continue;
            ChooseRow *r = &rows[n++];
            memset(r, 0, sizeof *r);
            r->id = g.id; r->qual = g.qual; r->member = m; r->index = i;
            int v = relic_virtue(g.id), have = virtue_of(m);
            char low[24] = "";
            if (have < v) snprintf(low, sizeof low, " (has %d)", have);          /* (who: the head at the right) */
            snprintf(r->info, sizeof r->info, "%sprayers cost {Divine Favor:%d} less, with {Virtue:%d}%s",
                     relic_is_weapon(g.id) ? "Wielded: " : "", relic_favor(g.id), v, low);
            if (r->info[0] == 'p') r->info[0] = 'P';
            r->bad = have < v;
        }
    }
    if (!n) { hook_call_original(); return; }                /* (none: the game's own words for it) */
    int c = choose_run("Which relic do you lay on the altar?",
                       "The relic stays with the cathedral. With a chance of 100 less the town's present opinion of you (for or against), "
                       "that opinion rises by 30; every member's {Divine Favor} is restored in full, and three hours pass.",
                       rows, n, "Keep them all");
    if (ns_unwinding) return;
    if (c < 0) {                                             /* none: back to the cathedral's own menu */
        u16 card = g_rw(0xa772);                            /* (the card on show: the option's function returns the next one, and sets it) */
        g_ww(GV_next_card, card); AX = card; DX = 0;
        fprintf(stderr, "relics: no relic given at the cathedral\n");
        hook_return_far(0);
        return;
    }
    give_m = rows[c].member; give_i = rows[c].index; give_code = rows[c].id;
    fprintf(stderr, "relics: member %d gives relic %#x at the cathedral\n", give_m, give_code);
    hook_call_original();
    give_m = give_i = -1;
}

/* ---------------------------------------------------------------- on and off */
void relics_set(int v)
{
    on = v;
    for (int i = 0; i < NSAINT; i++) {
        if (v) { hook_install(39, 0x165c, SAINT_OFF[i], saint_hook); saint_hook_at[i] = hook_point(39, 0x165c, SAINT_OFF[i]); }
        else hook_remove(39, 0x165c, SAINT_OFF[i], saint_hook);
    }
    static const struct { int sect; u16 seg, off; void (*fn)(void); } H[] = {
        { 19, 0x1ef8, 0x0968, wear_hook }, { 56, 0x19c4, 0x0c6e, gift_hook }, { 31, 0x18e7, 0x0fac, find_hook },
        { 31, 0x18e7, 0x0e14, take_hook }, { 31, 0x18e7, 0x11b2, ready_hook } };
    for (size_t k = 0; k < sizeof H / sizeof H[0]; k++) {
        if (v) hook_install(H[k].sect, H[k].seg, H[k].off, H[k].fn);
        else hook_remove(H[k].sect, H[k].seg, H[k].off, H[k].fn);
    }
}
void relics_init(void) { relics_set(config_get_int("rules.relics", 1)); }
