/* vtrace.h - DL_VTX_TRACE (a test): how many vertices each part of the interface adds, printed every
   120 frames for the parts adding more than 2000: VT(call); */
#pragma once
#include <stdio.h>
#include "imgui.h"
#include "imgui_internal.h"
extern int vt_on, vt_frame;
static inline int vt_total(void)
{
    ImGuiContext &g = *ImGui::GetCurrentContext(); int n = ImGui::GetForegroundDrawList()->VtxBuffer.Size + ImGui::GetBackgroundDrawList()->VtxBuffer.Size;
    for (ImGuiWindow *w : g.Windows) n += w->DrawList->VtxBuffer.Size;
    return n;
}
#define VT(x) do { int _v0 = vt_on > 0 ? vt_total() : 0; x; if (vt_on > 0 && vt_frame % 120 == 0) { int _d = vt_total() - _v0; if (_d > 2000) fprintf(stderr, "vtx %-40.40s %d\n", #x, _d); } } while (0)
