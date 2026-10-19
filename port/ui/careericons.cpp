// careericons.cpp - a picture for each kind of profession on the making of a character (arms,
// nobility, church, trade, learning, crafts, common folk), in the manner of the town's pictures
// (ui/townicons.cpp): a parchment disc, bold shapes with an ink outline. Learning and crafts are
// the town's own book and anvil.
#include "imgui.h"
#include <math.h>
#include "pen.h"
#include "townicons.h"

using pen::C;
using pen::Pen;

namespace {
const ImU32 INK = C(0.16f, 0.10f, 0.05f), PARCH = C(0.95f, 0.90f, 0.77f), RIM = C(0.47f, 0.31f, 0.16f);
const ImU32 STEEL = C(0.76f, 0.78f, 0.82f), IRON = C(0.42f, 0.43f, 0.47f), GOLD = C(0.93f, 0.72f, 0.20f);
const ImU32 RED = C(0.72f, 0.12f, 0.10f), WOOD = C(0.52f, 0.33f, 0.16f), LEATHER = C(0.55f, 0.36f, 0.20f);

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

void sword(const Pen &p, float dx, float dy, float o)             /* a sword along (dx, dy) from its hilt at the centre */
{
    float l = sqrtf(dx * dx + dy * dy); dx /= l; dy /= l;
    bar(p, dx * 1.0f, dy * 1.0f, dx, dy, 0, 13.0f, 1.6f, 0.2f, STEEL, o);           // the blade
    bar(p, dx * 1.0f, dy * 1.0f, -dy, dx, 4.6f, 4.6f, 0.9f, 0.9f, GOLD, o);         // the cross-guard
    bar(p, 0, 0, -dx, -dy, 0, 5.0f, 1.1f, 1.1f, C(0.30f, 0.18f, 0.10f), o);         // the grip
    p.disc(-dx * 6.2f, -dy * 6.2f, 1.6f + o, INK); p.disc(-dx * 6.2f, -dy * 6.2f, 1.6f, GOLD);   // the pommel
}
} // namespace

void ui_draw_career_icon(ImDrawList *dl, ImVec2 c, float s, int kind)
{
    if (kind == 4) { ui_draw_town_icon(dl, c, s, TI_UNIVERSITY); return; }   /* learning: the book */
    if (kind == 5) { ui_draw_town_icon(dl, c, s, TI_SMITH); return; }        /* crafts: the anvil */
    Pen p{ dl, c, s / 32.0f };
    p.mw = 1.0f;
    const float o = fmaxf(0.8f, 1.0f / p.u);
    p.disc(0, 0, 15.4f, RIM); p.disc(0, 0, 14.0f, PARCH);
    switch (kind) {
    case 0: {                                                   /* arms: two swords crossed, points up */
        p.dl->PushClipRect(p.P(-14.0f, -14.0f), p.P(14.0f, 14.0f), true);
        sword(p, 1.0f, -1.0f, o); sword(p, -1.0f, -1.0f, o);
        p.dl->PopClipRect();
        break; }
    case 1: {                                                   /* nobility: a crown */
        p.poly({ -11.4f, 7.6f, -12.2f, -6.6f, -6.0f, -0.6f, 0.0f, -9.6f, 6.0f, -0.6f, 12.2f, -6.6f, 11.4f, 7.6f }, INK);
        p.poly({ -10.4f, 6.8f, -11.0f, -4.4f, -5.8f, 0.8f, 0.0f, -7.8f, 5.8f, 0.8f, 11.0f, -4.4f, 10.4f, 6.8f }, GOLD);
        p.rect(-11.6f - o * 0.5f, 5.0f - o * 0.5f, 11.6f + o * 0.5f, 9.6f + o * 0.5f, INK, 0.6f);
        p.rect(-11.0f, 5.4f, 11.0f, 9.2f, C(0.98f, 0.80f, 0.30f), 0.5f);              // the band
        for (int k = -1; k <= 1; k++) p.disc(k * 6.4f, 7.3f, 1.3f, k ? C(0.25f, 0.40f, 0.75f) : RED);   // its stones
        for (int k = -1; k <= 1; k++) { float x = k * 11.6f * (k ? 1.0f : 0), y = k ? -7.2f : -10.6f; p.disc(k ? k * 11.6f : x, y, 1.4f + o * 0.5f, INK); p.disc(k ? k * 11.6f : x, y, 1.4f, C(0.98f, 0.86f, 0.40f)); }
        break; }
    case 2: {                                                   /* church: a cross, gold on its foot */
        p.rect(-2.2f - o, -12.6f - o, 2.2f + o, 9.0f + o, INK, 0.5f);
        p.rect(-8.2f - o, -6.8f - o, 8.2f + o, -2.6f + o, INK, 0.5f);
        p.rect(-2.2f, -12.6f, 2.2f, 9.0f, GOLD, 0.4f);
        p.rect(-8.2f, -6.8f, 8.2f, -2.6f, GOLD, 0.4f);
        p.line(0.6f, -11.6f, 0.6f, 8.0f, C(0.99f, 0.88f, 0.50f), 0.7f);            // its light edge
        p.poly({ -6.6f - o, 12.6f + o, -4.0f - o, 8.0f - o, 4.0f + o, 8.0f - o, 6.6f + o, 12.6f + o }, INK);   // the foot
        p.poly({ -6.0f, 12.0f, -3.6f, 8.6f, 3.6f, 8.6f, 6.0f, 12.0f }, C(0.80f, 0.60f, 0.18f));
        p.disc(0, -4.7f, 1.3f, RED);
        break; }
    case 3: {                                                   /* trade: a merchant's purse and its coins */
        p.poly({ -9.4f, 11.6f, -11.4f, 3.0f, -6.4f, -4.4f, 6.4f, -4.4f, 11.4f, 3.0f, 9.4f, 11.6f }, INK);
        p.poly({ -8.6f, 10.8f, -10.4f, 3.0f, -5.8f, -3.6f, 5.8f, -3.6f, 10.4f, 3.0f, 8.6f, 10.8f }, LEATHER);
        p.poly({ -4.6f, -4.0f, -6.6f, -10.4f, 6.6f, -10.4f, 4.6f, -4.0f }, INK);       // the gathered neck
        p.poly({ -3.8f, -4.4f, -5.6f, -9.6f, 5.6f, -9.6f, 3.8f, -4.4f }, C(0.62f, 0.42f, 0.24f));
        p.rect(-6.2f, -5.4f, 6.2f, -3.6f, RED, 0.4f);                              // the cord
        p.line(-6.0f, 3.0f, 6.0f, 3.0f, C(0.40f, 0.25f, 0.12f), 0.6f);              // a seam
        for (int k = 0; k < 3; k++) {                                             // the coins before it
            float x = 7.6f + k * 1.2f, y = 10.6f - k * 2.4f;
            p.disc(x, y, 3.0f + o * 0.5f, INK); p.disc(x, y, 3.0f, GOLD); p.ring(x, y, 1.8f, C(0.70f, 0.50f, 0.10f), 0.5f);
        }
        break; }
    case 6: {                                                   /* common folk: a pitchfork and a sheaf */
        for (int k = -2; k <= 2; k++) p.line(-4.0f + k * 1.6f, 11.0f, -2.0f + k * 1.0f, -6.0f, C(0.85f, 0.70f, 0.30f), 1.4f);   // the sheaf's stalks
        for (int k = -2; k <= 2; k++) p.ellipse(-2.0f + k * 1.4f, -7.6f - (k == 0 ? 1.4f : 0), 1.3f, 2.6f, C(0.90f, 0.75f, 0.32f), k * 0.25f);   // its ears
        p.rect(-5.4f, 2.0f, 1.4f, 3.6f, RED, 0.3f);                                   // its band
        bar(p, 6.0f, 12.6f, -0.18f, -1.0f, 0, 18.0f, 0.9f, 0.9f, WOOD, o);           // the fork's shaft
        float hx = 6.0f - 0.18f * 18.0f, hy = 12.6f - 18.0f;
        p.line(hx - 3.0f, hy + 0.6f, hx + 3.0f, hy - 0.6f, INK, 2.2f); p.line(hx - 3.0f, hy + 0.6f, hx + 3.0f, hy - 0.6f, IRON, 1.2f);   // its head
        for (int k = -1; k <= 1; k++) { p.line(hx + k * 2.9f, hy - k * 0.55f, hx + k * 2.9f - 0.8f, hy - k * 0.55f - 6.0f, INK, 1.6f);
                                        p.line(hx + k * 2.9f, hy - k * 0.55f, hx + k * 2.9f - 0.8f, hy - k * 0.55f - 6.0f, IRON, 0.8f); }   // the tines
        break; }
    }
    p.ring(0, 0, 15.4f, INK, 0.7f);
}
