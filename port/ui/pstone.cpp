/* pstone.cpp - the Philosopher's Stone, as the alchemists pictured it (display.pstone_look). The
 * Great Work went through its colours in order - nigredo (black: the raven), albedo (white: the
 * swan; the white stone, which makes silver), citrinitas (yellow; the many colours of the
 * peacock's tail between), rubedo (red: the phoenix; the red stone, which makes gold) - and the
 * party's stone shows the colour its quality has reached:
 *   quality 1-9 black, 10-19 white, 20-34 the peacock's colours, yellow, 35 and up red.
 *   0 the philosophers' egg: the sealed glass vessel of the Work (vas hermeticum), its neck melted
 *     shut ("Hermes' seal"), sitting in the athanor's mouth; the matter in it of the stage's colour,
 *     and the stage's bird over it;
 *   1 the stone itself: an unworked lump, glassy and heavy, black, white, yellow or (at the end)
 *     red to purple and shining - not a jeweller's cut gem.
 * Each drawn at c, s across; quality 0: none yet, the empty egg or a faint outline. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "../runtime/config.h"

namespace {
const float PI = 3.14159265f;
ImU32 A(ImU32 c, float a) { int al = (int)(((c >> IM_COL32_A_SHIFT) & 255) * a); return (c & ~IM_COL32_A_MASK) | ((ImU32)(al < 0 ? 0 : al > 255 ? 255 : al) << IM_COL32_A_SHIFT); }
void glow(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float strength)
{
    for (int i = 12; i >= 1; i--) { float f = i / 12.0f; dl->AddCircleFilled(c, r * f, A(col, strength * (1 - f) * (1 - f) * 0.35f), 32); }
}
/* the stage a quality has reached: 0 none, 1 black, 2 white, 3 peacock / yellow, 4 red */
/* (the best stone the game makes: a university's, 14..29 by the town; an alchemist's 1..25 (s056 18c94,
   s066 1957e): the stages in four steps up to 29) */
int stage_of(int q) { return q <= 0 ? 0 : q <= 7 ? 1 : q <= 14 ? 2 : q <= 21 ? 3 : 4; }
ImU32 matter(int st, int shade)                    /* shade 0 dark, 1 mid, 2 light */
{
    static const ImU32 C[5][3] = {
        { IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0) },
        { IM_COL32(14, 12, 14, 255), IM_COL32(40, 36, 42, 255), IM_COL32(90, 84, 100, 255) },
        { IM_COL32(170, 172, 180, 255), IM_COL32(222, 224, 230, 255), IM_COL32(252, 252, 255, 255) },
        { IM_COL32(176, 120, 20, 255), IM_COL32(226, 178, 40, 255), IM_COL32(255, 230, 120, 255) },
        { IM_COL32(96, 8, 30, 255), IM_COL32(178, 20, 40, 255), IM_COL32(250, 96, 90, 255) } };
    return C[st < 0 ? 0 : st > 4 ? 4 : st][shade];
}
/* the bird of the stage, small, wings spread over the matter: raven, swan, peacock, phoenix */
void bird(ImDrawList *dl, ImVec2 c, float u, int st, float t)
{
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    float flap = sinf(t * 3.0f) * 0.8f;
    ImU32 body = st == 1 ? IM_COL32(20, 18, 22, 255) : st == 2 ? IM_COL32(250, 250, 252, 255) : st == 3 ? IM_COL32(30, 110, 130, 255) : IM_COL32(210, 40, 20, 255);
    ImU32 edge = st == 2 ? IM_COL32(120, 120, 130, 255) : IM_COL32(0, 0, 0, 180);
    if (st == 3)                                       /* the peacock's tail: a fan of eyes */
        for (int i = 0; i < 7; i++) {
            float a = PI * (0.62f + 0.76f * i / 6.0f);
            ImVec2 e = P(cosf(a) * 8, 2 + sinf(a) * 8 * -1);
            dl->AddLine(P(0, 2), e, IM_COL32(40, 120, 70, 255), 0.8f * u);
            dl->AddCircleFilled(e, 1.6f * u, IM_COL32(30, 140, 110, 255), 10);
            dl->AddCircleFilled(e, 0.9f * u, IM_COL32(40, 60, 170, 255), 8);
            dl->AddCircleFilled(e, 0.4f * u, IM_COL32(230, 190, 60, 255), 6);
        }
    if (st == 4) {                                     /* the phoenix rises from flames */
        for (int i = -2; i <= 2; i++) dl->AddTriangleFilled(P(i * 1.8f - 1.4f, 5), P(i * 1.8f + 0.2f * sinf(t * 7 + i), 1.5f - (2 - abs(i)) * 0.8f), P(i * 1.8f + 1.4f, 5), IM_COL32(255, 170, 40, 230));
    }
    /* wings */
    dl->AddTriangleFilled(P(-0.5f, 0), P(-7, -3 + flap), P(-5, 1.5f), body);
    dl->AddTriangleFilled(P(0.5f, 0), P(7, -3 + flap), P(5, 1.5f), body);
    if (st == 2) { dl->AddTriangle(P(-0.5f, 0), P(-7, -3 + flap), P(-5, 1.5f), edge, 0.6f * u); dl->AddTriangle(P(0.5f, 0), P(7, -3 + flap), P(5, 1.5f), edge, 0.6f * u); }
    /* body and head */
    dl->AddEllipseFilled(P(0, 0.6f), ImVec2(2.0f * u, 3.0f * u), body, 0, 16);
    dl->AddCircleFilled(P(0, -3.2f), 1.4f * u, body, 12);
    if (st == 2) dl->AddEllipse(P(0, 0.6f), ImVec2(2.0f * u, 3.0f * u), edge, 0, 16, 0.6f * u);
    dl->AddTriangleFilled(P(-0.4f, -3.4f), P(0.4f, -3.4f), P(0, -1.9f), st == 1 ? IM_COL32(60, 56, 50, 255) : IM_COL32(240, 150, 30, 255));   /* the beak */
    if (st == 4) for (int i = 0; i < 3; i++) dl->AddLine(P(-0.8f + i * 0.8f, -4.5f), P(-1.2f + i * 1.2f, -6.2f), IM_COL32(255, 200, 60, 255), 0.6f * u);   /* (its crest) */
}
} // namespace

void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t)
{
    bool numbered = quality > 0;                        /* (a quality below 0: that stone, without its number) */
    if (quality < 0) quality = quality == -1 ? 0 : -quality;
    float u = s / 40.0f;
    int st = stage_of(quality);
    float pulse = 0.85f + 0.15f * sinf(t * 2.2f);
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    if (look == 2) {                                   /* the squared circle (Maier, Atalanta fugiens, emblem XXI, 1618):
           "make of a man and a woman a circle, of that a square, of this a triangle, then a circle again,
           and you will have the Philosophers' Stone" - drawn inward as the stone grows: the circle, then the
           square in it, the triangle, the small circle; the ink black, white, gold, red by the stage; the
           stone's quality in the middle */
        /* p: how far the Work has come, 0..4 over the qualities 0..29 (the best the game makes), without
           steps: the ink passes slowly from black to white to gold to red, each figure fades in over its
           span, the light grows */
        float R = 18 * u, p = fminf(4.0f, quality * 4.0f / 29.0f), q = (float)quality;
        auto mix = [](ImU32 a, ImU32 b, float t) {
            t = t < 0 ? 0 : t > 1 ? 1 : t;
            int r = (int)((a & 255) * (1 - t) + (b & 255) * t), g = (int)((a >> 8 & 255) * (1 - t) + (b >> 8 & 255) * t), bl = (int)((a >> 16 & 255) * (1 - t) + (b >> 16 & 255) * t), al = (int)((a >> 24 & 255) * (1 - t) + (b >> 24 & 255) * t);
            return IM_COL32(r, g, bl, al); };
        static const ImU32 INK[5] = { IM_COL32(120, 100, 80, 110), IM_COL32(24, 20, 22, 255), IM_COL32(236, 238, 244, 255), IM_COL32(222, 170, 40, 255), IM_COL32(190, 24, 30, 255) };
        static const ImU32 BACK[5] = { IM_COL32(244, 234, 208, 255), IM_COL32(244, 234, 208, 255), IM_COL32(60, 64, 76, 255), IM_COL32(244, 234, 208, 255), IM_COL32(244, 234, 208, 255) };
        int lo = (int)floorf(p); lo = lo < 1 ? 1 : lo > 3 ? 3 : lo;   /* (black up to 1, white at 2, gold at 3, red at 4) */
        int hi = lo + 1; float fr = p <= 1 ? 0.0f : fminf(1.0f, p - lo);
        ImU32 ink = quality <= 0 ? INK[0] : mix(INK[lo], INK[hi], fr), back = mix(BACK[lo], BACK[hi], fr);
        float lw = fmaxf(1.0f, 1.5f * u);
        auto fade = [&](float from) { float t = (p - from) / 0.9f; return t < 0 ? 0.0f : t > 1 ? 1.0f : t; };
        float gl = fade(2.2f);                                      /* (the light: from the gold onwards, redder and brighter to the end) */
        if (gl > 0) glow(dl, c, R * (1.3f + 0.25f * fade(3.0f)), mix(IM_COL32(255, 200, 60, 255), IM_COL32(255, 60, 40, 255), fade(3.0f)), (0.4f + 0.5f * fade(3.0f) + 0.1f * fade(3.9f)) * gl * pulse);
        dl->AddCircleFilled(c, R, back, 48);
        dl->AddCircle(c, R, ink, 48, lw * 1.3f);
        float sq = R * 0.7071f;                                    /* the square, its corners on the circle */
        ImVec2 S[4] = { P(-sq / u, -sq / u), P(sq / u, -sq / u), P(sq / u, sq / u), P(-sq / u, sq / u) };
        if (fade(0.4f) > 0) dl->AddPolyline(S, 4, A(ink, fade(0.4f)), ImDrawFlags_Closed, lw);
        /* the triangle, its corners on the square's sides: point up, base on the square's foot */
        ImVec2 T[3] = { P(0, -sq / u), P(sq / u, sq / u), P(-sq / u, sq / u) };
        if (fade(1.4f) > 0) dl->AddPolyline(T, 3, A(ink, fade(1.4f)), ImDrawFlags_Closed, lw);
        /* the small circle in the triangle (its incircle) */
        float ir = sq * 2 * 0.3090f, icy = c.y + sq - ir;     /* (inradius of that triangle: base 2sq, height 2sq) */
        float fi = fade(2.4f);
        if (fi > 0) {
            dl->AddCircleFilled(ImVec2(c.x, icy), ir, A(mix(IM_COL32(236, 196, 70, 255), IM_COL32(200, 30, 30, 255), fade(3.1f)), fi), 32);
            dl->AddCircle(ImVec2(c.x, icy), ir, A(ink, fi), 32, lw);
        }
        int st2 = fi > 0.5f ? 4 : p >= 1 && p < 2 ? 2 : 1;          /* (the number's colours: light on the filled circle or the dark ground) */
        st = st2 == 4 ? 4 : (back & 255) < 128 ? 2 : 1;
        /* the stone at its best (29, the finest a university makes): a crown of golden rays turning slowly
           round the figure, sparks along the circle */
        if (quality >= 29) {
            float rot = t * 0.25f;
            for (int k = 0; k < 16; k++) {
                float a = rot + k * PI / 8, l = (k & 1) ? 0.22f : 0.36f;
                ImVec2 d(cosf(a), sinf(a)), n(-d.y, d.x);
                ImVec2 b0(c.x + d.x * R * 1.06f + n.x * R * 0.07f, c.y + d.y * R * 1.06f + n.y * R * 0.07f), b1(c.x + d.x * R * 1.06f - n.x * R * 0.07f, c.y + d.y * R * 1.06f - n.y * R * 0.07f);
                dl->AddTriangleFilled(b0, ImVec2(c.x + d.x * R * (1.06f + l), c.y + d.y * R * (1.06f + l)), b1, IM_COL32(240, 196, 70, (int)(200 * pulse)));
            }
            for (int k = 0; k < 4; k++) {
                float a = -rot * 1.7f + k * PI / 2;
                ImVec2 sp(c.x + cosf(a) * R, c.y + sinf(a) * R);
                dl->AddCircleFilled(sp, R * 0.07f, IM_COL32(255, 250, 220, 230), 8);
            }
        }
        /* the number: the stone's quality, in the middle, as an alchemist would ink it (MedievalSharp, the
           manuscript hand of the game's fonts), dark ink on a halo of parchment so it reads on any stage */
        if (numbered && quality > 0) {
            char b[16]; snprintf(b, sizeof b, "%d", quality);
            extern ImFont *ui_font_game[];
            ImFont *f = ui_font_game[4] ? ui_font_game[4] : ImGui::GetFont();
            float fs = R * (quality >= 10 ? 0.82f : 0.95f);
            ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, b);
            ImVec2 tp(c.x - ts.x * 0.5f, icy - ts.y * 0.55f);
            float ry = fmaxf(ir * 0.85f, ts.y * 0.5f), rx = fmaxf(ts.x * 0.72f, ry);   /* (the halo wider than the number: no line crosses it) */
            dl->AddEllipseFilled(ImVec2(c.x, icy), ImVec2(rx, ry), IM_COL32(250, 242, 222, 235), 0, 28);
            dl->AddEllipse(ImVec2(c.x, icy), ImVec2(rx, ry), A(ink, 0.35f), 0, 28, fmaxf(1.0f, lw * 0.5f));
            dl->AddText(f, fs, tp, IM_COL32(40, 20, 12, 255), b);
            dl->AddText(f, fs, ImVec2(tp.x + fmaxf(0.5f, fs * 0.02f), tp.y), IM_COL32(40, 20, 12, 255), b);   /* (a heavier stroke) */
            (void)q; (void)st;
        }
        return;
    }
    const ImU32 GLASS = IM_COL32(214, 230, 236, 70), GLASS_EDGE = IM_COL32(80, 104, 116, 255), SHINE = IM_COL32(255, 255, 255, 150);
    if (look == 1) {                                   /* the stone: an unworked lump */
        static const float R[11] = { 1.0f, 0.86f, 0.95f, 0.78f, 0.9f, 1.02f, 0.84f, 0.96f, 0.8f, 0.92f, 0.88f };
        ImVec2 o[11];
        for (int i = 0; i < 11; i++) { float a = i * 2 * PI / 11 + 0.3f; o[i] = P(cosf(a) * 15 * R[i], sinf(a) * 12 * R[(i + 3) % 11] + 2); }
        if (st == 0) { dl->AddPolyline(o, 11, IM_COL32(120, 100, 80, 120), ImDrawFlags_Closed, 1.2f * u); return; }
        if (st == 4) glow(dl, P(0, 2), 26 * u, IM_COL32(255, 50, 40, 255), pulse);
        else if (st == 3) glow(dl, P(0, 2), 22 * u, IM_COL32(255, 200, 60, 255), 0.6f * pulse);
        else if (st == 2) glow(dl, P(0, 2), 20 * u, IM_COL32(255, 255, 255, 255), 0.5f);
        dl->AddConcavePolyFilled(o, 11, matter(st, 1));
        /* its glassy body: darker below, a lighter cleft above, a highlight; the red one shows depth */
        ImVec2 top[5] = { o[6], o[7], o[8], P(-2, -2), P(-8, 0) };
        dl->AddConcavePolyFilled(top, 5, matter(st, 2));
        ImVec2 bot[5] = { o[1], o[2], o[3], P(-4, 6), P(6, 5) };
        dl->AddConcavePolyFilled(bot, 5, matter(st, 0));
        if (st == 3) {                                 /* (the peacock's colours: a sheen of green and blue across the yellow) */
            dl->AddLine(P(-10, 3), P(6, -5), IM_COL32(40, 150, 120, 110), 2.2f * u);
            dl->AddLine(P(-8, 7), P(9, -2), IM_COL32(60, 80, 180, 90), 2.0f * u);
        }
        if (st == 4) dl->AddCircleFilled(P(1, 3), 5 * u, IM_COL32(255, 120, 100, (int)(110 * pulse)), 20);
        dl->AddPolyline(o, 11, st == 2 ? IM_COL32(120, 120, 130, 255) : IM_COL32(20, 10, 10, 220), ImDrawFlags_Closed, 1.1f * u);
        dl->AddLine(P(-7, -5), P(-3, -6.5f), SHINE, 1.4f * u);
        dl->AddCircleFilled(P(-8.5f, -4), 1.0f * u, SHINE, 8);
        return;
    }
    /* the philosophers' egg in the athanor's mouth */
    /* the athanor: a squat tower of brick, its fire showing at the door */
    dl->AddRectFilled(P(-12, 9), P(12, 22), IM_COL32(150, 80, 50, 255), 1.5f * u);
    for (int r = 0; r < 3; r++) for (int k = 0; k < 4; k++) dl->AddRect(P(-12 + k * 6 + (r & 1) * 3 - 3, 9 + r * 4.3f), P(-12 + k * 6 + (r & 1) * 3 + 3, 13.3f + r * 4.3f), IM_COL32(90, 44, 26, 160), 0, 0, 0.5f * u);
    dl->AddRectFilled(P(-14, 7.5f), P(14, 10), IM_COL32(110, 60, 36, 255), 1 * u);
    dl->AddRectFilled(P(-4, 15), P(4, 22), IM_COL32(30, 14, 8, 255), 3 * u, ImDrawFlags_RoundCornersTop);
    if (st > 0) { dl->AddTriangleFilled(P(-3, 22), P(0, 16.5f + sinf(t * 8) * 0.8f), P(3, 22), IM_COL32(255, 140, 30, 255)); dl->AddTriangleFilled(P(-1.5f, 22), P(0, 18.5f), P(1.5f, 22), IM_COL32(255, 230, 120, 255)); }
    /* the glow of the stage */
    if (st == 4) glow(dl, P(0, 2), 24 * u, IM_COL32(255, 60, 40, 255), pulse);
    else if (st == 3) glow(dl, P(0, 2), 20 * u, IM_COL32(255, 210, 80, 255), 0.6f * pulse);
    else if (st == 2) glow(dl, P(0, 2), 18 * u, IM_COL32(255, 255, 255, 255), 0.5f);
    /* the egg: a tall oval of glass, its neck drawn long and melted shut */
    dl->AddRectFilled(P(-1.6f, -21), P(1.6f, -10), GLASS);
    dl->AddLine(P(-1.6f, -21), P(-1.6f, -10.5f), GLASS_EDGE, 0.9f * u);
    dl->AddLine(P(1.6f, -21), P(1.6f, -10.5f), GLASS_EDGE, 0.9f * u);
    dl->AddCircleFilled(P(0, -21.5f), 1.9f * u, GLASS_EDGE, 12);                       /* (the seal: the glass melted into a knob) */
    dl->AddEllipseFilled(P(0, 0), ImVec2(9.5f * u, 12.5f * u), GLASS, 0, 40);
    /* the matter in it, filling its lower third: the colour of the stage */
    if (st > 0) {
        dl->PathClear();
        for (int i = 0; i <= 20; i++) { float a = PI * (0.06f + 0.88f * i / 20.0f); dl->PathLineTo(P(cosf(a) * 9.1f, sinf(a) * 12.1f)); }
        dl->PathFillConvex(matter(st, 1));
        dl->AddEllipseFilled(P(0, 1.2f), ImVec2(8.6f * u, 1.4f * u), matter(st, 2), 0, 24);
        if (st == 3)
            for (int i = 0; i < 5; i++) { static const ImU32 PC[5] = { IM_COL32(40, 140, 120, 160), IM_COL32(50, 70, 170, 150), IM_COL32(160, 40, 130, 140), IM_COL32(60, 150, 60, 150), IM_COL32(230, 120, 30, 150) };
                dl->AddCircleFilled(P(-6 + i * 3, 1.2f), 1.3f * u, PC[i], 10); }
        bird(dl, P(0, -4), u * 0.85f, st, t);
    }
    dl->AddEllipse(P(0, 0), ImVec2(9.5f * u, 12.5f * u), GLASS_EDGE, 0, 40, 1.1f * u);
    dl->AddLine(P(-6.5f, -6), P(-5.5f, 4), SHINE, 1.4f * u);
}
int ui_pstone_look(void) { int v = config_get_int("display.pstone_look", 2); return v >= 0 && v <= 2 ? v : 2; }
const char *ui_pstone_look_name(int k) { static const char *N[3] = { "The philosophers' egg", "The stone itself", "The squared circle" }; return k >= 0 && k <= 2 ? N[k] : "?"; }
const char *ui_pstone_stage_name(int q)
{
    static const char *N[5] = { "none yet", "black (nigredo): the raven", "white (albedo): the swan", "yellow (citrinitas): the peacock's colours", "red (rubedo): the phoenix" };
    return N[stage_of(q)];
}

/* test (DL_PSTONE_SHEET): both looks, through every stage */
extern "C" void ui_pstone_sheet_draw(void)
{
    static int on = -1; if (on < 0) on = getenv("DL_PSTONE_SHEET") != 0;
    if (!on) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList(); ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(242, 230, 200, 255));
    static const int Q[5] = { 0, 5, 12, 19, 29 };
    if (atoi(getenv("DL_PSTONE_SHEET")) == 2) {               /* the squared circle at every quality, 0..29 */
        for (int q = 0; q < 30; q++) {
            float cx = ds.x * (0.1f + (q % 6) * 0.16f), cy = ds.y * (0.11f + (q / 6) * 0.19f);
            ui_pstone_draw(dl, ImVec2(cx, cy), ds.y * 0.15f, 2, q, (float)ImGui::GetTime());
        }
        return;
    }
    static const char *SN[5] = { "none yet", "5 (1-7: black)", "12 (8-14: white)", "19 (15-21: yellow)", "29 (22-29: red)" };
    float colw = ds.x / 5.4f;
    for (int look = 0; look < 3; look++) {
        float y = ds.y * (0.02f + look * 0.32f);
        dl->AddText(ImGui::GetFont(), ds.y * 0.035f, ImVec2(ds.x * 0.03f, y), IM_COL32(110, 30, 20, 255), ui_pstone_look_name(look));
        for (int k = 0; k < 5; k++) {
            float cx = colw * (k + 0.75f);
            ui_pstone_draw(dl, ImVec2(cx, y + ds.y * 0.15f), ds.y * 0.2f, look, Q[k], (float)ImGui::GetTime());
            dl->AddText(ImGui::GetFont(), ds.y * 0.022f, ImVec2(cx - colw * 0.3f, y + ds.y * 0.27f), IM_COL32(60, 40, 20, 255), SN[k]);
            ui_pstone_draw(dl, ImVec2(cx + colw * 0.36f, y + ds.y * 0.27f), ds.y * 0.05f, look, Q[k], (float)ImGui::GetTime());   /* (and as small as it is drawn on the screens) */
        }
    }
}
