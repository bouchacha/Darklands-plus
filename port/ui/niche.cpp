/* niche.cpp - the character sheet's stone niche drawn anew (with the period ornaments): the
 * game's ARMSBACK.PIC (136 x 170) at the window's resolution. A round arch on two fluted
 * pilasters with square capitals, carved leaves in the spandrels, the niche within (its left
 * wall in shadow, the back wall lit), a gravel floor, three steps, and granite round it all.
 * The paper doll's armour and weapons are drawn over it (gamepic_layers_off_back). */
#include <math.h>
#include "imgui.h"
#include "../runtime/config.h"

static inline unsigned hash32(unsigned x) { x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x; }


/* the spandrels, the corners between the arch and the frame, carved in relief. Each is drawn on
   the left and mirrored; the arch drawn after covers whatever reaches inside it. Styles
   (display.niche_corners): 0 acanthus, a leaf curling out of the corner (the game's own picture
   has foliage there); 1 tracery, a cusped quatrefoil in a roundel, as the blind tracery of a
   church of about 1400; 2 a shield in a roundel, as in the spandrels of a tomb or a doorway;
   3 a rose of two rings of petals */
static void spandrels(ImDrawList *dl, ImVec2 o, float kx, float k, int style)
{
    const float PI = 3.14159265f;
    auto G = [](int v, int a = 255) { return IM_COL32(v, v, v, a); };
    for (int side = 0; side < 2; side++) {
        auto Q = [&](float x, float y) { return ImVec2(o.x + (side ? 136 - x : x) * kx, o.y + y * k); };
        float sh = 0.55f * k, lw = fmaxf(1.0f, 0.5f * k);
        /* the panel: smooth stone, a fillet round it */
        { ImVec2 q[4] = { Q(4, 0), Q(66, 0), Q(66, 40), Q(4, 40) };
          if (side) { ImVec2 t = q[0]; q[0] = q[1]; q[1] = t; t = q[2]; q[2] = q[3]; q[3] = t; }
          dl->AddConvexPolyFilled(q, 4, G(198)); }
        dl->AddLine(Q(4, 0.6f), Q(66, 0.6f), G(236), lw);
        dl->AddLine(Q(4.6f, 0), Q(4.6f, 36), side ? G(110) : G(236), lw);
        /* relief: shadow below right, light above left, the body */
        auto circ = [&](float x, float y, float r, int body, bool fill = true) {
            float sx = side ? -sh : sh;
            if (fill) {
                dl->AddCircleFilled(ImVec2(Q(x, y).x + sx, Q(x, y).y + sh), r * k, G(96), 24);
                dl->AddCircleFilled(ImVec2(Q(x, y).x - sx * 0.7f, Q(x, y).y - sh * 0.7f), r * k, G(240), 24);
                dl->AddCircleFilled(Q(x, y), r * k, G(body), 24);
            } else {
                dl->AddCircle(ImVec2(Q(x, y).x + sx, Q(x, y).y + sh), r * k, G(96), 32, 1.6f * k);
                dl->AddCircle(ImVec2(Q(x, y).x - sx * 0.7f, Q(x, y).y - sh * 0.7f), r * k, G(240), 32, 1.2f * k);
                dl->AddCircle(Q(x, y), r * k, G(body), 32, 1.4f * k);
            }
        };
        auto curve = [&](ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float w, int body) {
            float sx = side ? -sh : sh;
            auto off = [&](ImVec2 p, float dx, float dy) { return ImVec2(p.x + dx, p.y + dy); };
            dl->AddBezierCubic(off(p0, sx, sh), off(p1, sx, sh), off(p2, sx, sh), off(p3, sx, sh), G(92), w * k);
            dl->AddBezierCubic(off(p0, -sx * 0.6f, -sh * 0.6f), off(p1, -sx * 0.6f, -sh * 0.6f), off(p2, -sx * 0.6f, -sh * 0.6f), off(p3, -sx * 0.6f, -sh * 0.6f), G(240), w * k);
            dl->AddBezierCubic(p0, p1, p2, p3, G(body), w * k * 0.8f);
        };
        switch (style) {
        case 1: {                                                    /* tracery: a cusped quatrefoil in a roundel */
            float cx = 13.5f, cy = 10.5f, R = 7.2f;
            circ(cx, cy, R + 1.2f, 150, false);
            dl->AddCircleFilled(Q(cx, cy), R * k, G(150), 32);           /* the sunk field */
            for (int f = 0; f < 4; f++) {                                /* four lobes, raised */
                float t = f * PI / 2 + PI / 4;
                circ(cx + cosf(t) * R * 0.42f, cy + sinf(t) * R * 0.42f, R * 0.4f, 206);
            }
            dl->AddCircleFilled(Q(cx, cy), R * 0.28f * k, G(206), 16);
            for (int f = 0; f < 4; f++) {                                /* the cusps between, sunk */
                float t = f * PI / 2;
                dl->AddCircleFilled(Q(cx + cosf(t) * R * 0.62f, cy + sinf(t) * R * 0.62f), R * 0.12f * k, G(120), 10);
            }
            circ(28, 3.6f, 2.4f, 190);                                   /* small trefoils along the top and the side */
            for (int f = 0; f < 3; f++) { float t = -PI / 2 + f * 2 * PI / 3; dl->AddCircleFilled(Q(28 + cosf(t) * 1.1f, 3.6f + sinf(t) * 1.1f), 0.8f * k, G(140), 10); }
            circ(8.5f, 27, 2.4f, 190);
            for (int f = 0; f < 3; f++) { float t = -PI / 2 + f * 2 * PI / 3; dl->AddCircleFilled(Q(8.5f + cosf(t) * 1.1f, 27 + sinf(t) * 1.1f), 0.8f * k, G(140), 10); }
            break; }
        case 2: {                                                    /* a shield in a roundel */
            float cx = 13.5f, cy = 10.5f, R = 8.2f;
            circ(cx, cy, R, 176);
            circ(cx, cy, R - 1.0f, 196, false);
            ImVec2 a = Q(cx - 4.4f, cy - 4.8f), b = Q(cx + 4.4f, cy - 4.8f), c = Q(cx + 4.4f, cy + 0.8f), d = Q(cx, cy + 5.8f), e = Q(cx - 4.4f, cy + 0.8f);
            ImVec2 sh_[5] = { a, b, c, d, e };
            ImVec2 sdw[5]; for (int q = 0; q < 5; q++) sdw[q] = ImVec2(sh_[q].x + (side ? -sh : sh), sh_[q].y + sh);
            dl->AddConvexPolyFilled(sdw, 5, G(96));
            dl->AddConvexPolyFilled(sh_, 5, G(214));
            dl->AddPolyline(sh_, 5, G(120), ImDrawFlags_Closed, lw);
            dl->AddLine(Q(cx - 3.6f, cy - 4.0f), Q(cx + 3.6f, cy + 2.2f), G(140), 1.7f * k);   /* a bend across it */
            dl->AddLine(Q(cx - 3.9f, cy - 4.4f), Q(cx + 3.3f, cy + 1.8f), G(236), 0.5f * k);
            curve(Q(22, 3), Q(28, 1.5f), Q(32, 5), Q(28, 6.5f), 1.4f, 180);   /* curls of leaf off it */
            curve(Q(7, 20), Q(5.5f, 26), Q(9, 30), Q(10.5f, 27), 1.4f, 180);
            break; }
        case 3: {                                                    /* a rose: two rings of petals */
            float cx = 13.5f, cy = 10.5f, R = 7.4f;
            circ(cx, cy, R + 1.0f, 160, false);
            for (int f = 0; f < 5; f++) { float t = -PI / 2 + f * 2 * PI / 5; circ(cx + cosf(t) * R * 0.55f, cy + sinf(t) * R * 0.55f, R * 0.42f, 196); }
            for (int f = 0; f < 5; f++) { float t = -PI / 2 + PI / 5 + f * 2 * PI / 5; circ(cx + cosf(t) * R * 0.3f, cy + sinf(t) * R * 0.3f, R * 0.26f, 216); }
            circ(cx, cy, R * 0.18f, 150);
            for (int f = 0; f < 5; f++) { float t = -PI / 2 + PI / 5 + f * 2 * PI / 5; dl->AddLine(Q(cx + cosf(t) * R * 0.72f, cy + sinf(t) * R * 0.72f), Q(cx + cosf(t) * R * 0.98f, cy + sinf(t) * R * 0.98f), G(120), 0.9f * k); }   /* the sepals */
            break; }
        default: {                                                   /* acanthus, as the game's: broad leaves in relief fanning from a boss */
            /* the panel: sunk, a dark border round a mid-grey field */
            dl->AddRectFilled(Q(4, 0), Q(64, 42), G(150));
            dl->AddLine(Q(5, 1.2f), Q(64, 1.2f), G(92), 1.4f * k);
            dl->AddLine(Q(5.2f, 1), Q(5.2f, 38), side ? G(200) : G(92), 1.4f * k);
            /* a leaf: base to tip, w wide, its edge lobed; lit from the top left */
            auto leaf = [&](float bx, float by, float tx, float ty, float w, float curl) {
                const int N = 16;
                float dx = tx - bx, dy = ty - by, L = sqrtf(dx * dx + dy * dy), nx = -dy / L, ny = dx / L;
                ImVec2 lft[N + 1], rgt[N + 1], mid[N + 1];
                for (int q = 0; q <= N; q++) {
                    float u = (float)q / N, wd = w * sinf(PI * powf(u, 0.8f)) * (0.9f + 0.1f * cosf(u * 2 * PI * 3));
                    float bend = curl * u * u * L * 0.25f;
                    float cx = bx + dx * u + nx * bend, cy = by + dy * u + ny * bend;
                    mid[q] = Q(cx, cy); lft[q] = Q(cx + nx * wd, cy + ny * wd); rgt[q] = Q(cx - nx * wd, cy - ny * wd);
                }
                bool lit_left = (nx * (side ? 1 : -1) - ny) > 0;          /* the half facing the light */
                for (int q = 0; q < N; q++) {
                    ImVec2 a1[4] = { mid[q], lft[q], lft[q + 1], mid[q + 1] }, a2[4] = { mid[q], rgt[q], rgt[q + 1], mid[q + 1] };
                    dl->AddConvexPolyFilled(a1, 4, lit_left ? G(222) : G(178));
                    dl->AddConvexPolyFilled(a2, 4, lit_left ? G(178) : G(222));
                }
                dl->AddPolyline(lft, N + 1, G(96), 0, 0.7f * k);
                dl->AddPolyline(rgt, N + 1, G(96), 0, 0.7f * k);
                dl->AddPolyline(mid, N + 1, G(120), 0, 0.6f * k);           /* the midrib */
                for (int q = 3; q < N; q += 4) {                            /* the veins, to the lobes */
                    dl->AddLine(mid[q], ImVec2((mid[q].x + lft[q + 1].x) * 0.5f, (mid[q].y + lft[q + 1].y) * 0.5f), G(130), 0.5f * k);
                    dl->AddLine(mid[q], ImVec2((mid[q].x + rgt[q + 1].x) * 0.5f, (mid[q].y + rgt[q + 1].y) * 0.5f), G(130), 0.5f * k);
                }
            };
            const float bx = 15, by = 13;                                   /* the boss the leaves spring from */
            leaf(bx, by, 7, 3.5f, 5.0f, 0.3f);                              /* into the corner */
            leaf(bx, by, 7.5f, 24, 4.6f, -0.3f);                            /* down the side */
            leaf(bx, by, 26, 3.5f, 4.6f, -0.3f);                            /* along the top */
            leaf(bx, by, 6.5f, 13, 3.8f, 0.2f);                             /* out to the side, under the first */
            leaf(bx, by, 17, 3, 4.0f, 0.1f);                                /* up, between */
            circ(bx, by, 2.4f, 205);                                        /* the boss */
            circ(bx, by, 1.1f, 150);
        }
        }
    }
}

void ui_draw_doll_niche_style(ImDrawList *dl, ImVec2 o, float kx, float k, int style)
{
    const float PI = 3.14159265f;
    auto P = [&](float x, float y) { return ImVec2(o.x + x * kx, o.y + y * k); };
    auto G = [](int v, int a = 255) { return IM_COL32(v, v, v, a); };
    ImVec2 a = P(0, 0), b = P(136, 170);
    dl->PushClipRect(a, b, true);
    /* granite: a grey ground, flecked */
    dl->AddRectFilled(a, b, G(176));
    for (int i = 0; i < 900; i++) {
        unsigned h = hash32(i * 2654435761u + 17);
        float x = (h & 1023) / 1023.0f * 136, y = ((h >> 10) & 1023) / 1023.0f * 170;
        int v = 120 + (int)((h >> 20) % 120);
        float r = (0.35f + ((h >> 28) & 3) * 0.15f) * k;
        dl->AddRectFilled(P(x, y), ImVec2(P(x, y).x + r * 1.4f, P(x, y).y + r), G(v, 200));
    }
    spandrels(dl, o, kx, k, style);
    /* the niche: the back wall with a round top, the side walls between it and the arch */
    const ImVec2 C(68, 42);                               /* the arch's centre */
    const float RI = 50, RO = 55;                         /* its inner edge, its moulding's outer edge */
    {   /* inside the arch and down to the floor: the side walls first */
        const int N = 40;
        dl->PathClear();
        dl->PathLineTo(P(C.x - RI, 150));
        for (int i = 0; i <= N; i++) { float t = PI + PI * i / N; dl->PathLineTo(P(C.x + cosf(t) * RI, C.y + sinf(t) * RI)); }
        dl->PathLineTo(P(C.x + RI, 150));
        dl->PathFillConvex(G(112));
        dl->AddRectFilledMultiColor(P(C.x - RI, 30), P(32, 150), G(66), G(84), G(84), G(66));          /* the left wall, in shadow */
        dl->AddRectFilledMultiColor(P(97, 30), P(C.x + RI, 150), G(126), G(108), G(108), G(126));       /* the right wall */
        /* the back wall, its top round */
        dl->PathClear();
        dl->PathLineTo(P(32, 145));
        for (int i = 0; i <= N; i++) { float t = PI + PI * i / N; dl->PathLineTo(P(64.5f + cosf(t) * 32.5f, 33 + sinf(t) * 33)); }
        dl->PathLineTo(P(97, 145));
        dl->PathFillConvex(G(156));
        dl->AddRectFilledMultiColor(P(32, 30), P(97, 145), G(150, 0), G(150, 0), G(120, 90), G(120, 90));  /* (darker at its foot) */
        dl->PathClear();                                                       /* the back wall's edge */
        for (int i = 0; i <= N; i++) { float t = PI + PI * i / N; dl->PathLineTo(P(64.5f + cosf(t) * 32.5f, 33 + sinf(t) * 33)); }
        dl->PathStroke(G(96), 0, fmaxf(1.0f, 0.6f * k));
        dl->AddLine(P(32, 33), P(32, 145), G(96), fmaxf(1.0f, 0.6f * k));
        dl->AddLine(P(97, 33), P(97, 145), G(170), fmaxf(1.0f, 0.6f * k));
    }
    /* the floor: gravel between the walls, and its front edge */
    {
        ImVec2 q[4] = { P(32, 145), P(97, 145), P(117, 156), P(15, 156) };
        dl->AddConvexPolyFilled(q, 4, G(150));
        for (int i = 0; i < 260; i++) {
            unsigned h = hash32(i * 40503u + 991);
            float t = (h & 1023) / 1023.0f, u = ((h >> 10) & 1023) / 1023.0f;
            float y = 145 + u * 11, x0 = 32 - (y - 145) / 11 * 17, x1 = 97 + (y - 145) / 11 * 20;
            dl->AddRectFilled(P(x0 + t * (x1 - x0), y), P(x0 + t * (x1 - x0) + 0.8f, y + 0.6f), G(110 + (int)((h >> 20) % 110)));
        }
    }
    /* the arch's moulding, as the game's: a broad carved band, a fillet each side, rosettes along it */
    {
        const int N = 64;
        const float RM = RO + 3.5f;                                       /* (the band reaches into the corners) */
        for (int i = 0; i < N; i++) {                                     /* the band, shaded across the curve */
            float t0 = PI + PI * i / N, t1 = PI + PI * (i + 1) / N;
            ImVec2 q[4] = { P(C.x + cosf(t0) * RI, C.y + sinf(t0) * RI), P(C.x + cosf(t0) * RM, C.y + sinf(t0) * RM),
                            P(C.x + cosf(t1) * RM, C.y + sinf(t1) * RM), P(C.x + cosf(t1) * RI, C.y + sinf(t1) * RI) };
            float lit = 0.5f - 0.5f * cosf((t0 + t1) * 0.5f + 0.8f);
            dl->AddConvexPolyFilled(q, 4, G((int)(168 + 40 * lit)));
        }
        auto arc = [&](float r, ImU32 c, float w) {
            dl->PathClear();
            for (int i = 0; i <= N; i++) { float t = PI + PI * i / N; dl->PathLineTo(P(C.x + cosf(t) * r, C.y + sinf(t) * r)); }
            dl->PathStroke(c, 0, w * k);
        };
        arc(RI, G(240), 1.1f); arc(RI + 1.3f, G(110), 0.7f);
        arc(RM - 1.3f, G(240), 0.7f); arc(RM, G(96), 1.1f);
        for (int r = 0; r < 9; r++) {                                     /* the rosettes along it */
            float t = PI + PI * (r + 0.5f) / 9, rr = (RI + RM) * 0.5f;
            float cx = C.x + cosf(t) * rr, cy = C.y + sinf(t) * rr;
            for (int pt = 0; pt < 4; pt++) {
                float a = pt * PI / 2 + PI / 4;
                dl->AddCircleFilled(P(cx + cosf(a) * 1.5f, cy + sinf(a) * 1.5f), 1.25f * k, pt == 3 || pt == 2 ? G(120) : G(236), 10);
            }
            dl->AddCircleFilled(P(cx, cy), 1.6f * k, G(196), 12);
            dl->AddCircleFilled(P(cx, cy), 0.7f * k, G(104), 8);
        }
    }
    /* the pilasters: fluted shafts, square capitals with a rosette, a base */
    for (int side = 0; side < 2; side++) {
        float x0 = side ? 117 : 4, x1 = x0 + 11;
        dl->AddRectFilled(P(x0, 47), P(x1, 153), G(214));
        for (int f = 0; f < 4; f++) {                        /* the flutes, lit from the left */
            float fx = x0 + 1.6f + f * 2.5f;
            dl->AddRectFilled(P(fx, 49), P(fx + 1.3f, 151), G(150));
            dl->AddLine(P(fx, 49), P(fx, 151), G(110), fmaxf(1.0f, 0.4f * k));
            dl->AddLine(P(fx + 1.4f, 49), P(fx + 1.4f, 151), G(246), fmaxf(1.0f, 0.4f * k));
        }
        dl->AddRect(P(x0, 47), P(x1, 153), G(110), 0, 0, fmaxf(1.0f, 0.5f * k));
        /* the capital */
        dl->AddRectFilled(P(x0 - 1, 36), P(x1 + 1, 47), G(206));
        dl->AddRect(P(x0 - 1, 36), P(x1 + 1, 47), G(104), 0, 0, fmaxf(1.0f, 0.5f * k));
        dl->AddLine(P(x0 - 1, 36.5f), P(x1 + 1, 36.5f), G(244), fmaxf(1.0f, 0.5f * k));
        ImVec2 rc = P((x0 + x1) * 0.5f, 41.5f);
        for (int pt = 0; pt < 4; pt++) { float t = pt * PI / 2 + PI / 4; dl->AddCircleFilled(ImVec2(rc.x + cosf(t) * 2.0f * kx, rc.y + sinf(t) * 2.0f * k), 1.5f * k, pt < 2 ? G(150) : G(232), 10); }
        dl->AddCircleFilled(rc, 1.2f * k, G(96), 8);
        /* the base */
        dl->AddRectFilled(P(x0 - 1.5f, 152), P(x1 + 1.5f, 157), G(196));
        dl->AddLine(P(x0 - 1.5f, 152.3f), P(x1 + 1.5f, 152.3f), G(240), fmaxf(1.0f, 0.5f * k));
        dl->AddRect(P(x0 - 1.5f, 152), P(x1 + 1.5f, 157), G(104), 0, 0, fmaxf(1.0f, 0.5f * k));
    }
    /* the steps */
    for (int st = 0; st < 3; st++) {
        float y0 = 157 + st * 4.3f, inset = (2 - st) * 2.5f;
        dl->AddRectFilled(P(inset, y0), P(136 - inset, y0 + 4.3f), G(186 - st * 14));
        dl->AddLine(P(inset, y0 + 0.3f), P(136 - inset, y0 + 0.3f), G(238), fmaxf(1.0f, 0.5f * k));
        dl->AddLine(P(inset, y0 + 4.1f), P(136 - inset, y0 + 4.1f), G(92), fmaxf(1.0f, 0.5f * k));
    }
    dl->PopClipRect();
}

void ui_draw_doll_niche(ImDrawList *dl, ImVec2 o, float kx, float k)
{
    ui_draw_doll_niche_style(dl, o, kx, k, 0);
}

/* test: the four corners side by side (ucmd unichesheet:) */
static int niche_sheet;
extern "C" void niche_test_sheet(void) { niche_sheet = !niche_sheet; }
extern "C" void niche_test_sheet_draw(void)
{
    if (!niche_sheet) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float k = 2.6f;
    static const char *N[4] = { "Acanthus", "Tracery", "Shield", "Rose" };
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(4 * (136 * k + 12) + 12, 175 * k + 40), IM_COL32(40, 30, 22, 255));
    for (int st = 0; st < 4; st++) {
        ImVec2 o(12 + st * (136 * k + 12), 10);
        ui_draw_doll_niche_style(dl, o, k, k, st);
        dl->AddText(ImGui::GetFont(), 20, ImVec2(o.x, o.y + 170 * k + 6), IM_COL32(246, 236, 214, 255), N[st]);
    }
}
