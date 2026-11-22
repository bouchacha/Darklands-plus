/* checks.c - things that must never happen, watched while the game runs. Each is written to the
 * log once a session as a "check:" line with where it was seen (the card, the screen, the place,
 * the day), so that a playtest's log says what went wrong and when, not only what the player
 * noticed later. release_check.py and soaktest.py fail on any "check:" line. Cheap: a few reads a
 * frame.
 *
 * The checks are for the kinds of fault found so far:
 *  - memory written past the end of a buffer: the menu bar's titles (DS:9419.., set once at start;
 *    a card text past msg_buf's 900 bytes once turned "Game" into "MeetA01"), the members' item
 *    counts (one too many taken: a crash at the next death);
 *  - state that stuck: every step on the map an encounter (a night in camp's encounter waiting for
 *    a card it could replace, 1.4.6-RC6);
 *  - values out of range: the place, the party's size, the money, the date going backwards
 *    without a load, the game's 300 notes nearly full ("EVENT Failure");
 *  - a boost whose end was lost: an attribute above its maximum and its running boosts (CK_BOOST;
 *    his Jorg's Perception 60 of 33);
 *  - the game reading memory it has freed: its item list's pointers leading anywhere but to
 *    DARKLAND.LST's records (CK_ITEMLIST), its places' to anything but a place (CK_PLACES;
 *    mods/nscreen.c keeps both over a battle), and what that spoilt: a member's load other than
 *    the weights of what is equipped (CK_LOAD; his Dietrich's 198). */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "travel.h"
#include "gamemodel.h"
#include "datamods.h"
#include "checks.h"

extern unsigned saves_loaded_ms;
int saves_load_under_way(void);

static unsigned char said[64];
void check_fail(int kind, const char *fmt, ...)
{
    if (kind < 0 || kind >= (int)sizeof said || said[kind]) return;
    said[kind] = 1;
    char what[400]; va_list ap; va_start(ap, fmt); vsnprintf(what, sizeof what, fmt, ap); va_end(ap);
    char card[16] = ""; g_str(G_DGROUP, GV_card_name, card, sizeof card);
    int px = -1, py = -1; travel_party_tile(&px, &py);
    fprintf(stderr, "check: %s (mode %d, screen %d, card %s, place %d, tile %d,%d, day %d hour %d)\n", what, game_mode(), ns_screen, card,
            (s16)g_rw(G_CUR_LOC), px, py, (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR));
    fflush(stderr);
    { static int told; void ui_check_noticed(const char *); if (!told++) ui_check_noticed(what); }   /* (ui/report.cpp: a report made, the player told) */
}

/* the menu bar's titles: near pointers the game sets once (s000 0e76:083e) */
static u16 titles[4]; static int titles_known;
static void menus(void)
{
    if (!titles_known) {
        char g[8]; g_str(G_DGROUP, g_rw(0x9419), g, sizeof g);
        if (strcmp(g, "Game")) return;
        for (int k = 0; k < 4; k++) titles[k] = g_rw((u16)(0x9419 + 10 * k));
        titles_known = 1;
        return;
    }
    for (int k = 0; k < 4; k++)
        if (g_rw((u16)(0x9419 + 10 * k)) != titles[k]) {
            char t[24]; g_str(G_DGROUP, g_rw((u16)(0x9419 + 10 * k)), t, sizeof t);
            check_fail(CK_MENU, "the menu bar's title %d is now \"%s\" (memory written past a buffer?)", k, t);
            return;
        }
}

u32 plat_ticks_ms(void);
/* a value out of range for two seconds on end: told (a load, the start, a game's own sum take a few frames
   between values; the opening has no party yet) */
static int lasting(int kind, int bad)
{
    static u32 since[16];
    if (!bad) { since[kind] = 0; return 0; }
    if (!since[kind]) since[kind] = plat_ticks_ms() | 1;
    return plat_ticks_ms() - since[kind] >= 2000;
}

static void party(void)
{
    static int armed;                                         /* (a party seen since the title: the opening and the start's
                                                                 cards have none - Windows took over two seconds there) */
    int mode = game_mode(), size = (s16)g_rw(G_PARTY_SIZE), items = 0, im = -1;
    if (mode == GM_OTHER) armed = 0;
    if (mode != GM_CARD && mode != GM_MAP && mode != GM_BATTLE) { lasting(CK_PARTY, 0); lasting(CK_ITEMS, 0); lasting(CK_MONEY, 0); lasting(CK_LOC, 0); return; }
    if (size >= 1 && size <= 5) armed = 1;
    if (!armed) return;
    if (lasting(CK_PARTY, size < 1 || size > 5)) check_fail(CK_PARTY, "the party's size is %d", size);
    if (size < 1 || size > 5) return;
    for (int m = 0; m < size && im < 0; m++) {
        int n = (s8)g_rb((u16)(G_MEMBER(m) + 0x6a));
        if (g_rb(G_MEMBER(m)) && (n < 0 || n > 64)) { im = m; items = n; }
    }
    if (lasting(CK_ITEMS, im >= 0)) check_fail(CK_ITEMS, "member %d has %d items", im, items);
    int neg = (s16)g_rw((u16)(G_MONEY + 2)) < 0 || (s16)g_rw((u16)(G_MONEY + 4)) < 0;   /* (groschen, pfennigs; florins past 32767 are read as a sum elsewhere) */
    if (lasting(CK_MONEY, neg)) check_fail(CK_MONEY, "the party's money is below 0 (%d %d %d)", (s16)g_rw(G_MONEY), (s16)g_rw(G_MONEY + 2), (s16)g_rw(G_MONEY + 4));
    int loc = (s16)g_rw(G_CUR_LOC);
    if (lasting(CK_LOC, loc < -1 || loc >= G_NLOC)) check_fail(CK_LOC, "the current place is %d", loc);
}

/* the item list and the loads reckoned from it, once a second or so. Not for five seconds after a load, and
   never again once the mods in use have changed (the game reads a mod's item list at its next world load,
   Darklands+ at once: the two differ until then) */
int gamedata_item_raw(int id, int off, int size);
static u8 right[5][8]; static int edited;                 /* (below: an attribute 0..6 or the load, right once) */
void checks_edited(void) { edited = 1; }                  /* (checks.h: F12's editor wrote a member) */
static void items(void)
{
    static u32 t; static int gen = -1;
    if (plat_ticks_ms() - t < 1000) return;
    t = plat_ticks_ms();
    int mode = game_mode(), size = (s16)g_rw(G_PARTY_SIZE);
    if (gen == -1) gen = datamods_generation();
    if ((mode != GM_CARD && mode != GM_MAP && mode != GM_BATTLE) || gen != datamods_generation() || size < 1 || size > 5 ||
        saves_load_under_way() || (saves_loaded_ms && plat_ticks_ms() - saves_loaded_ms < 5000)) { lasting(CK_ITEMLIST, 0); lasting(CK_LOAD, 0); lasting(CK_PLACES, 0); lasting(CK_BOOST, 0); return; }
    int bad = -1;
    for (int id = 0; bad < 0 && gamedata_item_raw(id, 0, 1) >= 0; id++) {
        u32 d = g_itemdef(id);
        for (int k = 0; bad < 0 && k < 46; k++) if (!d || rb((u16)(d >> 16), (u16)((u16)d + k)) != gamedata_item_raw(id, k, 1)) bad = id;
    }
    if (lasting(CK_ITEMLIST, bad >= 0)) {
        u32 d = g_itemdef(bad);
        check_fail(CK_ITEMLIST, "the game's record of item %d (at %04x:%04x) is not DARKLAND.LST's (in a battle: %d): freed memory read?", bad, (unsigned)(d >> 16), (unsigned)(u16)d, g_rb(GV_in_battle) == 1);
    }
    int place = -1;                                           /* (a place: a kind, a tile of the map, a name) */
    for (int k = 0; k < G_NLOC && place < 0; k++) {
        u32 r = g_location(k); u16 sg = (u16)(r >> 16), of = (u16)r;
        if (!r || rw(sg, of) >= 64 || rw(sg, (u16)(of + 4)) >= 340 || rw(sg, (u16)(of + 6)) >= 950 || rb(sg, (u16)(of + 0x26)) < 0x20) place = k;
    }
    if (lasting(CK_PLACES, place >= 0)) {
        u32 r = g_location(place);
        check_fail(CK_PLACES, "the game's record of place %d (at %04x:%04x) is no place (in a battle: %d): freed memory read?", place, (unsigned)(r >> 16), (unsigned)(u16)r, g_rb(GV_in_battle) == 1);
    }
    /* a member's load is what is equipped, and no attribute stands above its maximum and its running boosts. Each
       counts once it has been right for that member since the game was loaded or F12's editor last wrote: the
       start's ready-made party comes with loads one off its own sum (its first member's 57 for 56) until something
       is equipped, a companion joins with 10, and a member taken back from an inn may bring a boost whose end the
       game left behind (party_remove, s037 1462:3b58) */
    static unsigned who[5], load_seen;
    if (load_seen != saves_loaded_ms || edited) { load_seen = saves_loaded_ms; edited = 0; memset(right, 0, sizeof right); }
    int lm = -1, bm = -1, ba = 0, run[5][7], known = gm_boosts_running(run);
    for (int m = 0; m < 5; m++) {
        char nm[40]; unsigned h = 0;
        gm_member_name(m, nm, sizeof nm);
        for (const char *c = nm; *c; c++) h = h * 31 + (unsigned char)*c;
        int here = m < size && g_rb(G_MEMBER(m)), same = here && gm_member_load(m) == (gm_member_equip_load(m) & 0xff);
        if (!here || h != who[m]) memset(right[m], 0, sizeof right[m]);
        who[m] = h;
        if (same) right[m][7] = 1;
        else if (here && right[m][7] && lm < 0) lm = m;
        for (int a = 0; a < 7 && here && known; a++) {
            int cur, max; gm_member_attr(m, a, &cur, &max);
            if (max <= 0 || cur <= max + run[m][a]) right[m][a] = 1;
            else if (right[m][a] && bm < 0) { bm = m; ba = a; }
        }
    }
    if (bad < 0 && lasting(CK_LOAD, lm >= 0))
        check_fail(CK_LOAD, "member %d's load is %d, but the things equipped weigh %d", lm, gm_member_load(lm), gm_member_equip_load(lm));
    if (lasting(CK_BOOST, bm >= 0)) {
        int cur, max; gm_member_attr(bm, ba, &cur, &max);
        check_fail(CK_BOOST, "member %d's %s is %d, above its maximum %d and its running boosts %d: a boost's end lost", bm, gm_attr_name(ba), cur, max, run[bm][ba]);
    }
}

static void notes(void)
{
    static unsigned t; if (++t % 60) return;                 /* (once a second or so) */
    int n = 0;
    for (int i = 0; i < G_NEVENTS; i++) if (g_event(i)) n++;
    if (n >= 285) check_fail(CK_EVENTS, "%d of the game's %d notes in use", n, G_NEVENTS);
}

static void calendar(void)
{
    /* (a load reads the date before it says it has loaded: none is looked at while one of ours is under way nor
       for five seconds after any load, and a step back is told only when no load followed within five seconds -
       the original's own Load. The soak test caught a quickload taken for a step back twice, 2026-11-04.) */
    static long last = -1, back_from, back_to; static unsigned load_seen; static u32 back_t, other_t, load_t;
    if (game_mode() == GM_OTHER) { other_t = plat_ticks_ms() | 1; last = -1; back_t = 0; return; }
    if (saves_load_under_way()) { last = -1; back_t = 0; return; }
    if (other_t && plat_ticks_ms() - other_t < 30000) { last = -1; return; }   /* (a new game, the title: the start's own date settles) */
    long now = (long)(s16)g_rw(G_CUR_YEAR) * 100000L + (long)(s16)g_rw(G_CUR_MONTH) * 5000L + (long)(s16)g_rw(G_CUR_DAY) * 100L + (s16)g_rw(G_CUR_HOUR);
    { static int lg = -1; static long shown = -1; if (lg < 0) lg = getenv("DL_CHECKS_LOG") != 0;
      if (lg && now != shown) { fprintf(stderr, "checks: date %ld (mode %d, load %u, loading %d)\n", now, game_mode(), saves_loaded_ms, saves_load_under_way()); shown = now; } }
    if (load_seen != saves_loaded_ms) { load_seen = saves_loaded_ms; load_t = plat_ticks_ms() | 1; }
    if (load_t && plat_ticks_ms() - load_t < 5000) { last = now; back_t = 0; return; }
    if (back_t && now >= back_from) back_t = 0;                 /* (back where it was: a moment's reading, not a step back) */
    if (last >= 0 && now < last && !back_t) { back_t = plat_ticks_ms() | 1; back_from = last; back_to = now; }
    if (back_t && plat_ticks_ms() - back_t > 5000) { back_t = 0; check_fail(CK_TIME, "the date went backwards (%ld to %ld) without a load", back_from, back_to); }
    last = now;
}

/* each step on the map (mods/travel.c): an encounter at every one of many steps in a row */
void checks_map_step(int result, int moved)
{
    static int run; static unsigned load_seen;
    if (load_seen != saves_loaded_ms) { load_seen = saves_loaded_ms; run = 0; }   /* (a load: the steps before it are another game's; the encounters sweep loaded before each one) */
    if (result == 1000 || result == 2000) run++;
    else if (moved) run = 0;
    if (run == 12) check_fail(CK_ENCOUNTERS, "12 steps on the map in a row each brought an encounter (one held waiting?)");
}

void checks_frame(void)
{
    if (config_get_int("original", 0)) return;
    menus(); party(); items(); notes(); calendar();
}
