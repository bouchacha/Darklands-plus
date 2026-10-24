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
#include "../mods/nscreen.h"
#include "skillicons.h"
#include "ui.h"
#include "gamedata.h"
#include "../mods/gamemodel.h"
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
    case K_HORSE:
        dl->PathArcTo(P(0, 1), 8 * u, -3.6f, 0.45f, 20); dl->PathStroke(steel, 0, 3 * u);
        break;
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
void ui_item_details(const ItemDef *d, int id, int q)    /* (q: the thing's quality, -1 unknown: its usual one) */
{
    if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_RELIC)) {
        /* the combat tables (far 20a5, 63 weapon types; s014 weapon_ready): skill used, blow
           time, hands and penetration, damage, skill and strength wanted */
        int w = d->type;
        int sk = rb(G_FAR20A5, (u16)(0x75ac + w)), speed = rb(G_FAR20A5, (u16)(0x7669 + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)),
            dmg = rb(G_FAR20A5, (u16)(0x76e7 + w)), minsk = rb(G_FAR20A5, (u16)(0x7726 + w)), minst = rb(G_FAR20A5, (u16)(0x7765 + w)),
            maxst = rb(G_FAR20A5, (u16)(0x77a4 + w)), reload = rb(G_FAR20A5, (u16)(0x77e3 + w));
        ImGui::Separator();
        ImGui::Text("%s, %s", gm_skill_name(sk), (cp >> 4) >= 2 ? "two hands" : "one hand");
        ImGui::Text("Damage %d, penetration %d", dmg, cp & 15);
        if (speed) ImGui::Text("Time for a blow %d", speed);
        if (reload) ImGui::Text("Time to %s %d", w >= 30 ? "reload" : w >= 27 ? "draw and loose" : "throw", reload);
        /* (the combatant's melee rating +0x20 and aim +0x21, weapon_ready: mods/gamemodel.h) */
        const bool shot = sk >= 4 && sk <= 6;                 /* (Thrown, Bows, Missile Devices: aimed) */
        if (shot) {
            ImGui::Text("Skill needed %d: each point short costs 2 aim", minsk);
            if (maxst < 99) ImGui::Text("Strength %d to %d: below that, each point short costs 1 aim and hits are softer; above %d, hits are harder", minst, maxst, maxst);
            else ImGui::Text("Strength %d: below that, each point short costs 1 aim and hits are softer", minst);
        } else {
            ImGui::Text("Skill needed %d: each point short costs 2 melee rating (attack and defense hand to hand)", minsk);
            if (maxst < 99) ImGui::Text("Strength %d to %d: below that, each point short costs 3 melee rating and blows hit softer;\nabove %d, each point adds 2 melee rating and blows hit harder", minst, maxst, maxst);
            else ImGui::Text("Strength %d: below that, each point short costs 3 melee rating and blows hit softer", minst);
        }
        ImGui::TextDisabled("Lower times are quicker.");
        /* who in the party could use it, and what it would cost them */
        ImGui::Separator();
        int size = (s16)g_rw(G_PARTY_SIZE); if (size < 0 || size > 5) size = 0;
        for (int m = 0; m < size; m++) {
            u16 b = G_MEMBER(m);
            char n[16]; g_str(G_DGROUP, (u16)(b + G_M_SHORTNAME), n, sizeof n);
            int have = sk < 7 ? (s8)g_rb((u16)(b + G_M_SKILLS + sk)) : 0, str = (s8)g_rb((u16)(b + G_M_ATTR_CUR + 1));
            int ss = minsk - have, ws = minst - str;
            std::string v;
            if (ss > 0) v += "needs " + std::to_string(ss) + " more skill";
            if (ws > 0) v += std::string(v.empty() ? "" : ", ") + "too weak by " + std::to_string(ws);
            if (v.empty()) v = str > maxst && maxst < 99 ? "suits (strong enough to hit harder)" : "suits";
            int loss = 2 * (ss > 0 ? ss : 0) + (shot ? 1 : 3) * (ws > 0 ? ws : 0);
            ImVec4 c = loss ? ImVec4(0.62f, 0.20f, 0.12f, 1) : ImVec4(0.18f, 0.40f, 0.16f, 1);
            if (loss) ImGui::TextColored(c, "%s: skill %d, Strength %d - %s (-%d %s)", game_text(n, sizeof n).c_str(), have, str, v.c_str(), loss, shot ? "aim" : "melee rating");
            else ImGui::TextColored(c, "%s: skill %d, Strength %d - %s", game_text(n, sizeof n).c_str(), have, str, v.c_str());
        }
    }
    if (d && d->type >= 67 && d->type <= 84) {
        /* the game's armour rule (s014 hit_damage / damage_queue): the armour's hardness (far
           20a5:781e by type, 0 cloth .. 5 plate) against the blow's penetration (the weapon's,
           less up to 3 on a good hit): a weapon that does not get through loses half (equal) or
           seven eighths (less) of its damage bonus, and wounds strength far less; the quality
           comes off the attacker's skill in every blow's damage (a tenth of it) */
        int h = rb(G_FAR20A5, (u16)(0x781e + d->type));
        ImGui::Separator();
        ImGui::Text(d->type <= 75 ? "Covers the vitals (body and arms)" : "Covers the legs");
        static const char *HN[6] = { "none", "soft", "stiff", "scale", "mail", "plate" };
        ImGui::Text("Hardness %d of 5 (%s)", h, h >= 0 && h < 6 ? HN[h] : "?");
        std::vector<std::string> grp[3];                          /* 0 deflected, 1 halved, 2 gets through */
        for (const ItemDef &w : gamedata.items) {
            if (w.type < 0 || w.type > 33 || w.name.empty() || (w.kind != K_WEAPON && w.kind != K_MISSILE)) continue;
            int pen = rb(G_FAR20A5, (u16)(0x76a8 + w.type)) & 15;
            grp[pen < h ? 0 : pen == h ? 1 : 2].push_back(game_text(w.name.c_str(), w.name.size()));
        }
        static const char *GL[3] = { "Deflects most of the blow (penetration below %d):", "Halves the blow (penetration %d):", "Pierced (penetration above %d):" };
        ImVec4 GC[3] = { ImVec4(0.18f, 0.40f, 0.16f, 1), ImVec4(0.6f, 0.42f, 0.08f, 1), ImVec4(0.62f, 0.20f, 0.12f, 1) };
        for (int g = 0; g < 3; g++) {
            if (grp[g].empty()) continue;
            std::string l;
            for (size_t k = 0; k < grp[g].size() && k < 6; k++) l += (k ? ", " : "") + grp[g][k];
            if (grp[g].size() > 6) l += " and " + std::to_string(grp[g].size() - 6) + " more";
            ImGui::TextColored(GC[g], GL[g], h);
            ImGui::Indent(UIS(12)); ImGui::TextWrapped("%s", l.c_str()); ImGui::Unindent(UIS(12));
        }
        ImGui::TextDisabled("A blow that pierces also wounds Strength; a good hit gets through more easily.");
        ImGui::TextDisabled("Quality: every 10 points take 1 off each blow's damage.");
    }
    if (d && d->type >= 95 && d->type <= 97) {             /* a shield: what each size adds to the defence (mods/gamemodel.h, the game's to-hit rolls) */
        ImGui::Separator();
        ImGui::TextUnformatted("Adds to its bearer's defense, more the better it is made; unusable with a two-handed weapon.");
        if (q < 0) q = d->quality;
        static const char *SZ[3] = { "Small", "Medium", "Large" };
        for (int t = 95; t <= 97; t++)
            (t == d->type ? ImGui::Text : ImGui::TextDisabled)("%s shield at %dq: %d against blows, %d against shots and throws", SZ[t - 95], q, gm_shield_defence(t, q, 0), gm_shield_defence(t, q, 1));
        ImGui::TextDisabled("No shield counts as -5, and a flail's blow passes any shield. Hand to hand, the chance to hit is 50 + 2/3 of the attacker's\nmelee rating minus (the defender's melee rating + the shield); against a shot or throw, the defense is the shield plus twice Agility (less under a load).");
    }
    if (d && d->kind == K_POTION) {
        int p = id - 95;
        ImGui::PushTextWrapPos(UIS(380));
        if (p >= 0 && 3 * p < (int)gamedata.formulas.size()) ImGui::TextWrapped("%s", gamedata.formulas[3 * p].text.c_str());
        { std::string ns_potion_effect(int id); std::string e = ns_potion_effect(id); if (!e.empty()) { ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); ImGui::TextWrapped("How it works: %s", e.c_str()); ImGui::PopStyleColor(); } }
        ImGui::PopTextWrapPos();
    }
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
