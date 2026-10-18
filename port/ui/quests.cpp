// quests.cpp - the quest log (Journal > Quests): the game keeps its quests as timed events
// (far table 2e38:1b2c, 300 pointers to 0x30-byte records) but never lists them; players
// used MicroProse's own DKQUE.EXE or other tools for that. Record fields (s16): +00 type0,
// +02/+0A/+12 dates written/start/end {hour, day, month 0-11, year}, +1A owner, +1C loc,
// +1E value, +22 kind, +28 tag, +2A rep/aux, +2C reward, +2E item. A quest is kind 8 while
// it runs and kind 0x24 once its task is done and the reward waits (tag = which quest).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "quests.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hooks.h"
#include "../mods/travel.h"
void ui_toggle(void);
}

static const char *MONTH[12] = { "January", "February", "March", "April", "May", "June", "July", "August",
                                 "September", "October", "November", "December" };

struct Ev { int i; u16 seg, off; int type0, owner, loc, value, kind, tag, aux, reward, item; int start[4], end[4]; };

static int ev_field(u16 s, u16 o, int k) { return (s16)rw(s, (u16)(o + k)); }

static std::vector<Ev> read_events(void)
{
    std::vector<Ev> v;
    for (int i = 0; i < 300; i++) {
        u16 o = rw(G_FAR2E38, (u16)(0x1b2c + 4 * i)), s = rw(G_FAR2E38, (u16)(0x1b2c + 4 * i + 2));
        if (!s) continue;
        Ev e; e.i = i; e.seg = s; e.off = o;
        e.type0 = ev_field(s, o, 0); e.owner = ev_field(s, o, 0x1a); e.loc = ev_field(s, o, 0x1c); e.value = ev_field(s, o, 0x1e);
        e.kind = ev_field(s, o, 0x22); e.tag = ev_field(s, o, 0x28); e.aux = ev_field(s, o, 0x2a); e.reward = ev_field(s, o, 0x2c);
        e.item = ev_field(s, o, 0x2e);
        for (int k = 0; k < 4; k++) { e.start[k] = ev_field(s, o, 0x0a + 2 * k); e.end[k] = ev_field(s, o, 0x12 + 2 * k); }
        v.push_back(e);
    }
    return v;
}

static std::string date(const int d[4])
{
    if (d[2] >= 12 || d[3] >= 1499) return "no time limit";
    char b[48]; snprintf(b, sizeof b, "%d %s %d", d[1], MONTH[d[2] % 12], d[3]);
    return b;
}

static bool loc_xy(int loc, int *x, int *y)
{
    u32 r = g_location(loc);
    if (!r) return false;
    *x = (s16)rw((u16)(r >> 16), (u16)(r + 4)); *y = (s16)rw((u16)(r >> 16), (u16)(r + 6));
    return true;
}

static std::string loc_name(int loc)
{
    if (loc >= 0 && loc < (int)gamedata.cities.size()) return gamedata.cities[loc].name;
    u32 r = g_location(loc);
    if (!r) return "?";
    char b[24]; g_str((u16)(r >> 16), (u16)(r + 0x26), b, sizeof b);
    return game_text(b, sizeof b);
}

/* "Burg Ebeleben, north-east of Erfurt" for places that are not towns */
static std::string where(int loc)
{
    std::string n = loc_name(loc);
    if (loc >= 0 && loc < (int)gamedata.cities.size()) return n;
    int x, y; if (!loc_xy(loc, &x, &y)) return n;
    int best = -1; double bd = 1e9;
    for (int c = 0; c < (int)gamedata.cities.size(); c++) {
        int cx, cy; if (!loc_xy(c, &cx, &cy)) continue;
        double d = hypot((double)(x - cx), (double)(y - cy) / 2.0);       /* rows are half as tall as columns are wide */
        if (d < bd) { bd = d; best = c; }
    }
    if (best < 0) return n;
    int cx, cy; loc_xy(best, &cx, &cy);
    double ang = atan2(-(double)(y - cy) / 2.0, (double)(x - cx)) * 180 / M_PI;
    static const char *DIR[8] = { "east", "north-east", "north", "north-west", "west", "south-west", "south", "south-east" };
    int k = (int)floor((ang + 22.5 + 360) / 45) % 8;
    return n + ", " + (bd < 3 ? "near " : std::string(DIR[k]) + " of ") + loc_name(best);
}

/* the town nearest a place (its own name for a town) */
static std::string nearest_town(int loc)
{
    if (loc >= 0 && loc < (int)gamedata.cities.size()) return loc_name(loc);
    int x, y; if (!loc_xy(loc, &x, &y)) return loc_name(loc);
    int best = -1; double bd = 1e9;
    for (int c = 0; c < (int)gamedata.cities.size(); c++) {
        int cx, cy; if (!loc_xy(c, &cx, &cy)) continue;
        double d = hypot((double)(x - cx), (double)(y - cy) / 2.0);
        if (d < bd) { bd = d; best = c; }
    }
    return best < 0 ? loc_name(loc) : loc_name(best);
}

/* a place for a title: its name, or for a site the map names only by its kind ("Lake", "Cave"
   ... DARKLAND.LOC), "the lake near Freiberg" */
static std::string site(int loc)
{
    std::string n = loc_name(loc);
    static const char *KIND[] = { "Cave", "Tomb", "Lair", "Spring", "Lake", "Shrine", "Pagan Altar" };
    for (const char *k : KIND)
        if (n == k) {
            std::string l = n;
            for (char &c : l) c = (char)tolower((unsigned char)c);
            return "the " + l + " near " + nearest_town(loc);
        }
    return n;
}

/* the game's own name for a person (random_name, s029 1367:0db4, seeded by the party seed);
 * asked side-effect free and kept */
static std::map<int, std::string> names;
static int names_seed = -1;
static std::string person(int seed)
{
    int ps = g_rw(G_PARTY_SEED);
    if (ps != names_seed) { names.clear(); names_seed = ps; }
    auto it = names.find(seed);
    if (it != names.end()) return it->second;
    int gm = game_mode();
    if (gm != GM_CARD && gm != GM_MAP && gm != GM_SCREEN) return "";
    void *snap = eval_snapshot();
    u16 args[3] = { (u16)seed, 0, 0 };
    std::string n;
    game_call_link(GF_random_name, 3, args);
    if (!game_call_timed_out) {
        char b[32]; g_str(G_FAR2E38, 0x1a96, b, sizeof b);
        n = game_text(b, sizeof b);
    }
    eval_restore(snap); eval_free(snap);
    names[seed] = n;
    return n;
}

static const char *giver(int id)
{
    switch (id) {
    case 0: return "a merchant"; case 4: return "a foreign trader"; case 5: return "a pharmacist";
    case 6: return "the Medici"; case 7: return "the Hanse"; case 8: return "the Fugger"; case 9: return "the villagers";
    case 10: return "the city's lord"; default: return "a townsman";
    }
}

static std::string item_name(int id)
{
    id &= 0xfff;
    if (id >= 0 && id < (int)gamedata.items.size()) return game_text(gamedata.items[id].name.c_str(), 64);
    return "an item";
}

struct Line { std::string text, when; int group; int loc = -1; std::string title, id; long end = -1; int ed = 0, em = 0, ey = 0; int order = 0; int cat = QC_NONE; int town = -1; };

/* a merchant's fetching errand (kind 8, tag 10; s039 1462:1712 picks the thing and the kind of
   place): a holy book, a crown, henbane ... from a cave, tomb, lake, spring, shrine or pagan altar;
   or papers (Treason Plan, Sworn Statement, Debt Note, Love Letters) to be got from someone in a town */
static int fetch_cat(const Ev &e)
{
    switch (e.item & 0xfff) {
    case 0x78: case 0x7f: case 0x89: case 0x8a: return QC_PAPERS;
    case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: return QC_RETRIEVE;
    }
    return e.loc >= 0 && e.loc < (int)gamedata.cities.size() ? QC_PAPERS : QC_RETRIEVE;
}

static bool spoilers = getenv("DL_QUEST_SPOILERS") != 0;

static long when_key(const int d[4]) { return (((long)d[3] * 12 + d[2]) * 31 + d[1]) * 24 + d[0]; }   /* (year, month, day, hour) in order */

/* ---------------------------------------------------------------- what the party learned of a Sabbat
 * The game schedules a Sabbat as an event (kind 0x1c, tag 1, type0 4; aux the place, end the
 * night; type0 5 while it is held) and lets the party learn of it on cards whose text holds
 * $SabbatLocation and/or $SabbatTime (a dying cultist, a witch's letter, a captured villager...).
 * When text_expand (s043 150b:06fe) expands such a block (as the card files have it, read by
 * ui/citynav.cpp), the place or the night is noted for the Sabbat coming next (not during the
 * card hints' trial runs, dos_no_writes), kept per game (the party's seed) beside the chronicle. */
struct SabbatKnown { int loc, day, month, year, what; };            /* what: 1 the place, 2 the night */
static std::vector<SabbatKnown> sab_known;
static int sab_seed = -1;
static std::string sab_path(int seed)
{
    extern char dlp_data_dir[];
    char b[1200]; snprintf(b, sizeof b, "%s/darklands_sabbats_%u.txt", dlp_data_dir, (unsigned)seed);
    return b;
}
static void sab_load(void)
{
    int seed = g_rw(G_PARTY_SEED);
    if (seed == sab_seed) return;
    sab_seed = seed; sab_known.clear();
    FILE *f = fopen(sab_path(seed).c_str(), "r");
    if (!f) return;
    SabbatKnown k;
    while (fscanf(f, "%d %d %d %d %d", &k.loc, &k.day, &k.month, &k.year, &k.what) == 5) sab_known.push_back(k);
    fclose(f);
}
static int sab_what(const Ev &e)
{
    sab_load();
    int w = 0;
    for (const SabbatKnown &k : sab_known) if (k.loc == e.aux && k.day == e.end[1] && k.month == e.end[2] && k.year == e.end[3]) w |= k.what;
    return w;
}
static void sab_learn(int what)
{
    std::vector<Ev> evs = read_events();
    const Ev *next = nullptr;
    for (const Ev &e : evs) if (e.kind == 0x1c && e.tag == 1 && e.type0 == 4 && (!next || when_key(e.end) < when_key(next->end))) next = &e;
    if (!next) return;
    sab_load();
    int had = sab_what(*next);
    if ((had | what) == had) return;
    SabbatKnown k = { next->aux, next->end[1], next->end[2], next->end[3], what };
    sab_known.push_back(k);
    FILE *f = fopen(sab_path(sab_seed).c_str(), "a");
    if (f) { fprintf(f, "%d %d %d %d %d\n", k.loc, k.day, k.month, k.year, k.what); fclose(f); }
    fprintf(stderr, "quests: learned of the Sabbat at %d on %d.%d.%d (%s)\n", k.loc, k.day, k.month + 1, k.year, what == 3 ? "place and night" : what == 1 ? "place" : "night");
}
extern "C" int citynav_card_block_has(const char *card, int block, const char *word);
static void sabbat_text_hook(void)          /* text_expand (idx, out) */
{
    extern int dos_no_writes;
    int idx = (s16)rw(SS, (u16)(SP + 4)), what = 0;
    if (!dos_no_writes && game_mode() != GM_OTHER) {
        char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
        if (citynav_card_block_has(nm, idx, "$SabbatLocation")) what |= 1;
        if (citynav_card_block_has(nm, idx, "$SabbatTime")) what |= 2;
    }
    hook_call_original();
    if (what) sab_learn(what);
}
extern "C" void quests_init(void) { hook_install(GF_text_expand, sabbat_text_hook); }
extern "C" void quests_test_learn(int what) { sab_learn(what); }

static std::vector<Line> build_lines(void)
{
    std::vector<Line> v;
    std::vector<Ev> evs = read_events();
    /* a town's bans and warrants: the game keeps one event for each time it was imposed;
       only the one that lasts longest still counts, and none that has run out */
    int today[4] = { (s16)g_rw(G_CUR_HOUR), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_YEAR) };
    long now = when_key(today);
    std::map<std::pair<int, int>, long> last;
    for (const Ev &e : evs) if (e.kind == 0x11 || e.kind == 0x12) { long k = when_key(e.end); auto &l = last[{ e.kind, e.loc }]; if (k > l) l = k; }
    std::map<std::pair<int, int>, int> shown;
    for (const Ev &e : evs) {
        if ((e.kind == 0x11 || e.kind == 0x12) && (when_key(e.end) < now || when_key(e.end) != last[{ e.kind, e.loc }] || shown[{ e.kind, e.loc }]++)) continue;
        std::string who = person(e.kind == 0x24 ? e.value : e.type0);
        std::string by = who.empty() ? giver(e.owner) : who + " (" + giver(e.owner) + ")";
        std::string place = loc_name(e.loc);
        auto add = [&](std::string title, std::string text, std::string when, int group, int loc, bool timed, int cat) {
            Line l; l.title = title; l.text = text; l.when = when; l.group = group; l.loc = loc; l.cat = cat;
            l.end = timed ? when_key(e.end) : -1; l.ed = e.end[1]; l.em = e.end[2]; l.ey = e.end[3]; l.order = e.i;
            char id[64]; snprintf(id, sizeof id, "%d.%d.%d.%d.%d.%d", e.kind, e.tag, e.loc, e.item, e.value, e.type0); l.id = id;
            v.push_back(l);
        };
        bool limit = date(e.end) != "no time limit";
        if (e.kind == 8 && e.tag == 3)
            add("Raubritter @ " + place, "Kill the robber knight " + (person(e.loc + 0x44c).empty() ? std::string("") : person(e.loc + 0x44c) + " ") + "at " + where(e.loc)
                + ", for " + by + " of " + loc_name(e.value) + ".", "by " + date(e.end), 0, e.loc, limit, QC_ROBBER);
        else if (e.kind == 0x24 && e.tag == 3)
            add("Bounty to collect @ " + place, "The robber knight is dead: collect the bounty from " + by + " in " + place + ".", "by " + date(e.end), 1, e.loc, limit, QC_REWARD);
        else if (e.kind == 8 && e.tag == 10)
            add(item_name(e.item) + " from " + site(e.loc), "Get the " + item_name(e.item) + " from " + where(e.loc) + " for " + by + " of " + loc_name(e.value) + ".",
                limit ? "by " + date(e.end) : "", 0, e.loc, limit, fetch_cat(e));
        else if (e.kind == 0x24 && e.tag == 10)
            add(item_name(e.item) + " to " + place, "Bring the " + item_name(e.item) + " to " + by + " in " + place + ".", "by " + date(e.end), 1, e.loc, limit, QC_REWARD);
        else if (e.kind == 8 && e.tag == 4)
            add("Dragon near " + nearest_town(e.loc), "A dragon terrorises the land around " + where(e.loc) + ".", "", 2, -1, false, QC_DRAGON);
        else if (e.kind == 8 && e.tag == 12)
            add("Mine @ " + place, "Trouble at the mine: " + where(e.loc) + ".", "", 2, e.loc, false, QC_MINE);
        else if (e.kind == 8 && e.tag == 7)
            add("The Wild Hunt", "The Wild Hunt pursues the party.", "", 2, -1, false, QC_HUNT);
        else if (e.kind == 0x11 && e.loc >= 0 && e.loc < (int)gamedata.cities.size())
            { add("Unwelcome @ " + place, "Not welcome in " + place + ": the town counts as hostile to the party (its gates and people turn it away).", "until " + date(e.end), 3, -1, true, QC_UNWELCOME); v.back().town = e.loc; }
        else if (e.kind == 0x12 && e.loc >= 0 && e.loc < (int)gamedata.cities.size())
            { add("Wanted @ " + place, "Wanted by the watch in " + place + ": guards who recognise the party try to arrest it, and talking or bribing one's way out is harder.", "until " + date(e.end), 3, -1, true, QC_WANTED); v.back().town = e.loc; }
        else if (e.kind == 0x1c && e.tag == 1 && e.type0 == 4 && (spoilers || sab_what(e))) {
            int known = sab_what(e), w = spoilers ? 3 : known, g = known ? 0 : 4;    /* (learned: among the tasks; else a secret) */
            if (w == 3) add("Sabbat @ " + loc_name(e.aux), "Witches' Sabbat at " + where(e.aux) + ".", "on " + date(e.end), g, e.aux, true, QC_SABBAT);
            else if (w == 1) add("Sabbat @ " + loc_name(e.aux), "Witches gather at " + where(e.aux) + " (the night is not known).", "", g, e.aux, false, QC_SABBAT);
            else add("Sabbat on " + date(e.end), "Witches gather on the night of " + date(e.end) + " (the place is not known).", "on " + date(e.end), g, -1, true, QC_SABBAT);
        }
        else if (e.kind == 0x1c && e.tag == 1 && e.type0 == 5 && (spoilers || [&] { sab_load(); for (const SabbatKnown &k : sab_known) if (k.loc == e.aux && (k.what & 1)) return true; return false; }()))
            add("Sabbat now @ " + loc_name(e.aux), "A witches' Sabbat is being held now at " + where(e.aux) + ".", "", spoilers ? 4 : 0, e.aux, false, QC_SABBAT);
    }
    return v;
}

/* places the quest log points to, for the world map */
void quest_marks_cat(std::vector<std::pair<int, std::string>> &out, std::vector<int> &cats)
{
    out.clear(); cats.clear();
    if (game_mode() == GM_OTHER) return;
    for (const Line &l : build_lines())
        if (l.loc >= 0) { out.push_back({ l.loc, l.text + (l.when.empty() ? "" : " (" + l.when + ")") }); cats.push_back(l.cat); }
}
void quest_marks(std::vector<std::pair<int, std::string>> &out) { std::vector<int> c; quest_marks_cat(out, c); }

/* the towns that bar the party or whose watch wants it, for the maps and the margin */
void town_marks(std::vector<TownMark> &out)
{
    out.clear();
    if (game_mode() == GM_OTHER) return;
    for (const Line &l : build_lines())
        if (l.town >= 0) out.push_back({ l.town, l.cat, l.when });
}

const char *QSORT_NAME[QSORT_N] = { "deadline", "nearest", "taken" };

static int day_number(int day, int month, int year)          /* days since a fixed day (the real month lengths) */
{
    static const int CUM[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    month = (month % 12 + 12) % 12;
    return year * 365 + year / 4 - year / 100 + year / 400 + CUM[month] + day;
}

/* the party's place on the map: the town or site it is at, else its tile (quests.h) */
bool party_xy(double *x, double *y)
{
    int cur = (s16)g_rw(G_CUR_LOC), px, py;
    if (cur >= 0 && loc_xy(cur, &px, &py)) { *x = px; *y = py; return true; }
    if (game_mode() != GM_MAP && game_mode() != GM_CARD) return false;
    travel_party_tile(&px, &py);
    if (px <= 0 && py <= 0) return false;
    *x = px; *y = py;
    return true;
}

void quest_rows(std::vector<QuestRow> &out, int sort, bool all)
{
    out.clear();
    int today[4] = { (s16)g_rw(G_CUR_HOUR), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_YEAR) };
    int dnow = day_number(today[1], today[2], today[3]);
    double px = 0, py = 0; bool here = party_xy(&px, &py);
    for (const Line &l : build_lines()) {
        if (!all && l.group > 1) continue;
        QuestRow r;
        r.id = l.id; r.title = l.title; r.text = l.text; r.when = l.when; r.group = l.group; r.loc = l.loc; r.order = l.order; r.cat = l.cat;
        r.days_left = -1;
        if (l.end >= 0) {
            int d = day_number(l.ed, l.em, l.ey) - dnow;
            r.days_left = d < 0 ? 0 : d;
        }
        r.dist = -1;
        int x, y;
        if (here && l.loc >= 0 && loc_xy(l.loc, &x, &y)) {
            double dx = x - px, dy = (y - py) / 2.0;          /* rows are half as tall as columns are wide */
            r.dist = hypot(dx, dy);
            if (r.dist >= 1.5) {
                static const char *DIR[8] = { "east", "north-east", "north", "north-west", "west", "south-west", "south", "south-east" };
                r.dir = DIR[(int)floor((atan2(-dy, dx) * 180 / M_PI + 22.5 + 360) / 45) % 8];
            }
        }
        out.push_back(r);
    }
    quest_sort(out, sort);
}

void quest_sort(std::vector<QuestRow> &out, int sort)
{
    std::stable_sort(out.begin(), out.end(), [sort](const QuestRow &a, const QuestRow &b) {
        if (sort == QSORT_NEAREST) {
            double da = a.dist < 0 ? 1e9 : a.dist, db = b.dist < 0 ? 1e9 : b.dist;
            if (da != db) return da < db;
        } else if (sort == QSORT_DEADLINE) {
            int da = a.days_left < 0 ? 1 << 30 : a.days_left, db = b.days_left < 0 ? 1 << 30 : b.days_left;
            if (da != db) return da < db;
        }
        return a.order < b.order;
    });
}

std::string quest_days(const QuestRow &r)
{
    if (r.days_left < 0) return "";
    if (r.days_left == 0) return "last day";
    return std::to_string(r.days_left) + (r.days_left == 1 ? " day" : " days");
}

std::string quest_distance(const QuestRow &r)
{
    if (r.dist < 0) return "";
    if (r.dist < 1.5) return "here";
    double days = r.dist / 10.0;                              /* (on foot, about ten map columns a day) */
    char b[64];
    if (days < 0.6) snprintf(b, sizeof b, "%s, less than a day away", r.dir.c_str());
    else snprintf(b, sizeof b, "%s, about %d day%s away", r.dir.c_str(), (int)(days + 0.5), (int)(days + 0.5) == 1 ? "" : "s");
    return b;
}

/* the tasks and rewards waiting (text, when): kept for anything that wants the plain list */
void quest_brief(std::vector<std::pair<std::string, std::string>> &out)
{
    out.clear();
    for (const Line &l : build_lines()) if (l.group <= 1) out.push_back({ l.text, l.when });
}

extern "C" void journal_quests(void)
{
    if (game_mode() == GM_OTHER) { ImGui::TextWrapped("Start or load a game first."); return; }
    static int sort = -1;
    if (sort < 0) sort = config_get_int("quests.sort", QSORT_DEADLINE) % QSORT_N;
    static std::vector<QuestRow> v;                     /* (read while a card or the map is up: battles and their loot reuse the tables) */
    if (game_mode() == GM_CARD || game_mode() == GM_MAP) quest_rows(v, config_get_int("quests.sort", sort) % QSORT_N, true);
    else quest_sort(v, config_get_int("quests.sort", sort) % QSORT_N);
    sort = config_get_int("quests.sort", sort) % QSORT_N;
    static const char *HEAD[5] = { "Tasks", "Rewards waiting", "Rumours", "Towns", "Secrets" };
    ImGui::TextDisabled("In order of:"); ImGui::SameLine();
    for (int k = 0; k < QSORT_N; k++) {
        if (k) ImGui::SameLine();
        if (ImGui::RadioButton(QSORT_NAME[k], sort == k)) { sort = k; config_set_int("quests.sort", k); config_save(); }
    }
    ImGui::SameLine(); ImGui::TextDisabled("  (a click on a quest opens it)");
    bool any = false;
    for (int g = 0; g < 5; g++) {
        bool head = false;
        for (const QuestRow &l : v) {
            if (l.group != g) continue;
            if (!head) { ImGui::SeparatorText(HEAD[g]); head = true; }
            ImGui::PushID(l.id.c_str());
            std::string days = g == 3 ? "" : quest_days(l), d = quest_distance(l);
            float isz = ImGui::GetTextLineHeight() * 1.2f, sp = ImGui::CalcTextSize(" ").x;
            std::string label = std::string((size_t)ceilf((isz + sp) / sp), ' ') + l.title;   /* (room for the picture) */
            bool open = ImGui::TreeNodeEx("##q", ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());
            {   ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
                ImVec2 ic(a.x + ImGui::GetTreeNodeToLabelSpacing() + isz * 0.5f, (a.y + b.y) * 0.5f), m = ImGui::GetIO().MousePos;
                ui_draw_quest_icon(ImGui::GetWindowDrawList(), ic, isz, l.cat);
                if (ImGui::IsItemHovered() && fabsf(m.x - ic.x) < isz * 0.5f && fabsf(m.y - ic.y) < isz * 0.5f) ImGui::SetTooltip("%s", QUEST_CAT_NAME[l.cat]); }
            if (!days.empty() || !d.empty()) {
                ImGui::SameLine();
                std::string right = days + (days.empty() || d.empty() ? "" : "  -  ") + d;
                ImGui::TextDisabled("   %s", right.c_str());
            }
            if (open) {
                ImGui::TextWrapped("%s", l.text.c_str());
                if (!l.when.empty()) ImGui::TextDisabled("%s", l.when.c_str());
                if (l.loc >= 0 && travel_enabled && game_mode() == GM_MAP) {
                    if (ImGui::SmallButton("Travel there") && travel_to_loc(l.loc, loc_name(l.loc).c_str())) ui_toggle();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
            any = true;
        }
    }
    if (!any) ImGui::TextWrapped("No quests at the moment. Robber-knight bounties, errands and deliveries the party takes on are listed here with their deadlines.");
    ImGui::Spacing();
    ImGui::Checkbox("Show secrets (Sabbat dates and places, even before the party learns them)", &spoilers);
}
