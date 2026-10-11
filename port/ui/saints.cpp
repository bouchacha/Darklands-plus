// saints.cpp - the saints (F12 > Saints): the blessings and potions in effect and how long they
// last, every saint's description from DARKLAND.SNT, who in the party knows them, and a finder by what they help with ("Strength", "fire", "heal",
// "travel on water" ...), like the saint finder of the Darklands Companion tool. Knowledge:
// saints_known, far 2e38:082c, 0x14 bytes per member, a bit per saint in DARKLAND.LST order,
// highest bit first.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
}

static bool knows(int m, int s)
{
    return (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7))) != 0;
}

static const char *ATTRS[7] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma", "Divine Favor" };
static const char *SKILLS[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows",
    "Missile Devices", "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing",
    "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };

/* hours since year 0 (the game's Julian calendar: day and month count from 0) */
static long abs_hours(int y, int mo, int d, int h)
{
    static const int DAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    long days = (long)y * 365 + y / 4;
    for (int i = 0; i < mo && i < 12; i++) days += DAYS[i] + (i == 1 && y % 4 == 0);
    return (days + d) * 24 + h;
}

/* the running temporary effects: events of kind 0x48 (attribute), 0x49 (skill), 0x4a (weapon
   potion); +0 what, +0x1a member, +0x1e amount, +0x12 end (hour, day, month, year) */
static void effects(int size)
{
    struct Eff { int m; std::string what; long left; };
    std::vector<Eff> v;
    long now = abs_hours((s16)g_rw(G_CUR_YEAR), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR));
    for (int i = 0; i < 300; i++) {
        u16 o = rw(G_FAR2E38, (u16)(0x1b2c + 4 * i)), sg = rw(G_FAR2E38, (u16)(0x1b2c + 4 * i + 2));
        if (!sg) continue;
        int kind = (s16)rw(sg, (u16)(o + 0x22));
        if (kind < 0x48 || kind > 0x4a) continue;
        int m = (s16)rw(sg, (u16)(o + 0x1a)), k = (s16)rw(sg, o), t = (s16)rw(sg, (u16)(o + 0x1e));
        if (m < 0 || m >= size || t <= 0) continue;
        long left = abs_hours((s16)rw(sg, (u16)(o + 0x18)), (s16)rw(sg, (u16)(o + 0x16)), (s16)rw(sg, (u16)(o + 0x14)), (s16)rw(sg, (u16)(o + 0x12))) - now;
        if (left <= 0) continue;
        char b[96];
        if (kind == 0x48 && k >= 0 && k < 7) snprintf(b, sizeof b, "%+d %s", t, ATTRS[k]);
        else if (kind == 0x49 && k >= 0 && k < 19) snprintf(b, sizeof b, "%+d %s", t, SKILLS[k]);
        else if (kind == 0x4a) {
            int item = k == 0x69 ? 0x69 : t <= 5 ? 0x6a : 0x6b;
            std::string n = item < (int)gamedata.items.size() ? game_text(gamedata.items[item].name.c_str(), 64) : "Potion";
            if (item == 0x69) snprintf(b, sizeof b, "%s: %+d damage", n.c_str(), t);
            else if (item == 0x6a) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t);
            else if (!(t >> 4)) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t & 15);
            else snprintf(b, sizeof b, "%s: %+d damage, %+d penetration", n.c_str(), t >> 4, t & 15);
        } else continue;
        v.push_back({ m, b, left });
    }
    char head[64]; snprintf(head, sizeof head, "In effect now (%d)###effects", (int)v.size());
    if (!ImGui::CollapsingHeader(head, v.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (v.empty()) { ImGui::TextDisabled("No blessings or potions are working on the party."); return; }
    std::sort(v.begin(), v.end(), [](const Eff &a, const Eff &b) { return a.m != b.m ? a.m < b.m : a.left < b.left; });
    if (ImGui::BeginTable("eff", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        for (const Eff &e : v) {
            char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(e.m) + G_M_SHORTNAME), n, sizeof n);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(game_text(n, sizeof n).c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(e.what.c_str());
            ImGui::TableNextColumn();
            if (e.left < 48) ImGui::TextDisabled("%ld hour%s left", e.left, e.left == 1 ? "" : "s");
            else ImGui::TextDisabled("%ld days left", (e.left + 12) / 24);
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
}

static std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

extern "C" void tab_saints(void)
{
    if (gamedata.saints.empty()) { ImGui::TextWrapped("DARKLAND.LST could not be read."); return; }
    bool in_game = game_mode() != GM_OTHER;
    int size = in_game ? (s16)g_rw(G_PARTY_SIZE) : 0;
    if (size < 0 || size > 5) size = 0;
    static char find[64];
    static int known_only = 0;
    ImGui::TextWrapped("What each saint can do, and who in the party knows them. Find saints by what they help with.");
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##sfind", "Strength, fire, heal, travel, Virtue ...", find, sizeof find);
    static const char *QUICK[] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma",
                                   "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile", "Alchemy", "Religion",
                                   "Virtue", "Speak Latin", "Read/Write", "Healing", "Artifice", "Stealth", "Streetwise",
                                   "Riding", "Woodwise", "fire", "water", "heal", "reputation", "Divine Favor" };
    ImGui::SameLine();
    if (ImGui::BeginCombo("##quick", "Helps with...", ImGuiComboFlags_HeightLarge)) {
        for (const char *q : QUICK) if (ImGui::Selectable(q)) snprintf(find, sizeof find, "%s", q);
        ImGui::EndCombo();
    }
    if (in_game) {
        ImGui::RadioButton("All saints", &known_only, 0); ImGui::SameLine();
        ImGui::RadioButton("Known in the party", &known_only, 1); ImGui::SameLine();
        ImGui::RadioButton("Not yet known", &known_only, 2);
    }
    if (in_game && size > 0) effects(size);
    ImGui::Separator();
    std::string f = lower(find);
    int shown = 0;
    ImGui::BeginChild("saints", ImVec2(0, 0));
    for (int s = 0; s < (int)gamedata.saints.size(); s++) {
        const Saint &t = gamedata.saints[s];
        std::string who;
        int nk = 0;
        for (int m = 0; m < size; m++)
            if (knows(m, s)) {
                char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
                if (!who.empty()) who += ", ";
                who += game_text(n, sizeof n); nk++;
            }
        if (known_only == 1 && !nk) continue;
        if (known_only == 2 && nk) continue;
        if (!f.empty() && lower(t.name).find(f) == std::string::npos && lower(t.text).find(f) == std::string::npos) continue;
        shown++;
        ImGui::PushID(s);
        bool open = ImGui::CollapsingHeader(t.name.c_str(), f.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen);
        if (nk) { ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.55f); ImGui::TextDisabled("known by %s", who.c_str()); }
        if (open) { ImGui::Indent(20); ImGui::TextWrapped("%s", t.text.c_str()); ImGui::Unindent(20); ImGui::Spacing(); }
        ImGui::PopID();
    }
    if (!shown) ImGui::TextDisabled("No saint matches.");
    ImGui::EndChild();
}
