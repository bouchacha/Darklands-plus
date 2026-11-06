// travel.cpp - the route of auto-travel drawn over the overland map (mods/travel.c walks it; the
// world map, ui/worldmap.cpp, is where a journey is chosen), and the campfire under the clock that
// makes the party stop to camp at 20:00 (travel.camp_at_8).
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include "imgui.h"
#include "worldmap_draw.h"
#include "richtext.h"
#include "marks.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
void ui_notice(const char *msg);
}

extern "C" int ns_showing(void);
/* the route over the map while travelling */
extern "C" void travel_overlay(void)
{
    if (!travel.active || game_mode() != GM_MAP) return;
    if (ns_showing()) return;
    if (!travel_map_live()) return;            /* a card is up over the map (a village, an encounter); the option
                                                  count of the last card stays set after it closes */
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    int vx = (s16)g_rw(GV_map_view_col), vr = (s16)g_rw(GV_map_buf_row) + (s16)g_rw(GV_map_view_row);
    /* the route ahead in straight legs (route_ahead, tile_px pixels: a tile 16 x 4, odd rows half a tile to the
       right), on the game's screen (the map's window from x 65, y 21: tile_px less 16 a column and 4 a row of the
       view, + 56, + 9), the part in the map's window */
    std::vector<ImVec2> pts;
    route_ahead(travel.rx, travel.ry, travel.n, travel.i - 1, pts);
    for (ImVec2 &q : pts) q = ImVec2(q.x + 56 - 16.0f * vx, q.y + 9 - 4.0f * vr);
    auto inside = [](ImVec2 q) { return q.x >= 62 && q.x <= 318 && q.y >= 16 && q.y <= 198; };
    ImVec2 c0, c1; plat_game_to_win(62, 16, &c0.x, &c0.y); plat_game_to_win(318, 198, &c1.x, &c1.y);
    dl->PushClipRect(c0, c1, true);                             /* (a leg may run out of the map's window: cut at its edge) */
    for (size_t k = 0; k < pts.size(); k++) {
        ImVec2 p; plat_game_to_win(pts[k].x, pts[k].y, &p.x, &p.y);
        if (k) {
            ImVec2 lastp; plat_game_to_win(pts[k - 1].x, pts[k - 1].y, &lastp.x, &lastp.y);
            dl->AddLine(lastp, p, IM_COL32(40, 20, 10, 170), 7.0f); dl->AddLine(lastp, p, IM_COL32(255, 200, 60, 235), 3.5f);
        }
        if (k + 1 == pts.size() && inside(pts[k])) { dl->AddCircleFilled(p, 7, IM_COL32(200, 40, 30, 230)); dl->AddCircle(p, 7, IM_COL32(255, 240, 200, 255), 0, 2); }
    }
    dl->PopClipRect();
    static double last; if (getenv("DL_TRAVEL_LOG") && ImGui::GetTime() - last > 1) { last = ImGui::GetTime();
        fprintf(stderr, "travel overlay: route %d/%d view %d,%d first tile %d,%d\n", travel.i, travel.n, vx, vr, travel.rx[travel.i], travel.ry[travel.i]); }
}

/* the campfire under the clock (the sky box at the banner's right end, 289..320 x 0..16): travel.camp_at_8, which
   mods/travel.c reads - unlit: off; lit with a small heart: at 20:00 when someone needs rest; lit: at 20:00 every
   night. A click (overlay_click: travel_camp_click) takes the next; what each does under the pointer. The small
   arrow at its right hides it (map.camp_button_hidden, his ask): it slides up under the clock, and comes down while
   the pointer is on the clock (or on it); its arrow then keeps it shown again */
static ImVec2 camp_a, camp_b, hide_a, hide_b; static int camp_frame = -10;
static const float CX0 = 296.5f, CY0 = 17.0f, CX1 = 312.5f, CY1 = 28.5f, HX0 = 313.2f, HX1 = 317.6f;
static float camp_up;                                          /* 0 down (shown) .. 1 up under the clock */
extern "C" void travel_camp_button(void)
{
    if (game_mode() != GM_MAP || ns_showing() || !travel_map_live() || config_get_int("original", 0)) { camp_frame = -10; return; }
    int mode = config_get_int("travel.camp_at_8", 0), hidden = config_get_int("map.camp_button_hidden", 0);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 mp = ImGui::GetIO().MousePos;
    auto in = [&](ImVec2 p0, ImVec2 p1) { return mp.x >= p0.x && mp.x < p1.x && mp.y >= p0.y && mp.y < p1.y; };
    ImVec2 s0, s1; plat_game_to_win(289, 0, &s0.x, &s0.y); plat_game_to_win(320, 16, &s1.x, &s1.y);
    bool over = in(s0, s1) || (camp_up < 0.99f && (in(camp_a, camp_b) || in(hide_a, hide_b)));   /* (the clock, or the campfire while out) */
    float want = hidden && !over ? 1.0f : 0.0f, dt = ImGui::GetIO().DeltaTime;
    camp_up += (want - camp_up) * fminf(1.0f, dt * 12.0f);
    if (fabsf(camp_up - want) < 0.01f) camp_up = want;
    if (camp_up >= 0.999f) { camp_frame = -10; return; }       /* (up under the clock: nothing to click) */
    float dy = -(CY1 - 16.0f) * camp_up;
    ImVec2 a, b; plat_game_to_win(CX0, CY0 + dy, &a.x, &a.y); plat_game_to_win(CX1, CY1 + dy, &b.x, &b.y);
    ImVec2 ha, hb; plat_game_to_win(HX0, CY0 + dy, &ha.x, &ha.y); plat_game_to_win(HX1, CY0 + 4.6f + dy, &hb.x, &hb.y);
    bool hot = in(a, b), hhot = in(ha, hb);
    float h = b.y - a.y, r = h * 0.22f;
    dl->PushClipRect(ImVec2(a.x - h, s1.y), ImVec2(hb.x + h, b.y + h), true);   /* (under the clock's lower edge only: it slides in behind it) */
    dl->AddRectFilled(ImVec2(a.x + 1.5f, a.y + 1.5f), ImVec2(b.x + 1.5f, b.y + 1.5f), IM_COL32(10, 6, 2, 110), r);
    dl->AddRectFilled(a, b, hot ? IM_COL32(64, 42, 22, 235) : IM_COL32(40, 26, 14, 215), r);
    dl->AddRect(a, b, hot ? IM_COL32(240, 202, 96, 255) : IM_COL32(190, 150, 64, 230), r, 0, fmaxf(1.0f, h * 0.07f));
    ImVec2 c((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f + h * 0.04f);
    int v0 = dl->VtxBuffer.Size;
    ui_campfire_icon(dl, c, h * 0.86f);
    if (!mode) {                                               /* off: the fire out, grey ashes */
        for (int i = v0; i < dl->VtxBuffer.Size; i++) {
            ImU32 &col = dl->VtxBuffer[i].col;
            unsigned g = (((col & 255) + ((col >> 8) & 255) + ((col >> 16) & 255)) / 3) * 3 / 5;
            col = (col & IM_COL32_A_MASK) | g | g << 8 | g << 16;
        }
    }
    if (mode == 1) {                                           /* when someone needs rest: a small red heart */
        float k = h * 0.13f; ImVec2 p(b.x - h * 0.24f, b.y - h * 0.26f);
        dl->AddCircleFilled(ImVec2(p.x - k * 0.5f, p.y - k * 0.3f), k * 0.62f, IM_COL32(200, 40, 34, 255), 12);
        dl->AddCircleFilled(ImVec2(p.x + k * 0.5f, p.y - k * 0.3f), k * 0.62f, IM_COL32(200, 40, 34, 255), 12);
        dl->AddTriangleFilled(ImVec2(p.x - k * 1.08f, p.y - k * 0.1f), ImVec2(p.x + k * 1.08f, p.y - k * 0.1f), ImVec2(p.x, p.y + k * 1.1f), IM_COL32(200, 40, 34, 255));
    }
    {   /* the arrow: up to hide it, down (while hidden and out) to keep it shown */
        float hr = (hb.y - ha.y) * 0.25f;
        dl->AddRectFilled(ha, hb, hhot ? IM_COL32(64, 42, 22, 235) : IM_COL32(40, 26, 14, 200), hr);
        dl->AddRect(ha, hb, hhot ? IM_COL32(240, 202, 96, 255) : IM_COL32(190, 150, 64, 200), hr, 0, fmaxf(1.0f, h * 0.05f));
        ImVec2 m((ha.x + hb.x) * 0.5f, (ha.y + hb.y) * 0.5f); float k = (hb.x - ha.x) * 0.26f;
        ImU32 ac = IM_COL32(236, 206, 120, 255);
        if (!hidden) dl->AddTriangleFilled(ImVec2(m.x - k, m.y + k * 0.6f), ImVec2(m.x + k, m.y + k * 0.6f), ImVec2(m.x, m.y - k * 0.7f), ac);
        else dl->AddTriangleFilled(ImVec2(m.x - k, m.y - k * 0.6f), ImVec2(m.x + k, m.y - k * 0.6f), ImVec2(m.x, m.y + k * 0.7f), ac);
    }
    dl->PopClipRect();
    camp_a = a; camp_b = b; hide_a = ha; hide_b = hb; camp_frame = ImGui::GetFrameCount();
    if (hhot) ImGui::SetTooltip("%s", hidden ? "Keep the campfire shown." : "Hide the campfire under the clock (point at the clock to see it again).");
    else if (hot) {
        static const char *const SAY[3] = {
            "Stop to camp at 20:00: off.",
            "At 20:00 the party stops and makes camp when someone is short of {Strength}, or of more {Divine Favor} than a night brings back ({Endurance} comes back on the road by itself).",
            "At 20:00 the party stops and makes camp every night (to brew potions on the road, say)." };
        static const char *const NEXT[3] = { "Click: stop at 20:00 when someone needs rest.", "Click: stop at 20:00 every night.", "Click: off." };
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
        ui_rich_text(SAY[mode]);
        ImGui::TextDisabled("A day in camp or at an inn lasts until 5:00: from 20:00 that is a night of 9 hours, from 21:00 it is 5:00 the day after (32 hours).");
        ImGui::TextDisabled("%s", NEXT[mode]);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
extern "C" int travel_camp_click(int x, int y)
{
    if (ImGui::GetFrameCount() - camp_frame > 2) return 0;
    if (x >= hide_a.x && x < hide_b.x && y >= hide_a.y && y < hide_b.y) {
        int hidden = !config_get_int("map.camp_button_hidden", 0);
        config_set_int("map.camp_button_hidden", hidden);
        fprintf(stderr, "travel: the campfire %s\n", hidden ? "hidden under the clock" : "kept shown");
        return 1;
    }
    if (x < camp_a.x || x >= camp_b.x || y < camp_a.y || y >= camp_b.y) return 0;
    int mode = (config_get_int("travel.camp_at_8", 0) + 1) % 3;
    config_set_int("travel.camp_at_8", mode);
    static const char *const SAY[3] = { "Stop to camp at 20:00: off", "Stop to camp at 20:00: when someone needs rest", "Stop to camp at 20:00: every night" };
    ui_notice(SAY[mode]);
    fprintf(stderr, "travel: camp at 20:00 set to %d (the campfire)\n", mode);
    return 1;
}
