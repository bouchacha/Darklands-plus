// pen.h - a small drawing helper for the pictures drawn with ImGui's shapes (alchemyicons.cpp,
// questicons.cpp): coordinates in a 32-unit box around a centre (-16 .. 16), `u` pixels a unit.
#pragma once
#include "imgui.h"
#include <initializer_list>

namespace pen {
inline ImU32 C(float r, float g, float b, float a = 1) { return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a)); }

struct Pen {
    ImDrawList *dl; ImVec2 c; float u; float mw = 0;      /* mw: the thinnest a line may be, in pixels */
    float W(float w) const { float x = w * u; return x < mw ? mw : x; }
    ImVec2 P(float x, float y) const { return ImVec2(c.x + x * u, c.y + y * u); }
    void line(float x0, float y0, float x1, float y1, ImU32 k, float w) const { dl->AddLine(P(x0, y0), P(x1, y1), k, W(w)); }
    void disc(float x, float y, float r, ImU32 k) const { dl->AddCircleFilled(P(x, y), r * u, k, 0); }
    void ring(float x, float y, float r, ImU32 k, float w) const { dl->AddCircle(P(x, y), r * u, k, 0, W(w)); }
    void arc(float x, float y, float r, float a0, float a1, ImU32 k, float w) const { dl->PathArcTo(P(x, y), r * u, a0, a1, 18); dl->PathStroke(k, 0, W(w)); }
    void tri(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k) const { dl->AddTriangleFilled(P(x0, y0), P(x1, y1), P(x2, y2), k); }
    void tri_line(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k, float w) const { dl->AddTriangle(P(x0, y0), P(x1, y1), P(x2, y2), k, W(w)); }
    void quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, ImU32 k) const { dl->AddQuadFilled(P(x0, y0), P(x1, y1), P(x2, y2), P(x3, y3), k); }
    void bez(float x0, float y0, float x1, float y1, float x2, float y2, ImU32 k, float w) const { dl->AddBezierQuadratic(P(x0, y0), P(x1, y1), P(x2, y2), k, W(w)); }
    void rect(float x0, float y0, float x1, float y1, ImU32 k, float r = 0) const { dl->AddRectFilled(P(x0, y0), P(x1, y1), k, r * u); }
    void rect_line(float x0, float y0, float x1, float y1, ImU32 k, float w, float r = 0) const { dl->AddRect(P(x0, y0), P(x1, y1), k, r * u, 0, W(w)); }
    void ellipse(float x, float y, float rx, float ry, ImU32 k, float rot = 0) const { dl->AddEllipseFilled(P(x, y), ImVec2(rx * u, ry * u), k, rot); }
    /* a filled polygon (any shape: concave too) and its outline, from x, y pairs */
    void poly(std::initializer_list<float> xy, ImU32 k) const
    {
        for (const float *p = xy.begin(); p + 1 < xy.end(); p += 2) dl->PathLineTo(P(p[0], p[1]));
        dl->PathFillConcave(k);
    }
    void poly_line(std::initializer_list<float> xy, ImU32 k, float w, bool closed = true) const
    {
        for (const float *p = xy.begin(); p + 1 < xy.end(); p += 2) dl->PathLineTo(P(p[0], p[1]));
        dl->PathStroke(k, closed ? ImDrawFlags_Closed : 0, W(w));
    }
};
} // namespace pen
