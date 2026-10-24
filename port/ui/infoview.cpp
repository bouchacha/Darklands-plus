// infoview.cpp - the party information page drawn by Darklands+ (mods/infoview.c runs it): the
// party's fame and its title, the time and date, where it is and its reputation there, its
// wealth, and its reputation in every city (sorted by reputation or by name; the city the party is
// in or nearest marked).
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include "nsdraw.h"
#include "money.h"
#include "drawcache.h"
#include "repwatch.h"
#include "alchemydesk.h"
void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t);
int ui_pstone_look(void);
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/infoview.h"
}

extern "C" void infoview_draw(void)
{
    const InfoModel &m = info_model;
    if (!m.open || ns_screen != NS_INFO || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 PANEL = IM_COL32(244, 234, 210, 255), HOVER = IM_COL32(232, 214, 176, 255), HERE = IM_COL32(222, 196, 146, 255);
    static int by_name = 0; static float scroll = 0; static unsigned seen_seq;
    char b[256];
    /* on the table: the party's charter at the left, sealed; the roll of the cities at the right */
    {   ImVec2 o = NsDraw::w(60, 0), e = NsDraw::w(320, 200);
        dc_draw(P.dl, o, e, 0x1f0c4a7ull, [&](ImDrawList *dl) {
            desk_table(dl);
            /* the charter: its shadow, the parchment, a ruled border, the seal on its ribbon */
            dl->AddRectFilled(NsDraw::w(65, 6), NsDraw::w(173, 198), IM_COL32(0, 0, 0, 100), 2 * P.k);
            dl->AddRectFilledMultiColor(NsDraw::w(63, 3), NsDraw::w(170, 195), IM_COL32(246, 236, 210, 255), IM_COL32(240, 228, 198, 255), IM_COL32(226, 210, 172, 255), IM_COL32(234, 220, 186, 255));
            dl->AddRect(NsDraw::w(65.5f, 5.5f), NsDraw::w(167.5f, 192.5f), IM_COL32(150, 40, 26, 160), 0, 0, 1.0f);
            dl->AddRect(NsDraw::w(66.6f, 6.6f), NsDraw::w(166.4f, 191.4f), IM_COL32(150, 40, 26, 90), 0, 0, 0.8f);
            ImVec2 sc = NsDraw::w(150, 182);
            dl->AddQuadFilled(NsDraw::w(146, 184), NsDraw::w(149, 184), NsDraw::w(147, 199), NsDraw::w(143, 197), IM_COL32(40, 70, 120, 255));
            dl->AddQuadFilled(NsDraw::w(151, 184), NsDraw::w(154, 184), NsDraw::w(157, 197), NsDraw::w(153, 199), IM_COL32(34, 60, 106, 255));
            for (int i = 0; i < 14; i++) { float a = i * 6.2832f / 14; dl->AddCircleFilled(ImVec2(sc.x + cosf(a) * 5.6f * P.kx, sc.y + sinf(a) * 5.6f * P.k), 1.6f * P.k, IM_COL32(150, 24, 20, 255), 10); }
            dl->AddCircleFilled(sc, 6.0f * P.k, IM_COL32(170, 30, 24, 255), 28);
            dl->AddCircle(sc, 4.0f * P.k, IM_COL32(110, 12, 10, 255), 28, 0.9f * P.k);
            dl->AddCircleFilled(ImVec2(sc.x - 1.6f * P.k, sc.y - 1.8f * P.k), 1.6f * P.k, IM_COL32(230, 110, 90, 120), 12);
            /* the roll: the paper between two wooden rollers */
            dl->AddRectFilled(NsDraw::w(176, 9), NsDraw::w(320, 197), IM_COL32(0, 0, 0, 100), 2 * P.k);
            dl->AddRectFilledMultiColor(NsDraw::w(176, 8), NsDraw::w(315, 191), IM_COL32(244, 234, 206, 255), IM_COL32(238, 226, 194, 255), IM_COL32(224, 208, 170, 255), IM_COL32(232, 218, 182, 255));
            for (int r = 0; r < 2; r++) {
                float yc = r ? 192 : 7;
                dl->AddRectFilledMultiColor(NsDraw::w(173, yc - 3), NsDraw::w(318, yc + 3), IM_COL32(150, 100, 56, 255), IM_COL32(150, 100, 56, 255), IM_COL32(70, 42, 20, 255), IM_COL32(70, 42, 20, 255));
                dl->AddRectFilledMultiColor(NsDraw::w(173, yc - 3), NsDraw::w(318, yc - 1), IM_COL32(200, 150, 96, 255), IM_COL32(200, 150, 96, 255), IM_COL32(150, 100, 56, 0), IM_COL32(150, 100, 56, 0));
                for (int s2 = 0; s2 < 2; s2++) dl->AddCircleFilled(NsDraw::w(s2 ? 319.5f : 171.5f, yc), 3.2f * P.k, IM_COL32(110, 70, 34, 255), 14);
            }
        });
    }
    P.text(f, NsDraw::FS_TITLE, 70, 8, IM_COL32(120, 30, 18, 255), "The party");
    if (P.button(284, 196 - 12, 315, 196 - 1, "Close", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(IV_CLOSE, 0, 0, 0);

    /* the party */
    const float LX0 = 66, LX1 = 166;
    float x = LX0 + 3, y = 20, w = LX1 - LX0 - 6;
    auto head = [&](const char *t) { P.text(fo, 4.2f, x, y, dim, t); y += 6; };
    head("Fame");
    y += P.para(f, NsDraw::FS_HEAD, x, y, w, ink, ns_gtext(m.fame_word)) + 0.5f;
    int lv = m.fame / 50;
    if (lv < 10) snprintf(b, sizeof b, "%d (the next title at %d)", m.fame, (lv + 1) * 50); else snprintf(b, sizeof b, "%d", m.fame);
    P.text(fo, NsDraw::FS_BODY, x, y, ink, b); y += 10;
    head("Time");
    snprintf(b, sizeof b, "%d:00, %s", m.hour, ns_gtext(m.bell).c_str());
    P.text(fo, NsDraw::FS_BODY, x, y, ink, b); y += 6.5f;
    snprintf(b, sizeof b, "%d %s %d", m.day, ns_gtext(m.month_name).c_str(), m.year);   /* (as the original and the margin show it) */
    P.text(fo, NsDraw::FS_BODY, x, y, ink, b); y += 10;
    head(m.in_city ? "In" : "Where");
    y += P.para(f, NsDraw::FS_NAME, x, y, w, ink, ns_gtext(m.where)) + 0.5f;
    if (m.in_city && m.here >= 0) {
        snprintf(b, sizeof b, "Reputation here: %s (%d)", ns_gtext(m.city[m.here].band_word).c_str(), m.city[m.here].rep);
        y += P.para(fo, NsDraw::FS_BODY, x, y, w, ink, b);
    }
    y += 4;
    head("Wealth");
    y += P.para(fo, NsDraw::FS_BODY, x, y, w, ink, money_fgp(m.money[0], m.money[1], m.money[2])) + 0.5f;
    snprintf(b, sizeof b, "%s in all", money_str(m.total_pf).c_str());
    P.text(fo, NsDraw::FS_SMALL, x, y, soft, b); y += 7.5f;
    if (m.notes > 0) {                                      /* (none: nothing to say) */
        snprintf(b, sizeof b, "Bank notes: %s", money_fgp(m.notes, 0, 0).c_str());
        P.text(fo, NsDraw::FS_BODY, x, y, ink, b); y += 6.5f;
    }
    y += 3;
    head("Philosopher's Stone");
    ui_pstone_draw(P.dl, NsDraw::w(x + 6, y + 6), 13 * P.k, ui_pstone_look(), m.phstone, (float)ImGui::GetTime());
    if (m.phstone) snprintf(b, sizeof b, "Quality %d: +%d to every batch", m.phstone, m.phstone); else snprintf(b, sizeof b, "None yet: an alchemist or a university makes one");
    P.para(fo, NsDraw::FS_BODY, x + 15, y + 1, w - 15, m.phstone ? ink : soft, b);

    /* every city */
    const float RX0 = 177, RX1 = 314, RY0 = 12, RY1 = 182;
    P.text(f, NsDraw::FS_HEAD, RX0 + 3, RY0 + 2, ink, "Reputation in the cities");
    float sy = RY0 + 11;
    const char *SORT[2] = { "by reputation", "by name" };
    for (int k = 0; k < 2; k++) {
        float sx = RX0 + 3 + k * 36;
        bool on = by_name == k;
        P.tab(sx, sy, sx + 34, sy + 7, on, false, 1.2f);
        P.text(fo, 4.0f, sx + (34 - P.width(fo, 4.0f, SORT[k])) * 0.5f, sy + 1.3f, on ? NsDraw::LIGHT : dim, SORT[k]);
        if (P.hit(sx, sy, sx + 34, sy + 7)) { by_name = k; scroll = 0; }
    }
        std::vector<int> ord;
    for (int i = 0; i < IV_CITIES; i++) ord.push_back(i);
    auto name = [&](int i) { return i < (int)gamedata.cities.size() ? gamedata.cities[i].name : std::string("?"); };
    if (by_name) std::stable_sort(ord.begin(), ord.end(), [&](int a, int c) { return name(a) < name(c); });
    else std::stable_sort(ord.begin(), ord.end(), [&](int a, int c) { return m.city[a].rep != m.city[c].rep ? m.city[a].rep > m.city[c].rep : name(a) < name(c); });
    const float RH = 6.6f, TY = sy + 9;
    int rows = (int)((RY1 - 1 - TY) / RH), maxtop = IV_CITIES > rows ? IV_CITIES - rows : 0;
    if (P.over(RX0, TY, RX1, RY1) && ns_in.wheel != 0) { scroll -= ns_in.wheel * 3; ns_in.wheel = 0; }
    if (m.seq != seen_seq) {                                    /* a new opening: the city of the party in sight */
        seen_seq = m.seq;
        int at = (int)(std::find(ord.begin(), ord.end(), m.here) - ord.begin());
        if (m.here >= 0 && at < IV_CITIES) scroll = (float)(at - rows / 2);
    }
    if (scroll < 0) scroll = 0;
    if (scroll > maxtop) scroll = (float)maxtop;
    int top = (int)scroll;
    for (int r = top; r < IV_CITIES && r < top + rows; r++) {
        int i = ord[r];
        float ry = TY + (r - top) * RH;
        if (i == m.here) P.rect(RX0 + 1, ry, RX1 - 1, ry + RH, HERE, 0.8f);
        else if (P.over(RX0 + 1, ry, RX1 - 1, ry + RH)) P.rect(RX0 + 1, ry, RX1 - 1, ry + RH, HOVER, 0.8f);
        P.text(f, 5.0f, RX0 + 3, ry + 0.8f, ink, name(i), -1, RX0 + 60);
        int sz = m.city[i].size;
        P.text(fo, 3.8f, RX0 + 61, ry + 1.5f, dim, sz <= 4 ? "small" : sz <= 6 ? "medium" : "large");
        P.text(fo, 4.4f, RX0 + 80, ry + 1.0f, m.city[i].rep < 0 ? NsDraw::RED : ink, ns_gtext(m.city[i].band_word), -1, RX1 - 14);
        snprintf(b, sizeof b, "%d", m.city[i].rep);
        P.text(fo, 4.4f, 0, ry + 1.0f, m.city[i].rep < 0 ? NsDraw::RED : soft, b, RX1 - 3);
        if (const RepChange *rc = repwatch_last(i)) {            /* (changed since the game was loaded: by how much, the cause in a tooltip) */
            snprintf(b, sizeof b, "%+d", rc->delta);
            P.text(fo, 3.6f, 0, ry + 1.6f, rc->delta < 0 ? NsDraw::RED : NsDraw::GREEN, b, RX1 - 13);
            if (P.over(RX0 + 1, ry, RX1 - 1, ry + RH)) {
                std::string t = std::string("Last change: ") + b + " on " + std::to_string(rc->day) + "." + std::to_string(rc->month + 1) + "." + std::to_string(rc->year);
                if (!rc->choice.empty()) t += ", after choosing \"" + rc->choice + "\"";
                ns_tip(t.c_str());
            }
        }
    }
    if (maxtop) {
        float th = (RY1 - 2 - TY) * rows / IV_CITIES, ty = TY + (RY1 - 2 - TY - th) * (scroll / maxtop);
        P.rect(RX1 - 2.2f, ty, RX1 - 1, ty + th, IM_COL32(120, 80, 36, 120), 0.6f);
    }
}
