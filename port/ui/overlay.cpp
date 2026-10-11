// overlay.cpp - things the port draws over the running game when the Darklands+ menu is
// closed: the chances of success next to card options (mods/assist.c computes them).
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <ctype.h>
#include <strings.h>
#include <algorithm>
#include <vector>
#include "imgui.h"
#include "../mods/hdart.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/assist.h"
#include "../mods/hdtext.h"
u32 plat_ticks_ms(void);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}

extern ImFont *ui_font_small, *ui_font_text;
#define UI_GAME_FONTS 6
extern ImFont *ui_font_game[UI_GAME_FONTS];
extern int ui_game_font, ui_text_edge;

/* text over any picture: an outline (or shadow) in the opposite brightness keeps it readable */
/* edge: 0 none, 1 shadow, 2 outline. The edge takes the tone away from the picture behind
 * (bg, 0xRRGGBB; -1 unknown): light text on a darker picture gets a dark edge, dark text on
 * parchment a faint light one */
static void text_edged(ImDrawList *dl, ImFont *f, float size, ImVec2 p, ImU32 c, const char *t, int edge, int bg = -1)
{
    if (edge) {
        int r = (c >> IM_COL32_R_SHIFT) & 255, g = (c >> IM_COL32_G_SHIFT) & 255, b = (c >> IM_COL32_B_SHIFT) & 255, a = (c >> IM_COL32_A_SHIFT) & 255;
        int lum = r * 299 + g * 587 + b * 114;
        bool light = bg < 0 ? lum > 115000 : lum >= ((bg >> 16 & 255) * 299 + (bg >> 8 & 255) * 587 + (bg & 255) * 114);
        ImU32 e = light ? IM_COL32(0, 0, 0, (int)(a * 0.80f)) : IM_COL32(255, 244, 225, (int)(a * 0.45f));
        float th = size * 0.055f; if (th < 1.0f) th = 1.0f;
        if (edge == 1) dl->AddText(f, size, ImVec2(p.x + th, p.y + th), e, t);
        else {
            static const float D[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{0.7f,0.7f},{-0.7f,0.7f},{0.7f,-0.7f},{-0.7f,-0.7f} };
            for (int i = 0; i < 8; i++) dl->AddText(f, size, ImVec2(p.x + D[i][0] * th, p.y + D[i][1] * th), e, t);
        }
    }
    dl->AddText(f, size, p, c, t);
}

/* text colours with too little contrast to the picture behind them (grey on parchment) move
 * away from it in brightness, keeping their hue: to a WCAG-like ratio of 4.5, or 2.5 for text
 * the game draws faint on purpose (greyed-out choices), which so stays fainter than the rest */
int ui_text_contrast = 1;
static double rel_lum(int r, int g, int b)
{
    auto ch = [](int v) { double c = v / 255.0; return c <= 0.03928 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4); };
    return 0.2126 * ch(r) + 0.7152 * ch(g) + 0.0722 * ch(b);
}
static ImU32 contrast_fix(ImU32 c, int bg)
{
    if (!ui_text_contrast || bg < 0) return c;
    int r = (c >> IM_COL32_R_SHIFT) & 255, g = (c >> IM_COL32_G_SHIFT) & 255, b = (c >> IM_COL32_B_SHIFT) & 255;
    int br = bg >> 16 & 255, bgc = bg >> 8 & 255, bb = bg & 255;
    double lb = rel_lum(br, bgc, bb), lt = rel_lum(r, g, b);
    auto ratio = [&](double l) { return l > lb ? (l + 0.05) / (lb + 0.05) : (lb + 0.05) / (l + 0.05); };
    double r0 = ratio(lt), want = r0 < 2.0 ? 2.5 : 4.5;
    if (r0 >= want) return c;
    bool up = lt > lb || (lt == lb && lb < 0.18);           /* away from the background */
    int tr = up ? 255 : 0;
    for (int k = 1; k <= 20; k++) {
        double t = k / 20.0;
        int nr = (int)(r + (tr - r) * t), ng = (int)(g + (tr - g) * t), nb = (int)(b + (tr - b) * t);
        if (ratio(rel_lum(nr, ng, nb)) >= want || k == 20) return IM_COL32(nr, ng, nb, (c >> IM_COL32_A_SHIFT) & 255);
    }
    return c;
}

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

/* where the crisp text of a run ends on screen (as hdtext_draw lays it out); window x */
static std::string utf8(const char *s);
static float run_end_win(const HdRun &r, float *cy, float *lh)
{
    ImFont *font = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    float x0, y0, x1, y1;
    plat_game_to_win((float)r.x, (float)r.y, &x0, &y0);
    plat_game_to_win((float)(r.x + r.w), (float)(r.y + r.h), &x1, &y1);
    float bw = x1 - x0, bh = y1 - y0, size = bh * 1.18f;
    std::string t = utf8(r.text);
    ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
    if (ts.x > bw * 1.04f) { size *= bw * 1.04f / ts.x; ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str()); }
    *cy = (y0 + y1) * 0.5f; *lh = bh;
    return x0 + ts.x;
}

/* the chance of success of each option, right after its text, at the size of the text */
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
    ImFont *f = ui_font_text;
    for (int i = 0; i < n; i++) {
        if (!cc.line[i].show) continue;
        int y0 = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i));
        float ex = -1, cy = 0, lh = 0;
        int best = -1;
        for (int k = 0; k < hdtext_nruns; k++) {             /* the option's last line of crisp text */
            const HdRun &r = hdtext_runs[k];
            if (r.y < y0 || r.y >= y0 + h || r.x < x0 - 4 || r.x > x0 + w) continue;
            if (best < 0 || r.y > hdtext_runs[best].y || (r.y == hdtext_runs[best].y && r.x > hdtext_runs[best].x)) best = k;
        }
        if (best >= 0) ex = run_end_win(hdtext_runs[best], &cy, &lh);
        else {                                                  /* the game's own lettering */
            int top = y0, bot = y0 + h - 1;
            int tx = text_end(x0, y0, w, h, tc, &top, &bot);
            ImVec2 a = g2w((float)(tx < 0 ? x0 + w : tx + 1), (float)top), b = g2w((float)(x0 + w), (float)(bot + 1));
            ex = a.x; cy = (a.y + b.y) * 0.5f; lh = b.y - a.y;
        }
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
        float size = lh * 1.05f; if (size < 16) size = 16;
        float subsize = size * 0.78f;
        ImVec2 ms = f->CalcTextSizeA(size, FLT_MAX, 0, main), ss = sub[0] ? f->CalcTextSizeA(subsize, FLT_MAX, 0, sub) : ImVec2(0, 0);
        float padx = size * 0.3f, gap = size * 0.35f;
        float bw = ms.x + (sub[0] ? ss.x + padx : 0) + 2 * padx, bh = ms.y * 0.92f;
        ImVec2 right = g2w((float)(x0 + w + 6), 0);
        float bx = ex + gap;
        if (bx + bw > right.x) bx = right.x - bw;
        ImVec2 p0(bx, cy - bh / 2), p1(bx + bw, cy + bh / 2);
        ImU32 fg = mode == 1 ? IM_COL32(245, 228, 185, 255)
                 : pct >= 70 ? IM_COL32(150, 230, 120, 255) : pct >= 40 ? IM_COL32(250, 210, 95, 255) : IM_COL32(250, 125, 100, 255);
        dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, 225), bh * 0.3f);
        dl->AddRect(p0, p1, IM_COL32(170, 125, 65, 230), bh * 0.3f, 0, 1.5f);
        dl->AddText(f, size, ImVec2(p0.x + padx, cy - ms.y * 0.52f), fg, main);
        if (sub[0]) dl->AddText(f, subsize, ImVec2(p0.x + padx + ms.x + padx, cy - ss.y * 0.5f), IM_COL32(215, 195, 160, 240), sub);
    }
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

/* abbreviations of the character sheet spelled out where they fit (the sheet's narrow
 * columns made the game write "wPol" for Polearms): each has a full and a shorter form */
int ui_spell_out = 1;
struct Abbr { const char *ab, *full, *mid; };
static const Abbr ABBR[] = {
    { "wEdg", "Edged Weapons", "Edged Wpns" }, { "wImp", "Impact Weapons", "Impact Wpns" }, { "wFll", "Flail Weapons", "Flail Wpns" },
    { "wPol", "Polearm Weapons", "Polearm Wpns" }, { "wThr", "Thrown Weapons", "Thrown Wpns" }, { "wBow", "Bow Weapons", "Bow Wpns" },
    { "wMsD", "Missile Devices", "Missile Dev." }, { "Alch", "Alchemy", 0 }, { "Relg", "Religion", 0 }, { "Virt", "Virtue", 0 },
    { "SpkC", "Speak Common", "Spk Common" }, { "SpkL", "Speak Latin", "Spk Latin" }, { "R&W", "Read & Write", 0 },
    { "Heal", "Healing", 0 }, { "Artf", "Artifice", 0 }, { "Stlh", "Stealth", 0 }, { "StrW", "Streetwise", 0 },
    { "Ride", "Riding", 0 }, { "WdWs", "Woodwise", 0 }, { "No Msl Wpn", "No Missile Weapon", "No Missile Wpn" },
    { "No V:Armor", "No Vitals Armor", "No V:Armor" }, { "No L:Armor", "No Limb Armor", "No L:Armor" },
    { "End", "Endurance", 0 }, { "Str", "Strength", 0 }, { "Agl", "Agility", 0 }, { "Per", "Perception", 0 },
    { "Int", "Intelligence", "Intellect" }, { "Chr", "Charisma", 0 }, { "DF", "Divine Favor", "Div. Favor" },
};
/* the item names the sheet shortens (ShrSword, V:Brgdn), from DARKLAND.LST */
static const char *item_full(const std::string &t)
{
    static std::vector<std::pair<std::string, std::string>> m; static bool built;
    if (!built && gamedata.loaded) {
        built = true;
        for (auto &it : gamedata.items) if (!it.shortname.empty() && it.shortname != it.name) {
            bool dup = false; for (auto &e : m) if (e.first == it.shortname) { dup = true; break; }
            if (!dup) m.push_back({ it.shortname, it.name });
        }
    }
    for (auto &e : m) if (e.first == t) return e.second.c_str();
    return 0;
}
static void abbr_forms(const std::string &t, const char **full, const char **mid)
{
    *full = *mid = 0;
    for (const Abbr &a : ABBR) if (t == a.ab) { *full = a.full; *mid = a.mid; return; }
    *full = item_full(t);
}
/* the room right of a run, in game pixels: up to the next text on its line, or the edge of
 * the sheet's column */
static int run_room_end(int i)
{
    const HdRun &r = hdtext_runs[i];
    int end = 318;
    if (r.x >= 195 && r.x < 215 && r.y >= 80) end = 256;          /* character sheet: equipment column */
    if (r.x >= 230 && r.x < 245 && r.y < 75) end = 257;           /* attributes */
    if (r.x >= 280 && r.x < 290) end = 316;                       /* skills */
    for (int k = 0; k < hdtext_nruns; k++) {
        const HdRun &o = hdtext_runs[k];
        if (k == i || o.x <= r.x || o.y + o.h <= r.y || o.y >= r.y + r.h) continue;
        if (o.x - 3 < end) end = o.x - 3;
    }
    return end;
}
/* spelled-out names, decided per column (the runs that start at one x) so that a column keeps
 * one left edge and one size: the longest form of each name that fits at 72% or more, all
 * drawn at the smallest scale any of them needs */
struct Spelled { std::string text; float x, scale; };
static void spell_columns(std::vector<Spelled> &out, ImFont *font, float size)
{
    out.assign(hdtext_nruns, Spelled{ std::string(), 0, 1 });
    if (!ui_spell_out) return;
    std::vector<int> done(hdtext_nruns, 0);
    for (int i = 0; i < hdtext_nruns; i++) {
        if (done[i] || hdtext_runs[i].h != 6) continue;
        std::vector<int> col;
        for (int k = i; k < hdtext_nruns; k++)
            if (!done[k] && hdtext_runs[k].h == 6 && hdtext_runs[k].x == hdtext_runs[i].x) { done[k] = 1; col.push_back(k); }
        int any = 0;
        for (int k : col) { const char *f, *m; abbr_forms(utf8(hdtext_runs[k].text), &f, &m); if (f) any = 1; }
        if (!any) continue;
        int start = hdtext_runs[i].x, end = 320, floor_ = 0;
        for (int k : col) {
            const HdRun &r = hdtext_runs[k];
            for (int j = 0; j < hdtext_nruns; j++) {
                const HdRun &o = hdtext_runs[j];
                if (j == k || o.x + o.w > r.x || o.x + o.w + 14 < r.x || o.y + o.h <= r.y || o.y >= r.y + r.h) continue;
                if (o.x + o.w + 3 > floor_) floor_ = o.x + o.w + 3;
            }
            int e = run_room_end(k); if (e < end) end = e;
        }
        if (floor_ && floor_ < start) start = floor_;
        float sx, sy, ex, ey; plat_game_to_win((float)start, 0, &sx, &sy); plat_game_to_win((float)end, 0, &ex, &ey);
        /* one form for the whole column: full names if they all fit at 85%, else the shorter
           forms at 72%, else the game's abbreviations */
        float room = ex - sx, scale = 1;
        std::vector<std::string> pick(col.size());
        for (int level = 0; level < 3; level++) {
            float minfit = 1;
            for (size_t c = 0; c < col.size(); c++) {
                std::string t = utf8(hdtext_runs[col[c]].text);
                const char *f, *m; abbr_forms(t, &f, &m);
                pick[c] = level == 0 && f ? f : level == 1 && (m || f) ? (m ? m : f) : t;
                float w = font->CalcTextSizeA(size, FLT_MAX, 0, pick[c].c_str()).x;
                if (w > room && room / w < minfit) minfit = room / w;
            }
            scale = minfit;
            if (level == 2 || minfit >= (level == 0 ? 0.85f : 0.72f)) break;
        }
        for (size_t c = 0; c < col.size(); c++) out[col[c]] = Spelled{ pick[c], sx, scale };
    }
}

/* small pictures for the skills of the character sheet, in the gap between value and name */
int ui_skill_icons = 1;
void ui_equip_icon(ImDrawList *dl, ImVec2 c, float s, int type);
static int skill_of(const char *t)
{
    static const char *SK[19] = { "wEdg", "wImp", "wFll", "wPol", "wThr", "wBow", "wMsD", "Alch", "Relg", "Virt", "SpkC", "SpkL", "R&W",
                                  "Heal", "Artf", "Stlh", "StrW", "Ride", "WdWs" };
    for (int i = 0; i < 19; i++) if (!strcmp(t, SK[i])) return i;
    return -1;
}
static void skill_icon(ImDrawList *dl, ImVec2 c, float s, int k)
{
    static const int WT[7] = { 3, 14, 17, 19, 23, 28, 31 };     /* a weapon of each class (LST type) */
    dl->AddCircleFilled(c, s * 0.66f, IM_COL32(24, 14, 6, 150));  /* a dark disc: the picture reads on any background */
    if (k < 7) { ui_equip_icon(dl, c, s * 1.25f, WT[k]); return; }
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196), green = C(84, 150, 62), steel = C(190, 194, 204), wood = C(134, 88, 46);
    switch (k) {
    case 7:  /* alchemy: a flask */
        dl->AddCircleFilled(P(0, 4), 6.2f * u, C(120, 200, 150)); dl->AddRectFilled(P(-1.8f, -9), P(1.8f, -1), C(200, 220, 225));
        dl->AddCircle(P(0, 4), 6.2f * u, ink, 0, 1.0f * u); dl->AddRect(P(-1.8f, -9), P(1.8f, -1.5f), ink, 0, 0, 1.0f * u);
        dl->AddRectFilled(P(-2.6f, -10.5f), P(2.6f, -8.5f), wood); break;
    case 8:  /* religion: a cross */
        dl->AddRectFilled(P(-1.6f, -10), P(1.6f, 10), gold); dl->AddRectFilled(P(-6.5f, -5), P(6.5f, -1.8f), gold);
        dl->AddRect(P(-1.6f, -10), P(1.6f, 10), ink, 0, 0, 0.8f * u); dl->AddRect(P(-6.5f, -5), P(6.5f, -1.8f), ink, 0, 0, 0.8f * u); break;
    case 9:  /* virtue: a halo with rays */
        for (int i = 0; i < 8; i++) { float a = i * 0.785f; dl->AddLine(P(cosf(a) * 6.5f, sinf(a) * 6.5f), P(cosf(a) * 10, sinf(a) * 10), gold, 1.2f * u); }
        dl->AddCircle(P(0, 0), 5.0f * u, gold, 0, 2.2f * u); break;
    case 10: /* speak common: a speech bubble */
        dl->AddRectFilled(P(-9, -8), P(9, 4), parch, 4 * u); dl->AddTriangleFilled(P(-4, 3.5f), P(-7, 9.5f), P(1, 3.5f), parch);
        dl->AddRect(P(-9, -8), P(9, 4), ink, 4 * u, 0, 1.0f * u);
        for (int i = 0; i < 3; i++) dl->AddCircleFilled(P(-4.5f + i * 4.5f, -2), 1.2f * u, ink); break;
    case 11: /* speak Latin: a book */
        dl->AddRectFilled(P(-9, -7), P(9, 7), C(120, 40, 30)); dl->AddRectFilled(P(-8, -6), P(-0.6f, 6), parch); dl->AddRectFilled(P(0.6f, -6), P(8, 6), parch);
        for (int i = 0; i < 3; i++) { dl->AddLine(P(-6.5f, -3 + i * 3), P(-2, -3 + i * 3), ink, 0.7f * u); dl->AddLine(P(2, -3 + i * 3), P(6.5f, -3 + i * 3), ink, 0.7f * u); } break;
    case 12: /* read & write: a quill */
        dl->AddTriangleFilled(P(-7, 9), P(8, -10), P(2, -10), C(245, 245, 240)); dl->AddTriangleFilled(P(-7, 9), P(8, -10), P(8, -4), C(220, 220, 214));
        dl->AddLine(P(-8, 10), P(7, -9), ink, 1.0f * u); break;
    case 13: /* healing: a sprig of herbs */
        dl->AddLine(P(0, 10), P(0, -8), C(60, 110, 40), 1.4f * u);
        for (int i = 0; i < 3; i++) { float y = -5 + i * 5.0f;
            dl->AddTriangleFilled(P(0, y), P(-7, y - 3), P(-3, y + 2), green); dl->AddTriangleFilled(P(0, y + 1), P(7, y - 2), P(3, y + 3), green); }
        dl->AddCircleFilled(P(0, -9), 2.2f * u, green); break;
    case 14: /* artifice: a cog */
        for (int i = 0; i < 8; i++) { float a = i * 0.785f; dl->AddLine(P(cosf(a) * 5, sinf(a) * 5), P(cosf(a) * 9, sinf(a) * 9), steel, 3.0f * u); }
        dl->AddCircleFilled(P(0, 0), 6.5f * u, steel); dl->AddCircleFilled(P(0, 0), 2.6f * u, C(60, 50, 40)); dl->AddCircle(P(0, 0), 6.5f * u, ink, 0, 0.8f * u); break;
    case 15: /* stealth: a half-closed eye */
        dl->AddBezierQuadratic(P(-10, 0), P(0, -8), P(10, 0), parch, 1.6f * u); dl->AddBezierQuadratic(P(-10, 0), P(0, 6), P(10, 0), parch, 1.6f * u);
        dl->AddCircleFilled(P(0, 0.5f), 3.0f * u, C(30, 30, 40)); dl->AddLine(P(-10, -0.5f), P(10, -0.5f), C(30, 30, 40), 2.2f * u); break;
    case 16: /* streetwise: a purse */
        dl->AddCircleFilled(P(0, 3), 7.0f * u, C(150, 104, 56)); dl->AddTriangleFilled(P(-3.5f, -5), P(3.5f, -5), P(0, -1), C(150, 104, 56));
        dl->AddLine(P(-4, -4), P(4, -4), ink, 1.2f * u); dl->AddCircle(P(0, 3), 7.0f * u, ink, 0, 0.8f * u); dl->AddCircleFilled(P(0, 4), 1.8f * u, gold); break;
    case 17: /* riding: a horseshoe */
        dl->PathArcTo(P(0, -1), 7.0f * u, 3.1416f * 0.95f, 3.1416f * 2.05f, 16); dl->PathStroke(steel, 0, 3.4f * u);
        dl->AddLine(P(-7, -1), P(-6, 8), steel, 3.4f * u); dl->AddLine(P(7, -1), P(6, 8), steel, 3.4f * u);
        for (int i = 0; i < 3; i++) { dl->AddCircleFilled(P(-6.6f + i * 0.2f, 1 + i * 2.5f), 0.7f * u, ink); dl->AddCircleFilled(P(6.6f - i * 0.2f, 1 + i * 2.5f), 0.7f * u, ink); } break;
    case 18: /* woodwise: a fir */
        dl->AddRectFilled(P(-1.2f, 6), P(1.2f, 10), wood);
        dl->AddTriangleFilled(P(0, -11), P(-5, -3), P(5, -3), C(52, 120, 60)); dl->AddTriangleFilled(P(0, -6), P(-7, 3), P(7, 3), C(52, 120, 60));
        dl->AddTriangleFilled(P(0, -1), P(-8.5f, 7), P(8.5f, 7), C(52, 120, 60)); break;
    }
}

extern "C" void hdtext_draw(void)
{
    if (!hdtext_mode || !ui_font_text) return;
    ImFont *font = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    static std::vector<Spelled> sp;
    { float a, b0, c, d; plat_game_to_win(0, 0, &a, &b0); plat_game_to_win(0, 6, &c, &d); spell_columns(sp, font, (d - b0) * 1.18f); }
    float prev_end = 0; int prev = -1;
    for (int i = 0; i < hdtext_nruns; i++) {
        const HdRun &r = hdtext_runs[i];
        std::string t = utf8(r.text);
        if (t.empty()) continue;
        float x0, y0, x1, y1;
        plat_game_to_win((float)r.x, (float)r.y, &x0, &y0);
        plat_game_to_win((float)(r.x + r.w), (float)(r.y + r.h), &x1, &y1);
        /* a run that continues the one before in another colour (a menu's red hotkey letter,
           then the rest of the word) starts where that one's letters end */
        if (prev >= 0) {
            const HdRun &q = hdtext_runs[prev];
            int gap = r.x - (q.x + q.w);
            if (q.y == r.y && q.h == r.h && q.font == r.font && gap >= 0 && gap <= 3) {
                float kx = (x1 - x0) / (r.w ? r.w : 1), nx = prev_end + gap * kx;
                x1 += nx - x0; x0 = nx;
            }
        }
        float bw = x1 - x0, bh = y1 - y0;
        float size = bh * 1.18f;
        ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        bool num = t.size() <= 5 && strspn(t.c_str(), "0123456789/") == t.size();
        float allow = bw * 1.04f + (num ? bh * 0.5f : 0);            /* "11" is 5 pixels wide in the game's font */
        if (ts.x > allow) size *= allow / ts.x;
        else if (r.font == 2 && t.size() >= 4 && ts.x < bw * 0.9f) {     /* the wide menu face: keep its width */
            float k = bw * 0.95f / ts.x; if (k > 1.15f) k = 1.15f; size *= k;
        }
        if (!sp[i].text.empty()) { t = sp[i].text; size = bh * 1.18f * sp[i].scale; x0 = sp[i].x; }
        ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        if (r.center) x0 += (bw - ts.x) * 0.5f;
        ImU32 c = contrast_fix(IM_COL32((r.rgb >> 16) & 255, (r.rgb >> 8) & 255, r.rgb & 255, 255), (int)(r.bgrgb & 0xffffff));
        float y = y0 + (bh - ts.y) * 0.5f - bh * 0.06f;
        text_edged(dl, font, size, ImVec2(x0, y), c, t.c_str(), ui_text_edge, (int)(r.bgrgb & 0xffffff));
        if (ui_skill_icons) { int k = skill_of(r.text); if (k >= 0) skill_icon(dl, ImVec2(x0 - bh * 0.47f, y0 + bh * 0.5f), bh * 0.56f, k); }
        prev = i; prev_end = x0 + ts.x;
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

/* short notices ("Game saved"), bottom centre, for a few seconds */
static char notice[160]; static u32 notice_t;
static u32 notice_ms = 3000;
extern "C" void ui_notice_for(const char *msg, unsigned ms)
{
    snprintf(notice, sizeof notice, "%s", msg);
    notice_t = plat_ticks_ms(); notice_ms = ms < 1000 ? 1000 : ms;
    fprintf(stderr, "notice: %s\n", msg);
}
extern "C" void ui_notice(const char *msg)
{
    snprintf(notice, sizeof notice, "%s", msg);
    notice_t = plat_ticks_ms(); notice_ms = 3000;
    fprintf(stderr, "notice: %s\n", msg);
}
extern "C" void ui_draw_notice(void)
{
    if (!notice[0]) return;
    u32 age = plat_ticks_ms() - notice_t;
    if (age > notice_ms) { notice[0] = 0; return; }
    float a = age + 600 < notice_ms ? 1.0f : (notice_ms - age) / 600.0f;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(ui_font_text);
    ImVec2 ts = ImGui::CalcTextSize(notice);
    ImVec2 p((io.DisplaySize.x - ts.x) * 0.5f, io.DisplaySize.y - ts.y - 28);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(ImVec2(p.x - 14, p.y - 7), ImVec2(p.x + ts.x + 14, p.y + ts.y + 7), IM_COL32(28, 20, 12, (int)(225 * a)), 6);
    dl->AddRect(ImVec2(p.x - 14, p.y - 7), ImVec2(p.x + ts.x + 14, p.y + ts.y + 7), IM_COL32(190, 150, 90, (int)(255 * a)), 6, 0, 1.5f);
    dl->AddText(p, IM_COL32(250, 232, 190, (int)(255 * a)), notice);
    ImGui::PopFont();
}

/* the saints and items a card accepts (card_saints[4] DS:ee4b, card_items[4] DS:ee43, set by
 * the card script, -1 = none), with who in the party knows or carries them: what the
 * Darklands Companion showed during encounters. The game's own pop-up lists only the saints
 * someone knows. */
static float card_help_top;              /* top of the card helper panel this frame (0: none) */
static void draw_card_help(void)
{
    card_help_top = 0;
    if (!assist_card_help || !assist_in_card_menu() || g_rb(0xee41) || g_rb(0xee42)) return;
    int npar = (s16)g_rw(G_PARTY_SIZE);
    if (npar < 1 || npar > 5) return;
    std::vector<std::pair<std::string, std::string>> lines;
    auto name_of = [](int m) { char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n); return game_text(n, sizeof n); };
    for (int j = 0; j < 4; j++) {
        int s = (s16)g_rw((u16)(0xee4b + 2 * j));
        if (s < 0 || s >= (int)gamedata.saints.size()) continue;
        std::string who;
        for (int m = 0; m < npar; m++)
            if (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7))) { if (!who.empty()) who += ", "; who += name_of(m); }
        lines.push_back({ gamedata.saints[s].name, who.empty() ? "nobody knows" : who });
    }
    int nsaints = (int)lines.size();
    for (int j = 0; j < 4; j++) {
        int it = (s16)g_rw((u16)(0xee43 + 2 * j));
        if (it <= 0 || it >= (int)gamedata.items.size()) continue;
        std::string who;
        for (int m = 0; m < npar; m++) {
            int n = 0, k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            for (int i = 0; i < k && i < 64; i++) if ((rw(G_FAR2E38, G_ITEM(m, i)) & 0xfff) == it) n += rb(G_FAR2E38, (u16)(G_ITEM(m, i) + 4));
            if (n) { if (!who.empty()) who += ", "; who += name_of(m) + (n > 1 ? " (" + std::to_string(n) + ")" : ""); }
        }
        lines.push_back({ game_text(gamedata.items[it].name.c_str(), 64), who.empty() ? "nobody has one" : who });
    }
    if (lines.empty()) return;
    /* sized with the picture, like the card's own text; smaller while it would cover any of
     * the card's lines */
    ImFont *f = ui_font_text;
    ImVec2 tl = g2w(0, 0), br = g2w(317, 197);
    float base = (br.y - tl.y) / 197.0f * 7.6f; if (base < 15) base = 15;
    const char *hs = "Saints that work here", *hi = "Items that work here";
    int nrows = (int)lines.size() + (nsaints ? 1 : 0) + (nsaints < (int)lines.size() ? 1 : 0);
    float size = base, w = 0, h = 0, gapx = 0, padx = 0, pady = 0, lh = 0;
    ImVec2 p0, p1;
    for (int tries = 0; tries < 6; tries++, size *= 0.93f) {
        lh = size * 1.12f; gapx = size * 0.8f; padx = size * 0.55f; pady = size * 0.3f;
        w = f->CalcTextSizeA(size * 0.92f, FLT_MAX, 0, hs).x;
        for (auto &l : lines) { float a = f->CalcTextSizeA(size, FLT_MAX, 0, l.first.c_str()).x + gapx + f->CalcTextSizeA(size, FLT_MAX, 0, l.second.c_str()).x; if (a > w) w = a; }
        h = lh * nrows + 2 * pady;
        p0 = ImVec2(br.x - w - 2 * padx, br.y - h); p1 = br;
        bool hit = false;
        for (int k = 0; k < hdtext_nruns && !hit; k++) {
            ImVec2 a = g2w((float)hdtext_runs[k].x, (float)hdtext_runs[k].y), b = g2w((float)(hdtext_runs[k].x + hdtext_runs[k].w), (float)(hdtext_runs[k].y + hdtext_runs[k].h));
            hit = a.x < p1.x && b.x > p0.x && a.y < p1.y && b.y > p0.y;
        }
        if (!hit) break;
    }
    card_help_top = p0.y;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float rr = size * 0.35f;
    dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, 215), rr);
    dl->AddRect(p0, p1, IM_COL32(170, 125, 65, 220), rr, 0, 1.5f);
    float y = p0.y + pady, x = p0.x + padx;
    float namew = 0;
    for (auto &l : lines) { float a = f->CalcTextSizeA(size, FLT_MAX, 0, l.first.c_str()).x; if (a > namew) namew = a; }
    for (int i = 0; i < (int)lines.size(); i++) {
        if (i == 0 && nsaints) { dl->AddText(f, size * 0.92f, ImVec2(x, y + size * 0.06f), IM_COL32(225, 190, 125, 255), hs); y += lh; }
        if (i == nsaints) { dl->AddText(f, size * 0.92f, ImVec2(x, y + size * 0.06f), IM_COL32(225, 190, 125, 255), hi); y += lh; }
        bool none = lines[i].second == "nobody knows" || lines[i].second == "nobody has one";
        dl->AddText(f, size, ImVec2(x + size * 0.4f, y), IM_COL32(248, 236, 205, 255), lines[i].first.c_str());
        float nx = x + size * 0.4f + namew + gapx * 0.6f;
        if (nx + f->CalcTextSizeA(size, FLT_MAX, 0, lines[i].second.c_str()).x > p1.x - padx * 0.5f) nx = p1.x - padx * 0.5f - f->CalcTextSizeA(size, FLT_MAX, 0, lines[i].second.c_str()).x;
        dl->AddText(f, size, ImVec2(nx, y), none ? IM_COL32(175, 155, 130, 220) : IM_COL32(150, 230, 120, 255), lines[i].second.c_str());
        y += lh;
    }
}

/* a saint learned: its description for a while (the Companion showed the clue book's text
 * then). saints_known (far 2e38:082c, 20 bytes per member) is watched; a bit or two
 * appearing while no game is being loaded or made is a saint learned. */
#include "../runtime/dispatch.h"
static u8 sk_prev[5][20]; static int sk_have, learned_saint = -1, learned_by = -1; static u32 learned_t;
static void watch_saints(void)
{
    u8 cur[5][20];
    for (int m = 0; m < 5; m++) for (int i = 0; i < 20; i++) cur[m][i] = rb(G_FAR2E38, (u16)(0x082c + m * 20 + i));
    bool quiet = game_mode() != GM_OTHER && !ovl_loaded[37] && !ovl_loaded[38];
    if (sk_have && quiet) {
        int n = 0, ls = -1, lm = -1;
        for (int m = 0; m < 5; m++) for (int i = 0; i < 20; i++) {
            u8 add = (u8)(cur[m][i] & ~sk_prev[m][i]);
            for (int b = 0; b < 8; b++) if (add & (0x80 >> b)) { n++; ls = i * 8 + b; lm = m; }
        }
        if (n >= 1 && n <= 2 && ls < (int)gamedata.saints.size()) { learned_saint = ls; learned_by = lm; learned_t = plat_ticks_ms(); }
    }
    memcpy(sk_prev, cur, sizeof cur); sk_have = 1;
}
static void draw_learned(void)
{
    if (learned_saint < 0) return;
    u32 age = plat_ticks_ms() - learned_t;
    if (age > 25000) { learned_saint = -1; return; }
    float a = age < 24000 ? 1.0f : (25000 - age) / 1000.0f;
    const Saint &t = gamedata.saints[learned_saint];
    char who[16] = ""; if (learned_by >= 0) g_str(G_DGROUP, (u16)(G_MEMBER(learned_by) + G_M_SHORTNAME), who, sizeof who);
    std::string head = (who[0] ? game_text(who, sizeof who) + " learned of " : std::string("Learned of ")) + t.name;
    ImFont *f = ui_font_text;
    ImVec2 tl = g2w(0, 0), br = g2w(317, 197);
    float size = (br.y - tl.y) / 197.0f * 6.8f; if (size < 15) size = 15;
    float w = (br.x - tl.x) * 0.46f, pad = size * 0.5f;
    ImVec2 hs = f->CalcTextSizeA(size * 1.05f, FLT_MAX, 0, head.c_str()), ts = f->CalcTextSizeA(size, FLT_MAX, w, t.text.c_str());
    float bh = hs.y + ts.y + pad * 2.4f;
    float bottom = card_help_top > 0 ? card_help_top - size * 0.4f : br.y;
    ImVec2 p0(br.x - w - 2 * pad, bottom - bh), p1(br.x, bottom);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, (int)(228 * a)), size * 0.35f);
    dl->AddRect(p0, p1, IM_COL32(170, 130, 70, (int)(230 * a)), size * 0.35f, 0, 1.5f);
    dl->AddText(f, size * 1.05f, ImVec2(p0.x + pad, p0.y + pad * 0.8f), IM_COL32(235, 200, 130, (int)(255 * a)), head.c_str());
    dl->AddText(f, size, ImVec2(p0.x + pad, p0.y + pad * 1.4f + hs.y), IM_COL32(240, 230, 210, (int)(255 * a)), t.text.c_str(), 0, w);
}

extern "C" void travel_overlay(void);
/* in a shop: what the item under the mouse is good for (its row in the merchant's or the
 * party's list: "17pf Torch 21q 2lbs", "59pf Short Sword ( 1) 25q") */
int ui_shop_info = 1;
void ui_item_details(const ItemDef *d, int id);
static void draw_shop_info(void)
{
    if (!ui_shop_info || game_mode() != GM_SCREEN || !hdtext_mode || !gamedata.loaded || config_get_int("original", 0)) return;
    int mx, my; plat_mouse_game(&mx, &my);
    { int cx, cy, cw, ch; if (hdtext_game_cursor(&cx, &cy, &cw, &ch)) { mx = cx; my = cy; } }   /* where the game shows its pointer (it moves it in lists) */
    int row = -1;
    for (int i = 0; i < hdtext_nruns && row < 0; i++) {
        const HdRun &r = hdtext_runs[i];
        if (my >= r.y - 1 && my < r.y + r.h + 2 && mx >= r.x - 2 && mx < r.x + r.w + 2) row = r.y;
    }
    if (row < 0) return;
    std::vector<const HdRun *> on;                          /* the whole row, left to right */
    for (int i = 0; i < hdtext_nruns; i++) if (hdtext_runs[i].y == row) on.push_back(&hdtext_runs[i]);
    std::sort(on.begin(), on.end(), [](const HdRun *a, const HdRun *b) { return a->x < b->x; });
    std::string t; for (const HdRun *r : on) { if (!t.empty()) t += ' '; t += r->text; }
    std::vector<std::string> tok; { size_t i = 0; while (i < t.size()) { while (i < t.size() && t[i] == ' ') i++; size_t j = i; while (j < t.size() && t[j] != ' ') j++; if (j > i) tok.push_back(t.substr(i, j - i)); i = j; } }
    auto price = [](const std::string &w) { size_t k = 0; while (k < w.size() && isdigit((unsigned char)w[k])) k++; return k > 0 && k + 2 == w.size() && (w.substr(k) == "pf" || w.substr(k) == "gr" || w.substr(k) == "fl"); };
    auto stop = [](const std::string &w) { if (w[0] == '(') return true; size_t k = 0; while (k < w.size() && isdigit((unsigned char)w[k])) k++; return k > 0 && (k == w.size() || (k + 1 == w.size() && (w[k] == 'q' || w[k] == ')'))); };
    size_t k = 0; while (k < tok.size() && !price(tok[k])) k++;
    if (k == tok.size()) return;
    std::string name;
    for (k++; k < tok.size() && !stop(tok[k]); k++) { if (!name.empty()) name += ' '; name += tok[k]; }
    if (name.empty()) return;
    int id = -1;
    for (size_t i = 0; i < gamedata.items.size(); i++) if (!strcasecmp(gamedata.items[i].name.c_str(), name.c_str())) { id = (int)i; break; }
    if (id < 0) return;
    const ItemDef *d = &gamedata.items[id];
    bool useful = (d->type >= 0 && d->type <= 33) || (d->type >= 67 && d->type <= 84) || (d->type >= 95 && d->type <= 97) || d->kind == K_POTION;
    if (!useful) return;
    ImVec2 p = g2w((float)mx, (float)my);
    ImGuiIO &io = ImGui::GetIO();
    bool right = p.x > io.DisplaySize.x * 0.55f, below = p.y > io.DisplaySize.y * 0.5f;     /* away from the mouse, towards the room */
    ImGui::SetNextWindowPos(ImVec2(p.x + (right ? -24.0f : 24.0f), p.y + (below ? -24.0f : 24.0f)), ImGuiCond_Always, ImVec2(right ? 1.0f : 0.0f, below ? 1.0f : 0.0f));
    ImGui::SetNextWindowBgAlpha(0.96f);
    if (ImGui::Begin("##shopinfo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::SetWindowFontScale(fminf(1.0f, fmaxf(0.72f, io.DisplaySize.y / 1000.0f)));
        ImGui::TextUnformatted(game_text(d->name.c_str(), d->name.size()).c_str());
        ui_item_details(d, id);
    }
    ImGui::End();
}

extern "C" void ui_overlay(void)
{
    draw_chances();
    draw_card_help();
    if (assist_card_help) { watch_saints(); draw_learned(); }
    travel_overlay();
    draw_picture_name();
    { void ui_margins(void); ui_margins(); }
    draw_shop_info();
}
