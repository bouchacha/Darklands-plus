/* screenpics.cpp - pictures of the new screens from the player's own game, for the settings' "New
 * screens" switch: the first time a card, a shop, a character sheet, the inn and the alchemy screen
 * have stood still on screen for a moment (no window of Darklands+'s over them), the whole window
 * is kept, small, in <data>/previews/<name>.png. The switch's row shows them one after another.
 * Taken on the player's machine from the player's own game: nothing of the game's art ships. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include "imgui.h"
#include <SDL.h>
extern "C" {
#include "../mods/nscreen.h"
u32 plat_ticks_ms(void);
void plat_capture_small(const char *path, int max_w);   /* runtime/platform_sdl.c: at the next present */
extern char dlp_data_dir[];
extern char game_dialog_text[600];
}
#include "../third_party/stb/stb_image.h"

namespace {
struct Pic { int screen; const char *name, *label; SDL_Texture *tex = nullptr; int w = 0, h = 0; bool tried = false, have = false; };
Pic P[5] = { { NS_CARD, "card", "A card" }, { NS_BARTER, "shop", "A shop" }, { NS_SHEET, "sheet", "The character sheet" },
             { NS_LODGE, "inn", "The inn" }, { NS_ALCHEMY, "alchemy", "Alchemy" } };
SDL_Renderer *ren;
std::string path_of(const Pic &p) { return std::string(dlp_data_dir) + "/previews/" + p.name + ".png"; }
bool exists(const std::string &f) { struct stat st; return stat(f.c_str(), &st) == 0; }
}

extern "C" void screenpics_set_renderer(SDL_Renderer *r) { ren = r; }

/* each frame, after drawing (ui/ui.cpp): a screen worth keeping, still for 1.5 s, and nothing over it */
extern "C" void screenpics_frame(int ui_window_open)
{
    static int last = -1; static u32 since;
    int s = ns_screen;
    if (s != last) { last = s; since = plat_ticks_ms(); return; }
    if (ui_window_open || ns_hidden() || game_dialog_text[0] || plat_ticks_ms() - since < 1500) return;
    for (Pic &p : P) {
        if (p.screen != s) continue;
        if (!p.have && !p.tried) { p.tried = true; p.have = exists(path_of(p)); }
        if (p.have) return;
        std::string dir = std::string(dlp_data_dir) + "/previews";
#ifdef _WIN32
        mkdir(dir.c_str());
#else
        mkdir(dir.c_str(), 0755);
#endif
        plat_capture_small(path_of(p).c_str(), 960);
        p.have = true; p.tried = false;                   /* (loaded afresh when next shown) */
        if (p.tex) { SDL_DestroyTexture(p.tex); p.tex = nullptr; }
        return;
    }
}

/* the settings' row: the pictures kept so far, one after another every few seconds, in a..b */
extern "C" bool screenpics_draw(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    int n = 0, idx[5];
    for (int i = 0; i < 5; i++) {
        Pic &p = P[i];
        if (!p.tex && !p.tried && ren) {
            p.tried = true;
            int c; unsigned char *px = stbi_load(path_of(p).c_str(), &p.w, &p.h, &c, 4);
            if (px) {
                p.tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, p.w, p.h);
                if (p.tex) { SDL_UpdateTexture(p.tex, 0, px, p.w * 4); SDL_SetTextureScaleMode(p.tex, SDL_ScaleModeLinear); }
                stbi_image_free(px);
            }
        }
        if (p.tex) idx[n++] = i;
    }
    if (!n) return false;
    double t = ImGui::GetTime() / 3.0;
    int k = (int)t % n; float f = (float)(t - (int)t);
    const Pic &p = P[idx[k]];
    /* fitted, kept in proportion */
    float bw = b.x - a.x, bh = b.y - a.y, sc = fminf(bw / p.w, bh / p.h), w = p.w * sc, h = p.h * sc;
    ImVec2 o(a.x + (bw - w) * 0.5f, a.y + (bh - h) * 0.5f);
    int al = f < 0.1f ? (int)(255 * f / 0.1f) : f > 0.9f ? (int)(255 * (1 - f) / 0.1f) : 255;   /* (a short fade between) */
    dl->AddRectFilled(a, b, IM_COL32(20, 14, 8, 255), 3);
    dl->AddImage((ImTextureID)(intptr_t)p.tex, o, ImVec2(o.x + w, o.y + h), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, al));
    ImVec2 ts = ImGui::CalcTextSize(p.label);
    ImVec2 lp(a.x + 8, b.y - ts.y - 8);
    dl->AddRectFilled(ImVec2(lp.x - 5, lp.y - 3), ImVec2(lp.x + ts.x + 5, lp.y + ts.y + 3), IM_COL32(30, 20, 10, 190), 3);
    dl->AddText(lp, IM_COL32(240, 226, 196, 255), p.label);
    return true;
}

/* the settings' pictures of the three ways to scale the game's picture: a piece of the screen
   now (plat_frame_crop), four times as large, with sharp pixels, with the diagonal steps rounded
   off (Scale2x twice: Scale4x) and softened (bilinear); made afresh every few seconds */
extern "C" int plat_frame_crop(unsigned *out, int cw, int ch);
extern "C" void plat_scale2x(const unsigned *src, int w, int h, unsigned *dst);
extern "C" bool screenpics_scaling(ImDrawList *dl, ImVec2 a, ImVec2 b, int mode)
{
    enum { PCW = 64, PCH = 40, PK = 4 };
    static SDL_Texture *tx[3]; static double made = -10;
    if (!ren) return false;
    if (ImGui::GetTime() - made > 3.0) {
        made = ImGui::GetTime();
        static unsigned src[PCW * PCH], x2[PCW * PCH * 4], x4[PCW * PCH * 16], soft[PCW * PCH * 16], sharp[PCW * PCH * 16];
        plat_frame_crop(src, PCW, PCH);
        for (int y = 0; y < PCH * PK; y++) for (int x = 0; x < PCW * PK; x++) sharp[y * PCW * PK + x] = src[(y / PK) * PCW + x / PK];
        plat_scale2x(src, PCW, PCH, x2); plat_scale2x(x2, PCW * 2, PCH * 2, x4);
        for (int y = 0; y < PCH * PK; y++) for (int x = 0; x < PCW * PK; x++) {      /* (bilinear, between pixel centres) */
            float fx = (x + 0.5f) / PK - 0.5f, fy = (y + 0.5f) / PK - 0.5f;
            int x0 = (int)floorf(fx), y0 = (int)floorf(fy); float tx_ = fx - x0, ty = fy - y0;
            auto at = [&](int xx, int yy) { xx = xx < 0 ? 0 : xx >= PCW ? PCW - 1 : xx; yy = yy < 0 ? 0 : yy >= PCH ? PCH - 1 : yy; return src[yy * PCW + xx]; };
            unsigned c00 = at(x0, y0), c10 = at(x0 + 1, y0), c01 = at(x0, y0 + 1), c11 = at(x0 + 1, y0 + 1), o = 0xff000000u;
            for (int sh = 0; sh < 24; sh += 8) {
                float v = ((c00 >> sh) & 255) * (1 - tx_) * (1 - ty) + ((c10 >> sh) & 255) * tx_ * (1 - ty) + ((c01 >> sh) & 255) * (1 - tx_) * ty + ((c11 >> sh) & 255) * tx_ * ty;
                o |= (unsigned)(v + 0.5f) << sh;
            }
            soft[y * PCW * PK + x] = o;
        }
        const unsigned *px[3] = { sharp, x4, soft };
        for (int m = 0; m < 3; m++) {
            if (!tx[m]) tx[m] = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, PCW * PK, PCH * PK);
            if (tx[m]) { SDL_UpdateTexture(tx[m], 0, px[m], PCW * PK * 4); SDL_SetTextureScaleMode(tx[m], SDL_ScaleModeNearest); }
        }
    }
    if (mode < 0 || mode > 2 || !tx[mode]) return false;
    dl->AddImage((ImTextureID)(intptr_t)tx[mode], a, b);
    return true;
}

/* the world map (M), kept the same way the first time it has been open a moment, for its switch's
   picture in the settings */
static Pic MAPPIC = { -1, "worldmap", "The world map (M)" };
extern "C" void screenpics_map_frame(int map_open, int other_open)
{
    static u32 since; static int was;
    if (!map_open || other_open) { was = 0; return; }
    if (!was) { was = 1; since = plat_ticks_ms(); return; }
    if (plat_ticks_ms() - since < 2000) return;
    static bool checked; if (!checked) { checked = true; MAPPIC.have = exists(path_of(MAPPIC)); }
    if (MAPPIC.have) return;
    std::string dir = std::string(dlp_data_dir) + "/previews";
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    plat_capture_small(path_of(MAPPIC).c_str(), 960);
    MAPPIC.have = true; MAPPIC.tried = false;                  /* (loaded afresh when next shown) */
    if (MAPPIC.tex) { SDL_DestroyTexture(MAPPIC.tex); MAPPIC.tex = nullptr; }
}
extern "C" bool screenpics_map_draw(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    Pic &p = MAPPIC;
    if (!p.tex && !p.tried && ren) {
        p.tried = true;
        int c; unsigned char *px = stbi_load(path_of(p).c_str(), &p.w, &p.h, &c, 4);
        if (px) {
            p.tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, p.w, p.h);
            if (p.tex) { SDL_UpdateTexture(p.tex, 0, px, p.w * 4); SDL_SetTextureScaleMode(p.tex, SDL_ScaleModeLinear); }
            stbi_image_free(px);
        }
    }
    if (!p.tex) return false;
    float bw = b.x - a.x, bh = b.y - a.y, sc = fminf(bw / p.w, bh / p.h), w = p.w * sc, h = p.h * sc;
    ImVec2 o(a.x + (bw - w) * 0.5f, a.y + (bh - h) * 0.5f);
    dl->AddRectFilled(a, b, IM_COL32(20, 14, 8, 255), 3);
    dl->AddImage((ImTextureID)(intptr_t)p.tex, o, ImVec2(o.x + w, o.y + h));
    return true;
}

/* the overland view (the game's own travel screen, with Darklands+'s names and quest marks on it),
   kept the same way, for those switches' pictures */
extern "C" int game_mode(void);
static Pic LANDPIC = { -1, "overland", "The overland view" };
extern "C" void screenpics_land_frame(int any_open)
{
    static u32 since; static int was; static bool checked;
    if (any_open || game_mode() != 2 /* GM_MAP */ || game_dialog_text[0]) { was = 0; return; }
    if (!was) { was = 1; since = plat_ticks_ms(); return; }
    if (plat_ticks_ms() - since < 2500) return;
    if (!checked) { checked = true; LANDPIC.have = exists(path_of(LANDPIC)); }
    if (LANDPIC.have) return;
    std::string dir = std::string(dlp_data_dir) + "/previews";
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    plat_capture_small(path_of(LANDPIC).c_str(), 960);
    LANDPIC.have = true; LANDPIC.tried = false;
    if (LANDPIC.tex) { SDL_DestroyTexture(LANDPIC.tex); LANDPIC.tex = nullptr; }
}
extern "C" bool screenpics_land_draw(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    Pic &p = LANDPIC;
    if (!p.tex && !p.tried && ren) {
        p.tried = true;
        int c; unsigned char *px = stbi_load(path_of(p).c_str(), &p.w, &p.h, &c, 4);
        if (px) {
            p.tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, p.w, p.h);
            if (p.tex) { SDL_UpdateTexture(p.tex, 0, px, p.w * 4); SDL_SetTextureScaleMode(p.tex, SDL_ScaleModeLinear); }
            stbi_image_free(px);
        }
    }
    if (!p.tex) return false;
    float bw = b.x - a.x, bh = b.y - a.y, sc = fminf(bw / p.w, bh / p.h), w = p.w * sc, h = p.h * sc;
    ImVec2 o(a.x + (bw - w) * 0.5f, a.y + (bh - h) * 0.5f);
    dl->AddRectFilled(a, b, IM_COL32(20, 14, 8, 255), 3);
    dl->AddImage((ImTextureID)(intptr_t)p.tex, o, ImVec2(o.x + w, o.y + h));
    return true;
}
