// townicons.cpp - see townicons.h. Drawn with ImGui's shapes (pen.h) in the manner of the quests'
// pictures: bold shapes with an ink outline, readable at the size of a line of text.
#include "imgui.h"
#include <math.h>
#include "pen.h"
#include "townicons.h"

using pen::C;
using pen::Pen;

namespace {
const ImU32 INK = C(0.16f, 0.10f, 0.05f), PARCH = C(0.95f, 0.90f, 0.77f), RIM = C(0.47f, 0.31f, 0.16f);
const ImU32 STEEL = C(0.76f, 0.78f, 0.82f), IRON = C(0.42f, 0.43f, 0.47f), GOLD = C(0.93f, 0.72f, 0.20f);
const ImU32 RED = C(0.72f, 0.12f, 0.10f), WOOD = C(0.52f, 0.33f, 0.16f), STONE = C(0.74f, 0.70f, 0.64f);

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
} // namespace

void ui_draw_town_icon(ImDrawList *dl, ImVec2 c, float s, int icon)
{
    Pen p{ dl, c, s / 32.0f };
    p.mw = 1.0f;
    const float o = fmaxf(0.8f, 1.0f / p.u);
    p.disc(0, 0, 15.4f, RIM); p.disc(0, 0, 14.0f, PARCH);
    switch (icon) {
    case TI_UNIVERSITY: {                                                   /* an open book, its ribbon */
        ImU32 cover = C(0.45f, 0.18f, 0.12f), page = C(0.99f, 0.97f, 0.90f), rule = C(0.55f, 0.45f, 0.30f);
        p.poly({ -12.6f, -6.6f, 0.0f, -4.2f, 12.6f, -6.6f, 12.6f, 8.4f, 0.0f, 10.6f, -12.6f, 8.4f }, INK);
        p.poly({ -12.0f, -5.8f, 0.0f, -3.4f, 12.0f, -5.8f, 12.0f, 7.8f, 0.0f, 10.0f, -12.0f, 7.8f }, cover);
        p.poly({ -11.0f, -7.4f, -0.6f, -5.0f, -0.6f, 8.6f, -11.0f, 6.4f }, page);
        p.poly({ 0.6f, -5.0f, 11.0f, -7.4f, 11.0f, 6.4f, 0.6f, 8.6f }, page);
        p.line(0, -5.0f, 0, 8.8f, INK, 0.7f);
        for (int k = 0; k < 4; k++) {
            float y = -3.6f + k * 2.8f;
            p.line(-9.4f, y - 1.6f + k * 0.0f, -2.2f, y, rule, 0.55f);
            p.line(2.2f, y, 9.4f, y - 1.6f, rule, 0.55f);
        }
        p.poly({ 4.0f, -6.0f, 6.0f, -6.4f, 6.0f, 12.4f, 5.0f, 10.8f, 4.0f, 12.6f }, RED);   // the ribbon
        break; }
    case TI_CATHEDRAL: {                                                    /* a cathedral's front: the spire and its cross */
        ImU32 dark = C(0.20f, 0.14f, 0.10f), roof = C(0.42f, 0.30f, 0.24f);
        p.rect(-10.0f - o, -1.0f - o, 10.0f + o, 11.0f + o, INK);
        p.rect(-10.0f, -1.0f, 10.0f, 11.0f, STONE);
        p.tri(-11.0f - o, -0.6f, 11.0f + o, -0.6f, 0, -6.4f - o, INK); p.tri(-10.0f, -1.0f, 10.0f, -1.0f, 0, -5.6f, roof);
        p.rect(-3.0f - o, -7.0f - o, 3.0f + o, 0.0f, INK); p.rect(-3.0f, -7.0f, 3.0f, 0.0f, STONE);   // the tower
        p.tri(-3.6f - o, -6.6f, 3.6f + o, -6.6f, 0, -13.0f - o, INK); p.tri(-3.0f, -7.0f, 3.0f, -7.0f, 0, -12.2f, roof);   // the spire
        p.line(0, -15.0f, 0, -11.6f, GOLD, 1.2f); p.line(-1.4f, -13.8f, 1.4f, -13.8f, GOLD, 1.0f);
        p.disc(0, -3.6f, 1.6f, C(0.30f, 0.45f, 0.72f));                      // the rose window
        p.rect(-2.4f, 4.4f, 2.4f, 11.0f, dark); p.disc(0, 4.4f, 2.4f, dark);  // the door
        for (int sd = -1; sd <= 1; sd += 2) { p.rect(sd * 6.6f - 1.0f, 3.2f, sd * 6.6f + 1.0f, 7.4f, dark); p.disc(sd * 6.6f, 3.2f, 1.0f, dark); }
        break; }
    case TI_ALCHEMIST: {                                                    /* a flask on the fire's glow */
        ImU32 glass = C(0.82f, 0.90f, 0.92f), brew = C(0.36f, 0.66f, 0.30f);
        p.rect(-2.4f - o, -12.0f - o, 2.4f + o, -1.0f, INK);
        p.disc(0, 4.0f, 8.4f + o, INK);
        p.rect(-2.4f, -12.0f, 2.4f, -1.0f, glass);
        p.disc(0, 4.0f, 8.4f, glass);
        p.dl->PathArcTo(p.P(0, 4.0f), 8.4f * p.u, 0.15f, 3.0f, 24); p.dl->PathFillConvex(brew);   // the brew, the lower part
        p.disc(-3.4f, 1.4f, 1.6f, C(1, 1, 1, 0.7f));                         // a gleam
        p.rect(-3.4f, -13.2f, 3.4f, -11.6f, C(0.50f, 0.36f, 0.22f), 0.5f);   // the stopper's rim
        p.disc(4.6f, 6.4f, 0.9f, C(0.70f, 0.90f, 0.60f)); p.disc(2.0f, 8.4f, 0.7f, C(0.70f, 0.90f, 0.60f));   // bubbles
        break; }
    case TI_PHYSICIAN: {                                                    /* a mortar and its pestle */
        ImU32 bowl = C(0.70f, 0.66f, 0.60f), shade = C(0.55f, 0.51f, 0.46f);
        bar(p, -1.0f, 2.0f, 0.55f, -1.0f, 0, 14.0f, 1.4f, 1.9f, WOOD, o);   // the pestle
        p.rect(-10.6f - o, -0.4f - o, 10.6f + o, 1.6f + o, INK, 0.8f);
        p.dl->PathArcTo(p.P(0, 0.6f), (10.0f + o) * p.u, 0.0f, 3.1416f, 24); p.dl->PathFillConvex(INK);
        p.dl->PathArcTo(p.P(0, 0.6f), 10.0f * p.u, 0.0f, 3.1416f, 24); p.dl->PathFillConvex(bowl);
        p.dl->PathArcTo(p.P(2.0f, 0.6f), 7.0f * p.u, 0.2f, 1.6f, 12); p.dl->PathStroke(shade, 0, p.W(1.0f));
        p.rect(-10.6f, -0.4f, 10.6f, 1.6f, C(0.82f, 0.78f, 0.72f), 0.8f);    // the rim
        p.rect(-5.0f - o, 10.0f, 5.0f + o, 12.4f + o, INK, 0.6f); p.rect(-5.0f, 10.2f, 5.0f, 12.4f, bowl, 0.6f);   // the foot
        p.ellipse(-3.6f, -2.0f, 1.6f, 1.0f, C(0.36f, 0.62f, 0.28f)); p.ellipse(3.2f, -1.6f, 1.4f, 0.9f, C(0.30f, 0.55f, 0.24f));   // herbs
        break; }
    case TI_DOCKS: {                                                        /* a river boat under sail */
        ImU32 hull = C(0.50f, 0.31f, 0.15f), sail = C(0.98f, 0.94f, 0.82f), water = C(0.30f, 0.50f, 0.76f);
        p.line(0, -12.0f, 0, 4.0f, INK, 1.6f);                               // the mast
        p.poly({ -0.4f, -11.0f, 8.8f, -9.4f, 8.0f, 1.6f, -0.4f, 2.6f }, INK); p.poly({ 0.4f, -10.2f, 8.0f, -8.8f, 7.3f, 0.9f, 0.4f, 1.8f }, sail);
        p.poly({ -0.6f, -9.0f, -7.8f, 1.8f, -0.6f, 2.6f }, INK); p.poly({ -1.3f, -7.2f, -6.6f, 1.2f, -1.3f, 1.8f }, sail);
        p.line(4.2f, -9.4f, 4.0f, 1.2f, RED, 0.9f);                          // a stripe on the sail
        p.poly({ -12.0f, 3.2f, 12.0f, 3.2f, 8.6f, 9.0f, -8.6f, 9.0f }, INK);
        p.poly({ -11.0f, 4.0f, 11.0f, 4.0f, 8.0f, 8.2f, -8.0f, 8.2f }, hull);
        p.line(-10.2f, 5.6f, 10.2f, 5.6f, C(0.36f, 0.21f, 0.09f), 0.6f);
        for (int k = -1; k <= 1; k++) p.arc(k * 6.0f, 13.4f, 3.0f, 3.6f, 5.8f, water, 1.3f);   // the waves
        break; }
    case TI_BANK: {                                                         /* the moneychanger's scales */
        bar(p, 0, 12.0f, 0, -1, 0, 21.0f, 1.0f, 1.0f, WOOD, o);              // the post
        p.rect(-6.0f - o, 10.0f - o, 6.0f + o, 12.6f + o, INK, 0.6f); p.rect(-6.0f, 10.0f, 6.0f, 12.6f, WOOD, 0.6f);   // its foot
        bar(p, 0, -7.4f, 1, 0, 11.0f, 11.0f, 0.8f, 0.8f, GOLD, o);           // the beam
        p.disc(0, -9.4f, 1.6f + o, INK); p.disc(0, -9.4f, 1.6f, GOLD);
        for (int sd = -1; sd <= 1; sd += 2) {
            float x = sd * 9.6f;
            p.line(x, -7.0f, x - 3.6f, 2.0f, INK, 0.6f); p.line(x, -7.0f, x + 3.6f, 2.0f, INK, 0.6f);
            p.dl->PathArcTo(p.P(x, 2.0f), (4.4f + o) * p.u, 0.0f, 3.1416f, 14); p.dl->PathFillConvex(INK);
            p.dl->PathArcTo(p.P(x, 2.0f), 4.4f * p.u, 0.0f, 3.1416f, 14); p.dl->PathFillConvex(GOLD);
        }
        p.disc(-9.6f, 1.0f, 2.0f, C(0.98f, 0.82f, 0.30f)); p.ring(-9.6f, 1.0f, 2.0f, C(0.70f, 0.50f, 0.10f), 0.5f);   // a coin in one pan
        break; }
    case TI_SWORD: {                                                        /* a sword, point up */
        bar(p, 0, 4.0f, 0, -1, 0, 16.0f, 1.9f, 0.2f, STEEL, o);              // the blade
        p.line(0, 3.0f, 0, -9.0f, C(0.60f, 0.62f, 0.68f), 0.6f);            // its fuller
        bar(p, 0, 4.0f, 1, 0, 6.6f, 6.6f, 1.0f, 1.0f, GOLD, o);              // the cross-guard
        bar(p, 0, 5.0f, 0, 1, 0, 6.0f, 1.2f, 1.2f, C(0.30f, 0.18f, 0.10f), o);   // the grip
        p.disc(0, 12.0f, 1.8f + o, INK); p.disc(0, 12.0f, 1.8f, GOLD);       // the pommel
        break; }
    case TI_ARMOUR: {                                                       /* a breastplate */
        ImU32 shade = C(0.58f, 0.60f, 0.66f);
        p.poly({ -9.6f, -10.4f, -4.4f, -9.0f, 0, -6.2f, 4.4f, -9.0f, 9.6f, -10.4f, 10.6f, -4.0f, 8.6f, 3.6f, 9.2f, 11.4f,
                 -9.2f, 11.4f, -8.6f, 3.6f, -10.6f, -4.0f }, INK);
        p.poly({ -8.8f, -9.4f, -4.2f, -8.0f, 0, -5.2f, 4.2f, -8.0f, 8.8f, -9.4f, 9.6f, -4.0f, 7.6f, 3.6f, 8.2f, 10.4f,
                 -8.2f, 10.4f, -7.6f, 3.6f, -9.6f, -4.0f }, STEEL);
        p.poly({ 1.0f, -5.8f, 4.2f, -8.0f, 8.8f, -9.4f, 9.6f, -4.0f, 7.6f, 3.6f, 8.2f, 10.4f, 1.0f, 10.4f }, shade);   // the shaded half
        p.line(0, -5.2f, 0, 10.4f, INK, 0.6f);                               // the ridge
        p.line(-7.6f, 4.6f, 7.6f, 4.6f, INK, 0.7f);                          // the waist's lame
        for (int sd = -1; sd <= 1; sd += 2) p.disc(sd * 6.2f, 7.6f, 0.7f, GOLD);   // rivets
        break; }
    case TI_BOW: {                                                          /* a drawn bow and its arrow */
        p.arc(4.0f, 0, 13.0f, 2.05f, 4.23f, INK, 3.0f); p.arc(4.0f, 0, 13.0f, 2.05f, 4.23f, WOOD, 1.8f);   // the bow
        float ty = 13.0f * sinf(2.05f), tx = 4.0f + 13.0f * cosf(2.05f);
        p.line(tx, ty, 7.0f, 0, C(0.30f, 0.24f, 0.18f), 0.6f); p.line(tx, -ty, 7.0f, 0, C(0.30f, 0.24f, 0.18f), 0.6f);   // the string, drawn
        p.line(7.0f, 0, -12.0f, 0, INK, 1.4f); p.line(7.0f, 0, -11.0f, 0, C(0.70f, 0.55f, 0.32f), 0.8f);   // the shaft
        p.tri(-13.6f, 0, -9.8f, -2.0f, -9.8f, 2.0f, INK); p.tri(-12.8f, 0, -10.4f, -1.2f, -10.4f, 1.2f, STEEL);   // the head
        p.tri(7.6f, 0, 4.0f, -2.6f, 4.6f, 0, RED); p.tri(7.6f, 0, 4.0f, 2.6f, 4.6f, 0, RED);   // the fletching
        break; }
    case TI_SMITH: {                                                        /* an anvil and a hammer */
        p.poly({ -12.0f, -1.0f, 9.0f, -1.0f, 9.0f, 3.0f, 3.0f, 4.0f, 4.6f, 9.6f, -4.6f, 9.6f, -3.0f, 4.0f, -8.0f, 3.0f }, INK);
        p.poly({ -11.0f, -0.2f, 8.2f, -0.2f, 8.2f, 2.3f, 2.4f, 3.3f, 3.6f, 8.8f, -3.6f, 8.8f, -2.4f, 3.3f, -7.6f, 2.3f }, IRON);
        p.rect(-6.4f, 9.4f, 6.4f, 12.0f, INK, 0.6f); p.rect(-5.8f, 9.8f, 5.8f, 11.6f, IRON, 0.4f);
        p.line(-10.0f, 0.6f, 7.6f, 0.6f, C(0.62f, 0.63f, 0.68f), 0.7f);
        bar(p, 9.0f, -3.4f, -0.7f, -1.0f, 0, 12.0f, 0.8f, 0.8f, WOOD, o);    // the hammer's helve
        bar(p, 1.4f, -12.6f, 1.0f, -0.7f, 3.0f, 3.0f, 1.8f, 1.8f, IRON, o);  // its head
        break; }
    case TI_TOOLS: {                                                        /* a halberd: the artificers' (the "metalworkers and
                                                                               artificiers" of the tinker's square) sell the hammers,
                                                                               maces, flails and polearms (DARKLAND.LST categories 2, 4, 8) */
        bar(p, 0, 12.4f, 0, -1, 0, 18.4f, 0.95f, 0.95f, WOOD, o);            // the shaft
        bar(p, 0, -5.0f, 0, -1, 0, 8.6f, 1.3f, 0.2f, STEEL, o);              // the spike
        p.poly({ 0.0f, -8.6f, 8.6f, -11.4f, 10.4f, -5.2f, 8.6f, 0.6f, 0.0f, -2.2f }, INK);           // the axe blade
        p.poly({ 0.8f, -7.8f, 7.9f, -10.1f, 9.3f, -5.2f, 7.9f, -0.6f, 0.8f, -3.0f }, STEEL);
        p.line(7.9f, -10.1f, 9.3f, -5.2f, C(0.86f, 0.88f, 0.92f), 0.6f);    // its edge
        p.line(9.3f, -5.2f, 7.9f, -0.6f, C(0.86f, 0.88f, 0.92f), 0.6f);
        p.poly({ 0.0f, -7.4f, -7.6f, -9.2f, -0.2f, -3.4f }, INK);          // the hook at the back
        p.poly({ -0.6f, -6.9f, -5.6f, -8.0f, -0.6f, -4.4f }, STEEL);
        break; }
    }
    p.ring(0, 0, 15.4f, INK, 0.7f);
}
