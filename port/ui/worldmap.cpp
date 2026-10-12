// worldmap.cpp - the world map: the whole of the Holy Roman Empire as the travel screen draws
// it (mods/worldmap.c), to pan and zoom, with towns and places named, the party, the route of
// auto-travel and the places the quest log points to. Click a place for its details, double-
// click (or Ctrl+click anywhere) to travel there. M on the travel map opens it; M or Esc closes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "imgui.h"
#include "gamedata.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/worldmap.h"
#include "../mods/travel.h"
int journal_visited(int loc);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
void quest_marks(std::vector<std::pair<int, std::string>> &out);
extern ImFont *ui_font_small, *ui_font_text;

static SDL_Renderer *wm_ren;
static SDL_Texture *tex[WM_LEVELS];
static int tex_gen = -1;
/* full scale, for zooming in (each travel-screen pixel its own): made in tiles when first
 * seen, nearest-pixel when magnified so the art stays sharp */
#define FT 1024
static SDL_Texture *ftex[64]; static int ftex_gen = -1;
static SDL_Texture *full_tile(int i, int j)
{
    if (ftex_gen != worldmap.generation) { for (auto &t : ftex) { if (t) SDL_DestroyTexture(t); t = 0; } ftex_gen = worldmap.generation; }
    int nx = (worldmap.full_w + FT - 1) / FT, k = j * nx + i;
    if (k < 0 || k >= 64 || !worldmap.idx) return 0;
    if (ftex[k]) return ftex[k];
    int x0 = i * FT, y0 = j * FT, w = std::min(FT, worldmap.full_w - x0), h = std::min(FT, worldmap.full_h - y0);
    if (w <= 0 || h <= 0) return 0;
    std::vector<unsigned> px((size_t)w * h);
    for (int y = 0; y < h; y++) {
        const unsigned char *s = worldmap.idx + (size_t)(y0 + y) * worldmap.full_w + x0;
        for (int x = 0; x < w; x++) px[(size_t)y * w + x] = worldmap.pal[s[x]];
    }
    ftex[k] = SDL_CreateTexture(wm_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (ftex[k]) SDL_UpdateTexture(ftex[k], 0, px.data(), w * 4);
    return ftex[k];
}
static float zoom = 0.5f, cx = -1, cy = -1;      // display pixels per travel-screen pixel; centre
static int selected = -1;
static bool show_villages = true, show_castles = true, show_holy = true, show_other = true, show_names = true, show_quests = true;

void worldmap_set_renderer(SDL_Renderer *r) { wm_ren = r; }

struct Place { int loc, kind, x, y; std::string name; };

static const char *kind_label(int k)
{
    switch (k) {
    case 0: return "town"; case 1: case 2: return "castle"; case 26: return "ruined castle"; case 3: return "monastery";
    case 5: case 19: case 24: return "cave"; case 6: return "mine"; case 8: case 10: return "village";
    case 9: return "ruined village"; case 13: return "tomb"; case 16: return "spring"; case 17: return "lake";
    case 18: return "shrine"; case 20: case 4: return "pagan altar"; default: return "place";
    }
}
static int kind_group(int k)        // 0 town 1 village 2 castle 3 holy 4 other
{
    switch (k) {
    case 0: return 0; case 8: case 10: case 9: return 1; case 1: case 2: case 22: case 23: case 26: return 2;
    case 3: case 13: case 18: case 16: return 3; default: return 4;
    }
}
static ImU32 kind_colour(int g)
{
    static const ImU32 C[5] = { IM_COL32(250, 236, 200, 255), IM_COL32(222, 190, 120, 255), IM_COL32(230, 90, 70, 255),
                                IM_COL32(150, 190, 255, 255), IM_COL32(200, 160, 220, 255) };
    return C[g];
}

static std::vector<Place> places(void)
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
        if (k < (int)gamedata.cities.size()) p.name = gamedata.cities[k].name;
        else { char b[24]; g_str(s, (u16)(o + 0x26), b, sizeof b); p.name = game_text(b, sizeof b); }
        v.push_back(p);
    }
    return v;
}

static float aspect(void) { return ui_settings.aspect ? 1.2f : 1.0f; }
static void tile_px(int x, int y, float *px, float *py) { *px = x * 16.0f + ((y & 1) ? 8 : 0) + 8; *py = y * 4.0f + 6; }

static void upload(void)
{
    if (tex_gen == worldmap.generation) return;
    for (int l = 0; l < WM_LEVELS; l++) {
        if (tex[l]) SDL_DestroyTexture(tex[l]);
        tex[l] = SDL_CreateTexture(wm_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, worldmap.w[l], worldmap.h[l]);
        if (tex[l]) {
            SDL_UpdateTexture(tex[l], 0, worldmap.px[l], worldmap.w[l] * 4);
            SDL_SetTextureScaleMode(tex[l], SDL_ScaleModeLinear);
        }
    }
    tex_gen = worldmap.generation;
}

static void party_tile(int *x, int *y)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    u32 r = cur >= 0 ? g_location(cur) : 0;
    if (r) { *x = (s16)rw((u16)(r >> 16), (u16)(r + 4)); *y = (s16)rw((u16)(r >> 16), (u16)(r + 6)); return; }
    travel_party_tile(x, y);
}

/* opening: draw the map again (seasons, castles burnt down) and look at the party */
/* opened from a city or encounter (M on a card): the route waits for the travel map */
extern "C" void ui_notice(const char *msg);
static void route_note(void) { if (game_mode() != GM_MAP) ui_notice("Route planned: the party sets off once it is on the travel map."); }

extern "C" int worldmap_open(void)
{
    if (game_mode() == GM_OTHER || game_mode() == GM_BATTLE) return 0;
    if (!worldmap_build()) return 0;
    upload();
    int px, py; party_tile(&px, &py);
    if (cx < 0 || px >= 0) tile_px(px, py, &cx, &cy);
    if (getenv("DL_WM_ZOOM")) zoom = (float)atof(getenv("DL_WM_ZOOM"));   /* test */
    return 1;
}

static std::string hhmm(int minutes)
{
    char b[48];
    if (minutes < 60) snprintf(b, sizeof b, "%d minutes", minutes);
    else if (minutes < 24 * 60) snprintf(b, sizeof b, "%d h %02d", minutes / 60, minutes % 60);
    else snprintf(b, sizeof b, "%.1f days", minutes / 1440.0);
    return b;
}

extern "C" void ui_toggle_map(void);

extern "C" void worldmap_draw(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("World map", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();
    upload();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos(), sz = io.DisplaySize;
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + sz.y), IM_COL32(18, 14, 10, 255));
    float ay = aspect();
    float fitz = fminf(sz.x / worldmap.full_w, sz.y / (worldmap.full_h * ay));
    if (zoom < fitz * 0.9f) zoom = fitz * 0.9f;
    if (zoom > 10) zoom = 10;
    ImVec2 mid(o.x + sz.x * 0.5f, o.y + sz.y * 0.5f);
    auto to_scr = [&](float x, float y) { return ImVec2(mid.x + (x - cx) * zoom, mid.y + (y - cy) * zoom * ay); };
    auto to_map = [&](ImVec2 p, float *x, float *y) { *x = cx + (p.x - mid.x) / zoom; *y = cy + (p.y - mid.y) / (zoom * ay); };

    // input on the map area
    ImGui::SetCursorScreenPos(o);
    ImGui::InvisibleButton("##map", sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool hovered = ImGui::IsItemHovered();
    if (hovered && io.MouseWheel != 0) {
        float mx, my; to_map(io.MousePos, &mx, &my);
        zoom *= powf(1.25f, io.MouseWheel);
        if (zoom < fitz * 0.9f) zoom = fitz * 0.9f;
        if (zoom > 10) zoom = 10;
        cx = mx - (io.MousePos.x - mid.x) / zoom; cy = my - (io.MousePos.y - mid.y) / (zoom * ay);
    }
    static bool dragged;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 3)) {
        cx -= io.MouseDelta.x / zoom; cy -= io.MouseDelta.y / (zoom * ay); dragged = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) zoom *= 1.25f;
    if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) zoom /= 1.25f;
    float step = 60 / zoom;
    if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) cx -= step * io.DeltaTime * 8;
    if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) cx += step * io.DeltaTime * 8;
    if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) cy -= step * io.DeltaTime * 8;
    if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) cy += step * io.DeltaTime * 8;
    if (cx < 0) cx = 0;
    if (cx > worldmap.full_w) cx = (float)worldmap.full_w;
    if (cy < 0) cy = 0;
    if (cy > worldmap.full_h) cy = (float)worldmap.full_h;

    // the picture: the level whose texels are closest to the screen's pixels
    int l = 0;
    while (l < WM_LEVELS - 1 && zoom * (2 << l) < 0.75f) l++;
    if (zoom > 0.75f && worldmap.idx) {                 /* closer than the half-scale picture: full scale, tile by tile */
        float vx0, vy0, vx1, vy1; to_map(o, &vx0, &vy0); to_map(ImVec2(o.x + sz.x, o.y + sz.y), &vx1, &vy1);
        int nx = (worldmap.full_w + FT - 1) / FT, ny = (worldmap.full_h + FT - 1) / FT;
        for (int j = 0; j < ny; j++)
            for (int i = 0; i < nx; i++) {
                float tx0 = (float)(i * FT), ty0 = (float)(j * FT), tx1 = (float)std::min((i + 1) * FT, worldmap.full_w), ty1 = (float)std::min((j + 1) * FT, worldmap.full_h);
                if (tx1 < vx0 || tx0 > vx1 || ty1 < vy0 || ty0 > vy1) continue;
                SDL_Texture *t = full_tile(i, j);
                if (!t) continue;
                SDL_SetTextureScaleMode(t, zoom >= 1.5f ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
                dl->AddImage((ImTextureID)t, to_scr(tx0, ty0), to_scr(tx1, ty1));
            }
    } else if (tex[l]) dl->AddImage((ImTextureID)tex[l], to_scr(0, 0), to_scr((float)worldmap.full_w, (float)worldmap.full_h));

    std::vector<Place> P = places();
    static std::vector<std::pair<int, std::string>> marks;
    static int marks_frame = -1000;
    if (ImGui::GetFrameCount() - marks_frame > 30) { quest_marks(marks); marks_frame = ImGui::GetFrameCount(); }

    // auto-travel route
    if (travel.active && travel.n > 1) {
        std::vector<ImVec2> pts;
        for (int i = travel.i > 0 ? travel.i - 1 : 0; i < travel.n; i++) { float x, y; tile_px(travel.rx[i], travel.ry[i], &x, &y); pts.push_back(to_scr(x, y)); }
        dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(0, 0, 0, 160), 0, 5);
        dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(255, 220, 90, 255), 0, 2.5f);
    }

    // places
    int hover = -1; float hd = 1e9f;
    /* names with the screen (ui_text_px: display.text_size), a little larger as one zooms in */
    ImFont *fs = ui_font_text;
    float ks = ui_text_px() / 22.0f;
    float fsize = ui_text_px() * 0.68f * fminf(fmaxf(powf(zoom / fitz, 0.3f), 0.9f), 1.6f);
    for (const Place &p : P) {
        int g = kind_group(p.kind);
        if ((g == 1 && !show_villages) || (g == 2 && !show_castles) || (g == 3 && !show_holy) || (g == 4 && !show_other)) continue;
        float x, y; tile_px(p.x, p.y, &x, &y);
        ImVec2 s = to_scr(x, y);
        if (s.x < o.x - 50 || s.y < o.y - 20 || s.x > o.x + sz.x + 50 || s.y > o.y + sz.y + 20) continue;
        float r = (g == 0 ? 4.5f : 3.0f) * fmaxf(ks, 1.0f);
        if (g != 0 && zoom < 0.22f) continue;
        dl->AddCircleFilled(s, r + 1.5f, IM_COL32(0, 0, 0, 200));
        dl->AddCircleFilled(s, r, kind_colour(g));
        float d = (io.MousePos.x - s.x) * (io.MousePos.x - s.x) + (io.MousePos.y - s.y) * (io.MousePos.y - s.y);
        float hr = 14 * fmaxf(ks, 1.0f);
        if (hovered && d < hr * hr && d < hd) { hd = d; hover = p.loc; }
        int big = p.loc < (int)gamedata.cities.size() ? gamedata.cities[p.loc].size : 0;
        bool label = show_names && (g == 0 ? (zoom >= 0.3f || (zoom >= 0.14f && big >= 6)) : zoom >= 0.9f);
        if (label || p.loc == selected) {
            ImVec2 ts = fs->CalcTextSizeA(fsize, FLT_MAX, 0, p.name.c_str());
            ImVec2 tp(s.x - ts.x * 0.5f, s.y + r + 2);
            float ow = fmaxf(1.0f, roundf(fsize / 18.0f));                       /* the dark outline, thicker with the letters */
            for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++)
                if (dx || dy) dl->AddText(fs, fsize, ImVec2(tp.x + dx * ow, tp.y + dy * ow), IM_COL32(0, 0, 0, 220), p.name.c_str());
            dl->AddText(fs, fsize, tp, g == 0 ? IM_COL32(255, 244, 214, 255) : IM_COL32(235, 225, 205, 255), p.name.c_str());
        }
    }
    // quest marks
    if (show_quests)
        for (auto &m : marks) {
            u32 r = g_location(m.first);
            if (!r) continue;
            float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y);
            float k = fmaxf(ks, 1.0f);
            ImVec2 s = to_scr(x, y); s.y -= 14 * k;
            ImVec2 pts[3] = { ImVec2(s.x - 7 * k, s.y - 8 * k), ImVec2(s.x + 7 * k, s.y - 8 * k), ImVec2(s.x, s.y + 4 * k) };
            dl->AddTriangleFilled(pts[0], pts[1], pts[2], IM_COL32(0, 0, 0, 220));
            ImVec2 q[3] = { ImVec2(s.x - 5 * k, s.y - 7 * k), ImVec2(s.x + 5 * k, s.y - 7 * k), ImVec2(s.x, s.y + 2 * k) };
            dl->AddTriangleFilled(q[0], q[1], q[2], IM_COL32(255, 210, 60, 255));
        }
    // the party
    {
        int px, py; party_tile(&px, &py);
        if (px >= 0 && py >= 0) {
            float x, y; tile_px(px, py, &x, &y);
            ImVec2 s = to_scr(x, y);
            float k = fmaxf(ks, 1.0f), t = (float)ImGui::GetTime(), pr = (9 + 3 * sinf(t * 4)) * k;
            dl->AddCircle(s, pr + k, IM_COL32(0, 0, 0, 220), 0, 4 * k);
            dl->AddCircle(s, pr, IM_COL32(255, 255, 255, 255), 0, 2.5f * k);
            dl->AddCircleFilled(s, 3.5f * k, IM_COL32(255, 60, 40, 255));
        }
    }

    // clicks
    bool on_map = game_mode() == GM_MAP && travel_enabled;
    if (ImGui::IsItemDeactivated() && !dragged) {
        float mx, my; to_map(io.MousePos, &mx, &my);
        if (io.KeyCtrl && on_map) {
            int ty = (int)floorf((my - 6) / 4 + 0.5f), tx = (int)floorf((mx - ((ty & 1) ? 8 : 0) - 8) / 16 + 0.5f);
            if (travel_to_tile(tx, ty, -1, "the marked spot")) ui_toggle_map();
        } else selected = hover;
    }
    if (ImGui::IsItemDeactivated()) dragged = false;
    if (hovered && hover >= 0 && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && on_map) {
        for (const Place &p : P) if (p.loc == hover && travel_to_loc(p.loc, p.name.c_str())) { route_note(); ui_toggle_map(); break; }
    }
    /* the panel and the tooltips: at least as big as the menus, bigger on a big screen */
    float pk = fmaxf(1.0f, ui_text_px() * 0.8f / ImGui::GetFontSize());
    if (hover >= 0) {
        for (const Place &p : P) if (p.loc == hover) {
            ImGui::BeginTooltip();
            ImGui::SetWindowFontScale(pk);
            ImGui::TextUnformatted(p.name.c_str());
            ImGui::TextDisabled("%s", kind_label(p.kind));
            if (p.loc < 92) ImGui::TextDisabled(journal_visited(p.loc) ? "In the town journal" : "Not visited yet");
            for (auto &m : marks) if (m.first == p.loc) { ImGui::PushTextWrapPos(UIS(380) * pk); ImGui::TextWrapped("%s", m.second.c_str()); ImGui::PopTextWrapPos(); }
            if (on_map) ImGui::TextDisabled("Double-click to travel there");
            ImGui::EndTooltip();
            break;
        }
    }

    // panel: search, layers, the selected place
    ImGui::SetNextWindowPos(ImVec2(o.x + 14, o.y + 14));
    ImGui::SetNextWindowBgAlpha(0.88f);
    ImGui::Begin("##wmpanel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowFontScale(pk);
    ImGui::TextUnformatted("The Holy Roman Empire");
    ImGui::TextDisabled("Drag to move, wheel to zoom. M or Esc closes.");
    static char find[48];
    ImGui::SetNextItemWidth(UIS(260) * pk);
    ImGui::InputTextWithHint("##find", "Find a place...", find, sizeof find);
    if (find[0]) {
        std::string f = find; std::transform(f.begin(), f.end(), f.begin(), ::tolower);
        int shown = 0;
        for (const Place &p : P) {
            std::string n = p.name; std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.find(f) == std::string::npos) continue;
            char lb[96]; snprintf(lb, sizeof lb, "%s (%s)##%d", p.name.c_str(), kind_label(p.kind), p.loc);
            if (ImGui::Selectable(lb)) { tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = p.loc; find[0] = 0; }
            if (++shown >= 8) break;
        }
    }
    if (ImGui::SmallButton("The party")) { int px, py; party_tile(&px, &py); if (px >= 0) tile_px(px, py, &cx, &cy); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Whole map")) { zoom = fitz; cx = worldmap.full_w * 0.5f; cy = worldmap.full_h * 0.5f; }
    ImGui::Checkbox("Villages", &show_villages); ImGui::SameLine(); ImGui::Checkbox("Castles", &show_castles);
    ImGui::Checkbox("Holy places", &show_holy); ImGui::SameLine(); ImGui::Checkbox("Other places", &show_other);
    ImGui::Checkbox("Names", &show_names); ImGui::SameLine(); ImGui::Checkbox("Quests", &show_quests);
    if (selected >= 0) {
        for (const Place &p : P) if (p.loc == selected) {
            ImGui::Separator();
            ImGui::TextUnformatted(p.name.c_str());
            ImGui::TextDisabled("%s", kind_label(p.kind));
            if (on_map) {
                int gx, gy, m = travel_minutes_loc(p.loc, &gx, &gy);
                if (m >= 0) ImGui::Text("About %s on the way", hhmm(m).c_str());
                if (ImGui::Button("Travel there")) { if (travel_to_loc(p.loc, p.name.c_str())) { route_note(); ui_toggle_map(); } }
            } else ImGui::TextDisabled("Auto-travel works from the travel map.");
            for (auto &m : marks) if (m.first == p.loc) { ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 320 * pk); ImGui::TextWrapped("%s", m.second.c_str()); ImGui::PopTextWrapPos(); }
            break;
        }
    }
    ImGui::End();
    ImGui::End();
}

/* names on the game's own travel map (map.names): the towns and places in view, under their
 * icons (the original names a place only in the top bar, once the pointer is on it). The view:
 * DS:286a its first column, DS:286c + DS:286e its first row; a tile is 16 x 4 game pixels,
 * odd rows half a tile to the right (the route in ui/travel.cpp is drawn the same way). */
extern "C" void travelmap_labels(void)
{
    if (!config_get_int("map.names", 1) || config_get_int("original", 0) || game_mode() != GM_MAP || !travel_map_live() || !ui_font_text) return;
    int vx = (s16)g_rw(0x286a), vr = (s16)g_rw(0x286e) + (s16)g_rw(0x286c);
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImFont *f = ui_font_text;
    float base = ui_text_px();
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int kind = (s16)rw(s, o), x = (s16)rw(s, (u16)(o + 4)), y = (s16)rw(s, (u16)(o + 6)), menu = (s16)rw(s, (u16)(o + 0x0c));
        if (k >= 92 && (menu == 0x62 || kind == 15)) continue;             /* not shown on the map */
        float sx = (float)(16 * (x - vx) + ((y & 1) ? 7 : -1) + 65), sy = (float)(4 * (y - vr) - 6 + 21);
        if (sx < 66 || sx > 314 || sy < 18 || sy > 190) continue;
        std::string name;
        if (k < (int)gamedata.cities.size()) name = gamedata.cities[k].name;
        else { char b[24]; g_str(s, (u16)(o + 0x26), b, sizeof b); name = game_text(b, sizeof b); }
        if (name.empty()) continue;
        bool town = k < 92;
        float size = base * (town ? 1.1f : 0.88f);
        ImVec2 at; plat_game_to_win(sx, sy + 5, &at.x, &at.y);
        ImVec2 ts = f->CalcTextSizeA(size, FLT_MAX, 0, name.c_str());
        ImVec2 tp(at.x - ts.x * 0.5f, at.y);
        float ow = fmaxf(1.0f, roundf(size / 16.0f));
        for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++)
            if (dx || dy) dl->AddText(f, size, ImVec2(tp.x + dx * ow, tp.y + dy * ow), IM_COL32(0, 0, 0, 200), name.c_str());
        dl->AddText(f, size, tp, town ? IM_COL32(255, 244, 214, 255) : IM_COL32(225, 215, 195, 240), name.c_str());
    }
}
