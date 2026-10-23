// overlay.cpp - things the port draws over the running game when the Darklands+ menu is
// closed: the chances of success next to card options (mods/assist.c computes them).
#include <stdio.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <ctype.h>
#include <strings.h>
#include <algorithm>
#include <vector>
#include "imgui.h"
#include <SDL.h>
#include "../mods/hdart.h"
#include "../mods/saintinfo.h"
#include "../mods/nscreen.h"
#include "gamedata.h"
#include "ui.h"
#include "skillicons.h"
#include "overlay.h"
extern "C" const char *gm_a_an(int n);   /* mods/gamemodel.c */
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "../mods/datamods.h"
#include "../mods/assist.h"
#include "../mods/cardview.h"
#include "../mods/hdtext.h"
#include "../mods/titleview.h"
u32 plat_ticks_ms(void);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}

extern ImFont *ui_font_small, *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
static ImDrawList *ovl_dl;               /* the overlay layer (ui_overlay): under tooltips and windows, over the picture */
static ImDrawList *ovl_list(void) { return ovl_dl ? ovl_dl : ImGui::GetForegroundDrawList(); }
ImDrawList *ui_overlay_list(void) { return ovl_list(); }
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

/* text the game draws faint on purpose (greyed-out menu entries): little contrast to what is behind */
static bool faint_text(ImU32 c, int bg)
{
    if (bg < 0) return false;
    double lt = rel_lum((c >> IM_COL32_R_SHIFT) & 255, (c >> IM_COL32_G_SHIFT) & 255, (c >> IM_COL32_B_SHIFT) & 255), lb = rel_lum(bg >> 16 & 255, bg >> 8 & 255, bg & 255);
    double r0 = lt > lb ? (lt + 0.05) / (lb + 0.05) : (lb + 0.05) / (lt + 0.05);
    return r0 < 2.0;
}

static ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }
ImVec2 ui_g2w(float x, float y) { return g2w(x, y); }
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

/* the typefaces' letters differ in size at one font size (MedievalSharp's x-height is half as
 * big again as EB Garamond's): each is drawn at the size that gives it EB Garamond's cap and
 * x-height, to which the layout was fitted, and on the baseline EB Garamond would have */
struct FaceMetrics { float k, asc; };
static FaceMetrics face_raw(ImFont *f)
{
    FaceMetrics m = { 1, 0.77f };
    if (!f || f->FontSize <= 0) return m;
    const ImFontGlyph *H = f->FindGlyphNoFallback('H'), *x = f->FindGlyphNoFallback('x');
    float cap = H ? (H->Y1 - H->Y0) / f->FontSize : 0.5f, xh = x ? (x->Y1 - x->Y0) / f->FontSize : 0.32f;
    m.k = cap > 0 && xh > 0 ? cap * xh : 0.16f;            /* (the product; k below is relative to the reference) */
    m.asc = f->Ascent / f->FontSize;
    return m;
}
extern ImFont *ui_font_game[UI_GAME_FONTS];
static FaceMetrics face_metrics(ImFont *f)
{
    static ImFont *last; static FaceMetrics lm;
    if (f == last) return lm;
    FaceMetrics ref = face_raw(ui_font_game[0]), m = face_raw(f);
    m.k = ref.k > 0 && m.k > 0 ? sqrtf(ref.k / m.k) : 1;
    if (m.k < 0.6f) m.k = 0.6f; if (m.k > 1.15f) m.k = 1.15f;
    last = f; lm = m;
    return m;
}
extern "C" int battleend_is_message(const char *t);      /* mods/battleend.c */
float ui_face_k(ImFont *f) { return face_metrics(f).k; }
static float face_ref_asc(void) { static float a = -1; if (a < 0) a = face_raw(ui_font_game[0]).asc; return a; }

/* where the crisp text of a run ends on screen (as hdtext_draw lays it out); window x */
static std::string utf8(const char *s);
static float run_end_win(const HdRun &r, float *cy, float *lh)
{
    ImFont *font = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    float x0, y0, x1, y1;
    plat_game_to_win((float)r.x, (float)r.y, &x0, &y0);
    plat_game_to_win((float)(r.x + r.w), (float)(r.y + r.h), &x1, &y1);
    float bw = x1 - x0, bh = y1 - y0, size = bh * 1.18f * face_metrics(font).k;
    std::string t = utf8(r.text);
    ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
    if (ts.x > bw * 1.04f) { size *= bw * 1.04f / ts.x; ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str()); }
    *cy = (y0 + y1) * 0.5f; *lh = bh;
    return x0 + ts.x;
}

/* the chance of success of each option, right after its text, at the size of the text */
static void draw_chances(void)
{
    if (cardview_showing()) return;                             /* the native card screen shows them itself */
    const CardChances &cc = card_chances;
    if (!assist_chances || !cc.active || plat_ticks_ms() - cc.polled > 300) return;
    if (g_rb(GV_card_box_shown) || g_rb(GV_card_box_pointer)) return;                   /* a skill-check box is up */
    int n = (s8)g_rb(GV_card_layout);
    if (n != cc.n) return;
    int diff = g_rb(G_DIFFICULTY) & 0x7f;
    int mode = assist_chances == 2 ? 0 : diff;                  /* 0 numbers, 1 words, 2 hidden */
    if (mode >= 2) return;
    int x0 = 60 + (s16)g_rw(GV_card_layout_x), w = (s16)g_rw(GV_card_layout_w), tc = (s16)g_rw(0xee59);
    int members = (s16)g_rw(G_PARTY_SIZE);
    ImDrawList *dl = ovl_list();
    ImFont *f = ui_font_overlay;
    int tip_line = -1, tip_pct = 0, tip_who = -1; ImVec2 tip_at;
    for (int i = 0; i < n; i++) {
        if (!cc.line[i].show) continue;
        int y0 = g_rb((u16)(GV_card_layout_y + i)), h = g_rb((u16)(GV_card_layout_h + i));
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
            u16 p = g_rw((u16)(GV_quality_words + 2 * q));
            if (pct <= 0) snprintf(main, sizeof main, "hopeless");
            else g_str(G_DGROUP, p, main, sizeof main);
        }
        if (who >= 0 && who < 5 && members > 1) g_str(G_DGROUP, (u16)(G_MEMBER(who) + G_M_SHORTNAME), sub, sizeof sub);
        else if (who == -2 && members > 1) snprintf(sub, sizeof sub, "all");      /* the whole party is tested */
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
        ImVec2 mp = ImGui::GetIO().MousePos;
        if (mp.x >= p0.x && mp.x < p1.x && mp.y >= p0.y && mp.y < p1.y) tip_line = i, tip_pct = pct, tip_who = who, tip_at = p1;
    }
    if (tip_line >= 0) {                                          /* what the badge is */
        char t[200], nm[24] = "";
        if (tip_who >= 0 && tip_who < 5) g_str(G_DGROUP, (u16)(G_MEMBER(tip_who) + G_M_SHORTNAME), nm, sizeof nm);
        if (nm[0]) snprintf(t, sizeof t, "This option tests %s: %s %d%% chance of success.\nThe original game shows this number while you hold Shift over an option; it rises with the skills and attributes tested.", nm, gm_a_an(tip_pct), tip_pct);
        else snprintf(t, sizeof t, "This option tests the whole party: %s %d%% chance of success.\nThe original game shows this number while you hold Shift over an option; it rises with the skills and attributes tested.", gm_a_an(tip_pct), tip_pct);
        ImFont *tf = f;
        float ts = ui_text_px() * 0.8f, pad = ts * 0.45f;           /* sized like the other tooltips */
        float wrap = fminf(ImGui::GetIO().DisplaySize.x * 0.45f, ts * 26);
        ImVec2 sz = tf->CalcTextSizeA(ts, FLT_MAX, wrap, t);
        ImVec2 q0(tip_at.x - sz.x - 2 * pad, tip_at.y + pad), q1(tip_at.x, tip_at.y + sz.y + 3 * pad);
        if (q0.x < 4) { q1.x += 4 - q0.x; q0.x = 4; }
        ImDrawList *fl = ImGui::GetForegroundDrawList();
        fl->AddRectFilled(q0, q1, IM_COL32(28, 18, 10, 240), pad * 0.6f);
        fl->AddRect(q0, q1, IM_COL32(170, 125, 65, 230), pad * 0.6f);
        fl->AddText(tf, ts, ImVec2(q0.x + pad, q0.y + pad), IM_COL32(240, 225, 195, 255), t, 0, wrap);
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
static int skill_of(const char *t)
{
    static const char *SK[19] = { "wEdg", "wImp", "wFll", "wPol", "wThr", "wBow", "wMsD", "Alch", "Relg", "Virt", "SpkC", "SpkL", "R&W",
                                  "Heal", "Artf", "Stlh", "StrW", "Ride", "WdWs" };
    for (int i = 0; i < 19; i++) if (!strcmp(t, SK[i])) return i;
    return -1;
}
static void skill_icon(ImDrawList *dl, ImVec2 c, float s, int k) { ui_skill_icon(dl, c, s, k); }
/* the attributes on the character sheet and in character creation: a picture each */
static int attr_of(const char *t)
{
    static const char *AT[7] = { "End", "Str", "Agl", "Per", "Int", "Chr", "DF" };       /* (the game's words, spelled out when drawn) */
    for (int i = 0; i < 7; i++) if (!strcmp(t, AT[i])) return i;
    return -1;
}

/* mods' text.txt: words on the screen said differently ("Purchase an item => Kaufen"), for
 * translations and renamings of the menus the game writes from its program; and, for
 * translators, a list of every line shown (F12 > Settings > Mods: "Collect the texts shown") */
#include <unordered_map>
#include <unordered_set>
int ui_collect_texts;
static std::unordered_map<std::string, std::string> textmap;
static void load_textmap(void)
{
    static int gen = -1;
    if (gen == datamods_generation()) return;
    gen = datamods_generation(); textmap.clear();
    for (int i = 0; i < datamods_n && datamods_enabled(); i++) {
        if (!datamods[i].on) continue;
        std::string p = std::string(datamods[i].path) + "/text.txt";
        FILE *f = fopen(p.c_str(), "rb");
        if (!f) continue;
        char line[600];
        while (fgets(line, sizeof line, f)) {
            std::string l = line;
            while (!l.empty() && (l.back() == '\n' || l.back() == '\r')) l.pop_back();
            if (l.empty() || l[0] == '#') continue;
            size_t a = l.find(" => ");
            if (a == std::string::npos) continue;
            textmap[l.substr(0, a)] = l.substr(a + 4);
        }
        fclose(f);
    }
}
static void collect_text(const std::string &t)
{
    static std::unordered_set<std::string> seen; static FILE *f;
    if (t.size() < 2 || !seen.insert(t).second) return;
    if (!f) {
        char md[1100]; datamods_folder(md, sizeof md);
        std::string d = std::string(md) + "/_export";
#ifdef _WIN32
        _mkdir(md); _mkdir(d.c_str());
#else
        mkdir(md, 0755); mkdir(d.c_str(), 0755);
#endif
        f = fopen((d + "/texts_seen.txt").c_str(), "ab");
        if (!f) return;
        fprintf(f, "# lines the game showed; copy the ones to change into your mod's text.txt and write the new words after =>\n");
    }
    fprintf(f, "%s => %s\n", t.c_str(), t.c_str()); fflush(f);
}

extern "C" void hdtext_draw(void)
{
    if (!hdtext_mode || !ui_font_text) return;
    load_textmap();
    ImFont *font = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    {   /* a big window: the larger set, which stays sharp (the 34-pixel one turned soft above ~44) */
        extern ImFont *ui_font_game_hi[UI_GAME_FONTS];
        float a, b0, c, d; plat_game_to_win(0, 0, &a, &b0); plat_game_to_win(0, 6, &c, &d);
        if ((d - b0) * 1.18f > 44 && ui_font_game_hi[ui_game_font]) font = ui_font_game_hi[ui_game_font];
    }
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    static std::vector<Spelled> sp;
    const FaceMetrics fm = face_metrics(font);
    { float a, b0, c, d; plat_game_to_win(0, 0, &a, &b0); plat_game_to_win(0, 6, &c, &d); spell_columns(sp, font, (d - b0) * 1.18f * fm.k); }
    float prev_end = 0; int prev = -1, skip = -1, on_sheet = -1;
    auto joins = [](const HdRun &q, const HdRun &r) { int gap = r.x - (q.x + q.w); return q.y == r.y && q.h == r.h && q.font == r.font && gap >= 0 && gap <= 3; };
    for (int i = 0; i < hdtext_nruns; i++) {
        if (i == skip) continue;
        HdRun r = hdtext_runs[i];
        std::string t = utf8(r.text);
        if (t.empty()) continue;
        /* a hotkey letter and the rest of its word ("P" + "urchase an item") count as one line */
        bool joined = i + 1 < hdtext_nruns && joins(r, hdtext_runs[i + 1]) && (prev < 0 || !joins(hdtext_runs[prev], r) || prev != i - 1);
        std::string whole = joined ? t + utf8(hdtext_runs[i + 1].text) : t;
        if (ui_collect_texts && (prev < 0 || prev != i - 1 || !joins(hdtext_runs[prev], r))) collect_text(whole);
        bool mapped = false;
        if (!textmap.empty()) {
            auto it = textmap.find(whole);
            if (it != textmap.end()) {
                t = it->second; mapped = true;
                if (joined) { const HdRun &n2 = hdtext_runs[i + 1]; r.w = n2.x + n2.w - r.x; r.rgb = n2.rgb; skip = i + 1; }
            } else if (joined) { it = textmap.find(t); if (it != textmap.end()) { t = it->second; mapped = true; } }
        }
        float x0, y0, x1, y1;
        plat_game_to_win((float)r.x, (float)r.y, &x0, &y0);
        plat_game_to_win((float)(r.x + r.w), (float)(r.y + r.h), &x1, &y1);
        /* a run that continues the one before in another colour (a menu's red hotkey letter,
           then the rest of the word) starts where that one's letters end */
        if (prev >= 0) {
            const HdRun &q = hdtext_runs[prev];
            int gap = r.x - (q.x + q.w);
            if (q.y == r.y && q.h == r.h && q.font == r.font && gap >= 0 && gap <= 3) {
                float kx = (x1 - x0) / (r.w ? r.w : 1), nx = prev_end + (r.cont || gap <= 1 ? 0 : gap - 1) * kx;   /* the bitmap's letter gap is in the crisp face's advance */
                x1 += nx - x0; x0 = nx;
            }
        }
        float bw = x1 - x0, bh = y1 - y0;
        float size = bh * 1.18f * fm.k;
        ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        bool num = t.size() <= 5 && strspn(t.c_str(), "0123456789/") == t.size();
        float allow = bw * 1.04f + (num ? bh * 0.5f : 0);            /* "11" is 5 pixels wide in the game's font */
        if (ts.x > allow) size *= allow / ts.x;
        else if (r.font == 2 && t.size() >= 4 && ts.x < bw * 0.9f) {     /* the wide menu face: keep its width */
            float k = bw * 0.95f / ts.x; if (k > 1.15f) k = 1.15f; size *= k;
        }
        if (!sp[i].text.empty() && !mapped) { t = sp[i].text; size = bh * 1.18f * fm.k * sp[i].scale; x0 = sp[i].x; }
        ts = font->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        if (r.center || battleend_is_message(r.text)) x0 += (bw - ts.x) * 0.5f;   /* (the end of a battle's words: centred in their box) */
        ImU32 c0 = IM_COL32((r.rgb >> 16) & 255, (r.rgb >> 8) & 255, r.rgb & 255, 255);
        bool faint = r.flatbg && faint_text(c0, (int)(r.bgrgb & 0xffffff));   /* greyed out in a menu: no outline, which would make it stand out */
        ImU32 c = contrast_fix(c0, (int)(r.bgrgb & 0xffffff));
        float seq = size / fm.k;                                     /* the size EB Garamond would have: its baseline */
        float y = y0 + (bh - seq) * 0.5f - bh * 0.06f + face_ref_asc() * seq - fm.asc * size;
        bool clipped = (r.clip0 > 0 || r.clip1 < r.h) && r.clip1 > r.clip0;   /* covered in part: only where it shows */
        if (clipped) {
            float cy0, cy1, dummy;
            plat_game_to_win(0, (float)(r.y + r.clip0), &dummy, &cy0); plat_game_to_win(0, (float)(r.y + r.clip1), &dummy, &cy1);
            dl->PushClipRect(ImVec2(0, cy0), ImVec2(ImGui::GetIO().DisplaySize.x, cy1), true);
        }
        text_edged(dl, font, size, ImVec2(x0, y), c, t.c_str(), faint ? 0 : ui_text_edge, (int)(r.bgrgb & 0xffffff));
        if (clipped) dl->PopClipRect();
        if (ui_skill_icons) {
            int k = skill_of(r.text);
            if (k >= 0) skill_icon(dl, ImVec2(x0 - bh * 0.47f, y0 + bh * 0.5f), bh * 0.56f, k);
            else if ((k = attr_of(r.text)) >= 0 && (on_sheet < 0 ? (on_sheet = hdart_on_screen("ARMBACK") || hdart_on_screen("CHARGEN")) : on_sheet))
                ui_attr_icon(dl, ImVec2(x0 - bh * 0.6f, y0 + bh * 0.5f), bh * 0.56f, k);
        }
        prev = i; prev_end = x0 + ts.x;
    }
}

/* the game's pointer again, over the crisp text and the helpers' marks (as over the picture),
 * where the mouse is now (hdtext.c: the game's own sprite is taken out of the picture) */
static int pointer_drawn;                                    /* this frame (hdtext_live_pointer for the next) */
static ImVec2 ptr_lo, ptr_hi; static int ptr_frame = -10;      /* where the pointer was drawn (window pixels), for tooltips beside it */
extern "C" void hdtext_draw_cursor(void)
{
    pointer_drawn = 0;
    if (config_get_int("original", 0)) return;              /* nothing of ours covers it then: the game draws its own */
    int x, y, w, h, st;
    const unsigned *px = hdtext_cursor(&x, &y, &w, &h, &st);
    if (!px) return;
    float fx = (float)x, fy = (float)y;
    static int scripted = -1;                                /* (scripted tests move the game's mouse, not the system's) */
    if (scripted < 0) scripted = (getenv("DL_SCRIPT") || getenv("DL_CTL")) && !getenv("DL_LIVE_POINTER");
    if (config_get_int("display.live_pointer", 1) && !scripted) {
        int mx, my; SDL_GetMouseState(&mx, &my);
        float ax, ay, bx, by; plat_game_to_win(0, 0, &ax, &ay); plat_game_to_win(320, 200, &bx, &by);
        pointer_drawn = 1;
        if (mx >= ax && mx < bx && my >= ay && my < by && bx > ax && by > ay) {   /* over the picture: at the mouse */
            int hx, hy; hdtext_pointer_hotspot(&hx, &hy);
            fx = (mx - ax) * 320.0f / (bx - ax) + hx; fy = (my - ay) * 200.0f / (by - ay) + hy;
        } else if (SDL_GetMouseFocus()) return;               /* in the margins the system's pointer shows instead (one pointer at a time) */
    }
    ptr_lo = g2w(fx, fy); ptr_hi = g2w(fx + w, fy + h); ptr_frame = ImGui::GetFrameCount();
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    for (int r = 0; r < h; r++) {
        if (fy + r < 0 || fy + r >= 200) continue;
        for (int c = 0; c < w; c++) {
            unsigned v = px[r * st + c];
            if (!v || fx + c < 0 || fx + c >= 320) continue;
            int e = c; while (e + 1 < w && fx + e + 1 < 320 && px[r * st + e + 1] == v) e++;
            dl->AddRectFilled(g2w(fx + c, fy + r), g2w(fx + e + 1, fy + r + 1), IM_COL32(v >> 16 & 255, v >> 8 & 255, v & 255, 255));
            c = e;
        }
    }
}
extern "C" void hdtext_pointer_off(void) { pointer_drawn = 0; }          /* not drawn this frame (a menu over the picture) */
extern "C" void hdtext_pointer_frame_end(void) { hdtext_live_pointer = pointer_drawn; }

/* "show picture names" (Display): the picture under the mouse, outlined and named,
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

/* the seal of a new encounter (mods/encounters.c): a little red wax seal in the corner of the card,
 * so the player can tell it from the game's own; its tooltip names it */
extern "C" int encounters_card_active(void);
extern "C" const char *encounters_title(int i);
extern "C" int encounters_builtin(int i);
static void draw_new_encounter_seal(void)
{
    int a = encounters_card_active();
    if (!a || game_mode() != GM_CARD || !config_get_int("encounters.seal", 1)) return;
    if (encounters_builtin(a - 1)) return;               /* (the game's own unused cards, put back: no mod's) */
    ImDrawList *dl = ovl_list();
    ImVec2 c = g2w(307.0f, 14.0f), e = g2w(313.0f, 14.0f);
    float r = e.x - c.x; if (r < 7) r = 7;
    const int N = 14;                                    /* the blob of wax: a wobbly rim */
    ImVec2 pts[N];
    for (int i = 0; i < N; i++) {
        float t = (float)i / N * 6.2831853f, rr = r * (1.0f + 0.09f * sinf(t * 3.0f + 0.7f) + 0.05f * sinf(t * 5.0f));
        pts[i] = ImVec2(c.x + cosf(t) * rr, c.y + sinf(t) * rr);
    }
    dl->AddConvexPolyFilled(pts, N, IM_COL32(118, 22, 18, 235));
    dl->AddPolyline(pts, N, IM_COL32(60, 8, 6, 220), ImDrawFlags_Closed, r * 0.12f);
    dl->AddCircle(c, r * 0.68f, IM_COL32(178, 62, 48, 230), 20, r * 0.11f);
    float k = r * 0.36f, w = r * 0.13f;                  /* a cross pattee in the middle */
    dl->AddRectFilled(ImVec2(c.x - w, c.y - k), ImVec2(c.x + w, c.y + k), IM_COL32(196, 84, 64, 240));
    dl->AddRectFilled(ImVec2(c.x - k, c.y - w), ImVec2(c.x + k, c.y + w), IM_COL32(196, 84, 64, 240));
    ImVec2 mp = ImGui::GetIO().MousePos;
    if ((mp.x - c.x) * (mp.x - c.x) + (mp.y - c.y) * (mp.y - c.y) < r * r * 1.4f) {
        char t[200]; snprintf(t, sizeof t, "A new encounter: %s\n(from a mod, not the original game)", encounters_title(a - 1));
        ImGui::SetTooltip("%s", t);
    }
}

/* the version at the bottom right, while no game is running (the opening, the title menu) */
extern "C" const char dlp_version[], dlp_release[];
static void draw_version(void)
{
    int m = game_mode();
    if (!(m == GM_OTHER || (m == GM_SCREEN && (s16)g_rw(G_PARTY_SIZE) == 0))) return;   /* (the title menu is a screen, before a party) */
    { extern TitleModel title_model; if (title_model.open && ns_screen == NS_TITLE && !ns_hidden()) return; }   /* (the start screen drawn anew writes it on its parchment) */
    char t[96]; snprintf(t, sizeof t, dlp_release[0] ? "Darklands+ %s (build %s)" : "Darklands+ build %s%s", dlp_release[0] ? dlp_release : dlp_version, dlp_release[0] ? dlp_version : "");
    ImDrawList *dl = ovl_list();
    ImVec2 br = g2w(318.0f, 198.0f), sz = ImGui::CalcTextSize(t);
    ImVec2 p(br.x - sz.x, br.y - sz.y);
    dl->AddText(ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 200), t);
    dl->AddText(p, IM_COL32(226, 210, 170, 230), t);
}

static void draw_lcd(void);
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
    draw_lcd();
    if (!notice[0]) return;
    u32 age = plat_ticks_ms() - notice_t;
    if (age > notice_ms) { notice[0] = 0; return; }
    float a = age + 600 < notice_ms ? 1.0f : (notice_ms - age) / 600.0f;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(ui_font_overlay);
    ImVec2 ts = ImGui::CalcTextSize(notice);
    ImVec2 p((io.DisplaySize.x - ts.x) * 0.5f, io.DisplaySize.y - ts.y - 28);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(ImVec2(p.x - 14, p.y - 7), ImVec2(p.x + ts.x + 14, p.y + ts.y + 7), IM_COL32(28, 20, 12, (int)(225 * a)), 6);
    dl->AddRect(ImVec2(p.x - 14, p.y - 7), ImVec2(p.x + ts.x + 14, p.y + ts.y + 7), IM_COL32(190, 150, 90, (int)(255 * a)), 6, 0, 1.5f);
    dl->AddText(p, IM_COL32(250, 232, 190, (int)(255 * a)), notice);
    ImGui::PopFont();
}

/* the MT-32's display (mt32.c): what the game writes there ("[ MPS Darklands ]") shown for a
 * few seconds in the top right corner on a little MT-32 front panel, so the player knows it plays */
static char lcd_next[24]; static volatile int lcd_new;
static char lcd[24]; static u32 lcd_t;
static char lcd_caption[120];                       /* a line under it, the first time */
extern "C" void ui_mt32_caption(const char *text) { snprintf(lcd_caption, sizeof lcd_caption, "%s", text); }
extern "C" void ui_mt32_lcd(const char *text)       /* from the synthesizer's thread */
{
    if (lcd_new) return;
    snprintf(lcd_next, sizeof lcd_next, "%-20.20s", text);
    lcd_new = 1;
}
extern ImFont *ui_font_bold;
static void draw_lcd(void)
{
    if (lcd_new) { memcpy(lcd, lcd_next, sizeof lcd); lcd_new = 0; lcd_t = plat_ticks_ms(); }
    if (!lcd[0] || !config_get_int("sound.mt32_lcd", 1)) return;
    u32 age = plat_ticks_ms() - lcd_t;
    const u32 SHOW = 6000;
    if (age > SHOW) { lcd[0] = 0; lcd_caption[0] = 0; return; }
    float a = age < 300 ? age / 300.0f : age + 800 > SHOW ? (SHOW - age) / 800.0f : 1.0f;
    /* a little MT-32 front panel: its dark grey face, "Roland" and the model, the MIDI MESSAGE and
       POWER lights, the part buttons, and the display (amber on black) with what the game wrote */
    ImGuiIO &io = ImGui::GetIO();
    ImFont *f = ImGui::GetIO().Fonts->Fonts[0];
    ImFont *fb = ui_font_bold ? ui_font_bold : f;
    float fs = fmaxf(14.0f, io.DisplaySize.y / 40.0f);
    float cw = fs * 0.62f;                                   /* a fixed pitch, as the display's cells */
    float lw = cw * 20 + fs, lh = fs * 1.7f;                 /* the display */
    float pad = fs * 0.55f, W = lw + 2 * pad, H = fs * 1.35f + lh + fs * 1.6f + 2 * pad;
    ImVec2 P0(io.DisplaySize.x - W - fs * 0.8f, fs * 0.8f), P1(P0.x + W, P0.y + H);
    auto A = [&](int al) { return (int)(al * a); };
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(ImVec2(P0.x + 3, P0.y + 4), ImVec2(P1.x + 3, P1.y + 4), IM_COL32(0, 0, 0, A(90)), 6);   /* its shadow */
    dl->AddRectFilledMultiColor(P0, P1, IM_COL32(66, 64, 62, A(250)), IM_COL32(66, 64, 62, A(250)), IM_COL32(40, 38, 37, A(250)), IM_COL32(40, 38, 37, A(250)));
    dl->AddRect(P0, P1, IM_COL32(110, 108, 104, A(255)), 6, 0, 1.5f);
    /* the top line: Roland, the model, MT-32 ACTIVE by a lit light */
    float ty = P0.y + pad * 0.7f, ls = fs * 0.95f;
    dl->AddText(fb, ls, ImVec2(P0.x + pad, ty), IM_COL32(236, 236, 232, A(255)), "Roland");
    const char *act = "MT-32 ACTIVE";
    ImVec2 as = fb->CalcTextSizeA(ls * 0.85f, FLT_MAX, 0, act);
    float ax = P1.x - pad - as.x;
    dl->AddText(fb, ls * 0.85f, ImVec2(ax, ty + ls * 0.08f), IM_COL32(236, 236, 232, A(255)), act);
    ImVec2 led(ax - fs * 0.6f, ty + ls * 0.5f);
    dl->AddCircleFilled(led, fs * 0.42f, IM_COL32(80, 255, 90, A(50)), 16);
    dl->AddCircleFilled(led, fs * 0.22f, IM_COL32(110, 255, 120, A(255)), 12);
    /* the display */
    ImVec2 p0(P0.x + pad, P0.y + pad + fs * 1.35f), p1(p0.x + lw, p0.y + lh);
    dl->AddRectFilled(ImVec2(p0.x - 3, p0.y - 3), ImVec2(p1.x + 3, p1.y + 3), IM_COL32(20, 20, 20, A(255)), 3);
    dl->AddRectFilled(p0, p1, IM_COL32(18, 8, 4, A(250)), 2);
    for (int i = 0; i < 20; i++) {
        char ch[2] = { lcd[i], 0 };
        float x = p0.x + fs * 0.5f + cw * i;
        dl->AddRectFilled(ImVec2(x + 1, p0.y + fs * 0.25f), ImVec2(x + cw - 1, p1.y - fs * 0.25f), IM_COL32(60, 22, 8, A(120)), 1);   /* the dim cells */
        if (ch[0] != ' ') {
            ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, ch);
            dl->AddText(f, fs, ImVec2(x + (cw - ts.x) * 0.5f, p0.y + (lh - ts.y) * 0.5f), IM_COL32(255, 150, 40, A(255)), ch);
        }
    }
    /* under it: MIDI MESSAGE (blinking now and then, as notes come), the part buttons, the volume knob */
    float by = p1.y + fs * 0.45f, bh = fs * 0.75f, small = fs * 0.55f;
    bool blink = ((age / 140) % 5) < 2;
    ImVec2 mled(p0.x + fs * 0.3f, by + bh * 0.5f);
    dl->AddCircleFilled(mled, fs * 0.2f, blink ? IM_COL32(255, 70, 50, A(255)) : IM_COL32(90, 30, 24, A(255)), 10);
    dl->AddText(f, small, ImVec2(mled.x + fs * 0.35f, by + (bh - small) * 0.5f), IM_COL32(200, 198, 192, A(255)), "MIDI MESSAGE");
    float bx = mled.x + fs * 0.35f + f->CalcTextSizeA(small, FLT_MAX, 0, "MIDI MESSAGE").x + fs * 0.6f;
    static const char *PARTS[6] = { "1", "2", "3", "4", "5", "R" };
    float bw = (p1.x - bx - fs * 1.6f) / 6.0f;
    for (int k = 0; k < 6; k++) {
        ImVec2 b0(bx + k * bw + 2, by), b1(bx + (k + 1) * bw - 2, by + bh);
        dl->AddRectFilled(b0, b1, IM_COL32(150, 148, 142, A(255)), 2);
        dl->AddLine(ImVec2(b0.x, b1.y), b1, IM_COL32(60, 58, 56, A(255)), 1.5f);
        ImVec2 ts = f->CalcTextSizeA(small, FLT_MAX, 0, PARTS[k]);
        dl->AddText(f, small, ImVec2((b0.x + b1.x - ts.x) * 0.5f, (b0.y + b1.y - ts.y) * 0.5f), IM_COL32(40, 38, 36, A(255)), PARTS[k]);
    }
    ImVec2 knob(p1.x - fs * 0.6f, by + bh * 0.5f);
    dl->AddCircleFilled(knob, fs * 0.5f, IM_COL32(28, 28, 28, A(255)), 20);
    dl->AddCircle(knob, fs * 0.5f, IM_COL32(120, 118, 114, A(255)), 20, 1.2f);
    dl->AddLine(knob, ImVec2(knob.x + fs * 0.32f, knob.y - fs * 0.32f), IM_COL32(230, 230, 226, A(255)), 1.5f);
    p1 = P1;
    if (lcd_caption[0]) {
        ImGui::PushFont(ui_font_text);
        ImVec2 ts = ImGui::CalcTextSize(lcd_caption);
        ImVec2 q(p1.x - ts.x, p1.y + fs * 0.6f);
        dl->AddRectFilled(ImVec2(q.x - 8, q.y - 4), ImVec2(p1.x + 4, q.y + ts.y + 4), IM_COL32(28, 20, 12, (int)(225 * a)), 4);
        dl->AddText(q, IM_COL32(250, 232, 190, (int)(255 * a)), lcd_caption);
        ImGui::PopFont();
    }
}

/* the saints and items a card accepts (card_saints[4] DS:ee4b, card_items[4] DS:ee43, set by
 * the card script, -1 = none), with who in the party knows or carries them: what the
 * Darklands Companion showed during encounters. The game's own pop-up lists only the saints
 * someone knows. */
static float card_help_top;              /* top of the card helper panel this frame (0: none) */
extern int ui_shop_info;
extern "C" void plat_mouse_game(int *x, int *y);
extern "C" int gm_knows_formula(int m, int f);
static void show_tip(ImVec2 pos, ImVec2 pivot, int item, int saint, int formula);
static void draw_card_help(void)
{
    card_help_top = 0;
    if (cardview_showing()) return;
    if (!assist_card_help || !assist_in_card_menu() || g_rb(GV_card_box_shown) || g_rb(GV_card_box_pointer)) return;
    int npar = (s16)g_rw(G_PARTY_SIZE);
    if (npar < 1 || npar > 5) return;
    {   /* only on a card with an option that takes a saint or an item (a box of names opens on it):
           the lists are the card script's, and a card without such an option leaves them as they
           were (four times St. Adrian, saint 0, on a town's main street) */
        int n = (s8)g_rb(GV_card_layout), any = 0;
        for (int i = 0; i < n && i < 20; i++) if ((s16)g_rw((u16)(GV_card_layout_kind + 2 * i)) > 0) any = 1;
        if (!any) return;
    }
    std::vector<std::pair<std::string, std::string>> lines;
    std::vector<int> ids;                                   /* a saint, or 1000 + an item */
    auto name_of = [](int m) { char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n); return game_text(n, sizeof n); };
    auto alive = [](int m) { int st = g_rb((u16)(G_MEMBER(m) + G_M_STATUS)); return st != 0 && st != 3; };   /* the dead know no saints and carry nothing */
    for (int j = 0; j < 4; j++) {
        int s = (s16)g_rw((u16)(GV_card_saints + 2 * j));
        if (s < 0 || s >= (int)gamedata.saints.size()) continue;
        std::string who;
        for (int m = 0; m < npar; m++)
            if (alive(m) && (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7)))) { if (!who.empty()) who += ", "; who += name_of(m); }
        lines.push_back({ gamedata.saints[s].name, who.empty() ? "nobody knows" : who });
        ids.push_back(s);
    }
    int nsaints = (int)lines.size();
    for (int j = 0; j < 4; j++) {
        int it = (s16)g_rw((u16)(GV_card_items + 2 * j));
        if (it <= 0 || it >= (int)gamedata.items.size()) continue;
        std::string who;
        for (int m = 0; m < npar; m++) {
            if (!alive(m)) continue;
            int n = 0, k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            for (int i = 0; i < k && i < 64; i++) if ((rw(G_FAR2E38, G_ITEM(m, i)) & 0xfff) == it) n += rb(G_FAR2E38, (u16)(G_ITEM(m, i) + 4));
            if (n) { if (!who.empty()) who += ", "; who += name_of(m) + (n > 1 ? " (" + std::to_string(n) + ")" : ""); }
        }
        lines.push_back({ game_text(gamedata.items[it].name.c_str(), 64), who.empty() ? "nobody has one" : who });
        ids.push_back(1000 + it);
    }
    if (lines.empty()) return;
    /* sized with the picture, like the card's own text; smaller while it would cover any of
     * the card's lines */
    ImFont *f = ui_font_overlay;
    ImVec2 tl = g2w(0, 0), br = g2w(317, 197);
    float base = (br.y - tl.y) / 197.0f * 7.6f; if (base < 15) base = 15;
    const char *hs = "Saints who help here", *hi = "Items that help here";
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
    ImDrawList *dl = ovl_list();
    float rr = size * 0.35f;
    dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, 215), rr);
    dl->AddRect(p0, p1, IM_COL32(170, 125, 65, 220), rr, 0, 1.5f);
    float y = p0.y + pady, x = p0.x + padx;
    float namew = 0;
    for (auto &l : lines) { float a = f->CalcTextSizeA(size, FLT_MAX, 0, l.first.c_str()).x; if (a > namew) namew = a; }
    int tip = -1, hmx, hmy; plat_mouse_game(&hmx, &hmy);
    ImVec2 hp = g2w((float)hmx, (float)hmy);
    for (int i = 0; i < (int)lines.size(); i++) {
        if (i == 0 && nsaints) { dl->AddText(f, size * 0.92f, ImVec2(x, y + size * 0.06f), IM_COL32(225, 190, 125, 255), hs); y += lh; }
        if (i == nsaints) { dl->AddText(f, size * 0.92f, ImVec2(x, y + size * 0.06f), IM_COL32(225, 190, 125, 255), hi); y += lh; }
        bool none = lines[i].second == "nobody knows" || lines[i].second == "nobody has one";
        dl->AddText(f, size, ImVec2(x + size * 0.4f, y), IM_COL32(248, 236, 205, 255), lines[i].first.c_str());
        float nx = x + size * 0.4f + namew + gapx * 0.6f;
        if (nx + f->CalcTextSizeA(size, FLT_MAX, 0, lines[i].second.c_str()).x > p1.x - padx * 0.5f) nx = p1.x - padx * 0.5f - f->CalcTextSizeA(size, FLT_MAX, 0, lines[i].second.c_str()).x;
        dl->AddText(f, size, ImVec2(nx, y), none ? IM_COL32(175, 155, 130, 220) : IM_COL32(150, 230, 120, 255), lines[i].second.c_str());
        if (hp.x >= p0.x && hp.x < p1.x && hp.y >= y && hp.y < y + lh) tip = i;
        y += lh;
    }
    if (tip >= 0 && ui_shop_info) {                          /* what the saint or the item under the pointer does, above the panel */
        int id = ids[tip];
        show_tip(ImVec2(p1.x, p0.y - size * 0.3f), ImVec2(1, 1), id >= 1000 ? id - 1000 : -1, id < 1000 ? id : -1, -1);
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
    if (age > 12500) { learned_saint = -1; return; }
    float a = age < 11500 ? 1.0f : (12500 - age) / 1000.0f;
    const Saint &t = gamedata.saints[learned_saint];
    char who[16] = ""; if (learned_by >= 0) g_str(G_DGROUP, (u16)(G_MEMBER(learned_by) + G_M_SHORTNAME), who, sizeof who);
    std::string head = (who[0] ? game_text(who, sizeof who) + " learned of " : std::string("Learned of ")) + t.name;
    ImFont *f = ui_font_overlay;
    ImVec2 tl = g2w(0, 0), br = g2w(317, 197);
    float size = (br.y - tl.y) / 197.0f * 6.8f; if (size < 15) size = 15;
    float w = (br.x - tl.x) * 0.46f, pad = size * 0.5f;
    /* the legend, then what the saint's help does (the game's code: mods/saintinfo.h) in gold */
    size_t at = saint_effect_at(t.text);
    std::string lore = t.text.substr(0, at), eff = std::string(saint_info_does(learned_saint)) + (*saint_info_note(learned_saint) ? " " : "") + saint_info_note(learned_saint);
    while (!lore.empty() && lore.back() == ' ') lore.pop_back();
    ImVec2 hs = f->CalcTextSizeA(size * 1.05f, FLT_MAX, 0, head.c_str());
    ImVec2 ls = lore.empty() ? ImVec2(0, 0) : f->CalcTextSizeA(size, FLT_MAX, w, lore.c_str());
    ImVec2 es = eff.empty() ? ImVec2(0, 0) : f->CalcTextSizeA(size, FLT_MAX, w, eff.c_str());
    float gap = !lore.empty() && !eff.empty() ? size * 0.4f : 0;
    float bh = hs.y + ls.y + gap + es.y + pad * 2.4f;
    float bottom = card_help_top > 0 ? card_help_top - size * 0.4f : br.y;
    ImVec2 p0(br.x - w - 2 * pad, bottom - bh), p1(br.x, bottom);
    ImDrawList *dl = ovl_list();
    dl->AddRectFilled(p0, p1, IM_COL32(28, 18, 10, (int)(228 * a)), size * 0.35f);
    dl->AddRect(p0, p1, IM_COL32(170, 130, 70, (int)(230 * a)), size * 0.35f, 0, 1.5f);
    dl->AddText(f, size * 1.05f, ImVec2(p0.x + pad, p0.y + pad * 0.8f), IM_COL32(250, 246, 236, (int)(255 * a)), head.c_str());
    float ty = p0.y + pad * 1.4f + hs.y;
    if (!lore.empty()) dl->AddText(f, size, ImVec2(p0.x + pad, ty), IM_COL32(215, 202, 180, (int)(255 * a)), lore.c_str(), 0, w);
    if (!eff.empty()) dl->AddText(f, size, ImVec2(p0.x + pad, ty + ls.y + gap), IM_COL32(255, 222, 140, (int)(255 * a)), eff.c_str(), 0, w);
}

extern "C" void travel_overlay(void);
/* what the pointer is on, in any list of the game's: an item (shops, the loot, the character
 * sheet, the equipment and trading screens: "17pf Torch 21q 2lbs", "Club ( 4) 5-Qual",
 * "V:Brigandine"), a saint (the saints lists, a card's prayer box) or an alchemy formula (the
 * formula book) - what it is good for, as a tooltip */
int ui_shop_info = 1;
void ui_item_details(const ItemDef *d, int id, int q = -1);
static void show_tip(ImVec2 pos, ImVec2 pivot, int item, int saint, int formula);
static std::string squash(const std::string &t)            /* letters and digits only, lower case: "St.Martin" = "St. Martin" */
{
    std::string o; for (unsigned char c : t) if (isalnum(c)) o += (char)tolower(c);
    return o;
}
extern "C" int ns_showing(void);
static void draw_shop_info(void)
{
    if (ns_showing()) return;   /* the native shop has its own strip */
    int gm = game_mode();
    static int tip_log = getenv("DL_TIP_LOG") != 0;
    static std::string last_log;
    auto tlog = [&](const std::string &m) { if (tip_log && m != last_log) { last_log = m; fprintf(stderr, "tip: %s\n", m.c_str()); } };
    if (!ui_shop_info || (gm != GM_SCREEN && gm != GM_CARD) || !hdtext_mode || !gamedata.loaded || config_get_int("original", 0)) { tlog("off, mode " + std::to_string(gm)); return; }
    static std::vector<std::string> item_keys, short_keys, saint_keys, formula_keys;
    if (item_keys.size() != gamedata.items.size()) {
        item_keys.clear(); short_keys.clear();
        for (const ItemDef &d : gamedata.items) { item_keys.push_back(squash(d.game_name)); short_keys.push_back(squash(d.shortname)); }   /* (the game's names: matched in the game's text) */
        saint_keys.clear(); for (const Saint &t : gamedata.saints) saint_keys.push_back(squash(t.game_name));
        formula_keys.clear(); for (const std::string &f : gamedata.formula_game_names) formula_keys.push_back(squash(f));
    }
    int mx, my; plat_mouse_game(&mx, &my);
    { int cx, cy, cw, ch; if (hdtext_game_cursor(&cx, &cy, &cw, &ch)) { mx = cx; my = cy; } }   /* where the game shows its pointer (it moves it in lists) */
    int hit = -1;
    for (int i = 0; i < hdtext_nruns && hit < 0; i++) {
        const HdRun &r = hdtext_runs[i];
        if (my >= r.y - 1 && my < r.y + r.h + 2 && mx >= r.x - 2 && mx < r.x + r.w + 2) hit = i;
    }
    if (hit < 0) { tlog("nothing at " + std::to_string(mx) + "," + std::to_string(my)); return; }
    /* the runs of that line next to each other (one column of the screen), left to right */
    int row = hdtext_runs[hit].y;
    std::vector<const HdRun *> on;
    for (int i = 0; i < hdtext_nruns; i++) if (hdtext_runs[i].y == row) on.push_back(&hdtext_runs[i]);
    std::sort(on.begin(), on.end(), [](const HdRun *a, const HdRun *b) { return a->x < b->x; });
    size_t at = 0; while (at < on.size() && on[at] != &hdtext_runs[hit]) at++;
    size_t a = at, b = at;
    while (a > 0 && on[a]->x - (on[a - 1]->x + on[a - 1]->w) <= 8) a--;
    while (b + 1 < on.size() && on[b + 1]->x - (on[b]->x + on[b]->w) <= 8) b++;
    std::string t; for (size_t k = a; k <= b; k++) { if (!t.empty()) t += ' '; t += on[k]->text; }
    std::vector<std::string> tok; { size_t i = 0; while (i < t.size()) { while (i < t.size() && t[i] == ' ') i++; size_t j = i; while (j < t.size() && t[j] != ' ') j++; if (j > i) tok.push_back(t.substr(i, j - i)); i = j; } }
    auto price = [](const std::string &w) { size_t k = 0; while (k < w.size() && isdigit((unsigned char)w[k])) k++; return k > 0 && k + 2 == w.size() && (w.substr(k) == "pf" || w.substr(k) == "gr" || w.substr(k) == "fl"); };
    auto stop = [](const std::string &w) { if (w[0] == '(') return true; size_t k = 0; while (k < w.size() && isdigit((unsigned char)w[k])) k++;
                                            return k > 0 && (k == w.size() || w[k] == 'q' || w[k] == ')' || w[k] == '-' || w.substr(k) == "lbs"); };
    size_t k = 0;
    while (k < tok.size() && price(tok[k])) k++;                     /* a shop's price first */
    std::string name;
    for (; k < tok.size() && !stop(tok[k]); k++) { if (!name.empty()) name += ' '; name += tok[k]; }
    std::string key = squash(name);
    if (key.size() < 3) return;
    int item = -1, saint = -1, formula = -1;
    for (size_t i = 0; i < item_keys.size() && item < 0; i++) if (key == item_keys[i] || (short_keys[i].size() >= 4 && key == short_keys[i])) item = (int)i;
    /* armour is "V:Brigandine", "L:Chainmail" (vital and limb) in the item list; a screen may leave the "V:" out */
    for (size_t i = 0; i < item_keys.size() && item < 0; i++) {
        const std::string &n = gamedata.items[i].game_name;
        if (n.size() > 2 && n[1] == ':' && key == item_keys[i].substr(1)) item = (int)i;
    }
    if (item < 0) for (size_t i = 0; i < saint_keys.size() && saint < 0; i++) if (key == saint_keys[i]) saint = (int)i;
    if (item < 0 && saint < 0) for (size_t i = 0; i < formula_keys.size() && formula < 0; i++) if (key == formula_keys[i]) formula = (int)i;
    tlog("'" + t + "' -> '" + key + "' item " + std::to_string(item) + " saint " + std::to_string(saint) + " formula " + std::to_string(formula));
    if (item < 0 && saint < 0 && formula < 0) return;
    ImVec2 p = g2w((float)mx, (float)my);
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 lo(p.x - 20, p.y - 20), hi(p.x + 20, p.y + 20);                      /* beside the pointer, not under it */
    if (ImGui::GetFrameCount() - ptr_frame <= 2) { lo = ptr_lo; hi = ptr_hi; }
    bool right = (lo.x + hi.x) / 2 > io.DisplaySize.x * 0.55f, below = (lo.y + hi.y) / 2 > io.DisplaySize.y * 0.5f;   /* towards the room */
    show_tip(ImVec2(right ? lo.x - 6 : hi.x + 6, below ? hi.y : lo.y), ImVec2(right ? 1.0f : 0.0f, below ? 1.0f : 0.0f), item, saint, formula);
}

/* the tooltip for an item, a saint or an alchemy formula, at pos (its corner given by pivot) */
static void show_tip(ImVec2 pos, ImVec2 pivot, int item, int saint, int formula)
{
    ImGuiIO &io = ImGui::GetIO();
    /* kept inside the window: placed by last frame's size (a long saint's text near the bottom of
       a card's list ran off the screen) */
    static ImVec2 last_size(0, 0);
    ImVec2 tl(pos.x - pivot.x * last_size.x, pos.y - pivot.y * last_size.y);
    if (tl.x + last_size.x > io.DisplaySize.x) tl.x = io.DisplaySize.x - last_size.x;
    if (tl.y + last_size.y > io.DisplaySize.y) tl.y = io.DisplaySize.y - last_size.y;
    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    ImGui::SetNextWindowPos(tl, ImGuiCond_Always, ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(fmaxf(UIS(380), fminf(io.DisplaySize.x * 0.32f, ui_text_px() * 26)), FLT_MAX));
    if (ImGui::Begin("##shopinfo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PushFont(ui_font_overlay);
        ImGui::SetWindowFontScale(ui_text_px() * 0.88f / ui_font_overlay->FontSize);
        ImGui::PushTextWrapPos(fmaxf(UIS(360), fminf(io.DisplaySize.x * 0.30f, ui_text_px() * 24)));
        if (item >= 0) {
            const ItemDef *d = &gamedata.items[item];
            std::string nm = game_text(d->name.c_str(), d->name.size());
            ImGui::TextUnformatted(nm.c_str());
            bool wearable = d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD;   /* (only equipped things count toward the load) */
            ImGui::TextDisabled("%s%s", kind_name_one(d->kind), wearable && d->weight ? (", weight " + std::to_string(d->weight) + " when equipped").c_str() : "");
            ui_item_details(d, item);
        } else if (saint >= 0) {
            const Saint &st = gamedata.saints[saint];
            ImGui::TextUnformatted(st.name.c_str());
            std::string who; int npar = (s16)g_rw(G_PARTY_SIZE);
            for (int m = 0; m < npar && m < 5; m++)
                if (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (saint >> 3))) & (0x80 >> (saint & 7))) {
                    char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
                    if (!who.empty()) who += ", "; who += game_text(n, sizeof n);
                }
            ImGui::TextDisabled("%s", who.empty() ? "Nobody in the party knows this saint" : ("Known to " + who).c_str());
            ImGui::Separator();
            ui_saint_info(saint);
        } else {
            ImGui::TextUnformatted(gamedata.formula_names[formula].c_str());
            if (formula < (int)gamedata.formulas.size()) {
                const Formula &F = gamedata.formulas[formula];
                const ItemDef *pd = 95 + formula / 3 < (int)gamedata.items.size() ? &gamedata.items[95 + formula / 3] : nullptr;
                static const char *RISK[3] = { "low risk", "medium risk", "high risk" };
                ImGui::TextDisabled("%s - difficulty %d, %s", pd ? game_text(pd->name.c_str(), pd->name.size()).c_str() : "", F.mystic, RISK[F.risk >= 0 && F.risk < 3 ? F.risk : 0]);
                /* its place among the potion's three formulae (by difficulty) */
                int base = formula / 3 * 3, rank = 0, n3 = 0;
                for (int k = base; k < base + 3 && k < (int)gamedata.formulas.size(); k++) {
                    if (k < (int)gamedata.formula_names.size() && gamedata.formula_names[k].empty()) continue;
                    n3++;
                    if (gamedata.formulas[k].mystic < F.mystic || (gamedata.formulas[k].mystic == F.mystic && k < formula)) rank++;
                }
                if (n3 == 3) {
                    static const char *ORD[3] = { "The easiest", "The middle", "The hardest" };
                    std::string others;
                    for (int k = base; k < base + 3; k++) if (k != formula && k < (int)gamedata.formula_names.size()) others += (others.empty() ? "" : " and ") + gamedata.formula_names[k];
                    ImGui::TextDisabled("%s of the three formulae for %s (the others: %s).", ORD[rank], pd ? game_text(pd->name.c_str(), pd->name.size()).c_str() : "it", others.c_str());
                }
                {
                    std::string who; int np = (s16)g_rw(G_PARTY_SIZE);
                    for (int m = 0; m < np && m < 5; m++) if (gm_knows_formula(m, formula)) {
                        char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
                        if (!who.empty()) who += ", "; who += game_text(n, strlen(n));
                    }
                    ImGui::TextDisabled("%s", who.empty() ? "Nobody in the party knows this formula." : ("Known to " + who + ".").c_str());
                }
                ImGui::Separator();
                ImGui::TextWrapped("%s", F.text.c_str());
                /* each ingredient with what the party carries of it (red: not enough) */
                int npar = (s16)g_rw(G_PARTY_SIZE), batches = 99;
                for (const FormulaIngr &g : F.ingr) {
                    int have = 0;
                    for (int m = 0; m < npar && m < 5; m++) {
                        int k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                        for (int i = 0; i < k && i < 64; i++) if ((rw(G_FAR2E38, G_ITEM(m, i)) & 0xfff) == g.item) have += rb(G_FAR2E38, (u16)(G_ITEM(m, i) + 4));
                    }
                    if (g.count > 0 && have / g.count < batches) batches = have / g.count;
                    std::string nm = g.item >= 0 && g.item < (int)gamedata.items.size() ? game_text(gamedata.items[g.item].name.c_str(), 64) : std::string("?");
                    ImGui::TextColored(have >= g.count ? ImVec4(0.2f, 0.12f, 0.06f, 1) : ImVec4(0.65f, 0.15f, 0.10f, 1), "%d %s", g.count, nm.c_str());
                    ImGui::SameLine(); ImGui::TextDisabled("(carried: %d)", have);
                }
                if (!F.ingr.empty()) ImGui::TextDisabled(batches > 0 ? "Enough for %d batch%s." : "Not enough ingredients for a batch.", batches, batches == 1 ? "" : "es");
            }
        }
        ImGui::PopTextWrapPos();
        ImGui::PopFont();
    }
    last_size = ImGui::GetWindowSize();
    ImGui::End();
}

int ui_creation_guide = 1;
/* the loot screen and the shops: two buttons beside the game's menu (loot: Take all (A),
 * Share out (S); shops: Sell the whole stack (A), Buy ten (T); mods/loot.c); clicks come
 * through ui_event (overlay_click) */
extern "C" { extern int loot_active, barter_active; int loot_enabled(void); void loot_press(int i); }
static ImVec2 loot_btn[2][2]; static int loot_btn_frame = -10, click_taken;
static void draw_loot_buttons(void)
{
    if (ns_showing()) return;
    if ((!loot_active && !barter_active) || !loot_enabled() || game_mode() != GM_SCREEN) return;
    if (hdart_on_screen("ARMBACK")) return;                  /* a character sheet opened from the shop or the loot */
    ImFont *f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    ImVec2 a = g2w(0, 0), b = g2w(0, 7);
    float size = (b.y - a.y) * 1.05f;
    static const char *L[2][2] = { { "A", "  Take all of it" }, { "S", "  Share it out among the party" } };
    static const char *B[2][2] = { { "A", "  Sell the whole stack" }, { "T", "  Buy ten" } };
    const char *(*T)[2] = loot_active ? L : B;
    ImDrawList *dl = ovl_list();
    ImVec2 mp = ImGui::GetIO().MousePos;
    for (int i = 0; i < 2; i++) {
        ImVec2 p0 = g2w(0xb6, (float)(0x1a + 8 * i)), p1 = g2w(0x12c, (float)(0x1a + 8 * i + 8));
        p0.y += 1; p1.y -= 1;
        bool hot = mp.x >= p0.x && mp.x < p1.x && mp.y >= p0.y && mp.y < p1.y;
        dl->AddRectFilled(p0, p1, hot ? IM_COL32(120, 70, 30, 70) : IM_COL32(90, 60, 30, 34), size * 0.25f);
        dl->AddRect(p0, p1, IM_COL32(110, 72, 36, hot ? 220 : 140), size * 0.25f, 0, 1.2f);
        float ty = (p0.y + p1.y) * 0.5f - size * 0.55f, tx = p0.x + size * 0.45f;
        dl->AddText(f, size, ImVec2(tx, ty), IM_COL32(200, 40, 30, 255), T[i][0]);
        dl->AddText(f, size, ImVec2(tx + f->CalcTextSizeA(size, FLT_MAX, 0, T[i][0]).x, ty), IM_COL32(60, 40, 25, 255), T[i][1]);
        loot_btn[i][0] = p0; loot_btn[i][1] = p1;
    }
    loot_btn_frame = ImGui::GetFrameCount();
}
/* a click outside the menus on something the overlay drew: 1 if taken */
extern "C" int battleoverlay_arrow_click(int x, int y);
extern "C" int overlay_click(int x, int y)
{
    if (battleoverlay_arrow_click(x, y)) { click_taken = 1; return 1; }
    if (ImGui::GetFrameCount() - loot_btn_frame <= 2 && (loot_active || barter_active))
        for (int i = 0; i < 2; i++)
            if (x >= loot_btn[i][0].x && x < loot_btn[i][1].x && y >= loot_btn[i][0].y && y < loot_btn[i][1].y) { loot_press(i); click_taken = 1; return 1; }
    return 0;
}
extern "C" int overlay_click_up(int x, int y) { (void)x; (void)y; int t = click_taken; click_taken = 0; return t; }   /* its release too */

static void draw_creation_guide(void)
{
    if (!ui_creation_guide || config_get_int("original", 0) || !hdart_on_screen("CHARGEN")) return;
    int st = (s16)g_rw(GV_create_state);
    static const char *T[5] = {
        "Making a character: name and sex first, then the family background, the attributes, and "
        "one profession after another, five years each, until you begin adventuring. Older characters "
        "know more, but age costs attributes.",
        "The family decides the first attributes and skills. The right side shows what the background "
        "under the pointer would give; EP (top right) are the points to spend next.",
        "Spend the EP on attributes: pick one, then + or -. Endurance and Strength are the hit points (and together set how much a character can carry); Agility counts in battle; Perception, Intelligence and Charisma serve many skills.",
        "Choose the profession for the next five years. The right side shows the one under the pointer: the EP it gives (top), the attributes after it, and for each skill gain:room, that is, what it adds at once and how many EP that skill can still take. Some professions open only after others, or need certain attributes.",
        "Spend the EP on skills: pick one, then + or -. A skill takes at most the room shown after the "
        "colon. Then choose the next profession, or begin adventuring.",
    };
    if (st < 0 || st > 4) return;
    ImVec2 a = g2w(3, 4), b = g2w(58, 196);
    ImGui::SetNextWindowPos(a); ImGui::SetNextWindowSize(ImVec2(b.x - a.x, b.y - a.y));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(20, 16, 12, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(236, 226, 205, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::Begin("##creation", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing |
                                            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PushFont(ui_font_overlay);
        /* about the size of the game's own text (6.5 of its 200 rows a line), smaller only if the
           text would not fit the panel */
        float k = (b.y - a.y) / 192.0f, wrap = b.x - a.x - 16, avail = b.y - a.y - 16;
        float fs = fmaxf(12.0f, k * 6.5f);
        while (fs > 12.0f && ui_font_overlay->CalcTextSizeA(fs, FLT_MAX, wrap, T[st]).y * 1.1f > avail) fs *= 0.92f;
        ImGui::SetWindowFontScale(fs / ui_font_overlay->FontSize);
        ImGui::TextWrapped("%s", T[st]);
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar(); ImGui::PopStyleColor(2);
}

extern "C" void travelmap_labels(void);
extern "C" void ui_overlay(void)
{
    {   /* a layer for the panels and badges over the picture: under the tooltips (windows), above the crisp text */
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##overlay_layer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ovl_dl = ImGui::GetWindowDrawList();             /* (left open to the end, so that its clipping and font stay set) */
    }
    draw_chances();
    draw_card_help();
    if (assist_card_help) { watch_saints(); draw_learned(); }
    travel_overlay();
    travelmap_labels();
    draw_picture_name();
    draw_new_encounter_seal();
    if (ns_screen != NS_CREATE && ns_screen != NS_ROSTER && ns_screen != NS_SAVES) draw_version();       /* (not over a character being made, the party chosen, the saves) */
    { void ui_margins(void); ui_margins(); }
    draw_shop_info();
    ui_draw_battle_bars();
    ui_draw_battle_info();
    ui_draw_battle_fast();
    { void battlelog_overlay(void); battlelog_overlay(); }
    if (ns_screen != NS_CREATE) draw_creation_guide();   /* (the page drawn by Darklands+ says it itself) */
    draw_loot_buttons();
    ImGui::End();                                       /* the layer */
    ovl_dl = nullptr;
}

/* the tooltip of a saint or an item, for ui/cardview.cpp */
void ui_card_formula_tip(ImVec2 pos, ImVec2 pivot, int formula)
{
    if (formula >= 0 && formula < (int)gamedata.formula_names.size()) show_tip(pos, pivot, -1, -1, formula);
}
void ui_card_tip(ImVec2 pos, ImVec2 pivot, int item, int saint)
{
    if (item >= (int)gamedata.items.size()) item = -1;
    if (saint >= (int)gamedata.saints.size()) saint = -1;
    if (item >= 0 || saint >= 0) show_tip(pos, pivot, item, saint, -1);
}
