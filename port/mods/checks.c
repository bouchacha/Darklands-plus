/* checks.c - things that must never happen, watched while the game runs. Each is written to the
 * log once a session as a "check:" line with where it was seen (the card, the screen, the place,
 * the day), so that a playtest's log says what went wrong and when, not only what the player
 * noticed later. release_check.py and soaktest.py fail on any "check:" line. Cheap: a few reads a
 * frame. The checks are for the kinds of fault found so far:
 *  - memory written past the end of a buffer: the menu bar's titles (DS:9419.., set once at start;
 *    a card text past msg_buf's 900 bytes once turned "Game" into "MeetA01"), the members' item
 *    counts (one too many taken: a crash at the next death);
 *  - state that stuck: every step on the map an encounter (a night in camp's encounter waiting for
 *    a card it could replace, 1.4.6-RC6);
 *  - values out of range: the place, the party's size, the money, the date going backwards
 *    without a load, the game's 300 notes nearly full ("EVENT Failure"). */
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
    menus(); party(); notes(); calendar();
}
