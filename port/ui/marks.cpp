/* marks.cpp - small signs drawn with ImGui shapes (marks.h) */
#include <math.h>
#include "marks.h"
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);

static ImVec2 at(ImVec2 c, float x, float y) { return ImVec2(c.x + x, c.y + y); }

void ui_mark_star(ImDrawList *dl, ImVec2 c, float r, ImU32 fill, ImU32 line, float th)
{
    ImVec2 p[10];
    for (int i = 0; i < 10; i++) { float a = -1.5708f + i * 0.6283f, rr = i & 1 ? r * 0.42f : r; p[i] = at(c, cosf(a) * rr, sinf(a) * rr); }
    if (fill) for (int i = 0; i < 10; i++) dl->AddTriangleFilled(c, p[i], p[(i + 1) % 10], fill);
    if (line) dl->AddPolyline(p, 10, line, ImDrawFlags_Closed, th);
}
void ui_mark_aside(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float th)
{
    dl->AddLine(at(c, 0, -r), at(c, 0, r * 0.35f), col, th);
    dl->AddTriangleFilled(at(c, -r * 0.55f, 0), at(c, r * 0.55f, 0), at(c, 0, r * 0.6f), col);
    dl->AddLine(at(c, -r * 0.8f, r * 0.95f), at(c, r * 0.8f, r * 0.95f), col, th);
}
void ui_mark_pentacle(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float th)
{
    dl->AddCircle(c, r, col, 20, th);
    ImVec2 p[5];
    for (int i = 0; i < 5; i++) { float a = -1.5708f + i * 1.2566f; p[i] = at(c, cosf(a) * r * 0.86f, sinf(a) * r * 0.86f); }
    for (int i = 0; i < 5; i++) dl->AddLine(p[i], p[(i + 2) % 5], col, th);
}
void ui_mark_falling_bars(ImDrawList *dl, ImVec2 c, float r, ImU32 col)
{
    for (int i = 0; i < 3; i++) {
        float bx = -r + i * r * 0.72f, h = r * (1.9f - i * 0.55f);
        dl->AddRectFilled(at(c, bx, r - h), at(c, bx + r * 0.52f, r), col);
    }
}
void ui_mark_blast(ImDrawList *dl, ImVec2 c, float r)
{
    ImVec2 p[16];
    for (int i = 0; i < 16; i++) { float a = i * 0.3927f, rr = i & 1 ? r * 0.5f : r; p[i] = at(c, cosf(a) * rr, sinf(a) * rr); }
    for (int i = 0; i < 16; i++) dl->AddTriangleFilled(c, p[i], p[(i + 1) % 16], IM_COL32(214, 70, 34, 255));
    for (int i = 0; i < 16; i++) { float a = i * 0.3927f, rr = (i & 1 ? 0.28f : 0.55f) * r; p[i] = at(c, cosf(a) * rr, sinf(a) * rr); }
    for (int i = 0; i < 16; i++) dl->AddTriangleFilled(c, p[i], p[(i + 1) % 16], IM_COL32(252, 210, 90, 255));
}
void ui_mark_stink(ImDrawList *dl, ImVec2 c, float r, float th)
{
    const ImU32 k = IM_COL32(150, 156, 70, 255);
    dl->AddCircleFilled(at(c, -r * 0.45f, r * 0.35f), r * 0.42f, k, 12);
    dl->AddCircleFilled(at(c, r * 0.45f, r * 0.35f), r * 0.42f, k, 12);
    dl->AddCircleFilled(at(c, 0, r * 0.05f), r * 0.5f, k, 12);
    for (int i = -1; i <= 1; i += 2)
        dl->AddBezierCubic(at(c, i * r * 0.3f, -r * 0.35f), at(c, i * r * 0.3f + r * 0.25f, -r * 0.6f), at(c, i * r * 0.3f - r * 0.25f, -r * 0.8f),
                           at(c, i * r * 0.3f, -r * 1.05f), IM_COL32(110, 120, 50, 255), th);
}
void ui_mark_lost(ImDrawList *dl, ImVec2 c, float r, int item, float th)
{
    ui_item_icon(dl, c, r * 2, item);
    dl->AddCircleFilled(c, r * 0.95f, IM_COL32(40, 24, 12, 110), 16);
    dl->AddLine(at(c, -r * 0.8f, -r * 0.8f), at(c, r * 0.8f, r * 0.8f), IM_COL32(200, 60, 40, 255), th * 1.6f);
}
