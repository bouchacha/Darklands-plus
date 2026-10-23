// mapmsg.cpp - a message of the travel map (mods/mapmsg.c runs it) in the cards' style: the text
// in the game's typeface on parchment over the map, and what goes on.
#include <stdio.h>
#include <string>
#include <vector>
#include "nsdraw.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/mapmsg.h"
}

extern "C" void mapmsg_draw(void)
{
    const MapMsgModel &m = mapmsg_model;
    extern int cardview_dialog, game_dialog_kind;
    if (!m.open || ns_screen != NS_MAPMSG || ns_hidden() || (cardview_dialog && !game_dialog_kind)) return;
    NsDraw P;
    if (!P.begin()) return;
    /* the paragraphs (the record's newlines), without their leading spaces */
    std::vector<std::string> paras;
    {
        std::string raw = m.text, cur;                       /* (split before the charset conversion: it makes the newline a space) */
        auto flush = [&]() { size_t a = cur.find_first_not_of(' '); if (a != std::string::npos) paras.push_back(game_text(cur.c_str() + a, cur.size() - a)); cur.clear(); };
        for (char c : raw) { if (c == '\n' || c == '\r') flush(); else cur += c; }
        flush();
    }
    const float X0 = 92, X1 = 288, W = X1 - X0 - 14, GS = 6.4f;
    /* measure, then draw centred over the map */
    float h = 0;
    for (const std::string &p : paras) if (!p.empty()) h += P.f->CalcTextSizeA(P.size(P.f, GS), FLT_MAX, W * P.kx, p.c_str()).y / P.k + 3;
    h += 18;
    float Y0 = 100 - h * 0.5f; if (Y0 < 6) Y0 = 6;
    float Y1 = Y0 + h; if (Y1 > 196) Y1 = 196;
    P.rect(X0 + 1.5f, Y0 + 1.5f, X1 + 1.5f, Y1 + 1.5f, IM_COL32(20, 12, 6, 120), 2.5f);    /* a shadow */
    P.rect(X0, Y0, X1, Y1, IM_COL32(242, 230, 200, 250), 2.5f);
    P.frame(X0, Y0, X1, Y1, IM_COL32(120, 80, 36, 200), 2.5f);
    float y = Y0 + 7;
    for (const std::string &p : paras) {
        if (p.empty()) continue;
        y += P.para(P.f, GS, X0 + 7, y, W, NsDraw::INK, p) + 3;
    }
    P.text(P.fo, NsDraw::FS_SMALL, 0, Y1 - 7.5f, NsDraw::INK_DIM, "press a key or click to go on", X1 - 6);
}
