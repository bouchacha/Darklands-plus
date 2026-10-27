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
 * as the game's. Coordinates in the sprite's pixels (26 x 15, the tip at 0,0), k window pixels to one. */
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
    /* the cuff: flaring from the wrist, a rolled rim at its mouth, two flutes, its leather lining */
    g.at(18.6f, 1.6f).curve(22.0f, 0.2f, 25.4f, -0.9f).curve(26.8f, 7.0f, 26.2f, 14.9f).curve(22.4f, 13.4f, 19.0f, 12.4f).curve(19.8f, 7.0f, 18.6f, 1.6f).plate(-0.9f, 14.9f);
    g.at(24.9f, -0.4f).curve(26.1f, 7.0f, 25.6f, 14.4f).stroke(LEATHER, 0.7f);
    g.at(25.5f, -0.7f).curve(26.6f, 7.0f, 26.1f, 14.7f).stroke(HI, 0.45f);
    g.at(20.6f, 1.6f).curve(21.4f, 7.0f, 20.9f, 12.6f).stroke(LO, 0.4f);
    g.at(22.6f, 0.9f).curve(23.5f, 7.0f, 23.0f, 13.1f).stroke(LO, 0.4f);
    g.rivet(21.7f, 2.6f); g.rivet(22.0f, 11.3f);
    /* the back of the hand: a domed plate, two lames across it toward the knuckles */
    g.at(11.2f, 0.9f).curve(15.0f, -0.4f, 18.8f, 0.6f).curve(20.2f, 6.0f, 19.2f, 11.4f).curve(16.0f, 13.0f, 12.4f, 11.8f).to(13.0f, 6.0f).curve(11.0f, 4.0f, 11.2f, 0.9f).plate(-0.2f, 12.6f);
    g.at(14.2f, 0.7f).curve(15.2f, 6.0f, 14.8f, 11.9f).stroke(GAP, 0.45f);
    g.at(16.6f, 0.5f).curve(17.6f, 6.0f, 17.2f, 12.1f).stroke(GAP, 0.45f);
    g.at(17.2f, 0.9f).curve(18.0f, 0.7f, 18.6f, 1.1f).stroke(HI, 0.5f);
    /* the thumb, folded under the palm: two lames */
    g.at(12.6f, 10.4f).curve(14.6f, 10.6f, 15.8f, 11.8f).curve(15.6f, 13.2f, 14.6f, 13.8f).curve(13.0f, 13.6f, 11.6f, 12.8f).curve(11.8f, 11.2f, 12.6f, 10.4f).plate(10.4f, 13.8f);
    g.at(9.6f, 10.8f).curve(11.4f, 10.2f, 12.8f, 10.8f).curve(12.6f, 12.2f, 11.8f, 12.9f).curve(10.2f, 13.2f, 9.0f, 12.6f).curve(9.0f, 11.6f, 9.6f, 10.8f).plate(10.2f, 13.2f);
    /* the other three fingers, curled: rounded knuckle plates one under another */
    for (int f = 2; f >= 0; f--) {
        float y = 3.9f + f * 2.2f, x0 = 8.7f + f * 0.5f, x1 = 13.6f;
        g.at(x0 + 0.9f, y).to(x1, y - 0.1f).curve(x1 + 0.6f, y + 1.0f, x1, y + 2.1f).to(x0 + 0.9f, y + 2.2f).curve(x0 - 0.4f, y + 1.1f, x0 + 0.9f, y).plate(y, y + 2.3f);
        g.at(x0 + 1.2f, y + 0.45f).to(x1 - 0.6f, y + 0.4f).stroke(HI, 0.35f);
    }
    if (pose == PTR_GRAB) {                                  /* a fist: the forefinger curled with the rest */
        g.at(9.0f, 1.4f).to(13.4f, 1.1f).curve(14.0f, 2.5f, 13.4f, 3.9f).to(9.0f, 4.0f).curve(7.6f, 2.7f, 9.0f, 1.4f).plate(1.1f, 4.0f);
        g.at(9.4f, 1.8f).to(12.8f, 1.6f).stroke(HI, 0.35f);
    } else {                                                  /* the forefinger out: three lames to a rounded point */
        g.at(7.8f, 0.5f).to(12.0f, 0.4f).curve(12.6f, 2.2f, 12.1f, 3.9f).to(7.9f, 3.8f).curve(7.4f, 2.1f, 7.8f, 0.5f).plate(0.4f, 3.9f);
        g.at(4.1f, 0.5f).to(8.1f, 0.4f).curve(8.6f, 2.0f, 8.2f, 3.6f).to(4.3f, 3.5f).curve(3.8f, 2.0f, 4.1f, 0.5f).plate(0.4f, 3.6f);
        g.at(0.0f, 0.0f).curve(2.0f, 0.2f, 4.4f, 0.5f).curve(4.9f, 1.9f, 4.5f, 3.4f).curve(2.4f, 3.4f, 1.0f, 2.2f).curve(0.2f, 1.3f, 0.0f, 0.0f).plate(0.0f, 3.4f);
        g.at(0.9f, 0.55f).curve(2.6f, 0.6f, 4.0f, 0.95f).stroke(HI, 0.4f);
        g.at(4.6f, 0.9f).to(7.7f, 0.85f).stroke(HI, 0.35f);
        g.at(8.3f, 0.9f).to(11.6f, 0.85f).stroke(HI, 0.35f);
    }
    /* the cuff's badge: on a little dark plate */
    if (badge != PTR_PLAIN) {
        ImVec2 c = g.P(23.2f, 7.0f); float r = 2.9f * k;
        dl->AddCircleFilled(ImVec2(c.x + 0.3f * k, c.y + 0.4f * k), r, IM_COL32(0, 0, 0, 90), 18);
        dl->AddCircleFilled(c, r, IM_COL32(44, 30, 22, 240), 18);
        dl->AddCircle(c, r, GOLD, 18, g.W(0.35f));
        if (badge == PTR_PAUSED)                              /* two gold bars */
            for (int b = 0; b < 2; b++) { float x = 22.2f + b * 1.8f; dl->AddRectFilled(g.P(x, 5.5f), g.P(x + 1.0f, 8.5f), GOLD, 0.2f * k); }
        else if (badge == PTR_ATTACK) {                       /* two blades crossed */
            for (int s = -1; s <= 1; s += 2) {
                dl->AddLine(g.P(23.2f - 1.8f * s, 5.2f), g.P(23.2f + 1.4f * s, 8.4f), HI, g.W(0.55f));
                dl->AddLine(g.P(23.2f + 0.9f * s, 7.6f), g.P(23.2f + 1.9f * s, 8.8f), GOLD, g.W(0.7f));
            }
        } else if (badge == PTR_BUSY) {                       /* an hourglass */
            dl->AddTriangleFilled(g.P(21.8f, 5.1f), g.P(24.6f, 5.1f), g.P(23.2f, 7.0f), GOLD);
            dl->AddTriangleFilled(g.P(23.2f, 7.0f), g.P(24.6f, 8.9f), g.P(21.8f, 8.9f), GOLD);
            dl->AddLine(g.P(21.6f, 5.0f), g.P(24.8f, 5.0f), HI, g.W(0.4f)); dl->AddLine(g.P(21.6f, 9.0f), g.P(24.8f, 9.0f), HI, g.W(0.4f));
        }
    }
}
