/* ornament.cpp - the period ornaments (ornament.h) */
#include "ornament.h"
#include "imgui_internal.h"
#include <math.h>
#include "../runtime/config.h"

static const ImU32 BAND = IM_COL32(52, 30, 16, 255), STRAND_EDGE = IM_COL32(36, 18, 8, 255);
static const ImU32 COPPER = IM_COL32(194, 114, 56, 255), COPPER_LIT = IM_COL32(244, 184, 116, 255), COPPER_DK = IM_COL32(132, 70, 30, 255);
static const ImU32 GOLD = IM_COL32(210, 166, 76, 255), GOLD_LIT = IM_COL32(252, 228, 150, 255), GOLD_DK = IM_COL32(128, 90, 34, 255);
static const ImU32 VERMILION = IM_COL32(150, 36, 24, 255);

bool orn_on(void) { return config_get_int("display.ornate", 1) != 0; }

static ImVec2 mix(ImVec2 p, ImVec2 d, float t, ImVec2 n, float s) { return ImVec2(p.x + d.x * t + n.x * s, p.y + d.y * t + n.y * s); }

/* one side of the band: P to Q along its middle */
static void braid_side(ImDrawList *dl, ImVec2 P, ImVec2 Q, float w)
{
    float L = sqrtf((Q.x - P.x) * (Q.x - P.x) + (Q.y - P.y) * (Q.y - P.y));
    if (L < w) return;
    ImVec2 d((Q.x - P.x) / L, (Q.y - P.y) / L), n(-d.y, d.x);
    float half = w * 0.85f;                                     /* half a period: crossing to crossing */
    int m = (int)roundf(L / half); if (m < 2) m = 2;
    half = L / m;
    float A = w * 0.24f, th = fmaxf(1.5f, w * 0.30f);
    ImVec2 c0 = mix(P, d, 0, n, -w * 0.5f), c1 = mix(Q, d, 0, n, w * 0.5f);
    ImVec2 lo(fminf(c0.x, c1.x), fminf(c0.y, c1.y)), hi(fmaxf(c0.x, c1.x), fmaxf(c0.y, c1.y));
    dl->AddRectFilled(lo, hi, BAND);
    dl->PushClipRect(lo, hi, true);
    const int S = 8;
    ImVec2 pts[S + 1];
    auto strand = [&](float t0, float t1, float sg) {
        for (int i = 0; i <= S; i++) { float t = t0 + (t1 - t0) * i / S; pts[i] = mix(P, d, t, n, sg * A * sinf(3.14159265f * t / half)); }
        dl->AddPolyline(pts, S + 1, STRAND_EDGE, 0, th + fmaxf(1.0f, w * 0.12f));
        dl->AddPolyline(pts, S + 1, COPPER, 0, th);
        for (int i = 0; i <= S; i++) { pts[i].x -= n.x * th * 0.22f; pts[i].y -= n.y * th * 0.22f; }
        dl->AddPolyline(pts, S + 1, COPPER_LIT, 0, fmaxf(1.0f, th * 0.28f));
    };
    for (int k = 0; k <= m; k++) {                              /* each crossing: the strand under, then the one over */
        float t0 = (k - 0.5f) * half, t1 = (k + 0.5f) * half;
        float over = (k & 1) ? 1.0f : -1.0f;
        strand(t0, t1, -over);
        strand(t0, t1, over);
    }
    dl->PopClipRect();
    dl->AddLine(mix(P, d, 0, n, -w * 0.5f), mix(Q, d, 0, n, -w * 0.5f), GOLD_DK, 1.0f);
    dl->AddLine(mix(P, d, 0, n, w * 0.5f), mix(Q, d, 0, n, w * 0.5f), GOLD_DK, 1.0f);
}

static void lozenge(ImDrawList *dl, ImVec2 c, float r, bool jewel)
{
    ImVec2 q[4] = { ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r), ImVec2(c.x - r, c.y) };
    dl->AddConvexPolyFilled(q, 4, GOLD);
    ImVec2 t[3] = { q[3], q[0], q[1] };                         /* the upper half lit */
    dl->AddConvexPolyFilled(t, 3, IM_COL32(236, 200, 112, 255));
    dl->AddPolyline(q, 4, GOLD_DK, ImDrawFlags_Closed, fmaxf(1.0f, r * 0.14f));
    if (jewel) {
        dl->AddCircleFilled(c, r * 0.36f, VERMILION, 12);
        dl->AddCircleFilled(ImVec2(c.x - r * 0.1f, c.y - r * 0.12f), r * 0.12f, IM_COL32(255, 170, 140, 220), 8);
    }
}

void orn_braid_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    float h = w * 0.5f;
    braid_side(dl, ImVec2(a.x + h, a.y + h), ImVec2(b.x - h, a.y + h), w);
    braid_side(dl, ImVec2(a.x + h, b.y - h), ImVec2(b.x - h, b.y - h), w);
    braid_side(dl, ImVec2(a.x + h, a.y + h), ImVec2(a.x + h, b.y - h), w);
    braid_side(dl, ImVec2(b.x - h, a.y + h), ImVec2(b.x - h, b.y - h), w);
    float r = w * 0.78f;
    lozenge(dl, ImVec2(a.x + h, a.y + h), r, true); lozenge(dl, ImVec2(b.x - h, a.y + h), r, true);
    lozenge(dl, ImVec2(a.x + h, b.y - h), r, true); lozenge(dl, ImVec2(b.x - h, b.y - h), r, true);
}

void orn_gilt_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float s)
{
    float o = fmaxf(2.0f, s * 0.18f), lw = fmaxf(1.0f, s * 0.08f);
    dl->AddRect(ImVec2(a.x + o, a.y + o), ImVec2(b.x - o, b.y - o), GOLD_DK, 0, 0, lw * 1.6f);
    dl->AddRect(ImVec2(a.x + o + lw * 2.2f, a.y + o + lw * 2.2f), ImVec2(b.x - o - lw * 2.2f, b.y - o - lw * 2.2f), IM_COL32(176, 134, 60, 200), 0, 0, lw);
    float r = fmaxf(3.0f, s * 0.32f);
    lozenge(dl, ImVec2(a.x + o, a.y + o), r, false); lozenge(dl, ImVec2(b.x - o, a.y + o), r, false);
    lozenge(dl, ImVec2(a.x + o, b.y - o), r, false); lozenge(dl, ImVec2(b.x - o, b.y - o), r, false);
}

void orn_rule(ImDrawList *dl, float x0, float x1, float y, float s)
{
    float lw = fmaxf(1.0f, s * 0.07f), g = fmaxf(1.5f, s * 0.12f), r = fmaxf(3.0f, s * 0.3f), mx = (x0 + x1) * 0.5f;
    for (int side = -1; side <= 1; side += 2) {
        float e = side < 0 ? x0 : x1, in = mx + side * r * 1.6f;
        dl->AddLine(ImVec2(fminf(e, in), y - g), ImVec2(fmaxf(e, in), y - g), GOLD_DK, lw);
        dl->AddLine(ImVec2(fminf(e, in), y + g), ImVec2(fmaxf(e, in), y + g), GOLD_DK, lw);
        dl->AddCircleFilled(ImVec2(e, y), g * 1.1f, GOLD_DK, 10);
        dl->AddCircleFilled(ImVec2(mx + side * r * 1.25f, y), g * 0.8f, GOLD, 8);
    }
    lozenge(dl, ImVec2(mx, y), r, true);
}

void orn_rivet(ImDrawList *dl, ImVec2 c, float r)
{
    dl->AddCircleFilled(ImVec2(c.x + r * 0.2f, c.y + r * 0.25f), r, IM_COL32(30, 16, 6, 140), 12);
    dl->AddCircleFilled(c, r, GOLD_DK, 12);
    dl->AddCircleFilled(ImVec2(c.x - r * 0.12f, c.y - r * 0.12f), r * 0.74f, GOLD, 12);
    dl->AddCircleFilled(ImVec2(c.x - r * 0.32f, c.y - r * 0.32f), r * 0.3f, GOLD_LIT, 8);
}

static ImU32 shade(ImU32 c, float k, int a = -1)
{
    ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
    if (k >= 1) { float t = k - 1; v.x += (1 - v.x) * t; v.y += (1 - v.y) * t; v.z += (1 - v.z) * t; }
    else { v.x *= k; v.y *= k; v.z *= k; }
    if (a >= 0) v.w = a / 255.0f;
    return ImGui::ColorConvertFloat4ToU32(v);
}

void orn_plaque(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets)
{
    ImU32 c = base ? base : IM_COL32(186, 150, 84, 255);
    if (hover) c = shade(c, 1.18f);
    float h = b.y - a.y, my = a.y + h * 0.5f;
    /* lit like a strip of metal: bright at the top edge, dark through the middle, a rim of light below */
    dl->AddRectFilledMultiColor(a, ImVec2(b.x, my), shade(c, 1.35f), shade(c, 1.35f), shade(c, 0.92f), shade(c, 0.92f));
    dl->AddRectFilledMultiColor(ImVec2(a.x, my), b, shade(c, 0.92f), shade(c, 0.92f), shade(c, 1.12f), shade(c, 1.12f));
    float lw = fmaxf(1.0f, h * 0.05f);
    dl->AddLine(ImVec2(a.x + lw, a.y + lw), ImVec2(b.x - lw, a.y + lw), shade(c, 1.6f, 200), lw);
    dl->AddRect(a, b, shade(c, 0.45f), 0, 0, lw);
    if (rivets && b.x - a.x > h * 2.4f) {
        float r = fminf(h * 0.13f, 4.5f) + 0.8f;
        orn_rivet(dl, ImVec2(a.x + r * 2.2f, my), r);
        orn_rivet(dl, ImVec2(b.x - r * 2.2f, my), r);
    }
}

void orn_parchment_edges(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    ImU32 dk = IM_COL32(150, 104, 52, 70), cl = IM_COL32(150, 104, 52, 0);
    dl->AddRectFilledMultiColor(a, ImVec2(b.x, a.y + w), dk, dk, cl, cl);
    dl->AddRectFilledMultiColor(ImVec2(a.x, b.y - w), b, cl, cl, dk, dk);
    dl->AddRectFilledMultiColor(a, ImVec2(a.x + w, b.y), dk, cl, cl, dk);
    dl->AddRectFilledMultiColor(ImVec2(b.x - w, a.y), b, cl, dk, dk, cl);
}

void orn_windows(void)
{
    if (!orn_on()) return;
    ImGuiContext &g = *GImGui;
    ImVec2 ds = g.IO.DisplaySize;
    for (ImGuiWindow *w : g.Windows) {
        if (!w->WasActive || w->Hidden || (w->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoBackground))) continue;
        if (w->Size.x >= ds.x * 0.95f && w->Size.y >= ds.y * 0.95f) continue;   /* (a whole-screen page: the world map ...) */
        if (w->Size.x < 24 || w->Size.y < 16) continue;
        ImDrawList *dl = w->DrawList;
        ImVec2 a = w->Pos, b(w->Pos.x + w->Size.x, w->Pos.y + w->Size.y);
        dl->PushClipRectFullScreen();
        float fs = g.FontSize > 0 ? g.FontSize : 16;
        if (w->Flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup)) {
            orn_parchment_edges(dl, a, b, fs * 0.6f);
            orn_gilt_frame(dl, a, b, fs * 0.8f);
        } else if (!(w->Flags & ImGuiWindowFlags_NoTitleBar)) {   /* a title bar: the band outside, round the whole window */
            float bw = fmaxf(6.0f, fs * 0.5f);
            orn_parchment_edges(dl, ImVec2(a.x, a.y + w->TitleBarHeight), b, fs * 0.9f);
            orn_braid_frame(dl, ImVec2(a.x - bw, a.y - bw), ImVec2(b.x + bw, b.y + bw), bw);
        } else {
            float bw = fminf(fmaxf(6.0f, fs * 0.5f), fminf(w->WindowPadding.x, w->WindowPadding.y) * 0.85f);
            orn_parchment_edges(dl, ImVec2(a.x + bw, a.y + bw), ImVec2(b.x - bw, b.y - bw), fs * 0.9f);
            orn_braid_frame(dl, a, b, bw);
        }
        dl->PopClipRect();
    }
}

void orn_septext(const char *label)
{
    if (!orn_on()) { ImGui::SeparatorText(label); return; }
    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(ImGui::GetFontSize() * 1.3f, ImGui::GetStyle().SeparatorTextPadding.y));
    ImGui::SeparatorText(label);
    ImGui::PopStyleVar(2);
    ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float fs = ImGui::GetFontSize(), y = (a.y + b.y) * 0.5f, r = fs * 0.22f;
    const char *end = ImGui::FindRenderedTextEnd(label);
    float tw = ImGui::CalcTextSize(label, end).x;
    float tx0 = a.x + fs * 1.3f, tx1 = tx0 + tw;
    lozenge(dl, ImVec2(a.x + fs * 0.55f, y), r, false);
    float x0 = tx1 + fs * 0.5f, x1 = b.x - fs * 0.5f, lw = fmaxf(1.0f, fs * 0.06f), g = fmaxf(1.5f, fs * 0.1f);
    if (x1 - x0 > fs) {
        dl->AddLine(ImVec2(x0, y - g), ImVec2(x1 - r, y - g), GOLD_DK, lw);
        dl->AddLine(ImVec2(x0, y + g), ImVec2(x1 - r, y + g), IM_COL32(176, 134, 60, 200), lw);
        lozenge(dl, ImVec2(x1, y), r, false);
    }
}
