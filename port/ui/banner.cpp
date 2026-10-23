/* banner.cpp - the travel map's banner drawn anew (with the period ornaments, display.ornate):
 * the plaited band across the top (MAPHEADA, 229 x 16 at (60, 0): two strands, a light copper
 * one and a dark brown one, over and under, on grey) and the sky box at its right end (MAPHEADB
 * and its eight frames MAPBRS00..21 at (289, 0): the game shows the hour as a sun eclipsed by a
 * dark disc). Here the box holds the armillary's disc of day and night (ui/hourglass.cpp), on a
 * sky that is light by day and dark with stars by night, so the clock reads the same everywhere. */
#include <math.h>
#include <stdio.h>
#include "imgui.h"
#include "ornament.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
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
    auto strand = [&](float t0, float t1, float sg, bool light, bool edge = true) {
        for (int i = 0; i <= S; i++) { float t = t0 + (t1 - t0) * i / S; pts[i] = ImVec2(a.x + t, my + sg * A * sinf(3.14159265f * t / half)); }
        if (edge) dl->AddPolyline(pts, S + 1, EDGE, 0, th + fmaxf(1.5f, h * 0.06f));
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
    for (int k = 0; k <= m; k++) {                               /* (the joins fall on the crests: drawn over again, whole) */
        float c = (k + 0.5f) * half, e = half * 0.22f;
        strand(c - e, c + e, 1.0f, true, true); strand(c - e, c + e, -1.0f, false, true);   /* (the light strand is always the +1 one) */
    }
    dl->PopClipRect();
    dl->AddLine(ImVec2(a.x, a.y + 0.5f), ImVec2(b.x, a.y + 0.5f), GREY_D, 1.0f);
}

/* the sky box, as the game has it (MAPBRS00..21: the sun, and a dark disc that slides over it with
   the hours, whole at midnight, gone at noon; what is left of the sun at night a crescent), drawn in
   the style of the other pieces and moving smoothly with the hour (the game had eight frames): the
   sky deep blue by night, paling at dawn and dusk, with stars that twinkle and a few clouds drifting;
   the sun a gilded disc with slowly turning rays, the dark disc the night sky's own colour with a
   faint lit rim; a gilt frame */
static ImU32 mixc(ImU32 a, ImU32 b, float t)
{
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    int r = (int)((a & 255) * (1 - t) + (b & 255) * t), g = (int)((a >> 8 & 255) * (1 - t) + (b >> 8 & 255) * t), bl = (int)((a >> 16 & 255) * (1 - t) + (b >> 16 & 255) * t);
    int al = (int)((a >> 24 & 255) * (1 - t) + (b >> 24 & 255) * t);
    return IM_COL32(r, g, bl, al);
}
static void sky(ImDrawList *dl, ImVec2 a, ImVec2 b, float hour, float t)
{
    const float PI = 3.14159265f;
    float day = hour >= 6 && hour < 18 ? 1.0f : 0.0f;
    if (hour >= 4 && hour < 7) day = (hour - 4) / 3;              /* dawn and dusk between */
    if (hour >= 17 && hour < 20) day = 1 - (hour - 17) / 3;
    float glow_t = fmaxf(0.0f, 1.0f - fabsf(day - 0.5f) * 2.0f);   /* (the reds of dawn and dusk) */
    ImU32 top = mixc(IM_COL32(8, 12, 52, 255), IM_COL32(70, 120, 214, 255), day), bot = mixc(IM_COL32(22, 30, 92, 255), IM_COL32(150, 186, 236, 255), day);
    bot = mixc(bot, IM_COL32(230, 140, 90, 255), glow_t * 0.7f);
    dl->PushClipRect(a, b, true);
    dl->AddRectFilledMultiColor(a, b, top, top, bot, bot);
    float w = b.x - a.x, h = b.y - a.y;
    if (day < 1) {                                                /* the stars, twinkling */
        static const float ST[9][2] = { { 0.1f, 0.18f }, { 0.84f, 0.16f }, { 0.9f, 0.6f }, { 0.18f, 0.74f }, { 0.7f, 0.86f }, { 0.06f, 0.46f }, { 0.95f, 0.36f }, { 0.28f, 0.1f }, { 0.62f, 0.12f } };
        for (int i = 0; i < 9; i++) {
            float tw = 0.55f + 0.45f * sinf(t * (1.3f + i * 0.37f) + i * 2.1f);
            ImVec2 p(a.x + ST[i][0] * w, a.y + ST[i][1] * h);
            float r = fmaxf(0.8f, h * 0.022f) * (0.8f + 0.4f * tw);
            int al = (int)(235 * (1 - day) * tw);
            dl->AddCircleFilled(p, r, IM_COL32(200, 230, 255, al), 6);
            if (tw > 0.9f) { dl->AddLine(ImVec2(p.x - r * 2.2f, p.y), ImVec2(p.x + r * 2.2f, p.y), IM_COL32(200, 230, 255, al / 2), 1.0f); dl->AddLine(ImVec2(p.x, p.y - r * 2.2f), ImVec2(p.x, p.y + r * 2.2f), IM_COL32(200, 230, 255, al / 2), 1.0f); }
        }
    }
    for (int k = 0; k < 3; k++) {                                  /* clouds drifting along the bottom */
        float cx = a.x + fmodf(t * w * 0.012f * (1 + k * 0.3f) + k * w * 0.45f, w * 1.4f) - w * 0.2f, cy = a.y + h * (0.78f + 0.08f * (k & 1));
        ImU32 cc = mixc(IM_COL32(40, 50, 100, 150), IM_COL32(250, 250, 255, 170), day);
        cc = mixc(cc, IM_COL32(250, 170, 130, 170), glow_t * 0.6f);
        for (int j = 0; j < 3; j++) dl->AddEllipseFilled(ImVec2(cx + (j - 1) * h * 0.18f, cy - (j == 1 ? h * 0.04f : 0)), ImVec2(h * 0.16f, h * 0.07f), cc, 0, 16);
    }
    /* the sun, and the dark disc over it by the hour */
    ImVec2 c((a.x + b.x) * 0.5f, a.y + h * 0.47f);
    float r = h * 0.3f;
    float rot = t * 0.15f;
    float off = hour < 12 ? -hour / 12.0f : (24 - hour) / 12.0f;   /* (0 at midnight: the dark disc over it; +-1 at noon: gone) */
    float shown_part = fminf(1.0f, fabsf(off) * 1.3f);             /* (the rays only as much as the sun shows) */
    for (int i = 0; i < 12; i++) {                                 /* rays, turning slowly */
        float an = rot + i * PI / 6, r0 = r * 1.12f, r1 = r * (i & 1 ? 1.32f : 1.5f);
        ImVec2 p0(c.x + cosf(an - 0.09f) * r0, c.y + sinf(an - 0.09f) * r0), p1(c.x + cosf(an + 0.09f) * r0, c.y + sinf(an + 0.09f) * r0), p2(c.x + cosf(an) * r1, c.y + sinf(an) * r1);
        dl->AddTriangleFilled(p0, p1, p2, IM_COL32(250, 210, 90, (int)(210 * shown_part * fmaxf(day, 0.12f))));   /* (by night hardly any) */
    }
    dl->AddCircleFilled(c, r * 1.06f, IM_COL32(120, 70, 10, 255), 40);
    for (int i = 8; i >= 1; i--) {                                 /* the gilded disc, lit at the upper left */
        float f = i / 8.0f;
        dl->AddCircleFilled(ImVec2(c.x - r * 0.18f * (1 - f), c.y - r * 0.18f * (1 - f)), r * f, mixc(IM_COL32(214, 150, 30, 255), IM_COL32(255, 244, 170, 255), 1 - f), 40);
    }
    ImVec2 dc(c.x + off * 2.15f * r, c.y);
    if (fabsf(off) < 1.0f) {
        ImU32 shade = mixc(top, bot, 0.45f);
        dl->AddCircleFilled(dc, r * 1.04f, shade, 40);
        dl->PathArcTo(dc, r * 1.0f, off < 0 ? -PI * 0.5f : PI * 0.5f, off < 0 ? PI * 0.5f : PI * 1.5f, 20);   /* (its rim lit faintly on the sun's side) */
        dl->PathStroke(IM_COL32(255, 220, 140, 70), 0, fmaxf(1.0f, r * 0.08f));
    }
    dl->PopClipRect();
    orn_gilt_frame(dl, a, b, h * 0.28f);
}

static int sky_sheet;                                           /* test (uskysheet): the sky box at every second hour, large */
extern "C" void banner_sky_sheet(int on) { sky_sheet = on; }
extern "C" void banner_draw(void)
{
    if (sky_sheet) {
        ImDrawList *fg = ImGui::GetForegroundDrawList();
        ImVec2 ds = ImGui::GetIO().DisplaySize;
        float tw = ds.x / 4.4f, th = tw * 16.0f / 31.0f * 1.2f;
        fg->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(30, 22, 16, 255));
        for (int k = 0; k < 12; k++) {
            ImVec2 a(ds.x * 0.04f + (k % 4) * tw * 1.06f, ds.y * 0.05f + (k / 4) * th * 1.25f);
            sky(fg, a, ImVec2(a.x + tw, a.y + th), (float)(k * 2), (float)ImGui::GetTime());
            char lb[16]; snprintf(lb, sizeof lb, "%d:00", k * 2);
            fg->AddText(ImVec2(a.x, a.y + th + 4), IM_COL32(240, 230, 210, 255), lb);
        }
        return;
    }
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
    sky(dl, g2w(289, 0), g2w(320, 16), shown, (float)now);       /* (over the game's own sky box, MAPBRS) */
}
