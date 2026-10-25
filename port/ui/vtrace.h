/* vtrace.h - DL_VTX_TRACE (a test): how many vertices each part of the interface adds, printed every
   120 frames for the parts adding more than 2000: VT(call); */
#pragma once
#include <stdio.h>
#include <SDL.h>
#include "imgui.h"
#include "imgui_internal.h"
extern int vt_on, vt_frame;
static inline int vt_total(void)
{
    ImGuiContext &g = *ImGui::GetCurrentContext(); int n = ImGui::GetForegroundDrawList()->VtxBuffer.Size + ImGui::GetBackgroundDrawList()->VtxBuffer.Size;
    for (ImGuiWindow *w : g.Windows) n += w->DrawList->VtxBuffer.Size;
    return n;
}
/* DL_UI_TIME (a test): the time each part takes, a frame on average over 120 frames, printed for the
   parts above 0.3 ms */
extern int vt_time;
#define VT(x) do { Uint64 _t0 = vt_time > 0 ? SDL_GetPerformanceCounter() : 0; int _v0 = vt_on > 0 ? vt_total() : 0; x; \
    if (vt_on > 0 && vt_frame % 120 == 0) { int _d = vt_total() - _v0; if (_d > 2000) fprintf(stderr, "vtx %-40.40s %d\n", #x, _d); } \
    if (vt_time > 0) { static double _acc; _acc += (double)(SDL_GetPerformanceCounter() - _t0) * 1000.0 / (double)SDL_GetPerformanceFrequency(); \
        if (vt_frame % 120 == 0) { if (_acc / 120 > 0.3) fprintf(stderr, "ms %-40.40s %.2f\n", #x, _acc / 120); _acc = 0; } } } while (0)
/* VTMARK("name"): the vertices added since the last mark (within one function, marks in order) */
#define VTMARK(name) do { static int _vm; if (vt_on > 0) { int _t = vt_total(); if (vt_frame % 120 == 0 && _t - vt_last_mark > 2000) fprintf(stderr, "vtx %-40.40s %d\n", name, _t - vt_last_mark); vt_last_mark = _t; } (void)_vm; } while (0)
extern int vt_last_mark;
