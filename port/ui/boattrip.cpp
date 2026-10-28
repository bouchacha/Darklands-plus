// boattrip.cpp - by boat from the world map. The game's boats ($Docks00, s051 19f2:0006; the arrival
// $Enter00, s066 19ec:000e) as they are:
//   - a town's docks list the towns its boats go to (DARKLAND.CTY +0x4a..+0x50; a fifth, +0x52, only for
//     those already aboard); boat k sails on the days when (loc_query(town, 0x21) + day of the month + k)
//     is a multiple of 3 (4 while the town prepares for war, loc_query 0x20 bit 1; at war, bit 2, the docks
//     are under the army and an arriving boat goes on to one town of four at random);
//   - boarding takes 1 to 3 hours and the voyage 48 to 95 more; arrived, "remain aboard and continue"
//     to a town whose boat sails today takes 0 to 11 hours (and no fee);
//   - a boat that does not sail today cannot be boarded: the party waits in town (here: a stay at the
//     inn, a day at a time).
// The way: the towns in between are tried (up to six boats), each way's time taken from 400 journeys
// played through with those rules from today (the hours aboard at random), the quickest on average kept.
// The journey: the walk to the docks (Go to, ui/citynav.cpp), a stay at the inn first when the boat
// sails on a later day, the boat chosen, and at each stop "remain aboard" while the boat on sails today;
// it stops, saying why, where the player must choose (going ashore at the end, a boat on that does not
// sail today, anything else the voyage brings: the river pirates, an encounter).
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
#include "boattrip.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../mods/nscreen.h"
#include "../mods/lodgeview.h"
#include "../mods/travel.h"
#include "../mods/assist.h"
u32 plat_ticks_ms(void);
void ui_notice(const char *msg);
int citynav_busy(void);
void citynav_go_place(const char *label);
int citynav_choose_option(int opt);
const char *citynav_card_name(void);
void citynav_read_on(void);
int citynav_choose_text(const char *words);
}

static int blog = -1;
#define LOG(...) do { if (blog < 0) blog = getenv("DL_BOAT_LOG") != 0; if (blog) fprintf(stderr, "boat: " __VA_ARGS__); } while (0)

/* ---------------------------------------------------------------- the calendar (s029 1367: no leap years) */
static const int MDAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
struct Date { int hour, day, month; };                     /* day 1.., month 0..11 */
static void add_hours(Date &d, int h)
{
    d.hour += h;
    while (d.hour >= 24) { d.hour -= 24; d.day++; }
    while (d.day > MDAYS[d.month % 12]) { d.day -= MDAYS[d.month % 12]; d.month = (d.month + 1) % 12; }
}
static long purse(void) { return (s16)g_rw(G_MONEY) * 240L + (s16)g_rw((u16)(G_MONEY + 2)) * 12L + (s16)g_rw((u16)(G_MONEY + 4)); }   /* (pfennigs) */
static Date today(void) { return Date{ (s16)g_rw(G_CUR_HOUR), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_MONTH) }; }

/* ---------------------------------------------------------------- the boats' days */
static int war_of(int loc) { u32 r = g_location(loc); return r ? rb((u16)(r >> 16), (u16)((u16)r + 0x14)) : 0; }   /* (loc_query 0x20) */
static int dock_index(int a, int b)
{
    if (a < 0 || a >= (int)gamedata.cities.size()) return -1;
    for (int k = 0; k < 5; k++) if (gamedata.cities[a].docks[k] == b) return k;
    return -1;
}
/* boat k from town loc sails on day `day` (the game's sum in its 16-bit ints) */
static bool sails(int loc, int k, int day, int war)
{
    s16 v = (s16)(u16)(town_seed(loc) + day + k);
    return v % ((war & 1) ? 4 : 3) == 0;
}
static bool boardable(int loc) { return !(war_of(loc) & 2) && loc >= 0 && loc < (int)gamedata.cities.size() && (gamedata.cities[loc].buildings >> 11 & 1); }
/* days until boat k sails from loc (from the date d), -1 none within a fortnight */
static int wait_days(int loc, int k, Date d)
{
    int war = war_of(loc);
    for (int w = 0; w < 14; w++) { if (sails(loc, k, d.day, war)) return w; add_hours(d, 24); }
    return -1;
}

/* ---------------------------------------------------------------- the way */
struct Rng { unsigned s; int operator()(int n) { s = s * 1103515245u + 12345u; return (int)((s >> 16) % (unsigned)n); } };
static float land_days_from(int from, int to)
{
    const City &c = gamedata.cities[from];
    int m = travel_minutes_loc_from(c.x, c.y, to);
    return m < 0 ? -1.0f : m / 1440.0f;
}
/* one journey along the way: hours, and whether it stayed aboard to the end */
static int journey(int from, const int *legs, int n, int to, Rng &r, bool *aboard_all)
{
    Date d = today();
    int h = 0, x = from, i = 0; bool aboard = false; *aboard_all = true;
    while (x != to && i < n) {
        int y = legs[i], k = dock_index(x, y);
        if (k < 0) return -1;
        if (aboard) {
            int war = war_of(x);
            bool on = (war & 2) ? r(4) == k : sails(x, k, d.day, war);
            if (on) { int t = r(12); add_hours(d, t); h += t; x = y; i++; continue; }
            aboard = false; *aboard_all = false;
            add_hours(d, 2); h += 2;                             /* (ashore, through the gate's toll) */
        }
        if (k >= 4 || !boardable(x)) {                          /* (no boat from these docks: on by land) */
            float l = land_days_from(x, to);
            return l < 0 ? -1 : h + (int)(l * 24);
        }
        Date at = d; add_hours(at, 1);                          /* (the walk to the docks) */
        int w = wait_days(x, k, at);
        if (w < 0) return -1;
        int t = 1 + 24 * w + 1 + r(3) + 48 + r(48);
        add_hours(d, t); h += t; x = y; i++; aboard = true;
    }
    return x == to ? h : -1;
}
static void judge(BoatPlan &p, int from, const int *legs, int n)
{
    Rng r{ 0x5eed1u + (unsigned)from * 977u + (unsigned)p.to * 131u };
    std::vector<int> hs; int all = 0;
    for (int s = 0; s < 400; s++) {
        bool ab; int h = journey(from, legs, n, p.to, r, &ab);
        if (h < 0) return;
        hs.push_back(h); all += ab;
    }
    std::sort(hs.begin(), hs.end());
    double sum = 0; for (int h : hs) sum += h;
    float mean = (float)(sum / hs.size() / 24.0);
    if (p.ok && mean >= p.days) return;
    p.ok = 1; p.days = mean; p.lo = hs[hs.size() / 10] / 24.0f; p.hi = hs[hs.size() * 9 / 10] / 24.0f;
    p.aboard = all / 400.0f; p.nlegs = n;
    for (int i = 0; i < n; i++) p.legs[i] = legs[i];
}
static void search(BoatPlan &p, int from, int x, int *legs, int n, int maxn, unsigned long long *seen, int *tries)
{
    if (x == p.to) { judge(p, from, legs, n); ++*tries; return; }
    if (n >= maxn || *tries > 200) return;
    for (int k = 0; k < 5; k++) {
        int y = gamedata.cities[x].docks[k];
        if (y < 0 || y >= (int)gamedata.cities.size() || y == x || (seen[y / 64] >> (y % 64) & 1)) continue;
        if (k >= 4 && n == 0) continue;                           /* (not from the docks) */
        seen[y / 64] |= 1ull << (y % 64); legs[n] = y;
        search(p, from, y, legs, n + 1, maxn, seen, tries);
        seen[y / 64] &= ~(1ull << (y % 64));
    }
}
BoatPlan boattrip_plan(int from, int to)
{
    BoatPlan p; p.from = from; p.to = to;
    int nc = (int)gamedata.cities.size();
    if (from < 0 || from >= nc || to < 0 || to >= nc || from == to || !boardable(from) || nc > 128) return p;
    int legs[8]; unsigned long long seen[2] = { 0, 0 }; seen[from / 64] |= 1ull << (from % 64);
    int tries = 0;
    search(p, from, from, legs, 0, 6, seen, &tries);
    if (!p.ok) return p;
    Date at = today(); add_hours(at, 1);
    p.wait_days = wait_days(from, dock_index(from, p.legs[0]), at);
    p.land_days = land_days_from(from, to);
    return p;
}

/* ---------------------------------------------------------------- the journey */
enum { B_OFF, B_INN, B_LODGE, B_DOCKS, B_VOYAGE, B_ASHORE };
static struct { int phase; BoatPlan p; int leg, tries, lodge_seen, toll_at; u32 t, act; Date stay_from; } bt;

extern "C" int boattrip_active(void) { return bt.phase != B_OFF; }
extern "C" void boattrip_stop(const char *why)
{
    if (bt.phase == B_OFF) return;
    LOG("stop: %s\n", why ? why : "");
    if (why) { char m[300]; snprintf(m, sizeof m, "By boat to %s: %s", gamedata.cities[bt.p.to].name.c_str(), why); ui_notice(m); }
    bt.phase = B_OFF;
}
/* a key or a click of the player's: the walk stops (aboard, the player answers what the voyage brings) */
extern "C" void boattrip_user_input(void) { if (bt.phase == B_INN || bt.phase == B_DOCKS) boattrip_stop("stopped."); }

static void go(int phase)
{
    bt.phase = phase; bt.t = plat_ticks_ms(); bt.lodge_seen = 0;
    if (phase == B_INN) citynav_go_place("Inn");
    if (phase == B_DOCKS && strcmp(citynav_card_name(), "$DOCKS00")) citynav_go_place("Docks");
}
void boattrip_start(const BoatPlan &p)
{
    bt.p = p; bt.leg = 0; bt.tries = 0; bt.toll_at = -1;
    std::string way = gamedata.cities[p.from].name;
    for (int i = 0; i < p.nlegs; i++) way += " - " + gamedata.cities[p.legs[i]].name;
    LOG("start: %s, the first boat in %d days\n", way.c_str(), p.wait_days);
    char m[300];
    if (p.wait_days > 0) snprintf(m, sizeof m, "By boat to %s (%s): the boat sails in %d day%s - a stay at the inn first. Press any key or click to stop.",
                                  gamedata.cities[p.to].name.c_str(), way.c_str(), p.wait_days, p.wait_days == 1 ? "" : "s");
    else snprintf(m, sizeof m, "By boat to %s (%s): to the docks. Press any key or click to stop.", gamedata.cities[p.to].name.c_str(), way.c_str());
    ui_notice(m);
    go(p.wait_days > 0 ? B_INN : B_DOCKS);
}

extern "C" void boattrip_tick(void)
{
    if (bt.phase == B_OFF) return;
    u32 now = plat_ticks_ms();
    if (now - bt.act < 400) return;
    std::string card = citynav_card_name();
    int cur = (s16)g_rw(G_CUR_LOC);
    switch (bt.phase) {
    case B_INN:                                                 /* walked to the inn: its days, then away */
        if (ns_screen == NS_LODGE) {
            Date d = today(); add_hours(d, 1);
            int w = wait_days(bt.p.from, dock_index(bt.p.from, bt.p.legs[0]), d);
            LOG("at the inn: %d days (now day %d of month %d, %d:00)\n", w, d.day, d.month, d.hour);
            if (w > 0) ns_push(LV_DAYS, w, 0, 0);
            ns_push(LV_LEAVE, 0, 0, 0);
            bt.phase = B_LODGE; bt.t = now; bt.act = now; bt.stay_from = today();
            return;
        }
        if (!citynav_busy() && assist_in_card_menu() && (citynav_choose_text("take up residence") || citynav_choose_text("spend some time"))) {   /* (the inn's card, $Urban00 - or $Party02 of the game's start: its rooms) */
            LOG("into the inn\n"); bt.act = now; return;
        }
        if (!citynav_busy() && now - bt.t > 15000) boattrip_stop("the inn could not be reached.");
        return;
    case B_LODGE:                                               /* the stay over: to the docks (or the inn again) */
        if (ns_screen == NS_LODGE || game_mode() != GM_CARD || !assist_in_card_menu()) { if (now - bt.t > 120000) boattrip_stop("the stay at the inn did not end."); return; }
        {
            Date d = today(); add_hours(d, 1);
            int k = dock_index(bt.p.from, bt.p.legs[0]), w = wait_days(bt.p.from, k, d);
            LOG("the stay over: day %d of month %d, %d:00: the boat in %d days\n", d.day, d.month, d.hour, w);
            if (w > 0) {
                if (++bt.tries > 2) { boattrip_stop("the stay at the inn ended before the boat's day (the purse, or something that happened). Stay on, then try again."); return; }
                go(B_INN); bt.act = now; return;
            }
        }
        go(B_DOCKS); bt.act = now;
        return;
    case B_DOCKS:                                               /* at the docks: the boat */
        if (citynav_busy()) return;
        if (card != "$DOCKS00" || !assist_in_card_menu()) { if (now - bt.t > 20000) boattrip_stop("the docks could not be reached."); return; }
        {
            int k = dock_index(bt.p.from, bt.p.legs[0]);
            if (k < 0 || k > 3) { boattrip_stop("no boat from these docks."); return; }
            int on = g_rb((u16)(GV_opt_on + k));
            if (on == 1 && citynav_choose_option(k)) { LOG("aboard for %s\n", gamedata.cities[bt.p.legs[0]].name.c_str()); bt.phase = B_VOYAGE; bt.t = now; bt.act = now; return; }
            if (on == 2 && ++bt.tries <= 2) { go(B_INN); bt.act = now; return; }   /* (the day turned on the way) */
            boattrip_stop("the boat cannot be boarded here today: choose at the docks.");
        }
        return;
    case B_VOYAGE: {                                            /* aboard: on from stop to stop */
        if (card == "$DOCKS00" && !assist_in_card_menu()) { citynav_read_on(); bt.act = now; return; }   /* ("You go aboard ...") */
        if (card != "$ENTER00" || !assist_in_card_menu()) {
            if (game_mode() == GM_MAP) boattrip_stop("the voyage ended on the road.");
            return;                                             /* (the voyage, and what it brings: the player's) */
        }
        if (cur == bt.p.to) { boattrip_stop(nullptr); ui_notice("Arrived by boat: choose how to go ashore."); return; }
        while (bt.leg < bt.p.nlegs && bt.p.legs[bt.leg] != cur) bt.leg++;
        if (bt.leg >= bt.p.nlegs - 1) { boattrip_stop("the boat came to a town off the way."); return; }
        int y = bt.p.legs[bt.leg + 1], k = dock_index(cur, y);
        if (k >= 0 && g_rb((u16)(GV_opt_on + 5 + k)) == 1 && citynav_choose_option(5 + k)) {
            LOG("aboard on to %s\n", gamedata.cities[y].name.c_str());
            bt.leg++; bt.act = now;
            return;
        }
        /* no boat on today: ashore through the wharf's toll (when the town will have the party: its opinion not
           below 0), and the journey taken on from there - a stay at the inn and the docks, or the way by land */
        {
            int toll = (gamedata.cities[cur].size / 3 + 2) * (s16)g_rw(G_PARTY_SIZE);
            u32 lr = g_location(cur); int rep = lr ? (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12)) : -1;
            if (bt.toll_at != cur && rep >= 0 && purse() >= toll && g_rb((u16)(GV_opt_on + 1)) == 1 && citynav_choose_option(1)) {
                LOG("ashore at %s (the toll), the boat on in another day\n", gamedata.cities[cur].name.c_str());
                bt.toll_at = cur; bt.phase = B_ASHORE; bt.t = now; bt.act = now;
                return;
            }
        }
        Date d = today();
        int w = k >= 0 && k < 4 ? wait_days(cur, k, d) : -1;
        char m[200];
        if (w > 0) snprintf(m, sizeof m, "no boat on to %s today (in %d day%s): choose how to go ashore; then the world map's \"By boat\" takes it on.", gamedata.cities[y].name.c_str(), w, w == 1 ? "" : "s");
        else snprintf(m, sizeof m, "no boat on to %s from here now: choose how to go ashore.", gamedata.cities[y].name.c_str());
        boattrip_stop(m);
        return;
    }
    case B_ASHORE: {                                            /* ashore on the way: on from here */
        if (game_mode() != GM_CARD || !assist_in_card_menu() || card == "$ENTER00") { if (now - bt.t > 30000) boattrip_stop("going ashore did not end in the streets."); return; }
        BoatPlan p = boattrip_plan(cur, bt.p.to);
        if (!boattrip_worth(p)) {
            char m[200]; snprintf(m, sizeof m, "ashore in %s: from here the way by land is as quick (the world map's \"Leave town and travel there\").", gamedata.cities[cur].name.c_str());
            boattrip_stop(m); return;
        }
        int toll_at = bt.toll_at;
        boattrip_start(p); bt.toll_at = toll_at; bt.act = now;
        return;
    }
    }
}

/* test (uboat:TO): the way from the town the party is in to TO (-1: the one the boat gains most on), logged,
   and the journey begun when there is one */
extern "C" void boattrip_test(int to)
{
    int cur = (s16)g_rw(G_CUR_LOC), best = -1; float gain = -1e9f;
    BoatPlan bp;
    for (int t = 0; t < (int)gamedata.cities.size(); t++) {
        if (to >= 0 && t != to) continue;
        BoatPlan p = boattrip_plan(cur, t);
        if (!p.ok) continue;
        fprintf(stderr, "boat: %s -> %s: %d boats, first in %d days, %.1f days (%.1f-%.1f), aboard %.0f%%, land %.1f days\n", gamedata.cities[cur].name.c_str(),
                gamedata.cities[t].name.c_str(), p.nlegs, p.wait_days, p.days, p.lo, p.hi, p.aboard * 100, p.land_days);
        float g = p.land_days < 0 ? 99 : p.land_days - p.days;
        if (g > gain) { gain = g; best = t; bp = p; }
    }
    if (best < 0) { fprintf(stderr, "boat: no way by boat from %d\n", cur); return; }
    fprintf(stderr, "boat: going to %s\n", gamedata.cities[best].name.c_str());
    boattrip_start(bp);
}
