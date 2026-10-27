/* alchemydesk.cpp - the alchemy screen's workbench (ui/alchemyview.cpp writes on it; where each
 * thing lies: ui/alchemydesk.h): an oak table; the formulary open on it, bound in red leather with
 * gilt tooling and brass corners, its ribbon out of the gutter; an apothecary's chest of walnut,
 * open, 8 x 3 compartments for the ingredients, a label inside its lid; a wax tablet in a wooden
 * frame with its stylus, for the reckoning; a small brick furnace with fire in its mouth and a
 * brass plate for Start. In game coordinates (x 60..320, y 0..200). */
#include "imgui.h"
#include <math.h>
#include "ornament.h"
#include "alchemydesk.h"
#include "drawcache.h"

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

void book_at(ImDrawList *dl, float k, float X0, float Y0, float X1, float Y1, float G, ImU32 leather)
{
    /* its shadow, the leather, the gilt line tooled round it */
    dl->AddRectFilled(W(X0 + 3, Y0 + 5), W(X1 + 3, Y1 + 4), IM_COL32(0, 0, 0, 90), 6 * k);
    ImVec2 ca = W(X0, Y0), cb = W(X1, Y1);
    {   ImU32 l0 = leather, l1 = mixc(leather, IM_COL32(0, 0, 0, 255), 0.13f), l2 = mixc(leather, IM_COL32(0, 0, 0, 255), 0.38f), l3 = mixc(leather, IM_COL32(0, 0, 0, 255), 0.26f);
        dl->AddRectFilledMultiColor(ca, cb, l0, l1, l2, l3); }
    dl->AddRect(ca, cb, IM_COL32(40, 6, 4, 255), 2 * k, 0, 1.5f);
    dl->AddRect(W(X0 + 1.2f, Y0 + 1.2f), W(X1 - 1.2f, Y1 - 1.2f), IM_COL32(222, 178, 84, 170), 0, 0, 1.0f);
    for (int c = 0; c < 4; c++) {                                  /* brass corners */
        float x = c & 1 ? X1 : X0, y = c & 2 ? Y1 : Y0, sx = c & 1 ? -1.0f : 1.0f, sy = c & 2 ? -1.0f : 1.0f;
        ImVec2 tri[3] = { W(x, y), W(x + sx * 7, y), W(x, y + sy * 10) };
        dl->AddTriangleFilled(tri[0], tri[1], tri[2], IM_COL32(196, 150, 70, 255));
        dl->AddLine(tri[1], tri[2], IM_COL32(250, 214, 130, 255), 1.0f);
    }
    /* the pages: the block of leaves at each side, then the open leaves darkening into the gutter */
    for (int s = 0; s < 2; s++) {
        float x0 = s ? G : X0 + 3, x1 = s ? X1 - 3 : G;
        for (int i = 0; i < 3; i++) {                              /* the edges of the leaves under */
            float o = 2.2f - i * 0.7f;
            ImVec2 pa = W(s ? x0 : x0 + o * 0.3f, Y0 + 3 + o * 0.2f), pb = W(s ? x1 - o * 0.3f : x1, Y1 - 4 + o);
            dl->AddRectFilled(pa, pb, i & 1 ? IM_COL32(214, 196, 160, 255) : IM_COL32(236, 222, 190, 255));
        }
        ImVec2 pa = W(s ? G + 0.5f : X0 + 4, Y0 + 3), pb = W(s ? X1 - 3 : G - 0.5f, Y1 - 5);
        ImU32 out = IM_COL32(246, 236, 210, 255), in = IM_COL32(226, 208, 170, 255);
        if (s) dl->AddRectFilledMultiColor(pa, pb, in, out, out, in);
        else dl->AddRectFilledMultiColor(pa, pb, out, in, in, out);
        float gw = (W(10, 0).x - W(0, 0).x);
        if (s) dl->AddRectFilledMultiColor(pa, ImVec2(pa.x + gw, pb.y), IM_COL32(90, 60, 30, 110), 0, 0, IM_COL32(90, 60, 30, 110));
        else dl->AddRectFilledMultiColor(ImVec2(pb.x - gw, pa.y), pb, 0, IM_COL32(90, 60, 30, 110), IM_COL32(90, 60, 30, 110), 0);
    }
    dl->AddLine(W(G, Y0 + 3), W(G, Y1 - 5), IM_COL32(70, 44, 20, 200), 1.5f);   /* the gutter */
    /* the ribbon, out of the gutter onto the table */
    ImVec2 r0 = W(G - 2, Y1 - 6), r1 = W(G + 2, Y1 - 6), r2 = W(G + 3.5f, Y1 + 6), r3 = W(G - 3.5f, Y1 + 6), rv = W(G, Y1 + 3);
    ImVec2 rq[4] = { r0, r1, r2, r3 };
    dl->AddConvexPolyFilled(rq, 4, IM_COL32(170, 24, 20, 255));
    ImVec2 notch[3] = { r3, r2, rv };
    dl->AddTriangleFilled(notch[0], notch[1], notch[2], IM_COL32(70, 42, 22, 255));
}

void book(ImDrawList *dl, float k) { book_at(dl, k, AD_BOOK_X0, AD_BOOK_Y0, AD_BOOK_X1, AD_BOOK_Y1, AD_GUTTER, IM_COL32(136, 36, 26, 255)); }

/* an apothecary's chest, open: its lid behind, inside it a label; the box of compartments */
void chest(ImDrawList *dl, float k, float u)
{
    const float X0 = AD_CHEST_X0, X1 = AD_CHEST_X1, LY = AD_CHEST_LID, Y0 = AD_CHEST_Y0, Y1 = AD_CHEST_Y1;
    dl->AddRectFilled(W(X0 + 3, LY + 4), W(X1 + 3, Y1 + 4), IM_COL32(0, 0, 0, 100), 3 * k);
    /* the lid, standing open behind the box: its inside with a paper label */
    dl->AddRectFilledMultiColor(W(X0 + 2, LY), W(X1 - 2, Y0), IM_COL32(120, 70, 36, 255), IM_COL32(120, 70, 36, 255), IM_COL32(70, 38, 18, 255), IM_COL32(70, 38, 18, 255));
    dl->AddRect(W(X0 + 2, LY), W(X1 - 2, Y0), IM_COL32(30, 14, 4, 255), 1 * k, 0, 1.2f);
    /* the box: thick walnut walls lit at the upper left, the floor of each compartment dark */
    ImVec2 a = W(X0, Y0), b = W(X1, Y1);
    dl->AddRectFilledMultiColor(a, b, IM_COL32(150, 94, 50, 255), IM_COL32(128, 78, 40, 255), IM_COL32(92, 54, 26, 255), IM_COL32(112, 66, 32, 255));
    dl->AddRect(a, b, IM_COL32(30, 14, 4, 255), 1.5f * k, 0, 1.5f);
    const float w = 2.4f, cx0 = X0 + w, cx1 = X1 - w, cy0 = Y0 + w, cy1 = Y1 - w, cw = (cx1 - cx0) / AD_CHEST_COLS, ch = (cy1 - cy0) / AD_CHEST_ROWS;
    for (int r = 0; r < AD_CHEST_ROWS; r++) for (int c = 0; c < AD_CHEST_COLS; c++) {
        float x0 = cx0 + c * cw + 0.7f, y0 = cy0 + r * ch + 0.7f, x1 = x0 + cw - 1.4f, y1 = y0 + ch - 1.4f;
        dl->AddRectFilledMultiColor(W(x0, y0), W(x1, y1), IM_COL32(40, 22, 10, 255), IM_COL32(52, 30, 14, 255), IM_COL32(70, 42, 20, 255), IM_COL32(58, 34, 16, 255));
        dl->AddLine(W(x0, y0), W(x1, y0), IM_COL32(10, 4, 0, 200), 1.2f);                 /* (the wall's shadow inside) */
        dl->AddLine(W(x0, y1), W(x1, y1), IM_COL32(200, 150, 96, 110), 1.0f);              /* (the lit lip of the divider) */
    }
    for (int c = 0; c < 4; c++) {                                  /* brass corner plates */
        float x = c & 1 ? X1 : X0, y = c & 2 ? Y1 : Y0;
        ImVec2 p = W(x + (c & 1 ? -1.2f : 1.2f), y + (c & 2 ? -1.2f : 1.2f));      /* (on the wall's corner, clear of the compartments) */
        dl->AddRectFilled(ImVec2(p.x - 1.6f * u, p.y - 1.6f * u), ImVec2(p.x + 1.6f * u, p.y + 1.6f * u), IM_COL32(196, 150, 70, 255), 0.4f * u);
        orn_rivet(dl, p, 0.7f * u);
    }
}

/* a wax tablet in its wooden frame: dark wax to write the reckoning in */
void tablet(ImDrawList *dl, float k)
{
    const float X0 = AD_TAB_X0, X1 = AD_TAB_X1, Y0 = AD_TAB_Y0, Y1 = AD_TAB_Y1;
    dl->AddRectFilled(W(X0 + 2, Y0 + 3), W(X1 + 2, Y1 + 2), IM_COL32(0, 0, 0, 100), 2 * k);
    dl->AddRectFilledMultiColor(W(X0, Y0), W(X1, Y1), IM_COL32(170, 118, 66, 255), IM_COL32(150, 100, 56, 255), IM_COL32(110, 70, 36, 255), IM_COL32(130, 86, 46, 255));
    dl->AddRect(W(X0, Y0), W(X1, Y1), IM_COL32(40, 20, 6, 255), 1.5f * k, 0, 1.2f);
    ImVec2 wa = W(X0 + 3, Y0 + 2.5f), wb = W(X1 - 3, Y1 - 2.5f);
    dl->AddRectFilledMultiColor(wa, wb, IM_COL32(52, 44, 30, 255), IM_COL32(46, 40, 28, 255), IM_COL32(34, 30, 20, 255), IM_COL32(40, 34, 24, 255));
    dl->AddRect(wa, wb, IM_COL32(20, 12, 4, 220), 0, 0, 1.0f);
    for (int i = 0; i < 6; i++) {                                  /* old strokes smoothed over, faint in the wax */
        float y = Y0 + 6 + 4.5f * i + hash(i) * 2, x = X0 + 8 + 60 * hash(i * 3);
        dl->AddLine(W(x, y), W(x + 20 + 30 * hash(i * 7), y + 0.6f), IM_COL32(90, 80, 60, 50), 1.0f);
    }
    /* the stylus lying across its corner */
    ImVec2 s0 = W(X0 + 6, Y1 + 1.6f), s1 = W(X0 + 40, Y1 + 0.4f);   /* (along the frame's lower edge, off the wax) */
    dl->AddLine(s0, s1, IM_COL32(0, 0, 0, 70), 2.6f * k * 0.5f + 2);
    dl->AddLine(s0, s1, IM_COL32(210, 196, 160, 255), 1.6f);
}

/* a small brick furnace (an athanor), fire in its mouth, a brass door plate for the button */
void furnace(ImDrawList *dl, float k, float u, float t)
{
    const float X0 = AD_FURN_X0, X1 = AD_FURN_X1, Y0 = AD_FURN_Y0, Y1 = AD_FURN_Y1;
    dl->AddRectFilled(W(X0 + 2, Y0 + 3), W(X1 + 2, Y1 + 2), IM_COL32(0, 0, 0, 110), 2 * k);
    dl->AddRectFilled(W(X0, Y0), W(X1, Y1), IM_COL32(120, 52, 34, 255), 1.5f * k);
    for (int r = 0; Y0 + r * 4 < Y1; r++) {                        /* courses of brick */
        float y = Y0 + r * 4;
        dl->AddLine(W(X0, y), W(X1, y), IM_COL32(60, 30, 20, 200), 1.0f);
        for (float x = X0 + (r & 1 ? 4 : 0); x < X1; x += 8) dl->AddLine(W(x, y), W(x, fminf(y + 4, Y1)), IM_COL32(60, 30, 20, 200), 1.0f);
        for (float x = X0 + (r & 1 ? 4 : 0); x + 7 < X1; x += 8) if (hash(r * 31 + (int)x) > 0.7f) dl->AddRectFilled(W(x + 0.6f, y + 0.6f), W(x + 7.4f, fminf(y + 3.4f, Y1)), IM_COL32(150, 70, 44, 120));
    }
    dl->AddRect(W(X0, Y0), W(X1, Y1), IM_COL32(40, 16, 8, 255), 1.5f * k, 0, 1.4f);
    /* the fire's mouth at the left, flickering */
    float fl = 0.8f + 0.2f * sinf(t * 7.3f) * sinf(t * 2.9f + 0.7f);
    ImVec2 mc = W(X0 + 15, Y1 - 8);
    glow(dl, mc, ImVec2(26 * u * fl, 20 * u * fl), IM_COL32(255, 150, 40, 200), 10);
    dl->AddRectFilled(W(X0 + 5, Y1 - 20), W(X0 + 25, Y1 - 1), IM_COL32(30, 10, 4, 255), 9 * u, ImDrawFlags_RoundCornersTop);
    glow(dl, mc, ImVec2(11 * u * fl, 10 * u * fl), IM_COL32(255, 200, 80, 255), 8);
    for (int i = 0; i < 3; i++) {                                  /* flames licking up */
        float fx = X0 + 9 + i * 5, h = (7 + 3 * sinf(t * (5 + i) + i * 2.1f)) * fl;
        ImVec2 b0 = W(fx - 2.4f, Y1 - 3), b1 = W(fx + 2.4f, Y1 - 3), tp = W(fx + 0.8f * sinf(t * 6 + i), Y1 - 3 - h);
        dl->AddTriangleFilled(b0, b1, tp, IM_COL32(255, 170, 60, 230));
        dl->AddTriangleFilled(W(fx - 1.1f, Y1 - 3), W(fx + 1.1f, Y1 - 3), W(fx, Y1 - 3 - h * 0.55f), IM_COL32(255, 240, 170, 240));
    }
    for (int i = 0; i < 4; i++) {                                  /* the coals */
        ImVec2 c = W(X0 + 8 + i * 4.5f, Y1 - 2.5f);
        dl->AddCircleFilled(c, 1.3f * u, mixc(IM_COL32(200, 60, 10, 255), IM_COL32(255, 220, 100, 255), 0.5f + 0.5f * sinf(t * 4 + i * 1.7f)), 8);
    }
    /* the brass plate the button sits on */
    ImVec2 pa = W(AD_START_X0 - 3, AD_START_Y0 - 3), pb = W(AD_START_X1 + 3, AD_START_Y1 + 3);
    dl->AddRectFilledMultiColor(pa, pb, IM_COL32(236, 196, 112, 255), IM_COL32(200, 154, 70, 255), IM_COL32(140, 96, 34, 255), IM_COL32(176, 128, 52, 255));
    dl->AddRect(pa, pb, IM_COL32(70, 40, 8, 255), 1.5f * u, 0, 1.2f);
    for (int c = 0; c < 4; c++) orn_rivet(dl, ImVec2(c & 1 ? pb.x - 2.5f * u : pa.x + 2.5f * u, c & 2 ? pb.y - 2.5f * u : pa.y + 2.5f * u), 1.2f * u);
}





} // namespace

static void desk(ImDrawList *dl, float t)
{
    ImVec2 o = W(60, 0), e = W(320, 200);
    float k = (e.y - o.y) / 200.0f, kx = (e.x - o.x) / 260.0f, u = fminf(k, kx);
    dl->PushClipRect(o, e, true);
    auto still = [&](ImDrawList *d) { table(d); book(d, k); chest(d, k, u); tablet(d, k); };
    if (in_box) still(dl);
    else dc_draw(dl, o, e, 0xa1c4e31ull, still);              /* (all but the fire: drawn once, kept: ui/drawcache.h) */
    furnace(dl, k, u, t);
    dl->PopClipRect();
}
/* the same oak table and a book on it, for other screens (the inn's ledger): game coordinates */
void desk_table(ImDrawList *dl) { in_box = false; table(dl); }
void desk_book(ImDrawList *dl, float x0, float y0, float x1, float y1, float gutter, ImU32 leather)
{
    in_box = false; ImVec2 o = W(60, 0), e = W(320, 200);
    book_at(dl, (e.y - o.y) / 200.0f, x0, y0, x1, y1, gutter, leather);
}
extern "C" void alchemy_desk_draw(float t) { in_box = false; desk(ImGui::GetBackgroundDrawList(), t); }
/* the settings' picture: the desk in a..b */
void alchemy_desk_preview(ImDrawList *dl, ImVec2 a, ImVec2 b, float t) { in_box = true; box_a = a; box_b = b; desk(dl, t); in_box = false; }
