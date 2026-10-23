/* pstone.cpp - the Philosopher's Stone, as the alchemists of the age pictured it (display.pstone_look):
 *   0 the red stone: the end of the Great Work (rubedo), a heavy, glassy stone the colour of a ruby,
 *     shining by its own light - "red as a ruby, transparent as crystal, fusible as wax";
 *   1 the powder of projection: the Stone ground to a red powder kept in a phial, a grain of which,
 *     thrown on molten lead, turns it to gold (the transmutation stories of the age);
 *   2 the philosophers' egg: the sealed glass vessel (vas hermeticum) in which the Work was done,
 *     the red stone ripening inside it, the sun and moon joined over it.
 * Each drawn at c, s across; quality > 0 lights it (the better the stone, the brighter); 0: none
 * yet, drawn faint. */
#include "imgui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/config.h"

static ImU32 A(ImU32 c, float a) { int al = (int)(((c >> IM_COL32_A_SHIFT) & 255) * a); return (c & ~IM_COL32_A_MASK) | ((ImU32)(al < 0 ? 0 : al > 255 ? 255 : al) << IM_COL32_A_SHIFT); }
static void glow(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float strength)
{
    for (int i = 12; i >= 1; i--) { float f = i / 12.0f; dl->AddCircleFilled(c, r * f, A(col, strength * (1 - f) * (1 - f) * 0.35f), 32); }
}

void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t)
{
    float u = s / 40.0f, on = quality > 0 ? 1.0f : 0.35f;
    float lum = quality > 0 ? 0.55f + 0.45f * fminf(1.0f, quality / 40.0f) : 0.0f;
    float pulse = 0.85f + 0.15f * sinf(t * 2.2f);
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    const ImU32 RUBY = IM_COL32(170, 18, 30, 255), RUBY_DK = IM_COL32(90, 6, 16, 255), RUBY_LT = IM_COL32(240, 90, 90, 255), GOLD = IM_COL32(214, 168, 60, 255), GOLD_DK = IM_COL32(130, 92, 30, 255);
    switch (look) {
    default:
    case 0: {                                                   /* the red stone: a cut ruby-like stone, its own light */
        if (lum > 0) glow(dl, P(0, 0), 26 * u, IM_COL32(255, 60, 40, 255), lum * pulse);
        ImVec2 o[8], in[8];
        for (int i = 0; i < 8; i++) { float a = i * 3.14159f / 4 + 3.14159f / 8; o[i] = P(cosf(a) * 15, sinf(a) * 13 + 1); in[i] = P(cosf(a) * 8, sinf(a) * 6.5f - 1); }
        dl->AddConvexPolyFilled(o, 8, A(RUBY_DK, on));
        for (int i = 0; i < 8; i++) {                           /* the crown's facets: lit from the upper left */
            int j = (i + 1) % 8;
            float l = cosf((i + 0.5f) * 3.14159f / 4 + 3.14159f / 8 - 3.14159f * 1.25f);
            ImU32 col = l > 0.5f ? RUBY_LT : l > -0.2f ? RUBY : RUBY_DK;
            ImVec2 q[4] = { o[i], o[j], in[j], in[i] };
            dl->AddConvexPolyFilled(q, 4, A(col, on));
            dl->AddLine(o[i], in[i], A(IM_COL32(60, 0, 8, 200), on), 0.8f * u);
        }
        dl->AddConvexPolyFilled(in, 8, A(IM_COL32(205, 36, 46, 255), on));                     /* the table */
        dl->AddTriangleFilled(in[4], in[5], in[6], A(IM_COL32(255, 170, 160, 220), on));         /* (its highlight) */
        dl->AddPolyline(in, 8, A(IM_COL32(110, 10, 20, 255), on), ImDrawFlags_Closed, 0.8f * u);
        dl->AddPolyline(o, 8, A(IM_COL32(60, 0, 8, 255), on), ImDrawFlags_Closed, 1.2f * u);
        if (lum > 0) dl->AddCircleFilled(P(-5, -5), 1.6f * u, IM_COL32(255, 255, 255, (int)(220 * pulse)), 8);
        break; }
    case 1: {                                                   /* the powder of projection: a phial of red powder, a grain spilt */
        if (lum > 0) glow(dl, P(0, 8), 20 * u, IM_COL32(255, 80, 40, 255), lum * pulse * 0.8f);
        /* the bottle: a round belly and a neck */
        dl->AddCircleFilled(P(0, 7), 12 * u, A(IM_COL32(200, 220, 230, 90), on), 32);
        dl->AddRectFilled(P(-4, -14), P(4, -2), A(IM_COL32(200, 220, 230, 90), on));
        /* the powder inside: the belly's lower half */
        dl->PathClear(); for (int i = 0; i <= 16; i++) { float a = 3.14159f * i / 16; dl->PathLineTo(P(cosf(a) * 11, 7 + sinf(a) * 11)); } dl->PathFillConvex(A(RUBY, on));
        dl->AddRectFilledMultiColor(P(-11, 7), P(11, 9), A(RUBY_LT, on), A(RUBY_LT, on), A(RUBY, on), A(RUBY, on));
        dl->AddCircle(P(0, 7), 12 * u, A(IM_COL32(90, 110, 120, 255), on), 32, 1.1f * u);
        dl->AddLine(P(-4, -14), P(-4, -4.5f), A(IM_COL32(90, 110, 120, 255), on), 1.1f * u);
        dl->AddLine(P(4, -14), P(4, -4.5f), A(IM_COL32(90, 110, 120, 255), on), 1.1f * u);
        dl->AddRectFilled(P(-5, -19), P(5, -14), A(IM_COL32(150, 100, 60, 255), on), 1.5f * u);   /* the cork, */
        dl->AddRectFilled(P(-6.5f, -15), P(6.5f, -13), A(RUBY_DK, on));                          /* sealed with red wax */
        dl->AddLine(P(-7, 0), P(-3, 13), A(IM_COL32(255, 255, 255, 120), on), 1.6f * u);         /* (the glass's shine) */
        for (int i = 0; i < 5; i++) dl->AddCircleFilled(P(14 + i * 1.7f, 17 - (i % 2) * 1.2f), 0.9f * u, A(RUBY, on), 6);   /* a grain spilt */
        break; }
    case 2: {                                                   /* the philosophers' egg: the sealed vessel, the stone within, sun and moon joined above */
        if (lum > 0) glow(dl, P(0, 4), 22 * u, IM_COL32(255, 70, 40, 255), lum * pulse);
        dl->AddEllipseFilled(P(0, 4), ImVec2(11 * u, 15 * u), A(IM_COL32(210, 228, 236, 80), on), 0, 36);
        dl->AddEllipse(P(0, 4), ImVec2(11 * u, 15 * u), A(IM_COL32(80, 100, 110, 255), on), 0, 36, 1.2f * u);
        dl->AddRectFilled(P(-2.5f, -14), P(2.5f, -10.5f), A(IM_COL32(80, 100, 110, 255), on));   /* the sealed neck */
        dl->AddCircleFilled(P(0, 9), 6 * u, A(RUBY, on), 20);                                     /* the stone ripening */
        dl->AddCircleFilled(P(-2, 7), 2 * u, A(RUBY_LT, on), 12);
        /* sun and moon joined over it (gold disc, silver crescent) */
        dl->AddCircleFilled(P(-2, -20), 4.5f * u, A(GOLD, on), 20);
        for (int i = 0; i < 8; i++) { float a = i * 3.14159f / 4; dl->AddLine(P(-2 + cosf(a) * 5.5f, -20 + sinf(a) * 5.5f), P(-2 + cosf(a) * 7.5f, -20 + sinf(a) * 7.5f), A(GOLD_DK, on), 1.0f * u); }
        dl->PathClear(); dl->PathArcTo(P(9, -20), 4.2f * u, -1.9f, 1.9f, 16); dl->PathArcTo(P(7.4f, -20), 3.4f * u, 1.75f, -1.75f, 16);
        dl->PathFillConcave(A(IM_COL32(214, 220, 232, 255), on));                                   /* the moon, a crescent beside it */
        /* a wooden stand */
        dl->AddQuadFilled(P(-9, 17), P(9, 17), P(12, 21), P(-12, 21), A(IM_COL32(110, 70, 36, 255), on));
        dl->AddLine(P(-7, -2), P(-6, 10), A(IM_COL32(255, 255, 255, 140), on), 1.6f * u);
        break; }
    }
}
int ui_pstone_look(void) { int v = config_get_int("display.pstone_look", 0); return v >= 0 && v <= 2 ? v : 0; }
const char *ui_pstone_look_name(int k) { static const char *N[3] = { "The red stone", "The powder of projection", "The philosophers' egg" }; return k >= 0 && k <= 2 ? N[k] : "?"; }

/* test (DL_PSTONE_SHEET): the three looks, large, without and with a stone */
extern "C" void ui_pstone_sheet_draw(void)
{
    static int on = -1; if (on < 0) on = getenv("DL_PSTONE_SHEET") != 0;
    if (!on) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList(); ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(242, 230, 200, 255));
    static const char *WHY[3] = { "Red as a ruby, glassy, heavy, shining by its own light: the end of the Great Work.",
                                  "Ground to a red powder and kept in a sealed phial: a grain on molten lead makes gold.",
                                  "The sealed glass vessel of the Work, the red stone ripening inside; sun and moon joined." };
    float colw = ds.x / 3;
    for (int k = 0; k < 3; k++) {
        float cx = colw * (k + 0.5f);
        dl->AddText(ImGui::GetFont(), ds.y * 0.04f, ImVec2(cx - colw * 0.42f, ds.y * 0.05f), IM_COL32(110, 30, 20, 255), ui_pstone_look_name(k));
        ui_pstone_draw(dl, ImVec2(cx, ds.y * 0.38f), ds.y * 0.42f, k, 30, (float)ImGui::GetTime());
        dl->AddText(ImGui::GetFont(), ds.y * 0.026f, ImVec2(cx - colw * 0.42f, ds.y * 0.66f), IM_COL32(60, 40, 20, 255), WHY[k], nullptr, colw * 0.84f);
        for (int q = 0; q < 3; q++) {                         /* small: none, a weak stone, a strong one */
            static const int Q[3] = { 0, 8, 40 };
            ui_pstone_draw(dl, ImVec2(cx - colw * 0.25f + q * colw * 0.25f, ds.y * 0.86f), ds.y * 0.09f, k, Q[q], (float)ImGui::GetTime());
        }
    }
}
