/* niche.cpp - the character sheet's stone niche drawn anew (with the period ornaments): the
 * game's ARMSBACK.PIC (136 x 170) at the window's resolution. A round arch on two fluted
 * pilasters with square capitals, carved leaves in the spandrels, the niche within (its left
 * wall in shadow, the back wall lit), a gravel floor, three steps, and granite round it all.
 * The paper doll's armour and weapons are drawn over it (gamepic_layers_off_back). */
#include <math.h>
#include "imgui.h"

static inline unsigned hash32(unsigned x) { x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x; }

void ui_draw_doll_niche(ImDrawList *dl, ImVec2 o, float kx, float k)
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
    /* the niche: the back wall with a round top, the side walls between it and the arch */
    const ImVec2 C(68, 40);                               /* the arch's centre */
    const float RI = 51, RO = 57;                         /* its inner edge, its moulding's outer edge */
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
    /* the arch's moulding: a band round the opening, lit on its inner edge */
    {
        const int N = 48;
        for (int ring = 0; ring < 3; ring++) {
            float r = RI + ring * (RO - RI) / 2.0f;
            dl->PathClear();
            for (int i = 0; i <= N; i++) { float t = PI + PI * i / N; dl->PathLineTo(P(C.x + cosf(t) * r, C.y + sinf(t) * r)); }
            dl->PathStroke(ring == 0 ? G(232) : ring == 1 ? G(196) : G(104), 0, (ring == 1 ? 3.0f : 1.0f) * k);
        }
    }
    /* the spandrels: carved leaves, curling out of a rosette */
    for (int side = -1; side <= 1; side += 2) {
        float cx = side < 0 ? 22 : 114, cy = 14;
        for (int l = 0; l < 4; l++) {                        /* four leaves round the rosette */
            float t = l * PI / 2 + PI / 4 + (side > 0 ? PI / 2 : 0);
            ImVec2 p0 = P(cx, cy), p1 = P(cx + cosf(t) * 9, cy + sinf(t) * 7), p2 = P(cx + cosf(t + 0.9f) * 13, cy + sinf(t + 0.9f) * 10);
            ImVec2 p3 = P(cx + cosf(t + 1.2f) * 8, cy + sinf(t + 1.2f) * 6);
            dl->AddBezierCubic(p0, p1, p2, p3, G(96), 2.2f * k);
            dl->AddBezierCubic(ImVec2(p0.x - 0.4f * k, p0.y - 0.4f * k), ImVec2(p1.x - 0.4f * k, p1.y - 0.4f * k), ImVec2(p2.x - 0.4f * k, p2.y - 0.4f * k), ImVec2(p3.x - 0.4f * k, p3.y - 0.4f * k), G(236), 0.8f * k);
            dl->AddCircleFilled(p3, 1.3f * k, G(100), 10);
            dl->AddCircleFilled(ImVec2(p3.x - 0.3f * k, p3.y - 0.3f * k), 0.6f * k, G(230), 8);
        }
        for (int pt = 0; pt < 6; pt++) {                     /* the rosette */
            float t = pt * PI / 3;
            dl->AddCircleFilled(P(cx + cosf(t) * 2.4f, cy + sinf(t) * 2.0f), 1.6f * k, pt < 3 ? G(206) : G(132), 10);
        }
        dl->AddCircleFilled(P(cx, cy), 1.4f * k, G(90), 10);
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
