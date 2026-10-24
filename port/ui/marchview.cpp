// marchview.cpp - Change Marching Order drawn by Darklands+ (mods/marchview.c runs it): a dialog
// with the members in their order, each with its figure and a handle to drag it to its place; Done
// keeps the new order, Cancel the old.
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include "nsdraw.h"
#include "ornament.h"
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/marchview.h"
}
const GamePic *ns_figure_pic(int cache, const char *code, const unsigned char *mp, const char *suffix);

extern "C" void marchview_draw(void)
{
    const MarchModel &m = march_model;
    if (!m.open || ns_screen != NS_MARCH || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    const ImU32 BROWN = IM_COL32(120, 84, 40, 235);
    static const char *NTH[5] = { "1st", "2nd", "3rd", "4th", "5th" };
    const float RH = 17, X0 = 120, X1 = 260;
    float h = 30 + m.n * RH + 16, Y0 = 100 - h * 0.5f, Y1 = Y0 + h;
    P.rect(60, 0, 320, 200, IM_COL32(20, 12, 6, 110));                 /* the game dimmed under it */
    P.dl->AddRectFilled(NsDraw::w(X0 + 2, Y0 + 3), NsDraw::w(X1 + 3, Y1 + 4), IM_COL32(0, 0, 0, 110), 3 * P.k);
    P.rect(X0, Y0, X1, Y1, IM_COL32(242, 230, 200, 255), 2);
    if (orn_on()) { orn_parchment_edges(P.dl, NsDraw::w(X0, Y0), NsDraw::w(X1, Y1), 5 * P.k); orn_gilt_frame(P.dl, NsDraw::w(X0, Y0), NsDraw::w(X1, Y1), 5 * P.k); }
    else P.frame(X0, Y0, X1, Y1, IM_COL32(120, 80, 36, 180), 2);
    {   /* the road the party walks in single file: a dusty track down through the rows, stones along it */
        float ry0 = Y0 + 24, ry1 = Y0 + 25 + m.n * RH - 2, rx0 = X0 + 17, rx1 = X0 + 33;
        P.dl->AddRectFilledMultiColor(NsDraw::w(rx0, ry0), NsDraw::w(rx1, ry1), IM_COL32(196, 166, 116, 255), IM_COL32(184, 152, 104, 255), IM_COL32(170, 138, 92, 255), IM_COL32(180, 148, 100, 255));
        for (int i = 0; i < 40; i++) {
            unsigned hsh = (unsigned)i * 2654435761u; hsh ^= hsh >> 13;
            float x = rx0 + 1 + (hsh & 255) / 255.0f * (rx1 - rx0 - 2), yy = ry0 + 1 + ((hsh >> 8) & 1023) / 1023.0f * (ry1 - ry0 - 2);
            P.dl->AddCircleFilled(NsDraw::w(x, yy), (0.35f + ((hsh >> 20) & 3) * 0.12f) * P.k, IM_COL32(120, 96, 64, 160), 6);
        }
        for (int s2 = 0; s2 < 2; s2++) P.dl->AddLine(NsDraw::w(s2 ? rx1 : rx0, ry0), NsDraw::w(s2 ? rx1 : rx0, ry1), IM_COL32(90, 120, 50, 200), 1.6f * P.k);   /* (grass at its edges) */
    }
    P.text(f, NsDraw::FS_HEAD, X0 + 4, Y0 + 3, ink, "Marching order");
    P.para(fo, 4.2f, X0 + 4, Y0 + 12, X1 - X0 - 8, soft, "The first leads the way; in single file the others follow in this order.");
    float y = Y0 + 25;
    /* each row dragged by its handle (three bars) to its new place; the others make room */
    static int drag = -1; static float grab;
    if (drag < 0 && ns_in.pressed && ns_in.px >= X0 + 34 && ns_in.px < X1 - 3) {
        int k = (int)floorf((ns_in.py - y) / RH);
        if (k >= 0 && k < m.n) { drag = k; grab = ns_in.py - (y + k * RH); ns_in.pressed = 0; }
    }
    int to = drag;
    if (drag >= 0) {
        to = (int)floorf((ns_in.y - grab + RH * 0.5f - y) / RH);
        if (to < 0) to = 0; if (to > m.n - 1) to = m.n - 1;
        if (!ns_in.down) { if (to != drag) ns_push(MV_MOVE, drag, to, 0); drag = -1; to = -1; }
    }
    auto row = [&](int k, int i, float ry, bool lifted) {
        if (lifted) P.dl->AddRectFilled(NsDraw::w(X0 + 35, ry + 1), NsDraw::w(X1 - 2, ry + RH - 1), IM_COL32(0, 0, 0, 70), 1.2f * P.k);
        P.rect(X0 + 34, ry, X1 - 3, ry + RH - 2, lifted ? IM_COL32(248, 236, 206, 255) : IM_COL32(236, 224, 196, 200), 1.2f);
        if (const GamePic *g = ns_figure_pic(40 + i, m.figure[i], m.pal[i], "SHORT")) {
            float sh = RH - 4, sw = g->w * sh / g->h;
            P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(X0 + 20, ry + 1), NsDraw::w(X0 + 20 + sw, ry + 1 + sh));
        }
        P.text(f, NsDraw::FS_NAME, X0 + 40, ry + 4.5f, ink, ns_gtext(m.name[i]), -1, X1 - 18);
        bool hv = P.over(X0 + 34, ry, X1 - 3, ry + RH - 2);
        for (int b = 0; b < 3; b++)                            /* the handle: three bars */
            P.dl->AddLine(NsDraw::w(X1 - 13, ry + 5 + b * 2.4f), NsDraw::w(X1 - 6, ry + 5 + b * 2.4f), hv || lifted ? IM_COL32(90, 56, 24, 255) : BROWN, 1.0f * P.k);
        (void)k;
    };
    for (int k = 0; k < m.n; k++) {                            /* the places: 1st, 2nd ... and the leader's pennant stay */
        float ry = y + k * RH;
        P.text(fo, NsDraw::FS_SMALL, X0 + 5, ry + 5, soft, NTH[k]);
        if (k == 0) {
            P.dl->AddLine(NsDraw::w(X0 + 31, ry - 1), NsDraw::w(X0 + 31, ry + RH - 3), IM_COL32(80, 50, 24, 255), 1.2f * P.k);
            ImVec2 fl[3] = { NsDraw::w(X0 + 31, ry - 1), NsDraw::w(X0 + 39, ry + 1.5f), NsDraw::w(X0 + 31, ry + 4) };
            P.dl->AddTriangleFilled(fl[0], fl[1], fl[2], IM_COL32(170, 30, 24, 255));
        }
    }
    {   /* the members, in the order they would have with the dragged one let go here */
        std::vector<int> ord(m.order, m.order + m.n);
        if (drag >= 0 && to >= 0 && to != drag) { int t = ord[drag]; ord.erase(ord.begin() + drag); ord.insert(ord.begin() + to, t); }
        for (int k = 0; k < m.n; k++) if (!(drag >= 0 && k == to)) row(k, ord[k], y + k * RH, false);
        if (drag >= 0) { float ry = ns_in.y - grab; if (ry < y - 4) ry = y - 4; if (ry > y + (m.n - 1) * RH + 4) ry = y + (m.n - 1) * RH + 4; row(to, ord[to], ry, true); }
    }
    float by = Y1 - 13;
    if (P.button(X1 - 70, by, X1 - 38, by + 10, "Done", IM_COL32(40, 92, 34, 235), NsDraw::FS_SMALL, PB_X)) ns_push(MV_DONE, 0, 0, 0);
    if (P.button(X1 - 35, by, X1 - 4, by + 10, "Cancel", IM_COL32(112, 40, 22, 235), NsDraw::FS_SMALL, PB_Y)) ns_push(MV_CANCEL, 0, 0, 0);
}
