/* hourglass.cpp - the game's time-passing clock (the "hourglass" of pass_hours_hourglass, s043
 * 150b:2a04), drawn by the interface wherever a Darklands+ page stands over the game's own: the
 * armillary of TIMER01.PIC with the sun-and-moon disc (TIMER03/04/05.PIC, turned by flipping
 * them as hourglass_frame 150b:2d3a does, one of eight positions for each three hours), and the
 * bell, the day and the month in its boxes. The game draws it at (60, 50) of the card window,
 * the disc at (108, 83). */
#include "imgui.h"
#include <stdio.h>
#include <string>
#include "gamepics.h"
#include "nsdraw.h"
#include "../mods/gamemodel.h"
#include <math.h>
extern "C" {
#include "../runtime/config.h"
}

/* ---------------------------------------------------------------- the Darklands+ clock
 * After the great clocks of the time, Wells (about 1390) and Prague (1410): a twenty-four hour dial,
 * noon at the top and midnight at the foot, its hours in Roman figures twice from I to XII; one
 * gilded hand bearing the sun, the moon a little silver disc across from it; within, the sky - the
 * day blue above the horizon, the night below with its stars, a red band at dawn and at dusk.
 * Under it a banderole with the canonical hour and the day. The hand sweeps from one bell to the
 * next. (display.clock: 1 this, 0 the game's own) */
void ui_draw_hourglass_original(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);
static const char *ROMAN[12] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII" };
static void period_clock(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    const float PI = 3.14159265f;
    ImVec2 c(tl.x + 60 * kx, tl.y + 44 * k);
    float R = 42 * k;
    /* the hand eases to the hour shown (wrapping round the day) */
    static float shown = -1; static double last_t;
    double now = ImGui::GetTime();
    float target = (float)hour;
    if (shown < 0 || now - last_t > 1.0) shown = target;
    float d = fmodf(target - shown + 36.0f, 24.0f) - 12.0f;
    shown += d * fminf(1.0f, (float)(now - last_t) * 5.0f);
    shown = fmodf(shown + 24.0f, 24.0f);
    last_t = now;
    auto at = [&](float h, float r) { float a = (h - 12.0f) / 24.0f * 2 * PI; return ImVec2(c.x + sinf(a) * r, c.y - cosf(a) * r); };
    const ImU32 gold = IM_COL32(222, 178, 74, 255), goldd = IM_COL32(150, 108, 34, 255), ink = IM_COL32(40, 24, 12, 255),
                lapis = IM_COL32(28, 46, 112, 255), night = IM_COL32(14, 20, 52, 255), day_c = IM_COL32(104, 150, 206, 255),
                dawn = IM_COL32(176, 64, 40, 255), ring = IM_COL32(236, 222, 180, 255);
    /* the square plate behind it, gilt corners */
    ImVec2 p0(c.x - R * 1.08f, c.y - R * 1.08f), p1(c.x + R * 1.08f, c.y + R * 1.08f);
    dl->AddRectFilled(p0, p1, lapis, R * 0.04f);
    dl->AddRect(p0, p1, gold, R * 0.04f, 0, fmaxf(1.5f, R * 0.035f));
    for (int q = 0; q < 4; q++) {                             /* a gilt trefoil in each corner */
        ImVec2 cc(q & 1 ? p1.x - R * 0.17f : p0.x + R * 0.17f, q & 2 ? p1.y - R * 0.17f : p0.y + R * 0.17f);
        for (int l = 0; l < 3; l++) { float a = l * 2 * PI / 3 + (q * PI / 2) + PI / 4; dl->AddCircleFilled(ImVec2(cc.x + cosf(a) * R * 0.05f, cc.y + sinf(a) * R * 0.05f), R * 0.045f, gold, 12); }
        dl->AddCircleFilled(cc, R * 0.03f, goldd, 10);
    }
    /* the hour ring */
    dl->AddCircleFilled(c, R, ring, 64);
    dl->AddCircle(c, R, goldd, 64, fmaxf(1.5f, R * 0.04f));
    float ri = R * 0.76f;
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    for (int h = 0; h < 24; h++) {
        dl->AddLine(at(h + 0.5f, ri), at(h + 0.5f, R), IM_COL32(150, 108, 34, 150), fmaxf(1.0f, R * 0.012f));   /* (between the figures) */
        const char *t = ROMAN[(h + 11) % 12];                 /* (XII at noon and at midnight) */
        float fs = R * 0.13f; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        if (ts.x > R * 0.2f) { fs *= R * 0.2f / ts.x; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t); }
        ImVec2 m = at((float)h, (ri + R) * 0.5f);
        dl->AddText(f, fs, ImVec2(m.x - ts.x * 0.5f, m.y - ts.y * 0.5f), h == 12 || h == 0 ? IM_COL32(150, 30, 20, 255) : ink, t);
    }
    /* the sky: night below the horizon, day above, red bands where the sun rises and sets */
    dl->AddCircleFilled(c, ri, night, 64);
    dl->PathArcTo(c, ri, PI, 2 * PI, 32); dl->PathFillConvex(day_c);
    dl->PathArcTo(c, ri, PI - 0.18f, PI + 0.02f, 8); dl->PathLineTo(c); dl->PathFillConvex(dawn);
    dl->PathArcTo(c, ri, -0.02f, 0.18f, 8); dl->PathLineTo(c); dl->PathFillConvex(dawn);
    static const float STARS[9][2] = { { -0.5f, 0.3f }, { -0.2f, 0.55f }, { 0.15f, 0.4f }, { 0.45f, 0.25f }, { 0.3f, 0.7f }, { -0.35f, 0.75f }, { 0.0f, 0.2f }, { 0.6f, 0.5f }, { -0.65f, 0.5f } };
    for (auto &st : STARS) { ImVec2 sp(c.x + st[0] * ri, c.y + st[1] * ri); float r = R * 0.025f; dl->AddLine(ImVec2(sp.x - r, sp.y), ImVec2(sp.x + r, sp.y), gold, 1); dl->AddLine(ImVec2(sp.x, sp.y - r), ImVec2(sp.x, sp.y + r), gold, 1); }
    dl->AddLine(ImVec2(c.x - ri, c.y), ImVec2(c.x + ri, c.y), goldd, fmaxf(1.0f, R * 0.02f));   /* the horizon */
    dl->AddCircle(c, ri, goldd, 64, fmaxf(1.0f, R * 0.025f));
    /* the moon across from the sun */
    ImVec2 mo = at(shown + 12.0f, ri * 0.55f);
    dl->AddCircleFilled(mo, R * 0.075f, IM_COL32(214, 218, 226, 255), 20);
    dl->AddCircleFilled(ImVec2(mo.x + R * 0.03f, mo.y - R * 0.01f), R * 0.065f, night, 20);   /* (a crescent) */
    /* the hand, gilt, with the sun */
    ImVec2 tip = at(shown, R * 0.97f), tail = at(shown + 12.0f, R * 0.18f);
    float a = (shown - 12.0f) / 24.0f * 2 * PI; ImVec2 nrm(cosf(a), sinf(a));
    float w = R * 0.035f;
    dl->AddQuadFilled(ImVec2(tail.x + nrm.x * w, tail.y + nrm.y * w), ImVec2(tip.x, tip.y), ImVec2(tail.x - nrm.x * w, tail.y - nrm.y * w), ImVec2(c.x, c.y), gold);
    dl->AddLine(tail, tip, goldd, fmaxf(1.0f, R * 0.01f));
    ImVec2 su = at(shown, ri * 0.72f);
    float sr = R * 0.12f;
    for (int ry = 0; ry < 16; ry++) { float b = ry * PI / 8; dl->AddLine(ImVec2(su.x + cosf(b) * sr, su.y + sinf(b) * sr), ImVec2(su.x + cosf(b) * sr * (ry % 2 ? 1.35f : 1.6f), su.y + sinf(b) * sr * (ry % 2 ? 1.35f : 1.6f)), gold, fmaxf(1.0f, R * 0.018f)); }
    dl->AddCircleFilled(su, sr, IM_COL32(244, 200, 80, 255), 24);
    dl->AddCircle(su, sr, goldd, 24, fmaxf(1.0f, R * 0.015f));
    dl->AddCircleFilled(ImVec2(su.x - sr * 0.35f, su.y - sr * 0.15f), sr * 0.12f, goldd, 8);   /* (a face: eyes and a mouth) */
    dl->AddCircleFilled(ImVec2(su.x + sr * 0.35f, su.y - sr * 0.15f), sr * 0.12f, goldd, 8);
    dl->PathArcTo(ImVec2(su.x, su.y + sr * 0.1f), sr * 0.4f, 0.4f, PI - 0.4f, 8); dl->PathStroke(goldd, 0, fmaxf(1.0f, R * 0.012f));
    dl->AddCircleFilled(c, R * 0.05f, gold, 16); dl->AddCircle(c, R * 0.05f, goldd, 16, 1);
    /* the banderole: the bell and the day */
    char bell[24]; gm_hour_name(hour, bell, sizeof bell);
    std::string b = ns_gtext(bell);
    char dd[48]; snprintf(dd, sizeof dd, "%d %s", day, gm_month_name(month));
    float by = c.y + R * 1.14f, bh = R * 0.36f, bw = R * 1.3f;
    ImU32 red = IM_COL32(150, 40, 28, 255), redd = IM_COL32(96, 22, 14, 255);
    dl->AddTriangleFilled(ImVec2(c.x - bw - bh * 0.6f, by + bh * 0.2f), ImVec2(c.x - bw, by + bh * 0.2f), ImVec2(c.x - bw, by + bh * 0.9f), redd);   /* (its tails folded behind) */
    dl->AddTriangleFilled(ImVec2(c.x - bw - bh * 0.6f, by + bh * 1.1f), ImVec2(c.x - bw, by + bh * 0.2f), ImVec2(c.x - bw, by + bh * 0.9f), redd);
    dl->AddTriangleFilled(ImVec2(c.x + bw + bh * 0.6f, by + bh * 0.2f), ImVec2(c.x + bw, by + bh * 0.2f), ImVec2(c.x + bw, by + bh * 0.9f), redd);
    dl->AddTriangleFilled(ImVec2(c.x + bw + bh * 0.6f, by + bh * 1.1f), ImVec2(c.x + bw, by + bh * 0.2f), ImVec2(c.x + bw, by + bh * 0.9f), redd);
    dl->AddRectFilled(ImVec2(c.x - bw, by), ImVec2(c.x + bw, by + bh), red, R * 0.02f);
    dl->AddRect(ImVec2(c.x - bw, by), ImVec2(c.x + bw, by + bh), gold, R * 0.02f, 0, fmaxf(1.0f, R * 0.02f));
    float fs = bh * 0.62f; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, b.c_str());
    float fs2 = bh * 0.42f; ImVec2 ts2 = f->CalcTextSizeA(fs2, FLT_MAX, 0, dd);
    float total = ts.x + R * 0.12f + ts2.x;
    if (total > bw * 1.9f) { float sc = bw * 1.9f / total; fs *= sc; fs2 *= sc; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, b.c_str()); ts2 = f->CalcTextSizeA(fs2, FLT_MAX, 0, dd); total = ts.x + R * 0.12f + ts2.x; }
    float tx = c.x - total * 0.5f;
    dl->AddText(f, fs, ImVec2(tx, by + (bh - ts.y) * 0.5f), IM_COL32(250, 230, 160, 255), b.c_str());
    dl->AddText(f, fs2, ImVec2(tx + ts.x + R * 0.12f, by + (bh - ts2.y) * 0.5f + fs * 0.08f), IM_COL32(236, 206, 170, 255), dd);
}

/* ---------------------------------------------------------------- the game's armillary, drawn anew
 * The game's clock is an armillary sphere - bronze rings for the heavens round a disc of day and
 * night - over a plate with three windows (hours, days, months). Here the same, drawn smooth at any
 * size: the outer ring with its studs and a fleur-de-lis on top, the crossing rings of the
 * meridian and the ecliptic, and in the middle the disc, its day half (the sun) and night half (the
 * moon) turning with the hours instead of in eight jumps. */
static void armillary_clock(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    const float PI = 3.14159265f;
    ImVec2 c(tl.x + 60 * kx, tl.y + 41 * k);
    float R = 36 * k;
    static float shown = -1; static double last_t;
    double now = ImGui::GetTime();
    if (shown < 0 || now - last_t > 1.0) shown = (float)hour;
    float d = fmodf((float)hour - shown + 36.0f, 24.0f) - 12.0f;
    shown = fmodf(shown + d * fminf(1.0f, (float)(now - last_t) * 5.0f) + 24.0f, 24.0f);
    last_t = now;
    const ImU32 bronze = IM_COL32(176, 104, 38, 255), bronzel = IM_COL32(230, 160, 70, 255), bronzed = IM_COL32(96, 52, 18, 255),
                gold = IM_COL32(246, 210, 80, 255), ink = IM_COL32(24, 14, 6, 255);
    float th = fmaxf(2.0f, R * 0.19f);
    const float ax = 1.12f;                                   /* (wider than tall, as the game's) */
    const ImVec2 c0 = c;
    /* a bronze ring seen at a slant (rx by ry): its dark rim, the metal, the light along its upper
       arc and the shade along its lower */
    auto ring = [&](float rx, float ry, float w, float lit, float dy = 0) {
        const int N = 72; ImVec2 pts[N];
        ImVec2 c(c0.x, c0.y + dy);
        for (int i = 0; i < N; i++) { float a = i * 2 * PI / N; pts[i] = ImVec2(c.x + cosf(a) * rx, c.y + sinf(a) * ry); }
        dl->AddPolyline(pts, N, ink, ImDrawFlags_Closed, w + fmaxf(1.5f, w * 0.3f));
        dl->AddPolyline(pts, N, bronze, ImDrawFlags_Closed, w);
        ImVec2 arc[N / 2 + 1];
        int n = 0;
        for (int i = N / 2; i <= N; i++) { float a = i * 2 * PI / N; arc[n++] = ImVec2(c.x + cosf(a) * (rx - w * 0.18f), c.y + sinf(a) * (ry - w * 0.18f)); }
        dl->AddPolyline(arc, n, lit > 0.5f ? bronzel : IM_COL32(206, 136, 56, 255), 0, fmaxf(1.0f, w * 0.28f));
        n = 0;
        for (int i = 0; i <= N / 2; i++) { float a = i * 2 * PI / N; arc[n++] = ImVec2(c.x + cosf(a) * (rx + w * 0.2f), c.y + sinf(a) * (ry + w * 0.2f)); }
        dl->AddPolyline(arc, n, bronzed, 0, fmaxf(1.0f, w * 0.25f));
    };
    /* the heavens behind: dark, the wheels of the works dim within */
    dl->AddEllipseFilled(c, ImVec2(R * ax, R), IM_COL32(16, 10, 6, 240), 0, 64);
    for (int g = 0; g < 2; g++) {                             /* two cog wheels, half hidden */
        ImVec2 gc(c.x + (g ? R * 0.42f : -R * 0.38f), c.y + (g ? -R * 0.32f : R * 0.36f));
        float gr = R * (g ? 0.42f : 0.36f);
        dl->AddCircle(gc, gr, IM_COL32(86, 86, 92, 200), 32, fmaxf(1.0f, R * 0.06f));
        for (int t = 0; t < 14; t++) { float a = t * 2 * PI / 14; dl->AddLine(ImVec2(gc.x + cosf(a) * gr, gc.y + sinf(a) * gr), ImVec2(gc.x + cosf(a) * gr * 1.14f, gc.y + sinf(a) * gr * 1.14f), IM_COL32(86, 86, 92, 200), fmaxf(1.0f, R * 0.05f)); }
    }
    /* the inner rings, nearly face on, one a little above the middle and one a little below (as
       the game draws them: their arcs pair up over and under the disc), and one round the disc */
    ring(R * ax * 0.62f, R * 0.56f, th * 0.55f, 0, -R * 0.12f);
    ring(R * ax * 0.62f, R * 0.56f, th * 0.55f, 1, R * 0.12f);
    /* the disc of day and night, turning, in its own small ring */
    float rd = R * 0.34f, a = (shown - 12.0f) / 24.0f * 2 * PI;     /* (the sun at the top at noon) */
    dl->AddCircleFilled(c, rd, IM_COL32(20, 48, 110, 255), 40);        /* night */
    dl->PathArcTo(c, rd, a - PI, a, 24); dl->PathFillConvex(IM_COL32(178, 190, 200, 255));   /* day: the half towards the sun */
    ImVec2 sun(c.x + sinf(a) * rd * 0.55f, c.y - cosf(a) * rd * 0.55f), moon(c.x - sinf(a) * rd * 0.55f, c.y + cosf(a) * rd * 0.55f);
    dl->AddCircleFilled(sun, rd * 0.3f, IM_COL32(252, 236, 60, 255), 20);
    dl->AddCircleFilled(moon, rd * 0.2f, IM_COL32(170, 176, 186, 255), 20);
    ring(rd + th * 0.25f, rd + th * 0.25f, th * 0.45f, 1);
    /* the great outer ring, face on: broad, its notches at the quarters */
    ring(R * ax - th * 0.5f, R - th * 0.5f, th, 1);
    for (int q = 0; q < 8; q++) {
        float b = q * PI / 4, x0 = cosf(b), y0 = sinf(b);
        ImVec2 p0(c.x + x0 * (R * ax - th), c.y + y0 * (R - th)), p1(c.x + x0 * R * ax, c.y + y0 * R);
        dl->AddLine(p0, p1, q % 2 ? bronzed : IM_COL32(236, 170, 90, 255), fmaxf(1.0f, th * (q % 2 ? 0.18f : 0.32f)));
    }
    /* gold clasps where the rings meet it, top and bottom, and the finial */
    for (int e = -1; e <= 1; e += 2) {
        float y = c.y + e * (R - th * 0.5f);
        for (int t = -1; t <= 1; t++) dl->AddRectFilled(ImVec2(c.x + t * R * 0.09f - R * 0.025f, y - th * 0.45f), ImVec2(c.x + t * R * 0.09f + R * 0.025f, y + th * 0.45f), gold);
        dl->AddRectFilled(ImVec2(c.x - R * 0.13f, y + e * th * 0.45f - R * 0.012f), ImVec2(c.x + R * 0.13f, y + e * th * 0.45f + R * 0.02f), gold);
    }
    ImVec2 top(c.x, c.y - R);
    dl->AddTriangleFilled(ImVec2(top.x, top.y - R * 0.3f), ImVec2(top.x - R * 0.06f, top.y), ImVec2(top.x + R * 0.06f, top.y), gold);
    dl->AddBezierQuadratic(ImVec2(top.x - R * 0.02f, top.y - R * 0.05f), ImVec2(top.x - R * 0.2f, top.y - R * 0.2f), ImVec2(top.x - R * 0.14f, top.y - R * 0.02f), gold, fmaxf(1.0f, R * 0.04f));
    dl->AddBezierQuadratic(ImVec2(top.x + R * 0.02f, top.y - R * 0.05f), ImVec2(top.x + R * 0.2f, top.y - R * 0.2f), ImVec2(top.x + R * 0.14f, top.y - R * 0.02f), gold, fmaxf(1.0f, R * 0.04f));
    /* the plate with its three windows */
    float py0 = tl.y + 79 * k, py1 = tl.y + 101 * k;
    ImVec2 a0(tl.x + 2 * kx, py0), a1(tl.x + 118 * kx, py1);
    dl->AddRectFilled(a0, a1, IM_COL32(120, 72, 36, 255), 3 * k);
    dl->AddRect(a0, a1, bronzel, 3 * k, 0, fmaxf(1.0f, k));
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    char bell[24]; gm_hour_name(hour, bell, sizeof bell);
    std::string b = ns_gtext(bell);
    char dd[8], mm[8]; snprintf(dd, sizeof dd, "%d", day); snprintf(mm, sizeof mm, "%d", month % 12 + 1);
    auto win = [&](float x0, float x1, const char *t, const char *lab) {
        ImVec2 w0(tl.x + x0 * kx, py0 + 2 * k), w1(tl.x + x1 * kx, py0 + 12 * k);
        dl->AddRectFilled(w0, w1, ink, 1.5f * k);
        dl->AddRect(w0, w1, gold, 1.5f * k, 0, fmaxf(1.0f, 0.8f * k));
        float fs = 8.0f * k; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        if (ts.x > w1.x - w0.x - 3) { fs *= (w1.x - w0.x - 3) / ts.x; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t); }
        dl->AddText(f, fs, ImVec2((w0.x + w1.x - ts.x) * 0.5f, (w0.y + w1.y - ts.y) * 0.5f), IM_COL32(252, 236, 84, 255), t);
        float ls = 5.5f * k; ImVec2 lsz = f->CalcTextSizeA(ls, FLT_MAX, 0, lab);
        dl->AddText(f, ls, ImVec2((w0.x + w1.x - lsz.x) * 0.5f, w1.y + 0.8f * k), IM_COL32(250, 214, 140, 255), lab);
    };
    win(7, 61, b.c_str(), "HOURS");
    win(68, 87, dd, "DAYS");
    win(93, 113, mm, "MONTHS");
}

const char *ui_clock_name(int k) { return k == 0 ? "The game's own" : k == 1 ? "Tower dial (about 1400)" : "Armillary, drawn anew"; }
void ui_draw_clock_style(int style, ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    if (style == 1) period_clock(dl, tl, kx, k, hour, day, month);
    else if (style == 2) armillary_clock(dl, tl, kx, k, hour, day, month);
    else {
        const GamePic *base = gamepic("PICS\\TIMER01.PIC");
        if (!base || !base->tex) { armillary_clock(dl, tl, kx, k, hour, day, month); return; }
        ui_draw_hourglass_original(dl, tl, kx, k, hour, day, month);
    }
}

void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    int st = config_get_int("display.clock", 1);
    if (st == 1) { period_clock(dl, tl, kx, k, hour, day, month); return; }
    if (st == 2) { armillary_clock(dl, tl, kx, k, hour, day, month); return; }
    ui_draw_hourglass_original(dl, tl, kx, k, hour, day, month);
}

void ui_draw_hourglass_original(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    const GamePic *base = gamepic("PICS\\TIMER01.PIC");
    if (!base || !base->tex) return;
    dl->AddImage((ImTextureID)(intptr_t)base->tex, tl, ImVec2(tl.x + base->w * kx, tl.y + base->h * k));
    /* the disc: the picture and its flips for each band of three hours (hourglass_frame) */
    static const struct { const char *pic; int fx, fy; } F[8] = {
        { "PICS\\TIMER03.PIC", 0, 1 }, { "PICS\\TIMER04.PIC", 1, 1 }, { "PICS\\TIMER05.PIC", 1, 0 }, { "PICS\\TIMER04.PIC", 1, 0 },
        { "PICS\\TIMER03.PIC", 0, 0 }, { "PICS\\TIMER04.PIC", 0, 0 }, { "PICS\\TIMER05.PIC", 0, 0 }, { "PICS\\TIMER04.PIC", 0, 1 },
    };
    int band = ((hour / 3) % 8 + 8) % 8;
    const GamePic *disc = gamepic(F[band].pic);
    if (disc && disc->tex) {
        ImVec2 a(tl.x + 48 * kx, tl.y + 33 * k), b(a.x + disc->w * kx, a.y + disc->h * k);
        ImVec2 uv0(F[band].fx ? 1.0f : 0.0f, F[band].fy ? 1.0f : 0.0f), uv1(F[band].fx ? 0.0f : 1.0f, F[band].fy ? 0.0f : 1.0f);
        dl->AddImage((ImTextureID)(intptr_t)disc->tex, a, b, uv0, uv1);
    }
    /* the boxes: the bell, the day, the month (as the game writes them: colour 14) */
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float fs = 8.0f * k;
    const ImU32 yellow = IM_COL32(252, 236, 84, 255);
    char bell[24]; gm_hour_name(hour, bell, sizeof bell);
    std::string b = ns_gtext(bell);
    char d[8], m[8]; snprintf(d, sizeof d, "%d", day); snprintf(m, sizeof m, "%d", month % 12 + 1);
    auto box = [&](float x0, float x1, const char *t) {
        ImVec2 sz = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        float s = fs;
        if (sz.x > (x1 - x0 - 2) * kx) { s = fs * (x1 - x0 - 2) * kx / sz.x; sz = f->CalcTextSizeA(s, FLT_MAX, 0, t); }
        dl->AddText(f, s, ImVec2(tl.x + (x0 + x1) * 0.5f * kx - sz.x * 0.5f, tl.y + 83.0f * k - sz.y * 0.5f), yellow, t);
    };
    box(8, 60, b.c_str());
    box(69, 86, d);
    box(95, 113, m);
}
