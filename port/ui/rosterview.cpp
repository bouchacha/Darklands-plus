// rosterview.cpp - the party of a new game, drawn by Darklands+ (mods/rosterview.c runs it). Top:
// the four places of the party, each member's figure in their colours with their name and
// numbers. Below: the characters kept (click to choose, double-click to add or drop), and the
// chosen one: attributes, best skills, what they know and carry, and what can be done with them
// (add or drop, the sheet, the figure, the herald and the colours, delete). New character and
// Begin along the bottom.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <algorithm>
#include "nsdraw.h"
#include "gamepics.h"
#include "skillicons.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/rosterview.h"
#include "../mods/gamemodel.h"
u32 plat_ticks_ms(void);
int figpick_is_open(void);
}
const GamePic *ns_figure_pic(int cache, const char *code, const unsigned char *mp, const char *suffix);

static const char *ATTR_S[7] = { "End", "Str", "Agl", "Per", "Int", "Chr", "DF" };

enum { K_ADD = 0x1e41, K_DROP = 0x2044, K_KILL = 0x254b, K_NEW = 0x2e43, K_HERALD = 0x2348, K_FIGURE = 0x1f53,
       K_C1 = 0x0231, K_C2 = 0x0332, K_C3 = 0x0433, K_BEGIN = 0x3042 };

static std::string word(const RosterModel &r, int i, const char *fallback)
{
    std::string w = ns_gtext(r.word[i]);
    while (!w.empty() && (w.back() == ' ' || w.back() == '.')) w.pop_back();
    size_t a = w.find_first_not_of(' ');                    /* (the game pads some: " Add to the Party") */
    w = a == std::string::npos ? std::string() : w.substr(a);
    return w.empty() ? fallback : w;
}

static void draw_pic(NsDraw &P, const GamePic *g, float x0, float y0, float x1, float y1)
{
    if (!g || !g->tex) return;
    float s = fminf((x1 - x0) / g->w, (y1 - y0) / g->h);
    float cx = (x0 + x1) * 0.5f, cy = (y0 + y1) * 0.5f, hw = g->w * s * 0.5f, hh = g->h * s * 0.5f;
    P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(cx - hw, cy - hh), NsDraw::w(cx + hw, cy + hh));
}

extern "C" void rosterview_draw(void)
{
    const RosterModel &r = roster_model;
    if (!r.open || ns_screen != NS_ROSTER || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    if (figpick_is_open()) P.click = false;                    /* (the figure picker over it takes the clicks) */
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 GREEN = IM_COL32(40, 92, 34, 235), BROWN = IM_COL32(120, 84, 40, 235), RED = IM_COL32(112, 40, 22, 235),
                GREY = IM_COL32(150, 136, 112, 200), PANEL = IM_COL32(244, 234, 210, 255), HOVER = IM_COL32(232, 214, 176, 255),
                CHOSEN = IM_COL32(222, 196, 146, 255);
    static int del_ask = -1; static unsigned del_t;
    static int last_row = -1; static unsigned last_click;
    static float scroll = 0;
    char b[256];

    /* the page, the whole screen */
    P.rect(0, 0, 320, 200, IM_COL32(236, 222, 188, 255));
    P.rect(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
    P.frame(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);
    P.text(f, NsDraw::FS_TITLE, 6, 3, ink, r.inn ? "Who travels on" : "Your party");
    P.text(fo, NsDraw::FS_SMALL, 6, 12.5f, soft, r.inn ? "Up to four travel together. Those who stay behind are kept here and can join again; new characters can join too."
                                                       : "Up to four travel together. Choose who goes, make new characters, then begin.");
    if (r.inn) { if (P.button(286, 3.5f, 316, 12, "Done", GREEN)) ns_push(RV_KEY, 0x1352, 0, 0); }     /* (the inn's R) */
    else { std::string t = word(r, 10, "Return"); float w = P.width(fo, NsDraw::FS_SMALL, t) + 8;
      if (P.button(316 - w, 3.5f, 316, 12, t, RED)) ns_push(RV_BACK, 0, 0, 0); }

    /* the four places; the first empty one says how to fill it, the others are only outlines */
    int first_empty = -1; for (int s = 0; s < 4; s++) if (r.slot_char[s] < 0) { first_empty = s; break; }
    for (int s = 0; s < 4; s++) {
        float x0 = 4 + s * 78.2f, x1 = x0 + 76, y0 = 19.5f, y1 = 73;
        int c = r.slot_char[s];
        bool chosen = c >= 0 && c == r.sel, h = P.over(x0, y0, x1, y1);
        if (c < 0) {                                        /* (empty: an outline, no fill) */
            P.frame(x0, y0, x1, y1, IM_COL32(120, 80, 36, 70), 1.8f);
            if (s == first_empty) {
                std::string t = r.n ? "Choose someone below, then " + word(r, 1, "Add to the party") : std::string("Make a character first");
                P.para(fo, NsDraw::FS_SMALL, x0 + 8, (y0 + y1) * 0.5f - 5, x1 - x0 - 16, dim, t);
            }
            continue;
        }
        P.rect(x0, y0, x1, y1, chosen ? CHOSEN : h ? HOVER : PANEL, 1.8f);
        P.frame(x0, y0, x1, y1, IM_COL32(120, 80, 36, chosen ? 200 : 90), 1.8f);
        const RosterChar &ch = r.ch[c];
        draw_pic(P, ns_figure_pic(20 + s, r.figure[s], r.pal[s], "SHORT"), x0 + 1.5f, y0 + 3, x0 + 25, y1 - 3);
        float tx = x0 + 26;
        P.text(f, NsDraw::FS_NAME, tx, y0 + 2.5f, ink, ns_gtext(ch.nick[0] ? ch.nick : ch.name), -1, x1 - 2);
        snprintf(b, sizeof b, "%s, %d", ch.female ? "woman" : "man", ch.age);
        P.text(fo, 4.2f, tx, y0 + 10, soft, b);
        static const int SHOW[3] = { 0, 1, 6 };
        for (int k = 0; k < 3; k++) {
            float yy = y0 + 17 + k * 7;
            ui_attr_icon(P.dl, NsDraw::w(tx + 2.2f, yy + 2.8f), 4.2f * P.k, SHOW[k]);
            if (k < 2) snprintf(b, sizeof b, "%s %d/%d", ATTR_S[SHOW[k]], ch.attr[SHOW[k]], ch.attr_max[SHOW[k]]);
            else snprintf(b, sizeof b, "%s %d", ATTR_S[SHOW[k]], ch.attr[SHOW[k]]);
            P.text(fo, 4.4f, tx + 6.5f, yy, ink, b);
        }
        if (P.hit(x0, y0, x1, y1)) ns_push(RV_SELECT, c, 0, 0);
    }

    /* the party's skills together: for each skill outside weapons, the best member's (a party
       needs someone to read, to heal, to find the way ...) */
    {
        static const int PK[12] = { 10, 11, 12, 13, 14, 15, 16, 18, 17, 7, 8, 9 };
        static const char *FOR[19] = { "", "", "", "", "", "", "", "brewing potions from the formulae the party knows",
            "the saints: learning them and calling on them; churches and the clergy", "standing firm against evil; the saints' favor; some cards",
            "talking: bargaining, persuading, most people on the cards", "talking with the learned, the clergy and the nobility",
            "reading letters, books, papers and inscriptions", "tending wounds after a fight", "locks, traps and devices",
            "sneaking, hiding, getting in unseen", "the towns' underside: thieves, rumors, the streets at night",
            "riding: faster travel, outriding pursuers on the cards", "the wilds: finding the way, hunting, tracking, keeping out of danger" };
        const float SY = 75.5f, CX0 = 52, CW = (316 - CX0) / 12.0f;
        P.text(fo, 4.4f, 6, SY + 1.2f, soft, "The party's best skills");
        std::string desc = r.party_n ? "Each skill's best level in the party. Point at one to see what it is for." : "Add characters to the party to see its skills together.";
        for (int i = 0; i < 12; i++) {
            int k = PK[i], best = -1, who = -1;
            for (int s = 0; s < 4; s++) { int c = r.slot_char[s]; if (c >= 0 && c < r.n && r.ch[c].skill[k] > best) { best = r.ch[c].skill[k]; who = c; } }
            float cx = CX0 + i * CW;
            bool h = P.over(cx, SY - 0.5f, cx + CW - 1, SY + 8.5f);
            if (h) P.rect(cx, SY - 0.5f, cx + CW - 1, SY + 8.5f, IM_COL32(226, 204, 160, 255), 1.2f);
            ui_skill_icon(P.dl, NsDraw::w(cx + 3.2f, SY + 4), 4.6f * P.k, k);
            ImU32 col = best < 0 ? dim : best < 20 ? NsDraw::RED : best >= 40 ? NsDraw::GREEN : ink;
            snprintf(b, sizeof b, best < 0 ? "-" : "%d", best);
            P.text(f, NsDraw::FS_BODY, cx + 7.6f, SY + 1.2f, col, b);
            if (h) {
                if (who >= 0) snprintf(b, sizeof b, "%s %d (%s): %s.", gm_skill_short(k), best, ns_gtext(r.ch[who].nick[0] ? r.ch[who].nick : r.ch[who].name).c_str(), FOR[k]);
                else snprintf(b, sizeof b, "%s: %s.", gm_skill_short(k), FOR[k]);
                desc = b;
            }
        }
        P.text(fo, NsDraw::FS_SMALL, 6, SY + 9.6f, dim, desc, -1, 316);
    }

    /* the characters kept */
    const float LX0 = 4, LX1 = 140, LY0 = 93, LY1 = 184;
    P.rect(LX0, LY0, LX1, LY1, PANEL, 1.8f);
    snprintf(b, sizeof b, "Characters (%d)", r.n);
    P.text(f, NsDraw::FS_HEAD, LX0 + 3, LY0 + 2, ink, b);
    P.text(fo, 3.9f, 0, LY0 + 3.6f, dim, "double-click: add / remove", LX1 - 3);
    const float RH = 8.4f, RY0 = LY0 + 11;
    int rows = (int)((LY1 - 1 - RY0) / RH);
    if (P.over(LX0, RY0, LX1, LY1) && ns_in.wheel != 0) { scroll -= ns_in.wheel; ns_in.wheel = 0; }
    int maxtop = r.n > rows ? r.n - rows : 0;
    if (scroll < 0) scroll = 0;
    if (scroll > maxtop) scroll = (float)maxtop;
    static int last_sel = -2;
    if (r.sel != last_sel) {                                   /* keep the chosen one in sight (the arrow keys) */
        last_sel = r.sel;
        if (r.sel >= 0 && r.sel < (int)scroll) scroll = (float)r.sel;
        if (r.sel >= (int)scroll + rows) scroll = (float)(r.sel - rows + 1);
    }
    int top = (int)scroll;
    for (int i = top; i < r.n && i < top + rows; i++) {
        float ry = RY0 + (i - top) * RH;
        const RosterChar &ch = r.ch[i];
        bool h = P.over(LX0 + 1, ry, LX1 - 1, ry + RH);
        if (i == r.sel) P.rect(LX0 + 1, ry, LX1 - 1, ry + RH, CHOSEN, 1.0f);
        else if (h) P.rect(LX0 + 1, ry, LX1 - 1, ry + RH, HOVER, 1.0f);
        if (ch.slot >= 0) P.rect(LX0 + 2, ry + 1.5f, LX0 + 3.6f, ry + RH - 1.5f, NsDraw::GREEN, 0.5f);
        P.text(f, NsDraw::FS_NAME, LX0 + 5.5f, ry + 1.1f, ink, ns_gtext(ch.name), -1, LX1 - 30);
        if (ch.slot >= 0) P.text(fo, 3.9f, 0, ry + 2.3f, NsDraw::GREEN, "in the party", LX1 - 3);
        else { snprintf(b, sizeof b, "%s %d", ch.female ? "f" : "m", ch.age); P.text(fo, 3.9f, 0, ry + 2.3f, dim, b, LX1 - 3); }
        if (P.hit(LX0 + 1, ry, LX1 - 1, ry + RH)) {
            unsigned now = plat_ticks_ms();
            if (last_row == i && now - last_click < 450) { ns_push(RV_SELECT, i, 0, 0); ns_push(RV_KEY, ch.slot >= 0 ? K_DROP : K_ADD, 0, 0); last_row = -1; }
            else { ns_push(RV_SELECT, i, 0, 0); last_row = i; last_click = now; }
        }
    }
    if (maxtop) {                                               /* where in the list */
        float th = (LY1 - 2 - RY0) * rows / r.n, ty = RY0 + (LY1 - 2 - RY0 - th) * (scroll / maxtop);
        P.rect(LX1 - 2.2f, ty, LX1 - 1, ty + th, IM_COL32(120, 80, 36, 120), 0.6f);
    }
    if (!r.n) P.para(fo, NsDraw::FS_SMALL, LX0 + 4, RY0 + 2, LX1 - LX0 - 8, soft, "No characters are kept yet. Make one with Create a Character.");

    /* the chosen one */
    const float DX0 = 143, DX1 = 316, DY0 = 93, DY1 = 184;
    P.rect(DX0, DY0, DX1, DY1, PANEL, 1.8f);
    if (r.sel >= 0 && r.sel < r.n) {
        const RosterChar &ch = r.ch[r.sel];
        int slot = ch.slot;
        float x = DX0 + 3, y = DY0 + 2;
        P.text(f, NsDraw::FS_HEAD, x, y, ink, ns_gtext(ch.name), -1, DX1 - 3);
        y += 8.5f;
        snprintf(b, sizeof b, "%s of %d, called %s%s", ch.female ? "A woman" : "A man", ch.age, ns_gtext(ch.nick).c_str(), slot >= 0 ? "  -  in the party" : "");
        P.text(fo, NsDraw::FS_SMALL, x, y, soft, b);
        y += 7.5f;
        for (int a = 0; a < 7; a++) {                           /* the attributes */
            float ax = x + a * 24.5f;
            P.rect(ax, y, ax + 23, y + 9.5f, IM_COL32(236, 224, 196, 255), 1.2f);
            ui_attr_icon(P.dl, NsDraw::w(ax + 3.4f, y + 4.75f), 4.6f * P.k, a);
            P.text(fo, 3.8f, 0, y + 0.4f, dim, ATTR_S[a], ax + 22);
            snprintf(b, sizeof b, "%d", ch.attr[a]);
            P.text(f, NsDraw::FS_BODY, 0, y + 3.6f, ink, b, ax + 22);
        }
        y += 12;
        int ord[19]; for (int i = 0; i < 19; i++) ord[i] = i;    /* the best skills */
        std::stable_sort(ord, ord + 19, [&](int p, int q) { return ch.skill[p] > ch.skill[q]; });
        P.text(fo, 4.2f, x, y, soft, "Best skills");
        if (slot >= 0)                                          /* the figure, front and back, in their colours */
            draw_pic(P, ns_figure_pic(30 + slot, r.figure[slot], r.pal[slot], "SMALL"), DX1 - 44, y - 1, DX1 - 3, y + 33);
        y += 6;
        for (int i = 0; i < 8; i++) {
            int k = ord[i];
            float sx = x + (i % 2) * 64, sy = y + (i / 2) * 6.6f;
            ui_skill_icon(P.dl, NsDraw::w(sx + 2.4f, sy + 2.9f), 4.2f * P.k, k);
            P.text(fo, 4.4f, sx + 6.8f, sy, ink, gm_skill_short(k));
            snprintf(b, sizeof b, "%d", ch.skill[k]);
            P.text(fo, 4.4f, 0, sy, ink, b, sx + 60);
        }
        y += 4 * 6.6f + 1;
        snprintf(b, sizeof b, "Knows %d saint%s and %d formula%s; carries %d item%s.", ch.nsaints, ch.nsaints == 1 ? "" : "s",
                 ch.nformulae, ch.nformulae == 1 ? "" : "e", ch.nitems, ch.nitems == 1 ? "" : "s");
        P.text(fo, 4.2f, x, y, soft, b);
        y += 7;
        /* what can be done */
        float bx = x, by = DY1 - 22, bh = 9;
        if (slot >= 0) {
            if (P.button(bx, by, bx + 40, by + bh, r.inn ? std::string("Leave at the inn") : std::string("Remove from the party"), ch.can_leave ? BROWN : GREY)) ns_push(RV_KEY, K_DROP, 0, 0);
            if (P.button(bx + 42, by, bx + 72, by + bh, "Sheet", BROWN)) ns_push(RV_SHEET, slot, 0, 0);
        } else if (P.button(bx, by, bx + 72, by + bh, std::string("Add to the party"), r.party_n >= 4 ? GREY : GREEN)) ns_push(RV_KEY, K_ADD, 0, 0);
        bool asking = del_ask == r.sel && plat_ticks_ms() - del_t < 4000;
        if (!r.inn && P.button(DX1 - 3 - (asking ? 62 : 36), by, DX1 - 3, by + bh, asking ? "Click again to delete" : std::string("Delete character"), RED)) {
            if (asking) { ns_push(RV_KEY, K_KILL, 0, 0); del_ask = -1; }
            else { del_ask = r.sel; del_t = plat_ticks_ms(); }
        }
        by += bh + 2;
        if (slot >= 0) {                                        /* the looks (the game keeps them per party place) */
            const GamePic *her = nullptr;
            if (r.herald[slot] >= 'A' && r.herald[slot] <= 'Z') { snprintf(b, sizeof b, "PICS\\SHIELD%c.PIC", r.herald[slot]); her = gamepic(b); }
            if (P.button(bx, by, bx + 36, by + bh, "Sprite...", BROWN)) ns_push(RV_KEY, K_FIGURE, 0, 0);
            if (P.button(bx + 38, by, bx + 74, by + bh, "Heraldry >", BROWN)) ns_push(RV_KEY, K_HERALD, 0, 0);
            draw_pic(P, her, bx + 75, by - 0.5f, bx + 84, by + bh + 0.5f);
            float cx = bx + 87;
            P.text(fo, 4.2f, cx, by + 2, soft, "Colors");
            cx += P.width(fo, 4.2f, "Colors") + 2;
            static const int CK[3] = { K_C1, K_C2, K_C3 };
            for (int k = 0; k < 3; k++) {
                std::string tip = word(r, 4 + k, "");
                if (P.button(cx, by, cx + 9, by + bh, std::to_string(k + 1), BROWN)) ns_push(RV_KEY, CK[k], 0, 0);
                if (P.over(cx, by, cx + 9, by + bh) && !tip.empty()) P.text(fo, 3.9f, 0, by - 5, dim, tip, DX1 - 3);
                cx += 10.5f;
            }
        } else P.text(fo, 4.0f, bx, by + 2, dim, "Add to the party to set sprite, heraldry and colors.");
    } else P.text(fo, NsDraw::FS_SMALL, DX0 + 4, DY0 + 4, soft, r.n ? "Choose a character on the left." : "");

    /* along the bottom */
    if (P.button(4, 187, 70, 196.5f, word(r, 0, "New character"), BROWN, 4.4f)) ns_push(RV_KEY, K_NEW, 0, 0);
    if (!r.inn && P.button(230, 187, 316, 196.5f, word(r, 9, "Begin the adventure"), r.party_n ? GREEN : GREY, NsDraw::FS_BODY)) ns_push(RV_KEY, K_BEGIN, 0, 0);
    if (r.note[0] && plat_ticks_ms() - r.note_t < 6000) P.text(fo, NsDraw::FS_SMALL, 74, 189.5f, NsDraw::RED, r.note, -1, 228);
    else {
        snprintf(b, sizeof b, "%d of 4 in the party", r.party_n);
        P.text(fo, NsDraw::FS_SMALL, 74, 189.5f, soft, b);
    }
}
