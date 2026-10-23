/* cacheview.cpp - draws the store room (mods/cacheview.c runs it; docs/NATIVE_SCREENS.md).
 *
 * The store and the member's things side by side; Take (the whole stack, or 1) on a stored row,
 * Put (the stack, or 1) on the member's; the members as tabs (one pack for the party: none, the
 * party's pack with each potion marked with its owner); Put away (all not in use);
 * a fixed strip describes the row under the pointer. Drawn from cache_model alone. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/cacheview.h"
#include "nsdraw.h"

extern "C" u32 plat_ticks_ms(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

namespace {
struct CacheView {
    int scroll[2] = {};
    int last_who = -1;
    int hover_list = -1, hover_row = -1; unsigned hover_seq = 0;
} V;
}

extern "C" int cacheview_showing(void) { return ns_screen == NS_CACHE && cache_model.open && !ns_hidden(); }

extern "C" void cacheview_draw(void)
{
    const CacheModel &l = cache_model;
    if (!cacheview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM, red = NsDraw::RED;
    ns_counter_backdrop(P, 30, 0, true);                     /* (a cellar: stone walls, the table in it) */
    const ImU32 cream = IM_COL32(246, 232, 200, 255), cream_soft = IM_COL32(226, 212, 186, 235);

    /* the header */
    P.text(f, NsDraw::FS_TITLE, 65, 3, cream, "The store room");
    if (l.every_inn) P.text(fo, NsDraw::FS_BODY, 65, 12.5f, cream_soft, "The party's items: the same store at every inn.");
    else if (l.place[0]) P.text(fo, NsDraw::FS_BODY, 65, 12.5f, cream_soft, "The party's items kept in " + ns_gtext(l.place) + ", for when it returns.");
    float bx = 284;
    if (P.button(bx, 4, 315, 15, "Leave", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) ns_push(CV_LEAVE, 0, 0, 0);
    if (l.nitems && P.button(bx - 46, 4, bx - 3, 15, "Put away", IM_COL32(40, 92, 34, 235), NsDraw::FS_HEADBTN)) ns_push(CV_PUTAWAY, 0, 0, 0);
    if (l.nitems && P.over(bx - 46, 4, bx - 3, 15)) {
        if (l.shared) ImGui::SetTooltip("Everything in the party's pack goes into the store, except kept items (Keep on the character sheet) and potions.");
        else ImGui::SetTooltip("Everything %s carries that is not in use goes into the store.", ns_gtext(l.names[l.who]).c_str());
    }

    /* the member tabs */
    const float LX0 = 63, LX1 = 180, RX0 = 184, RX1 = 317, LY0 = 31, ROWH = 9; const int NROWS = 13;
    const float LY1 = LY0 + NROWS * ROWH;
    P.text(f, NsDraw::FS_HEAD, LX0 + 1, 21, cream, "In store");
    if (l.shared) P.text(f, NsDraw::FS_HEAD, RX0 + 1, 21, cream, "The party's pack");   /* (one pack: no member to choose) */
    else {
        int n = 0; for (int m = 0; m < 5; m++) if (l.present[m]) n++;
        float tw = n ? (RX1 - RX0) / n : 0, x = RX0;
        for (int m = 0; m < 5; m++) {
            if (!l.present[m]) continue;
            bool sel = m == l.who, h = P.over(x, 19, x + tw - 1, 29);
            ImU32 bg = sel ? IM_COL32(112, 40, 22, 240) : !l.able[m] ? IM_COL32(210, 200, 180, 255) : h ? IM_COL32(214, 190, 150, 255) : IM_COL32(222, 204, 168, 255);
            if (sel || l.able[m]) P.tab(x, 19, x + tw - 1, 29, sel, h, 2); else P.rect(x, 19, x + tw - 1, 29, bg, 2);
            P.text(f, NsDraw::FS_NAME, x + 2.5f, 20.5f, sel ? NsDraw::LIGHT : l.able[m] ? ink : dim, ns_gtext(l.names[m]), -1, x + tw - 2);
            if (l.able[m] && P.hit(x, 19, x + tw - 1, 29)) ns_push(CV_WHO, m, 0, 0);
            x += tw;
        }
    }
    if (l.who != V.last_who) { V.scroll[1] = 0; V.last_who = l.who; }

    int new_hl = -1, new_hr = -1;
    for (int L = 0; L < 2; L++) {
        float x0 = L ? RX0 : LX0, x1 = L ? RX1 : LX1;
        int n = L ? l.nitems : l.nstore;
        ns_sheet(P, x0, LY0 - 1, x1, LY1 + 1, IM_COL32(250, 242, 222, 255));
        P.frame(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
        bool over = P.over(x0, LY0, x1, LY1) || (!L && !P.over(RX0, LY0, RX1, LY1));
        if (over && ns_in.wheel != 0) { V.scroll[L] -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
        int maxs = n - NROWS; if (maxs < 0) maxs = 0;
        if (V.scroll[L] > maxs) V.scroll[L] = maxs;
        if (V.scroll[L] < 0) V.scroll[L] = 0;
        if (!n) P.text(fo, NsDraw::FS_BODY, x0 + 4, LY0 + 3, dim, L ? (l.shared ? "No potions or spare items in the pack." : "Carries nothing.") : "Nothing stored here.");
        for (int r = 0; r < NROWS && V.scroll[L] + r < n; r++) {
            int i = V.scroll[L] + r;
            float y = LY0 + r * ROWH;
            int id = L ? l.item[i].id : l.store[i].id, qual = L ? l.item[i].qual : l.store[i].qual, count = L ? l.item[i].count : l.store[i].count;
            int eq = L ? l.item[i].equipped : 0, kept = L ? l.item[i].kept : 0, own = L && l.shared && l.item[i].potion;
            bool h = P.over(x0, y, x1, y + ROWH);
            if (h) { new_hl = L; new_hr = i; }
            if (h) P.rect(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
            else if (r & 1) P.rect(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
            P.icon(x0 + 6.5f, y + ROWH * 0.5f, 7.0f, id, qual);
            std::string nm = ns_item_name(id);
            if (count > 1) nm += " (" + std::to_string(count) + ")";
            char qs[16]; snprintf(qs, sizeof qs, "%s", ns_quality_tag(id, qual).c_str());
            int ow = L ? l.item[i].owner : -1;
            std::string owner = own && ow >= 0 && ow < 5 ? ns_gtext(l.names[ow]) + "'s" : std::string();
            if (eq) P.rect(x0 + 0.5f, y + 1, x0 + 1.8f, y + ROWH - 1, NsDraw::RED);   /* equipped: a red mark at its left (said in the strip) */
            const char *tag = own ? owner.c_str() : kept ? "kept" : NULL;
            float tagw = tag ? P.tag_width(tag) : 0;
            float name_end = h ? x1 - (count > 1 ? 36 : 24) : x1 - 3 - P.width(fo, NsDraw::FS_BODY, qs) - 3;
            P.text(f, NsDraw::FS_NAME, x0 + 12, y + 1.3f, ink, nm, -1, name_end - tagw);
            if (tag) {
                float nx = fminf(x0 + 12 + P.width(f, NsDraw::FS_NAME, nm) + 2, name_end - tagw);
                P.tag(nx, y, ROWH, tag, own ? NsDraw::TAG_OWNER : NsDraw::TAG_KEPT);
            }
            if (!h) { P.text(fo, NsDraw::FS_BODY, 0, y + 2.2f, soft, qs, x1 - 3); continue; }
            /* the actions: the whole stack, or one */
            bool can = L || l.room;
            float b1 = x1 - 2;
            ImU32 bc = L ? IM_COL32(120, 44, 24, 235) : IM_COL32(40, 92, 34, 235);
            if (!can) P.text(fo, NsDraw::FS_SMALL, 0, y + 2.4f, red, "no room", b1);
            else if (count > 1) {                                   /* (the stack, or one of it: said so) */
                if (P.button(b1 - 17, y + 1, b1, y + ROWH - 1, L ? "Put all" : "Take all", bc, 4.6f)) ns_push(L ? CV_PUT : CV_TAKE, i, 255, 0);
                if (P.button(b1 - 32, y + 1, b1 - 18, y + ROWH - 1, L ? "Put 1" : "Take 1", IM_COL32(120, 84, 40, 235), 4.6f)) ns_push(L ? CV_PUT : CV_TAKE, i, 1, 0);
            } else if (P.button(b1 - 20, y + 1, b1, y + ROWH - 1, L ? "Put" : "Take", bc, 4.6f)) ns_push(L ? CV_PUT : CV_TAKE, i, 255, 0);
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
    ns_sheet(P, 63, SY0, 317, SY1, IM_COL32(236, 222, 188, 255));
    if (V.hover_list >= 0 && (V.hover_list ? V.hover_row < l.nitems : V.hover_row < l.nstore)) {
        int L = V.hover_list, i = V.hover_row;
        int id = L ? l.item[i].id : l.store[i].id, qual = L ? l.item[i].qual : l.store[i].qual, wt = L ? l.item[i].weight : l.store[i].weight, cnt = L ? l.item[i].count : l.store[i].count;
        NsStripThing t;
        t.id = id; t.qual = qual; t.weight = wt; t.count = cnt;
        t.status = L && l.item[i].equipped ? "equipped" : L && l.item[i].kept ? "kept" : "";
        t.cmp_who = L ? -1 : l.shared ? -2 : l.who;      /* (one pack: whom in the party it would suit) */
        ns_item_strip(P, SY0, t);
    } else P.text(fo, NsDraw::FS_BODY, 68, SY0 + 3, dim, l.shared ? "Point at an item to see it here; F11 shows what you can do."
                                                         : "Point at an item to see it here. Click a name above the right list (or press C) for another member.");
    if (l.note[0] && plat_ticks_ms() - l.note_t < 6000) P.text(fo, NsDraw::FS_BODY, 68, SY1 - 8.5f, IM_COL32(112, 40, 22, 255), ns_gtext(l.note));
}
