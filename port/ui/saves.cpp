// saves.cpp - the Saves tab (F12 > Saves): every saved game with where and when, the party in
// it and its comment, newest first; load or delete any of them; quick save / quick load and
// the autosave settings (mods/saves.c does the work through the game's own routines).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include "imgui.h"
#include "money.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/saves.h"
u32 plat_ticks_ms(void);
void ui_toggle(void);
}

static const char *MONTHS[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static SaveInfo *list; static int nlist = -1;
static u32 listed_at;
static char confirm_delete[16];

static void refresh(void)
{
    free(list); list = 0;
    nlist = saves_list(&list);
    listed_at = plat_ticks_ms();
}

static std::string when(long long t)
{
    if (!t) return "";
    time_t tt = (time_t)t, now = time(0);
    struct tm a = *localtime(&tt), b = *localtime(&now);
    char s[48];
    if (a.tm_year == b.tm_year && a.tm_yday == b.tm_yday) strftime(s, sizeof s, "today %H:%M", &a);
    else if (a.tm_year == b.tm_year && a.tm_yday == b.tm_yday - 1) strftime(s, sizeof s, "yesterday %H:%M", &a);
    else strftime(s, sizeof s, "%Y-%m-%d %H:%M", &a);
    return s;
}

extern "C" int saves_restart;
extern "C" void tab_saves(void)
{
    if (nlist < 0 || plat_ticks_ms() - listed_at > 3000) refresh();
    bool in_game = game_mode() != GM_OTHER;
    char cur[16]; g_str(G_DGROUP, 0xa680, cur, sizeof cur);

    bool on = saves_on != 0;
    if (ImGui::Checkbox("Quick save, quick load and autosaves", &on)) saves_enable(on);
    { bool rs = saves_restart != 0;
      if (ImGui::Checkbox("When the party's journey ends, start again at the title (the original closes)", &rs)) { saves_restart = rs; config_set_int("saves.restart_on_end", rs); } }
    if (on) {
        ImGui::TextDisabled("Ctrl+S saves at once (no comment to type); F9 or Ctrl+L loads the last quicksave.");
        ImGui::Indent(UIS(20));
        bool a = saves_auto_arrive != 0, b = saves_auto_battle != 0;
        if (ImGui::Checkbox("Autosave on arriving at a city, castle, village ...", &a)) { saves_auto_arrive = a; config_set_int("saves.autosave_arrive", a); }
        if (ImGui::Checkbox("Autosave before every battle", &b)) { saves_auto_battle = b; config_set_int("saves.autosave_battle", b); }
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::SliderInt("autosaves kept", &saves_keep_auto, 1, 9)) config_set_int("saves.keep_auto", saves_keep_auto);
        ImGui::SameLine(0, UIS(30)); ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::SliderInt("quicksaves kept", &saves_keep_quick, 1, 9)) config_set_int("saves.keep_quick", saves_keep_quick);
        ImGui::Unindent(UIS(20));
        if (in_game) {
            if (ImGui::Button("Save now")) { saves_quick_save(); ui_toggle(); }
            ImGui::SameLine();
            ImGui::TextDisabled("as a quicksave, when the game next allows saving (not in shops or during conversations).");
        }
    }
    ImGui::Separator();
    if (nlist == 0) { ImGui::TextDisabled("No saved games yet."); return; }
    ImGui::Text("%d saved game%s, newest first", nlist, nlist == 1 ? "" : "s");
    ImGui::SameLine(); if (ImGui::SmallButton("Refresh")) refresh();
    if (!in_game) ImGui::TextDisabled("Loading from here works once a game is running; before that use the game's own Load.");
    if (!ImGui::BeginTable("saves", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerH, ImVec2(0, 0)))
        return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Where and when", ImGuiTableColumnFlags_WidthStretch, 2.2f);
    ImGui::TableSetupColumn("Party", ImGuiTableColumnFlags_WidthStretch, 2.0f);
    ImGui::TableSetupColumn("Comment", ImGuiTableColumnFlags_WidthStretch, 1.6f);
    ImGui::TableSetupColumn("Saved", ImGuiTableColumnFlags_WidthStretch, 1.3f);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, UIS(150));
    ImGui::TableHeadersRow();
    for (int i = 0; i < nlist; i++) {
        const SaveInfo &s = list[i];
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(game_text(s.place, sizeof s.place).c_str());
        ImGui::TextDisabled("%d %s %d, %d:00%s", s.day, MONTHS[(unsigned)s.month % 12], s.year, s.hour, s.battle ? "  (in battle)" : "");
        ImGui::TableNextColumn();
        std::string party;
        for (int m = 0; m < s.nmembers; m++) { if (m) party += ", "; party += game_text(s.members[m], sizeof s.members[m]); }
        ImGui::TextWrapped("%s", party.c_str());
        ImGui::TextDisabled("%s, fame %d", money_fgp(s.florins, s.groschen, s.pfennigs).c_str(), s.fame);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", game_text(s.comment, sizeof s.comment).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(when(s.mtime).c_str());
        if (!strcasecmp(cur, s.file)) ImGui::TextDisabled("the game now");   /* (its file name says nothing more) */
        else ImGui::TextDisabled("%s", s.file);
        ImGui::TableNextColumn();
        if (in_game) {
            if (ImGui::Button("Load")) { if (saves_load(s.file)) ui_toggle(); }
            ImGui::SameLine();
        }
        if (!strcmp(confirm_delete, s.file)) {
            if (ImGui::Button("Delete it?")) { saves_delete(s.file); confirm_delete[0] = 0; refresh(); }
        } else if (strcasecmp(cur, s.file) && ImGui::Button("Delete")) snprintf(confirm_delete, sizeof confirm_delete, "%s", s.file);
        ImGui::PopID();
    }
    ImGui::EndTable();
}
