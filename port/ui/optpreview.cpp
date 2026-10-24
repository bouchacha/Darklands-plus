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
    void bar(float x0, float y, float w, float f, ImU32 c, bool pale_loss) const {
        rect(x0 - 0.5f, y - 0.5f, x0 + w + 0.5f, y + 4.5f, IM_COL32(0, 0, 0, 200));
        rect(x0, y, x0 + w * f, y + 4, c);
        if (pale_loss && f < 1) rect(x0 + w * f, y + 1.2f, x0 + w, y + 2.8f, CREAM);
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
void reload_bar(const S &s, bool on)
{
    s.ground();
    s.figure(45, 50, OURS);
    s.dl->AddLine(s.p(49, 33), s.p(62, 26), IM_COL32(120, 80, 40, 255), 2.5f * s.k);   /* (a crossbow) */
    if (on) { s.rect(33, 52.5f, 57, 56.5f, IM_COL32(0, 0, 0, 200)); s.rect(33.6f, 53.1f, 33.6f + 22.8f * 0.6f, 55.9f, GOLD); }
}
void click_hint(const S &s, bool on)
{
    s.ground();
    s.figure(22, 56, OURS);
    s.figure(72, 50, FOE);
    s.pointer(68, 30);
    if (on) { s.rect(38, 22, 66, 32, IM_COL32(244, 232, 200, 240), 1); s.text(40, 22.6f, 5.2f, INK, "Shoot"); }
}
void order_icons(const S &s, bool on)
{
    s.rect(0, 0, 100, 60, PANEL);
    const char *L[3] = { "Std Attack", "Parry", "Walk to" };
    for (int i = 0; i < 3; i++) {
        float y = 6 + i * 17;
        s.rect(8, y, 92, y + 13, IM_COL32(120, 84, 40, 255), 2);
        s.frame(8, y, 92, y + 13, GOLD, 1, 2);
        float tx = 14;
        if (on) {
            ImVec2 c = s.p(16, y + 6.5f);
            if (i == 0) s.dl->AddLine(ImVec2(c.x - 3 * s.k, c.y + 3 * s.k), ImVec2(c.x + 3 * s.k, c.y - 3 * s.k), CREAM, 1.5f);
            else if (i == 1) s.dl->AddCircleFilled(c, 3 * s.k, CREAM, 12);
            else { s.dl->AddCircleFilled(ImVec2(c.x - 1.5f * s.k, c.y + 1.5f * s.k), 1.4f * s.k, CREAM, 8); s.dl->AddCircleFilled(ImVec2(c.x + 1.5f * s.k, c.y - 1.5f * s.k), 1.4f * s.k, CREAM, 8); }
            tx = 23;
        }
        s.text(tx, y + 1.5f, 5.5f, CREAM, L[i]);
    }
}
void skill_gains(const S &s, bool on)
{
    s.ground();
    s.figure(50, 56, OURS);
    if (on) { s.text(34, 10, 6.5f, IM_COL32(255, 230, 120, 255), "+1 Edged"); }
}
void battle_log(const S &s, bool on)
{
    s.ground();
    s.figure(20, 40, OURS); s.figure(36, 40, FOE);
    if (on) {
        const char *L[2] = { "Gunther hits Bandit", "Bandit falls" };
        for (int i = 0; i < 2; i++) { s.rect(42, 40 + i * 10, 99, 49 + i * 10, IM_COL32(20, 14, 10, 210)); s.text(43, 40.5f + i * 10, 5, CREAM, L[i]); }
    }
}
void battle_info(const S &s, bool on)
{
    s.ground();
    s.figure(22, 56, FOE);
    s.pointer(24, 36);
    if (on) {
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
void full_names(const S &s, bool on)
{
    s.rect(0, 0, 100, 60, PANEL);
    s.text(5, 8, 6, GOLD, on ? "Gretchen Wilburg" : "Gretch");
    s.text(5, 32, 6, GOLD, on ? "Ebhard of Achdorf" : "Ebhard");
}
void volley_toggle(const S &s, bool on)
{
    s.rect(0, 0, 100, 60, PANEL);
    s.frame(3, 3, 97, 57, GOLD, 1, 2);
    s.text(8, 5, 5.5f, GOLD, "Hans");
    s.bar(8, 30, 60, 0.9f, END_C, false); s.bar(8, 42, 60, 1.0f, STR_C, false);
    if (on) {                                                  /* the bow on the box */
        ImVec2 c = s.p(76, 22);
        s.dl->PathArcTo(c, 9 * s.k, -1.2f, 1.2f, 12); s.dl->PathStroke(IM_COL32(200, 150, 80, 255), 0, 2.0f);
        s.dl->AddLine(s.p(76 + 9 * 0.36f, 22 - 9 * 0.93f), s.p(76 + 9 * 0.36f, 22 + 9 * 0.93f), CREAM, 1.0f);
    }
}
void order_bar(const S &s, bool on)
{
    s.ground();
    s.figure(40, 40, OURS); s.figure(70, 34, FOE);
    if (on) {
        s.rect(0, 46, 100, 60, IM_COL32(30, 22, 16, 235));
        const char *L[3] = { "Attack", "Parry", "Halt" };
        for (int i = 0; i < 3; i++) { s.rect(3 + i * 32, 48.5f, 31 + i * 32, 58, IM_COL32(120, 84, 40, 255), 1.5f); s.text(6 + i * 32, 48.8f, 4.6f, CREAM, L[i]); }
    }
}
void los_line(const S &s, bool on)
{
    s.ground();
    s.figure(18, 50, OURS);
    s.figure(84, 30, FOE);
    ImVec2 r0 = s.p(46, 30), r1 = s.p(58, 42);                          /* a rock between */
    s.dl->AddEllipseFilled(ImVec2((r0.x + r1.x) * 0.5f, (r0.y + r1.y) * 0.5f), ImVec2((r1.x - r0.x) * 0.5f, (r1.y - r0.y) * 0.5f), IM_COL32(150, 150, 150, 255));
    if (!on) return;
    const ImU32 RED = IM_COL32(235, 60, 40, 255);
    ImVec2 q[4] = { s.p(44, 39), s.p(60, 39), s.p(56, 45), s.p(40, 45) };
    s.dl->AddQuadFilled(q[0], q[1], q[2], q[3], IM_COL32(235, 60, 40, 90));
    s.dl->AddPolyline(q, 4, RED, ImDrawFlags_Closed, 1.5f);
    ImVec2 a = s.p(18, 50), h = s.p(50, 41), z = s.p(84, 30);
    s.dl->AddLine(a, h, IM_COL32(250, 214, 110, 230), 1.5f);
    for (int k = 0; k < 6; k += 2) { float t0 = k / 6.0f, t1 = (k + 1) / 6.0f; s.dl->AddLine(ImVec2(h.x + (z.x - h.x) * t0, h.y + (z.y - h.y) * t0), ImVec2(h.x + (z.x - h.x) * t1, h.y + (z.y - h.y) * t1), RED, 1.5f); }
    s.dl->AddCircle(z, 2.5f * s.k, RED, 14, 1.5f);
}
void alchemy_art(const S &s, bool on)
{
    const GamePic *g = on ? gamepic("PICS\\ALCSCRN0.PIC") : nullptr;
    if (g && g->tex) { s.dl->AddImage((ImTextureID)(intptr_t)g->tex, s.p(0, 0), s.p(100, 60), ImVec2(60.0f / 320, 0), ImVec2(1, 1)); return; }
    s.rect(0, 0, 100, 60, IM_COL32(242, 230, 200, 255));
    s.text(4, 3, 6, INK, "Alchemy");
    for (int i = 0; i < 5; i++) s.rect(4, 14 + i * 7, 44, 19 + i * 7, i == 1 ? IM_COL32(214, 186, 140, 255) : IM_COL32(230, 216, 186, 255));
    for (int i = 0; i < 6; i++) s.rect(52, 14 + i * 6, i ? 92 : 80, 16 + i * 6, IM_COL32(150, 120, 90, 255));
}
void shop_compare(const S &s, bool on)
{
    s.rect(0, 0, 100, 60, IM_COL32(250, 242, 222, 255));
    const char *L[3] = { "Padded (body)", "Chainmail (body)", "Plate Armor (body)" };
    int v[3] = { -1, 0, 1 };
    for (int i = 0; i < 3; i++) {
        float y = 6 + i * 13;
        if (i & 1) s.rect(2, y - 2, 98, y + 9, IM_COL32(240, 230, 204, 255));
        s.text(5, y, 6, INK, L[i]);
        if (on && v[i]) {
            float ax = 5 + ImGui::GetFont()->CalcTextSizeA(6 * s.k, FLT_MAX, 0, L[i]).x / s.k + 4, ay = y + 3.5f;
            ImU32 c = v[i] > 0 ? IM_COL32(40, 96, 34, 255) : IM_COL32(140, 30, 18, 255);
            s.dl->AddTriangleFilled(s.p(ax - 2.2f, v[i] < 0 ? ay - 1.6f : ay + 1.6f), s.p(ax + 2.2f, v[i] < 0 ? ay - 1.6f : ay + 1.6f), s.p(ax, v[i] < 0 ? ay + 2 : ay - 2), c);
        }
    }
    float y = 46;
    if (on) s.rect(2, y - 2, 98, y + 9, IM_COL32(214, 170, 60, 110));
    s.text(5, y, 6, INK, "Worn: Brigandine");
}
} // namespace

/* the switch's picture, off or on, in a..b (its height about 0.6 of its width); false: none */
bool ui_option_preview(const char *key, ImDrawList *dl, ImVec2 a, ImVec2 b, bool on)
{
    struct E { const char *key; int id; };
    static const E T[] = { { "battle.controls", 5 }, { "battle.helpers", 9 }, { "assist.card_helpers", 18 }, { "battle.reload_bar", 4 },
                           { "battle.click_hint", 5 }, { "battle.order_icons", 6 }, { "battle.skill_gains", 7 }, { "battle.log", 8 },
                           { "battle.info", 9 }, { "map.names", 10 }, { "map.quest_marks", 11 }, { "display.full_names", 12 }, { "battle.volley_toggle", 13 }, { "battle.order_bar", 14 }, { "battle.los_line", 15 }, { "display.shop_compare", 17 } };
    int id = 0;
    for (const E &e : T) if (!strcmp(e.key, key)) id = e.id;
    if (!id) return false;
    if (!dl) return true;
    S s{ dl, a, (b.x - a.x) / 100.0f };
    dl->PushClipRect(a, b, true);
    switch (id) {
    case 4: reload_bar(s, on); break;
    case 5: click_hint(s, on); break;
    case 6: order_icons(s, on); break;
    case 7: skill_gains(s, on); break;
    case 8: battle_log(s, on); break;
    case 9: battle_info(s, on); break;
    case 10: map_scene(s, on, false); break;
    case 11: map_scene(s, true, on); break;
    case 12: full_names(s, on); break;
    case 13: volley_toggle(s, on); break;
    case 14: order_bar(s, on); break;
    case 15: los_line(s, on); break;
    case 16: alchemy_art(s, on); break;
    case 17: shop_compare(s, on); break;
    case 18: card_helpers(s, on); break;
    }
    dl->PopClipRect();
    return true;
}
