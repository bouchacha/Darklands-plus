// alerts.cpp - news the player must read: a window to click through, the game waiting meanwhile
// (ui_is_open). For what cannot be undone: the years' toll (mods/aging.c), an attribute or a skill
// lost for good, armour worn down or ruined (mods/losses.c). Several at once are read one after the
// other. Its OK button closes one, but not in its first half second, and Enter or Space (his wish,
// 2026-11-01: the pointer could not be found over it) not in its first second: a key (Space in a battle,
// Enter on a card) or a click meant for what was under it closed it before it could be read (1.4-RC7).
// In the test sessions (DL_CTL) they are logged and pass by themselves, unless DL_ALERT_KEEP.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <deque>
#include "imgui.h"
#include "ui.h"
#include "uiinternal.h"
#include "gamedata.h"
#include "alerts.h"
#include "skillicons.h"
#include <vector>
#include "../mods/gamemodel.h"

namespace {
struct Alert { std::string title, body, foot; std::vector<UiAlertRow> rows; double shown = -1; };
std::deque<Alert> queue;
bool auto_pass(void) { return getenv("DL_CTL") && !getenv("DL_ALERT_KEEP"); }
}

extern "C" void ui_alert(const char *title, const char *body, const char *foot)
{
    fprintf(stderr, "alert: %s: %s\n", title, body);
    if (auto_pass()) return;
    queue.push_back({ title ? title : "", body ? body : "", foot ? foot : "" });
}
extern "C" void ui_alert_rows(const char *title, const char *body, const UiAlertRow *rows, int nrows, const char *foot)
{
    fprintf(stderr, "alert: %s: %s\n", title, body);
    if (auto_pass()) return;
    Alert a{ title ? title : "", body ? body : "", foot ? foot : "", {} };
    for (int i = 0; i < nrows; i++) a.rows.push_back(rows[i]);
    queue.push_back(a);
}
extern "C" int ui_alert_open(void) { return !queue.empty(); }

/* the rows: a member's name, the age, then each attribute's or skill's shield with was and now */
static void alert_rows(const std::vector<UiAlertRow> &rows)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float lh = ImGui::GetTextLineHeight(), is = lh * 1.35f, gap = lh * 0.6f, right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    for (const UiAlertRow &r : rows) {
        ImGui::PushFont(ui_font_head); ImGui::TextUnformatted(game_text(r.name, strlen(r.name)).c_str()); ImGui::PopFont();
        bool first = true;
        auto chip = [&](float w) { if (!first) { ImGui::SameLine(0, gap); if (ImGui::GetCursorScreenPos().x + w > right) ImGui::NewLine(); } first = false;
                                   ImVec2 p = ImGui::GetCursorScreenPos(); ImGui::Dummy(ImVec2(w, is)); return p; };
        if (r.age >= 0) {                                       /* the age: an hourglass and the years */
            char t[16]; snprintf(t, sizeof t, "%d", r.age);
            ImVec2 p = chip(is + lh * 0.25f + ImGui::CalcTextSize(t).x);
            ui_age_icon(dl, ImVec2(p.x + is * 0.5f, p.y + is * 0.5f), is * 0.92f);
            dl->AddText(ImVec2(p.x + is + lh * 0.25f, p.y + (is - lh) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), t);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Age %d", r.age);
        }
        for (int i = 0; i < r.n; i++) {                         /* each change: the shield, was (dim), an arrow, now (red) */
            const UiAlertChange &c = r.ch[i];
            char was[12], now[12]; snprintf(was, sizeof was, "%d", c.was); snprintf(now, sizeof now, "%d", c.now);
            float ww = ImGui::CalcTextSize(was).x, nw = ImGui::CalcTextSize(now).x, aw = lh * 0.8f;
            ImVec2 p = chip(is + lh * 0.25f + ww + aw + nw);
            ImVec2 ic(p.x + is * 0.5f, p.y + is * 0.5f);
            if (c.key < 7) ui_attr_icon(dl, ic, is * 0.92f, c.key); else ui_skill_icon(dl, ic, is * 0.92f, c.key - 7);
            float x = p.x + is + lh * 0.25f, y = p.y + (is - lh) * 0.5f;
            dl->AddText(ImVec2(x, y), ImGui::GetColorU32(ImGuiCol_TextDisabled), was); x += ww;
            float ay = y + lh * 0.52f;
            dl->AddTriangleFilled(ImVec2(x + aw * 0.25f, ay - lh * 0.22f), ImVec2(x + aw * 0.25f, ay + lh * 0.22f), ImVec2(x + aw * 0.75f, ay), ImGui::GetColorU32(ImGuiCol_TextDisabled));
            x += aw;
            dl->AddText(ImVec2(x, y), IM_COL32(170, 40, 30, 255), now);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s: %d, was %d", c.key < 7 ? gm_attr_name(c.key) : gm_skill_name(c.key - 7), c.now, c.was);
        }
        ImGui::Spacing();
    }
}
extern "C" void ui_alert_dismiss(void) { if (!queue.empty()) queue.pop_front(); }   /* (the test command ualert) */
/* a window that takes the events before the notice (F12, the welcome, the tips: ui.cpp ui_event) is up: the
   notice waits behind it, unseen, and when it comes back its OK waits its half second again, so that the click
   that closed that window does not close the notice unread (1.4.6-RC14: it stood over F12 and took its place) */
void ui_alert_hold(void) { if (!queue.empty()) queue.front().shown = -1; }

void ui_alert_draw(void)
{
    if (queue.empty()) return;
    Alert &a = queue.front();
    if (a.shown < 0) a.shown = ImGui::GetTime();
    const bool ready = ImGui::GetTime() - a.shown >= 0.5;
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
                     | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {   /* (OK after half a second; Enter or Space after a second) */
        ImGui::PushFont(ui_font_head);
        ImGui::TextColored(ImVec4(0.55f, 0.16f, 0.10f, 1), "%s", a.title.c_str());
        ImGui::PopFont();
        ImGui::Spacing();
        if (a.rows.empty()) ImGui::TextWrapped("%s", a.body.c_str());
        else alert_rows(a.rows);
        if (!a.foot.empty()) { ImGui::Spacing(); ui_hint("%s", a.foot.c_str()); }
        ImGui::Spacing(); ImGui::Spacing();
        char lab[48]; snprintf(lab, sizeof lab, queue.size() > 1 ? "  OK (%d more)  " : "  OK  ", (int)queue.size() - 1);
        ImGui::BeginDisabled(!ready);
        bool key = ImGui::GetTime() - a.shown >= 1.0
                   && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false) || ImGui::IsKeyPressed(ImGuiKey_Space, false));
        bool done = (ImGui::Button(lab) || key) && ready;   /* (the notice takes every event while up: the key goes no further, ui.cpp) */
        ImGui::EndDisabled();
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

/* its text in lines of at most width (font f at size) */
static void wrap_lines(ImFont *f, float size, const std::string &s, float width, std::vector<std::string> &out)
{
    const char *p = s.c_str(), *e = p + s.size();
    while (p < e) {
        const char *q = f->CalcWordWrapPositionA(size / f->FontSize, p, e, width);
        if (q <= p) q = p + 1;
        const char *qe = q; while (qe > p && qe[-1] == ' ') qe--;
        out.push_back(std::string(p, qe));
        p = q; while (p < e && *p == ' ') p++;
    }
}
/* The box is as wide as its words, everything in it centred: a heading of the message's own words ("Alchemy
   Success", "Saint Call Fails" - what comes before " - ", or the whole of a short message), in green for a
   success and red for a failure, beside the shield of what it took (Alchemy, Divine Favor); a gilt rule; the
   rest of the words; for a message that waits, what goes on. (1.4.6-RC12, his words: "one giant blank box and
   the text shows up left and top aligned".) */
extern "C" void game_dialog_draw(void)
{
    if (!cardview_dialog || !game_dialog_kind || ns_screen == NS_NONE) return;
    std::string t = game_text(game_dialog_text, strlen(game_dialog_text)), title;
    for (char &c : t) if (c == '\n' || c == '\r') c = ' ';
    while (!t.empty() && t.back() == ' ') t.pop_back();
    size_t d = t.find(" - ");
    if (d != std::string::npos && d < 40) { title = t.substr(0, d); t = t.substr(d + 3); }
    else if (t.size() <= 40 && t.find_first_of(".!?:,") == std::string::npos) { title = t; t.clear(); }
    const bool good = title.find("Success") != std::string::npos, bad = title.find("Fail") != std::string::npos;
    const int key = title.find("Saint") != std::string::npos ? ATTR_DF : title.find("Alchemy") != std::string::npos ? ui_key_of("Alchemy", 7) : -1;
    const ImU32 head = good ? IM_COL32(40, 98, 38, 255) : bad ? IM_COL32(156, 34, 20, 255) : IM_COL32(120, 36, 20, 255);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float x0, y0, x1, y1; plat_game_to_win(60, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    const float k = (y1 - y0) / 200.0f, pad = 11 * k, sh = 10 * k, sb = 7 * k, lh = sb * 1.25f, is = sh * 0.95f;
    ImFont *fh = ui_font_head ? ui_font_head : ImGui::GetFont(), *fb = ui_font_text ? ui_font_text : ImGui::GetFont();
    const float wmax = (x1 - x0) * 0.62f, wmin = (x1 - x0) * 0.34f;
    const float iw = key >= 0 ? is * UI_SHIELD_W + 4 * k : 0;
    const float tw = title.empty() ? 0 : fh->CalcTextSizeA(sh, FLT_MAX, 0, title.c_str()).x + iw;
    std::vector<std::string> lines; float bw = 0;
    if (!t.empty()) wrap_lines(fb, sb, t, wmax - pad * 2, lines);
    for (const std::string &l : lines) bw = fmaxf(bw, fb->CalcTextSizeA(sb, FLT_MAX, 0, l.c_str()).x);
    const char *foot = game_dialog_kind == 3 ? "Y: yes     N: no" : game_dialog_kind == 2 ? "Any key or click: continue" : "";
    const float fs = sb * 0.85f, fw = *foot ? fb->CalcTextSizeA(fs, FLT_MAX, 0, foot).x : 0;
    const float w = fminf(wmax, fmaxf(wmin, fmaxf(tw, fmaxf(bw, fw)) + pad * 2));
    const float rule = !title.empty() && !lines.empty() ? 9 * k : 0;
    const float h = pad * 2 + (title.empty() ? 0 : sh * 1.2f) + rule + lines.size() * lh + (*foot ? fs * 1.9f : 0);
    const ImVec2 a(roundf((x0 + x1 - w) * 0.5f), roundf((y0 + y1 - h) * 0.5f)), b(a.x + w, a.y + h);
    const float mx = (a.x + b.x) * 0.5f;
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(10, 6, 2, 110));
    dl->AddRectFilled(a, b, IM_COL32(242, 230, 200, 255), 3 * k);
    if (orn_on()) { orn_parchment_edges(dl, a, b, 6 * k); orn_gilt_frame(dl, a, b, 6 * k); }
    else dl->AddRect(a, b, IM_COL32(120, 80, 36, 255), 3 * k, 0, fmaxf(1, k * 0.6f));
    float y = a.y + pad;
    if (!title.empty()) {
        float sx = mx - tw * 0.5f;
        if (key >= 0) ui_key_icon(dl, ImVec2(sx + is * UI_SHIELD_W * 0.5f, y + sh * 0.6f), is, key);
        dl->AddText(fh, sh, ImVec2(sx + iw, y), head, title.c_str());
        y += sh * 1.2f;
    }
    if (rule > 0) {
        if (orn_on()) orn_rule(dl, mx - w * 0.22f, mx + w * 0.22f, y + rule * 0.45f, 5 * k);
        else dl->AddLine(ImVec2(mx - w * 0.22f, y + rule * 0.45f), ImVec2(mx + w * 0.22f, y + rule * 0.45f), IM_COL32(150, 110, 60, 255), fmaxf(1, k * 0.5f));
        y += rule;
    }
    const bool centred = lines.size() <= 2;                 /* (a long text reads better from a straight left edge) */
    for (const std::string &l : lines) {
        float lx = centred ? mx - fb->CalcTextSizeA(sb, FLT_MAX, 0, l.c_str()).x * 0.5f : a.x + pad;
        dl->AddText(fb, sb, ImVec2(lx, y), IM_COL32(44, 26, 10, 255), l.c_str());
        y += lh;
    }
    if (*foot) dl->AddText(fb, fs, ImVec2(mx - fw * 0.5f, y + fs * 0.7f), IM_COL32(110, 80, 50, 255), foot);
}
