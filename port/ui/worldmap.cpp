// worldmap.cpp - the world map: the whole of the Holy Roman Empire as the travel screen draws
// it (mods/worldmap.c), to pan and zoom, with towns and places named, the party, the route of
// auto-travel and the places the quest log points to. Click a place for its details, double-
// click (or Ctrl+click anywhere) to travel there. M on the travel map or a card opens it; M or Esc closes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <strings.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "imgui.h"
extern "C" int ns_showing(void);
#include "gamedata.h"
#include "teachersui.h"
#include "quests.h"
#include "townicons.h"
#include "libraries.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/worldmap.h"
#include "../mods/travel.h"
int journal_visited(int loc);
/* a soft golden halo behind a quest's picture, breathing slowly, with a few sparks turning about it:
   the quests stand out from the towns' and places' own marks */
static void quest_glow(ImDrawList *dl, ImVec2 c, float is)
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
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
void quest_marks(std::vector<std::pair<int, std::string>> &out);
extern ImFont *ui_font_small, *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */

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
static bool show_villages = true, show_castles = true, show_holy = true, show_other = true, show_names = true, show_quests = true, show_icons = true;

/* "Towns with ...": the towns that have a place most lack (ui/townicons.h TI_UNIVERSITY .. TI_BANK)
   or a notable shop (wares of 30q and better), marked with its picture on the map and listed */
struct TownFind { const char *label; int icon; int shop; };      /* shop: DARKLAND.CTY's shop index, -1 a place */
static const TownFind TFIND[] = {
    { "a university", TI_UNIVERSITY, -1 }, { "a cathedral", TI_CATHEDRAL, -1 }, { "an alchemist", TI_ALCHEMIST, -1 },
    { "a physician", TI_PHYSICIAN, -1 }, { "docks (boats)", TI_DOCKS, -1 }, { "a bank", TI_BANK, -1 },
    { "a swordsmith of 30q+", TI_SWORD, 2 }, { "an armourer of 30q+", TI_ARMOUR, 3 }, { "a bowyer of 30q+", TI_BOW, 5 },
    { "an artificer of 30q+", TI_TOOLS, 6 } };   /* (a library: nearly every town has one; the character sheet's Saints say where each saint is taught) */
static const int NTFIND = (int)(sizeof TFIND / sizeof TFIND[0]);
static int tfind = -1;                                    /* -1: off */
static int town_find_q(int loc)                           /* > 0: town loc matches (for a shop: its quality) */
{
    if (tfind < 0 || loc < 0 || loc >= (int)gamedata.cities.size()) return 0;
    const TownFind &f = TFIND[tfind];
    /* (a university, a cathedral, docks, a bank: known far and wide; the alchemists, a physician and
       how good a shop is, once the party has been there: ui/libraries.h) */
    if ((f.shop >= 0 || f.icon == TI_ALCHEMIST || f.icon == TI_PHYSICIAN) && !place_offers_known(loc)) return 0;
    if (f.shop < 0) return citynav_town_offers(loc, f.icon) ? 1 : 0;
    int q = gamedata.cities[loc].shopq[f.shop] > 0 ? shop_wares_q(gamedata.cities[loc].shopq[f.shop]) : 0;
    return q >= 30 ? q : 0;
}

void worldmap_set_renderer(SDL_Renderer *r) { wm_ren = r; }

struct Place { int loc, kind, x, y; std::string name; };

const char *loc_kind_label(int k);
static const char *kind_label(int k) { return loc_kind_label(k); }
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
enum IconKind { IK_TOWN, IK_VILLAGE, IK_CASTLE, IK_MONASTERY, IK_SHRINE, IK_CAVE, IK_MINE, IK_WATER, IK_TOMB, IK_STONES, IK_RUIN, IK_OTHER, IK_N };
static int icon_kind(int k)
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
static std::string kind_line(const std::vector<Place> &P, const Place &p)
{
    const char *k = kind_label(p.kind);
    if (strcasecmp(p.name.c_str(), k)) return k;
    const Place *best = nullptr; double bd = 1e18;
    for (const Place &t : P)
        if (t.kind == 0) { double d = hypot((double)(t.x - p.x), (t.y - p.y) / 2.0); if (d < bd) { bd = d; best = &t; } }
    return best ? std::string(k) + " near " + best->name : std::string(k);
}
static const char *IK_LABEL[IK_N] = { "Town", "Village", "Castle", "Monastery", "Shrine", "Cave", "Mine", "Spring, lake", "Tomb", "Pagan site", "Ruins", "Other" };
static void draw_icon(ImDrawList *dl, ImVec2 c, float s, int ik)
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
        p.name = loc_display_name(k);
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
    if (minutes < 60) snprintf(b, sizeof b, "%d minute%s", minutes, minutes == 1 ? "" : "s");
    else if (minutes < 24 * 60) snprintf(b, sizeof b, "%d h %02d", minutes / 60, minutes % 60);
    else snprintf(b, sizeof b, "%.1f days", minutes / 1440.0);
    return b;
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
/* Esc on the map: a name being looked for is cleared first (1: by the box itself while typing in
   it), the map closed only then */
extern "C" int worldmap_find_escape(int typing)
{
    if (!wm_find[0]) return 0;
    if (!typing) wm_find[0] = 0;
    return 1;
}

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

    // places
    int hover = -1; float hd = 1e9f;
    /* names with the screen (ui_text_px: display.text_size), a little larger as one zooms in */
    ImFont *fs = ui_font_overlay;
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
        bool unseen = g == 0 && p.loc < 92 && !journal_visited(p.loc);   /* a town the party has not been to: faded, a ring */
        if (show_icons) {                                    /* an icon for its kind, bigger as one zooms in */
            float is = r * 4.4f * fminf(fmaxf(powf(zoom / fitz, 0.25f), 1.0f), 1.6f);
            draw_icon(dl, s, is, icon_kind(p.kind));
            r = is * 0.5f;
            if (unseen) dl->AddCircleFilled(s, r * 0.9f, IM_COL32(236, 226, 204, 150), 20);   /* (washed out, as not yet seen) */
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
        if (hovered && d < hr * hr && d < hd) { hd = d; hover = p.loc; }
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
    }
    // quest marks
    if (show_quests)                                         /* the quest's picture on a pin above the place (several: side by side) */
        for (size_t i = 0; i < marks.size(); i++) {
            auto &m = marks[i];
            u32 r = g_location(m.first);
            if (!r) continue;
            int nth = 0, n = 0;
            for (size_t j = 0; j < marks.size(); j++) if (marks[j].first == m.first) { if (j < i) nth++; n++; }
            float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y);
            float k = fmaxf(ks, 1.0f), is = 22 * k;
            ImVec2 s = to_scr(x, y); s.y -= 10 * k;
            ImVec2 c(s.x + (nth - (n - 1) * 0.5f) * is * 1.05f, s.y - is * 0.55f);
            dl->AddTriangleFilled(ImVec2(c.x - 4 * k, c.y + is * 0.3f), ImVec2(c.x + 4 * k, c.y + is * 0.3f), ImVec2(s.x, s.y + 2 * k), IM_COL32(20, 14, 8, 230));
            quest_glow(dl, c, is);
            ui_draw_quest_icon(dl, c, is, i < mark_cats.size() ? mark_cats[i] : 0);
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
            ImGui::TextDisabled("%s", kind_line(P, p).c_str());
            if (p.loc < 92) ui_town_offers(p.loc, true, ImGui::GetFontSize() * 16, true);
            bool known_here = place_offers_known(p.loc);           /* (ui/libraries.h: been there, or the switch) */
            if (!known_here && p.loc < 92)
                ImGui::TextDisabled("Its saints, formulae and teachers: known once the party has been there.");
            else if (!known_here && library_at(p.loc))
                ImGui::TextDisabled("The saints its library teaches: known once the party has been there.");
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
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
            }
            {   int fo[4], pr[4];                                /* the formulae its alchemist sells */
                if (known_here && p.loc < 92 && alchemist_formulae(p.loc, fo, pr)) {
                    std::string t;
                    for (int k = 0; k < 4; k++) if (fo[k] >= 0 && fo[k] < (int)gamedata.formula_names.size()) t += (t.empty() ? "" : ";  ") + gamedata.formula_names[fo[k]] + (party_knows_formula(fo[k]) ? "" : " (new)");
                    ImGui::TextDisabled("Formulae sold here:");
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
                }
            }
            if (known_here && p.loc < 92 && ui_teachers_on()) {      /* the teachers it can offer (mods/teachers.c) */
                TchSource ts[12]; int n = ui_town_teachers(p.loc, ts, 12);
                std::string t;
                for (int k = 0; k < n; k++) t += (t.empty() ? "" : ";  ") + std::string(ui_teacher_short(ts[k].src)) + ": " + ui_teacher_skills(ts[k]);
                if (n) {
                    ImGui::TextDisabled("Teachers to be hired:");
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextWrapped("%s", t.c_str()); ImGui::PopTextWrapPos();
                }
            }
            if (p.loc < 92) ImGui::TextDisabled(journal_visited(p.loc) ? "In the town journal" : "Not visited yet");
            for (auto &m : marks) if (m.first == p.loc) { ImGui::PushTextWrapPos(UIS(380) * pk); ImGui::TextWrapped("%s", m.second.c_str()); ImGui::PopTextWrapPos(); }
            for (const TownMark &t : tmarks) if (t.loc == p.loc) ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.15f, 1), "%s %s", t.cat == QC_UNWELCOME ? "Unwelcome here" : "Wanted by the watch here", t.until.c_str());
            if (on_map) ImGui::TextDisabled("Double-click to travel there");
            ImGui::EndTooltip();
            break;
        }
    }

    // panel: search, layers, the selected place
    ImGui::SetNextWindowPos(ImVec2(o.x + 14, o.y + 14));
    ImGui::SetNextWindowBgAlpha(0.97f);                    /* (the map must not show through the words) */
    ImGui::Begin("##wmpanel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowFontScale(pk);
    ImGui::TextUnformatted("The Holy Roman Empire");
    ImGui::TextDisabled("Drag to move, wheel to zoom. M or Esc closes.");
    char *find = wm_find;
    ImGui::SetNextItemWidth(UIS(260) * pk);
    bool enter = ImGui::InputTextWithHint("##find", "Find a place...", find, sizeof wm_find, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll);
    if (find[0]) {                                         /* the places named so, the nearest first on the travel map, with the journey */
        std::string f = fold_name(find, false);
        std::vector<std::pair<int, const Place *>> hits;      /* (minutes on the way, -1 unknown) */
        for (const Place &p : P)
            if (fold_name(p.name, false).find(f) != std::string::npos || fold_name(p.name, true).find(f) != std::string::npos)
                hits.push_back({ on_map ? travel_minutes_loc(p.loc, nullptr, nullptr) : -1, &p });
        std::stable_sort(hits.begin(), hits.end(), [](const std::pair<int, const Place *> &a, const std::pair<int, const Place *> &b) {
            return (a.first < 0 ? INT_MAX : a.first) < (b.first < 0 ? INT_MAX : b.first); });
        /* a row a place: its picture, the name, what it is (dim, cut short when the row is full) and
           the journey at the right; the whole row to click */
        float rw = UIS(260) * pk, lh = ImGui::GetTextLineHeight(), isz = lh * 0.95f, gap = ImGui::GetStyle().ItemSpacing.x;
        ImDrawList *pdl = ImGui::GetWindowDrawList();
        for (size_t i = 0; i < hits.size() && i < 8; i++) {
            const Place &p = *hits[i].second;
            ImGui::PushID(p.loc);
            ImVec2 a = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##hit", false, 0, ImVec2(rw, 0)) || (enter && i == 0)) {   /* (Enter: the first) */ tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = p.loc; find[0] = 0; }
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
                pdl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(kx, a.y), dimc, k.c_str(), nullptr, 0, &clip);
            }
            if (!t.empty()) pdl->AddText(ImVec2(tx, a.y), dimc, t.c_str());
        }
        if (hits.size() > 8) ImGui::TextDisabled("and %d more: a longer name narrows it", (int)hits.size() - 8);
        if (hits.empty()) ImGui::TextDisabled("No place of that name.");
    }
    if (travel.active) {                                   /* the journey under way */
        ImGui::TextColored(ImVec4(0.55f, 0.24f, 0.12f, 1), "Travelling to %s", travel.name);
        ImGui::SameLine();
        if (ImGui::SmallButton("Stop")) travel_stop();
    }
    if (ImGui::SmallButton("The party")) { int px, py; party_tile(&px, &py); if (px >= 0) tile_px(px, py, &cx, &cy); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Whole map")) { zoom = fitz; cx = worldmap.full_w * 0.5f; cy = worldmap.full_h * 0.5f; }
    {   /* what the map shows: which places (the towns always), and how */
        float lw = fmaxf(ImGui::CalcTextSize("Places").x, ImGui::CalcTextSize("Marks").x) + ImGui::GetFontSize() * 1.4f;
        auto label = [&](const char *t) { ImGui::AlignTextToFramePadding(); ImGui::TextDisabled("%s", t); ImGui::SameLine(lw); };
        label("Places"); ImGui::Checkbox("Villages", &show_villages); ImGui::SameLine(); ImGui::Checkbox("Castles", &show_castles);
        ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine(lw); ImGui::Checkbox("Holy places", &show_holy); ImGui::SameLine(); ImGui::Checkbox("Other", &show_other);
        label("Marks"); ImGui::Checkbox("Names", &show_names); ImGui::SameLine(); ImGui::Checkbox("Quests", &show_quests); ImGui::SameLine(); ImGui::Checkbox("Pictures", &show_icons);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Each place as a little picture of its kind (else a coloured dot)");
        label("Towns");
        ImGui::SetNextItemWidth(UIS(230) * pk);
        if (ImGui::BeginCombo("##tfind", tfind < 0 ? "with ..." : (std::string("with ") + TFIND[tfind].label).c_str())) {
            if (ImGui::Selectable("(none marked)", tfind < 0)) tfind = -1;
            for (int k = 0; k < NTFIND; k++) {
                ImVec2 a = ImGui::GetCursorScreenPos(); float isz = ImGui::GetTextLineHeight();
                int nsp = (int)ceilf(isz * 1.45f / fmaxf(1.0f, ImGui::CalcTextSize(" ").x));      /* (room for the picture) */
                char lb[96]; snprintf(lb, sizeof lb, "%*swith %s##tf%d", nsp, "", TFIND[k].label, k);
                if (ImGui::Selectable(lb, tfind == k)) tfind = k;
                ui_draw_town_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + isz * 0.6f, a.y + isz * 0.5f), isz * 1.1f, TFIND[k].icon);
            }
            ImGui::EndCombo();
        }
        if (tfind >= 0) {                                   /* the towns, by name (a shop's: the best first), a click shows one */
            std::vector<std::pair<int, const Place *>> m;
            for (const Place &p : P) if (kind_group(p.kind) == 0) { int q = town_find_q(p.loc); if (q) m.push_back({ q, &p }); }
            std::sort(m.begin(), m.end(), [](const std::pair<int, const Place *> &a, const std::pair<int, const Place *> &b) {
                return a.first != b.first ? a.first > b.first : a.second->name < b.second->name; });
            const TownFind &tf = TFIND[tfind];
            bool seen_only = (tf.shop >= 0 || tf.icon == TI_ALCHEMIST || tf.icon == TI_PHYSICIAN) && !config_get_int("map.offers_unvisited", 0);
            ImGui::TextDisabled("%d town%s%s", (int)m.size(), m.size() == 1 ? "" : "s", seen_only ? " the party has been to:" : ":");
            float wmax = UIS(380) * pk, linew = 0, sp = ImGui::GetStyle().ItemSpacing.x;
            int listed = 0;
            for (auto &e : m) {
                if (++listed > 18) { ImGui::TextDisabled("... and %d more (ringed on the map)", (int)m.size() - 18); break; }
                char lb[64]; if (TFIND[tfind].shop >= 0) snprintf(lb, sizeof lb, "%s %dq##tl%d", e.second->name.c_str(), e.first, e.second->loc);
                else snprintf(lb, sizeof lb, "%s##tl%d", e.second->name.c_str(), e.second->loc);
                float w = ImGui::CalcTextSize(lb, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2;
                if (linew > 0 && linew + sp + w <= wmax) { ImGui::SameLine(); linew += sp + w; }   /* (a row of buttons, wrapped) */
                else linew = w;
                if (ImGui::SmallButton(lb)) { tile_px(e.second->x, e.second->y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; selected = e.second->loc; }
            }
        }
    }
    {   /* the party's tasks: a click shows the place (and chooses it, for Travel there) */
        static std::vector<QuestRow> qr; static int qf = -1000;
        if (ImGui::GetFrameCount() - qf > 30) { quest_rows(qr, config_get_int("quests.sort", QSORT_DEADLINE) % QSORT_N, true); qf = ImGui::GetFrameCount(); }
        for (int part = 0; part < 2; part++) {                  /* the tasks; the rumours heard (marked on the map too) */
        bool any = false;
        auto in_part = [&](const QuestRow &r) { return r.loc >= 0 && (part ? r.group == 2 || r.group == 4 : r.group <= 1); };
        for (const QuestRow &r : qr) if (in_part(r)) { any = true; break; }
        if (any && ImGui::TreeNodeEx(part ? "Rumours" : "Tasks", part ? 0 : ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const QuestRow &r : qr) {
                if (!in_part(r)) continue;
                std::string d = quest_days(r);
                float isz = ImGui::GetTextLineHeight() * 1.15f, sp = ImGui::CalcTextSize(" ").x;
                std::string pad((size_t)ceilf((isz + sp) / sp), ' ');      /* (room for the picture) */
                char lb[192]; snprintf(lb, sizeof lb, "%s%s%s%s##q%s", pad.c_str(), r.title.c_str(), d.empty() ? "" : "   ", d.c_str(), r.id.c_str());
                bool sel = ImGui::Selectable(lb, selected == r.loc);
                { ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); ui_draw_quest_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + isz * 0.5f, (a.y + b.y) * 0.5f), isz, r.cat); }
                if (sel) {
                    for (const Place &p : P) if (p.loc == r.loc) { tile_px(p.x, p.y, &cx, &cy); if (zoom < 0.6f) zoom = 0.6f; }
                    selected = r.loc;
                }
                if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextUnformatted(r.text.c_str()); if (!r.when.empty()) ImGui::TextDisabled("%s", r.when.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
            }
            ImGui::TreePop();
        }
        }
    }
    if (ImGui::TreeNode("Legend")) {   /* what the marks mean (closed at first: the map is the point) */
        float is = ImGui::GetFontSize() * 1.1f, colw = ImGui::GetFontSize() * 8.5f;
        ImDrawList *pl = ImGui::GetWindowDrawList();
        for (int k = 0; k < IK_N; k++) {
            if (k % 2) ImGui::SameLine(colw);
            ImVec2 q = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(is * 1.2f, is));
            if (show_icons) draw_icon(pl, ImVec2(q.x + is * 0.6f, q.y + is * 0.5f), is, k);
            else {
                static const int G[IK_N] = { 0, 1, 2, 3, 3, 4, 4, 3, 3, 4, 1, 4 };
                pl->AddCircleFilled(ImVec2(q.x + is * 0.6f, q.y + is * 0.5f), is * 0.25f, kind_colour(G[k]));
            }
            ImGui::SameLine(); ImGui::TextUnformatted(IK_LABEL[k]);
        }
        {   ImVec2 q = ImGui::GetCursorScreenPos();                  /* quest marks and the party */
            ImGui::Dummy(ImVec2(is * 1.2f, is));
            ImVec2 m(q.x + is * 0.6f, q.y + is * 0.5f);
            ui_draw_quest_icon(pl, m, is * 1.1f, QC_ROBBER);
            ImGui::SameLine(); ImGui::TextUnformatted("A quest (its kind)");
            ImGui::SameLine(colw);
            q = ImGui::GetCursorScreenPos(); ImGui::Dummy(ImVec2(is * 1.2f, is)); m = ImVec2(q.x + is * 0.6f, q.y + is * 0.5f);
            pl->AddCircle(m, is * 0.4f, IM_COL32(255, 255, 255, 255), 0, 2); pl->AddCircleFilled(m, is * 0.15f, IM_COL32(255, 60, 40, 255));
            ImGui::SameLine(); ImGui::TextUnformatted("The party");
            q = ImGui::GetCursorScreenPos(); ImGui::Dummy(ImVec2(is * 1.2f, is)); m = ImVec2(q.x + is * 0.6f, q.y + is * 0.5f);   /* a town not yet visited */
            if (show_icons) { draw_icon(pl, m, is, 0); pl->AddCircleFilled(m, is * 0.45f, IM_COL32(236, 226, 204, 150), 20); }
            else { pl->AddCircle(m, is * 0.25f, IM_COL32(0, 0, 0, 200), 0, 3.0f); pl->AddCircle(m, is * 0.25f, kind_colour(0), 0, 1.6f); }
            ImGui::SameLine(); ImGui::TextUnformatted("A town not visited yet (washed out)");
        }
        if (ImGui::TreeNode("The quests' pictures")) {
            for (int k = 1; k < QC_N; k++) {
                if ((k - 1) % 2) ImGui::SameLine(colw);
                ImVec2 q = ImGui::GetCursorScreenPos();
                ImGui::Dummy(ImVec2(is * 1.3f, is * 1.1f));
                ui_draw_quest_icon(pl, ImVec2(q.x + is * 0.6f, q.y + is * 0.55f), is * 1.1f, k);
                ImGui::SameLine(); ImGui::TextUnformatted(QUEST_CAT_NAME[k]);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    if (selected >= 0) {
        for (const Place &p : P) if (p.loc == selected) {
            ImGui::Separator();
            ImGui::TextUnformatted(p.name.c_str());
            ImGui::TextDisabled("%s", kind_line(P, p).c_str());
            if (on_map) {
                int gx, gy, m = travel_minutes_loc(p.loc, &gx, &gy);
                if (m >= 0) ImGui::Text("About %s on the way", hhmm(m).c_str());
                if (ImGui::Button("Travel there")) { if (travel_to_loc(p.loc, p.name.c_str())) { route_note(); ui_toggle_map(); } }
            } else ImGui::TextDisabled("Auto-travel works from the travel map.");
            for (auto &m : marks) if (m.first == p.loc) { ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 320 * pk); ImGui::TextWrapped("%s", m.second.c_str()); ImGui::PopTextWrapPos(); }
            for (const TownMark &t : tmarks) if (t.loc == p.loc) { ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 320 * pk); ImGui::TextWrapped("%s %s.", t.cat == QC_UNWELCOME ? "Unwelcome here" : "Wanted by the watch here", t.until.c_str()); ImGui::PopTextWrapPos(); }
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
                ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextDisabled("%s", loc_kind_label(kind));
                if (tip_min > 0) ImGui::Text("About %s on the way", hhmm(tip_min).c_str());
                if (config_get_int("travel.auto", 1)) ImGui::TextDisabled("Ctrl + click: travel there along the roads");
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
        {                                                     /* a quest that leads here: its picture above the place */
            int nq = 0, iq = 0;
            for (size_t i = 0; i < qmarks.size(); i++) if (qmarks[i].first == k) nq++;
            for (size_t i = 0; i < qmarks.size(); i++)
                if (qmarks[i].first == k) {
                    float is = base * 1.3f;
                    ImVec2 top; plat_game_to_win(sx, sy - 6, &top.x, &top.y);
                    ImVec2 c(top.x + (iq - (nq - 1) * 0.5f) * is * 1.05f, top.y - is * 0.55f);
                    quest_glow(dl, c, is);
                    ui_draw_quest_icon(dl, c, is, i < qcats.size() ? qcats[i] : 0);
                    ImVec2 m = ImGui::GetIO().MousePos;             /* (the pointer on it: the task) */
                    if (fabsf(m.x - c.x) < is * 0.5f && fabsf(m.y - c.y) < is * 0.5f && !ImGui::GetIO().WantCaptureMouse) {
                        ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22); ImGui::TextUnformatted(qmarks[i].second.c_str()); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
                    }
                    iq++;
                }
        }
    }
}
