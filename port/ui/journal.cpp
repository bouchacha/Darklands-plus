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
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/hooks.h"
#include "../runtime/config.h"
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
    int d = (s16)g_rw(G_CUR_DAY) + 1, m = g_rw(G_CUR_MONTH), y = g_rw(G_CUR_YEAR);
    char b[32]; snprintf(b, sizeof b, "%d %s %d", d, MONTHS[m % 12], y);
    return b;
}

static std::string loc_name(int loc)
{
    if (loc >= 0 && loc < (int)gamedata.cities.size()) return gamedata.cities[loc].name;
    u32 r = g_location(loc);
    if (!r) return "?";
    char b[24]; g_str((u16)(r >> 16), (u16)(r + 0x26), b, sizeof b);
    return game_text(b, sizeof b);
}

static std::string money(int pf)
{
    int fl = pf / 240, gr = pf % 240 / 12, p = pf % 12;
    char b[40];
    if (fl) snprintf(b, sizeof b, "%dfl %dgr %dpf", fl, gr, p);
    else if (gr) snprintf(b, sizeof b, "%dgr %dpf", gr, p);
    else snprintf(b, sizeof b, "%dpf", p);
    return b;
}

static std::string item_name(int id)
{
    if (id >= 0 && id < (int)gamedata.items.size()) return game_text(gamedata.items[id].name.c_str(), 64);
    return "#" + std::to_string(id);
}

/* ---------------------------------------------------------------- file */
static std::string path_for(int seed)
{
    char b[1200]; snprintf(b, sizeof b, "%s/darklands_journal_%u.txt", game_dir, (unsigned)seed);
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
    { 8, 0x04000000u, "Clothmaker" }, { -1, 0x00000400u, "Herbalist" } };

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
        g_ww(0x8834, (u16)shopq); g_ww(0x8836, (u16)k.shop);
        for (SeenItem &i : sh.items) { u16 id = (u16)i.id; i.price = (u16)(game_call_link(32, 0x1462, 0x192e, 1, &id) & 0xffff); }
        bool have = false;
        for (Shop &o : t.shops) if (o.shop == sh.shop && o.cats == sh.cats) { have = true; if (!o.seen) o = sh; }
        if (!have) t.shops.push_back(sh);
    }
    eval_restore(snap);
    eval_free(snap);
}

/* ---------------------------------------------------------------- hooks */
static void card_load_hook(void)
{
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
    int n = (s16)g_rw(0x885c);
    if (n < 0 || n > 200) return;
    Shop sh;
    sh.seen = true;
    sh.shop = (s16)g_rw(0x8836); sh.shopq = (s16)g_rw(0x8834);
    sh.cats = g_rw(0x8868) | ((u32)g_rw(0x886a) << 16);
    char nm[48]; g_str(g_rw(0x869e), g_rw(0x869c), nm, sizeof nm);
    sh.name = game_text(nm, sizeof nm);
    sh.date = game_date();
    void *snap = eval_snapshot();
    for (int i = 0; i < n; i++) {
        u16 id = g_rw((u16)(0x86a2 + 2 * i));
        SeenItem it; it.id = id;
        it.q = (s16)(game_call_link(32, 0x1462, 0x0efa, 1, &id) & 0xffff);
        eval_restore(snap);
        it.price = (u16)(game_call_link(32, 0x1462, 0x192e, 1, &id) & 0xffff);
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

extern "C" void journal_init(void)
{
    journal_on = config_get_int("journal.enabled", 1);
    hook_install(43, 0x150b, 0x000a, card_load_hook);
    hook_install(32, 0x1462, 0x078e, barter_list_hook);
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

static const char *BUILDING[12] = { nullptr, "town hall", "fortress", nullptr, "cathedral", "Zeughaus (armoury)", "slums",
                                     "Kloster", "university", "pawnshop", "Muenze (mint)", "docks" };

static void item_row(const SeenItem &i, const char *where = nullptr)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(24, 24));
    ui_item_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + 12, p.y + 12), 22, i.id);
    ImGui::SameLine();
    ImGui::TextUnformatted(item_name(i.id).c_str());
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
        for (int k = 0; k < 9; k++) if (c.shopq[k] && k != 4 && k != 7) {     /* no card opens the gunsmith or jeweller */
            char e[64]; snprintf(e, sizeof e, "%s%s %d", sh.empty() ? "" : ",  ", SHOP_NAMES[k], c.shopq[k]);
            sh += e;
        }
        ImGui::TextWrapped("Shops (quality): %s", sh.c_str());
        ImGui::TextDisabled("Goods are about (item quality + shop quality) / 2.");
    }
    if (t.shops.empty()) { ImGui::Spacing(); ImGui::TextDisabled("Look at a shop's wares and they are noted here."); return; }
    for (size_t k = 0; k < t.shops.size(); k++) {
        const Shop &s = t.shops[k];
        ImGui::PushID((int)k);
        char hd[160]; snprintf(hd, sizeof hd, "%s  (%d wares, noted %s)", s.name.c_str(), (int)s.items.size(), s.date.c_str());
        if (ImGui::CollapsingHeader(hd, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("wares", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
                ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 140);
                ImGui::TableHeadersRow();
                for (const SeenItem &i : s.items) item_row(i);
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }
}

struct Hit { SeenItem it; int loc; std::string shop, date; };

static void find_page(void)
{
    ImGui::SetNextItemWidth(320);
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
        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_DefaultSort, 80);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableSetupColumn("Where", ImGuiTableColumnFlags_WidthFixed, 300);
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

static void ingredients_page(void)
{
    ImGui::TextWrapped("Where each alchemical ingredient is sold, in the towns the party has visited (bought ingredients are always quality 25).");
    if (ImGui::BeginTable("ingr", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY,
                          ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Ingredient", ImGuiTableColumnFlags_WidthFixed, 230);
        ImGui::TableSetupColumn("Sold at", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (int id = 72; id <= 94; id++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(26, 26));
            ui_item_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + 13, p.y + 13), 24, id);
            ImGui::SameLine(); ImGui::TextUnformatted(item_name(id).c_str());
            ImGui::TableNextColumn();
            std::string where;
            for (auto &kv : towns) {
                int best = -1;
                for (const Shop &s : kv.second.shops) for (const SeenItem &i : s.items) if (i.id == id && (best < 0 || i.price < best)) best = i.price;
                if (best >= 0) { if (!where.empty()) where += ",  "; where += loc_name(kv.first) + " " + money(best); }
            }
            if (where.empty()) ImGui::TextDisabled("not seen yet");
            else ImGui::TextWrapped("%s", where.c_str());
        }
        ImGui::EndTable();
    }
}

int ui_want_subtab = -1;               /* scripted tests: utab 2.1 */

extern "C" void tab_journal(void)
{
    if (cur_seed < 0 && game_mode() != GM_OTHER) ensure_game();
    int want = ui_want_subtab; ui_want_subtab = -1;
    if (ImGui::BeginTabBar("jtabs")) {
        if (ImGui::BeginTabItem("Towns", nullptr, want == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {
            ImGui::BeginChild("list", ImVec2(250, 0), ImGuiChildFlags_Border);
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
        if (ImGui::BeginTabItem("Find goods", nullptr, want == 1 ? ImGuiTabItemFlags_SetSelected : 0)) { find_page(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Ingredients", nullptr, want == 2 ? ImGuiTabItemFlags_SetSelected : 0)) { ingredients_page(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
}

/* for other tabs: has the party been to city loc? */
extern "C" int journal_visited(int loc) { auto it = towns.find(loc); return it != towns.end() && it->second.visited; }
