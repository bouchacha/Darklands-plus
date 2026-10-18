// skillicons.cpp - the skills', attributes' and F12 tabs' pictures, drawn with ImGui shapes so they
// read at any size (the character sheet, character creation, the roster, card chances, the tabs).
#include <math.h>
#include "imgui.h"
#include "skillicons.h"

void ui_skill_icon(ImDrawList *dl, ImVec2 c, float s, int k)
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

void ui_attr_icon(ImDrawList *dl, ImVec2 c, float s, int k)
{
    dl->AddCircleFilled(c, s * 0.66f, IM_COL32(24, 14, 6, 150));
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
    case 4: /* intellect: a candle */
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

/* a skill's picture in the flow of a window's text, then the text goes on beside it */
void ui_skill_icon_inline(int k)
{
    float s = ImGui::GetFontSize();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(s * 1.05f, s));
    ui_skill_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + s * 0.5f, p.y + s * 0.52f), s * 0.6f, k);
    ImGui::SameLine(0, s * 0.2f);
}

/* the F12 menu's tabs: a small picture for each, drawn in the room left at the start of the label */
void ui_tab_icon(ImDrawList *dl, ImVec2 c, float s, int tab)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    auto C = [](int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); };
    ImU32 ink = C(40, 26, 14), gold = C(226, 182, 64), parch = C(238, 226, 196), red = C(170, 40, 30), steel = C(190, 194, 204), wood = C(150, 100, 52);
    switch (tab) {
    case 0:  /* inventory: a satchel */
        dl->AddRectFilled(P(-8, -3), P(8, 9), wood, 2 * u); dl->AddRect(P(-8, -3), P(8, 9), ink, 2 * u, 0, 1.0f * u);
        dl->AddRectFilled(P(-8, -3), P(8, 3), C(120, 78, 40), 2 * u); dl->AddRect(P(-8, -3), P(8, 3), ink, 2 * u, 0, 1.0f * u);
        dl->PathArcTo(P(0, -3), 5.5f * u, 3.1416f, 6.2832f, 12); dl->PathStroke(ink, 0, 1.6f * u);
        dl->AddRectFilled(P(-1.5f, 1), P(1.5f, 5), gold); break;
    case 1:  /* party: three heads */
        for (int i = -1; i <= 1; i++) {
            float x = i * 6.0f, y = i == 0 ? -1.0f : 1.0f;
            ImU32 col = i == 0 ? parch : C(205, 190, 160);
            dl->AddCircleFilled(P(x, y - 4), 3.2f * u, col); dl->AddCircle(P(x, y - 4), 3.2f * u, ink, 0, 0.8f * u);
            dl->PathArcTo(P(x, y + 6), 5.0f * u, 3.1416f, 6.2832f, 12); dl->PathFillConvex(col);
        }
        break;
    case 2: ui_skill_icon(dl, c, s, 7); break;                 /* alchemy: the flask */
    case 3: ui_skill_icon(dl, c, s, 9); break;                 /* saints: the halo */
    case 4: ui_skill_icon(dl, c, s, 11); break;                /* journal: the book */
    case 5:  /* saves: a sealed letter */
        dl->AddRectFilled(P(-9, -6), P(9, 7), parch); dl->AddRect(P(-9, -6), P(9, 7), ink, 0, 0, 1.0f * u);
        dl->AddLine(P(-9, -6), P(0, 1), ink, 0.9f * u); dl->AddLine(P(9, -6), P(0, 1), ink, 0.9f * u);
        dl->AddCircleFilled(P(0, 2), 3.4f * u, red); break;
    case 6:  /* travel: a compass rose */
        dl->AddCircle(P(0, 0), 8.5f * u, gold, 0, 1.4f * u);
        dl->AddTriangleFilled(P(0, -9), P(-2.4f, 0), P(2.4f, 0), red); dl->AddTriangleFilled(P(0, 9), P(-2.4f, 0), P(2.4f, 0), parch);
        dl->AddTriangleFilled(P(-9, 0), P(0, -2), P(0, 2), C(205, 190, 160)); dl->AddTriangleFilled(P(9, 0), P(0, -2), P(0, 2), C(205, 190, 160)); break;
    case 7:  /* helpers: a lit candle */
        dl->AddRectFilled(P(-2.5f, -2), P(2.5f, 9), parch); dl->AddRect(P(-2.5f, -2), P(2.5f, 9), ink, 0, 0, 0.8f * u);
        dl->AddLine(P(0, -2), P(0, -4), ink, 0.8f * u);
        dl->AddTriangleFilled(P(0, -11), P(-2.6f, -5), P(2.6f, -5), C(250, 190, 60)); dl->AddCircleFilled(P(0, -5.5f), 2.6f * u, C(250, 150, 40)); break;
    case 8: ui_skill_icon(dl, c, s, 14); break;                /* mods: the cog */
    case 9:  /* display: a framed picture */
        dl->AddRectFilled(P(-9, -7), P(9, 7), C(110, 160, 210)); dl->AddTriangleFilled(P(-8, 6), P(-2, -2), P(4, 6), C(70, 120, 60));
        dl->AddTriangleFilled(P(0, 6), P(4, 1), P(8, 6), C(90, 140, 70)); dl->AddCircleFilled(P(4, -3), 1.8f * u, C(250, 230, 120));
        dl->AddRect(P(-9, -7), P(9, 7), gold, 0, 0, 1.8f * u); break;
    case 10: /* records: a crown */
        dl->AddRectFilled(P(-8, 2), P(8, 7), gold);
        dl->AddTriangleFilled(P(-8, 2), P(-8, -7), P(-3, 2), gold); dl->AddTriangleFilled(P(-4, 2), P(0, -8), P(4, 2), gold); dl->AddTriangleFilled(P(3, 2), P(8, -7), P(8, 2), gold);
        dl->AddCircleFilled(P(0, 4.5f), 1.4f * u, red); dl->AddRect(P(-8, 2), P(8, 7), ink, 0, 0, 0.8f * u); break;
    case 11: /* about: i in a circle */
        dl->AddCircleFilled(P(0, 0), 8.5f * u, C(60, 90, 140)); dl->AddCircle(P(0, 0), 8.5f * u, ink, 0, 0.8f * u);
        dl->AddRectFilled(P(-1.3f, -1.5f), P(1.3f, 5.5f), parch); dl->AddCircleFilled(P(0, -4.5f), 1.5f * u, parch); break;
    case 12: ui_skill_icon(dl, c, s, 12); break;               /* editor: the quill */
    case 13: /* guide: an open book */
        dl->AddQuadFilled(P(-9, -6), P(0, -4), P(0, 8), P(-9, 6), parch); dl->AddQuadFilled(P(0, -4), P(9, -6), P(9, 6), P(0, 8), C(226, 214, 184));
        dl->AddQuad(P(-9, -6), P(0, -4), P(0, 8), P(-9, 6), ink, 0.8f * u); dl->AddQuad(P(0, -4), P(9, -6), P(9, 6), P(0, 8), ink, 0.8f * u);
        for (int i = 0; i < 3; i++) { dl->AddLine(P(-7, -2 + i * 3.0f), P(-2, -1 + i * 3.0f), C(120, 100, 80), 0.7f * u); dl->AddLine(P(2, -1 + i * 3.0f), P(7, -2 + i * 3.0f), C(120, 100, 80), 0.7f * u); }
        break;
    default: (void)steel; break;
    }
}
