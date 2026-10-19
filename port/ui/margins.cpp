// margins.cpp - on a wide screen the 4:3 picture leaves dark margins; they can show what the
// game otherwise keeps behind menus: the date and hour, where the party is, the purse, the
// tasks the party has taken on (with their deadlines), and on the other side the town the party
// is in, the blessings and potions working on the party, and who carries too much.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "imgui.h"
#include "gamedata.h"
#include "teachersui.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
#include "../mods/menuview.h"
#include "../mods/assist.h"
#include "../mods/foesleft.h"
#include "../mods/gamemodel.h"
#include "../mods/fixes.h"
#include "../mods/saintinfo.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
extern ImFont *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
#include "effects.h"
#include "quests.h"
#include "townicons.h"
#include "libraries.h"
#include <functional>
#include <set>
#include "money.h"
extern "C" int citynav_available(void);
extern "C" int battlelog_margin(float x, float w, float h, float fs);
extern "C" int battle_showing(void);                  /* ui/battlelog.cpp: a battle is on (not the marching order) */

int ui_margins_on = 1;

extern "C" int levelmap_available(void);
static const char *MONTHS[12] = { "January", "February", "March", "April", "May", "June", "July", "August",
                                  "September", "October", "November", "December" };

static std::string place_name(void)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (cur < 0) {                                   /* in the wilds: the nearest town or place */
        int px, py; travel_party_tile(&px, &py);
        int best = -1; long bd = 1L << 30;
        for (int k = 0; k < G_NLOC; k++) {
            u32 r = g_location(k);
            if (!r) continue;
            int x = (s16)rw((u16)(r >> 16), (u16)((u16)r + 4)), y = (s16)rw((u16)(r >> 16), (u16)((u16)r + 6));
            long dx = (x - px) * 4, dy = (y - py), d = dx * dx + dy * dy;      /* tiles are 16x4 apart */
            if (d < bd) { bd = d; best = k; }
        }
        if (best < 0 || bd > 60 * 60) return "In the wilds";
        return "Travelling, near " + loc_display_name(best);
    }
    return loc_display_name(cur);
}

static std::string member_name(int m)
{
    char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
    return game_text(n, sizeof n);
}

static bool begin_panel(const char *id, float x, float w, float h, float size)
{
    ImGui::SetNextWindowPos(ImVec2(x, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(size * 0.7f, size * 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(size * 0.3f, size * 0.22f));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(232, 218, 186, 255));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, IM_COL32(170, 150, 120, 255));
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(120, 92, 56, 200));
    bool open = ImGui::Begin(id, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
    ImGui::PushFont(ui_font_overlay);
    ImGui::SetWindowFontScale(size / ui_font_overlay->FontSize);
    return open;
}
static void end_panel(void)
{
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
/* the last item under the pointer: the margins' windows take no input (a click there must not
   reach ImGui's windows nor the game), so ImGui never counts their items as hovered */
static bool item_hovered(void) { return ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), false); }
/* a tooltip from the margin: in the tooltips' own ink (the margin's text is light, for its black) */
static void margin_tip(const char *text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(44, 26, 10, 255));
    ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20); ImGui::TextUnformatted(text); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
    ImGui::PopStyleColor();
}
static void heading(const char *t)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(222, 178, 104, 255));
    ImGui::TextUnformatted(t);
    ImGui::PopStyleColor();
}

/* in a battle: everybody's condition at a glance - the party with endurance and strength,
 * the enemies the party has seen (in the battle's hot spots) with how hurt they are */
std::string battlelog_name(int i);
static void bar(float frac, ImU32 col, float w, float hgt)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + hgt), IM_COL32(60, 46, 30, 255), hgt * 0.4f);
    if (frac > 0) dl->AddRectFilled(p, ImVec2(p.x + w * frac, p.y + hgt), col, hgt * 0.4f);
    ImGui::Dummy(ImVec2(w, hgt));
}
extern "C" int partypanel_shown(void);
/* the orders for the member being given them: the game's Attack and Orders menus as buttons
   (only what the game allows now; chosen at the battle's next round, as from the menu). The
   margins take no input from ImGui: the buttons are drawn here and their clicks come through
   margins_click (ui_event) */
static void place_quests(int cur);

/* what a town offers that is worth a detour, as a row of pictures (what each offers under the
 * pointer, in the words of the town's own cards): the places most towns lack - a university (13
 * towns), a cathedral, an alchemist, a physician (some towns, some games), the docks' boats, a
 * Fugger or Medici bank. Not shown: what nearly every town has (a monastery: 90 of 92) or what
 * only pays later with reputation (the town hall's and the fortress's audiences); the rest is in
 * F12 > Journal > Towns. Then the notable shops: those whose wares come out at 30q or better (the
 * standard is 25q), a picture and the quality each. `tables`: the location table may be read;
 * `width`: where the rows wrap (0: the room left); `words`: each picture's name after it (where
 * no tooltip can show: the world map's tooltip on a town). */
static const char *TOWN_OFFER_TIP[6] = {
    "University: new saints and alchemical formulae to learn, rare alchemical materials, a better philosopher's stone, teachers to hire (it expects Latin and a sharp mind).",
    "Cathedral: donations for divine favor (and maybe virtue), a relic from the canon, the patron saint's story, sanctuary.",
    "An alchemist: formulae to buy or trade, a better philosopher's stone, lessons in alchemy.",
    "A physician: wounds treated for a fee, lessons in healing, alchemical components bought and sold.",
    "Docks: boats to other towns, for a fee (each on the day it leaves).",
    "A bank (Fugger or Medici): heavy coin into a bank note that weighs nothing, and back, for a charge; tasks for the bold." };
static const struct { int shop, icon; const char *wares; } NOTABLE_SHOPS[] = {
    { 2, TI_SWORD, "weapons" }, { 3, TI_ARMOUR, "armour and shields" }, { 5, TI_BOW, "bows, crossbows and guns" },
    { 0, TI_SMITH, "weapons" }, { 6, TI_TOOLS, "hammers, maces, flails and polearms (pikes, halberds, spears, staves)" } };
enum { NOTABLE_Q = 30 };
static const char *TOWN_OFFER_WORD[6] = { "University", "Cathedral", "Alchemist", "Physician", "Docks", "Bank" };
void ui_town_offers(int loc, bool tables, float width, bool words)
{
    static int seen_loc = -1; static bool has[6];
    if (tables || seen_loc != loc) { for (int k = 0; k < 6; k++) has[k] = citynav_town_offers(loc, k); seen_loc = loc; }
    const City &c = gamedata.cities[loc];
    float lh = ImGui::GetTextLineHeight(), gap = lh * 0.3f;
    float x0 = ImGui::GetCursorScreenPos().x, w = width > 0 ? width : ImGui::GetContentRegionAvail().x;
    int nf = 0; for (int k = 0; k < 6; k++) nf += has[k];
    float isz = lh * 1.35f;                                 /* (smaller, down to a line's height, to keep the row on one line) */
    if (!words && nf > 1 && nf * isz + (nf - 1) * gap > w) isz = fmaxf(lh * 1.0f, (w - (nf - 1) * gap) / nf);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    /* a row of chips that wraps: a picture, and a short text after it */
    auto chip = [&](bool &first, int icon, const char *text, const char *tip) {
        float tw = text ? ImGui::CalcTextSize(text).x + lh * 0.25f : 0, cw = isz + tw;
        if (!first) { ImGui::SameLine(0, gap); if (ImGui::GetCursorScreenPos().x + cw > x0 + w) ImGui::NewLine(); }
        first = false;
        ImVec2 a = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(cw, isz));
        ui_draw_town_icon(dl, ImVec2(a.x + isz * 0.5f, a.y + isz * 0.5f), isz, icon);
        if (text) dl->AddText(ImVec2(a.x + isz + lh * 0.2f, a.y + (isz - lh) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), text);
        if (item_hovered()) margin_tip(tip);
    };
    bool first = true;
    for (int k = 0; k < 6; k++) if (has[k]) chip(first, k, words ? TOWN_OFFER_WORD[k] : nullptr, TOWN_OFFER_TIP[k]);
    bool head = false;
    for (const auto &s : NOTABLE_SHOPS) {
        int q = c.shopq[s.shop] > 0 ? shop_wares_q(c.shopq[s.shop]) : 0;
        if (q < NOTABLE_Q) continue;
        if (!head) { ImGui::Spacing(); if (words) ImGui::TextDisabled("Notable shops"); else heading("Notable shops"); head = true; first = true; }
        char t[48], tip[160]; snprintf(t, sizeof t, words ? "%s %dq" : "%.0s%dq", SHOP_NAMES[s.shop], q);
        snprintf(tip, sizeof tip, "%s: %s of %dq (most shops' are about 25q).", SHOP_NAMES[s.shop], s.wares, q);
        chip(first, s.icon, t, tip);
    }
}
/* the saints a place's library teaches (ui/libraries.h), one a line (their names may hold a comma:
   "St.Edward, Confessor"): those nobody in the party knows in the margin's ink, the others dim;
   what each does under the pointer */
static void library_line(int loc)
{
    const Library *L = library_at(loc);
    if (!L) return;
    ImGui::Spacing(); heading("Saints taught here");
    int shown[4], ns = 0;
    for (int k = 0; k < 4; k++) {
        int s = L->saint[k]; bool dup = false;
        for (int j = 0; j < ns; j++) if (shown[j] == s) dup = true;
        if (dup || s < 0 || s >= (int)gamedata.saints.size()) continue;
        shown[ns++] = s;
    }
    for (int j = 0; j < ns; j++) {
        int s = shown[j]; bool known = party_knows_saint(s);
        const std::string &t = gamedata.saints[s].name;
        if (known) ImGui::TextDisabled("%s", t.c_str()); else ImGui::TextUnformatted(t.c_str());
        if (item_hovered()) {
            std::string tip = gamedata.saints[s].name + (known ? " (known in the party): " : " (nobody in the party knows this saint): ") + saint_info_does(s);   /* (what its help really does: mods/saintinfo.h) */
            margin_tip(tip.c_str());
        }
    }
}
extern "C" const char *journal_town_fact(int loc, const char *key);   /* ui/journal.cpp */
/* the formulae a town's alchemist sells (ui/libraries.h), one a line: those nobody in the party knows
   in the margin's ink, the others dim; the potion, its grade, the price and what it does under the
   pointer */
static void formulae_line(int town)
{
    int fo[4], pr[4];
    if (town < 0 || town >= (int)gamedata.cities.size() || !alchemist_formulae(town, fo, pr)) return;
    ImGui::Spacing(); heading("Formulae sold here");
    for (int k = 0; k < 4; k++) {
        int f = fo[k];
        if (f < 0 || f >= (int)gamedata.formula_names.size()) continue;
        bool known = party_knows_formula(f);
        const std::string &t = gamedata.formula_names[f];
        char price[16]; snprintf(price, sizeof price, "%d fl", pr[k]);
        bool fits = ImGui::CalcTextSize(t.c_str()).x + ImGui::GetStyle().ItemSpacing.x + ImGui::CalcTextSize(price).x <= ImGui::GetContentRegionAvail().x;
        ImGui::BeginGroup();                                 /* (the price beside the name when it fits; always under the pointer) */
        if (known) ImGui::TextDisabled("%s", t.c_str()); else ImGui::TextUnformatted(t.c_str());
        if (fits) { ImGui::SameLine(); ImGui::TextDisabled("%s", price); }
        ImGui::EndGroup();
        if (item_hovered()) {
            const ItemDef *pd = 95 + f / 3 < (int)gamedata.items.size() ? &gamedata.items[95 + f / 3] : nullptr;
            char head[160]; snprintf(head, sizeof head, "%s: %s, grade %d of 3, %d florins%s. ", t.c_str(), pd ? pd->name.c_str() : "?", f % 3 + 1, pr[k],
                                     known ? " (known in the party)" : " (nobody in the party knows it)");
            std::string tip = head + (f < (int)gamedata.formulas.size() ? gamedata.formulas[f].text : std::string());
            margin_tip(tip.c_str());
        }
    }
}
/* the teachers the town can offer (mods/teachers.c), a line a source: who, what, the chance of a
   day's lesson; dim when not to be had now; the whole under the pointer. Those hired now first. */
static void teachers_lines(int loc)
{
    if (!ui_teachers_on()) return;
    TchSource ts[12];
    int n = ui_town_teachers(loc, ts, 12);
    std::string hired = ui_teachers_hired_line(loc);
    if (!n && hired.empty()) return;
    ImGui::Spacing(); heading("Teachers to be hired");
    if (!hired.empty()) {
        ImGui::TextDisabled("At the inn now:"); ImGui::SameLine(); ImGui::TextWrapped("%s", hired.c_str());
        if (item_hovered()) margin_tip("Hired teachers: at the inn, \"Train or study\", a lesson a day for a fee.");
    }
    for (int k = 0; k < n; k++) {
        const TchSource &s = ts[k];
        char line[160]; snprintf(line, sizeof line, fix_is_on("teachcap") ? "%s (to %d)" : "%s (skill %d)", ui_teacher_skills(s).c_str(), s.strength);
        ImGui::BeginGroup();
        ImGui::TextDisabled("%s:", ui_teacher_short(s.src)); ImGui::SameLine();
        if (s.off) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", line);
        if (s.off) ImGui::PopStyleColor();
        ImGui::EndGroup();
        if (item_hovered()) margin_tip(ui_teacher_text(s).c_str());
    }
}
extern "C" int ui_nearest_town(void);
/* the right margin names the place (a town, or near one on the map, or the place of a card) */
static bool right_shows_place(void)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (cur < 0) return game_mode() == GM_MAP && ui_nearest_town() >= 0;
    return cur < (int)gamedata.cities.size() || (cur < G_NLOC && game_mode() == GM_CARD);
}
/* "until 16 July 1400" -> "until 16 July" in 1400 */
static std::string short_date(const std::string &d, int year)
{
    std::string y = " " + std::to_string(year);
    return d.size() > y.size() && d.compare(d.size() - y.size(), y.size(), y) == 0 ? d.substr(0, d.size() - y.size()) : d;
}
struct OrderBtn { ImVec2 a, b; int m, i, s; };
static std::vector<OrderBtn> order_btns; static int order_btns_frame = -100;
/* other things in the margins that take a click (a quest opened or closed, the order of the
   tasks): their places as last drawn */
struct MClick { ImVec2 a, b; std::function<void()> f; };
static std::vector<MClick> mclicks; static int mclicks_frame = -100;
static void mclick(ImVec2 a, ImVec2 b, std::function<void()> f)
{
    if (mclicks_frame != ImGui::GetFrameCount()) { mclicks.clear(); mclicks_frame = ImGui::GetFrameCount(); }
    mclicks.push_back({ a, b, f });
}
extern "C" int margins_click(int x, int y)
{
    if (ImGui::GetFrameCount() - mclicks_frame <= 2)
        for (const MClick &c : mclicks)
            if (x >= c.a.x && x < c.b.x && y >= c.a.y && y < c.b.y) { c.f(); return 1; }
    if (ImGui::GetFrameCount() - order_btns_frame > 2) return 0;
    for (const OrderBtn &o : order_btns)
        if (x >= o.a.x && x < o.b.x && y >= o.a.y && y < o.b.y) { menuview_request(o.m, o.i, o.s); return 1; }
    return 0;
}
/* a potion among the orders (the game's short name; its quality is the entry's "key", "q30"),
   for its picture on the button; -1 none */
void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q);
static int potion_of(const std::string &name, const char *key, int *q)
{
    for (int id = 95; id <= 116 && id < (int)gamedata.items.size(); id++)
        if (!gamedata.items[id].shortname.empty() && name == gamedata.items[id].shortname) {
            *q = key[0] == 'q' ? atoi(key + 1) : -1;
            return id;
        }
    return -1;
}
static void order_buttons(float width)
{
    order_btns.clear();
    if (!menuview_is_on() || g_rb(GV_order_mode) != 1) return;
    int sel = (s8)g_rb(GV_selected_member);
    if (sel < 0 || sel > 4) return;
    const MenuModel *M = menuview_live();
    char nm[32]; { u16 b = G_MEMBER(sel); int i = 0; for (; i < 31; i++) { char c = (char)g_rb((u16)(b + 0x2a + i)); if (!c) break; nm[i] = c; } nm[i] = 0; }
    std::string who = game_text(nm, strlen(nm)); size_t sp = who.find(' '); if (sp != std::string::npos) who = who.substr(0, sp);
    bool head = false;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o0 = ImGui::GetCursorScreenPos(), mp = ImGui::GetIO().MousePos;
    float x0 = o0.x, x1 = o0.x + width - ImGui::GetStyle().WindowPadding.x * 2, x = x0, y = 0, lh = ImGui::GetTextLineHeight(), pad = lh * 0.3f, gap = lh * 0.25f;
    for (int j = 0; j < M->nmenus; j++) {
        std::string mn = M->menu[j].name;
        if (!M->menu[j].enabled || (mn != "Attack" && mn != "Orders")) continue;
        for (int k = 0; k < M->menu[j].n; k++) {
            int ix = M->menu[j].first + k;
            const MvItem &it = M->items[ix];
            int nsub = M->sub_n[ix];
            for (int s2 = nsub ? 0 : -1; s2 < (nsub ? nsub : 0); s2++) {
                const MvItem &e = s2 >= 0 ? M->items[M->sub_first[ix] + s2] : it;
                if (!e.enabled || (s2 >= 0 && !it.enabled)) continue;
                if (!head) { heading(("Orders for " + who).c_str()); head = true; y = ImGui::GetCursorScreenPos().y; x = x0; }
                std::string label = game_text(e.name, strlen(e.name));
                if (s2 >= 0) label = game_text(it.name, strlen(it.name)) + ": " + (e.count > 1 ? std::to_string(e.count) + " " : "") + label;
                int pq = -1, pid = s2 >= 0 ? potion_of(game_text(e.name, strlen(e.name)), e.key, &pq) : -1;
                if (e.key[0] && pid < 0) label += std::string(" (") + e.key + ")";      /* (a potion's quality: the dots on its picture) */
                float iw = pid >= 0 ? lh * 1.15f : 0;                /* (a potion: its picture, with its grade) */
                ImVec2 ts = ImGui::CalcTextSize(label.c_str());
                float w = ts.x + pad * 2 + iw, h = lh + pad;
                if (x > x0 && x + w > x1) { x = x0; y += h + gap; }
                ImVec2 a(x, y), b(x + w, y + h);
                bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
                dl->AddRectFilled(a, b, hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), h * 0.2f);
                dl->AddRect(a, b, IM_COL32(170, 132, 70, 255), h * 0.2f);
                if (pid >= 0) ui_item_icon_q(dl, ImVec2(a.x + pad + iw * 0.45f, a.y + h * 0.5f), iw, pid, pq);
                dl->AddText(ImVec2(a.x + pad + iw, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), label.c_str());
                order_btns.push_back({ a, b, j, k, s2 });
                x += w + gap;
            }
        }
    }
    if (head) {
        ImGui::SetCursorScreenPos(ImVec2(x0, y + lh + pad + gap));
        ImGui::Dummy(ImVec2(1, 1));
        ImGui::Separator();
        order_btns_frame = ImGui::GetFrameCount();
    }
}

/* the foes the place still holds, not yet met (mods/foesleft.c): where the rest of a castle's or
   dungeon's defenders wait, so a fight that "does not end" says why */
static void foes_left_section(void)
{
    FoesLeft fl;
    if (!foesleft_on() || !foesleft_get(&fl)) return;
    int here = fl.foes[fl.floor], others = 0;
    for (int f = 0; f < fl.floors; f++) if (f != fl.floor && fl.known[f]) others += fl.foes[f];
    if (fl.floors <= 1 && !here) return;                     /* (an open field with no one waiting: nothing to say) */
    ImGui::Spacing();
    heading("Not yet met");
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (!here && !others) ImGui::TextWrapped("No one else waits in this place.");
    else {
        if (here && fl.floors <= 1) ImGui::TextWrapped("Here: %d more %s, in %d %s.", here, here == 1 ? "foe" : "foes", fl.groups[fl.floor], fl.groups[fl.floor] == 1 ? "group" : "groups");
        else if (here) ImGui::TextWrapped("On this floor (%d): %d %s, in %d %s.", fl.floor + 1, here, here == 1 ? "foe" : "foes", fl.groups[fl.floor], fl.groups[fl.floor] == 1 ? "group" : "groups");
        else ImGui::TextWrapped("No one on this floor (%d).", fl.floor + 1);
        std::string o;
        for (int f = 0; f < fl.floors; f++) {
            if (f == fl.floor || !fl.known[f] || !fl.foes[f]) continue;
            char b[48]; snprintf(b, sizeof b, "%sfloor %d: %d", o.empty() ? "" : ", ", f + 1, fl.foes[f]);
            o += b;
        }
        if (!o.empty()) ImGui::TextWrapped("Elsewhere: %s.", o.c_str());
        if (fl.master_floor >= 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(214, 120, 90, 255));
            if (fl.master_floor == fl.floor) ImGui::TextWrapped("The master of this place waits on this floor.");
            else ImGui::TextWrapped("The master of this place waits on floor %d.", fl.master_floor + 1);
            ImGui::PopStyleColor();
        }
    }
    ImGui::PopStyleColor();
}

static void battle_panel(float width, float fs)
{
    static bool seen[13]; static u16 last_tick = 0xffff;
    static int last_st[13]; static std::string last_name[13];      /* the fallen: the game clears their records */
    u16 tick = g_rw(0xe092);                        /* the battle's clock: back at 0 in a new battle */
    if (tick < last_tick) for (int i = 0; i < 13; i++) { seen[i] = false; last_st[i] = 0; last_name[i].clear(); }
    last_tick = tick;
    int n = (s8)g_rb(GV_hotspot_count);
    for (int k = 0; k < n && k < 40; k++) { int m = g_rb((u16)(GV_hotspot_cbt + k)); if (m < 13) seen[m] = true; }
    float bw = width - fs * 1.6f, bh = fs * 0.32f;
    if (config_get_int("battle.fast_key", 1) && !config_get_int("original", 0)) {   /* fast forward (Z, mods/pathfix.c) */
        const char *t = battle_fast ? "Back to the chosen speed (Z)" : "Fast forward (Z)";
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos(), ts = ImGui::CalcTextSize(t), mp = ImGui::GetIO().MousePos;
        float pad = ImGui::GetTextLineHeight() * 0.3f;
        ImVec2 a = p, b(p.x + ts.x + pad * 2, p.y + ts.y + pad);
        bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
        dl->AddRectFilled(a, b, battle_fast ? IM_COL32(150, 60, 30, 255) : hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), (b.y - a.y) * 0.2f);
        dl->AddText(ImVec2(a.x + pad, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), t);
        mclick(a, b, [] { battle_fast_set(!battle_fast); });
        ImGui::Dummy(ImVec2(b.x - a.x, b.y - a.y + pad));
    }
    order_buttons(width);
    if (!partypanel_shown()) heading("The party");      /* (the party panel drawn anew has them already) */
    for (int i = 0; i < 5 && !partypanel_shown(); i++) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + 0x14));
        if (!st) continue;
        int end = (s8)g_rb((u16)(b + 0x5d)), endm = (s8)g_rb((u16)(b + 0x64)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
        const char *cond = st == 3 ? "dead" : st == 2 ? "unconscious" : st == 5 ? "fled" : "";
        if (*cond) ImGui::TextDisabled("%s - %s", member_name(i).c_str(), cond);
        else {
            ImGui::TextUnformatted(member_name(i).c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%d/%d  %d/%d", end, endm, str, strm);
            bar(endm > 0 ? (float)end / endm : 0, IM_COL32(110, 130, 230, 255), bw, bh);
            bar(strm > 0 ? (float)str / strm : 0, IM_COL32(90, 180, 90, 255), bw, bh);
        }
    }
    if (!partypanel_shown()) ImGui::Separator();
    heading("Enemies");
    int shown = 0, down = 0;
    for (int i = 5; i < 13; i++) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + 0x14)), endm = (s8)g_rb((u16)(b + 0x64));
        if (!seen[i]) continue;
        if (!st || endm <= 0) {
            if (last_st[i] >= 2 && !last_name[i].empty()) { ImGui::TextDisabled("%s - %s", last_name[i].c_str(), last_st[i] == 3 ? "dead" : last_st[i] == 5 ? "fled" : "unconscious"); shown++; down++; }
            continue;
        }
        int end = (s8)g_rb((u16)(b + 0x5d)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
        std::string nm = battlelog_name(i);
        last_st[i] = st; last_name[i] = nm;
        const char *cond = st == 3 ? "dead" : st == 2 ? "unconscious" : st == 5 ? "fled" : end * 4 <= endm ? "badly hurt" : end * 2 <= endm ? "hurt" : end < endm ? "a little hurt" : "unhurt";
        if (st == 1 && strm > 0 && str * 2 <= strm) cond = "gravely wounded";
        if (st != 1) { ImGui::TextDisabled("%s - %s", nm.c_str(), cond); down++; }
        else {
            ImGui::TextUnformatted(nm.c_str()); ImGui::SameLine(); ImGui::TextDisabled("%s", cond);
            bar((float)end / endm, IM_COL32(200, 90, 70, 255), bw, bh);
            if (g_rb((u16)(GV_cbt_ends_battle + i)) == 1) { ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(214, 120, 90, 255)); ImGui::TextWrapped("The master of this place: once this one has fallen and the fight is over, the place is won."); ImGui::PopStyleColor(); }
        }
        shown++;
    }
    if (!shown) {
        ImGui::TextDisabled("None in sight yet.");
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("To explore: in Single File (Q) or as a Group (G), a click on a door, a passage or stairs walks the party there%s.",
                           !levelmap_available() ? "" : config_get_int("assist.level_map_walk", 1) ? "; Tab shows the level as far as seen, and a click on it walks the party there" : "; Tab shows the level as far as seen");
        ImGui::PopStyleColor();
    }
    else if (down == shown) {
        ImGui::TextDisabled("All down.");
        /* a fight in a street or a dungeon: the party stays there afterwards (combat_flags 1) and the
           game goes on only when it walks out through a way out (s014 combat_over) */
        if ((g_rb(GV_combat_flags) & 1) && !g_rb(GV_real_combat)) {
            ImGui::Spacing();
            ImGui::TextWrapped("The fight is won. To go on, walk the party out of this place: click a way out (the edge of the street, a door, stairs).");
        }
    }
    foes_left_section();
}

/* the text size for what the interface writes over and beside the picture (side panels,
 * tooltips, names on the map): grows with the picture, times display.text_size (%) */
extern "C" float ui_text_px(void)
{
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float ph = y1 - y0; if (ph < 200) ph = ImGui::GetIO().DisplaySize.y;
    float k = config_get_int("display.text_size", 100) / 100.0f; if (k < 0.5f) k = 0.5f; if (k > 2.5f) k = 2.5f;
    float px = ph / 34.0f; if (px < 14) px = 14;
    return px * k;
}

/* the right margin, out of battles: what is not on the party's cards at the left - the town
 * the party is in (its standing there, the buildings and shops), the blessings and potions
 * working and how long they last, and who carries too much */
extern "C" int ui_nearest_town(void)                /* in the wilds: the nearest town, if one is near (-1 none) */
{
    int px, py; travel_party_tile(&px, &py);
    int best = -1; long bd = 1L << 30;
    for (int k = 0; k < (int)gamedata.cities.size() && k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        int x = (s16)rw((u16)(r >> 16), (u16)((u16)r + 4)), y = (s16)rw((u16)(r >> 16), (u16)((u16)r + 6));
        long dx = (x - px) * 4, dy = (y - py), d = dx * dx + dy * dy;      /* tiles are 16x4 apart */
        if (d < bd) { bd = d; best = k; }
    }
    return bd <= 60 * 60 ? best : -1;
}

static void right_panel(int size)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    bool any = false, near = false;
    if (cur < 0 && game_mode() == GM_MAP) { cur = ui_nearest_town(); near = true; }
    if (cur >= 0 && cur < (int)gamedata.cities.size()) {
        const City &c = gamedata.cities[cur];
        heading(((near ? "Near " : "") + c.name).c_str());
        if (!near && citynav_available() && config_get_int("assist.city_goto", 1)) ImGui::TextDisabled("G: go to a place here");
        /* the location table is read while a card or the map is up; battles and their loot
           reuse that memory, so meanwhile the last value read stands */
        static std::map<int, int> rep_seen; static int rep_game = -1;
        if (rep_game != (s16)g_rw(G_PARTY_SEED)) { rep_seen.clear(); rep_game = (s16)g_rw(G_PARTY_SEED); }
        int gm = game_mode();
        u32 lr = g_location(cur);
        if (lr && (gm == GM_CARD || gm == GM_MAP)) rep_seen[cur] = (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12));
        int fame = (s16)g_rw(GV_party_fame);
        auto rp = rep_seen.find(cur);
        if (rp != rep_seen.end()) { ImGui::Text("Reputation %d", rp->second); ImGui::SameLine(); ImGui::TextDisabled(" fame %d", fame); }
        else ImGui::TextDisabled("Fame %d", fame);
        /* the town's attitude: it bars the party, or its watch wants it (its sign, a short line, the
           meaning under the pointer) */
        static std::vector<TownMark> tm; static int tmf = -1000;
        if (ImGui::GetFrameCount() - tmf > 30 && (gm == GM_CARD || gm == GM_MAP)) { town_marks(tm); tmf = ImGui::GetFrameCount(); }
        for (const TownMark &t : tm) if (t.loc == cur) {
            float lh = ImGui::GetTextLineHeight(), is = lh * 1.15f;
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(p0.x + is * 0.5f, p0.y + lh * 0.5f), is, t.cat);
            ImGui::Indent(is + lh * 0.3f);
            ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.43f, 1), "%s %s", t.cat == QC_UNWELCOME ? "Barred" : "Wanted", short_date(t.until, (s16)g_rw(G_CUR_YEAR)).c_str());
            if (item_hovered()) margin_tip(t.cat == QC_UNWELCOME ? "The town counts as hostile: its gates and people turn the party away."
                                                                                       : "The watch is after the party: guards who recognise it try to arrest it, and talking or bribing one's way out is harder.");
            ImGui::Unindent(is + lh * 0.3f);
        }
        ui_town_offers(cur, gm == GM_CARD || gm == GM_MAP);
        {   /* what the party learned of its physician and alchemist (ui/journal.cpp) */
            const char *ph = journal_town_fact(cur, "physician"), *al = journal_town_fact(cur, "alchemist");
            if (ph) { ImGui::TextDisabled("Physician:"); ImGui::SameLine(); ImGui::TextWrapped("%s", ph); }
            if (al) { ImGui::TextDisabled("Alchemist:"); ImGui::SameLine(); ImGui::TextWrapped("%s", al); }
        }
        library_line(cur);
        formulae_line(cur);
        teachers_lines(cur);
        place_quests(cur);
        any = true;
    }
    else if (cur >= (int)gamedata.cities.size() && cur < G_NLOC && game_mode() == GM_CARD) {   /* (battles reuse the location table) */
        /* a castle, monastery, village ...: what it is, and the quests that point there */
        u32 r = g_location(cur);
        if (r) {
            const char *loc_kind_label(int k);
            heading(loc_display_name(cur).c_str());
            ImGui::TextDisabled("%s", loc_kind_label((s16)rw((u16)(r >> 16), (u16)r)));
            library_line(cur);
            place_quests(cur);
            if ((s16)rw((u16)(r >> 16), (u16)((u16)r + 0x18)) > 0) ImGui::TextWrapped("The party has goods stored here (F12 > Journal > Stored).");
            any = true;
        }
    }
    std::vector<Eff> e = effects_now(size);
    if (!e.empty()) {
        if (any) ImGui::Separator();
        heading("In force");
        /* one paragraph a member: what is at work, then how long it lasts (from the soonest to the last) */
        auto span = [](long h) { char b[24]; if (h < 48) snprintf(b, sizeof b, "%ld h", h); else snprintf(b, sizeof b, "%ld d", (h + 12) / 24); return std::string(b); };
        for (int m = 0; m < 5; m++) {
            std::string what; long lo = -1, hi = -1;
            for (const Eff &x : e) {
                if (x.m != m) continue;
                if (!what.empty()) what += ", ";
                what += x.what;
                if (lo < 0 || x.left < lo) lo = x.left;
                if (x.left > hi) hi = x.left;
            }
            if (what.empty()) continue;
            std::string when = span(lo); if (span(hi) != when) when += " to " + span(hi);
            ImGui::TextWrapped("%s: %s (%s)", member_name(m).c_str(), what.c_str(), when.c_str());
        }
        any = true;
    }
    if (!any) { ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)); ImGui::TextWrapped("No blessings or potions at work."); ImGui::PopStyleColor(); }
}

/* the quests that lead to place `cur`, each with its picture (the place's section of the margin) */
static void place_quests(int cur)
{
    static std::vector<std::pair<int, std::string>> marks; static std::vector<int> cats; static int mf = -1000;
    int gm = game_mode();                                   /* (read while a card or the map is up: battles and their loot reuse the tables) */
    if (ImGui::GetFrameCount() - mf > 30 && (gm == GM_CARD || gm == GM_MAP)) { quest_marks_cat(marks, cats); mf = ImGui::GetFrameCount(); }
    for (size_t i = 0; i < marks.size(); i++) {
        if (marks[i].first != cur) continue;
        float lh = ImGui::GetTextLineHeight(), is = lh * 1.15f;
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(p0.x + is * 0.5f, p0.y + lh * 0.5f), is, i < cats.size() ? cats[i] : 0);
        ImGui::Indent(is + lh * 0.3f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.78f, 0.40f, 1)); ImGui::TextWrapped("%s", marks[i].second.c_str()); ImGui::PopStyleColor();
        ImGui::Unindent(is + lh * 0.3f);
    }
}

/* the party's tasks: a short title each (a click opens it: the whole task, the deadline, how
   far), the days left; in order of the deadline, of the distance or as taken (a click on the
   order changes it) */
static void tasks_section(bool tables)
{
    static std::vector<QuestRow> q; static int qf = -1000; static bool q_read = false;
    static std::set<std::string> open;
    static int sort = -1;
    { int cs = config_get_int("quests.sort", QSORT_DEADLINE) % QSORT_N; if (cs != sort) { sort = cs; quest_sort(q, sort); } }   /* (the journal may change it) */
    if (ImGui::GetFrameCount() - qf > 30 && (tables || !q_read)) { quest_rows(q, sort); qf = ImGui::GetFrameCount(); q_read = tables; }
    ImVec2 mp = ImGui::GetIO().MousePos;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float w = ImGui::GetContentRegionAvail().x, lh = ImGui::GetTextLineHeight();
    auto over = [&](ImVec2 a, ImVec2 b) { return mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y; };
    {   /* the heading, and the order at its right */
        ImVec2 h0 = ImGui::GetCursorScreenPos();
        heading("Tasks");
        if (q.size() > 1) {
            std::string t = std::string("by ") + QSORT_NAME[sort];
            float tw = ImGui::CalcTextSize(t.c_str()).x;
            ImVec2 a(h0.x + w - tw - lh * 0.2f, h0.y), b(h0.x + w, h0.y + lh * 1.2f);
            bool h = over(a, b);
            dl->AddText(ImVec2(a.x, a.y + lh * 0.1f), h ? IM_COL32(232, 200, 130, 255) : IM_COL32(170, 150, 120, 255), t.c_str());
            dl->AddLine(ImVec2(a.x, a.y + lh * 1.05f), ImVec2(a.x + tw, a.y + lh * 1.05f), h ? IM_COL32(232, 200, 130, 200) : IM_COL32(170, 150, 120, 110));
            mclick(a, b, [&]() { sort = (sort + 1) % QSORT_N; config_set_int("quests.sort", sort); config_save(); quest_sort(q, sort); });
        }
    }
    if (q.empty()) { ImGui::TextDisabled("None taken on."); return; }
    for (size_t i = 0; i < q.size() && i < 10; i++) {
        const QuestRow &r = q[i];
        bool is_open = open.count(r.id) != 0;
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        std::string days = r.group == 3 ? "" : quest_days(r);
        float dw = days.empty() ? 0 : ImGui::CalcTextSize(days.c_str()).x + lh * 0.4f;
        float isz = lh * 1.2f, ind = lh * 0.85f + isz + lh * 0.3f;     /* the arrow, the quest's picture, the title */
        /* the arrow, the title (wrapped short of the days), the days */
        ImU32 tc = r.group == 1 ? IM_COL32(150, 210, 120, 255) : r.group == 3 ? IM_COL32(230, 140, 110, 255) : IM_COL32(232, 218, 186, 255);
        if (r.days_left >= 0 && r.days_left <= 7 && r.group != 3) tc = IM_COL32(240, 170, 90, 255);
        ImGui::SetCursorScreenPos(ImVec2(p0.x + ind, p0.y));
        ImGui::PushTextWrapPos(p0.x + w - dw - ImGui::GetWindowPos().x);
        ImGui::PushStyleColor(ImGuiCol_Text, tc);
        ImGui::TextWrapped("%s", r.title.c_str());
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();
        float ty1 = ImGui::GetCursorScreenPos().y;
        float ax = p0.x + lh * 0.3f, ay = p0.y + lh * 0.5f, s = lh * 0.22f;
        ImU32 ac = IM_COL32(200, 170, 120, 255);
        if (is_open) dl->AddTriangleFilled(ImVec2(ax - s, ay - s * 0.6f), ImVec2(ax + s, ay - s * 0.6f), ImVec2(ax, ay + s * 0.8f), ac);
        else dl->AddTriangleFilled(ImVec2(ax - s * 0.6f, ay - s), ImVec2(ax - s * 0.6f, ay + s), ImVec2(ax + s * 0.8f, ay), ac);
        ui_draw_quest_icon(dl, ImVec2(p0.x + lh * 0.85f + isz * 0.5f, p0.y + lh * 0.52f), isz, r.cat);
        if (!days.empty()) dl->AddText(ImVec2(p0.x + w - dw + lh * 0.4f, p0.y), r.days_left <= 7 ? IM_COL32(240, 170, 90, 255) : IM_COL32(170, 150, 120, 255), days.c_str());
        if (is_open) {
            ImGui::Indent(ind);
            ImGui::TextWrapped("%s", r.text.c_str());
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));   /* (wrapped: the margin is narrow) */
            if (!r.when.empty()) ImGui::TextWrapped("%s", r.when.c_str());
            std::string d = quest_distance(r);
            if (!d.empty()) { d[0] = (char)toupper((unsigned char)d[0]); ImGui::TextWrapped("%s", d.c_str()); }
            ImGui::PopStyleColor();
            if (r.loc >= 0 && r.dist >= 1.5 && travel_enabled && game_mode() == GM_MAP) {     /* (on the travel map: go there) */
                const char *t = "Travel there";
                ImVec2 tp = ImGui::GetCursorScreenPos(), tsz = ImGui::CalcTextSize(t);
                bool h = over(tp, ImVec2(tp.x + tsz.x, tp.y + tsz.y));
                ImU32 tc = h ? IM_COL32(250, 220, 150, 255) : IM_COL32(220, 186, 120, 255);
                dl->AddText(tp, tc, t);
                dl->AddLine(ImVec2(tp.x, tp.y + tsz.y), ImVec2(tp.x + tsz.x, tp.y + tsz.y), tc);
                ImGui::Dummy(tsz);
                int loc = r.loc; std::string name = r.title;
                mclick(tp, ImVec2(tp.x + tsz.x, tp.y + tsz.y), [loc, name]() { travel_to_loc(loc, name.c_str()); });
            }
            ImGui::Unindent(ind);
        }
        ImVec2 a(p0.x - lh * 0.15f, p0.y - lh * 0.1f), b(p0.x + w + lh * 0.15f, (is_open ? ImGui::GetCursorScreenPos().y : ty1) - lh * 0.05f);
        if (over(a, ImVec2(b.x, ty1))) dl->AddRectFilled(a, ImVec2(b.x, ty1), IM_COL32(232, 200, 130, 28), lh * 0.2f);
        std::string id = r.id;
        mclick(a, ImVec2(b.x, ty1), [id]() { if (open.count(id)) open.erase(id); else open.insert(id); });
    }
    if (q.size() > 10) ImGui::TextDisabled("... %d more (F12 > Journal)", (int)q.size() - 10);
}

/* in a town at night: the streets' dangers (mods/travel.c has the rolls: the night watch at each
   move, more on the main street and in a town on its guard; leaving a side street, cutthroats
   unless (the party's average Stealth + Streetwise) / 2 beats a roll of 100, s046 194b:02d6) */
static void night_streets(bool night)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (!night || game_mode() != GM_CARD || cur < 0 || cur >= (int)gamedata.cities.size() || config_get_int("original", 0)) return;
    int st = 0, sw = 0, n = 0;
    for (int m = 0; m < 5; m++) if (gm_member_status(m) == 1) { st += gm_member_skill(m, 15); sw += gm_member_skill(m, 16); n++; }  /* (party_avg_skill: the members on their feet) */
    if (!n) return;
    int t = (st / n + sw / n) / 2;
    t = t < 0 ? 0 : t > 99 ? 99 : t;
    int pct = config_get_int("travel.street_encounters", 100), slip = 100 - (100 - t) * pct / 100;
    ImGui::TextColored(ImVec4(0.92f, 0.70f, 0.42f, 1), "Night in the streets");
    if (item_hovered()) {
        char b[460], often[40]; int k = (slip + 5) / 10;
        if (k <= 0) snprintf(often, sizeof often, "hardly ever");
        else if (k >= 10) snprintf(often, sizeof often, "almost always");
        else snprintf(often, sizeof often, "about %d time%s in 10", k, k == 1 ? "" : "s");
        snprintf(b, sizeof b, "From 19:00 to 5:00 the town is dark. Each move in its streets may meet the night watch (more often on the main street, "
                 "and where the town is on its guard). Leaving a side street the party slips past cutthroats %s "
                 "(the party's Stealth and Streetwise)%s. An inn is the safe place to wait for morning.",
                 often, pct != 100 ? ", with street trouble made rarer in F12 > Settings > Helpers" : "");
        margin_tip(b);
    }
}

extern "C" void ui_margins(void)
{
    if (!ui_margins_on || game_mode() == GM_OTHER || !ui_font_overlay) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    ImGuiIO &io = ImGui::GetIO();
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float left = x0, right = io.DisplaySize.x - x1;
    if (left < 150 && right < 150) return;
    float h = io.DisplaySize.y;
    /* with the picture (the margins' width keeps lines from getting too short), and the setting */
    float k = config_get_int("display.text_size", 100) / 100.0f;
    float fs = std::max(std::min(std::min(left, right) / 9.5f * k, ui_text_px()), 14.0f);

    if (left >= 150 && !battlelog_margin(0, left, h, fs) && begin_panel("##margin_l", 0, left, h, fs)) {
        int hour = (s16)g_rw(G_CUR_HOUR), day = (s16)g_rw(G_CUR_DAY), mon = (s16)g_rw(G_CUR_MONTH), year = (s16)g_rw(G_CUR_YEAR);
        /* the date and hour, the purse, then the tasks: no labels where the thing speaks for itself */
        char date[48]; snprintf(date, sizeof date, "%d %s %d", day, MONTHS[(unsigned)mon % 12], year);
        heading(date);
        /* the game's own day: daylight from 5:00 to 18:59 (is_daylight, s029 1367:072a); then the towns' night streets */
        const bool night = hour < 5 || hour >= 19;
        {   /* the hour by the clock and by the bells, as the game's texts tell time (Prime, Vespers, Compline ...) */
            char bell[24]; gm_hour_name(hour, bell, sizeof bell);
            ImGui::TextDisabled("%d:00, %s", hour, bell);
            if (item_hovered()) {
                std::string t = std::string("The canonical hours, as the game's people tell the time: Matins 0-2, Lauds 3-5, Prime 6-8, Terce 9-11, "
                                            "Sexts 12-14, Nones 15-17, Vespers 18-20, Compline 21-23. ") +
                                (night ? "It is night (from 19:00 to 5:00): town gates may be shut, and the streets have the watch and cutthroats." : "It is day (from 5:00 to 19:00).");
                margin_tip(t.c_str());
            }
        }
        night_streets(night);
        int f = (s16)g_rw(G_MONEY), g = (s16)g_rw(G_MONEY + 2), p = (s16)g_rw(G_MONEY + 4), bn = (s16)g_rw(0x9072);
        ImGui::TextWrapped("%s", money_fgp(f, g, p).c_str());
        { long pf = (long)f * 240 + g * 12 + p; if (pf >= 12 && item_hovered()) margin_tip(money_total(pf).c_str()); }
        if (bn > 0) ImGui::TextDisabled("%s in bank notes", money_fgp(bn, 0, 0).c_str());
        /* the place and the tasks come from the location and event tables, which battles and their
           loot reuse: read while a card or the map is up, meanwhile the last reading stands */
        const int gm = game_mode();
        const bool tables = gm == GM_CARD || gm == GM_MAP;
        static std::string where; static bool where_read = false;
        if (tables || !where_read) { where = place_name(); where_read = tables; }
        if (!(right >= 150 && right_shows_place())) ImGui::TextWrapped("%s", where.c_str());   /* (else the right margin names it) */
        ImGui::Separator();
        tasks_section(tables);
        {   /* the hints, at the foot of the margin (always in the same place) */
            char h1[96]; snprintf(h1, sizeof h1, "Right click or F10: the menus (save, load, party%s).", menuview_is_on() && !config_get_int("original", 0) ? ", Darklands+" : "");
            const char *h2 = config_get_int("guide.seen", 0) ? nullptr : "New to Darklands? F12 > Guide.";
            float ww = ImGui::GetContentRegionAvail().x, sp = ImGui::GetStyle().ItemSpacing.y;
            float hh = ImGui::CalcTextSize(h1, nullptr, false, ww).y + (h2 ? sp + ImGui::CalcTextSize(h2, nullptr, false, ww).y : 0) + sp * 2;
            float foot = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - hh;
            if (foot > ImGui::GetCursorPosY() + sp * 2) ImGui::SetCursorPosY(foot);
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            ImGui::TextWrapped("%s", h1);
            if (h2) ImGui::TextWrapped("%s", h2);
            ImGui::PopStyleColor();
        }
        end_panel();
    }

    if (right >= 150 && battle_showing() && !config_get_int("original", 0) && begin_panel("##margin_rb", x1, right, h, fs)) {
        battle_panel(right, fs);
        end_panel();
    }
    else if (right >= 150 && begin_panel("##margin_r", x1, right, h, fs)) {
        right_panel(size);
        end_panel();
    }
}
