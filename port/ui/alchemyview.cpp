/* alchemyview.cpp - draws the alchemy screen (mods/alchemyview.c runs it; docs/NATIVE_SCREENS.md)
 * on an alchemist's workbench (ui/alchemydesk.cpp): the formulary open, its left page the index
 * of the formulas the alchemist knows (how many batches the party has the makings of), its right
 * page the chosen one written out (what it does, its risk, its ingredients needed and at hand);
 * the apothecary's chest beside it, a compartment for each of the 23 ingredients with what the
 * party carries, those the formula needs tagged; the wax tablet below with the batches (- and +)
 * and the chance of success reckoned; Start on the furnace. Drawn from alchemy_model. */
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
void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t);
int ui_pstone_look(void);
const char *ui_pstone_stage_name(int q);
#include "../mods/gamemodel.h"
#include "alchemydesk.h"
#include <map>
std::string journal_where_sold(int id);              /* ui/journal.cpp */

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
int ui_potion_tier(int q, int base);   /* ui/alchemyicons.cpp */

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
    const ImU32 cream = IM_COL32(244, 230, 196, 255), cream_soft = IM_COL32(226, 204, 160, 235);
    const ImU32 wax = IM_COL32(222, 210, 176, 255), wax_soft = IM_COL32(176, 164, 134, 230), wax_red = IM_COL32(240, 120, 96, 255), wax_green = IM_COL32(150, 214, 120, 255);
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

    /* the head, on the table */
    P.rect(63, 2, 214, 19, IM_COL32(24, 12, 4, 150), 1.5f);
    P.text(f, NsDraw::FS_TITLE, 65, 3, cream, "Alchemy");
    P.text(fo, NsDraw::FS_BODY, 65, 12.5f, cream_soft, ns_gtext(a.name) + " mixes; the whole party's ingredients count.", -1, 213);
    {   /* the party's Philosopher's Stone, on its own plaque: what it adds to every batch */
        P.rect(216, 2, 279, 19, IM_COL32(24, 12, 4, 170), 1.5f);
        ui_pstone_draw(P.dl, NsDraw::w(225, 10.5f), 15 * P.k, ui_pstone_look(), a.stone, (float)ImGui::GetTime());
        P.text(f, NsDraw::FS_NAME, 233, 3, a.stone ? IM_COL32(255, 196, 150, 255) : cream_soft, a.stone ? "Stone +" + std::to_string(a.stone) : std::string("No Stone"));
        P.text(fo, NsDraw::FS_SMALL, 233, 11.5f, cream_soft, a.stone ? "to every batch" : "yet", -1, 278);
        if (P.over(216, 2, 279, 19))
            ns_tip(a.stone ? ("The party's Philosopher's Stone, of quality " + std::to_string(a.stone) + ": it adds " + std::to_string(a.stone) + " to the chance of every batch, whoever mixes. Its picture shows the stage of the alchemists' Great Work its quality has reached: " + std::string(ui_pstone_stage_name(a.stone)) + ". An alchemist (up to 25) or a university (up to 29, depending on the town) can make it better, for a price.").c_str()
                           : "The party has no Philosopher's Stone yet. An alchemist (up to quality 25) or a university (up to 29) makes one, for a price; its quality adds to the chance of every batch.");
    }
    if (P.button(282, 4, 316, 16, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(AV_LEAVE, 0, 0, 0);

    /* ---- the formulary's left page: the index */
    const float LX0 = AD_BOOK_X0 + 6, LX1 = AD_GUTTER - 4, LY0 = AD_BOOK_Y0 + 11, ROWH = 12.5f; const int NROWS = 7;
    const float LY1 = LY0 + NROWS * ROWH;
    P.text(fo, 4.3f, LX0 + 1, AD_BOOK_Y0 + 5, soft, "Formulae known");
    if (P.over(LX0, LY0, LX1, LY1) && ns_in.wheel != 0) { V.scroll -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = a.nrows - NROWS; if (maxs < 0) maxs = 0;
    if (V.scroll > maxs) V.scroll = maxs;
    if (V.scroll < 0) V.scroll = 0;
    if (!a.nrows) P.para(fo, NsDraw::FS_BODY, LX0 + 2, LY0 + 2, LX1 - LX0 - 4, dim, ns_gtext(a.name) + " knows no formulae yet.");
    for (int r = 0; r < NROWS && V.scroll + r < a.nrows; r++) {
        int i = V.scroll + r;
        const AvRow &w = a.row[i];
        float y = LY0 + r * ROWH;
        bool h = P.over(LX0, y, LX1, y + ROWH), sel = i == a.sel;
        if (sel) P.rect(LX0, y + 0.3f, LX1, y + ROWH - 0.3f, IM_COL32(150, 60, 40, 70), 1.5f);
        else if (h) P.rect(LX0, y + 0.3f, LX1, y + ROWH - 0.3f, IM_COL32(150, 60, 40, 36), 1.5f);
        P.icon(LX0 + 5, y + ROWH * 0.5f, 7.5f, 95 + w.formula / 3, -1, w.formula % 3 + 1);
        std::string nm = w.formula < (int)gamedata.formula_names.size() ? gamedata.formula_names[w.formula] : "?";
        int bt = batches_of(w.formula);
        int pc = potions_of(w.formula);                                   /* (the potion carried already, any grade) */
        std::string pcs = pc ? std::to_string(pc) + " carried" : "";
        float pcw = pc ? P.width(fo, 4.2f, pcs) + 2 : 0;
        P.text(f, 5.0f, LX0 + 10.5f, y + 0.9f, bt || w.count ? ink : dim, nm, -1, LX1 - 1 - pcw);
        if (pc) P.text(fo, 4.2f, 0, y + 1.6f, soft, pcs, LX1 - 1);
        std::string l2;
        if (bt) l2 = "makings of " + std::to_string(bt) + (bt == 1 ? " batch" : " batches");
        else if (w.formula < (int)gamedata.formulas.size()) {                /* what is short: the first, and how many more */
            int nshort = 0; std::string first;
            for (const FormulaIngr &q : gamedata.formulas[w.formula].ingr) if (q.item >= 0 && q.item < 200 && have[q.item] < q.count) { if (!nshort) first = ns_item_name(q.item); nshort++; }
            l2 = "needs " + first + (nshort > 1 ? " and " + std::to_string(nshort - 1) + " more" : "");
        }
        if (w.count) l2 = std::to_string(w.count) + (w.count == 1 ? " batch set" : " batches set") + " - " + l2;
        P.text(fo, 4.2f, LX0 + 10.5f, y + 6.8f, bt ? green : red, l2, -1, LX1 - 1);
        if (P.hit(LX0, y, LX1, y + ROWH) && !sel) ns_push(AV_SELECT, i, 0, 0);
    }
    if (a.nrows > NROWS) {
        float th = (LY1 - LY0) * NROWS / a.nrows, ty = LY0 + (LY1 - LY0 - th) * V.scroll / (float)(a.nrows - NROWS);
        P.rect(LX1 + 0.6f, ty, LX1 + 1.6f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
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
        P.text(fo, NsDraw::FS_SMALL, RX0, y, a.risk >= 2 ? red : soft, rl);
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
            char v[16]; snprintf(v, sizeof v, "%d / %d", q.have, q.need);
            P.text(fo, NsDraw::FS_BODY, 0, y + 0.6f, q.have >= q.need ? green : red, v, RX1);
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

    /* ---- the wax tablet: the batches, and the chance reckoned */
    {
        const float X0 = AD_TAB_X0 + 5, X1 = AD_TAB_X1 - 5, Y0 = AD_TAB_Y0 + 4;
        if (sel_f >= 0) {
            float y = Y0;
            P.text(f, NsDraw::FS_NAME, X0, y + 0.5f, wax, "Batches");
            float bx = X0 + 26;
            if (a.page && a.count > 0) { if (P.button(bx, y, bx + 8, y + 7.5f, "-", IM_COL32(120, 84, 40, 235), NsDraw::FS_BODY)) ns_push(AV_FEWER, 0, 0, 0); }
            else P.button_off(bx, y, bx + 8, y + 7.5f, "-", NsDraw::FS_BODY);
            std::string cnt = std::to_string(a.page ? a.count : 0);
            P.text(f, NsDraw::FS_HEAD, bx + 13 - P.width(f, NsDraw::FS_HEAD, cnt) * 0.5f, y - 0.4f, wax, cnt);
            float px = bx + 18;
            if (a.can_more) { if (P.button(px, y, px + 8, y + 7.5f, "+", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_MORE, 0, 0, 0); }
            else P.button_off(px, y, px + 8, y + 7.5f, "+", NsDraw::FS_BODY);
            y += 9.0f;
            if (!a.can_more) { P.text(fo, NsDraw::FS_SMALL, X0, y, wax_red, "not enough for more", -1, X1); y += 6.0f; }
            int pc = a.page && a.count > 0 ? a.pct : a.pct1;
            char chs[64]; snprintf(chs, sizeof chs, a.page && a.count > 0 ? "Chance %d%%" : "Chance, one batch: %d%%", pc);
            P.text(f, NsDraw::FS_NAME, X0, y, pc >= 50 ? wax_green : wax_red, chs);
            y += 7.5f;
            if (a.note[0] && plat_ticks_ms() - a.note_t < 6000) P.para(fo, NsDraw::FS_SMALL, X0, y, X1 - X0, wax, ns_gtext(a.note));
            else {
                char parts[120]; snprintf(parts, sizeof parts, "%d (fewer batches: higher) + %d Int + %d Alchemy + %d Stone - %d mystic",
                                          a.base, a.intel, a.alch, a.stone, a.mystic);
                P.para(fo, 4.0f, X0, y, X1 - X0, wax_soft, parts);
            }
        } else P.para(fo, NsDraw::FS_SMALL, X0, Y0 + 2, X1 - X0, wax_soft, "Choose a formula in the book.");
    }

    /* ---- Start, on the furnace */
    {
        bool can = sel_f >= 0 && a.page && a.count > 0;
        if (can) { if (P.button(AD_START_X0, AD_START_Y0, AD_START_X1, AD_START_Y1, "Start mixing", IM_COL32(40, 92, 34, 235), NsDraw::FS_BODY)) ns_push(AV_START, 0, 0, 0); }
        else P.button_off(AD_START_X0, AD_START_Y0, AD_START_X1, AD_START_Y1, "Start mixing", NsDraw::FS_BODY);
    }
    if (risk_hover) {                  /* what the risk means (mods/fixes.c, alchrisk) */
        static const char *IN[5] = { "", "one time", "two times", "three times", "" }, *OF[5] = { "", "", "two", "three", "four" };
        int num, den; fixes_alchemy_blast_odds(a.risk, &num, &den);
        char t[400];
        snprintf(t, sizeof t, "The mystic number: the higher, the harder the formula is to brew. The risk is what a failed batch can cause. At an inn, in a village or a hideout, one failure in two ends in a mishap: "
                 "%s in %s it is an explosion (the party hurt, damages to pay, no room at the inn for four months), otherwise a stink (no room for a month). "
                 "In a camp in the wild a failure only costs the ingredients.", IN[num], OF[den]);
        ns_tip(t);
    }
}
