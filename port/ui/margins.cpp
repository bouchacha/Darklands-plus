// margins.cpp - on a wide screen the 4:3 picture leaves dark margins; they can show what the
// game otherwise keeps behind menus: the date and hour, where the party is, the purse, the
// tasks the party has taken on (with their deadlines), and on the other side the blessings
// and potions working on each member and how heavily each one is loaded.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
extern ImFont *ui_font_text;
struct Eff { int m; std::string what; long left; };
std::vector<Eff> effects_now(int size);
void quest_brief(std::vector<std::pair<std::string, std::string>> &out);

int ui_margins_on = 1;

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
        std::string n = best < (int)gamedata.cities.size() ? gamedata.cities[best].name : "";
        if (n.empty()) { u32 r = g_location(best); char b[24]; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b); n = game_text(b, sizeof b); }
        return "Travelling, near " + n;
    }
    if (cur < (int)gamedata.cities.size()) return gamedata.cities[cur].name;
    u32 r = g_location(cur);
    if (!r) return "";
    char b[24]; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b);
    return game_text(b, sizeof b);
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
    ImGui::PushFont(ui_font_text);
    ImGui::SetWindowFontScale(size / ui_font_text->FontSize);
    return open;
}
static void end_panel(void)
{
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
static void heading(const char *t)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(222, 178, 104, 255));
    ImGui::TextUnformatted(t);
    ImGui::PopStyleColor();
}

extern "C" void ui_margins(void)
{
    if (!ui_margins_on || game_mode() == GM_OTHER || !ui_font_text) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    ImGuiIO &io = ImGui::GetIO();
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float left = x0, right = io.DisplaySize.x - x1;
    if (left < 150 && right < 150) return;
    float h = io.DisplaySize.y;
    float fs = std::min(std::max(std::min(left, right) / 9.5f, 15.0f), 26.0f);

    if (left >= 150 && begin_panel("##margin_l", 0, left, h, fs)) {
        int hour = (s16)g_rw(G_CUR_HOUR), day = (s16)g_rw(G_CUR_DAY), mon = (s16)g_rw(G_CUR_MONTH), year = (s16)g_rw(G_CUR_YEAR);
        heading("Today");
        ImGui::TextWrapped("%d %s %d", day, MONTHS[(unsigned)mon % 12], year);
        ImGui::TextDisabled("%d:00, %s", hour, hour < 5 || hour >= 21 ? "night" : hour < 9 ? "morning" : hour < 17 ? "day" : "evening");
        ImGui::Separator();
        heading("Where");
        ImGui::TextWrapped("%s", place_name().c_str());
        ImGui::Separator();
        heading("Purse");
        int f = (s16)g_rw(G_MONEY), g = (s16)g_rw(G_MONEY + 2), p = (s16)g_rw(G_MONEY + 4), bn = (s16)g_rw(0x9072);
        ImGui::TextWrapped("%d florins, %d groschen, %d pfennigs", f, g, p);
        if (bn > 0) ImGui::TextDisabled("%d florins in bank notes", bn);
        ImGui::Separator();
        static std::vector<std::pair<std::string, std::string>> q; static int qf = -1000;
        if (ImGui::GetFrameCount() - qf > 30) { quest_brief(q); qf = ImGui::GetFrameCount(); }
        heading("Tasks");
        if (q.empty()) ImGui::TextDisabled("None taken on.");
        for (size_t i = 0; i < q.size() && i < 6; i++) {
            ImGui::TextWrapped("%s", q[i].first.c_str());
            if (!q[i].second.empty()) ImGui::TextDisabled("%s", q[i].second.c_str());
        }
        if (q.size() > 6) ImGui::TextDisabled("... %d more (F12 > Journal)", (int)q.size() - 6);
        end_panel();
    }

    if (right >= 150 && begin_panel("##margin_r", x1, right, h, fs)) {
        std::vector<Eff> e = effects_now(size);
        for (int m = 0; m < size; m++) {
            u16 b = G_MEMBER(m);
            heading(member_name(m).c_str());
            int end = g_rb((u16)(b + G_M_ATTR_CUR)), endm = g_rb((u16)(b + G_M_ATTR_MAX)), str = g_rb((u16)(b + G_M_ATTR_CUR + 1)), strm = g_rb((u16)(b + G_M_ATTR_MAX + 1));
            ImGui::TextDisabled("End %d/%d  Str %d/%d", end, endm, str, strm);
            int load = g_rb((u16)(b + G_M_LOAD)), cap = endm + strm;
            if (load > cap) ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.35f, 1), "Load %d/%d: overloaded", load, cap);
            else if (load * 10 > cap * 9) ImGui::TextColored(ImVec4(0.92f, 0.78f, 0.40f, 1), "Load %d/%d", load, cap);
            for (const Eff &x : e) {
                if (x.m != m) continue;
                if (x.left < 48) ImGui::TextWrapped("%s (%ld h)", x.what.c_str(), x.left);
                else ImGui::TextWrapped("%s (%ld d)", x.what.c_str(), (x.left + 12) / 24);
            }
            if (m < size - 1) ImGui::Separator();
        }
        end_panel();
    }
}
