// travel.cpp - the route of auto-travel drawn over the overland map (mods/travel.c walks it; the
// world map, ui/worldmap.cpp, is where a journey is chosen).
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "imgui.h"
#include "worldmap_draw.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../mods/travel.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
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
