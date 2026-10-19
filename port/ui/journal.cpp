// journal.cpp - the town journal: as the party travels, it notes the towns it reaches (with
// the shops each one has, from DARKLAND.CTY) and the full stock of every shop whose wares it
// looks at, with qualities and prices. The Journal tab answers "where did we see plate
// armour of quality 35?" and "who sells Brimstone?".
//
// Hooks: card_load (s043 150b:000a) notices arrivals (cur_loc changes); after the barter
// screen has built its list (s032 1462:078e) the stock is read: item ids D_86a2[D_885c],
// shop D_8836, shop quality D_8834, wares mask D_8868, shop name far pointer D_869c. The
// item quality (s032 0efa) and price (buy_price, 192e) are asked from the game itself in a
// side-effect-free evaluation. One journal per game (party_seed), kept as a text file next
// to the game: darklands_journal_<seed>.txt.
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include "imgui.h"
#include "tablesort.h"
#include "ui.h"
#include "gamedata.h"
#include "uiinternal.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/hooks.h"
#include "../runtime/config.h"
#include "../mods/hdtext.h"
#include "money.h"
extern "C" void travel_party_tile(int *x, int *y);
extern char game_dir[1024];
void ovl_enter(int sect);
}

extern ImFont *ui_font_small;
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

struct SeenItem { int id, q, price; };
struct Shop { std::string name, date; int shop = 0, shopq = 0; u32 cats = 0; bool seen = false; std::vector<SeenItem> items; };
struct Town { bool visited = false; int visits = 0; std::string first, last; std::vector<Shop> shops; };

static std::map<int, Town> towns;
static int journal_on = 1;
static int cur_seed = -1, last_loc = -2;

/* ---------------------------------------------------------------- helpers */
static const char *MONTHS[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static std::string game_date(void)
{
    int d = (s16)g_rw(G_CUR_DAY), m = g_rw(G_CUR_MONTH), y = g_rw(G_CUR_YEAR);
    char b[32]; snprintf(b, sizeof b, "%d %s %d", d, MONTHS[m % 12], y);
    return b;
}

static std::string loc_name(int loc)
{
    std::string n = loc_display_name(loc);
    return n.empty() ? "?" : n;
}

static std::string money(int pf) { return money_str(pf); }

static std::string item_name(int id)
{
    if (id >= 0 && id < (int)gamedata.items.size()) return game_text(gamedata.items[id].name.c_str(), 64);
    return "#" + std::to_string(id);
}

/* ---------------------------------------------------------------- file */
static std::string path_for(int seed)
{
    extern char dlp_data_dir[];         /* the game folder, or %LOCALAPPDATA%\Darklands+ when it is read-only */
    char b[1200]; snprintf(b, sizeof b, "%s/darklands_journal_%u.txt", dlp_data_dir, (unsigned)seed);
    return b;
}

static void save(void)
{
    if (cur_seed < 0) return;
    std::string p = path_for(cur_seed), tmp = p + ".new";
    FILE *f = fopen(tmp.c_str(), "w");
    if (!f) return;
    fprintf(f, "# Darklands+ town journal (game %u)\n", (unsigned)cur_seed);
    for (auto &kv : towns) {
        const Town &t = kv.second;
        fprintf(f, "T %d %d %d|%s|%s\n", kv.first, t.visited, t.visits, t.first.c_str(), t.last.c_str());
        for (const Shop &s : t.shops) {
            fprintf(f, "S %d %d %08x %d|%s|%s\n", s.shop, s.shopq, (unsigned)s.cats, s.seen, s.date.c_str(), s.name.c_str());
            for (const SeenItem &i : s.items) fprintf(f, "I %d %d %d\n", i.id, i.q, i.price);
        }
    }
    fclose(f);
    remove(p.c_str());
    rename(tmp.c_str(), p.c_str());
}

static void load(int seed)
{
    towns.clear();
    FILE *f = fopen(path_for(seed).c_str(), "r");
    if (!f) return;
    char line[512];
    Town *t = nullptr; Shop *s = nullptr;
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        char *bar = strchr(line, '|');
        if (line[0] == 'T' && bar) {
            int loc, vis, n;
            if (sscanf(line + 2, "%d %d %d", &loc, &vis, &n) != 3) continue;
            t = &towns[loc]; t->visited = vis; t->visits = n; s = nullptr;
            char *bar2 = strchr(bar + 1, '|');
            if (bar2) { t->first.assign(bar + 1, bar2 - bar - 1); t->last = bar2 + 1; }
        } else if (line[0] == 'S' && bar && t) {
            Shop sh; unsigned cats; int seen = 1;
            if (sscanf(line + 2, "%d %d %x %d", &sh.shop, &sh.shopq, &cats, &seen) < 3) continue;
            sh.cats = cats; sh.seen = seen;
            char *bar2 = strchr(bar + 1, '|');
            if (bar2) { sh.date.assign(bar + 1, bar2 - bar - 1); sh.name = bar2 + 1; }
            t->shops.push_back(sh); s = &t->shops.back();
        } else if (line[0] == 'I' && s) {
            SeenItem i;
            if (sscanf(line + 2, "%d %d %d", &i.id, &i.q, &i.price) == 3) s->items.push_back(i);
        }
    }
    fclose(f);
}

static void ensure_game(void)
{
    int seed = g_rw(G_PARTY_SEED);
    if (seed != cur_seed) { cur_seed = seed; last_loc = -2; load(seed); }
}

/* ---------------------------------------------------------------- shop stock
 * A shop's wares are not stored: the barter screen (s032 1462:078e) rebuilds the list each
 * time from srand(city seed + party_seed), so they are the same for the whole game and can be
 * worked out as soon as the party reaches a town. This is that loop (shop_has_item, 1d18),
 * including its quirk: it reads its city size from an uninitialised stack slot, which holds
 * a stale value at the first candidate and after each of the first four rows it draws. */
static const unsigned char CHANCE[9][12] = {
    {50,20,15,10,5,1,1,0,0,0,0,0}, {75,50,35,25,15,10,5,1,1,0,0,0}, {99,75,65,60,45,30,20,10,7,4,2,1},
    {99,99,90,80,65,50,35,20,15,7,4,2}, {100,99,99,99,85,70,50,35,25,15,7,3}, {100,100,99,99,90,80,65,55,40,30,15,6},
    {100,100,100,100,99,99,85,75,60,50,20,12}, {100,100,100,100,100,100,100,100,85,75,30,20},
    {100,100,100,100,100,100,100,100,99,99,50,30} };

struct MsRand {                          /* Microsoft C rand() */
    u32 h;
    explicit MsRand(unsigned seed) : h(seed & 0xffff) {}
    int rand() { h = h * 214013u + 2531011u; return (int)((h >> 16) & 0x7fff); }
    int random_n(int n) { return (int)((((u32)(rand() << 1) & 0xffff) * (u32)n) >> 16); }
};

static int item_quality(int id, int shopq)          /* s032 1462:0efa */
{
    const ItemDef &d = gamedata.items[id];
    if (d.type == 0x40 || d.type == 0x41 || d.type == 0x42 || d.type == 0x17 || d.type == 0x18 || d.type == 0x19 || d.cats() == 0x400)
        return 25;
    return ((s8)d.quality + shopq) / 2;
}

static std::vector<SeenItem> predict_stock(int city, int seed, int shopq, u32 mask)
{
    std::vector<SeenItem> out;
    const City &c = gamedata.cities[city];
    MsRand rng((unsigned)(c.seed + seed));
    int slot = 0x1000;
    int n = (int)gamedata.items.size(); if (n > 200) n = 200;
    for (int k = 0; k < n; k++) {
        const ItemDef &d = gamedata.items[k];
        if (!(d.cats() & mask) || (d.flags5 & 0x80)) continue;
        int q = item_quality(k, shopq);
        bool hit;
        if (d.type == 0x40 || d.type == 0x41 || d.type == 0x42) hit = true;      /* ammunition: always */
        else {
            int size = (slot < 0x5c && slot != -1) ? c.size - 1 : 1;
            if (size > 8) size = 8;
            if (size < 0) size = 0;
            slot = size;
            int rr = (s8)d.rarity + (q < 23 ? 1 : q > 29 ? -2 : q > 26 ? -1 : 0);
            rr = rr < 0 ? 0 : rr > 11 ? 11 : rr;
            hit = rng.random_n(100) <= CHANCE[size][rr];
        }
        if (hit) {
            if (out.size() < 4) slot = 0x1000;
            out.push_back({ k, q, 0 });
        }
    }
    return out;
}

struct ShopKind { int shop; u32 cats; const char *name; };
static const ShopKind SHOPS[] = {          /* what each town shop sells (the cards' barter calls) */
    { 0, 0x040000ffu, "Blacksmith" }, { 1, 0x0002c100u, "Goods merchant" }, { 2, 0x040000ffu, "Swordsmith" },
    { 3, 0x040000ffu, "Armourer" }, { 5, 0x083c0030u, "Bowyer" }, { 6, 0x0000800eu, "Artificer" },
    { 8, 0x04000000u, "Clothmaker" }, { -1, 0x00000400u, "Pharmacist" } };

/* every shop of the town the party has just reached; prices from the game's own rule
 * (buy_price, s032 192e: the leader's charisma, the town's opinion of the party ...),
 * asked in a side-effect-free evaluation */
static void record_town_stock(int loc)
{
    if (loc < 0 || loc >= (int)gamedata.cities.size() || gamedata.items.empty()) return;
    const City &c = gamedata.cities[loc];
    int seed = g_rw(G_PARTY_SEED);
    Town &t = towns[loc];
    std::string d = game_date();
    void *snap = eval_snapshot();
    ovl_enter(32);
    for (const ShopKind &k : SHOPS) {
        int shopq = k.shop < 0 ? 25 : c.shopq[k.shop];
        if (k.shop >= 0 && !shopq) continue;
        Shop sh;
        sh.shop = k.shop; sh.shopq = shopq; sh.cats = k.cats; sh.name = k.name; sh.date = d;
        sh.items = predict_stock(loc, seed, shopq, k.cats);
        g_ww(GV_barter_quality, (u16)shopq); g_ww(GV_barter_shop, (u16)k.shop);
        for (SeenItem &i : sh.items) { u16 id = (u16)i.id; i.price = (u16)(game_call_link(GF_buy_price, 1, &id) & 0xffff); }
        bool have = false;
        for (Shop &o : t.shops) if (o.shop == sh.shop && o.cats == sh.cats) { have = true; if (!o.seen) o = sh; }
        if (!have) t.shops.push_back(sh);
    }
    eval_restore(snap);
    eval_free(snap);
}

/* ---------------------------------------------------------------- hooks */
/* ---------------------------------------------------------------- the chronicle
 * Every text the cards show, word for word, with the date, the place and the option chosen, so
 * a clue read weeks ago can be read again. The card engine expands the text block on show into
 * msg_buf (DS:9085, s043 text_expand: the narrative, then the option lines, each "\x15...\x1d"
 * and its words; \x14 starts a paragraph); card_menu (s043 1142, hooked in mods/assist.c)
 * returns the option number chosen, whose line on screen is found through the option layout
 * D_e96e (line y e974[], height e988[], option opt e9b0[]). Kept per game in
 * darklands_chronicle_<seed>.txt, one entry a line (date, place, text, choice, hour; tabs between;
 * older files have no hour). */
struct ChronEntry { std::string date, place, text, choice; int hour = -1; };   /* (hour: -1 in files from before it was kept) */
static std::vector<ChronEntry> chron;
static int chron_seed = -1;
static bool chron_pending;                 /* the last entry is not in the file yet (its choice may come) */

static std::string chron_path(int seed)
{
    extern char dlp_data_dir[];
    char b[1200]; snprintf(b, sizeof b, "%s/darklands_chronicle_%u.txt", dlp_data_dir, (unsigned)seed);
    return b;
}
static std::string esc(const std::string &t) { std::string o; for (char c : t) { if (c == '\n') o += "\\n"; else if (c == '\t') o += ' '; else if (c == '\\') o += "\\\\"; else o += c; } return o; }
static std::string unesc(const std::string &t)
{
    std::string o;
    for (size_t i = 0; i < t.size(); i++) {
        if (t[i] == '\\' && i + 1 < t.size()) { o += t[i + 1] == 'n' ? '\n' : t[i + 1]; i++; }
        else o += t[i];
    }
    return o;
}
static void chron_flush(void)
{
    if (!chron_pending || chron.empty() || chron_seed < 0) return;
    chron_pending = false;
    FILE *f = fopen(chron_path(chron_seed).c_str(), "a");
    if (!f) return;
    const ChronEntry &e = chron.back();
    fprintf(f, "%s\t%s\t%s\t%s\t%d\n", esc(e.date).c_str(), esc(e.place).c_str(), esc(e.text).c_str(), esc(e.choice).c_str(), e.hour);
    fclose(f);
}
static void chron_load(int seed)
{
    chron_flush();
    chron.clear(); chron_seed = seed; chron_pending = false;
    FILE *f = fopen(chron_path(seed).c_str(), "r");
    if (!f) return;
    std::string line; int c;
    while ((c = fgetc(f)) != EOF) {
        if (c != '\n') { line += (char)c; continue; }
        std::vector<std::string> fl;                       /* date, place, text, choice[, hour] */
        for (size_t a = 0;;) { size_t t = line.find('\t', a); fl.push_back(line.substr(a, t == std::string::npos ? std::string::npos : t - a)); if (t == std::string::npos) break; a = t + 1; }
        if (fl.size() >= 4) chron.push_back({ unesc(fl[0]), unesc(fl[1]), unesc(fl[2]), unesc(fl[3]), fl.size() >= 5 ? atoi(fl[4].c_str()) : -1 });
        line.clear();
    }
    fclose(f);
}
static int chron_on(void) { return journal_on && config_get_int("journal.chronicle", 1) && !config_get_int("original", 0); }

/* the narrative of the text block now in msg_buf (without the option lines), as UTF-8 */
static std::vector<std::string> chron_opts_parsed;   /* the option lines of the last block read (when the crisp text has none) */
static std::string chron_text_now(void)
{
    std::string out, para; bool any = false;
    chron_opts_parsed.clear();
    char line[600]; int n = 0;
    auto end_line = [&](void) {
        line[n] = 0;
        const char *p = line; bool opt = false;
        while (*p == 0x14) { p++; if (!para.empty()) { out += (out.empty() ? "" : "\n\n") + para; para.clear(); } }
        if (*p == 0x15) {
            opt = true;
            std::string o;
            for (const char *q = p + 1; *q; q++) if ((unsigned char)*q >= 0x20) o += *q; else if (*q == 0x1d) o += ' ';
            size_t k = 0; while (k < o.size() && (o[k] == '.' || o[k] == ' ')) k++;
            o.erase(0, k);
            bool digits = !o.empty(); for (char c : o) if (c < '0' || c > '9') digits = false;
            if (!o.empty() && !digits) chron_opts_parsed.push_back(game_text(o.c_str(), o.size()));
        }
        if (!opt) {
            std::string clean;
            for (const char *q = p; *q; q++) if ((unsigned char)*q >= 0x20) clean += *q; else if (*q == 0x1d) clean += ' ';
            if (!clean.empty()) { para += (para.empty() ? "" : " ") + game_text(clean.c_str(), clean.size()); any = true; }
        }
        n = 0;
    };
    for (int i = 0; i < 4000; i++) {
        char c = (char)g_rb((u16)(GV_msg_buf + i));
        if (!c) break;
        if (c == '\n') { end_line(); continue; }
        if (n < (int)sizeof line - 1) line[n++] = c;
    }
    if (n) end_line();
    if (!para.empty()) out += (out.empty() ? "" : "\n\n") + para;
    return any ? out : "";
}

/* text_expand has just put a block into msg_buf: it is what the card shows next (kept aside,
 * as msg_buf is also where option help such as "Success is assured." is written) */
static std::string chron_next;
static std::vector<std::string> chron_next_opts, chron_cur_opts;
static void chronicle_expanded(void)
{
    if (!chron_on()) return;
    chron_next = chron_text_now();
    chron_next_opts = chron_opts_parsed;
}

/* the block on show is read: into the chronicle (once) */
extern "C" void chronicle_capture(void)
{
    if (!chron_on() || game_mode() == GM_OTHER || chron_next.empty()) return;
    int seed = g_rw(G_PARTY_SEED);
    if (seed != chron_seed) chron_load(seed);
    std::string t; t.swap(chron_next);
    chron_cur_opts.swap(chron_next_opts); chron_next_opts.clear();
    if (getenv("DL_CHRON_LOG")) fprintf(stderr, "chronicle: %d bytes: %.60s\n", (int)t.size(), t.c_str());
    if (t.size() < 8 || (!chron.empty() && chron.back().text == t)) return;
    chron_flush();
    int loc = (s16)g_rw(G_CUR_LOC);
    std::string place;
    if (loc >= 0) place = loc_name(loc);
    else { int ui_nearest_town(void); int k = ui_nearest_town(); place = k >= 0 ? "near " + loc_name(k) : "the countryside"; }
    chron.push_back({ game_date(), place, t, "", (s16)g_rw(G_CUR_HOUR) });
    chron_pending = true;
    if (chron.size() > 20000) chron.erase(chron.begin(), chron.begin() + 1000);
}

/* card_menu returned option sel: the words of its line on screen */
extern "C" void chronicle_choice(int sel)
{
    if (!chron_on() || chron.empty() || !chron_pending || !chron.back().choice.empty()) return;
    int n = (s8)g_rb(GV_card_layout);
    if (getenv("DL_CHRON_LOG")) { fprintf(stderr, "chronicle: choice %d of %d lines, %d parsed:", sel, n, (int)chron_cur_opts.size()); for (int i = 0; i < n && i < 20; i++) fprintf(stderr, " %d", (s8)g_rb((u16)(GV_card_layout_opt + i))); fprintf(stderr, "\n"); }
    for (int i = 0; i < n && i < 20; i++) {
        if ((s8)g_rb((u16)(GV_card_layout_opt + i)) != sel) continue;
        int y = g_rb((u16)(GV_card_layout_y + i)), h = g_rb((u16)(GV_card_layout_h + i));
        std::vector<const HdRun *> rs;
        for (int k = 0; k < hdtext_nruns; k++)
            if (hdtext_runs[k].x >= 60 && hdtext_runs[k].y >= y - 1 && hdtext_runs[k].y < y + h - 1) rs.push_back(&hdtext_runs[k]);
        std::sort(rs.begin(), rs.end(), [](const HdRun *a, const HdRun *b) { return a->y != b->y ? a->y < b->y : a->x < b->x; });
        std::string t;
        for (const HdRun *r : rs) t += (t.empty() ? "" : " ") + game_text(r->text, sizeof r->text);
        while (!t.empty() && (t[0] == '.' || t[0] == ' ')) t.erase(0, 1);
        if (t.empty() && sel >= 0 && sel < (int)chron_cur_opts.size()) t = chron_cur_opts[(size_t)sel];   /* no crisp text: the block's option lines, numbered in order */
        if (!t.empty()) chron.back().choice = t;
        break;
    }
    chron_flush();
}

/* the chronicle's days, for its navigation: entries are kept in the order read, so a day is a run
   of entries with the same date (a date seen again later, after a load of an older save, is
   another run) */
struct ChronDay { std::string date; int y, m, d; std::vector<int> idx; std::vector<std::pair<std::string, int>> places; };   /* places: name, first entry */
static void chron_days(const std::string &find, std::vector<ChronDay> &out)
{
    out.clear();
    for (int i = 0; i < (int)chron.size(); i++) {
        const ChronEntry &e = chron[(size_t)i];
        if (!find.empty()) {
            std::string a = e.text + " " + e.place + " " + e.choice;
            for (char &c : a) c = (char)tolower((unsigned char)c);
            if (a.find(find) == std::string::npos) continue;
        }
        if (out.empty() || out.back().date != e.date) {
            ChronDay D; D.date = e.date; D.y = D.m = D.d = 0;
            char mon[8] = ""; sscanf(e.date.c_str(), "%d %7s %d", &D.d, mon, &D.y);
            for (int k = 0; k < 12; k++) if (!strcmp(mon, MONTHS[k])) D.m = k;
            out.push_back(D);
        }
        ChronDay &D = out.back();
        D.idx.push_back(i);
        if (D.places.empty() || D.places.back().first != e.place) D.places.push_back({ e.place, i });
    }
}

/* test (ucmd uchronfill:n): n made-up texts over some weeks and towns, in memory only */
extern "C" void journal_test_chronicle(int n)
{
    static const char *T[4] = { "Strassburg", "Basel", "near Basel", "the countryside" };
    for (int i = 0; i < n; i++) {
        char d[32]; snprintf(d, sizeof d, "%d %s 1400", 1 + (i / 6) % 28, MONTHS[(5 + i / 40) % 12]);
        chron.push_back({ d, T[(i / 3) % 4], "A made-up text, number " + std::to_string(i) + ", to try the chronicle's days and places. "
                          "It runs on a little so that it wraps over two lines in the window.", i % 2 ? "...leave." : "", 6 + (i % 6) * 2 });
    }
}

static char chron_find[60];
static void chronicle_page(void)
{
    int seed = g_rw(G_PARTY_SEED);
    if (game_mode() != GM_OTHER && seed != chron_seed) chron_load(seed);
    if (!chron_on()) { ImGui::TextWrapped("The chronicle is off (F12 > Settings > Helpers)."); return; }
    static bool nav_open = true;
    if (ImGui::SmallButton(nav_open ? "Hide the days" : "Show the days")) nav_open = !nav_open;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(UIS(260)); ImGui::InputTextWithHint("##cfind", "find words (a name, a place, a clue)", chron_find, sizeof chron_find);
    ImGui::SameLine(); ImGui::TextDisabled("%d texts read in this game", (int)chron.size());
    ImGui::SameLine();
    static std::string wrote;
    if (ImGui::SmallButton("Write it out as a book")) {
        extern char dlp_data_dir[];
        char b[1200]; snprintf(b, sizeof b, "%s/darklands_chronicle_%u_book.txt", dlp_data_dir, (unsigned)chron_seed);
        FILE *f = fopen(b, "w");
        if (f) {
            std::string last;
            for (const ChronEntry &e : chron) {
                std::string head = e.date + ", " + e.place;
                if (head != last) { fprintf(f, "\n=== %s ===\n\n", head.c_str()); last = head; }
                fprintf(f, "%s\n", e.text.c_str());
                if (!e.choice.empty()) fprintf(f, "  > %s\n", e.choice.c_str());
                fprintf(f, "\n");
            }
            fclose(f); wrote = b;
        } else wrote = "Could not write the file.";
    }
    if (!wrote.empty()) ImGui::TextDisabled("%s", wrote.c_str());

    std::string f = chron_find; for (char &c : f) c = (char)tolower((unsigned char)c);
    static std::vector<ChronDay> days; static size_t days_n = (size_t)-1; static std::string days_f;
    if (days_n != chron.size() || days_f != f) { chron_days(f, days); days_n = chron.size(); days_f = f; }
    static int sel = -1, scroll_to = -1;                  /* the day shown (an index into days), the entry to bring into view */
    static size_t sel_n = 0; static std::string sel_f = "\x01";
    if (sel_f != f || sel < 0 || sel >= (int)days.size() || (sel_n != chron.size() && sel == (int)days.size() - 2)) sel = (int)days.size() - 1;   /* (the newest day, and it follows new texts) */
    sel_n = chron.size(); sel_f = f;
    if (days.empty()) {
        ImGui::BeginChild("chron", ImVec2(0, 0), ImGuiChildFlags_Border);
        ImGui::TextWrapped(chron.empty() ? "Every text the cards show is kept here, with the date, the place and the choice you made." : "Nothing found.");
        ImGui::EndChild();
        return;
    }

    /* the days, by month, newest first: a day opens to its places */
    if (nav_open) {
        ImGui::BeginChild("chronnav", ImVec2(UIS(240), 0), ImGuiChildFlags_Border);
        int last_month = -1;
        bool month_open = false;
        for (int k = (int)days.size() - 1; k >= 0; k--) {
            const ChronDay &D = days[(size_t)k];
            int mk = D.y * 12 + D.m;
            if (mk != last_month) {
                if (month_open) ImGui::TreePop();
                static const char *MN[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
                char ml[48]; snprintf(ml, sizeof ml, "%s %d###m%d", MN[D.m % 12], D.y, mk);
                bool newest = k == (int)days.size() - 1;
                bool has_sel = sel >= 0 && days[(size_t)sel].y * 12 + days[(size_t)sel].m == mk;
                if (has_sel) ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                month_open = ImGui::TreeNodeEx(ml, newest ? ImGuiTreeNodeFlags_DefaultOpen : 0);
                last_month = mk;
            }
            if (!month_open) continue;
            ImGui::PushID(k);
            char dl[64]; snprintf(dl, sizeof dl, "%d %s  (%d)", D.d, MONTHS[D.m % 12], (int)D.idx.size());
            ImGuiTreeNodeFlags fl = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (k == sel ? ImGuiTreeNodeFlags_Selected : 0)
                                  | (D.places.size() <= 1 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : 0);
            bool open = ImGui::TreeNodeEx("day", fl, "%s", dl);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) { sel = k; scroll_to = D.idx.front(); }
            if (D.places.size() <= 1) { ImGui::SameLine(); ImGui::TextDisabled("%s", D.places.empty() ? "" : D.places[0].first.c_str()); }
            else if (open) {
                for (size_t p = 0; p < D.places.size(); p++) {
                    ImGui::PushID((int)p);
                    if (ImGui::Selectable(D.places[p].first.c_str(), false)) { sel = k; scroll_to = D.places[p].second; }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (month_open) ImGui::TreePop();
        ImGui::EndChild();
        ImGui::SameLine();
    }

    /* the day chosen, in the order it was read: its places as headings, the time, the choice */
    ImGui::BeginChild("chron", ImVec2(0, 0), ImGuiChildFlags_Border);
    const ChronDay &D = days[(size_t)sel];
    ImGui::BeginDisabled(sel <= 0); if (ImGui::SmallButton("< Day before")) { sel--; scroll_to = -2; } ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(sel >= (int)days.size() - 1); if (ImGui::SmallButton("Day after >")) { sel++; scroll_to = -2; } ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("%s%s", D.date.c_str(), f.empty() ? "" : " (the texts found)");
    const ChronDay &S = days[(size_t)sel];
    if (scroll_to == -2) { ImGui::SetScrollY(0); scroll_to = -1; }
    std::string place;
    for (int i : S.idx) {
        const ChronEntry &e = chron[(size_t)i];
        ImGui::PushID(i);
        if (e.place != place) {
            ImGui::Spacing();
            ImGui::SeparatorText(e.place.c_str());
            place = e.place;
        }
        if (i == scroll_to) { ImGui::SetScrollHereY(0.0f); scroll_to = -1; }
        ImGui::PushTextWrapPos(0);
        if (e.hour >= 0) { ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "%d:00", e.hour); ImGui::SameLine(); }   /* (the hour before the text, which wraps under it) */
        ImGui::TextUnformatted(e.text.c_str());
        if (!e.choice.empty()) ImGui::TextColored(ImVec4(0.18f, 0.32f, 0.55f, 1), "> %s", e.choice.c_str());
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        ImGui::PopID();
    }
    ImGui::EndChild();
}

/* ---------------------------------------------------------------- what the party learned of a town's
 * physician and alchemist, from the cards' own words as they are shown (per game, in
 * darklands_townfacts_<seed>.txt beside the journal): $Physi00 block 8 "... decides that $NamedOneName
 * has $Text1 skill." and block 10 "... is a complete idiot"; $Alche00 block 7 "... improve your
 * philosopher's stone to quality $Number1" and block 5 "your abilities are beyond my own" */
static std::map<int, std::map<std::string, std::string>> facts; static int facts_seed = -1;
static std::string facts_path(int seed) { extern char dlp_data_dir[]; char b[1200]; snprintf(b, sizeof b, "%s/darklands_townfacts_%u.txt", dlp_data_dir, (unsigned)seed); return b; }
static void facts_sync(void)
{
    int seed = g_rw(G_PARTY_SEED);
    if (seed == facts_seed) return;
    facts_seed = seed; facts.clear();
    FILE *f = fopen(facts_path(seed).c_str(), "r");
    if (!f) return;
    char line[400];
    while (fgets(line, sizeof line, f)) {
        char *a = strchr(line, '|'), *b = a ? strchr(a + 1, '|') : nullptr;
        if (!a || !b || line[0] == '#') continue;
        *a = 0; *b = 0; char *v = b + 1; v[strcspn(v, "\r\n")] = 0;
        facts[atoi(line)][a + 1] = v;
    }
    fclose(f);
}
static void facts_set(int loc, const char *key, const std::string &v)
{
    facts_sync();
    if (facts[loc][key] == v) return;
    facts[loc][key] = v;
    FILE *f = fopen(facts_path(facts_seed).c_str(), "w");
    if (!f) return;
    fprintf(f, "# Darklands+: what the party learned of the towns' physicians and alchemists (town|what|words)\n");
    for (auto &t : facts) for (auto &kv : t.second) fprintf(f, "%d|%s|%s\n", t.first, kv.first.c_str(), kv.second.c_str());
    fclose(f);
}
/* "physician" / "alchemist": what is known, or NULL */
extern "C" const char *journal_town_fact(int loc, const char *key)
{
    if (game_mode() == GM_OTHER) return nullptr;
    facts_sync();
    auto t = facts.find(loc); if (t == facts.end()) return nullptr;
    auto k = t->second.find(key); return k == t->second.end() ? nullptr : k->second.c_str();
}
static void facts_scan(int block)
{
    int loc = (s16)g_rw(G_CUR_LOC);
    if (loc < 0 || loc >= 92 || game_mode() == GM_OTHER) return;
    char cn[13]; g_str(G_DGROUP, 0xe3d4, cn, sizeof cn);
    for (char *p = cn; *p; p++) *p = (char)toupper((unsigned char)*p);
    bool phys = !strcmp(cn, "$PHYSI00"), alch = !strcmp(cn, "$ALCHE00");
    if (!phys && !alch) return;
    std::string t = chron_text_now();
    if (phys && block == 8) {
        size_t e = t.rfind(" skill"), h = e == std::string::npos ? e : t.rfind(" has ", e);
        if (h != std::string::npos) { std::string w = t.substr(h + 5, e - h - 5); if (!w.empty()) w[0] = (char)tolower((unsigned char)w[0]); facts_set(loc, "physician", w + " skill"); }
    } else if (phys && block == 10) facts_set(loc, "physician", "a complete idiot: worse than none");
    else if (alch && block == 7) {
        size_t q = t.find("to quality ");
        if (q != std::string::npos) facts_set(loc, "alchemist", "can raise a Philosopher's Stone to " + std::to_string(atoi(t.c_str() + q + 11)) + "q");
    } else if (alch && block == 5) facts_set(loc, "alchemist", "no better Philosopher's Stone than the party's");
}

static void text_expand_hook(void)         /* s043 150b:06fe text_expand(idx, out): the block shown before is done */
{
    bool mine = (u16)rw(SS, (u16)(SP + 6)) == 0x9085;
    int block = (s16)rw(SS, (u16)(SP + 4));
    if (mine) chronicle_capture();
    hook_call_original();
    if (mine) { chronicle_expanded(); facts_scan(block); }
}

/* ---------------------------------------------------------------- the player's own notes (per game) */
static std::string notes_text; static int notes_seed = -1; static bool notes_dirty; static double notes_edit_t;
static std::string notes_path(int seed) { extern char dlp_data_dir[]; char b[1200]; snprintf(b, sizeof b, "%s/darklands_notes_%u.txt", dlp_data_dir, (unsigned)seed); return b; }
static void notes_save(void)
{
    if (!notes_dirty || notes_seed < 0) return;
    notes_dirty = false;
    FILE *f = fopen(notes_path(notes_seed).c_str(), "w");
    if (f) { fwrite(notes_text.data(), 1, notes_text.size(), f); fclose(f); }
}
static int notes_resize(ImGuiInputTextCallbackData *d)
{
    if (d->EventFlag == ImGuiInputTextFlags_CallbackResize) { std::string *s = (std::string *)d->UserData; s->resize((size_t)d->BufTextLen); d->Buf = (char *)s->c_str(); }
    return 0;
}
static void notes_page(void)
{
    int seed = g_rw(G_PARTY_SEED);
    if (game_mode() == GM_OTHER) { ImGui::TextWrapped("Start or load a game first: the notes belong to a game."); return; }
    if (seed != notes_seed) {
        notes_save(); notes_seed = seed; notes_text.clear();
        FILE *f = fopen(notes_path(seed).c_str(), "r");
        if (f) { char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) notes_text.append(b, n); fclose(f); }
    }
    ImGui::TextDisabled("Your own notes for this game: clues, plans, who wanted what. Kept as darklands_notes_%u.txt.", (unsigned)seed);
    if (ImGui::InputTextMultiline("##notes", (char *)notes_text.c_str(), notes_text.capacity() + 1, ImVec2(-1, -1),
                                  ImGuiInputTextFlags_CallbackResize, notes_resize, &notes_text)) { notes_dirty = true; notes_edit_t = ImGui::GetTime(); }
    if (notes_dirty && ImGui::GetTime() - notes_edit_t > 1.5) notes_save();     /* a moment after typing stops */
    if (!ImGui::IsItemActive() && notes_dirty) notes_save();
}

static void card_load_hook(void)
{
    chronicle_capture();                   /* the last text before the card changes (a result with no options) */
    if (!journal_on) { hook_call_original(); return; }
    ensure_game();
    int loc = (s16)g_rw(G_CUR_LOC);
    if (loc != last_loc) {
        last_loc = loc;
        if (loc >= 0 && loc < (int)gamedata.cities.size()) {
            Town &t = towns[loc];
            std::string d = game_date();
            if (!t.visited) { t.visited = true; t.first = d; }
            t.last = d; t.visits++;
            if (journal_on) record_town_stock(loc);
            save();
        }
    }
    hook_call_original();
}

static void barter_list_hook(void)
{
    hook_call_original();
    if (!journal_on) return;
    ensure_game();
    int loc = (s16)g_rw(G_CUR_LOC);
    int n = (s16)g_rw(GV_barter_nwares);
    if (n < 0 || n > 200) return;
    Shop sh;
    sh.seen = true;
    sh.shop = (s16)g_rw(GV_barter_shop); sh.shopq = (s16)g_rw(GV_barter_quality);
    sh.cats = g_rw(GV_barter_cats) | ((u32)g_rw(0x886a) << 16);
    char nm[48]; g_str(g_rw(0x869e), g_rw(GV_barter_name), nm, sizeof nm);
    sh.name = game_text(nm, sizeof nm);
    sh.date = game_date();
    void *snap = eval_snapshot();
    for (int i = 0; i < n; i++) {
        u16 id = g_rw((u16)(GV_barter_wares + 2 * i));
        SeenItem it; it.id = id;
        it.q = (s16)(game_call_link(GF_ware_quality, 1, &id) & 0xffff);
        eval_restore(snap);
        it.price = (u16)(game_call_link(GF_buy_price, 1, &id) & 0xffff);
        eval_restore(snap);
        sh.items.push_back(it);
    }
    eval_free(snap);
    Town &t = towns[loc];
    if (loc >= 0 && loc < (int)gamedata.cities.size() && !t.visited) { t.visited = true; t.first = t.last = sh.date; t.visits = 1; }
    bool replaced = false;
    for (Shop &o : t.shops)
        if (o.shop == sh.shop && o.cats == sh.cats && (o.name == sh.name || !o.seen)) {
            if (!o.seen && getenv("DL_JOURNAL_LOG")) {             /* check the prediction */
                int same = o.items.size() == sh.items.size();
                for (size_t i = 0; same && i < o.items.size(); i++) same = o.items[i].id == sh.items[i].id && o.items[i].q == sh.items[i].q;
                fprintf(stderr, "journal: predicted %s: %s (%d vs %d items)\n", sh.name.c_str(), same ? "exact" : "DIFFERENT",
                        (int)o.items.size(), (int)sh.items.size());
            }
            if (!replaced) { o = sh; replaced = true; }
        }
    if (!replaced) t.shops.push_back(sh);
    save();
    if (getenv("DL_JOURNAL_LOG")) fprintf(stderr, "journal: %s, %s: %d items\n", loc_name(loc).c_str(), sh.name.c_str(), n);
}

extern "C" void journal_set(int on) { journal_on = on; config_set_int("journal.enabled", on); }
extern "C" int journal_enabled(void) { return journal_on; }

extern "C" void quests_init(void);
extern "C" void journal_init(void)
{
    journal_on = config_get_int("journal.enabled", 1);
    quests_init();                                                 /* (what the party learns of a Sabbat) */
    hook_install(GF_card_load, card_load_hook);
    hook_install(GF_text_expand, text_expand_hook);
    { void assist_watch_cards(void); assist_watch_cards(); }      /* the chronicle needs the card menu's hook (mods/assist.c) */
    hook_install(GF_barter_draw, barter_list_hook);
}

/* ---------------------------------------------------------------- the Journal tab */
static int sel_town = -1;
static char find_buf[64];

static void town_list(void)
{
    std::vector<int> v;
    for (auto &kv : towns) if (kv.second.visited || !kv.second.shops.empty()) v.push_back(kv.first);
    std::sort(v.begin(), v.end(), [](int a, int b) { return loc_name(a) < loc_name(b); });
    if (v.empty()) { ImGui::TextDisabled("No towns yet."); return; }
    if (sel_town < 0 || !towns.count(sel_town)) sel_town = v[0];
    for (int loc : v) {
        const Town &t = towns[loc];
        std::string lab = loc_name(loc);
        if (ImGui::Selectable((lab + "##" + std::to_string(loc)).c_str(), sel_town == loc)) sel_town = loc;
        if (!t.shops.empty()) { ImGui::SameLine(); ImGui::TextDisabled("(%d)", (int)t.shops.size()); }
    }
}

static const char *BUILDING[12] = { nullptr, "town hall", "fortress", nullptr, "cathedral", "barracks", "slums",   /* (bit 10, the mint: no card leads there) */
                                     "monastery", "university", "pawnshop", nullptr, "docks" };

/* an item's picture and name in a table cell: the picture inside the cell, the name centred on it */
static void icon_name_cell(int id, const char *name)
{
    float h = fmaxf(UIS(26), ImGui::GetTextLineHeight());
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(h + UIS(4), h));
    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(p, ImVec2(p.x + h + UIS(4), p.y + h), true);      /* (some pictures reach past their box: kept in the row) */
    ui_item_icon(dl, ImVec2(p.x + UIS(2) + h * 0.5f, p.y + h * 0.5f), h * 0.76f, id);
    dl->PopClipRect();
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (h - ImGui::GetTextLineHeight()) * 0.5f);
    ImGui::TextUnformatted(name);
}
static void item_row(const SeenItem &i, const char *where = nullptr)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    icon_name_cell(i.id, item_name(i.id).c_str());
    ImGui::TableNextColumn(); ImGui::Text("%d", i.q);
    ImGui::TableNextColumn(); ImGui::TextUnformatted(money(i.price).c_str());
    if (where) { ImGui::TableNextColumn(); ImGui::TextUnformatted(where); }
}

static void town_page(int loc)
{
    const Town &t = towns[loc];
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::TextUnformatted(loc_name(loc).c_str());
    ImGui::PopFont();
    if (t.visited) ImGui::TextDisabled("First reached %s, last %s (%d visit%s).", t.first.c_str(), t.last.c_str(), t.visits, t.visits == 1 ? "" : "s");
    if (loc >= 0 && loc < (int)gamedata.cities.size()) {
        const City &c = gamedata.cities[loc];
        std::string b;
        for (int k = 1; k < 12; k++) if (BUILDING[k] && (c.buildings >> k) & 1) { if (!b.empty()) b += ", "; b += BUILDING[k]; }
        ImGui::TextWrapped("Size %d.%s%s", c.size, b.empty() ? "" : " Has: ", b.c_str());
        std::string sh;
        for (int k = 0; k < 9; k++) if (c.shopq[k] && shop_open_ever(k)) {
            char e[80]; snprintf(e, sizeof e, "%s%s %dq", sh.empty() ? "" : ",  ", SHOP_NAMES[k], shop_wares_q(c.shopq[k]));
            sh += e;
        }
        ImGui::TextWrapped("Shops: %s", sh.c_str());
        ImGui::TextDisabled("The quality of a shop's weapons, armour and most wares (ammunition and ingredients are always 25q).");
    }
    if (t.shops.empty()) { ImGui::Spacing(); ImGui::TextDisabled("Look at a shop's wares and they are noted here."); return; }
    for (size_t k = 0; k < t.shops.size(); k++) {
        const Shop &s = t.shops[k];
        ImGui::PushID((int)k);
        char hd[160]; snprintf(hd, sizeof hd, "%s  (%d wares, noted %s)", s.name.c_str(), (int)s.items.size(), s.date.c_str());
        if (ImGui::CollapsingHeader(hd, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("wares", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti)) {
                ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed, UIS(80));
                ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, UIS(140));
                ImGui::TableHeadersRow();
                std::vector<int> rows; for (int r = 0; r < (int)s.items.size(); r++) rows.push_back(r);
                static std::map<std::string, TableSort> sorts;
                auto key = [&](int r, int c) -> SortKey { const SeenItem &i = s.items[r]; return c == 0 ? SortKey(item_name(i.id)) : c == 1 ? SortKey(i.q) : SortKey(i.price); };
                for (int r : table_order(sorts[loc_name(loc) + "/" + s.name], rows, key)) item_row(s.items[r]);
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }
}

struct Hit { SeenItem it; int loc; std::string shop, date; };

static void find_page(void)
{
    ImGui::SetNextItemWidth(UIS(320));
    ImGui::InputTextWithHint("##find", "item name, e.g. plate or brimstone", find_buf, sizeof find_buf);
    std::string q = find_buf;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    std::vector<Hit> hits;
    for (auto &kv : towns)
        for (const Shop &s : kv.second.shops)
            for (const SeenItem &i : s.items) {
                std::string n = item_name(i.id);
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                if (q.empty() || n.find(q) != std::string::npos) hits.push_back({ i, kv.first, s.name, s.date });
            }
    ImGui::SameLine(); ImGui::TextDisabled("%d found", (int)hits.size());
    if (ImGui::BeginTable("hits", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY,
                          ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_DefaultSort,
                                ImGui::CalcTextSize("Quality").x + ImGui::GetFontSize() * 1.3f);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("88fl 88gr 88pf").x + UIS(8));
        ImGui::TableSetupColumn("Where", ImGuiTableColumnFlags_WidthStretch, 1.25f);
        ImGui::TableHeadersRow();
        ImGuiTableSortSpecs *ss = ImGui::TableGetSortSpecs();
        int col = 1; bool desc = true;
        if (ss && ss->SpecsCount) { col = ss->Specs[0].ColumnIndex; desc = ss->Specs[0].SortDirection == ImGuiSortDirection_Descending; }
        std::stable_sort(hits.begin(), hits.end(), [&](const Hit &a, const Hit &b) {
            int c = 0;
            switch (col) {
            case 0: c = item_name(a.it.id).compare(item_name(b.it.id)); break;
            case 1: c = a.it.q - b.it.q; break;
            case 2: c = a.it.price - b.it.price; break;
            default: c = loc_name(a.loc).compare(loc_name(b.loc)); break;
            }
            return desc ? c > 0 : c < 0;
        });
        for (const Hit &h : hits) { std::string w = loc_name(h.loc) + ", " + h.shop; item_row(h.it, w.c_str()); }
        ImGui::EndTable();
    }
}

/* where the party is: the town it is in, else its tile on the map */
static bool party_where(int *x, int *y)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    u32 r = cur >= 0 ? g_location(cur) : 0;
    if (r) { *x = (s16)rw((u16)(r >> 16), (u16)(r + 4)); *y = (s16)rw((u16)(r >> 16), (u16)(r + 6)); return true; }
    if (game_mode() == GM_OTHER) return false;
    travel_party_tile(x, y);
    return *x > 0 && *y > 0;
}
static int loc_distance(int loc, int px, int py)          /* in map tiles (rows are half a tile apart) */
{
    u32 r = g_location(loc);
    if (!r) return 1 << 20;
    int dx = (s16)rw((u16)(r >> 16), (u16)(r + 4)) - px, dy = ((s16)rw((u16)(r >> 16), (u16)(r + 6)) - py) / 2;
    return (int)sqrtf((float)(dx * dx + dy * dy));
}
/* where the party can buy a thing, from the shops seen: "sold here", "Köln, 1gr 7pf" (the nearest
   town that sells it), "" when no shop seen sells it */
std::string journal_where_sold(int id)
{
    ensure_game();
    int px = 0, py = 0; bool here = party_where(&px, &py);
    int cur = (s16)g_rw(G_CUR_LOC);
    int best = -1, bd = 1 << 30, bp = 0;
    for (auto &kv : towns) {
        int price = -1; std::string shop;
        for (const Shop &s : kv.second.shops) for (const SeenItem &i : s.items) if (i.id == id && (price < 0 || i.price < price)) { price = i.price; shop = s.name; }
        if (price < 0) continue;
        if (kv.first == cur) {
            for (char &c : shop) c = (char)tolower((unsigned char)c);
            return shop.empty() ? "sold here" : "sold here (" + shop + ")";
        }
        int d = here ? loc_distance(kv.first, px, py) : 0;
        if (d < bd) { bd = d; best = kv.first; bp = price; }
    }
    if (best >= 0) return loc_name(best) + ", " + money(bp);
    if (!config_get_int("assist.where_sold_unvisited", 1)) return "";
    /* no shop seen sells it: the nearest town whose shops will have it (the wares are fixed for
       the whole game by the town's and the party's seeds: predict_stock), "expected" */
    if (id < 0 || id >= (int)gamedata.items.size()) return "";
    static int pseed = -1; static std::map<int, std::vector<int>> sold_at;      /* item -> towns */
    int seed = g_rw(G_PARTY_SEED);
    if (seed != pseed) {
        pseed = seed; sold_at.clear();
        for (int c = 0; c < (int)gamedata.cities.size(); c++)
            for (const ShopKind &k : SHOPS) {
                int shopq = k.shop < 0 ? 25 : gamedata.cities[c].shopq[k.shop];
                if (k.shop >= 0 && !shopq) continue;
                for (const SeenItem &i : predict_stock(c, seed, shopq, k.cats)) sold_at[i.id].push_back(c);
            }
    }
    auto it = sold_at.find(id);
    if (it == sold_at.end()) return "";
    bd = 1 << 30; best = -1;
    for (int c : it->second) {
        if (c == cur) return "sold here (expected)";
        int d = here ? loc_distance(c, px, py) : 0; if (d < bd) { bd = d; best = c; }
    }
    return best < 0 ? "" : loc_name(best) + " (expected)";
}

static void ingredients_page(void)
{
    int px = 0, py = 0; bool here = party_where(&px, &py);
    ImGui::TextWrapped("Where each alchemical ingredient is sold, in the towns the party has visited (bought ingredients are always "
                       "quality 25)%s.", here ? ": the nearest town first; point at a row for every town" : "");
    if (ImGui::BeginTable("ingr", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY,
                          ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Ingredient", ImGuiTableColumnFlags_WidthFixed, UIS(230));
        ImGui::TableSetupColumn(here ? "Nearest town" : "Sold at", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Also sold at", ImGuiTableColumnFlags_WidthStretch, 1.2f);
        ImGui::TableHeadersRow();
        for (int id = 72; id <= 94; id++) {
            struct Seen { int loc, price, dist; };
            std::vector<Seen> at;
            for (auto &kv : towns) {
                int best = -1;
                for (const Shop &s : kv.second.shops) for (const SeenItem &i : s.items) if (i.id == id && (best < 0 || i.price < best)) best = i.price;
                if (best >= 0) at.push_back({ kv.first, best, here ? loc_distance(kv.first, px, py) : 0 });
            }
            std::stable_sort(at.begin(), at.end(), [&](const Seen &a, const Seen &b) { return here ? a.dist < b.dist : loc_name(a.loc) < loc_name(b.loc); });
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            icon_name_cell(id, item_name(id).c_str());
            ImGui::TableNextColumn();
            if (at.empty()) { ImGui::TextDisabled("not seen yet"); ImGui::TableNextColumn(); continue; }
            int cheapest = 0; for (size_t k = 1; k < at.size(); k++) if (at[k].price < at[cheapest].price) cheapest = (int)k;
            ImGui::Text("%s  %s", loc_name(at[0].loc).c_str(), money(at[0].price).c_str());
            if (here && at[0].dist == 0) { ImGui::SameLine(); ImGui::TextDisabled("(here)"); }
            ImGui::TableNextColumn();
            /* the other towns, the nearest first, without their prices: as many as fit on the line,
               then "..."; every town with its price under the pointer */
            std::string more;
            float room = ImGui::GetContentRegionAvail().x, ell = ImGui::CalcTextSize(", ...").x;
            size_t k = 1;
            for (; k < at.size(); k++) {
                std::string next = (more.empty() ? "" : ", ") + loc_name(at[k].loc);
                bool last = k + 1 == at.size();
                if (ImGui::CalcTextSize((more + next).c_str()).x + (last ? 0 : ell) > room) break;
                more += next;
            }
            if (k < at.size()) more += more.empty() ? "..." : ", ...";
            ImGui::TextUnformatted(more.empty() ? "-" : more.c_str());
            if (ImGui::IsItemHovered() && at.size() > 1) {
                ImGui::BeginTooltip();
                ImGui::TextDisabled(here ? "Every town seen selling it, the nearest first:" : "Every town seen selling it:");
                for (size_t j = 0; j < at.size(); j++) ImGui::Text("%s  %s%s", loc_name(at[j].loc).c_str(), money(at[j].price).c_str(), (int)j == cheapest && at.size() > 2 ? "  (the cheapest)" : "");
                ImGui::EndTooltip();
            }
        }
        ImGui::EndTable();
    }
}

/* ---------------------------------------------------------------- stored goods
 * What the party has left with innkeepers (the store rooms of s035): the game keeps them in
 * CACHE.TMP (saved with the game): byte 1 the number of caches, a 16-bit offset per cache at
 * 2 x handle, there a count and {item u16, quality, number} each; a location record's +0x18 is
 * its cache handle (from 1). The original lists a store only when the party stands in it. With
 * one store room for every inn (mods/sharedstore.c) the towns point at the same cache: listed once. */
extern "C" void dos_host_path(const char *dos, char *out, size_t outn);
extern "C" int sharedstore_on(void);       /* mods/sharedstore.c */
static void storage_page(void)
{
    int gm = game_mode();
    if (gm == GM_OTHER) { ImGui::TextWrapped("Start or load a game first."); return; }
    if (gm == GM_BATTLE) { ImGui::TextWrapped("After the battle."); return; }
    std::vector<unsigned char> buf;
    { char path[1200]; dos_host_path("CACHE.TMP", path, sizeof path);
      FILE *f = fopen(path, "rb");
      if (f) { unsigned char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) buf.insert(buf.end(), b, b + n); fclose(f); } }
    int ncache = buf.size() > 1 ? buf[1] : 0;
    {
      if (sharedstore_on()) ImGui::TextWrapped("What the party has left with innkeepers (\"store items with the innkeeper\" at a town's inn): one store room, the same at every inn (F12 > Settings > Helpers).");
      else ImGui::TextWrapped("What the party has left with innkeepers (\"store items with the innkeeper\" at a town's inn), and where. "
                              "The game shows a store only while the party is there."); }
    ImGui::Spacing();
    int px, py; travel_party_tile(&px, &py);
    struct Store { int loc, places; double d; std::vector<SeenItem> items; };
    std::vector<Store> stores;
    std::map<int, int> at;                                     /* handle -> its entry in stores */
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        int h = (s16)rw((u16)(r >> 16), (u16)((u16)r + 0x18));
        if (h < 1 || h > ncache || (size_t)(2 * h + 1) >= buf.size()) continue;
        int x = (s16)rw((u16)(r >> 16), (u16)((u16)r + 4)), y = (s16)rw((u16)(r >> 16), (u16)((u16)r + 6));
        double d = px >= 0 ? hypot((double)(x - px), (y - py) / 4.0) : 0;
        auto it0 = at.find(h);
        if (it0 != at.end()) { Store &st = stores[it0->second]; st.places++; if (d < st.d) { st.d = d; st.loc = k; } continue; }
        size_t o = buf[2 * h] | buf[2 * h + 1] << 8;
        if (o >= buf.size()) continue;
        int n = buf[o];
        Store st; st.loc = k; st.places = 1; st.d = d;
        for (int i = 0; i < n && o + 1 + 4 * i + 3 < buf.size(); i++) {
            const unsigned char *e = &buf[o + 1 + 4 * i];
            SeenItem it; it.id = (e[0] | e[1] << 8) & 0xfff; it.q = e[2]; it.price = e[3];   /* (price: the number) */
            if (it.price) st.items.push_back(it);
        }
        at[h] = (int)stores.size();
        stores.push_back(st);
    }
    stores.erase(std::remove_if(stores.begin(), stores.end(), [](const Store &s) { return s.items.empty(); }), stores.end());
    if (stores.empty()) { ImGui::TextDisabled("Nothing stored anywhere."); return; }
    std::sort(stores.begin(), stores.end(), [](const Store &a, const Store &b) { return a.places != b.places ? a.places > b.places : a.d < b.d; });
    int cur = (s16)g_rw(G_CUR_LOC);
    ImGui::BeginChild("stores", ImVec2(0, 0));
    for (const Store &st : stores) {
        ImGui::PushID(st.loc);
        std::string kinds = std::to_string(st.items.size()) + (st.items.size() == 1 ? " kind of thing" : " kinds of things");
        std::string head = st.places > 1 ? "Every inn's store room  -  " + kinds
                                         : loc_name(st.loc) + (st.loc == cur ? "  (here)" : "") + "  -  " + kinds;
        if (ImGui::CollapsingHeader(head.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("st", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Quality").x + UIS(10));
                ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Number").x + UIS(10));
                ImGui::TableHeadersRow();
                for (const SeenItem &it : st.items) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); icon_name_cell(it.id, item_name(it.id).c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%d", it.q);
                    ImGui::TableNextColumn(); ImGui::Text("%d", it.price);
                }
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
}

extern "C" void battlelog_tab(void);
int ui_want_subtab = -1;
extern "C" void journal_quests(void);               /* scripted tests: utab 2.1 */

extern "C" void halloffame_open(void);
extern "C" int halloffame_record(void);
extern "C" void tab_journal(void)
{
    if (cur_seed < 0 && game_mode() != GM_OTHER) ensure_game();
    int want = ui_want_subtab; ui_want_subtab = -1;
    if (ui_subtabs_begin("jtabs")) {
        if (ui_subtab("Quests", want == 3)) {
            ImGui::BeginChild("q", ImVec2(0, 0));
            journal_quests();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ui_subtab("Towns", want == 0)) {
            ImGui::BeginChild("list", ImVec2(UIS(250), 0), ImGuiChildFlags_Border);
            town_list();
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("page", ImVec2(0, 0), ImGuiChildFlags_Border);
            if (sel_town >= 0 && towns.count(sel_town)) town_page(sel_town);
            else ImGui::TextWrapped("The towns the party reaches are noted here, with their shops, and the wares of "
                                    "every shop you look at (quality and price).");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ui_subtab("Chronicle", want == 7)) { chronicle_page(); ImGui::EndTabItem(); }
        if (ui_subtab("Notes", want == 8)) { notes_page(); ImGui::EndTabItem(); }
        if (ui_subtab("Find goods", want == 1)) { find_page(); ImGui::EndTabItem(); }
        if (ui_subtab("Ingredients", want == 2)) { ingredients_page(); ImGui::EndTabItem(); }
        if (ui_subtab("Stored", want == 6)) { storage_page(); ImGui::EndTabItem(); }
        if (ui_subtab("Battle log", want == 5)) { battlelog_tab(); ImGui::EndTabItem(); }
        if (ui_subtab("Honor Roll", want == 4)) {
            ImGui::TextWrapped("The Honor Roll of Glory: the four most famous parties. MicroProse drew it and wrote the code for it, "
                               "but never finished it (the manual mentions a Hall of Fame, the game's README says there is none). "
                               "A party enters it when its journey ends, or now with the button.");
            ImGui::Spacing();
            if (ImGui::Button("Show the Honor Roll of Glory")) halloffame_open();
            ImGui::SameLine();
            static std::string msg;
            ImGui::BeginDisabled(game_mode() == GM_OTHER);
            if (ImGui::Button("Enter the party now")) msg = halloffame_record() ? "The party is on the roll." : "Not famous enough yet for the four best (or the roll could not be written).";
            ImGui::EndDisabled();
            if (!msg.empty()) ImGui::TextDisabled("%s", msg.c_str());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

/* for other tabs: has the party been to city loc? */
extern "C" int journal_visited(int loc) { auto it = towns.find(loc); return it != towns.end() && it->second.visited; }
