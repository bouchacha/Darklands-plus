// alerts.cpp - news the player must read: a window to click through, the game waiting meanwhile
// (ui_is_open). For what cannot be undone: the years' toll (mods/aging.c), an attribute or a skill
// lost for good, armour worn down or ruined (mods/losses.c). Several at once are read one after the
// other. In the test sessions (DL_CTL) they are logged and pass by themselves, unless DL_ALERT_KEEP.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <deque>
#include "imgui.h"
#include "ui.h"
#include "uiinternal.h"
#include "gamedata.h"

namespace {
struct Alert { std::string title, body, foot; };
std::deque<Alert> queue;
bool auto_pass(void) { return getenv("DL_CTL") && !getenv("DL_ALERT_KEEP"); }
}

extern "C" void ui_alert(const char *title, const char *body, const char *foot)
{
    fprintf(stderr, "alert: %s: %s\n", title, body);
    if (auto_pass()) return;
    queue.push_back({ title ? title : "", body ? body : "", foot ? foot : "" });
}
extern "C" int ui_alert_open(void) { return !queue.empty(); }
extern "C" void ui_alert_dismiss(void) { if (!queue.empty()) queue.pop_front(); }   /* (the test command ualert) */

void ui_alert_draw(void)
{
    if (queue.empty()) return;
    const Alert &a = queue.front();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 120));
    float w = fminf(io.DisplaySize.x * 0.9f, UIS(640));
    ImGui::SetNextWindowSize(ImVec2(w, 0));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.42f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowFocus();
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.55f, 0.16f, 0.10f, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UIS(3));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UIS(22), UIS(18)));
    if (ImGui::Begin("##alert", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
                     | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushFont(ui_font_head);
        ImGui::TextColored(ImVec4(0.55f, 0.16f, 0.10f, 1), "%s", a.title.c_str());
        ImGui::PopFont();
        ImGui::Spacing();
        ImGui::TextWrapped("%s", a.body.c_str());
        if (!a.foot.empty()) { ImGui::Spacing(); ui_hint("%s", a.foot.c_str()); }
        ImGui::Spacing(); ImGui::Spacing();
        char lab[48]; snprintf(lab, sizeof lab, queue.size() > 1 ? "  Next (%d more)  " : "  Continue  ", (int)queue.size() - 1);
        bool done = ImGui::Button(lab);
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere(-1);
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Space)) done = true;
        if (done) queue.pop_front();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

/* the game's characters ('{' for o umlaut ...) as UTF-8, for the mods' texts */
extern "C" void ui_text_utf8(const char *game, char *out, int n)
{
    std::string t = game_text(game, strlen(game));
    snprintf(out, (size_t)n, "%s", t.c_str());
}
