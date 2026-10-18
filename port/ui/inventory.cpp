// inventory.cpp - the graphical inventory and the alchemy overview of the Darklands+ menu.
// Items are drawn as procedural icons by kind (weapons, armour, ingredients, potions ...);
// dragging an item onto another member gives it to them through the game's own routine
// (give_stack, s031 18e7:007c), so equipment, weights and stacking stay right.
#include <stdio.h>
#include "money.h"
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include "imgui.h"
#include "../mods/nscreen.h"
#include "tablesort.h"
#include "skillicons.h"
#include "ui.h"
#include "gamedata.h"
#include "libraries.h"
extern "C" {
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
}

extern ImFont *ui_font_small;

struct InvItem { int idx, id, type, qual, count, weight; bool equipped; };

static const int MAXM = 5;
static int sel_member = 0;
static int kind_filter = -1;
static std::string last_msg;

static bool member_present(int m)
{
    if (m >= (s16)g_rw(G_PARTY_SIZE)) return false;
    char nm[4]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), nm, sizeof nm);
    return nm[0] != 0;
}
static std::string member_name(int m, bool full)
{
    char nm[32]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + (full ? G_M_FULLNAME : G_M_SHORTNAME)), nm, sizeof nm);
    return nm;
}

static std::vector<InvItem> member_items(int m)
{
    std::vector<InvItem> v;
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
    if (n > 64) n = 64;
    u16 r0 = G_MEMBER(m);
    int eq_type[5] = { (s8)g_rb((u16)(r0 + 0x3d)), (s8)g_rb((u16)(r0 + 0x0e)), (s8)g_rb((u16)(r0 + 0x37)), (s8)g_rb((u16)(r0 + 0x38)), (s8)g_rb((u16)(r0 + 0x48)) };
    int eq_q[5] = { g_rb((u16)(r0 + 0x44)), g_rb((u16)(r0 + 0x46)), g_rb((u16)(r0 + 0x3b)), g_rb((u16)(r0 + 0x3c)), g_rb((u16)(r0 + 0x47)) };
    for (int i = 0; i < n; i++) {
        u16 r = G_ITEM(m, i);
        InvItem it;
        it.idx = i; it.id = rw(G_FAR2E38, r); it.type = rb(G_FAR2E38, (u16)(r + 2)); it.qual = rb(G_FAR2E38, (u16)(r + 3));
        it.count = rb(G_FAR2E38, (u16)(r + 4)); it.weight = rb(G_FAR2E38, (u16)(r + 5));
        it.equipped = false;
        for (int k = 0; k < 5; k++) if (eq_type[k] >= 0 && eq_type[k] == it.type && eq_q[k] == it.qual) it.equipped = true;
        v.push_back(it);
    }
    return v;
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
    if (d) for (const Tint &t : TINTS) if (d->name == t.name) return ImVec4(t.r, t.g, t.b, 1);
    // stable pseudo-random hue from the id
    float h = fmodf(id * 0.61803f, 1.0f), r, g, b;
    ImGui::ColorConvertHSVtoRGB(h, 0.55f, 0.85f, r, g, b);
    return ImVec4(r, g, b, 1);
}

static bool is_liquid(const ItemDef *d)
{
    if (!d) return false;
    return d->name == "Naptha" || d->name == "Aqua Regia" || d->name.find("Base") != std::string::npos;
}
static bool is_plant(const ItemDef *d)
{
    if (!d) return false;
    return d->name == "Solanaceae" || d->name == "Mondragora" || d->name == "Camomile" || d->name == "Gum" || d->name == "East. Black Bean";
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
static void draw_icon(ImDrawList *dl, ImVec2 c, float s, const ItemDef *d, int id)
{
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
        } else if (d && d->name == "Marsh Vapor") {
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

/* the icon of item id, for the other tabs */
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id) { draw_icon(dl, c, s, def_of(id), id); }
/* the same with the thing's quality (a potion shows its grade), or a formula's grade (1-3) */
void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q)
{
    const ItemDef *d = def_of(id);
    icon_tier = d && q >= 0 ? ui_potion_tier(q, d->quality) : 0;
    draw_icon(dl, c, s, d, id);
    icon_tier = 0;
}
void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier) { icon_tier = tier; draw_icon(dl, c, s, def_of(id), id); icon_tier = 0; }
void ui_equip_icon(ImDrawList *dl, ImVec2 c, float s, int type) { draw_equipment(dl, c, s, type, false); }

/* ---------------------------------------------------------------- giving items */
struct DragItem { int from, idx; };

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

static bool equippable(const ItemDef *d, int type)
{
    if (!d) return false;
    return (d->flags & 0x0f) || (d->flags & 0x30) || (type >= 0x43 && type <= 0x61);
}

extern "C" void ui_give(int from, int to, int idx);
/* done by the game's own routines (s031: give_one, give_stack, item_ready, item_unequip) at the
 * next safe point (mods/nscreen.h ns_defer), not in the middle of drawing */
static void give_now(int from, int to, int idx, int one)
{
    u16 args[4] = { (u16)from, (u16)to, (u16)idx, 0 };
    if (one) game_call_far(GF_give_one, 4, args); else game_call_far(GF_give_stack, 4, args);
    last_msg = std::string(one ? "Gave one to " : "Gave to ") + member_name(to, false) + ".";
    fprintf(stderr, "ui: %s\n", last_msg.c_str());
}
static void give(int from, int to, int idx, bool one = false)
{
    if (const char *w = why_locked()) { last_msg = w; return; }
    int before = g_rb((u16)(G_MEMBER(to) + G_M_NITEMS));
    if (before >= 64) { last_msg = member_name(to, false) + " cannot carry any more items."; return; }
    ns_defer(give_now, from, to, idx, one);
}

static void equip_now(int m, int idx, int on, int)
{
    u16 a[3] = { (u16)m, (u16)idx, 0 };
    if (on) game_call_far(GF_item_ready, 2, a); else game_call_far(GF_item_unequip, 3, a);
    last_msg = member_name(m, false) + (on ? " takes it up." : " puts it away.");
}
static void equip(int m, int idx, bool on)
{
    if (const char *w = why_locked()) { last_msg = w; return; }
    ns_defer(equip_now, m, idx, on, 0);
}

/* ---------------------------------------------------------------- the inventory tab */
static void member_cards(void)
{
    float w = (ImGui::GetContentRegionAvail().x - 4 * ImGui::GetStyle().ItemSpacing.x) / 5;
    for (int m = 0; m < MAXM; m++) {
        if (!member_present(m)) continue;
        ImGui::PushID(m);
        bool sel = m == sel_member;
        if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.18f, 0.10f, 1));
        else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.60f, 0.42f, 1));
        int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
        u16 r0 = G_MEMBER(m);
        int load = g_rb((u16)(r0 + G_M_LOAD)), cap = (s8)g_rb((u16)(r0 + G_M_ATTR_CUR)) + (s8)g_rb((u16)(r0 + G_M_ATTR_CUR + 1));
        char label[96]; snprintf(label, sizeof label, "%s\n%d/64 kinds  load %d/%d", member_name(m, false).c_str(), n, load, cap);   /* (at most 64 kinds of things each) */
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? ImVec4(1, 0.95f, 0.85f, 1) : ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (ImGui::Button(label, ImVec2(w, UIS(58)))) sel_member = m;
        ImGui::PopStyleColor(2);
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload("DL_ITEM")) {
                DragItem di; memcpy(&di, p->Data, sizeof di);
                if (di.from != m) give(di.from, m, di.idx, ImGui::GetIO().KeyShift);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

/* what an item is good for: weapon numbers and who could use it, what armour covers, a potion's
 * effect (the inventory's tooltips, the shops' rows) */
void ui_item_details(const ItemDef *d, int id)
{
    if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_RELIC)) {
        /* the combat tables (far 20a5, 63 weapon types; s014 weapon_ready): skill used, blow
           time, hands and penetration, damage, skill and strength wanted */
        int w = d->type;
        static const char *SKILL[7] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows", "Missile Devices" };
        int sk = rb(G_FAR20A5, (u16)(0x75ac + w)), speed = rb(G_FAR20A5, (u16)(0x7669 + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)),
            dmg = rb(G_FAR20A5, (u16)(0x76e7 + w)), minsk = rb(G_FAR20A5, (u16)(0x7726 + w)), minst = rb(G_FAR20A5, (u16)(0x7765 + w)),
            maxst = rb(G_FAR20A5, (u16)(0x77a4 + w)), reload = rb(G_FAR20A5, (u16)(0x77e3 + w));
        ImGui::Separator();
        ImGui::Text("%s, %s", sk < 7 ? SKILL[sk] : "?", (cp >> 4) >= 2 ? "two hands" : "one hand");
        ImGui::Text("Damage %d, penetration %d", dmg, cp & 15);
        if (speed) ImGui::Text("Time for a blow %d", speed);
        if (reload) ImGui::Text("Time to %s %d", w >= 30 ? "reload" : w >= 27 ? "draw and loose" : "throw", reload);
        ImGui::Text("Skill wanted %d: each point short costs 2 attack and 2 defence", minsk);
        if (maxst < 99) ImGui::Text("Strength %d to %d: below, each point costs 1 attack and 3 defence and blows hit softer;\nabove %d, each point adds 2 defence and blows hit harder", minst, maxst, maxst);
        else ImGui::Text("Strength %d: below, each point costs 1 attack and 3 defence and blows hit softer", minst);
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
            if (ss > 0) v += "lacks " + std::to_string(ss) + " skill";
            if (ws > 0) v += std::string(v.empty() ? "" : ", ") + "too weak by " + std::to_string(ws);
            if (v.empty()) v = str > maxst && maxst < 99 ? "suits (strong: hits harder)" : "suits";
            int att = 2 * (ss > 0 ? ss : 0) + (ws > 0 ? ws : 0), def = 2 * (ss > 0 ? ss : 0) + 3 * (ws > 0 ? ws : 0);
            ImVec4 c = att ? ImVec4(0.62f, 0.20f, 0.12f, 1) : ImVec4(0.18f, 0.40f, 0.16f, 1);
            if (att) ImGui::TextColored(c, "%s: skill %d, strength %d - %s (-%d attack, -%d defence)", game_text(n, sizeof n).c_str(), have, str, v.c_str(), att, def);
            else ImGui::TextColored(c, "%s: skill %d, strength %d - %s", game_text(n, sizeof n).c_str(), have, str, v.c_str());
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
        ImGui::TextDisabled("A blow that pierces also wounds strength; a good hit gets through more easily.");
        ImGui::TextDisabled("Quality: every 10 points take 1 off each blow's damage.");
    }
    if (d && d->type >= 95 && d->type <= 97) { ImGui::Separator(); ImGui::Text("Helps against blows and missiles; not with a two-handed weapon."); }
    if (d && d->kind == K_POTION) {
        int p = id - 95;
        if (p >= 0 && 3 * p < (int)gamedata.formulas.size()) { ImGui::PushTextWrapPos(UIS(320)); ImGui::TextWrapped("%s", gamedata.formulas[3 * p].text.c_str()); ImGui::PopTextWrapPos(); }
    }
}

static void tooltip(const InvItem &it, const ItemDef *d)
{
    ImGui::BeginTooltip();
    ImGui::Text("%s", d ? d->name.c_str() : "?");
    ImGui::Separator();
    if (d) ImGui::TextDisabled("%s", kind_name_one(d->kind));
    ImGui::Text("%dq", it.qual);
    if (it.count > 1) ImGui::Text("%d of them", it.count);
    ImGui::Text("Weight %d", it.weight);
    if (d && d->value) ImGui::Text("Worth about %s", money_with_total(d->value).c_str());
    if (it.equipped) ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "Equipped");
    ui_item_details(d, it.id);
    ImGui::TextDisabled("Drag onto a name to give it (Shift: just one). Right-click for more.");
    ImGui::EndTooltip();
}

/* tidy a pack: the item rows sorted by kind, then name, then the best first. The game finds what
 * a member has in hand and wears by type and quality, not by row, so the order is free (as long
 * as no screen of the game shows the rows: why_locked) */
static int kind_rank(const ItemDef *d)
{
    static const int R[K_OTHER + 1] = { 0, 1, 2, 3, 6, 7, 8, 5, 4, 9, 10, 11, 12, 13 };
    return d ? R[d->kind] : 13;
}
static void sort_pack(int m)
{
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)); if (n > 64) n = 64;
    std::vector<std::array<u8, 6>> rows((size_t)n);
    for (int i = 0; i < n; i++) for (int k = 0; k < 6; k++) rows[(size_t)i][(size_t)k] = rb(G_FAR2E38, (u16)(G_ITEM(m, i) + k));
    std::stable_sort(rows.begin(), rows.end(), [](const std::array<u8, 6> &a, const std::array<u8, 6> &b) {
        int ia = (a[0] | a[1] << 8) & 0xfff, ib = (b[0] | b[1] << 8) & 0xfff;
        const ItemDef *da = def_of(ia), *db = def_of(ib);
        int ka = kind_rank(da), kb = kind_rank(db);
        if (ka != kb) return ka < kb;
        std::string na = da ? da->name : "", nb = db ? db->name : "";
        if (na != nb) return na < nb;
        return a[3] > b[3];
    });
    for (int i = 0; i < n; i++) for (int k = 0; k < 6; k++) wb(G_FAR2E38, (u16)(G_ITEM(m, i) + k), rows[(size_t)i][(size_t)k]);
}

extern "C" int sheetview_shared(void);
void ui_hint(const char *fmt, ...);
extern "C" void tab_inventory(void)
{
    if (!gamedata.loaded) { ImGui::Text("The item list (DARKLAND.LST) could not be read."); return; }
    if (!member_present(sel_member)) for (int m = 0; m < MAXM; m++) if (member_present(m)) { sel_member = m; break; }
    {   /* with one pack the character sheet (F1) shows the party's things together; here is who holds what */
        if (sheetview_shared()) ui_hint("The game itself keeps each thing with one member. With one pack for the party, the character sheet (F1) shows them together; this page shows who holds what, as the game does.");
    }
    member_cards();
    // kind filter chips
    std::vector<InvItem> items = member_items(sel_member);
    bool present[K_OTHER + 1] = {};
    for (auto &it : items) { const ItemDef *d = def_of(it.id); present[d ? d->kind : K_OTHER] = true; }
    if (ImGui::RadioButton("Everything", kind_filter < 0)) kind_filter = -1;
    for (int k = 0; k <= K_OTHER; k++) if (present[k]) { ImGui::SameLine(); if (ImGui::RadioButton(kind_name((ItemKind)k), kind_filter == k)) kind_filter = k; }
    int sort_now = -1;
    {   /* tidy the pack(s), at the right of the row */
        float w = ImGui::CalcTextSize("Tidy this pack").x + ImGui::CalcTextSize("Tidy all packs").x + UIS(40);
        ImGui::SameLine(ImGui::GetContentRegionMax().x - w);
        ImGui::BeginDisabled(why_locked() != nullptr);
        if (ImGui::SmallButton("Tidy this pack")) sort_now = sel_member;
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Sort the items by kind (weapons, armour, shields, potions, ingredients ...), then name, the best first.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Tidy all packs")) sort_now = 99;
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Sort every member's items the same way.");
        ImGui::EndDisabled();
    }
    ImGui::BeginChild("grid", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 4), ImGuiChildFlags_Border);
    const float tile = UIS(104), pad = UIS(8);
    int cols = (int)((ImGui::GetContentRegionAvail().x + pad) / (tile + pad)); if (cols < 1) cols = 1;
    int col_i = 0;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    for (auto &it : items) {
        const ItemDef *d = def_of(it.id);
        if (kind_filter >= 0 && (d ? d->kind : K_OTHER) != kind_filter) continue;
        if (col_i) ImGui::SameLine(0, pad);
        ImGui::PushID(it.idx);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("tile", ImVec2(tile, tile));
        bool hov = ImGui::IsItemHovered();
        ImU32 bg = it.equipped ? col(0.87f, 0.76f, 0.55f) : col(0.96f, 0.92f, 0.82f);
        dl->AddRectFilled(p, ImVec2(p.x + tile, p.y + tile), bg, 6);
        if (it.equipped) dl->AddRect(ImVec2(p.x + 3, p.y + 3), ImVec2(p.x + tile - 3, p.y + tile - 3), col(0.72f, 0.55f, 0.18f), 4, 0, 2.0f);
        dl->AddRect(p, ImVec2(p.x + tile, p.y + tile), hov ? col(0.55f, 0.24f, 0.13f) : col(0.55f, 0.42f, 0.25f), 6, 0, hov ? 2.5f : 1.0f);
        icon_tier = d ? ui_potion_tier(it.qual, d->quality) : 0; draw_icon(dl, ImVec2(p.x + tile / 2, p.y + UIS(40)), UIS(46), d, it.id); icon_tier = 0;
        const char *nm = d ? (ImGui::CalcTextSize(d->name.c_str()).x * 0.8f < tile - 6 ? d->name.c_str() : d->shortname.c_str()) : "?";
        ImGui::PushFont(ui_font_small);
        ImVec2 ts = ImGui::CalcTextSize(nm);
        dl->AddText(ImVec2(p.x + (tile - ts.x) / 2, p.y + tile - ts.y - 4), col(0.2f, 0.12f, 0.06f), nm);
        if (!d || d->kind != K_POTION) {                           /* (a potion: the dots on its flask) */
            char q[16]; snprintf(q, sizeof q, "%dq", it.qual);
            dl->AddText(ImVec2(p.x + 5, p.y + 3), col(0.45f, 0.35f, 0.25f), q);
        }
        if (it.count > 1) {
            char c[16]; snprintf(c, sizeof c, "x%d", it.count);
            ImVec2 cs = ImGui::CalcTextSize(c);
            dl->AddRectFilled(ImVec2(p.x + tile - cs.x - 10, p.y + 3), ImVec2(p.x + tile - 3, p.y + cs.y + 5), col(0.55f, 0.24f, 0.13f), 4);
            dl->AddText(ImVec2(p.x + tile - cs.x - 6, p.y + 4), col(1, 0.95f, 0.85f), c);
        }
        if (it.equipped) {
            const char *e = (d && (d->kind == K_ARMOR)) ? "worn" : "in hand";
            ImVec2 es = ImGui::CalcTextSize(e);
            dl->AddText(ImVec2(p.x + (tile - es.x) / 2, p.y + tile - 2 * es.y - 4), col(0.45f, 0.18f, 0.10f), e);
        }
        ImGui::PopFont();
        if (hov && !ImGui::IsMouseDragging(0) && !ImGui::IsPopupOpen("item_ctx")) tooltip(it, d);
        if (ImGui::BeginPopupContextItem("item_ctx")) {
            ImGui::TextDisabled("%s", d ? d->name.c_str() : "?");
            for (int m = 0; m < MAXM; m++) {
                if (m == sel_member || !member_present(m)) continue;
                if (ImGui::MenuItem(("Give to " + member_name(m, false)).c_str())) give(sel_member, m, it.idx);
                if (it.count > 1 && ImGui::MenuItem(("Give one to " + member_name(m, false)).c_str())) give(sel_member, m, it.idx, true);
            }
            if (equippable(d, it.type)) {
                ImGui::Separator();
                if (!it.equipped && ImGui::MenuItem((d && d->kind == K_ARMOR) ? "Wear" : "Take in hand")) equip(sel_member, it.idx, true);
                if (it.equipped && ImGui::MenuItem((d && d->kind == K_ARMOR) ? "Take off" : "Put away")) equip(sel_member, it.idx, false);
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginDragDropSource()) {
            DragItem di = { sel_member, it.idx };
            ImGui::SetDragDropPayload("DL_ITEM", &di, sizeof di);
            ImVec2 cp = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(UIS(52), UIS(52)));
            draw_icon(ImGui::GetWindowDrawList(), ImVec2(cp.x + UIS(26), cp.y + UIS(26)), UIS(44), d, it.id);
            ImGui::Text("%s", d ? d->name.c_str() : "?");
            ImGui::EndDragDropSource();
        }
        ImGui::PopID();
        col_i = (col_i + 1) % cols;
    }
    if (items.empty()) ImGui::TextDisabled("Carries nothing.");
    ImGui::EndChild();
    if (sort_now >= 0 && !why_locked()) {
        for (int m = 0; m < MAXM; m++) if ((sort_now == 99 || m == sort_now) && member_present(m)) sort_pack(m);
        last_msg = sort_now == 99 ? "Everyone's pack is in order." : member_name(sort_now, false) + "'s pack is in order.";
    }
    static std::string shown; static double shown_t;
    if (last_msg != shown) { shown = last_msg; shown_t = ImGui::GetTime(); }
    else if (!last_msg.empty() && ImGui::GetTime() - shown_t > 8) last_msg.clear();
    const char *lock = why_locked();
    ImGui::TextDisabled("%s", !last_msg.empty() ? last_msg.c_str() : lock ? lock
                        : "Drag an item onto a name to hand it over (hold Shift for just one); right-click for more.");
}

/* ---------------------------------------------------------------- the alchemy tab */
static int party_stock(int item, int *by_member)
{
    int total = 0;
    for (int m = 0; m < MAXM; m++) {
        if (by_member) by_member[m] = 0;
        if (!member_present(m)) continue;
        for (auto &it : member_items(m)) if (it.id == item) { total += it.count; if (by_member) by_member[m] += it.count; }
    }
    return total;
}

static bool knows(int m, int f)
{
    u16 o = (u16)(0x07be + m * 22 + f / 3);
    return (rb(G_FAR2E38, o) >> (f % 3)) & 1;
}

static std::string virtue_name(int f)
{
    u16 o = rw(G_FAR2E38, (u16)(0x1736 + f * 4)), s = rw(G_FAR2E38, (u16)(0x1736 + f * 4 + 2));
    char b[48]; if (!s) return "";
    g_str(s, o, b, sizeof b);
    return b;
}

extern "C" void tab_alchemy(void)
{
    if (!gamedata.loaded || gamedata.formulas.empty()) { ImGui::Text("The formula list (DARKLAND.ALC) could not be read."); return; }
    ImGui::BeginChild("alc", ImVec2(0, 0));
    // ingredient shelf
    ImGui::SeparatorText("The party's ingredients");
    ImDrawList *dl = ImGui::GetWindowDrawList();
    /* those carried, as tiles; the others folded away (their signs, for reading the formulae) */
    static bool show_missing = false;
    int carried = 0; for (int id = 72; id <= 94; id++) { int byM[MAXM]; if (party_stock(id, byM)) carried++; }
    if (!carried) ImGui::TextDisabled("The party carries none. Alchemists and pharmacists sell them (F12 > Journal > Ingredients: where).");
    int shown = 0;
    for (int pass = 0; pass < 2; pass++) {
    if (pass == 1) {
        if (carried == 23) break;
        char lb[64]; snprintf(lb, sizeof lb, "%s the %d not carried", show_missing ? "Hide" : "Show", 23 - carried);
        if (shown) ImGui::NewLine();
        if (ImGui::SmallButton(lb)) show_missing = !show_missing;
        if (!show_missing) break;
        shown = 0;
    }
    for (int id = 72; id <= 94; id++) {
        int byM[MAXM]; int n = party_stock(id, byM);
        if ((n > 0) != (pass == 0)) continue;
        const ItemDef *d = def_of(id);
        if (shown % 8) ImGui::SameLine();
        ImGui::PushID(id);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("ing", ImVec2(UIS(88), UIS(92)));
        dl->AddRectFilled(p, ImVec2(p.x + UIS(88), p.y + UIS(92)), n ? col(0.96f, 0.92f, 0.82f) : col(0.88f, 0.83f, 0.72f, 0.6f), 6);
        dl->AddRect(p, ImVec2(p.x + UIS(88), p.y + UIS(92)), col(0.55f, 0.42f, 0.25f), 6);
        draw_icon(dl, ImVec2(p.x + UIS(44), p.y + UIS(34)), UIS(40), d, id);
        ImGui::PushFont(ui_font_small);
        const char *sn = d ? (ImGui::CalcTextSize(d->name.c_str()).x <= UIS(84) ? d->name.c_str() : d->shortname.c_str()) : "?";   /* (the whole name where it fits) */
        ImVec2 ts = ImGui::CalcTextSize(sn);
        dl->AddText(ImVec2(p.x + (UIS(88) - ts.x) / 2, p.y + UIS(92) - ts.y - 3), n ? col(0.2f, 0.12f, 0.06f) : col(0.5f, 0.42f, 0.32f), sn);
        char c[12]; snprintf(c, sizeof c, "%d", n);
        dl->AddText(ImVec2(p.x + 5, p.y + 3), n ? col(0.55f, 0.24f, 0.13f) : col(0.6f, 0.5f, 0.4f), c);
        ImGui::PopFont();
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s: %d", d ? d->name.c_str() : "?", n);
            for (int m = 0; m < MAXM; m++) if (byM[m]) ImGui::Text("  %s carries %d", member_name(m, false).c_str(), byM[m]);
            ImGui::EndTooltip();
        }
        ImGui::PopID();
        shown++;
    }
    }
    // formulas
    static bool all_formulas = false;
    ImGui::SeparatorText(all_formulas ? "All formulae" : "Formulae the party knows");
    ImGui::Checkbox("Show all formulae (to judge one before buying it, and see where it is sold)", &all_formulas);
    if (ImGui::BeginTable("forms", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti)) {
        ImGui::TableSetupColumn("Potion", ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn(all_formulas ? "Known by, or sold in" : "Known by", ImGuiTableColumnFlags_WidthStretch, all_formulas ? 0.95f : 0.6f);
        ImGui::TableSetupColumn("Difficulty", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Difficulty").x + UIS(28));   /* (room for the sort arrow) */
        ImGui::TableSetupColumn("Risk", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("medium").x + UIS(28));
        ImGui::TableSetupColumn("Ingredients (the party has)", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 1.2f);
        ImGui::TableSetupColumn("Batches", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Batches").x + UIS(28));
        ImGui::TableHeadersRow();
        int listed = 0;
        auto who_of = [&](int f) { std::string who; for (int m = 0; m < MAXM; m++) if (member_present(m) && knows(m, f)) { if (!who.empty()) who += ", "; who += member_name(m, false); } return who; };
        auto batches_of = [&](int f) { const Formula &F = gamedata.formulas[f]; int b = 999;
            for (size_t k = 0; k < F.ingr.size(); k++) { int have = party_stock(F.ingr[k].item, nullptr), x = F.ingr[k].count ? have / F.ingr[k].count : 999; if (x < b) b = x; }
            return b == 999 ? 0 : b; };
        std::vector<int> rows;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) if (all_formulas || !who_of(f).empty()) rows.push_back(f);
        static TableSort ts;
        auto key = [&](int f, int c) -> SortKey {
            const Formula &F = gamedata.formulas[f];
            switch (c) {
            case 0: { std::string vn = virtue_name(f); const ItemDef *pd = def_of(95 + f / 3); return vn.empty() ? (pd ? game_text(pd->name.c_str(), 64) : std::string("?")) : vn; }
            case 1: return who_of(f);
            case 2: return F.mystic;
            case 3: return F.risk;
            case 5: return batches_of(f);
            default: return f;
            }
        };
        for (int f : table_order(ts, rows, key)) {
            std::string who = who_of(f);
            listed++;
            const Formula &F = gamedata.formulas[f];
            const ItemDef *pd = def_of(95 + f / 3);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(UIS(28), UIS(28)));
            icon_tier = f % 3 + 1; draw_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + UIS(14), p.y + UIS(14)), UIS(26), pd, 95 + f / 3); icon_tier = 0;
            ImGui::SameLine();
            ImGui::BeginGroup();
            std::string vn = virtue_name(f), pn = pd ? game_text(pd->name.c_str(), 64) : std::string("?");
            ImGui::TextWrapped("%s", vn.empty() ? pn.c_str() : vn.c_str());
            if (!vn.empty() && vn != pn) {                       /* the potion it makes (three formulas each) */
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                ImGui::TextWrapped("%s", pn.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::EndGroup();
            if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::PushTextWrapPos(UIS(360)); ImGui::TextWrapped("%s", F.text.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
            ImGui::TableNextColumn();
            if (!who.empty()) ImGui::TextWrapped("%s", who.c_str());
            else {                                               /* nobody knows it: where an alchemist sells it (ui/libraries.h) */
                std::vector<SellerHit> sh = game_mode() != GM_OTHER ? formula_sellers(f) : std::vector<SellerHit>();
                if (sh.empty()) ImGui::TextDisabled(f < 22 ? "no alchemist sells it\nin this game" : "not sold: trade for it\nor find it");
                else {
                    int d = (int)(sh[0].days + 0.5);
                    bool here = sh[0].days >= 0 && sh[0].days < 0.6;
                    ImGui::BeginGroup();
                    if (here) ImGui::TextUnformatted("sold here");
                    else ImGui::TextWrapped("sold in %s", sh[0].name.c_str());
                    if (here || sh[0].days < 0) ImGui::TextDisabled("%d fl", sh[0].price_fl);
                    else ImGui::TextDisabled("%d day%s away, %d fl", d, d == 1 ? "" : "s", sh[0].price_fl);
                    ImGui::EndGroup();
                    if (ImGui::IsItemHovered()) {
                        std::string t = sh.size() == 1 ? "Only the alchemist of " + sh[0].name + " sells it." : "The alchemists who sell it, the nearest first: ";
                        if (sh.size() > 1) for (size_t i = 0; i < sh.size(); i++) t += (i ? ", " : "") + sh[i].name;
                        if (sh.size() > 1) t += ".";
                        t += "\n\nEach town's alchemist sells four formulae, the same all game. Only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one of Thunderbolt are ever sold; the others are traded for or found.";
                        ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
                    }
                }
            }
            ImGui::TableNextColumn(); ImGui::Text("%d", F.mystic);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The formula's mystical number: the higher, the harder to brew (it is set against the alchemist's skill).");
            ImGui::TableNextColumn();
            { static const char *RISK[3] = { "low", "medium", "high" }; int r = F.risk >= 0 && F.risk < 3 ? F.risk : 0;
              ImGui::TextColored(r == 0 ? ImVec4(0.2f, 0.45f, 0.15f, 1) : r == 1 ? ImVec4(0.6f, 0.42f, 0.08f, 1) : ImVec4(0.65f, 0.15f, 0.10f, 1), "%s", RISK[r]);
              if (ImGui::IsItemHovered()) ImGui::SetTooltip("How badly a failed brew can go wrong."); }
            ImGui::TableNextColumn();
            int batches = 999;
            for (size_t k = 0; k < F.ingr.size(); k++) {           /* one ingredient a line */
                int have = party_stock(F.ingr[k].item, nullptr);
                const ItemDef *d = def_of(F.ingr[k].item);
                ImVec2 q = ImGui::GetCursorScreenPos();
                ImGui::Dummy(ImVec2(UIS(22), UIS(22)));
                draw_icon(ImGui::GetWindowDrawList(), ImVec2(q.x + UIS(11), q.y + UIS(11)), UIS(20), d, F.ingr[k].item);
                ImGui::SameLine(0, UIS(4));
                ImGui::TextColored(have >= F.ingr[k].count ? ImVec4(0.2f, 0.12f, 0.06f, 1) : ImVec4(0.65f, 0.15f, 0.10f, 1), "%d %s", F.ingr[k].count, d ? d->name.c_str() : "?");
                ImGui::SameLine(); ImGui::TextDisabled("(%d)", have);
                int b = F.ingr[k].count ? have / F.ingr[k].count : 999;
                if (b < batches) batches = b;
            }
            ImGui::TableNextColumn();
            if (batches == 999) batches = 0;
            ImGui::TextColored(batches ? ImVec4(0.2f, 0.45f, 0.15f, 1) : ImVec4(0.6f, 0.5f, 0.4f, 1), "%d", batches);
        }
        ImGui::EndTable();
        if (!listed) ImGui::TextDisabled("Nobody in the party knows a formula.");
    }
    ImGui::EndChild();
}

extern "C" void ui_give(int from, int to, int idx) { give(from, to < 0 ? -to - 1 : to, idx, to < 0); }
