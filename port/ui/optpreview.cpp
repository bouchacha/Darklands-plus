/* optpreview.cpp - small pictures of what a switch in the settings turns on, up to four in a row
 * under the switch, each with a word (ui/settings.cpp option_row). Drawn, not taken from the
 * game's art: a sketch of the interface in its own colors. */
#include <math.h>
#include <float.h>
#include <string.h>
#include "richtext.h"
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
    void text(float x, float y, float size, ImU32 c, const char *t) const { rich_draw(dl, ImGui::GetFont(), size * 1.45f * k, p(x, y), 0, c, t); }   /* (larger than drawn to scale: to be read; shields as ui/richtext.h) */
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
/* the line of a shot: the shooter, an ally in the way (red), and a clear line (green) to another enemy */
void los_line(const S &s)
{
    s.ground();
    s.figure(14, 54, OURS); s.figure(52, 40, OURS); s.figure(86, 30, FOE); s.figure(80, 58, FOE);
    s.dl->AddLine(s.p(18, 36), s.p(50, 26), IM_COL32(200, 50, 40, 255), 1.6f * s.k);
    s.dl->AddLine(s.p(50, 26), s.p(84, 14), IM_COL32(200, 50, 40, 120), 1.0f * s.k);
    s.dl->AddLine(s.p(18, 40), s.p(78, 44), IM_COL32(120, 220, 90, 255), 1.6f * s.k);
}
void skill_gain(const S &s)
{
    s.ground();
    s.figure(46, 56, OURS);
    s.text(38, 8, 5.4f, IM_COL32(240, 200, 70, 255), "{Edged} +2");
    s.rect(44, 57, 60, 58.6f, IM_COL32(10, 6, 2, 200)); s.rect(44.3f, 57.3f, 54, 58.3f, IM_COL32(230, 160, 50, 255));   /* (the reload bar) */
}
void battle_log(const S &s)
{
    s.rect(0, 0, 100, 60, IM_COL32(236, 222, 190, 255));
    static const char *L[5] = { "Jorg hits the bandit", "The bandit parries", "Anna shoots: a wound", "The bandit falls", "Jorg: {Edged:+1 (now 31)}" };
    for (int i = 0; i < 5; i++) s.text(5, 4 + i * 11, 4.4f, i == 3 ? IM_COL32(150, 40, 25, 255) : INK, L[i]);
}
void shoot_switch(const S &s)
{
    s.rect(0, 0, 100, 60, PANEL);
    for (int k = 0; k < 2; k++) {
        float y = 6 + k * 27;
        s.rect(6, y, 94, y + 22, IM_COL32(60, 44, 30, 255), 2); s.frame(6, y, 94, y + 22, GOLD, 0.8f, 2);
        s.text(12, y + 5, 5.0f, CREAM, k ? "Anna" : "Jorg");
        float cx = 78, cy = y + 11;
        if (k) {   /* a bow */
            s.dl->AddBezierQuadratic(s.p(cx - 3, cy - 8), s.p(cx + 7, cy), s.p(cx - 3, cy + 8), GOLD, 1.6f * s.k);
            s.dl->AddLine(s.p(cx - 3, cy - 8), s.p(cx - 3, cy + 8), CREAM, 0.8f * s.k);
        } else {   /* a sword */
            s.dl->AddLine(s.p(cx - 7, cy + 7), s.p(cx + 6, cy - 6), CREAM, 1.8f * s.k);
            s.dl->AddLine(s.p(cx - 7, cy + 1), s.p(cx - 1, cy + 7), GOLD, 1.6f * s.k);
        }
    }
}
void order_bar(const S &s)
{
    s.ground();
    s.rect(0, 40, 100, 60, PANEL);
    static const char *B[4] = { "Parry", "Berserk", "Missile", "Halt" };
    for (int k = 0; k < 4; k++) { float x = 3 + k * 24.5f; s.rect(x, 44, x + 22, 56, IM_COL32(112, 40, 22, 235), 1.5f); s.text(x + 2, 46, 3.6f, CREAM, B[k]); }
    s.figure(30, 36, OURS); s.figure(70, 30, FOE);
}
void fast_forward(const S &s)
{
    s.ground();
    s.figure(30, 54, OURS); s.figure(66, 50, FOE);
    for (int k = 0; k < 2; k++) {   /* the fast-forward mark */
        float x = 40 + k * 10;
        s.dl->AddTriangleFilled(s.p(x, 6), s.p(x, 22), s.p(x + 10, 14), IM_COL32(250, 240, 220, 230));
    }
    s.text(64, 8, 5.4f, CREAM, "Z");
}
void level_map(const S &s)
{
    s.rect(0, 0, 100, 60, IM_COL32(28, 22, 16, 255));
    const ImU32 FL = IM_COL32(196, 176, 136, 255), WL = IM_COL32(90, 70, 46, 255);
    struct R { float x0, y0, x1, y1; } rooms[5] = { { 6, 8, 30, 26 }, { 30, 14, 58, 20 }, { 58, 6, 86, 30 }, { 68, 30, 74, 46 }, { 56, 46, 92, 56 } };
    for (const R &r : rooms) { s.rect(r.x0, r.y0, r.x1, r.y1, FL); s.frame(r.x0, r.y0, r.x1, r.y1, WL, 1.0f); }
    s.rect(10, 34, 36, 54, IM_COL32(60, 52, 42, 255));            /* (not yet seen) */
    for (int k = 0; k < 4; k++) s.dl->AddCircleFilled(s.p(14 + k * 4.5f, 18), 1.6f * s.k, OURS, 10);
    s.dl->AddCircleFilled(s.p(78, 52), 1.8f * s.k, FOE, 10);
    s.text(78, 10, 4.2f, INK, "stairs"); s.rect(80, 18, 84, 24, WL);
    s.pointer(64, 40);
}
} // namespace

/* what a switch turns on, as up to four small pictures with a word under each (settings.cpp
   option_row): the number of pictures, 0 for none; caps[i] the words */
int ui_option_pics(const char *key, const char **caps)
{
    struct E { const char *key; int n; const char *cap[4]; };
    static const E T[] = {
        { "battle.helpers", 4, { "Pointing at an enemy", "The line of a shot", "Skill gains, reloading", "The battle log" } },
        { "battle.controls", 4, { "One click to attack", "Shoot or fight", "Order buttons", "Z: fast-forward" } },
        { "assist.level_map", 1, { "Tab: the level map; a click walks there" } },
        { "assist.card_helpers", 1, { "Chances, and who would try" } },
    };
    for (const E &e : T) if (!strcmp(e.key, key)) { for (int i = 0; i < e.n; i++) caps[i] = e.cap[i]; return e.n; }
    return 0;
}
void ui_option_pic(const char *key, int i, ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    S s{ dl, a, (b.x - a.x) / 100.0f };
    dl->PushClipRect(a, b, true);
    if (!strcmp(key, "battle.helpers")) { if (i == 0) battle_info(s, true); else if (i == 1) los_line(s); else if (i == 2) skill_gain(s); else battle_log(s); }
    else if (!strcmp(key, "battle.controls")) { if (i == 0) click_hint(s, true); else if (i == 1) shoot_switch(s); else if (i == 2) order_bar(s); else fast_forward(s); }
    else if (!strcmp(key, "assist.level_map")) level_map(s);
    else if (!strcmp(key, "assist.card_helpers")) card_helpers(s, true);
    else if (!strcmp(key, "map.names")) map_scene(s, true, false);
    else if (!strcmp(key, "map.quest_marks")) map_scene(s, true, true);
    dl->PopClipRect();
}
