// tipplace.cpp - where every tooltip goes: beside the thing it describes, never over it nor over the
// pointer - below it, or above when there is no room below, else at its side, and inside the window.
// A tooltip placed by its code (ui_tip_beside) goes beside the thing named; any other (third_party/imgui
// BeginTooltipEx calls tip_auto) beside the smallest thing under the pointer: the last ImGui item, or a
// thing a native screen drew by hand (NsDraw::over, ui_hit_area), else beside the pointer itself. Each
// frame a check logs a tooltip found over the pointer or its thing (tools/uitour.py fails on it).
// ImGui places a tooltip by its size, which is known once it has been drawn: the size it had the frame
// before is used (the same tooltip, while the pointer stays).
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

static ImVec2 anc0, anc1; static int anc_frame = -1;     /* (the last thing a tooltip was put beside: the audit below) */

extern "C" { const unsigned *hdtext_cursor(int *x, int *y, int *w, int *h, int *stride); void hdtext_pointer_hotspot(int *hx, int *hy);
             void plat_game_to_win(float gx, float gy, float *wx, float *wy); }
/* where the pointer is drawn, in window pixels: the game's hand (its sprite's size, from its point)
   where the game draws it, else the system's arrow (its point at the mouse; larger on a large screen) */
ImRect ui_pointer_rect(void)
{
    ImVec2 mp = ImGui::GetIO().MousePos, ds = ImGui::GetIO().DisplaySize;
    int cx, cy, cw, ch, st;
    if (SDL_ShowCursor(SDL_QUERY) != SDL_ENABLE && hdtext_cursor(&cx, &cy, &cw, &ch, &st) && cw > 0 && ch > 0) {
        int hx, hy; hdtext_pointer_hotspot(&hx, &hy);
        float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
        float k = (y1 - y0) / 200.0f;
        return ImRect(mp.x - hx * k, mp.y - hy * k, mp.x + (cw - hx) * k, mp.y + (ch - hy) * k);
    }
    float f = fmaxf(1.0f, ds.y / 1080.0f);
    return ImRect(mp.x - 1, mp.y - 1, mp.x + 20 * f, mp.y + 32 * f);
}

ImVec4 ui_pointer_area(void) { ImRect r = ui_pointer_rect(); return ImVec4(r.Min.x, r.Min.y, r.Max.x, r.Max.y); }

/* beside the thing a0-a1, clear of it and of the pointer, inside the window: the places tried in the
   order given ('b' below, 'a' above, 'r' at its right, 'l' at its left), the first where the tooltip
   fits; none: the first, pushed inside the window */
void ui_tip_beside_in(ImVec2 a0, ImVec2 a1, const char *order)
{
    ImGuiContext &g = *ImGui::GetCurrentContext();
    anc0 = a0; anc1 = a1; anc_frame = g.FrameCount;
    ImVec2 ds = ImGui::GetIO().DisplaySize, sz(ImGui::GetFontSize() * 22, ImGui::GetFontSize() * 4);
    char name[32]; ImFormatString(name, sizeof name, "##Tooltip_%02d", g.TooltipOverrideCount);
    if (ImGuiWindow *w = ImGui::FindWindowByName(name)) if (w->Size.x > 0 && w->Size.y > 0) sz = w->Size;
    float m = 4.0f, gap = ImGui::GetFontSize() * 0.3f;
    ImRect p = ui_pointer_rect();
    bool valid = ImGui::IsMousePosValid();
    auto clampx = [&](float x) { return fminf(fmaxf(x, m), ds.x - sz.x - m); };
    auto clampy = [&](float y) { return fminf(fmaxf(y, m), ds.y - sz.y - m); };
    /* (none fits as it is: of the places pushed inside the window, the one least over the pointer, then
       least over the thing - a tooltip too big for the window may have to cover something) */
    auto over = [](ImRect a, ImRect b) { float w = fminf(a.Max.x, b.Max.x) - fmaxf(a.Min.x, b.Min.x), h = fminf(a.Max.y, b.Max.y) - fmaxf(a.Min.y, b.Min.y); return w > 0 && h > 0 ? w * h : 0.0f; };
    ImVec2 best(m, m); float best_pen = FLT_MAX;
    for (const char *o = order; *o; o++) {
        float x, y; bool fits;
        if (*o == 'b' || *o == 'a') {
            x = clampx(a0.x);
            bool cols = valid && p.Max.x > x && p.Min.x < x + sz.x;      /* (the pointer in the tooltip's columns: clear of it too) */
            if (*o == 'b') { y = fmaxf(a1.y, cols ? p.Max.y : -FLT_MAX) + gap; fits = y + sz.y <= ds.y - m; }
            else { y = fminf(a0.y, cols ? p.Min.y : FLT_MAX) - gap - sz.y; fits = y >= m; }
        } else {
            y = clampy(a0.y);
            bool rows = valid && p.Max.y > y && p.Min.y < y + sz.y;      /* (the pointer in its rows) */
            if (*o == 'r') { x = fmaxf(a1.x, rows ? p.Max.x : -FLT_MAX) + gap; fits = x + sz.x <= ds.x - m; }
            else { x = fminf(a0.x, rows ? p.Min.x : FLT_MAX) - gap - sz.x; fits = x >= m; }
        }
        if (fits) { ImGui::SetNextWindowPos(ImVec2(x, y)); return; }
        x = clampx(x); y = clampy(y);
        ImRect t(x, y, x + sz.x, y + sz.y);
        float pen = (valid ? over(t, p) * 1000.0f : 0.0f) + over(t, ImRect(a0, a1));
        if (pen < best_pen) { best_pen = pen; best = ImVec2(x, y); }
    }
    ImGui::SetNextWindowPos(best);
}
void ui_tip_beside(ImVec2 a0, ImVec2 a1) { ui_tip_beside_in(a0, a1, "barl"); }

/* the hit areas of this frame (NsDraw::over, a native screen's things under the pointer), each with the
   window it was drawn in */
struct Hit { ImVec2 a0, a1; ImGuiWindow *win; };
static Hit hits[64]; static int nhits, hits_frame = -1;
void ui_hit_area(ImVec2 a0, ImVec2 a1)
{
    ImGuiContext &g = *ImGui::GetCurrentContext();
    if (g.FrameCount != hits_frame) { hits_frame = g.FrameCount; nhits = 0; }
    if (nhits < 64) hits[nhits++] = { a0, a1, g.CurrentWindow };
}

/* every other tooltip (third_party/imgui BeginTooltipEx calls this when no place was given): beside
   the smallest thing under the pointer - the last ImGui item, or a hit area of the same window - and,
   when there is none (a thing drawn by hand), beside the pointer itself */
extern void (*GImGuiTooltipPlace)(void);
static void tip_auto(void)
{
    ImGuiContext &g = *ImGui::GetCurrentContext();
    ImVec2 mp = ImGui::GetIO().MousePos, ds = ImGui::GetIO().DisplaySize;
    float best = FLT_MAX; ImVec2 b0, b1;
    auto consider = [&](ImVec2 a0, ImVec2 a1) {
        float w = a1.x - a0.x, h = a1.y - a0.y;
        if (w <= 0 || h <= 0 || w > ds.x * 0.6f || h > ds.y * 0.6f) return;     /* (a whole window's button: not a thing) */
        if (mp.x < a0.x || mp.x >= a1.x || mp.y < a0.y || mp.y >= a1.y) return;
        if (w * h < best) { best = w * h; b0 = a0; b1 = a1; }
    };
    consider(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max);
    if (hits_frame == g.FrameCount) for (int i = 0; i < nhits; i++) if (hits[i].win == g.CurrentWindow) consider(hits[i].a0, hits[i].a1);
    if (best < FLT_MAX) { ui_tip_beside(b0, b1); return; }
    ImRect p = ui_pointer_rect();
    ui_tip_beside(p.Min, p.Max);
}
struct TipAutoInit { TipAutoInit() { GImGuiTooltipPlace = tip_auto; } };
static TipAutoInit tip_auto_init;

/* the check, every frame: a tooltip shown over the pointer, or over the thing it was put beside, goes
   to the log ("tooltip over ..."; the first dozen of a session, each place once) - tools/uitour.py
   points at every part of the screens and looks for it */
void ui_tip_audit(void)
{
    static int logged = 0, cap = -1; static ImRect last;
    static ImGuiID prev_id[8]; static ImRect prev_r[8]; static int nprev;      /* (each tooltip's place the frame before) */
    ImGuiID cur_id[8]; ImRect cur_r[8]; int ncur = 0;
    if (cap < 0) cap = getenv("DL_TIP_CHECK") ? 1 << 30 : 12;   /* (tools/uitour.py: all of them) */
    ImGuiContext &g = *ImGui::GetCurrentContext();
    if (logged >= cap || !ImGui::IsMousePosValid()) return;
    ImRect ptr = ui_pointer_rect(), anc(anc0.x + 1, anc0.y + 1, anc1.x - 1, anc1.y - 1);
    for (ImGuiWindow *w : g.Windows) {
        if (!(w->Flags & ImGuiWindowFlags_Tooltip) || !w->Active || w->Hidden || !strncmp(w->Name, "##Tooltip_DragDrop", 18)) continue;
        ImRect r = w->Rect();
        if (ncur < 8) { cur_id[ncur] = w->ID; cur_r[ncur++] = r; }
        bool stayed = false;
        for (int i = 0; i < nprev; i++) stayed = stayed || (prev_id[i] == w->ID && prev_r[i].Min.x == r.Min.x && prev_r[i].Min.y == r.Min.y && prev_r[i].Max.x == r.Max.x && prev_r[i].Max.y == r.Max.y);
        if (!stayed) continue;
        const char *what = r.Overlaps(ptr) ? "the pointer" : (anc_frame == g.FrameCount && r.Overlaps(anc)) ? "what it describes" : nullptr;
        if (!what) continue;
        ImRect rr(floorf(r.Min.x), floorf(r.Min.y), floorf(r.Max.x), floorf(r.Max.y));
        if (rr.Min.x == last.Min.x && rr.Min.y == last.Min.y && rr.Max.x == last.Max.x && rr.Max.y == last.Max.y) continue;
        last = rr; logged++;
        fprintf(stderr, "tooltip over %s: the pointer at %.0f,%.0f, the tooltip %.0f,%.0f-%.0f,%.0f\n", what, g.IO.MousePos.x, g.IO.MousePos.y, r.Min.x, r.Min.y, r.Max.x, r.Max.y);
    }
    nprev = ncur; memcpy(prev_id, cur_id, sizeof(ImGuiID) * ncur); memcpy(prev_r, cur_r, sizeof(ImRect) * ncur);
}
