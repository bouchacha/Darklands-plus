/* banner.cpp - the travel map's banner drawn anew (with the period ornaments, display.ornate):
 * the plaited band across the top (MAPHEADA, 229 x 16 at (60, 0): two strands, a light copper
 * one and a dark brown one, over and under, on grey) and the sky box at its right end (MAPHEADB
 * and its eight frames MAPBRS00..21 at (289, 0): the game shows the hour as a sun eclipsed by a
 * dark disc). Here the box holds the armillary's disc of day and night (ui/hourglass.cpp), on a
 * sky that is light by day and dark with stars by night, so the clock reads the same everywhere. */
#include <math.h>
#include "imgui.h"
#include "ornament.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
void ui_draw_daynight(ImDrawList *dl, ImVec2 c, float rd, float hour, bool rim);   /* ui/hourglass.cpp */
extern "C" int ns_showing(void);

static ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }

/* the plaited band: P to Q along its middle, w high */
static void band(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    const ImU32 GREY = IM_COL32(118, 116, 112, 255), GREY_D = IM_COL32(72, 70, 66, 255), GREY_L = IM_COL32(150, 148, 142, 255);
    const ImU32 LIGHT = IM_COL32(204, 128, 72, 255), LIGHT_HI = IM_COL32(240, 176, 116, 255), DARK = IM_COL32(104, 62, 30, 255), DARK_HI = IM_COL32(140, 92, 52, 255),
                EDGE = IM_COL32(40, 22, 10, 255);
    float h = b.y - a.y, w = b.x - a.x, my = (a.y + b.y) * 0.5f;
    dl->AddRectFilled(a, b, GREY);
    dl->AddRectFilled(a, ImVec2(b.x, a.y + h * 0.08f), GREY_L);
    dl->AddRectFilled(ImVec2(a.x, b.y - h * 0.1f), b, GREY_D);
    float half = h * 0.62f;                                      /* crossing to crossing */
    int m = (int)roundf(w / half); if (m < 2) m = 2;
    half = w / m;
    float A = h * 0.27f, th = fmaxf(2.0f, h * 0.2f);
    const int S = 10;
    ImVec2 pts[S + 1];
    dl->PushClipRect(a, b, true);
    auto strand = [&](float t0, float t1, float sg, bool light) {
        for (int i = 0; i <= S; i++) { float t = t0 + (t1 - t0) * i / S; pts[i] = ImVec2(a.x + t, my + sg * A * sinf(3.14159265f * t / half)); }
        dl->AddPolyline(pts, S + 1, EDGE, 0, th + fmaxf(1.5f, h * 0.06f));
        dl->AddPolyline(pts, S + 1, light ? LIGHT : DARK, 0, th);
        for (int i = 0; i <= S; i++) pts[i].y -= th * 0.22f;
        dl->AddPolyline(pts, S + 1, light ? LIGHT_HI : DARK_HI, 0, fmaxf(1.0f, th * 0.28f));
    };
    for (int k = 0; k <= m; k++) {                               /* at each crossing, the strand under, then the one over */
        float t0 = (k - 0.5f) * half, t1 = (k + 0.5f) * half;
        bool light_over = (k & 1) != 0;
        strand(t0, t1, light_over ? -1.0f : 1.0f, !light_over);
        strand(t0, t1, light_over ? 1.0f : -1.0f, light_over);
    }
    dl->PopClipRect();
    dl->AddLine(ImVec2(a.x, a.y + 0.5f), ImVec2(b.x, a.y + 0.5f), GREY_D, 1.0f);
}

/* the sky box: the sky by the hour, the disc of day and night, a gilt frame */
static void sky(ImDrawList *dl, ImVec2 a, ImVec2 b, float hour)
{
    float day = hour >= 5 && hour < 19 ? 1.0f : 0.0f;
    if (hour >= 4 && hour < 6) day = (hour - 4) / 2;            /* dawn and dusk between */
    if (hour >= 18 && hour < 20) day = 1 - (hour - 18) / 2;
    auto mix = [&](int n0, int n1) { return (int)(n0 + (n1 - n0) * day); };
    ImU32 top = IM_COL32(mix(10, 96), mix(16, 132), mix(70, 220), 255), bot = IM_COL32(mix(24, 150), mix(34, 178), mix(110, 236), 255);
    dl->AddRectFilledMultiColor(a, b, top, top, bot, bot);
    float w = b.x - a.x, h = b.y - a.y;
    if (day < 1) {                                              /* the stars */
        static const float ST[7][2] = { { 0.12f, 0.2f }, { 0.82f, 0.18f }, { 0.9f, 0.62f }, { 0.2f, 0.72f }, { 0.68f, 0.84f }, { 0.08f, 0.45f }, { 0.94f, 0.36f } };
        for (auto &s : ST) dl->AddCircleFilled(ImVec2(a.x + s[0] * w, a.y + s[1] * h), fmaxf(1.0f, h * 0.025f), IM_COL32(170, 230, 255, (int)(230 * (1 - day))), 6);
    }
    ui_draw_daynight(dl, ImVec2((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f), h * 0.34f, hour, true);
    orn_gilt_frame(dl, a, b, h * 0.28f);
}

extern "C" void banner_draw(void)
{
    if (!orn_on() || game_mode() != GM_MAP || ns_showing()) return;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    band(dl, g2w(60, 0), g2w(289, 16));
    static float shown = -1; static double last;
    float hour = (float)(s16)g_rw(G_CUR_HOUR);
    double now = ImGui::GetTime();
    if (shown < 0 || now - last > 1.0) shown = hour;
    float d = fmodf(hour - shown + 36.0f, 24.0f) - 12.0f;        /* (the disc eases round with the hours) */
    shown = fmodf(shown + d * fminf(1.0f, (float)(now - last) * 4.0f) + 24.0f, 24.0f);
    last = now;
    sky(dl, g2w(289, 0), g2w(320, 16), shown);
}
