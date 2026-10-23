/* ornament.cpp - the period ornaments (ornament.h) */
#include "ornament.h"
#include "imgui_internal.h"
#include <math.h>
#include <float.h>
#include "../runtime/config.h"
#include "drawcache.h"

static const ImU32 BAND = IM_COL32(52, 30, 16, 255), STRAND_EDGE = IM_COL32(36, 18, 8, 255);
static const ImU32 COPPER = IM_COL32(194, 114, 56, 255), COPPER_LIT = IM_COL32(244, 184, 116, 255), COPPER_DK = IM_COL32(132, 70, 30, 255);
static const ImU32 GOLD = IM_COL32(210, 166, 76, 255), GOLD_LIT = IM_COL32(252, 228, 150, 255), GOLD_DK = IM_COL32(128, 90, 34, 255);
static const ImU32 VERMILION = IM_COL32(150, 36, 24, 255);

int orn_style(void) { int v = config_get_int("display.ornate", 1); return v < 0 || v > 4 ? 1 : v; }
bool orn_on(void) { return orn_style() != 0; }

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
    unsigned long long key = 0xb4a1dull;                       /* (drawn once, kept as a picture: ui/drawcache.h) */
    for (float v : { P.x - floorf(lo.x), P.y - floorf(lo.y), Q.x - floorf(lo.x), Q.y - floorf(lo.y), w }) key = dc_key(key, v);
    dc_draw(dl, lo, hi, key, [&](ImDrawList *dl) {
    dl->AddRectFilled(lo, hi, BAND);
    dl->PushClipRect(lo, hi, true);
    const int S = 8;
    ImVec2 pts[S + 1];
    auto strand = [&](float t0, float t1, float sg, bool edge = true) {
        for (int i = 0; i <= S; i++) { float t = t0 + (t1 - t0) * i / S; pts[i] = mix(P, d, t, n, sg * A * sinf(3.14159265f * t / half)); }
        if (edge) dl->AddPolyline(pts, S + 1, STRAND_EDGE, 0, th + fmaxf(1.0f, w * 0.12f));
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
    for (int k = 0; k <= m; k++) {                              /* (the joins fall on the crests: drawn over again, whole) */
        float c = (k + 0.5f) * half, e = half * 0.22f;
        strand(c - e, c + e, 1.0f, true); strand(c - e, c + e, -1.0f, true);
    }
    dl->PopClipRect();
    });
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
/* the same band with plaited corners, as the game's cards: the sides run corner to corner, their
   strands meeting at each corner where a little copper loop ties them (no jewel) */
void orn_braid_frame_knotted(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    float h = w * 0.5f;
    ImVec2 C[4] = { ImVec2(a.x + h, a.y + h), ImVec2(b.x - h, a.y + h), ImVec2(b.x - h, b.y - h), ImVec2(a.x + h, b.y - h) };
    for (int q = 0; q < 4; q++) {                               /* the corner squares, under all */
        ImVec2 lo(C[q].x - h, C[q].y - h), hi(C[q].x + h, C[q].y + h);
        dl->AddRectFilled(lo, hi, BAND);
    }
    braid_side(dl, C[0], C[1], w); braid_side(dl, C[3], C[2], w);
    braid_side(dl, C[0], C[3], w); braid_side(dl, C[1], C[2], w);
    float th = fmaxf(1.5f, w * 0.30f), r = w * 0.33f;   /* (wide enough to take in the strands where they leave the sides) */
    for (int q = 0; q < 4; q++) {                               /* the loop where the four strands meet */
        dl->AddCircle(C[q], r, STRAND_EDGE, 20, th + fmaxf(1.0f, w * 0.12f));
        dl->AddCircle(C[q], r, COPPER, 20, th);
        dl->PathArcTo(C[q], r - th * 0.22f, 3.14159265f * 1.05f, 3.14159265f * 1.6f, 10); dl->PathStroke(COPPER_LIT, 0, fmaxf(1.0f, th * 0.28f));
    }
}

static void gilt_frame_rule(ImDrawList *dl, ImVec2 a, ImVec2 b, float s)
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

static void plaque_metal(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets)
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
            float bw = fmaxf(8.0f, fs * 0.66f);
            orn_parchment_edges(dl, ImVec2(a.x, a.y + w->TitleBarHeight), b, fs * 0.9f);
            orn_frame(dl, ImVec2(a.x - bw, a.y - bw), ImVec2(b.x + bw, b.y + bw), bw);
        } else {
            float bw = fminf(fmaxf(6.0f, fs * 0.5f), fminf(w->WindowPadding.x, w->WindowPadding.y) * 0.85f);
            orn_parchment_edges(dl, ImVec2(a.x + bw, a.y + bw), ImVec2(b.x - bw, b.y - bw), fs * 0.9f);
            orn_frame(dl, a, b, bw);
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

void orn_hairline(ImDrawList *dl, ImVec2 a, ImVec2 b, float g, bool across)
{
    float lw = fmaxf(1.0f, g * 0.45f), h = g * 0.5f;
    if (across) {
        float r = g * 1.6f;
        dl->AddLine(ImVec2(a.x, a.y - h), ImVec2(b.x - r, a.y - h), IM_COL32(150, 108, 44, 220), lw);
        dl->AddLine(ImVec2(a.x, a.y + h), ImVec2(b.x - r, a.y + h), IM_COL32(196, 156, 80, 170), lw);
        lozenge(dl, ImVec2(b.x - r * 0.6f, a.y), r * 0.6f, false);
    } else {
        dl->AddLine(ImVec2(a.x - h, a.y), ImVec2(a.x - h, b.y), IM_COL32(150, 108, 44, 200), lw);
        dl->AddLine(ImVec2(a.x + h, a.y), ImVec2(a.x + h, b.y), IM_COL32(196, 156, 80, 150), lw);
    }
}

void orn_tab_mark(ImDrawList *dl, ImVec2 a, ImVec2 b, float k)
{
    dl->AddLine(ImVec2(a.x + k, a.y - k * 0.6f), ImVec2(b.x - k, b.y - k * 0.6f), GOLD, fmaxf(1.0f, k * 0.35f));
    lozenge(dl, ImVec2((a.x + b.x) * 0.5f, a.y - k * 0.6f), k * 0.9f, false);
}

/* ---------------------------------------------------------------- the other bands
   walnut and brass: the party sheet's wooden panels and brass name plates (PTYSTATS);
   illuminated: the illuminated capitals' vermilion and gold diaper (ILLMCAPS), as a manuscript's border;
   gothic stone: the carved stone niche the paper doll stands in, quatrefoils in a moulding */
static void band_quads(ImDrawList *dl, ImVec2 a, ImVec2 b, float w, ImU32 c)   /* the band's four sides, filled */
{
    dl->AddRectFilled(a, ImVec2(b.x, a.y + w), c);
    dl->AddRectFilled(ImVec2(a.x, b.y - w), b, c);
    dl->AddRectFilled(ImVec2(a.x, a.y + w), ImVec2(a.x + w, b.y - w), c);
    dl->AddRectFilled(ImVec2(b.x - w, a.y + w), ImVec2(b.x, b.y - w), c);
}
/* along the band's middle line, every `step`: fn(point, along-x?) */
template <class F> static void along(ImVec2 a, ImVec2 b, float w, float step, F fn)
{
    float h = w * 0.5f;
    for (int side = 0; side < 4; side++) {
        bool across = side < 2;
        float L = across ? (b.x - a.x - 2 * w) : (b.y - a.y - 2 * w);
        int n = (int)floorf(L / step); if (n < 1) continue;
        float st = L / n;
        for (int i = 0; i < n; i++) {
            float t = (i + 0.5f) * st + w;
            ImVec2 p = across ? ImVec2(a.x + t, side == 0 ? a.y + h : b.y - h) : ImVec2(side == 2 ? a.x + h : b.x - h, a.y + t);
            fn(p, across);
        }
    }
}

static void walnut_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    const ImU32 dark = IM_COL32(58, 32, 16, 255), mid = IM_COL32(112, 64, 30, 255), lite = IM_COL32(156, 96, 48, 255);
    /* the moulding: dark at its outer edge, lit along its crest, a groove inside */
    band_quads(dl, a, b, w, mid);
    float h = w * 0.5f; (void)h;
    for (int i = 0; i < 3; i++) {                                 /* the grain: wavering lines along each side */
        float off = w * (0.22f + 0.28f * i);
        ImU32 gc = i == 1 ? IM_COL32(90, 50, 22, 200) : IM_COL32(134, 80, 38, 200);
        const int N = 40;
        for (int side = 0; side < 4; side++) {
            ImVec2 pts[N];
            for (int j = 0; j < N; j++) {
                float t = (float)j / (N - 1), wob = sinf(t * 23.0f + i * 2.1f + side) * w * 0.05f;
                if (side == 0) pts[j] = ImVec2(a.x + w + t * (b.x - a.x - 2 * w), a.y + off + wob);
                else if (side == 1) pts[j] = ImVec2(a.x + w + t * (b.x - a.x - 2 * w), b.y - off + wob);
                else if (side == 2) pts[j] = ImVec2(a.x + off + wob, a.y + w + t * (b.y - a.y - 2 * w));
                else pts[j] = ImVec2(b.x - off + wob, a.y + w + t * (b.y - a.y - 2 * w));
            }
            dl->AddPolyline(pts, N, gc, 0, fmaxf(1.0f, w * 0.07f));
        }
    }
    float lw = fmaxf(1.0f, w * 0.1f);
    dl->AddRect(a, b, dark, 0, 0, lw * 1.5f);
    dl->AddRect(ImVec2(a.x + w * 0.3f, a.y + w * 0.3f), ImVec2(b.x - w * 0.3f, b.y - w * 0.3f), lite, 0, 0, lw);
    dl->AddRect(ImVec2(a.x + w - lw, a.y + w - lw), ImVec2(b.x - w + lw, b.y - w + lw), dark, 0, 0, lw * 1.4f);
    /* brass corner plates, each with its rivets, and rivets along the sides */
    float cs = w * 2.6f;
    for (int cx = 0; cx < 2; cx++) for (int cy = 0; cy < 2; cy++) {
        ImVec2 o(cx ? b.x : a.x, cy ? b.y : a.y);
        float sx = cx ? -1.0f : 1.0f, sy = cy ? -1.0f : 1.0f;
        ImU32 br = IM_COL32(204, 160, 72, 255);                  /* an L of brass along the band, no further in */
        ImVec2 q[6] = { o, ImVec2(o.x + sx * cs, o.y), ImVec2(o.x + sx * cs, o.y + sy * w), ImVec2(o.x + sx * w, o.y + sy * w),
                        ImVec2(o.x + sx * w, o.y + sy * cs), ImVec2(o.x, o.y + sy * cs) };
        dl->AddRectFilled(ImVec2(fminf(o.x, q[2].x), fminf(o.y, q[2].y)), ImVec2(fmaxf(o.x, q[2].x), fmaxf(o.y, q[2].y)), br);
        dl->AddRectFilled(ImVec2(fminf(o.x, q[4].x), fminf(o.y, q[4].y)), ImVec2(fmaxf(o.x, q[4].x), fmaxf(o.y, q[4].y)), br);
        dl->AddLine(ImVec2(o.x, o.y + sy * lw), ImVec2(q[1].x, o.y + sy * lw), GOLD_LIT, lw);
        dl->AddPolyline(q, 6, GOLD_DK, ImDrawFlags_Closed, lw);
        orn_rivet(dl, ImVec2(o.x + sx * w * 0.55f, o.y + sy * w * 0.55f), w * 0.2f);
        orn_rivet(dl, ImVec2(o.x + sx * cs * 0.75f, o.y + sy * w * 0.5f), w * 0.14f);
        orn_rivet(dl, ImVec2(o.x + sx * w * 0.5f, o.y + sy * cs * 0.75f), w * 0.14f);
    }
    along(a, b, w, w * 7, [&](ImVec2 p, bool across) {
        float t0 = across ? p.x - a.x : p.y - a.y, t1 = across ? b.x - p.x : b.y - p.y;
        if (t0 < cs + w || t1 < cs + w) return;               /* (not on the corner plates) */
        orn_rivet(dl, p, w * 0.15f);
    });
}

static void illum_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    const ImU32 red = IM_COL32(150, 34, 22, 255), redd = IM_COL32(104, 20, 12, 255), blue = IM_COL32(34, 58, 140, 255);
    band_quads(dl, a, b, w, red);
    /* the diaper: gold lozenges with a white pearl between, as on the capitals */
    float s = w * 0.62f;
    along(a, b, w, s * 1.2f, [&](ImVec2 p, bool across) {
        float r = w * 0.26f;
        ImVec2 q[4] = { ImVec2(p.x, p.y - r), ImVec2(p.x + r, p.y), ImVec2(p.x, p.y + r), ImVec2(p.x - r, p.y) };
        dl->AddConvexPolyFilled(q, 4, IM_COL32(226, 180, 84, 255));
        ImVec2 pe = across ? ImVec2(p.x + s * 0.6f, p.y) : ImVec2(p.x, p.y + s * 0.6f);
        dl->AddCircleFilled(pe, w * 0.07f, IM_COL32(250, 240, 220, 230), 6);
    });
    float lw = fmaxf(1.0f, w * 0.12f);
    dl->AddRect(a, b, GOLD, 0, 0, lw * 1.4f);
    dl->AddRect(ImVec2(a.x + w, a.y + w), ImVec2(b.x - w, b.y - w), GOLD, 0, 0, lw * 1.2f);
    dl->AddRect(ImVec2(a.x + w + lw * 1.6f, a.y + w + lw * 1.6f), ImVec2(b.x - w - lw * 1.6f, b.y - w - lw * 1.6f), redd, 0, 0, lw * 0.7f);
    /* the corners: a gold square, blue within, a red quatrefoil and a gold pearl */
    for (int cx = 0; cx < 2; cx++) for (int cy = 0; cy < 2; cy++) {
        ImVec2 c(cx ? b.x - w * 0.5f : a.x + w * 0.5f, cy ? b.y - w * 0.5f : a.y + w * 0.5f);
        float r = w * 0.85f;
        dl->AddRectFilled(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), GOLD);
        dl->AddRectFilled(ImVec2(c.x - r * 0.78f, c.y - r * 0.78f), ImVec2(c.x + r * 0.78f, c.y + r * 0.78f), blue);
        for (int i = 0; i < 4; i++) { float an = i * 1.5708f; dl->AddCircleFilled(ImVec2(c.x + cosf(an) * r * 0.32f, c.y + sinf(an) * r * 0.32f), r * 0.3f, red, 12); }
        dl->AddCircleFilled(c, r * 0.2f, GOLD_LIT, 10);
        dl->AddRect(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), GOLD_DK, 0, 0, lw);
    }
}

static void gothic_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w)
{
    const ImU32 stone = IM_COL32(176, 168, 152, 255), lit = IM_COL32(222, 216, 200, 255), shadow = IM_COL32(104, 96, 84, 255), deep = IM_COL32(70, 62, 54, 255);
    band_quads(dl, a, b, w, stone);
    float lw = fmaxf(1.0f, w * 0.1f);
    /* the moulding: lit above and left, in shadow below and right */
    dl->AddLine(ImVec2(a.x, a.y + lw * 0.5f), ImVec2(b.x, a.y + lw * 0.5f), lit, lw * 1.5f);
    dl->AddLine(ImVec2(a.x + lw * 0.5f, a.y), ImVec2(a.x + lw * 0.5f, b.y), lit, lw * 1.5f);
    dl->AddLine(ImVec2(a.x, b.y - lw * 0.5f), ImVec2(b.x, b.y - lw * 0.5f), shadow, lw * 1.5f);
    dl->AddLine(ImVec2(b.x - lw * 0.5f, a.y), ImVec2(b.x - lw * 0.5f, b.y), shadow, lw * 1.5f);
    dl->AddRect(ImVec2(a.x + w - lw, a.y + w - lw), ImVec2(b.x - w + lw, b.y - w + lw), deep, 0, 0, lw * 1.3f);
    /* quatrefoils carved along it */
    along(a, b, w, w * 1.5f, [&](ImVec2 p, bool) {
        float r = w * 0.17f;
        for (int i = 0; i < 4; i++) {
            float an = i * 1.5708f + 0.7854f;
            ImVec2 q(p.x + cosf(an) * r, p.y + sinf(an) * r);
            dl->AddCircleFilled(ImVec2(q.x + lw * 0.4f, q.y + lw * 0.4f), r * 0.95f, lit, 12);
            dl->AddCircleFilled(q, r * 0.95f, shadow, 12);
        }
        dl->AddCircleFilled(p, r * 0.5f, deep, 8);
    });
    /* the corners: a square boss with a rose */
    for (int cx = 0; cx < 2; cx++) for (int cy = 0; cy < 2; cy++) {
        ImVec2 c(cx ? b.x - w * 0.5f : a.x + w * 0.5f, cy ? b.y - w * 0.5f : a.y + w * 0.5f);
        float r = w * 0.8f;
        dl->AddRectFilled(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), stone);
        dl->AddRect(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), deep, 0, 0, lw);
        for (int i = 0; i < 6; i++) {
            float an = i * 1.0472f;
            dl->AddCircleFilled(ImVec2(c.x + cosf(an) * r * 0.42f, c.y + sinf(an) * r * 0.42f), r * 0.3f, i < 3 ? shadow : lit, 12);
        }
        dl->AddCircleFilled(c, r * 0.3f, IM_COL32(150, 36, 24, 255), 12);
        dl->AddCircleFilled(ImVec2(c.x - r * 0.08f, c.y - r * 0.08f), r * 0.1f, IM_COL32(255, 180, 150, 220), 8);
    }
}

void orn_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w) { orn_frame_style(dl, a, b, w, orn_style()); }
void orn_frame_style(ImDrawList *dl, ImVec2 a, ImVec2 b, float w, int style)
{
    switch (style) {
    case 2: walnut_frame(dl, a, b, w * 1.15f); break;
    case 3: illum_frame(dl, a, b, w); break;
    case 4: gothic_frame(dl, a, b, w * 1.1f); break;
    default: orn_braid_frame(dl, a, b, w); break;
    }
}
const char *orn_style_name(int s) { static const char *N[5] = { "Off", "Plaited copper", "Walnut and brass", "Illuminated", "Gothic stone" }; return s >= 0 && s <= 4 ? N[s] : "?"; }

/* a small window in the given style, for the settings' choice */
void orn_preview(ImDrawList *dl, ImVec2 a, ImVec2 b, int style, ImFont *f)
{
    float w = (b.x - a.x) * 0.06f;
    dl->AddRectFilled(a, b, IM_COL32(237, 222, 189, 255), style ? 0 : 6);
    if (!style) {
        dl->AddRect(a, b, IM_COL32(107, 77, 41, 204), 6, 0, 1.5f);
        dl->AddLine(ImVec2(a.x + w * 1.5f, a.y + w * 3.2f), ImVec2(b.x - w * 1.5f, a.y + w * 3.2f), IM_COL32(140, 107, 64, 255), 1.5f);
    } else {
        orn_parchment_edges(dl, ImVec2(a.x + w, a.y + w), ImVec2(b.x - w, b.y - w), w * 1.5f);
        orn_frame_style(dl, a, b, w, style);
        float y = a.y + w * 3.2f, x0 = a.x + w * 1.8f;
        lozenge(dl, ImVec2(x0, y), w * 0.35f, false);
        dl->AddLine(ImVec2(x0 + w * 5, y - w * 0.12f), ImVec2(b.x - w * 2.2f, y - w * 0.12f), GOLD_DK, 1.2f);
        dl->AddLine(ImVec2(x0 + w * 5, y + w * 0.12f), ImVec2(b.x - w * 2.2f, y + w * 0.12f), IM_COL32(176, 134, 60, 200), 1.2f);
        lozenge(dl, ImVec2(b.x - w * 1.9f, y), w * 0.3f, false);
    }
    float ui_face_k(ImFont *f);
    if (f) dl->AddText(f, w * 1.6f * ui_face_k(f), ImVec2(a.x + w * (style ? 2.6f : 1.6f), a.y + w * 2.3f), IM_COL32(44, 26, 10, 255), "Tasks");
    for (int i = 0; i < 3; i++) {                                  /* lines of text */
        float y = a.y + w * (5.2f + i * 1.4f);
        dl->AddRectFilled(ImVec2(a.x + w * 1.8f, y), ImVec2(a.x + w * (1.8f + 9 - i * 2.2f), y + w * 0.5f), IM_COL32(120, 92, 60, 150), 2);
    }
    ImVec2 p0(b.x - w * 7.5f, b.y - w * 3.6f), p1(b.x - w * 1.8f, b.y - w * 1.6f);
    if (style) orn_plaque(dl, p0, p1, IM_COL32(120, 36, 20, 255), false, true);
    else dl->AddRectFilled(p0, p1, IM_COL32(112, 40, 22, 235), 4);
}

/* ---------------------------------------------------------------- the choices for frames and buttons
   frames (display.frame_style): 0 a double gilt rule with lozenges; 1 rubricated: a vermilion
   rule inside a fine ink one, a small fleuron at each corner (as a manuscript's ruled text block);
   2 pen flourishes at the corners only; 3 blind tooling: a dark leather line and a dotted one
   within, as a binding.
   buttons (display.button_style): 0 a lit metal plaque; 1 a rubric: flat colour in a fine gold
   line, nothing raised; 2 a seal: a rounded tab, softly domed; 3 lettered: the word on parchment,
   underlined, coloured only when pointed at */
static int frame_style(void) { return config_get_int("display.frame_style", 0) & 3; }
static int button_style(void) { return config_get_int("display.button_style", 0) & 3; }

static void fleuron(ImDrawList *dl, ImVec2 c, float r, ImU32 col)       /* a small three-leaved sprig */
{
    for (int i = 0; i < 3; i++) { float t = -1.5708f + (i - 1) * 0.9f; dl->AddCircleFilled(ImVec2(c.x + cosf(t) * r * 0.55f, c.y + sinf(t) * r * 0.55f), r * 0.38f, col, 10); }
    dl->AddCircleFilled(c, r * 0.3f, col, 8);
}
void orn_frame_style_box(ImDrawList *dl, ImVec2 a, ImVec2 b, float s, int st)
{
    float lw = fmaxf(1.0f, s * 0.08f), o = fmaxf(2.0f, s * 0.18f);
    switch (st) {
    case 1: {
        const ImU32 VER = IM_COL32(168, 40, 26, 230), INKC = IM_COL32(40, 24, 12, 160);
        dl->AddRect(ImVec2(a.x + o, a.y + o), ImVec2(b.x - o, b.y - o), VER, 0, 0, lw * 1.4f);
        dl->AddRect(ImVec2(a.x + o + lw * 2.4f, a.y + o + lw * 2.4f), ImVec2(b.x - o - lw * 2.4f, b.y - o - lw * 2.4f), INKC, 0, 0, lw * 0.6f);
        float r = fmaxf(3.0f, s * 0.3f);
        fleuron(dl, ImVec2(a.x + o, a.y + o), r, VER); fleuron(dl, ImVec2(b.x - o, a.y + o), r, VER);
        fleuron(dl, ImVec2(a.x + o, b.y - o), r, VER); fleuron(dl, ImVec2(b.x - o, b.y - o), r, VER);
        break; }
    case 2: {                                                            /* pen flourishes at the corners */
        const ImU32 PEN = IM_COL32(60, 34, 16, 210);
        float L = fminf(s * 2.6f, fminf(b.x - a.x, b.y - a.y) * 0.3f);
        for (int cx = 0; cx < 2; cx++) for (int cy = 0; cy < 2; cy++) {
            float sx = cx ? -1.0f : 1.0f, sy = cy ? -1.0f : 1.0f;
            ImVec2 c(cx ? b.x - o : a.x + o, cy ? b.y - o : a.y + o);
            dl->AddBezierCubic(ImVec2(c.x + sx * L, c.y), ImVec2(c.x + sx * L * 0.3f, c.y - sy * L * 0.12f), ImVec2(c.x, c.y), ImVec2(c.x, c.y + sy * L), PEN, lw * 1.2f);
            dl->AddBezierCubic(ImVec2(c.x + sx * L * 0.2f, c.y + sy * L * 0.2f), ImVec2(c.x + sx * L * 0.55f, c.y + sy * L * 0.05f), ImVec2(c.x + sx * L * 0.5f, c.y + sy * L * 0.45f), ImVec2(c.x + sx * L * 0.3f, c.y + sy * L * 0.35f), PEN, lw);
            dl->AddCircleFilled(ImVec2(c.x + sx * L, c.y), lw * 1.3f, PEN, 8);
            dl->AddCircleFilled(ImVec2(c.x, c.y + sy * L), lw * 1.3f, PEN, 8);
        }
        break; }
    case 3: {                                                            /* blind tooling */
        const ImU32 LEA = IM_COL32(86, 52, 26, 220), LEA_L = IM_COL32(160, 118, 72, 160);
        dl->AddRect(ImVec2(a.x + o, a.y + o), ImVec2(b.x - o, b.y - o), LEA, 0, 0, lw * 1.8f);
        dl->AddRect(ImVec2(a.x + o + lw, a.y + o + lw), ImVec2(b.x - o + lw, b.y - o + lw), LEA_L, 0, 0, lw * 0.6f);
        float in = o + lw * 3.4f, step = fmaxf(4.0f, s * 0.42f);
        for (float x = a.x + in; x <= b.x - in; x += step) { dl->AddCircleFilled(ImVec2(x, a.y + in), lw * 0.75f, LEA, 6); dl->AddCircleFilled(ImVec2(x, b.y - in), lw * 0.75f, LEA, 6); }
        for (float y = a.y + in + step; y <= b.y - in - step * 0.5f; y += step) { dl->AddCircleFilled(ImVec2(a.x + in, y), lw * 0.75f, LEA, 6); dl->AddCircleFilled(ImVec2(b.x - in, y), lw * 0.75f, LEA, 6); }
        break; }
    default: gilt_frame_rule(dl, a, b, s);
    }
}
void orn_gilt_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float s) { orn_frame_style_box(dl, a, b, s, frame_style()); }

void orn_button_style(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets, int st)
{
    ImU32 c = base ? base : IM_COL32(186, 150, 84, 255);
    float h = b.y - a.y, lw = fmaxf(1.0f, h * 0.05f);
    switch (st) {
    case 1: {                                                            /* a rubric: flat, a fine gold line within */
        dl->AddRectFilled(a, b, hover ? shade(c, 1.15f) : c, h * 0.08f);
        dl->AddRect(ImVec2(a.x + lw * 1.6f, a.y + lw * 1.6f), ImVec2(b.x - lw * 1.6f, b.y - lw * 1.6f), IM_COL32(226, 186, 96, 200), h * 0.05f, 0, lw * 0.8f);
        break; }
    case 2: {                                                            /* a seal: a rounded tab, domed */
        float r = h * 0.5f;
        dl->AddRectFilled(ImVec2(a.x, a.y + lw), ImVec2(b.x, b.y + lw), IM_COL32(0, 0, 0, 60), r);
        dl->AddRectFilled(a, b, hover ? shade(c, 1.12f) : c, r);
        dl->AddRectFilled(ImVec2(a.x + r * 0.5f, a.y + h * 0.1f), ImVec2(b.x - r * 0.5f, a.y + h * 0.42f), shade(c, 1.3f, 110), r * 0.6f);
        dl->AddRect(a, b, shade(c, 0.55f), r, 0, lw);
        break; }
    case 3: {                                                            /* lettered: only the word, underlined */
        if (hover) dl->AddRectFilled(a, b, shade(c, 1.0f, 60), h * 0.1f);
        dl->AddLine(ImVec2(a.x + h * 0.4f, b.y - lw * 1.5f), ImVec2(b.x - h * 0.4f, b.y - lw * 1.5f), shade(c, 0.9f), lw * 1.2f);
        dl->AddCircleFilled(ImVec2(a.x + h * 0.3f, b.y - lw * 1.5f), lw * 1.3f, shade(c, 0.9f), 8);
        dl->AddCircleFilled(ImVec2(b.x - h * 0.3f, b.y - lw * 1.5f), lw * 1.3f, shade(c, 0.9f), 8);
        break; }
    default: plaque_metal(dl, a, b, base, hover, rivets);
    }
}
void orn_plaque(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets) { orn_button_style(dl, a, b, base, hover, rivets, button_style()); }
int orn_button_lettered(void) { return orn_on() && button_style() == 3; }

/* test: the frames and buttons of each style side by side (ucmd uornsheet:) */
static int orn_sheet;
extern "C" void orn_test_sheet(void) { orn_sheet = !orn_sheet; }
extern "C" void orn_test_sheet_draw(void)
{
    if (!orn_sheet) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImFont *f = ImGui::GetFont(); float fs = 17;
    ImVec2 o(16, 16);
    float cw = 226, ch = 150;
    dl->AddRectFilled(o, ImVec2(o.x + cw * 4 + 30, o.y + ch * 2 + 70), IM_COL32(60, 44, 30, 255), 6);
    static const char *FN[4] = { "Gilt rule", "Rubricated", "Pen flourishes", "Blind tooling" };
    static const char *BN[4] = { "Metal plaque", "Rubric", "Seal tab", "Lettered" };
    for (int i = 0; i < 4; i++) {          /* frames: a tooltip-like panel */
        ImVec2 a(o.x + 10 + i * (cw + 4), o.y + 10), b(a.x + cw - 6, a.y + ch - 30);
        dl->AddRectFilled(a, b, IM_COL32(237, 222, 189, 255));
        orn_parchment_edges(dl, a, b, 10);
        orn_frame_style_box(dl, a, b, 16, i);
        dl->AddText(f, fs, ImVec2(a.x + 22, a.y + 24), IM_COL32(44, 26, 10, 255), "St. Adrian");
        dl->AddText(f, fs * 0.8f, ImVec2(a.x + 22, a.y + 50), IM_COL32(110, 84, 56, 255), "Heals 8 to 15 Endurance");
        dl->AddText(f, fs, ImVec2(a.x + 8, b.y + 6), IM_COL32(246, 236, 214, 255), FN[i]);
    }
    for (int i = 0; i < 4; i++) {          /* buttons: on parchment, in three colours */
        ImVec2 a(o.x + 10 + i * (cw + 4), o.y + ch + 20), b(a.x + cw - 6, a.y + ch - 30);
        dl->AddRectFilled(a, b, IM_COL32(242, 230, 200, 255));
        static const ImU32 C[3] = { IM_COL32(112, 40, 22, 235), IM_COL32(40, 92, 34, 235), IM_COL32(120, 84, 40, 235) };
        static const char *L[3] = { "Leave", "Buy", "Sell" };
        for (int k = 0; k < 3; k++) {
            ImVec2 p0(a.x + 20, a.y + 14 + k * 34), p1(a.x + 150, a.y + 40 + k * 34);
            orn_button_style(dl, p0, p1, C[k], k == 1, true, i);
            ImU32 tc = i == 3 ? (C[k] | IM_COL32(0, 0, 0, 255)) : IM_COL32(250, 240, 220, 255);
            ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, L[k]);
            dl->AddText(f, fs, ImVec2((p0.x + p1.x - ts.x) * 0.5f, (p0.y + p1.y - ts.y) * 0.5f), tc, L[k]);
        }
        dl->AddText(f, fs * 0.75f, ImVec2(a.x + 158, a.y + 54), IM_COL32(110, 84, 56, 255), "(pointed)");
        dl->AddText(f, fs, ImVec2(a.x + 8, b.y + 6), IM_COL32(246, 236, 214, 255), BN[i]);
    }
}
