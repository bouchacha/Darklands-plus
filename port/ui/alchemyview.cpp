/* alchemyview.cpp - draws the alchemy screen (mods/alchemyview.c runs it; docs/NATIVE_SCREENS.md).
 * Left the formulas the alchemist knows (greyed when an ingredient is short); right the chosen
 * one: what it does, its ingredients needed and at hand, the batches (- and +), the chance of
 * success with its parts, Start. Drawn from alchemy_model alone. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/alchemyview.h"
#include "../mods/fixes.h"
#include "nsdraw.h"
#include "../mods/gamemodel.h"
#include "../mods/gamemodel.h"
#include "gamepics.h"
#include <map>
std::string journal_where_sold(int id);              /* ui/journal.cpp */

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

namespace { struct AlchemyView { int scroll = 0; } V; }

extern "C" int alchemyview_showing(void) { return ns_screen == NS_ALCHEMY && alchemy_model.open && !ns_hidden(); }

extern "C" void alchemyview_draw(void)
{
    const AlchemyModel &a = alchemy_model;
    if (!alchemyview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM, red = NsDraw::RED, green = NsDraw::GREEN;
    /* the desk (ui/alchemydesk.cpp): the formulae on the book's left page and under them all the
       party's ingredients, the chosen formula on the right page */
    void alchemy_desk_draw(float t); alchemy_desk_draw((float)ImGui::GetTime());
    if (orn_on()) orn_frame(P.dl, NsDraw::w(60, 0), NsDraw::w(320, 200), 1.9f * P.k);
    const bool art = true;
    const ImU32 cream = IM_COL32(244, 230, 196, 255), cream_soft = IM_COL32(226, 204, 160, 235);
    auto shade = [&](float x0, float y0, float x1, float y1) { if (art) P.rect(x0, y0, x1, y1, IM_COL32(24, 12, 4, 150), 1.5f); };
    shade(63, 2, 240, 20);
    P.text(f, NsDraw::FS_TITLE, 65, 3, art ? cream : ink, "Alchemy");
    P.text(fo, NsDraw::FS_BODY, 65, 12.5f, art ? cream_soft : soft, ns_gtext(a.name) + " mixes; ingredients carried by the whole party count.");
    if (art) { if (P.button(246, 5, 277, 16, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(AV_LEAVE, 0, 0, 0); }
    else if (P.button(284, 4, 315, 15, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(AV_LEAVE, 0, 0, 0);

    /* the formulas */
    const float LX0 = 70, LX1 = 186, LY0 = 48, ROWH = 9; const int NROWS = 6;
    const float LY1 = LY0 + NROWS * ROWH;
    if (!art) {
        P.rect(LX0, LY0 - 1, LX1, LY1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
        P.frame(LX0, LY0 - 1, LX1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
    }
    if (P.over(LX0, LY0, LX1, LY1) && ns_in.wheel != 0) { V.scroll -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = a.nrows - NROWS; if (maxs < 0) maxs = 0;
    if (V.scroll > maxs) V.scroll = maxs;
    if (V.scroll < 0) V.scroll = 0;
    if (!a.nrows) P.para(fo, NsDraw::FS_BODY, LX0 + 4, LY0 + 3, LX1 - LX0 - 8, dim, ns_gtext(a.name) + " knows no formulae yet.");
    for (int r = 0; r < NROWS && V.scroll + r < a.nrows; r++) {
        int i = V.scroll + r;
        const AvRow &w = a.row[i];
        float y = LY0 + r * ROWH;
        bool h = P.over(LX0, y, LX1, y + ROWH), sel = i == a.sel;
        if (sel) P.rect(LX0 + 0.5f, y, LX1 - 0.5f, y + ROWH, art ? IM_COL32(150, 60, 40, 70) : IM_COL32(214, 186, 140, 255), art ? 1.5f : 0);
        else if (h) P.rect(LX0 + 0.5f, y, LX1 - 0.5f, y + ROWH, art ? IM_COL32(150, 60, 40, 36) : IM_COL32(226, 204, 160, 255), art ? 1.5f : 0);
        else if ((r & 1) && !art) P.rect(LX0 + 0.5f, y, LX1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
        P.icon(LX0 + 6.5f, y + ROWH * 0.5f, 7.0f, 95 + w.formula / 3, -1, w.formula % 3 + 1);
        std::string nm = w.formula < (int)gamedata.formula_names.size() ? gamedata.formula_names[w.formula] : "?";
        float end = LX1 - 3;
        if (w.count) { std::string c = std::to_string(w.count) + "x"; P.text(fo, NsDraw::FS_BODY, 0, y + 2, ink, c, end); end -= P.width(fo, NsDraw::FS_BODY, c) + 2; }
        P.text(f, NsDraw::FS_NAME, LX0 + 12, y + 1.3f, w.can || w.count ? ink : dim, nm, -1, end);
        if (P.hit(LX0, y, LX1, y + ROWH) && !sel) ns_push(AV_SELECT, i, 0, 0);
    }
    if (a.nrows > NROWS) {
        float th = (LY1 - LY0) * NROWS / a.nrows, ty = LY0 + (LY1 - LY0 - th) * V.scroll / (float)(a.nrows - NROWS);
        P.rect(LX1 - 1.6f, ty, LX1 - 0.4f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
    }

    /* the party's ingredients (as on the character sheet's Formulae page): all 23 always in the same
       places, those the party has none of faded; what the chosen formula calls for ringed, green when
       there is enough for a batch */
    {
        static int have[200]; GmItem g;
        for (int i = 0; i < 200; i++) have[i] = 0;
        for (int m = 0; m < 5; m++) if (gm_member_present(m)) for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &g) && g.id >= 0 && g.id < 200) have[g.id] += g.count;
        int fm = a.sel >= 0 && a.sel < a.nrows ? a.row[a.sel].formula : -1;
        const float SY0 = LY0 + NROWS * ROWH + 3, GY0 = SY0 + 6, cellw = (LX1 - LX0 - 2) / 8.0f, cellh = (165 - GY0) / 3.0f;
        P.rect(LX0 + 1, SY0 - 1.5f, LX1 - 1, SY0 - 1.2f, IM_COL32(120, 80, 36, 90));
        P.text(fo, 4.3f, LX0 + 2, SY0, soft, "The party's ingredients");
        int tip = -1;
        for (int i = 0; i < 23; i++) {
            int id = 72 + i, hv = have[id], need = 0;
            if (fm >= 0 && fm < (int)gamedata.formulas.size()) for (const FormulaIngr &q : gamedata.formulas[fm].ingr) if (q.item == id) need += q.count;
            float x0 = LX0 + 1 + (i % 8) * cellw, y0 = GY0 + (i / 8) * cellh, cxm = x0 + cellw * 0.5f;
            if (need) P.rect(x0 + 0.3f, y0 + 0.3f, x0 + cellw - 0.3f, y0 + cellh - 0.3f, hv >= need ? IM_COL32(150, 190, 120, 120) : IM_COL32(210, 120, 100, 110), 1.5f);
            if (P.over(x0, y0, x0 + cellw, y0 + cellh)) { tip = id; P.rect(x0 + 0.3f, y0 + 0.3f, x0 + cellw - 0.3f, y0 + cellh - 0.3f, IM_COL32(150, 60, 40, 40), 1.5f); }
            float is = fminf(cellw * 0.5f, cellh * 0.46f), iy = y0 + cellh * 0.36f;
            P.icon(cxm, iy, is, id);
            if (!hv) P.dl->AddCircleFilled(NsDraw::w(cxm, iy), is * 0.7f * P.k, need ? IM_COL32(238, 196, 180, 160) : IM_COL32(240, 228, 200, 165));   /* (none: the sign faded) */
            std::string c = need ? std::to_string(hv) + "/" + std::to_string(need) : hv ? std::to_string(hv) : "";
            ImU32 cc = need ? (hv >= need ? green : red) : ink;
            if (!c.empty()) P.text(fo, NsDraw::FS_SMALL, cxm - P.width(fo, NsDraw::FS_SMALL, c) * 0.5f, y0 + cellh * 0.68f, cc, c);
        }
        if (tip >= 0) {
            std::string use = ns_item_use(tip);
            char hl[64]; snprintf(hl, sizeof hl, "The party has %d.", have[tip]);
            ns_tip_begin(); ImGui::TextUnformatted(ns_item_name(tip).c_str()); ImGui::TextDisabled("%s", hl);
            if (!use.empty()) ImGui::TextUnformatted(use.c_str());
            ns_tip_end();
        }
    }

    /* the chosen formula */
    const float RX0 = art ? 194 : 186, RX1 = art ? 309 : 316;
    bool risk_hover = false;
    if (a.sel >= 0 && a.sel < a.nrows) {
        int fm = a.row[a.sel].formula;
        float y = art ? 48 : 21;
        std::string nm = fm < (int)gamedata.formula_names.size() ? gamedata.formula_names[fm] : "?";
        y += P.para(f, NsDraw::FS_HEAD, RX0, y, (art ? 248 : RX1) - RX0, ink, nm) + 0.5f;   /* (on the desk: clear of the bowl) */
        static const char *RISK[3] = { "Low risk", "Some risk", "High risk" };
        char rl[64]; snprintf(rl, sizeof rl, "%s, mystic number %d", a.risk >= 0 && a.risk < 3 ? RISK[a.risk] : "?", a.mystic);
        P.text(fo, NsDraw::FS_BODY, RX0, y, a.risk >= 2 ? red : soft, rl);
        risk_hover = P.over(RX0, y, RX1, y + 7);
        y += 7.5f;
        if (fm < (int)gamedata.formulas.size()) {
            const std::string &t = gamedata.formulas[fm].text;
            y += P.para(fo, NsDraw::FS_SMALL, RX0, y, RX1 - RX0, soft, game_text(t.c_str(), t.size())) + 0.5f;
            ImU32 ec; std::string enc = ns_item_encounters(95 + fm / 3, &ec);
            if (!enc.empty()) y += P.para(fo, NsDraw::FS_SMALL, RX0, y, RX1 - RX0, ec, enc);
            y += 1.5f;
        }
        P.text(f, NsDraw::FS_NAME, RX0, y, ink, "Ingredients for a batch"); y += 8;
        for (int k = 0; k < a.ningr; k++) {
            const AvIngr &g = a.ingr[k];
            P.icon(RX0 + 4, y + 3.5f, 6.5f, g.item);
            P.text(fo, NsDraw::FS_BODY, RX0 + 10, y + 0.8f, ink, std::to_string(g.need) + " " + ns_item_name(g.item), -1, RX1 - 30);
            std::string hv = g.have ? "at hand " + std::to_string(g.have) : std::string("none at hand");
            if (g.have < g.need && config_get_int("assist.where_sold", 1)) {    /* where to get more (the shops the party has seen) */
                static std::map<int, std::string> where; static unsigned wseq = 0;
                if (wseq != a.seq) { where.clear(); wseq = a.seq; }
                auto it = where.find(g.item);
                if (it == where.end()) it = where.emplace(g.item, journal_where_sold(g.item)).first;
                if (!it->second.empty()) {
                    float hw = P.width(fo, NsDraw::FS_SMALL, hv);
                    P.text(fo, NsDraw::FS_SMALL, 0, y + 1.2f, soft, it->second + "  -  ", RX1 - hw);
                }
            }
            P.text(fo, NsDraw::FS_SMALL, 0, y + 1.2f, g.have >= g.need ? green : red, hv, RX1);
            y += 7.5f;
        }
        y += art ? 1 : 2;
        /* the batches */
        P.text(f, NsDraw::FS_NAME, RX0, y + 1, ink, "Batches");
        float bx = RX0 + 32;
        if (a.page && a.count > 0) { if (P.button(bx, y, bx + 9, y + 8.5f, "-", IM_COL32(120, 84, 40, 235), NsDraw::FS_BODY)) ns_push(AV_FEWER, 0, 0, 0); }
        else P.button_off(bx, y, bx + 9, y + 8.5f, "-", NsDraw::FS_BODY);
        P.text(f, NsDraw::FS_HEAD, bx + 11, y, ink, std::to_string(a.page ? a.count : 0));
        float px = bx + 22;
        if (a.can_more) { if (P.button(px, y, px + 9, y + 8.5f, "+", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_MORE, 0, 0, 0); }
        else { P.button_off(px, y, px + 9, y + 8.5f, "+", NsDraw::FS_BODY); P.text(fo, NsDraw::FS_SMALL, px + 12, y + 1.6f, red, "not enough for more"); }
        y += 11;
        /* the chance */
        int pc = a.page && a.count > 0 ? a.pct : a.pct1;
        char ch[64]; snprintf(ch, sizeof ch, a.page && a.count > 0 ? "Chance of success %d%%" : "Chance with one batch %d%%", pc);
        P.text(f, NsDraw::FS_NAME, RX0, y, pc >= 50 ? green : red, ch); y += 7.5f;
        char parts[120]; snprintf(parts, sizeof parts, "%d (fewer batches: higher) + %d Intelligence + %d Alchemy - %d mystic number%s",
                                  a.base, a.intel, a.alch, a.mystic, a.stone ? (" + " + std::to_string(a.stone) + " Philosopher's Stone").c_str() : "");
        y += P.para(fo, NsDraw::FS_SMALL, RX0, y, RX1 - RX0, soft, parts) + 3;
        bool can = a.page && a.count > 0;
        float sx0 = art ? 216 : RX0, sy0 = art ? 174 : y, sx1 = art ? 296 : RX0 + 50, sy1 = art ? 186 : y + 10;   /* (on the desk: over the game's own bar) */
        if (can) { if (P.button(sx0, sy0, sx1, sy1, "Start mixing", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_START, 0, 0, 0); }
        else P.button_off(sx0, sy0, sx1, sy1, "Start mixing", NsDraw::FS_BODY);
    }
    if (a.note[0] && plat_ticks_ms() - a.note_t < 6000) { shade(63, 188, 318, 198); P.text(fo, NsDraw::FS_BODY, 65, 190, art ? cream : IM_COL32(112, 40, 22, 255), ns_gtext(a.note)); }
    else if (risk_hover) {                  /* what the risk means (mods/fixes.c, alchrisk) */
        static const char *IN[5] = { "", "one time", "two times", "three times", "" }, *OF[5] = { "", "", "two", "three", "four" };
        int num, den; fixes_alchemy_blast_odds(a.risk, &num, &den);
        char t[400];
        snprintf(t, sizeof t, "The risk is what a failed batch can cause. At an inn, in a village or a hideout, one failure in two goes wrong: "
                 "%s in %s it is an explosion (the party hurt, damages to pay, no room at the inn for four months), otherwise a stink (no room for a month). "
                 "In a camp in the wild a failure only costs the ingredients.", IN[num], OF[den]);
        if (art) { ns_tip_begin(); ImGui::TextUnformatted(t); ns_tip_end(); }
        else P.para(fo, NsDraw::FS_SMALL, 65, 186, 250, soft, t);
    }
}
