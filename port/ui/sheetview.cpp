/* sheetview.cpp - draws a member's sheet (mods/sheetview.c runs it; docs/NATIVE_SCREENS.md).
 *
 * Left: the game's own paper doll of the member in their armour and arms, with the load under
 * it, as the original draws it (kept: the game's finest art; redrawn by the game after each
 * change). Right, in tabs: the whole pack (Ready / Put away / Drink / Give / Drop on the row
 * under the pointer), the attributes and skills, the formulae and the saints the member knows.
 * A fixed strip at the bottom describes what the pointer is on, and says what the last action
 * did. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/sheetview.h"
#include "nsdraw.h"

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
const char *ui_attr_help(int a);
const char *ui_skill_help(int k);
std::string rules_weapons_of_skill(int k);

namespace {
const char *ATTR[7] = { "Endurance", "Strength", "Agility", "Perception", "Intellect", "Charisma", "Divine Favor" };
const char *SKILL[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows", "Missile Devices",
    "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing", "Artifice", "Stealth",
    "Streetwise", "Riding", "Woodwise" };
enum { TAB_THINGS, TAB_STATS, TAB_FORMULAE, TAB_SAINTS };
struct SheetView {
    int tab = TAB_THINGS;
    int scroll[4] = {};
    int last_who = -1;
    /* what the strip describes: 1 an item (row), 2 a skill, 3 a formula, 4 a saint, 5 an attribute */
    int what = 0, which = -1; unsigned seq = 0;
    int give_row = -1, drop_row = -1; unsigned act_seq = 0;      /* a row asking "to whom?" / "sure?" */
} V;
bool knows_formula(const SheetModel &s, int f) { return f >= 0 && f / 3 < 22 && ((s.formulas[f / 3] >> (f % 3)) & 1); }
bool knows_saint(const SheetModel &s, int t) { return t >= 0 && t < 160 && (s.saints[t >> 3] & (0x80 >> (t & 7))); }
}

extern "C" int sheetview_showing(void) { return ns_screen == NS_SHEET && sheet_model.open && !ns_hidden(); }

extern "C" void sheetview_draw(void)
{
    const SheetModel &s = sheet_model;
    if (!sheetview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM, red = NsDraw::RED, green = NsDraw::GREEN;
    if (s.who != V.last_who) { V.scroll[TAB_THINGS] = 0; V.give_row = V.drop_row = -1; V.what = 0; V.last_who = s.who; }
    if (V.act_seq != s.seq) { V.give_row = V.drop_row = -1; }
    int new_what = 0, new_which = -1;

    /* the right half: the doll (game x 60..198) stays the game's own */
    const float X0 = 199, X1 = 318;
    P.rect(X0 - 0.5f, 0, 320, 200, IM_COL32(236, 222, 188, 255));
    P.rect(192, 0, X0, 12.5f, IM_COL32(236, 222, 188, 255));       /* (the end of the original's name bar: "Leader" cut in two) */
    P.rect(X0, 1.5f, X1 + 0.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
    P.frame(X0, 1.5f, X1 + 0.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);

    /* the header */
    P.text(f, 7.0f, X0 + 3, 3, ink, ns_gtext(s.fullname), -1, X1 - 2);
    char sub[64]; snprintf(sub, sizeof sub, "%s, age %d", s.female ? "Woman" : "Man", s.age);
    P.text(fo, 5.0f, X0 + 3, 11.5f, soft, sub);
    if (s.who == s.leader) {
        P.rect(X1 - 52, 11, X1 - 26, 19.5f, IM_COL32(214, 172, 60, 255), 1.5f);
        P.text(fo, 4.8f, X1 - 48.5f, 12.6f, ink, "Leader");
    } else if (P.button(X1 - 52, 11, X1 - 26, 19.5f, "Lead", IM_COL32(120, 84, 40, 235), 4.8f)) ns_push(SV_LEADER, s.who, 0, 0);
    if (P.button(X1 - 24, 11, X1 - 2, 19.5f, "Close", IM_COL32(112, 40, 22, 235), 4.8f)) ns_push(SV_CLOSE, 0, 0, 0);

    /* the tabs */
    const float RX0 = X0 + 2, RX1 = X1 - 2, LY0 = 33, ROWH = 8.6f; const int NROWS = 13;
    const float LY1 = LY0 + NROWS * ROWH;
    {
        static const char *TABS[4] = { "Things", "Stats", "Formulae", "Saints" };
        float tw = (RX1 - RX0) / 4, x = RX0;
        for (int t = 0; t < 4; t++) {
            bool sel = t == V.tab, h = P.over(x, 22, x + tw - 0.8f, 31);
            P.rect(x, 22, x + tw - 0.8f, 31, sel ? IM_COL32(112, 40, 22, 240) : h ? IM_COL32(214, 190, 150, 255) : IM_COL32(222, 204, 168, 255), 1.5f);
            P.text(f, 5.0f, x + (tw - 0.8f - P.width(f, 5.0f, TABS[t])) * 0.5f, 23.6f, sel ? NsDraw::LIGHT : ink, TABS[t]);
            if (P.hit(x, 22, x + tw - 0.8f, 31)) { V.tab = t; V.give_row = V.drop_row = -1; V.what = 0; }
            x += tw;
        }
    }
    P.rect(RX0, LY0 - 1, RX1, LY1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
    P.frame(RX0, LY0 - 1, RX1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);

    /* the rows of the tab: Things (items), Stats (7 attributes then 19 skills), Formulae, Saints */
    std::vector<int> rows;
    if (V.tab == TAB_THINGS) for (int i = 0; i < s.nitems; i++) rows.push_back(i);
    else if (V.tab == TAB_STATS) for (int i = 0; i < 26; i++) rows.push_back(i);
    else if (V.tab == TAB_FORMULAE) { for (int fm = 0; fm < (int)gamedata.formula_names.size() && fm < 66; fm++) if (knows_formula(s, fm)) rows.push_back(fm); }
    else for (int t = 0; t < (int)gamedata.saints.size(); t++) if (knows_saint(s, t)) rows.push_back(t);
    int n = (int)rows.size();
    int &sc = V.scroll[V.tab];
    if (ns_in.wheel != 0) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    if (!n) P.text(fo, 5.2f, RX0 + 3, LY0 + 2, dim, V.tab == TAB_THINGS ? "Carries nothing." : V.tab == TAB_FORMULAE ? "Knows no formulae." : "Knows no saints.");
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int i = rows[sc + r];
        float y = LY0 + r * ROWH;
        bool h = P.over(RX0, y, RX1, y + ROWH);
        bool busy = V.tab == TAB_THINGS && (i == V.give_row || i == V.drop_row);
        if (h || busy) P.rect(RX0 + 0.5f, y, RX1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
        else if (r & 1) P.rect(RX0 + 0.5f, y, RX1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
        if (V.tab == TAB_STATS) {
            if (i < 7) {
                if (h) { new_what = 5; new_which = i; }
                int c = s.attr[i], m = s.attr_max[i] > 0 ? s.attr_max[i] : 1;
                P.text(fo, 5.0f, RX0 + 3, y + 1.7f, ink, ATTR[i]);
                float bx0 = RX0 + 50, bx1 = RX1 - 24, fill = fminf(1.0f, fmaxf(0.0f, (float)c / m));
                P.rect(bx0, y + 3, bx1, y + 5.4f, IM_COL32(214, 196, 160, 255), 1);
                P.rect(bx0, y + 3, bx0 + (bx1 - bx0) * fill, y + 5.4f, c < m ? IM_COL32(170, 60, 36, 255) : IM_COL32(96, 120, 60, 255), 1);
                char v[16]; snprintf(v, sizeof v, "%d/%d", c, s.attr_max[i]);
                P.text(fo, 5.0f, 0, y + 1.7f, c < s.attr_max[i] ? red : ink, v, RX1 - 3);
            } else {
                int k2 = i - 7;
                if (h) { new_what = 2; new_which = k2; }
                if (k2 == 0) P.rect(RX0 + 2, y + 0.2f, RX1 - 2, y + 0.6f, IM_COL32(120, 80, 36, 120));
                P.text(fo, 5.0f, RX0 + 3, y + 1.7f, ink, SKILL[k2]);
                P.text(f, 5.6f, 0, y + 1.3f, ink, std::to_string(s.skill[k2]), RX1 - 3);
            }
            continue;
        }
        if (V.tab == TAB_FORMULAE) {
            if (h) { new_what = 3; new_which = i; }
            ui_item_icon(P.dl, NsDraw::w(RX0 + 5.5f, y + ROWH * 0.5f), 6.5f * P.k, 95 + i / 3);
            P.text(f, 5.4f, RX0 + 11, y + 1.3f, ink, gamedata.formula_names[i], -1, RX1 - 2);
            continue;
        }
        if (V.tab == TAB_SAINTS) {
            if (h) { new_what = 4; new_which = i; }
            P.text(f, 5.4f, RX0 + 3, y + 1.3f, ink, gamedata.saints[i].name, -1, RX1 - 2);
            continue;
        }
        const SvItem &it = s.item[i];
        if (h) { new_what = 1; new_which = i; }
        if (it.equipped) P.rect(RX0 + 0.5f, y + 1, RX0 + 1.8f, y + ROWH - 1, red);
        ui_item_icon(P.dl, NsDraw::w(RX0 + 5.5f, y + ROWH * 0.5f), 6.5f * P.k, it.id);
        std::string nm = ns_item_name(it.id);
        if (it.count > 1) nm += " (" + std::to_string(it.count) + ")";
        if (V.give_row == i) {                         /* to whom? */
            P.text(fo, 4.6f, RX0 + 11, y + 2.2f, ink, "To:");
            float x = RX0 + 11 + P.width(fo, 4.6f, "To:") + 2;
            for (int m = 0; m < 5; m++) {
                if (m == s.who || !s.present[m]) continue;
                std::string mn = ns_gtext(s.names[m]);
                float bw = P.width(fo, 4.4f, mn) + 3;
                if (x + bw > RX1 - 8) break;
                if (P.button(x, y + 1, x + bw, y + ROWH - 1, mn, IM_COL32(40, 92, 34, 235), 4.4f)) ns_push(SV_GIVE, i, m, P.shift ? 255 : 1);
                x += bw + 1.2f;
            }
            if (P.button(RX1 - 7, y + 1, RX1 - 1.5f, y + ROWH - 1, "x", IM_COL32(120, 84, 40, 235), 4.6f)) V.give_row = -1;
            continue;
        }
        char qs[16]; snprintf(qs, sizeof qs, "q%d", it.qual);
        float name_end = (h || busy) ? RX1 - 53 : RX1 - 3 - P.width(fo, 4.8f, qs) - 2;
        P.text(f, 5.4f, RX0 + 11, y + 1.3f, it.equipped ? IM_COL32(112, 30, 16, 255) : ink, nm, -1, name_end);
        if (!(h || busy)) { P.text(fo, 4.8f, 0, y + 2.1f, soft, qs, RX1 - 3); continue; }
        /* the actions */
        float x = RX1 - 52;
        const ItemDef *d = ns_itemdef(it.id);
        bool gear = d && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD);
        if (it.drinkable) { if (P.button(x, y + 1, x + 16, y + ROWH - 1, "Drink", IM_COL32(52, 70, 120, 235), 4.3f)) ns_push(SV_DRINK, i, 0, 0); }
        else if (it.equipped) { if (P.button(x, y + 1, x + 16, y + ROWH - 1, "Away", IM_COL32(120, 84, 40, 235), 4.3f)) ns_push(SV_PUTAWAY, i, 0, 0); }
        else if (gear) { if (P.button(x, y + 1, x + 16, y + ROWH - 1, "Ready", IM_COL32(40, 92, 34, 235), 4.3f)) ns_push(SV_READY, i, 0, 0); }
        x += 17;
        if (P.button(x, y + 1, x + 15, y + ROWH - 1, "Give", IM_COL32(40, 92, 34, 235), 4.3f)) { V.give_row = i; V.drop_row = -1; V.act_seq = s.seq; }
        x += 16;
        if (V.drop_row == i) {
            if (P.button(x, y + 1, x + 18, y + ROWH - 1, P.shift ? "All?" : "Sure?", IM_COL32(150, 30, 18, 255), 4.3f)) { ns_push(SV_DROP, i, P.shift ? 255 : 1, 0); V.drop_row = -1; }
        } else if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Drop", IM_COL32(120, 44, 24, 235), 4.3f)) { V.drop_row = i; V.give_row = -1; V.act_seq = s.seq; }
    }
    if (n > NROWS) {
        float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(RX1 - 1.4f, ty, RX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
    if (new_what) { V.what = new_what; V.which = new_which; V.seq = s.seq; }
    else if (V.seq != s.seq) V.what = 0;

    /* the strip */
    const float SX0 = RX0, SX1 = RX1, SY0 = LY1 + 4, SY1 = 197, SW = SX1 - SX0 - 5;
    P.rect(SX0, SY0 - 1.5f, SX1, SY1, IM_COL32(230, 214, 178, 255), 1.5f);
    float y = SY0;
    if (V.what == 1 && V.which < s.nitems) {
        const SvItem &it = s.item[V.which];
        const ItemDef *d = ns_itemdef(it.id);
        y += P.para(f, 6.0f, SX0 + 2.5f, y, SW, ink, ns_item_name(it.id)) + 0.5f;
        std::string l1 = std::string(d ? kind_name(d->kind) : "") + ", quality " + std::to_string(it.qual) + ", weight " + std::to_string(it.weight);
        if (it.count > 1) l1 += " each, " + std::to_string(it.count) + " of them";
        if (it.equipped) l1 += "; in use";
        y += P.para(fo, 4.8f, SX0 + 2.5f, y, SW, soft, l1) + 0.5f;
        std::string l2 = ns_item_line(it.id);
        if (!l2.empty()) y += P.para(fo, 4.8f, SX0 + 2.5f, y, SW, soft, l2) + 0.5f;
        if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
            int sk = rb(G_FAR20A5, (u16)(0x75ac + d->type)), minsk = rb(G_FAR20A5, (u16)(0x7726 + d->type)), minst = rb(G_FAR20A5, (u16)(0x7765 + d->type));
            int have = sk < 7 ? s.skill[sk] : 0;
            std::string fit = (have < minsk ? "lacks " + std::to_string(minsk - have) + " skill" : std::string("")) +
                              (s.attr[1] < minst ? std::string(have < minsk ? ", " : "") + "too weak by " + std::to_string(minst - s.attr[1]) : "");
            y += P.para(fo, 4.8f, SX0 + 2.5f, y, SW, fit.empty() ? green : red, fit.empty() ? ns_gtext(s.names[s.who]) + " can use it well" : ns_gtext(s.names[s.who]) + ": " + fit);
        } else if (l2.empty() && it.drinkable && it.id >= 95 && 3 * (it.id - 95) < (int)gamedata.formulas.size())
            P.para(fo, 4.6f, SX0 + 2.5f, y, SW, soft, game_text(gamedata.formulas[3 * (it.id - 95)].text.c_str(), gamedata.formulas[3 * (it.id - 95)].text.size()));
    } else if (V.what == 2) {
        std::string help = ui_skill_help(V.which);
        if (V.which < 7) { std::string w = rules_weapons_of_skill(V.which); help = "Attack and defence with " + (w.empty() ? std::string("these weapons") : w) + "."; }
        y += P.para(f, 6.0f, SX0 + 2.5f, y, SW, ink, SKILL[V.which]) + 0.5f;
        P.para(fo, 4.8f, SX0 + 2.5f, y, SW, soft, help);
    } else if (V.what == 3 && V.which < (int)gamedata.formulas.size()) {
        const Formula &fm = gamedata.formulas[V.which];
        y += P.para(f, 6.0f, SX0 + 2.5f, y, SW, ink, gamedata.formula_names[V.which]) + 0.5f;
        std::string ing;
        for (const FormulaIngr &g : fm.ingr) ing += (ing.empty() ? "" : ", ") + std::to_string(g.count) + " " + ns_item_name(g.item);
        static const char *RISK[3] = { "low risk", "some risk", "high risk" };
        char mr[48]; snprintf(mr, sizeof mr, "Mystic number %d, %s", fm.mystic, fm.risk >= 0 && fm.risk < 3 ? RISK[fm.risk] : "?");
        y += P.para(fo, 4.6f, SX0 + 2.5f, y, SW, soft, std::string(mr) + (ing.empty() ? "" : ": " + ing)) + 0.5f;
        P.para(fo, 4.6f, SX0 + 2.5f, y, SW, soft, game_text(fm.text.c_str(), fm.text.size()));
    } else if (V.what == 4 && V.which < (int)gamedata.saints.size()) {
        y += P.para(f, 6.0f, SX0 + 2.5f, y, SW, ink, gamedata.saints[V.which].name) + 0.5f;
        P.para(fo, 4.3f, SX0 + 2.5f, y, SW, soft, gamedata.saints[V.which].text);
    } else if (V.what == 5) {
        y += P.para(f, 6.0f, SX0 + 2.5f, y, SW, ink, ATTR[V.which]) + 0.5f;
        P.para(fo, 4.8f, SX0 + 2.5f, y, SW, soft, ui_attr_help(V.which));
    } else P.para(fo, 4.8f, SX0 + 2.5f, y + 1, SW, dim, "Point at anything to see it here. This member's portrait, Esc or Close: back.");
    if (s.note[0] && plat_ticks_ms() - s.note_t < 6000) {
        P.rect(SX0 + 1, SY1 - 8.6f, SX1 - 1, SY1 - 0.6f, IM_COL32(230, 214, 178, 255));
        P.text(fo, 4.8f, SX0 + 2.5f, SY1 - 7.8f, IM_COL32(112, 40, 22, 255), ns_gtext(s.note), -1, SX1 - 1);
    }
}
