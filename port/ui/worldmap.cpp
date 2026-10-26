#include "vtrace.h"
// worldmap.cpp - the world map: the whole of the Holy Roman Empire as the travel screen draws
// it (mods/worldmap.c), to pan and zoom, with towns and places named, the party, the route of
// auto-travel and the places the quest log points to. Click a place for its details, double-
// click (or Ctrl+click anywhere) to travel there. M on the travel map or a card opens it; M or Esc closes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <functional>
#include <strings.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "padglyphs.h"
#include "imgui.h"
extern "C" int ns_showing(void);
#include "gamedata.h"
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
static float wm_panel_right;                      // the panel's right side (window pixels, last frame): the view's west limit
static float wm_find_left = 1e9f, wm_find_bottom, wm_foot_top = 1e9f;   // the top right panel's left side and foot, the place panel's top (last frame): room past them
static int wm_panel_run, wm_panel_frame = -10;    // frames the panel has been drawn in a row (its size settles after the first), the last
std::string journal_travel_days(int loc);   /* ui/journal.cpp: "3 days" */
extern "C" int journal_buy_window(bool *open, int what), journal_town_noted(int loc); extern "C" void journal_town_wares(int loc);   /* ui/journal.cpp */
static int selected = -1;
static int dock_hover = -1, dock_hover_next = -1;          /* the place panel's boat destination under the pointer (last frame): ringed on the map */
static bool show_names = true, show_quests = true, show_icons = true, show_boats = true, show_days = false;
static bool show_kind[16] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };   /* by icon kind (the legend's entries switch them) */

/* "Towns with ...": the towns that have a place most lack (ui/townicons.h TI_UNIVERSITY .. TI_BANK)
   or a notable shop (wares of 30q and better), marked with its picture on the map and listed */
#include "worldmap_draw.h"
float ui_face_k(ImFont *f);                                  /* overlay.cpp */
struct TownFind { const char *label; int icon; int shop; const char *shrt; };      /* shop: DARKLAND.CTY's shop index, -1 a place */
static const TownFind TFIND[] = {
    { "a university", TI_UNIVERSITY, -1, "University" }, { "a cathedral", TI_CATHEDRAL, -1, "Cathedral" }, { "an alchemist", TI_ALCHEMIST, -1, "Alchemist" },
    { "a physician", TI_PHYSICIAN, -1, "Physician" }, { "docks (boats)", TI_DOCKS, -1, "Docks" }, { "a bank", TI_BANK, -1, "Bank" },
    { "a swordsmith of 30q+", TI_SWORD, 2, "Swords 30q+" }, { "an armorer of 30q+", TI_ARMOUR, 3, "Armor 30q+" }, { "a bowyer of 30q+", TI_BOW, 5, "Bows 30q+" },
    { "an artificer of 30q+", TI_TOOLS, 6, "Artificer 30q+" } };   /* (a library: nearly every town has one; the character sheet's Saints say where each saint is taught) */
static const int NTFIND = (int)(sizeof TFIND / sizeof TFIND[0]);
static int tfind = -1;                                    /* -1: off */
/* the panels' sections folded (map.folded, kept): a bit each */
enum { FOLD_TASKS = 1, FOLD_RUMOURS = 2, FOLD_LEGEND = 4, FOLD_TOWNS = 8, FOLD_SHOW = 16, FOLD_BUY = 32 };
static bool fold_head(const char *label, int bit)
{
    int f = config_get_int("map.folded", FOLD_LEGEND | FOLD_TOWNS | FOLD_BUY);   /* (folded at first: the map in view) */
    ImGui::SetNextItemOpen(!(f & bit), ImGuiCond_Always);
    bool open = ImGui::CollapsingHeader(label);
    if (open == !!(f & bit)) { config_set_int("map.folded", open ? f & ~bit : f | bit); extern int ui_dirty_config; ui_dirty_config = 1; }
    return open;
}
static int town_find_q(int loc)                           /* > 0: town loc matches (for a shop: its quality) */
{
    if (tfind < 0 || loc < 0 || loc >= (int)gamedata.cities.size()) return 0;
    const TownFind &f = TFIND[tfind];
    /* (a university, a cathedral, docks, a bank, and how good a town's smiths and the like are: known
       far and wide, as "everyone knows the swordsmiths of Nuremberg"; the alchemists and a physician,
       once the party has been there: ui/libraries.h) */
    if ((f.icon == TI_ALCHEMIST || f.icon == TI_PHYSICIAN) && !place_offers_known(loc)) return 0;
    if (f.shop < 0) return citynav_town_offers(loc, f.icon) ? 1 : 0;
    int q = gamedata.cities[loc].shopq[f.shop] > 0 ? shop_wares_q(gamedata.cities[loc].shopq[f.shop]) : 0;
    return q >= 30 ? q : 0;
}

void worldmap_set_renderer(SDL_Renderer *r) { wm_ren = r; }

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
extern "C" int travel_preview_loc(int loc, short *xs, short *ys, int max);
static void route_note(void) { if (game_mode() != GM_MAP) ui_notice("Route planned: the party sets off once it is in the overland view."); }

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


/* a name for the search: lower case, the accents off ("Köln" -> "koln"), or with German's "oe"
   for an umlaut ("koeln"), ß as "ss"; so "Kol", "Koln", "Koeln" and "Köln" all find Köln */
static std::string fold_name(const std::string &s, bool umlaut_e)
{
    /* U+00C0..U+00FF, lower case without the accent ('-': the Latin-1 signs kept out) */
    static const char *L1 = "aaaaaaaceeeeiiii" "dnooooo-ouuuuyts" "aaaaaaaceeeeiiii" "dnooooo-ouuuuyty";
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == 0xC3 && i + 1 < s.size()) {
            int cp = 0xC0 + ((unsigned char)s[++i] & 0x3F);
            if (cp == 0xDF) { o += "ss"; continue; }
            char b = L1[cp - 0xC0];
            if (b == '-') continue;
            o += b;
            if (umlaut_e && (cp == 0xC4 || cp == 0xD6 || cp == 0xDC || cp == 0xE4 || cp == 0xF6 || cp == 0xFC)) o += 'e';
        }
        else if (c < 0x80) o += (char)tolower(c);
        else o += (char)c;                                   /* (other letters as they are) */
    }
    return o;
}

extern "C" void ui_toggle_map(void);

static char wm_find[48];                                   /* the Find box's text */
static int wm_test_enter;
static float wm_layers_bottom, wm_layers_left;                             /* the switches' box's foot (last frame), from the window's top */                                   /* (test command ufind: the text, then Enter) */
extern "C" void worldmap_test_find(const char *t) { snprintf(wm_find, sizeof wm_find, "%s", t); wm_test_enter = 1; }
/* Esc on the map: a name being looked for is cleared first (1: by the box itself while typing in
   it), the map closed only then */
extern "C" int worldmap_find_escape(int typing)
{
    if (!wm_find[0]) return 0;
    if (!typing) wm_find[0] = 0;
    return 1;
}

/* what a place has, for the pointer on it and for the place picked in the panel: its offers, the
 * saints its library teaches, the formulae its alchemist sells, its teachers (once known), whether
 * it is in the journal */
static void place_details(const Place &p, float wrap)
{
    auto dim = [wrap](const char *t) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap); ImGui::TextWrapped("%s", t); ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    };
    if (p.loc < 92) ui_town_offers(p.loc, true, ImGui::GetFontSize() * 16, true);
    bool known_here = place_offers_known(p.loc);           /* (ui/libraries.h: been there, or the switch) */
    if (!known_here && p.loc < 92)
        dim("Its saints, formulae and teachers: known once the party has been there.");
    else if (!known_here && library_at(p.loc))
        dim("The saints its library teaches: known once the party has been there.");
    if (const Library *L = known_here ? library_at(p.loc) : nullptr) {        /* the saints its library teaches; new ones marked */
        std::string t; int done[4], nd = 0;
        for (int k = 0; k < 4; k++) {
            int s = L->saint[k]; bool dup = false;
            for (int j = 0; j < nd; j++) if (done[j] == s) dup = true;
            if (dup || s < 0 || s >= (int)gamedata.saints.size()) continue;
            done[nd++] = s;
            t += (t.empty() ? "" : ";  ") + gamedata.saints[s].name + (party_knows_saint(s) ? "" : " (new)");
        }
        ImGui::TextDisabled("Saints taught here:");
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
    }
    {   int fo[4], pr[4];                                /* the formulae its alchemist sells */
        if (known_here && p.loc < 92 && alchemist_formulae(p.loc, fo, pr)) {
            std::string t;
            for (int k = 0; k < 4; k++) if (fo[k] >= 0 && fo[k] < (int)gamedata.formula_names.size()) t += (t.empty() ? "" : ";  ") + gamedata.formula_names[fo[k]] + (party_knows_formula(fo[k]) ? "" : " (new)");
            ImGui::TextDisabled("Formulae sold here:");
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
        }
    }
    if (known_here && p.loc < 92 && ui_teachers_on()) {      /* the teachers it can offer (mods/teachers.c) */
        TchSource ts[12]; int n = ui_town_teachers(p.loc, ts, 12);
        std::string t;
        for (int k = 0; k < n; k++) t += (t.empty() ? "" : ";  ") + std::string(ui_teacher_short(ts[k].src)) + ": " + ui_teacher_skills(ts[k]);
        if (n) {
            ImGui::TextDisabled("Teachers to be hired:");
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
        }
    }
    if (p.loc < 92) ImGui::TextDisabled(journal_visited(p.loc) ? "Visited" : "Not visited yet");
}


/* the frame round the map, after the printed map that came with the game: a checkered green and
   white rule, then a band of coloured fields lettered A-H along the top and the foot and numbered
   1-8 down the sides, a thin dark line each side; a compass rose at the lower left */
/* what the map shows, switched by the buttons on it; the fifth (off at first: busy) the days of
   travel under every town */
#define NLAYER 5
static bool *const LAYER[NLAYER] = { &show_names, &show_quests, &show_boats, &show_icons, &show_days };
static const char *const LAYER_NAME[NLAYER] = { "Names", "Quests", "Boats", "Icons", "Days away" };
static const char *const LAYER_TIP[NLAYER] = { "Place names", "Quest markers", "Boat routes between the towns' docks", "Each place as an icon of its kind (else a dot)",
                                               "The days of travel from the party to every town, under its name (off at first: it crowds the map)" };
static const bool LAYER_DEF[NLAYER] = { true, true, true, true, false };
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
    if (sz.x < 64 || sz.y < 64) { ImGui::End(); return; }      /* (the window minimized: nothing to draw into) */
    dock_hover = dock_hover_next; dock_hover_next = -1;
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + sz.y), IM_COL32(236, 226, 200, 255));   /* (the paper round the map, as the printed one) */
    float ay = aspect();
    float fb = map_border_px() * 2 + map_border_px() * 1.6f;     /* (the frame and the title over it) */
    float fitz = fminf((sz.x - fb) / worldmap.full_w, (sz.y - fb) / (worldmap.full_h * ay));
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
    if (wm_panel_run >= 3 && ImGui::GetFrameCount() - wm_panel_frame <= 1) {   /* the view stays on the map: an edge goes no further in than the window's edge (the west one
           than the panel's right side, so the places under the panel can be seen); a map smaller
           than the window sits in the middle of what the panel leaves */
        float bm = map_border_px() / zoom;                       /* (room for the frame) */
        float hw = sz.x * 0.5f / zoom - bm, hh = sz.y * 0.5f / (zoom * ay) - bm / ay, L = wm_panel_right / zoom;
        /* (and the other edges may come in past the panels over them, and a little more: a corner under a
           panel can be brought out into the open) */
        float slack = 0.12f * fminf(sz.x, sz.y) / zoom;
        float E = fmaxf(0.0f, sz.x - fminf(wm_find_left, sz.x)) / zoom, N = wm_find_bottom / zoom, S = fmaxf(0.0f, sz.y - fminf(wm_foot_top, sz.y)) / zoom;
        float lo = hw - L - slack, hi = worldmap.full_w - hw + E + slack;
        cx = lo > hi ? (lo + hi) * 0.5f : fminf(fmaxf(cx, lo), hi);
        lo = hh - (N + slack) / ay; hi = worldmap.full_h - hh + (S + slack) / ay;
        cy = lo > hi ? (lo + hi) * 0.5f : fminf(fmaxf(cy, lo), hi);
    }

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
    VTMARK("wm start");
    draw_map_frame(dl, to_scr(0, 0), to_scr((float)worldmap.full_w, (float)worldmap.full_h));
    VTMARK("wm frame");

    std::vector<Place> P = places();
    static std::vector<std::pair<int, std::string>> marks;
    static std::vector<int> mark_cats;
    static int marks_frame = -1000;
    static std::vector<TownMark> tmarks;
    if (ImGui::GetFrameCount() - marks_frame > 30) { quest_marks_cat(marks, mark_cats); town_marks(tmarks); marks_frame = ImGui::GetFrameCount(); }

    // auto-travel route
    if (travel.active && travel.n > 1) {
        std::vector<ImVec2> pts;
        for (int i = travel.i > 0 ? travel.i - 1 : 0; i < travel.n; i++) { float x, y; tile_px(travel.rx[i], travel.ry[i], &x, &y); pts.push_back(to_scr(x, y)); }
        dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(0, 0, 0, 160), 0, 5);
        dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(255, 220, 90, 255), 0, 2.5f);
    }

    // the boats between the towns' docks: a glowing dashed blue line along the water
    if (show_boats) {
        float t = (float)ImGui::GetTime();
        float lw = fmaxf(1.5f, 2.2f * fminf(1.6f, zoom / fitz));
        for (int a = 0; a < (int)gamedata.cities.size(); a++)
            for (int k = 0; k < 5; k++) {
                int b = gamedata.cities[a].docks[k];
                if (b < 0 || b >= (int)gamedata.cities.size() || b == a) continue;
                bool both = false; for (int j = 0; j < 5; j++) if (gamedata.cities[b].docks[j] == a) both = true;
                if (both && b < a) continue;                         /* (a way both ways: drawn once) */
                bool lit = dock_hover >= 0 ? ((a == selected && b == dock_hover) || (b == selected && a == dock_hover)) : (selected == a || selected == b);
                const short *xs, *ys; int n = travel_boat_way(a, b, &xs, &ys);
                std::vector<ImVec2> pts;
                if (n >= 2) for (int i = 0; i < n; i += (n > 60 ? 2 : 1)) { float x, y; tile_px(xs[i], ys[i], &x, &y); pts.push_back(to_scr(x, y)); }
                if (n >= 2) { float x, y; tile_px(xs[n - 1], ys[n - 1], &x, &y); pts.push_back(to_scr(x, y)); }
                else for (int e = 0; e < 2; e++) { u32 r = g_location(e ? b : a); if (!r) continue; float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y); pts.push_back(to_scr(x, y)); }
                if (pts.size() < 2) continue;
                {   /* (off the window: not drawn; points closer than a few pixels: one, the way looks the same with far fewer vertices) */
                    ImVec2 lo = pts[0], hi = pts[0];
                    for (const ImVec2 &q : pts) { lo.x = fminf(lo.x, q.x); lo.y = fminf(lo.y, q.y); hi.x = fmaxf(hi.x, q.x); hi.y = fmaxf(hi.y, q.y); }
                    ImVec2 ds = ImGui::GetIO().DisplaySize;
                    if (hi.x < -20 || hi.y < -20 || lo.x > ds.x + 20 || lo.y > ds.y + 20) continue;
                    std::vector<ImVec2> q; q.reserve(pts.size());
                    for (size_t i = 0; i < pts.size(); i++) {
                        if (!q.empty() && i + 1 < pts.size()) { float dx = pts[i].x - q.back().x, dy = pts[i].y - q.back().y; if (dx * dx + dy * dy < 36) continue; }
                        q.push_back(pts[i]);
                    }
                    pts.swap(q);
                }
                int al = selected >= 0 && !lit ? 90 : 255;
                boat_way_draw(dl, pts, lw, lit, al, t, both);
            }
    }

    VTMARK("wm boats");
    // the way the party would take to the place chosen: a dotted line (on the travel map; the
    // planned journey's own way while travelling there)
    if (selected >= 0 && game_mode() == GM_MAP && travel_enabled) {

        static short px_[4096], py_[4096]; static int pn, p_loc = -2, p_frame;
        int px0, py0; party_tile(&px0, &py0);
        static int ppx = -1, ppy = -1;
        if (p_loc != selected || ppx != px0 || ppy != py0 || ImGui::GetFrameCount() - p_frame > 120) {
            pn = travel_preview_loc(selected, px_, py_, 4096); if (getenv("DL_TRAVEL_LOG")) fprintf(stderr, "world map: the way to %d: %d tiles\n", selected, pn); p_loc = selected; ppx = px0; ppy = py0; p_frame = ImGui::GetFrameCount();
        }
        if (pn >= 2) {
            float dz = fmaxf(3.5f, 4.0f * fminf(1.6f, zoom / fitz));
            float t = (float)ImGui::GetTime(), sp = dz * 4.0f, acc = sp - fmodf(t * 30.0f, sp);   /* (the dots run from the party towards the place) */
            ImVec2 prev = ImVec2(0, 0);
            for (int i = 0; i < pn; i++) {
                float x, y; tile_px(px_[i], py_[i], &x, &y); ImVec2 q = to_scr(x, y);
                if (i) {
                    float L = sqrtf((q.x - prev.x) * (q.x - prev.x) + (q.y - prev.y) * (q.y - prev.y));
                    for (float s0 = fmodf(sp - acc + 1000 * sp, sp); s0 < L; s0 += sp) {
                        ImVec2 d(prev.x + (q.x - prev.x) * s0 / L, prev.y + (q.y - prev.y) * s0 / L);
                        dl->AddCircleFilled(d, dz * 1.05f, IM_COL32(40, 20, 8, 230), 10);
                        dl->AddCircleFilled(d, dz * 0.7f, IM_COL32(255, 240, 180, 255), 10);
                    }
                    acc = fmodf(acc + L, sp);
                }
                prev = q;
            }
        }
    }

    if (dock_hover >= 0) {                                         /* a boat's destination pointed at in the place panel: ringed, pulsing */
        u32 r = g_location(dock_hover);
        if (r) {
            float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y);
            ImVec2 c = to_scr(x, y); float pr = 18.0f + 4.0f * sinf((float)ImGui::GetTime() * 5.0f);
            dl->AddCircle(c, pr + 3, IM_COL32(40, 20, 8, 200), 32, 5.0f);
            dl->AddCircle(c, pr, IM_COL32(255, 220, 110, 255), 32, 3.0f);
        }
    }
    // places
    int hover = -1; float hd = 1e9f; ImVec2 hov0, hov1;      /* (the place under the pointer, and where: its tooltip beside it) */
    VTMARK("wm way");
    /* names with the screen (ui_text_px: display.text_size), a little larger as one zooms in */
    ImFont *fs = ui_font_overlay;
    float ks = ui_text_px() / 22.0f;
    float fsize = ui_text_px() * 0.68f * fminf(fmaxf(powf(zoom / fitz, 0.3f), 0.9f), 1.6f);
    for (const Place &p : P) {
        int g = kind_group(p.kind);
        if (!show_kind[icon_kind(p.kind)]) continue;
        float x, y; tile_px(p.x, p.y, &x, &y);
        ImVec2 s = to_scr(x, y);
        if (s.x < o.x - 50 || s.y < o.y - 20 || s.x > o.x + sz.x + 50 || s.y > o.y + sz.y + 20) continue;
        float r = (g == 0 ? 4.5f : 3.0f) * fmaxf(ks, 1.0f);
        /* the towns at every zoom; every other place comes in slowly as one zooms closer (far out the map
           would be all icons) */
        float fade = g == 0 ? 1.0f : fminf(1.0f, fmaxf(0.0f, (zoom - 0.61f) / 0.48f));
        if (fade <= 0.02f && p.loc != selected) continue;
        if (p.loc == selected) fade = 1.0f;
        int v_fade = dl->VtxBuffer.Size;
        bool unseen = g == 0 && p.loc < 92 && !journal_visited(p.loc);   /* a town the party has not been to: faded, a ring */
        if (show_icons) {                                    /* an icon for its kind, bigger as one zooms in */
            float is = r * 4.4f * fminf(fmaxf(powf(zoom / fitz, 0.25f), 1.0f), 1.6f);
            if (g == 0 && p.loc < (int)gamedata.cities.size()) {   /* a town: on its seal of wax, as large as the town, the party's ring pressed in once visited */
                r = draw_town_seal(dl, s, is, gamedata.cities[p.loc].size, !unseen);
            } else {
                draw_icon(dl, s, is, icon_kind(p.kind));
                r = is * 0.5f;
                if (unseen) dl->AddCircleFilled(s, r * 0.9f, IM_COL32(236, 226, 204, 150), 20);   /* (washed out, as not yet seen) */
            }
        } else if (unseen) {
            dl->AddCircle(s, r, IM_COL32(0, 0, 0, 200), 0, 3.0f);
            dl->AddCircle(s, r, kind_colour(g), 0, 1.6f);
        } else {
            dl->AddCircleFilled(s, r + 1.5f, IM_COL32(0, 0, 0, 200));
            dl->AddCircleFilled(s, r, kind_colour(g));
        }
        if (g == 0 && tfind >= 0 && town_find_q(p.loc)) {    /* "Towns with ...": its picture over the town, a gold ring round it */
            float k = fmaxf(ks, 1.0f), is = 24 * k;
            dl->AddCircle(s, r + 3 * k, IM_COL32(20, 14, 8, 230), 0, 4 * k);
            dl->AddCircle(s, r + 3 * k, IM_COL32(240, 198, 92, 255), 0, 2.2f * k);
            ui_draw_town_icon(dl, ImVec2(s.x, s.y - r - is * 0.55f - 2 * k), is, TFIND[tfind].icon);
        }
        {   int n = 0;                                       /* a town that bars the party, or whose watch wants it: its sign beside it */
            for (const TownMark &t : tmarks)
                if (t.loc == p.loc) { float ts = fmaxf(r * 1.5f, 16.0f * fmaxf(ks, 1.0f)); ui_draw_quest_icon(dl, ImVec2(s.x + r + ts * (0.45f + 1.0f * n), s.y - r * 0.6f), ts, t.cat); n++; }
        }
        float d = (io.MousePos.x - s.x) * (io.MousePos.x - s.x) + (io.MousePos.y - s.y) * (io.MousePos.y - s.y);
        float hr = 14 * fmaxf(ks, 1.0f);
        if (hovered && fade > 0.5f && d < hr * hr && d < hd) { hd = d; hover = p.loc; hov0 = ImVec2(s.x - hr, s.y - hr); hov1 = ImVec2(s.x + hr, s.y + hr); }
        int big = p.loc < (int)gamedata.cities.size() ? gamedata.cities[p.loc].size : 0;
        bool label = show_names && (g == 0 ? (zoom >= 0.3f || (zoom >= 0.14f && big >= 6)) : zoom >= 0.9f);
        if (label || p.loc == selected) {
            ImVec2 ts = fs->CalcTextSizeA(fsize, FLT_MAX, 0, p.name.c_str());
            ImVec2 tp(s.x - ts.x * 0.5f, s.y + r + 2);
            float ow = fmaxf(1.0f, roundf(fsize / 18.0f));                       /* the dark outline, thicker with the letters */
            for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++)
                if (dx || dy) dl->AddText(fs, fsize, ImVec2(tp.x + dx * ow, tp.y + dy * ow), IM_COL32(0, 0, 0, 220), p.name.c_str());
            dl->AddText(fs, fsize, tp, g == 0 ? (unseen ? IM_COL32(205, 192, 168, 255) : IM_COL32(255, 244, 214, 255)) : IM_COL32(235, 225, 205, 255), p.name.c_str());
        }
        if (show_days && g == 0) {                           /* the days of travel to it, under its name */
            static std::map<int, std::string> days; static int df = -1000;      /* (worked out again every two seconds or so) */
            if (ImGui::GetFrameCount() - df > 120) { days.clear(); df = ImGui::GetFrameCount(); }
            auto it = days.find(p.loc);
            if (it == days.end()) it = days.emplace(p.loc, journal_travel_days(p.loc)).first;
            const std::string &t = it->second;
            if (!t.empty()) {
                float fs2 = fsize * 0.8f; ImVec2 ts = fs->CalcTextSizeA(fs2, FLT_MAX, 0, t.c_str());
                ImVec2 tp(s.x - ts.x * 0.5f, s.y + r + 2 + fsize * ((label || p.loc == selected) ? 1.05f : 0.0f));
                dl->AddRectFilled(ImVec2(tp.x - 3, tp.y), ImVec2(tp.x + ts.x + 3, tp.y + ts.y), IM_COL32(40, 26, 12, 200), 3);
                dl->AddText(fs, fs2, tp, IM_COL32(250, 214, 120, 255), t.c_str());
            }
        }
        if (fade < 1.0f)                                     /* (coming in: all it drew, faint) */
            for (int vi = v_fade; vi < dl->VtxBuffer.Size; vi++) { ImU32 &cc = dl->VtxBuffer[vi].col; int al = (int)((cc >> IM_COL32_A_SHIFT) & 255); cc = (cc & ~IM_COL32_A_MASK) | ((ImU32)(al * fade) << IM_COL32_A_SHIFT); }
    }
    // quest marks
    if (show_quests) {                                       /* the quest's picture on a pin above the place; several: round the place
                                                                on an arc of fixed size over it (many quests there: a crowded arc,
                                                                never a long row) */
        std::vector<int> done;
        for (size_t i = 0; i < marks.size(); i++) {
            int loc = marks[i].first;
            if (std::find(done.begin(), done.end(), loc) != done.end()) continue;
            done.push_back(loc);
            u32 r = g_location(loc);
            if (!r) continue;
            std::vector<size_t> qs; for (size_t j = 0; j < marks.size(); j++) if (marks[j].first == loc) qs.push_back(j);
            int n = (int)qs.size();
            float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y);
            float k = fmaxf(ks, 1.0f), is = 22 * k;
            ImVec2 mid = to_scr(x, y), s = mid; s.y -= 10 * k;
            std::vector<ImVec2> at(n);
            for (int j = 0; j < n; j++) {
                if (n == 1) { at[j] = ImVec2(s.x, s.y - is * 0.55f); continue; }
                /* (an arc over the place, open below where its name is: up to 60 degrees apart, 240 in all) */
                float span = fminf(240.0f, 60.0f * (n - 1)), a = (270.0f - span * 0.5f + span * j / (n - 1)) * 0.0174533f, R = is * 1.2f;
                at[j] = ImVec2(mid.x + cosf(a) * R, mid.y + sinf(a) * R);
            }
            int hv = -1;                                     /* (the pointer on one: the topmost) */
            for (int j = n - 1; j >= 0 && hv < 0; j--) {
                float qd = (io.MousePos.x - at[j].x) * (io.MousePos.x - at[j].x) + (io.MousePos.y - at[j].y) * (io.MousePos.y - at[j].y);
                if (hovered && qd < is * is * 0.36f) hv = j;
            }
            if (n == 1) dl->AddTriangleFilled(ImVec2(at[0].x - 4 * k, at[0].y + is * 0.3f), ImVec2(at[0].x + 4 * k, at[0].y + is * 0.3f), ImVec2(s.x, s.y + 2 * k), IM_COL32(20, 14, 8, 230));
            for (int j = 0; j < n; j++) quest_glow(dl, at[j], is);
            for (int j = 0; j < n; j++) if (j != hv) ui_draw_quest_icon(dl, at[j], is, qs[j] < mark_cats.size() ? mark_cats[qs[j]] : 0);
            if (hv >= 0) ui_draw_quest_icon(dl, at[hv], is * 1.12f, qs[hv] < mark_cats.size() ? mark_cats[qs[hv]] : 0);
            /* each picture is the place too: pointed at, clicked (the place chosen), double-clicked (travel
               there), even with the places' icons hidden */
            if (hv >= 0) { hd = 0; hover = loc; hov0 = ImVec2(at[hv].x - is * 0.5f, at[hv].y - is * 0.5f); hov1 = ImVec2(at[hv].x + is * 0.5f, at[hv].y + is * 0.5f); }
        }
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
    VTMARK("wm places");
    /* the panel and the tooltips: at least as big as the menus, bigger on a big screen */
    float pk = fmaxf(1.0f, ui_text_px() * 0.8f / ImGui::GetFontSize());
    if (hover >= 0) {
        for (const Place &p : P) if (p.loc == hover) {
            ui_tip_beside(hov0, hov1);
            ImGui::BeginTooltip();
            ImGui::SetWindowFontScale(pk);
            ImGui::TextUnformatted(p.name.c_str());
            ImGui::TextDisabled("%s", kind_line(P, p).c_str());
            if (selected != p.loc) ImGui::TextDisabled("Click for what it offers, and to travel there");
            for (auto &m : marks) if (m.first == p.loc) { ImGui::PushTextWrapPos(UIS(380) * pk); ImGui::TextWrapped("%s", m.second.c_str()); ImGui::PopTextWrapPos(); }
            for (const TownMark &t : tmarks) if (t.loc == p.loc) ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.15f, 1), "%s %s", t.cat == QC_UNWELCOME ? "Unwelcome here" : "Wanted by the watch here", t.until.c_str());
            ImGui::EndTooltip();
            break;
        }
    }

    /* three panels round the map: the party's tasks at the left; finding, the legend (which also
       switches what the map shows) and the towns with something at the top right; the place chosen
       at the foot, its Travel there always in view */
    float pad = 14 * fmaxf(1.0f, pk * 0.8f);
    bool in_town = !on_map && citynav_available() && !config_get_int("original", 0);
    float foot_h = 0;
    const Place *sel = nullptr;
    for (const Place &p : P) if (p.loc == selected) sel = &p;

    /* ---- the foot: the place chosen */
    if (sel) {
        const Place &p = *sel;
        float fw = fminf(sz.x - 2 * pad, UIS(820) * pk), fh = fminf(sz.y * 0.4f, UIS(340) * pk);   /* (no wider than it needs: the map round it) */
        ImGui::SetNextWindowPos(ImVec2(o.x + sz.x * 0.5f, o.y + sz.y - pad), ImGuiCond_Always, ImVec2(0.5f, 1));
        ImGui::SetNextWindowSizeConstraints(ImVec2(fw, 0), ImVec2(fw, fh));    /* (as tall as what it says, no more) */
        ImGui::SetNextWindowBgAlpha(0.97f);
        ImGui::Begin("##wmplace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(pk);
        foot_h = ImGui::GetWindowHeight();
        wm_foot_top = ImGui::GetWindowPos().y - o.y;
        /* its head, always in view: the name, what it is, the journey and the button */
        if (ui_font_bold) ImGui::PushFont(ui_font_bold);
        ImGui::TextUnformatted(p.name.c_str());
        if (ui_font_bold) ImGui::PopFont();
        ImGui::SameLine(); ImGui::TextDisabled("%s", kind_line(P, p).c_str());
        float bx = ImGui::GetWindowContentRegionMax().x;
        if (on_map) {
            int m = travel_minutes_loc(p.loc, nullptr, nullptr);
            std::string j = m >= 0 ? "About " + hhmm(m) + " of travel" : std::string("No way there from here");
            float bw = ImGui::CalcTextSize("Travel there").x + ImGui::GetStyle().FramePadding.x * 2;
            ImGui::SameLine(bx - bw - ImGui::CalcTextSize(j.c_str()).x - ImGui::GetStyle().ItemSpacing.x * 2);
            ImGui::TextDisabled("%s", j.c_str());
            ImGui::SameLine(bx - bw);
            ImGui::BeginDisabled(m < 0);
            if (ImGui::Button("Travel there")) { if (travel_to_loc(p.loc, p.name.c_str())) { route_note(); ui_toggle_map(); } }
            ImGui::EndDisabled();
        } else if (in_town) {
            const char *lb = "Leave town and travel there";
            float bw = ImGui::CalcTextSize(lb).x + ImGui::GetStyle().FramePadding.x * 2;
            ImGui::SameLine(bx - bw);
            if (ImGui::Button(lb)) ImGui::OpenPopup("##leavetravel");
        } else {
            const char *t = "To travel, be in the overland view or in a town's streets";
            ImGui::SameLine(bx - ImGui::CalcTextSize(t).x); ImGui::TextDisabled("%s", t);
        }
        if (ImGui::BeginPopupModal("##leavetravel", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            int cur = (s16)g_rw(G_CUR_LOC);
            std::string town = cur >= 0 && cur < (int)gamedata.cities.size() ? gamedata.cities[cur].name : std::string("the town");
            ImGui::Text("Walk out of %s's gate and travel to %s?", town.c_str(), p.name.c_str());
            ImGui::TextDisabled("The party walks through the streets to a gate and out; in the overland view the journey begins.");
            if (ImGui::Button("Leave and travel")) {
                void citynav_leave_town(void);
                travel_after_leaving(p.loc, p.name.c_str());
                citynav_leave_town();
                ImGui::CloseCurrentPopup(); ImGui::EndPopup(); ImGui::End(); ui_toggle_map(); ImGui::End(); return;
            }
            ImGui::SameLine();
            if (ImGui::Button("Stay")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SameLine(); ImGui::Dummy(ImVec2(0, 0));
        ImGui::Separator();
        /* the rest */
        float colw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        if (ImGui::BeginTable("##wmpt", 2, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();
            place_details(p, colw - 8);
            {   /* the wares noted in its shops (ui/journal.cpp) */
                int ns = journal_town_noted(p.loc);
                if (ns) { char lb[64]; snprintf(lb, sizeof lb, "The wares noted here (%d shop%s)", ns, ns == 1 ? "" : "s");
                          if (ImGui::TreeNode(lb)) { journal_town_wares(p.loc); ImGui::TreePop(); } }
            }
            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(0);
            if (p.loc >= 0 && p.loc < (int)gamedata.cities.size()) {        /* its docks: the towns its boats go to (a click shows one) */
                const City &town = gamedata.cities[p.loc];
                /* a row of town buttons, wrapped to the column: pointed at, the route lights; a click shows the town */
                auto boat_row = [&](const char *head, const std::vector<int> &to, int tag) {
                    for (size_t k = 0; k < to.size(); k++) {
                        const City &d2 = gamedata.cities[to[k]];
                        char lb[64]; snprintf(lb, sizeof lb, "%s##boat%d_%d", d2.name.c_str(), tag, (int)k);
                        float bw2 = ImGui::CalcTextSize(lb, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2;
                        if (!k) ImGui::TextDisabled("%s", head);
                        else if (ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + bw2 <= ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x) ImGui::SameLine();
                        bool clk = ImGui::SmallButton(lb);
                        if (ImGui::IsItemHovered()) dock_hover_next = to[k];
                        if (clk) { selected = to[k]; for (const Place &q : P) if (q.loc == selected) { tile_px(q.x, q.y, &cx, &cy); } }
                    }
                };
                std::vector<int> out, in;
                for (int k = 0; k < 5; k++) if (town.docks[k] >= 0 && town.docks[k] < (int)gamedata.cities.size() && town.docks[k] != p.loc) out.push_back(town.docks[k]);   /* (Groningen's lists itself) */
                for (int o = 0; o < (int)gamedata.cities.size(); o++) if (o != p.loc) for (int k = 0; k < 5; k++) if (gamedata.cities[o].docks[k] == p.loc) { in.push_back(o); break; }
                boat_row("Boats leaving here for:", out, 0);
                if (out.empty() && citynav_town_offers(p.loc, TI_DOCKS)) ImGui::TextDisabled("Its docks have no boats leaving (the original game has none either).");
                if (!in.empty()) { if (!out.empty()) ImGui::Spacing(); boat_row("Boats arriving here from:", in, 1); }
                if (!out.empty() || !in.empty()) ImGui::TextDisabled("Not every boat sails every day; the docks say which leave today.");
            }
            for (auto &m : marks) if (m.first == p.loc) ImGui::TextWrapped("%s", m.second.c_str());
            for (const TownMark &t : tmarks) if (t.loc == p.loc) ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.15f, 1), "%s %s.", t.cat == QC_UNWELCOME ? "Unwelcome here" : "Wanted by the watch here", t.until.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTable();
        }
        ImGui::End();
    }
    if (!sel) wm_foot_top = 1e9f;
    float avail_h = sz.y - 2 * pad - (foot_h > 0 ? foot_h + pad : 0);

    VTMARK("wm foot");
    /* ---- the left: what the map is, the view buttons, the journey, the tasks and rumours */
    ImGui::SetNextWindowPos(ImVec2(o.x + pad, o.y + pad));
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(fminf(UIS(440) * pk, sz.x * 0.3f), avail_h));
    ImGui::Begin("##wmpanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    if (ImGui::GetFrameCount() - wm_panel_frame > 1) wm_panel_run = 0;
    wm_panel_run++; wm_panel_frame = ImGui::GetFrameCount();
    wm_panel_right = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - o.x;
    ImGui::SetWindowFontScale(pk);
    ImGui::TextUnformatted("The Holy Roman Empire");
    if (ui_pad_prompts()) ui_pad_text("{LSTICK} moves the pointer, hold {A} to drag, {RSTICK} zooms. {BACK} or {Y} closes the world map.", true);
    else ImGui::TextDisabled("Drag to move, wheel to zoom. M or Esc closes.");
    if (travel.active) {                                   /* the journey under way */
        ImGui::TextColored(ImVec4(0.55f, 0.24f, 0.12f, 1), "Traveling to %s", travel.name);
        ImGui::SameLine();
        if (ImGui::SmallButton("Stop")) travel_stop();
    }
    {   /* the party's quests, in groups by kind (a reward waiting first), each folding away and sorted
           nearest first, or soonest due for a kind that has deadlines; then the rumors heard. A click
           shows the place (and chooses it, for Travel there) */
        static std::vector<QuestRow> qr; static int qf = -1000;
        if (ImGui::GetFrameCount() - qf > 30) { quest_rows(qr, QSORT_NEAREST, true); quest_merge(qr); qf = ImGui::GetFrameCount(); }
        auto row = [&](const QuestRow &r) {
            std::string d = quest_days(r);
            float isz = ImGui::GetTextLineHeight() * 1.15f, sp = ImGui::CalcTextSize(" ").x;
            std::string pad2((size_t)ceilf((isz + sp) / sp), ' ');      /* (room for the picture) */
            char lb[192]; snprintf(lb, sizeof lb, "%s%s%s%s##q%s", pad2.c_str(), r.title.c_str(), d.empty() ? "" : "   ", d.c_str(), r.id.c_str());
            bool sl = ImGui::Selectable(lb, selected == r.loc);
            { ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + isz * 0.5f, (a.y + b.y) * 0.5f), isz, r.cat); }
            if (sl) {
                for (const Place &p : P) if (p.loc == r.loc) { tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; }
                selected = r.loc;
            }
            if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextUnformatted(r.text.c_str()); if (!r.when.empty()) ImGui::TextDisabled("%s", r.when.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
        };
        bool none = true;
        static const int ORDER[] = { QC_REWARD, QC_ROBBER, QC_RETRIEVE, QC_PAPERS, QC_SABBAT, QC_POLITICS, QC_WAR, QC_MINE, QC_DRAGON, QC_HUNT, QC_NONE };
        static const char *const HEAD[] = { "Collect your reward", "Robber knights", "Treasures to fetch", "Papers to get in a town", "Sabbats", "City politics", "Cities at war", "Mines", "Dragons", "The Wild Hunt", "Other quests" };
        bool qopen = false; int nq = 0;
        for (const QuestRow &r : qr) if (r.loc >= 0 && r.group <= 1) nq++;
        if (nq) { none = false; qopen = fold_head("Quests", FOLD_TASKS); }
        for (int g = 0; qopen && g < (int)(sizeof ORDER / sizeof ORDER[0]); g++) {
            std::vector<const QuestRow *> v; bool dated = false;
            for (const QuestRow &r : qr) {
                if (r.loc < 0 || r.group > 1) continue;
                bool known = false; for (int c : ORDER) if (c == r.cat && c != QC_NONE) known = true;
                if (ORDER[g] == QC_NONE ? known : r.cat != ORDER[g]) continue;
                v.push_back(&r); if (r.days_left >= 0) dated = true;
            }
            if (v.empty()) continue;
            char key[32]; snprintf(key, sizeof key, "map.qsort.%d", ORDER[g]);
            int soon = dated ? config_get_int(key, 0) : 0;
            if (soon) std::stable_sort(v.begin(), v.end(), [](const QuestRow *a, const QuestRow *b) {
                int da = a->days_left < 0 ? 1 << 30 : a->days_left, db = b->days_left < 0 ? 1 << 30 : b->days_left; return da < db; });
            ImGui::PushID(g);
            char lab[96]; snprintf(lab, sizeof lab, "%s (%d)###qg", HEAD[g], (int)v.size());
            bool open = ImGui::TreeNodeEx(lab, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
            if (dated && v.size() > 1) {   /* nearest or soonest due: a click on the word flips it */
                const char *w = soon ? "soonest due" : "nearest";
                ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(w).x - ImGui::GetStyle().FramePadding.x * 2);
                if (ImGui::SmallButton(w)) { config_set_int(key, !soon); extern int ui_dirty_config; ui_dirty_config = 1; }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sorted %s first: click for %s first.", w, soon ? "the nearest" : "the soonest due");
            }
            if (open) { for (const QuestRow *r : v) row(*r); ImGui::TreePop(); }
            ImGui::PopID();
        }
        {   /* the rumors heard (marked on the map too) */
            bool any = false; for (const QuestRow &r : qr) if (r.loc >= 0 && (r.group == 2 || r.group == 4)) any = true;
            if (any) { none = false; if (fold_head("Rumors", FOLD_RUMOURS)) for (const QuestRow &r : qr) if (r.loc >= 0 && (r.group == 2 || r.group == 4)) row(r); }
        }
        if (none) ImGui::TextDisabled("No quests or rumors yet.");
    }
    ImGui::End();

    /* ---- the top right: find, the legend (a click on an entry hides or shows those places), marks, towns with ... */
    ImGui::SetNextWindowPos(ImVec2(o.x + sz.x - pad, o.y + pad), ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(fminf(UIS(380) * pk, sz.x * 0.27f), avail_h));
    ImGui::Begin("##wmfind", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    wm_find_left = ImGui::GetWindowPos().x - o.x; wm_find_bottom = ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - o.y;
    ImGui::SetWindowFontScale(pk);
    float rw = fminf(UIS(300) * pk, sz.x * 0.27f - 2 * ImGui::GetStyle().WindowPadding.x);
    {   /* the two finders: windows of their own (ui/journal.cpp); a click on a town there shows it here */
        static bool buy_open[2];
        if (ImGui::Button("Find alchemical ingredients", ImVec2(rw, 0))) buy_open[0] = !buy_open[0];
        if (ImGui::Button("Find any ware", ImVec2(rw, 0))) buy_open[1] = !buy_open[1];
        for (int w = 0; w < 2; w++) if (buy_open[w]) {
            int pick = journal_buy_window(&buy_open[w], w);
            if (pick >= 0) for (const Place &p : P) if (p.loc == pick) { tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = pick; }
        }
    }
    char *find = wm_find;
    ImGui::SetNextItemWidth(rw);
    bool enter = ImGui::InputTextWithHint("##find", "Find a place...", find, sizeof wm_find, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll);
    if (wm_test_enter) { enter = true; wm_test_enter = 0; }
    if (find[0]) {                                         /* the places named so, the nearest first on the travel map, with the journey */
        std::string f = fold_name(find, false);
        std::vector<std::pair<int, const Place *>> hits;      /* (minutes on the way, -1 unknown) */
        for (const Place &p : P)
            if (fold_name(p.name, false).find(f) != std::string::npos || fold_name(p.name, true).find(f) != std::string::npos)
                hits.push_back({ on_map ? travel_minutes_loc(p.loc, nullptr, nullptr) : -1, &p });
        std::stable_sort(hits.begin(), hits.end(), [](const std::pair<int, const Place *> &a, const std::pair<int, const Place *> &b) {
            return (a.first < 0 ? INT_MAX : a.first) < (b.first < 0 ? INT_MAX : b.first); });
        /* a row a place: its picture, the name, what it is (dim, left out when the row has no room) and
           the journey at the right; the whole row to click */
        float lh = ImGui::GetTextLineHeight(), isz = lh * 0.95f, gap = ImGui::GetStyle().ItemSpacing.x;
        ImDrawList *pdl = ImGui::GetWindowDrawList();
        for (size_t i = 0; i < hits.size() && i < 8; i++) {
            const Place &p = *hits[i].second;
            ImGui::PushID(p.loc);
            ImVec2 a = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##hit", false, 0, ImVec2(rw, 0)) || (enter && i == 0)) {   /* (Enter: the first) */ fprintf(stderr, "world map: found %s\n", p.name.c_str()); tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = p.loc; find[0] = 0; }
            ImGui::PopID();
            draw_icon(pdl, ImVec2(a.x + isz * 0.5f, a.y + lh * 0.5f), isz, icon_kind(p.kind));
            std::string t = hits[i].first >= 0 ? hhmm(hits[i].first) : std::string();
            float tx = a.x + rw - ImGui::CalcTextSize(t.c_str()).x, nx = a.x + isz + gap * 0.6f;
            ImU32 ink = ImGui::GetColorU32(ImGuiCol_Text), dimc = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            ImVec4 clip(nx, a.y, tx - gap, a.y + lh);
            pdl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(nx, a.y), ink, p.name.c_str(), nullptr, 0, &clip);
            float kx = nx + ImGui::CalcTextSize(p.name.c_str()).x + gap * 0.7f;
            if (kx < tx - gap) {
                std::string k = kind_line(P, p);
                if (kx + ImGui::CalcTextSize(k.c_str()).x > tx - gap) k = kind_label(p.kind);   /* ("cave near Köln": "cave") */
                if (kx + ImGui::CalcTextSize(k.c_str()).x <= tx - gap)                        /* (whole or not at all: the picture says it too) */
                    pdl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(kx, a.y), dimc, k.c_str(), nullptr, 0, &clip);
            }
            if (!t.empty()) pdl->AddText(ImVec2(tx, a.y), dimc, t.c_str());
        }
        if (hits.size() > 8) ImGui::TextDisabled("and %d more: type more of the name to narrow the list", (int)hits.size() - 8);
        if (hits.empty()) ImGui::TextDisabled("No place of that name.");
    }
    /* a grid of entries, two to a row, each a picture and a word, in the small font: the legend's
       kinds of place (a click shows or hides them) and the towns' offers (a click marks the towns
       with it, again unmarks) */
    float rcolw = rw * 0.5f;
    auto grid_entry = [&](int k, bool on, bool lit, const char *label, auto draw_pic) {
        float is = ImGui::GetFontSize(), sp = ImGui::GetStyle().ItemSpacing.x;
        if (k % 2) ImGui::SameLine(rcolw);
        ImVec2 q = ImGui::GetCursorScreenPos();
        ImGui::PushID(k);
        bool click = ImGui::InvisibleButton("##ge", ImVec2(rcolw - sp, is * 1.15f));
        bool hv = ImGui::IsItemHovered();
        ImGui::PopID();
        ImDrawList *pl = ImGui::GetWindowDrawList();
        ImVec2 q2(q.x + rcolw - sp, q.y + is * 1.15f);
        if (lit) pl->AddRectFilled(q, q2, IM_COL32(196, 150, 84, 120), 3);
        if (hv) pl->AddRectFilled(q, q2, IM_COL32(200, 170, 120, 90), 3);
        ImVec2 c(q.x + is * 0.6f, q.y + is * 0.575f);
        draw_pic(pl, c, is);
        pl->AddText(ImVec2(q.x + is * 1.35f, q.y + is * 0.075f), on ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled), label);
        if (!on) {                                              /* hidden: the picture faded and struck through */
            pl->AddCircleFilled(c, is * 0.55f, IM_COL32(236, 226, 204, 170));
            pl->AddLine(ImVec2(c.x - is * 0.45f, c.y + is * 0.45f), ImVec2(c.x + is * 0.45f, c.y - is * 0.45f), IM_COL32(150, 50, 30, 220), 2);
        }
        return click;
    };
    ImGui::Spacing();
    if (fold_head(tfind < 0 ? "Towns with ..." : (std::string("Towns with ") + TFIND[tfind].label + "###tw").c_str(), FOLD_TOWNS)) {
        if (ui_font_small) ImGui::PushFont(ui_font_small);
        ImGui::TextDisabled("Click one to circle those towns on the map.");
        for (int k = 0; k < NTFIND; k++)
            if (grid_entry(100 + k, true, tfind == k, TFIND[k].shrt, [&](ImDrawList *pl, ImVec2 c, float is) { ui_draw_town_icon(pl, c, is * 1.1f, TFIND[k].icon); }))
                tfind = tfind == k ? -1 : k;
        if (ui_font_small) ImGui::PopFont();
        if (tfind >= 0) {                                   /* the towns, by name (a shop's: the best first), a click shows one */
            std::vector<std::pair<int, const Place *>> m;
            for (const Place &p : P) if (kind_group(p.kind) == 0) { int q = town_find_q(p.loc); if (q) m.push_back({ q, &p }); }
            std::sort(m.begin(), m.end(), [](const std::pair<int, const Place *> &a, const std::pair<int, const Place *> &b) {
                return a.first != b.first ? a.first > b.first : a.second->name < b.second->name; });
            const TownFind &tf = TFIND[tfind];
            bool seen_only = (tf.icon == TI_ALCHEMIST || tf.icon == TI_PHYSICIAN) && !config_get_int("map.offers_unvisited", 0);
            ImGui::TextDisabled("%d town%s%s", (int)m.size(), m.size() == 1 ? "" : "s", seen_only ? " the party has been to:" : ":");
            float wmax = rw, linew = 0, sp = ImGui::GetStyle().ItemSpacing.x;
            int listed = 0;
            for (auto &e : m) {
                if (++listed > 18) { ImGui::TextDisabled("... and %d more (circled on the map)", (int)m.size() - 18); break; }
                char lb[64]; if (TFIND[tfind].shop >= 0) snprintf(lb, sizeof lb, "%s %dq##tl%d", e.second->name.c_str(), e.first, e.second->loc);
                else snprintf(lb, sizeof lb, "%s##tl%d", e.second->name.c_str(), e.second->loc);
                float w = ImGui::CalcTextSize(lb, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2;
                if (linew > 0 && linew + sp + w <= wmax) { ImGui::SameLine(); linew += sp + w; }   /* (a row of buttons, wrapped) */
                else linew = w;
                if (ImGui::SmallButton(lb)) { tile_px(e.second->x, e.second->y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = e.second->loc; }
            }
        }
    }
    if (fold_head("Map Legend", FOLD_LEGEND)) {           /* the legend */
        if (ui_font_small) ImGui::PushFont(ui_font_small);       /* (smaller than the panel's text: the map is the point) */
        ImGui::TextDisabled("Click a kind to show or hide it.");
        for (int k = 0; k < IK_N; k++)
            if (grid_entry(k, show_kind[k], false, IK_LABEL[k], [&](ImDrawList *pl, ImVec2 c, float is) {
                    if (show_icons) draw_icon(pl, c, is, k);
                    else { static const int G[IK_N] = { 0, 1, 2, 3, 3, 4, 4, 3, 3, 4, 1, 4 }; pl->AddCircleFilled(c, is * 0.25f, kind_colour(G[k])); } }))
                show_kind[k] = !show_kind[k];
        ImGui::Spacing();
        /* what the map shows: two to a row, as the entries above */
        if (ui_font_small) ImGui::PopFont();
    }
    ImGui::End();
    {   /* the lower right: back to the party (a click centres the map on it) */
        int px = -1, py = -1; party_tile(&px, &py);
        if (px >= 0) {
            const char *lb = "Snap to party";
            float bw = ImGui::CalcTextSize(lb).x * pk + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().WindowPadding.x * 2;
            float fw = foot_h > 0 ? fminf(sz.x - 2 * pad, UIS(820) * pk) : 0;
            bool under = foot_h > 0 && (sz.x + fw) * 0.5f > sz.x - pad - bw - pad;   /* (the place chosen at the foot reaches this far: above it) */
            float by = sz.y - pad - (under ? foot_h + pad : 0), bh = ImGui::GetFrameHeight() * pk + ImGui::GetStyle().WindowPadding.y * 2;
            float bx = sz.x - pad;
            if (under && by - bh < wm_layers_bottom + pad) bx = wm_layers_left - pad;   /* (no room under the switches: beside them, above the foot) */
            ImGui::SetNextWindowPos(ImVec2(o.x + bx, o.y + by), ImGuiCond_Always, ImVec2(1, 1));
            ImGui::SetNextWindowBgAlpha(0.97f);
            ImGui::Begin("##wmsnap", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
            ImGui::SetWindowFontScale(pk);
            if (ImGui::Button(lb)) { tile_px(px, py, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Center the map on the party.");
            ImGui::End();
        }
    }
    {   /* what the map shows: five switches on the map, under the top right panel, each a button with
           its name beside it ("Show on the map" over them); a struck-out button is hidden */
        static bool loaded;
        if (!loaded) { loaded = true; for (int k = 0; k < NLAYER; k++) { char key[32]; snprintf(key, sizeof key, "map.show_%d", k); *LAYER[k] = config_get_int(key, LAYER_DEF[k]) != 0; } }
        float bs = fmaxf(34.0f, ui_text_px() * 1.6f), gap = bs * 0.18f;
        ImFont *lf = ui_font_bold ? ui_font_bold : ImGui::GetFont(); float lfs = fmaxf(15.0f, ui_text_px());
        float lw = 0; for (int k = 0; k < NLAYER; k++) lw = fmaxf(lw, lf->CalcTextSizeA(lfs, FLT_MAX, 0, LAYER_NAME[k]).x);
        const char *head = "Show on the map";
        float hh = lfs * 1.3f, hw = lf->CalcTextSizeA(lfs, FLT_MAX, 0, head).x;
        float x0 = o.x + sz.x - pad - bs, y0 = o.y + wm_find_bottom + pad + hh;
        float wx0 = x0 - fmaxf(lw + gap * 2, hw - bs) - gap;
        ImGui::SetNextWindowPos(ImVec2(wx0 - gap, y0 - hh - gap));
        ImGui::SetNextWindowSize(ImVec2(x0 + bs + gap - wx0 + gap, NLAYER * bs + (NLAYER + 1) * gap + hh));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##wmlayers", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::PopStyleVar();
        ImDrawList *bl = ImGui::GetWindowDrawList();
        wm_layers_bottom = y0 + NLAYER * (bs + gap) - o.y; wm_layers_left = wx0 - gap * 0.5f - o.x;   /* (the button back to the party keeps clear of it) */
        bl->AddRectFilled(ImVec2(wx0 - gap * 0.5f, y0 - hh - gap * 0.5f), ImVec2(x0 + bs + gap * 0.5f, y0 + NLAYER * (bs + gap)), IM_COL32(244, 234, 210, 215), bs * 0.2f);
        bl->AddRect(ImVec2(wx0 - gap * 0.5f, y0 - hh - gap * 0.5f), ImVec2(x0 + bs + gap * 0.5f, y0 + NLAYER * (bs + gap)), IM_COL32(150, 96, 44, 200), bs * 0.2f, 0, 1.2f);
        bl->AddText(lf, lfs, ImVec2(x0 + bs - hw, y0 - hh), IM_COL32(110, 60, 24, 255), head);
        for (int k = 0; k < NLAYER; k++) {
            ImVec2 a(x0, y0 + k * (bs + gap)), b(a.x + bs, a.y + bs);
            ImGui::SetCursorScreenPos(ImVec2(wx0, a.y));
            ImGui::PushID(400 + k);
            if (ImGui::InvisibleButton("##lay", ImVec2(b.x - wx0, bs))) {    /* (the name is part of the button) */
                *LAYER[k] = !*LAYER[k];
                char key[32]; snprintf(key, sizeof key, "map.show_%d", k); config_set_int(key, *LAYER[k]); extern int ui_dirty_config; ui_dirty_config = 1;   /* (kept) */
            }
            bool hv = ImGui::IsItemHovered(), on = *LAYER[k];
            if (hv) ImGui::SetTooltip("%s: %s", LAYER_TIP[k], on ? "shown (click to hide)" : "hidden (click to show)");
            ImGui::PopID();
            ImVec2 ls = lf->CalcTextSizeA(lfs, FLT_MAX, 0, LAYER_NAME[k]);
            bl->AddText(lf, lfs, ImVec2(x0 - gap - ls.x, (a.y + b.y - ls.y) * 0.5f), on ? IM_COL32(44, 26, 10, 255) : IM_COL32(130, 112, 92, 255), LAYER_NAME[k]);
            bl->AddRectFilled(ImVec2(a.x + 1.5f, a.y + 2.0f), ImVec2(b.x + 1.5f, b.y + 2.0f), IM_COL32(0, 0, 0, 70), bs * 0.18f);
            bl->AddRectFilled(a, b, on ? IM_COL32(244, 234, 210, 250) : IM_COL32(200, 188, 164, 220), bs * 0.18f);
            bl->AddRect(a, b, on ? IM_COL32(150, 96, 44, 255) : IM_COL32(120, 100, 80, 200), bs * 0.18f, 0, on ? 2.0f : 1.2f);
            if (hv) bl->AddRect(a, b, IM_COL32(255, 220, 140, 255), bs * 0.18f, 0, 2.0f);
            ImVec2 c((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
            layer_glyph(bl, c, bs * 0.62f, k, IM_COL32(44, 26, 10, 255));
            if (!on) layer_struck(bl, c, bs * 0.62f);
        }
        ImGui::End();
    }
    ImGui::End();
}

/* names on the game's own travel map (map.names): the towns and places in view, under their
 * icons (the original names a place only in the top bar, once the pointer is on it). The view:
 * DS:286a its first column, DS:286c + DS:286e its first row; a tile is 16 x 4 game pixels,
 * odd rows half a tile to the right (the route in ui/travel.cpp is drawn the same way). */
extern "C" int worldmap_docks_inset(float x0, float y0, float x1, float y1, int here, const int *dest, const int *opt,
                                    const int *state, const char *const *fare, int n, int hover_opt)
{
    if (n <= 0 || x1 - x0 < 80 || y1 - y0 < 60) return -1;
    static int built_frame = -1000;                       /* (built once as the card shows, not each frame) */
    if (ImGui::GetFrameCount() - built_frame > 1 && !worldmap_build()) return -1;
    built_frame = ImGui::GetFrameCount();
    upload();
    auto loc_px = [](int loc, float *x, float *y) {
        u32 r = g_location(loc); if (!r) return false;
        tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), x, y); return true; };
    float hx, hy; if (!loc_px(here, &hx, &hy)) return -1;
    float bx0 = hx, by0 = hy, bx1 = hx, by1 = hy;
    for (int i = 0; i < n; i++) {
        const short *xs, *ys; int m = travel_boat_way(here, dest[i], &xs, &ys);
        for (int k = 0; k < m; k++) { float x, y; tile_px(xs[k], ys[k], &x, &y); bx0 = fminf(bx0, x); bx1 = fmaxf(bx1, x); by0 = fminf(by0, y); by1 = fmaxf(by1, y); }
        float x, y; if (loc_px(dest[i], &x, &y)) { bx0 = fminf(bx0, x); bx1 = fmaxf(bx1, x); by0 = fminf(by0, y); by1 = fmaxf(by1, y); }
    }
    float ay = aspect();
    ImFont *ft = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float fsz = fmaxf(22.0f, ui_text_px()), title = fsz * 1.5f, pad = fsz * 0.5f;
    const float tk = ui_face_k(ft);   /* (the face's letters to the size asked) */
    float mx0 = x0 + pad, my0 = y0 + title, mx1 = x1 - pad, my1 = y1 - pad;
    float m = 60;                                                 /* (room round the towns for their names) */
    bx0 -= m * 1.6f; bx1 += m * 1.6f; by0 -= m; by1 += m * 2.2f;      /* (more below: the names hang under the towns) */
    float z = fminf((mx1 - mx0) / (bx1 - bx0), (my1 - my0) / ((by1 - by0) * ay));
    if (z > 1.0f) z = 1.0f;
    float vcx = (bx0 + bx1) * 0.5f, vcy = (by0 + by1) * 0.5f;
    float hw = (mx1 - mx0) * 0.5f / z, hh = (my1 - my0) * 0.5f / (z * ay);  /* the view fills the box (and stays on the map) */
    vcx = fminf(fmaxf(vcx, hw), worldmap.full_w - hw); vcy = fminf(fmaxf(vcy, hh), worldmap.full_h - hh);
    ImVec2 mid((mx0 + mx1) * 0.5f, (my0 + my1) * 0.5f);
    auto to_scr = [&](float x, float y) { return ImVec2(mid.x + (x - vcx) * z, mid.y + (y - vcy) * z * ay); };

    ImGui::SetNextWindowPos(ImVec2(x0, y0));
    ImGui::SetNextWindowSize(ImVec2(x1 - x0, y1 - y0));
    ImGui::SetNextWindowBgAlpha(0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("##docksmap", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar(2);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float rr = fsz * 0.35f;
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(240, 226, 192, 232), rr);
    dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(120, 80, 36, 140), rr, 0, 1.5f);
    if (orn_on()) orn_gilt_frame(dl, ImVec2(x0, y0), ImVec2(x1, y1), fsz * 0.8f);
    std::string hn = here < (int)gamedata.cities.size() ? gamedata.cities[here].name : std::string("here");
    std::string tt = "Boats from " + hn;
    dl->AddText(ft, fsz * 1.05f * tk, ImVec2(mx0, y0 + (title - fsz * 1.05f * tk) * 0.5f), IM_COL32(44, 26, 10, 255), tt.c_str());
    {
        const char *lg = "faded: leaves later this week";
        ImVec2 ls = ft->CalcTextSizeA(fsz * 0.8f * tk, FLT_MAX, 0, lg);
        if (mx1 - ls.x > mx0 + ft->CalcTextSizeA(fsz * 1.05f * tk, FLT_MAX, 0, tt.c_str()).x + fsz)
            dl->AddText(ft, fsz * 0.8f * tk, ImVec2(mx1 - ls.x, y0 + (title - ls.y) * 0.5f), IM_COL32(92, 60, 30, 200), lg);
    }
    dl->PushClipRect(ImVec2(mx0, my0), ImVec2(mx1, my1), true);
    int l = 0;
    while (l < WM_LEVELS - 1 && z * (2 << l) < 0.75f) l++;
    if (tex[l]) {
        ImVec2 a = to_scr(0, 0), b = to_scr((float)worldmap.full_w, (float)worldmap.full_h);
        dl->AddImage((ImTextureID)tex[l], a, b);
    }
    float t = (float)ImGui::GetTime();
    ImGuiIO &io = ImGui::GetIO();
    int over = -1;
    std::vector<ImVec2> dpos(n);
    for (int i = 0; i < n; i++) { float x, y; dpos[i] = loc_px(dest[i], &x, &y) ? to_scr(x, y) : ImVec2(-1e5f, -1e5f); }
    for (int i = 0; i < n; i++) {
        float dx = io.MousePos.x - dpos[i].x, dy = io.MousePos.y - dpos[i].y;
        if (state[i] == 1 && dx * dx + dy * dy < (fsz * 1.6f) * (fsz * 1.6f) && ImGui::IsWindowHovered()) over = i;
    }
    for (int pass = 0; pass < 2; pass++)                          /* the faded ways first, the lit one on top */
        for (int i = 0; i < n; i++) {
            bool lit = over == i || (over < 0 && hover_opt >= 0 && opt[i] == hover_opt);
            if ((pass == 1) != lit) continue;
            const short *xs, *ys; int m2 = travel_boat_way(here, dest[i], &xs, &ys);
            std::vector<ImVec2> pts;
            if (m2 >= 2) for (int k = 0; k < m2; k++) { float x, y; tile_px(xs[k], ys[k], &x, &y); pts.push_back(to_scr(x, y)); }
            else { pts.push_back(to_scr(hx, hy)); pts.push_back(dpos[i]); }
            float lw = lit ? 3.2f : 2.2f;
            int al = state[i] == 1 ? 255 : 110;
            boat_way_draw(dl, pts, lw, lit, al, t);
        }
    auto label = [&](ImVec2 c, const std::string &s, ImU32 col, bool bold) {
        float fs2 = fsz * tk * (bold ? 1.0f : 0.9f);
        ImVec2 ts = ft->CalcTextSizeA(fs2, FLT_MAX, 0, s.c_str());
        ImVec2 p(c.x - ts.x * 0.5f, c.y + fsz * 0.9f);
        dl->AddRectFilled(ImVec2(p.x - 3, p.y - 1), ImVec2(p.x + ts.x + 3, p.y + ts.y), IM_COL32(240, 226, 192, bold ? 235 : 190), 3);
        dl->AddText(ft, fs2, p, col, s.c_str());
    };
    for (int i = 0; i < n; i++) {
        bool lit = over == i || (over < 0 && hover_opt >= 0 && opt[i] == hover_opt);
        draw_icon(dl, dpos[i], fsz * 1.5f, 0);
        if (state[i] != 1) dl->AddCircleFilled(dpos[i], fsz * 0.7f, IM_COL32(236, 226, 204, 150), 20);
        if (lit) dl->AddCircle(dpos[i], fsz * 1.1f, IM_COL32(200, 120, 20, 255), 24, 2.5f);
        std::string nm = dest[i] < (int)gamedata.cities.size() ? gamedata.cities[dest[i]].name : std::string("?");
        if (fare && fare[i] && fare[i][0]) nm += std::string(" \xc2\xb7 ") + fare[i];   /* (and its fare) */
        label(dpos[i], nm, state[i] == 1 ? IM_COL32(44, 26, 10, 255) : IM_COL32(44, 26, 10, 130), lit);
    }
    ImVec2 hp = to_scr(hx, hy);
    draw_icon(dl, hp, fsz * 1.7f, 0);
    dl->AddCircle(hp, fsz * 1.2f, IM_COL32(170, 30, 20, 255), 24, 2.5f);
    label(hp, hn, IM_COL32(120, 20, 10, 255), true);
    dl->PopClipRect();
    int clicked = -1;
    if (over >= 0) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        std::string nm = dest[over] < (int)gamedata.cities.size() ? gamedata.cities[dest[over]].name : std::string("?");
        {   /* what the town offers, as on the world map */
            ui_tip_beside(ImVec2(dpos[over].x - fsz * 1.6f, dpos[over].y - fsz * 1.6f), ImVec2(dpos[over].x + fsz * 1.6f, dpos[over].y + fsz * 1.6f));
            ImGui::BeginTooltip();
            if (ui_font_overlay) ImGui::PushFont(ui_font_overlay);
            ImGui::TextUnformatted(nm.c_str());
            std::vector<Place> P = places();
            for (const Place &pl : P) if (pl.loc == dest[over]) { place_details(pl, ImGui::GetFontSize() * 18); break; }
            ImGui::TextDisabled("Click to take the boat.");
            if (ui_font_overlay) ImGui::PopFont();
            ImGui::EndTooltip();
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) clicked = opt[over];
    }
    ImGui::End();
    return clicked;
}

/* test: every place picture of each set, large (ucmd uiconsheet:) */
extern "C" int ui_map_open(void);   /* ui/ui.cpp */
/* the right-click menu's Quests: the world map with its quests unfolded */
extern "C" void worldmap_open_quests(void)
{
    config_set_int("map.folded", config_get_int("map.folded", FOLD_LEGEND | FOLD_TOWNS | FOLD_BUY) & ~FOLD_TASKS);
    if (!ui_map_open()) ui_toggle_map();
}

