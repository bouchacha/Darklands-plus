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
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
void ui_notice(const char *msg);
}
void ui_campfire_icon(ImDrawList *dl, ImVec2 c, float s);   /* ui/margins.cpp */

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
    /* the tiles' middles on the game's screen (a tile 16 x 4 pixels, odd rows half a tile to the right), the way
       drawn as it goes (route_smooth), the part in the map's window */
    std::vector<ImVec2> pts;
    for (int k = travel.i - 1; k < travel.n; k++) {
        if (k < 0) continue;
        int tx = travel.rx[k], ty = travel.ry[k];
        pts.push_back(ImVec2((float)(16 * (tx - vx) + ((ty & 1) ? 7 : -1) + 65), (float)(4 * (ty - vr) - 6 + 21)));
    }
    route_smooth(pts);
    auto inside = [](ImVec2 q) { return q.x >= 62 && q.x <= 318 && q.y >= 16 && q.y <= 198; };
    for (size_t k = 0; k < pts.size(); k++) {
        ImVec2 p; plat_game_to_win(pts[k].x, pts[k].y, &p.x, &p.y);
        if (k && inside(pts[k]) && inside(pts[k - 1])) {
            ImVec2 lastp; plat_game_to_win(pts[k - 1].x, pts[k - 1].y, &lastp.x, &lastp.y);
            dl->AddLine(lastp, p, IM_COL32(40, 20, 10, 170), 7.0f); dl->AddLine(lastp, p, IM_COL32(255, 200, 60, 235), 3.5f);
        }
        if (k + 1 == pts.size() && inside(pts[k])) { dl->AddCircleFilled(p, 7, IM_COL32(200, 40, 30, 230)); dl->AddCircle(p, 7, IM_COL32(255, 240, 200, 255), 0, 2); }
    }
    static double last; if (getenv("DL_TRAVEL_LOG") && ImGui::GetTime() - last > 1) { last = ImGui::GetTime();
        fprintf(stderr, "travel overlay: route %d/%d view %d,%d first tile %d,%d\n", travel.i, travel.n, vx, vr, travel.rx[travel.i], travel.ry[travel.i]); }
}

/* the campfire under the clock (the sky box at the banner's right end, 289..320 x 0..16): travel.camp_at_8, which
   mods/travel.c reads - unlit: off; lit with a small heart: at 20:00 when someone needs rest; lit: at 20:00 every
   night. A click (overlay_click: travel_camp_click) takes the next; what each does under the pointer */
static ImVec2 camp_a, camp_b; static int camp_frame = -10;
static const float CX0 = 296.5f, CY0 = 17.0f, CX1 = 312.5f, CY1 = 28.5f;
extern "C" void travel_camp_button(void)
{
    if (game_mode() != GM_MAP || ns_showing() || !travel_map_live() || config_get_int("original", 0)) return;
    int mode = config_get_int("travel.camp_at_8", 0);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 a, b; plat_game_to_win(CX0, CY0, &a.x, &a.y); plat_game_to_win(CX1, CY1, &b.x, &b.y);
    ImVec2 mp = ImGui::GetIO().MousePos;
    bool hot = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
    float h = b.y - a.y, r = h * 0.22f;
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
    camp_a = a; camp_b = b; camp_frame = ImGui::GetFrameCount();
    if (hot) {
        static const char *const SAY[3] = {
            "Stop to camp at 20:00: off.",
            "At 20:00 the party stops and makes camp when someone is short of {Endurance}, {Strength} or {Divine Favor}.",
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
    if (ImGui::GetFrameCount() - camp_frame > 2 || x < camp_a.x || x >= camp_b.x || y < camp_a.y || y >= camp_b.y) return 0;
    int mode = (config_get_int("travel.camp_at_8", 0) + 1) % 3;
    config_set_int("travel.camp_at_8", mode);
    static const char *const SAY[3] = { "Stop to camp at 20:00: off", "Stop to camp at 20:00: when someone needs rest", "Stop to camp at 20:00: every night" };
    ui_notice(SAY[mode]);
    fprintf(stderr, "travel: camp at 20:00 set to %d (the campfire)\n", mode);
    return 1;
}
