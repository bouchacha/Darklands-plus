// tipplace.cpp - where a tooltip goes when it belongs to a thing on screen (a chance on a card, a
// member's chip ...): beside that thing, never over it nor over the pointer - below it, or above
// when there is no room below, and inside the window. ImGui places a tooltip by its size, which is
// known once it has been drawn: the size it had the frame before is used (the same tooltip, while
// the pointer stays).
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>

void ui_tip_beside(ImVec2 a0, ImVec2 a1)
{
    ImGuiContext &g = *ImGui::GetCurrentContext();
    ImVec2 ds = ImGui::GetIO().DisplaySize, sz(ImGui::GetFontSize() * 22, ImGui::GetFontSize() * 4);
    char name[32]; ImFormatString(name, sizeof name, "##Tooltip_%02d", g.TooltipOverrideCount);
    if (ImGuiWindow *w = ImGui::FindWindowByName(name)) if (w->Size.x > 0 && w->Size.y > 0) sz = w->Size;
    float m = 4.0f, gap = ImGui::GetFontSize() * 0.3f;
    ImVec2 mp = ImGui::GetIO().MousePos;
    float clear = ImGui::GetFontSize() * 1.8f;                   /* (the game's pointer, a gauntlet, hangs below and right of its point) */
    float below = fmaxf(a1.y + gap, ImGui::IsMousePosValid() && mp.x >= a0.x - clear && mp.x <= a1.x + clear ? mp.y + clear : 0);
    float y;
    if (below + sz.y <= ds.y - m) y = below;                      /* below */
    else if (a0.y - gap - sz.y >= m) y = a0.y - gap - sz.y;       /* above */
    else y = (ds.y - a1.y > a0.y) ? ds.y - m - sz.y : m;          /* (no room either way: the side with more, at the window's edge) */
    float x = fminf(fmaxf(a0.x, m), ds.x - sz.x - m);
    ImGui::SetNextWindowPos(ImVec2(x, y));
}
