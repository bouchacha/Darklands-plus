// margins.cpp - on a wide screen the 4:3 picture leaves dark margins; they can show what the
// game otherwise keeps behind menus: the date and hour, where the party is, the purse, the
// tasks the party has taken on (with their deadlines), and on the other side the town the party
// is in, the blessings and potions working on the party, and who carries too much.
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "padglyphs.h"
#include "imgui.h"
#include "gamedata.h"
#include "teachersui.h"
#include "ordericons.h"
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
#include "margins_parts.h"
extern "C" int encounters_card_active(void);
extern "C" int encounters_builtin(int i);
extern "C" void ui_open_goto(void);
extern "C" int citynav_available(void);
extern "C" int battlelog_margin(float x, float w, float h, float fs);
extern "C" int battle_showing(void);                  /* ui/battlelog.cpp: a battle is on (not the marching order) */

int ui_margins_on = 1;

extern "C" int levelmap_available(void);

extern "C" int politics_unrest_at(int loc);           /* (mods/politics.c) */
extern "C" int war_state(int loc, int *left);          /* (mods/war.c) */
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
        return "Traveling, near " + loc_display_name(best);
    }
    return loc_display_name(cur);
}

std::string member_name(int m)
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
   reach ImGui's windows nor the game), so ImGui never counts their items as hovered. Not while
   another window lies over it there (Go to, the world map, F12's window): its tooltip would show
   over that window */
bool item_hovered(void)
{
    return ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), false) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}
/* a tooltip from the margin: in the tooltips' own ink (the margin's text is light, for its black) */
void margin_tip(const char *text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(44, 26, 10, 255));
    ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20); ImGui::TextUnformatted(text); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
    ImGui::PopStyleColor();
}
void heading(const char *t)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(222, 178, 104, 255));
    ImGui::TextUnformatted(t);
    ImGui::PopStyleColor();
}
/* a name on a line of its own, in the margin's ink or dim, wrapped at the margin's edge by words (a
   long formula's name ran past it), the lines after the first indented; a dim tail (a price) after
   its last word, or under it when there is no room. One group: under the pointer as one. */
void name_line(const std::string &t, bool dim, const char *tail)
{
    float avail = ImGui::GetContentRegionAvail().x;
    float indent = ImGui::GetFontSize() * 0.8f;
    std::vector<std::string> lines; std::string cur;
    size_t i = 0;
    while (i < t.size()) {                                  /* greedy, a word at a time */
        size_t j = t.find(' ', i); if (j == std::string::npos) j = t.size();
        std::string w = t.substr(i, j - i), next = cur.empty() ? w : cur + " " + w;
        float room = avail - (lines.empty() ? 0 : indent);
        if (!cur.empty() && ImGui::CalcTextSize(next.c_str()).x > room) { lines.push_back(cur); cur = w; }
        else cur = next;
        i = j + 1;
    }
    if (!cur.empty()) lines.push_back(cur);
    ImGui::BeginGroup();
    if (dim) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    for (size_t k = 0; k < lines.size(); k++) {
        if (k) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        ImGui::TextUnformatted(lines[k].c_str());
    }
    if (dim) ImGui::PopStyleColor();
    if (tail && !lines.empty()) {
        float used = ImGui::CalcTextSize(lines.back().c_str()).x + (lines.size() > 1 ? indent : 0);
        if (used + ImGui::GetStyle().ItemSpacing.x + ImGui::CalcTextSize(tail).x <= avail) ImGui::SameLine();
        else ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        ImGui::TextDisabled("%s", tail);
    }
    ImGui::EndGroup();
}

/* in a battle: everybody's condition at a glance - the party with endurance and strength,
 * the enemies the party has seen (in the battle's hot spots) with how hurt they are */
std::string battlelog_name(int i);
bool battlelog_met(int i);
void bar(float frac, ImU32 col, float w, float hgt)
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
 * the world map (the place panel). Then the notable shops: those whose wares come out at 30q or better (the
 * standard is 25q), a picture and the quality each. `tables`: the location table may be read;
 * `width`: where the rows wrap (0: the room left); `words`: each picture's name after it (where
 * no tooltip can show: the world map's tooltip on a town). */
static const char *TOWN_OFFER_TIP[6] = {
    "University: new saints and alchemical formulae to learn, rare alchemical materials, a better Philosopher's Stone, teachers to hire (it expects Latin and a sharp mind).",
    "Cathedral: donations for Divine Favor (and maybe Virtue), a relic from the canon, the patron saint's story, sanctuary.",
    "An alchemist: formulae to buy or trade, a better Philosopher's Stone, lessons in Alchemy.",
    "A physician: wounds treated for a fee, lessons in Healing, alchemical components bought and sold.",
    "Docks: boats to other towns, for a fee (each boat leaves on its own day).",
    "A bank (Fugger or Medici): heavy coins changed into a bank note that weighs nothing, and back, for a fee; quests for the bold." };
static const struct { int shop, icon; const char *wares; } NOTABLE_SHOPS[] = {
    { 2, TI_SWORD, "weapons" }, { 3, TI_ARMOUR, "armor and shields" }, { 5, TI_BOW, "bows, crossbows and guns" },
    { 0, TI_SMITH, "weapons" }, { 6, TI_TOOLS, "hammers, maces, flails and polearms (pikes, halberds, spears, staves)" },
    { 8, TI_CLOTH, "clothing and soft armor (padded, leather, studded leather, cuir bouilli)" } };   /* (the clothmaker: only with the cardopts fix, which opens its shopfronts by day) */
enum { NOTABLE_Q = 30 };
static const char *TOWN_OFFER_WORD[6] = { "University", "Cathedral", "Alchemist", "Physician", "Docks", "Bank" };
/* what was drawn into dl since vertex v0, faded to alpha k (a known formula, a teacher nobody needs) */
static void dim_since(ImDrawList *dl, int v0, float k)
{
    for (int i = v0; i < dl->VtxBuffer.Size; i++) {
        ImU32 &c = dl->VtxBuffer[i].col;
        unsigned al = (c >> IM_COL32_A_SHIFT) & 255;
        c = (c & ~IM_COL32_A_MASK) | ((ImU32)(al * k) << IM_COL32_A_SHIFT);
    }
}
/* the teachers to be had in town loc (mods/teachers.c), and whether anyone in the party could use
   them: a fighting skill only if some member's best weapon of that kind (hand to hand: Edged,
   Impact, Flails, Polearms; at range: Thrown, Bows, Missile Devices) is below what the teacher
   teaches to; a skill the party needs once (Alchemy, Healing, Artifice, Latin, Read & Write, Speak
   Common, Streetwise, Woodwise) only if the party's best is below it; a skill each member needs
   (Religion, Virtue, Stealth, Riding) if anyone's is */
extern "C" void ui_text_utf8(const char *game, char *out, int n);
extern "C" u32 plat_ticks_ms(void);
extern "C" int ns_showing(void);
static bool margin_town_row;                               /* (the margin's own row of the town's offers: the teachers too) */
static int skill_of(int m, int k) { return (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)); }
static bool teach_useful(int k, int to, std::string *who)
{
    int best = -1; std::string w;
    for (int m = 0; m < 5; m++) {
        if (!gm_member_present(m)) continue;
        int v;
        if (k <= 3) { v = 0; for (int j = 0; j <= 3; j++) v = std::max(v, skill_of(m, j)); }
        else if (k <= 6) { v = 0; for (int j = 4; j <= 6; j++) v = std::max(v, skill_of(m, j)); }
        else v = skill_of(m, k);
        bool each = k <= 6 || k == 8 || k == 9 || k == 15 || k == 17;
        if (each) { if (v < to) { char nm[24]; gm_member_name(m, nm, sizeof nm); char nu[48]; ui_text_utf8(nm, nu, sizeof nu); if (!w.empty()) w += ", "; w += nu; } }
        else best = std::max(best, v);
    }
    bool each = k <= 6 || k == 8 || k == 9 || k == 15 || k == 17;
    if (each) { if (who) *who = w; return !w.empty(); }
    if (who) *who = best >= 0 && best < to ? "the party (best " + std::to_string(best) + ")" : "";
    return best >= 0 && best < to;
}
static void teachers_chip(int loc, bool &first, float isz, float gap, float x0, float w)
{
    if (!ui_teachers_on()) return;
    TchSource ts[12];
    int n = ui_town_teachers(loc, ts, 12);
    if (!n) return;
    int useful = 0;
    struct Row { std::string head, who, closed; bool any, off; };
    std::vector<Row> rows;
    for (int i = 0; i < n; i++) {
        const TchSource &t = ts[i];
        Row r; r.any = false; r.off = t.off != 0;
        for (int j = 0; j < t.n; j++) { std::string who; if (teach_useful(t.skill[j], t.strength, &who)) { r.any = true; if (!r.who.empty()) r.who += "; "; r.who += std::string(t.n > 1 ? std::string(gm_skill_name(t.skill[j])) + ": " : "") + who; } }
        if (r.any && !r.off) useful++;
        char line[160]; snprintf(line, sizeof line, "%s: %s to %d", ui_teacher_short(t.src), ui_teacher_skills(t).c_str(), t.strength);
        r.head = line;
        if (r.off) { r.closed = t.now; size_t p2 = r.closed.find(" ("); if (p2 != std::string::npos) r.closed = r.closed.substr(0, p2) + "."; }
        rows.push_back(r);
    }
    char num[8]; snprintf(num, sizeof num, "%d", n);
    float lh = ImGui::GetTextLineHeight(), tw = ImGui::CalcTextSize(num).x + lh * 0.25f, cw = isz + tw;
    if (!first) { ImGui::SameLine(0, gap); if (ImGui::GetCursorScreenPos().x + cw > x0 + w) ImGui::NewLine(); }
    first = false;
    ImVec2 a = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(cw, isz));
    ImDrawList *dl = ImGui::GetWindowDrawList();
    int v0 = dl->VtxBuffer.Size;
    ImVec2 c(a.x + isz * 0.5f, a.y + isz * 0.5f);              /* on a parchment roundel, as the town's other icons */
    dl->AddCircleFilled(c, isz * 0.48f, IM_COL32(240, 228, 198, 255), 24);
    dl->AddCircle(c, isz * 0.48f, IM_COL32(92, 60, 30, 255), 24, fmaxf(1.0f, isz * 0.05f));
    ui_draw_teach_mark(dl, ImVec2(c.x, c.y + isz * 0.02f), isz * 0.72f, IM_COL32(70, 44, 20, 255), IM_COL32(252, 246, 228, 255));
    dl->AddText(ImVec2(a.x + isz + lh * 0.2f, a.y + (isz - lh) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), num);
    if (!useful) dim_since(dl, v0, 0.38f);                   /* nobody would learn anything now: dim, as a known saint */
    if (item_hovered()) {                                    /* a row a teacher: who would learn, or why not now */
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(44, 26, 10, 255));
        ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24);
        ImGui::TextUnformatted("Teachers here: a lesson a day at the inn, for a fee");
        ImGui::Separator();
        for (const Row &r : rows) {
            if (r.any && !r.off) ImGui::TextUnformatted(r.head.c_str()); else ImGui::TextDisabled("%s", r.head.c_str());
            ImGui::Indent();
            if (r.off) ImGui::TextColored(ImVec4(0.6f, 0.2f, 0.1f, 1), "Not now: %s", r.closed.c_str());
            if (r.any) ImGui::TextColored(ImVec4(0.15f, 0.42f, 0.15f, 1), "Would teach %s", r.who.c_str());
            else ImGui::TextDisabled("Nobody in the party would learn anything");
            ImGui::Unindent();
        }
        ImGui::PopTextWrapPos(); ImGui::EndTooltip();
        ImGui::PopStyleColor();
    }
}
void ui_town_offers(int loc, bool tables, float width, bool words)
{
    static int seen_loc = -1; static bool has[6];
    if (tables || seen_loc != loc) { for (int k = 0; k < 6; k++) has[k] = citynav_town_offers(loc, k); seen_loc = loc; }
    const City &c = gamedata.cities[loc];
    float lh = ImGui::GetTextLineHeight(), gap = lh * 0.3f;
    float x0 = ImGui::GetCursorScreenPos().x, w = width > 0 ? width : ImGui::GetContentRegionAvail().x;
    int nf = 0; for (int k = 0; k < 6; k++) nf += has[k];
    if (margin_town_row && ui_teachers_on()) nf++;                /* (the lectern too) */
    float isz = lh * 1.35f;                                 /* (smaller, down to a line's height, to keep the row on one line) */
    float wn = margin_town_row ? w - lh * 1.0f : w;              /* (the lectern's number beside it) */
    if (!words && nf > 1 && nf * isz + (nf - 1) * gap > wn) isz = fmaxf(lh * 1.0f, (wn - (nf - 1) * gap) / nf);
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
    if (!words && margin_town_row) teachers_chip(loc, first, isz, gap, x0, w);   /* (the margin: the lectern and how many) */
    bool head = false;
    for (const auto &s : NOTABLE_SHOPS) {
        int q = c.shopq[s.shop] > 0 ? shop_wares_q(c.shopq[s.shop]) : 0;
        if (q < NOTABLE_Q || (s.shop == 8 && !fix_is_on("cardopts"))) continue;
        if (!head) { ImGui::Spacing(); if (words) ImGui::TextDisabled("Notable shops"); else heading("Notable shops"); head = true; first = true; }
        char t[48], tip[160]; snprintf(t, sizeof t, words ? "%s %dq" : "%.0s%dq", SHOP_NAMES[s.shop], q);
        snprintf(tip, sizeof tip, "%s: %s of %dq (most shops sell about 25q).", SHOP_NAMES[s.shop], s.wares, q);
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
        name_line(t, known);
        if (item_hovered()) {
            std::string tip = gamedata.saints[s].name + (known ? " (someone in the party knows this saint): " : " (nobody in the party knows this saint): ") + saint_info_does(s) + (*saint_info_note(s) ? "\n" : "") + saint_info_note(s);   /* (what its help really does: mods/saintinfo.h) */
            margin_tip(tip.c_str());
        }
    }
}
extern "C" const char *journal_town_fact(int loc, const char *key);   /* ui/journal.cpp */
/* the formulae a town's alchemist sells (ui/libraries.h): a row of the potions' icons, each with its
   grade; those someone in the party knows dim (as the saints taught). The formula, its price and what it does under the
   pointer */
bool ui_draw_potion(ImDrawList *dl, ImVec2 c, float s, int id, int tier);   /* ui/alchemyicons.cpp */
std::string ns_item_encounters(int id, ImU32 *col);   /* ui/nsdraw.cpp */
static void formulae_line(int town)
{
    int fo[4], pr[4];
    if (town < 0 || town >= (int)gamedata.cities.size() || !alchemist_formulae(town, fo, pr)) return;
    ImGui::Spacing(); heading("Formulae sold here");
    float gap = ImGui::GetStyle().ItemSpacing.x, s = fminf(ImGui::GetTextLineHeight() * 1.35f, (ImGui::GetContentRegionAvail().x - 3 * gap) / 4 - 1);   /* (as the town's icons above) */
    bool first = true;
    for (int k = 0; k < 4; k++) {
        int f = fo[k];
        if (f < 0 || f >= (int)gamedata.formula_names.size()) continue;
        bool known = party_knows_formula(f);
        if (!first) ImGui::SameLine(0, gap);
        first = false;
        ImVec2 a = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(s, s));
        ImDrawList *dl = ImGui::GetWindowDrawList();
        int v0 = dl->VtxBuffer.Size;
        ui_draw_potion(dl, ImVec2(a.x + s * 0.5f, a.y + s * 0.5f), s, 95 + f / 3, f % 3 + 1);
        if (known) dim_since(dl, v0, 0.38f);                 /* known to the party: dim, as the saints above (the whole icon) */
        if (item_hovered()) {
            const std::string &t = gamedata.formula_names[f];
            const ItemDef *pd = 95 + f / 3 < (int)gamedata.items.size() ? &gamedata.items[95 + f / 3] : nullptr;
            char head[200]; snprintf(head, sizeof head, "%s: %s, grade %d of 3, %s.\n%s\n\n", t.c_str(), pd ? pd->name.c_str() : "?", f % 3 + 1, money_str(pr[k] * 240L).c_str(),
                                     known ? "Someone in the party knows this formula." : "Nobody in the party knows this formula.");
            std::string tip = head + (f < (int)gamedata.formulas.size() ? gamedata.formulas[f].text : std::string());
            { std::string enc = ns_item_encounters(95 + f / 3, nullptr); if (!enc.empty()) tip += "\n\n" + enc; }
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
    /* one line, the list under the pointer: the margin is for what is at hand */
    ImGui::Spacing();
    int open = 0; for (int k = 0; k < n; k++) if (!ts[k].off) open++;
    if (!hired.empty()) ImGui::TextUnformatted("Teachers: some at the inn now");
    else if (open) ImGui::Text("Teachers: %d here", open);
    else ImGui::TextDisabled("Teachers: none to be had now");
    if (item_hovered()) {
        std::string tip;
        if (!hired.empty()) tip += "At the inn now: " + hired + "\nHired teachers give lessons at the inn: set a member to \"Train or study\" there, a lesson a day, for a fee.\n\n";
        for (int k = 0; k < n; k++) {
            const TchSource &t = ts[k];
            char line[200]; snprintf(line, sizeof line, fix_is_on("teachcap") ? "%s: %s (to %d)%s\n" : "%s: %s (skill %d)%s\n", ui_teacher_short(t.src), ui_teacher_skills(t).c_str(), t.strength, t.off ? " - not now" : "");
            tip += line;
        }
        margin_tip(tip.c_str());
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
/* other things in the margins that take a click (a quest opened or closed, the order of the
   tasks): their places as last drawn */
struct MClick { ImVec2 a, b; std::function<void()> f; };
static std::vector<MClick> mclicks; static int mclicks_frame = -100;
void mclick(ImVec2 a, ImVec2 b, std::function<void()> f)
{
    if (mclicks_frame != ImGui::GetFrameCount()) { mclicks.clear(); mclicks_frame = ImGui::GetFrameCount(); }
    mclicks.push_back({ a, b, f });
}
extern "C" int margins_click(int x, int y)
{
    if (ImGui::GetFrameCount() - mclicks_frame <= 2)
        for (const MClick &c : mclicks)
            if (x >= c.a.x && x < c.b.x && y >= c.a.y && y < c.b.y) { c.f(); return 1; }
    return margins_order_click(x, y);                  /* (the order buttons: ui/margins_battle.cpp) */
}
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

#include "repwatch.h"
static bool goto_hint, camp_hint;
extern "C" void plat_press_key(int scancode);
/* a small campfire: two crossed logs, the flames over them (the map's "C: camp here") */
static void campfire_icon(ImDrawList *dl, ImVec2 c, float s)
{
    float u = s / 20.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    ImU32 wood = IM_COL32(120, 74, 36, 255), wood_dk = IM_COL32(70, 40, 18, 255);
    dl->AddLine(P(-8, 8), P(8, 3), wood_dk, 3.6f * u); dl->AddLine(P(-8, 3), P(8, 8), wood_dk, 3.6f * u);
    dl->AddLine(P(-8, 8), P(8, 3), wood, 2.4f * u); dl->AddLine(P(-8, 3), P(8, 8), wood, 2.4f * u);
    ImVec2 f1[3] = { P(-6, 4), P(0, -10), P(6, 4) }, f2[3] = { P(-3.5f, 4), P(0.5f, -5), P(3.5f, 4) }, f3[3] = { P(-1.6f, 4), P(0, -1), P(1.6f, 4) };
    dl->AddTriangleFilled(f1[0], f1[1], f1[2], IM_COL32(200, 70, 20, 255));
    dl->AddTriangleFilled(f2[0], f2[1], f2[2], IM_COL32(240, 150, 40, 255));
    dl->AddTriangleFilled(f3[0], f3[1], f3[2], IM_COL32(255, 230, 140, 255));
}
static void right_panel(int size)
{
    goto_hint = false; camp_hint = false;
    int cur = (s16)g_rw(G_CUR_LOC);
    bool any = false, near = false;
    if (cur < 0 && game_mode() == GM_MAP) { cur = ui_nearest_town(); near = true; camp_hint = true; }
    else if (cur >= 0 && encounters_card_active() && !encounters_builtin(encounters_card_active() - 1)) near = true;    /* (a new encounter on the road out of town; not the restored hospice, a place in town) */
    if (cur >= 0 && cur < (int)gamedata.cities.size()) {
        const City &c = gamedata.cities[cur];
        heading(((near ? "Near " : "") + c.name).c_str());
        goto_hint = !near && citynav_available() && config_get_int("assist.city_goto", 1);   /* (at the foot, as the left margin's hints) */
        /* the location table is read while a card or the map is up; battles and their loot
           reuse that memory, so meanwhile the last value read stands */
        static std::map<int, int> rep_seen; static int rep_game = -1;
        if (rep_game != (s16)g_rw(G_PARTY_SEED)) { rep_seen.clear(); rep_game = (s16)g_rw(G_PARTY_SEED); }
        int gm = game_mode();
        u32 lr = g_location(cur);
        if (lr && (gm == GM_CARD || gm == GM_MAP)) rep_seen[cur] = (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12));
        int fame = (s16)g_rw(GV_party_fame);
        auto rp = rep_seen.find(cur);
        if (rp != rep_seen.end()) {                                   /* coloured by how the town sees the party */
            int r = rp->second;
            ImVec4 col = r <= -30 ? ImVec4(0.95f, 0.22f, 0.16f, 1) : r < -10 ? ImVec4(0.95f, 0.45f, 0.35f, 1) : r < 0 ? ImVec4(0.95f, 0.68f, 0.55f, 1)
                       : r >= 30 ? ImVec4(0.55f, 0.88f, 0.50f, 1) : ImGui::GetStyleColorVec4(ImGuiCol_Text);
            ImGui::TextColored(col, "Reputation %d", r);
            bool rep_hov = item_hovered();
            const RepChange *rc = repwatch_last(cur);
            u32 rage = rc ? plat_ticks_ms() - rc->t : 999999;
            if (rc && rage < 12000) {                                 /* just changed: the change, beside it, for a while */
                ImGui::SameLine(0, ImGui::GetFontSize() * 0.3f);
                float pulse = rage < 3000 ? 0.65f + 0.35f * sinf(rage * 0.012f) : rage > 10000 ? (12000 - rage) / 2000.0f : 1.0f;
                ImVec4 dc = rc->delta < 0 ? ImVec4(1.0f, 0.38f, 0.28f, pulse) : ImVec4(0.55f, 0.95f, 0.45f, pulse);
                ImGui::TextColored(dc, "%+d", rc->delta);
                rep_hov = rep_hov || item_hovered();
            }
            if (rep_hov) {
                std::vector<RepChange> L; repwatch_list(cur, L);
                if (!L.empty()) {
                    std::string t = "Changes seen since this game was loaded:";
                    for (int k = (int)L.size() - 1, n = 0; k >= 0 && n < 6; k--, n++) {
                        const RepChange &c = L[k]; char b[64]; snprintf(b, sizeof b, "\n%+d on %d.%d.%d", c.delta, c.day, c.month + 1, c.year); t += b;
                        t += c.choice.empty() ? " (" + std::string(c.card[0] == '$' ? "a card" : "the overland view") + ")" : ", after choosing \"" + c.choice + "\"";
                    }
                    margin_tip(t.c_str());
                }
            }
            if (rep_hov && !repwatch_last(cur)) margin_tip(r <= -30 ? "The town hates the party: guards, shopkeepers and its people will turn on you." : r < -10 ? "The town distrusts the party: expect cold welcomes and worse." : r < 0 ? "The town is wary of the party." : r >= 30 ? "The party is well thought of here." : "The town's opinion of the party (rises with good deeds here, falls with crimes).");
            ImGui::SameLine(); ImGui::TextDisabled(" fame %d", fame);
        }
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
                                                                                       : "The watch is after the party: guards who recognize the party try to arrest it, and talking or bribing your way out is harder.");
            ImGui::Unindent(is + lh * 0.3f);
        }
        {   /* a season of city politics (mods/politics.c, experimental): the town in turmoil, until the party takes a side
               (then it is a task) */
            if (politics_unrest_at(cur) == 2) {
                float lh = ImGui::GetTextLineHeight(), is = lh * 1.15f;
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(p0.x + is * 0.5f, p0.y + lh * 0.5f), is, QC_POLITICS);
                ImGui::Indent(is + lh * 0.3f);
                ImGui::TextColored(ImVec4(0.95f, 0.78f, 0.45f, 1), "In turmoil");
                if (item_hovered()) margin_tip("Rebels and the government's men are hiring in this town. The news (the posted notices and gossip) "
                                               "and its rumors of special jobs lead to them; so does the guild where the agitators meet.");
                ImGui::Unindent(is + lh * 0.3f);
            }
        }
        {   /* a city preparing for war or at war (mods/war.c, experimental) */
            int left = 0, w = war_state(cur, &left);
            if (w && !politics_unrest_at(cur)) {
                float lh = ImGui::GetTextLineHeight(), is = lh * 1.15f;
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(p0.x + is * 0.5f, p0.y + lh * 0.5f), is, QC_WAR);
                ImGui::Indent(is + lh * 0.3f);
                ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.43f, 1), "%s", w == 1 ? "Preparing for war" : "At war");
                if (item_hovered()) margin_tip(w == 1 ? "Prices are a quarter higher, the gate turns travelers away and patrols watch the walls; "
                                                        "Soldiers' Road and its market are open, and few boats take passengers."
                                                      : "Prices are 50% higher, food is short and the docks are barricaded; "
                                                        "the gate turns travelers away and patrols watch the walls. Soldiers' Road and its market are open.");
                ImGui::Unindent(is + lh * 0.3f);
            }
        }
        margin_town_row = true; ui_town_offers(cur, gm == GM_CARD || gm == GM_MAP); margin_town_row = false;
        {   /* what the party learned of its physician and alchemist (ui/journal.cpp) */
            const char *ph = journal_town_fact(cur, "physician"), *al = journal_town_fact(cur, "alchemist");
            if (ph) { ImGui::TextDisabled("Physician:"); ImGui::SameLine(); ImGui::TextWrapped("%s", ph); }
            if (al) { ImGui::TextDisabled("Alchemist:"); ImGui::SameLine(); ImGui::TextWrapped("%s", al); }
        }
        library_line(cur);
        formulae_line(cur);
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
    if (camp_hint) {                                                /* on the map: the original's C, to camp in the wild */
        float sp = ImGui::GetStyle().ItemSpacing.y, lh = ImGui::GetTextLineHeight(), hh = lh * 1.4f + sp * 2;
        float foot = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - hh;
        if (foot > ImGui::GetCursorPosY() + sp * 2) ImGui::SetCursorPosY(foot);
        ImGui::Separator();
        ImVec2 p = ImGui::GetCursorScreenPos();
        campfire_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + lh * 0.7f, p.y + lh * 0.7f), lh * 1.4f);
        ImGui::SetCursorScreenPos(ImVec2(p.x + lh * 1.6f, p.y + lh * 0.2f));
        ImGui::TextDisabled("%s", ui_pad_prompts() ? "Camp here" : "C: camp here");
        if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); margin_tip("Click (or press C) to make camp in the wild: rest, heal, brew potions, pray. Someone should guard the camp."); }
        if (ImGui::IsItemClicked()) plat_press_key(SDL_SCANCODE_C);
    }
    if (goto_hint) {                                                /* the foot, as the left margin's hints */
        const char *h = ui_pad_prompts() ? "Go to a place here" : "G: go to a place here";
        float sp = ImGui::GetStyle().ItemSpacing.y, hh = ImGui::GetTextLineHeight() + sp * 2;
        float foot = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - hh;
        if (foot > ImGui::GetCursorPosY() + sp * 2) ImGui::SetCursorPosY(foot);
        ImGui::Separator();
        ImGui::TextDisabled("%s", h);
        if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); margin_tip("Click (or press G) to open Go to: a list of the places in this town; pick one and the party walks there."); }
        if (ImGui::IsItemClicked()) ui_open_goto();
    }
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
    if (ImGui::GetFrameCount() - qf > 30 && (tables || !q_read)) { quest_rows(q, sort); quest_merge(q); qf = ImGui::GetFrameCount(); q_read = tables; }
    ImVec2 mp = ImGui::GetIO().MousePos;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float w = ImGui::GetContentRegionAvail().x, lh = ImGui::GetTextLineHeight();
    auto over = [&](ImVec2 a, ImVec2 b) { return mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y; };
    {   /* the heading, and the order at its right */
        ImVec2 h0 = ImGui::GetCursorScreenPos();
        heading("Quests");
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
            ImGui::PopStyleColor();
            if (r.dist >= 0) { ImGui::Dummy(ImVec2(0, 0)); quest_way_inline(r); }
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
    if (q.size() > 10) ImGui::TextDisabled("... %d more (M: the world map)", (int)q.size() - 10);
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
        snprintf(b, sizeof b, "From 19:00 to 5:00 the town is dark. Each move through its streets may run into the night watch (more often on the main street, "
                 "and where the town is on its guard). When leaving a side street, the party slips past cutthroats %s "
                 "(depending on the party's Stealth and Streetwise)%s. An inn is the safe place to wait for morning.",
                 often, pct != 100 ? ", as set in F12 > Mods (Night encounters in the streets)" : "");
        margin_tip(b);
    }
}

void ui_draw_daynight(ImDrawList *dl, ImVec2 c, float rd, float hour, bool rim);   /* ui/hourglass.cpp */
extern "C" void ui_margins(void)
{
    if (game_mode() == GM_OTHER || !ui_font_overlay) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    ImGuiIO &io = ImGui::GetIO();
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float left = x0, right = io.DisplaySize.x - x1;
    if (!ui_margins_on || right < 150) order_bar(x0 + (x1 - x0) * 60 / 320, x1, y1);   /* (the field: right of the party's panel) */
    else ui_orderbar_top = 0;
    if (!ui_margins_on) return;
    if (left < 150 && right < 150) return;
    float h = io.DisplaySize.y;
    /* with the picture (the margins' width keeps lines from getting too short), and the setting */
    float k = config_get_int("display.text_size", 100) / 100.0f;
    float fs = std::max(std::min(std::min(left, right) / 9.5f * k, ui_text_px()), 14.0f);

    if (left >= 150 && !battlelog_margin(0, left, h, fs) && begin_panel("##margin_l", 0, left, h, fs)) {
        int hour = (s16)g_rw(G_CUR_HOUR), day = (s16)g_rw(G_CUR_DAY), mon = (s16)g_rw(G_CUR_MONTH), year = (s16)g_rw(G_CUR_YEAR);
        /* the date and hour, the purse, then the tasks: no labels where the thing speaks for itself */
        char date[48]; snprintf(date, sizeof date, "%d %s %d", day, gm_month_name((unsigned)mon % 12), year);
        heading(date);
        /* the game's own day: daylight from 5:00 to 18:59 (is_daylight, s029 1367:072a); then the towns' night streets */
        const bool night = hour < 5 || hour >= 19;
        {   /* the hour by the clock and by the bells, as the game's texts tell time (Prime, Vespers, Compline ...) */
            char bell[24]; gm_hour_name(hour, bell, sizeof bell);
            {   /* the armillary's disc of day and night before it (ui/hourglass.cpp) */
                float lh = ImGui::GetTextLineHeight(), r = lh * 0.62f;
                ImVec2 p = ImGui::GetCursorScreenPos();
                ui_draw_daynight(ImGui::GetWindowDrawList(), ImVec2(p.x + r + 1, p.y + lh * 0.5f), r, (float)hour, true);
                ImGui::Dummy(ImVec2(2 * r + 2, lh)); ImGui::SameLine(0, lh * 0.45f);
            }
            ImGui::TextDisabled("%d:00, %s", hour, bell);
            if (item_hovered()) {
                std::string t = std::string("The canonical hours, as people in the game tell the time: Matins 0-2, Lauds 3-5, Prime 6-8, Terce 9-11, "
                                            "Sexts 12-14, Nones 15-17, Vespers 18-20, Compline 21-23. ") +
                                (night ? "It is night (from 19:00 to 5:00): town gates may be shut, and the streets have the watch and cutthroats." : "It is day (from 5:00 to 19:00).");
                margin_tip(t.c_str());
            }
        }
        night_streets(night);
        int f = (s16)g_rw(G_MONEY), g = (s16)g_rw(G_MONEY + 2), p = (s16)g_rw(G_MONEY + 4), bn = (s16)g_rw(GV_party_notes);
        ImGui::TextWrapped("%s", money_fgp(f, g, p).c_str());
        {   /* money come in on a card (a reward, a find; not a shop's sale): "+25 florins" rises beside it and fades */
            static long last_pf = -1; static long gain; static u32 gain_t; static int last_seed;
            long pf = (long)f * 240 + g * 12 + p;
            int seed = (s16)g_rw(G_PARTY_SEED);
            u32 now = plat_ticks_ms();
            if (last_pf >= 0 && seed == last_seed && pf > last_pf && game_mode() == GM_CARD && !ns_showing()) { gain = (now - gain_t < 4500 ? gain : 0) + (pf - last_pf); gain_t = now; }
            last_pf = pf; last_seed = seed;
            if (gain > 0 && now - gain_t < 4500) {
                float t = (now - gain_t) / 4500.0f, al = t < 0.75f ? 1.0f : (1.0f - t) / 0.25f;
                ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
                std::string s2 = "+" + money_fgp((int)(gain / 240), (int)(gain / 12 % 20), (int)(gain % 12));
                float fs = ImGui::GetFontSize() * (1.15f + 0.25f * (1.0f - fminf(1.0f, t * 4)));
                ImVec2 ts = ImGui::GetFont()->CalcTextSizeA(fs, FLT_MAX, 0, s2.c_str());
                ImVec2 pos(fminf(b.x + 6, ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - ts.x - 4), a.y - t * ImGui::GetFontSize() * 0.4f);
                ImDrawList *fl = ImGui::GetForegroundDrawList();
                fl->AddRectFilled(ImVec2(pos.x - 4, pos.y - 1), ImVec2(pos.x + ts.x + 4, pos.y + ts.y + 1), IM_COL32(30, 20, 8, (int)(200 * al)), 4);
                fl->AddText(ImGui::GetFont(), fs, pos, IM_COL32(255, 214, 80, (int)(255 * al)), s2.c_str());
            }
        }
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
            const bool pad = ui_pad_prompts();             /* (a controller in use: its buttons, padglyphs.h) */
            char h1[96]; snprintf(h1, sizeof h1, pad ? "{B} the menus (save, load, party%s)." : "Right-click or F10: the menus (save, load, party%s).", menuview_is_on() && !config_get_int("original", 0) ? ", Darklands+" : "");
            const char *h2 = config_get_int("guide.seen", 0) ? nullptr : pad ? "New to Darklands? {START} > Guide." : "New to Darklands? F12 > New Player Guide.";
            const char *h3 = pad ? nullptr : "F11: the keys and clicks of this screen.";
            float ww = ImGui::GetContentRegionAvail().x, sp = ImGui::GetStyle().ItemSpacing.y;
            float hh = ImGui::CalcTextSize(h1, nullptr, false, ww).y + (h2 ? sp + ImGui::CalcTextSize(h2, nullptr, false, ww).y : 0)
                     + (h3 ? sp + ImGui::CalcTextSize(h3, nullptr, false, ww).y : 0) + sp * 2;
            float foot = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - hh;
            if (foot > ImGui::GetCursorPosY() + sp * 2) ImGui::SetCursorPosY(foot);
            ImGui::Separator();
            if (pad) { ui_pad_text(h1, true); if (h2) ui_pad_text(h2, true); }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                ImGui::TextWrapped("%s", h1);
                if (h3) ImGui::TextWrapped("%s", h3);
                if (h2) ImGui::TextWrapped("%s", h2);
                ImGui::PopStyleColor();
            }
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
