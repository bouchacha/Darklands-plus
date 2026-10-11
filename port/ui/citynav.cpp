// citynav.cpp - "Go to" in cities. A city in Darklands is a maze of card menus (main street,
// market, churches, the crafts district, side streets ...); the list here names every place at
// once, and picking one walks the party there through the game's own menus - the very options a
// player would click, one card after the other, at the game's pace. It stops at anything it
// does not know (an encounter on the way, a closed gate), at a key press or click of the
// player, and when it has chosen the place.
//
// Cards are told apart by their names (s043 card_name, DS:e3d4: "$MainS01" ...); the option
// lines by their text in MSGFILES (the card file's text block D_ee53, option k = the k-th
// option mark), so nothing depends on the crisp text or on the screen.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
int datamods_generation(void);
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "../mods/assist.h"
u32 plat_ticks_ms(void);
void plat_click_game(int gx, int gy);
void ui_notice(const char *msg);
}

/* ---------------------------------------------------------------- the card files */
static std::map<std::string, std::vector<std::string>> cards;   /* "$MAINS01" -> text blocks */
static bool cards_loaded;

static void load_cards(void)
{
    cards_loaded = true;
    char path[1300]; dos_host_path_read("MSGFILES", path, sizeof path);
    FILE *f = fopen(path, "rb");
    if (!f) return;
    std::vector<unsigned char> d;
    unsigned char buf[65536]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) d.insert(d.end(), buf, buf + n);
    fclose(f);
    if (d.size() < 2) return;
    unsigned cnt = d[0] | d[1] << 8;
    for (unsigned i = 0; i < cnt && 2 + (i + 1) * 24 <= d.size(); i++) {
        const unsigned char *e = &d[2 + i * 24];
        char name[13]; memcpy(name, e, 12); name[12] = 0;
        for (char *p = name; *p; p++) { *p = (char)toupper((unsigned char)*p); if (*p == '.') *p = 0; }
        unsigned len = e[16] | e[17] << 8 | e[18] << 16 | (unsigned)e[19] << 24, off = e[20] | e[21] << 8 | e[22] << 16 | (unsigned)e[23] << 24;
        if ((size_t)off + len > d.size() || len < 1) continue;
        const unsigned char *m = &d[off], *end = m + len;
        unsigned nb = *m++;
        std::vector<std::string> blocks;
        for (unsigned b = 0; b < nb && m + 5 <= end; b++) {
            m += 5;
            const unsigned char *z = m; while (z < end && *z) z++;
            blocks.emplace_back((const char *)m, (size_t)(z - m));
            m = z + 1;
        }
        cards[name] = blocks;
    }
}

/* the option texts of a text block, by option number (lower case, "$vars" kept) */
static std::vector<std::string> block_options(const std::string &t)
{
    std::vector<std::string> o;
    for (size_t i = 0; i < t.size(); i++) {
        unsigned char c = (unsigned char)t[i];
        if (c != 0x15 && c != 0x16 && c != 0x10 && c != 0x06) continue;
        size_t e = t.find('\n', i + 1); if (e == std::string::npos) e = t.size();
        std::string s = t.substr(i + 1, e - i - 1);
        size_t sep = s.find('\x1d'); if (sep != std::string::npos) s = s.substr(sep + 1);
        for (char &ch : s) ch = (char)tolower((unsigned char)ch);
        o.push_back(s);
        i = e - 1;
    }
    return o;
}

static std::string card_now(void)
{
    char n[13]; g_str(G_DGROUP, 0xe3d4, n, sizeof n);
    for (char *p = n; *p; p++) *p = (char)toupper((unsigned char)*p);
    return n;
}

/* ---------------------------------------------------------------- the city */
enum Hub { H_MAIN, H_MARKET, H_CHURCHES, H_CRAFTS, H_ARMS, H_CIVIL, H_SIDE, H_N, H_NONE = -1 };
static const char *HUB_CARD[H_N] = { "$MAINS", "$MARKE", "$CHURC", "$BUSIN", "$MILCR", "$CIVCR", "$SIDES" };

enum Dest {
    D_MERCHANT, D_FOREIGN, D_SWORD, D_ARMOR, D_BOW, D_BLACKSMITH, D_SOLDIERS, D_JEWEL, D_TINKER, D_CLOTH,
    D_ALCHEMIST, D_HERBAL, D_PHARMACIST, D_PHYSICIAN,
    D_FUGGER, D_MEDICI, D_HANSE, D_PAWN,
    D_CATHEDRAL, D_CHURCH, D_MONASTERY, D_UNIVERSITY, D_HOSPITAL,
    D_INN, D_SQUARE, D_FORTRESS, D_DOCKS, D_SLUM, D_GROVE, D_WALL, D_LEAVE, D_N
};
struct DestInfo { const char *label; int group; int building; int shop; };   /* building bit / shop quality index of DARKLAND.CTY, -1 = any */
static const DestInfo DEST[D_N] = {
    { "Merchants (everyday goods)", 0, -1, 1 }, { "Foreign traders", 0, -1, -1 }, { "Swordsmith", 0, -1, 2 }, { "Armorer", 0, -1, 3 },
    { "Bowyers and gunsmiths", 0, -1, -2 }, { "Blacksmiths", 0, -1, 0 }, { "Soldiers' Road", 0, -1, -1 }, { "Jewelers", 0, -1, 7 },
    { "Tinkers and artificers", 0, -1, 6 }, { "Clothmakers", 0, -1, 8 },
    { "Alchemists (Astrologists' Lane)", 1, -1, -1 }, { "Herbalists and barber-surgeons", 1, -1, -1 }, { "Pharmacists", 1, -1, -1 }, { "Physician", 1, -1, -1 },
    { "Fugger bank", 2, -1, -1 }, { "Medici office", 2, -1, -1 }, { "Hanseatic League hall", 2, -1, -1 }, { "Pawnshop (Leihhaus)", 2, 9, -1 },
    { "Cathedral", 3, 4, -1 }, { "Church", 3, -1, -1 }, { "Monastery (Kloster)", 3, 7, -1 }, { "University", 3, 8, -1 }, { "Hospital", 3, -1, -1 },
    { "Inn", 4, -1, -1 }, { "Town square (city council)", 4, 1, -1 }, { "Fortress", 4, 2, -1 }, { "Docks", 4, 11, -1 }, { "Slums", 4, 6, -1 },
    { "Grove (to wait)", 4, -1, -1 }, { "City wall", 4, -1, -1 }, { "Leave the city (a gate)", 4, -1, -1 },
};
static const char *GROUPS[5] = { "Shops and crafts", "Healing and alchemy", "Money", "Church and learning", "Places" };

/* what an option of a hub card leads to: a place (D_*) or another hub (100 + H_*) */
struct Rule { int hub; const char *key; int to; };
static const Rule RULES[] = {
    { H_MAIN, "$citysquare", D_SQUARE }, { H_MAIN, "$fortress", D_FORTRESS }, { H_MAIN, "$marketplace", 100 + H_MARKET },
    { H_MAIN, "spires", 100 + H_CHURCHES }, { H_MAIN, "craft", 100 + H_CRAFTS }, { H_MAIN, "$inn", D_INN }, { H_MAIN, "docks", D_DOCKS },
    { H_MAIN, "side street", 100 + H_SIDE }, { H_MAIN, "grove", D_GROVE }, { H_MAIN, "gate", D_LEAVE },
    { H_MARKET, "everyday", D_MERCHANT }, { H_MARKET, "foreign", D_FOREIGN }, { H_MARKET, "pharmacist", D_PHARMACIST },
    { H_MARKET, "fugger bank", D_FUGGER }, { H_MARKET, "medici", D_MEDICI }, { H_MARKET, "hanseatic", D_HANSE },
    { H_MARKET, "leihhaus", D_PAWN }, { H_MARKET, "main street", 100 + H_MAIN }, { H_MARKET, "side street", 100 + H_SIDE },
    { H_CHURCHES, "$cathedral", D_CATHEDRAL }, { H_CHURCHES, "$citychurch", D_CHURCH }, { H_CHURCHES, "$hospital", D_HOSPITAL },
    { H_CHURCHES, "$monastery", D_MONASTERY }, { H_CHURCHES, "$university", D_UNIVERSITY }, { H_CHURCHES, "$citysquare", D_SQUARE },
    { H_CHURCHES, "$fortress", D_FORTRESS }, { H_CHURCHES, "main street", 100 + H_MAIN }, { H_CHURCHES, "side street", 100 + H_SIDE },
    { H_CRAFTS, "arms and armor", 100 + H_ARMS }, { H_CRAFTS, "less violent", 100 + H_CIVIL }, { H_CRAFTS, "$inn", D_INN },
    { H_CRAFTS, "physician", D_PHYSICIAN }, { H_CRAFTS, "grove", D_GROVE }, { H_CRAFTS, "$slum", D_SLUM },
    { H_CRAFTS, "main street", 100 + H_MAIN }, { H_CRAFTS, "side street", 100 + H_SIDE }, { H_CRAFTS, "city wall", D_WALL },
    { H_CRAFTS, "gate", D_LEAVE },
    { H_ARMS, "soldier", D_SOLDIERS }, { H_ARMS, "blacksmith", D_BLACKSMITH }, { H_ARMS, "swordsmith", D_SWORD },
    { H_ARMS, "armorer", D_ARMOR }, { H_ARMS, "bowyer", D_BOW }, { H_ARMS, "other crafts", 100 + H_CIVIL },
    { H_ARMS, "side street", 100 + H_SIDE }, { H_ARMS, "main street", 100 + H_MAIN },
    { H_CIVIL, "physician", D_PHYSICIAN }, { H_CIVIL, "astrologist", D_ALCHEMIST }, { H_CIVIL, "herbalist", D_HERBAL },
    { H_CIVIL, "jeweler", D_JEWEL }, { H_CIVIL, "tinker", D_TINKER }, { H_CIVIL, "clothmaker", D_CLOTH },
    { H_CIVIL, "-making", 100 + H_ARMS }, { H_CIVIL, "side street", 100 + H_SIDE }, { H_CIVIL, "main street", 100 + H_MAIN },
    { H_SIDE, "main street", 100 + H_MAIN }, { H_SIDE, "$inn", D_INN }, { H_SIDE, "$cityslums", D_SLUM },
    { H_SIDE, "grove", D_GROVE }, { H_SIDE, "city wall", D_WALL },
};

static int hub_of(const std::string &card)
{
    for (int h = 0; h < H_N; h++) if (!strncmp(card.c_str(), HUB_CARD[h], 6)) return h;
    return H_NONE;
}
static int rule_for(int hub, const std::string &text)
{
    for (const Rule &r : RULES) if (r.hub == hub && text.find(r.key) != std::string::npos) return r.to;
    return -1;
}

/* steps from each hub to a place, through the hubs (side streets cost more: more encounters) */
static int dist_to(int dest, int from, unsigned avoid)
{
    int d[H_N]; for (int h = 0; h < H_N; h++) d[h] = 1000;
    for (const Rule &r : RULES) if (r.to == dest && !(avoid >> r.hub & 1)) d[r.hub] = 1;
    for (int it = 0; it < H_N; it++)
        for (const Rule &r : RULES)
            if (r.to >= 100 && !(avoid >> r.hub & 1)) {
                int c = d[r.to - 100] + (r.to - 100 == H_SIDE ? 3 : 1);
                if (c < d[r.hub]) d[r.hub] = c;
            }
    return d[from];
}

/* ---------------------------------------------------------------- walking */
static int goal = -1, steps_taken;
static unsigned visited;                                   /* hubs passed on this walk: not again */
static int conts, leaves;                                  /* texts read on, places left before the first hub */
static std::string last_key; static int same_count;
static u32 phase_t, last_act;
static std::string phase_key;

static int logon = -1;
#define LOG(...) do { if (logon < 0) logon = getenv("DL_CITYNAV_LOG") != 0; if (logon) fprintf(stderr, "citynav: " __VA_ARGS__); } while (0)

static void stop(const char *why)
{
    LOG("stop: %s\n", why ? why : "");
    if (goal >= 0 && why) { char m[160]; snprintf(m, sizeof m, "Go to %s: %s", DEST[goal].label, why); ui_notice(m); }
    goal = -1;
}

extern "C" void citynav_user_input(void) { if (goal >= 0) stop("stopped."); }
extern "C" int citynav_busy(void) { return goal >= 0; }

static bool in_city(void)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    return cur >= 0 && cur < 0x5c && game_mode() == GM_CARD;
}
extern "C" int citynav_available(void) { return in_city(); }

static bool leave_option(const std::string &t)
{
    if (t.find("main street") == std::string::npos) return false;
    static const char *verbs[] = { "leave", "return", "go out", "move away", "stroll away", "back down", "way to", "exit" };
    for (const char *v : verbs) if (t.find(v) != std::string::npos) return true;
    return false;
}

static int choose_leave(const std::vector<std::string> &line)
{
    for (size_t i = 0; i < line.size(); i++) if (leave_option(line[i])) return (int)i;
    for (size_t i = 0; i < line.size(); i++) if (line[i].find("return to") != std::string::npos && line[i].find("side") == std::string::npos) return (int)i;
    for (size_t i = 0; i < line.size(); i++)
        if ((line[i].find("leave") != std::string::npos || line[i].find("depart") != std::string::npos) && line[i].find("side") == std::string::npos
            && line[i].find("alley") == std::string::npos) return (int)i;
    for (size_t i = 0; i < line.size(); i++)                 /* the only way out may be a side street (the slums) or a step back */
        if (line[i].find("leave") != std::string::npos || line[i].find("fall back") != std::string::npos || line[i].find("reconsider") != std::string::npos) return (int)i;
    return -1;
}

static void click_line(int i)
{
    LOG("click line %d\n", i);
    int x0 = 60 + (s16)g_rw(0xe972), y0 = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i));
    plat_click_game(x0 + 10, y0 + (h > 8 ? 4 : h / 2));
    last_act = plat_ticks_ms(); steps_taken++;
}

extern "C" void citynav_tick(void)
{
    if (goal < 0) return;
    { static int gen = -1; int datamods_generation(void); if (gen != datamods_generation()) { gen = datamods_generation(); cards.clear(); cards_loaded = false; } }
    if (!cards_loaded) load_cards();
    u32 now = plat_ticks_ms();
    if (now - last_act < 350) return;                      /* let the game take the last click */
    if (!in_city()) { stop(game_mode() == GM_CARD ? "left the city." : "interrupted."); return; }
    if (steps_taken > 20) { stop("too many steps; stopped."); return; }
    std::string card = card_now();
    int blk = (s16)g_rw(0xee53);
    char kb[48]; snprintf(kb, sizeof kb, "%s/%d/%d", card.c_str(), blk, assist_in_card_menu());
    if (phase_key != kb) { phase_key = kb; phase_t = now; }
    int hub = hub_of(card);
    if (!assist_in_card_menu()) {                        /* the card's text: read on (hubs, and the place the walk starts from) */
        if (now - phase_t < 450) return;
        if (hub == H_NONE) {
            if (visited || conts >= 4) { stop(steps_taken ? "something happened on the way." : "leave this place first."); return; }
            conts++;
        }
        LOG("%s: read on\n", kb);
        plat_click_game(304, 12); last_act = now;
        return;
    }
    if (now - phase_t < 250) return;                       /* the option list fading in */
    if (kb == last_key) { if (++same_count > 3) { stop("the way is blocked."); return; } } else { last_key = kb; same_count = 0; }
    auto it = cards.find(card);
    if (it == cards.end() || blk < 0 || blk >= (int)it->second.size()) { stop("unknown card."); return; }
    std::vector<std::string> opts = block_options(it->second[blk]);
    int n = (s8)g_rb(0xe96e);
    if (n <= 0 || n > 20) return;
    if (g_rb(0xee41)) return;                              /* a skill-check box is up */
    std::vector<std::string> line(n);
    for (int i = 0; i < n; i++) { int o = (s8)g_rb((u16)(0xe9b0 + i)); line[i] = o >= 0 && o < (int)opts.size() ? opts[o] : ""; }
    if (logon) { LOG("%s hub %d visited %x:\n", kb, hub, visited); for (int i = 0; i < n; i++) LOG("  %d: %s -> %d\n", i, line[i].c_str(), hub >= 0 ? rule_for(hub, line[i]) : -1); }
    if (hub == H_NONE) {                                   /* a shop or other place of the city: out the way the player would leave */
        if (visited || leaves >= 3) { stop("something happened on the way."); return; }
        int pick = choose_leave(line);
        if (pick < 0) { stop(leaves ? "something happened on the way." : "leave this place first."); return; }
        leaves++; click_line(pick);
        return;
    }
    for (int i = 0; i < n; i++) if (rule_for(hub, line[i]) == goal) { click_line(i); goal = -1; return; }   /* there */
    visited |= 1u << hub;
    int best = -1, bd = 1000;
    for (int i = 0; i < n; i++) {
        int to = rule_for(hub, line[i]);
        if (to < 100 || (visited >> (to - 100) & 1)) continue;
        int d = dist_to(goal, to - 100, visited) + (to - 100 == H_SIDE ? 3 : 1);
        if (d < bd) { bd = d; best = i; }
    }
    if (best < 0 || bd >= 1000) { stop("not found in this city."); return; }
    click_line(best);
}

extern "C" void citynav_go(int dest)
{
    if (dest < 0 || dest >= D_N) return;
    assist_watch_cards();
    goal = dest; steps_taken = 0; visited = 0; conts = leaves = 0; same_count = 0; last_key.clear(); phase_key.clear(); last_act = 0;
    char m[120]; snprintf(m, sizeof m, "Going to: %s (a key or click stops)", DEST[dest].label); ui_notice(m);
}

/* ---------------------------------------------------------------- the list */
static bool present(int d)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (cur < 0 || cur >= (int)gamedata.cities.size()) return true;
    const City &c = gamedata.cities[cur];
    if (DEST[d].building >= 0 && !(c.buildings >> DEST[d].building & 1)) return false;
    if (DEST[d].shop >= 0 && !c.shopq[DEST[d].shop]) return false;
    if (DEST[d].shop == -2 && !c.shopq[4] && !c.shopq[5]) return false;
    return true;
}

/* the "Go to" window; returns false when it closes */
extern "C" int citynav_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 90));
    float w = fminf(io.DisplaySize.x * 0.94f, 1000), maxh = io.DisplaySize.y * 0.94f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, maxh));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    bool keep = true;
    int cur = (s16)g_rw(G_CUR_LOC);
    std::string title = "Go to...";
    if (cur >= 0 && cur < (int)gamedata.cities.size()) title += "   (" + gamedata.cities[cur].name + ")";
    title += "###citynav";
    int picked = -1;
    float scale = fminf(1.0f, fmaxf(0.7f, io.DisplaySize.y / 900.0f));
    if (ImGui::Begin(title.c_str(), &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetWindowFontScale(scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8 * scale, 6 * scale));
        ImGui::TextDisabled("The party walks there through the city's own menus. A key or a click stops it.");
        const int cols = 4;
        float bw = (ImGui::GetContentRegionAvail().x - (cols - 1) * ImGui::GetStyle().ItemSpacing.x) / cols;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.78f, 0.66f, 0.47f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.24f, 0.13f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.18f, 0.10f, 1));
        for (int g = 0; g < 5; g++) {
            ImGui::SeparatorText(GROUPS[g]);
            int col = 0;
            for (int d = 0; d < D_N; d++) {
                if (DEST[d].group != g) continue;
                if (col % cols) ImGui::SameLine();
                bool here = present(d);
                if (!here) ImGui::BeginDisabled();
                ImGui::PushID(d);
                bool hov = false;
                ImVec2 p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("b", ImVec2(bw, ImGui::GetFrameHeight() * 1.1f))) picked = d;
                hov = ImGui::IsItemHovered();
                ImDrawList *dl = ImGui::GetWindowDrawList();
                ImU32 bg = !here ? IM_COL32(205, 188, 158, 255) : hov ? IM_COL32(140, 61, 33, 255) : IM_COL32(199, 168, 120, 255);
                ImU32 fg = !here ? IM_COL32(130, 112, 90, 255) : hov ? IM_COL32(252, 242, 220, 255) : IM_COL32(51, 31, 15, 255);
                ImVec2 sz = ImGui::GetItemRectSize();
                dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), bg, 4);
                ImVec2 ts = ImGui::CalcTextSize(DEST[d].label);
                dl->AddText(ImVec2(p.x + (sz.x - ts.x) * 0.5f, p.y + (sz.y - ts.y) * 0.5f), fg, DEST[d].label);
                ImGui::PopID();
                if (!here) { ImGui::EndDisabled(); if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Not in this city."); }
                col++;
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
    }
    ImGui::End();
    if (picked >= 0) { citynav_go(picked); return 0; }
    return keep;
}
