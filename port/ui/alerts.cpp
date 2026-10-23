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

/* the game's own message or question ("Alchemy Success - ...", "Quit to DOS? y/n"), over a native
   screen: drawn here as a note on parchment instead of letting the old screen show for it
   (mods/cardview.c catches its words; the game still waits for its key as before) */
#include "../mods/nscreen.h"
#include "ornament.h"
extern "C" { extern int cardview_dialog, game_dialog_kind; extern char game_dialog_text[600]; void plat_game_to_win(float gx, float gy, float *wx, float *wy); }
extern ImFont *ui_font_text;

extern "C" void game_dialog_draw(void)
{
    if (!cardview_dialog || !game_dialog_kind || ns_screen == NS_NONE) return;
    std::string t = game_text(game_dialog_text, strlen(game_dialog_text)), title;
    size_t d = t.find(" - ");
    if (d != std::string::npos && d < 40) { title = t.substr(0, d); t = t.substr(d + 3); }
    for (char &c : t) if (c == '\n' || c == '\r') c = ' ';
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float x0, y0, x1, y1; plat_game_to_win(60, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float k = (y1 - y0) / 200.0f, w = (x1 - x0) * 0.62f, pad = 8 * k;
    ImFont *fh = ui_font_head ? ui_font_head : ImGui::GetFont(), *fb = ui_font_text ? ui_font_text : ImGui::GetFont();
    float sh = 9 * k, sb = 7 * k;
    ImVec2 tb = fb->CalcTextSizeA(sb, FLT_MAX, w - pad * 2, t.c_str());
    float h = pad * 2 + (title.empty() ? 0 : sh * 1.4f) + tb.y + sb * 1.8f;
    ImVec2 a((x0 + x1 - w) * 0.5f, (y0 + y1 - h) * 0.5f), b(a.x + w, a.y + h);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(10, 6, 2, 110));
    dl->AddRectFilled(a, b, IM_COL32(242, 230, 200, 255), 3 * k);
    if (orn_on()) { orn_parchment_edges(dl, a, b, 6 * k); orn_gilt_frame(dl, a, b, 6 * k); }
    else dl->AddRect(a, b, IM_COL32(120, 80, 36, 255), 3 * k, 0, fmaxf(1, k * 0.6f));
    float y = a.y + pad;
    if (!title.empty()) { ImVec2 ts = fh->CalcTextSizeA(sh, FLT_MAX, 0, title.c_str()); dl->AddText(fh, sh, ImVec2((a.x + b.x - ts.x) * 0.5f, y), IM_COL32(120, 36, 20, 255), title.c_str()); y += sh * 1.4f; }
    dl->AddText(fb, sb, ImVec2(a.x + pad, y), IM_COL32(44, 26, 10, 255), t.c_str(), nullptr, w - pad * 2);
    y += tb.y + sb * 0.6f;
    const char *foot = game_dialog_kind == 3 ? "Y: yes     N: no" : game_dialog_kind == 2 ? "Any key or click: go on" : "";
    if (*foot) { ImVec2 fs = fb->CalcTextSizeA(sb * 0.85f, FLT_MAX, 0, foot); dl->AddText(fb, sb * 0.85f, ImVec2((a.x + b.x - fs.x) * 0.5f, y), IM_COL32(110, 80, 50, 255), foot); }
}
