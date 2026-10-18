// menuview.cpp - the game's menu bar drawn by Darklands+ (mods/menuview.c runs it): the bar over
// the top of the picture, the open menu under its title, a sub-menu beside its item. The pointer
// picks (a click, or letting go of the right button held since it opened the menu), the keyboard
// moves the choice (mods/menuview.c), a click outside closes it.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/menuview.h"
u32 plat_ticks_ms(void);
}

static const ImU32 BAR = IM_COL32(30, 22, 15, 246), BOX = IM_COL32(40, 30, 21, 250), EDGE = IM_COL32(150, 112, 60, 255);
static const ImU32 CREAM = IM_COL32(238, 226, 200, 255), GOLD = IM_COL32(240, 198, 92, 255), GREY = IM_COL32(128, 116, 102, 255);
static const ImU32 HI = IM_COL32(104, 72, 34, 255), DIMK = IM_COL32(186, 168, 140, 255);

static const float BAR_Y1 = 11.0f, ROW = 9.0f;

struct Box { float x0, y0, x1, y1; };

/* a list of items (a menu, or a sub-menu) at (x, y): its box, the row under the pointer */
static Box list_box(NsDraw &P, int first, int n, float x, float y)
{
    float wn = 0, wk = 0, wc = 0;
    for (int i = 0; i < n; i++) {
        const MvItem &it = menu_model.items[first + i];
        wn = fmaxf(wn, P.width(P.f, NsDraw::FS_NAME, ns_gtext(it.name)));
        if (it.key[0]) wk = fmaxf(wk, P.width(P.fo, NsDraw::FS_SMALL, it.key));
        if (it.count > 1) wc = fmaxf(wc, P.width(P.fo, NsDraw::FS_SMALL, std::to_string(it.count)));
    }
    float w = 8.0f + (wc > 0 ? wc + 2.5f : 0) + wn + (wk > 0 ? wk + 7.0f : 0) + 6.0f;
    Box b = { x, y, x + w, y + 2.0f + n * ROW + 1.5f };
    if (b.x1 > 319.5f) { b.x0 -= b.x1 - 319.5f; b.x1 = 319.5f; }
    if (b.x0 < 60.5f) { b.x1 += 60.5f - b.x0; b.x0 = 60.5f; }
    if (b.y1 > 199.5f) { b.y0 -= b.y1 - 199.5f; b.y1 = 199.5f; }
    return b;
}
static void tick(NsDraw &P, float x, float y, ImU32 c)
{
    ImVec2 a = NsDraw::w(x, y + 2.6f), m = NsDraw::w(x + 1.3f, y + 4.0f), e = NsDraw::w(x + 3.6f, y + 0.6f);
    P.dl->AddLine(a, m, c, fmaxf(1.5f, 0.55f * P.k)); P.dl->AddLine(m, e, c, fmaxf(1.5f, 0.55f * P.k));
}
static void draw_list(NsDraw &P, const Box &b, int first, int n, int sel)
{
    P.rect(b.x0 + 0.8f, b.y0 + 0.8f, b.x1 + 0.8f, b.y1 + 0.8f, IM_COL32(0, 0, 0, 90), 1.5f);
    P.rect(b.x0, b.y0, b.x1, b.y1, BOX, 1.5f);
    P.frame(b.x0, b.y0, b.x1, b.y1, EDGE, 1.5f);
    float wc = 0;
    for (int i = 0; i < n; i++) if (menu_model.items[first + i].count > 1) wc = fmaxf(wc, P.width(P.fo, NsDraw::FS_SMALL, std::to_string(menu_model.items[first + i].count)));
    for (int i = 0; i < n; i++) {
        const MvItem &it = menu_model.items[first + i];
        float y = b.y0 + 2.0f + i * ROW;
        if (i == sel) P.rect(b.x0 + 1.0f, y, b.x1 - 1.0f, y + ROW, it.enabled ? HI : IM_COL32(60, 46, 32, 255), 1.0f);
        ImU32 c = !it.enabled ? GREY : i == sel ? GOLD : CREAM;
        if (it.tick) tick(P, b.x0 + 2.2f, y + 2.4f, c);
        float x = b.x0 + 8.0f;
        if (wc > 0) {
            if (it.count > 1) P.text(P.fo, NsDraw::FS_SMALL, 0, y + 1.9f, it.enabled ? DIMK : GREY, std::to_string(it.count), x + wc);
            x += wc + 2.5f;
        }
        P.text(P.f, NsDraw::FS_NAME, x, y + 1.0f, c, ns_gtext(it.name));
        if (it.key[0]) P.text(P.fo, NsDraw::FS_SMALL, 0, y + 1.9f, it.enabled ? DIMK : GREY, it.key, b.x1 - (menu_model.sub_n[first + i] ? 7.0f : 3.5f));
        if (menu_model.sub_n[first + i]) {                     /* a sub-menu: an arrow */
            ImVec2 p0 = NsDraw::w(b.x1 - 4.6f, y + 2.6f), p1 = NsDraw::w(b.x1 - 4.6f, y + 6.4f), p2 = NsDraw::w(b.x1 - 2.4f, y + 4.5f);
            P.dl->AddTriangleFilled(p0, p1, p2, c);
        }
    }
}

extern "C" void menuview_draw(void)
{
    MenuModel &M = menu_model;
    if (!M.open) return;
    NsDraw P;
    /* (NsDraw::begin takes the views' clicks: the menu's are its own, mv_in) */
    P.f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    P.fo = ui_font_overlay ? ui_font_overlay : P.f;
    if (!P.f) return;
    ImVec2 o = NsDraw::w(0, 0), e = NsDraw::w(320, 200);
    P.k = (e.y - o.y) / 200.0f; P.kx = (e.x - o.x) / 320.0f;
    if (P.k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) P.f = ui_font_game_hi[ui_game_font];
    P.fk = ui_face_k(P.f); P.fok = ui_face_k(P.fo);
    P.dl = ImGui::GetBackgroundDrawList();
    P.mx = ns_in.x; P.my = ns_in.y;
    bool click = mv_in.click != 0, rclick = mv_in.rclick != 0;
    int cx = mv_in.cx, cy = mv_in.cy;
    mv_in.click = mv_in.rclick = 0;
    static int lmx = -1, lmy = -1; static unsigned lseq;
    bool moved = P.mx != lmx || P.my != lmy || lseq != M.seq;
    if (lseq != M.seq) moved = false;                         /* (the pointer only counts once it moves) */
    lmx = P.mx; lmy = P.my; lseq = M.seq;

    /* the bar */
    P.rect(60, 0, 320, BAR_Y1, BAR);
    P.dl->AddLine(NsDraw::w(60, BAR_Y1), NsDraw::w(320, BAR_Y1), EDGE, fmaxf(1.0f, 0.4f * P.k));
    float tx[8], tw[8], x = 64;
    float right = 319.0f;                                     /* (Darklands+'s own menu at the right end) */
    for (int j = 0; j < M.nmenus; j++) {
        tw[j] = P.width(P.f, NsDraw::FS_NAME, ns_gtext(M.menu[j].name)) + 8.0f;
        if (M.menu[j].native) { tx[j] = right - tw[j]; right = tx[j] - 1.0f; continue; }
        tx[j] = x; x += tw[j] + 1.0f;
    }
    {   const char *h = M.keyboard ? "Arrows, Enter; Esc closes" : "Click to choose; Esc or a click elsewhere closes";
        float hw = P.width(P.fo, NsDraw::FS_SMALL, h);
        if (x + hw + 4 < right) P.text(P.fo, NsDraw::FS_SMALL, 0, 3.0f, GREY, h, right - 2.5f); }
    int over_title = -1;
    for (int j = 0; j < M.nmenus; j++) if (P.over(tx[j], 0, tx[j] + tw[j], BAR_Y1)) over_title = j;
    if (moved && over_title >= 0 && M.menu[over_title].enabled && (M.cur >= 0 || M.held)) { M.cur = over_title; M.row = -1; M.subrow = -1; }
    for (int j = 0; j < M.nmenus; j++) {
        bool open = j == M.cur;
        if (open) P.rect(tx[j], 1.0f, tx[j] + tw[j], BAR_Y1 - 0.5f, HI, 1.0f);
        else if (j == over_title && M.menu[j].enabled) P.rect(tx[j], 1.0f, tx[j] + tw[j], BAR_Y1 - 0.5f, IM_COL32(64, 46, 26, 255), 1.0f);
        P.text(P.f, NsDraw::FS_NAME, tx[j] + 4.0f, 2.0f, !M.menu[j].enabled ? GREY : open ? GOLD : CREAM, ns_gtext(M.menu[j].name));
    }

    /* the open menu and its sub-menu */
    bool in_any = over_title >= 0;
    int hit_row = -1, hit_sub = -1;
    if (M.cur >= 0 && M.cur < M.nmenus && M.menu[M.cur].n > 0) {
        const MvMenu &m = M.menu[M.cur];
        Box b = list_box(P, m.first, m.n, tx[M.cur], BAR_Y1 + 0.5f);
        int r = P.over(b.x0, b.y0 + 2.0f, b.x1, b.y0 + 2.0f + m.n * ROW) ? (int)((P.my - b.y0 - 2.0f) / ROW) : -1;
        if (P.over(b.x0, b.y0, b.x1, b.y1)) in_any = true;
        if (moved && r >= 0 && r != M.row) { M.row = r; M.subrow = -1; }
        hit_row = r;
        draw_list(P, b, m.first, m.n, M.row);
        int ix = M.row >= 0 ? m.first + M.row : -1;
        if (ix >= 0 && M.sub_n[ix] > 0 && M.items[ix].enabled) {
            Box s = list_box(P, M.sub_first[ix], M.sub_n[ix], b.x1 - 1.0f, b.y0 + M.row * ROW);
            if (s.x0 < b.x1 - 1.0f && b.x0 - (s.x1 - s.x0) + 1.0f >= 60.5f) { float w = s.x1 - s.x0; s.x1 = b.x0 + 1.0f; s.x0 = s.x1 - w; }
            int sr = P.over(s.x0, s.y0 + 2.0f, s.x1, s.y0 + 2.0f + M.sub_n[ix] * ROW) ? (int)((P.my - s.y0 - 2.0f) / ROW) : -1;
            if (P.over(s.x0, s.y0, s.x1, s.y1)) in_any = true;
            if (moved && sr >= 0) M.subrow = sr;
            if (sr >= 0) { hit_sub = sr; hit_row = M.row; }
            draw_list(P, s, M.sub_first[ix], M.sub_n[ix], M.subrow);
        }
    }

    /* choosing */
    auto choose = [&](int row, int sub) -> bool {
        if (M.cur < 0 || row < 0) return false;
        int ix = M.menu[M.cur].first + row;
        if (sub >= 0) { if (!M.items[M.sub_first[ix] + sub].enabled) return false; }
        else if (!M.items[ix].enabled || M.sub_n[ix] > 0) return false;
        menuview_push(MV_CHOOSE, M.cur, row, sub);
        return true;
    };
    if (click) {
        bool on_title = false;
        for (int j = 0; j < M.nmenus; j++)
            if (cx >= tx[j] && cx < tx[j] + tw[j] && cy >= 0 && cy < BAR_Y1) {
                on_title = true;
                if (!M.menu[j].enabled) break;
                if (M.cur == j) menuview_push(MV_CLOSE, 0, 0, 0);
                else { M.cur = j; M.row = -1; M.subrow = -1; }
            }
        if (!on_title) {
            if (hit_sub >= 0) choose(hit_row, hit_sub);
            else if (hit_row >= 0 && M.cur >= 0 && M.sub_n[M.menu[M.cur].first + hit_row] > 0) { /* (its sub-menu is open) */ }
            else if (hit_row >= 0) choose(hit_row, -1);
            else if (!in_any) menuview_push(MV_CLOSE, 0, 0, 0);
        }
    }
    if (rclick) {
        bool chosen = hit_sub >= 0 ? choose(hit_row, hit_sub) : hit_row >= 0 ? choose(hit_row, -1) : false;
        if (!chosen && (!M.held || plat_ticks_ms() - M.t_open > 350) && !in_any) menuview_push(MV_CLOSE, 0, 0, 0);
        M.held = 0;
    }
}
