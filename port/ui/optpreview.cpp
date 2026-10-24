/* optpreview.cpp - small pictures of what a switch in the settings does: the same little scene drawn
 * with it off and on, side by side under the switch (ui/settings.cpp option_row). Drawn, not taken
 * from the game's art: a sketch of the interface in its own colours. */
#include <math.h>
#include <float.h>
#include <string.h>
#include "imgui.h"
#include "gamepics.h"

namespace {
const ImU32 GRASS = IM_COL32(58, 92, 40, 255), GRASS_D = IM_COL32(44, 72, 30, 255), INK = IM_COL32(40, 26, 12, 255),
            PANEL = IM_COL32(30, 22, 16, 255), GOLD = IM_COL32(210, 170, 90, 255), CREAM = IM_COL32(240, 230, 210, 255),
            END_C = IM_COL32(96, 120, 236, 255), STR_C = IM_COL32(80, 190, 80, 255), FOE = IM_COL32(150, 60, 40, 255),
            OURS = IM_COL32(60, 90, 170, 255), SEA = IM_COL32(40, 80, 170, 255), LAND = IM_COL32(70, 130, 60, 255);

struct S {                                     /* the scene's box: x, y in 0..100 across, 0..60 down */
    ImDrawList *dl; ImVec2 a; float k;
    ImVec2 p(float x, float y) const { return ImVec2(a.x + x * k, a.y + y * k); }
    void rect(float x0, float y0, float x1, float y1, ImU32 c, float r = 0) const { dl->AddRectFilled(p(x0, y0), p(x1, y1), c, r * k); }
    void frame(float x0, float y0, float x1, float y1, ImU32 c, float w = 1, float r = 0) const { dl->AddRect(p(x0, y0), p(x1, y1), c, r * k, 0, w); }
    void text(float x, float y, float size, ImU32 c, const char *t) const { dl->AddText(ImGui::GetFont(), size * 1.45f * k, p(x, y), c, t); }   /* (larger than drawn to scale: to be read) */
    void ground() const { rect(0, 0, 100, 60, GRASS); for (int i = 0; i < 14; i++) { float x = fmodf(i * 37.0f, 100), y = fmodf(i * 23.0f, 60); rect(x, y, x + 3, y + 1, GRASS_D); } }
    /* a fighter standing at x (feet at y) */
    void figure(float x, float y, ImU32 body) const {       /* (drawn large: the scene is small) */
        const float g = 1.9f;
        dl->AddCircleFilled(p(x, y - 13 * g), 2.2f * g * k, IM_COL32(230, 190, 150, 255), 12);
        rect(x - 2.4f * g, y - 11 * g, x + 2.4f * g, y - 3.5f * g, body, 1);
        rect(x - 2.2f * g, y - 3.5f * g, x - 0.5f * g, y, IM_COL32(50, 40, 30, 255));
        rect(x + 0.5f * g, y - 3.5f * g, x + 2.2f * g, y, IM_COL32(50, 40, 30, 255));
    }
    void pointer(float x, float y) const {
        ImVec2 q[3] = { p(x, y), p(x, y + 6), p(x + 4, y + 4.5f) };
        dl->AddTriangleFilled(q[0], q[1], q[2], IM_COL32(250, 250, 250, 255));
        dl->AddTriangle(q[0], q[1], q[2], INK, 1);
    }
};

/* Card helpers: an option with its chance and who would try it, and the saint it accepts */
void card_helpers(const S &s, bool on)
{
    s.rect(0, 0, 100, 60, IM_COL32(240, 228, 198, 255));
    s.text(5, 5, 4.8f, INK, "The gate is barred. You...");
    s.rect(4, 17, 96, 27, IM_COL32(214, 170, 60, 110), 1);
    s.text(6, 18, 4.8f, IM_COL32(140, 30, 18, 255), "1."); s.text(12, 18, 4.6f, INK, "...climb the wall");
    s.text(6, 31, 4.8f, IM_COL32(140, 30, 18, 255), "2."); s.text(12, 31, 4.6f, INK, "...pray to a saint");
    s.text(6, 44, 4.8f, IM_COL32(140, 30, 18, 255), "3."); s.text(12, 44, 4.6f, INK, "...leave");
    if (!on) return;
    s.rect(70, 18.3f, 94, 25.7f, IM_COL32(250, 242, 222, 255), 1); s.frame(70, 18.3f, 94, 25.7f, IM_COL32(150, 110, 60, 255), 0.8f, 1);
    s.text(72, 18.8f, 4.2f, IM_COL32(40, 96, 34, 255), "61% Ursula");
    s.rect(64, 31.3f, 94, 38.7f, IM_COL32(250, 242, 222, 255), 1); s.frame(64, 31.3f, 94, 38.7f, IM_COL32(150, 110, 60, 255), 0.8f, 1);
    s.text(66, 31.8f, 4.2f, IM_COL32(70, 70, 120, 255), "St. Odilia: Jorg");
}
void click_hint(const S &s, bool on)
{
    s.ground();
    s.figure(22, 56, OURS);
    s.figure(72, 50, FOE);
    s.pointer(68, 30);
    if (on) { s.rect(38, 22, 66, 32, IM_COL32(244, 232, 200, 240), 1); s.text(40, 22.6f, 5.2f, INK, "Shoot"); }
}
void battle_info(const S &s, bool on)
{
    s.ground();
    s.figure(22, 56, FOE);
    s.pointer(24, 36);
    if (on) {
        s.rect(13.5f, 22.5f, 30.5f, 25.5f, IM_COL32(10, 6, 2, 200)); s.rect(14, 23, 22, 25, IM_COL32(215, 95, 70, 235));   /* the wounded enemy's bar */
        s.rect(40, 4, 98, 52, IM_COL32(244, 232, 200, 245), 2);
        s.text(43, 6, 5.5f, INK, "Bandit");
        s.text(43, 20, 4.4f, IM_COL32(110, 70, 40, 255), "badly hurt");
        s.text(43, 31, 4.4f, IM_COL32(110, 70, 40, 255), "melee 32");
        s.text(43, 42, 4.4f, IM_COL32(110, 70, 40, 255), "falchion");
    }
}
void map_scene(const S &s, bool names, bool marks)
{
    s.rect(0, 0, 100, 60, LAND);
    s.dl->AddRectFilled(s.p(0, 0), s.p(100, 14), SEA);
    s.dl->AddBezierCubic(s.p(10, 60), s.p(30, 40), s.p(40, 30), s.p(60, 14), SEA, 2.5f * s.k);
    for (int i = 0; i < 2; i++) {
        float x = i ? 70 : 30, y = i ? 40 : 28;
        s.rect(x - 3, y - 3, x + 3, y + 3, CREAM); s.frame(x - 3, y - 3, x + 3, y + 3, INK);
        s.dl->AddTriangleFilled(s.p(x - 3.5f, y - 3), s.p(x + 3.5f, y - 3), s.p(x, y - 7), IM_COL32(160, 60, 40, 255));
        if (names) s.text(x - 12, y + 4, 5, CREAM, i ? "Hannover" : "Bremen");
        if (marks && i) { s.dl->AddCircleFilled(s.p(x + 7, y - 7), 3.2f * s.k, IM_COL32(176, 40, 30, 255), 14); s.dl->AddCircle(s.p(x + 7, y - 7), 3.2f * s.k, GOLD, 14, 1.2f); }
    }
}
} // namespace

/* the switch's picture, off or on, in a..b (its height about 0.6 of its width); false: none */
bool ui_option_preview(const char *key, ImDrawList *dl, ImVec2 a, ImVec2 b, bool on)
{
    struct E { const char *key; int id; };
    static const E T[] = { { "battle.controls", 5 }, { "battle.helpers", 9 }, { "assist.card_helpers", 18 }, { "map.names", 10 }, { "map.quest_marks", 11 } };
    int id = 0;
    for (const E &e : T) if (!strcmp(e.key, key)) id = e.id;
    if (!id) return false;
    if (!dl) return true;
    S s{ dl, a, (b.x - a.x) / 100.0f };
    dl->PushClipRect(a, b, true);
    switch (id) {
    case 5: click_hint(s, on); break;
    case 9: battle_info(s, on); break;
    case 10: map_scene(s, on, false); break;
    case 11: map_scene(s, true, on); break;
    case 18: card_helpers(s, on); break;
    }
    dl->PopClipRect();
    return true;
}
