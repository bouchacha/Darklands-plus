/* hourglass.cpp - the game's time-passing clock (the "hourglass" of pass_hours_hourglass, s043
 * 150b:2a04), drawn by the interface wherever a Darklands+ page stands over the game's own: the
 * armillary of TIMER01.PIC with the sun-and-moon disc (TIMER03/04/05.PIC, turned by flipping
 * them as hourglass_frame 150b:2d3a does, one of eight positions for each three hours), and the
 * bell, the day and the month in its boxes. The game draws it at (60, 50) of the card window,
 * the disc at (108, 83). */
#include "imgui.h"
#include <stdio.h>
#include <string>
#include "gamepics.h"
#include "nsdraw.h"
#include "../mods/gamemodel.h"

void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month)
{
    const GamePic *base = gamepic("PICS\\TIMER01.PIC");
    if (!base || !base->tex) return;
    dl->AddImage((ImTextureID)(intptr_t)base->tex, tl, ImVec2(tl.x + base->w * kx, tl.y + base->h * k));
    /* the disc: the picture and its flips for each band of three hours (hourglass_frame) */
    static const struct { const char *pic; int fx, fy; } F[8] = {
        { "PICS\\TIMER03.PIC", 0, 1 }, { "PICS\\TIMER04.PIC", 1, 1 }, { "PICS\\TIMER05.PIC", 1, 0 }, { "PICS\\TIMER04.PIC", 1, 0 },
        { "PICS\\TIMER03.PIC", 0, 0 }, { "PICS\\TIMER04.PIC", 0, 0 }, { "PICS\\TIMER05.PIC", 0, 0 }, { "PICS\\TIMER04.PIC", 0, 1 },
    };
    int band = ((hour / 3) % 8 + 8) % 8;
    const GamePic *disc = gamepic(F[band].pic);
    if (disc && disc->tex) {
        ImVec2 a(tl.x + 48 * kx, tl.y + 33 * k), b(a.x + disc->w * kx, a.y + disc->h * k);
        ImVec2 uv0(F[band].fx ? 1.0f : 0.0f, F[band].fy ? 1.0f : 0.0f), uv1(F[band].fx ? 0.0f : 1.0f, F[band].fy ? 0.0f : 1.0f);
        dl->AddImage((ImTextureID)(intptr_t)disc->tex, a, b, uv0, uv1);
    }
    /* the boxes: the bell, the day, the month (as the game writes them: colour 14) */
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float fs = 8.0f * k;
    const ImU32 yellow = IM_COL32(252, 236, 84, 255);
    char bell[24]; gm_hour_name(hour, bell, sizeof bell);
    std::string b = ns_gtext(bell);
    char d[8], m[8]; snprintf(d, sizeof d, "%d", day); snprintf(m, sizeof m, "%d", month % 12 + 1);
    auto box = [&](float x0, float x1, const char *t) {
        ImVec2 sz = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        float s = fs;
        if (sz.x > (x1 - x0 - 2) * kx) { s = fs * (x1 - x0 - 2) * kx / sz.x; sz = f->CalcTextSizeA(s, FLT_MAX, 0, t); }
        dl->AddText(f, s, ImVec2(tl.x + (x0 + x1) * 0.5f * kx - sz.x * 0.5f, tl.y + 83.0f * k - sz.y * 0.5f), yellow, t);
    };
    box(8, 60, b.c_str());
    box(69, 86, d);
    box(95, 113, m);
}
