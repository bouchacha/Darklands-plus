/* worldmap_draw.cpp - the world map's pictures (the places' seals and painted icons, the frame,
 * the compass rose, the boats' red ink and ships, the layer buttons' glyphs, a quest's halo), its
 * list of places, the names over the game's own travel map, and a test sheet of the pictures.
 * Split from ui/worldmap.cpp, which draws the map window with them (worldmap_draw.h). */
#include "vtrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <functional>
#include <strings.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "padglyphs.h"
#include "imgui.h"
extern "C" int ns_showing(void);
#include "gamedata.h"
#include "worldmap_draw.h"
#include "teachersui.h"
#include "quests.h"
#include "townicons.h"
#include "libraries.h"
#include "ui.h"
#include "ornament.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/worldmap.h"
#include "../mods/travel.h"
int journal_visited(int loc);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
void quest_marks(std::vector<std::pair<int, std::string>> &out);
extern ImFont *ui_font_small, *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
extern ImFont *ui_font_bold;
extern "C" int citynav_available(void);
std::string hhmm(int minutes)      /* a time on the way, as an estimate reads: "40 minutes", "22 hours", "1.4 days", "14 days" */
{
    char b[48];
    if (minutes < 60) snprintf(b, sizeof b, "%d minute%s", minutes, minutes == 1 ? "" : "s");
    else if (minutes < 23 * 60 + 30) { int h = (minutes + 30) / 60; snprintf(b, sizeof b, "%d hour%s", h, h == 1 ? "" : "s"); }
    else {
        int t = (minutes * 10 + 720) / 1440;                  /* tenths of a day */
        if (t == 10) snprintf(b, sizeof b, "1 day");
        else if (t < 100) snprintf(b, sizeof b, "%d.%d days", t / 10, t % 10);
        else snprintf(b, sizeof b, "%d days", (t + 5) / 10);
    }
    return b;
}
/* a soft golden halo behind a quest's picture, breathing slowly, with a few sparks turning about it:
   the quests stand out from the towns' and places' own marks */
/* where the pictures of the n quests that lead to one place go: one on a pin above it (pin), several
   evenly round it (mid) on a ring of fixed size, turning slowly about it - a turn in about 20
   seconds - so a place many quests lead to grows crowded, not wide. The ring stands still while
   the pointer is near it (pointer_ok: the pointer is on the map), so a picture is easy to point at. */
static int ring_held_frame = -10;
static float ring_phase(void)
{
    static float ph = 0; static double t0 = -1; static int frame = -1;
    int f = ImGui::GetFrameCount();
    if (f != frame) {
        double now = ImGui::GetTime();
        if (t0 >= 0 && ring_held_frame < f - 1) ph = fmodf(ph + (float)(now - t0) * 0.3f, 6.2831853f);
        t0 = now; frame = f;
    }
    return ph;
}
void quest_ring(std::vector<ImVec2> &at, int n, ImVec2 mid, ImVec2 pin, float is, bool pointer_ok)
{
    at.assign(n > 0 ? n : 0, mid);
    if (n == 1) { at[0] = ImVec2(pin.x, pin.y - is * 0.55f); return; }
    float R = is * fminf(1.6f, n > 4 ? 1.1f + 0.1f * (n - 4) : 1.1f), ph = ring_phase();
    for (int j = 0; j < n; j++) { float a = -1.5707963f + ph + 6.2831853f * j / n; at[j] = ImVec2(mid.x + cosf(a) * R, mid.y + sinf(a) * R); }
    ImVec2 m = ImGui::GetIO().MousePos;
    float dx = m.x - mid.x, dy = m.y - mid.y, o = R + is * 0.8f;
    if (n > 1 && pointer_ok && dx * dx + dy * dy < o * o) ring_held_frame = ImGui::GetFrameCount();
}

void quest_glow(ImDrawList *dl, ImVec2 c, float is)
{
    float t = (float)ImGui::GetTime(), p = 1.0f + 0.07f * sinf(t * 2.4f);
    for (int k = 3; k >= 1; k--) dl->AddCircleFilled(c, is * (0.42f + 0.12f * k) * p, IM_COL32(255, 214, 110, 40 + 26 * (3 - k)), 24);
    for (int k = 0; k < 4; k++) {
        float a = t * 0.9f + k * 1.5708f, tw = 0.5f + 0.5f * sinf(t * 3.1f + k * 1.7f);
        ImVec2 q(c.x + cosf(a) * is * 0.68f * p, c.y + sinf(a) * is * 0.68f * p);
        float r = is * 0.07f * (0.6f + 0.6f * tw);
        ImU32 col = IM_COL32(255, 240, 190, (int)(90 + 140 * tw));
        dl->AddLine(ImVec2(q.x - r * 1.6f, q.y), ImVec2(q.x + r * 1.6f, q.y), col, fmaxf(1.0f, r * 0.5f));
        dl->AddLine(ImVec2(q.x, q.y - r * 1.6f), ImVec2(q.x, q.y + r * 1.6f), col, fmaxf(1.0f, r * 0.5f));
    }
}


const char *kind_label(int k) { return loc_kind_label(k); }
const char *loc_kind_label(int k)        /* a location's type (its record's +0) in words */
{
    switch (k) {
    case 0: return "town"; case 1: case 2: return "castle"; case 26: return "ruined castle"; case 3: return "monastery";
    case 5: case 19: return "cave"; case 24: return "alpine cave"; case 6: return "mine"; case 8: case 10: return "village";
    case 9: return "ruined village"; case 13: return "tomb"; case 16: return "spring"; case 17: case 25: return "lake";
    case 18: return "shrine"; case 20: return "pagan altar"; case 4: return "standing stone"; case 21: return "witches' meeting place";
    case 22: case 23: return "castle"; default: return "place";
    }
}

/* a small picture for each kind of place, drawn at c, about s pixels across: icons rather than
 * dots on the world map (towns walled with a spire, villages a cottage, castles a keep,
 * monasteries and shrines a cross, caves and mines a dark mouth, springs and lakes water,
 * pagan places standing stones, ruins broken walls) */
int icon_kind(int k)
{
    switch (k) {
    case 0: return IK_TOWN; case 8: case 10: return IK_VILLAGE; case 1: case 2: case 22: case 23: return IK_CASTLE;
    case 3: return IK_MONASTERY; case 18: return IK_SHRINE; case 5: case 19: case 24: return IK_CAVE; case 6: return IK_MINE;
    case 16: case 17: case 25: return IK_WATER; case 13: return IK_TOMB; case 4: case 20: case 21: return IK_STONES;
    case 9: case 26: return IK_RUIN; default: return IK_OTHER;
    }
}
/* the line under a place's name: its kind, or for a site the map names only by its kind
   ("Lake", "Cave" ...), which town it is near */
std::string kind_line(const std::vector<Place> &P, const Place &p)
{
    const char *k = kind_label(p.kind);
    if (strcasecmp(p.name.c_str(), k)) return k;
    const Place *best = nullptr; double bd = 1e18;
    for (const Place &t : P)
        if (t.kind == 0) { double d = hypot((double)(t.x - p.x), (t.y - p.y) / 2.0); if (d < bd) { bd = d; best = &t; } }
    return best ? std::string(k) + " near " + best->name : std::string(k);
}
const char *IK_LABEL[IK_N] = { "Town", "Village", "Castle", "Monastery", "Shrine", "Cave", "Mine", "Spring or lake", "Tomb", "Pagan site", "Ruins", "Other" };
void draw_icon_painted(ImDrawList *dl, ImVec2 c, float s, int ik)
{
    const ImU32 K = IM_COL32(20, 14, 8, 235);                          /* the dark outline */
    float h = s * 0.5f, t = fmaxf(1.0f, s / 9.0f);
    auto box = [&](float x0, float y0, float x1, float y1, ImU32 col) {
        dl->AddRectFilled(ImVec2(c.x + x0 * h, c.y + y0 * h), ImVec2(c.x + x1 * h, c.y + y1 * h), col);
        dl->AddRect(ImVec2(c.x + x0 * h, c.y + y0 * h), ImVec2(c.x + x1 * h, c.y + y1 * h), K, 0, 0, t);
    };
    auto tri = [&](float x0, float y0, float x1, float y1, float x2, float y2, ImU32 col) {
        ImVec2 a(c.x + x0 * h, c.y + y0 * h), b(c.x + x1 * h, c.y + y1 * h), d(c.x + x2 * h, c.y + y2 * h);
        dl->AddTriangleFilled(a, b, d, col); dl->AddTriangle(a, b, d, K, t);
    };
    switch (ik) {
    case IK_TOWN: {                                                     /* a wall with a gate and a spire */
        ImU32 w = IM_COL32(244, 230, 196, 255);
        box(-1.0f, -0.2f, 1.0f, 0.9f, w);
        for (int i = 0; i < 3; i++) box(-1.0f + i * 0.8f, -0.5f, -0.6f + i * 0.8f, -0.2f, w);
        tri(-0.25f, -0.2f, 0.25f, -0.2f, 0.0f, -1.1f, IM_COL32(190, 70, 50, 255));
        box(-0.22f, 0.35f, 0.22f, 0.9f, IM_COL32(60, 40, 25, 255));
        break; }
    case IK_VILLAGE:                                                    /* a cottage */
        box(-0.7f, -0.1f, 0.7f, 0.8f, IM_COL32(222, 190, 130, 255));
        tri(-0.95f, -0.05f, 0.95f, -0.05f, 0.0f, -0.85f, IM_COL32(150, 95, 50, 255));
        break;
    case IK_CASTLE:                                                     /* a keep */
        box(-0.6f, -0.45f, 0.6f, 0.9f, IM_COL32(200, 196, 190, 255));
        for (int i = 0; i < 3; i++) box(-0.6f + i * 0.45f, -0.85f, -0.3f + i * 0.45f, -0.45f, IM_COL32(200, 196, 190, 255));
        box(-0.18f, 0.35f, 0.18f, 0.9f, IM_COL32(60, 40, 25, 255));
        box(-0.6f, -0.45f, 0.6f, -0.25f, IM_COL32(200, 60, 45, 255));
        break;
    case IK_MONASTERY:                                                  /* a church with a cross */
        box(-0.8f, -0.1f, 0.8f, 0.85f, IM_COL32(236, 226, 205, 255));
        tri(-0.95f, -0.05f, 0.95f, -0.05f, 0.0f, -0.6f, IM_COL32(120, 140, 200, 255));
        box(-0.09f, -1.1f, 0.09f, -0.55f, IM_COL32(250, 240, 200, 255));
        box(-0.3f, -0.95f, 0.3f, -0.8f, IM_COL32(250, 240, 200, 255));
        break;
    case IK_SHRINE:                                                     /* a wayside cross */
        box(-0.14f, -0.95f, 0.14f, 0.9f, IM_COL32(170, 200, 255, 255));
        box(-0.6f, -0.55f, 0.6f, -0.3f, IM_COL32(170, 200, 255, 255));
        break;
    case IK_CAVE: case IK_MINE: {                                       /* a hill with a dark mouth (mines: a prop) */
        tri(-1.05f, 0.85f, 1.05f, 0.85f, 0.0f, -0.85f, ik == IK_MINE ? IM_COL32(170, 140, 110, 255) : IM_COL32(150, 140, 125, 255));
        dl->AddCircleFilled(ImVec2(c.x, c.y + 0.85f * h), 0.42f * h, IM_COL32(15, 10, 6, 255), 12);
        if (ik == IK_MINE) { dl->AddLine(ImVec2(c.x - 0.45f * h, c.y + 0.45f * h), ImVec2(c.x + 0.45f * h, c.y + 0.45f * h), IM_COL32(220, 180, 90, 255), t * 1.4f); }
        break; }
    case IK_WATER:                                                      /* a drop of water */
        dl->AddCircleFilled(ImVec2(c.x, c.y + 0.25f * h), 0.62f * h, IM_COL32(90, 160, 240, 255), 14);
        tri(-0.55f, 0.0f, 0.55f, 0.0f, 0.0f, -0.95f, IM_COL32(90, 160, 240, 255));
        dl->AddCircle(ImVec2(c.x, c.y + 0.25f * h), 0.62f * h, K, 14, t);
        break;
    case IK_TOMB:                                                       /* a gravestone */
        dl->AddRectFilled(ImVec2(c.x - 0.5f * h, c.y - 0.75f * h), ImVec2(c.x + 0.5f * h, c.y + 0.85f * h), IM_COL32(190, 185, 175, 255), 0.45f * h, ImDrawFlags_RoundCornersTop);
        dl->AddRect(ImVec2(c.x - 0.5f * h, c.y - 0.75f * h), ImVec2(c.x + 0.5f * h, c.y + 0.85f * h), K, 0.45f * h, ImDrawFlags_RoundCornersTop, t);
        break;
    case IK_STONES:                                                     /* standing stones */
        box(-0.85f, -0.4f, -0.4f, 0.9f, IM_COL32(190, 150, 220, 255));
        box(0.4f, -0.4f, 0.85f, 0.9f, IM_COL32(190, 150, 220, 255));
        box(-0.95f, -0.75f, 0.95f, -0.4f, IM_COL32(190, 150, 220, 255));
        break;
    case IK_RUIN:                                                       /* broken walls */
        box(-0.9f, 0.0f, -0.35f, 0.85f, IM_COL32(150, 140, 130, 255));
        box(-0.35f, 0.4f, 0.3f, 0.85f, IM_COL32(150, 140, 130, 255));
        box(0.3f, -0.45f, 0.9f, 0.85f, IM_COL32(150, 140, 130, 255));
        break;
    default:
        dl->AddCircleFilled(c, 0.5f * h, IM_COL32(200, 160, 220, 255));
        dl->AddCircle(c, 0.5f * h, K, 0, t);
    }
}

/* the seal set: a roundel in the colour of the place's group (towns, villages, castles, the
   holy places, the wild), the painted picture small and pale within */
int kind_group_of_icon(int ik) { static const int G[IK_N] = { 0, 1, 2, 3, 3, 4, 4, 4, 3, 4, 1, 4 }; return G[ik]; }
void draw_icon_seal(ImDrawList *dl, ImVec2 c, float s, int ik)
{
    float r = s * 0.5f;
    ImU32 col = kind_colour(kind_group_of_icon(ik));
    dl->AddCircleFilled(ImVec2(c.x + r * 0.08f, c.y + r * 0.1f), r, IM_COL32(0, 0, 0, 90), 24);
    dl->AddCircleFilled(c, r, col, 24);
    dl->AddCircle(c, r * 0.86f, IM_COL32(255, 244, 214, 170), 24, fmaxf(1.0f, r * 0.07f));
    dl->AddCircle(c, r, IM_COL32(30, 18, 8, 230), 24, fmaxf(1.0f, r * 0.1f));
    draw_icon_painted(dl, c, s * 0.62f, ik);
}

void draw_icon(ImDrawList *dl, ImVec2 c, float s, int ik) { draw_icon_seal(dl, c, s, ik); }   /* (the seals: the one set) */

/* a town on a seal of muted wax (style 3, in use: draw_town_seal; 1 and 2 were the other proposals), its size the
   town's (small, medium, large: DARKLAND.CTY), and been there or not, three ways - 1 the wax cold and
   grey until the party has been there, warm red after; 2 a ribbon hanging from the seal of a town not
   yet visited (a letter still unopened); 3 the wax pressed with the party's ring once visited, smooth before */
static void wax_blob(ImDrawList *dl, ImVec2 c, float r, ImU32 wax, int seed)
{
    const int N = 40; ImVec2 pts[N], sh[N];
    for (int k = 0; k < N; k++) {
        float th = k * 6.2832f / N, rr = r * (1.0f + 0.05f * sinf(5 * th + seed) + 0.03f * sinf(9 * th + 2.0f * seed));
        pts[k] = ImVec2(c.x + cosf(th) * rr, c.y + sinf(th) * rr);
        sh[k] = ImVec2(pts[k].x + r * 0.07f, pts[k].y + r * 0.09f);
    }
    dl->AddConcavePolyFilled(sh, N, IM_COL32(0, 0, 0, 70));
    dl->AddConcavePolyFilled(pts, N, wax);
    dl->AddPolyline(pts, N, IM_COL32(60, 30, 24, 200), ImDrawFlags_Closed, fmaxf(1.0f, r * 0.05f));
}
/* a town on the world map: on its seal of wax, as large as the town (DARKLAND.CTY's size: up to 4
   small, 6 medium, more large), the party's ring pressed into the wax once it has been there; before,
   the wax washed out as the town's picture is */
float draw_town_seal(ImDrawList *dl, ImVec2 c, float s, int city_size, bool visited)
{
    static const float K[3] = { 0.86f, 1.0f, 1.18f };
    s *= K[city_size <= 4 ? 0 : city_size <= 6 ? 1 : 2];
    float r = s * 0.62f;
    wax_blob(dl, c, r, visited ? IM_COL32(150, 74, 62, 255) : IM_COL32(186, 150, 130, 255), 3);
    if (visited) {                                   /* the party's ring pressed into it */
        dl->AddCircle(c, r * 0.86f, IM_COL32(90, 40, 32, 220), 0, fmaxf(1.0f, r * 0.08f));
        dl->AddCircle(c, r * 0.86f + fmaxf(1.0f, r * 0.06f), IM_COL32(196, 120, 100, 160), 0, fmaxf(1.0f, r * 0.04f));
    }
    draw_icon_seal(dl, c, s * 0.82f, IK_TOWN);
    if (!visited) dl->AddCircleFilled(c, s * 0.37f, IM_COL32(236, 226, 204, 120), 20);
    return r;
}

int kind_group(int k)        // 0 town 1 village 2 castle 3 holy 4 other
{
    switch (k) {
    case 0: return 0; case 8: case 10: case 9: return 1; case 1: case 2: case 22: case 23: case 26: return 2;
    case 3: case 13: case 18: case 16: return 3; default: return 4;
    }
}
ImU32 kind_colour(int g)
{
    static const ImU32 C[5] = { IM_COL32(250, 236, 200, 255), IM_COL32(222, 190, 120, 255), IM_COL32(222, 190, 120, 255),   /* (castles as villages and ruins: */
                                IM_COL32(150, 190, 255, 255), IM_COL32(200, 160, 220, 255) };                               /*  red, they outshone the towns) */
    return C[g];
}

std::vector<Place> places(void)
{
    std::vector<Place> v;
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int kind = (s16)rw(s, o), x = (s16)rw(s, (u16)(o + 4)), y = (s16)rw(s, (u16)(o + 6)), menu = (s16)rw(s, (u16)(o + 0x0c));
        if (k >= 92 && (menu == 0x62 || kind == 15)) continue;          // hidden on the game's map too
        if (x < 0 || y < 0 || x >= worldmap.tiles_w || y >= worldmap.tiles_h) continue;
        Place p; p.loc = k; p.kind = k < 92 ? 0 : kind; p.x = x; p.y = y;
        p.name = loc_display_name(k);
        v.push_back(p);
    }
    return v;
}

float aspect(void) { return ui_settings.aspect ? 1.2f : 1.0f; }
void tile_px(int x, int y, float *px, float *py) { *px = x * 16.0f + ((y & 1) ? 8 : 0) + 8; *py = y * 4.0f + 6; }


float map_border_px(void) { return fmaxf(26.0f, ui_text_px() * 1.25f); }
void compass_rose(ImDrawList *dl, ImVec2 c, float r)
{
    const ImU32 ink = IM_COL32(60, 40, 24, 255), gold = IM_COL32(214, 160, 60, 255), red = IM_COL32(160, 52, 34, 255), blue = IM_COL32(52, 82, 130, 255);
    dl->AddCircleFilled(c, r * 0.98f, IM_COL32(236, 226, 200, 230));
    dl->AddCircle(c, r * 0.98f, ink, 48, fmaxf(1.0f, r * 0.04f));
    dl->AddCircle(c, r * 0.80f, ink, 48, fmaxf(1.0f, r * 0.02f));
    for (int k = 0; k < 16; k++) {                          /* the points: four long, four middle, eight short */
        float a = k * 3.14159265f / 8 - 3.14159265f / 2, len = k % 4 == 0 ? 1.0f : k % 2 == 0 ? 0.72f : 0.5f, w = k % 4 == 0 ? 0.16f : 0.11f;
        ImVec2 tip(c.x + cosf(a) * r * len, c.y + sinf(a) * r * len);
        ImVec2 l(c.x + cosf(a - 1.5708f) * r * w, c.y + sinf(a - 1.5708f) * r * w), rr(c.x + cosf(a + 1.5708f) * r * w, c.y + sinf(a + 1.5708f) * r * w);
        ImU32 f1 = k % 4 == 0 ? (k == 0 ? red : blue) : k % 2 == 0 ? gold : IM_COL32(170, 140, 90, 255);
        dl->AddTriangleFilled(c, l, tip, f1);
        dl->AddTriangleFilled(c, tip, rr, IM_COL32(250, 240, 214, 255));
        dl->AddTriangle(l, tip, rr, ink, fmaxf(1.0f, r * 0.015f));
    }
    dl->AddCircleFilled(c, r * 0.08f, gold);
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float fs = r * 0.36f; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, "N");
    dl->AddText(f, fs, ImVec2(c.x - ts.x * 0.5f, c.y - r * 1.02f - ts.y), ink, "N");
}
void draw_map_frame(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    float B = map_border_px(), chk = B * 0.3f, band = B - chk;
    const ImU32 ink = IM_COL32(48, 32, 20, 255), green = IM_COL32(58, 128, 72, 255), white = IM_COL32(246, 240, 224, 255);
    static const ImU32 FIELD[3] = { IM_COL32(214, 140, 52, 255), IM_COL32(176, 64, 44, 255), IM_COL32(198, 160, 70, 255) };
    /* the checkered rule, just outside the map */
    ImVec2 c0(a.x - chk, a.y - chk), c1(b.x + chk, b.y + chk);
    dl->AddRectFilled(c0, ImVec2(c1.x, a.y), white); dl->AddRectFilled(ImVec2(c0.x, b.y), c1, white);
    dl->AddRectFilled(ImVec2(c0.x, a.y), ImVec2(a.x, b.y), white); dl->AddRectFilled(ImVec2(b.x, a.y), ImVec2(c1.x, b.y), white);
    for (float x = c0.x, i = 0; x < c1.x; x += chk, i++) if ((int)i % 2 == 0) {
        dl->AddRectFilled(ImVec2(x, c0.y), ImVec2(fminf(x + chk, c1.x), a.y), green);
        dl->AddRectFilled(ImVec2(x, b.y), ImVec2(fminf(x + chk, c1.x), c1.y), green);
    }
    for (float y = a.y, i = 0; y < b.y; y += chk, i++) if ((int)i % 2 == 1) {
        dl->AddRectFilled(ImVec2(c0.x, y), ImVec2(a.x, fminf(y + chk, b.y)), green);
        dl->AddRectFilled(ImVec2(b.x, y), ImVec2(c1.x, fminf(y + chk, b.y)), green);
    }
    /* the lettered band */
    ImVec2 o0(c0.x - band, c0.y - band), o1(c1.x + band, c1.y + band);
    ImFont *f = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float fs = band * 0.85f;
    for (int k = 0; k < 8; k++) {
        float x0 = c0.x + (c1.x - c0.x) * k / 8, x1 = c0.x + (c1.x - c0.x) * (k + 1) / 8;
        float y0 = c0.y + (c1.y - c0.y) * k / 8, y1 = c0.y + (c1.y - c0.y) * (k + 1) / 8;
        ImU32 fc = FIELD[k % 3];
        dl->AddRectFilled(ImVec2(x0, o0.y), ImVec2(x1, c0.y), fc); dl->AddRectFilled(ImVec2(x0, c1.y), ImVec2(x1, o1.y), fc);
        dl->AddRectFilled(ImVec2(o0.x, y0), ImVec2(c0.x, y1), fc); dl->AddRectFilled(ImVec2(c1.x, y0), ImVec2(o1.x, y1), fc);
        dl->AddLine(ImVec2(x0, o0.y), ImVec2(x0, c0.y), ink); dl->AddLine(ImVec2(x0, c1.y), ImVec2(x0, o1.y), ink);
        dl->AddLine(ImVec2(o0.x, y0), ImVec2(c0.x, y0), ink); dl->AddLine(ImVec2(c1.x, y0), ImVec2(o1.x, y0), ink);
        char L[2] = { (char)('A' + k), 0 }, N[2] = { (char)('1' + k), 0 };
        ImVec2 ls = f->CalcTextSizeA(fs, FLT_MAX, 0, L), ns = f->CalcTextSizeA(fs, FLT_MAX, 0, N);
        float mx = (x0 + x1) * 0.5f, my = (y0 + y1) * 0.5f;
        dl->AddText(f, fs, ImVec2(mx - ls.x * 0.5f, o0.y + (band - ls.y) * 0.5f), ink, L);
        dl->AddText(f, fs, ImVec2(mx - ls.x * 0.5f, c1.y + (band - ls.y) * 0.5f), ink, L);
        dl->AddText(f, fs, ImVec2(o0.x + (band - ns.x) * 0.5f, my - ns.y * 0.5f), ink, N);
        dl->AddText(f, fs, ImVec2(c1.x + (band - ns.x) * 0.5f, my - ns.y * 0.5f), ink, N);
    }
    for (int s2 = 0; s2 < 4; s2++) {                         /* the corners: plain fields */
        ImVec2 p0(s2 & 1 ? c1.x : o0.x, s2 & 2 ? c1.y : o0.y);
        dl->AddRectFilled(p0, ImVec2(p0.x + band, p0.y + band), FIELD[1]);
    }
    dl->AddRect(o0, o1, ink, 0, 0, 2.0f);
    dl->AddRect(c0, c1, ink, 0, 0, 1.0f);
    dl->AddRect(a, b, ink, 0, 0, 1.0f);
    /* the title over the top, and the rose at the lower left */
    const char *t = "Map of Greater Germany";
    float tfs = band * 1.3f; ImVec2 tsz = f->CalcTextSizeA(tfs, FLT_MAX, 0, t);
    float tx = (a.x + b.x) * 0.5f - tsz.x * 0.5f, ty = o0.y - tsz.y - band * 0.3f;
    dl->AddText(f, tfs, ImVec2(tx, ty), ink, t);
    float r = fminf((b.x - a.x) * 0.06f, 70.0f);
    if (r > 14) compass_rose(dl, ImVec2(o0.x + r * 0.35f, o1.y - r * 0.35f), r);   /* (over the lower left corner, as on the printed map) */
}

/* a boat's way: a chart's red ink, as the rhumb lines of a portolan, its dots running along, and a
   little ship now and then sailing on it. al: 255 for a boat leaving today (or any, on the world map),
   less for one leaving later; lit: the one chosen; both_ways: boats sail it both ways (most do): two
   rows of dots running opposite ways, and the ships sail both ways */
void boat_way_draw(ImDrawList *dl, const std::vector<ImVec2> &pts, float lw, bool lit, int al, float t, bool both_ways)
{
    if (pts.size() < 2) return;
    int n = (int)pts.size();
    auto along = [&](float spacing, float phase, const std::function<void(ImVec2, ImVec2)> &at) {   /* points every `spacing` along the way, with the way's direction */
        float acc = -phase;
        for (int k = 1; k < n; k++) {
            ImVec2 p0 = pts[k - 1], p1 = pts[k];
            float L = sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
            if (L <= 0) continue;
            float s0 = fmodf(spacing - fmodf(acc, spacing) + spacing, spacing);
            for (; s0 < L; s0 += spacing) at(ImVec2(p0.x + (p1.x - p0.x) * s0 / L, p0.y + (p1.y - p0.y) * s0 / L), ImVec2((p1.x - p0.x) / L, (p1.y - p0.y) / L));
            acc += L;
        }
    };
    if (lit) {                                                    /* the chosen way glows */
        float pulse = 0.75f + 0.25f * sinf(t * 4.0f);
        dl->AddPolyline(pts.data(), n, IM_COL32(255, 220, 120, (int)(70 * pulse)), 0, lw * 7.0f);
        dl->AddPolyline(pts.data(), n, IM_COL32(255, 236, 160, (int)(110 * pulse)), 0, lw * 4.0f);
    }
    auto red_ink = [&]() {                                        /* the way inked in red, as on the maps of the age; dots running along it */
        dl->AddPolyline(pts.data(), n, IM_COL32(250, 236, 200, al / 2), 0, lw * 3.0f);
        dl->AddPolyline(pts.data(), n, lit ? IM_COL32(190, 30, 16, 255) : IM_COL32(150, 34, 20, al), 0, lw * 1.6f);
        {   /* the dots run along it, as the ink of a ship's track drawn on: both ways, two rows of them going opposite ways */
            ImU32 dc = lit ? IM_COL32(190, 30, 16, 255) : IM_COL32(150, 34, 20, al);
            if (!both_ways) along(lw * 16, -t * lw * 8, [&](ImVec2 p, ImVec2) { dl->AddCircleFilled(p, lw * 0.95f, dc, 8); });
            else for (int dir = 0; dir < 2; dir++)
                along(lw * 16, dir ? t * lw * 8 : -t * lw * 8, [&](ImVec2 p, ImVec2 d) {
                    float o = (dir ? -1.0f : 1.0f) * lw * 0.9f;
                    dl->AddCircleFilled(ImVec2(p.x + d.y * o, p.y - d.x * o), lw * 0.8f, dc, 8);
                });
        }
    };
    red_ink();
    for (int dir = 0; dir < (both_ways ? 2 : 1); dir++)     /* (both ways: half of them sail back) */
    along(lw * 200, dir ? lw * 100 - t * lw * 10 : t * lw * 10, [&](ImVec2 p, ImVec2 d) {   /* (a ship every so often, not a fleet) */
        if (dir) d = ImVec2(-d.x, -d.y);
        float h = lw * 2.6f; bool right = d.x >= 0;
        ImVec2 hull[4] = { ImVec2(p.x - h * 1.1f, p.y), ImVec2(p.x + h * 1.1f, p.y), ImVec2(p.x + h * 0.7f, p.y + h * 0.55f), ImVec2(p.x - h * 0.7f, p.y + h * 0.55f) };
        dl->AddConvexPolyFilled(hull, 4, IM_COL32(110, 64, 30, al));
        dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y - h * 1.5f), IM_COL32(60, 36, 16, al), fmaxf(1.0f, lw * 0.35f));
        float sx = right ? 1.0f : -1.0f;
        dl->AddTriangleFilled(ImVec2(p.x, p.y - h * 1.45f), ImVec2(p.x, p.y - h * 0.2f), ImVec2(p.x + sx * h * 0.95f, p.y - h * 0.35f), IM_COL32(250, 246, 232, al));
    });
}

/* a town's boats in the place panel: a dock, a boat and which way they go - 0 leaving (the arrow
   after the boat, away from the dock), 1 arriving (the arrow before it, towards the dock), 2 both
   ways (two arrows between). Drawn in a box at a, BOAT_WAY_W(h) wide and h high */
void boat_way_icon(ImDrawList *dl, ImVec2 a, float h, int kind)
{
    ImU32 wood = IM_COL32(110, 64, 30, 255), dark = IM_COL32(60, 36, 16, 255), sail = IM_COL32(250, 246, 232, 255);
    ImU32 water = IM_COL32(70, 110, 150, 200), ink = IM_COL32(150, 40, 25, 255);
    float lw = fmaxf(1.0f, h * 0.07f), base = a.y + h * 0.78f;
    dl->AddLine(ImVec2(a.x, base + h * 0.1f), ImVec2(a.x + BOAT_WAY_W(h), base + h * 0.1f), water, lw);     /* the water */
    dl->AddRectFilled(ImVec2(a.x, a.y + h * 0.42f), ImVec2(a.x + h * 0.34f, base), wood);                       /* the dock: its planks */
    dl->AddRectFilled(ImVec2(a.x + h * 0.04f, a.y + h * 0.3f), ImVec2(a.x + h * 0.12f, base + h * 0.16f), dark);  /* and a post */
    for (int k = 1; k < 3; k++) dl->AddLine(ImVec2(a.x, a.y + h * (0.42f + 0.12f * k)), ImVec2(a.x + h * 0.34f, a.y + h * (0.42f + 0.12f * k)), dark, fmaxf(1.0f, h * 0.03f));
    auto boat = [&](float cx, bool right) {
        float b = h * 0.24f;
        ImVec2 hull[4] = { ImVec2(cx - b * 1.5f, base - b * 0.55f), ImVec2(cx + b * 1.5f, base - b * 0.55f), ImVec2(cx + b * 1.0f, base), ImVec2(cx - b * 1.0f, base) };
        dl->AddConvexPolyFilled(hull, 4, wood);
        dl->AddLine(ImVec2(cx, base - b * 0.55f), ImVec2(cx, a.y + h * 0.08f), dark, lw);
        float sx = right ? 1.0f : -1.0f;
        dl->AddTriangleFilled(ImVec2(cx, a.y + h * 0.1f), ImVec2(cx, base - b * 0.7f), ImVec2(cx + sx * b * 1.3f, base - b * 0.8f), sail);
        dl->AddTriangle(ImVec2(cx, a.y + h * 0.1f), ImVec2(cx, base - b * 0.7f), ImVec2(cx + sx * b * 1.3f, base - b * 0.8f), dark, fmaxf(1.0f, h * 0.03f));
    };
    auto arrow = [&](float x0, float x1, float y) {                                   /* from x0 to x1, its head at x1 */
        float d = x1 > x0 ? 1.0f : -1.0f, hw = h * 0.13f;
        dl->AddLine(ImVec2(x0, y), ImVec2(x1 - d * hw, y), ink, lw * 1.3f);
        dl->AddTriangleFilled(ImVec2(x1, y), ImVec2(x1 - d * hw * 1.6f, y - hw), ImVec2(x1 - d * hw * 1.6f, y + hw), ink);
    };
    float d0 = a.x + h * 0.48f;                                                       /* past the dock */
    if (kind == 0) { boat(d0 + h * 0.4f, true); arrow(d0 + h * 0.85f, d0 + h * 1.75f, a.y + h * 0.5f); }
    else if (kind == 1) { arrow(d0 + h * 0.85f, d0 + h * 0.02f, a.y + h * 0.5f); boat(d0 + h * 1.3f, false); }
    else { arrow(d0 + h * 0.02f, d0 + h * 0.8f, a.y + h * 0.36f); arrow(d0 + h * 0.8f, d0 + h * 0.02f, a.y + h * 0.64f); boat(d0 + h * 1.3f, true); }
}

/* the four things the map can show or not (names, quest marks, boats, the places' icons): buttons
   floating on the map, under the top right panel */

void layer_glyph(ImDrawList *dl, ImVec2 c, float s, int which, ImU32 ink)
{
    switch (which) {
    case 0: { ImFont *f = ui_font_bold ? ui_font_bold : ImGui::GetFont(); float fs = s * 0.8f; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, "Aa");
              dl->AddText(f, fs, ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.52f), ink, "Aa"); break; }
    case 1: ui_draw_quest_icon(dl, c, s * 0.95f, QC_ROBBER); break;
    case 2: { float h = s * 0.3f;
              ImVec2 hull[4] = { ImVec2(c.x - h * 1.3f, c.y + h * 0.3f), ImVec2(c.x + h * 1.3f, c.y + h * 0.3f), ImVec2(c.x + h * 0.85f, c.y + h * 1.0f), ImVec2(c.x - h * 0.85f, c.y + h * 1.0f) };
              dl->AddConvexPolyFilled(hull, 4, IM_COL32(110, 64, 30, 255));
              dl->AddLine(ImVec2(c.x, c.y + h * 0.3f), ImVec2(c.x, c.y - h * 1.5f), IM_COL32(60, 36, 16, 255), fmaxf(1.0f, s * 0.05f));
              dl->AddTriangleFilled(ImVec2(c.x, c.y - h * 1.45f), ImVec2(c.x, c.y + h * 0.05f), ImVec2(c.x + h * 1.05f, c.y - h * 0.1f), IM_COL32(250, 246, 232, 255));
              dl->AddTriangle(ImVec2(c.x, c.y - h * 1.45f), ImVec2(c.x, c.y + h * 0.05f), ImVec2(c.x + h * 1.05f, c.y - h * 0.1f), IM_COL32(60, 36, 16, 200), 1.0f);
              break; }
    case 3: draw_icon(dl, c, s * 0.9f, 0); break;
    case 4: {   /* days away: an hourglass */
              float w = s * 0.34f, h = s * 0.46f; ImU32 sand = IM_COL32(214, 168, 70, 255);
              dl->AddLine(ImVec2(c.x - w, c.y - h), ImVec2(c.x + w, c.y - h), ink, fmaxf(1.5f, s * 0.08f)); dl->AddLine(ImVec2(c.x - w, c.y + h), ImVec2(c.x + w, c.y + h), ink, fmaxf(1.5f, s * 0.08f));
              dl->AddTriangleFilled(ImVec2(c.x - w * 0.8f, c.y - h * 0.85f), ImVec2(c.x + w * 0.8f, c.y - h * 0.85f), ImVec2(c.x, c.y), IM_COL32(236, 228, 206, 255));
              dl->AddTriangleFilled(ImVec2(c.x - w * 0.8f, c.y + h * 0.85f), ImVec2(c.x, c.y), ImVec2(c.x + w * 0.8f, c.y + h * 0.85f), sand);
              dl->AddTriangle(ImVec2(c.x - w * 0.8f, c.y - h * 0.85f), ImVec2(c.x + w * 0.8f, c.y - h * 0.85f), ImVec2(c.x, c.y), ink, 1.0f);
              dl->AddTriangle(ImVec2(c.x - w * 0.8f, c.y + h * 0.85f), ImVec2(c.x, c.y), ImVec2(c.x + w * 0.8f, c.y + h * 0.85f), ink, 1.0f);
              break; }
    }
}
void layer_struck(ImDrawList *dl, ImVec2 c, float s)
{
    dl->AddCircleFilled(c, s * 0.55f, IM_COL32(236, 226, 204, 170));
    dl->AddLine(ImVec2(c.x - s * 0.45f, c.y + s * 0.45f), ImVec2(c.x + s * 0.45f, c.y - s * 0.45f), IM_COL32(150, 50, 30, 220), 2);
}


extern "C" void travelmap_labels(void)
{
    bool names = config_get_int("map.names", 1) != 0, signs = config_get_int("map.quest_marks", 1) != 0;   /* (names; the quests' and towns' signs) */
    if ((!names && !signs) || config_get_int("original", 0) || game_mode() != GM_MAP || !travel_map_live() || !ui_font_overlay) return;
    if (ns_showing()) return;                                   /* (a native box over the map: a map message) */
    int vx = (s16)g_rw(GV_map_view_col), vr = (s16)g_rw(GV_map_buf_row) + (s16)g_rw(GV_map_view_row);
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImFont *f = ui_font_overlay;
    float base = ui_text_px();
    static std::vector<TownMark> tmarks; static int tf = -1000;
    static std::vector<std::pair<int, std::string>> qmarks; static std::vector<int> qcats;
    if (ImGui::GetFrameCount() - tf > 30) { town_marks(tmarks); quest_marks_cat(qmarks, qcats); tf = ImGui::GetFrameCount(); }
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int kind = (s16)rw(s, o), x = (s16)rw(s, (u16)(o + 4)), y = (s16)rw(s, (u16)(o + 6)), menu = (s16)rw(s, (u16)(o + 0x0c));
        if (k >= 92 && (menu == 0x62 || kind == 15)) continue;             /* not shown on the map */
        float sx = (float)(16 * (x - vx) + ((y & 1) ? 7 : -1) + 65), sy = (float)(4 * (y - vr) - 6 + 21);
        if (sx < 66 || sx > 314 || sy < 18 || sy > 190) continue;
        std::string name = loc_display_name(k);
        if (name.empty()) continue;
        bool town = k < 92;
        float size = base * (town ? 1.1f : 0.88f);
        ImVec2 at; plat_game_to_win(sx, sy + 5, &at.x, &at.y);
        ImVec2 ts = f->CalcTextSizeA(size, FLT_MAX, 0, name.c_str());
        ImVec2 tp(at.x - ts.x * 0.5f, at.y);
        {   /* the pointer on the place or its name: what it is, the journey, and that Ctrl+click goes there */
            ImVec2 m = ImGui::GetIO().MousePos, i0, i1;
            plat_game_to_win(sx - 7, sy - 5, &i0.x, &i0.y); plat_game_to_win(sx + 7, sy + 4, &i1.x, &i1.y);
            bool on_icon = m.x >= i0.x && m.x < i1.x && m.y >= i0.y && m.y < i1.y;
            bool on_name = names && m.x >= tp.x && m.x < tp.x + ts.x && m.y >= tp.y && m.y < tp.y + ts.y;
            if ((on_icon || on_name) && !ImGui::GetIO().WantCaptureMouse) {
                static int tip_loc = -1, tip_min = -1, tip_frame = -1000;
                if (tip_loc != k || ImGui::GetFrameCount() - tip_frame > 120) { tip_loc = k; tip_min = travel_minutes_loc(k, nullptr, nullptr); }   /* (a route search: once, while the pointer stays) */
                tip_frame = ImGui::GetFrameCount();
                if (on_icon) ui_tip_beside(i0, i1); else ui_tip_beside(tp, ImVec2(tp.x + ts.x, tp.y + ts.y));
                ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextDisabled("%s", loc_kind_label(kind));
                if (tip_min > 0) ImGui::Text("About %s of travel", hhmm(tip_min).c_str());
                if (config_get_int("travel.auto", 1)) ImGui::TextDisabled("Ctrl+click: travel there along the roads");
                ImGui::PopTextWrapPos(); ImGui::EndTooltip();
            }
        }
        if (names) {
            float ow = fmaxf(1.0f, roundf(size / 16.0f));
            for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++)
                if (dx || dy) dl->AddText(f, size, ImVec2(tp.x + dx * ow, tp.y + dy * ow), IM_COL32(0, 0, 0, 200), name.c_str());
            dl->AddText(f, size, tp, town ? IM_COL32(255, 244, 214, 255) : IM_COL32(225, 215, 195, 240), name.c_str());
        } else { tp.x = at.x - size * 0.6f; ts.x = 0; }             /* (the signs where the name would begin) */
        if (!signs) continue;
        int n = 0;                                            /* the town bars the party, or its watch wants it */
        for (const TownMark &t : tmarks)
            if (t.loc == k) { float is = size * 1.15f; ui_draw_quest_icon(dl, ImVec2(tp.x + ts.x + is * (0.65f + 1.05f * n), tp.y + ts.y * 0.5f), is, t.cat); n++; }
        {   /* a quest that leads here: its picture above the place; several: on a ring round it, turning
               slowly (quest_ring) */
            std::vector<size_t> qs;
            for (size_t i = 0; i < qmarks.size(); i++) if (qmarks[i].first == k) qs.push_back(i);
            if (!qs.empty()) {
                float is = base * 1.3f;
                ImVec2 top; plat_game_to_win(sx, sy - 6, &top.x, &top.y);
                ImVec2 mid; plat_game_to_win(sx, sy, &mid.x, &mid.y);
                int nq = (int)qs.size();
                std::vector<ImVec2> at;
                quest_ring(at, nq, mid, top, is, !ImGui::GetIO().WantCaptureMouse);
                ImVec2 m = ImGui::GetIO().MousePos;              /* (the pointer on one: the topmost, the last drawn) */
                int hov = -1;
                if (!ImGui::GetIO().WantCaptureMouse)
                    for (int j = nq - 1; j >= 0 && hov < 0; j--) if (fabsf(m.x - at[j].x) < is * 0.5f && fabsf(m.y - at[j].y) < is * 0.5f) hov = j;
                for (int j = 0; j < nq; j++) quest_glow(dl, at[j], is);   /* (the halos under all the pictures) */
                for (int j = 0; j < nq; j++) if (j != hov) ui_draw_quest_icon(dl, at[j], is, qs[j] < qcats.size() ? qcats[qs[j]] : 0);
                if (hov >= 0) {                                  /* (the one pointed at over the others, and its task) */
                    ui_draw_quest_icon(dl, at[hov], is * 1.12f, qs[hov] < qcats.size() ? qcats[qs[hov]] : 0);
                    ui_tip_beside(ImVec2(at[hov].x - is * 0.5f, at[hov].y - is * 0.5f), ImVec2(at[hov].x + is * 0.5f, at[hov].y + is * 0.5f));
                    ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextUnformatted(qmarks[qs[hov]].second.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
                }
            }
        }
    }
}

float ui_face_k(ImFont *f);                                  /* overlay.cpp */
/* assist.docks_map: beside the docks' list of boats ($Docks00), a small map of the town and the
   towns its boats go to, the way each boat takes along the rivers and coasts. The boats leaving
   today are bright (a click chooses that boat on the card), the others faded; the option under
   the pointer on the card lights its way. Returns the option clicked on the map, or -1. */

static int icon_sheet;
extern "C" void worldmap_icon_sheet(void) { icon_sheet = !icon_sheet; }
extern "C" void worldmap_icon_sheet_draw(void)
{
    if (!icon_sheet) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 o(20, 20); float sz = 52, gx = 70, gy = 96;
    dl->AddRectFilled(o, ImVec2(o.x + gx * IK_N + 20, o.y + gy + 20), IM_COL32(220, 206, 170, 255), 6);
    for (int k = 0; k < IK_N; k++) {
        ImVec2 c(o.x + 20 + gx * k + gx * 0.5f, o.y + 20 + sz * 0.6f);
        draw_icon(dl, c, sz, k);
        draw_icon(dl, ImVec2(c.x + 18, c.y + sz * 0.75f), sz * 0.36f, k);
    }
}

