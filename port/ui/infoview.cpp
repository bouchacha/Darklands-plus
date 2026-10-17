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
    P.page();
    P.text(f, NsDraw::FS_TITLE, 65, 2.5f, ink, "The party");
    if (P.button(290, 3, 316, 11.5f, "Close", IM_COL32(112, 40, 22, 235))) ns_push(IV_CLOSE, 0, 0, 0);

    /* the party */
    const float LX0 = 63, LX1 = 170;
    P.rect(LX0, 15, LX1, 196, PANEL, 1.8f);
    float x = LX0 + 3, y = 17, w = LX1 - LX0 - 6;
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
    snprintf(b, sizeof b, "%d florins, %d groschen, %d pfennigs", m.money[0], m.money[1], m.money[2]);
    y += P.para(fo, NsDraw::FS_BODY, x, y, w, ink, b) + 0.5f;
    snprintf(b, sizeof b, "%ld pfennigs in all", m.total_pf);
    P.text(fo, NsDraw::FS_SMALL, x, y, soft, b); y += 7.5f;
    snprintf(b, sizeof b, "Notes: %d florins", m.notes);
    P.text(fo, NsDraw::FS_BODY, x, y, ink, b); y += 6.5f;
    if (m.phstone) snprintf(b, sizeof b, "Philosopher's Stone: +%d to alchemy", m.phstone); else snprintf(b, sizeof b, "No Philosopher's Stone yet");
    y += P.para(fo, NsDraw::FS_BODY, x, y, w, ink, b);

    /* every city */
    const float RX0 = 173, RX1 = 317, RY0 = 15, RY1 = 196;
    P.rect(RX0, RY0, RX1, RY1, PANEL, 1.8f);
    P.text(f, NsDraw::FS_HEAD, RX0 + 3, RY0 + 2, ink, "Reputation in the cities");
    float sy = RY0 + 11;
    const char *SORT[2] = { "by reputation", "by name" };
    for (int k = 0; k < 2; k++) {
        float sx = RX0 + 3 + k * 36;
        bool on = by_name == k;
        P.rect(sx, sy, sx + 34, sy + 7, on ? IM_COL32(112, 40, 22, 230) : IM_COL32(222, 204, 168, 255), 1.2f);
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
    }
    if (maxtop) {
        float th = (RY1 - 2 - TY) * rows / IV_CITIES, ty = TY + (RY1 - 2 - TY - th) * (scroll / maxtop);
        P.rect(RX1 - 2.2f, ty, RX1 - 1, ty + th, IM_COL32(120, 80, 36, 120), 0.6f);
    }
}
