// combaticons.cpp - see combaticons.h. Line art on a 24-unit box (the armor's tunic on a 20-unit one), centred.
#include "combaticons.h"
#include "pen.h"

void layer_glyph(ImDrawList *dl, ImVec2 c, float s, int which, ImU32 ink);   /* (ui/worldmap_draw.cpp: 4, the hourglass) */

void ui_combat_icon(ImDrawList *dl, ImVec2 c, float s, int kind, ImU32 ink)
{
    if (kind == CI_TIME) { layer_glyph(dl, c, s * 0.9f, 4, ink); return; }
    pen::Pen p{ dl, c, s / 24.0f, 1.0f };
    const float w = 1.7f;
    switch (kind) {
    case CI_TWO_HANDS:                                  /* two fists on one grip */
        p.line(0, -10, 0, 10, ink, w);
        for (int k = 0; k < 2; k++) {
            float y = k ? 1 : -5;
            p.rect(-4, y, 4, y + 4, (ink & 0x00ffffff) | 0x50000000, 2);
            p.rect_line(-4, y, 4, y + 4, ink, w * 0.85f, 2);
        }
        break;
    case CI_DAMAGE:                                     /* a blow's burst */
        p.poly_line({ 0, -10, 2, -4, 8, -6, 5, -1, 10, 2, 4, 3, 5, 10, 0, 6, -5, 10, -4, 3, -10, 2, -5, -1, -8, -6, -2, -4 }, ink, w);
        break;
    case CI_PEN:                                        /* a shaft through a plate: the plate broken where it passes, the head clear beyond it */
        p.line(-2, -10, -2, -3.2f, ink, w * 1.7f);
        p.line(-2, 3.2f, -2, 10, ink, w * 1.7f);
        p.line(-11, 0, 4, 0, ink, w);
        p.tri(11, 0, 4, -3.6f, 4, 3.6f, ink);
        break;
    case CI_HIT:                                        /* a blade, struck home */
        p.line(-9, 9, 2, -2, ink, w);
        p.line(-7, 3, -3, 7, ink, w);
        p.line(5, -10, 5, -7, ink, w * 0.9f); p.line(10, -5, 7, -5, ink, w * 0.9f);
        p.line(8.5f, -8.5f, 6.5f, -6.5f, ink, w * 0.9f); p.line(1.5f, -8.5f, 3.5f, -6.5f, ink, w * 0.9f); p.line(8.5f, -1.5f, 6.5f, -3.5f, ink, w * 0.9f);
        break;
    case CI_DODGE: case CI_DODGE_SHOT: {                /* a figure, and the blow going past it (or a shot) */
        p.ring(-4, -1, 3.5f, ink, w);
        dl->AddBezierCubic(p.P(-9, 10), p.P(-9, 6), p.P(-7, 4), p.P(-4, 4), ink, p.W(w));
        dl->AddBezierCubic(p.P(-4, 4), p.P(-1, 4), p.P(1, 6), p.P(1, 10), ink, p.W(w));
        if (kind == CI_DODGE) {
            dl->AddBezierCubic(p.P(2, -9), p.P(8, -7), p.P(10, 0), p.P(6, 6), ink, p.W(w));
            p.line(6, 6, 5.5f, 2.5f, ink, w); p.line(6, 6, 2.6f, 5, ink, w);
        } else {
            p.line(1, -7, 10, -7, ink, w);
            p.line(7, -10, 10, -7, ink, w); p.line(7, -4, 10, -7, ink, w);
        }
        break;
    }
    }
}

static const char *const TIER_NAME[6] = { "cloth", "soft", "stiff", "scale", "mail", "plate" };
static const char *const TIER_KINDS[6] = { "no armor", "padded or leather", "studded or cuir-bouilli", "scale", "chain or brigandine", "plate" };
const char *ui_armor_tier_name(int t) { return t >= 0 && t < 6 ? TIER_NAME[t] : "?"; }
const char *ui_armor_tier_kinds(int t) { return t >= 0 && t < 6 ? TIER_KINDS[t] : "?"; }

void ui_armor_icon(ImDrawList *dl, ImVec2 c, float s, int tier, ImU32 ink)
{
    static const ImU32 FILL[6] = { IM_COL32(242, 228, 194, 255), IM_COL32(160, 118, 74, 255), IM_COL32(122, 85, 50, 255),
                                   IM_COL32(138, 129, 114, 255), IM_COL32(154, 160, 166, 255), IM_COL32(201, 205, 210, 255) };
    const ImU32 light = IM_COL32(242, 228, 194, 255);
    if (tier < 0) tier = 0;
    if (tier > 5) tier = 5;
    pen::Pen p{ dl, c, s / 20.0f, 1.0f };
    p.poly({ -5, -8, -2, -9, 2, -9, 5, -8, 9, -4, 6, -2, 6, 9, -6, 9, -6, -2, -9, -4 }, FILL[tier]);
    switch (tier) {
    case 2: for (int k = 0; k < 4; k++) p.disc(k & 1 ? 2.0f : -2.0f, k & 2 ? 4.0f : 0.0f, 1.2f, light); break;   /* studs */
    case 3:                                                                                                       /* scales */
        for (float y : { 0.0f, 4.0f }) { p.bez(-5, y, -3, y + 2, -1, y, light, 1.0f); p.bez(-1, y, 1, y + 2, 3, y, light, 1.0f); p.bez(3, y, 4, y + 2, 5, y, light, 1.0f); }
        break;
    case 4:                                                                                                       /* rings */
        for (float y : { -1.0f, 2.0f, 5.0f }) p.line(-4, y, 4, y, ink, 0.8f);
        for (float x : { -2.0f, 2.0f }) p.line(x, -2, x, 7, ink, 0.8f);
        break;
    case 5: p.line(0, -7, 0, 8, ink, 1.0f); p.line(-5, 1, 5, 1, ink, 1.0f); break;                             /* plates */
    }
    p.poly_line({ -5, -8, -2, -9, 2, -9, 5, -8, 9, -4, 6, -2, 6, 9, -6, 9, -6, -2, -9, -4 }, ink, 1.3f);
}

static float inline_at(void (*draw)(ImDrawList *, ImVec2, float, int, ImU32), int which, ImU32 ink)
{
    float fs = ImGui::GetFontSize(), sz = fs * 1.15f;
    if (!ink) ink = ImGui::GetColorU32(ImGuiCol_Text);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(sz, fs));
    draw(ImGui::GetWindowDrawList(), ImVec2(p.x + sz * 0.5f, p.y + fs * 0.5f), sz, which, ink);
    ImGui::SameLine(0, fs * 0.25f);
    return sz;
}
float ui_combat_icon_inline(int kind, ImU32 ink) { return inline_at(ui_combat_icon, kind, ink); }
float ui_armor_icon_inline(int tier, ImU32 ink) { return inline_at(ui_armor_icon, tier, ink); }
