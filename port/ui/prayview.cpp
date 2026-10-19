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
        y += P.para(fo, NsDraw::FS_BODY, tx, y, tw, IM_COL32(26, 64, 132, 255), saint_info_does(p.saint)) + 2;
        const char *un = saint_info_unlike(p.saint);
        if (*un) y += P.para(fo, 4.4f, tx, y, tw, IM_COL32(140, 50, 30, 255), std::string("Unlike the text: ") + un) + 1.5f;
        y += P.para(fo, 4.4f, tx, y, tw, soft, gamedata.saints[p.saint].text) + 2;
    }

    /* the favor, the chance, the choice */
    float by = fmaxf(y + 2, 128);
    P.rect(tx - 1.5f, by, 317, 196, IM_COL32(236, 224, 196, 255), 1.5f);
    float x = tx + 1.5f, yy = by + 2.5f;
    P.text(f, NsDraw::FS_HEAD, x, yy, ink, "Divine favor to spend");
    char b[64]; snprintf(b, sizeof b, "%d", p.df);
    float bx = 316 - 2;
    if (P.button(bx - 9, yy, bx, yy + 8.5f, "+", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(PV_MORE, 0, 0, 0);
    P.text(f, NsDraw::FS_HEAD, 0, yy, ink, b, bx - 12);
    if (P.button(bx - 33, yy, bx - 24, yy + 8.5f, "-", IM_COL32(120, 84, 40, 235), NsDraw::FS_BODY)) ns_push(PV_LESS, 0, 0, 0);
    yy += 10;
    snprintf(b, sizeof b, "%s has %d; %d will be left.", who.c_str(), p.maxdf, p.maxdf - p.df);
    P.text(fo, NsDraw::FS_SMALL, x, yy, soft, b);
    yy += 7.5f;
    snprintf(b, sizeof b, "Chance the saint hears: %d%%", p.chance);
    P.text(f, NsDraw::FS_HEAD, x, yy, p.chance >= 50 ? NsDraw::GREEN : p.chance > 0 ? ink : NsDraw::RED, b);
    yy += 9;
    P.text(fo, NsDraw::FS_SMALL, x, yy, dim, "More favor, a better chance (up to 99%); the favor is spent either way.");
    yy += 8;
    float bw = 40;
    if (P.button(316 - bw, 184, 316, 194, "Don't pray", IM_COL32(112, 40, 22, 235))) ns_push(PV_CANCEL, 0, 0, 0);
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
    } else P.text(fo, NsDraw::FS_SMALL, x, 186.5f, NsDraw::RED, "No chance as it is: spend more favor (or the saint does not help here).");
}
