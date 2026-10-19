// marchview.cpp - Change Marching Order drawn by Darklands+ (mods/marchview.c runs it): a dialog
// with the members in their order, each with its figure and buttons to move it up or down; Done
// keeps the new order, Cancel the old.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
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
    P.rect(X0, Y0, X1, Y1, IM_COL32(242, 230, 200, 255), 2);
    P.frame(X0, Y0, X1, Y1, IM_COL32(120, 80, 36, 180), 2);
    P.text(f, NsDraw::FS_HEAD, X0 + 4, Y0 + 3, ink, "Marching order");
    P.para(fo, 4.2f, X0 + 4, Y0 + 12, X1 - X0 - 8, soft, "The first leads the way; in single file the others follow in this order.");
    float y = Y0 + 25;
    for (int k = 0; k < m.n; k++) {
        int i = m.order[k];
        float ry = y + k * RH;
        P.rect(X0 + 3, ry, X1 - 3, ry + RH - 2, IM_COL32(236, 224, 196, 255), 1.2f);
        P.text(fo, NsDraw::FS_SMALL, X0 + 6, ry + 5, soft, NTH[k]);
        if (const GamePic *g = ns_figure_pic(40 + i, m.figure[i], m.pal[i], "SHORT")) {
            float sh = RH - 4, sw = g->w * sh / g->h;
            P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(X0 + 20, ry + 1), NsDraw::w(X0 + 20 + sw, ry + 1 + sh));
        }
        P.text(f, NsDraw::FS_NAME, X0 + 34, ry + 4.5f, ink, ns_gtext(m.name[i]), -1, X1 - 32);
        bool up = k > 0, down = k + 1 < m.n;
        if (P.arrow_button(X1 - 28, ry + 3.5f, X1 - 17, ry + 12.5f, true, up, BROWN)) ns_push(MV_UP, k, 0, 0);
        if (P.arrow_button(X1 - 15, ry + 3.5f, X1 - 4, ry + 12.5f, false, down, BROWN)) ns_push(MV_DOWN, k, 0, 0);
    }
    float by = Y1 - 13;
    if (P.button(X1 - 70, by, X1 - 38, by + 10, "Done", IM_COL32(40, 92, 34, 235))) ns_push(MV_DONE, 0, 0, 0);
    if (P.button(X1 - 35, by, X1 - 4, by + 10, "Cancel", IM_COL32(112, 40, 22, 235))) ns_push(MV_CANCEL, 0, 0, 0);
}
