// skillicons.cpp - the skills', attributes' and F12 tabs' pictures, drawn with ImGui shapes so they
// read at any size (the character sheet, character creation, the roster, card chances, the tabs).
#include <math.h>
#include "imgui.h"
#include "uitabs.h"
#include "skillicons.h"

static void skill_body(ImDrawList *dl, ImVec2 c, float s, int k)
{
    static const int WT[7] = { 3, 14, 17, 19, 23, 28, 31 };     /* a weapon of each class (LST type) */
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
    case 10: /* speak common: a herald's trumpet with its banner */
        dl->AddQuadFilled(P(-9, -1.2f), P(5, -2.4f), P(5, 2.4f), P(-9, 1.2f), gold);
        dl->AddTriangleFilled(P(5, -2.4f), P(10, -6), P(10, 6), gold); dl->AddTriangleFilled(P(5, 2.4f), P(10, 6), P(5, -2.4f), gold);
        dl->AddLine(P(10, -6), P(10, 6), ink, 0.9f * u); dl->AddCircleFilled(P(-9.5f, 0), 1.6f * u, gold);
        dl->AddQuadFilled(P(-5, 1.5f), P(3, 1.5f), P(3, 8), P(-5, 8), C(170, 40, 30));
        dl->AddTriangleFilled(P(-5, 8), P(3, 8), P(-1, 5.5f), C(238, 226, 196, 0) ); dl->AddLine(P(-5, 8), P(-1, 5.5f), ink, 0.6f * u); dl->AddLine(P(3, 8), P(-1, 5.5f), ink, 0.6f * u);
        dl->AddCircleFilled(P(-1, 4.3f), 1.3f * u, gold); break;
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
    case 14: /* artifice: a key, its bow and wards (locks, traps, devices) */
        dl->AddCircle(P(-5.5f, -5.5f), 3.6f * u, steel, 0, 2.2f * u);
        dl->AddLine(P(-3, -3), P(7.5f, 7.5f), steel, 2.2f * u);
        dl->AddLine(P(4.5f, 4.5f), P(2, 7), steel, 2.0f * u); dl->AddLine(P(6.5f, 6.5f), P(4.5f, 9), steel, 2.0f * u);
        dl->AddCircle(P(-5.5f, -5.5f), 4.8f * u, ink, 0, 0.6f * u); break;
    case 15: /* stealth: a dark hood, the face in shadow */
        dl->AddTriangleFilled(P(0, -10), P(-8, 9), P(8, 9), C(70, 60, 70));
        dl->AddCircleFilled(P(0, -1), 4.6f * u, C(70, 60, 70));
        dl->AddEllipseFilled(P(0, 0), ImVec2(3.0f * u, 3.8f * u), C(20, 16, 22));
        dl->AddCircleFilled(P(-1.2f, -0.4f), 0.55f * u, C(230, 220, 190)); dl->AddCircleFilled(P(1.2f, -0.4f), 0.55f * u, C(230, 220, 190)); break;
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

static void attr_body(ImDrawList *dl, ImVec2 c, float s, int k)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196);
    switch (k) {
    case 0: { /* endurance: a heart */
        ImU32 red = C(200, 50, 45);
        dl->AddCircleFilled(P(-3.6f, -2.5f), 4.6f * u, red); dl->AddCircleFilled(P(3.6f, -2.5f), 4.6f * u, red);
        dl->AddTriangleFilled(P(-8.1f, -1.2f), P(8.1f, -1.2f), P(0, 9), red);
        dl->AddCircleFilled(P(-4.5f, -4), 1.3f * u, C(250, 170, 160)); break; }
    case 1: /* strength: an iron weight */
        dl->AddCircle(P(0, -6), 3.2f * u, C(110, 110, 118), 0, 2.0f * u);
        dl->AddQuadFilled(P(-6, -3), P(6, -3), P(8.5f, 8), P(-8.5f, 8), C(96, 96, 104));
        dl->AddQuad(P(-6, -3), P(6, -3), P(8.5f, 8), P(-8.5f, 8), ink, 0.9f * u);
        dl->AddLine(P(-4, 1), P(-5, 6), C(150, 150, 160), 1.0f * u); break;
    case 2: /* agility: a wing */
        for (int i = 0; i < 3; i++) {
            float y = -6 + i * 4.5f, x = 9 - i * 3.0f;
            dl->AddTriangleFilled(P(-8, -8 + i * 1.5f), P(-8, y + 4.5f), P(x, y + 1), parch);
            dl->AddTriangle(P(-8, -8 + i * 1.5f), P(-8, y + 4.5f), P(x, y + 1), ink, 0.7f * u);
        }
        break;
    case 3: /* perception: an open eye */
        dl->AddBezierQuadratic(P(-10, 0), P(0, -9), P(10, 0), parch, 1.6f * u); dl->AddBezierQuadratic(P(-10, 0), P(0, 9), P(10, 0), parch, 1.6f * u);
        dl->AddCircleFilled(P(0, 0), 4.0f * u, C(80, 130, 200)); dl->AddCircleFilled(P(0, 0), 1.8f * u, C(15, 15, 20));
        dl->AddCircleFilled(P(1.2f, -1.2f), 0.8f * u, C(255, 255, 255)); break;
    case 4: /* intelligence: a candle */
        dl->AddRectFilled(P(-3, -2), P(3, 9), C(240, 232, 210)); dl->AddRect(P(-3, -2), P(3, 9), ink, 0, 0, 0.8f * u);
        dl->AddLine(P(0, -2), P(0, -4), ink, 0.8f * u);
        dl->AddTriangleFilled(P(-2.4f, -4.5f), P(2.4f, -4.5f), P(0, -11), C(255, 200, 70));
        dl->AddCircleFilled(P(0, -5.2f), 2.4f * u, C(255, 200, 70)); dl->AddCircleFilled(P(0, -5.5f), 1.1f * u, C(255, 250, 210)); break;
    case 5: /* charisma: a crown */
        dl->AddQuadFilled(P(-9, 6), P(9, 6), P(9, -5), P(-9, -5), gold);
        dl->AddTriangleFilled(P(-9, -5), P(-4, -5), P(-6.5f, -10), gold); dl->AddTriangleFilled(P(-2.5f, -5), P(2.5f, -5), P(0, -11), gold);
        dl->AddTriangleFilled(P(4, -5), P(9, -5), P(6.5f, -10), gold);
        dl->AddRect(P(-9, -5), P(9, 6), ink, 0, 0, 0.8f * u);
        dl->AddCircleFilled(P(0, 0.5f), 1.8f * u, C(190, 40, 40)); dl->AddCircleFilled(P(-5.5f, 0.5f), 1.3f * u, C(60, 110, 200)); dl->AddCircleFilled(P(5.5f, 0.5f), 1.3f * u, C(60, 110, 200)); break;
    case 6: /* divine favor: a chalice */
        dl->AddBezierQuadratic(P(-7, -9), P(-6, 2), P(0, 2), gold, 2.2f * u); dl->AddBezierQuadratic(P(7, -9), P(6, 2), P(0, 2), gold, 2.2f * u);
        dl->AddQuadFilled(P(-7, -9), P(7, -9), P(5, -1), P(-5, -1), gold);
        dl->AddRectFilled(P(-1, 2), P(1, 7), gold); dl->AddRectFilled(P(-5, 7), P(5, 9.5f), gold);
        dl->AddLine(P(-7, -9), P(7, -9), ink, 0.9f * u); break;
    }
}

/* the look of the skills' and attributes' pictures (display.skill_icon_look): 0 drawn in colour, 2
   heraldic (a charge in gold on a shield, its tincture by the group: sable the attributes, gules the
   weapons, azure knowledge, vert the abilities), on the character sheet only. ui_icon_disc: the dark disc behind a coloured one, where it must read on any ground */
int ui_icon_disc = 1;
#include "../runtime/config.h"
int ui_icon_heraldic_here;                    /* (the character sheet's skills page sets it while it draws: the heraldic look is for there only) */
static int icon_style(void) { int v = config_get_int("display.skill_icon_look", 2); return v == 0 || !ui_icon_heraldic_here ? 0 : 2; }   /* (0 in colour, 2 heraldic, the default; the woodcut look (1) was dropped) */
static int style_override = -1;
void ui_icon_style_preview(int st) { style_override = st; }
/* two tones: the light parts of a picture (a page, a flame's heart, a lens) become `light`, the rest
   `col`, so the detail drawn in pale paint stays visible instead of filling in */
static void recolor_since(ImDrawList *dl, int v0, ImU32 col, ImU32 light, int threshold = 185, ImU32 dark = 0, int dark_below = -1)
{
    for (int i = v0; i < dl->VtxBuffer.Size; i++) {
        ImU32 &c = dl->VtxBuffer[i].col; int a = (int)(c >> IM_COL32_A_SHIFT) & 255;
        int r = c & 255, g = (c >> 8) & 255, b = (c >> 16) & 255, lum = (r * 3 + g * 6 + b) / 10;
        ImU32 to = lum >= threshold ? light : lum < dark_below ? dark : col;   /* (dark: the picture's ink lines stay dark, its detail kept) */
        c = (to & ~IM_COL32_A_MASK) | ((ImU32)a << IM_COL32_A_SHIFT);
    }
}
static void shield(ImDrawList *dl, ImVec2 c, float s, ImU32 field)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    ImVec2 pts[12]; int n = 0;
    pts[n++] = P(-10, -11); pts[n++] = P(10, -11); pts[n++] = P(10, 0);
    for (int i = 1; i <= 4; i++) { float t = i / 5.0f; pts[n++] = P(10 * (1 - t) * (1 - t) + 0, 0 + 12 * t * (2 - t)); }
    pts[n++] = P(0, 12);
    for (int i = 4; i >= 1; i--) { float t = i / 5.0f; pts[n++] = P(-10 * (1 - t) * (1 - t), 12 * t * (2 - t)); }
    pts[n++] = P(-10, 0);
    dl->AddConvexPolyFilled(pts, n, field);
    dl->AddPolyline(pts, n, IM_COL32(210, 170, 70, 255), ImDrawFlags_Closed, 1.1f * u);
}
static void styled(ImDrawList *dl, ImVec2 c, float s, int group, void (*body)(ImDrawList *, ImVec2, float, int), int k)
{
    int st = style_override >= 0 ? style_override : icon_style();
    if (st == 2) {
        static const ImU32 FIELD[4] = { IM_COL32(36, 30, 30, 255), IM_COL32(160, 30, 30, 255), IM_COL32(30, 60, 140, 255), IM_COL32(30, 110, 50, 255) };
        shield(dl, c, s, FIELD[group]);
        /* the charge: as large as the shield allows, in gold with a dark edge under it (it must read when
           small); only near-white details (a page, a highlight) take the field's colour, so a steel key
           or a blade stays gold */
        float cs = s * 0.74f, sh = fmaxf(1.0f, s * 0.045f);
        ImVec2 cc(c.x, c.y - s * 0.05f);
        int v0 = dl->VtxBuffer.Size;
        body(dl, ImVec2(cc.x + sh, cc.y + sh), cs, k);
        recolor_since(dl, v0, IM_COL32(20, 10, 4, 230), IM_COL32(20, 10, 4, 230), 256);
        v0 = dl->VtxBuffer.Size;
        body(dl, cc, cs, k);
        recolor_since(dl, v0, IM_COL32(240, 202, 86, 255), FIELD[group], 236, IM_COL32(60, 34, 10, 255), 45);
        return;
    }
    if (ui_icon_disc) dl->AddCircleFilled(c, s * 0.66f, IM_COL32(24, 14, 6, 150));   /* a dark disc: the picture reads on any background */
    body(dl, c, s, k);
}
void ui_skill_icon(ImDrawList *dl, ImVec2 c, float s, int k) { styled(dl, c, s, k < 7 ? 1 : k < 13 ? 2 : 3, skill_body, k); }
void ui_attr_icon(ImDrawList *dl, ImVec2 c, float s, int k) { styled(dl, c, s, 0, attr_body, k); }

/* a skill's picture in the flow of a window's text, then the text goes on beside it */
void ui_skill_icon_inline(int k)
{
    float s = ImGui::GetFontSize();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(s * 1.05f, s));
    ui_skill_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + s * 0.5f, p.y + s * 0.52f), s * 0.6f, k);
    ImGui::SameLine(0, s * 0.2f);
}

/* the F12 menu's tabs (ui/uitabs.h): a small picture for each, drawn in the room left at the start of the label */
void ui_tab_icon(ImDrawList *dl, ImVec2 c, float s, int tab)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196), red = C(170, 40, 30), steel = C(190, 194, 204), wood = C(150, 100, 52);
    switch (tab) {
    case UI_PAGE_JOURNAL: {  /* journal: a travelling diary, closed, its strap buckled (not Speak Latin's open book) */
        dl->AddRectFilled(P(-7, -9), P(7, 9), C(120, 62, 30), 1.5f * u);
        dl->AddRectFilled(P(-7, -9), P(-4.5f, 9), C(90, 44, 20));                         /* the spine */
        dl->AddRect(P(-7, -9), P(7, 9), ink, 1.5f * u, 0, 0.9f * u);
        dl->AddRectFilled(P(5.5f, -8), P(7.6f, 8), parch);                                 /* the pages' edge */
        dl->AddRectFilled(P(-4.5f, -1.4f), P(9, 1.4f), C(70, 36, 16));                     /* the strap */
        dl->AddRect(P(6.4f, -2.2f), P(9.4f, 2.2f), gold, 0, 0, 0.9f * u);                  /* the buckle */
        dl->AddRectFilled(P(-1, -9), P(1, -11.5f), red); break; }                          /* the ribbon */
    case UI_PAGE_HELPERS: {  /* the game's settings: a mill's gear wheel (not Intelligence's candle) */
        const int N = 8;
        for (int i = 0; i < N; i++) {
            float a = i * 6.2832f / N;
            ImVec2 d(cosf(a), sinf(a)), n(-sinf(a), cosf(a));
            ImVec2 q[4] = { P(d.x * 6 + n.x * 1.8f, d.y * 6 + n.y * 1.8f), P(d.x * 9.5f + n.x * 1.4f, d.y * 9.5f + n.y * 1.4f),
                            P(d.x * 9.5f - n.x * 1.4f, d.y * 9.5f - n.y * 1.4f), P(d.x * 6 - n.x * 1.8f, d.y * 6 - n.y * 1.8f) };
            dl->AddConvexPolyFilled(q, 4, wood);
        }
        dl->AddCircleFilled(P(0, 0), 7 * u, wood, 20); dl->AddCircle(P(0, 0), 7 * u, ink, 20, 0.8f * u);
        dl->AddCircleFilled(P(0, 0), 2.6f * u, parch, 12); dl->AddCircle(P(0, 0), 2.6f * u, ink, 12, 0.7f * u); break; }
    case UI_PAGE_MODS: {  /* mods: a chest, its lid open (not Artifice's key) */
        dl->AddRectFilled(P(-9, -1), P(9, 8), wood, 0.8f * u); dl->AddRect(P(-9, -1), P(9, 8), ink, 0.8f * u, 0, 0.8f * u);
        ImVec2 lid[4] = { P(-9, -1), P(9, -1), P(7, -8), P(-7, -8) };
        dl->AddConvexPolyFilled(lid, 4, C(170, 118, 64)); dl->AddPolyline(lid, 4, ink, ImDrawFlags_Closed, 0.8f * u);
        dl->AddRectFilled(P(-9, 2), P(9, 3.4f), steel); dl->AddRectFilled(P(-1.6f, 1), P(1.6f, 5), gold);   /* the band, the lock */
        dl->AddCircleFilled(P(-3, -2.5f), 1.4f * u, gold); dl->AddCircleFilled(P(2.5f, -3), 1.2f * u, C(120, 170, 220)); break; }
    case UI_PAGE_DISPLAY:  /* display: a framed picture */
        dl->AddRectFilled(P(-9, -7), P(9, 7), C(110, 160, 210)); dl->AddTriangleFilled(P(-8, 6), P(-2, -2), P(4, 6), C(70, 120, 60));
        dl->AddTriangleFilled(P(0, 6), P(4, 1), P(8, 6), C(90, 140, 70)); dl->AddCircleFilled(P(4, -3), 1.8f * u, C(250, 230, 120));
        dl->AddRect(P(-9, -7), P(9, 7), gold, 0, 0, 1.8f * u); break;
    case UI_PAGE_ABOUT: /* about: i in a circle */
        dl->AddCircleFilled(P(0, 0), 8.5f * u, C(60, 90, 140)); dl->AddCircle(P(0, 0), 8.5f * u, ink, 0, 0.8f * u);
        dl->AddRectFilled(P(-1.3f, -1.5f), P(1.3f, 5.5f), parch); dl->AddCircleFilled(P(0, -4.5f), 1.5f * u, parch); break;
    case UI_PAGE_EDITOR: {  /* editor: a pair of dividers, as a draughtsman measures and redraws (not Read & Write's quill) */
        dl->AddLine(P(0, -8), P(-6, 9), steel, 1.6f * u); dl->AddLine(P(0, -8), P(6, 9), steel, 1.6f * u);
        dl->AddLine(P(-6, 9), P(-6.6f, 10.5f), ink, 0.9f * u); dl->AddLine(P(6, 9), P(6.6f, 10.5f), ink, 0.9f * u);
        dl->AddLine(P(-3.5f, 2), P(3.5f, 2), C(150, 120, 80), 1.0f * u);                 /* the arc that holds them */
        dl->AddCircleFilled(P(0, -8), 2.2f * u, gold); dl->AddCircle(P(0, -8), 2.2f * u, ink, 12, 0.7f * u); break; }
    case UI_PAGE_GUIDE: /* guide: an open book */
        dl->AddQuadFilled(P(-9, -6), P(0, -4), P(0, 8), P(-9, 6), parch); dl->AddQuadFilled(P(0, -4), P(9, -6), P(9, 6), P(0, 8), C(226, 214, 184));
        dl->AddQuad(P(-9, -6), P(0, -4), P(0, 8), P(-9, 6), ink, 0.8f * u); dl->AddQuad(P(0, -4), P(9, -6), P(9, 6), P(0, 8), ink, 0.8f * u);
        for (int i = 0; i < 3; i++) { dl->AddLine(P(-7, -2 + i * 3.0f), P(-2, -1 + i * 3.0f), C(120, 100, 80), 0.7f * u); dl->AddLine(P(2, -1 + i * 3.0f), P(7, -2 + i * 3.0f), C(120, 100, 80), 0.7f * u); }
        break;
    default: (void)steel; (void)wood; break;
    }
}

/* the F12 window's tabs (ui/ui.cpp): the tab icons, and Reconstructed Content a scroll mended with thread */
void ui_page_icon(ImDrawList *dl, ImVec2 c, float s, int page)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196);
    if (page == UI_PAGE_RESTORED) {
        dl->AddRectFilled(P(-6, -7), P(6, 7), parch); dl->AddRect(P(-6, -7), P(6, 7), ink, 0, 0, 0.7f * u);
        dl->AddCircleFilled(P(-6, -7), 2.2f * u, C(210, 196, 160), 10); dl->AddCircleFilled(P(6, 7), 2.2f * u, C(210, 196, 160), 10);   /* the rolled ends */
        dl->AddLine(P(-3, -7), P(1, -1), ink, 0.8f * u); dl->AddLine(P(1, -1), P(-1, 3), ink, 0.8f * u); dl->AddLine(P(-1, 3), P(3, 7), ink, 0.8f * u);   /* the tear */
        for (int i = 0; i < 3; i++) { float y = -4 + i * 3.5f; dl->AddLine(P(-2.5f + i * 0.8f, y), P(2.5f + i * 0.8f, y + 1.2f), C(170, 40, 30), 1.0f * u); }   /* the stitches */
    } else ui_tab_icon(dl, c, s, page);
}

/* the Settings page's sections (ui/settings.cpp tab_helpers): a small picture beside each name in
   the list at the left, none the same as a skill's, an attribute's or a tab's */
void ui_section_icon(ImDrawList *dl, ImVec2 c, float s, const char *name)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196), red = C(170, 40, 30), steel = C(176, 182, 194), wood = C(150, 100, 52), stone = C(168, 158, 140);
    auto is = [&](const char *n) { return !strcmp(name, n); };
    if (is("Preset")) {                 /* a lever on its dial, three marks: one setting of many */
        dl->AddCircleFilled(P(0, 2), 8 * u, parch, 20); dl->AddCircle(P(0, 2), 8 * u, ink, 20, 0.9f * u);
        for (int i = 0; i < 3; i++) { float a = 3.1416f * (1.15f + i * 0.35f); dl->AddCircleFilled(P(cosf(a) * 6, 2 + sinf(a) * 6), 1.1f * u, red, 8); }
        dl->AddLine(P(0, 2), P(3.5f, -4.5f), ink, 1.6f * u); dl->AddCircleFilled(P(0, 2), 1.8f * u, gold, 10);
    } else if (is("New screens")) {      /* an arched window, its panes leaded */
        dl->AddRectFilled(P(-6, -2), P(6, 9), C(120, 170, 210)); dl->AddCircleFilled(P(0, -2), 6 * u, C(120, 170, 210), 20);
        dl->AddLine(P(0, -8), P(0, 9), ink, 0.9f * u); dl->AddLine(P(-6, 2), P(6, 2), ink, 0.9f * u);
        dl->AddRect(P(-6, -2), P(6, 9), stone, 0, 0, 1.6f * u); dl->PathArcTo(P(0, -2), 6 * u, 3.1416f, 6.2832f, 16); dl->PathStroke(stone, 0, 1.6f * u);
    } else if (is("Text")) {             /* an illuminated capital */
        dl->AddRectFilled(P(-8, -8), P(8, 8), red, 1.2f * u); dl->AddRect(P(-8, -8), P(8, 8), gold, 1.2f * u, 0, 1.0f * u);
        ImFont *f = ImGui::GetFont(); const char *t = "A"; ImVec2 ts = f->CalcTextSizeA(14 * u, FLT_MAX, 0, t);
        dl->AddText(f, 14 * u, ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f), gold, t);
    } else if (is("Frames and clock")) { /* a gilt frame's corner with its rosette */
        dl->AddRectFilled(P(-8, -8), P(8, 8), parch); dl->AddRect(P(-8, -8), P(8, 8), gold, 0, 0, 2.2f * u);
        dl->AddRect(P(-5, -5), P(5, 5), C(170, 130, 50), 0, 0, 0.8f * u);
        for (int i = 0; i < 4; i++) dl->AddCircleFilled(P(i & 1 ? 8 : -8, i & 2 ? 8 : -8), 2.0f * u, gold, 10);
    } else if (is("Cards and towns")) {  /* a town gate between two towers */
        dl->AddRectFilled(P(-9, -4), P(-4, 9), stone); dl->AddRectFilled(P(4, -4), P(9, 9), stone); dl->AddRectFilled(P(-4, 0), P(4, 9), C(150, 140, 122));
        dl->AddTriangleFilled(P(-10, -4), P(-3, -4), P(-6.5f, -10), red); dl->AddTriangleFilled(P(3, -4), P(10, -4), P(6.5f, -10), red);
        dl->AddRectFilled(P(-2.2f, 3), P(2.2f, 9), C(60, 40, 24)); dl->AddCircleFilled(P(0, 3), 2.2f * u, C(60, 40, 24), 10);
        dl->AddRect(P(-9, -4), P(9, 9), ink, 0, 0, 0.7f * u);
    } else if (is("The party and its items")) {   /* a traveller's pack, its flap buckled */
        dl->AddRectFilled(P(-7, -4), P(7, 9), wood, 2 * u); dl->AddRect(P(-7, -4), P(7, 9), ink, 2 * u, 0, 0.8f * u);
        dl->AddRectFilled(P(-7, -4), P(7, 2), C(120, 76, 36), 2 * u);
        dl->AddRectFilled(P(-1.4f, 0), P(1.4f, 4), gold);
        dl->PathArcTo(P(0, -4), 4 * u, 3.1416f, 6.2832f, 12); dl->PathStroke(C(90, 56, 26), 0, 1.4f * u);
    } else if (is("Battles")) {          /* a great helm, its eye slit and breaths */
        dl->AddRectFilled(P(-6.5f, -6), P(6.5f, 8), steel, 2 * u); dl->AddCircleFilled(P(0, -5), 6.5f * u, steel, 18);
        dl->AddRect(P(-6.5f, -6), P(6.5f, 8), ink, 2 * u, 0, 0.8f * u);
        dl->AddRectFilled(P(-6.5f, -1.5f), P(6.5f, 0.2f), ink); dl->AddLine(P(0, -11), P(0, 8), C(120, 126, 140), 0.9f * u);
        for (int i = 0; i < 3; i++) dl->AddCircleFilled(P(3, 3 + i * 1.8f), 0.6f * u, ink, 6);
    } else if (is("Travel")) {           /* a signpost, two arms */
        dl->AddRectFilled(P(-1, -8), P(1, 10), wood);
        ImVec2 a1[5] = { P(-1, -7), P(7, -7), P(9.5f, -5), P(7, -3), P(-1, -3) }; dl->AddConvexPolyFilled(a1, 5, parch); dl->AddPolyline(a1, 5, ink, ImDrawFlags_Closed, 0.7f * u);
        ImVec2 a2[5] = { P(1, -1), P(-7, -1), P(-9.5f, 1), P(-7, 3), P(1, 3) }; dl->AddPolyline(a2, 5, ink, ImDrawFlags_Closed, 0.7f * u);
        ImVec2 a3[5] = { P(1, -1), P(1, 3), P(-7, 3), P(-9.5f, 1), P(-7, -1) }; dl->AddConvexPolyFilled(a3, 5, parch); dl->AddPolyline(a2, 5, ink, ImDrawFlags_Closed, 0.7f * u);
    } else if (is("Fixes for bugs of the original")) {   /* a mallet mending */
        ImVec2 h[4] = { P(-8, -3), P(-2, -9), P(1, -6), P(-5, 0) }; dl->AddConvexPolyFilled(h, 4, wood); dl->AddPolyline(h, 4, ink, ImDrawFlags_Closed, 0.7f * u);
        dl->AddLine(P(-2, -3), P(8, 8), C(110, 76, 40), 2.0f * u);
        dl->AddLine(P(3, 9), P(9, 9), steel, 1.4f * u);
    } else if (is("Screen")) {           /* a reading glass */
        dl->AddLine(P(3.5f, 3.5f), P(9, 9), wood, 2.4f * u);
        dl->AddCircleFilled(P(-2, -2), 6.5f * u, C(190, 220, 236), 20); dl->AddCircle(P(-2, -2), 6.5f * u, gold, 20, 1.6f * u);
        dl->AddLine(P(-5, -5), P(-2.5f, -6.5f), C(250, 250, 250), 1.0f * u);
    } else if (is("Intro")) {            /* a stage's curtains drawn apart */
        dl->AddRectFilled(P(-9, -8), P(9, 9), C(40, 30, 24)); dl->AddRectFilled(P(-9, -9), P(9, -6), red);
        ImVec2 l[3] = { P(-9, -6), P(-2, -6), P(-9, 9) }, r[3] = { P(9, -6), P(2, -6), P(9, 9) };
        dl->AddTriangleFilled(l[0], l[1], l[2], red); dl->AddTriangleFilled(r[0], r[2], r[1], red);
        dl->AddRect(P(-9, -9), P(9, 9), gold, 0, 0, 0.8f * u);
    } else if (is("Sound")) {            /* a hunting horn */
        dl->PathArcTo(P(1, 2), 7 * u, 3.4f, 6.0f, 16); dl->PathStroke(gold, 0, 2.2f * u);
        ImVec2 bell[3] = { P(7.3f, -1), P(10, -6), P(5, -3.5f) }; dl->AddTriangleFilled(bell[0], bell[1], bell[2], gold);
        dl->AddCircleFilled(P(-6, 0), 1.3f * u, ink, 8);
    } else if (is("Controller")) {       /* a game controller (an object of today: drawn plain) */
        dl->AddRectFilled(P(-8, -4), P(8, 4), C(70, 70, 80), 4 * u); dl->AddCircleFilled(P(-6, 3), 3.5f * u, C(70, 70, 80), 12); dl->AddCircleFilled(P(6, 3), 3.5f * u, C(70, 70, 80), 12);
        dl->AddRectFilled(P(-6.5f, -0.6f), P(-2.5f, 0.6f), parch); dl->AddRectFilled(P(-5.1f, -2), P(-3.9f, 2), parch);
        dl->AddCircleFilled(P(4, -1), 1.0f * u, C(88, 170, 60), 8); dl->AddCircleFilled(P(6, 1), 1.0f * u, C(214, 52, 54), 8);
    } else if (is("Saves")) {            /* a page kept with a ribbon */
        dl->AddRectFilled(P(-6, -8), P(6, 8), parch); dl->AddRect(P(-6, -8), P(6, 8), ink, 0, 0, 0.7f * u);
        for (int i = 0; i < 4; i++) dl->AddLine(P(-4, -4 + i * 3.0f), P(3, -4 + i * 3.0f), C(150, 130, 100), 0.6f * u);
        ImVec2 rb[5] = { P(2, -9), P(5, -9), P(5, 3), P(3.5f, 1.5f), P(2, 3) }; dl->AddConvexPolyFilled(rb, 5, red);
    } else {                            /* anything new: a plain gilt lozenge */
        ImVec2 q[4] = { P(0, -8), P(8, 0), P(0, 8), P(-8, 0) }; dl->AddConvexPolyFilled(q, 4, gold); dl->AddPolyline(q, 4, ink, ImDrawFlags_Closed, 0.8f * u);
    }
}

/* test (uiconstyles): the pictures in each look, side by side */
static int sheet_on;
extern "C" void ui_icon_styles_sheet(int on) { sheet_on = on; }
extern "C" const char *gm_skill_name(int k); extern "C" const char *gm_attr_name(int a);
extern "C" void ui_icon_styles_draw(void)
{
    if (!sheet_on) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList(); ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(246, 236, 212, 255));
    static const char *TITLE[2] = { "Drawn in color (no disc)", "Heraldic" }; static const int ST[2] = { 0, 2 };
    float colw = ds.x / 2, row = ds.y / 27.5f, sz = row * 0.8f;
    for (int j = 0; j < 2; j++) {
        float x = j * colw + 20;
        dl->AddText(ImGui::GetFont(), row * 0.7f, ImVec2(x, row * 0.3f), IM_COL32(60, 40, 20, 255), TITLE[j]);
        ui_icon_style_preview(ST[j]); int keep = ui_icon_disc; ui_icon_disc = 0;
        for (int i = 0; i < 26; i++) {
            float y = row * (1.4f + i);
            ImVec2 c(x + sz * 0.6f, y + row * 0.5f);
            if (i < 7) ui_attr_icon(dl, c, sz, i); else ui_skill_icon(dl, c, sz, i - 7);
            dl->AddText(ImGui::GetFont(), row * 0.55f, ImVec2(x + sz * 1.5f, y + row * 0.2f), IM_COL32(60, 40, 20, 255), i < 7 ? gm_attr_name(i) : gm_skill_name(i - 7));
        }
        ui_icon_style_preview(-1); ui_icon_disc = keep;
    }
}
