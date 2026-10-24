/* screenpics.cpp - pictures of the new screens from the player's own game, for the settings' "New
 * screens" switch: the first time a card, a shop, a character sheet, the inn and the alchemy screen
 * have stood still on screen for a moment (no window of Darklands+'s over them), the whole window
 * is kept, small, in <data>/previews/<name>.png. The switch's row shows them one after another.
 * Taken on the player's machine from the player's own game: nothing of the game's art ships. */
#include <stdio.h>
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
