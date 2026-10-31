// choose.cpp - the box asking which of the party's things (mods/choose.c runs it): over the card,
// in the cards' style, a line each (the thing's picture, name and quality, who carries it: their
// head), a click or its number picks it; and a button for none.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
#include "richtext.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/choose.h"
#include "../mods/gamemodel.h"
}

extern "C" void choose_draw(void)
{
    const ChooseModel &c = choose_model;
    if (!c.open || ns_screen != NS_CHOOSE || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    const float X0 = 92, X1 = 288, ROWH = 12, W = X1 - X0 - 14;
    float th = P.para_height(P.f, NsDraw::FS_HEAD, W, ns_gtext(c.title));
    float h = 7 + th + 4 + c.n * ROWH + 4 + 10 + 6;
    float Y0 = 100 - h * 0.5f; if (Y0 < 4) Y0 = 4;
    float Y1 = Y0 + h; if (Y1 > 197) Y1 = 197;
    P.rect(X0 + 1.5f, Y0 + 1.5f, X1 + 1.5f, Y1 + 1.5f, IM_COL32(20, 12, 6, 120), 2.5f);    /* a shadow */
    P.rect(X0, Y0, X1, Y1, IM_COL32(242, 230, 200, 250), 2.5f);
    P.frame(X0, Y0, X1, Y1, IM_COL32(120, 80, 36, 200), 2.5f);
    float y = Y0 + 7;
    y += P.para(P.f, NsDraw::FS_HEAD, X0 + 7, y, W, NsDraw::INK, ns_gtext(c.title)) + 4;
    for (int i = 0; i < c.n && y + ROWH <= Y1 - 16; i++, y += ROWH) {
        const ChooseRow &r = c.row[i];
        bool h = P.over(X0 + 4, y, X1 - 4, y + ROWH);
        if (h) P.rect(X0 + 4, y, X1 - 4, y + ROWH, IM_COL32(214, 170, 60, 90), 1.5f);
        char num[4]; snprintf(num, sizeof num, "%d", i + 1);
        if (i < 9) P.text(P.fo, NsDraw::FS_SMALL, X0 + 6, y + 3.6f, NsDraw::INK_DIM, num);
        P.icon(X0 + 17, y + ROWH * 0.5f, 10.0f, r.id, r.qual);
        std::string nm = ns_item_name(r.id), q = ns_quality_tag(r.id, r.qual);
        float nx = X0 + 25;
        P.text(P.f, NsDraw::FS_NAME, nx, y + 2.6f, NsDraw::INK, nm, -1, X1 - 22);
        if (!q.empty()) P.text(P.fo, NsDraw::FS_SMALL, nx + P.width(P.f, NsDraw::FS_NAME, nm) + 2, y + 3.4f, NsDraw::INK_SOFT, q);
        ui_member_head(P.dl, r.member, NsDraw::w(X1 - 16, y + 1), 10 * P.k);          /* (who carries it: their head; the name under the pointer) */
        if (P.over(X1 - 16, y, X1 - 5, y + ROWH)) { char n[24]; gm_member_name(r.member, n, sizeof n); ns_tip(ns_gtext(n).c_str(), 12); }
        if (P.hit(X0 + 4, y, X1 - 4, y + ROWH)) ns_push(CH_PICK, i, 0, 0);
    }
    float bw = P.width(P.fo, NsDraw::FS_SMALL, c.none) + 10;
    if (P.button(X1 - 6 - bw, Y1 - 13, X1 - 6, Y1 - 4, c.none, IM_COL32(120, 84, 40, 235))) ns_push(CH_NONE, 0, 0, 0);
}
