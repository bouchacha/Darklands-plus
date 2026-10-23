/* alchemydesk.cpp - the alchemy screen's desk drawn anew (display.alchemy_art 2; ui/alchemyview.cpp):
 * the game's own arrangement (ALCSCRN0.PIC: an open book on a wooden table, a bowl at the upper
 * right, the bag and ribbon below, a bar at the lower right) in the style of the other drawn
 * pieces: an oak table, a book bound in red leather with gilt tooling, a brass mortar and its
 * pestle, a tallow candle in a pewter dish, a glass flask, the red ribbon, a brass plate for the
 * button. In game coordinates (x 60..320, y 0..200), the book's pages where the screen writes:
 * x 67..189 and 191..312, y 45..166. */
#include "imgui.h"
#include <math.h>
#include "ornament.h"

extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);

namespace {
const float PI = 3.14159265f;
bool in_box; ImVec2 box_a, box_b;              /* drawn into a box (the settings' picture) instead of over the screen */
ImVec2 W(float x, float y)
{
    if (in_box) return ImVec2(box_a.x + (x - 60) / 260.0f * (box_b.x - box_a.x), box_a.y + y / 200.0f * (box_b.y - box_a.y));
    float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b);
}
ImU32 mixc(ImU32 a, ImU32 b, float t)
{
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    int r = (int)((a & 255) * (1 - t) + (b & 255) * t), g = (int)((a >> 8 & 255) * (1 - t) + (b >> 8 & 255) * t), bl = (int)((a >> 16 & 255) * (1 - t) + (b >> 16 & 255) * t);
    int al = (int)((a >> 24 & 255) * (1 - t) + (b >> 24 & 255) * t);
    return IM_COL32(r, g, bl, al);
}
/* a band between two ellipses, lit at the upper left */
void lit_band(ImDrawList *dl, ImVec2 c, ImVec2 ro, ImVec2 ri, ImU32 dark, ImU32 mid, ImU32 light)
{
    const int N = 96;
    for (int i = 0; i < N; i++) {
        float a0 = i * 2 * PI / N, a1 = (i + 1) * 2 * PI / N, am = (a0 + a1) * 0.5f;
        float l = cosf(am - PI * 1.25f);
        ImU32 col = l > 0 ? mixc(mid, light, l) : mixc(mid, dark, -l);
        ImVec2 q[4] = { ImVec2(c.x + cosf(a0) * ro.x, c.y + sinf(a0) * ro.y), ImVec2(c.x + cosf(a1) * ro.x, c.y + sinf(a1) * ro.y),
                        ImVec2(c.x + cosf(a1) * ri.x, c.y + sinf(a1) * ri.y), ImVec2(c.x + cosf(a0) * ri.x, c.y + sinf(a0) * ri.y) };
        dl->AddConvexPolyFilled(q, 4, col);
    }
}
/* a soft round glow (light or shadow): rings fading out */
void glow(ImDrawList *dl, ImVec2 c, ImVec2 r, ImU32 col, int steps = 14)
{
    int a0 = (int)(col >> IM_COL32_A_SHIFT) & 255;
    for (int i = steps; i >= 1; i--) {
        float f = i / (float)steps;
        ImU32 cc = (col & ~IM_COL32_A_MASK) | ((ImU32)(a0 * (1 - f) * (1 - f) / steps * 3.0f) << IM_COL32_A_SHIFT);
        dl->AddEllipseFilled(c, ImVec2(r.x * f, r.y * f), cc, 0, 40);
    }
}
float hash(int i) { unsigned h = (unsigned)i * 2654435761u; h ^= h >> 13; h *= 0x5bd1e995; h ^= h >> 15; return (h & 0xffff) / 65535.0f; }

void table(ImDrawList *dl)
{
    ImVec2 a = W(60, 0), b = W(320, 200);
    dl->AddRectFilledMultiColor(a, b, IM_COL32(104, 64, 34, 255), IM_COL32(98, 60, 32, 255), IM_COL32(66, 38, 20, 255), IM_COL32(72, 42, 22, 255));
    /* planks, lengthwise: seams and the grain's long wavy lines */
    const float PL = 28;
    for (int p = 0; p * PL < 200; p++) {
        float y0 = p * PL, y1 = y0 + PL;
        float tone = (hash(p * 7 + 1) - 0.5f) * 0.18f;
        ImU32 t = tone > 0 ? IM_COL32(255, 220, 170, (int)(tone * 120)) : IM_COL32(20, 8, 0, (int)(-tone * 160));
        dl->AddRectFilled(W(60, y0), W(320, y1), t);
        for (int g = 0; g < 9; g++) {
            float gy = y0 + 2 + (PL - 4) * hash(p * 31 + g), amp = 0.6f + 1.4f * hash(p * 17 + g), ph = 6.28f * hash(p * 5 + g * 3), fr = 0.03f + 0.04f * hash(g * 11 + p);
            ImVec2 pts[40];
            for (int i = 0; i < 40; i++) { float x = 60 + i * (260.0f / 39); pts[i] = W(x, gy + amp * sinf(x * fr + ph)); }
            dl->AddPolyline(pts, 40, hash(g * 5 + p) > 0.35f ? IM_COL32(40, 20, 6, 70 + (int)(60 * hash(g + p * 3))) : IM_COL32(170, 120, 70, 50), 0, 1.0f);
        }
        if (hash(p * 13) > 0.45f) {                               /* a knot */
            float kx = 80 + 220 * hash(p * 19), ky = y0 + PL * (0.3f + 0.4f * hash(p * 23));
            glow(dl, W(kx, ky), ImVec2(9, 5), IM_COL32(30, 12, 2, 120), 8);
        }
        float jx = 90 + 200 * hash(p * 41 + 2);                  /* where two boards meet end to end */
        dl->AddLine(W(jx, y0), W(jx, y1), IM_COL32(24, 10, 2, 200), 1.5f);
        dl->AddLine(W(jx + 0.6f, y0), W(jx + 0.6f, y1), IM_COL32(150, 104, 62, 70), 1.0f);
        for (int n = 0; n < 2; n++) dl->AddCircleFilled(W(jx + (n ? 3.0f : -3.0f), y0 + PL * 0.5f), 1.0f, IM_COL32(30, 20, 14, 220), 8);   /* (iron nails) */
        dl->AddLine(W(60, y1 - 0.4f), W(320, y1 - 0.4f), IM_COL32(24, 10, 2, 220), 2.0f);
        dl->AddLine(W(60, y1 + 0.5f), W(320, y1 + 0.5f), IM_COL32(150, 104, 62, 90), 1.0f);
    }
    /* the light from the upper left, the room dark at the edges */
    glow(dl, W(150, 60), ImVec2((b.x - a.x) * 0.75f, (b.y - a.y) * 0.8f), IM_COL32(255, 220, 160, 70), 18);
    float ex = (b.x - a.x) * 0.10f, ey = (b.y - a.y) * 0.10f;
    dl->AddRectFilledMultiColor(a, ImVec2(a.x + ex, b.y), IM_COL32(0, 0, 0, 120), 0, 0, IM_COL32(0, 0, 0, 120));
    dl->AddRectFilledMultiColor(ImVec2(b.x - ex, a.y), b, 0, IM_COL32(0, 0, 0, 130), IM_COL32(0, 0, 0, 130), 0);
    dl->AddRectFilledMultiColor(ImVec2(a.x, b.y - ey), b, 0, 0, IM_COL32(0, 0, 0, 130), IM_COL32(0, 0, 0, 130));
    dl->AddRectFilledMultiColor(a, ImVec2(b.x, a.y + ey), IM_COL32(0, 0, 0, 80), IM_COL32(0, 0, 0, 80), 0, 0);
}

void book(ImDrawList *dl, float k)
{
    /* its shadow, the leather, the gilt line tooled round it */
    dl->AddRectFilled(W(66, 46), W(318, 175), IM_COL32(0, 0, 0, 90), 6 * k);
    dl->AddRectFilled(W(64, 46), W(316, 172), IM_COL32(0, 0, 0, 60), 4 * k);
    ImVec2 ca = W(63, 41), cb = W(316, 171);
    dl->AddRectFilledMultiColor(ca, cb, IM_COL32(136, 36, 26, 255), IM_COL32(118, 30, 22, 255), IM_COL32(84, 18, 14, 255), IM_COL32(100, 24, 18, 255));
    dl->AddRect(ca, cb, IM_COL32(40, 6, 4, 255), 2 * k, 0, 1.5f);
    dl->AddRect(W(64.2f, 42.2f), W(314.8f, 169.8f), IM_COL32(222, 178, 84, 170), 0, 0, 1.0f);
    for (int c = 0; c < 4; c++) {                                  /* brass corners */
        float x = c & 1 ? 316 : 63, y = c & 2 ? 171 : 41, sx = c & 1 ? -1.0f : 1.0f, sy = c & 2 ? -1.0f : 1.0f;
        ImVec2 tri[3] = { W(x, y), W(x + sx * 7, y), W(x, y + sy * 10) };
        dl->AddTriangleFilled(tri[0], tri[1], tri[2], IM_COL32(196, 150, 70, 255));
        dl->AddLine(tri[1], tri[2], IM_COL32(250, 214, 130, 255), 1.0f);
    }
    /* the pages: the block of leaves at each side, then the open leaves darkening into the gutter */
    for (int s = 0; s < 2; s++) {
        float x0 = s ? 190 : 66, x1 = s ? 314 : 190;
        for (int i = 0; i < 3; i++) {                              /* the edges of the leaves under */
            float o = 2.2f - i * 0.7f;
            ImVec2 pa = W(s ? x0 : x0 + o * 0.3f, 44 + o * 0.2f), pb = W(s ? x1 - o * 0.3f : x1, 167 + o);
            dl->AddRectFilled(pa, pb, i & 1 ? IM_COL32(214, 196, 160, 255) : IM_COL32(236, 222, 190, 255));
        }
        ImVec2 pa = W(s ? 190.5f : 67, 44), pb = W(s ? 313 : 189.5f, 166);
        ImU32 out = IM_COL32(246, 236, 210, 255), in = IM_COL32(226, 208, 170, 255);
        if (s) dl->AddRectFilledMultiColor(pa, pb, in, out, out, in);
        else dl->AddRectFilledMultiColor(pa, pb, out, in, in, out);
        float gw = (W(10, 0).x - W(0, 0).x);
        if (s) dl->AddRectFilledMultiColor(pa, ImVec2(pa.x + gw, pb.y), IM_COL32(90, 60, 30, 110), 0, 0, IM_COL32(90, 60, 30, 110));
        else dl->AddRectFilledMultiColor(ImVec2(pb.x - gw, pa.y), pb, 0, IM_COL32(90, 60, 30, 110), IM_COL32(90, 60, 30, 110), 0);
        /* a ruled margin, as a scribe's page (faint) */
        float mx = s ? 309.5f : 70.5f;
        dl->AddLine(W(mx, 47), W(mx, 163), IM_COL32(170, 60, 40, 46), 1.0f);
    }
    dl->AddLine(W(190, 44), W(190, 166), IM_COL32(70, 44, 20, 200), 1.5f);   /* the gutter */
    /* the ribbon, out of the gutter and over the table */
    ImVec2 r0 = W(186, 160), r1 = W(190, 160), r2 = W(191.5f, 192), r3 = W(184.5f, 192), rv = W(188, 188);
    ImVec2 rib[5] = { r0, r1, r2, rv, r3 };
    dl->AddRectFilled(W(187, 166), W(193, 195), IM_COL32(0, 0, 0, 70));
    dl->AddConvexPolyFilled(rib, 3, IM_COL32(170, 24, 20, 255));
    ImVec2 rq[4] = { r0, r1, r2, r3 };
    dl->AddConvexPolyFilled(rq, 4, IM_COL32(170, 24, 20, 255));
    ImVec2 notch[3] = { r3, r2, rv };
    dl->AddTriangleFilled(notch[0], notch[1], notch[2], IM_COL32(70, 42, 22, 255));
    dl->AddLine(W(187, 166), W(186, 191), IM_COL32(230, 90, 70, 200), 1.0f);
}

void mortar(ImDrawList *dl, float u)
{
    ImVec2 c = W(297, 21);
    glow(dl, ImVec2(c.x + 4 * u, c.y + 6 * u), ImVec2(30 * u, 30 * u), IM_COL32(0, 0, 0, 160), 12);
    const ImU32 br_d = IM_COL32(110, 70, 20, 255), br = IM_COL32(184, 134, 52, 255), br_l = IM_COL32(250, 214, 130, 255);
    lit_band(dl, c, ImVec2(25 * u, 25 * u), ImVec2(19 * u, 19 * u), br_d, br, br_l);
    /* the hollow: dark brass going down, the powder at its bottom */
    dl->AddCircleFilled(c, 19 * u, IM_COL32(70, 44, 14, 255), 48);
    glow(dl, ImVec2(c.x - 4 * u, c.y - 4 * u), ImVec2(17 * u, 17 * u), IM_COL32(200, 150, 70, 160), 10);
    dl->AddCircleFilled(ImVec2(c.x + 2 * u, c.y + 2 * u), 9 * u, IM_COL32(120, 140, 70, 255), 32);       /* green herbs ground */
    for (int i = 0; i < 18; i++) {
        float a = 6.28f * hash(i * 3), r = 8 * u * sqrtf(hash(i * 7 + 1));
        dl->AddCircleFilled(ImVec2(c.x + 2 * u + cosf(a) * r, c.y + 2 * u + sinf(a) * r), 0.9f * u, hash(i) > 0.5f ? IM_COL32(160, 176, 90, 255) : IM_COL32(80, 100, 44, 255), 6);
    }
    dl->AddCircle(c, 19 * u, IM_COL32(40, 22, 4, 255), 48, 1.2f);
    /* the pestle across it, wood with a worn brass end */
    ImVec2 p0(c.x - 2 * u, c.y + 3 * u), p1(c.x - 34 * u, c.y + 26 * u);
    dl->AddLine(ImVec2(p0.x + 2 * u, p0.y + 3 * u), ImVec2(p1.x + 2 * u, p1.y + 3 * u), IM_COL32(0, 0, 0, 90), 6.5f * u);
    dl->AddLine(p0, p1, IM_COL32(120, 76, 40, 255), 5.5f * u);
    dl->AddLine(ImVec2(p0.x - 0.8f * u, p0.y - 1.2f * u), ImVec2(p1.x - 0.8f * u, p1.y - 1.2f * u), IM_COL32(176, 128, 80, 255), 1.6f * u);
    dl->AddCircleFilled(p1, 3.4f * u, IM_COL32(110, 70, 36, 255), 16);
    dl->AddCircleFilled(p0, 3.6f * u, IM_COL32(184, 134, 52, 255), 16);
    dl->AddCircleFilled(ImVec2(p0.x - 1 * u, p0.y - 1 * u), 1.4f * u, IM_COL32(250, 214, 130, 255), 10);
}

void candle(ImDrawList *dl, float u, float t)
{
    /* a tallow candle standing in a pewter dish, seen from the front and a little above */
    ImVec2 c = W(84, 191);
    float fl = 0.85f + 0.15f * sinf(t * 9.0f) * sinf(t * 3.7f + 1.0f), sw = 0.6f * sinf(t * 5.3f);
    glow(dl, ImVec2(c.x, c.y - 22 * u), ImVec2(80 * u * fl, 70 * u * fl), IM_COL32(255, 190, 90, 80), 16);   /* its light on the table */
    glow(dl, ImVec2(c.x + 6 * u, c.y + 3 * u), ImVec2(16 * u, 6 * u), IM_COL32(0, 0, 0, 160), 8);
    lit_band(dl, c, ImVec2(13 * u, 4.6f * u), ImVec2(9 * u, 3.0f * u), IM_COL32(70, 74, 80, 255), IM_COL32(140, 144, 150, 255), IM_COL32(220, 224, 228, 255));
    dl->AddEllipseFilled(c, ImVec2(9 * u, 3.0f * u), IM_COL32(96, 100, 106, 255), 0, 32);
    lit_band(dl, ImVec2(c.x + 14 * u, c.y), ImVec2(3.4f * u, 2.0f * u), ImVec2(2.0f * u, 1.0f * u), IM_COL32(70, 74, 80, 255), IM_COL32(140, 144, 150, 255), IM_COL32(220, 224, 228, 255));
    float top = c.y - 20 * u, hw = 3.2f * u;
    dl->AddRectFilledMultiColor(ImVec2(c.x - hw, top), ImVec2(c.x + hw, c.y), IM_COL32(250, 240, 210, 255), IM_COL32(206, 190, 150, 255), IM_COL32(206, 190, 150, 255), IM_COL32(250, 240, 210, 255));
    dl->AddEllipseFilled(ImVec2(c.x, c.y), ImVec2(hw, 1.1f * u), IM_COL32(206, 190, 150, 255), 0, 16);
    dl->AddEllipseFilled(ImVec2(c.x, top), ImVec2(hw, 1.1f * u), IM_COL32(240, 220, 170, 255), 0, 16);
    dl->AddRectFilled(ImVec2(c.x - hw - 0.4f * u, top + 1 * u), ImVec2(c.x - hw + 1.0f * u, top + 7 * u), IM_COL32(246, 236, 206, 255), 0.8f * u);   /* a drip */
    dl->AddLine(ImVec2(c.x, top), ImVec2(c.x + 0.3f * u, top - 1.6f * u), IM_COL32(30, 20, 10, 255), 0.8f * u);   /* the wick */
    /* the flame */
    ImVec2 fb(c.x + 0.2f * u, top - 2.2f * u), ft(c.x + sw * 1.5f * u, top - 14.0f * u * fl);
    glow(dl, ImVec2(fb.x, fb.y - 3 * u), ImVec2(12 * u * fl, 14 * u * fl), IM_COL32(255, 220, 130, 200), 10);
    dl->PathClear();
    for (int i = 0; i <= 20; i++) {
        float a = i / 20.0f * 2 * PI, x = sinf(a), y = cosf(a);
        float r = 2.8f * u;
        ImVec2 p = y >= 0 ? ImVec2(fb.x + x * r, fb.y + y * r * 0.6f) : ImVec2(fb.x + (ft.x - fb.x) * (-y) + x * r * (1 + y), fb.y + (ft.y - fb.y) * (-y));
        dl->PathLineTo(p);
    }
    dl->PathFillConvex(IM_COL32(255, 196, 80, 240));
    dl->AddEllipseFilled(ImVec2(fb.x + sw * 0.3f * u, fb.y - 3.0f * u), ImVec2(1.5f * u, 3.6f * u), IM_COL32(255, 250, 220, 255), 0, 12);
    dl->AddEllipseFilled(ImVec2(fb.x, fb.y + 0.2f * u), ImVec2(1.2f * u, 1.0f * u), IM_COL32(90, 120, 220, 200), 0, 10);   /* (blue at its root) */
}

void flask(ImDrawList *dl, float u)
{
    ImVec2 c = W(122, 186);
    glow(dl, ImVec2(c.x + 3 * u, c.y + 5 * u), ImVec2(13 * u, 9 * u), IM_COL32(0, 0, 0, 130), 8);
    /* lying on its side: the round body, the neck to the right, a cork */
    ImVec2 n0(c.x + 7 * u, c.y - 2.5f * u), n1(c.x + 19 * u, c.y + 1 * u);
    dl->AddQuadFilled(ImVec2(c.x + 6 * u, c.y - 5 * u), ImVec2(n1.x, n1.y - 3.2f * u), ImVec2(n1.x, n1.y + 2.6f * u), ImVec2(c.x + 6 * u, c.y + 4 * u), IM_COL32(150, 200, 190, 150));
    dl->AddRectFilled(ImVec2(n1.x - 0.5f * u, n1.y - 3.0f * u), ImVec2(n1.x + 3.5f * u, n1.y + 2.6f * u), IM_COL32(150, 104, 60, 255), 1 * u);
    dl->AddCircleFilled(c, 9 * u, IM_COL32(120, 180, 170, 140), 40);
    dl->AddCircleFilled(ImVec2(c.x + 1 * u, c.y + 3 * u), 6.5f * u, IM_COL32(150, 40, 40, 200), 32);   /* a red tincture */
    dl->AddCircle(c, 9 * u, IM_COL32(220, 250, 240, 170), 40, 1.0f);
    dl->PathArcTo(c, 6.5f * u, PI * 1.05f, PI * 1.55f, 12);
    dl->PathStroke(IM_COL32(255, 255, 255, 220), 0, 1.6f * u);
    dl->AddCircleFilled(ImVec2(c.x - 3 * u, c.y - 4.5f * u), 1.2f * u, IM_COL32(255, 255, 255, 230), 8);
    (void)n0;
}

void inkwell(ImDrawList *dl, float u)
{
    ImVec2 c = W(152, 189);
    glow(dl, ImVec2(c.x + 4 * u, c.y + 3 * u), ImVec2(12 * u, 6 * u), IM_COL32(0, 0, 0, 150), 8);
    /* a horn inkwell: a squat dark pot, its mouth with ink in it */
    dl->AddRectFilledMultiColor(ImVec2(c.x - 7 * u, c.y - 8 * u), ImVec2(c.x + 7 * u, c.y), IM_COL32(90, 70, 50, 255), IM_COL32(40, 30, 20, 255), IM_COL32(30, 22, 14, 255), IM_COL32(80, 60, 42, 255));
    dl->AddEllipseFilled(c, ImVec2(7 * u, 2.4f * u), IM_COL32(36, 26, 16, 255), 0, 24);
    lit_band(dl, ImVec2(c.x, c.y - 8 * u), ImVec2(7 * u, 2.4f * u), ImVec2(4 * u, 1.4f * u), IM_COL32(40, 28, 16, 255), IM_COL32(110, 84, 58, 255), IM_COL32(170, 140, 100, 255));
    dl->AddEllipseFilled(ImVec2(c.x, c.y - 8 * u), ImVec2(4 * u, 1.4f * u), IM_COL32(8, 8, 14, 255), 0, 16);
    /* the quill standing in it, leaning back over the table */
    ImVec2 q0(c.x + 0.5f * u, c.y - 8 * u), q1(c.x + 20 * u, c.y - 34 * u);
    dl->AddLine(ImVec2(q0.x + 3 * u, q0.y + 2 * u), ImVec2(q1.x + 5 * u, q1.y + 6 * u), IM_COL32(0, 0, 0, 28), 2.5f * u);
    float dx = q1.x - q0.x, dy = q1.y - q0.y, L = sqrtf(dx * dx + dy * dy), nx = -dy / L, ny = dx / L;
    for (int i = 0; i < 16; i++) {                                  /* the vane: barbs along the upper part of the shaft */
        float f = 0.35f + 0.65f * i / 15.0f, w = (i < 12 ? 4.5f : 4.5f * (16 - i) / 4.0f) * u;
        ImVec2 p(q0.x + dx * f, q0.y + dy * f);
        dl->AddLine(p, ImVec2(p.x + nx * w - dx / L * 2.5f * u, p.y + ny * w - dy / L * 2.5f * u), IM_COL32(236, 230, 214, 230), 1.4f * u);
        dl->AddLine(p, ImVec2(p.x - nx * w * 0.5f - dx / L * 2 * u, p.y - ny * w * 0.5f - dy / L * 2 * u), IM_COL32(206, 198, 180, 220), 1.2f * u);
    }
    dl->AddLine(q0, q1, IM_COL32(220, 206, 170, 255), 1.1f * u);
}

void plate(ImDrawList *dl, float u)
{
    /* a brass plate under the Start button (216..296, 174..186) */
    ImVec2 a = W(212, 171.5f), b = W(300, 188.5f);
    dl->AddRectFilled(ImVec2(a.x + 2 * u, a.y + 3 * u), ImVec2(b.x + 2 * u, b.y + 3 * u), IM_COL32(0, 0, 0, 110), 2 * u);
    dl->AddRectFilledMultiColor(a, b, IM_COL32(236, 196, 112, 255), IM_COL32(200, 154, 70, 255), IM_COL32(140, 96, 34, 255), IM_COL32(176, 128, 52, 255));
    dl->AddRect(a, b, IM_COL32(70, 40, 8, 255), 2 * u, 0, 1.2f);
    dl->AddRect(ImVec2(a.x + 1.5f * u, a.y + 1.5f * u), ImVec2(b.x - 1.5f * u, b.y - 1.5f * u), IM_COL32(255, 236, 170, 120), 1 * u, 0, 1.0f);
    for (int c = 0; c < 4; c++) orn_rivet(dl, ImVec2(c & 1 ? b.x - 3.0f * u : a.x + 3.0f * u, c & 2 ? b.y - 3.0f * u : a.y + 3.0f * u), 1.5f * u);
}
} // namespace

static void desk(ImDrawList *dl, float t)
{
    ImVec2 o = W(60, 0), e = W(320, 200);
    float k = (e.y - o.y) / 200.0f, kx = (e.x - o.x) / 260.0f, u = fminf(k, kx);
    dl->PushClipRect(o, e, true);
    table(dl);
    book(dl, k);
    flask(dl, u);
    inkwell(dl, u);
    candle(dl, u, t);
    mortar(dl, u);
    plate(dl, u);
    dl->PopClipRect();
}
extern "C" void alchemy_desk_draw(float t) { in_box = false; desk(ImGui::GetBackgroundDrawList(), t); }
/* the settings' picture: the desk in a..b */
void alchemy_desk_preview(ImDrawList *dl, ImVec2 a, ImVec2 b, float t) { in_box = true; box_a = a; box_b = b; desk(dl, t); in_box = false; }
