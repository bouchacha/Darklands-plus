// travel.cpp - the Travel tab (pick a destination, see how long it takes) and the route
// drawn over the overland map (mods/travel.c walks it).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/travel.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
void ui_toggle(void);
int journal_visited(int loc);
}

extern ImFont *ui_font_small;

static std::string when(int minutes)
{
    int h = (minutes + 30) / 60, d = h / 24; h %= 24;
    char b[48];
    if (d) snprintf(b, sizeof b, "%d day%s %d h", d, d == 1 ? "" : "s", h);
    else snprintf(b, sizeof b, "%d h", h ? h : 1);
    return b;
}

static const char *kind(int t)
{
    switch (t) {
    case 0: return "city"; case 1: return "castle"; case 2: return "robber knight's castle"; case 3: return "monastery";
    case 6: return "mine"; case 8: return "village"; default: return "place";
    }
}

struct Dest { int loc, type, minutes; std::string name; };

static std::string loc_name(int i, int *type)
{
    u32 r = g_location(i);
    if (!r) return "";
    *type = rw((u16)(r >> 16), (u16)r);
    if (i < (int)gamedata.cities.size()) return gamedata.cities[i].name;
    char b[24]; g_str((u16)(r >> 16), (u16)(r + 0x26), b, sizeof b);
    return game_text(b, sizeof b);
}

extern "C" void tab_travel(void)
{
    static char filter[48];
    static bool show[4] = { true, true, false, true };     /* cities, castles & monasteries, villages, mines */
    if (!travel_enabled) { ImGui::TextWrapped("Auto-travel is switched off (Helpers tab)."); return; }
    if (travel.active) {
        ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "Travelling to %s, about %s to go.", travel.name, when(travel.left > 0 ? travel.left : travel.minutes).c_str());
        ImGui::SameLine();
        if (ImGui::Button("Stop")) travel_stop();
    }
    if (game_mode() != GM_MAP) {
        ImGui::TextWrapped("Journeys are planned on the travel map: leave the town (or finish the encounter) and open this tab there.");
        ImGui::TextDisabled("On the map you can also hold Ctrl and click any spot to travel there.");
        return;
    }
    ImGui::TextWrapped("Pick a destination: the party travels by the quickest way, keeping to roads and bridges. "
                       "Encounters interrupt the journey; it carries on afterwards. Any key or click stops it. "
                       "Ctrl+click on the map travels to that spot.");
    ImGui::SetNextItemWidth(260);
    ImGui::InputTextWithHint("##f", "name", filter, sizeof filter);
    ImGui::SameLine(); ImGui::Checkbox("Cities", &show[0]);
    ImGui::SameLine(); ImGui::Checkbox("Castles & monasteries", &show[1]);
    ImGui::SameLine(); ImGui::Checkbox("Villages", &show[2]);
    ImGui::SameLine(); ImGui::Checkbox("Mines", &show[3]);
    std::vector<Dest> v;
    std::string f = filter; std::transform(f.begin(), f.end(), f.begin(), ::tolower);
    for (int i = 0; i < G_NLOC; i++) {
        int t = -1; std::string n = loc_name(i, &t);
        if (n.empty()) continue;
        bool ok = (t == 0 && show[0]) || ((t == 1 || t == 2 || t == 3) && show[1]) || (t == 8 && show[2]) || (t == 6 && show[3]);
        if (!ok) continue;
        std::string l = n; std::transform(l.begin(), l.end(), l.begin(), ::tolower);
        if (!f.empty() && l.find(f) == std::string::npos) continue;
        int m = travel_minutes_loc(i, nullptr, nullptr);
        if (m < 0) continue;
        v.push_back({ i, t, m, n });
    }
    std::sort(v.begin(), v.end(), [](const Dest &a, const Dest &b) { return a.minutes < b.minutes; });
    if (ImGui::BeginTable("dest", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 230);
        ImGui::TableSetupColumn("Journey", ImGuiTableColumnFlags_WidthFixed, 130);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableHeadersRow();
        for (const Dest &d : v) {
            ImGui::PushID(d.loc);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(d.name.c_str());
            if (d.type == 0 && journal_visited(d.loc)) { ImGui::SameLine(); ImGui::TextDisabled("(visited)"); }
            ImGui::TableNextColumn(); ImGui::TextDisabled("%s", kind(d.type));
            ImGui::TableNextColumn(); ImGui::TextUnformatted(when(d.minutes).c_str());
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Go") && travel_to_loc(d.loc, d.name.c_str())) ui_toggle();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

/* the route over the map while travelling */
extern "C" void travel_overlay(void)
{
    if (!travel.active || game_mode() != GM_MAP) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    int vx = (s16)g_rw(0x286a), vr = (s16)g_rw(0x286e) + (s16)g_rw(0x286c);
    ImVec2 lastp; bool have = false;
    for (int k = travel.i - 1; k < travel.n; k++) {
        if (k < 0) continue;
        int tx = travel.rx[k], ty = travel.ry[k];
        float sx = (float)(16 * (tx - vx) + ((ty & 1) ? 7 : -1) + 65), sy = (float)(4 * (ty - vr) - 6 + 21);
        if (sx < 62 || sx > 318 || sy < 16 || sy > 198) { have = false; continue; }
        ImVec2 p; plat_game_to_win(sx, sy, &p.x, &p.y);
        if (have) { dl->AddLine(lastp, p, IM_COL32(40, 20, 10, 170), 7.0f); dl->AddLine(lastp, p, IM_COL32(255, 200, 60, 235), 3.5f); }
        lastp = p; have = true;
        if (k == travel.n - 1) { dl->AddCircleFilled(p, 7, IM_COL32(200, 40, 30, 230)); dl->AddCircle(p, 7, IM_COL32(255, 240, 200, 255), 0, 2); }
    }
    static double last; if (getenv("DL_TRAVEL_LOG") && ImGui::GetTime() - last > 1) { last = ImGui::GetTime();
        fprintf(stderr, "travel overlay: route %d/%d view %d,%d first tile %d,%d\n", travel.i, travel.n, vx, vr, travel.rx[travel.i], travel.ry[travel.i]); }
}
