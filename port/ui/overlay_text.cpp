// overlay_text.cpp - crisp text over the game's own screens (mods/hdtext.c finds the game's text
// runs; here they are drawn again at the window's resolution in the chosen typeface, with the
// abbreviations spelled out and the skills' pictures), the chances beside the original cards'
// options, and the pointer drawn live. Split from ui/overlay.cpp (overlay_parts.h).
#include "money.h"
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
#include "overlay_parts.h"
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
extern ImFont *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS];
extern int ui_game_font, ui_text_edge;
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
void draw_chances(void)
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
    { "Int", "Intelligence", "Intellect" }, { "Chr", "Charisma", 0 }, { "DF", "Divine Favor", "Divine Favor" },
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
 * translators, a list of every line shown (F12 > Mods: "Collect the texts shown") */
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
    /* a Darklands+ page over the picture (the character sheet, the loot, a prayer: NsDraw::page,
       opaque from x 60 on) last frame and still up: the game's text under it is not drawn (the
       sheet's ran to 26,000 vertices a frame, all hidden) */
    extern int ns_page_frame;
    const bool paged = ImGui::GetFrameCount() - ns_page_frame <= 1 && ns_showing();
    auto joins = [](const HdRun &q, const HdRun &r) { int gap = r.x - (q.x + q.w); return q.y == r.y && q.h == r.h && q.font == r.font && gap >= 0 && gap <= 3; };
    for (int i = 0; i < hdtext_nruns; i++) {
        if (i == skip) continue;
        HdRun r = hdtext_runs[i];
        if (paged && r.x >= 60) continue;
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
        t = money_words(t);                                          /* ("12pfs": with its coin, display.coins on) */
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
ImVec2 ptr_lo, ptr_hi; int ptr_frame = -10;      /* where the pointer was drawn (window pixels), for tooltips beside it */
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
