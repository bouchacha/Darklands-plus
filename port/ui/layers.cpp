// layers.cpp - the one order of ui/layers.h: who takes a click (the rects of the panels over the
// picture, this frame's and the last frame's; the views and the windows known from their state), and
// the drawing in the same order (the tiers' windows stacked under every other window).
#include <vector>
#include <stdio.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "layers.h"
extern "C" {
#include "../mods/nscreen.h"
#include "../mods/menuview.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
int plat_win_outside(int x, int y);
int ui_is_open(void), savesview_opts_open(void);
extern int cardview_dialog, game_dialog_kind;
}

namespace {
struct Rect { ImVec2 a, b; int layer; };
std::vector<Rect> now, before;                        /* (this frame's, the last frame's) */
int now_frame = -100, before_frame = -100;

void roll(void)
{
    int f = ImGui::GetFrameCount();
    if (now_frame == f) return;
    before.swap(now); before_frame = now_frame;
    now.clear(); now_frame = f;
}
int top_in(const std::vector<Rect> &v, float x, float y)
{
    int t = UI_FIELD;
    for (const Rect &r : v) if (x >= r.a.x && x < r.b.x && y >= r.a.y && y < r.b.y && r.layer > t) t = r.layer;
    return t;
}
}

void ui_layer_rect(ImVec2 a, ImVec2 b, int layer)
{
    roll();
    now.push_back({ a, b, layer });
}

/* a window that takes every event: ours (ui/ui.cpp ui_is_open: F12, the maps, Go to, an alert, a picker,
   a tip; the Save page's options), or the game's own message over a page (ui/alerts.cpp game_dialog_draw:
   the game waits for its key) */
extern "C" int ui_modal_up(void)
{
    return ui_is_open() || savesview_opts_open() || (cardview_dialog && game_dialog_kind && ns_screen != NS_NONE);
}

extern "C" int ui_menu_up(void) { return menu_model.open; }

bool ui_layer_clear(ImVec2 a, ImVec2 b)
{
    int f = ImGui::GetFrameCount();
    auto clear = [&](const std::vector<Rect> &v) {
        for (const Rect &r : v) if (r.layer >= UI_PANEL && a.x < r.b.x && b.x > r.a.x && a.y < r.b.y && b.y > r.a.y) return false;
        return true;
    };
    return (f - now_frame > 1 || clear(now)) && (f - before_frame > 2 || clear(before));
}

extern "C" int ui_layer_at(float x, float y)
{
    if (ui_modal_up()) return UI_MODAL;
    if (menu_model.open) return UI_MENU;                  /* (its click anywhere: on it chooses, elsewhere closes) */
    int f = ImGui::GetFrameCount(), t = UI_FIELD;
    if (f - now_frame <= 1) { int k = top_in(now, x, y); if (k > t) t = k; }
    if (f - before_frame <= 2) { int k = top_in(before, x, y); if (k > t) t = k; }
    if (t < UI_VIEW) {                                /* the views over the whole picture, while up */
        float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
        if (x >= x0 && x < x1 && y >= y0 && y < y1 && ns_showing()) t = UI_VIEW;
    }
    return t;
}

extern "C" int ui_pointer_layer(void)
{
    ImVec2 m = ImGui::GetIO().MousePos;
    return ui_layer_at(m.x, m.y);
}

extern "C" int ui_pointer_on_field(void)
{
    ImVec2 m = ImGui::GetIO().MousePos;
    return ImGui::IsMousePosValid() && !plat_win_outside((int)m.x, (int)m.y) && ui_layer_at(m.x, m.y) == UI_FIELD;
}

/* the tiers: each a window over the whole screen for its lines and shapes (ui_tier_list), and the windows
   said to be in it (ui_tier); at the frame's end all of them go under every other window, the lowest tier
   at the very back, each tier's list under its windows, the windows in the order they were drawn - and the
   passing notices over every window (under the tooltips, which ImGui draws over all windows) */
namespace {
struct TierWin { ImGuiWindow *w; int tier, order; };
std::vector<TierWin> tiers; int tiers_frame = -100;
const char *TIER_NAME[] = { "##tier_cover", "##tier_field", "##tier_panel", "##tier_menu", "##tier_dim", "##tier_toast" };
void tier_add(ImGuiWindow *w, int tier, int order)
{
    int f = ImGui::GetFrameCount();
    if (tiers_frame != f) { tiers.clear(); tiers_frame = f; }
    for (const TierWin &t : tiers) if (t.w == w) return;
    tiers.push_back({ w, tier, order });
}
}
void ui_tier(int tier) { if (ImGuiWindow *w = ImGui::GetCurrentWindow()) tier_add(w, tier, (int)tiers.size() + 1); }

ImDrawList *ui_tier_list(int tier)
{
    if (tier < 0 || tier > UI_TIER_TOAST) tier = UI_TIER_PANEL;
    ImGuiContext &g = *ImGui::GetCurrentContext();
    ImGuiWindow *w = ImGui::FindWindowByName(TIER_NAME[tier]);
    if (!w || w->LastFrameActive != g.FrameCount) {             /* (begun once a frame, at the first line drawn in it) */
        ImGuiNextWindowData keep = g.NextWindowData;           /* (what the caller set for its own next window stays) */
        g.NextWindowData.ClearFlags();
        ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(g.IO.DisplaySize);
        ImGui::Begin(TIER_NAME[tier], nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings
                                               | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        w = ImGui::GetCurrentWindow();
        ImGui::End();                                          /* (its list stays open to the frame's end, clipped to the screen) */
        g.NextWindowData = keep;
        tier_add(w, tier, 0);
    }
    return w->DrawList;
}

void ui_tiers_stack(void)
{
    if (tiers_frame != ImGui::GetFrameCount()) return;
    std::vector<TierWin> v = tiers;
    for (int t = UI_TIER_DIM; t >= UI_TIER_COVER; t--)        /* (from the top down, each to the back) */
        for (int pass = 1; pass >= 0; pass--)
            for (int i = (int)v.size() - 1; i >= 0; i--)
                if (v[i].tier == t && (v[i].order > 0) == (pass == 1)) ImGui::BringWindowToDisplayBack(v[i].w);
    for (const TierWin &t : v) if (t.tier == UI_TIER_TOAST) ImGui::BringWindowToDisplayFront(t.w);
}

/* test (ucmd ulayer): ImGui's pointer, its layer, the rects of the panels known */
extern "C" void ui_layer_test(void)
{
    ImVec2 m = ImGui::GetIO().MousePos;
    fprintf(stderr, "layer: ImGui's pointer %.0f,%.0f: at it %d, on the field %d (%d rects this frame, %d the last)\n", m.x, m.y, ui_pointer_layer(), ui_pointer_on_field(), (int)now.size(), (int)before.size());
}
