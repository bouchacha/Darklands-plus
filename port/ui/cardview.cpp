/* cardview.cpp - draws the card screen (mods/cardview.c runs it): the card's picture stays the
 * game's own (with its palette, frame and any replacement art), the text and the options are
 * set here in the chosen typeface, sized to the text block the card file gives for them.
 *
 *  - the narrative in paragraphs, the first letter as an illuminated initial (the original
 *    drew a 21-pixel picture of it);
 *  - the options as rows: the hovered one highlighted, a number for the keys, the chance of
 *    success at the end of the row, options that cannot be chosen greyed;
 *  - an option that calls on a saint or uses an item opens, under it, the list of who can
 *    (the original's pop-up box beside the option), each with its tooltip;
 *  - while the options are up the picture is dimmed (the original faded it);
 *  - everything shrinks a little when a long text would run off the card.
 * The rows' places go back to the engine in game pixels (cardview.row_y0 ...), where the
 * game's own pointer is tested against them. */
#include "imgui.h"
#include <math.h>
#include <string.h>
#include <string>
#include <vector>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/cardview.h"
#include "../mods/assist.h"
#include "ui.h"
#include "gamedata.h"

extern ImFont *ui_font_text, *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS], *ui_font_game_hi[UI_GAME_FONTS];
extern int ui_game_font;
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
extern "C" u32 plat_ticks_ms(void);
float ui_face_k(ImFont *f);                                   /* overlay.cpp */
void ui_card_tip(ImVec2 pos, ImVec2 pivot, int item, int saint);

static ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }

struct Line { std::string t; float x, y; };
struct Laid {
    std::vector<Line> lines;
    float y0, y1;            /* the row (options), window pixels */
    float text_end_x, last_y;
};

/* words of t, wrapped to width w (the first `indent_lines` lines narrower by `indent`) */
static void wrap(ImFont *f, float size, const std::string &t, float w, float indent, int indent_lines,
                 std::vector<std::string> &out, float cont = 0)
{
    out.clear();
    std::string line, word;
    auto width = [&](const std::string &s) { return f->CalcTextSizeA(size, FLT_MAX, 0, s.c_str()).x; };
    auto avail = [&]() { return (int)out.size() < indent_lines ? w - indent : w - (out.empty() ? 0 : cont); };
    auto flush_word = [&]() {
        if (word.empty()) return;
        std::string cand = line.empty() ? word : line + " " + word;
        if (!line.empty() && width(cand) > avail()) { out.push_back(line); line = word; }
        else line = cand;
        word.clear();
    };
    for (size_t i = 0; i <= t.size(); i++) {
        char ch = i < t.size() ? t[i] : 0;
        if (ch == ' ' || ch == 0 || ch == '\n') {
            flush_word();
            if (ch == '\n') { out.push_back(line); line.clear(); }
        } else word += ch;
    }
    if (!line.empty()) out.push_back(line);
}

static std::string gtext(const char *s) { return game_text(s, strlen(s)); }

extern "C" void cardview_draw(void)
{
    CardView &c = cardview;
    if (!c.phase) return;
    if (c.nested == 1 || (c.nested == 2 && g_rw(0xa2e3))) return;   /* a member's sheet ..., or the menu bar open over the card */
    ImFont *f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    if (!f) return;
    ImVec2 top = g2w(0, 0), bot = g2w(0, 200);
    float k = (bot.y - top.y) / 200.0f;                         /* window pixels per game row */
    if (k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) f = ui_font_game_hi[ui_game_font];
    ImFont *fs = ui_font_overlay ? ui_font_overlay : f;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    const bool dark = c.dark;                                    /* light letters on a dark picture */
    const bool menu = c.phase == 2;
    const ImU32 ink = dark ? IM_COL32(246, 234, 208, 255) : IM_COL32(44, 26, 10, 255);
    const ImU32 ink_dim = dark ? IM_COL32(246, 234, 208, 115) : IM_COL32(44, 26, 10, 110);
    const ImU32 ink_soft = dark ? IM_COL32(220, 200, 165, 220) : IM_COL32(92, 60, 30, 230);
    float fk = ui_face_k(f);

    /* the card window: the dimmed picture under an option list */
    ImVec2 cw0 = g2w(67, 6), cw1 = g2w(312, 194);
    if (menu || (c.how && c.how != 2)) dl->AddRectFilled(cw0, cw1, IM_COL32(8, 5, 2, menu ? 120 : 60));

    /* the chances (mods/assist.c asks the game's option handlers) */
    const CardChances &cc = card_chances;
    bool chances = assist_chances && cc.active && cc.n == c.nlines && plat_ticks_ms() - cc.polled < 400;
    int diff = g_rb(G_DIFFICULTY) & 0x7f;
    int cmode = assist_chances == 2 ? 0 : diff;
    int members = (s16)g_rw(G_PARTY_SIZE);
    auto chance_shown = [&](int line) { return menu && chances && cmode < 2 && line >= 0 && line < 20 && cc.line[line].show; };

    /* lay the text out at a size that fits (the block's box, down to the frame) */
    float X0 = g2w((float)c.x, 0).x, X1 = g2w((float)(c.x + c.w), 0).x;
    float bottom = g2w(0, 191).y;
    float Y0 = g2w(0, (float)c.y).y;
    float scale = 1.0f;
    std::vector<Laid> laid(c.npieces);
    float size = 0, L = 0, gap = 0, pad = 0, cap = 0, total = 0;
    std::vector<std::string> ws;
    std::vector<std::string> utf(c.npieces);
    for (int j = 0; j < c.npieces; j++) utf[j] = gtext(c.piece[j].text);
    for (int tries = 0; tries < 10; tries++) {
        size = 7.1f * k * fk * scale;
        L = 9.0f * k * scale;
        gap = 3.0f * k * scale;
        pad = 1.2f * size;                                       /* an option's number */
        cap = c.dropcap ? 2 * L - 1.0f * k : 0;
        float y = Y0;
        for (int j = 0; j < c.npieces; j++) {
            const CvPiece &p = c.piece[j];
            Laid &la = laid[j];
            la.lines.clear();
            float x = X0, w = X1 - X0;
            float ind = 0; int indl = 0;
            if (j == 0 && cap > 0) { ind = cap + 0.35f * size; indl = 2; }
            if (p.is_opt) { x += pad; w -= pad; }
            std::string body = utf[j];
            float hang = 0;                                      /* the lines of an option under its words, after the "..." */
            if (p.is_opt && p.lead[0]) { std::string ld = gtext(p.lead); hang = f->CalcTextSizeA(size, FLT_MAX, 0, ld.c_str()).x; body = ld + body; }
            float reserve = p.is_opt && p.type == -1 ? 1.6f * size : 0;     /* room for the crossed blades */
            wrap(f, size, body, w - reserve, ind, indl, ws, hang);
            if (p.is_opt && j > 0 && !c.piece[j - 1].is_opt) y += gap * 0.6f;
            la.y0 = y - 0.12f * L;
            for (size_t i = 0; i < ws.size(); i++) {
                float lx = x + ((int)i < indl ? ind : i > 0 ? hang : 0);
                la.lines.push_back({ ws[i], lx, y });
                la.text_end_x = lx + f->CalcTextSizeA(size, FLT_MAX, 0, ws[i].c_str()).x;
                la.last_y = y;
                y += L;
            }
            if (ws.empty()) y += L;
            if (j == 0 && cap > 0 && (int)ws.size() < 2) y = fmaxf(y, Y0 + cap + 0.2f * L);
            la.y1 = y - 0.12f * L + 0.5f * k;
            /* the members' saints or items under the option */
            if (p.is_opt && c.popup_piece == j) y += (float)c.nentries * L * 0.92f + (c.nentries ? 0.4f * L : L);
            if (!p.is_opt) y += gap;
        }
        total = y - Y0;
        if (y <= bottom || scale < 0.74f) break;
        scale *= 0.94f;
    }
    float shift = 0;                                            /* still too long: start higher */
    if (Y0 + total > bottom) { shift = fminf(Y0 + total - bottom, Y0 - g2w(0, 9).y); if (shift < 0) shift = 0; }
    float yend = Y0 + total - shift;

    /* the box behind the text */
    float bx0 = X0 - 3.5f * k, bx1 = X1 + 3.5f * k, by0 = Y0 - shift - 3.5f * k, by1 = yend + 1.0f * k;
    {
        ImU32 bg = dark ? IM_COL32(14, 9, 4, menu ? 175 : 150) : IM_COL32(240, 226, 192, menu ? 222 : 196);
        ImU32 edge = dark ? IM_COL32(170, 125, 65, 120) : IM_COL32(120, 80, 36, 110);
        dl->AddRectFilled(ImVec2(bx0, by0), ImVec2(bx1, by1), bg, 5.0f * k * 0.5f);
        dl->AddRect(ImVec2(bx0, by0), ImVec2(bx1, by1), edge, 5.0f * k * 0.5f, 0, fmaxf(1.0f, 0.35f * k));
    }
    {   /* the box in game pixels, for the engine's pointer tests */
        ImVec2 a = g2w(0, 0), b = g2w(320, 0);
        float kx = (b.x - a.x) / 320.0f;
        c.box_x0 = (bx0 - a.x) / kx; c.box_x1 = (bx1 - a.x) / kx;
    }
    auto wy2g = [&](float wy) { return (wy - top.y) / k; };

    /* the illuminated initial */
    if (c.dropcap) {
        ImVec2 p0(X0, Y0 - shift - 0.05f * L), p1(X0 + cap, Y0 - shift - 0.05f * L + cap);
        dl->AddRectFilled(p0, p1, IM_COL32(112, 24, 16, 255), 1.5f * k);
        float s = cap * 0.16f;                                  /* a diaper of small diamonds */
        for (float yy = p0.y + s; yy < p1.y - s * 0.5f; yy += 2 * s)
            for (float xx = p0.x + s + (fmodf((yy - p0.y) / (2 * s), 2.0f) > 0.5f ? s : 0); xx < p1.x - s * 0.5f; xx += 2 * s) {
                ImVec2 q[4] = { ImVec2(xx, yy - s * 0.45f), ImVec2(xx + s * 0.45f, yy), ImVec2(xx, yy + s * 0.45f), ImVec2(xx - s * 0.45f, yy) };
                dl->AddConvexPolyFilled(q, 4, IM_COL32(150, 44, 30, 255));
            }
        dl->AddRect(p0, p1, IM_COL32(214, 172, 92, 255), 1.5f * k, 0, fmaxf(1.0f, 0.55f * k));
        char t[2] = { (char)c.dropcap, 0 };
        float cs = cap * 1.25f;                                 /* the letter's ink centred in the square */
        const ImFontGlyph *g = f->FindGlyph((ImWchar)c.dropcap);
        ImVec2 at(p0.x, p0.y);
        if (g) {
            float sc = cs / f->FontSize, gw = (g->X1 - g->X0) * sc, gh = (g->Y1 - g->Y0) * sc;
            if (gh > cap * 0.74f) { cs *= cap * 0.74f / gh; sc = cs / f->FontSize; gw = (g->X1 - g->X0) * sc; gh = (g->Y1 - g->Y0) * sc; }
            at = ImVec2(p0.x + (cap - gw) * 0.5f - g->X0 * sc, p0.y + (cap - gh) * 0.5f - g->Y0 * sc);
        }
        dl->AddText(f, cs, ImVec2(at.x + 0.5f * k, at.y + 0.5f * k), IM_COL32(40, 6, 4, 200), t);
        dl->AddText(f, cs, at, IM_COL32(250, 236, 200, 255), t);
    }

    int tip_item = -1, tip_saint = -1; ImVec2 tip_at;
    for (int j = 0; j < c.npieces; j++) {
        const CvPiece &p = c.piece[j];
        Laid &la = laid[j];
        float y0 = la.y0 - shift, y1 = la.y1 - shift;
        bool hov = menu && p.is_opt && p.state == 1 && (c.hover == j || (c.key_line >= 0 && p.line == c.key_line));
        if (hov) {
            ImU32 hl = dark ? IM_COL32(255, 214, 140, 46) : IM_COL32(150, 92, 30, 52);
            dl->AddRectFilled(ImVec2(X0 - 1.5f * k, y0), ImVec2(X1 + 1.5f * k, y1), hl, 1.5f * k);
            dl->AddRectFilled(ImVec2(X0 - 1.5f * k, y0), ImVec2(X0 - 0.6f * k, y1), dark ? IM_COL32(240, 190, 100, 220) : IM_COL32(150, 60, 20, 220));
        }
        ImU32 col = !p.is_opt ? ink : p.state == 1 ? ink : ink_dim;
        if (p.is_opt && menu && p.state == 1 && p.line >= 0 && p.line < 9) {           /* the key that chooses it */
            char n[4]; snprintf(n, sizeof n, "%d", p.line + 1);
            float ns = size * 0.72f;
            ImVec2 nt = fs->CalcTextSizeA(ns, FLT_MAX, 0, n);
            ImVec2 cpos(X0 + pad * 0.42f, la.lines.empty() ? y0 + 0.5f * L : la.lines[0].y - shift + size * 0.52f);
            float r = ns * 0.62f;
            dl->AddCircle(cpos, r, ink_soft, 16, fmaxf(1.0f, 0.3f * k));
            dl->AddText(fs, ns, ImVec2(cpos.x - nt.x * 0.5f, cpos.y - nt.y * 0.5f), ink_soft, n);
        }
        if (p.is_opt && p.type == -1 && p.state == 1) {          /* a fight: two crossed blades */
            ImVec2 cpos(X1 - 0.9f * size, (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.5f);
            float r = size * 0.36f, th = fmaxf(1.2f, 0.32f * k);
            ImU32 bc = dark ? IM_COL32(230, 120, 100, 230) : IM_COL32(140, 30, 20, 230);
            dl->AddLine(ImVec2(cpos.x - r, cpos.y + r), ImVec2(cpos.x + r, cpos.y - r), bc, th);
            dl->AddLine(ImVec2(cpos.x + r, cpos.y + r), ImVec2(cpos.x - r, cpos.y - r), bc, th);
        }
        for (const Line &l : la.lines) dl->AddText(f, size, ImVec2(l.x, l.y - shift), col, l.t.c_str());
        if (p.is_opt && (p.type == 1 || p.type == 2) && p.state == 1) {               /* opens a list: a small arrow */
            float ax = la.text_end_x + 0.45f * size, ay = la.last_y - shift + size * 0.55f, r = size * 0.22f;
            bool open = c.popup_piece == j;
            ImU32 ac = ink_soft;
            if (open) dl->AddTriangleFilled(ImVec2(ax - r, ay - r * 0.5f), ImVec2(ax + r, ay - r * 0.5f), ImVec2(ax, ay + r * 0.7f), ac);
            else dl->AddTriangleFilled(ImVec2(ax - r * 0.5f, ay - r), ImVec2(ax - r * 0.5f, ay + r), ImVec2(ax + r * 0.7f, ay), ac);
        }
        if (p.is_opt) { c.row_y0[j] = wy2g(y0); c.row_y1[j] = wy2g(y1); }
        else c.row_y0[j] = c.row_y1[j] = -1;
        /* the chance of success */
        if (p.is_opt && chance_shown(p.line)) {
            int pct = cc.line[p.line].pct, who = cc.line[p.line].who;
            char main[40], sub[24] = "";
            if (cmode == 0) snprintf(main, sizeof main, "%d%%", pct);
            else {
                int q = pct / 20; if (q > 6) q = 6;
                u16 pp = g_rw((u16)(0x4fce + 2 * q));
                if (pct <= 0) snprintf(main, sizeof main, "hopeless");
                else g_str(G_DGROUP, pp, main, sizeof main);
            }
            if (who >= 0 && who < 5 && members > 1) { char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(who) + G_M_SHORTNAME), nm, sizeof nm); snprintf(sub, sizeof sub, "%s", game_text(nm, strlen(nm)).c_str()); }
            else if (who == -2 && members > 1) snprintf(sub, sizeof sub, "all");
            float ms = size * 0.82f, ss = size * 0.66f;
            ImVec2 mt = fs->CalcTextSizeA(ms, FLT_MAX, 0, main), st = sub[0] ? fs->CalcTextSizeA(ss, FLT_MAX, 0, sub) : ImVec2(0, 0);
            float bw = mt.x + (sub[0] ? st.x + 0.3f * ms : 0) + 0.6f * ms, bh = ms * 1.02f;
            /* beside the box, level with the option's first line: a tab on the card's edge */
            float cy = (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.52f;
            float bxl = X1 + 4.5f * k, bxr = bxl + bw;
            ImU32 fg = cmode == 1 ? IM_COL32(245, 228, 185, 255)
                     : pct >= 70 ? IM_COL32(150, 230, 120, 255) : pct >= 40 ? IM_COL32(250, 210, 95, 255) : IM_COL32(250, 125, 100, 255);
            ImVec2 q0(bxl, cy - bh * 0.5f), q1(bxr, cy + bh * 0.5f);
            dl->AddRectFilled(q0, q1, IM_COL32(28, 18, 10, 228), bh * 0.3f);
            dl->AddRect(q0, q1, hov ? IM_COL32(230, 185, 100, 240) : IM_COL32(170, 125, 65, 200), bh * 0.3f, 0, fmaxf(1.0f, 0.3f * k));
            dl->AddText(fs, ms, ImVec2(q0.x + 0.3f * ms, cy - mt.y * 0.52f), fg, main);
            if (sub[0]) dl->AddText(fs, ss, ImVec2(q0.x + 0.3f * ms + mt.x + 0.3f * ms, cy - st.y * 0.5f), IM_COL32(215, 195, 160, 240), sub);
        }
        /* the list under it */
        if (p.is_opt && c.popup_piece == j) {
            float ey = y1 + 0.15f * L;
            float ex = X0 + pad + 0.8f * size;
            if (!c.nentries) {
                const char *none = p.type == 2 ? "Nobody in the party knows a saint who can help here." : "Nobody in the party carries anything that helps here.";
                dl->AddText(fs, size * 0.86f, ImVec2(ex, ey), ink_dim, none);
            }
            float namew = 0;
            for (int e = 0; e < c.nentries; e++) if (c.entry[e].who[0]) namew = fmaxf(namew, fs->CalcTextSizeA(size * 0.82f, FLT_MAX, 0, gtext(c.entry[e].who).c_str()).x);
            for (int e = 0; e < c.nentries; e++) {
                const CvEntry &en = c.entry[e];
                float h = L * 0.92f;
                bool eh = c.hover_entry == e && en.on;
                if (eh) dl->AddRectFilled(ImVec2(ex - 0.4f * size, ey - 0.1f * h), ImVec2(X1 + 1.0f * k, ey + 0.95f * h), dark ? IM_COL32(255, 214, 140, 50) : IM_COL32(150, 92, 30, 55), 1.2f * k);
                if (en.who[0]) dl->AddText(fs, size * 0.82f, ImVec2(ex, ey + 0.08f * h), ink_soft, gtext(en.who).c_str());
                ImU32 ec = !en.on || en.code == 0x83 ? ink_dim : en.code == 0x87 ? (dark ? IM_COL32(250, 210, 110, 255) : IM_COL32(150, 90, 0, 255)) : ink;
                std::string et = gtext(en.text);
                dl->AddText(f, size * 0.95f, ImVec2(ex + namew + 0.9f * size, ey), ec, et.c_str());
                c.entry_y0[e] = wy2g(ey - 0.1f * h); c.entry_y1[e] = wy2g(ey + 0.95f * h);
                if (c.hover_entry == e) { tip_at = ImVec2(X1 + 2 * k, ey); if (c.popup_kind == 2) tip_saint = en.id; else tip_item = en.id; }
                ey += h;
            }
        }
    }
    /* go on: a quiet sign at the end of a text that waits */
    if (!menu) {
        float t = (plat_ticks_ms() % 1600) / 1600.0f, a = 0.45f + 0.35f * sinf(t * 6.2831853f);
        float r = size * 0.28f, ax = X1 - r, ay = yend - shift * 0 - 0.62f * L;
        ImU32 ac = dark ? IM_COL32(246, 220, 160, (int)(255 * a)) : IM_COL32(110, 50, 15, (int)(255 * a));
        dl->AddTriangleFilled(ImVec2(ax - r, ay - r * 0.55f), ImVec2(ax + r, ay - r * 0.55f), ImVec2(ax, ay + r * 0.75f), ac);
    }
    if (tip_item >= 0 || tip_saint >= 0) ui_card_tip(tip_at, ImVec2(0, 0), tip_item, tip_saint);
    else if (menu && assist_card_help && c.hover >= 0 && c.hover < c.npieces && c.popup_piece != c.hover
             && (c.piece[c.hover].type == 1 || c.piece[c.hover].type == 2)) {
        /* which saints or items this option takes, and who in the party knows or carries them
           (even when nobody does: the option is then greyed) */
        const CvPiece &p = c.piece[c.hover];
        int npar = (s16)g_rw(G_PARTY_SIZE);
        auto name_of = [](int m) { char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n); return game_text(n, strlen(n)); };
        auto alive = [](int m) { int st = g_rb((u16)(G_MEMBER(m) + G_M_STATUS)); return st != 0 && st != 3; };
        std::vector<std::pair<std::string, std::string>> rows;
        for (int j = 0; j < 4; j++) {
            int id = (s16)g_rw((u16)((p.type == 2 ? 0xee4b : 0xee43) + 2 * j));
            std::string what, who;
            if (p.type == 2) {
                if (id < 0 || id >= (int)gamedata.saints.size()) continue;
                what = gamedata.saints[id].name;
                for (int m = 0; m < npar && m < 5; m++)
                    if (alive(m) && (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (id >> 3))) & (0x80 >> (id & 7)))) { if (!who.empty()) who += ", "; who += name_of(m); }
            } else {
                if (id <= 0 || id >= (int)gamedata.items.size()) continue;
                what = game_text(gamedata.items[id].name.c_str(), gamedata.items[id].name.size());
                for (int m = 0; m < npar && m < 5; m++) {
                    if (!alive(m)) continue;
                    int n = 0, k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                    for (int i = 0; i < k && i < 64; i++) if ((rw(G_FAR2E38, G_ITEM(m, i)) & 0xfff) == id) n += rb(G_FAR2E38, (u16)(G_ITEM(m, i) + 4));
                    if (n) { if (!who.empty()) who += ", "; who += name_of(m) + (n > 1 ? " (" + std::to_string(n) + ")" : ""); }
                }
            }
            rows.push_back({ what, who.empty() ? (p.type == 2 ? "nobody knows this saint" : "nobody carries one") : who });
        }
        if (!rows.empty()) {
            ImGui::SetNextWindowPos(ImVec2(bx1 + 4.5f * k, laid[c.hover].y0 - shift));     /* beside the option (the pointer may be the game's only) */
            ImGui::BeginTooltip();
            ImGui::TextDisabled("%s", p.type == 2 ? "Saints who help here" : "Items that help here");
            for (auto &r : rows) { ImGui::TextUnformatted(r.first.c_str()); ImGui::SameLine(); ImGui::TextDisabled("  %s", r.second.c_str()); }
            if (p.state == 1) ImGui::TextDisabled("(click to choose who)");
            ImGui::EndTooltip();
        }
    }
}
