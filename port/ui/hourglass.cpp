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
    dl->AddCircle(su, sr * 0.62f, goldd, 20, fmaxf(1.0f, R * 0.01f));          /* (a gilt boss, no face: an inner ring, a gleam) */
    dl->AddCircleFilled(ImVec2(su.x - sr * 0.28f, su.y - sr * 0.28f), sr * 0.2f, IM_COL32(255, 238, 170, 255), 12);
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

/* the armillary's disc of day and night (also beside the time in the margin, and in the travel
   map's banner): the day half, sky-grey with the sun, turned towards the sun's place in the sky,
   the night half deep blue with the moon; noon puts the sun at the top. rim: a bronze ring round it */
void ui_draw_daynight(ImDrawList *dl, ImVec2 c, float rd, float hour, bool rim)
{
    const float PI = 3.14159265f;
    float a = (hour - 12.0f) / 24.0f * 2 * PI;
    dl->AddCircleFilled(c, rd, IM_COL32(20, 48, 110, 255), 40);        /* night */
    dl->PathArcTo(c, rd, a - PI, a, 24); dl->PathFillConvex(IM_COL32(178, 190, 200, 255));   /* day: the half towards the sun */
    ImVec2 sun(c.x + sinf(a) * rd * 0.55f, c.y - cosf(a) * rd * 0.55f), moon(c.x - sinf(a) * rd * 0.55f, c.y + cosf(a) * rd * 0.55f);
    dl->AddCircleFilled(sun, rd * 0.3f, IM_COL32(252, 236, 60, 255), 20);
    dl->AddCircleFilled(moon, rd * 0.2f, IM_COL32(170, 176, 186, 255), 20);
    if (rim) {
        float w = fmaxf(1.5f, rd * 0.16f);
        dl->AddCircle(c, rd + w * 0.5f, IM_COL32(24, 14, 6, 255), 40, w + 1.5f);
        dl->AddCircle(c, rd + w * 0.5f, IM_COL32(176, 104, 38, 255), 40, w);
        dl->PathArcTo(c, rd + w * 0.4f, PI * 1.05f, PI * 1.95f, 20); dl->PathStroke(IM_COL32(230, 160, 70, 255), 0, fmaxf(1.0f, w * 0.35f));
    }
}

/* ---------------------------------------------------------------- the astrolabe clock
 * The game's own clock looks most like an astrolabe hung on a clock: the throne at the top, a
 * rim with its notches, off-centre rings, a turning disc, the works dim behind. 
 *  3: an astrolabe in the game's own manner (its copper oval, rings, tabs and plate);
 */
static float eased(int slot, int hour)
{
    static float shown[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; static double last[8];
    double now = ImGui::GetTime();
    if (shown[slot] < 0 || now - last[slot] > 1.0) shown[slot] = (float)hour;
    float d = fmodf((float)hour - shown[slot] + 36.0f, 24.0f) - 12.0f;
    shown[slot] = fmodf(shown[slot] + d * fminf(1.0f, (float)(now - last[slot]) * 4.0f) + 24.0f, 24.0f);
    last[slot] = now;
    return shown[slot];
}
static void clock_plate(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month, ImU32 fill, ImU32 edge, ImU32 gold)
{
    float py0 = tl.y + 79 * k, py1 = tl.y + 101 * k;
    ImVec2 a0(tl.x + 2 * kx, py0), a1(tl.x + 118 * kx, py1);
    dl->AddRectFilled(a0, a1, fill, 3 * k);
    dl->AddRect(a0, a1, edge, 3 * k, 0, fmaxf(1.0f, k));
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    char bell[24]; gm_hour_name(hour, bell, sizeof bell);
    std::string b = ns_gtext(bell);
    char dd[8], mm[8]; snprintf(dd, sizeof dd, "%d", day); snprintf(mm, sizeof mm, "%d", month % 12 + 1);
    auto win = [&](float x0, float x1, const char *t, const char *lab) {
        ImVec2 w0(tl.x + x0 * kx, py0 + 2 * k), w1(tl.x + x1 * kx, py0 + 12 * k);
        dl->AddRectFilled(w0, w1, IM_COL32(24, 14, 6, 255), 1.5f * k);
        dl->AddRect(w0, w1, gold, 1.5f * k, 0, fmaxf(1.0f, 0.8f * k));
        float fs = 8.0f * k; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        if (ts.x > w1.x - w0.x - 3) { fs *= (w1.x - w0.x - 3) / ts.x; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t); }
        dl->AddText(f, fs, ImVec2((w0.x + w1.x - ts.x) * 0.5f, (w0.y + w1.y - ts.y) * 0.5f), IM_COL32(252, 236, 84, 255), t);
        float ls = 5.5f * k; ImVec2 lsz = f->CalcTextSizeA(ls, FLT_MAX, 0, lab);
        dl->AddText(f, ls, ImVec2((w0.x + w1.x - lsz.x) * 0.5f, w1.y + 0.8f * k), IM_COL32(250, 214, 140, 255), lab);
    };
    win(7, 61, b.c_str(), "HOURS"); win(68, 87, dd, "DAYS"); win(93, 113, mm, "MONTHS");
}
static void gear(ImDrawList *dl, ImVec2 c, float r, int teeth, float rot, ImU32 col, float w)
{
    const float PI = 3.14159265f;
    dl->AddCircle(c, r, col, 40, w);
    dl->AddCircle(c, r * 0.3f, col, 20, w);
    for (int s = 0; s < 4; s++) { float a = rot + s * PI / 2; dl->AddLine(ImVec2(c.x + cosf(a) * r * 0.3f, c.y + sinf(a) * r * 0.3f), ImVec2(c.x + cosf(a) * r, c.y + sinf(a) * r), col, w); }
    for (int t = 0; t < teeth; t++) { float a = rot + t * 2 * PI / teeth; dl->AddLine(ImVec2(c.x + cosf(a) * r, c.y + sinf(a) * r), ImVec2(c.x + cosf(a) * r * 1.15f, c.y + sinf(a) * r * 1.15f), col, w * 1.6f); }
}
static void throne(ImDrawList *dl, ImVec2 top, float R, ImU32 gold, ImU32 dark)
{
    /* the throne: a crown-like bracket on the rim, the shackle and its ring above */
    ImVec2 q[6] = { ImVec2(top.x - R * 0.22f, top.y + R * 0.04f), ImVec2(top.x - R * 0.14f, top.y - R * 0.12f), ImVec2(top.x - R * 0.06f, top.y - R * 0.06f),
                    ImVec2(top.x + R * 0.06f, top.y - R * 0.06f), ImVec2(top.x + R * 0.14f, top.y - R * 0.12f), ImVec2(top.x + R * 0.22f, top.y + R * 0.04f) };
    dl->AddConvexPolyFilled(q, 6, gold);
    dl->AddPolyline(q, 6, dark, ImDrawFlags_Closed, fmaxf(1.0f, R * 0.025f));
    dl->AddCircleFilled(ImVec2(top.x, top.y - R * 0.14f), R * 0.05f, gold, 12);
    dl->AddCircle(ImVec2(top.x, top.y - R * 0.24f), R * 0.09f, gold, 20, fmaxf(1.5f, R * 0.035f));
}

/* 3: an astrolabe in the game's own manner (TIMER01.PIC): its broad copper oval, lit from the upper
   left, notched and scaled; the dark works behind; the rete, two great rings and a small one, turning
   with the day; the disc of day and night at the pin; gold tabs at top and foot, the finial; the
   copper plate with its yellow-edged windows */
extern ImFont *ui_font_bold;
static ImU32 mixc(ImU32 a, ImU32 b, float t)
{
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    int r = (int)((a & 255) * (1 - t) + (b & 255) * t), g = (int)((a >> 8 & 255) * (1 - t) + (b >> 8 & 255) * t), bl = (int)((a >> 16 & 255) * (1 - t) + (b >> 16 & 255) * t);
    return IM_COL32(r, g, bl, 255);
}
/* a band between two ellipses about c (radii ro, ri), coloured by its angle: lit at the upper left */
static void lit_band(ImDrawList *dl, ImVec2 c, ImVec2 ro, ImVec2 ri, ImU32 dark, ImU32 mid, ImU32 light, float rot = 0)
{
    const float PI = 3.14159265f; const int N = 96;
    for (int i = 0; i < N; i++) {
        float a0 = i * 2 * PI / N, a1 = (i + 1) * 2 * PI / N, am = (a0 + a1) * 0.5f;
        float l = cosf(am - rot - PI * 1.25f);                   /* 1 at the upper left, -1 at the lower right */
        ImU32 col = l > 0 ? mixc(mid, light, l) : mixc(mid, dark, -l);
        ImVec2 q[4] = { ImVec2(c.x + cosf(a0) * ro.x, c.y + sinf(a0) * ro.y), ImVec2(c.x + cosf(a1) * ro.x, c.y + sinf(a1) * ro.y),
                        ImVec2(c.x + cosf(a1) * ri.x, c.y + sinf(a1) * ri.y), ImVec2(c.x + cosf(a0) * ri.x, c.y + sinf(a0) * ri.y) };
        dl->AddConvexPolyFilled(q, 4, col);
    }
}
static void astrolabe_clock(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    const float PI = 3.14159265f;
    ImVec2 c(tl.x + 60.5f * kx, tl.y + 38.5f * k);
    float u = fminf(kx, k), RX = 36.0f * u, RY = 36.0f * u, h = eased(3, hour);   /* (a true circle, whatever the pixels' shape) */
    const ImU32 cu_d = IM_COL32(124, 56, 18, 255), cu = IM_COL32(194, 100, 40, 255), cu_l = IM_COL32(244, 164, 92, 255),
                gold = IM_COL32(252, 220, 60, 255), gold_d = IM_COL32(150, 98, 10, 255), edge = IM_COL32(30, 14, 4, 255);
    float rimx = 7.5f * u, rimy = 7.5f * u;
    /* the works behind: dark, two grey wheels */
    dl->AddEllipseFilled(c, ImVec2(RX - rimx * 0.5f, RY - rimy * 0.5f), IM_COL32(16, 10, 6, 255), 0, 64);
    gear(dl, ImVec2(c.x - 13 * u, c.y - 9 * u), 10 * u, 12, 0.3f + h * 0.05f, IM_COL32(86, 84, 86, 255), fmaxf(1.0f, 1.2f * u));
    gear(dl, ImVec2(c.x + 16 * u, c.y + 7 * u), 8 * u, 10, -h * 0.06f, IM_COL32(74, 72, 76, 255), fmaxf(1.0f, 1.1f * u));
    gear(dl, ImVec2(c.x + 12 * u, c.y - 15 * u), 5 * u, 8, 0.6f, IM_COL32(64, 62, 66, 255), fmaxf(1.0f, 0.9f * u));
    /* the rete, turning once a day about the pin: two great rings either side, a small one under */
    float rot = (h - 12.0f) / 24.0f * 2 * PI;
    auto rp = [&](float x, float y) { float xx = x * u, yy = y * u; return ImVec2(c.x + xx * cosf(rot) - yy * sinf(rot), c.y + xx * sinf(rot) + yy * cosf(rot)); };
    auto ring = [&](ImVec2 rc, float r, float w) {
        dl->AddCircle(ImVec2(rc.x + 0.8f * u, rc.y + 1.0f * u), r, IM_COL32(0, 0, 0, 70), 64, w);   /* (a soft shadow on the works, no outline) */
        lit_band(dl, rc, ImVec2(r + w * 0.5f, r + w * 0.5f), ImVec2(r - w * 0.5f, r - w * 0.5f), cu_d, cu, cu_l);
    };
    ring(rp(-8, -2), 17 * u, 3.4f * u);
    ring(rp(8, -2), 17 * u, 3.4f * u);
    ring(rp(0, 6), 11 * u, 3.0f * u);
    for (int j = 0; j < 4; j++) {                                 /* the stars' gold points where the rings cross */
        static const float P[4][2] = { { 0, -17.5f }, { 0, 13.5f }, { -9, 12 }, { 9, 12 } };
        ImVec2 q = rp(P[j][0], P[j][1]);
        dl->AddCircleFilled(q, 1.1f * u, gold_d, 10); dl->AddCircleFilled(ImVec2(q.x - 0.3f * u, q.y - 0.3f * u), 0.75f * u, gold, 10);
    }
    /* the disc of day and night at the pin */
    ui_draw_daynight(dl, c, 9.0f * u, h, true);
    /* the broad copper rim over all: lit, its edges dark, notched at the twelve hours, scaled */
    dl->AddCircleFilled(ImVec2(c.x + 1.2f * u, c.y + 1.6f * u), RX + 0.4f * u, IM_COL32(0, 0, 0, 60), 96);   /* (its shadow) */
    lit_band(dl, c, ImVec2(RX, RY), ImVec2(RX - rimx, RY - rimy), cu_d, cu, cu_l);
    dl->AddEllipse(c, ImVec2(RX - rimx, RY - rimy), IM_COL32(90, 40, 12, 200), 0, 96, 0.9f * u);   /* (the rim's inner lip) */
    dl->AddEllipse(c, ImVec2(RX - rimx * 0.5f, RY - rimy * 0.5f), IM_COL32(110, 50, 18, 160), 0, 96, 0.8f * u);   /* (a groove round its middle) */
    for (int t = 0; t < 72; t++) {                                /* the degree scale on its inner edge */
        float a = t * 2 * PI / 72, l = t % 6 == 0 ? 0.42f : 0.2f;
        ImVec2 p0(c.x + cosf(a) * (RX - rimx), c.y + sinf(a) * (RY - rimy)), p1(c.x + cosf(a) * (RX - rimx * (1 - l)), c.y + sinf(a) * (RY - rimy * (1 - l)));
        dl->AddLine(p0, p1, IM_COL32(70, 30, 8, 220), fmaxf(1.0f, 0.5f * u));
    }
    for (int t = 0; t < 12; t++) {                                /* the hour notches, engraved: dark, a light lip beside */
        if (t % 3 == 0) continue;                                 /* (the quarters: the tabs and the sides) */
        float a = t * 2 * PI / 12;
        ImVec2 p0(c.x + cosf(a) * (RX - rimx * 0.8f), c.y + sinf(a) * (RY - rimy * 0.8f)), p1(c.x + cosf(a) * (RX - rimx * 0.15f), c.y + sinf(a) * (RY - rimy * 0.15f));
        dl->AddLine(p0, p1, IM_COL32(110, 48, 14, 220), fmaxf(1.0f, 0.9f * u));
        dl->AddLine(ImVec2(p0.x + 0.7f * u, p0.y + 0.7f * u), ImVec2(p1.x + 0.7f * u, p1.y + 0.7f * u), IM_COL32(240, 160, 96, 200), fmaxf(1.0f, 0.6f * u));
    }
    for (int sd = -1; sd <= 1; sd += 2) {                         /* notches at the sides */
        ImVec2 p0(c.x + sd * (RX - rimx * 0.85f), c.y), p1(c.x + sd * (RX - rimx * 0.1f), c.y);
        dl->AddLine(p0, p1, IM_COL32(110, 48, 14, 220), fmaxf(1.0f, 1.0f * u));
    }
    /* the gold tabs at top and foot (two bars and a cap, as the game's), and the finial */
    for (int e = -1; e <= 1; e += 2) {
        float y = c.y + e * (RY - rimy * 0.5f);
        for (int t = -1; t <= 1; t += 2) {
            ImVec2 a0(c.x + t * 3.2f * u - 1.6f * u, y - 2.2f * u), a1(c.x + t * 3.2f * u + 1.6f * u, y + 2.2f * u);
            dl->AddRectFilledMultiColor(a0, a1, IM_COL32(255, 236, 140, 255), IM_COL32(230, 190, 70, 255), IM_COL32(150, 98, 20, 255), IM_COL32(200, 150, 40, 255));   /* (lit from the upper left) */
        }
        float cy = y - e * 3.0f * u;
        dl->AddRectFilledMultiColor(ImVec2(c.x - 5.2f * u, cy - 0.8f * u), ImVec2(c.x + 5.2f * u, cy + 0.8f * u), IM_COL32(255, 236, 140, 255), IM_COL32(230, 190, 70, 255), IM_COL32(150, 98, 20, 255), IM_COL32(200, 150, 40, 255));
    }
    {   ImVec2 t0(c.x, c.y - RY - 0.5f * u);                      /* the finial: a gold fleur on a stem */
        dl->AddRectFilled(ImVec2(t0.x - 1.0f * u, t0.y - 3.2f * u), ImVec2(t0.x + 1.0f * u, t0.y), gold);
        dl->AddCircleFilled(ImVec2(t0.x, t0.y - 5.2f * u), 1.8f * u, gold, 12);
        dl->AddCircleFilled(ImVec2(t0.x - 2.3f * u, t0.y - 3.8f * u), 1.3f * u, gold, 10);
        dl->AddCircleFilled(ImVec2(t0.x + 2.3f * u, t0.y - 3.8f * u), 1.3f * u, gold, 10);
        dl->AddCircleFilled(ImVec2(t0.x - 0.5f * u, t0.y - 5.8f * u), 0.7f * u, IM_COL32(255, 250, 210, 255), 8);
    }
    /* the plate under it: copper, mottled; the windows edged in yellow, their names in orange */
    {
        float py0 = tl.y + 73 * k, py1 = tl.y + 101 * k;
        ImVec2 a0(tl.x + 1 * kx, py0), a1(tl.x + 120 * kx, py1);
        dl->AddRectFilledMultiColor(a0, a1, IM_COL32(150, 80, 40, 255), IM_COL32(130, 66, 32, 255), IM_COL32(104, 50, 22, 255), IM_COL32(118, 60, 28, 255));
        unsigned sd = 12345u;
        for (int i = 0; i < 260; i++) {                            /* (mottled, as the game's dithered copper) */
            sd = sd * 1103515245u + 12345u; float fx = (sd >> 8 & 1023) / 1023.0f;
            sd = sd * 1103515245u + 12345u; float fy = (sd >> 8 & 1023) / 1023.0f;
            sd = sd * 1103515245u + 12345u; bool lt = (sd >> 12 & 1) != 0;
            ImVec2 q(a0.x + fx * (a1.x - a0.x), a0.y + fy * (a1.y - a0.y));
            dl->AddRectFilled(q, ImVec2(q.x + 0.9f * kx, q.y + 0.9f * k), lt ? IM_COL32(196, 116, 64, 200) : IM_COL32(80, 36, 14, 200));
        }
        dl->AddRect(a0, a1, edge, 0, 0, fmaxf(1.0f, 1.2f * u));
        dl->AddLine(ImVec2(a0.x, a0.y + 1.0f * k), ImVec2(a1.x, a0.y + 1.0f * k), IM_COL32(220, 140, 80, 255), fmaxf(1.0f, 0.8f * u));
        ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
        char bell[24]; gm_hour_name(hour, bell, sizeof bell);
        std::string bn = ns_gtext(bell);
        char dd[8], mm[8]; snprintf(dd, sizeof dd, "%d", day); snprintf(mm, sizeof mm, "%d", month % 12 + 1);
        auto win = [&](float x0, float x1, const char *t, const char *lab) {
            ImVec2 w0(tl.x + x0 * kx, py0 + 4 * k), w1(tl.x + x1 * kx, py0 + 15 * k);
            dl->AddRectFilled(ImVec2(w0.x - 1.4f * u, w0.y - 1.4f * u), ImVec2(w1.x + 1.4f * u, w1.y + 1.4f * u), IM_COL32(252, 236, 84, 255), 1.2f * u);
            dl->AddRectFilled(w0, w1, IM_COL32(6, 4, 2, 255), 0.8f * u);
            float fs = 8.0f * k; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
            if (ts.x > w1.x - w0.x - 3) { fs *= (w1.x - w0.x - 3) / ts.x; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t); }
            dl->AddText(f, fs, ImVec2((w0.x + w1.x - ts.x) * 0.5f, (w0.y + w1.y - ts.y) * 0.5f), IM_COL32(252, 236, 84, 255), t);
            ImFont *bf = ui_font_bold ? ui_font_bold : f;
            float ls = 6.5f * k; ImVec2 lsz = bf->CalcTextSizeA(ls, FLT_MAX, 0, lab);
            ImVec2 lp((w0.x + w1.x - lsz.x) * 0.5f, w1.y + 2.2f * k);
            dl->AddText(bf, ls, ImVec2(lp.x + 0.5f * u, lp.y + 0.5f * u), IM_COL32(60, 24, 6, 255), lab);
            dl->AddText(bf, ls, lp, IM_COL32(250, 158, 96, 255), lab);
        };
        win(7, 61, bn.c_str(), "HOURS"); win(70, 87, dd, "DAYS"); win(95, 113, mm, "MONTHS");
    }
}


const char *ui_clock_name(int k)
{
    static const char *N[6] = { "The game's own", "Tower dial (about 1400)", "Armillary, drawn anew", "Copper astrolabe", "Town clock (Prague, 1410)", "The game's, drawn true" };
    return k >= 0 && k < 6 ? N[k] : "?";
}
void ui_draw_clock_style(int style, ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    if (style == 1) period_clock(dl, tl, kx, k, hour, day, month);
    else if (style == 3) astrolabe_clock(dl, tl, kx, k, hour, day, month);
    else {
        const GamePic *base = gamepic("PICS\\TIMER01.PIC");
        if (!base || !base->tex) { period_clock(dl, tl, kx, k, hour, day, month); return; }
        ui_draw_hourglass_original(dl, tl, kx, k, hour, day, month);
    }
}

void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    int st = config_get_int("display.clock", 1);
    if (st == 1) { period_clock(dl, tl, kx, k, hour, day, month); return; }
    if (st == 2 || st == 4) { period_clock(dl, tl, kx, k, hour, day, month); return; }   /* (two clocks since dropped: the tower dial) */
    if (st == 3) { ui_draw_clock_style(st, dl, tl, kx, k, hour, day, month); return; }
    if (st == 5) { ui_draw_clock_style(3, dl, tl, kx, k, hour, day, month); return; }   /* (dropped: the copper astrolabe) */
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

/* test: the astrolabe clocks large, beside the game's own (ucmd uclocksheet:) */
static int clock_sheet;
extern "C" void clock_test_sheet(void) { clock_sheet = !clock_sheet; }
extern "C" void clock_test_sheet_draw(void)
{
    if (!clock_sheet) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float k = 3.1f;
    static const int ST[4] = { 0, 3, 1, 1 };
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(4 * (122 * k + 10) + 10, 110 * k + 40), IM_COL32(52, 40, 28, 255));
    for (int i = 0; i < 4; i++) {
        ImVec2 tl(10 + i * (122 * k + 10), 10 + 6 * k);
        ui_draw_clock_style(ST[i], dl, tl, k, k, 14, 21, 9);
        dl->AddText(ImGui::GetFont(), 20, ImVec2(tl.x, tl.y + 104 * k), IM_COL32(246, 236, 214, 255), ui_clock_name(ST[i]));
    }
}
