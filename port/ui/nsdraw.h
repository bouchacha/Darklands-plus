/* nsdraw.h - drawing for the native screens (ui/barterview.cpp, ui/sheetview.cpp ...): one
 * painter in game coordinates (x 0..320, y 0..200, as the game's pointer), the page style they
 * share, and the hit tests against the game's pointer and its clicks (mods/nscreen.h). */
#ifndef DL_NSDRAW_H
#define DL_NSDRAW_H
#include "imgui.h"
#include <string.h>
#include <string>
#include "../runtime/cpu.h"
#include "../mods/nscreen.h"
#include "ui.h"
#include "gamedata.h"

extern ImFont *ui_font_text, *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS], *ui_font_game_hi[UI_GAME_FONTS];
extern int ui_game_font;
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
float ui_face_k(ImFont *f);

struct NsDraw {
    ImDrawList *dl = nullptr;
    ImFont *f = nullptr, *fo = nullptr;      /* the game's typeface (names, headings), the plain one (numbers, notes) */
    float k = 1, kx = 1, fk = 1, fok = 1;    /* window pixels per game row / column; the faces' size factors */
    int mx = 0, my = 0;                      /* the pointer */
    bool click = false; int cx = -1, cy = -1;
    bool rclick = false;
    bool shift = false;

    /* the colours of the page */
    static constexpr ImU32 INK = IM_COL32(44, 26, 10, 255), INK_SOFT = IM_COL32(92, 60, 30, 230), INK_DIM = IM_COL32(44, 26, 10, 120);
    /* the type sizes, in game rows, the same on every native screen */
    static constexpr float FS_TITLE = 8.5f;   /* the screen's name (the game's typeface) */
    static constexpr float FS_HEAD = 6.8f;    /* headings, the strip's item name */
    static constexpr float FS_NAME = 5.8f;    /* the rows of the lists (the game's typeface) */
    static constexpr float FS_BODY = 5.3f;    /* numbers, descriptions (the plain typeface) */
    static constexpr float FS_SMALL = 4.7f;   /* buttons, tags, hints */
    static constexpr ImU32 RED = IM_COL32(140, 30, 18, 255), GREEN = IM_COL32(40, 96, 34, 255), LIGHT = IM_COL32(250, 238, 212, 255);

    bool begin()
    {
        f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
        fo = ui_font_overlay ? ui_font_overlay : f;
        if (!f) return false;
        ImVec2 o = w(0, 0), e = w(320, 200);
        k = (e.y - o.y) / 200.0f; kx = (e.x - o.x) / 320.0f;
        if (k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) f = ui_font_game_hi[ui_game_font];
        fk = ui_face_k(f); fok = ui_face_k(fo);
        dl = ImGui::GetBackgroundDrawList();
        mx = ns_in.x; my = ns_in.y;
        click = ns_take_click(&cx, &cy);
        rclick = ns_in.rclick != 0; ns_in.rclick = 0;
        shift = ImGui::GetIO().KeyShift || (rb(0x0040, 0x0017) & 3);   /* (or the game's own keyboard: scripts, the controller) */
        return true;
    }
    static ImVec2 w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }
    float size(ImFont *ff, float gs) const { return gs * k * (ff == f ? fk : fok); }

    void rect(float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { dl->AddRectFilled(w(x0, y0), w(x1, y1), c, rnd * k); }
    void frame(float x0, float y0, float x1, float y1, ImU32 c, float rnd = 0) { dl->AddRect(w(x0, y0), w(x1, y1), c, rnd * k, 0, fmaxf(1.0f, 0.4f * k)); }
    /* text at (x, y) (its top); right_to >= 0: ending there; clip_to >= 0: cut there */
    void text(ImFont *ff, float gs, float x, float y, ImU32 c, const std::string &s, float right_to = -1, float clip_to = -1)
    {
        float sz = size(ff, gs);
        ImVec2 p = w(x, y);
        if (right_to >= 0) p.x = w(right_to, 0).x - ff->CalcTextSizeA(sz, FLT_MAX, 0, s.c_str()).x;
        if (clip_to >= 0) { ImVec4 cr(p.x, p.y - sz, w(clip_to, 0).x, p.y + 2 * sz); dl->AddText(ff, sz, p, c, s.c_str(), nullptr, 0, &cr); }
        else dl->AddText(ff, sz, p, c, s.c_str());
    }
    /* wrapped to width (game pixels); returns the height used, game pixels */
    float para(ImFont *ff, float gs, float x, float y, float width, ImU32 c, const std::string &s)
    {
        float sz = size(ff, gs), ww = width * kx;
        dl->AddText(ff, sz, w(x, y), c, s.c_str(), nullptr, ww);
        return ff->CalcTextSizeA(sz, FLT_MAX, ww, s.c_str()).y / k;
    }
    float width(ImFont *ff, float gs, const std::string &s) const { return ff->CalcTextSizeA(size(ff, gs), FLT_MAX, 0, s.c_str()).x / kx; }

    bool over(float x0, float y0, float x1, float y1) const { return mx >= x0 && mx < x1 && my >= y0 && my < y1; }
    bool hit(float x0, float y0, float x1, float y1) const { return click && cx >= x0 && cx < x1 && cy >= y0 && cy < y1; }

    /* a button; true when clicked */
    bool button(float x0, float y0, float x1, float y1, const std::string &label, ImU32 base, float gs = FS_SMALL)
    {
        bool h = over(x0, y0, x1, y1);
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(base);
        if (h) { c.x = fminf(1, c.x * 1.25f + 0.04f); c.y = fminf(1, c.y * 1.25f + 0.04f); c.z = fminf(1, c.z * 1.25f + 0.04f); c.w = 1; }
        rect(x0, y0, x1, y1, ImGui::ColorConvertFloat4ToU32(c), 1.5f);
        float tw = width(fo, gs, label);
        text(fo, gs, x0 + (x1 - x0 - tw) * 0.5f, y0 + (y1 - y0) * 0.5f - gs * 0.62f, LIGHT, label);
        return hit(x0, y0, x1, y1);
    }
    /* an item's picture (ui/inventory.cpp), centred at (x, y), `size` rows across; kept inside
       its square (a pike or a halberd is drawn long) */
    void icon(float x, float y, float size, int id, int qual = -1, int tier = 0)   /* (qual: a potion shows its grade; tier: a formula's, 1-3) */
    {
        void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q);
        void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier);
        ImVec2 c = w(x, y); float s = size * k, h = s * 0.62f;
        dl->PushClipRect(ImVec2(c.x - h, c.y - h * 1.15f), ImVec2(c.x + h, c.y + h), true);
        if (tier > 0) ui_item_icon_tier(dl, c, s, id, tier); else ui_item_icon_q(dl, c, s, id, qual);
        dl->PopClipRect();
    }
    /* the page under a native screen (over the picture beside the party panel) */
    void page()
    {
        rect(60, 0, 320, 200, IM_COL32(236, 222, 188, 255));
        rect(61.5f, 1.5f, 318.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
        frame(61.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);
    }
};

static inline std::string ns_gtext(const char *s) { return game_text(s, strlen(s)); }
static inline const ItemDef *ns_itemdef(int id) { return id >= 0 && id < (int)gamedata.items.size() ? &gamedata.items[id] : nullptr; }
static inline std::string ns_item_name(int id) { const ItemDef *d = ns_itemdef(id); return d ? game_text(d->name.c_str(), d->name.size()) : "?"; }
/* a quality worth showing: equipment, potions and tools; not for the things a shop always sells
 * at 25 (throwing axes, javelins, darts, arrows, quarrels, balls, ingredients) unless one differs */
static inline bool ns_shows_quality(int id, int q)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return false;
    bool fixed = (d->type >= 0x17 && d->type <= 0x19) || (d->type >= 0x40 && d->type <= 0x42) || d->kind == K_INGREDIENT;
    if (fixed) return q != 25;
    return d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD || d->kind == K_POTION ||
           d->kind == K_TOOL || d->kind == K_INSTRUMENT;
}
/* one line on a weapon or an armour (the combat tables, far 20a5), "" for other things */
std::string ns_item_line(int id);
std::string ns_quality_words(int id, int qual);       /* ", quality 30"; a potion: ", grade 2 of 3 (quality 35)" */
/* what a thing is good for, where its kind does not say it (ingredients: the potions; rope,
   lockpicks, instruments; goods nothing asks for), "" for others */
std::string ns_item_use(int id);
/* a weapon or an armour against what member m has in that place now, "" for other things */
std::string ns_item_compare(int id, int q, int m);

#endif
