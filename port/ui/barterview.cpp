/* barterview.cpp - draws the shop screen (mods/barterview.c runs it; docs/NATIVE_SCREENS.md).
 *
 * Over the picture beside the party panel (game x 61..319):
 *   - the shop's name, the quality of its wares, the purse and who holds it, Leave;
 *   - two lists side by side: the merchant's wares (each with its price) and the things of the
 *     member bartering (with what the merchant pays; what is in hand or worn marked "in use"),
 *     the member chosen by the tabs above it;
 *   - Buy / Sell on the row under the pointer (Shift: ten / the whole stack; All on a stack);
 *   - a fixed strip at the bottom describes the row under the pointer (no floating tooltips),
 *     and says what the last purchase or sale did.
 * Drawn from barter_model alone; clicks become actions (BV_*) for the screen's loop. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/nscreen.h"
#include "../mods/barterview.h"
#include "ui.h"
#include "gamedata.h"

extern ImFont *ui_font_text, *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS], *ui_font_game_hi[UI_GAME_FONTS];
extern int ui_game_font;
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
extern "C" u32 plat_ticks_ms(void);
float ui_face_k(ImFont *f);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

namespace {
ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }
std::string gt(const char *s) { return game_text(s, strlen(s)); }
const ItemDef *def(int id) { return id >= 0 && id < (int)gamedata.items.size() ? &gamedata.items[id] : nullptr; }
std::string item_name(int id) { const ItemDef *d = def(id); return d ? game_text(d->name.c_str(), d->name.size()) : "?"; }
std::string money(long pf)
{
    if (pf <= 0) return "0pf";
    long fl = pf / 240, gr = pf % 240 / 12, p = pf % 12;
    std::string s;
    if (fl) s += std::to_string(fl) + "fl";
    if (gr) s += (s.empty() ? "" : " ") + std::to_string(gr) + "gr";
    if (p) s += (s.empty() ? "" : " ") + std::to_string(p) + "pf";
    return s;
}
struct View {
    int scroll[2];          /* first row shown: wares, member's things */
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
    ImFont *f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    ImFont *fo = ui_font_overlay ? ui_font_overlay : f;
    if (!f) return;
    ImVec2 o = g2w(0, 0), e = g2w(320, 200);
    float k = (e.y - o.y) / 200.0f, kx = (e.x - o.x) / 320.0f;   /* (game pixels are taller than wide) */
    if (k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) f = ui_font_game_hi[ui_game_font];
    float fk = ui_face_k(f), fok = ui_face_k(fo);
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImGuiIO &io = ImGui::GetIO();
    const ImU32 ink = IM_COL32(44, 26, 10, 255), ink_soft = IM_COL32(92, 60, 30, 230), ink_dim = IM_COL32(44, 26, 10, 120);
    const ImU32 red = IM_COL32(140, 30, 18, 255), green = IM_COL32(40, 96, 34, 255);
    auto R = [&](float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { dl->AddRectFilled(g2w(x0, y0), g2w(x1, y1), c, rnd * k); };
    auto Rl = [&](float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { dl->AddRect(g2w(x0, y0), g2w(x1, y1), c, rnd * k, 0, fmaxf(1.0f, 0.4f * k)); };
    auto T = [&](ImFont *ff, float gs, float x, float y, ImU32 c, const std::string &s, float right_to = -1, float clip_to = -1) {
        float sz = gs * k * (ff == f ? fk : fok);
        ImVec2 p = g2w(x, y);
        if (right_to >= 0) p.x = g2w(right_to, 0).x - ff->CalcTextSizeA(sz, FLT_MAX, 0, s.c_str()).x;
        if (clip_to >= 0) { ImVec4 cr(p.x, p.y - sz, g2w(clip_to, 0).x, p.y + 2 * sz); dl->AddText(ff, sz, p, c, s.c_str(), nullptr, 0, &cr); }
        else dl->AddText(ff, sz, p, c, s.c_str());
    };
    auto W = [&](ImFont *ff, float gs, const std::string &s) { float sz = gs * k * (ff == f ? fk : fok); return ff->CalcTextSizeA(sz, FLT_MAX, 0, s.c_str()).x / kx; };

    int mx = ns_in.x, my = ns_in.y;
    int cx = -1, cy = -1; bool click = ns_take_click(&cx, &cy);
    bool shift = io.KeyShift;
    auto in = [&](int x, int y, float x0, float y0, float x1, float y1) { return x >= x0 && x < x1 && y >= y0 && y < y1; };
    auto hit = [&](float x0, float y0, float x1, float y1) { return click && in(cx, cy, x0, y0, x1, y1); };

    /* the page */
    R(60, 0, 320, 200, IM_COL32(236, 222, 188, 255));
    R(60, 0, 320, 200, IM_COL32(120, 80, 36, 40));
    R(61.5f, 1.5f, 318.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
    Rl(61.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);

    /* the header: shop, quality, purse, Leave */
    T(f, 8.5f, 65, 3, ink, gt(b.shopname), -1, 205);
    char q[48]; snprintf(q, sizeof q, "Wares of quality %d", b.quality);
    T(fo, 5.5f, 65, 12.5f, ink_soft, q);
    {
        std::string purse = money(b.total_pf), who = b.payer >= 0 && b.payer < 5 ? gt(b.names[b.payer]) : "";
        T(fo, 5.5f, 0, 3.5f, ink_soft, "Purse (" + who + ")", 280);
        T(f, 7.0f, 0, 9.5f, ink, purse, 280);
        bool h = in(mx, my, 284, 4, 315, 15);
        R(284, 4, 315, 15, h ? IM_COL32(140, 44, 24, 255) : IM_COL32(112, 40, 22, 235), 2);
        T(f, 6.5f, 290, 5.5f, IM_COL32(250, 236, 200, 255), "Leave");
        if (hit(284, 4, 315, 15)) ns_push(BV_LEAVE, 0, 0, 0);
    }

    /* the member tabs (right column); right-click: holds the purse */
    const float LX0 = 63, LX1 = 180, RX0 = 184, RX1 = 317, LY0 = 31, ROWH = 9;
    T(f, 7, LX0 + 1, 21, ink, "For sale");
    {
        int n = 0; for (int m = 0; m < 5; m++) if (b.present[m]) n++;
        float tw = n ? (RX1 - RX0) / n : 0, x = RX0;
        for (int m = 0; m < 5; m++) {
            if (!b.present[m]) continue;
            bool sel = m == b.who, h = in(mx, my, x, 19, x + tw - 1, 29);
            R(x, 19, x + tw - 1, 29, sel ? IM_COL32(112, 40, 22, 240) : h ? IM_COL32(214, 190, 150, 255) : IM_COL32(222, 204, 168, 255), 2);
            std::string nm = gt(b.names[m]);
            T(f, 6.0f, x + 2.5f + (m == b.payer ? 4 : 0), 20.5f, sel ? IM_COL32(250, 236, 200, 255) : ink, nm, -1, x + tw - 2);
            if (m == b.payer) dl->AddCircleFilled(g2w(x + 3.5f, 24), 1.6f * k, IM_COL32(214, 172, 60, 255));
            if (click && in(cx, cy, x, 19, x + tw - 1, 29)) ns_push(BV_WHO, m, 0, 0);
            if (ns_in.rclick && in(mx, my, x, 19, x + tw - 1, 29)) { ns_in.rclick = 0; ns_push(BV_PAYER, m, 0, 0); }
            x += tw;
        }
    }
    if (b.who != V.last_who) { V.scroll[1] = 0; V.last_who = b.who; }

    /* the lists */
    const int NROWS = 13;
    float LY1 = LY0 + NROWS * ROWH;
    int new_hl = -1, new_hr = -1;
    for (int L = 0; L < 2; L++) {
        float x0 = L ? RX0 : LX0, x1 = L ? RX1 : LX1;
        int n = L ? b.nitems : b.nwares;
        R(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
        Rl(x0, LY0 - 1, x1, LY1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
        bool over = in(mx, my, x0, LY0, x1, LY1) || (!L && !in(mx, my, RX0, LY0, RX1, LY1));
        if (over && ns_in.wheel != 0) { V.scroll[L] -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
        int maxs = n - NROWS; if (maxs < 0) maxs = 0;
        if (V.scroll[L] > maxs) V.scroll[L] = maxs;
        if (V.scroll[L] < 0) V.scroll[L] = 0;
        if (!n) T(fo, 5.5f, x0 + 4, LY0 + 3, ink_dim, L ? "Nothing to sell." : "Nothing for sale today.");
        for (int r = 0; r < NROWS && V.scroll[L] + r < n; r++) {
            int i = V.scroll[L] + r;
            float y = LY0 + r * ROWH;
            int id, qual, price, count = 1, eq = 0;
            if (L) { const BvItem &it = b.item[i]; id = it.id; qual = it.qual; price = it.price; count = it.count; eq = it.equipped; }
            else { const BvWare &w = b.ware[i]; id = w.id; qual = w.qual; price = w.price; }
            bool h = in(mx, my, x0, y, x1, y + ROWH);
            if (h) { new_hl = L; new_hr = i; }
            if (h) R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
            else if (r & 1) R(x0 + 0.5f, y, x1 - 0.5f, y + ROWH, IM_COL32(240, 230, 204, 255));
            if (eq) R(x0 + 0.5f, y + 1, x0 + 1.8f, y + ROWH - 1, red);
            ImVec2 ic = g2w(x0 + 6.5f, y + ROWH * 0.5f);
            ui_item_icon(dl, ic, 7.0f * k, id);
            std::string nm = item_name(id);
            if (count > 1) nm += " (" + std::to_string(count) + ")";
            char qs[16]; snprintf(qs, sizeof qs, "q%d", qual);
            std::string ps = price > 0 ? money(price) : L ? "-" : "?";
            float pw = W(fo, 5.2f, ps);
            float name_end = x1 - 2 - pw - 5 - W(fo, 5.0f, qs) - 3;
            float tagw = eq ? W(fo, 4.6f, "in use") + 3 : 0;
            T(f, 5.8f, x0 + 12, y + 1.3f, ink, nm, -1, name_end - tagw);
            if (eq) {
                float nx = fminf(x0 + 12 + W(f, 5.8f, nm) + 2, name_end - tagw);
                R(nx, y + 2, nx + tagw - 1, y + ROWH - 1.5f, IM_COL32(140, 30, 18, 230), 1);
                T(fo, 4.6f, nx + 1.2f, y + 2.6f, IM_COL32(250, 236, 200, 255), "in use");
            }
            T(fo, 5.0f, 0, y + 2.2f, ink_soft, qs, x1 - 2 - pw - 5);
            bool cant = !L && price > b.total_pf;
            T(fo, 5.2f, 0, y + 2.0f, price <= 0 ? ink_dim : cant ? red : ink, ps, x1 - 2);
            /* the action on the row under the pointer */
            if (h) {
                const char *lab = L ? "Sell" : "Buy";
                bool two = L && count > 1;
                float bw = 15, bx1 = x1 - 2 - pw - 2, bx0 = bx1 - bw, ax0 = bx0 - (two ? 13 : 0);
                if (!(L && price <= 0)) {
                    R(ax0 - 1, y + 0.8f, bx1, y + ROWH - 0.8f, IM_COL32(226, 204, 160, 255));
                    bool hb = in(mx, my, bx0, y, bx1, y + ROWH);
                    R(bx0, y + 1, bx1, y + ROWH - 1, hb ? (L ? IM_COL32(150, 56, 30, 255) : IM_COL32(52, 112, 44, 255)) : (L ? IM_COL32(120, 44, 24, 235) : IM_COL32(40, 92, 34, 235)), 1.5f);
                    T(fo, 5.0f, bx0 + (bw - W(fo, 5.0f, lab)) * 0.5f, y + 2.2f, IM_COL32(250, 240, 214, 255), lab);
                    if (hit(bx0, y, bx1, y + ROWH)) ns_push(L ? BV_SELL : BV_BUY, i, L ? (shift ? 255 : 1) : (shift ? 10 : 1), 0);
                    if (two) {
                        bool ha = in(mx, my, ax0, y, bx0 - 1, y + ROWH);
                        R(ax0, y + 1, bx0 - 1, y + ROWH - 1, ha ? IM_COL32(150, 56, 30, 255) : IM_COL32(120, 44, 24, 200), 1.5f);
                        T(fo, 5.0f, ax0 + 1.5f, y + 2.2f, IM_COL32(250, 240, 214, 255), "All");
                        if (hit(ax0, y, bx0 - 1, y + ROWH)) ns_push(BV_SELL, i, 255, 0);
                    }
                }
            }
        }
        if (n > NROWS) {                                       /* where in the list */
            float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * V.scroll[L] / (float)(n - NROWS);
            R(x1 - 1.6f, ty, x1 - 0.4f, ty + th, IM_COL32(120, 80, 36, 140), 0.6f);
        }
    }
    if (new_hl >= 0) { V.hover_list = new_hl; V.hover_row = new_hr; V.hover_seq = b.seq; }
    else if (V.hover_seq != b.seq) V.hover_list = -1;

    /* the strip: the row under the pointer (or the last one), what the last action did */
    float SY0 = LY1 + 3, SY1 = 196;
    R(63, SY0, 317, SY1, IM_COL32(230, 214, 178, 255), 1.5f);
    if (V.hover_list >= 0) {
        int L = V.hover_list, i = V.hover_row;
        bool ok = L ? i < b.nitems : i < b.nwares;
        if (ok) {
            int id = L ? b.item[i].id : b.ware[i].id, qual = L ? b.item[i].qual : b.ware[i].qual;
            int price = L ? b.item[i].price : b.ware[i].price, wt = L ? b.item[i].weight : b.ware[i].weight;
            const ItemDef *d = def(id);
            ui_item_icon(dl, g2w(70, SY0 + 8), 12.0f * k, id);
            T(f, 7.0f, 79, SY0 + 1.5f, ink, item_name(id));
            std::string l1 = std::string(d ? kind_name(d->kind) : "") + ", quality " + std::to_string(qual) + ", weight " + std::to_string(wt);
            if (L && b.item[i].count > 1) l1 += " each, " + std::to_string(b.item[i].count) + " of them";
            T(fo, 5.3f, 79, SY0 + 10, ink_soft, l1);
            std::string l2;
            if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
                int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + w));
                static const char *SK[7] = { "edged", "impact", "flail", "polearm", "thrown", "bow", "missile device" };
                l2 = std::string(sk < 7 ? SK[sk] : "?") + ", " + ((cp >> 4) >= 2 ? "two hands" : "one hand") + ", damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15);
            } else if (d && d->type >= 67 && d->type <= 84) {
                static const char *HN[6] = { "none", "soft", "stiff", "scale", "mail", "plate" };
                int h = rb(G_FAR20A5, (u16)(0x781e + d->type));
                l2 = std::string(d->type <= 75 ? "Covers body and arms" : "Covers the legs") + ", hardness " + std::to_string(h) + " (" + (h >= 0 && h < 6 ? HN[h] : "?") + ")";
            }
            if (!l2.empty()) T(fo, 5.3f, 79, SY0 + 17, ink_soft, l2);
            std::string pr = L ? (price > 0 ? "The merchant pays " + money(price) : "The merchant will not buy this")
                               : "Costs " + money(price) + (price > b.total_pf ? " - more than the purse holds" : "");
            if (L && b.item[i].equipped) pr += "; in use";
            T(fo, 5.6f, 0, SY0 + 1.8f, L ? (price > 0 ? green : red) : (price > b.total_pf ? red : ink), pr, 314);
        }
    } else T(fo, 5.3f, 68, SY0 + 3, ink_dim, "Point at a ware or at one of your things to see it here.");
    if (b.note[0] && plat_ticks_ms() - b.note_t < 6000) T(fo, 5.5f, 68, SY1 - 8.5f, IM_COL32(112, 40, 22, 255), gt(b.note));
    else T(fo, 4.8f, 68, SY1 - 7.5f, ink_dim, "Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  right-click a name: that member pays");
    ns_in.rclick = 0;
}
