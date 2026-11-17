// layers.cpp - who takes a click (the order is in layers.h): the rects of the panels over the
// picture, this frame's and the last frame's, and the views known from their state.
#include <vector>
#include "imgui.h"
#include "layers.h"
extern "C" {
#include "../mods/nscreen.h"
#include "../mods/menuview.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
int plat_win_outside(int x, int y);
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

extern "C" int ui_layer_at(float x, float y)
{
    int f = ImGui::GetFrameCount(), t = UI_FIELD;
    if (f - now_frame <= 1) { int k = top_in(now, x, y); if (k > t) t = k; }
    if (f - before_frame <= 2) { int k = top_in(before, x, y); if (k > t) t = k; }
    if (t < UI_VIEW) {                                /* the views over the whole picture, while up */
        float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
        if (x >= x0 && x < x1 && y >= y0 && y < y1 && (menu_model.open || ns_showing())) t = UI_VIEW;
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
    return !plat_win_outside((int)m.x, (int)m.y) && ui_layer_at(m.x, m.y) == UI_FIELD;
}
