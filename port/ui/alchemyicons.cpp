// alchemyicons.cpp - the pictures of the potions and of the alchemical ingredients, drawn with ImGui's
// shapes (no image files).
//
// Potions (items 95-116): one round-bellied flask for all, so a potion reads as a potion at a glance,
// with the liquid in the potion's own colour and its emblem: what it does, in the manner of a
// fifteenth-century woodcut (a green cloud for Noxious Aroma, a bolt for Thunderbolt, a winged foot
// for Quickmove ...). Its grade is told by one, two or three gold dots down the neck: a formula of
// grade g brews quality base + 10 g (s029 0e76:3fb2), so the dots are (quality - base) / 10 + 1.
//
// Ingredients (items 72-94): a parchment roundel with the symbol the alchemists' tables give it (the
// planets' signs for the metals, the elements' triangles for the four humours' bases, sulphur's
// triangle and cross ...), or for the herbs and roots a small drawing as in a herbal (the mandrake
// a little man).
#include "imgui.h"
#include <math.h>

namespace {
ImU32 C(float r, float g, float b, float a = 1) { return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a)); }

struct Pen {
    ImDrawList *dl; ImVec2 c; float u;
    ImVec2 P(float x, float y) const { return ImVec2(c.x + x * u, c.y + y * u); }
    void line(float x0, float y0, float x1, float y1, ImU32 k, float w) const { dl->AddLine(P(x0, y0), P(x1, y1), k, w * u); }
    void disc(float x, float y, float r, ImU32 k) const { dl->AddCircleFilled(P(x, y), r * u, k, 0); }
    void ring(float x, float y, float r, ImU32 k, float w) const { dl->AddCircle(P(x, y), r * u, k, 0, w * u); }
    void arc(float x, float y, float r, float a0, float a1, ImU32 k, float w) const { dl->PathArcTo(P(x, y), r * u, a0, a1, 18); dl->PathStroke(k, 0, w * u); }
    void tri(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k) const { dl->AddTriangleFilled(P(x0, y0), P(x1, y1), P(x2, y2), k); }
    void tri_line(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k, float w) const { dl->AddTriangle(P(x0, y0), P(x1, y1), P(x2, y2), k, w * u); }
    void quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, ImU32 k) const { dl->AddQuadFilled(P(x0, y0), P(x1, y1), P(x2, y2), P(x3, y3), k); }
    void bez(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k, float w) const { dl->AddBezierQuadratic(P(x0, y0), P(x1, y1), P(x2, y2), k, w * u); }
    void rect(float x0, float y0, float x1, float y1, ImU32 k, float r = 0) const { dl->AddRectFilled(P(x0, y0), P(x1, y1), k, r * u); }
};

const ImU32 INK = 0, GLASS_LINE = 1;     // (placeholders: the colours are made where used)

/* the liquid of each potion, 95..116 */
const float LIQ[22][3] = {
    { 0.45f, 0.62f, 0.20f },   // Noxious Aroma: bilious green
    { 0.85f, 0.40f, 0.15f },   // Eyeburn: pepper red-orange
    { 0.22f, 0.22f, 0.26f },   // Black Cloud
    { 0.66f, 0.55f, 0.35f },   // Fleadust: dun
    { 0.70f, 0.78f, 0.25f },   // Eater-Water: acid yellow-green
    { 0.45f, 0.25f, 0.50f },   // Breath of Death: purple
    { 0.98f, 0.80f, 0.25f },   // Sunburst: gold
    { 0.45f, 0.62f, 0.90f },   // Thunderbolt: storm blue
    { 0.90f, 0.30f, 0.12f },   // Arabian Fire
    { 0.16f, 0.12f, 0.10f },   // Stone-tar
    { 0.62f, 0.10f, 0.12f },   // Deadly Blade: blood
    { 0.62f, 0.66f, 0.74f },   // Strongedge: steel
    { 0.80f, 0.45f, 0.18f },   // Great Power: amber
    { 0.30f, 0.55f, 0.35f },   // Trueflight: green
    { 0.50f, 0.52f, 0.58f },   // Hardarmor: iron
    { 0.75f, 0.45f, 0.70f },   // Transformation: rose
    { 0.30f, 0.50f, 0.85f },   // Truesight: clear blue
    { 0.70f, 0.85f, 0.90f },   // New-wind: pale sky
    { 0.40f, 0.40f, 0.45f },   // Ironarm
    { 0.95f, 0.70f, 0.30f },   // Quickmove: quicksilver-gold
    { 0.95f, 0.92f, 0.75f },   // Essence o'Grace: pearl
    { 0.85f, 0.25f, 0.10f },   // Firewall
};

/* the flask: a round belly on the lower part, a neck and a cork; `fill` how full */
void flask(const Pen &p, ImU32 liq, float fill = 0.62f)
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
    float r = fmaxf(1.3f, 2.0f / p.u), step = fmaxf(3.3f, 2 * r + 0.5f / p.u);
    for (int k = 0; k < tier; k++) {
        float x = 5.4f + (r - 1.3f) * 0.8f, y = -10.6f + k * step;
        p.disc(x, y, r, C(0.98f, 0.80f, 0.20f));
        p.ring(x, y, r, C(0.30f, 0.18f, 0.06f), fmaxf(0.45f, 0.8f / p.u));
    }
}
void puff(const Pen &p, float x, float y, float r, ImU32 k) { p.disc(x, y, r, k); }
void flame(const Pen &p, float x, float y, float h, ImU32 outer, ImU32 inner)
{
    p.tri(x - h * 0.35f, y, x + h * 0.35f, y, x, y - h, outer);
    p.disc(x, y - h * 0.08f, h * 0.36f, outer);
    p.tri(x - h * 0.18f, y, x + h * 0.18f, y, x, y - h * 0.6f, inner);
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
    case 95: flask(p, liq);                                           // Noxious Aroma: green fumes from the neck
        puff(p, -3.4f, -10.6f, 2.4f, C(0.45f, 0.70f, 0.25f, 0.85f)); puff(p, 0.6f, -12.4f, 2.2f, C(0.55f, 0.78f, 0.30f, 0.85f)); puff(p, -0.2f, -9.6f, 1.6f, C(0.40f, 0.62f, 0.20f, 0.85f)); break;
    case 96: flask(p, liq); eye(p, 0, 3.0f, 4.2f, white, C(0.85f, 0.15f, 0.10f));   // Eyeburn: a burning eye
        for (int r = 0; r < 5; r++) { float a = -2.4f + r * 0.45f; p.line(cosf(a) * 5.2f, 3 + sinf(a) * 5.2f, cosf(a) * 6.6f, 3 + sinf(a) * 6.6f, C(1, 0.85f, 0.3f), 0.8f); } break;
    case 97: flask(p, liq);                                           // Black Cloud: black smoke
        puff(p, -3.4f, -10.6f, 2.6f, C(0.10f, 0.10f, 0.12f, 0.92f)); puff(p, 0.8f, -12.4f, 2.4f, C(0.15f, 0.15f, 0.17f, 0.92f)); puff(p, -0.4f, -9.4f, 1.8f, C(0.08f, 0.08f, 0.10f, 0.92f)); break;
    case 98: flask(p, liq);                                           // Fleadust: a cloud of motes and fleas
        for (int i = 0; i < 8; i++) { float x = -6.0f + (i * 37 % 9), y = -12.6f + (i * 23 % 4); if (fabsf(x) > 2.4f || y < -11.8f) p.disc(x, y, 0.65f, C(0.35f, 0.25f, 0.15f)); }
        p.disc(3.6f, 2.5f, 1.3f, ink); p.line(3.6f, 2.5f, 5.4f, 1.2f, ink, 0.5f); p.line(3.6f, 2.5f, 5.6f, 3.6f, ink, 0.5f); p.disc(-2.8f, 5.0f, 1.1f, ink); break;
    case 99: flask(p, liq);                                           // Eater-Water: a drop that eats into a bar
        p.tri(-1.6f, 3.6f, 1.6f, 3.6f, 0, -0.6f, C(0.95f, 1.0f, 0.55f)); p.disc(0, 4.4f, 1.65f, C(0.95f, 1.0f, 0.55f));   /* a corrosive drop over a pitted bar */
        p.rect(-4.5f, 7.0f, 4.5f, 8.6f, C(0.55f, 0.56f, 0.60f)); p.disc(-1.0f, 7.1f, 0.8f, C(0.15f, 0.12f, 0.10f)); p.disc(1.4f, 7.0f, 0.6f, C(0.15f, 0.12f, 0.10f)); break;
    case 100: flask(p, liq);                                          // Breath of Death: a skull
        p.disc(0, 2.2f, 3.3f, white); p.rect(-1.9f, 4.0f, 1.9f, 6.6f, white, 0.6f);
        p.disc(-1.3f, 2.2f, 0.95f, ink); p.disc(1.3f, 2.2f, 0.95f, ink); p.tri(-0.5f, 4.4f, 0.5f, 4.4f, 0, 3.5f, ink);
        p.line(-0.9f, 6.6f, -0.9f, 5.4f, ink, 0.4f); p.line(0.9f, 6.6f, 0.9f, 5.4f, ink, 0.4f); break;
    case 101: flask(p, liq);                                          // Sunburst: a sun with a face of rays
        for (int r = 0; r < 12; r++) { float a = r * 0.5236f; p.line(cosf(a) * 3.4f, 3 + sinf(a) * 3.4f, cosf(a) * (r & 1 ? 5.4f : 6.4f), 3 + sinf(a) * (r & 1 ? 5.4f : 6.4f), C(0.95f, 0.55f, 0.10f), 0.8f); }
        p.disc(0, 3.0f, 3.0f, gold); p.ring(0, 3.0f, 3.0f, C(0.70f, 0.40f, 0.08f), 0.5f); break;
    case 102: flask(p, liq);                                          // Thunderbolt: a bolt bursting from the glass
        p.dl->PathLineTo(p.P(2.5f, -6)); p.dl->PathLineTo(p.P(-2.5f, 2.5f)); p.dl->PathLineTo(p.P(0.6f, 2.5f)); p.dl->PathLineTo(p.P(-2.8f, 10.5f));
        p.dl->PathLineTo(p.P(3.8f, 0.5f)); p.dl->PathLineTo(p.P(0.4f, 0.5f)); p.dl->PathLineTo(p.P(3.5f, -6)); p.dl->PathFillConcave(C(1.0f, 0.92f, 0.35f));
        for (int r = 0; r < 4; r++) { float a = -0.7f + r * 0.5f; p.line(6.2f + cosf(a) * 1.0f, -3 + sinf(a) * 1.0f, 6.2f + cosf(a) * 3.0f, -3 + sinf(a) * 3.0f, C(1.0f, 0.92f, 0.35f), 0.8f); } break;
    case 103: flask(p, liq);                                          // Arabian Fire: flames out of the neck
        flame(p, -3.6f, -8.0f, 5.0f, C(0.95f, 0.45f, 0.10f, 0.95f), C(1.0f, 0.85f, 0.30f));
        flame(p, -0.6f, -9.4f, 3.8f, C(0.90f, 0.25f, 0.08f, 0.95f), C(1.0f, 0.75f, 0.25f)); break;
    case 104: flask(p, liq, 0.8f);                                    // Stone-tar: tar dripping on stones
        p.bez(1.9f, -5, 3.8f, -4, 3.6f, 0, C(0.12f, 0.09f, 0.08f), 1.4f); p.disc(3.6f, 0.4f, 1.0f, C(0.12f, 0.09f, 0.08f));
        p.disc(-7.0f, 9.8f, 1.6f, C(0.55f, 0.52f, 0.48f)); p.disc(6.8f, 9.6f, 1.4f, C(0.50f, 0.48f, 0.45f)); break;
    case 105: flask(p, liq);                                          // Deadly Blade: a blade and a drop of blood
        p.quad(-6.4f, 9.4f, -4.6f, 10.4f, 6.0f, -0.6f, 4.4f, -2.0f, ink); p.quad(-6, 9, -5, 10, 5.5f, -0.5f, 4.5f, -1.5f, C(0.85f, 0.87f, 0.90f)); p.line(-6.8f, 7.4f, -3.6f, 10.6f, C(0.55f, 0.40f, 0.15f), 1.1f);
        p.tri(4.8f, 3.4f, 6.8f, 3.4f, 5.8f, 0.8f, red); p.disc(5.8f, 4.0f, 1.05f, red); break;
    case 106: flask(p, liq);                                          // Strongedge: a blade with a gleaming edge
        p.quad(-6.4f, 9.4f, -4.6f, 10.4f, 6.0f, -0.6f, 4.4f, -2.0f, ink); p.quad(-6, 9, -5, 10, 5.5f, -0.5f, 4.5f, -1.5f, C(0.85f, 0.87f, 0.90f)); p.line(-6.8f, 7.4f, -3.6f, 10.6f, C(0.55f, 0.40f, 0.15f), 1.1f);
        for (int r = 0; r < 4; r++) { float a = r * 1.5708f; p.line(4.6f, -1.2f, 4.6f + cosf(a) * 2.8f, -1.2f + sinf(a) * 2.8f, gold, 0.7f); } break;
    case 107: flask(p, liq);                                          // Great Power: a broken chain
        p.ring(-2.6f, 3.0f, 2.1f, ink, 1.0f); p.arc(2.4f, 3.0f, 2.1f, 0.5f, 5.0f, ink, 1.0f);
        for (int r = 0; r < 3; r++) { float a = -0.6f + r * 0.6f; p.line(5.0f + cosf(a) * 0.8f, 3 + sinf(a) * 0.8f, 5.0f + cosf(a) * 2.4f, 3 + sinf(a) * 2.4f, gold, 0.6f); } break;
    case 108: flask(p, liq);                                          // Trueflight: an arrow in the target's eye
        p.ring(0, 3.0f, 4.2f, white, 1.0f); p.ring(0, 3.0f, 2.2f, red, 1.0f); p.disc(0, 3.0f, 0.9f, red);
        p.line(0, 3.0f, 7.5f, -4.5f, C(0.55f, 0.38f, 0.18f), 0.8f); p.line(7.5f, -4.5f, 8.8f, -4.4f, white, 0.6f); p.line(7.5f, -4.5f, 7.6f, -5.8f, white, 0.6f); break;
    case 109: flask(p, liq);                                          // Hardarmor: a heater shield
        p.dl->PathLineTo(p.P(-3.6f, -0.5f)); p.dl->PathLineTo(p.P(3.6f, -0.5f)); p.dl->PathLineTo(p.P(3.6f, 3.6f)); p.dl->PathLineTo(p.P(0, 7.6f)); p.dl->PathLineTo(p.P(-3.6f, 3.6f));
        p.dl->PathFillConvex(C(0.82f, 0.84f, 0.88f)); p.line(0, -0.5f, 0, 7.4f, red, 0.9f); p.line(-3.5f, 2.0f, 3.5f, 2.0f, red, 0.9f); break;
    case 110: flask(p, liq);                                          // Transformation: a butterfly, the soul changed
        p.disc(-2.0f, 1.6f, 2.0f, C(0.95f, 0.75f, 0.30f)); p.disc(2.0f, 1.6f, 2.0f, C(0.95f, 0.75f, 0.30f));
        p.disc(-1.6f, 4.6f, 1.4f, C(0.80f, 0.35f, 0.45f)); p.disc(1.6f, 4.6f, 1.4f, C(0.80f, 0.35f, 0.45f));
        p.line(0, 0.2f, 0, 6.0f, ink, 0.7f); p.line(0, 0.4f, -1.2f, -1.4f, ink, 0.4f); p.line(0, 0.4f, 1.2f, -1.4f, ink, 0.4f); break;
    case 111: flask(p, liq); eye(p, 0, 3.0f, 4.4f, white, C(0.30f, 0.55f, 0.90f));   // Truesight: the open eye, rays above
        for (int r = 0; r < 5; r++) { float a = -2.6f + r * 0.4f; p.line(cosf(a) * 4.6f, 3 + sinf(a) * 4.6f, cosf(a) * 6.4f, 3 + sinf(a) * 6.4f, gold, 0.6f); } break;
    case 112: flask(p, liq);                                          // New-wind: the breath of a wind's head
        for (int r = 0; r < 3; r++) { float y = 1.0f + r * 2.2f; p.bez(-5.0f, y, 0, y - 1.6f, 4.0f, y, white, 0.9f); p.arc(4.0f, y - 0.9f, 0.9f, -1.2f, 2.6f, white, 0.7f); }
        break;
    case 113: flask(p, liq);                                          // Ironarm: a mailed fist
        p.rect(-3.2f, 0.6f, 3.0f, 5.4f, C(0.78f, 0.80f, 0.85f), 1.2f); for (int f = 0; f < 3; f++) p.line(-1.6f + f * 1.6f, 0.8f, -1.6f + f * 1.6f, 3.0f, ink, 0.5f);
        p.rect(-2.4f, 5.4f, 2.2f, 8.0f, C(0.60f, 0.62f, 0.68f)); p.disc(3.4f, 3.0f, 1.2f, C(0.78f, 0.80f, 0.85f)); break;
    case 114: flask(p, liq);                                          // Quickmove: Mercury's winged foot
        p.quad(-3.5f, 6.5f, 3.8f, 6.5f, 3.8f, 4.6f, -1.0f, 3.4f, C(0.55f, 0.36f, 0.20f)); p.rect(-3.5f, 0.0f, -1.0f, 6.0f, C(0.55f, 0.36f, 0.20f));
        for (int f = 0; f < 3; f++) p.bez(-1.0f, 1.0f + f * 1.2f, 2.5f, -1.5f + f * 1.0f, 5.5f, -2.5f + f * 1.5f, white, 0.8f); break;
    case 115: flask(p, liq);                                          // Essence o'Grace: a dove under a halo
        p.ring(0, -0.8f, 3.0f, gold, 0.9f);
        p.disc(0, 4.0f, 1.9f, white); p.tri(-5.0f, 2.2f, -0.5f, 3.6f, -1.0f, 5.0f, white); p.tri(5.0f, 2.2f, 0.5f, 3.6f, 1.0f, 5.0f, white); p.disc(1.6f, 3.0f, 0.9f, white); break;
    case 116: flask(p, liq);                                          // Firewall: a wall of stone with flames over it
        p.rect(-5.6f, 3.5f, 5.6f, 8.5f, C(0.55f, 0.45f, 0.38f));
        p.line(-5.6f, 6.0f, 5.6f, 6.0f, ink, 0.5f); p.line(-1.8f, 3.5f, -1.8f, 6.0f, ink, 0.5f); p.line(2.0f, 3.5f, 2.0f, 6.0f, ink, 0.5f); p.line(0, 6.0f, 0, 8.5f, ink, 0.5f); p.line(-3.8f, 6.0f, -3.8f, 8.5f, ink, 0.5f); p.line(3.8f, 6.0f, 3.8f, 8.5f, ink, 0.5f);
        flame(p, -3.0f, 3.5f, 4.0f, C(0.95f, 0.45f, 0.10f), C(1.0f, 0.85f, 0.30f)); flame(p, 1.0f, 3.5f, 5.0f, C(0.95f, 0.40f, 0.10f), C(1.0f, 0.85f, 0.30f)); flame(p, 4.4f, 3.5f, 3.4f, C(0.95f, 0.45f, 0.10f), C(1.0f, 0.85f, 0.30f)); break;
    }
    grade_dots(p, tier);
    (void)INK; (void)GLASS_LINE;
    return true;
}

/* the ingredients, 72..94 */
bool ui_draw_ingredient(ImDrawList *dl, ImVec2 c, float s, int id)
{
    if (id < 72 || id > 94) return false;
    Pen p{ dl, c, s / 16.0f };
    ImU32 parch = C(0.95f, 0.90f, 0.76f), edge = C(0.45f, 0.30f, 0.15f);
    ImU32 ink = C(0.20f, 0.12f, 0.07f), red = C(0.66f, 0.12f, 0.08f), gold = C(0.80f, 0.58f, 0.10f), green = C(0.28f, 0.48f, 0.20f), blue = C(0.20f, 0.35f, 0.62f);
    p.disc(0, 0, 10.2f, parch); p.ring(0, 0, 10.2f, edge, 0.9f); p.ring(0, 0, 9.0f, C(0.45f, 0.30f, 0.15f, 0.35f), 0.4f);
    const float W = 1.5f;                          // the symbols' stroke
    auto cross = [&](float x, float y, float h, ImU32 k) { p.line(x, y - h, x, y + h, k, W); p.line(x - h, y, x + h, y, k, W); };
    switch (id) {
    case 72: p.ring(0, 0, 5.5f, gold, W); p.disc(0, 0, 1.4f, gold); break;                        // Pure Gold: the Sun
    case 73: p.ring(0, -1.5f, 4.0f, ink, W); p.line(0, -5.5f, 0, 7.5f, ink, W); p.line(-3, 5.5f, 3, 5.5f, ink, W); break;   // Manganes: the "magnesia" sign
    case 74: p.tri(-1.8f, 5.0f, 1.8f, 5.0f, 0, -1.0f, C(0.20f, 0.18f, 0.15f)); p.disc(0, 5.6f, 1.9f, C(0.20f, 0.18f, 0.15f));   // Naptha: oil, a drop with a flame
        p.tri(-1.6f, -2.2f, 1.6f, -2.2f, 0, -7.6f, C(0.92f, 0.45f, 0.10f)); p.disc(0, -2.6f, 1.6f, C(0.92f, 0.45f, 0.10f)); break;
    case 75: p.tri_line(-5.5f, 4.5f, 5.5f, 4.5f, 0, -6.0f, blue, W); p.line(-6.8f, 0.6f, 6.8f, 0.6f, blue, W); break;   // Marsh Vapor: Air
    case 76: for (int k = 0; k < 2; k++) { float x = k ? 2.6f : -2.6f, y = k ? 1.4f : -1.4f;     // Eastern Black Bean: two beans
        p.disc(x, y, 3.0f, C(0.10f, 0.08f, 0.08f)); p.arc(x + 0.4f, y, 1.6f, 2.2f, 4.0f, C(0.85f, 0.80f, 0.70f), 0.6f); } break;
    case 77: p.line(-4, -5, 4, -5, ink, W); p.line(4, -5, -4, 5, ink, W); p.line(-4, 5, 4, 5, ink, W); p.line(-2.5f, 0, 2.5f, 0, ink, W); break;   // Zincblende: the zinc sign
    case 78: p.ring(0, 2.0f, 4.3f, ink, W); cross(0, -5.5f, 2.2f, ink); break;                       // Antimoni: the orb and cross
    case 79: p.tri_line(-5, 5, 5, 5, 0, -4.5f, C(0.82f, 0.62f, 0.10f), W); p.line(-3.2f, 2.0f, 3.2f, 2.0f, C(0.82f, 0.62f, 0.10f), W);   // Orpiment: arsenic's mark
        p.ring(0, -6.2f, 1.6f, C(0.82f, 0.62f, 0.10f), 1.0f); break;
    case 80: p.ring(0, 0.5f, 3.2f, red, W); p.arc(0, -5.0f, 2.6f, 0.3f, 2.84f, red, W); cross(0, 6.5f, 1.8f, red); break;   // White Cinnabar: Mercury's sign in vermilion
    case 81: p.bez(-6, 7, -1, 0, 4, -6, green, 1.1f);                                            // Solanaceae: nightshade, berries and leaves
        p.disc(-3.0f, -2.5f, 2.3f, C(0.35f, 0.55f, 0.25f)); p.disc(4.2f, 2.0f, 1.9f, C(0.35f, 0.55f, 0.25f));
        p.disc(-0.5f, 3.6f, 1.7f, C(0.12f, 0.08f, 0.15f)); p.disc(2.2f, 5.6f, 1.6f, C(0.12f, 0.08f, 0.15f)); p.disc(-2.8f, 5.8f, 1.5f, C(0.12f, 0.08f, 0.15f)); break;
    case 82: p.tri_line(-5.5f, -3.0f, 5.5f, -3.0f, 0, 7.0f, blue, W);                           // Aqua Regia: Water crowned (the royal water)
        p.line(-4, -5, 4, -5, gold, 1.2f); p.line(-4, -5, -4.5f, -8, gold, 1.2f); p.line(0, -5, 0, -8.5f, gold, 1.2f); p.line(4, -5, 4.5f, -8, gold, 1.2f); break;
    case 83: p.tri(-3.2f, 3.0f, 3.2f, 3.0f, 0, -6.5f, C(0.85f, 0.55f, 0.15f)); p.disc(0, 3.4f, 3.3f, C(0.85f, 0.55f, 0.15f));   // Gum: a tear of resin
        p.disc(-1.2f, 2.0f, 0.9f, C(1, 0.95f, 0.75f, 0.8f)); break;
    case 84: { ImU32 root = C(0.62f, 0.45f, 0.28f);                                              // Mondragora: the mandrake, a little man of root
        p.disc(0, -2.5f, 1.8f, root); p.rect(-1.6f, -1.0f, 1.6f, 3.5f, root, 1.0f);
        p.line(-1.4f, 0, -4.2f, 2.5f, root, 1.1f); p.line(1.4f, 0, 4.2f, 2.5f, root, 1.1f);
        p.line(-0.8f, 3.4f, -2.4f, 7.5f, root, 1.2f); p.line(0.8f, 3.4f, 2.4f, 7.5f, root, 1.2f);
        for (int l = 0; l < 3; l++) p.bez(0, -4.0f, -3.0f + l * 3.0f, -7.0f, -3.5f + l * 3.5f, -8.0f, green, 1.1f); break; }
    case 85: p.ring(0, 0, 5.0f, ink, W); cross(0, 0, 5.0f, ink);                                    // Alum: the quartered circle, a grain in each
        p.disc(-2.4f, -2.4f, 0.8f, ink); p.disc(2.4f, -2.4f, 0.8f, ink); p.disc(-2.4f, 2.4f, 0.8f, ink); p.disc(2.4f, 2.4f, 0.8f, ink); break;
    case 86: p.bez(0, 8, 0.5f, 3, 0, 1.5f, green, 1.0f);                                           // Camomile: the flower
        for (int k = 0; k < 10; k++) { float a = k * 0.6283f; p.disc(cosf(a) * 3.3f, -1.5f + sinf(a) * 3.3f, 1.25f, C(0.99f, 0.98f, 0.95f)); }
        p.disc(0, -1.5f, 2.0f, C(0.95f, 0.75f, 0.15f)); break;
    case 87: p.ring(0, -1.8f, 3.4f, C(0.70f, 0.40f, 0.15f), W); cross(0, 4.6f, 2.2f, C(0.70f, 0.40f, 0.15f));   // Nikel: Venus's copper sign with the devil's horns ("Kupfernickel")
        p.bez(-2.2f, -4.4f, -3.8f, -6.4f, -2.6f, -8.2f, red, 1.0f); p.bez(2.2f, -4.4f, 3.8f, -6.4f, 2.6f, -8.2f, red, 1.0f); break;
    case 88: p.disc(0, 0.5f, 5.6f, C(0.12f, 0.11f, 0.10f));                                       // Pitchblende: a black, pitch-bright lump
        p.disc(-2.0f, -1.6f, 1.2f, C(0.55f, 0.55f, 0.50f, 0.7f)); p.line(-1.5f, 3.0f, 3.0f, -0.6f, C(0.40f, 0.36f, 0.30f), 0.6f); break;
    case 89: p.line(-1, -6, -1, 6, ink, W); p.line(-1, 2, 5, 2, ink, W); p.bez(-1, -6, 4.5f, -5.5f, -4, -1, ink, W); break;   // Zinken: Jupiter's sign (tin and zinc)
    case 90: p.tri_line(-4.2f, -1.5f, 4.2f, -1.5f, 0, -8.0f, C(0.82f, 0.62f, 0.10f), W);         // Brimstone: sulphur, the triangle on the cross
        p.line(0, -1.5f, 0, 8, C(0.82f, 0.62f, 0.10f), W); p.line(-3.5f, 2.5f, 3.5f, 2.5f, C(0.82f, 0.62f, 0.10f), W); p.line(-2.5f, 5.5f, 2.5f, 5.5f, C(0.82f, 0.62f, 0.10f), W); break;
    case 91: p.tri_line(-5.5f, 4.5f, 5.5f, 4.5f, 0, -6.0f, C(0.90f, 0.60f, 0.10f), W); break;      // Choleric Base: Fire (yellow bile)
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
