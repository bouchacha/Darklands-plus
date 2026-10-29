/* pointer.cpp - the game's pointer drawn anew (display.pointer_drawn): its armored gauntlet in plate,
 * lames and rivets, lit from the upper left, drawn at any size in place of the game's little sprite
 * (COMMONSP.IMG's first, cursor_default; in a battle the two of BATTLEGR's, unpaused and paused).
 * The hand:
 *   POINT  the gauntlet's forefinger out, its tip on the spot (the game's own);
 *   GRAB   a fist (the button held and the pointer moved: something dragged, the map or a thing);
 * and on its cuff what the pointer is about: the pause's two gold bars (a battle paused: the game's
 * "P"), two crossed blades (in a battle, over a foe: a click attacks, as the cards' fights), an
 * hourglass (Go to walking the party through the streets).
 * ui/overlay_text.cpp draws it where the game's sprite would be (its box, its hot spot); a sprite
 * of the game's this does not know (the sheet's hands, the arrows at the battle's edges) is drawn
 * as the game's. Coordinates in the sprite's pixels (26 x 15, the tip at 0,0), k window pixels to one.
 * (Redrawn for 1.4-RC6 in proportion: RC5's had a forefinger as thick as a thumb, a fist as tall as the hand
 * was long and a cuff wider still - the maintainer: "like a child hastily drew it".) */
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include "pointer.h"

namespace {
const ImU32 OUT = IM_COL32(22, 18, 24, 255), HI = IM_COL32(246, 248, 252, 255), LO = IM_COL32(78, 84, 100, 255),
            GAP = IM_COL32(40, 38, 48, 255), LEATHER = IM_COL32(120, 76, 40, 255), GOLD = IM_COL32(236, 190, 70, 255);
struct G {
    ImDrawList *dl; ImVec2 o; float k;
    ImVector<ImVec2> v;
    ImVec2 P(float x, float y) const { return ImVec2(o.x + x * k, o.y + y * k); }
    float W(float w) const { return fmaxf(1.0f, w * k); }
    /* a path in the sprite's pixels: straight runs and quadratic curves (c a control point) */
    G &at(float x, float y) { v.clear(); v.push_back(P(x, y)); return *this; }
    G &to(float x, float y) { v.push_back(P(x, y)); return *this; }
    G &curve(float cx, float cy, float x, float y, int n = 6)
    {
        ImVec2 a = v.back(), c = P(cx, cy), b = P(x, y);
        for (int i = 1; i <= n; i++) { float t = (float)i / n, u = 1 - t; v.push_back(ImVec2(u * u * a.x + 2 * u * t * c.x + t * t * b.x, u * u * a.y + 2 * u * t * c.y + t * t * b.y)); }
        return *this;
    }
    /* the path as a plate: its shadow, then filled and shaded from lit (top) to dark (bottom), its edge */
    void plate(float y_lit, float y_dark, ImU32 lit = HI, ImU32 dark = LO)
    {
        ImVector<ImVec2> sh; for (const ImVec2 &p : v) sh.push_back(ImVec2(p.x + 0.7f * k, p.y + 0.9f * k));
        dl->AddConcavePolyFilled(sh.Data, sh.Size, IM_COL32(0, 0, 0, 70));
        int v0 = dl->VtxBuffer.Size;
        dl->AddConcavePolyFilled(v.Data, v.Size, IM_COL32_WHITE);
        ImGui::ShadeVertsLinearColorGradientKeepAlpha(dl, v0, dl->VtxBuffer.Size, P(0, y_lit), P(0, y_dark), lit, dark);
        dl->AddPolyline(v.Data, v.Size, OUT, ImDrawFlags_Closed, W(0.38f));
    }
    void stroke(ImU32 c, float w) { dl->AddPolyline(v.Data, v.Size, c, 0, W(w)); }
    void rivet(float x, float y)
    {
        dl->AddCircleFilled(P(x + 0.2f, y + 0.25f), 0.62f * k, IM_COL32(0, 0, 0, 90), 12);
        dl->AddCircleFilled(P(x, y), 0.6f * k, LO, 12);
        dl->AddCircleFilled(P(x - 0.15f, y - 0.18f), 0.3f * k, HI, 10);
    }
};
}

void ui_pointer_draw(ImDrawList *dl, ImVec2 tip, float k, int pose, int badge)
{
    G g{ dl, tip, k, {} };
    /* in proportion (1.4-RC6 on): the hand some 17 long and 7.5 high, the forefinger a little over half of
       that, the cuff a short flare behind the wrist. The thumb side toward us: the forefinger out on top, the
       other three curled under it, their roots under the plate of the hand, the thumb laid along them.
       Drawn back to front: the cuff, the curled fingers, the forefinger, the hand's plate, the thumb */
    /* the cuff: from the wrist, flaring to a rolled rim, its leather lining, a flute, two rivets */
    g.at(17.2f, 0.2f).curve(21.0f, -0.6f, 24.4f, -1.9f).curve(25.4f, 3.4f, 24.6f, 8.9f).curve(21.0f, 7.6f, 17.4f, 6.6f).curve(17.8f, 3.4f, 17.2f, 0.2f).plate(-1.9f, 8.9f);
    g.at(23.6f, -1.4f).curve(24.5f, 3.4f, 23.7f, 8.3f).stroke(LEATHER, 0.55f);
    g.at(24.3f, -1.7f).curve(25.1f, 3.4f, 24.3f, 8.6f).stroke(HI, 0.4f);
    g.at(19.4f, -0.2f).curve(20.0f, 3.4f, 19.6f, 7.0f).stroke(LO, 0.35f);
    g.rivet(18.6f, 0.9f); g.rivet(18.7f, 5.8f);
    /* the middle, ring and little fingers curled, each a little behind the one above: from the bottom up */
    static const float CURL[3][3] = { { 5.5f, 7.2f, 9.7f }, { 3.9f, 5.7f, 9.0f }, { 2.3f, 4.1f, 8.4f } };   /* top, bottom, front */
    for (const float *c : CURL) {
        float y0 = c[0], y1 = c[1], x0 = c[2], x1 = 13.6f, r = (y1 - y0) * 0.5f, ym = (y0 + y1) * 0.5f;
        g.at(x0 + r, y0).to(x1, y0).to(x1, y1).to(x0 + r, y1).curve(x0 - r * 0.9f, ym, x0 + r, y0).plate(y0, y1 + 0.3f);
        g.at(x0 + r * 0.8f, y0 + 0.35f).curve(x0 + 0.15f, y0 + 0.6f, x0 + 0.2f, ym).stroke(HI, 0.3f);
    }
    if (pose == PTR_GRAB) {                                  /* a fist: the forefinger curled with the rest */
        g.at(8.8f, 0.3f).to(12.6f, 0.1f).to(12.6f, 2.5f).to(8.8f, 2.6f).curve(7.4f, 1.5f, 8.8f, 0.3f).plate(0.1f, 2.6f);
        g.at(8.4f, 1.3f).curve(8.6f, 0.7f, 9.4f, 0.6f).stroke(HI, 0.3f);
    } else {                                                  /* the forefinger out: three lames, the tip a rounded cap */
        g.at(6.0f, -0.15f).to(11.4f, -0.4f).to(11.4f, 2.7f).to(6.0f, 2.45f).curve(5.6f, 1.1f, 6.0f, -0.15f).plate(-0.4f, 2.7f);
        g.at(2.8f, 0.0f).to(6.3f, -0.1f).curve(6.8f, 1.15f, 6.3f, 2.35f).to(2.8f, 2.2f).curve(2.4f, 1.1f, 2.8f, 0.0f).plate(-0.1f, 2.35f);
        g.at(0.3f, 0.15f).to(3.1f, 0.05f).curve(3.6f, 1.1f, 3.1f, 2.1f).to(0.9f, 2.0f).curve(-0.5f, 1.9f, -0.4f, 1.0f).curve(-0.3f, 0.3f, 0.3f, 0.15f).plate(0.05f, 2.1f);
        g.at(0.4f, 0.5f).to(2.7f, 0.42f).stroke(HI, 0.3f);
        g.at(3.2f, 0.35f).to(5.9f, 0.28f).stroke(HI, 0.3f);
        g.at(6.4f, 0.2f).to(9.8f, 0.08f).stroke(HI, 0.3f);
    }
    /* the back of the hand: a domed plate from the knuckles to the wrist, over the fingers' roots; a ridge at
       the knuckles, two narrow lames at the wrist */
    g.at(10.0f, -0.7f).curve(13.6f, -1.3f, 17.6f, -0.1f).curve(18.4f, 3.4f, 17.8f, 6.8f).curve(15.8f, 7.7f, 13.6f, 7.5f).curve(12.2f, 4.4f, 11.3f, 2.6f).curve(9.5f, 1.2f, 10.0f, -0.7f).plate(-1.1f, 7.6f);
    g.at(11.0f, -0.85f).curve(11.9f, 0.8f, 11.9f, 2.7f).stroke(LO, 0.32f);
    g.at(15.4f, -0.6f).curve(16.1f, 3.2f, 15.7f, 7.3f).stroke(GAP, 0.35f);
    g.at(16.6f, -0.4f).curve(17.3f, 3.2f, 16.9f, 7.1f).stroke(GAP, 0.35f);
    g.at(11.6f, -0.65f).curve(13.4f, -0.95f, 15.0f, -0.7f).stroke(HI, 0.4f);
    /* the thumb, laid forward along the curled middle finger: two lames, the tip rounded */
    g.at(12.8f, 2.9f).curve(15.0f, 2.5f, 16.6f, 3.1f).curve(17.0f, 4.3f, 16.4f, 5.3f).curve(14.6f, 5.5f, 12.6f, 5.0f).curve(12.2f, 3.9f, 12.8f, 2.9f).plate(2.6f, 5.4f);
    g.at(10.0f, 3.2f).to(13.2f, 3.0f).curve(13.7f, 4.0f, 13.2f, 4.9f).to(10.4f, 4.9f).curve(8.8f, 4.7f, 9.0f, 4.0f).curve(9.1f, 3.3f, 10.0f, 3.2f).plate(3.0f, 4.9f);
    g.at(9.8f, 3.55f).to(12.8f, 3.4f).stroke(HI, 0.3f);
    g.at(13.2f, 3.2f).curve(14.9f, 2.9f, 16.2f, 3.4f).stroke(HI, 0.3f);
    /* the cuff's badge: on a little dark plate (sizes in the badge's own units, f of the sprite's each) */
    if (badge != PTR_PLAIN) {
        const float bx = 21.4f, by = 3.4f, f = 0.86f;
        ImVec2 c = g.P(bx, by); float r = 2.5f * k;
        auto B = [&](float dx, float dy) { return g.P(bx + dx * f, by + dy * f); };
        dl->AddCircleFilled(ImVec2(c.x + 0.3f * k, c.y + 0.4f * k), r, IM_COL32(0, 0, 0, 90), 18);
        dl->AddCircleFilled(c, r, IM_COL32(44, 30, 22, 240), 18);
        dl->AddCircle(c, r, GOLD, 18, g.W(0.35f));
        if (badge == PTR_PAUSED)                              /* two gold bars */
            for (int b = 0; b < 2; b++) { float x = -1.0f + b * 1.8f; dl->AddRectFilled(B(x, -1.5f), B(x + 1.0f, 1.5f), GOLD, 0.2f * k); }
        else if (badge == PTR_ATTACK) {                       /* two blades crossed */
            for (int s = -1; s <= 1; s += 2) {
                dl->AddLine(B(-1.8f * s, -1.8f), B(1.4f * s, 1.4f), HI, g.W(0.5f));
                dl->AddLine(B(0.9f * s, 0.6f), B(1.9f * s, 1.8f), GOLD, g.W(0.6f));
            }
        } else if (badge == PTR_BUSY) {                       /* an hourglass */
            dl->AddTriangleFilled(B(-1.4f, -1.9f), B(1.4f, -1.9f), B(0, 0), GOLD);
            dl->AddTriangleFilled(B(0, 0), B(1.4f, 1.9f), B(-1.4f, 1.9f), GOLD);
            dl->AddLine(B(-1.6f, -2.0f), B(1.6f, -2.0f), HI, g.W(0.35f)); dl->AddLine(B(-1.6f, 2.0f), B(1.6f, 2.0f), HI, g.W(0.35f));
        }
    }
}
