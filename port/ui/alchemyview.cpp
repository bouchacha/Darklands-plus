/* alchemyview.cpp - draws the alchemy screen (mods/alchemyview.c runs it; docs/NATIVE_SCREENS.md)
 * on an alchemist's workbench (ui/alchemydesk.cpp): the formulary open, its left page the index
 * of the formulas the alchemist knows (how many potions the party's ingredients make; a star puts
 * one first, a mark puts it aside, last), its right page the chosen one written out (what it does,
 * its mystic number and risk, its ingredients needed and at hand); the apothecary's chest beside
 * it, a compartment for each of the 23 ingredients with what the party carries, those the formula
 * needs tagged; the wax tablet below: the batches (- and +), the chance for every number of them
 * the ingredients allow (a click there sets it), what may come of the brew and what the chance is
 * made of, in pictures; Start on the furnace. Drawn from alchemy_model. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/alchemyview.h"
#include "../mods/fixes.h"
#include "../mods/keep.h"
#include "nsdraw.h"
#include "pstone.h"
#include "skillicons.h"
#include "marks.h"
#include "../mods/gamemodel.h"
#include "alchemydesk.h"
#include <map>
std::string journal_where_sold(int id);              /* ui/journal.cpp */

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
int ui_potion_tier(int q, int base);   /* ui/alchemyicons.cpp */

namespace {
struct AlchemyView { int scroll = 0; bool was_open = false; bool drag = false; } V;

/* the small signs (ui/marks.h) at game coordinates: x, y the middle, r the half size in game pixels */
ImVec2 W(float x, float y) { return NsDraw::w(x, y); }
float R(float r) { return NsDraw::w(r, 0).x - NsDraw::w(0, 0).x; }
void star(ImDrawList *dl, float x, float y, float r, ImU32 fill, ImU32 line, float th) { ui_mark_star(dl, W(x, y), R(r), fill, line, th); }
void aside(ImDrawList *dl, float x, float y, float r, ImU32 col, float th) { ui_mark_aside(dl, W(x, y), R(r), col, th); }
void pentacle(ImDrawList *dl, float x, float y, float r, ImU32 col, float th) { ui_mark_pentacle(dl, W(x, y), R(r), col, th); }
void falling_bars(ImDrawList *dl, float x, float y, float r, ImU32 col) { ui_mark_falling_bars(dl, W(x, y), R(r), col); }
void blast(ImDrawList *dl, float x, float y, float r) { ui_mark_blast(dl, W(x, y), R(r)); }
void stink(ImDrawList *dl, float x, float y, float r, float th) { ui_mark_stink(dl, W(x, y), R(r), th); }
void lost(ImDrawList *dl, float x, float y, float r, int item, float th) { ui_mark_lost(dl, W(x, y), R(r), item, th); }
std::string pct_text(float v) { char b[16]; if (v > 0 && v < 1) snprintf(b, sizeof b, "<1%%"); else snprintf(b, sizeof b, "%d%%", (int)(v + 0.5f)); return b; }
}

extern "C" int alchemyview_showing(void) { return ns_screen == NS_ALCHEMY && alchemy_model.open && !ns_hidden(); }

extern "C" void alchemyview_draw(void)
{
    const AlchemyModel &a = alchemy_model;
    if (!alchemyview_showing()) { V.was_open = false; return; }
    NsDraw P;
    if (!P.begin()) return;
    if (!V.was_open) { V.scroll = 0; V.drag = false; V.was_open = true; }
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM, red = NsDraw::RED, green = NsDraw::GREEN;
    const ImU32 cream = IM_COL32(244, 230, 196, 255), cream_soft = IM_COL32(226, 204, 160, 235);
    const ImU32 wax = IM_COL32(222, 210, 176, 255), wax_soft = IM_COL32(176, 164, 134, 230), wax_red = IM_COL32(240, 120, 96, 255), wax_green = IM_COL32(150, 214, 120, 255);
    const float th = fmaxf(1.0f, 0.35f * P.k);
    void alchemy_desk_draw(float t); alchemy_desk_draw((float)ImGui::GetTime());
    if (orn_on()) orn_frame(P.dl, NsDraw::w(60, 0), NsDraw::w(320, 200), 1.9f * P.k);

    /* what the party carries, all of it (the pack shared or not) */
    static int have[200]; GmItem g;
    for (int i = 0; i < 200; i++) have[i] = 0;
    for (int m = 0; m < 5; m++) if (gm_member_present(m)) for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &g) && g.id >= 0 && g.id < 200) have[g.id] += g.count;
    auto batches_of = [&](int fm) {
        if (fm < 0 || fm >= (int)gamedata.formulas.size() || gamedata.formulas[fm].ingr.empty()) return 0;
        int b = 1 << 20;
        for (const FormulaIngr &q : gamedata.formulas[fm].ingr) if (q.count > 0) { int c = (q.item >= 0 && q.item < 200 ? have[q.item] : 0) / q.count; if (c < b) b = c; }
        return b == 1 << 20 ? 0 : b;
    };
    /* the potions carried, by grade (1 to 3, as the formula's: ui_potion_tier) */
    static int pot[22][4];
    for (int k = 0; k < 22; k++) for (int g2 = 0; g2 < 4; g2++) pot[k][g2] = 0;
    for (int m = 0; m < 5; m++) if (gm_member_present(m)) for (int i = 0; i < gm_item_count(m); i++)
        if (gm_item(m, i, &g) && g.id >= 95 && g.id <= 116) {
            const ItemDef *d = ns_itemdef(g.id);
            int t = d ? ui_potion_tier(g.qual, d->quality) : 0; if (t < 1 || t > 3) t = 0;
            pot[g.id - 95][t] += g.count;
        }
    auto potions_of = [&](int fm) { int k = fm / 3; return k >= 0 && k < 22 ? pot[k][0] + pot[k][1] + pot[k][2] + pot[k][3] : 0; };
    int sel_f = a.sel >= 0 && a.sel < a.nrows ? a.row[a.sel].formula : -1;
    std::string who = ns_gtext(a.name);

    /* the head, on the table */
    P.rect(63, 2, 170, 19, IM_COL32(24, 12, 4, 150), 1.5f);
    P.text(f, NsDraw::FS_TITLE, 65, 3, cream, "Alchemy");
    P.text(fo, NsDraw::FS_BODY, 65, 12.5f, cream_soft, who + " mixes; the whole party's ingredients count.", -1, 169);
    {   /* the party's Philosopher's Stone, on its own plaque: what it adds to every batch */
        P.rect(173, 2, 279, 19, IM_COL32(24, 12, 4, 170), 1.5f);
        ui_pstone_draw(P.dl, NsDraw::w(182, 10.5f), 15 * P.k, ui_pstone_look(), a.stone, (float)ImGui::GetTime());
        P.text(f, NsDraw::FS_NAME, 191, 3, a.stone ? IM_COL32(255, 196, 150, 255) : cream_soft, a.stone ? "The party's Philosopher's Stone" : "No Philosopher's Stone", -1, 278);
        char sl[80];
        if (a.stone) snprintf(sl, sizeof sl, "Quality %d: +%d to every batch's chance", a.stone, a.stone);
        else snprintf(sl, sizeof sl, "An alchemist or a university sells one");
        P.text(fo, NsDraw::FS_SMALL, 191, 11.5f, cream_soft, sl, -1, 278);
        if (P.over(173, 2, 279, 19))
            ns_tip(a.stone ? ("The party's Philosopher's Stone, of quality " + std::to_string(a.stone) + ": it adds " + std::to_string(a.stone) + " to the chance of every batch, whoever mixes. Its picture shows the stage of the alchemists' Great Work its quality has reached: " + std::string(ui_pstone_stage_name(a.stone)) + ". A town's alchemist (up to 25) or university (up to 29) may sell a better one, each always the same quality; the card's option is greyed where it is no better than the party's.").c_str()
                           : "The party has no Philosopher's Stone yet. A town's alchemist (up to quality 25) or university (up to 29) sells one; its quality adds to the chance of every batch.");
    }
    if (P.button(282, 4, 316, 16, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(AV_LEAVE, 0, 0, 0);

    /* ---- the formulary's left page: the index, the starred first and those put aside last */
    const float LX0 = AD_BOOK_X0 + 6, LX1 = AD_GUTTER - 4, LY0 = AD_BOOK_Y0 + 11, ROWH = 12.5f; const int NROWS = 7;
    const float LY1 = LY0 + NROWS * ROWH;
    std::vector<int> order;
    for (int mk : { FMARK_FIRST, FMARK_NONE, FMARK_LAST })
        for (int i = 0; i < a.nrows; i++) { int m = formula_mark(a.row[i].formula); if (m == mk || (mk == FMARK_NONE && m != FMARK_FIRST && m != FMARK_LAST)) order.push_back(i); }
    P.text(fo, 4.3f, LX0 + 1, AD_BOOK_Y0 + 5, soft, "Formulae known");
    if (P.over(LX0, LY0, LX1, LY1) && ns_in.wheel != 0) { V.scroll -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = a.nrows - NROWS; if (maxs < 0) maxs = 0;
    if (V.scroll > maxs) V.scroll = maxs;
    if (V.scroll < 0) V.scroll = 0;
    if (!a.nrows) P.para(fo, NsDraw::FS_BODY, LX0 + 2, LY0 + 2, LX1 - LX0 - 4, dim, who + " knows no formulae yet.");
    const char *mark_tip = nullptr;
    for (int r = 0; r < NROWS && V.scroll + r < (int)order.size(); r++) {
        int i = order[V.scroll + r];
        const AvRow &w = a.row[i];
        int mk = formula_mark(w.formula);
        float y = LY0 + r * ROWH;
        bool h = P.over(LX0, y, LX1, y + ROWH), sel = i == a.sel, put = mk == FMARK_LAST;
        if (sel) P.rect(LX0, y + 0.3f, LX1, y + ROWH - 0.3f, IM_COL32(150, 60, 40, 70), 1.5f);
        else if (h) P.rect(LX0, y + 0.3f, LX1, y + ROWH - 0.3f, IM_COL32(150, 60, 40, 36), 1.5f);
        P.icon(LX0 + 5, y + ROWH * 0.5f, 7.5f, 95 + w.formula / 3, -1, w.formula % 3 + 1);
        if (put) P.dl->AddCircleFilled(NsDraw::w(LX0 + 5, y + ROWH * 0.5f), 4.2f * P.k, IM_COL32(236, 222, 186, 150));   /* (put aside: its flask faint) */
        std::string nm = w.formula < (int)gamedata.formula_names.size() ? gamedata.formula_names[w.formula] : "?";
        int bt = batches_of(w.formula);
        int pc = potions_of(w.formula);                                   /* (the potion carried already, any grade) */
        std::string pcs = pc ? std::to_string(pc) + " carried" : "";
        float pcw = pc ? P.width(fo, 4.2f, pcs) + 2 : 0;
        P.text(f, 5.0f, LX0 + 10.5f, y + 0.9f, (bt || w.count) && !put ? ink : dim, nm, -1, LX1 - 1 - pcw);
        if (pc) P.text(fo, 4.2f, 0, y + 1.6f, put ? dim : soft, pcs, LX1 - 1);
        std::string l2;
        if (bt) l2 = "enough ingredients to make " + std::to_string(bt) + (bt == 1 ? " potion" : " potions");
        else if (w.formula < (int)gamedata.formulas.size()) {                /* what is short: the first, and how many more */
            int nshort = 0; std::string first;
            for (const FormulaIngr &q : gamedata.formulas[w.formula].ingr) if (q.item >= 0 && q.item < 200 && have[q.item] < q.count) { if (!nshort) first = ns_item_name(q.item); nshort++; }
            l2 = "needs " + first + (nshort > 1 ? " and " + std::to_string(nshort - 1) + " more" : "");
        }
        if (w.count) l2 = std::to_string(w.count) + (w.count == 1 ? " batch set - " : " batches set - ") + l2;
        /* the marks, at the end of the second line: shown when set, offered when the row is pointed at */
        const float SX = LX1 - 9.5f, AX2 = LX1 - 3.5f, MY = y + 9.0f;
        bool on_star = P.over(SX - 2.8f, y + 5.8f, SX + 2.8f, y + ROWH), on_aside = P.over(AX2 - 2.8f, y + 5.8f, AX2 + 2.8f, y + ROWH);
        P.text(fo, 4.2f, LX0 + 10.5f, y + 6.8f, put ? dim : bt ? green : red, l2, -1, (h || mk) ? SX - 3.5f : LX1 - 1);
        if (mk == FMARK_FIRST || h)
            star(P.dl, SX, MY, 2.3f, mk == FMARK_FIRST ? IM_COL32(222, 168, 40, 255) : on_star ? IM_COL32(222, 168, 40, 110) : 0,
                 mk == FMARK_FIRST ? IM_COL32(120, 70, 10, 255) : IM_COL32(120, 80, 36, on_star ? 230 : 140), th);
        if (put || h) aside(P.dl, AX2, MY - 0.3f, 2.2f, put ? IM_COL32(120, 80, 36, 255) : IM_COL32(120, 80, 36, on_aside ? 230 : 120), th);
        if (on_star) mark_tip = mk == FMARK_FIRST ? "Starred: first in the list. Click to take the star off." : "Star it: first in the list.";
        else if (on_aside) mark_tip = put ? "Put aside: last in the list, still there. Click to bring it back." : "Put it aside: last in the list, still there.";
        if (P.hit(SX - 2.8f, y + 5.8f, SX + 2.8f, y + ROWH)) formula_mark_set(w.formula, mk == FMARK_FIRST ? FMARK_NONE : FMARK_FIRST);
        else if (P.hit(AX2 - 2.8f, y + 5.8f, AX2 + 2.8f, y + ROWH)) formula_mark_set(w.formula, put ? FMARK_NONE : FMARK_LAST);
        else if (P.hit(LX0, y, LX1, y + ROWH) && !sel) ns_push(AV_SELECT, i, 0, 0);
    }
    if (a.nrows > NROWS) {
        float th2 = (LY1 - LY0) * NROWS / a.nrows, ty = LY0 + (LY1 - LY0 - th2) * V.scroll / (float)(a.nrows - NROWS);
        P.rect(LX1 + 0.6f, ty, LX1 + 1.6f, ty + th2, IM_COL32(120, 80, 36, 140), 0.6f);
    }

    /* ---- the right page: the chosen formula written out */
    const float RX0 = AD_GUTTER + 8, RX1 = AD_BOOK_X1 - 7;   /* (clear of the shade in the gutter) */
    bool risk_hover = false;
    if (sel_f >= 0) {
        int fm = sel_f;
        float y = AD_BOOK_Y0 + 5;
        std::string nm = fm < (int)gamedata.formula_names.size() ? gamedata.formula_names[fm] : "?";
        P.icon(RX0 + 4, y + 4.5f, 8, 95 + fm / 3, -1, fm % 3 + 1);
        float ty0 = y;
        y += P.para(f, NsDraw::FS_HEAD * 0.92f, RX0 + 10, y, RX1 - RX0 - 10, ink, nm) + 0.5f;
        if (y < ty0 + 10.5f) y = ty0 + 10.5f;                        /* (below the flask) */
        static const char *RISK[3] = { "low risk", "medium risk", "high risk" };
        char rl[64]; snprintf(rl, sizeof rl, "Mystic number %d, %s", a.mystic, a.risk >= 0 && a.risk < 3 ? RISK[a.risk] : "?");
        pentacle(P.dl, RX0 + 2.3f, y + 2.7f, 2.3f, IM_COL32(110, 60, 140, 255), th);
        P.text(fo, NsDraw::FS_SMALL, RX0 + 6, y, a.risk >= 2 ? red : soft, rl);
        risk_hover = P.over(RX0, y, RX1, y + 6);
        y += 6.5f;
        if (fm < (int)gamedata.formulas.size()) {
            const std::string &t = gamedata.formulas[fm].text;
            y += P.para(fo, NsDraw::FS_SMALL, RX0, y, RX1 - RX0, IM_COL32(26, 64, 132, 255), game_text(t.c_str(), t.size())) + 0.5f;
            ImU32 ec; std::string enc = ns_item_encounters(95 + fm / 3, &ec);
            if (!enc.empty()) y += P.para(fo, NsDraw::FS_SMALL, RX0, y, RX1 - RX0, ec, enc);
            y += 2;
        }
        {   /* the potion carried already: a flask of each grade the party has, with how many */
            int k = fm / 3; float x = RX0;
            P.text(fo, NsDraw::FS_SMALL, x, y + 0.8f, soft, "Carried"); x += P.width(fo, NsDraw::FS_SMALL, "Carried") + 3;
            bool any = false;
            if (k >= 0 && k < 22) for (int g2 = 0; g2 <= 3; g2++) {
                int nn = pot[k][g2 == 0 ? 0 : g2]; if (g2 == 0 || !nn) continue;
                P.icon(x + 3, y + 3.2f, 6.5f, 95 + k, -1, g2);
                std::string c = std::to_string(nn);
                P.text(fo, NsDraw::FS_SMALL, x + 7.5f, y + 0.8f, ink, c);
                x += 10 + P.width(fo, NsDraw::FS_SMALL, c); any = true;
            }
            if (k >= 0 && k < 22 && pot[k][0]) { P.icon(x + 3, y + 3.2f, 6.5f, 95 + k); std::string c = std::to_string(pot[k][0]); P.text(fo, NsDraw::FS_SMALL, x + 7.5f, y + 0.8f, ink, c); any = true; }
            if (!any) P.text(fo, NsDraw::FS_SMALL, x, y + 0.8f, dim, "none");
            y += 10;
        }
        P.rect(RX0, y - 0.6f, RX1, y - 0.3f, IM_COL32(120, 80, 36, 90));
        P.text(fo, 4.3f, RX0, y + 0.5f, soft, "For a batch: at hand / needed"); y += 6.5f;
        for (int kk = 0; kk < a.ningr; kk++) {
            const AvIngr &q = a.ingr[kk];
            P.icon(RX0 + 3.3f, y + 3.3f, 5.4f, q.item);   /* (a gap between the medallions, and before the name) */
            P.text(fo, NsDraw::FS_BODY, RX0 + 8.5f, y + 0.6f, ink, ns_item_name(q.item), -1, RX1 - 14);
            int hv = q.item >= 0 && q.item < 200 ? have[q.item] : q.have;   /* (what the party carries, as the chest's tags and the batches a row can make: the game's own count leaves out the batches already set) */
            char v[16]; snprintf(v, sizeof v, "%d / %d", hv, q.need);
            P.text(fo, NsDraw::FS_BODY, 0, y + 0.6f, hv >= q.need ? green : red, v, RX1);
            y += 7;
        }
    }

    /* ---- the apothecary's chest: a compartment an ingredient */
    {
        const float X0 = AD_CHEST_X0, X1 = AD_CHEST_X1, Y0 = AD_CHEST_Y0, Y1 = AD_CHEST_Y1, wl = 2.4f;
        const float cx0 = X0 + wl, cy0 = Y0 + wl, cw = (X1 - wl - cx0) / AD_CHEST_COLS, ch = (Y1 - wl - cy0) / AD_CHEST_ROWS;
        /* the label inside the lid */
        P.rect((X0 + X1) * 0.5f - 34, AD_CHEST_LID + 1.6f, (X0 + X1) * 0.5f + 34, Y0 - 1.4f, IM_COL32(236, 222, 186, 255), 0.6f);
        const char *lab = "The party's ingredients";
        P.text(fo, 4.0f, (X0 + X1) * 0.5f - P.width(fo, 4.0f, lab) * 0.5f, AD_CHEST_LID + 2.6f, ink, lab);
        int tip = -1;
        for (int i = 0; i < 23; i++) {
            int id = 72 + i, hv = have[id], need = 0;
            if (sel_f >= 0 && sel_f < (int)gamedata.formulas.size()) for (const FormulaIngr &q : gamedata.formulas[sel_f].ingr) if (q.item == id) need += q.count;
            float x0 = cx0 + (i % AD_CHEST_COLS) * cw + 0.7f, y0 = cy0 + (i / AD_CHEST_COLS) * ch + 0.7f, x1 = x0 + cw - 1.4f, y1 = y0 + ch - 1.4f;
            bool hov = P.over(x0, y0, x1, y1);
            if (hov) { tip = id; P.dl->AddRect(NsDraw::w(x0, y0), NsDraw::w(x1, y1), IM_COL32(250, 214, 130, 255), 0, 0, 1.5f); }
            if (need) P.dl->AddRect(NsDraw::w(x0 + 0.3f, y0 + 0.3f), NsDraw::w(x1 - 0.3f, y1 - 0.3f), hv >= need ? IM_COL32(150, 214, 120, 255) : IM_COL32(240, 110, 86, 255), 0, 0, 2.0f);
            float is = fminf(ch * 0.5f, cw * 0.5f), icx = (x0 + x1) * 0.5f, icy = (y0 + y1) * 0.5f;   /* (in the middle of its compartment) */
            P.icon(icx, icy, is, id);
            if (!hv) P.dl->AddCircleFilled(NsDraw::w(icx, icy), is * 0.72f * P.k, IM_COL32(40, 24, 12, 175));   /* (none: the compartment empty, its sign faint) */
            if (need) {                                              /* a paper tag: needed for a batch, and at hand */
                char tg[16]; snprintf(tg, sizeof tg, "%d/%d", hv, need);
                float tw = P.width(fo, 4.0f, tg) + 2.2f, tx1 = x1 - 0.5f, ty0 = y0 + 1.2f;
                P.rect(tx1 - tw, ty0, tx1, ty0 + 5.6f, IM_COL32(238, 226, 196, 255), 0.6f);
                P.text(fo, 4.0f, tx1 - tw + 1.1f, ty0 + 0.5f, hv >= need ? green : red, tg);
            } else if (hv) {                                         /* how many, in the compartment's lower right corner */
                std::string c = std::to_string(hv);
                float cwid = P.width(fo, 4.4f, c);
                P.text(fo, 4.4f, x1 - 1.0f - cwid, y1 - 5.6f, cream, c);
            }
        }
        if (tip >= 0) {
            std::string use = ns_item_use(tip), where;
            if (have[tip] == 0 && config_get_int("assist.where_sold", 1)) where = journal_where_sold(tip);
            ns_tip_begin();
            ImGui::TextUnformatted(ns_item_name(tip).c_str());
            ImGui::TextDisabled(have[tip] ? "The party carries %d." : "The party has none.", have[tip]);
            if (!where.empty()) ImGui::TextUnformatted(where.c_str());
            if (!use.empty()) ImGui::TextUnformatted(use.c_str());
            ns_tip_end();
        }
    }

    /* ---- the wax tablet: the batches; the chance for each number of them the ingredients allow (a
       click or a drag there sets it); what may come of the brew; what the chance is made of */
    std::string tablet_tip;
    {
        const float X0 = AD_TAB_X0 + 5, X1 = AD_TAB_X1 - 5, Y0 = AD_TAB_Y0 + 4;
        if (sel_f >= 0) {
            int cnt = a.page ? a.count : 0, most = a.most;
            float y = Y0;
            P.text(f, NsDraw::FS_NAME, X0, y + 0.5f, wax, "Batches");
            float bx = X0 + 26;
            if (cnt > 0) { if (P.button(bx, y, bx + 8, y + 7.5f, "-", IM_COL32(120, 84, 40, 235), NsDraw::FS_BODY)) ns_push(AV_FEWER, 0, 0, 0); }
            else P.button_off(bx, y, bx + 8, y + 7.5f, "-", NsDraw::FS_BODY);
            std::string cs = std::to_string(cnt);
            P.text(f, NsDraw::FS_HEAD, bx + 13 - P.width(f, NsDraw::FS_HEAD, cs) * 0.5f, y - 0.4f, wax, cs);
            float px = bx + 18;
            if (a.can_more) { if (P.button(px, y, px + 8, y + 7.5f, "+", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_MORE, 0, 0, 0); }
            else P.button_off(px, y, px + 8, y + 7.5f, "+", NsDraw::FS_BODY);
            P.text(fo, NsDraw::FS_SMALL, 0, y + 1.6f, most ? wax_soft : wax_red, most ? "of " + std::to_string(most) : std::string("none"), X1);
            if (P.over(px + 9, y, X1, y + 7.5f))
                tablet_tip = most ? "The party's ingredients make " + std::to_string(most) + (most == 1 ? " batch" : " batches") + " of this formula at most. A batch makes one potion."
                                  : std::string("The party lacks the ingredients for a batch of this formula (the right page shows what is short).");
            y += 9.0f;

            /* the chance by the number of batches: one roll decides them all */
            const float CY0 = y, CY1 = y + 11;
            int hover_n = 0;
            if (most > 0) {
                float bw = (X1 - X0) / most;
                bool on = P.over(X0, CY0 - 1, X1, CY1 + 1.5f);
                if (on) {
                    hover_n = 1 + (int)((P.mx - X0) / bw); if (hover_n < 1) hover_n = 1; if (hover_n > most) hover_n = most;
                    tablet_tip = "The chance for each number of batches, 1 to " + std::to_string(most) + " (what the ingredients allow). One roll decides them all: "
                                 "more batches, more potions, a lower chance (1 to 3 alike, then 4 and 5, 6 and 7 ...). Click to set the batches; below, what may come of " +
                                 std::to_string(hover_n) + (hover_n == 1 ? " batch." : " batches.");
                }
                for (int n = 1; n <= most; n++) {
                    int c = alchemy_chance(&a, n);
                    float x0 = X0 + (n - 1) * bw, x1 = x0 + bw - (bw > 1.6f ? 0.35f : 0), hh = (CY1 - CY0) * c / 100.0f;
                    ImU32 col = n == cnt ? (c >= 50 ? wax_green : wax_red) : n == hover_n ? wax : IM_COL32(176, 164, 134, 120);
                    if (hh > 0) P.dl->AddRectFilled(NsDraw::w(x0, CY1 - hh), NsDraw::w(x1, CY1), col);
                }
                for (float x = X0; x < X1; x += 2.0f)                     /* (an even chance: dotted) */
                    P.dl->AddRectFilled(NsDraw::w(x, CY0 + (CY1 - CY0) * 0.5f - 0.15f), NsDraw::w(x + 0.7f, CY0 + (CY1 - CY0) * 0.5f + 0.15f), IM_COL32(222, 210, 176, 110));
                P.rect(X0, CY1, X1, CY1 + 0.3f, wax_soft);
                if (cnt > 0 && cnt <= most) {                           /* the batches set: a notch under its bar */
                    float cx = X0 + (cnt - 0.5f) * bw;
                    P.dl->AddTriangleFilled(NsDraw::w(cx, CY1 + 0.4f), NsDraw::w(cx - 1.3f, CY1 + 2.0f), NsDraw::w(cx + 1.3f, CY1 + 2.0f), wax);
                }
                if (!ns_in.down) V.drag = false;
                else if (ns_in.px >= X0 && ns_in.px < X1 && ns_in.py >= CY0 - 1 && ns_in.py < CY1 + 1.5f) V.drag = true;   /* (went down on it) */
                if (P.hit(X0, CY0 - 1, X1, CY1 + 1.5f)) V.drag = true;
                if (V.drag) {
                    int n = 1 + (int)((P.mx - X0) / bw); if (n < 1) n = 1; if (n > most) n = most;
                    if (n != cnt) ns_push(AV_SET, n, 0, 0);
                    hover_n = n;
                }
            } else P.para(fo, NsDraw::FS_SMALL, X0, CY0 + 2, X1 - X0, wax_red, "Not enough ingredients for one batch.");
            y = CY1 + 2.6f;

            /* what may come of it (the number pointed at in the chart, else the batches set) */
            int sn = hover_n ? hover_n : cnt;
            if (sn > 0) {
                float ok = (float)alchemy_chance(&a, sn), fail = 100 - ok;
                int num, den; fixes_alchemy_blast_odds(a.risk, &num, &den);
                float mis = a.at_place ? fail * 0.5f : 0, bl = mis * num / den, st = mis - bl, ls = fail - mis;
                struct { float v; int kind; } parts[4] = { { ok, 0 }, { ls, 1 }, { st, 2 }, { bl, 3 } };
                float x = X0, iy = y + 2.6f;
                for (auto &pt : parts) {
                    if (pt.kind >= 2 && !a.at_place) continue;
                    float x0 = x;
                    if (pt.kind == 0) P.icon(x + 2.6f, iy, 6.0f, 95 + sel_f / 3, -1, sel_f % 3 + 1);
                    else if (pt.kind == 1) lost(P.dl, x + 2.4f, iy, 2.3f, a.ningr ? a.ingr[0].item : 72, th);
                    else if (pt.kind == 2) stink(P.dl, x + 2.4f, iy + 0.4f, 2.3f, th);
                    else blast(P.dl, x + 2.4f, iy, 2.5f);
                    std::string v = pct_text(pt.v);
                    ImU32 vc = pt.kind == 0 ? (ok >= 50 ? wax_green : wax_red) : pt.kind == 3 && pt.v > 0 ? wax_red : wax;
                    P.text(pt.kind == 0 ? f : fo, pt.kind == 0 ? NsDraw::FS_NAME : NsDraw::FS_SMALL, x + 5.6f, iy - (pt.kind == 0 ? 3.2f : 2.6f), vc, v);
                    x += 5.6f + P.width(pt.kind == 0 ? f : fo, pt.kind == 0 ? NsDraw::FS_NAME : NsDraw::FS_SMALL, v) + 2.6f;
                    if (P.over(x0, iy - 3.2f, x - 1, iy + 3.2f)) {
                        char b[400]; std::string bn = std::to_string(sn) + (sn == 1 ? " batch" : " batches");
                        if (pt.kind == 0) snprintf(b, sizeof b, "Success, a chance of %d%%: %d %s of %s. One roll decides all %s.", (int)ok, sn, sn == 1 ? "potion" : "potions",
                                                   gamedata.formula_names[sel_f].c_str(), bn.c_str());
                        else if (pt.kind == 1) snprintf(b, sizeof b, a.at_place ? "Failure and no more: the ingredients of the %s are lost (half the failures here)." : "Failure: the ingredients of the %s are lost; in a camp in the wild nothing worse can happen.", bn.c_str());
                        else if (pt.kind == 2) snprintf(b, sizeof b, "Failure, and a stink: the ingredients lost, and no room at this inn for a month.");
                        else snprintf(b, sizeof b, "Failure, and an explosion: the ingredients lost, the party hurt, damages to pay, and no room at this inn for four months.");
                        tablet_tip = b;
                    }
                }
                y = iy + 3.6f;
            } else { P.text(fo, NsDraw::FS_SMALL, X0, y, wax_soft, "No batch set."); y += 6.5f; }

            /* what the chance is made of, in pictures; or a word from the screen */
            if (a.note[0] && plat_ticks_ms() - a.note_t < 6000) P.para(fo, NsDraw::FS_SMALL, X0, y, X1 - X0, wax, ns_gtext(a.note));
            else {
                int nb = sn > 0 ? sn : 1, bp = alchemy_batch_part(nb), sum = bp + a.intel + a.alch + a.stone - a.mystic;
                float x = X0, cy = y + 2.6f; const float fs = 4.4f;
                auto num = [&](const char *op, int v) {
                    if (op) { P.text(fo, fs, x, cy - 2.4f, wax_soft, op); x += P.width(fo, fs, op) + 0.8f; }
                    std::string s = std::to_string(v); P.text(fo, fs, x, cy - 2.4f, wax, s); x += P.width(fo, fs, s) + 1.2f;
                };
                falling_bars(P.dl, x + 2, cy, 2.0f, wax); x += 4.6f; num(nullptr, bp);
                ui_attr_icon(P.dl, NsDraw::w(x + 2.6f, cy), 3.4f * P.k, ATTR_INT); x += 5.2f; num("+", a.intel);
                ui_skill_icon(P.dl, NsDraw::w(x + 2.6f, cy), 3.4f * P.k, SKILL_ALCHEMY); x += 5.2f; num("+", a.alch);
                ui_pstone_draw(P.dl, NsDraw::w(x + 2.4f, cy), 4.6f * P.k, ui_pstone_look(), a.stone > 0 ? -a.stone : 0, (float)ImGui::GetTime()); x += 5.0f; num("+", a.stone);
                pentacle(P.dl, x + 2.2f, cy, 2.1f, IM_COL32(190, 150, 230, 255), th); x += 4.6f; num("-", a.mystic);
                char eq[16]; snprintf(eq, sizeof eq, "= %d%%", sum < 0 ? 0 : sum > 99 ? 99 : sum);
                P.text(fo, fs, x, cy - 2.4f, wax, eq);
                if (P.over(X0, cy - 3, X1, cy + 3)) {
                    char b[600];
                    snprintf(b, sizeof b, "The chance: %d for %d %s (100 for 1 to 3, 50 for 4 or 5, 33 for 6 or 7, 25 for 8 or 9 ...: 100 divided by half the batches) "
                             "+ %s's Intelligence %d + Alchemy %d + the party's Philosopher's Stone %d - the formula's mystic number %d = %d, at most 99%%.",
                             bp, nb, nb == 1 ? "batch" : "batches", who.c_str(), a.intel, a.alch, a.stone, a.mystic, sum);
                    tablet_tip = b;
                }
            }
        } else P.para(fo, NsDraw::FS_SMALL, X0, Y0 + 2, X1 - X0, wax_soft, "Choose a formula in the book.");
    }

    /* ---- Start, on the furnace */
    {
        bool can = sel_f >= 0 && a.page && a.count > 0;
        if (can) { if (P.button(AD_START_X0, AD_START_Y0, AD_START_X1, AD_START_Y1, "Start mixing", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_START, 0, 0, 0); }
        else P.button_off(AD_START_X0, AD_START_Y0, AD_START_X1, AD_START_Y1, "Start mixing", NsDraw::FS_BODY);
    }
    if (mark_tip) ns_tip(mark_tip);
    else if (!tablet_tip.empty()) ns_tip(tablet_tip.c_str());
    else if (risk_hover) {             /* what the risk means (mods/fixes.c, alchrisk) */
        static const char *IN[5] = { "", "one time", "two times", "three times", "" }, *OF[5] = { "", "", "two", "three", "four" };
        int num, den; fixes_alchemy_blast_odds(a.risk, &num, &den);
        char t[400];
        snprintf(t, sizeof t, "The mystic number: the higher, the harder the formula is to brew. The risk is what a failed batch can cause. At an inn, in a village or a hideout, one failure in two ends in a mishap: "
                 "%s in %s it is an explosion (the party hurt, damages to pay, no room at the inn for four months), otherwise a stink (no room for a month). "
                 "In a camp in the wild a failure only costs the ingredients.", IN[num], OF[den]);
        ns_tip(t);
    }
}
