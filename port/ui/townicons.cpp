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
    const float u0 = p.u;
    if (icon > TI_TOOLS) p.u *= 0.86f;                                       /* (the places' pictures fill their square: kept inside the roundel) */
    switch (icon) {
    case TI_UNIVERSITY: {   /* a pile of three closed books, their spines out, each with its bands and label (an open book was
                               too like the teachers' lectern, an open book on its stand: ui_draw_teach_mark) */
        struct B { float x0, x1, y0, y1; ImU32 k; } bk[3] = {
            { -11.5f, 10.0f, 4.0f, 9.8f, C(0.62f, 0.16f, 0.12f) },          // the bottom one, red
            { -9.0f, 11.5f, -1.6f, 4.0f, C(0.20f, 0.33f, 0.60f) },          // blue
            { -10.5f, 8.0f, -7.4f, -1.6f, C(0.24f, 0.46f, 0.26f) } };       // the top one, green
        for (const B &b : bk) {
            p.rect(b.x0 - o, b.y0 - o, b.x1 + o, b.y1 + o, INK, 1.6f);
            p.rect(b.x0, b.y0, b.x1, b.y1, b.k, 1.2f);
            float h = b.y1 - b.y0;
            p.rect(b.x0, b.y0 + h * 0.12f, b.x1, b.y0 + h * 0.28f, C(1, 1, 1, 0.16f));            // the light on the spine's round
            for (int e = 0; e < 2; e++) {                                                        // the gilt bands at each end
                float x = e ? b.x1 - 2.6f : b.x0 + 2.6f;
                p.line(x - 0.8f, b.y0 + 0.4f, x - 0.8f, b.y1 - 0.4f, GOLD, 0.8f);
                p.line(x + 0.8f, b.y0 + 0.4f, x + 0.8f, b.y1 - 0.4f, GOLD, 0.8f);
            }
            float mx = (b.x0 + b.x1) * 0.5f;                                                     // the label
            p.rect(mx - 2.6f, b.y0 + h * 0.3f, mx + 2.6f, b.y1 - h * 0.3f, PARCH, 0.4f);
        }
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
    case TI_MARKET: {                                                       /* a market stall: a striped awning, goods on the counter */
        ImU32 white = C(0.97f, 0.93f, 0.84f), apple = C(0.80f, 0.20f, 0.14f), sack = C(0.78f, 0.64f, 0.42f);
        p.rect(-11.0f - o, -3.0f, -9.0f + o, 11.0f, INK); p.rect(9.0f - o, -3.0f, 11.0f + o, 11.0f, INK);   // the posts
        p.rect(-10.6f, -3.0f, -9.4f, 11.0f, WOOD); p.rect(9.4f, -3.0f, 10.6f, 11.0f, WOOD);
        p.poly({ -13.0f - o, -3.0f + o, -9.0f, -11.0f - o, 9.0f, -11.0f - o, 13.0f + o, -3.0f + o }, INK);   // the awning
        for (int k = 0; k < 6; k++) {
            float x0 = -9.0f + k * 3.0f, x1 = x0 + 3.0f, b0 = -13.0f + k * 4.333f, b1 = b0 + 4.333f;
            p.quad(x0, -11.0f, x1, -11.0f, b1, -3.0f, b0, -3.0f, k % 2 ? white : RED);
        }
        for (int k = 0; k < 6; k++) p.disc(-10.8f + k * 4.333f + 2.17f, -3.0f, 2.17f, k % 2 ? white : RED);   // its scalloped edge
        p.rect(-12.0f - o, 4.0f - o, 12.0f + o, 7.0f + o, INK); p.rect(-12.0f, 4.0f, 12.0f, 7.0f, WOOD);   // the counter
        p.disc(-5.0f, 1.6f, 3.0f + o, INK); p.disc(-5.0f, 1.6f, 3.0f, sack);                       // a sack
        p.disc(1.0f, 2.4f, 1.7f + o, INK); p.disc(1.0f, 2.4f, 1.7f, apple); p.disc(4.4f, 2.4f, 1.7f + o, INK); p.disc(4.4f, 2.4f, 1.7f, apple);   // apples
        break; }
    case TI_FOREIGN: {                                                      /* a bale of goods from afar, corded, with a merchant's mark */
        ImU32 cloth = C(0.50f, 0.28f, 0.52f), cord = C(0.86f, 0.70f, 0.38f);
        p.poly({ -11.0f - o, -6.0f - o, 0, -11.0f - o, 11.0f + o, -6.0f - o, 11.0f + o, 8.0f + o, 0, 12.6f + o, -11.0f - o, 8.0f + o }, INK);
        p.poly({ -11.0f, -6.0f, 0, -10.6f, 11.0f, -6.0f, 0, -1.4f }, C(0.62f, 0.38f, 0.64f));     // the top
        p.poly({ -11.0f, -6.0f, 0, -1.4f, 0, 12.0f, -11.0f, 7.6f }, cloth);
        p.poly({ 0, -1.4f, 11.0f, -6.0f, 11.0f, 7.6f, 0, 12.0f }, C(0.40f, 0.21f, 0.42f));
        p.line(-5.5f, -3.7f, -5.5f, 9.8f, cord, 1.1f); p.line(5.5f, -3.7f, 5.5f, 9.8f, cord, 1.1f); // the cords
        p.line(-5.5f, -8.3f, 5.5f, -3.7f, cord, 1.1f); p.line(5.5f, -8.3f, -5.5f, -3.7f, cord, 1.1f);
        p.line(-8.4f, 0.4f, -8.4f, 6.0f, GOLD, 0.9f); p.line(-8.4f, 0.4f, -2.6f, 3.0f, GOLD, 0.9f);   // a merchant's mark (a 4)
        p.line(-10.0f, 4.0f, -2.6f, 4.0f, GOLD, 0.9f);
        break; }
    case TI_PHARMACY: {                                                     /* an apothecary's jar (an albarello), its lid */
        ImU32 glaze = C(0.93f, 0.92f, 0.86f), blue = C(0.18f, 0.32f, 0.62f);
        p.poly({ -7.0f - o, -6.0f, -5.4f - o, -1.0f, -5.4f - o, 6.0f, -7.0f - o, 11.4f + o, 7.0f + o, 11.4f + o, 5.4f + o, 6.0f, 5.4f + o, -1.0f, 7.0f + o, -6.0f }, INK);
        p.poly({ -6.2f, -5.6f, -4.6f, -1.0f, -4.6f, 6.0f, -6.2f, 10.6f, 6.2f, 10.6f, 4.6f, 6.0f, 4.6f, -1.0f, 6.2f, -5.6f }, glaze);
        p.rect(-5.0f, 0.4f, 5.0f, 4.6f, blue);                                // its band
        p.line(-3.0f, 2.5f, 3.0f, 2.5f, glaze, 0.7f);
        p.line(-6.0f, -2.0f, 6.0f, -2.0f, blue, 0.8f); p.line(-6.0f, 8.0f, 6.0f, 8.0f, blue, 0.8f);
        p.rect(-7.6f - o, -9.0f - o, 7.6f + o, -5.4f + o, INK, 1.0f); p.rect(-7.6f, -9.0f, 7.6f, -5.4f, C(0.62f, 0.44f, 0.26f), 1.0f);   // the lid
        p.disc(0, -10.4f, 1.8f + o, INK); p.disc(0, -10.4f, 1.8f, C(0.62f, 0.44f, 0.26f));
        break; }
    case TI_PAWN: {                                                         /* the pawnbrokers' three golden balls on their bar */
        p.line(-12.0f, -9.0f, 12.0f, -9.0f, INK, 2.2f); p.line(-12.0f, -9.0f, 12.0f, -9.0f, WOOD, 1.2f);
        const float bx[3] = { -6.4f, 6.4f, 0.0f }, by[3] = { -1.6f, -1.6f, 7.4f };
        p.line(-6.4f, -9.0f, -6.4f, -5.4f, INK, 0.7f); p.line(6.4f, -9.0f, 6.4f, -5.4f, INK, 0.7f); p.line(0, -9.0f, 0, 3.6f, INK, 0.7f);
        for (int k = 0; k < 3; k++) {
            p.disc(bx[k], by[k], 4.4f + o, INK); p.disc(bx[k], by[k], 4.4f, GOLD);
            p.disc(bx[k] - 1.4f, by[k] - 1.4f, 1.3f, C(1.0f, 0.92f, 0.60f));   // a gleam
        }
        break; }
    case TI_CLOTH: {                                                        /* a bolt of cloth and the shears */
        ImU32 cloth = C(0.66f, 0.16f, 0.14f), dark = C(0.48f, 0.10f, 0.09f);
        p.rect(-11.0f - o, -6.0f - o, 6.0f + o, 4.0f + o, INK, 1.0f);
        p.rect(-11.0f, -6.0f, 6.0f, 4.0f, cloth, 1.0f);                      // the bolt, rolled
        p.ellipse(6.0f, -1.0f, 2.6f + o, 5.0f + o, INK); p.ellipse(6.0f, -1.0f, 2.6f, 5.0f, dark);
        p.ellipse(6.0f, -1.0f, 1.0f, 2.0f, C(0.86f, 0.70f, 0.50f));
        for (int k = 0; k < 4; k++) p.line(-9.0f + k * 4.0f, -6.0f, -9.0f + k * 4.0f, 4.0f, dark, 0.6f);
        p.poly({ -11.0f, 4.0f, 6.0f, 4.0f, 2.0f, 9.0f, -13.0f, 9.0f }, cloth);                 // a length unrolled
        p.line(-13.0f, 9.0f, 2.0f, 9.0f, INK, 0.6f);
        bar(p, 9.0f, 9.0f, -1.0f, -1.2f, 0, 12.0f, 0.9f, 0.3f, STEEL, o);   // the shears: two blades
        bar(p, 9.0f, 9.0f, -1.2f, -0.8f, 0, 12.0f, 0.9f, 0.3f, STEEL, o);
        p.ring(11.0f, 11.4f, 1.8f, INK, 1.2f); p.ring(11.8f, 9.4f, 1.8f, INK, 1.2f);          // their bows
        break; }
    case TI_SOLDIERS: {                                                     /* a kettle hat on a spear: the soldiers' road */
        bar(p, 0, 13.0f, 0, -1, 0, 25.0f, 0.8f, 0.8f, WOOD, o);              // the spear
        p.tri(-2.4f - o, -10.4f, 2.4f + o, -10.4f, 0, -15.6f - o, INK); p.tri(-2.0f, -10.4f, 2.0f, -10.4f, 0, -14.8f, STEEL);
        p.ellipse(0, 1.4f, 12.4f + o, 3.0f + o, INK); p.ellipse(0, 1.4f, 12.4f, 3.0f, STEEL);   // the hat's brim
        p.dl->PathArcTo(p.P(0, 1.0f), (7.2f + o) * p.u, 3.1416f, 6.2832f, 18); p.dl->PathFillConvex(INK);
        p.dl->PathArcTo(p.P(0, 1.0f), 7.2f * p.u, 3.1416f, 6.2832f, 18); p.dl->PathFillConvex(STEEL);   // its crown
        p.arc(0, 1.0f, 5.0f, 3.6f, 4.4f, C(0.92f, 0.94f, 0.98f), 1.0f);    // a gleam
        p.line(-12.0f, 1.6f, 12.0f, 1.6f, C(0.58f, 0.60f, 0.66f), 0.6f);
        break; }
    case TI_FUGGER: {                                                       /* the arms of the Fugger "von der Lilie", the banking line: per pale gold and
                                                                               blue, two lilies counterchanged (granted 1473; the bank itself came after 1400) */
        ImU32 blue = C(0.22f, 0.34f, 0.66f);
        p.poly({ -10.0f - o, -11.0f - o, 10.0f + o, -11.0f - o, 10.0f + o, 1.0f, 0, 12.6f + o, -10.0f - o, 1.0f }, INK);
        p.poly({ -10.0f, -11.0f, 0, -11.0f, 0, 11.6f, -10.0f, 1.0f }, GOLD);
        p.poly({ 0, -11.0f, 10.0f, -11.0f, 10.0f, 1.0f, 0, 11.6f }, blue);
        for (int sd = -1; sd <= 1; sd += 2) {                                 // a lily on each half, in the other's colour
            float x = sd * 5.0f; ImU32 k = sd < 0 ? blue : GOLD;
            p.ellipse(x, -4.6f, 1.2f, 3.6f, k);
            p.ellipse(x - 2.0f, -2.6f, 1.0f, 2.6f, k, -0.6f); p.ellipse(x + 2.0f, -2.6f, 1.0f, 2.6f, k, 0.6f);
            p.rect(x - 2.8f, -0.4f, x + 2.8f, 0.8f, k);
            p.tri(x - 1.2f, 0.8f, x + 1.2f, 0.8f, x, 4.0f, k);
        }
        break; }
    case TI_MEDICI: {                                                       /* the Medici's arms as about 1400: seven red balls (palle) in an orle on gold,
                                                                               as in the Old Sacristy of San Lorenzo (the blue ball with France's lilies came in 1465) */
        p.poly({ -10.0f - o, -11.0f - o, 10.0f + o, -11.0f - o, 10.0f + o, 1.0f, 0, 12.6f + o, -10.0f - o, 1.0f }, INK);
        p.poly({ -10.0f, -11.0f, 10.0f, -11.0f, 10.0f, 1.0f, 0, 11.6f, -10.0f, 1.0f }, GOLD);
        const float bx[7] = { -5.6f, 0, 5.6f, -6.2f, 6.2f, -3.4f, 3.4f }, by[7] = { -7.0f, -7.4f, -7.0f, -1.8f, -1.8f, 3.8f, 3.8f };
        for (int k = 0; k < 7; k++) { p.disc(bx[k], by[k], 2.2f + o * 0.6f, INK); p.disc(bx[k], by[k], 2.2f, RED); p.disc(bx[k] - 0.7f, by[k] - 0.7f, 0.6f, C(0.95f, 0.55f, 0.45f)); }
        break; }
    case TI_HANSE: {                                                        /* a Hanseatic cog: a round hull, one square sail in red and white */
        ImU32 hull = C(0.46f, 0.28f, 0.13f), white = C(0.97f, 0.94f, 0.86f), water = C(0.30f, 0.50f, 0.76f);
        p.line(0, -13.0f, 0, 3.0f, INK, 1.6f);
        p.rect(-7.6f - o, -11.0f - o, 7.6f + o, 1.0f + o, INK);
        for (int k = 0; k < 4; k++) p.rect(-7.6f, -11.0f + k * 3.0f, 7.6f, -8.0f + k * 3.0f, k % 2 ? white : RED);   // the sail
        p.poly({ -13.0f, 2.0f, 13.0f, 2.0f, 10.0f, 9.0f, -10.0f, 9.0f }, INK);
        p.poly({ -12.0f, 2.8f, 12.0f, 2.8f, 9.4f, 8.2f, -9.4f, 8.2f }, hull);
        p.rect(-12.6f - o, -0.6f - o, -8.0f + o, 3.0f, INK); p.rect(-12.4f, -0.4f, -8.4f, 3.0f, hull);   // the castles, fore and aft
        p.rect(8.0f - o, -0.6f - o, 12.6f + o, 3.0f, INK); p.rect(8.4f, -0.4f, 12.4f, 3.0f, hull);
        for (int k = -1; k <= 1; k++) p.arc(k * 6.0f, 13.4f, 3.0f, 3.6f, 5.8f, water, 1.3f);
        p.poly({ 0, -13.2f, 5.4f, -12.4f, 0, -11.4f }, RED);                 // the pennant at the masthead, red and white (Lübeck's, the Hanse's colours)
        p.poly({ 0, -12.3f, 5.4f, -12.4f, 0, -11.4f }, white);
        break; }
    case TI_CHURCH: {                                                       /* a town church: a nave and its bell tower */
        ImU32 roof = C(0.66f, 0.26f, 0.18f), dark = C(0.20f, 0.14f, 0.10f);
        p.rect(-12.0f - o, -1.0f - o, 4.0f + o, 11.0f + o, INK); p.rect(-12.0f, -1.0f, 4.0f, 11.0f, STONE);   // the nave
        p.tri(-13.0f - o, -0.6f, 5.0f + o, -0.6f, -4.0f, -7.4f - o, INK); p.tri(-12.0f, -1.0f, 4.0f, -1.0f, -4.0f, -6.6f, roof);
        p.rect(3.0f - o, -6.0f - o, 10.6f + o, 11.0f + o, INK); p.rect(3.0f, -6.0f, 10.6f, 11.0f, STONE);   // the tower
        p.tri(2.4f - o, -5.6f, 11.2f + o, -5.6f, 6.8f, -12.6f - o, INK); p.tri(3.0f, -6.0f, 10.6f, -6.0f, 6.8f, -11.8f, roof);
        p.line(6.8f, -15.2f, 6.8f, -11.6f, GOLD, 1.1f); p.line(5.4f, -14.0f, 8.2f, -14.0f, GOLD, 0.9f);
        p.rect(5.4f, -3.0f, 8.2f, 1.0f, dark); p.disc(6.8f, -3.0f, 1.4f, dark);   // the belfry
        p.rect(-6.0f, 4.0f, -2.0f, 11.0f, dark); p.disc(-4.0f, 4.0f, 2.0f, dark);   // the door
        break; }
    case TI_MONASTERY: {                                                    /* a monk's cowl */
        ImU32 habit = C(0.45f, 0.32f, 0.20f), shade = C(0.33f, 0.23f, 0.14f), face = C(0.14f, 0.10f, 0.07f);
        p.poly({ -12.0f - o, 12.6f + o, -10.0f - o, 2.0f, -7.0f - o, -6.0f, 0, -12.6f - o, 7.0f + o, -6.0f, 10.0f + o, 2.0f, 12.0f + o, 12.6f + o }, INK);
        p.poly({ -11.0f, 12.0f, -9.2f, 2.0f, -6.4f, -5.6f, 0, -11.8f, 6.4f, -5.6f, 9.2f, 2.0f, 11.0f, 12.0f }, habit);
        p.poly({ 0, -11.8f, 6.4f, -5.6f, 9.2f, 2.0f, 11.0f, 12.0f, 3.0f, 12.0f }, shade);
        p.ellipse(0, -2.6f, 4.4f, 5.6f, face);                               // the face in the hood's shadow
        p.line(-2.0f, 6.0f, -2.0f, 12.0f, INK, 0.6f); p.line(2.0f, 6.0f, 2.0f, 12.0f, INK, 0.6f);   // the scapular
        p.line(-1.0f, 8.0f, -1.0f, 12.0f, C(0.86f, 0.74f, 0.46f), 0.8f);    // a cord
        break; }
    case TI_INN: {                                                          /* an inn's signboard, hung from its iron arm, a star painted on it (not the tankard: Regain Endurance's) */
        ImU32 board = C(0.62f, 0.40f, 0.20f), dark = C(0.44f, 0.27f, 0.12f);
        p.line(-12.0f, -11.0f, 11.0f, -11.0f, INK, 2.6f); p.line(-12.0f, -11.0f, 11.0f, -11.0f, IRON, 1.4f);   // the arm
        p.line(-12.0f, -4.0f, -4.0f, -11.0f, INK, 1.8f); p.line(-12.0f, -4.0f, -4.0f, -11.0f, IRON, 0.9f);     // its brace
        p.line(-6.0f, -11.0f, -6.0f, -6.0f, INK, 1.0f); p.line(7.0f, -11.0f, 7.0f, -6.0f, INK, 1.0f);           // the chains
        p.rect(-9.5f - o, -6.0f - o, 10.5f + o, 11.0f + o, INK, 1.6f); p.rect(-9.5f, -6.0f, 10.5f, 11.0f, board, 1.6f);
        p.rect_line(-8.0f, -4.5f, 9.0f, 9.5f, dark, 0.8f, 1.0f);
        {   const float cx = 0.5f, cy = 2.6f, R = 5.4f, r = 2.2f; float pts[20];   // the star
            for (int k = 0; k < 10; k++) { float a = -1.5708f + k * 0.62832f, rr = k % 2 ? r : R; pts[2 * k] = cx + cosf(a) * rr; pts[2 * k + 1] = cy + sinf(a) * rr; }
            for (int k = 0; k < 10; k++) p.dl->PathLineTo(p.P(pts[2 * k], pts[2 * k + 1]));
            p.dl->PathFillConcave(GOLD); }
        break; }
    case TI_SQUARE: {                                                       /* a notice board on its posts: news and gossip */
        ImU32 paper = C(0.98f, 0.95f, 0.86f), board = C(0.58f, 0.38f, 0.20f);
        p.rect(-9.0f - o, -2.0f, -7.0f + o, 13.0f, INK); p.rect(7.0f - o, -2.0f, 9.0f + o, 13.0f, INK);
        p.rect(-8.6f, -2.0f, -7.4f, 13.0f, WOOD); p.rect(7.4f, -2.0f, 8.6f, 13.0f, WOOD);
        p.rect(-12.0f - o, -11.0f - o, 12.0f + o, 4.0f + o, INK, 0.6f); p.rect(-12.0f, -11.0f, 12.0f, 4.0f, board, 0.6f);
        p.tri(-13.6f, -10.4f, 13.6f, -10.4f, 0, -14.0f, INK);               // its little roof
        p.rect(-10.0f, -8.6f, -3.0f, -1.0f, paper); p.rect(-1.4f, -9.4f, 4.4f, -3.0f, paper); p.rect(5.4f, -7.6f, 10.4f, 2.0f, paper);
        for (int k = 0; k < 3; k++) { p.line(-9.0f, -6.6f + k * 2.0f, -4.0f, -6.6f + k * 2.0f, INK, 0.45f); p.line(6.4f, -5.6f + k * 2.2f, 9.4f, -5.6f + k * 2.2f, INK, 0.45f); }
        p.line(-0.4f, -7.4f, 3.4f, -7.4f, INK, 0.45f); p.line(-0.4f, -5.4f, 3.4f, -5.4f, INK, 0.45f);
        p.disc(-6.5f, -8.2f, 0.6f, RED); p.disc(1.5f, -9.0f, 0.6f, RED); p.disc(7.9f, -7.2f, 0.6f, RED);   // the nails
        break; }
    case TI_TOWNHALL: {                                                     /* a town hall: a stepped gable and its banner */
        ImU32 wall = C(0.86f, 0.80f, 0.68f), dark = C(0.20f, 0.14f, 0.10f), roof = C(0.42f, 0.30f, 0.24f);
        p.poly({ -12.0f - o, 11.0f + o, -12.0f - o, -1.0f, -9.0f - o, -1.0f, -9.0f - o, -4.0f, -6.0f - o, -4.0f, -6.0f - o, -7.0f, -3.0f - o, -7.0f,
                 -3.0f - o, -10.0f - o, 3.0f + o, -10.0f - o, 3.0f + o, -7.0f, 6.0f + o, -7.0f, 6.0f + o, -4.0f, 9.0f + o, -4.0f, 9.0f + o, -1.0f, 12.0f + o, -1.0f, 12.0f + o, 11.0f + o }, INK);
        p.poly({ -12.0f, 11.0f, -12.0f, -1.0f, -9.0f, -1.0f, -9.0f, -4.0f, -6.0f, -4.0f, -6.0f, -7.0f, -3.0f, -7.0f,
                 -3.0f, -10.0f, 3.0f, -10.0f, 3.0f, -7.0f, 6.0f, -7.0f, 6.0f, -4.0f, 9.0f, -4.0f, 9.0f, -1.0f, 12.0f, -1.0f, 12.0f, 11.0f }, wall);
        p.disc(0, -4.6f, 2.4f, roof); p.ring(0, -4.6f, 2.4f, INK, 0.5f);   // the clock
        p.line(0, -4.6f, 0, -6.2f, INK, 0.5f); p.line(0, -4.6f, 1.2f, -4.6f, INK, 0.5f);
        for (int k = -1; k <= 1; k++) { p.rect(k * 6.0f - 1.6f, 1.0f, k * 6.0f + 1.6f, 5.0f, dark); p.disc(k * 6.0f, 1.0f, 1.6f, dark); }   // windows
        p.rect(-2.0f, 7.0f, 2.0f, 11.0f, dark);
        p.line(0, -10.0f, 0, -15.0f, INK, 0.8f); p.poly({ 0, -15.0f, 5.0f, -14.0f, 0, -12.6f }, RED);   // the banner
        break; }
    case TI_PRISON: {                                                       /* a barred window in a stone wall */
        ImU32 dark = C(0.12f, 0.09f, 0.07f), mortar = C(0.56f, 0.52f, 0.46f);
        p.rect(-12.0f, -11.0f, 12.0f, 11.0f, STONE, 1.0f);
        for (int r = 0; r < 4; r++) {                                         // the courses of stone
            float y = -11.0f + r * 5.5f;
            p.line(-12.0f, y, 12.0f, y, mortar, 0.5f);
            for (int c = 0; c < 3; c++) { float x = -8.0f + c * 8.0f + (r % 2) * 4.0f; p.line(x, y, x, y + 5.5f, mortar, 0.5f); }
        }
        p.rect(-7.0f - o, -6.0f - o, 7.0f + o, 6.0f + o, INK); p.rect(-7.0f, -6.0f, 7.0f, 6.0f, dark);
        for (int k = -1; k <= 1; k++) { p.line(k * 3.5f, -6.0f, k * 3.5f, 6.0f, INK, 2.0f); p.line(k * 3.5f, -6.0f, k * 3.5f, 6.0f, IRON, 1.2f); }
        p.line(-7.0f, 0, 7.0f, 0, INK, 2.0f); p.line(-7.0f, 0, 7.0f, 0, IRON, 1.2f);
        break; }
    case TI_BARRACKS: {                                                     /* the guard: two crossed spears behind a round shield */
        for (int sd = -1; sd <= 1; sd += 2) {
            p.line(-sd * 11.0f, 12.0f, sd * 9.0f, -10.0f, INK, 2.4f); p.line(-sd * 11.0f, 12.0f, sd * 9.0f, -10.0f, WOOD, 1.4f);   // the shafts
            p.tri(sd * 13.0f, -14.4f, sd * 7.4f, -10.6f, sd * 10.6f, -7.6f, INK);   // the heads
            p.tri(sd * 12.2f, -13.6f, sd * 8.2f, -10.6f, sd * 10.4f, -8.6f, STEEL);
        }
        p.disc(0, 2.0f, 8.0f + o, INK); p.disc(0, 2.0f, 8.0f, RED);         // the shield
        p.disc(0, 2.0f, 5.6f, C(0.97f, 0.93f, 0.84f));
        p.line(0, -3.6f, 0, 7.6f, RED, 2.4f); p.line(-5.6f, 2.0f, 5.6f, 2.0f, RED, 2.4f);   // the city's cross
        p.disc(0, 2.0f, 1.6f, GOLD);
        break; }
    case TI_FORTRESS: {                                                     /* a keep with its battlements and flag */
        ImU32 dark = C(0.20f, 0.14f, 0.10f);
        p.rect(-11.0f - o, -4.0f - o, 11.0f + o, 12.0f + o, INK); p.rect(-11.0f, -4.0f, 11.0f, 12.0f, STONE);
        for (int k = 0; k < 4; k++) { float x = -11.0f + k * 6.33f; p.rect(x - o, -8.0f - o, x + 3.4f + o, -4.0f, INK); p.rect(x, -8.0f, x + 3.4f, -4.0f, STONE); }   // merlons
        p.rect(-4.0f - o, -12.0f - o, 4.0f + o, -4.0f, INK); p.rect(-4.0f, -12.0f, 4.0f, -4.0f, STONE);   // the tower
        p.line(0, -12.0f, 0, -16.0f, INK, 0.8f); p.poly({ 0, -16.0f, 6.0f, -14.8f, 0, -13.4f }, RED);
        p.rect(-3.0f, 4.0f, 3.0f, 12.0f, dark); p.disc(0, 4.0f, 3.0f, dark);   // the gate
        p.rect(-8.0f, 0, -6.0f, 4.0f, dark); p.rect(6.0f, 0, 8.0f, 4.0f, dark); p.rect(-1.0f, -9.0f, 1.0f, -6.0f, dark);   // slits
        break; }
    case TI_SLUMS: {                                                        /* a crooked hovel, its roof patched */
        ImU32 wall = C(0.62f, 0.50f, 0.36f), thatch = C(0.66f, 0.56f, 0.30f), dark = C(0.20f, 0.14f, 0.10f);
        p.poly({ -11.0f - o, 11.0f + o, -10.0f - o, -1.0f, 9.0f + o, 0.6f, 11.0f + o, 11.0f + o }, INK);
        p.poly({ -10.2f, 10.4f, -9.4f, -0.6f, 8.4f, 1.0f, 10.2f, 10.4f }, wall);
        p.poly({ -13.0f - o, 0.4f + o, -2.0f, -10.0f - o, 12.0f + o, 2.0f + o }, INK);
        p.poly({ -12.0f, -0.2f, -2.0f, -9.0f, 11.0f, 1.4f }, thatch);
        p.rect(-5.0f, -5.0f, 1.0f, -1.0f, C(0.48f, 0.38f, 0.22f)); p.rect(3.0f, -3.4f, 7.0f, 0.4f, C(0.54f, 0.30f, 0.20f));   // patches
        p.poly({ -4.0f, 3.0f, 1.0f, 3.4f, 1.6f, 10.4f, -4.6f, 10.4f }, dark);   // a sagging door
        p.line(4.0f, 4.0f, 7.4f, 4.6f, INK, 0.6f); p.line(5.4f, 3.0f, 5.8f, 6.4f, INK, 0.6f);     // a boarded window
        break; }
    case TI_GROVE: {                                                        /* a tree */
        ImU32 leaf = C(0.32f, 0.54f, 0.22f), dark = C(0.22f, 0.40f, 0.16f);
        bar(p, 0, 12.0f, 0, -1, 0, 10.0f, 2.2f, 1.6f, WOOD, o);              // the trunk
        p.disc(-5.0f, -2.0f, 5.6f + o, INK); p.disc(5.0f, -2.0f, 5.6f + o, INK); p.disc(0, -7.0f, 6.4f + o, INK); p.disc(0, 0, 6.0f + o, INK);
        p.disc(-5.0f, -2.0f, 5.6f, dark); p.disc(5.0f, -2.0f, 5.6f, leaf); p.disc(0, -7.0f, 6.4f, leaf); p.disc(0, 0, 6.0f, leaf);
        p.disc(-2.0f, -9.0f, 2.0f, C(0.46f, 0.68f, 0.32f));
        p.line(-12.0f, 12.0f, 12.0f, 12.0f, C(0.36f, 0.50f, 0.22f), 1.4f); // the grass
        break; }
    case TI_WALL: {                                                         /* a stretch of the city wall */
        ImU32 mortar = C(0.56f, 0.52f, 0.46f);
        p.rect(-13.0f - o, -3.0f - o, 13.0f + o, 11.0f + o, INK); p.rect(-13.0f, -3.0f, 13.0f, 11.0f, STONE);
        for (int k = 0; k < 5; k++) { float x = -13.0f + k * 5.6f; p.rect(x - o, -8.0f - o, x + 3.4f + o, -3.0f, INK); p.rect(x, -8.0f, x + 3.4f, -3.0f, STONE); }
        for (int r = 0; r < 3; r++) {
            float y = -3.0f + r * 4.66f;
            p.line(-13.0f, y, 13.0f, y, mortar, 0.5f);
            for (int c = 0; c < 4; c++) { float x = -10.0f + c * 6.6f + (r % 2) * 3.3f; p.line(x, y, x, y + 4.66f, mortar, 0.5f); }
        }
        break; }
    case TI_GATE: {                                                         /* a gate tower, its arch open, the road leading out */
        ImU32 dark = C(0.20f, 0.14f, 0.10f), road = C(0.70f, 0.58f, 0.38f), roof = C(0.66f, 0.26f, 0.18f);
        p.rect(-10.0f - o, -6.0f - o, 10.0f + o, 11.0f + o, INK); p.rect(-10.0f, -6.0f, 10.0f, 11.0f, STONE);
        p.tri(-11.6f - o, -5.6f, 11.6f + o, -5.6f, 0, -13.6f - o, INK); p.tri(-11.0f, -6.0f, 11.0f, -6.0f, 0, -12.8f, roof);
        p.rect(-5.0f, 1.0f, 5.0f, 11.0f, C(0.62f, 0.78f, 0.90f)); p.disc(0, 1.0f, 5.0f, C(0.62f, 0.78f, 0.90f));   // the sky through the arch
        p.poly({ -2.0f, 4.0f, 2.0f, 4.0f, 5.0f, 11.0f, -5.0f, 11.0f }, road);
        for (int k = -1; k <= 1; k++) p.line(k * 3.0f, -2.0f, k * 3.0f, 3.0f, dark, 0.8f);   // the portcullis, raised
        p.line(-4.6f, -1.0f, 4.6f, -1.0f, dark, 0.8f);
        p.ring(0, 1.0f, 5.0f, INK, 0.6f);
        break; }
    }
    p.u = u0;
    p.ring(0, 0, 15.4f, INK, 0.7f);
}
