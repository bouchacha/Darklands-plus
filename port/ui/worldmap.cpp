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
#include "padglyphs.h"
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
static int wm_panel_run, wm_panel_frame = -10;    // frames the panel has been drawn in a row (its size settles after the first), the last
static int selected = -1;
static bool show_names = true, show_quests = true, show_icons = true, show_boats = true;
static bool show_kind[16] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };   /* by icon kind (the legend's entries switch them) */

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
    /* (a university, a cathedral, docks, a bank, and how good a town's smiths and the like are: known
       far and wide, as "everyone knows the swordsmiths of Nuremberg"; the alchemists and a physician,
       once the party has been there: ui/libraries.h) */
    if ((f.icon == TI_ALCHEMIST || f.icon == TI_PHYSICIAN) && !place_offers_known(loc)) return 0;
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
static const char *IK_LABEL[IK_N] = { "Town", "Village", "Castle", "Monastery", "Shrine", "Cave", "Mine", "Spring or lake", "Tomb", "Pagan site", "Ruins", "Other" };
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

static std::string hhmm(int minutes)      /* a time on the way, as an estimate reads: "40 minutes", "22 hours", "1.4 days", "14 days" */
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
static int wm_test_enter;                                   /* (test command ufind: the text, then Enter) */
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
    if (p.loc < 92) ImGui::TextDisabled(journal_visited(p.loc) ? "In the town journal" : "Not visited yet");
}


/* the frame round the map, after the printed map that came with the game: a checkered green and
   white rule, then a band of coloured fields lettered A-H along the top and the foot and numbered
   1-8 down the sides, a thin dark line each side; a compass rose at the lower left */
static float map_border_px(void) { return fmaxf(26.0f, ui_text_px() * 1.25f); }
static void compass_rose(ImDrawList *dl, ImVec2 c, float r)
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
static void draw_map_frame(ImDrawList *dl, ImVec2 a, ImVec2 b)
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
        float lo = hw - L, hi = worldmap.full_w - hw;
        cx = lo > hi ? (lo + hi) * 0.5f : fminf(fmaxf(cx, lo), hi);
        lo = hh; hi = worldmap.full_h - hh;
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
    draw_map_frame(dl, to_scr(0, 0), to_scr((float)worldmap.full_w, (float)worldmap.full_h));

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
                bool lit = selected == a || selected == b;
                const short *xs, *ys; int n = travel_boat_way(a, b, &xs, &ys);
                std::vector<ImVec2> pts;
                if (n >= 2) for (int i = 0; i < n; i += (n > 60 ? 2 : 1)) { float x, y; tile_px(xs[i], ys[i], &x, &y); pts.push_back(to_scr(x, y)); }
                if (n >= 2) { float x, y; tile_px(xs[n - 1], ys[n - 1], &x, &y); pts.push_back(to_scr(x, y)); }
                else for (int e = 0; e < 2; e++) { u32 r = g_location(e ? b : a); if (!r) continue; float x, y; tile_px((s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), &x, &y); pts.push_back(to_scr(x, y)); }
                if (pts.size() < 2) continue;
                int al = selected >= 0 && !lit ? 90 : 255;
                dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(60, 150, 255, lit ? 110 : al / 4), 0, lw * 3.2f);   /* the glow */
                /* the dashes, moving with the boats */
                float dash = 9 * lw, gap = 6 * lw, ph = fmodf(t * 18 * lw, dash + gap), acc = -ph;
                for (size_t i = 1; i < pts.size(); i++) {
                    ImVec2 p0 = pts[i - 1], p1 = pts[i];
                    float L = sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
                    if (L <= 0) continue;
                    float s0 = 0;
                    while (s0 < L) {
                        float cyc = fmodf(acc + s0 + 10000 * (dash + gap), dash + gap);
                        float run = cyc < dash ? fminf(dash - cyc, L - s0) : fminf(dash + gap - cyc, L - s0);
                        if (cyc < dash) {
                            ImVec2 q0(p0.x + (p1.x - p0.x) * s0 / L, p0.y + (p1.y - p0.y) * s0 / L), q1(p0.x + (p1.x - p0.x) * (s0 + run) / L, p0.y + (p1.y - p0.y) * (s0 + run) / L);
                            dl->AddLine(q0, q1, lit ? IM_COL32(200, 235, 255, 255) : IM_COL32(150, 205, 255, al), lw);
                        }
                        s0 += fmaxf(run, 0.5f);
                    }
                    acc += L;
                }
            }
    }

    // places
    int hover = -1; float hd = 1e9f;
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
        float fw = sz.x - 2 * pad, fh = fminf(sz.y * 0.34f, UIS(300) * pk);
        ImGui::SetNextWindowPos(ImVec2(o.x + pad, o.y + sz.y - pad), ImGuiCond_Always, ImVec2(0, 1));
        ImGui::SetNextWindowSize(ImVec2(fw, fh));
        ImGui::SetNextWindowBgAlpha(0.97f);
        ImGui::Begin("##wmplace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(pk);
        foot_h = fh;
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
            const char *t = "Travel from the travel map, or from a town's streets";
            ImGui::SameLine(bx - ImGui::CalcTextSize(t).x); ImGui::TextDisabled("%s", t);
        }
        if (ImGui::BeginPopupModal("##leavetravel", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            int cur = (s16)g_rw(G_CUR_LOC);
            std::string town = cur >= 0 && cur < (int)gamedata.cities.size() ? gamedata.cities[cur].name : std::string("the town");
            ImGui::Text("Walk out of %s's gate and travel to %s?", town.c_str(), p.name.c_str());
            ImGui::TextDisabled("The party walks through the streets to a gate and out; on the travel map the journey begins.");
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
        /* the rest, scrolling under the head */
        ImGui::BeginChild("##wmplacebody", ImVec2(0, 0));
        float colw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        if (ImGui::BeginTable("##wmpt", 2, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();
            place_details(p, colw - 8);
            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(0);
            if (p.loc >= 0 && p.loc < (int)gamedata.cities.size()) {        /* its docks: the towns its boats go to (a click shows one) */
                const City &town = gamedata.cities[p.loc];
                bool any = false;
                for (int k = 0; k < 5; k++) if (town.docks[k] >= 0 && town.docks[k] < (int)gamedata.cities.size()) {
                    if (!any) { ImGui::TextDisabled("Boats from its docks to:"); any = true; } else ImGui::SameLine();
                    const City &d2 = gamedata.cities[town.docks[k]];
                    char lb[64]; snprintf(lb, sizeof lb, "%s##boat%d", d2.name.c_str(), k);
                    if (ImGui::SmallButton(lb)) { selected = town.docks[k]; for (const Place &q : P) if (q.loc == selected) { tile_px(q.x, q.y, &cx, &cy); } }
                }
                if (any) ImGui::TextDisabled("Not every boat sails every day; the docks say which leave today.");
            }
            for (auto &m : marks) if (m.first == p.loc) ImGui::TextWrapped("%s", m.second.c_str());
            for (const TownMark &t : tmarks) if (t.loc == p.loc) ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.15f, 1), "%s %s.", t.cat == QC_UNWELCOME ? "Unwelcome here" : "Wanted by the watch here", t.until.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTable();
        }
        ImGui::EndChild();
        ImGui::End();
    }
    float avail_h = sz.y - 2 * pad - (foot_h > 0 ? foot_h + pad : 0);

    /* ---- the left: what the map is, the view buttons, the journey, the tasks and rumours */
    ImGui::SetNextWindowPos(ImVec2(o.x + pad, o.y + pad));
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(fminf(UIS(470) * pk, sz.x * 0.36f), avail_h));
    ImGui::Begin("##wmpanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    if (ImGui::GetFrameCount() - wm_panel_frame > 1) wm_panel_run = 0;
    wm_panel_run++; wm_panel_frame = ImGui::GetFrameCount();
    wm_panel_right = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - o.x;
    ImGui::SetWindowFontScale(pk);
    ImGui::TextUnformatted("The Holy Roman Empire");
    if (ui_pad_prompts()) ui_pad_text("{LSTICK} moves the pointer, hold {A} to drag, {RSTICK} zooms. {BACK} or {Y} closes the map.", true);
    else ImGui::TextDisabled("Drag to move, wheel to zoom. M or Esc closes.");
    if (travel.active) {                                   /* the journey under way */
        ImGui::TextColored(ImVec4(0.55f, 0.24f, 0.12f, 1), "Travelling to %s", travel.name);
        ImGui::SameLine();
        if (ImGui::SmallButton("Stop")) travel_stop();
    }
    if (ImGui::SmallButton("The party")) { int px, py; party_tile(&px, &py); if (px >= 0) tile_px(px, py, &cx, &cy); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Whole map")) { zoom = fitz; cx = worldmap.full_w * 0.5f; cy = worldmap.full_h * 0.5f; }
    {   /* the party's tasks: a click shows the place (and chooses it, for Travel there) */
        static std::vector<QuestRow> qr; static int qf = -1000;
        if (ImGui::GetFrameCount() - qf > 30) { quest_rows(qr, config_get_int("quests.sort", QSORT_DEADLINE) % QSORT_N, true); qf = ImGui::GetFrameCount(); }
        bool none = true;
        for (int part = 0; part < 2; part++) {                  /* the tasks; the rumours heard (marked on the map too) */
        bool any = false;
        auto in_part = [&](const QuestRow &r) { return r.loc >= 0 && (part ? r.group == 2 || r.group == 4 : r.group <= 1); };
        for (const QuestRow &r : qr) if (in_part(r)) { any = true; break; }
        if (any) none = false;
        if (any && ImGui::TreeNodeEx(part ? "Rumours" : "Tasks", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const QuestRow &r : qr) {
                if (!in_part(r)) continue;
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
            }
            ImGui::TreePop();
        }
        }
        if (none) ImGui::TextDisabled("No tasks or rumours yet.");
    }
    ImGui::End();

    /* ---- the top right: find, the legend (a click on an entry hides or shows those places), marks, towns with ... */
    ImGui::SetNextWindowPos(ImVec2(o.x + sz.x - pad, o.y + pad), ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(fminf(UIS(420) * pk, sz.x * 0.34f), avail_h));
    ImGui::Begin("##wmfind", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowFontScale(pk);
    float rw = UIS(300) * pk;
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
    {   /* the legend: each kind of place, a click shows or hides them */
        float is = ImGui::GetFontSize() * 1.15f, colw = rw * 0.5f;
        ImDrawList *pl = ImGui::GetWindowDrawList();
        ImGui::Spacing();
        ImGui::TextDisabled("Places (click to show or hide)");
        for (int k = 0; k < IK_N; k++) {
            if (k % 2) ImGui::SameLine(colw);
            ImVec2 q = ImGui::GetCursorScreenPos();
            ImGui::PushID(k);
            char lb[48]; snprintf(lb, sizeof lb, "##lg%d", k);
            if (ImGui::InvisibleButton(lb, ImVec2(colw - ImGui::GetStyle().ItemSpacing.x, is))) show_kind[k] = !show_kind[k];
            bool hv = ImGui::IsItemHovered();
            ImGui::PopID();
            if (hv) pl->AddRectFilled(q, ImVec2(q.x + colw - ImGui::GetStyle().ItemSpacing.x, q.y + is), IM_COL32(200, 170, 120, 90), 3);
            ImVec2 c(q.x + is * 0.6f, q.y + is * 0.5f);
            if (show_icons) draw_icon(pl, c, is, k);
            else { static const int G[IK_N] = { 0, 1, 2, 3, 3, 4, 4, 3, 3, 4, 1, 4 }; pl->AddCircleFilled(c, is * 0.25f, kind_colour(G[k])); }
            ImU32 ink = show_kind[k] ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
            pl->AddText(ImVec2(q.x + is * 1.35f, q.y + (is - ImGui::GetFontSize()) * 0.5f), ink, IK_LABEL[k]);
            if (!show_kind[k]) {                                   /* hidden: the picture faded and struck through */
                pl->AddCircleFilled(c, is * 0.55f, IM_COL32(236, 226, 204, 170));
                pl->AddLine(ImVec2(c.x - is * 0.45f, c.y + is * 0.45f), ImVec2(c.x + is * 0.45f, c.y - is * 0.45f), IM_COL32(150, 50, 30, 220), 2);
            }
        }
        {   ImVec2 q = ImGui::GetCursorScreenPos();                  /* quest marks, the party, a town not visited */
            ImGui::Dummy(ImVec2(is * 1.2f, is));
            ImVec2 m(q.x + is * 0.6f, q.y + is * 0.5f);
            ui_draw_quest_icon(pl, m, is * 1.1f, QC_ROBBER);
            ImGui::SameLine(); ImGui::TextUnformatted("A quest");
            ImGui::SameLine(colw);
            q = ImGui::GetCursorScreenPos(); ImGui::Dummy(ImVec2(is * 1.2f, is)); m = ImVec2(q.x + is * 0.6f, q.y + is * 0.5f);
            pl->AddCircle(m, is * 0.4f, IM_COL32(255, 255, 255, 255), 0, 2); pl->AddCircleFilled(m, is * 0.15f, IM_COL32(255, 60, 40, 255));
            ImGui::SameLine(); ImGui::TextUnformatted("The party");
            q = ImGui::GetCursorScreenPos(); ImGui::Dummy(ImVec2(is * 1.2f, is)); m = ImVec2(q.x + is * 0.6f, q.y + is * 0.5f);
            if (show_icons) { draw_icon(pl, m, is, 0); pl->AddCircleFilled(m, is * 0.45f, IM_COL32(236, 226, 204, 150), 20); }
            else { pl->AddCircle(m, is * 0.25f, IM_COL32(0, 0, 0, 200), 0, 3.0f); pl->AddCircle(m, is * 0.25f, kind_colour(0), 0, 1.6f); }
            ImGui::SameLine(); ImGui::TextUnformatted("A town not visited yet");
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
        ImGui::Spacing();
        ImGui::TextDisabled("Show"); ImGui::SameLine(); ImGui::Checkbox("Names", &show_names); ImGui::SameLine(); ImGui::Checkbox("Quests", &show_quests); ImGui::SameLine(); ImGui::Checkbox("Pictures", &show_icons);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Each place as a small picture of its kind. When off: a coloured dot.");
        ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine(ImGui::CalcTextSize("Show").x + ImGui::GetStyle().ItemSpacing.x); ImGui::Checkbox("Boats", &show_boats);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Each place as a small picture of its kind. When off: a coloured dot.");
    }
    {
        ImGui::AlignTextToFramePadding(); ImGui::TextDisabled("Towns"); ImGui::SameLine();
        {   /* (wide enough for the longest choice: "with an artificer of 30q+") */
            float cw = rw - ImGui::CalcTextSize("Towns ").x - ImGui::GetStyle().ItemSpacing.x;
            for (int k = 0; k < NTFIND; k++) cw = fmaxf(cw, ImGui::CalcTextSize((std::string("with ") + TFIND[k].label).c_str()).x + ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2);
            ImGui::SetNextItemWidth(cw);
        }
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
            bool seen_only = (tf.icon == TI_ALCHEMIST || tf.icon == TI_PHYSICIAN) && !config_get_int("map.offers_unvisited", 0);
            ImGui::TextDisabled("%d town%s%s", (int)m.size(), m.size() == 1 ? "" : "s", seen_only ? " the party has been to:" : ":");
            float wmax = rw, linew = 0, sp = ImGui::GetStyle().ItemSpacing.x;
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

/* assist.docks_map: beside the docks' list of boats ($Docks00), a small map of the town and the
   towns its boats go to, the way each boat takes along the rivers and coasts. The boats leaving
   today are bright (a click chooses that boat on the card), the others faded; the option under
   the pointer on the card lights its way. Returns the option clicked on the map, or -1. */
extern "C" int worldmap_docks_inset(float x0, float y0, float x1, float y1, int here, const int *dest, const int *opt,
                                    const int *state, int n, int hover_opt)
{
    if (n <= 0 || x1 - x0 < 80 || y1 - y0 < 60 || !worldmap_build()) return -1;
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
    float fsz = fmaxf(17.0f, ui_text_px() * 0.8f), title = fsz * 1.6f, pad = fsz * 0.6f;
    float mx0 = x0 + pad, my0 = y0 + title, mx1 = x1 - pad, my1 = y1 - pad;
    float m = 60;                                                 /* (room round the towns for their names) */
    bx0 -= m * 1.6f; bx1 += m * 1.6f; by0 -= m; by1 += m;
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
    ImGui::Begin("##docksmap", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar(2);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float rr = fsz * 0.35f;
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(240, 226, 192, 232), rr);
    dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(120, 80, 36, 140), rr, 0, 1.5f);
    std::string hn = here < (int)gamedata.cities.size() ? gamedata.cities[here].name : std::string("here");
    std::string tt = "Boats from " + hn;
    dl->AddText(ft, fsz * 1.05f, ImVec2(mx0, y0 + (title - fsz * 1.05f) * 0.5f), IM_COL32(44, 26, 10, 255), tt.c_str());
    {
        const char *lg = "faded: leaves later this week";
        ImVec2 ls = ft->CalcTextSizeA(fsz * 0.8f, FLT_MAX, 0, lg);
        if (mx1 - ls.x > mx0 + ft->CalcTextSizeA(fsz * 1.05f, FLT_MAX, 0, tt.c_str()).x + fsz)
            dl->AddText(ft, fsz * 0.8f, ImVec2(mx1 - ls.x, y0 + (title - ls.y) * 0.5f), IM_COL32(92, 60, 30, 200), lg);
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
            dl->AddPolyline(pts.data(), (int)pts.size(), IM_COL32(20, 60, 140, lit ? 150 : al / 3), 0, lw * 3.0f);
            float dash = 9 * lw, gap = 6 * lw, acc = -fmodf(t * 18 * lw, dash + gap);
            ImU32 dc = lit ? IM_COL32(255, 236, 150, 255) : IM_COL32(170, 215, 255, al);
            for (size_t k = 1; k < pts.size(); k++) {
                ImVec2 p0 = pts[k - 1], p1 = pts[k];
                float L = sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
                if (L <= 0) continue;
                for (float s0 = 0; s0 < L;) {
                    float cyc = fmodf(acc + s0 + 10000 * (dash + gap), dash + gap);
                    float run = cyc < dash ? fminf(dash - cyc, L - s0) : fminf(dash + gap - cyc, L - s0);
                    if (cyc < dash) dl->AddLine(ImVec2(p0.x + (p1.x - p0.x) * s0 / L, p0.y + (p1.y - p0.y) * s0 / L),
                                                ImVec2(p0.x + (p1.x - p0.x) * (s0 + run) / L, p0.y + (p1.y - p0.y) * (s0 + run) / L), dc, lw);
                    s0 += fmaxf(run, 0.5f);
                }
                acc += L;
            }
        }
    auto label = [&](ImVec2 c, const std::string &s, ImU32 col, bool bold) {
        float fs2 = fsz * (bold ? 1.0f : 0.9f);
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
        ImGui::SetTooltip("Take the boat to %s", nm.c_str());
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) clicked = opt[over];
    }
    ImGui::End();
    return clicked;
}
