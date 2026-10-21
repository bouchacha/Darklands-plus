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

void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    if (config_get_int("display.clock", 1)) { period_clock(dl, tl, kx, k, hour, day, month); return; }
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
