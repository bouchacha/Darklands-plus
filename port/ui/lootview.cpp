/* lootview.cpp - draws the loot screen (mods/lootview.c runs it; docs/NATIVE_SCREENS.md).
 *
 * The pile and the member's things side by side; Take (the whole stack, or 1) on a pile row,
 * Put back (the stack, or 1) on the member's; the members as tabs; Take all and Share out;
 * a fixed strip describes the row under the pointer. Drawn from loot_model alone. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/lootview.h"
#include "nsdraw.h"
#include "../mods/gamemodel.h"
#include "../mods/assist.h"
#include "../runtime/config.h"
#include "money.h"

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

namespace {
struct LootView {
    int scroll[2] = {};
    int last_who = -1;
    int hover_list = -1, hover_row = -1; unsigned hover_seq = 0;
} V;
}

extern "C" int lootview_showing(void) { return ns_screen == NS_LOOT && loot_model.open && !ns_hidden(); }

/* about what a shop pays for one: the game's sell_price (s032 1b26) without the town, whose size
   takes 2% a step off a common thing and adds as much to a rare one (the reputation and fame
   terms come to nothing in its integer arithmetic, but for a bad reputation's 1%) */
static long sale_worth(int id, int qual)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d || gm_itemdef_no_trade(id)) return 0;
    long price = (long)qual * (d->value / 4) * 2 / 25;
    int cha = 25, mx = 25; gm_member_attr(assist_bargainer(), 5, &cha, &mx);    /* (who bargains: mods/assist.c) */
    price += price * (25 - cha) / -100;
    return price < 1 ? 0 : price;
}

extern "C" void lootview_draw(void)
{
    const LootModel &l = loot_model;
    if (!lootview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM, red = NsDraw::RED;
    P.page();

    /* the header */
    P.text(f, NsDraw::FS_TITLE, 65, 3, ink, "The loot");
    {
        bool cash = l.cash_fl || l.cash_gr || l.cash_pf;
        P.text(fo, NsDraw::FS_BODY, 65, 12.5f, soft, cash ? "Found " + money_fgp(l.cash_fl, l.cash_gr, l.cash_pf) + " in cash (into the purse)." : std::string("No money found."));
    }
    float bx = 284;
    if (P.button(bx, 4, 315, 15, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_SMALL)) ns_push(LV_LEAVE, 0, 0, 0);
    if (l.shared && l.npile) {                                  /* (one pack: everything into it) */
        if (P.button(bx - 33, 4, bx - 3, 15, "Take all", IM_COL32(40, 92, 34, 235), NsDraw::FS_SMALL)) ns_push(LV_SHARE, 0, 0, 0);
    } else if (l.assist && l.npile) {
        if (P.button(bx - 36, 4, bx - 3, 15, "Share out", IM_COL32(40, 92, 34, 235), NsDraw::FS_SMALL)) ns_push(LV_SHARE, 0, 0, 0);
        if (P.button(bx - 70, 4, bx - 39, 15, "Take all", IM_COL32(40, 92, 34, 235), NsDraw::FS_SMALL)) ns_push(LV_TAKEALL, 0, 0, 0);
    }

    /* the member tabs */
    const float LX0 = 63, LX1 = 180, RX0 = 184, RX1 = 317, LY0 = 31, ROWH = 9; const int NROWS = 13;
    const float LY1 = LY0 + NROWS * ROWH;
    P.text(f, NsDraw::FS_HEAD, LX0 + 1, 21, ink, "The pile");
    {                                                       /* (shared: the tab is whose potions are shown, and whom a potion taken goes to) */
        int n = 0; for (int m = 0; m < 5; m++) if (l.present[m]) n++;
        float tw = n ? (RX1 - RX0) / n : 0, x = RX0;
        for (int m = 0; m < 5; m++) {
            if (!l.present[m]) continue;
            bool sel = m == l.who, h = P.over(x, 19, x + tw - 1, 29);
            ImU32 bg = sel ? IM_COL32(112, 40, 22, 240) : !l.able[m] ? IM_COL32(210, 200, 180, 255) : h ? IM_COL32(214, 190, 150, 255) : IM_COL32(222, 204, 168, 255);
            P.rect(x, 19, x + tw - 1, 29, bg, 2);
            P.text(f, NsDraw::FS_NAME, x + 2.5f, 20.5f, sel ? NsDraw::LIGHT : l.able[m] ? ink : dim, ns_gtext(l.names[m]), -1, x + tw - 2);
            if (l.able[m] && P.hit(x, 19, x + tw - 1, 29)) ns_push(LV_WHO, m, 0, 0);
            x += tw;
        }
    }
    if (l.who != V.last_who) { V.scroll[1] = 0; V.last_who = l.who; }

    int new_hl = -1, new_hr = -1;
    for (int L = 0; L < 2; L++) {
        float x0 = L ? RX0 : LX0, x1 = L ? RX1 : LX1;
        int n = L ? l.nitems : l.npile;
        P.rect(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
        P.frame(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
        bool over = P.over(x0, LY0, x1, LY1) || (!L && !P.over(RX0, LY0, RX1, LY1));
        if (over && ns_in.wheel != 0) { V.scroll[L] -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
        int maxs = n - NROWS; if (maxs < 0) maxs = 0;
        if (V.scroll[L] > maxs) V.scroll[L] = maxs;
        if (V.scroll[L] < 0) V.scroll[L] = 0;
        if (!n) P.text(fo, NsDraw::FS_BODY, x0 + 4, LY0 + 3, dim, L ? (l.shared ? "No potions, nothing spare in the pack." : "Carries nothing.") : "Nothing left in the pile.");
        for (int r = 0; r < NROWS && V.scroll[L] + r < n; r++) {
            int i = V.scroll[L] + r;
            float y = LY0 + r * ROWH;
            int id = L ? l.item[i].id : l.pile[i].id, qual = L ? l.item[i].qual : l.pile[i].qual, count = L ? l.item[i].count : l.pile[i].count;
            int eq = L ? l.item[i].equipped : 0, kept = L ? l.item[i].kept : 0, own = L && l.shared && l.item[i].potion;
            bool h = P.over(x0, y, x1, y + ROWH);
            if (h) { new_hl = L; new_hr = i; }
            if (h) P.rect(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
            else if (r & 1) P.rect(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
            if (eq) P.rect(x0 + 0.5f, y + 1, x0 + 1.8f, y + ROWH - 1, red);
            P.icon(x0 + 6.5f, y + ROWH * 0.5f, 7.0f, id, qual);
            std::string nm = ns_item_name(id);
            if (count > 1) nm += " (" + std::to_string(count) + ")";
            char qs[16] = ""; if (ns_shows_quality(id, qual)) snprintf(qs, sizeof qs, "q%d", qual);
            std::string owner = own ? ns_gtext(l.names[l.who]) + "'s" : std::string();
            const char *tag = eq ? "in use" : own ? owner.c_str() : kept ? "kept" : NULL;
            float tagw = tag ? P.width(fo, NsDraw::FS_SMALL, tag) + 3 : 0;
            float name_end = h ? x1 - (count > 1 ? 36 : 24) : x1 - 3 - P.width(fo, NsDraw::FS_BODY, qs) - 3;
            P.text(f, NsDraw::FS_NAME, x0 + 12, y + 1.3f, ink, nm, -1, name_end - tagw);
            if (tag) {
                float nx = fminf(x0 + 12 + P.width(f, NsDraw::FS_NAME, nm) + 2, name_end - tagw);
                P.rect(nx, y + 2, nx + tagw - 1, y + ROWH - 1.5f, eq ? IM_COL32(140, 30, 18, 230) : IM_COL32(70, 70, 120, 230), 1);
                P.text(fo, NsDraw::FS_SMALL, nx + 1.2f, y + 2.6f, NsDraw::LIGHT, tag);
            }
            if (!h) { P.text(fo, NsDraw::FS_BODY, 0, y + 2.2f, soft, qs, x1 - 3); continue; }
            /* the actions: the whole stack, or one */
            bool can = L || l.room;
            float b1 = x1 - 2;
            ImU32 bc = L ? IM_COL32(120, 44, 24, 235) : IM_COL32(40, 92, 34, 235);
            if (!can) P.text(fo, NsDraw::FS_SMALL, 0, y + 2.4f, red, "no room", b1);
            else if (kept) P.text(fo, NsDraw::FS_SMALL, 0, y + 2.4f, dim, "kept: stays", b1);
            else if (count > 1) {                                   /* (the stack, or one of it: said so) */
                if (P.button(b1 - 17, y + 1, b1, y + ROWH - 1, L ? "Put all" : "Take all", bc, 4.6f)) ns_push(L ? LV_PUT : LV_TAKE, i, 255, 0);
                if (P.button(b1 - 32, y + 1, b1 - 18, y + ROWH - 1, L ? "Put 1" : "Take 1", IM_COL32(120, 84, 40, 235), 4.6f)) ns_push(L ? LV_PUT : LV_TAKE, i, 1, 0);
            } else if (P.button(b1 - 20, y + 1, b1, y + ROWH - 1, L ? "Put back" : "Take", bc, 4.6f)) ns_push(L ? LV_PUT : LV_TAKE, i, 255, 0);
        }
        if (n > NROWS) {
            float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * V.scroll[L] / (float)(n - NROWS);
            P.rect(x1 - 1.6f, ty, x1 - 0.4f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
        }
    }
    if (new_hl >= 0) { V.hover_list = new_hl; V.hover_row = new_hr; V.hover_seq = l.seq; }
    else if (V.hover_seq != l.seq) V.hover_list = -1;

    /* the strip */
    float SY0 = LY1 + 3, SY1 = 196;
    P.rect(63, SY0, 317, SY1, IM_COL32(230, 214, 178, 255), 1.5f);
    if (V.hover_list >= 0 && (V.hover_list ? V.hover_row < l.nitems : V.hover_row < l.npile)) {
        int L = V.hover_list, i = V.hover_row;
        int id = L ? l.item[i].id : l.pile[i].id, qual = L ? l.item[i].qual : l.pile[i].qual, wt = L ? l.item[i].weight : l.pile[i].weight, cnt = L ? l.item[i].count : l.pile[i].count;
        const ItemDef *d = ns_itemdef(id);
        P.icon(70, SY0 + 8, 12.0f, id, qual);
        P.text(f, NsDraw::FS_HEAD, 79, SY0 + 1.5f, ink, ns_item_name(id));
        std::string l1 = std::string(d ? kind_name(d->kind) : "") + ns_quality_words(id, qual) + ", weight " + std::to_string(wt);
        if (cnt > 1) l1 += " each, " + std::to_string(cnt) + " of them";
        if (L && l.item[i].equipped) l1 += "; in use";
        if (L && l.item[i].kept) l1 += "; kept";
        if (config_get_int("assist.loot_worth", 1)) {
            long w = sale_worth(id, qual);
            if (w > 0) l1 += "; a shop pays about " + money_with_total(w) + (cnt > 1 ? " each" : "");
        }
        P.text(fo, NsDraw::FS_BODY, 79, SY0 + 10, soft, l1);
        std::string l2 = ns_item_line(id);
        if (!l2.empty()) P.text(fo, NsDraw::FS_BODY, 79, SY0 + 17, soft, l2);
        std::string use = ns_item_use(id);
        if (!use.empty()) P.para(fo, NsDraw::FS_SMALL, 79, SY0 + (l2.empty() ? 17 : 24), 235, soft, use);
        else if (!L) {                                          /* a weapon or an armour in the pile: against what the member has */
            std::string cmp = ns_item_compare(id, qual, l.who);
            if (!cmp.empty()) P.para(fo, NsDraw::FS_SMALL, 79, SY0 + (l2.empty() ? 17 : 24), 235, ink, cmp);
        }
    } else P.text(fo, NsDraw::FS_BODY, 68, SY0 + 3, dim, l.shared ? "Point at a thing to see it here. Potions go to the member above (each member's own); the rest into the party's pack."
                                                         : "Point at a thing to see it here. Click a name above the right list (or D) for another member.");
    if (l.note[0] && plat_ticks_ms() - l.note_t < 6000) P.text(fo, NsDraw::FS_BODY, 68, SY1 - 8.5f, IM_COL32(112, 40, 22, 255), ns_gtext(l.note));
}
