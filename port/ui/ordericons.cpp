// ordericons.cpp - the pictures on the battle's order buttons (ui/margins.cpp: the game's Orders and
// Attack menus as buttons): a sword for Std Attack, a shield for Parry, footprints for Walk towards,
// a white flag for Surrender ... Drawn with ImGui's shapes (pen.h) in the manner of the town and quest
// pictures - bold shapes with an ink outline - but without their parchment roundel: light shapes
// that read on the dark buttons at the size of a line of text.
#include "imgui.h"
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "pen.h"
#include "ordericons.h"

using pen::C;
using pen::Pen;

namespace {
const ImU32 INK = C(0.10f, 0.06f, 0.03f), PARCH = C(0.96f, 0.91f, 0.78f), SKIN = C(0.93f, 0.78f, 0.62f);
const ImU32 STEEL = C(0.80f, 0.82f, 0.86f), IRON = C(0.46f, 0.47f, 0.51f), GOLD = C(0.95f, 0.74f, 0.22f);
const ImU32 RED = C(0.82f, 0.18f, 0.13f), WOOD = C(0.62f, 0.40f, 0.20f), STONE = C(0.78f, 0.74f, 0.68f);
const ImU32 GREEN = C(0.40f, 0.78f, 0.30f), BLUE = C(0.34f, 0.52f, 0.80f), WHITE = C(0.98f, 0.97f, 0.93f);

/* a bar from (cx, cy) along (dx, dy): lb back, lf forward, half-widths wb and wf, an outline o */
void bar(const Pen &p, float cx, float cy, float dx, float dy, float lb, float lf, float wb, float wf, ImU32 k, float o = 0)
{
    float l = sqrtf(dx * dx + dy * dy); dx /= l; dy /= l;
    float nx = -dy, ny = dx;
    auto q = [&](float b, float f, float w0, float w1, ImU32 col) {
        p.quad(cx - dx * b + nx * w0, cy - dy * b + ny * w0, cx + dx * f + nx * w1, cy + dy * f + ny * w1,
               cx + dx * f - nx * w1, cy + dy * f - ny * w1, cx - dx * b - nx * w0, cy - dy * b - ny * w0, col);
    };
    if (o > 0) q(lb + o, lf + o, wb + o, wf + o * 0.6f, INK);
    q(lb, lf, wb, wf, k);
}
/* a thick stroke with an ink edge under it */
void inked(const Pen &p, float x0, float y0, float x1, float y1, ImU32 k, float w, float o)
{
    p.line(x0, y0, x1, y1, INK, w + 2 * o); p.line(x0, y0, x1, y1, k, w);
}
/* a sword from its pommel (px, py) towards (dx, dy), length l */
void sword(const Pen &p, float px, float py, float dx, float dy, float l, float o)
{
    float n = sqrtf(dx * dx + dy * dy); dx /= n; dy /= n;
    float gx = px + dx * l * 0.30f, gy = py + dy * l * 0.30f;                  // the guard
    bar(p, gx, gy, dx, dy, 0, l * 0.70f, 1.9f, 0.25f, STEEL, o);               // the blade
    bar(p, gx, gy, -dy, dx, 5.6f, 5.6f, 0.95f, 0.95f, GOLD, o);                // the cross-guard
    bar(p, gx, gy, -dx, -dy, 0, l * 0.24f, 1.1f, 1.1f, C(0.32f, 0.19f, 0.10f), o);   // the grip
    p.disc(px, py, 1.7f + o, INK); p.disc(px, py, 1.7f, GOLD);                // the pommel
}
/* a heater shield, centred on (x, y), r its half-width */
void shield(const Pen &p, float x, float y, float r, ImU32 face, float o)
{
    auto sh = [&](float k, ImU32 col) {
        p.poly({ x - r * k, y - r * 1.0f * k, x + r * k, y - r * 1.0f * k, x + r * k, y + r * 0.15f * k, x + r * 0.55f * k, y + r * 0.85f * k,
                 x, y + r * 1.15f * k, x - r * 0.55f * k, y + r * 0.85f * k, x - r * k, y + r * 0.15f * k }, col);
    };
    sh(1.0f + o / r * 1.4f, INK); sh(1.0f, face);
}
/* a footprint (a shoe's sole and heel), its toe towards angle a (radians, 0 up) */
void footprint(const Pen &p, float x, float y, float a, ImU32 k, float o)
{
    float s = sinf(a), c = cosf(a);
    float tx = x + s * 2.6f, ty = y - c * 2.6f, hx = x - s * 4.6f, hy = y + c * 4.6f;
    p.ellipse(tx, ty, 3.3f + o, 4.6f + o, INK, a); p.ellipse(hx, hy, 2.6f + o, 2.6f + o, INK, a);
    p.ellipse(tx, ty, 3.3f, 4.6f, k, a); p.ellipse(hx, hy, 2.6f, 2.6f, k, a);
}
/* a pawn-like figure (a head and shoulders) */
void figure(const Pen &p, float x, float y, ImU32 k, float o)
{
    p.dl->PathArcTo(p.P(x, y + 5.6f), (4.6f + o) * p.u, 3.1416f, 6.2832f, 14); p.dl->PathFillConvex(INK);
    p.dl->PathArcTo(p.P(x, y + 5.6f), 4.6f * p.u, 3.1416f, 6.2832f, 14); p.dl->PathFillConvex(k);
    p.disc(x, y - 1.2f, 2.8f + o, INK); p.disc(x, y - 1.2f, 2.8f, k);
}
/* a padlock: its shackle and body */
void padlock(const Pen &p, float x, float y, float o)
{
    p.arc(x, y - 3.0f, 5.0f, 3.1416f, 6.2832f, INK, 2.6f + 2 * o); p.arc(x, y - 3.0f, 5.0f, 3.1416f, 6.2832f, STEEL, 2.6f);
    p.line(x - 5.0f, y - 3.0f, x - 5.0f, y, INK, 2.6f + 2 * o); p.line(x + 5.0f, y - 3.0f, x + 5.0f, y, INK, 2.6f + 2 * o);
    p.line(x - 5.0f, y - 3.0f, x - 5.0f, y, STEEL, 2.6f); p.line(x + 5.0f, y - 3.0f, x + 5.0f, y, STEEL, 2.6f);
    p.rect(x - 7.6f - o, y - o, x + 7.6f + o, y + 10.0f + o, INK, 1.4f); p.rect(x - 7.6f, y, x + 7.6f, y + 10.0f, GOLD, 1.2f);
    p.disc(x, y + 4.0f, 1.6f, INK); p.tri(x - 1.0f, y + 4.2f, x + 1.0f, y + 4.2f, x, y + 7.6f, INK);   // the keyhole
}
} // namespace

void ui_draw_order_icon(ImDrawList *dl, ImVec2 c, float s, int icon)
{
    Pen p{ dl, c, s / 32.0f };
    p.mw = 1.0f;
    const float o = fmaxf(0.9f, 1.1f / p.u);
    switch (icon) {
    case OI_RESUME:                                                         /* play */
        p.tri(-7.4f - o, -11.0f - o * 1.6f, -7.4f - o, 11.0f + o * 1.6f, 12.0f + o * 1.8f, 0, INK);
        p.tri(-7.4f, -11.0f, -7.4f, 11.0f, 12.0f, 0, GOLD);
        break;
    case OI_DONE:                                                           /* a tick */
        p.poly_line({ -11.0f, 0.5f, -3.4f, 8.4f, 11.6f, -9.0f }, INK, 4.6f + 2 * o, false);
        p.poly_line({ -11.0f, 0.5f, -3.4f, 8.4f, 11.6f, -9.0f }, GREEN, 4.6f, false);
        break;
    case OI_INFO: {                                                         /* an eye */
        auto almond = [&](float k, ImU32 col) {
            p.dl->PathArcTo(p.P(0, 9.0f * k), 15.0f * k * p.u, 3.1416f * 1.22f, 3.1416f * 1.78f, 16);
            p.dl->PathArcTo(p.P(0, -9.0f * k), 15.0f * k * p.u, 3.1416f * 0.22f, 3.1416f * 0.78f, 16);
            p.dl->PathFillConvex(col);
        };
        almond(1.0f + o / 7.0f, INK); almond(1.0f, WHITE);
        p.disc(0, 0, 4.6f, C(0.42f, 0.30f, 0.16f)); p.disc(0, 0, 2.2f, INK); p.disc(-1.4f, -1.4f, 1.0f, WHITE);
        break; }
    case OI_WALK:                                                           /* footprints */
        footprint(p, -4.4f, 4.0f, -0.18f, PARCH, o);
        footprint(p, 5.0f, -4.6f, 0.18f, PARCH, o);
        break;
    case OI_FLEE:                                                           /* footprints in a hurry */
        footprint(p, 1.0f, 4.6f, 0.95f, PARCH, o);
        footprint(p, 8.0f, -5.0f, 1.2f, PARCH, o);
        for (int k = 0; k < 3; k++) inked(p, -14.0f + k * 1.4f, -6.0f + k * 5.0f, -6.4f + k * 1.4f, -6.0f + k * 5.0f, RED, 1.5f, o * 0.7f);
        break;
    case OI_HALT: {                                                         /* a raised hand */
        auto hand = [&](float e, ImU32 k) {
            p.rect(-6.8f - e, -2.4f - e, 6.8f + e, 10.6f + e, k, 3.4f);       // the palm
            static const float fx[4] = { -5.1f, -1.7f, 1.7f, 5.1f }, ft[4] = { -10.4f, -13.4f, -12.6f, -9.4f };
            for (int f = 0; f < 4; f++) p.rect(fx[f] - 1.55f - e, ft[f] - e, fx[f] + 1.55f + e, 1.0f, k, 1.5f + e);
            bar(p, -6.0f, 5.0f, -0.75f, -1.0f, 0, 8.6f, 1.9f + e, 1.6f + e, k);   // the thumb
        };
        hand(o, INK); hand(0, SKIN);
        for (int f = 1; f < 4; f++) p.line(-6.8f + f * 3.4f, -0.6f, -6.8f + f * 3.4f, 1.6f, C(0.70f, 0.52f, 0.38f), 0.6f);
        break; }
    case OI_GROUP:                                                          /* three together */
        figure(p, -6.4f, 1.4f, PARCH, o); figure(p, 6.4f, 1.4f, PARCH, o); figure(p, 0, -4.4f, PARCH, o);
        break;
    case OI_FILE:                                                           /* three one behind the other */
        figure(p, -9.0f, 4.0f, STONE, o); figure(p, 0, 0, STONE, o); figure(p, 9.0f, -4.0f, PARCH, o);
        break;
    case OI_DOOR: {                                                         /* an arched door, ajar */
        p.rect(-9.0f - o, -4.0f, 9.0f + o, 13.0f + o, INK); p.disc(0, -4.0f, 9.0f + o, INK);
        p.rect(-9.0f, -4.0f, 9.0f, 13.0f, C(0.12f, 0.09f, 0.07f)); p.disc(0, -4.0f, 9.0f, C(0.12f, 0.09f, 0.07f));   // the dark within
        p.poly({ -8.0f, -6.6f, -1.0f, -11.6f, 3.0f, -10.0f, 3.0f, 13.0f, -8.0f, 13.0f }, WOOD);   // the leaf, open towards us
        for (int k = 0; k < 3; k++) p.line(-4.6f + k * 3.0f, -9.6f + k * 0.8f, -4.6f + k * 3.0f, 12.6f, C(0.40f, 0.24f, 0.10f), 0.6f);
        p.line(-8.0f, -2.0f, 3.0f, -3.4f, IRON, 1.2f); p.line(-8.0f, 8.0f, 3.0f, 8.0f, IRON, 1.2f);
        p.ring(0.4f, 3.0f, 1.4f, GOLD, 0.8f);
        break; }
    case OI_STAIRS: {                                                       /* steps */
        p.poly({ -13.0f - o, 12.0f + o, -13.0f - o, 5.0f - o, -6.0f - o, 5.0f - o, -6.0f - o, -2.0f - o, 1.0f - o, -2.0f - o, 1.0f - o, -9.0f - o,
                 8.0f - o, -9.0f - o, 8.0f - o, -14.0f - o, 13.0f + o, -14.0f - o, 13.0f + o, 12.0f + o }, INK);
        p.poly({ -13.0f, 12.0f, -13.0f, 5.0f, -6.0f, 5.0f, -6.0f, -2.0f, 1.0f, -2.0f, 1.0f, -9.0f, 8.0f, -9.0f, 8.0f, -14.0f, 13.0f, -14.0f, 13.0f, 12.0f }, STONE);
        for (int k = 0; k < 3; k++) p.line(-13.0f + k * 7.0f, 5.0f - k * 7.0f + 0.9f, -6.0f + k * 7.0f, 5.0f - k * 7.0f + 0.9f, C(0.55f, 0.51f, 0.46f), 1.0f);
        break; }
    case OI_CHEST: {                                                        /* a chest */
        p.rect(-12.0f - o, -1.0f - o, 12.0f + o, 11.0f + o, INK, 1.0f);
        p.dl->PathArcTo(p.P(0, -1.0f), (12.0f + o) * p.u, 3.1416f, 6.2832f, 18); p.dl->PathFillConvex(INK);
        p.rect(-12.0f, -1.0f, 12.0f, 11.0f, WOOD, 0.8f);
        p.dl->PathArcTo(p.P(0, -1.0f), 12.0f * p.u, 3.1416f, 6.2832f, 18); p.dl->PathFillConvex(C(0.70f, 0.47f, 0.24f));   // the lid
        p.line(-12.0f, -1.0f, 12.0f, -1.0f, INK, 1.0f);
        for (int sd = -1; sd <= 1; sd += 2) { p.line(sd * 7.0f, -10.6f, sd * 7.0f, 11.0f, IRON, 1.6f); }
        p.rect(-2.4f, -3.0f, 2.4f, 3.4f, GOLD, 0.6f); p.disc(0, 0.6f, 0.8f, INK);
        break; }
    case OI_PICK:                                                           /* a lock and a pick */
        padlock(p, -2.0f, -1.0f, o);
        inked(p, -2.0f, 4.6f, 12.0f, 12.0f, STEEL, 1.3f, o * 0.8f);
        inked(p, 12.0f, 12.0f, 14.0f, 9.0f, STEEL, 1.3f, o * 0.8f);
        break;
    case OI_DISSOLVE:                                                       /* a lock eaten by a green drop */
        padlock(p, -2.0f, -1.0f, o);
        for (int k = 0; k < 2; k++) {
            float x = 8.6f + k * 3.0f, y = -10.0f + k * 9.0f, r = 2.6f - k * 0.6f;
            p.disc(x, y, r + o, INK); p.tri(x - r - o, y, x + r + o, y, x, y - r * 2.2f - o, INK);
            p.disc(x, y, r, GREEN); p.tri(x - r, y, x + r, y, x, y - r * 2.2f, GREEN);
        }
        break;
    case OI_DISARM: {                                                       /* a trap's jaws */
        p.arc(0, 2.0f, 11.0f, 3.1416f, 6.2832f, INK, 2.6f + 2 * o); p.arc(0, 2.0f, 11.0f, 3.1416f, 6.2832f, IRON, 2.6f);
        p.arc(0, -2.0f, 11.0f, 0, 3.1416f, INK, 2.6f + 2 * o); p.arc(0, -2.0f, 11.0f, 0, 3.1416f, IRON, 2.6f);
        for (int k = -2; k <= 2; k++) {
            float x = k * 4.2f;
            p.tri(x - 1.8f, 2.0f - sqrtf(121.0f - x * x) + 1.4f, x + 1.8f, 2.0f - sqrtf(121.0f - x * x) + 1.4f, x, 0.6f, STEEL);
            p.tri(x - 1.8f, -2.0f + sqrtf(121.0f - x * x) - 1.4f, x + 1.8f, -2.0f + sqrtf(121.0f - x * x) - 1.4f, x, -0.6f, STEEL);
        }
        p.disc(-11.0f, 0, 2.2f, INK); p.disc(11.0f, 0, 2.2f, INK);
        break; }
    case OI_SURRENDER:                                                      /* a white flag */
        inked(p, -9.0f, -13.0f, -9.0f, 14.0f, WOOD, 1.8f, o);
        p.poly({ -8.6f - o, -13.0f - o, 3.0f, -14.8f - o, 12.6f + o, -12.0f - o, 12.6f + o, 1.0f + o, 3.0f, -1.6f + o, -8.6f - o, 0.6f + o }, INK);
        p.poly({ -8.4f, -12.0f, 3.0f, -13.8f, 11.6f, -11.0f, 11.6f, 0.0f, 3.0f, -2.6f, -8.4f, -0.4f }, WHITE);
        break;
    case OI_LOOT: {                                                         /* a sack and coins */
        p.poly({ -4.0f, -9.0f, 4.0f, -9.0f, 2.4f, -6.0f, 9.6f, 2.0f, 9.0f, 11.0f, -9.0f, 11.0f, -9.6f, 2.0f, -2.4f, -6.0f }, INK);
        p.poly({ -3.0f, -8.0f, 3.0f, -8.0f, 1.6f, -5.4f, 8.6f, 2.2f, 8.0f, 10.0f, -8.0f, 10.0f, -8.6f, 2.2f, -1.6f, -5.4f }, C(0.76f, 0.62f, 0.40f));
        p.line(-3.2f, -5.6f, 3.2f, -5.6f, C(0.40f, 0.24f, 0.10f), 1.2f);
        for (int k = 0; k < 2; k++) { float x = 9.4f + k * 3.0f, y = 9.0f - k * 4.0f; p.disc(x, y, 3.0f + o, INK); p.disc(x, y, 3.0f, GOLD); p.ring(x, y, 1.8f, C(0.70f, 0.50f, 0.10f), 0.5f); }
        break; }
    case OI_EXIT:                                                           /* out through the edge */
        p.rect(-12.0f - o, -12.0f - o, -4.0f + o, 12.0f + o, INK); p.rect(-12.0f, -12.0f, -4.0f, 12.0f, C(0.12f, 0.09f, 0.07f));
        p.rect_line(-12.0f, -12.0f, -4.0f, 12.0f, STONE, 1.6f);
        inked(p, -8.0f, 0, 7.0f, 0, PARCH, 3.0f, o);
        p.tri(4.0f - o, -7.0f - o * 1.4f, 4.0f - o, 7.0f + o * 1.4f, 14.0f + o * 1.6f, 0, INK); p.tri(4.0f, -7.0f, 4.0f, 7.0f, 14.0f, 0, PARCH);
        break;
    case OI_CANCEL:                                                         /* a cross */
        inked(p, -9.0f, -9.0f, 9.0f, 9.0f, RED, 4.2f, o); inked(p, 9.0f, -9.0f, -9.0f, 9.0f, RED, 4.2f, o);
        p.line(-9.0f, -9.0f, 9.0f, 9.0f, RED, 4.2f);
        break;
    case OI_ATTACK:                                                         /* a sword */
        sword(p, -9.0f, 9.0f, 1, -1, 25.0f, o);
        break;
    case OI_VULNERABLE:                                                     /* a sword over a cracked shield: harder blows, a weaker defence */
        shield(p, -5.0f, 2.0f, 7.4f, C(0.50f, 0.52f, 0.58f), o);
        p.poly_line({ -6.6f, -5.4f, -3.6f, -1.0f, -6.4f, 2.6f, -3.8f, 9.4f }, INK, 1.2f, false);
        sword(p, -10.0f, 10.0f, 1, -1, 26.0f, o);
        p.tri(10.4f, 6.0f, 14.6f, 6.0f, 12.5f, 10.4f, RED); p.rect(11.5f, 1.0f, 13.5f, 6.2f, RED);   // (the defence lowered)
        break;
    case OI_BERSERK:                                                        /* two swords crossed, and blood */
        sword(p, -10.0f, 10.0f, 1, -1, 25.0f, o);
        sword(p, 10.0f, 10.0f, -1, -1, 25.0f, o);
        for (int k = 0; k < 2; k++) {
            float x = -2.0f + k * 5.0f, y = 6.4f + k * 4.0f, r = 1.8f - k * 0.3f;
            p.disc(x, y, r, RED); p.tri(x - r, y, x + r, y, x, y - r * 2.4f, RED);
        }
        break;
    case OI_PARRY:                                                          /* a shield */
        shield(p, 0, -0.6f, 10.6f, C(0.30f, 0.40f, 0.66f), o);
        p.rect(-1.4f, -10.0f, 1.4f, 11.0f, GOLD); p.rect(-9.0f, -4.2f, 9.0f, -1.4f, GOLD);
        break;
    case OI_MISSILE: {                                                      /* a drawn bow and its arrow */
        p.arc(4.0f, 0, 13.0f, 2.05f, 4.23f, INK, 3.4f); p.arc(4.0f, 0, 13.0f, 2.05f, 4.23f, WOOD, 2.0f);
        float ty = 13.0f * sinf(2.05f), tx = 4.0f + 13.0f * cosf(2.05f);
        p.line(tx, ty, 7.0f, 0, PARCH, 0.7f); p.line(tx, -ty, 7.0f, 0, PARCH, 0.7f);   // the string, drawn
        p.line(7.0f, 0, -12.0f, 0, INK, 1.8f); p.line(7.0f, 0, -11.0f, 0, C(0.80f, 0.64f, 0.40f), 1.0f);   // the shaft
        p.tri(-14.2f, 0, -9.6f, -2.4f, -9.6f, 2.4f, INK); p.tri(-13.2f, 0, -10.2f, -1.4f, -10.2f, 1.4f, STEEL);   // the head
        p.tri(8.0f, 0, 4.0f, -2.8f, 4.6f, 0, RED); p.tri(8.0f, 0, 4.0f, 2.8f, 4.6f, 0, RED);   // the fletching
        break; }
    case OI_FAST:                                                           /* fast forward */
        for (int k = 0; k < 2; k++) {
            float x = -11.0f + k * 10.0f;
            p.tri(x - o, -9.0f - o * 1.6f, x - o, 9.0f + o * 1.6f, x + 11.0f + o * 1.8f, 0, INK);
            p.tri(x, -9.0f, x, 9.0f, x + 11.0f, 0, GOLD);
        }
        break;
    }
}

/* the picture for an entry of the game's battle menus, by its name (s000 0e76, the Orders menu
   D_9607 and the Attack menu D_97db; the names as the game writes them, the trailing blanks
   dropped): OI_NONE for one with no picture (a potion to throw has its own) */
int ui_order_icon_for(const char *name)
{
    char t[48]; int n = 0;
    for (const char *s = name; *s && n < 47; s++) if ((unsigned char)*s >= ' ') t[n++] = (char)tolower((unsigned char)*s);
    while (n && t[n - 1] == ' ') n--;
    t[n] = 0;
    static const struct { const char *pre; int icon; } M[] = {
        { "resume", OI_RESUME }, { "enemy info", OI_INFO }, { "walk towards", OI_WALK }, { "flee towards", OI_FLEE },
        { "halt", OI_HALT }, { "travel as group", OI_GROUP }, { "travel single file", OI_FILE }, { "use door", OI_DOOR },
        { "use stairs", OI_STAIRS }, { "open chest", OI_CHEST }, { "pick lock", OI_PICK }, { "dissolve lock", OI_DISSOLVE },
        { "disarm", OI_DISARM }, { "surrender", OI_SURRENDER }, { "loot bodies", OI_LOOT }, { "exit battlefield", OI_EXIT },
        { "cancel giving order", OI_CANCEL }, { "std attack", OI_ATTACK }, { "vulnerable", OI_VULNERABLE },
        { "berserk", OI_BERSERK }, { "parry", OI_PARRY }, { "use missile", OI_MISSILE },
    };
    for (const auto &m : M) if (!strncmp(t, m.pre, strlen(m.pre))) return m.icon;
    if (n > 9 && !strcmp(t + n - 9, " finished")) return OI_DONE;          /* ("<name> finished": the order given) */
    return OI_NONE;
}

/* the pictures of the members' ways of fighting (mods/styles.h: on their box in the party panel, and in
   its chooser), unlike any order's: a target struck (Archer), a bow and a sword (Skirmisher), a spear
   thrust forward (Charger), a ring struck through (Manual) */
void ui_draw_style_icon(ImDrawList *dl, ImVec2 c, float s, int style)
{
    Pen p{ dl, c, s / 32.0f };
    p.mw = 1.0f;
    const float o = fmaxf(0.9f, 1.1f / p.u);
    switch (style) {
    case 0:                                                                 /* Manual: a ring struck through */
        p.ring(0, 0, 10.5f, INK, 4.2f + 2 * o); p.ring(0, 0, 10.5f, STONE, 4.2f);
        inked(p, -7.4f, 7.4f, 7.4f, -7.4f, STONE, 4.2f, o);
        break;
    case 1:                                                                 /* Archer: a target, an arrow in it */
        p.disc(-2.0f, 2.0f, 12.6f + o, INK); p.disc(-2.0f, 2.0f, 12.6f, WHITE);
        p.disc(-2.0f, 2.0f, 8.6f, RED); p.disc(-2.0f, 2.0f, 4.8f, WHITE); p.disc(-2.0f, 2.0f, 2.2f, RED);
        inked(p, -2.0f, 2.0f, 12.0f, -12.0f, C(0.80f, 0.64f, 0.40f), 1.6f, o * 0.8f);   // the shaft
        p.tri(12.0f, -12.0f, 6.6f, -11.4f, 9.6f, -8.2f, INK); p.tri(14.6f, -14.6f, 9.0f, -14.0f, 12.0f, -10.8f, GREEN);   // the fletching
        break;
    case 2:                                                                 /* Skirmisher: a bow, then a sword */
        p.arc(-2.0f, 0, 12.0f, 2.15f, 4.13f, INK, 3.4f); p.arc(-2.0f, 0, 12.0f, 2.15f, 4.13f, WOOD, 2.0f);
        { float ty = 12.0f * sinf(2.15f), tx = -2.0f + 12.0f * cosf(2.15f); p.line(tx, ty, tx, -ty, PARCH, 0.7f); }
        sword(p, 1.0f, 12.0f, 0.55f, -1, 24.0f, o);
        break;
    case 4:                                                                 /* Charger: a spear thrust forward, at speed */
        inked(p, -13.0f, 9.0f, 9.0f, -6.0f, WOOD, 2.2f, o);
        p.tri(14.6f - o * 0.2f, -10.0f - o, 6.4f - o, -7.8f, 10.0f, -2.6f + o, INK); p.tri(14.0f, -9.6f, 7.4f, -7.4f, 10.2f, -3.4f, STEEL);   // the head
        for (int k = 0; k < 3; k++) inked(p, -14.0f + k * 1.8f, -8.0f + k * 4.6f, -6.0f + k * 1.8f, -12.4f + k * 4.6f, GOLD, 1.3f, o * 0.6f);   // the rush
        break;
    }
}
