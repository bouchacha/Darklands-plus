// overlay.cpp - things the port draws over the running game when the Darklands+ menu is
// closed: the chances of success next to card options (mods/assist.c computes them).
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <string>
#include "imgui.h"
#include "../mods/hdart.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/assist.h"
#include "../mods/hdtext.h"
u32 plat_ticks_ms(void);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}

extern ImFont *ui_font_small, *ui_font_text;

static ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }
static u8 px(int x, int y) { return (x < 0 || x >= 320 || y < 0 || y >= 200) ? 0 : M[0xa0000 + y * 320 + x]; }

/* rightmost text pixel in the last text row of an option line: returns its x, and the
 * row's top/bottom (game pixels); -1 if no text pixel was found */
static int text_end(int x0, int y0, int w, int h, int tc, int *top, int *bot)
{
    int last = -1;
    for (int y = y0 + h - 1; y >= y0 && last < 0; y--)
        for (int x = x0; x < x0 + w; x++) { u8 c = px(x, y); if (c == tc || c == 9) { last = y; break; } }
    if (last < 0) return -1;
    *bot = last; *top = last - 7 < y0 ? y0 : last - 7;
    int right = -1;
    for (int y = *top; y <= *bot; y++)
        for (int x = x0 + w - 1; x > right; x--) { u8 c = px(x, y); if (c == tc || c == 9) { right = x; break; } }
    return right;
}

static void draw_chances(void)
{
    const CardChances &cc = card_chances;
    if (!assist_chances || !cc.active || plat_ticks_ms() - cc.polled > 300) return;
    if (g_rb(0xee41) || g_rb(0xee42)) return;                   /* a skill-check box is up */
    int n = (s8)g_rb(0xe96e);
    if (n != cc.n) return;
    int diff = g_rb(G_DIFFICULTY) & 0x7f;
    int mode = assist_chances == 2 ? 0 : diff;                  /* 0 numbers, 1 words, 2 hidden */
    if (mode >= 2) return;
    int x0 = 60 + (s16)g_rw(0xe972), w = (s16)g_rw(0xe970), tc = (s16)g_rw(0xee59);
    int members = (s16)g_rw(G_PARTY_SIZE);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImGui::PushFont(ui_font_small);
    for (int i = 0; i < n; i++) {
        if (!cc.line[i].show) continue;
        int y0 = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i));
        int top = y0, bot = y0 + h - 1;
        int tx = text_end(x0, y0, w, h, tc, &top, &bot);
        int pct = cc.line[i].pct, who = cc.line[i].who;
        char main[40], sub[24] = "";
        if (mode == 0) snprintf(main, sizeof main, "%d%%", pct);
        else {
            int q = pct / 20; if (q > 6) q = 6;
            u16 p = g_rw((u16)(0x4fce + 2 * q));
            if (pct <= 0) snprintf(main, sizeof main, "hopeless");
            else g_str(G_DGROUP, p, main, sizeof main);
        }
        if (who >= 0 && who < 5 && members > 1) g_str(G_DGROUP, (u16)(G_MEMBER(who) + G_M_SHORTNAME), sub, sizeof sub);
        ImVec2 ms = ImGui::CalcTextSize(main), ss = sub[0] ? ImGui::CalcTextSize(sub) : ImVec2(0, 0);
        float padx = 6, bw = ms.x + (sub[0] ? ss.x + 6 : 0) + 2 * padx, bh = ms.y + 2;
        ImVec2 a = g2w((float)(tx < 0 ? x0 + w : tx + 4), (float)top), b = g2w((float)(x0 + w), (float)(bot + 1));
        float cy = (a.y + b.y) / 2;
        float bx = tx < 0 ? b.x - bw : a.x;
        if (bx + bw > b.x) bx = b.x - bw;
        ImVec2 p0(bx, cy - bh / 2), p1(bx + bw, cy + bh / 2);
        ImU32 fg = mode == 1 ? IM_COL32(240, 220, 170, 255)
                 : pct >= 70 ? IM_COL32(150, 225, 120, 255) : pct >= 40 ? IM_COL32(245, 205, 90, 255) : IM_COL32(245, 120, 95, 255);
        dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, 215), 5);
        dl->AddRect(p0, p1, IM_COL32(150, 110, 60, 200), 5);
        dl->AddText(ImVec2(p0.x + padx, p0.y + 1), fg, main);
        if (sub[0]) dl->AddText(ImVec2(p0.x + padx + ms.x + 6, p0.y + 1), IM_COL32(200, 180, 150, 230), sub);
    }
    ImGui::PopFont();
}

/* crisp text: the runs hdtext.c verified, drawn with a real font into the picture's place */
static std::string utf8(const char *s)
{
    std::string o;
    for (; *s; s++) {
        switch ((unsigned char)*s) {
        case 0x1f: o += "\xc3\xa4"; break; case '[': o += "\xc3\x84"; break; case '\\': o += "\xc3\x96"; break;
        case ']': o += "\xc3\x9c"; break; case '_': o += "\xc3\x9f"; break; case '{': o += "\xc3\xb6"; break;
        case '|': o += "\xc3\xbc"; break; case '}': o += "\xc3\xa4"; break;
        default: if ((unsigned char)*s >= 32 && (unsigned char)*s < 127) o += *s;
        }
    }
    return o;
}

extern "C" void hdtext_draw(void)
{
    if (!hdtext_mode || !ui_font_text) return;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    for (int i = 0; i < hdtext_nruns; i++) {
        const HdRun &r = hdtext_runs[i];
        std::string t = utf8(r.text);
        if (t.empty()) continue;
        float x0, y0, x1, y1;
        plat_game_to_win((float)r.x, (float)r.y, &x0, &y0);
        plat_game_to_win((float)(r.x + r.w), (float)(r.y + r.h), &x1, &y1);
        float bw = x1 - x0, bh = y1 - y0;
        float size = bh * 1.18f;
        ImVec2 ts = ui_font_text->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        if (ts.x > bw * 1.04f) size *= bw * 1.04f / ts.x;
        ts = ui_font_text->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        ImU32 c = IM_COL32((r.rgb >> 16) & 255, (r.rgb >> 8) & 255, r.rgb & 255, 255);
        float y = y0 + (bh - ts.y) * 0.5f - bh * 0.06f;
        dl->AddText(ui_font_text, size, ImVec2(x0, y), c, t.c_str());
    }
}

/* "show picture names" (Display & sound): the picture under the mouse, outlined and named,
 * so that artists know which file to replace */
extern "C" void plat_mouse_game(int *x, int *y);
static void draw_picture_name(void)
{
    if (!hdart_show_names) return;
    int mx, my, x, y, w, h;
    plat_mouse_game(&mx, &my);
    int pic = hdart_pick(mx, my, &x, &y, &w, &h);
    if (pic < 0) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 a = g2w((float)x, (float)y), b = g2w((float)(x + w), (float)(y + h));
    dl->AddRect(a, b, IM_COL32(0, 0, 0, 200), 0, 0, 5);
    dl->AddRect(a, b, IM_COL32(255, 220, 80, 255), 0, 0, 2);
    char t[96]; snprintf(t, sizeof t, "%s  %dx%d%s", hdart_name(pic), w, h, hdart_has_art(pic) ? "  (replaced)" : "");
    ImVec2 ts = ImGui::CalcTextSize(t);
    ImVec2 m = g2w((float)mx, (float)my);
    ImVec2 p(m.x + 24, m.y + 24);
    ImGuiIO &io = ImGui::GetIO();
    if (p.x + ts.x + 12 > io.DisplaySize.x) p.x = io.DisplaySize.x - ts.x - 12;
    if (p.y + ts.y + 8 > io.DisplaySize.y) p.y = m.y - ts.y - 16;
    dl->AddRectFilled(ImVec2(p.x - 6, p.y - 3), ImVec2(p.x + ts.x + 6, p.y + ts.y + 3), IM_COL32(20, 16, 10, 230), 4);
    dl->AddText(p, IM_COL32(255, 230, 160, 255), t);
}

extern "C" void travel_overlay(void);
extern "C" void ui_overlay(void)
{
    draw_chances();
    travel_overlay();
    draw_picture_name();
}
