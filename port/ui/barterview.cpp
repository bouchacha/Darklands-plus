/* barterview.cpp - draws the shop screen (mods/barterview.c runs it; docs/NATIVE_SCREENS.md).
 *
 * Over the picture beside the party panel (game x 61..319):
 *   - the shop's name, the quality of its wares, the purse and who holds it, Leave;
 *   - two lists side by side: the merchant's wares (each with its price) and the things of the
 *     member bartering (with what the merchant pays; what is in hand or worn marked red at its left),
 *     the member chosen by the tabs above it;
 *   - Buy / Sell on the row under the pointer (Shift: ten / the whole stack; All on a stack);
 *   - a fixed strip at the bottom describes the row under the pointer (no floating tooltips),
 *     and says what the last purchase or sale did.
 * Drawn from barter_model alone; clicks become actions (BV_*) for the screen's loop. */
#include "imgui.h"
#include <map>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/nscreen.h"
#include "../mods/barterview.h"
#include "nsdraw.h"
#include "../runtime/config.h"
#include <vector>
extern "C" {
#include "../mods/gamemodel.h"
#include "../mods/assist.h"
#include "money.h"
}
bool ui_item_tidy_less(int ida, int qa, int idb, int qb);    /* (inventory.cpp) */

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

namespace {
ImVec2 g2w(float x, float y) { return NsDraw::w(x, y); }
std::string gt(const char *s) { return ns_gtext(s); }
const ItemDef *def(int id) { return ns_itemdef(id); }
std::string item_name(int id) { return ns_item_name(id); }
std::string money(long pf) { return money_str(pf < 0 ? 0 : pf); }
struct View {
    int scroll[2];          /* first row shown: wares, member's things */
    bool formulae;          /* the right column lists the party's formulae (a shop with ingredients) */
    int fscroll;
    std::string fnote;      /* what the formula button under the pointer buys (or the spare gear) */
    std::string fhead;      /* its heading, "" for the formula's */
    int last_who = -1;
    int hover_list = -1, hover_row = -1;       /* the row described in the strip */
    unsigned hover_seq;
} V;
}

/* the hidden VGA under the native screen still holds the original's; the crisp text and the
 * shop tooltips leave it alone while this is up */
extern "C" int barterview_showing(void) { return ns_screen == NS_BARTER && barter_model.open && !ns_hidden(); }

extern "C" void barterview_draw(void)
{
    const BarterModel &b = barter_model;
    if (!barterview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const float k = P.k;
    ImDrawList *dl = P.dl;
    const ImU32 ink = NsDraw::INK, ink_soft = NsDraw::INK_SOFT, ink_dim = NsDraw::INK_DIM, red = NsDraw::RED, green = NsDraw::GREEN;
    auto R = [&](float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { P.rect(x0, y0, x1, y1, c, rnd); };
    auto Rl = [&](float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { P.frame(x0, y0, x1, y1, c, rnd); };
    auto T = [&](ImFont *ff, float gs, float x, float y, ImU32 c, const std::string &s, float right_to = -1, float clip_to = -1) { P.text(ff, gs, x, y, c, s, right_to, clip_to); };
    auto W = [&](ImFont *ff, float gs, const std::string &s) { return P.width(ff, gs, s); };
    int mx = P.mx, my = P.my, cx = P.cx, cy = P.cy;
    bool click = P.click, shift = P.shift;
    auto in = [&](int x, int y, float x0, float y0, float x1, float y1) { return x >= x0 && x < x1 && y >= y0 && y < y1; };
    auto hit = [&](float x0, float y0, float x1, float y1) { return P.hit(x0, y0, x1, y1); };
    (void)click;

    /* the shop: its counter, the wall behind (ui/nsdraw.cpp) */
    const ImU32 cream = IM_COL32(246, 232, 200, 255), cream_soft = IM_COL32(222, 204, 170, 235);
    ns_counter_backdrop(P, 30, IM_COL32(150, 36, 26, 255));

    /* the header: shop, quality, purse, Leave */
    { std::string sn = gt(b.shopname); if (!sn.empty() && sn[0] >= 'a' && sn[0] <= 'z') sn[0] = (char)(sn[0] - 32);     /* ("herbalist": some cards name the shop in lower case) */
      T(f, NsDraw::FS_TITLE, 65, 3, cream, sn, -1, 205); }
    {   /* the subtitle: the wares' quality and who haggles (or pays); the purse beside Leave, no label */
        std::string purse = money(b.total_pf), who = b.payer >= 0 && b.payer < 5 ? gt(b.names[b.payer]) : "";
        int barg = assist_bargainer();                         /* (the best charisma bargains: mods/assist.c) */
        std::string sub = "Wares " + std::to_string(shop_wares_q(b.quality)) + "q";
        if (assist_best_leader && barg >= 0 && barg < 5) sub += "  -  " + gt(b.names[barg]) + " bargains";
        else if (!who.empty()) sub += "  -  " + who + " pays";
        T(fo, NsDraw::FS_BODY, 65, 12.5f, cream_soft, sub);
        T(f, NsDraw::FS_HEAD, 0, 6.0f, cream, purse, 280);
        if (P.button(284, 4, 315, 15, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(BV_LEAVE, 0, 0, 0);
    }

    /* the member tabs (right column); the coin (or a right click): holds the purse */
    const float LX0 = 63, LX1 = 180, RX0 = 184, RX1 = 317, LY0 = 31, ROWH = 9;
    T(f, NsDraw::FS_HEAD, LX0 + 1, 21, cream, "For sale");
    /* a shop with ingredients and a party that knows formulae: the formulae, to buy for batches */
    bool can_formulae = false;
    for (int i = 0; i < b.nwares && !can_formulae; i++) { const ItemDef *d = def(b.ware[i].id); if (d && d->kind == K_INGREDIENT) can_formulae = true; }
    if (can_formulae) {
        bool kn = false;
        for (int m = 0; m < 5 && !kn; m++) if (b.present[m]) for (int fm = 0; fm < 66 && !kn; fm++) if (gm_knows_formula(m, fm)) kn = true;
        can_formulae = kn;
    }
    if (!can_formulae) V.formulae = false;
    else {
        const char *lab = V.formulae ? "Your items" : "Formulae";
        float bx1 = LX1, bx0 = bx1 - W(fo, NsDraw::FS_SMALL, lab) - 6;
        if (P.button(bx0, 20.5f, bx1, 28.5f, lab, IM_COL32(120, 84, 40, 235))) { V.formulae = !V.formulae; V.fscroll = 0; }
    }
    V.fnote.clear(); V.fhead.clear();
    {
        int n = 0; for (int m = 0; m < 5; m++) if (b.present[m]) n++;
        float tw = n ? (RX1 - RX0) / n : 0, x = RX0;
        for (int m = 0; m < 5; m++) {
            if (!b.present[m]) continue;
            bool sel = m == b.who, h = in(mx, my, x, 19, x + tw - 1, 29);
            P.tab(x, 19, x + tw - 1, 29, sel, h, 2);
            std::string nm = gt(b.names[m]);
            /* the purse: a gold coin on the tab of who holds it; a click on another tab's (faint) coin hands it over */
            bool coin_h = in(mx, my, x, 19, x + 7, 29);
            T(f, NsDraw::FS_NAME, x + 7.0f, 20.5f, sel ? IM_COL32(250, 236, 200, 255) : ink, nm, -1, x + tw - 2);
            if (assist_best_leader) {                          /* the best charisma bargains, whoever holds the purse: no coins to hand over */
                if (click && in(cx, cy, x, 19, x + tw - 1, 29)) ns_push(BV_WHO, m, 0, 0);
                x += tw; continue;
            }
            if (m == b.payer) dl->AddCircleFilled(g2w(x + 3.5f, 24), 1.8f * k, IM_COL32(214, 172, 60, 255));
            else dl->AddCircle(g2w(x + 3.5f, 24), 1.8f * k, coin_h ? IM_COL32(214, 172, 60, 255) : sel ? IM_COL32(250, 236, 200, 110) : IM_COL32(120, 90, 40, 110), 0, fmaxf(1.0f, 0.4f * k));
            if (coin_h && m != b.payer) ImGui::SetTooltip("Click to have %s pay from now on", nm.c_str());
            if (click && in(cx, cy, x, 19, x + 7, 29)) ns_push(BV_PAYER, m, 0, 0);
            else if (click && in(cx, cy, x, 19, x + tw - 1, 29)) ns_push(BV_WHO, m, 0, 0);
            if (P.rclick && in(mx, my, x, 19, x + tw - 1, 29)) ns_push(BV_PAYER, m, 0, 0);
            x += tw;
        }
    }
    if (b.who != V.last_who) { V.scroll[1] = 0; V.last_who = b.who; }

    /* the lists */
    const int NROWS = 13;
    float LY1 = LY0 + NROWS * ROWH;
    int new_hl = -1, new_hr = -1;
    /* a thing "Sell spare gear" would sell: the weapons, armour and shields not in use (not
       arrows, quarrels, balls or things to throw), that this shop pays for, not kept */
    auto is_spare = [&](int i) {
        const BvItem &it = b.item[i];
        const ItemDef *d = def(it.id);
        if (!d || it.equipped || it.price <= 0 || it.kept) return false;
        if (!(d->kind == K_WEAPON || d->kind == K_ARMOR || d->kind == K_SHIELD || d->kind == K_MISSILE)) return false;
        if ((d->type >= 0x17 && d->type <= 0x1a) || (d->type >= 0x40 && d->type <= 0x42)) return false;
        return true;
    };
    /* how many of each thing the party carries (an ingredient for sale says it) */
    std::map<int, int> have_n;
    { GmItem g; for (int m = 0; m < 5; m++) if (b.present[m]) for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &g)) have_n[g.id] += g.count; }
    auto ingredient = [&](int id) { const ItemDef *d = def(id); return d && d->kind == K_INGREDIENT; };
    const bool cmp_on = config_get_int("display.shop_compare", 1) != 0;
    const int cmp_slot = cmp_on && V.hover_list == 0 && V.hover_row >= 0 && V.hover_row < b.nwares ? ns_slot_of(b.ware[V.hover_row].id) : 0;
    static bool spare_lit;                       /* the button under the pointer (last frame): its things pulse in the list */
    float pulse = 0.5f + 0.5f * sinf(plat_ticks_ms() * 0.008f);
    for (int L = 0; L < 2; L++) {
        if (L == 1 && V.formulae) continue;
        float x0 = L ? RX0 : LX0, x1 = L ? RX1 : LX1;
        /* the rows: the wares as the shop has them; the party's things as the character sheet has
           them (Equipped, Potions, the rest; each in the inventory's tidy order), a heading before each */
        std::vector<int> order;
        if (!L) for (int i = 0; i < b.nwares; i++) order.push_back(i);
        else {
            auto tidy = [&](size_t from) { std::stable_sort(order.begin() + from, order.end(), [&](int a, int c) { return ui_item_tidy_less(b.item[a].id, b.item[a].qual, b.item[c].id, b.item[c].qual); }); };
            for (int sec = 0; sec < 6; sec++) {                    /* (the character sheet's sections: ns_item_section) */
                size_t head = order.size(); order.push_back(-1 - sec);
                for (int i = 0; i < b.nitems; i++) if ((sec == 0) == (b.item[i].equipped != 0) && (sec == 0 || ns_item_section(b.item[i].id) == sec)) order.push_back(i);
                if (order.size() == head + 1) order.pop_back(); else tidy(head + 1);
            }
        }
        int n = (int)order.size();
        ns_sheet(P, x0, LY0 - 1, x1, LY1 + 1, IM_COL32(250, 242, 222, 255));
        Rl(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
        bool over = in(mx, my, x0, LY0, x1, LY1) || (!L && !in(mx, my, RX0, LY0, RX1, LY1));
        if (over && ns_in.wheel != 0) { V.scroll[L] -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
        int maxs = n - NROWS; if (maxs < 0) maxs = 0;
        if (V.scroll[L] > maxs) V.scroll[L] = maxs;
        if (V.scroll[L] < 0) V.scroll[L] = 0;
        if (!n) T(fo, NsDraw::FS_BODY, x0 + 4, LY0 + 3, ink_dim, L ? "Nothing to sell." : "Nothing for sale today.");
        for (int r = 0; r < NROWS && V.scroll[L] + r < n; r++) {
            int i = order[V.scroll[L] + r];
            float y = LY0 + r * ROWH;
            if (i < 0) {                                         /* a heading, as on the character sheet */
                const char *hd = ns_item_section_name(-1 - i);
                T(fo, NsDraw::FS_SMALL, x0 + 3, y + 2.2f, ink_soft, hd);
                float lw = 0;
                if (i == -1) {                                    /* Equipped: the member's load, and what the thing pointed at would make it */
                    int after = -1;
                    if (V.hover_list == 0 && V.hover_row >= 0 && V.hover_row < b.nwares) after = ns_load_with(b.ware[V.hover_row].id, b.who);
                    else if (V.hover_list == 1 && V.hover_row >= 0 && V.hover_row < b.nitems && !b.item[V.hover_row].equipped) after = ns_load_with(b.item[V.hover_row].id, b.who);
                    lw = ns_load_note(P, x1 - 3, y + 2.2f, b.who, after) + 3;
                }
                R(x0 + 3 + W(fo, NsDraw::FS_SMALL, hd) + 2, y + 4.8f, x1 - 3 - lw, y + 5.1f, IM_COL32(120, 80, 36, 90));
                continue;
            }
            int id, qual, price, count = 1, eq = 0, kept = 0, pack = 0;
            if (L) { const BvItem &it = b.item[i]; id = it.id; qual = it.qual; price = it.price; count = it.count; eq = it.equipped; kept = it.kept; pack = it.holder < 0; }
            else { const BvWare &w = b.ware[i]; id = w.id; qual = w.qual; price = w.price; }
            bool h = in(mx, my, x0, y, x1, y + ROWH);
            if (h) { new_hl = L; new_hr = i; }
            if (h) R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
            else if (r & 1) R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
            if (L && spare_lit && is_spare(i)) {
                R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(200, 120, 60, (int)(60 + 90 * pulse)));
                Rl(x0 + 0.8f, y + 0.3f, x1 - 0.8f, y + ROWH - 0.3f, IM_COL32(150, 56, 30, (int)(120 + 120 * pulse)), 1.0f);
            }
            P.icon(x0 + 6.5f, y + ROWH * 0.5f, 7.0f, id, qual);
            std::string nm = item_name(id);
            if (count > 1) nm += " (" + std::to_string(count) + ")";
            char qs[16]; snprintf(qs, sizeof qs, "%s", ns_quality_tag(id, qual).c_str());
            std::string ps = price > 0 ? money(price) : L ? "-" : "?";
            float pw = W(fo, NsDraw::FS_BODY, ps);
            float name_end = x1 - 2 - pw - 5 - W(fo, NsDraw::FS_BODY, qs) - 3;
            int vs = !L && cmp_on ? ns_against_worn(id, qual, b.who) : 0;     /* (a ware: better or worse than what the member has there) */
            if (vs) {
                float ax = fminf(x0 + 12 + W(f, NsDraw::FS_NAME, nm) + 4.5f, name_end - 2.5f), ay = y + ROWH * 0.5f;   /* (just after the name) */
                ns_compare_arrow(P.dl, ax, ay, vs, P.k);
                name_end -= 6;
            }
            if (L && cmp_slot && b.item[i].equipped == 1 && ns_slot_of(id) == cmp_slot) {      /* what the ware pointed at would replace */
                R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(214, 170, 60, (int)(40 + 50 * pulse)));
                Rl(x0 + 0.8f, y + 0.3f, x1 - 0.8f, y + ROWH - 0.3f, IM_COL32(170, 120, 30, (int)(150 + 100 * pulse)), 1.0f);
            }
            if (eq) R(x0 + 0.5f, y + 1, x0 + 1.8f, y + ROWH - 1, NsDraw::RED);     /* equipped: a red mark at its left (said in the strip) */
            char haves[24] = "";
            if (!L && ingredient(id)) snprintf(haves, sizeof haves, "have %d", have_n.count(id) ? have_n[id] : 0);
            const char *tag = eq == 2 ? "rides it" : kept ? "kept" : haves[0] ? haves : nullptr;
            float tagw = tag ? P.tag_width(tag) : 0;
            T(f, NsDraw::FS_NAME, x0 + 12, y + 1.3f, ink, nm, -1, name_end - tagw);
            if (tag) {
                float nx = fminf(x0 + 12 + W(f, NsDraw::FS_NAME, nm) + 2, name_end - tagw);
                P.tag(nx, y, ROWH, tag, haves[0] ? (have_n.count(id) ? NsDraw::TAG_OWNER : NsDraw::INK_DIM) : NsDraw::TAG_KEPT);
            }
            T(fo, NsDraw::FS_BODY, 0, y + 2.2f, ink_soft, qs, x1 - 2 - pw - 5);
            bool cant = !L && price > b.total_pf;
            T(fo, NsDraw::FS_BODY, 0, y + 2.0f, price <= 0 ? ink_dim : cant ? red : ink, ps, x1 - 2);
            /* the action on the row under the pointer */
            if (h) {
                const char *lab = L ? "Sell" : "Buy";
                bool ten = !L && ingredient(id);                      /* (an ingredient: ten at once) */
                bool two = (L && count > 1) || ten;
                float bw = 15, bx1 = x1 - 2 - pw - 2, bx0 = bx1 - bw, ax0 = bx0 - (two ? 13 : 0);
                if (!(L && (price <= 0 || kept))) {
                    R(ax0 - 1, y + 0.8f, bx1, y + ROWH - 0.8f, IM_COL32(226, 204, 160, 255));
                    bool hb = in(mx, my, bx0, y, bx1, y + ROWH);
                    R(bx0, y + 1, bx1, y + ROWH - 1, hb ? (L ? IM_COL32(150, 56, 30, 255) : IM_COL32(52, 112, 44, 255)) : (L ? IM_COL32(120, 44, 24, 235) : IM_COL32(40, 92, 34, 235)), 1.5f);
                    T(fo, NsDraw::FS_BODY, bx0 + (bw - W(fo, NsDraw::FS_BODY, lab)) * 0.5f, y + 2.2f, IM_COL32(250, 240, 214, 255), lab);
                    if (hit(bx0, y, bx1, y + ROWH)) {
                        if (L && pack) ns_push(BV_PSELL, id, qual, shift ? 255 : 1);
                        else ns_push(L ? BV_SELL : BV_BUY, L ? b.item[i].idx : i, L ? (shift ? 255 : 1) : (shift ? 10 : 1), 0);   /* (a member's own: their record) */
                    }
                    if (two) {
                        bool ha = in(mx, my, ax0, y, bx0 - 1, y + ROWH);
                        R(ax0, y + 1, bx0 - 1, y + ROWH - 1, ha ? IM_COL32(150, 56, 30, 255) : IM_COL32(120, 44, 24, 200), 1.5f);
                        if (ten) R(ax0, y + 1, bx0 - 1, y + ROWH - 1, ha ? IM_COL32(52, 112, 44, 255) : IM_COL32(40, 92, 34, 200), 1.5f);
                        T(fo, NsDraw::FS_BODY, ax0 + 1.5f, y + 2.2f, IM_COL32(250, 240, 214, 255), ten ? "x10" : "All");
                        if (hit(ax0, y, bx0 - 1, y + ROWH)) { if (ten) ns_push(BV_BUY, i, 10, 0); else if (pack) ns_push(BV_PSELL, id, qual, 255); else ns_push(BV_SELL, b.item[i].idx, 255, 0); }
                    }
                }
            }
        }
        if (n > NROWS) {                                       /* where in the list */
            float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * V.scroll[L] / (float)(n - NROWS);
            R(x1 - 1.6f, ty, x1 - 0.4f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
        }
    }
    if (V.formulae) {
        /* the formulae the party knows whose ingredients this shop sells: what is carried, what a
           batch more needs, and +1 / +5: buy what that many batches more need (for the member of the
           tab; the game's own purchases, one by one) */
        ns_sheet(P, RX0, LY0 - 1, RX1, LY1 + 1, IM_COL32(250, 242, 222, 255));
        Rl(RX0, LY0 - 1, RX1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
        auto carried = [&](int id) {
            int n = 0; GmItem g;
            for (int m = 0; m < 5; m++) if (b.present[m]) for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &g) && g.id == id) n += g.count;
            return n;
        };
        auto ware_of = [&](int id) { for (int i = 0; i < b.nwares; i++) if (b.ware[i].id == id) return i; return -1; };
        struct FRow { int f; std::string who; int batches; };
        std::vector<FRow> rows;
        for (int fm = 0; fm < 66 && fm < (int)gamedata.formulas.size() && fm < (int)gamedata.formula_names.size(); fm++) {
            std::string who;
            for (int m = 0; m < 5; m++) if (b.present[m] && gm_knows_formula(m, fm)) { if (!who.empty()) who += ", "; who += gt(b.names[m]); }
            if (who.empty()) continue;
            const Formula &F = gamedata.formulas[fm];
            bool here = false; int bt = 99;
            for (const FormulaIngr &g : F.ingr) { if (ware_of(g.item) >= 0) here = true; if (g.count > 0) { int c = carried(g.item) / g.count; if (c < bt) bt = c; } }
            if (!here || F.ingr.empty()) continue;
            rows.push_back({ fm, who, bt == 99 ? 0 : bt });
        }
        const float FH = ROWH * 2;
        int vis = (int)((LY1 - LY0) / FH), maxs = (int)rows.size() - vis; if (maxs < 0) maxs = 0;
        if (in(mx, my, RX0, LY0, RX1, LY1) && ns_in.wheel != 0) { V.fscroll -= (int)ns_in.wheel; ns_in.wheel = 0; }
        if (V.fscroll > maxs) V.fscroll = maxs;
        if (V.fscroll < 0) V.fscroll = 0;
        if (rows.empty()) P.para(fo, NsDraw::FS_BODY, RX0 + 3, LY0 + 2, RX1 - RX0 - 6, ink_dim, "None of the party's formulae uses what this shop sells.");
        std::string to = b.who >= 0 && b.who < 5 ? gt(b.names[b.who]) : "";
        for (int r = 0; r < vis && V.fscroll + r < (int)rows.size(); r++) {
            const FRow &fr = rows[V.fscroll + r];
            const Formula &F = gamedata.formulas[fr.f];
            float y = LY0 + r * FH;
            if (r & 1) R(RX0 + 0.5f, y, RX1 - 0.5f, y + FH, IM_COL32(240, 230, 204, 255));
            /* +1 / +5 batches */
            float bx = RX1 - 2;
            for (int k2 = 1; k2 >= 0; k2--) {
                int nb = k2 ? 5 : 1, target = fr.batches + nb;
                long cost = 0; bool ok = true; std::string list, notsold;
                std::vector<std::pair<int, int>> buys;
                for (const FormulaIngr &g : F.ingr) {
                    int need = g.count * target - carried(g.item);
                    if (need <= 0) continue;
                    int w = ware_of(g.item);
                    if (w < 0) { ok = false; notsold += (notsold.empty() ? "" : ", ") + std::to_string(need) + " " + item_name(g.item); continue; }
                    buys.push_back({ w, need }); cost += (long)need * b.ware[w].price;
                    list += (list.empty() ? "" : ", ") + std::to_string(need) + " " + item_name(g.item);
                }
                std::string lab = "+" + std::to_string(nb);
                float bw = W(fo, NsDraw::FS_SMALL, lab) + 5;
                bool en = !buys.empty();                       /* (what is sold here, even when something must be found elsewhere) */
                if (P.button(bx - bw, y + 1.2f, bx, y + 8.2f, lab, en ? (cost > b.total_pf ? IM_COL32(140, 60, 40, 200) : IM_COL32(40, 92, 34, 235)) : IM_COL32(150, 136, 112, 140)) && en)
                    for (auto &bb : buys) ns_push(BV_BUY, bb.first, bb.second, 0);
                if (in(mx, my, bx - bw, y + 1.2f, bx, y + 8.2f))
                    V.fnote = buys.empty() ? (!ok ? "Not sold here: " + notsold + "; the party has the rest." : "The party has enough for that already.") :
                              "Buys " + list + " for " + money(cost) + (cost > b.total_pf ? " (more than the purse holds)" : "") + "; they go to " + to + "." +
                              (!ok ? " Not sold here (to find elsewhere): " + notsold + "." : "");
                bx -= bw + 1.5f;
            }
            if (V.fnote.empty() && in(mx, my, RX0, y, RX1, y + FH)) {     /* the row: what a batch takes, and where it is */
                std::string d = "A batch takes ";
                for (size_t gi = 0; gi < F.ingr.size(); gi++) {
                    const FormulaIngr &g = F.ingr[gi];
                    int w = ware_of(g.item);
                    d += (gi ? (gi + 1 == F.ingr.size() ? " and " : ", ") : "") + std::to_string(g.count) + " " + item_name(g.item) +
                         " (the party has " + std::to_string(carried(g.item)) + (w >= 0 ? "; " + money(b.ware[w].price) + " here)" : "; not sold here)");
                }
                V.fnote = d + ". +1 and +5 buy what one or five more batches need.";
            }
            T(f, NsDraw::FS_NAME * 0.9f, RX0 + 3, y + 1.2f, ink, gamedata.formula_names[fr.f], -1, bx - 1);
            std::string l2 = fr.batches ? "Enough for " + std::to_string(fr.batches) + (fr.batches == 1 ? " batch" : " batches") : "Not enough for a batch";
            l2 += " - " + fr.who;
            T(fo, NsDraw::FS_SMALL, RX0 + 3, y + 10.2f, fr.batches ? ink_soft : red, l2, -1, RX1 - 2);
        }
        if ((int)rows.size() > vis) {
            float th = (LY1 - LY0) * vis / rows.size(), ty = LY0 + (LY1 - LY0 - th) * V.fscroll / (float)(rows.size() - vis);
            R(RX1 - 1.6f, ty, RX1 - 0.4f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
        }
    }
    if (new_hl >= 0) { V.hover_list = new_hl; V.hover_row = new_hr; V.hover_seq = b.seq; }
    else if (V.hover_seq != b.seq) V.hover_list = -1;

    /* the strip: the row under the pointer (or the last one), what the last action did */
    float SY0 = LY1 + 3, SY1 = 196;
    /* sell the spare gear: the weapons, armour and shields the member carries but does not use (not
       arrows, quarrels, balls or things to throw), that this shop pays for; asked twice */
    static unsigned ask_t; static int ask_who = -1;
    std::vector<int> spare; long spare_pf = 0; int spare_n = 0;
    for (int i = 0; i < b.nitems; i++) {
        if (!is_spare(i)) continue;
        const BvItem &it = b.item[i];
        spare.push_back(b.shared ? i : it.idx); spare_pf += (long)it.price * it.count; spare_n += it.count;
    }
    bool asking = ask_who == b.who && plat_ticks_ms() - ask_t < 5000;
    std::string slab = asking ? "Click again to sell them" : "Sell spare gear";
    float sbx1 = 314, sbx0 = sbx1 - W(fo, NsDraw::FS_SMALL, slab) - 6;
    bool show_spare = !spare.empty() && !V.formulae;
    bool spare_h = show_spare && (asking || in(mx, my, sbx0, SY1 - 9.5f, sbx1, SY1 - 1.5f));
    spare_lit = spare_h;
    if (spare_h && V.fnote.empty()) {
        char t[200];
        snprintf(t, sizeof t, "%d item%s not in use (weapons, armor, shields; not ammunition or throwing weapons), for about %s in all: the ones pulsing in the list.%s",
                 spare_n, spare_n == 1 ? "" : "s", money(spare_pf).c_str(), asking ? " Click again to sell them all." : "");
        V.fnote = t; V.fhead = "Spare gear";
    }
    ns_sheet(P, 63, SY0, 317, SY1, IM_COL32(236, 222, 188, 255));
    if (!V.fnote.empty()) {
        T(f, NsDraw::FS_HEAD, 68, SY0 + 1.5f, ink, V.fhead.empty() ? "For the formula" : V.fhead);
        P.para(fo, NsDraw::FS_BODY, 68, SY0 + 10, 245, ink_soft, V.fnote);
    } else if (V.hover_list >= 0) {
        int L = V.hover_list, i = V.hover_row;
        bool ok = L ? i < b.nitems : i < b.nwares;
        if (ok) {
            int id = L ? b.item[i].id : b.ware[i].id, qual = L ? b.item[i].qual : b.ware[i].qual;
            int price = L ? b.item[i].price : b.ware[i].price, wt = L ? b.item[i].weight : b.ware[i].weight;
            NsStripThing t;
            t.id = id; t.qual = qual; t.weight = wt; t.count = L ? b.item[i].count : 1;
            t.status = L && b.item[i].equipped == 2 ? "the mount this member rides" : L && b.item[i].equipped ? "equipped" : "";
            t.cmp_who = L ? -1 : b.who;
            t.right = L ? (price > 0 ? "The merchant pays " + money_with_total(price) : "The merchant will not buy this")
                        : "Costs " + money_with_total(price) + (price > b.total_pf ? " - more than the purse holds" : "");
            t.right_col = L ? (price > 0 ? green : red) : (price > b.total_pf ? red : ink);
            ns_item_strip(P, SY0, t);
        }
    } else T(fo, NsDraw::FS_BODY, 68, SY0 + 3, ink_dim, "Point at a ware or one of your items to see it here.");
    if (show_spare) {
        if (P.button(sbx0, SY1 - 9.5f, sbx1, SY1 - 1.5f, slab, asking ? IM_COL32(140, 60, 40, 235) : IM_COL32(120, 84, 40, 235))) {
            if (asking) {
                std::sort(spare.begin(), spare.end());
                ns_push(BV_BATCH_START, 0, 0, 0);
                if (b.shared) for (int k : spare) ns_push(BV_PSELL, b.item[k].id, b.item[k].qual, 255);   /* (the pack's: by kind and quality) */
                else for (int k = (int)spare.size() - 1; k >= 0; k--) ns_push(BV_SELL, spare[k], 255, 0);   /* (the last first: the others keep their places) */
                ns_push(BV_BATCH_END, 0, 0, 0);
                ask_who = -1;
            } else { ask_who = b.who; ask_t = plat_ticks_ms(); }
        }
        sbx0 -= 3;
    } else sbx0 = 316;
    if (b.note[0] && plat_ticks_ms() - b.note_t < 6000) T(fo, NsDraw::FS_BODY, 68, SY1 - 8.5f, IM_COL32(112, 40, 22, 255), gt(b.note), -1, sbx0);
    else T(fo, NsDraw::FS_SMALL, 68, SY1 - 7.5f, ink_dim, b.shared ? "Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  purchases go to the member picked above"
                                              : assist_best_leader ? "Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  click a name to see their items"
                                              : "Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  click a name's coin to make that member pay", -1, sbx0);
}
