// alchemyicons.cpp - the pictures of the potions and of the alchemical ingredients, drawn with ImGui's
// shapes (no image files).
//
// Potions (items 95-116): one round-bellied flask for all, so a potion reads as a potion at a glance,
// full to the neck (one solid colour behind the emblem, so the emblem reads at a list's small size),
// with the liquid in the potion's own colour and its emblem: what it does, in the manner of a
// fifteenth-century woodcut (a green cloud for Noxious Aroma, a bolt for Thunderbolt, a winged foot
// for Quickmove ...). Its grade is told by one, two or three gold dots down the neck: a formula of
// grade g brews quality base + 10 g (s029 0e76:3fb2), so the dots are (quality - base) / 10 + 1.
//
// Ingredients (items 72-94): a parchment roundel with the symbol the alchemists' tables give it (the
// planets' signs for the metals, the elements' triangles for the four humours' bases, sulphur's
// triangle and cross ...), or for the herbs and roots a small drawing as in a herbal (the mandrake
// a little man).
#include "../runtime/config.h"
#include "imgui.h"
#include <math.h>

#include "pen.h"

extern int potion_grade_preview;
int potion_grade_preview = -1;                 /* (the settings' pictures: this style, not the chosen one) */
namespace {
using pen::C;
using pen::Pen;

const ImU32 INK = 0, GLASS_LINE = 1;     // (placeholders: the colours are made where used)

/* the liquid of each potion, 95..116 */
const float LIQ[22][3] = {
    { 0.45f, 0.62f, 0.20f },   // Noxious Aroma: bilious green
    { 0.85f, 0.40f, 0.15f },   // Eyeburn: pepper red-orange
    { 0.70f, 0.70f, 0.74f },   // Black Cloud: grey (the black cloud in it shows)
    { 0.66f, 0.55f, 0.35f },   // Fleadust: dun
    { 0.46f, 0.58f, 0.10f },   // Eater-Water: acid yellow-green (deep: the pale drop shows)
    { 0.45f, 0.25f, 0.50f },   // Breath of Death: purple
    { 0.98f, 0.80f, 0.25f },   // Sunburst: gold
    { 0.22f, 0.33f, 0.62f },   // Thunderbolt: storm blue (dark: the yellow bolt shows)
    { 0.90f, 0.30f, 0.12f },   // Arabian Fire
    { 0.16f, 0.12f, 0.10f },   // Stone-tar
    { 0.62f, 0.10f, 0.12f },   // Deadly Blade: blood
    { 0.38f, 0.42f, 0.52f },   // Strongedge: steel (dark: the bright blade shows)
    { 0.80f, 0.45f, 0.18f },   // Great Power: amber
    { 0.30f, 0.55f, 0.35f },   // Trueflight: green
    { 0.50f, 0.52f, 0.58f },   // Hardarmor: iron
    { 0.62f, 0.36f, 0.58f },   // Transformation: rose
    { 0.30f, 0.50f, 0.85f },   // Truesight: clear blue
    { 0.70f, 0.85f, 0.90f },   // New-wind: pale sky
    { 0.40f, 0.40f, 0.45f },   // Ironarm
    { 0.95f, 0.70f, 0.30f },   // Quickmove: quicksilver-gold
    { 0.95f, 0.92f, 0.75f },   // Essence o'Grace: pearl
    { 0.36f, 0.11f, 0.07f },   // Firewall: smouldering (dark: the stones and flames show)
};

/* the flask: a round belly on the lower part, a neck and a cork; `fill` how full */
void flask(const Pen &p, ImU32 liq, float fill = 1.0f)
{
    ImU32 glass = C(0.86f, 0.92f, 0.97f, 0.50f), rim = C(0.20f, 0.14f, 0.09f, 0.95f);
    p.rect(-1.9f, -9.5f, 1.9f, -4.0f, glass);                       // neck
    p.disc(0, 3.0f, 7.4f, glass);                                     // belly
    float r = 7.4f, top = 3.0f + r - 2 * r * fill;                    // the liquid: a chord of the belly
    float a = acosf(fminf(1.f, fmaxf(-1.f, (top - 3.0f) / r)));
    p.dl->PathArcTo(p.P(0, 3.0f), (r - 0.6f) * p.u, 1.5708f - a, 1.5708f + a, 20); p.dl->PathFillConvex(liq);
    p.ring(0, 3.0f, 7.4f, rim, 0.9f);
    p.line(-1.9f, -9.5f, -1.9f, -3.9f, rim, 0.8f); p.line(1.9f, -9.5f, 1.9f, -3.9f, rim, 0.8f);
    p.rect(-2.5f, -11.6f, 2.5f, -9.2f, C(0.55f, 0.36f, 0.20f), 0.6f);   // the cork
    p.disc(-3.4f, 0.0f, 1.3f, C(1, 1, 1, 0.65f));                     // a gleam
}
/* the grade: one, two or three gold dots down the neck's right side (at least a few pixels each, so
   that they stay apart and countable in a list's small icon) */
void grade_dots(const Pen &p, int tier)
{
    if (tier < 1) return;
    if (tier > 3) tier = 3;
    int gm = potion_grade_preview >= 0 ? potion_grade_preview : config_get_int("display.potion_grade", 4);
    if (gm == 3 || gm == 4) {                                    /* chevrons, as a rank's: one to three, stacked */
        float hw = fmaxf(2.4f, 3.4f / p.u), hh = hw * 0.62f, th = fmaxf(1.1f, 1.7f / p.u), step = hh + th * 1.25f;
        float x = gm == 3 ? 5.6f : 5.8f, y0 = gm == 3 ? -10.4f : 9.6f - (tier - 1) * step;
        if (gm == 4) {                                           /* a dark badge in the corner, the chevrons on it */
            p.dl->AddRectFilled(p.P(x - hw - th, y0 - hh * 0.5f - th * 1.3f), p.P(x + hw + th, y0 + (tier - 1) * step + hh * 0.5f + th * 1.6f), C(0.16f, 0.10f, 0.05f, 0.92f), th * p.u);
        }
        for (int k = 0; k < tier; k++) {
            float y = y0 + k * step;
            ImVec2 pts[3] = { p.P(x - hw, y + hh * 0.5f), p.P(x, y - hh * 0.5f), p.P(x + hw, y + hh * 0.5f) };
            if (gm == 3) p.dl->AddPolyline(pts, 3, C(0.20f, 0.12f, 0.04f, 0.95f), 0, (th + 1.2f / p.u) * p.u);
            p.dl->AddPolyline(pts, 3, C(1.0f, 0.82f, 0.20f), 0, th * p.u);
        }
        return;
    }
    if (gm == 0) {     /* (the earlier dots, for a comparison) */
        float r = fmaxf(1.3f, 2.0f / p.u), step = fmaxf(3.3f, 2 * r + 0.5f / p.u);
        for (int k = 0; k < tier; k++) {
            float x = 5.4f + (r - 1.3f) * 0.8f, y = -10.6f + k * step;
            p.disc(x, y, r, C(0.98f, 0.80f, 0.20f));
            p.ring(x, y, r, C(0.30f, 0.18f, 0.06f), fmaxf(0.45f, 0.8f / p.u));
        }
        return;
    }
    float r = fmaxf(1.7f, 2.6f / p.u), step = fmaxf(3.9f, 2 * r + 0.7f / p.u);
    float x = 5.4f + (r - 1.3f) * 0.8f, y0 = -10.6f;
    /* on a dark strip, so they stand out from the flask and the page */
    p.dl->AddRectFilled(p.P(x - r - 0.7f, y0 - r - 0.7f), p.P(x + r + 0.7f, y0 + (tier - 1) * step + r + 0.7f), C(0.16f, 0.10f, 0.05f, 0.85f), (r + 0.7f) * p.u);
    for (int k = 0; k < tier; k++) {
        float y = y0 + k * step;
        p.disc(x, y, r, C(1.0f, 0.84f, 0.22f));
        p.disc(x - r * 0.3f, y - r * 0.3f, r * 0.35f, C(1.0f, 0.97f, 0.75f));
    }
}
void puff(const Pen &p, float x, float y, float r, ImU32 k) { p.disc(x, y, r, k); }
void flame(const Pen &p, float x, float y, float h, ImU32 outer, ImU32 inner)
{
    p.tri(x - h * 0.35f, y, x + h * 0.35f, y, x, y - h, outer);
    p.disc(x, y - h * 0.08f, h * 0.36f, outer);
    p.tri(x - h * 0.18f, y, x + h * 0.18f, y, x, y - h * 0.6f, inner);
}
/* a triangle, and the same grown by o units from its middle under it in ink (the woodcut's outline) */
void tri_o(const Pen &p, float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k, ImU32 ink, float o)
{
    float cx = (x0 + x1 + x2) / 3, cy = (y0 + y1 + y2) / 3;
    auto g = [&](float &x, float &y) { float dx = x - cx, dy = y - cy, d = sqrtf(dx * dx + dy * dy); if (d > 0.01f) { x += dx / d * o * 1.6f; y += dy / d * o * 1.6f; } };
    float a0 = x0, b0 = y0, a1 = x1, b1 = y1, a2 = x2, b2 = y2; g(a0, b0); g(a1, b1); g(a2, b2);
    p.tri(a0, b0, a1, b1, a2, b2, ink);
    p.tri(x0, y0, x1, y1, x2, y2, k);
}
void eye(const Pen &p, float x, float y, float w, ImU32 lid, ImU32 iris)
{
    p.bez(x - w, y, x, y - w * 0.9f, x + w, y, lid, 1.0f);
    p.bez(x - w, y, x, y + w * 0.9f, x + w, y, lid, 1.0f);
    p.disc(x, y, w * 0.38f, iris); p.disc(x, y, w * 0.16f, C(0.05f, 0.05f, 0.05f));
}
} // namespace

/* id 95..116, tier 0 (unknown) or 1..3 */
bool ui_draw_potion(ImDrawList *dl, ImVec2 c, float s, int id, int tier)
{
    if (id < 95 || id > 116) return false;
    Pen p{ dl, ImVec2(c.x, c.y + s * 0.03f), s / 16.0f * 0.86f };   /* (a little smaller: room for fumes and flames inside the icon's square) */
    int k = id - 95;
    ImU32 liq = C(LIQ[k][0], LIQ[k][1], LIQ[k][2], 0.95f);
    ImU32 ink = C(0.18f, 0.11f, 0.06f), white = C(0.98f, 0.97f, 0.92f), gold = C(0.98f, 0.82f, 0.25f), red = C(0.80f, 0.12f, 0.10f);
    switch (id) {
    case 95: flask(p, liq);                                           // Noxious Aroma: a reeking cloud (puffs)
        for (int pass = 0; pass < 2; pass++) { float o = pass ? 0 : 0.55f; ImU32 k = pass ? C(0.86f, 0.92f, 0.48f) : ink;
            p.disc(-2.7f, 4.9f, 2.2f + o, k); p.disc(0.5f, 2.4f, 2.9f + o, k); p.disc(2.9f, 5.3f, 2.0f + o, k); }
        p.disc(-0.6f, 1.4f, 0.7f, C(1, 1, 0.85f)); break;
    case 96: flask(p, liq); eye(p, 0, 3.0f, 4.2f, white, C(0.85f, 0.15f, 0.10f));   // Eyeburn: a burning eye
        for (int r = 0; r < 5; r++) { float a = -2.4f + r * 0.45f; p.line(cosf(a) * 5.2f, 3 + sinf(a) * 5.2f, cosf(a) * 6.6f, 3 + sinf(a) * 6.6f, C(1, 0.96f, 0.82f), 0.8f); } break;
    case 97: flask(p, liq);                                           // Black Cloud: black smoke (puffs) on grey
        for (int pass = 0; pass < 2; pass++) { float o = pass ? 0 : 0.5f; ImU32 k = pass ? C(0.07f, 0.07f, 0.09f) : C(0.97f, 0.96f, 0.92f);
            p.disc(-2.7f, 4.9f, 2.2f + o, k); p.disc(0.5f, 2.4f, 2.9f + o, k); p.disc(2.9f, 5.3f, 2.0f + o, k); }
        break;
    case 98: flask(p, liq);                                           // Fleadust: a cloud of motes and fleas
        for (int i = 0; i < 9; i++) { float x = -4.6f + (i * 37 % 9), y = -1.8f + (i * 23 % 5); p.disc(x, y, 0.6f, C(0.25f, 0.17f, 0.10f)); }
        p.disc(3.2f, 5.0f, 1.3f, ink); p.line(3.2f, 5.0f, 5.0f, 3.7f, ink, 0.5f); p.line(3.2f, 5.0f, 5.2f, 6.1f, ink, 0.5f); p.disc(-2.8f, 6.4f, 1.1f, ink); break;
    case 99: flask(p, liq);                                           // Eater-Water: a drop that eats into a bar
        p.disc(0, 3.6f, 2.15f, ink); tri_o(p, -1.6f, 2.8f, 1.6f, 2.8f, 0, -1.4f, C(0.95f, 1.0f, 0.55f), ink, 0.5f); p.disc(0, 3.6f, 1.65f, C(0.95f, 1.0f, 0.55f));   /* a corrosive drop over a pitted bar */
        p.rect(-4.3f, 5.9f, 4.3f, 8.5f, ink); p.rect(-3.8f, 6.4f, 3.8f, 8.0f, C(0.82f, 0.84f, 0.88f)); p.disc(-1.0f, 6.5f, 0.8f, ink); p.disc(1.4f, 6.4f, 0.6f, ink); break;
    case 100: flask(p, liq);                                          // Breath of Death: a skull
        p.disc(0, 2.2f, 3.3f, white); p.rect(-1.9f, 4.0f, 1.9f, 6.6f, white, 0.6f);
        p.disc(-1.3f, 2.2f, 0.95f, ink); p.disc(1.3f, 2.2f, 0.95f, ink); p.tri(-0.5f, 4.4f, 0.5f, 4.4f, 0, 3.5f, ink);
        p.line(-0.9f, 6.6f, -0.9f, 5.4f, ink, 0.4f); p.line(0.9f, 6.6f, 0.9f, 5.4f, ink, 0.4f); break;
    case 101: flask(p, liq);                                          // Sunburst: a sun with a face of rays
        for (int r = 0; r < 12; r++) { float a = r * 0.5236f; p.line(cosf(a) * 3.4f, 3 + sinf(a) * 3.4f, cosf(a) * (r & 1 ? 5.4f : 6.4f), 3 + sinf(a) * (r & 1 ? 5.4f : 6.4f), C(0.62f, 0.26f, 0.04f), 0.9f); }
        p.disc(0, 3.0f, 3.0f, C(1.0f, 0.96f, 0.62f)); p.ring(0, 3.0f, 3.0f, C(0.55f, 0.24f, 0.04f), 0.8f); break;
    case 102: flask(p, liq);                                          // Thunderbolt: a bolt in the storm
        p.dl->PathLineTo(p.P(1.6f, -2.0f)); p.dl->PathLineTo(p.P(-2.2f, 3.8f)); p.dl->PathLineTo(p.P(0.2f, 3.8f)); p.dl->PathLineTo(p.P(-1.6f, 8.4f));
        p.dl->PathLineTo(p.P(3.2f, 2.2f)); p.dl->PathLineTo(p.P(0.8f, 2.2f)); p.dl->PathLineTo(p.P(3.0f, -2.0f)); p.dl->PathFillConcave(C(1.0f, 0.92f, 0.35f));
        p.poly_line({ 1.6f, -2.0f, -2.2f, 3.8f, 0.2f, 3.8f, -1.6f, 8.4f, 3.2f, 2.2f, 0.8f, 2.2f, 3.0f, -2.0f }, ink, 0.5f); break;
    case 103: flask(p, liq);                                          // Arabian Fire: one great flame (Firewall: a wall and flames)
        flame(p, 0, 7.3f, 8.8f, ink, ink);
        flame(p, 0, 6.9f, 7.9f, C(1.0f, 0.86f, 0.30f), C(1.0f, 0.98f, 0.82f)); break;
    case 104: flask(p, liq);                                          // Stone-tar: a sludge that holds fast: a boot stuck in it
        p.ellipse(0, 7.4f, 5.2f, 1.9f, C(0.92f, 0.88f, 0.78f)); p.ellipse(0, 7.4f, 4.6f, 1.45f, C(0.04f, 0.03f, 0.03f));   /* the pool, its rim lit */
        p.poly({ -1.4f, -1.2f, 1.6f, -1.2f, 1.6f, 4.4f, 4.0f, 5.4f, 4.0f, 7.4f, -1.4f, 7.4f }, ink);                          /* the boot, sunk */
        p.poly({ -0.9f, -0.7f, 1.1f, -0.7f, 1.1f, 4.8f, 3.5f, 5.8f, 3.5f, 7.0f, -0.9f, 7.0f }, C(0.66f, 0.50f, 0.32f));
        p.ellipse(1.2f, 7.1f, 3.2f, 0.75f, C(0.04f, 0.03f, 0.03f));                                                    /* the tar over its foot */
        p.line(-2.6f, 6.9f, -2.8f, 5.0f, C(0.04f, 0.03f, 0.03f), 0.7f); p.line(3.6f, 6.6f, 3.9f, 4.8f, C(0.04f, 0.03f, 0.03f), 0.6f); break;
    case 105: flask(p, liq);                                          // Deadly Blade: more damage: blood from the blade
        p.quad(-4.9f, 7.3f, -3.4f, 8.6f, 4.6f, 0.0f, 3.2f, -1.1f, ink); p.quad(-4.5f, 7.2f, -3.5f, 8.1f, 4.1f, 0.0f, 3.2f, -0.7f, C(0.88f, 0.90f, 0.93f));   /* the oiled blade (the three weapon oils share it) */
        p.line(-5.4f, 6.1f, -2.4f, 9.1f, ink, 1.6f); p.line(-5.2f, 6.3f, -2.6f, 8.9f, C(0.70f, 0.52f, 0.20f), 0.9f);
        p.disc(3.4f, 5.6f, 1.6f, white); tri_o(p, 2.4f, 5.0f, 4.4f, 5.0f, 3.4f, 2.4f, red, white, 0.55f); p.disc(3.4f, 5.6f, 1.05f, red); break;
    case 106: flask(p, liq);                                          // Strongedge: more penetration: the point through a plate
        p.quad(-4.9f, 7.3f, -3.4f, 8.6f, 4.6f, 0.0f, 3.2f, -1.1f, ink); p.quad(-4.5f, 7.2f, -3.5f, 8.1f, 4.1f, 0.0f, 3.2f, -0.7f, C(0.88f, 0.90f, 0.93f));   /* the oiled blade (the three weapon oils share it) */
        p.line(-5.4f, 6.1f, -2.4f, 9.1f, ink, 1.6f); p.line(-5.2f, 6.3f, -2.6f, 8.9f, C(0.70f, 0.52f, 0.20f), 0.9f);
        p.rect(0.4f, -2.4f, 2.6f, 5.4f, ink, 0.4f); p.rect(0.9f, -1.9f, 2.1f, 4.9f, C(0.74f, 0.76f, 0.82f), 0.3f);   /* the plate, the point through it */
        p.quad(2.1f, 0.6f, 2.9f, 0.0f, 4.6f, 0.0f, 2.1f, 2.2f, ink); p.quad(2.1f, 1.0f, 3.2f, -0.6f, 4.1f, 0.0f, 2.1f, 1.8f, C(0.88f, 0.90f, 0.93f));
        p.disc(1.5f, -1.0f, 0.35f, ink); p.disc(1.5f, 4.0f, 0.35f, ink); break;
    case 107: flask(p, liq);                                          // Great Power: the weapon's quality raised: gold chevrons
        p.quad(-4.9f, 7.3f, -3.4f, 8.6f, 4.6f, 0.0f, 3.2f, -1.1f, ink); p.quad(-4.5f, 7.2f, -3.5f, 8.1f, 4.1f, 0.0f, 3.2f, -0.7f, C(0.88f, 0.90f, 0.93f));   /* the oiled blade (the three weapon oils share it) */
        p.line(-5.4f, 6.1f, -2.4f, 9.1f, ink, 1.6f); p.line(-5.2f, 6.3f, -2.6f, 8.9f, C(0.70f, 0.52f, 0.20f), 0.9f);
        for (int k = 0; k < 2; k++) { float y = 5.6f + k * 2.2f;
            p.poly_line({ 1.4f, y + 1.3f, 3.2f, y - 0.5f, 5.0f, y + 1.3f }, ink, 1.6f, false);
            p.poly_line({ 1.4f, y + 1.3f, 3.2f, y - 0.5f, 5.0f, y + 1.3f }, C(1.0f, 0.82f, 0.22f), 0.8f, false); }
        break;
    case 108: flask(p, liq);                                          // Trueflight: an arrow in the target's eye
        p.disc(-0.8f, 3.8f, 4.4f, white); p.ring(-0.8f, 3.8f, 4.4f, ink, 0.5f); p.ring(-0.8f, 3.8f, 2.7f, red, 1.0f); p.disc(-0.8f, 3.8f, 1.0f, red);
        p.line(-0.8f, 3.8f, 4.2f, -1.2f, ink, 0.9f); p.line(4.2f, -1.2f, 5.4f, -1.1f, ink, 0.7f); p.line(4.2f, -1.2f, 4.3f, -2.4f, ink, 0.7f); break;
    case 109: flask(p, liq);                                          // Hardarmor: a heater shield
        p.dl->PathLineTo(p.P(-3.6f, -0.5f)); p.dl->PathLineTo(p.P(3.6f, -0.5f)); p.dl->PathLineTo(p.P(3.6f, 3.6f)); p.dl->PathLineTo(p.P(0, 7.6f)); p.dl->PathLineTo(p.P(-3.6f, 3.6f));
        p.dl->PathFillConvex(C(0.86f, 0.88f, 0.92f)); p.poly_line({ -3.6f, -0.5f, 3.6f, -0.5f, 3.6f, 3.6f, 0, 7.6f, -3.6f, 3.6f }, ink, 0.6f); p.line(0, -0.5f, 0, 7.4f, red, 0.9f); p.line(-3.5f, 2.0f, 3.5f, 2.0f, red, 0.9f); break;
    case 110: flask(p, liq);                                          // Transformation: base metal made gold (it purifies, "and sometimes appears to make more gold")
        p.disc(-1.8f, 4.4f, 3.0f, ink); p.disc(-1.8f, 4.4f, 2.5f, C(0.58f, 0.60f, 0.64f)); p.ring(-1.8f, 4.4f, 1.6f, C(0.42f, 0.44f, 0.48f), 0.5f);   /* lead */
        p.disc(1.8f, 2.4f, 3.0f, ink); p.disc(1.8f, 2.4f, 2.5f, C(0.98f, 0.80f, 0.22f)); p.ring(1.8f, 2.4f, 1.6f, C(0.75f, 0.52f, 0.06f), 0.5f);    /* gold */
        p.line(1.8f, 1.2f, 1.8f, 3.6f, C(0.75f, 0.52f, 0.06f), 0.5f); p.line(0.6f, 2.4f, 3.0f, 2.4f, C(0.75f, 0.52f, 0.06f), 0.5f);
        for (int r = 0; r < 4; r++) { float a = r * 1.5708f; p.line(4.2f, -1.4f, 4.2f + cosf(a) * 1.5f, -1.4f + sinf(a) * 1.5f, white, 0.6f); } break;
    case 111: flask(p, liq); eye(p, 0, 3.0f, 4.4f, white, C(0.30f, 0.55f, 0.90f));   // Truesight: the open eye, rays above
        for (int r = 0; r < 5; r++) { float a = -2.6f + r * 0.4f; p.line(cosf(a) * 4.6f, 3 + sinf(a) * 4.6f, cosf(a) * 6.4f, 3 + sinf(a) * 6.4f, gold, 0.6f); } break;
    case 112: flask(p, liq);                                          // New-wind: the breath of a wind's head
        for (int r = 0; r < 3; r++) { float y = 1.0f + r * 2.2f; p.bez(-5.0f, y, 0, y - 1.6f, 4.0f, y, C(0.14f, 0.28f, 0.48f), 0.9f); p.arc(4.0f, y - 0.9f, 0.9f, -1.2f, 2.6f, C(0.14f, 0.28f, 0.48f), 0.7f); }
        break;
    case 113: flask(p, liq);                                          // Ironarm: a mailed fist, clenched (knuckles up)
        { ImU32 steel = C(0.86f, 0.88f, 0.92f);
          for (int pass = 0; pass < 2; pass++) { float o = pass ? 0 : 0.5f; ImU32 k = pass ? steel : ink;
              for (int f = 0; f < 4; f++) p.disc(-2.7f + f * 1.8f, 0.8f, 1.15f + o, k);       /* the knuckles */
              p.rect(-3.85f - o, 0.8f, 3.85f + o, 5.0f + o, k, 1.0f); }                      /* the curled fingers */
          for (int f = 0; f < 3; f++) p.line(-1.8f + f * 1.8f, 1.2f, -1.8f + f * 1.8f, 2.9f, ink, 0.45f);
          p.rect(-4.2f, 2.7f, 1.4f, 4.7f, ink, 1.0f); p.rect(-3.8f, 3.1f, 1.0f, 4.3f, C(0.70f, 0.72f, 0.78f), 0.7f);   /* the thumb across them */
          p.poly({ -2.9f, 5.0f, 2.9f, 5.0f, 3.8f, 9.0f, -3.8f, 9.0f }, ink); p.poly({ -2.4f, 5.5f, 2.4f, 5.5f, 3.2f, 8.5f, -3.2f, 8.5f }, C(0.58f, 0.60f, 0.66f));   /* the cuff */
          p.line(-2.9f, 7.0f, 2.9f, 7.0f, ink, 0.45f); }
        break;
    case 114: flask(p, liq);                                          // Quickmove: Mercury's winged foot
        p.quad(-3.5f, 7.5f, 3.4f, 7.5f, 3.4f, 5.6f, -1.0f, 4.4f, C(0.45f, 0.28f, 0.14f)); p.rect(-3.5f, 1.0f, -1.0f, 7.0f, C(0.45f, 0.28f, 0.14f));
        for (int f = 0; f < 3; f++) p.bez(-1.0f, 2.0f + f * 1.1f, 1.8f, -0.2f + f * 0.9f, 4.2f, -0.8f + f * 1.3f, ink, 1.6f);
        for (int f = 0; f < 3; f++) p.bez(-1.0f, 2.0f + f * 1.1f, 1.8f, -0.2f + f * 0.9f, 4.2f, -0.8f + f * 1.3f, white, 0.8f); break;
    case 115: flask(p, liq);                                          // Essence o'Grace: a dove under a halo
        p.ring(0, -0.8f, 3.0f, ink, 1.8f); p.ring(0, -0.8f, 3.0f, C(0.95f, 0.70f, 0.10f), 0.9f);
        p.disc(0, 4.0f, 2.45f, ink); p.disc(1.6f, 3.0f, 1.45f, ink);         /* the dove's outline, then the dove */
        tri_o(p, -5.0f, 2.2f, -0.5f, 3.6f, -1.0f, 5.0f, white, ink, 0.55f); tri_o(p, 5.0f, 2.2f, 0.5f, 3.6f, 1.0f, 5.0f, white, ink, 0.55f);
        p.disc(0, 4.0f, 1.9f, white); p.disc(1.6f, 3.0f, 0.9f, white); p.disc(2.0f, 2.8f, 0.3f, ink); break;
    case 116: flask(p, liq);                                          // Firewall: a wall of stone with flames over it (Arabian Fire: one great flame)
        p.rect(-4.6f, 4.6f, 4.6f, 8.6f, C(0.80f, 0.72f, 0.62f));
        p.line(-4.6f, 6.6f, 4.6f, 6.6f, ink, 0.5f); p.line(-1.6f, 4.6f, -1.6f, 6.6f, ink, 0.5f); p.line(1.6f, 4.6f, 1.6f, 6.6f, ink, 0.5f); p.line(0, 6.6f, 0, 8.6f, ink, 0.5f); p.line(-3.2f, 6.6f, -3.2f, 8.6f, ink, 0.5f); p.line(3.2f, 6.6f, 3.2f, 8.6f, ink, 0.5f);
        flame(p, -2.8f, 4.6f, 3.8f, C(0.98f, 0.55f, 0.12f), C(1.0f, 0.90f, 0.40f)); flame(p, 0.4f, 4.6f, 4.8f, C(0.98f, 0.50f, 0.12f), C(1.0f, 0.90f, 0.40f)); flame(p, 3.2f, 4.6f, 3.4f, C(0.98f, 0.55f, 0.12f), C(1.0f, 0.90f, 0.40f)); break;
    }
    grade_dots(p, tier);
    (void)INK; (void)GLASS_LINE;
    return true;
}

/* a proposal, for review (the icon sheet uiconstyles:4): every ingredient a sign of the alchemists'
   tables in the same thick stroke, by family - the four bases the elements' triangles in their
   colours; the metals and their ores the planets' signs (gold in gold, copper in copper, the rest
   in steel); the sulphurs, salts and poisons their own signs in their own colours; the waters, oils
   and vapours their signs (aqua regia's has Water's triangle); the herbs, which have no sign, small
   drawings in the same green line. Not in use until chosen */
int ingredient_style_preview = 0;
int herb_variant = 0;                            /* (the herbs' three tries, for the sheet uiconstyles:4) */
static bool draw_ingredient_signs(Pen &p, int id)
{
    const float W = 1.5f;
    ImU32 steel = C(0.24f, 0.26f, 0.32f), gold = C(0.66f, 0.45f, 0.04f), copper = C(0.66f, 0.34f, 0.12f), lead = C(0.16f, 0.15f, 0.16f);
    ImU32 sulphur = C(0.62f, 0.46f, 0.02f), orpi = C(0.72f, 0.50f, 0.00f), vermilion = C(0.70f, 0.14f, 0.08f), salt = C(0.30f, 0.30f, 0.34f);
    ImU32 water = C(0.20f, 0.35f, 0.62f), oil = C(0.22f, 0.16f, 0.10f), amber = C(0.70f, 0.40f, 0.04f), marsh = C(0.26f, 0.40f, 0.34f);
    ImU32 herb = C(0.24f, 0.46f, 0.16f), ink = C(0.20f, 0.12f, 0.07f);
    /* crisp strokes, as a table's engraved signs (round ends read as a comic's lettering). A line of
       the drawing that turns a corner is one stroke, its corners mitred (two strokes that only touch
       leave a notch at the corner); a stroke that ends on another ends inside it */
    struct Stroke {
        const Pen &p; ImVector<ImVec2> pts; float lx = 0, ly = 0;
        Stroke &to(float x, float y) { ImVec2 q = p.P(x, y); lx = x; ly = y;
            if (!pts.Size || fabsf(pts.back().x - q.x) > 0.05f || fabsf(pts.back().y - q.y) > 0.05f) pts.push_back(q); return *this; }
        static int segs(float units, const Pen &p) { int n = (int)(units * p.u / 2.5f); return n < 10 ? 10 : n > 96 ? 96 : n; }   /* (smooth at any size: a point every few pixels) */
        Stroke &quad(float cx, float cy, float x, float y, int n = 0) { float x0 = lx, y0 = ly;
            if (!n) n = segs(hypotf(cx - x0, cy - y0) + hypotf(x - cx, y - cy), p);
            for (int i = 1; i <= n; i++) { float t = (float)i / n, a = (1 - t) * (1 - t), b = 2 * t * (1 - t), c = t * t; to(a * x0 + b * cx + c * x, a * y0 + b * cy + c * y); } return *this; }
        Stroke &cubic(float c1x, float c1y, float c2x, float c2y, float x, float y, int n = 0) { float x0 = lx, y0 = ly;
            if (!n) n = segs(hypotf(c1x - x0, c1y - y0) + hypotf(c2x - c1x, c2y - c1y) + hypotf(x - c2x, y - c2y), p);
            for (int i = 1; i <= n; i++) { float t = (float)i / n, u = 1 - t;
                to(u * u * u * x0 + 3 * u * u * t * c1x + 3 * u * t * t * c2x + t * t * t * x, u * u * u * y0 + 3 * u * u * t * c1y + 3 * u * t * t * c2y + t * t * t * y); } return *this; }
        Stroke &arc(float cx, float cy, float r, float a0, float a1, int n = 0) {
            if (!n) n = segs(fabsf(a1 - a0) * r, p);
            for (int i = 0; i <= n; i++) { float a = a0 + (a1 - a0) * i / n; to(cx + cosf(a) * r, cy + sinf(a) * r); } return *this; }
        void done(ImU32 k, float w, bool closed = false) {
            if (closed && pts.Size > 2 && fabsf(pts.back().x - pts[0].x) < 0.05f && fabsf(pts.back().y - pts[0].y) < 0.05f) pts.pop_back();
            p.dl->AddPolyline(pts.Data, pts.Size, k, closed ? ImDrawFlags_Closed : 0, p.W(w)); pts.clear(); }
    };
    Stroke S{ p };
    auto L = [&](float x0, float y0, float x1, float y1, ImU32 k, float w = 1.5f) { p.line(x0, y0, x1, y1, k, w); };
    auto ellipse_line = [&](float x, float y, float rx, float ry, float rot, ImU32 k, float w = 1.5f) { p.dl->AddEllipse(p.P(x, y), ImVec2(rx * p.u, ry * p.u), k, rot, 0, p.W(w)); };
    switch (id) {
    /* the metals and their ores: the planets' signs */
    case 72: p.ring(0, 0, 5.5f, gold, W); p.disc(0, 0, 1.4f, gold); break;                                     // Pure Gold: the Sun
    case 89: S.arc(-1.6f, -4.2f, 2.6f, 3.4f, 6.6f, 10).to(-4.6f, 2.8f).to(5.8f, 2.8f).done(steel, W);          // Zinken: Jupiter (tin): the hook, down to the bar, the bar;
        L(2.6f, -2.2f, 2.6f, 7.8f, steel); break;                                                                  //   the stroke through it
    case 87: p.ring(-0.6f, -3.0f, 3.4f, copper, W); L(-0.6f, -0.2f, -0.6f, 7.6f, copper); L(-3.4f, 4.4f, 2.2f, 4.4f, copper);   // Nikel: copper ore (Venus, the ore's ring)
        p.ring(4.6f, 4.4f, 1.3f, copper, 1.1f); break;
    case 88: L(-2, -7, -2, 2.5f, lead); L(-4.6f, -4.4f, 0.6f, -4.4f, lead);                                    // Pitchblende: lead ore (Saturn, the ore's ring)
        S.to(-2, -0.2f).quad(3.8f, -2.2f, 2.6f, 3.6f, 8).quad(1.8f, 6.0f, 4.6f, 7.0f, 8).done(lead, W); p.ring(3.8f, -5.6f, 1.3f, lead, 1.1f); break;
    case 78: p.ring(0, 2.2f, 4.3f, steel, W); L(0, -8.2f, 0, -1.5f, steel); L(-2.4f, -5.6f, 2.4f, -5.6f, steel); break;   // Antimoni: the orb and cross
    case 77: S.to(-4, -5).to(4, -5).to(-4, 5).to(4, 5).done(steel, W); L(-2.5f, 0, 2.5f, 0, steel); break;     // Zincblende: the zinc sign (kept: the player knows it)
    case 73: p.ring(0, -1.5f, 4.0f, steel, W); L(0, -5.5f, 0, 7.5f, steel); L(-3, 5.5f, 3, 5.5f, steel); break;   // Manganes: the "magnesia" sign
    /* the sulphurs, salts and poisons */
    case 90: p.tri_line(-4.2f, -1.5f, 4.2f, -1.5f, 0, -8.0f, sulphur, W);                                    // Brimstone: sulphur, the triangle on the cross
        L(0, -1.5f, 0, 8, sulphur); L(-3.5f, 2.5f, 3.5f, 2.5f, sulphur); L(-2.5f, 5.5f, 2.5f, 5.5f, sulphur); break;
    case 79: p.rect_line(-7.6f, -3.3f, 7.6f, 3.3f, orpi, W, 3.3f); p.ring(-4.3f, 0, 1.25f, orpi, 1.0f); p.ring(4.3f, 0, 1.25f, orpi, 1.0f); break;   // Orpiment: auripigment's sign (each ring in the middle of an end: the same gap all round)
    case 80: p.ring(-0.6f, -1.0f, 2.9f, vermilion, W);                                                     // White Cinnabar: mercury's ore (Mercury, the ore's ring, as Nikel and Pitchblende;
        S.arc(-0.6f, -6.6f, 2.5f, 0.25f, 2.89f).done(vermilion, W);                                           //   the tables' own sign for cinnabar, two barred 3s, read as a number)
        L(-0.6f, 1.6f, -0.6f, 8.0f, vermilion); L(-3.4f, 5.0f, 2.2f, 5.0f, vermilion);
        p.ring(4.4f, 5.0f, 1.3f, vermilion, 1.1f); break;
    case 85: p.rect_line(-3.4f, -0.6f, 3.4f, 6.2f, salt, W); L(-1.3f, -0.6f, -1.3f, -6.4f, salt); L(1.3f, -0.6f, 1.3f, -6.4f, salt); break;   // Alum: alum's sign
    /* the waters, oils and vapours */
    case 82: {                                                                                                // Aqua Regia: its sign, Water's triangle and an R on one stroke
        const float w2 = 1.35f;                                                                                // (the R's stem is the triangle's right side, slanted):
        S.to(2.4f, -4.7f).to(-6.6f, -4.7f).to(-2.0f, 4.6f).to(2.5f, -4.7f).done(water, w2);                    //   the top, the left side, the right side up to the top;
        S.to(2.0f, -4.7f).to(3.0f, -4.7f).cubic(6.8f, -4.7f, 6.8f, 0.2f, 3.0f, 0.2f).to(0.1f, 0.2f).done(water, w2);   //   the bowl, from the top out and back to the stem
        L(3.4f, 0.2f, 6.7f, 4.9f, water, w2); break; }                                                          //   the leg
    case 74: p.ring(0, -3.4f, 2.4f, oil, W); p.ring(-3.6f, 2.8f, 2.4f, oil, W); p.ring(3.6f, 2.8f, 2.4f, oil, W); break;   // Naptha: oil's sign (a rock oil)
    case 83: for (int k = 0; k < 2; k++) { float x = k ? 4.4f : -4.4f;                                    // Gum: gum's sign (two esses, a loop between): each ess two half
            S.arc(x, -2.4f, 2.4f, -0.35f, -4.7124f).arc(x, 2.4f, 2.4f, -1.5708f, 2.79f).done(amber, W); }        //   circles in one stroke (round and smooth at any size)
        p.ring(0, -5.0f, 1.0f, amber, 1.0f); L(0, -4.0f, 0, 0.6f, amber, 1.1f); break;
    case 75: L(0, -6.8f, 0, 7.2f, marsh);                                                                    // Marsh Vapor: putrefaction's sign (the gas of rot): the stem,
        S.to(-4.6f, -6.2f).cubic(-4.6f, 0.4f, 4.4f, 0.4f, 4.4f, -5.0f).done(marsh, W);                       //   the cup one stroke round its foot,
        p.tri(4.4f, -7.8f, 2.6f, -4.6f, 6.2f, -4.6f, marsh); break;                                          //   its right arm ending in a solid point
    /* the herbs: no sign in the alchemists' tables; one made as the tables make theirs (rings, strokes, a
       cross, a fork), abstract, in the same green line (three tries each, herb_variant) */
    case 76:                                                                                                  // Eastern Black Bean
        if (herb_variant == 0) { p.ring(-3.4f, -3.4f, 2.2f, herb, W); p.ring(3.4f, 3.4f, 2.2f, herb, W);       /* two seeds, joined */
            L(-1.8f, -1.8f, 1.8f, 1.8f, herb); }
        else if (herb_variant == 1) { p.ring(-3.6f, -3.6f, 2.0f, herb, W); p.ring(3.6f, 3.6f, 2.0f, herb, W);  /* two seeds, a stroke between */
            L(4.4f, -5.2f, -4.4f, 5.2f, herb); }
        else { p.ring(-2.9f, -0.8f, 2.3f, herb, W); p.ring(2.9f, -0.8f, 2.3f, herb, W);                       /* a pair, on a bar */
            L(-5.8f, 3.6f, 5.8f, 3.6f, herb); }
        break;
    case 81:                                                                                                  // Solanaceae (nightshade)
        if (herb_variant == 0) { p.ring(0, 2.4f, 3.2f, herb, W);                                               /* the berry, its calyx three strokes */
            for (int r = 0; r < 3; r++) { float a = 3.6652f + r * 1.0472f; L(cosf(a) * 0.6f, -0.8f + sinf(a) * 0.6f, cosf(a) * 4.2f, -0.8f + sinf(a) * 4.2f, herb, W); } }
        else if (herb_variant == 1) { p.ring(0, 0, 4.4f, herb, W);                                              /* the berry crossed out: a poison */
            L(-3.1f, -3.1f, 3.1f, 3.1f, herb); L(-3.1f, 3.1f, 3.1f, -3.1f, herb); }
        else { p.ring(0, 0, 4.4f, herb, W); p.ring(0, 0, 1.6f, herb, W); }                                     /* the berry and its gleam */
        break;
    case 84:                                                                                                  // Mondragora (mandrake)
        if (herb_variant == 0) { S.to(-2.6f, -8.0f).to(0, -5.0f).to(2.6f, -8.0f).done(herb, W);                /* leaves, head and forked root: a V, a ring, a fork */
            p.ring(0, -2.6f, 2.0f, herb, W); L(0, -5.4f, 0, -4.6f, herb);
            S.to(-3.0f, 6.6f).to(0, -0.6f).to(3.0f, 6.6f).done(herb, W); }
        else if (herb_variant == 1) { S.to(-3.0f, -7.0f).to(0, -3.0f).to(3.0f, -7.0f).done(herb, W);          /* the root forked at both ends: leaves and legs */
            L(0, -3.6f, 0, 3.0f, herb); S.to(-3.0f, 7.0f).to(0, 2.4f).to(3.0f, 7.0f).done(herb, W); }
        else { p.ring(0, -3.6f, 2.4f, herb, W); S.to(-3.4f, 6.8f).to(0, -1.2f).to(3.4f, 6.8f).done(herb, W); }   /* a head on a forked root */
        break;
    case 86:                                                                                                  // Camomile
        if (herb_variant == 0) { p.ring(0, 0, 2.8f, herb, W);                                                  /* the flower's heart, its petals six dots */
            for (int r = 0; r < 6; r++) { float a = -1.5708f + r * 1.0472f; p.disc(cosf(a) * 5.4f, sinf(a) * 5.4f, 1.1f, herb); } }
        else if (herb_variant == 1) { p.ring(0, 0, 1.5f, herb, 1.1f);                                           /* a heart and four petals, as rings */
            for (int r = 0; r < 4; r++) { float a = r * 1.5708f; p.ring(cosf(a) * 3.8f, sinf(a) * 3.8f, 1.5f, herb, 1.1f); } }
        else { p.ring(0, 0, 4.8f, herb, W);                                                                     /* a star in a ring */
            for (int r = 0; r < 3; r++) { float a = r * 1.0472f; L(cosf(a) * -2.9f, sinf(a) * -2.9f, cosf(a) * 2.9f, sinf(a) * 2.9f, herb, 1.3f); } }
        break;
    /* the bases: the elements, as before */
    default: return false;
    }
    (void)ink;
    return true;
}

/* the ingredients, 72..94 */
bool ui_draw_ingredient(ImDrawList *dl, ImVec2 c, float s, int id)
{
    if (id < 72 || id > 94) return false;
    if (ingredient_style_preview == 1 && id < 91) {   /* (the proposal: the parchment the same, the sign new) */
        Pen q{ dl, c, s / 16.0f * 0.92f };
        q.disc(0, 0, 10.2f, C(0.95f, 0.90f, 0.76f)); q.ring(0, 0, 10.2f, C(0.45f, 0.30f, 0.15f), 0.9f); q.ring(0, 0, 9.0f, C(0.45f, 0.30f, 0.15f, 0.35f), 0.4f);
        if (draw_ingredient_signs(q, id)) return true;
    }
    Pen p{ dl, c, s / 16.0f * 0.92f };            // (the medallion, ring included, inside the icon's square: item pictures are clipped to it)
    ImU32 parch = C(0.95f, 0.90f, 0.76f), edge = C(0.45f, 0.30f, 0.15f);
    ImU32 ink = C(0.20f, 0.12f, 0.07f), red = C(0.66f, 0.12f, 0.08f), gold = C(0.66f, 0.45f, 0.04f), green = C(0.28f, 0.48f, 0.20f), blue = C(0.20f, 0.35f, 0.62f);
    ImU32 sulphur = C(0.62f, 0.46f, 0.02f), fire = C(0.80f, 0.38f, 0.02f);   /* (dark enough on the parchment: contrast 3.5 and more) */
    p.disc(0, 0, 10.2f, parch); p.ring(0, 0, 10.2f, edge, 0.9f); p.ring(0, 0, 9.0f, C(0.45f, 0.30f, 0.15f, 0.35f), 0.4f);
    const float W = 1.5f;                          // the symbols' stroke
    auto cross = [&](float x, float y, float h, ImU32 k) { p.line(x, y - h, x, y + h, k, W); p.line(x - h, y, x + h, y, k, W); };
    switch (id) {
    case 72: p.ring(0, 0, 5.5f, gold, W); p.disc(0, 0, 1.4f, gold); break;                        // Pure Gold: the Sun
    case 73: p.ring(0, -1.5f, 4.0f, ink, W); p.line(0, -5.5f, 0, 7.5f, ink, W); p.line(-3, 5.5f, 3, 5.5f, ink, W); break;   // Manganes: the "magnesia" sign
    case 74: p.tri(-1.8f, 5.0f, 1.8f, 5.0f, 0, -1.0f, C(0.20f, 0.18f, 0.15f)); p.disc(0, 5.6f, 1.9f, C(0.20f, 0.18f, 0.15f));   // Naptha: oil, a drop with a flame
        p.tri(-1.6f, -2.2f, 1.6f, -2.2f, 0, -7.6f, C(0.86f, 0.36f, 0.04f)); p.disc(0, -2.6f, 1.6f, C(0.86f, 0.36f, 0.04f)); break;
    case 75: p.tri_line(-5.5f, 4.5f, 5.5f, 4.5f, 0, -6.0f, blue, W); p.line(-6.8f, 0.6f, 6.8f, 0.6f, blue, W); break;   // Marsh Vapor: Air
    case 76: for (int k = 0; k < 2; k++) { float x = k ? 2.6f : -2.6f, y = k ? 1.4f : -1.4f;     // Eastern Black Bean: two beans
        p.disc(x, y, 3.0f, C(0.10f, 0.08f, 0.08f)); p.arc(x + 0.4f, y, 1.6f, 2.2f, 4.0f, C(0.85f, 0.80f, 0.70f), 0.6f); } break;
    case 77: p.line(-4, -5, 4, -5, ink, W); p.line(4, -5, -4, 5, ink, W); p.line(-4, 5, 4, 5, ink, W); p.line(-2.5f, 0, 2.5f, 0, ink, W); break;   // Zincblende: the zinc sign
    case 78: p.ring(0, 2.0f, 4.3f, ink, W); cross(0, -5.5f, 2.2f, ink); break;                       // Antimoni: the orb and cross
    case 79: p.tri_line(-5, 5, 5, 5, 0, -4.5f, sulphur, W); p.line(-3.2f, 2.0f, 3.2f, 2.0f, sulphur, W);   // Orpiment: arsenic's mark
        p.ring(0, -6.2f, 1.6f, sulphur, 1.0f); break;
    case 80: p.ring(0, 0.5f, 3.2f, red, W); p.arc(0, -5.0f, 2.6f, 0.3f, 2.84f, red, W); cross(0, 6.5f, 1.8f, red); break;   // White Cinnabar: Mercury's sign in vermilion
    case 81: p.bez(-6, 7, -1, 0, 4, -6, green, 1.1f);                                            // Solanaceae: nightshade, berries and leaves
        p.disc(-3.0f, -2.5f, 2.3f, C(0.35f, 0.55f, 0.25f)); p.disc(4.2f, 2.0f, 1.9f, C(0.35f, 0.55f, 0.25f));
        p.disc(-0.5f, 3.6f, 1.7f, C(0.12f, 0.08f, 0.15f)); p.disc(2.2f, 5.6f, 1.6f, C(0.12f, 0.08f, 0.15f)); p.disc(-2.8f, 5.8f, 1.5f, C(0.12f, 0.08f, 0.15f)); break;
    case 82: p.tri_line(-5.5f, -3.0f, 5.5f, -3.0f, 0, 7.0f, blue, W);                           // Aqua Regia: Water crowned (the royal water)
        p.line(-4, -5, 4, -5, gold, 1.2f); p.line(-4, -5, -4.5f, -8, gold, 1.2f); p.line(0, -5, 0, -8.5f, gold, 1.2f); p.line(4, -5, 4.5f, -8, gold, 1.2f); break;
    case 83: p.tri(-3.2f, 3.0f, 3.2f, 3.0f, 0, -6.5f, C(0.74f, 0.42f, 0.06f)); p.disc(0, 3.4f, 3.3f, C(0.74f, 0.42f, 0.06f));   // Gum: a tear of resin
        p.disc(-1.2f, 2.0f, 0.9f, C(1, 0.95f, 0.75f, 0.8f)); break;
    case 84: { ImU32 root = C(0.62f, 0.45f, 0.28f);                                              // Mondragora: the mandrake, a little man of root
        p.disc(0, -2.5f, 1.8f, root); p.rect(-1.6f, -1.0f, 1.6f, 3.5f, root, 1.0f);
        p.line(-1.4f, 0, -4.2f, 2.5f, root, 1.1f); p.line(1.4f, 0, 4.2f, 2.5f, root, 1.1f);
        p.line(-0.8f, 3.4f, -2.4f, 7.5f, root, 1.2f); p.line(0.8f, 3.4f, 2.4f, 7.5f, root, 1.2f);
        for (int l = 0; l < 3; l++) p.bez(0, -4.0f, -3.0f + l * 3.0f, -7.0f, -3.5f + l * 3.5f, -8.0f, green, 1.1f); break; }
    case 85: p.ring(0, 0, 5.0f, ink, W); cross(0, 0, 5.0f, ink);                                    // Alum: the quartered circle, a grain in each
        p.disc(-2.4f, -2.4f, 0.8f, ink); p.disc(2.4f, -2.4f, 0.8f, ink); p.disc(-2.4f, 2.4f, 0.8f, ink); p.disc(2.4f, 2.4f, 0.8f, ink); break;
    case 86: p.bez(0, 8, 0.5f, 3, 0, 1.5f, green, 1.0f);                                           // Camomile: the flower
        for (int k = 0; k < 10; k++) { float a = k * 0.6283f; p.disc(cosf(a) * 3.3f, -1.5f + sinf(a) * 3.3f, 1.7f, C(0.45f, 0.36f, 0.22f)); }   /* the petals' outline */
        for (int k = 0; k < 10; k++) { float a = k * 0.6283f; p.disc(cosf(a) * 3.3f, -1.5f + sinf(a) * 3.3f, 1.25f, C(1.0f, 1.0f, 0.98f)); }
        p.disc(0, -1.5f, 2.0f, C(0.88f, 0.60f, 0.04f)); p.ring(0, -1.5f, 2.0f, C(0.45f, 0.30f, 0.10f), 0.4f); break;
    case 87: p.ring(0, -1.8f, 3.4f, C(0.70f, 0.40f, 0.15f), W); cross(0, 4.6f, 2.2f, C(0.70f, 0.40f, 0.15f));   // Nikel: Venus's copper sign with the devil's horns ("Kupfernickel")
        p.bez(-2.2f, -4.4f, -3.8f, -6.4f, -2.6f, -8.2f, red, 1.0f); p.bez(2.2f, -4.4f, 3.8f, -6.4f, 2.6f, -8.2f, red, 1.0f); break;
    case 88: p.disc(0, 0.5f, 5.6f, C(0.12f, 0.11f, 0.10f));                                       // Pitchblende: a black, pitch-bright lump
        p.disc(-2.0f, -1.6f, 1.2f, C(0.55f, 0.55f, 0.50f, 0.7f)); p.line(-1.5f, 3.0f, 3.0f, -0.6f, C(0.40f, 0.36f, 0.30f), 0.6f); break;
    case 89: p.line(-1, -6, -1, 6, ink, W); p.line(-1, 2, 5, 2, ink, W); p.bez(-1, -6, 4.5f, -5.5f, -4, -1, ink, W); break;   // Zinken: Jupiter's sign (tin and zinc)
    case 90: p.tri_line(-4.2f, -1.5f, 4.2f, -1.5f, 0, -8.0f, sulphur, W);         // Brimstone: sulphur, the triangle on the cross
        p.line(0, -1.5f, 0, 8, sulphur, W); p.line(-3.5f, 2.5f, 3.5f, 2.5f, sulphur, W); p.line(-2.5f, 5.5f, 2.5f, 5.5f, sulphur, W); break;
    case 91: p.tri_line(-5.5f, 4.5f, 5.5f, 4.5f, 0, -6.0f, fire, W); break;      // Choleric Base: Fire (yellow bile)
    case 92: p.tri_line(-5.5f, -4.5f, 5.5f, -4.5f, 0, 6.0f, blue, W); break;                       // Phlegmatic Base: Water
    case 93: p.tri_line(-5.5f, 4.5f, 5.5f, 4.5f, 0, -6.0f, red, W); p.line(-6.8f, 0.6f, 6.8f, 0.6f, red, W); break;   // Sanguine Base: Air (blood)
    case 94: p.tri_line(-5.5f, -4.5f, 5.5f, -4.5f, 0, 6.0f, ink, W); p.line(-6.8f, -0.6f, 6.8f, -0.6f, ink, W); break;   // Melancholic Base: Earth (black bile)
    }
    return true;
}

/* the grade of a potion of quality q (base: the item's own quality in DARKLAND.LST) */
int ui_potion_tier(int q, int base)
{
    if (q < 0) return 0;
    int t = (int)floorf((q - base) / 10.0f + 0.5f) + 1;
    return t < 1 ? 1 : t > 3 ? 3 : t;
}
