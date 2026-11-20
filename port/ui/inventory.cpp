// inventory.cpp - the things' pictures, drawn by kind (weapons, armour, ingredients, potions ...)
// for every screen that lists them; what a thing is good for (ui_item_details: the numbers of a
// weapon, armour or shield and who in the party could use it); the tidy order of a pack; and
// giving a thing to another member through the game's own routine (give_stack, s031 18e7:007c:
// the test command ugive). The party's things themselves are on the character sheet.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "imgui_internal.h"
#include "../mods/nscreen.h"
#include "richtext.h"
#include "nsdraw.h"
#include "skillicons.h"
#include "ui.h"
#include "gamedata.h"
#include "../mods/gamemodel.h"
#include "../mods/relics.h"
#include "combaticons.h"
extern "C" {
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
}


static std::string member_name(int m, bool full)
{
    char nm[32]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + (full ? G_M_FULLNAME : G_M_SHORTNAME)), nm, sizeof nm);
    return nm;
}

static const ItemDef *def_of(int id)
{
    if (id < 0 || id >= (int)gamedata.items.size()) return nullptr;
    return &gamedata.items[id];
}

/* ---------------------------------------------------------------- icons */
static ImU32 col(float r, float g, float b, float a = 1) { return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a)); }

struct Tint { const char *name; float r, g, b; };
static const Tint TINTS[] = {
    { "Pure Gold", 0.95f, 0.78f, 0.20f }, { "Manganes", 0.38f, 0.30f, 0.42f }, { "Naptha", 0.45f, 0.30f, 0.10f },
    { "Marsh Vapor", 0.55f, 0.70f, 0.45f }, { "East. Black Bean", 0.16f, 0.12f, 0.10f }, { "Zincblende", 0.55f, 0.38f, 0.22f },
    { "Antimoni", 0.70f, 0.72f, 0.76f }, { "Orpiment", 0.97f, 0.65f, 0.15f }, { "White Cinnabar", 0.95f, 0.88f, 0.90f },
    { "Solanaceae", 0.48f, 0.22f, 0.55f }, { "Aqua Regia", 0.95f, 0.62f, 0.18f }, { "Gum", 0.82f, 0.55f, 0.20f },
    { "Mondragora", 0.55f, 0.40f, 0.25f }, { "Alum", 0.92f, 0.94f, 0.96f }, { "Camomile", 0.98f, 0.92f, 0.55f },
    { "Nikel", 0.62f, 0.66f, 0.62f }, { "Pitchblende", 0.18f, 0.24f, 0.18f }, { "Zinken", 0.52f, 0.60f, 0.70f },
    { "Brimstone", 0.95f, 0.88f, 0.20f }, { "Choleric Base", 0.90f, 0.35f, 0.15f }, { "Phleqmatic Base", 0.30f, 0.50f, 0.90f },
    { "Sanguine Base", 0.80f, 0.12f, 0.15f }, { "Melanc. Base", 0.25f, 0.20f, 0.18f },
};

static ImVec4 tint_for(const ItemDef *d, int id)
{
    if (d) for (const Tint &t : TINTS) if (d->game_name == t.name) return ImVec4(t.r, t.g, t.b, 1);
    // stable pseudo-random hue from the id
    float h = fmodf(id * 0.61803f, 1.0f), r, g, b;
    ImGui::ColorConvertHSVtoRGB(h, 0.55f, 0.85f, r, g, b);
    return ImVec4(r, g, b, 1);
}

static bool is_liquid(const ItemDef *d)
{
    if (!d) return false;
    return d->game_name == "Naptha" || d->game_name == "Aqua Regia" || d->game_name.find("Base") != std::string::npos;
}
static bool is_plant(const ItemDef *d)
{
    if (!d) return false;
    return d->game_name == "Solanaceae" || d->game_name == "Mondragora" || d->game_name == "Camomile" || d->game_name == "Gum" || d->game_name == "East. Black Bean";
}

/* equipment drawn by item type (LST +30): each weapon class has its own shape, armour shows
 * whether it covers the body (V:) or the legs (L:) and what it is made of, shields by size */
static bool draw_equipment(ImDrawList *dl, ImVec2 c, float s, int t, bool relic)
{
    float u = s / 16.0f;
    ImU32 ink = col(0.15f, 0.10f, 0.06f), steel = col(0.74f, 0.76f, 0.80f), steeld = col(0.46f, 0.48f, 0.53f),
          wood = col(0.52f, 0.34f, 0.18f), woodd = col(0.36f, 0.22f, 0.11f), gold = col(0.88f, 0.70f, 0.22f),
          leath = col(0.42f, 0.26f, 0.14f);
    bool weapon = (t >= 0 && t <= 33) || t == 101 || t == 105 || t == 154, diag = weapon && !(t >= 27 && t <= 33) && t != 105 && t != 154;
    /* weapons lie diagonally, point to the upper right */
    float ca = diag ? 0.7071f : 1.0f, sa = diag ? 0.7071f : 0.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + (x * ca + y * sa) * u, c.y + (-x * sa + y * ca) * u); };
    auto poly = [&](std::initializer_list<ImVec2> pts, ImU32 fill, bool outline = true) {
        ImVec2 v[16]; int n = 0; for (ImVec2 q : pts) if (n < 16) v[n++] = P(q.x, q.y);
        dl->AddConvexPolyFilled(v, n, fill);
        if (outline) dl->AddPolyline(v, n, ink, ImDrawFlags_Closed, 0.6f * u);
    };
    auto rect = [&](float x0, float y0, float x1, float y1, ImU32 fill) { poly({ ImVec2(x0, y0), ImVec2(x1, y0), ImVec2(x1, y1), ImVec2(x0, y1) }, fill); };
    auto line = [&](float x0, float y0, float x1, float y1, ImU32 cl, float w) { dl->AddLine(P(x0, y0), P(x1, y1), cl, w * u); };
    auto dot = [&](float x, float y, float r, ImU32 cl) { dl->AddCircleFilled(P(x, y), r * u, cl); };
    if (relic) dl->AddCircleFilled(c, 11.5f * u, col(1.0f, 0.85f, 0.35f, 0.28f));
    auto hilt = [&](float y, float guard, float grip) {
        rect(-guard, y, guard, y + 1.4f, gold);
        rect(-0.8f, y + 1.4f, 0.8f, y + 1.4f + grip, leath);
        dot(0, y + 2.2f + grip, 1.3f, gold);
    };
    auto shaft = [&](float y0, float y1, float w, ImU32 cl) { rect(-w, y0, w, y1, cl); };
    auto axe_blade = [&](float side, float y, float k) {    /* side +1 right, -1 left; k size */
        poly({ ImVec2(side * 0.8f, y - 2.5f * k), ImVec2(side * 4.0f * k, y - 5.0f * k), ImVec2(side * 6.5f * k, y - 3.5f * k),
               ImVec2(side * 7.0f * k, y), ImVec2(side * 6.5f * k, y + 3.5f * k), ImVec2(side * 4.0f * k, y + 4.5f * k), ImVec2(side * 0.8f, y + 2.0f * k) }, steel);
    };
    switch (t) {
    /* swords and knives */
    case 0: poly({ ImVec2(-1.5f, 5), ImVec2(1.5f, 5), ImVec2(1.2f, -12.5f), ImVec2(0, -14.5f), ImVec2(-1.2f, -12.5f) }, steel); line(0, 4, 0, -12, steeld, 0.6f); hilt(5, 5.2f, 5); break;
    case 1: poly({ ImVec2(-1.3f, 5), ImVec2(1.3f, 5), ImVec2(1.0f, -10.5f), ImVec2(0, -12.5f), ImVec2(-1.0f, -10.5f) }, steel); line(0, 4, 0, -10, steeld, 0.5f); hilt(5, 4.2f, 3.5f); break;
    case 2: poly({ ImVec2(-1.2f, 5), ImVec2(1.4f, 5), ImVec2(3.0f, -6), ImVec2(1.5f, -10.5f), ImVec2(-0.6f, -11.5f), ImVec2(-1.2f, -8) }, steel); hilt(5, 3.8f, 3.5f); break;
    case 3: poly({ ImVec2(-1.4f, 5), ImVec2(1.4f, 5), ImVec2(1.2f, -5), ImVec2(0, -7.5f), ImVec2(-1.2f, -5) }, steel); line(0, 4, 0, -5, steeld, 0.5f); hilt(5, 3.6f, 3.2f); break;
    case 4: poly({ ImVec2(-0.7f, 3), ImVec2(0.7f, 3), ImVec2(0, -8) }, steel); hilt(3, 2.6f, 3.2f); break;
    case 5: case 26: poly({ ImVec2(-1.1f, 3), ImVec2(1.1f, 3), ImVec2(1.0f, -3.5f), ImVec2(0, -6.5f), ImVec2(-1.0f, -3.5f) }, steel); hilt(3, t == 26 ? 1.6f : 2.4f, t == 26 ? 2.4f : 3.0f); break;
    /* axes */
    case 6: shaft(-10, 11, 0.8f, wood); axe_blade(1, -6.5f, 1.0f); axe_blade(-1, -6.5f, 1.0f); break;
    case 7: case 23: shaft(-6, t == 23 ? 7 : 9, 0.7f, wood); axe_blade(1, -4.0f, 0.75f); break;
    case 8: shaft(-10, 11, 0.8f, wood); axe_blade(1, -6.5f, 1.15f); break;
    /* impact weapons */
    case 10: shaft(-8, 11, 0.8f, wood); rect(-5.5f, -11, 5.5f, -6.5f, steel); break;
    case 12: shaft(-8, 11, 1.0f, wood); rect(-5, -12.5f, 5, -5.5f, steeld); line(-5, -9, 5, -9, ink, 0.5f); break;
    case 13: shaft(-8, 11, 0.7f, wood); rect(-4.5f, -10, 1.8f, -7, steel); poly({ ImVec2(1.8f, -10), ImVec2(7.5f, -8.5f), ImVec2(1.8f, -7) }, steel); break;
    case 14: shaft(-6, 11, 0.8f, steeld); dot(0, -8, 3.6f, steel);
        for (int k = 0; k < 6; k++) { float a = k * 1.047f; line(0, -8, sinf(a) * 4.8f, -8 + cosf(a) * 4.8f, steeld, 1.2f); }
        dl->AddCircle(P(0, -8), 3.6f * u, ink, 0, 0.6f * u); break;
    case 11: case 15: { float k = t == 11 ? 1.3f : 1.0f;
        poly({ ImVec2(-0.9f, 11), ImVec2(0.9f, 11), ImVec2(2.6f * k, -8), ImVec2(1.6f * k, -11.5f), ImVec2(-1.6f * k, -11.5f), ImVec2(-2.6f * k, -8) }, wood);
        dot(1.2f * k, -6, 0.6f, woodd); dot(-1.0f * k, -2, 0.6f, woodd); dot(0.6f, 3, 0.5f, woodd); break; }
    /* flails */
    case 16: case 17: { float top = t == 16 ? -1 : 2;
        shaft(top, 11, 0.8f, wood);
        for (int k = 1; k <= 4; k++) dl->AddCircle(P(k * 0.9f, top - k * 1.5f), 0.7f * u, steeld, 0, 0.6f * u);
        float bx = 4.6f, by = top - 8.0f; dot(bx, by, 2.8f, steel);
        for (int k = 0; k < 8; k++) { float a = k * 0.785f; line(bx, by, bx + sinf(a) * 3.9f, by + cosf(a) * 3.9f, steeld, 0.7f); }
        dl->AddCircle(P(bx, by), 2.8f * u, ink, 0, 0.6f * u);
        if (t == 16) { dot(-3.8f, top - 6.0f, 2.2f, steel); line(0, top, -3.8f, top - 6.0f, steeld, 0.6f); }
        break; }
    /* polearms */
    case 18: shaft(-9, 14, 0.6f, wood); poly({ ImVec2(-0.8f, -9), ImVec2(0.8f, -9), ImVec2(0, -15) }, steel); break;
    case 19: shaft(-9, 14, 0.65f, wood); poly({ ImVec2(-0.7f, -9), ImVec2(0.7f, -9), ImVec2(0, -15) }, steel);
        poly({ ImVec2(0.65f, -9.5f), ImVec2(5.5f, -11), ImVec2(6.5f, -6.5f), ImVec2(5.5f, -3.5f), ImVec2(0.65f, -5) }, steel);
        poly({ ImVec2(-0.65f, -8.5f), ImVec2(-3.5f, -9), ImVec2(-0.65f, -6.5f) }, steel); break;
    case 20: case 21: case 24: { float bot = t == 20 ? 14 : t == 21 ? 10 : 10, w = t == 24 ? 0.45f : 0.6f;
        shaft(-9, bot, w, wood);
        poly({ ImVec2(-1.7f * (t == 24 ? 0.6f : 1), -10), ImVec2(0, -15), ImVec2(1.7f * (t == 24 ? 0.6f : 1), -10), ImVec2(0, -8.5f) }, steel); break; }
    case 22: shaft(-12, 12, 0.9f, wood); rect(-1.1f, -12.5f, 1.1f, -10.5f, steeld); rect(-1.1f, 10.5f, 1.1f, 12.5f, steeld); break;
    case 25: shaft(-4, 6, 0.4f, wood); poly({ ImVec2(-0.9f, -4), ImVec2(0, -8), ImVec2(0.9f, -4) }, steel);
        poly({ ImVec2(0, 4), ImVec2(-2.2f, 7.5f), ImVec2(0, 6.5f) }, col(0.85f, 0.85f, 0.80f)); poly({ ImVec2(0, 4), ImVec2(2.2f, 7.5f), ImVec2(0, 6.5f) }, col(0.85f, 0.85f, 0.80f)); break;
    /* bows: long, short, composite (recurved) */
    case 27: case 28: case 29: { float h = t == 28 ? 11.5f : t == 29 ? 8.5f : 9.5f, sx = 2.5f;
        if (t == 27) { dl->AddBezierCubic(P(sx, -h), P(-6, -h + 3), P(-6, h - 3), P(sx, h), wood, 2.2f * u);
                       line(sx, -h, sx + 2, -h - 1.5f, woodd, 1.6f); line(sx, h, sx + 2, h + 1.5f, woodd, 1.6f); }
        else dl->AddBezierQuadratic(P(sx, -h), P(-h * 0.9f, 0), P(sx, h), wood, 2.2f * u);
        line(sx, -h, sx, h, col(0.92f, 0.90f, 0.82f), 0.6f);
        rect(-h * 0.9f * 0.5f + sx * 0.5f - 0.9f, -1.3f, -h * 0.9f * 0.5f + sx * 0.5f + 0.9f, 1.3f, leath); break; }
    /* gear and tools */
    case 98: shaft(-4, 11, 1.0f, wood); rect(-1.6f, -5, 1.6f, -2.5f, leath);
        poly({ ImVec2(-2.2f, -5), ImVec2(2.2f, -5), ImVec2(1.2f, -10), ImVec2(0, -13.5f), ImVec2(-1.4f, -9.5f) }, col(0.98f, 0.55f, 0.12f), false);
        poly({ ImVec2(-1.0f, -5), ImVec2(1.0f, -5), ImVec2(0.3f, -10) }, col(1.0f, 0.88f, 0.35f), false); break;
    case 99: rect(-2.2f, -4, 2.2f, 10, col(0.95f, 0.93f, 0.85f)); rect(-4, 10, 4, 11.5f, steeld); line(0, -4, 0, -5.5f, ink, 0.5f);
        poly({ ImVec2(-1.1f, -5.5f), ImVec2(0, -10), ImVec2(1.1f, -5.5f), ImVec2(0, -4.6f) }, col(1.0f, 0.75f, 0.2f), false); break;
    case 100: for (int k = 0; k < 3; k++) dl->AddCircle(P(0, 1), (7.5f - k * 2.2f) * u, col(0.72f, 0.58f, 0.35f), 0, 1.6f * u);
        line(6.5f, 3, 10, 9, col(0.72f, 0.58f, 0.35f), 1.6f); break;
    case 101: poly({ ImVec2(-1.4f, -8), ImVec2(1.4f, -8), ImVec2(0, 11) }, steeld); rect(-3, -10.5f, 3, -8, steel); break;
    case 102: rect(-5, -6, 5, 8, col(0.95f, 0.80f, 0.40f, 0.55f)); dl->AddRect(P(-5, -6), P(5, 8), steeld, 0, 0, 1.2f * u);
        line(-5, 1, 5, 1, steeld, 0.8f); line(0, -6, 0, 8, steeld, 0.8f);
        poly({ ImVec2(-6, -6), ImVec2(6, -6), ImVec2(0, -10) }, steeld); dl->AddCircle(P(0, -11.5f), 1.6f * u, steeld, 0, 0.9f * u); break;
    case 103: dot(0, 0, 8.5f, gold); dot(0, 0, 7, col(0.96f, 0.92f, 0.80f)); dl->AddCircle(c, 8.5f * u, ink, 0, 0.7f * u);
        for (int k = 0; k < 12; k++) { float a = k * 0.5236f; dot(sinf(a) * 5.8f, -cosf(a) * 5.8f, 0.45f, ink); }
        line(0, 0, 0, -5, ink, 0.9f); line(0, 0, 3.5f, 1.5f, ink, 0.9f); break;
    case 104: shaft(-3, 11, 0.7f, steeld);
        for (int sgn = -1; sgn <= 1; sgn += 2) dl->AddBezierQuadratic(P(0, -3), P(sgn * 7.5f, -3), P(sgn * 6.5f, -9.5f), steel, 1.5f * u);
        dl->AddBezierQuadratic(P(0, -3), P(0, -8), P(0, -10.5f), steel, 1.5f * u); break;
    case 105: line(-7, 7, 4, -4, steel, 1.1f); line(4, -4, 7, -4, steel, 1.1f); line(7, -4, 7, -6.5f, steel, 1.1f);
        dl->AddCircle(P(-8.2f, 8.2f), 2.2f * u, steeld, 0, 1.2f * u); break;
    case 106: poly({ ImVec2(-6, 9), ImVec2(-6, -8), ImVec2(-3, -10), ImVec2(-4, 6) }, wood);
        dl->AddBezierQuadratic(P(-4, -9), P(6, -9), P(6, 9), wood, 1.8f * u); line(-6, 9, 6, 9, woodd, 1.6f);
        for (int k = 0; k < 5; k++) { float x = -3.2f + k * 1.9f; line(x, -7.5f + k * 0.4f, x, 8, col(0.92f, 0.88f, 0.75f), 0.4f); } break;
    case 107: { float ca2 = 0.7071f; (void)ca2;
        dl->AddLine(P(-9, 9), P(9, -9), col(0.80f, 0.62f, 0.35f), 2.6f * u);
        for (int k = 0; k < 5; k++) { float q = -3 + k * 2.4f; dot(q, -q, 0.55f, woodd); } break; }
    case 154: dl->AddCircle(P(-5, 0), 3.2f * u, gold, 0, 1.4f * u); line(-1.8f, 0, 9, 0, gold, 1.4f); line(7, 0, 7, 3, gold, 1.4f); line(9, 0, 9, 3.5f, gold, 1.4f); break;
    case 155: case 156: rect(-7, -9, 7, 9, col(0.95f, 0.90f, 0.75f));
        for (int r = -6; r < 5; r += 3) line(-5, (float)r, 5, (float)r, col(0.4f, 0.3f, 0.2f), 0.5f);
        dot(4, 7, 2.6f, t == 155 ? col(0.7f, 0.1f, 0.1f) : col(0.15f, 0.25f, 0.6f));
        if (t == 155) { dl->AddText(nullptr, 8 * u, P(-5.5f, 3), col(0.5f, 0.35f, 0.1f), "fl"); } break;
    /* crossbows */
    case 30: case 31: {
        rect(-1.3f, -6, 1.3f, 11, wood); line(0, -4, 0, 6, woodd, 0.6f);
        dl->AddBezierQuadratic(P(-9, -3), P(0, -10), P(9, -3), t == 30 ? steel : woodd, 2.0f * u);
        line(-9, -3, 0, 1, col(0.9f, 0.88f, 0.8f), 0.6f); line(9, -3, 0, 1, col(0.9f, 0.88f, 0.8f), 0.6f);
        rect(1.3f, 6, 2.6f, 8.5f, steeld); break; }
    /* hand guns: iron and brass */
    case 32: case 33: {
        ImU32 barrel = t == 32 ? col(0.32f, 0.33f, 0.36f) : col(0.86f, 0.66f, 0.25f);
        rect(-11, -3.2f, 5, -0.6f, barrel); rect(-11.5f, -3.6f, -10.2f, -0.2f, barrel);
        poly({ ImVec2(4, -3.6f), ImVec2(12, -1), ImVec2(12, 5), ImVec2(9, 5), ImVec2(4, 0.4f) }, wood);
        dot(3, -3.4f, 0.6f, ink); break; }
    /* ammunition */
    case 64: case 65:
        for (int k = -1; k <= 1; k++) {
            float x = k * 3.0f, len = t == 64 ? 11 : 7.5f;
            line(x, len, x, -len + 2, wood, 0.7f);
            poly({ ImVec2(x - 1.0f, -len + 2), ImVec2(x, -len), ImVec2(x + 1.0f, -len + 2) }, steel, false);
            poly({ ImVec2(x, len - 3), ImVec2(x - 1.3f, len + 0.5f), ImVec2(x, len - 0.5f) }, t == 64 ? col(0.9f, 0.9f, 0.85f) : col(0.65f, 0.25f, 0.2f), false);
            poly({ ImVec2(x, len - 3), ImVec2(x + 1.3f, len + 0.5f), ImVec2(x, len - 0.5f) }, t == 64 ? col(0.9f, 0.9f, 0.85f) : col(0.65f, 0.25f, 0.2f), false);
        }
        break;
    case 66: { ImU32 b = col(0.28f, 0.29f, 0.32f);
        dot(-3.5f, 4, 3, b); dot(3.5f, 4, 3, b); dot(0, -1.5f, 3, b);
        dot(-4.4f, 3.0f, 0.9f, col(0.7f, 0.72f, 0.75f)); dot(2.6f, 3.0f, 0.9f, col(0.7f, 0.72f, 0.75f)); dot(-0.9f, -2.5f, 0.9f, col(0.7f, 0.72f, 0.75f)); break; }
    /* shields */
    case 95: dot(0, 0, 8, col(0.62f, 0.50f, 0.32f)); dl->AddCircle(c, 8 * u, steeld, 0, 1.4f * u); dot(0, 0, 2.6f, steel); dl->AddCircle(c, 2.6f * u, ink, 0, 0.6f * u); break;
    case 96: dl->PathLineTo(P(-8, -9)); dl->PathLineTo(P(8, -9)); dl->PathLineTo(P(8, 1)); dl->PathLineTo(P(0, 11)); dl->PathLineTo(P(-8, 1));
        dl->PathFillConvex(col(0.62f, 0.18f, 0.12f));
        line(0, -9, 0, 11, gold, 1.6f); line(-8, -3, 8, -3, gold, 1.6f); break;
    case 97: rect(-7, -11, 7, 9, col(0.20f, 0.30f, 0.55f)); poly({ ImVec2(-7, 9), ImVec2(7, 9), ImVec2(0, 13) }, col(0.20f, 0.30f, 0.55f));
        line(0, -11, 0, 12, gold, 1.8f); dot(0, -2, 2.0f, gold); break;
    default: {
        if (t < 67 || t > 84) return false;
        /* armour: V: covers the body, L: the legs; material by colour and pattern */
        bool body = t <= 75; int m = (t - 67) % 9;
        static const float MC[9][3] = { { 0.62f, 0.22f, 0.20f }, { 0.86f, 0.79f, 0.60f }, { 0.56f, 0.36f, 0.19f }, { 0.50f, 0.32f, 0.17f },
            { 0.38f, 0.23f, 0.12f }, { 0.60f, 0.61f, 0.65f }, { 0.68f, 0.70f, 0.74f }, { 0.50f, 0.14f, 0.18f }, { 0.82f, 0.84f, 0.88f } };
        ImU32 base = col(MC[m][0], MC[m][1], MC[m][2]), dk = col(MC[m][0] * 0.62f, MC[m][1] * 0.62f, MC[m][2] * 0.62f);
        float x0, x1, y0, y1;                     /* pattern area */
        if (body) {
            if (m != 8) { poly({ ImVec2(-10, -8), ImVec2(-6.5f, -9), ImVec2(-6.5f, -2), ImVec2(-10, -3) }, base); poly({ ImVec2(6.5f, -9), ImVec2(10, -8), ImVec2(10, -3), ImVec2(6.5f, -2) }, base); }
            poly({ ImVec2(-6.5f, -9), ImVec2(-2.5f, -9), ImVec2(0, -7), ImVec2(2.5f, -9), ImVec2(6.5f, -9), ImVec2(6, 9), ImVec2(-6, 9) }, base);
            if (m == 8) { dot(-6.5f, -7.5f, 2.6f, base); dot(6.5f, -7.5f, 2.6f, base); dl->AddCircle(P(-6.5f, -7.5f), 2.6f * u, ink, 0, 0.6f * u); dl->AddCircle(P(6.5f, -7.5f), 2.6f * u, ink, 0, 0.6f * u); }
            x0 = -5.2f; x1 = 5.2f; y0 = -6; y1 = 8;
        } else {
            rect(-6.5f, -10, 6.5f, -7.5f, dk);
            poly({ ImVec2(-6.5f, -7.5f), ImVec2(-0.5f, -7.5f), ImVec2(-1.2f, 10.5f), ImVec2(-5.5f, 10.5f) }, base);
            poly({ ImVec2(0.5f, -7.5f), ImVec2(6.5f, -7.5f), ImVec2(5.5f, 10.5f), ImVec2(1.2f, 10.5f) }, base);
            if (m == 8 || m == 6 || m == 5) { dot(-3.3f, 1.5f, 1.7f, dk); dot(3.3f, 1.5f, 1.7f, dk); }
            x0 = -5.6f; x1 = 5.6f; y0 = -6.5f; y1 = 9.5f;
        }
        auto inside = [&](float x, float y) { return body || fabsf(x) > 1.3f; };
        switch (m) {
        case 1: for (float y = y0; y <= y1; y += 2.6f) for (float x = x0; x <= x1; x += 2.6f) if (inside(x, y)) { line(x - 0.9f, y - 0.9f, x + 0.9f, y + 0.9f, dk, 0.4f); line(x - 0.9f, y + 0.9f, x + 0.9f, y - 0.9f, dk, 0.4f); } break;
        case 3: case 7: for (float y = y0 + 0.6f; y <= y1; y += 2.4f) for (float x = x0 + 0.6f; x <= x1; x += 2.4f) if (inside(x, y)) dot(x, y, 0.55f, m == 7 ? gold : col(0.82f, 0.80f, 0.74f)); break;
        case 4: for (float y = y0; y <= y1; y += 3.0f) line(x0, y, x1, y, dk, 0.6f); break;
        case 5: for (float y = y0; y <= y1; y += 2.0f) for (float x = x0 + ((int)((y - y0) / 2) & 1 ? 1.0f : 0); x <= x1; x += 2.0f)
                    if (inside(x, y)) { dl->PathArcTo(P(x, y), 1.0f * u, 0.2f, 2.94f, 6); dl->PathStroke(dk, 0, 0.5f * u); } break;
        case 6: for (float y = y0; y <= y1; y += 1.5f) for (float x = x0; x <= x1; x += 1.5f) if (inside(x, y)) dl->AddCircle(P(x, y), 0.55f * u, dk, 6, 0.35f * u); break;
        case 8: line(0, body ? -7 : -7.5f, 0, body ? 9 : -7.5f, col(1, 1, 1, 0.75f), body ? 0.8f : 0);
            for (float y = (body ? 2.5f : -3.5f); y <= y1; y += 3.2f) line(x0, y, x1, y, dk, 0.5f);
            if (body) line(-3, -6, -3.5f, 6, col(1, 1, 1, 0.55f), 0.9f); break;
        default: break;
        }
        break; }
    }
    (void)woodd;
    return true;
}

bool ui_draw_potion(ImDrawList *dl, ImVec2 c, float s, int id, int tier);   /* ui/alchemyicons.cpp */
bool ui_draw_ingredient(ImDrawList *dl, ImVec2 c, float s, int id);
int ui_potion_tier(int q, int base);
static int icon_tier;                       /* a potion's grade (1-3) for the next draw_icon, 0 unknown */

/* the quest things and trophies, each its own picture (they were all a sheet of paper) */
static bool draw_special(ImDrawList *dl, ImVec2 c, float s, int id)
{
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    ImU32 ink = col(0.18f, 0.12f, 0.07f), gold = col(0.90f, 0.72f, 0.24f), goldd = col(0.62f, 0.46f, 0.12f), bone = col(0.93f, 0.89f, 0.78f),
          boned = col(0.70f, 0.64f, 0.52f), silver = col(0.80f, 0.82f, 0.86f), silverd = col(0.52f, 0.54f, 0.60f), red = col(0.62f, 0.14f, 0.10f),
          brown = col(0.46f, 0.30f, 0.16f), cloth = col(0.40f, 0.33f, 0.24f), green = col(0.30f, 0.50f, 0.22f), parch = col(0.94f, 0.88f, 0.70f);
    auto book = [&](ImU32 cover, ImU32 trim) {
        dl->AddRectFilled(P(-7, -9), P(7, 9), cover, 1.2f * u);
        dl->AddRectFilled(P(5, -8), P(7, 8), parch);
        dl->AddRect(P(-7, -9), P(7, 9), ink, 1.2f * u, 0, 0.8f * u);
        dl->AddRectFilled(P(-1, -6), P(1, 4), trim); dl->AddRectFilled(P(-4, -3), P(4, -1), trim);
    };
    switch (id) {
    case 126:                                                                          /* Evil Medallion */
        dl->AddLine(P(-5, -10), P(0, -4), goldd, 0.8f * u); dl->AddLine(P(5, -10), P(0, -4), goldd, 0.8f * u);
        dl->AddCircleFilled(P(0, 2), 7 * u, col(0.30f, 0.26f, 0.22f)); dl->AddCircle(P(0, 2), 7 * u, goldd, 0, 1.2f * u);
        dl->AddTriangleFilled(P(0, 7), P(-4, -2), P(4, -2), red); dl->AddTriangleFilled(P(-4, -2), P(-5, -5), P(-2, -2), red); dl->AddTriangleFilled(P(4, -2), P(5, -5), P(2, -2), red);
        return true;
    case 128:                                                                          /* Counterfeits: false coins */
        dl->AddEllipseFilled(P(-2, 4), ImVec2(7 * u, 3 * u), goldd); dl->AddEllipseFilled(P(-2, 2.5f), ImVec2(7 * u, 3 * u), gold);
        dl->AddEllipseFilled(P(2, -2), ImVec2(7 * u, 3 * u), goldd); dl->AddEllipseFilled(P(2, -3.5f), ImVec2(7 * u, 3 * u), gold);
        dl->AddLine(P(-2, -8), P(6, 2), red, 1.4f * u);
        return true;
    case 129: book(col(0.30f, 0.18f, 0.10f), gold); return true;                     /* Prayerbook */
    case 130: book(red, gold); dl->AddRect(P(-6, -8), P(4, 8), gold, 0, 0, 0.8f * u); return true;   /* Illuminated Gospel */
    case 131:                                                                          /* Hermit's Habit: a hooded robe */
        dl->AddQuadFilled(P(-4, -6), P(4, -6), P(8, 10), P(-8, 10), cloth);
        dl->AddCircleFilled(P(0, -7), 4.5f * u, cloth); dl->AddCircleFilled(P(0, -6.5f), 2.6f * u, col(0.16f, 0.12f, 0.08f));
        dl->AddLine(P(-3, 1), P(3, 1), brown, 1.2f * u);
        return true;
    case 132:                                                                          /* History Scroll */
        dl->AddRectFilled(P(-6, -7), P(6, 7), parch);
        dl->AddRectFilled(P(-8, -9), P(8, -6), brown, 1.5f * u); dl->AddRectFilled(P(-8, 6), P(8, 9), brown, 1.5f * u);
        for (int r = -4; r <= 4; r += 2) dl->AddLine(P(-4, (float)r), P(4, (float)r), col(0.4f, 0.3f, 0.2f), 0.6f * u);
        return true;
    case 133:                                                                          /* Ancient Crown */
        dl->PathLineTo(P(-9, 6)); dl->PathLineTo(P(-9, -6)); dl->PathLineTo(P(-5, -1)); dl->PathLineTo(P(-2.5f, -8)); dl->PathLineTo(P(0, -2));
        dl->PathLineTo(P(2.5f, -8)); dl->PathLineTo(P(5, -1)); dl->PathLineTo(P(9, -6)); dl->PathLineTo(P(9, 6)); dl->PathFillConcave(gold);
        dl->AddRectFilled(P(-9, 3), P(9, 7), goldd);
        dl->AddCircleFilled(P(0, 5), 1.4f * u, red); dl->AddCircleFilled(P(-5, 5), 1.1f * u, col(0.2f, 0.4f, 0.8f)); dl->AddCircleFilled(P(5, 5), 1.1f * u, col(0.2f, 0.6f, 0.3f));
        return true;
    case 134:                                                                          /* Hyoscyamus (henbane): a stem and pale flowers */
        dl->AddBezierQuadratic(P(0, 10), P(-1, 0), P(1, -8), green, 1.4f * u);
        dl->AddCircleFilled(P(-4, -4), 3 * u, col(0.86f, 0.82f, 0.50f)); dl->AddCircleFilled(P(-4, -4), 1.1f * u, col(0.35f, 0.15f, 0.30f));
        dl->AddCircleFilled(P(4, -7), 3 * u, col(0.86f, 0.82f, 0.50f)); dl->AddCircleFilled(P(4, -7), 1.1f * u, col(0.35f, 0.15f, 0.30f));
        dl->AddTriangleFilled(P(0, 3), P(-6, 1), P(-1, 6), green); dl->AddTriangleFilled(P(0, 1), P(6, -1), P(1, 4), green);
        return true;
    case 135:                                                                          /* Silver Mace */
        dl->AddLine(P(-6, 9), P(3, -3), brown, 2.0f * u);
        dl->AddCircleFilled(P(4.5f, -4.5f), 4.2f * u, silver); dl->AddCircle(P(4.5f, -4.5f), 4.2f * u, silverd, 0, 0.8f * u);
        for (int k = 0; k < 6; k++) { float a = k * 1.047f; dl->AddTriangleFilled(P(4.5f + cosf(a) * 4, -4.5f + sinf(a) * 4), P(4.5f + cosf(a + 0.35f) * 4, -4.5f + sinf(a + 0.35f) * 4), P(4.5f + cosf(a + 0.17f) * 6.5f, -4.5f + sinf(a + 0.17f) * 6.5f), silverd); }
        return true;
    case 136:                                                                          /* Tarnhelm */
        dl->PathArcTo(P(0, 2), 8 * u, 3.14159f, 6.28318f, 20); dl->PathFillConvex(silver);
        dl->AddRectFilled(P(-8, 2), P(8, 4.5f), silverd);
        dl->AddRectFilled(P(-1, 2), P(1, 9), silverd);
        dl->AddTriangleFilled(P(-8, -2), P(-12, -9), P(-6, -5), bone); dl->AddTriangleFilled(P(8, -2), P(12, -9), P(6, -5), bone);
        return true;
    case 139:                                                                          /* a quest's thing: a tied bundle */
        dl->AddRectFilled(P(-7, -5), P(7, 8), brown, 2 * u); dl->AddLine(P(0, -5), P(0, 8), gold, 1.2f * u); dl->AddLine(P(-7, 1), P(7, 1), gold, 1.2f * u);
        dl->AddCircleFilled(P(0, -6), 2.2f * u, gold);
        return true;
    case 140:                                                                          /* Globe with Fish */
        dl->AddCircleFilled(P(0, 0), 8.5f * u, col(0.62f, 0.80f, 0.92f, 0.8f)); dl->AddCircle(P(0, 0), 8.5f * u, ink, 0, 0.8f * u);
        dl->AddTriangleFilled(P(-4, 0), P(2, -3), P(2, 3), col(0.95f, 0.55f, 0.15f)); dl->AddTriangleFilled(P(2, 0), P(5, -2.5f), P(5, 2.5f), col(0.95f, 0.55f, 0.15f));
        dl->AddCircleFilled(P(-3, -4.5f), 1.4f * u, col(1, 1, 1, 0.7f));
        return true;
    case 141:                                                                          /* Lantern of Light */
        dl->AddCircleFilled(P(0, 1), 9 * u, col(1.0f, 0.9f, 0.5f, 0.30f));
        dl->AddRectFilled(P(-5, -6), P(5, 8), col(1.0f, 0.90f, 0.50f)); dl->AddRect(P(-5, -6), P(5, 8), goldd, 0, 0, 1.2f * u);
        dl->AddLine(P(0, -6), P(0, 8), goldd, 0.8f * u); dl->AddTriangleFilled(P(-6, -6), P(6, -6), P(0, -10), goldd);
        return true;
    case 142:                                                                          /* Magic Honey: a pot */
        dl->AddCircleFilled(P(0, 3), 7.5f * u, col(0.80f, 0.60f, 0.30f)); dl->AddRectFilled(P(-5, -7), P(5, -3), col(0.80f, 0.60f, 0.30f));
        dl->AddRectFilled(P(-6, -8), P(6, -6), brown); dl->AddCircleFilled(P(-2, 3), 2.5f * u, col(0.95f, 0.75f, 0.20f));
        dl->AddBezierQuadratic(P(-6, -7), P(-3, -1), P(-4, 4), col(0.98f, 0.78f, 0.20f), 1.6f * u);
        return true;
    case 143:                                                                          /* Seed of Life: a glowing seed with a sprout */
        dl->AddCircleFilled(P(0, 3), 8 * u, col(0.6f, 0.9f, 0.5f, 0.25f));
        dl->AddEllipseFilled(P(0, 4), ImVec2(4.5f * u, 6 * u), col(0.55f, 0.38f, 0.18f));
        dl->AddBezierQuadratic(P(0, -2), P(0, -7), P(4, -9), green, 1.5f * u); dl->AddTriangleFilled(P(1, -6), P(-5, -9), P(-1, -4), green);
        return true;
    case 144:                                                                          /* Magic Balance */
        dl->AddRectFilled(P(-0.8f, -8), P(0.8f, 8), goldd); dl->AddRectFilled(P(-5, 8), P(5, 10), goldd);
        dl->AddLine(P(-8, -6), P(8, -6), gold, 1.4f * u);
        dl->AddLine(P(-8, -6), P(-10, 1), goldd, 0.6f * u); dl->AddLine(P(-8, -6), P(-6, 1), goldd, 0.6f * u);
        dl->AddLine(P(8, -6), P(6, 1), goldd, 0.6f * u); dl->AddLine(P(8, -6), P(10, 1), goldd, 0.6f * u);
        dl->PathArcTo(P(-8, 1), 3 * u, 0, 3.14159f, 10); dl->PathFillConvex(gold);
        dl->PathArcTo(P(8, 1), 3 * u, 0, 3.14159f, 10); dl->PathFillConvex(gold);
        return true;
    case 146:                                                                          /* Tusk of a Boar: two curved tusks */
        dl->AddBezierQuadratic(P(-7, 8), P(-9, -4), P(-1, -9), bone, 3.0f * u); dl->AddBezierQuadratic(P(-7, 8), P(-9, -4), P(-1, -9), boned, 0.8f * u);
        dl->AddBezierQuadratic(P(2, 9), P(0, -2), P(8, -6), bone, 2.6f * u); dl->AddBezierQuadratic(P(2, 9), P(0, -2), P(8, -6), boned, 0.7f * u);
        dl->AddLine(P(-8, 8), P(3, 9), brown, 1.5f * u);
        return true;
    case 147:                                                                          /* Wolfskin: a grey pelt */
        dl->PathLineTo(P(-5, -9)); dl->PathLineTo(P(5, -9)); dl->PathLineTo(P(9, -3)); dl->PathLineTo(P(7, 3)); dl->PathLineTo(P(9, 9));
        dl->PathLineTo(P(-9, 9)); dl->PathLineTo(P(-7, 3)); dl->PathLineTo(P(-9, -3)); dl->PathFillConcave(col(0.55f, 0.55f, 0.56f));
        dl->AddTriangleFilled(P(-3, -9), P(3, -9), P(0, -12), col(0.55f, 0.55f, 0.56f));
        dl->AddLine(P(0, -7), P(0, 7), col(0.40f, 0.40f, 0.42f), 1.0f * u);
        return true;
    case 148:                                                                          /* Bone */
        dl->AddLine(P(-6, 6), P(6, -6), bone, 3.0f * u);
        dl->AddCircleFilled(P(-7.5f, 5), 2.2f * u, bone); dl->AddCircleFilled(P(-5, 7.5f), 2.2f * u, bone);
        dl->AddCircleFilled(P(7.5f, -5), 2.2f * u, bone); dl->AddCircleFilled(P(5, -7.5f), 2.2f * u, bone);
        return true;
    case 149:                                                                          /* Gold Cup */
        dl->PathArcTo(P(0, -5), 7 * u, 0, 3.14159f, 16); dl->PathFillConvex(gold);
        dl->AddRectFilled(P(-7, -6), P(7, -4.5f), goldd);
        dl->AddRectFilled(P(-1, 1.5f), P(1, 7), goldd); dl->AddRectFilled(P(-5, 7), P(5, 9), gold);
        dl->AddCircleFilled(P(0, -2), 1.3f * u, red);
        return true;
    }
    return false;
}
static void draw_icon(ImDrawList *dl, ImVec2 c, float s, const ItemDef *d, int id)
{
    if (((id >= 129 && id <= 149) || id == 126 || id == 128) && id != 137 && id != 138 && id != 145 && draw_special(dl, c, s, id)) return;
    if (id >= 95 && id <= 116 && (!d || d->kind == K_POTION) && ui_draw_potion(dl, c, s, id, icon_tier)) return;
    if (id >= 72 && id <= 94 && (!d || d->kind == K_INGREDIENT) && ui_draw_ingredient(dl, c, s, id)) return;
    if (d && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD || (d->kind == K_RELIC && d->type <= 33)
              || (d->type >= 98 && d->type <= 107) || (d->type >= 154 && d->type <= 156))
          && draw_equipment(dl, c, s, d->type, d->kind == K_RELIC)) return;
    ItemKind k = d ? d->kind : K_OTHER;
    ImU32 ink = col(0.22f, 0.14f, 0.08f), steel = col(0.72f, 0.74f, 0.78f), dark = col(0.35f, 0.25f, 0.15f);
    ImVec4 t = tint_for(d, id);
    ImU32 tc = ImGui::ColorConvertFloat4ToU32(t), tl = col(fminf(1, t.x + 0.25f), fminf(1, t.y + 0.25f), fminf(1, t.z + 0.25f));
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    switch (k) {
    case K_WEAPON: case K_MISSILE:
        if (k == K_MISSILE && d && (d->flags & 0x20)) {          // bows
            dl->PathArcTo(P(-2, 0), 9 * u, -1.2f, 1.2f, 16); dl->PathStroke(dark, 0, 2.2f * u);
            dl->AddLine(P(2.8f, -8.4f), P(2.8f, 8.4f), ink, 0.8f * u);
        } else {
            dl->AddQuadFilled(P(-1, 6), P(1, 6), P(1, -9), P(-1, -9), steel);
            dl->AddTriangleFilled(P(-1, -9), P(1, -9), P(0, -11.5f), steel);
            dl->AddRectFilled(P(-4.5f, 5.5f), P(4.5f, 7), dark);
            dl->AddRectFilled(P(-0.8f, 7), P(0.8f, 11), col(0.45f, 0.30f, 0.18f));
            dl->AddCircleFilled(P(0, 11.5f), 1.4f * u, col(0.75f, 0.6f, 0.2f));
        }
        break;
    case K_ARMOR:
        dl->AddQuadFilled(P(-7, -8), P(7, -8), P(6, 9), P(-6, 9), steel);
        dl->AddQuadFilled(P(-10, -8), P(-7, -8), P(-7, -2), P(-10, -3), steel);
        dl->AddQuadFilled(P(7, -8), P(10, -8), P(10, -3), P(7, -2), steel);
        for (int r = -5; r < 9; r += 3) dl->AddLine(P(-6, (float)r), P(6, (float)r), col(0.5f, 0.52f, 0.56f), 0.7f * u);
        break;
    case K_SHIELD:
        dl->PathLineTo(P(-8, -9)); dl->PathLineTo(P(8, -9)); dl->PathLineTo(P(8, 1)); dl->PathLineTo(P(0, 11)); dl->PathLineTo(P(-8, 1));
        dl->PathFillConvex(col(0.62f, 0.18f, 0.12f));
        dl->AddLine(P(0, -9), P(0, 11), col(0.9f, 0.8f, 0.3f), 1.6f * u);
        dl->AddLine(P(-8, -3), P(8, -3), col(0.9f, 0.8f, 0.3f), 1.6f * u);
        break;
    case K_INGREDIENT:
        if (is_liquid(d)) {
            dl->AddRectFilled(P(-2, -10), P(2, -6), col(0.75f, 0.7f, 0.6f));
            dl->AddCircleFilled(P(0, 3), 7.5f * u, col(0.85f, 0.9f, 0.95f, 0.5f));
            dl->AddCircleFilled(P(0, 4), 6.5f * u, tc);
            dl->AddCircle(P(0, 3), 7.5f * u, ink, 0, 1.0f * u);
        } else if (is_plant(d)) {
            dl->AddBezierQuadratic(P(-6, 9), P(-6, -6), P(5, -9), col(0.30f, 0.45f, 0.20f), 1.6f * u);
            dl->AddCircleFilled(P(-3, -1), 4 * u, tc);
            dl->AddCircleFilled(P(3, -6), 3.5f * u, tc);
            dl->AddCircleFilled(P(2, 3), 3 * u, tl);
        } else if (d && d->game_name == "Marsh Vapor") {
            dl->AddCircleFilled(P(-4, 2), 5 * u, tc); dl->AddCircleFilled(P(3, -1), 6 * u, tc); dl->AddCircleFilled(P(4, 5), 4 * u, tl);
        } else {                                                  // minerals: crystals
            dl->AddTriangleFilled(P(-8, 9), P(-3, -7), P(1, 9), tc);
            dl->AddTriangleFilled(P(-1, 9), P(4, -10), P(9, 9), tl);
            dl->AddTriangle(P(-1, 9), P(4, -10), P(9, 9), ink, 0.8f * u);
            dl->AddTriangle(P(-8, 9), P(-3, -7), P(1, 9), ink, 0.8f * u);
        }
        break;
    case K_POTION:
        dl->AddRectFilled(P(-2, -11), P(2, -5), col(0.80f, 0.75f, 0.65f));
        dl->AddRectFilled(P(-2.5f, -12), P(2.5f, -10), col(0.55f, 0.35f, 0.20f));
        dl->AddCircleFilled(P(0, 3), 8 * u, col(0.85f, 0.92f, 0.98f, 0.55f));
        dl->PathArcTo(P(0, 3), 7 * u, 0.15f, 3.0f, 20); dl->PathFillConvex(tc);
        dl->AddCircle(P(0, 3), 8 * u, ink, 0, 1.0f * u);
        dl->AddCircleFilled(P(-3, 0), 1.5f * u, col(1, 1, 1, 0.7f));
        break;
    case K_RELIC:
        dl->AddCircle(P(0, -2), 9 * u, col(0.95f, 0.8f, 0.3f), 0, 1.4f * u);
        dl->AddRectFilled(P(-1.2f, -8), P(1.2f, 10), col(0.85f, 0.65f, 0.15f));
        dl->AddRectFilled(P(-5, -4), P(5, -1.6f), col(0.85f, 0.65f, 0.15f));
        break;
    case K_HORSE: {
        /* a mount: its head in profile (the horseshoe is the Riding skill's; his ask, 1.5.1: the arc read as a
           half circle). The coat by the mount's quality, as the list sorts them (Superb 70: a dark bay, Fast 40:
           chestnut, Average 25: bay, Pack 15: dun); a mule grey, with its long ears and a short mane */
        int q = d ? d->quality : 25;
        bool mule = d && d->name.find("Mule") != std::string::npos;
        ImU32 coat = mule ? col(0.55f, 0.52f, 0.50f) : q >= 60 ? col(0.36f, 0.20f, 0.11f) : q >= 40 ? col(0.68f, 0.36f, 0.16f)
                   : q >= 25 ? col(0.52f, 0.32f, 0.18f) : col(0.66f, 0.56f, 0.40f);
        ImU32 mane = mule ? col(0.30f, 0.27f, 0.25f) : q >= 40 && q < 60 ? col(0.55f, 0.26f, 0.10f) : col(0.12f, 0.08f, 0.05f);
        ImVec2 neck[4] = { P(0.5f, -3), P(4, -7.5f), P(8.5f, 9), P(0, 9) };
        dl->AddConvexPolyFilled(neck, 4, coat);
        ImVec2 head[8] = { P(-1, -9), P(3, -8), P(4, -4), P(0, 2), P(-6, 3.2f), P(-8.6f, 1.6f), P(-8.4f, -0.6f), P(-5, -5) };
        dl->AddConvexPolyFilled(head, 8, coat);
        if (mule) { dl->AddTriangleFilled(P(0.6f, -8.6f), P(3, -8.2f), P(2.4f, -15), coat); dl->AddTriangleFilled(P(2, -8.2f), P(4, -7.4f), P(5.4f, -14), coat); }
        else dl->AddTriangleFilled(P(1.2f, -8.6f), P(3.4f, -8), P(3, -12), coat);
        dl->AddLine(P(3.6f, -8.6f), P(8.6f, 8), mane, (mule ? 1.4f : 2.4f) * u);            /* the mane down the neck */
        if (!mule) dl->AddTriangleFilled(P(-1.5f, -9.4f), P(1.4f, -8.8f), P(-0.6f, -6.8f), mane);   /* the forelock */
        dl->AddCircleFilled(P(-1.4f, -5), 0.9f * u, ink);                                       /* the eye */
        dl->AddCircleFilled(P(-7.6f, 0.4f), 0.55f * u, ink);                                    /* the nostril */
        dl->AddLine(P(-6.2f, 2.6f), P(-0.2f, -4.2f), dark, 0.7f * u);                           /* the halter */
        dl->AddLine(P(-2.6f, -6.6f), P(-0.2f, -4.2f), dark, 0.7f * u);
        dl->AddPolyline(head, 8, ink, ImDrawFlags_Closed, 0.6f * u);
        break; }
    case K_DOCUMENT: case K_QUEST:
        dl->AddRectFilled(P(-7, -9), P(7, 9), col(0.95f, 0.90f, 0.75f));
        dl->AddRect(P(-7, -9), P(7, 9), ink, 0, 0, 0.8f * u);
        for (int r = -6; r < 7; r += 3) dl->AddLine(P(-5, (float)r), P(5, (float)r), col(0.4f, 0.3f, 0.2f), 0.6f * u);
        if (k == K_DOCUMENT) dl->AddCircleFilled(P(4, 7), 2.5f * u, col(0.7f, 0.1f, 0.1f));
        break;
    case K_INSTRUMENT:
        dl->AddCircleFilled(P(-2, 4), 6 * u, col(0.62f, 0.42f, 0.22f));
        dl->AddRectFilled(P(1, -10), P(3, 2), col(0.45f, 0.30f, 0.15f));
        break;
    default:                                                      // gear, tools, other
        dl->AddRectFilled(P(-7, -5), P(7, 8), col(0.55f, 0.40f, 0.25f), 2 * u);
        dl->AddRect(P(-7, -5), P(7, 8), ink, 2 * u, 0, 0.8f * u);
        dl->AddLine(P(-7, 0), P(7, 0), ink, 0.8f * u);
        break;
    }
}

/* the long weapons (a maul, a field axe, a two-handed flail, lying diagonally) reach well past
   their square and were cut off at the edge of a list: an icon is drawn, then shrunk round its
   centre until it fits a square a third larger than its size (most icons already do: unchanged) */
static void fit_icon(ImDrawList *dl, int v0, ImVec2 c, float s)
{
    int v1 = dl->VtxBuffer.Size;
    if (v1 <= v0) return;
    float ext = 0;
    for (int i = v0; i < v1; i++) { ImVec2 p = dl->VtxBuffer[i].pos; ext = fmaxf(ext, fmaxf(fabsf(p.x - c.x), fabsf(p.y - c.y))); }
    float lim = s * 0.66f;
    if (ext <= lim) return;
    float k = lim / ext;
    for (int i = v0; i < v1; i++) { ImVec2 &p = dl->VtxBuffer[i].pos; p.x = c.x + (p.x - c.x) * k; p.y = c.y + (p.y - c.y) * k; }
}
static void draw_icon_fit(ImDrawList *dl, ImVec2 c, float s, const ItemDef *d, int id)
{
    int v0 = dl->VtxBuffer.Size;
    draw_icon(dl, c, s, d, id);
    fit_icon(dl, v0, c, s);
}
/* the icon of item id, for the other tabs */
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id) { draw_icon_fit(dl, c, s, def_of(id), id); }
/* the same with the thing's quality (a potion shows its grade), or a formula's grade (1-3) */
void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q)
{
    const ItemDef *d = def_of(id);
    icon_tier = d && q >= 0 ? ui_potion_tier(q, d->quality) : 0;
    draw_icon_fit(dl, c, s, d, id);
    icon_tier = 0;
}
void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier) { icon_tier = tier; draw_icon_fit(dl, c, s, def_of(id), id); icon_tier = 0; }
void ui_equip_icon(ImDrawList *dl, ImVec2 c, float s, int type) { int v0 = dl->VtxBuffer.Size; draw_equipment(dl, c, s, type, false); fit_icon(dl, v0, c, s); }

/* ---------------------------------------------------------------- giving items */

/* Items may change hands only where the game does not hold inventory rows on screen:
 * in a town or encounter card and on the travel map (not in battle, not on the character
 * sheet or the barter screen, whose lists refer to items by row). */
static const char *why_locked(void)
{
    switch (game_mode()) {
    case GM_CARD: case GM_MAP: return nullptr;
    case GM_BATTLE: return "Items can't change hands during a battle.";
    case GM_SCREEN: return "Close the game's current screen (sheet, shop ...) to hand items over.";
    default: return "Start or load a game first.";
    }
}

extern "C" void ui_give(int from, int to, int idx);
/* done by the game's own routines (s031: give_one, give_stack, item_ready, item_unequip) at the
 * next safe point (mods/nscreen.h ns_defer), not in the middle of drawing */
static void give_now(int from, int to, int idx, int one)
{
    u16 args[4] = { (u16)from, (u16)to, (u16)idx, 0 };
    if (one) game_call_far(GF_give_one, 4, args); else game_call_far(GF_give_stack, 4, args);
    fprintf(stderr, "ui: gave %sto %s\n", one ? "one " : "", member_name(to, false).c_str());
}
static void give(int from, int to, int idx, bool one = false)
{
    if (const char *w = why_locked()) { fprintf(stderr, "ui: %s\n", w); return; }
    int before = g_rb((u16)(G_MEMBER(to) + G_M_NITEMS));
    if (before >= 64) { fprintf(stderr, "ui: %s cannot carry any more items\n", member_name(to, false).c_str()); return; }
    ns_defer(give_now, from, to, idx, one);
}

/* what an item is good for: weapon numbers and who could use it, what armour covers, a potion's
 * effect (the character sheet's and the shops' tooltips) */
/* what a potion is, as the native screens write it (ns_potion_about, ui/nsdraw.cpp): the game's
   words for it; how it is used, a picture, and what it does; the numbers by grade as a table, a
   column a grade under the potion's picture at that grade, the one in hand (grade) marked; what
   stays the same; how much the encounters ask for it */
void ui_potion_about(int id, int grade)
{
    float wrap = ImGui::GetCurrentWindow()->DC.TextWrapPos;     /* (the width the tooltip wraps its text at: the table kept inside it) */
    float W = wrap > 0 ? wrap - ImGui::GetCursorPosX() : UIS(360);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + W);
    int p = id - 95;
    if (p >= 0 && 3 * p < (int)gamedata.formulas.size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(NS_POTION_WORDS));
        ImGui::TextWrapped("%s", gamedata.formulas[3 * p].text.c_str());
        ImGui::PopStyleColor();
    }
    PotionFx f = ns_potion_fx(id);
    const ImVec4 dimc = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
    if (f.how >= 0) {
        float is = ImGui::GetTextLineHeight() * 1.3f;
        ImVec2 c = ImGui::GetCursorScreenPos();
        ns_potion_how_icon(ImGui::GetWindowDrawList(), ImVec2(c.x, c.y + UIS(1)), is, f.how, true);
        ImGui::Dummy(ImVec2(is, is)); ImGui::SameLine(0, UIS(6));
        ImGui::PushStyleColor(ImGuiCol_Text, dimc); ui_rich_text(f.what.c_str()); ImGui::PopStyleColor();
        if (!f.keys.empty()) { ImGui::PushStyleColor(ImGuiCol_Text, dimc); ui_rich_text(f.keys.c_str()); ImGui::PopStyleColor(); }   /* the shields its rows strike */
        const float by0 = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemSpacing.y * 0.5f;   /* (the bracket from them down to their rows) */
        float by1 = -1, bx = ImGui::GetCursorScreenPos().x + UIS(5);
        if (!f.rows.empty()) {
            /* as the native screens' table (nsdraw.cpp potion_table): the rows the same at every grade first, a
               label and one number across the three columns; then the potion at each grade over its column and
               the rows that differ; how long, the hourglass */
            float hs = ImGui::GetTextLineHeight() * 1.5f, pad = ImGui::GetStyle().CellPadding.x * 2, cw = hs, lh = ImGui::GetTextLineHeight();
            bool varies = false;
            for (const FxRow &r : f.rows) if (!r.same()) { varies = true; for (int g = 0; g < 3; g++) cw = fmaxf(cw, ImGui::CalcTextSize(r.v[g].c_str()).x); }
            cw += UIS(10);
            float ind = f.keys.empty() ? 0 : UIS(15), nat = 0;
            for (const FxRow &r : f.rows) nat = fmaxf(nat, rich_size(ImGui::GetFont(), ImGui::GetFontSize(), FLT_MAX, r.label.c_str()).x + (r.time ? lh * 0.9f + UIS(3) : 0));
            float lw = fminf(nat + ind + UIS(14), fmaxf(UIS(80), W - 4 - 3 * (cw + pad) - pad));   /* (the labels on one line where they fit, the numbers right after; else what the columns leave, wrapped) */
            if (ImGui::BeginTable("##potion", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, lw);
            for (int g = 1; g <= 3; g++) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, cw);
            const ImU32 band = IM_COL32(214, 176, 100, 60);
            auto label = [&](const FxRow &r) {
                ImGui::TableSetColumnIndex(0);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ind);
                if (r.time) {
                    void layer_glyph(ImDrawList *, ImVec2, float, int, ImU32);
                    ImVec2 q = ImGui::GetCursorScreenPos();
                    layer_glyph(ImGui::GetWindowDrawList(), ImVec2(q.x + lh * 0.45f, q.y + lh * 0.52f), lh * 0.95f, 4, ImGui::GetColorU32(dimc));
                    ImGui::Dummy(ImVec2(lh * 0.9f, lh));
                    if (r.label.empty()) return;
                    ImGui::SameLine(0, UIS(3));
                }
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + lw - ind);
                ImGui::PushStyleColor(ImGuiCol_Text, dimc); ui_rich_text(r.label.c_str()); ImGui::PopStyleColor();
                ImGui::PopTextWrapPos();
            };
            for (const FxRow &r : f.rows) {                          /* the same at every grade: one number, across the three */
                if (!r.same()) continue;
                ImGui::TableNextRow();
                if (!f.keys.empty() && !r.time) {                    /* (under the shorthand: a tick from its bracket) */
                    ImGui::TableSetColumnIndex(0);
                    ImVec2 q = ImGui::GetCursorScreenPos();
                    by1 = q.y + lh * 0.55f;
                    ImGui::GetWindowDrawList()->AddLine(ImVec2(bx, by1), ImVec2(q.x + ind - UIS(4), by1), ImGui::GetColorU32(dimc), fmaxf(1.0f, UIS(1)));
                }
                label(r);
                ImGui::TableSetColumnIndex(1);
                ImVec2 q0 = ImGui::GetCursorScreenPos();
                float span = 3 * cw + 2 * pad, tw = ImGui::CalcTextSize(r.v[0].c_str()).x;
                ImGui::PushClipRect(ImVec2(q0.x - pad, q0.y), ImVec2(q0.x + span + pad, q0.y + lh), false);   /* (past the cell's own clip) */
                ImGui::GetWindowDrawList()->AddText(ImVec2(q0.x + (span - tw) * 0.5f, q0.y), ImGui::GetColorU32(ImGuiCol_Text), r.v[0].c_str());
                ImGui::PopClipRect();
                ImGui::Dummy(ImVec2(1, lh));
            }
            if (varies) {
                ImGui::TableNextRow();
                for (int g = 1; g <= 3; g++) {
                    ImGui::TableSetColumnIndex(g);
                    if (g == grade) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, band);
                    float cw2 = ImGui::GetContentRegionAvail().x;
                    ImVec2 q0 = ImGui::GetCursorScreenPos();
                    ui_item_icon_tier(ImGui::GetWindowDrawList(), ImVec2(q0.x + cw2 * 0.5f, q0.y + hs * 0.5f), hs, id, g);
                    ImGui::Dummy(ImVec2(cw2, hs));
                }
                for (const FxRow &r : f.rows) {                      /* those that differ: a column a grade */
                    if (r.same()) continue;
                    ImGui::TableNextRow();
                    label(r);
                    for (int g = 1; g <= 3; g++) {
                        ImGui::TableSetColumnIndex(g);
                        if (g == grade) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, band);
                        float cw2 = ImGui::GetContentRegionAvail().x, tw = ImGui::CalcTextSize(r.v[g - 1].c_str()).x;
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + fmaxf(0.0f, (cw2 - tw) * 0.5f));
                        if (grade == 0 || g == grade) ImGui::TextUnformatted(r.v[g - 1].c_str());
                        else ImGui::TextDisabled("%s", r.v[g - 1].c_str());
                    }
                }
            }
            ImGui::EndTable();
            if (by1 > 0) ImGui::GetWindowDrawList()->AddLine(ImVec2(bx, by0), ImVec2(bx, by1), ImGui::GetColorU32(dimc), fmaxf(1.0f, UIS(1)));
            }
        }
        if (!f.after.empty()) { ImGui::PushStyleColor(ImGuiCol_Text, dimc); ui_rich_text(f.after.c_str()); ImGui::PopStyleColor(); }
    }
    ImU32 ec; std::string enc = ns_item_encounters(id, &ec);
    if (!enc.empty()) { ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(ec)); ImGui::TextWrapped("%s", enc.c_str()); ImGui::PopStyleColor(); }
    ImGui::PopTextWrapPos();
}

/* ---- a weapon's numbers for each member (his design, 2026-11-19: a column a member under their heads, a row a
   property with its picture; the chances against a typical foe; the damage by the armor it meets). The numbers
   are the game's (mods/gamemodel.h: weapon_ready, the to-hit rolls, hit_damage, attack_rate) */
static const ImU32 FIT_GREEN = IM_COL32(46, 102, 41, 255), FIT_RED = IM_COL32(158, 51, 31, 255);
static void cell_center(float w) { float a = ImGui::GetContentRegionAvail().x; if (a > w) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (a - w) * 0.5f); }
static void cell_big(const char *t, ImU32 c)                /* a number large, centred in its cell */
{
    ImFont *f = ImGui::GetFont(); float fs = ImGui::GetFontSize() * 1.3f;
    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
    cell_center(ts.x);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddText(f, fs, p, c ? c : ImGui::GetColorU32(ImGuiCol_Text), t);
    ImGui::Dummy(ts);
}
static void cell_rich(const std::string &t, ImU32 c, float k = 1, bool centre = true)   /* rich text (shields), k: its size */
{
    ImFont *f = ImGui::GetFont(); float fs = ImGui::GetFontSize() * k;
    ImVec2 ts = rich_size(f, fs, 0, t.c_str());
    if (centre) cell_center(ts.x);
    ImVec2 p = ImGui::GetCursorScreenPos();
    rich_draw(ImGui::GetWindowDrawList(), f, fs, p, 0, c ? c : ImGui::GetColorU32(ImGuiCol_Text), t.c_str());
    ImGui::Dummy(ts);
}
static void cell_dim(const char *t)                        /* faint words wrapped in their cell */
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", t);
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();
}
static void row_label(int icon, const char *text, const char *sub = nullptr, int sub_icon = -1)
{
    ImGui::TableNextColumn();
    ui_combat_icon_inline(icon);
    if (!sub) { ImGui::TextUnformatted(text); return; }
    ImGui::BeginGroup();
    ImGui::TextUnformatted(text);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (sub_icon >= 0) ui_combat_icon_inline(sub_icon);
    ImGui::TextUnformatted(sub);
    ImGui::PopStyleColor();
    ImGui::EndGroup();
}
/* the damage by the armor it meets: a box for each part that gets through (all, half, an eighth), its tiers under it */
static void damage_strip(const GmWeapon &w)
{
    float fs = ImGui::GetFontSize(), isz = fs * 0.8f, pad = fs * 0.2f, gap = fs * 0.25f;   /* (at most 6.5 fs wide: the weapon column) */
    ImDrawList *dl = ImGui::GetWindowDrawList(); ImFont *f = ImGui::GetFont();
    ImVec2 p = ImGui::GetCursorScreenPos(); float x = p.x, hmax = 0;
    for (int g = 0; g < 3; g++) {
        int t0 = 6, t1 = -1;
        for (int t = 0; t < 6; t++) if ((g == 0 && t < w.pen) || (g == 1 && t == w.pen) || (g == 2 && t > w.pen)) { if (t < t0) t0 = t; t1 = t; }
        if (t1 < 0) continue;
        int v = gm_through(w.dmg, w.pen, t0), n = t1 - t0 + 1;
        char b[12]; snprintf(b, sizeof b, "%d", v);
        float bw = n * isz + 2 * pad, bh = fs * 1.1f + isz + pad;
        ImU32 bg = g == 0 ? IM_COL32(58, 38, 20, 255) : g == 1 ? IM_COL32(168, 138, 90, 255) : IM_COL32(230, 211, 168, 255);
        ImU32 tx = g == 0 ? IM_COL32(242, 228, 194, 255) : IM_COL32(46, 34, 22, 255);
        dl->AddRectFilled(ImVec2(x, p.y), ImVec2(x + bw, p.y + bh), bg, fs * 0.25f);
        if (g == 2) dl->AddRect(ImVec2(x, p.y), ImVec2(x + bw, p.y + bh), IM_COL32(138, 106, 47, 255), fs * 0.25f, 0, 1.0f);
        ImVec2 ts = f->CalcTextSizeA(fs * 1.05f, FLT_MAX, 0, b);
        dl->AddText(f, fs * 1.05f, ImVec2(x + (bw - ts.x) * 0.5f, p.y + pad * 0.5f), tx, b);
        for (int t = t0; t <= t1; t++) ui_armor_icon(dl, ImVec2(x + pad + (t - t0 + 0.5f) * isz, p.y + fs * 1.1f + isz * 0.5f), isz, t, tx);
        x += bw + gap; hmax = fmaxf(hmax, bh);
    }
    ImGui::Dummy(ImVec2(x - p.x - gap, hmax));
}
static void weapon_table(const GmWeapon &w)
{
    struct Mem { int m, skill, str, agl, sh_t, sh_q; };
    std::vector<Mem> ms;
    for (int m = 0; m < gm_party_size() && m < 5; m++) {
        if (!gm_member_present(m) || gm_member_status(m) == 3) continue;
        int cur, mx, agl;
        gm_member_attr(m, ATTR_STR, &cur, &mx); gm_member_attr(m, ATTR_AGL, &agl, &cur);
        u16 b = G_MEMBER(m);
        ms.push_back({ m, gm_member_skill(m, w.skill), mx, agl, (s8)g_rb((u16)(b + G_M_SHIELD)), g_rb((u16)(b + 0x47)) });
    }
    const bool shot = gm_weapon_shot(&w);
    const char *sk = gm_skill_name(w.skill);
    float fs = ImGui::GetFontSize();
    ImU32 dim = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    /* (narrow, to sit beside the row it describes: the labels as wide as they are, the weapon's 6.6 fs with its
       words wrapped, the members' as wide as the widest skill and Strength) */
    ImFont *f = ImGui::GetFont();
    auto needs_of = [&](const Mem &e, float *wa, float *wb) {
        std::string a = std::string("{") + sk + ":" + std::to_string(e.skill) + "}", b = "{Strength:" + std::to_string(e.str) + "}";
        *wa = rich_size(f, fs, 0, a.c_str()).x; *wb = rich_size(f, fs, 0, b.c_str()).x;
    };
    float mw = fs * 2.9f;
    for (const Mem &e : ms) { float wa, wb; needs_of(e, &wa, &wb); mw = fmaxf(mw, wa + wb + fs * 0.3f); }
    if (!ImGui::BeginTable("##weaponfit", 2 + (int)ms.size(), ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_NoSavedSettings)) return;
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, fs * 6.6f);
    for (size_t i = 0; i < ms.size(); i++) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, mw);
    /* the heads */
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn(); cell_dim("the weapon");
    for (const Mem &e : ms) {
        ImGui::TableNextColumn();
        float hs = fs * 1.9f; cell_center(hs / 1.2f);
        ImVec2 q = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(hs / 1.2f, hs));
        ui_member_head(ImGui::GetWindowDrawList(), e.m, q, hs);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", ui_member_name(e.m).c_str());
    }
    /* what it needs, and each one's */
    ImGui::TableNextRow();
    ImGui::TableNextColumn(); ImGui::TextUnformatted("Needs");
    ImGui::TableNextColumn();
    cell_rich(std::string("{") + sk + ":" + std::to_string(w.need) + "}  {Strength:" + std::to_string(w.str_lo) + "}", 0, 1, false);
    if (!shot && w.str_hi < 99) cell_rich("bonus from {Strength:" + std::to_string(w.str_hi + 1) + "}", FIT_GREEN, 0.85f, false);
    for (const Mem &e : ms) {
        ImGui::TableNextColumn();
        ImU32 cs = e.skill < w.need ? FIT_RED : 0, ct = e.str < w.str_lo ? FIT_RED : !shot && w.str_hi < 99 && e.str > w.str_hi ? FIT_GREEN : 0;
        std::string a = std::string("{") + sk + ":" + std::to_string(e.skill) + "}", b = "{Strength:" + std::to_string(e.str) + "}";
        float wa, wb; needs_of(e, &wa, &wb);
        cell_center(wa + wb + fs * 0.3f);
        ImVec2 q = ImGui::GetCursorScreenPos();
        rich_draw(ImGui::GetWindowDrawList(), f, fs, q, 0, cs ? cs : ImGui::GetColorU32(ImGuiCol_Text), a.c_str());
        rich_draw(ImGui::GetWindowDrawList(), f, fs, ImVec2(q.x + wa + fs * 0.3f, q.y), 0, ct ? ct : ImGui::GetColorU32(ImGuiCol_Text), b.c_str());
        ImGui::Dummy(ImVec2(wa + wb + fs * 0.3f, fs));
    }
    /* the chances: each one's, what the misfit takes or the Strength adds (and whose shield says why) */
    auto causes = [&](const Mem &e, int delta) {
        std::string c = (delta > 0 ? "+" : "") + std::to_string(delta) + " ";
        if (delta > 0) return c + "{Strength}";
        if (e.skill < w.need) c += std::string("{") + sk + "}";
        if (e.str < w.str_lo) c += "{Strength}";
        return c;
    };
    auto chance_cell = [&](const Mem &e, int now, int fit) {
        ImGui::TableNextColumn();
        char b[16]; snprintf(b, sizeof b, "%d%%", now);
        ImU32 c = now < fit ? FIT_RED : now > fit ? FIT_GREEN : 0;
        cell_big(b, c);
        if (now != fit) cell_rich(causes(e, now - fit), c, 0.8f);
    };
    ImGui::TableNextRow();
    row_label(CI_HIT, "Hit chance");
    ImGui::TableNextColumn(); cell_dim(shot ? "your shot, on a typical foe" : "your blow, on a typical foe");
    const int foe_def = shot ? GM_FOE_DEFENCE_SHOT : GM_FOE_DEFENCE;
    for (const Mem &e : ms) chance_cell(e, gm_hit_chance(gm_rating(e.skill, e.str, &w), foe_def), gm_hit_chance(e.skill, foe_def));
    if (!shot) {
        ImGui::TableNextRow();
        row_label(CI_DODGE, "Dodge chance");
        ImGui::TableNextColumn(); cell_dim(w.two_hands ? "his blow; no shield (two hands)" : "his blow, with each one's shield");
        for (const Mem &e : ms) {
            int now = 100 - gm_hit_chance(GM_FOE_RATING, gm_defence(gm_rating(e.skill, e.str, &w), e.sh_t, e.sh_q, w.two_hands, 0));
            int fit = 100 - gm_hit_chance(GM_FOE_RATING, gm_defence(e.skill, e.sh_t, e.sh_q, w.two_hands, 0));
            chance_cell(e, now, fit);
        }
    }
    /* the damage by the armor it meets; each one's Strength */
    ImGui::TableNextRow();
    { char pb[24]; snprintf(pb, sizeof pb, "penetration %d", w.pen); row_label(CI_DAMAGE, "Damage", pb, CI_PEN); }
    ImGui::TableNextColumn(); damage_strip(w);
    for (const Mem &e : ms) {
        ImGui::TableNextColumn();
        int sd = gm_str_damage(e.str, w.str_lo, w.str_hi);
        if (!sd) { cell_rich("-", dim); continue; }
        cell_rich((sd > 0 ? "+" : "") + std::to_string(sd) + " {Strength}", sd > 0 ? FIT_GREEN : FIT_RED, 1.1f);
    }
    /* the time per blow (hand to hand), each one's with their Agility; a shot's: the weapon's time to throw, draw or reload */
    ImGui::TableNextRow();
    if (!shot) {
        row_label(CI_TIME, "Time per blow");
        int lo = 999, hi = 0;                              /* (the weapon's word by the thirds of the range of all hand-to-hand weapons) */
        for (int t = 0; t <= 33; t++) { GmWeapon o; if (gm_weapon(t, &o) && !gm_weapon_shot(&o)) { lo = std::min(lo, o.time); hi = std::max(hi, o.time); } }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(hi <= lo || (w.time - lo) * 3 < hi - lo ? "quick" : (w.time - lo) * 3 < 2 * (hi - lo) ? "middling" : "slow");
        cell_dim("(lower is quicker)");
        for (const Mem &e : ms) {
            ImGui::TableNextColumn();
            char b[12]; snprintf(b, sizeof b, "%d", gm_time_per_blow(e.skill, e.agl, &w));
            cell_rich(b, 0, 1.1f);
            cell_rich("{Agility:" + std::to_string(e.agl) + "}", dim, 0.8f);
        }
    } else {
        row_label(CI_TIME, w.skill == 4 ? "Time per throw" : "Time per shot");
        ImGui::TableNextColumn(); ImGui::Text("%d", w.reload);
        for (size_t i = 0; i < ms.size(); i++) ImGui::TableNextColumn();
    }
    ImGui::EndTable();
    /* what is behind the numbers, small and faint */
    float tw = ImGui::GetItemRectSize().x;
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + fmaxf(tw, fs * 20));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 0.88f);
    ImGui::TextWrapped("Typical foe: weapon skill 40, no shield, like a seasoned bandit. In a fight, the foe you point at.");
    if (shot) ImGui::TextWrapped("A shot's chance: your aim against his shield and twice his Agility, with nobody in the way.");
    else ImGui::TextWrapped("Both chances come from your weapon skill against his. A shield adds to dodging%s", w.two_hands ? ", but two hands leave no hand for one." : "; each member's own is counted.");
    std::string dmg = "Damage depends on the armor where the blow lands: ";
    if (w.pen > 5) dmg += "all of it through any armor";
    else {
        dmg += w.pen > 0 ? std::string("all of it through anything lighter than ") + ui_armor_tier_kinds(w.pen) + ", " : std::string("");
        dmg += std::string("half through ") + ui_armor_tier_kinds(w.pen);
        if (w.pen < 5) dmg += ", an eighth through anything heavier";
    }
    dmg += " (penetration " + std::to_string(w.pen) + ").";
    if (!shot && w.str_hi < 99) dmg += " Strength above " + std::to_string(w.str_hi) + " adds 1 a blow, below " + std::to_string(w.str_lo) + " takes 1 away, and 1 more for every 5 points beyond.";
    else if (w.str_lo > 0) dmg += " Strength below " + std::to_string(w.str_lo) + " takes 1 a blow away, and 1 more for every 5 points short.";
    ImGui::TextWrapped("%s", dmg.c_str());
    if (!shot) ImGui::TextWrapped("Time per blow: the weapon's %d, plus (200 - skill - Agility) / 4.", w.time + 15);
    ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale / 0.88f);
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();
}

void ui_item_details(const ItemDef *d, int id, int q)    /* (q: the thing's quality, -1 unknown: its usual one) */
{
    if (d && d->kind == K_HORSE) { ImGui::Separator(); ImGui::TextWrapped("%s", ns_mount_words(id).c_str()); }
    if (d && d->kind == K_RELIC && d->relic_df > 0) {      /* a relic: what it takes off a prayer, and who could bear it (mods/relics.c) */
        bool weapon = relic_is_weapon(id);
        ImGui::Separator();
        if (weapon) ui_rich_textf("Wielded, it takes {Divine Favor:%d} off every prayer of the one wielding it, to any saint (none below 5). "
                                  "Wielding it asks {Virtue:%d}.", d->relic_df, d->relic_virtue);
        else ui_rich_textf("It takes {Divine Favor:%d} off every prayer of the one carrying it, to any saint (none below 5), "
                           "if they have {Virtue:%d}.", d->relic_df, d->relic_virtue);
        ui_rich_textf_dim("Of several relics, the best counts.");
        int size = (s16)g_rw(G_PARTY_SIZE); if (size < 0 || size > 5) size = 0;
        for (int m = 0; m < size; m++) {
            int v = (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + 9));
            ui_rich_textf_c(v >= d->relic_virtue ? ImVec4(0.18f, 0.40f, 0.16f, 1) : ImVec4(0.62f, 0.20f, 0.12f, 1), "%s {%sVirtue:%d}%s", rich_member(m).c_str(), v >= d->relic_virtue ? "" : "~", v,
                            v >= d->relic_virtue ? "" : weapon ? " - too little to wield it" : " - too little");
        }
        if (!relics_on()) ui_rich_textf_dim("Off: Relics as the manual has them (F12 > Settings > The party and its items).");
        else ui_rich_textf_dim("This blessed item is immune from any quality degradation.");
    }
    if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_RELIC)) {
        GmWeapon w;
        if (gm_weapon(d->type, &w)) {
            ImGui::Separator();
            ui_rich_textf("{%s}", gm_skill_name(w.skill));
            ImGui::SameLine();
            if (w.two_hands) { ui_combat_icon_inline(CI_TWO_HANDS); ImGui::TextUnformatted("two hands"); }
            else ImGui::TextUnformatted("one hand");
            weapon_table(w);
        }
    }
    if (d && d->type >= 67 && d->type <= 84) {
        /* an armor: its tier, and what a blow does through it by the weapon's penetration (s014 hit_damage, mods/gamemodel.h
           gm_through: all of it below the penetration, half at it, an eighth above); the six tiers, this one marked */
        int h = gm_armor_tier(d->type);
        float fs = ImGui::GetFontSize();
        ImGui::Separator();
        ui_armor_icon_inline(h);
        ImGui::Text("%s%s", ui_armor_tier_name(h), d->type <= 75 ? ", body and arms" : ", the legs");
        ImGui::TextUnformatted("What a blow does through it, by the weapon's penetration:");
        std::vector<std::pair<int, std::string>> grp[3];          /* 0 all of it, 1 half, 2 an eighth: (damage, name) */
        for (const ItemDef &wd : gamedata.items) {
            if (wd.type < 0 || wd.type > 33 || wd.name.empty() || (wd.kind != K_WEAPON && wd.kind != K_MISSILE)) continue;
            GmWeapon w; if (!gm_weapon(wd.type, &w)) continue;
            std::string n = game_text(wd.name.c_str(), wd.name.size());
            int g = h < w.pen ? 0 : h == w.pen ? 1 : 2;
            if (std::find_if(grp[g].begin(), grp[g].end(), [&](const std::pair<int, std::string> &e) { return e.second == n; }) == grp[g].end()) grp[g].push_back({ w.dmg, n });
        }
        for (auto &gl : grp) std::stable_sort(gl.begin(), gl.end(), [](const std::pair<int, std::string> &a, const std::pair<int, std::string> &b) { return a.first > b.first; });   /* (the hardest-hitting first) */
        if (ImGui::BeginTable("##armorpen", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, fs * 20.0f);
            for (int g = 0; g < 3; g++) {
                if (grp[g].empty()) continue;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                char lab[40];
                if (g == 0) snprintf(lab, sizeof lab, "%d+  all of it", h + 1);
                else if (g == 1) snprintf(lab, sizeof lab, "%d  half", h);
                else if (h - 1 > 1) snprintf(lab, sizeof lab, "1-%d  an eighth", h - 1);
                else snprintf(lab, sizeof lab, "1  an eighth");
                ImU32 bg = g == 0 ? IM_COL32(58, 38, 20, 255) : g == 1 ? IM_COL32(168, 138, 90, 255) : IM_COL32(230, 211, 168, 255);
                ImU32 tx = g == 0 ? IM_COL32(242, 228, 194, 255) : IM_COL32(46, 34, 22, 255);
                ImVec2 p = ImGui::GetCursorScreenPos(), ts = ImGui::CalcTextSize(lab);
                float bw = fs * 1.1f + ts.x + fs * 0.7f, bh = fs * 1.35f;
                ImDrawList *dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(p, ImVec2(p.x + bw, p.y + bh), bg, fs * 0.25f);
                if (g == 2) dl->AddRect(p, ImVec2(p.x + bw, p.y + bh), IM_COL32(138, 106, 47, 255), fs * 0.25f, 0, 1.0f);
                ui_combat_icon(dl, ImVec2(p.x + fs * 0.75f, p.y + bh * 0.5f), fs * 0.9f, CI_PEN, tx);
                dl->AddText(ImVec2(p.x + fs * 1.35f, p.y + (bh - ts.y) * 0.5f), tx, lab);
                ImGui::Dummy(ImVec2(bw, bh));
                ImGui::TableNextColumn();
                std::string l;
                for (size_t k = 0; k < grp[g].size() && k < 9; k++) l += (k ? ", " : "") + grp[g][k].second;
                if (grp[g].size() > 9) l += " and " + std::to_string(grp[g].size() - 9) + " more";
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + fs * 20.0f);
                ImGui::TextWrapped("%s", l.c_str());
                ImGui::PopTextWrapPos();
            }
            ImGui::EndTable();
        }
        ImGui::TextDisabled("The six tiers:");
        ImGui::SameLine();
        for (int t = 0; t < 6; t++) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            ui_armor_icon_inline(t);
            if (t == h) ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x - 1, p.y - 1), ImVec2(p.x + fs * 1.15f + 1, p.y + fs + 1), ImGui::GetColorU32(ImGuiCol_Text), fs * 0.15f, 0, 1.5f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s: %s", ui_armor_tier_name(t), ui_armor_tier_kinds(t));
        }
        ImGui::NewLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + fs * 27.0f);
        ImGui::TextWrapped("The tiers count 0 (cloth) to 5 (plate). A weapon gets all its damage through a tier below its penetration, half through the tier equal to it, an eighth through any higher one.");
        ui_rich_textf("A blow that pierces also wounds {Strength}; a good hit gets through more easily. Quality: every 10 points take 1 off each blow's damage.");
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }
    if (d && d->type >= 95 && d->type <= 97) {             /* a shield: what it adds to dodging, in chances (mods/gamemodel.h; the game's to-hit rolls) */
        if (q < 0) q = d->quality;
        float fs = ImGui::GetFontSize();
        auto plus = [](int t, int q, int missile) { return (gm_shield_defence(t, q, missile) + 5) * 2 / 3; };   /* (no shield: -5; 3 points are 2 in 100) */
        ImGui::Separator();
        if (ImGui::BeginTable("##shield", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            for (int k = 0; k < 2; k++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ui_combat_icon_inline(k ? CI_DODGE_SHOT : CI_DODGE); ImGui::TextUnformatted(k ? "Dodge chance, a shot or throw" : "Dodge chance, a blow");
                ImGui::TableNextColumn();
                char b[16]; snprintf(b, sizeof b, "+%d", plus(d->type, q, k));
                cell_big(b, FIT_GREEN); ImGui::SameLine(); ImGui::TextDisabled("in 100");
            }
            ImGui::EndTable();
        }
        ImGui::TextDisabled("Next to carrying no shield. One hand: not with a two-handed weapon.");
        if (ImGui::BeginTable("##shieldsizes", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
            static const char *SZ[3] = { "Small", "Medium", "Large" };
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, fs * 8.0f);
            for (int t = 0; t < 3; t++) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, fs * 4.0f);
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextDisabled("At %dq", q);
            for (int t = 95; t <= 97; t++) { ImGui::TableNextColumn(); (t == d->type ? ImGui::Text : ImGui::TextDisabled)("%s", SZ[t - 95]); }
            for (int k = 0; k < 2; k++) {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(k ? "a shot or throw" : "a blow");
                for (int t = 95; t <= 97; t++) { ImGui::TableNextColumn(); (t == d->type ? ImGui::Text : ImGui::TextDisabled)("+%d", plus(t, q, k)); }
            }
            ImGui::EndTable();
        }
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + fs * 24.0f);
        ImGui::TextWrapped("How it works: a fighter with no shield defends 5 points short. A shield takes that away and adds its own points, more with size and quality. Every 3 points are 2 in 100.");
        ImGui::TextWrapped("A flail's blow goes around a shield: against one, it counts as no shield.");
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }
    if (d && d->kind == K_POTION) ui_potion_about(id, ns_potion_grade(id, q));
}

/* a pack is always shown tidy: by kind (weapons, armour, shields, potions, ingredients ...), then
 * name, the best first. Only the order shown: the game's own rows stay as they are (the game finds
 * what a member has in hand and wears by type and quality, not by row) */
static int kind_rank(const ItemDef *d)
{
    static const int R[K_OTHER + 1] = { 0, 1, 2, 3, 6, 7, 8, 5, 4, 9, 10, 11, 12, 13 };
    return d ? R[d->kind] : 13;
}
/* the tidy order of two things (id, quality): kind, then name, then the better first (the
   character sheet's lists use it too) */
bool ui_item_tidy_less(int ida, int qa, int idb, int qb)
{
    const ItemDef *da = def_of(ida), *db = def_of(idb);
    int ka = kind_rank(da), kb = kind_rank(db);
    if (ka != kb) return ka < kb;
    const std::string &na = da ? da->name : std::string(), &nb = db ? db->name : std::string();
    if (na != nb) return na < nb;
    return qa > qb;
}
extern "C" void ui_give(int from, int to, int idx) { give(from, to < 0 ? -to - 1 : to, idx, to < 0); }
