// prayview.cpp - the prayer screen drawn by Darklands+ (mods/prayview.c runs it): the saint's
// statue, legend and what the saint does, the favor to spend with - and +, the chance of success,
// and Pray (for a member, when the saint helps one) or Don't pray.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/prayview.h"
#include "../mods/saintinfo.h"
}
const GamePic *ns_saint_statue(int cls);

extern "C" void prayview_draw(void)
{
    const PrayModel &p = pray_model;
    if (!p.open || ns_screen != NS_PRAY || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    P.page();
    bool known = p.saint >= 0 && p.saint < (int)gamedata.saints.size();

    /* the statue */
    float tx = 66;
    if (const GamePic *g = ns_saint_statue(p.cls)) {
        float sh = 190, sw = g->w * sh / g->h;
        P.dl->AddImageRounded((ImTextureID)(intptr_t)g->tex, NsDraw::w(63, 4), NsDraw::w(63 + sw, 4 + sh), ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 1.5f * P.k);
        tx = 63 + sw + 4;
    }
    float tw = 316 - tx, y = 4;
    std::string who = ns_gtext(p.names[p.who < 0 || p.who > 4 ? 0 : p.who]);
    y += P.para(fo, NsDraw::FS_SMALL, tx, y, tw, soft, who + (p.party ? " calls on the saint for the whole party:" : " calls on the saint:")) + 0.5f;
    y += P.para(f, NsDraw::FS_TITLE, tx, y, tw, ink, known ? gamedata.saints[p.saint].name : "?") + 0.5f;
    if (p.day[0]) y += P.para(fo, NsDraw::FS_SMALL, tx, y, tw, dim, std::string("Saint's day: ") + p.day) + 1.5f;
    if (known) {
        /* what the help really does (mods/saintinfo.h), where the text says otherwise, then the text */
        y += P.para(fo, NsDraw::FS_BODY, tx, y, tw, IM_COL32(26, 64, 132, 255), saint_info_does(p.saint)) + 0.5f;
        if (*saint_info_note(p.saint)) y += P.para(fo, 4.2f, tx, y, tw, dim, saint_info_note(p.saint)) + 1.5f;
        const char *un = saint_info_unlike(p.saint);
        if (*un) y += P.para(fo, 4.4f, tx, y, tw, IM_COL32(140, 50, 30, 255), std::string("Differs from the description: ") + un) + 1.5f;
        y += P.para(fo, 4.4f, tx, y, tw, soft, gamedata.saints[p.saint].text) + 2;
    }

    /* the favor, the chance, the choice */
    float by = fmaxf(y + 2, 128);
    P.rect(tx - 1.5f, by, 317, 196, IM_COL32(236, 224, 196, 255), 1.5f);
    float x = tx + 1.5f, yy = by + 2.5f;
    P.text(f, NsDraw::FS_HEAD, x, yy, ink, "Divine Favor to spend");
    char b[64]; snprintf(b, sizeof b, "%d", p.df);
    float bx = 316 - 2;
    if (p.why != 1 && P.button(bx - 9, yy, bx, yy + 8.5f, "+", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(PV_MORE, 0, 0, 0);
    P.text(f, NsDraw::FS_HEAD, 0, yy, ink, b, bx - 12);
    if (p.why != 1 && P.button(bx - 33, yy, bx - 24, yy + 8.5f, "-", IM_COL32(120, 84, 40, 235), NsDraw::FS_BODY)) ns_push(PV_LESS, 0, 0, 0);
    yy += 10;
    if (p.why == 1) {                                         /* Virtue too low: no favor helps, the bar greyed and still */
        P.rect(x, yy, bx, yy + 5.5f, IM_COL32(170, 166, 158, 255), 1.5f);
        yy += 8;
    } else {   /* the bar: drag (or click) to set the favor; the mark where the best chance is reached */
        float x0 = x, x1 = bx, y0 = yy, y1 = yy + 5.5f;
        P.rect(x0, y0, x1, y1, IM_COL32(120, 100, 70, 255), 1.5f);
        float f = p.maxdf > 0 ? (float)p.df / p.maxdf : 0;
        P.rect(x0 + 0.5f, y0 + 0.5f, x0 + 0.5f + (x1 - x0 - 1) * f, y1 - 0.5f, IM_COL32(222, 186, 70, 255), 1.2f);
        if (p.df99 >= 0 && p.maxdf > 0) {
            float mx2 = x0 + (x1 - x0) * p.df99 / p.maxdf;
            P.rect(mx2 - 0.4f, y0 - 1.5f, mx2 + 0.4f, y1 + 1.5f, NsDraw::GREEN);
        }
        float hx = x0 + (x1 - x0) * f;
        P.rect(hx - 1.2f, y0 - 1.0f, hx + 1.2f, y1 + 1.0f, IM_COL32(90, 60, 20, 255), 0.8f);
        static bool drag;
        if (!ns_in.down) drag = false;
        else if (ns_in.px >= x0 - 2 && ns_in.px < x1 + 2 && ns_in.py >= y0 - 3 && ns_in.py < y1 + 3) drag = true;   /* (went down on the bar) */
        if (P.hit(x0 - 2, y0 - 3, x1 + 2, y1 + 3)) drag = true;                                                     /* (a click on it) */
        if (drag && p.maxdf > 0) {
            float t = (P.mx - x0) / (x1 - x0); t = t < 0 ? 0 : t > 1 ? 1 : t;
            int v = (int)(t * p.maxdf + 0.5f);
            if (p.df99 >= 0 && v > p.df99 && v < p.df99 + p.maxdf / 40 + 1) v = p.df99;   /* (snaps to the best chance's mark) */
            if (v != p.df) ns_push(PV_SET, v, 0, 0);
            if (!ns_in.down) drag = false;
        }
        yy += 8;
    }
    snprintf(b, sizeof b, p.df99 >= 0 && p.df > p.df99 ? "%s has %d; %d will be left. Past the green mark the chance rises no more." : "%s has %d; %d will be left.", who.c_str(), p.maxdf, p.maxdf - p.df);
    P.text(fo, NsDraw::FS_SMALL, x, yy, soft, b);
    yy += 7.5f;
    snprintf(b, sizeof b, "Chance the saint hears: %d%%", p.chance);
    P.text(f, NsDraw::FS_HEAD, x, yy, p.chance >= 50 ? NsDraw::GREEN : p.chance > 0 ? ink : NsDraw::RED, b);
    yy += 9;
    if (p.why != 1) P.text(fo, NsDraw::FS_SMALL, x, yy, dim, "More Divine Favor gives a better chance (up to 99%); it is spent either way.");
    yy += 8;
    float bw = 40;
    if (P.button(316 - bw, 184, 316, 194, "Don't pray", IM_COL32(112, 40, 22, 235), NsDraw::FS_SMALL, PB_Y)) ns_push(PV_CANCEL, 0, 0, 0);
    if (p.chance > 0) {
        if (!p.choose) { if (P.button(316 - 2 * bw - 3, 184, 316 - bw - 3, 194, "Pray now", IM_COL32(40, 92, 34, 235))) ns_push(PV_PRAY, -1, 0, 0); }
        else {
            P.text(fo, NsDraw::FS_SMALL, x, yy, soft, "Pray for:");
            float px = x + P.width(fo, NsDraw::FS_SMALL, "Pray for:") + 3, py = yy - 1.5f;
            for (int m = 0; m < 5; m++) {
                if (!p.present[m]) continue;
                std::string nm = ns_gtext(p.names[m]);
                float w = P.width(fo, NsDraw::FS_SMALL, nm) + 6;
                if (px + w > 316) { px = x; py += 10; }
                if (P.button(px, py, px + w, py + 8.5f, nm, IM_COL32(40, 92, 34, 235))) ns_push(PV_PRAY, m, 0, 0);
                px += w + 2;
            }
        }
    } else {
        char b[160];
        if (p.why == 1) snprintf(b, sizeof b, "%s's Virtue (%d) is below the %d this saint asks: more Divine Favor will not help.", who.c_str(), p.virtue, p.vmin);
        else if (p.why == 2 && p.maxdf < p.xmin) snprintf(b, sizeof b, "This saint asks at least %d Divine Favor; %s has only %d.", p.xmin, who.c_str(), p.maxdf);
        else if (p.why == 2) snprintf(b, sizeof b, "This saint asks at least %d Divine Favor: spend more.", p.xmin);
        else snprintf(b, sizeof b, "No chance as it is: spend more Divine Favor (or the saint does not help here).");
        P.text(fo, NsDraw::FS_SMALL, x, 186.5f, NsDraw::RED, b);
    }
}
