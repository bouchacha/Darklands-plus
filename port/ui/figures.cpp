// figures.cpp - the figure picker of the creation menus: Select Character Image (S) shows every
// figure at once (the game's own four and the enemies' fourteen, mods/looks.c), each in the
// colours the member would have; a click (or Enter) takes one, Esc keeps the old. The choice
// goes to the game as another S, which looks.c's hook turns into that figure, so the game does
// everything it does for S itself (charactr.tmp, the portrait). Off: S cycles as in the original.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <string>
#include <vector>
#include <SDL.h>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/looks.h"
}
extern ImFont *ui_font_small, *ui_font_text;

static SDL_Renderer *fp_ren;
static bool fp_open;
static int fp_member, fp_sel, fp_cur, fp_frame;
struct Fig { std::string code, title; SDL_Texture *tex = nullptr; int w = 0, h = 0; };
static std::vector<Fig> figs;

void figures_set_renderer(SDL_Renderer *r) { fp_ren = r; }
extern "C" int figpick_is_open(void) { return fp_open; }

static void drop_textures(void) { for (Fig &f : figs) if (f.tex) SDL_DestroyTexture(f.tex); figs.clear(); }

extern "C" int figpick_open(int member, int count)
{
    (void)count;
    if (!fp_ren || member < 0 || member > 4) return 0;
    drop_textures();
    char list[48][4]; int n = looks_list(list, 48);
    if (n < 2) return 0;
    char cur[4]; looks_member_figure(member, cur);
    static unsigned char rgba[64 * 64 * 4];
    fp_cur = 0;
    for (int i = 0; i < n; i++) {
        Fig f; f.code = list[i]; f.title = looks_title_of(list[i]);
        if (f.title.empty()) f.title = f.code;
        if (looks_thumb(list[i], member, rgba, 24, 40, &f.w, &f.h)) {
            f.tex = SDL_CreateTexture(fp_ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, f.w, f.h);
            if (f.tex) {
                SDL_UpdateTexture(f.tex, nullptr, rgba, f.w * 4);
                SDL_SetTextureBlendMode(f.tex, SDL_BLENDMODE_BLEND);
#if SDL_VERSION_ATLEAST(2, 0, 12)
                SDL_SetTextureScaleMode(f.tex, SDL_ScaleModeNearest);
#endif
            }
        }
        if (!strncasecmp(list[i], cur, 3)) fp_cur = i;
        figs.push_back(f);
    }
    fp_member = member; fp_sel = fp_cur; fp_open = true; fp_frame = ImGui::GetFrameCount();
    return 1;
}

static void choose(int i)
{
    fp_open = false;
    if (i >= 0 && i < (int)figs.size() && i != fp_cur) looks_pick(figs[i].code.c_str());
}

extern "C" void figpick_window(void)
{
    if (!fp_open) return;
    ImGuiIO &io = ImGui::GetIO();
    int n = (int)figs.size(), cols = n > 12 ? 6 : 4, rows = (n + cols - 1) / cols;
    /* the figures at a whole multiple of their pixels (40 high), as big as the screen allows */
    float name_h = UIS(40), avail = io.DisplaySize.y * 0.86f - UIS(120);
    int k = (int)((avail / rows - name_h - UIS(16)) / 40); if (k > 4) k = 4; if (k < 1) k = 1;
    float img_h = 40.0f * k + UIS(12), cell = 26.0f * k + UIS(40); if (cell < UIS(124)) cell = UIS(124);
    float grid_w = cols * cell, grid_h = rows * (img_h + name_h + UIS(8));
    static int logged; if (!logged++ && getenv("DL_LOOKS_LOG")) fprintf(stderr, "figures: picker %gx%g, ui scale %g, pixels x%d\n", io.DisplaySize.x, io.DisplaySize.y, ui_scale, k);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowFocus();
    char nick[16]; g_str(G_DGROUP, (u16)(0x9c55 + 0x80 * fp_member + 0x3e), nick, sizeof nick);
    std::string title = "Choose a figure for " + game_text(nick, sizeof nick) + "##figpick";
    bool keep = true;
    if (ImGui::Begin(title.c_str(), &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + grid_w);
        if (ui_font_small) ImGui::PushFont(ui_font_small);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Click a figure (or the arrow keys and Enter). The first four are the game's own; the others are its enemies'. 1st/2nd/3rd color dye them afterwards.");
        ImGui::PopStyleColor();
        if (ui_font_small) ImGui::PopFont();
        ImGui::Spacing();
        bool fresh = ImGui::GetFrameCount() - fp_frame < 2;          /* (the S that opened it is not a key here) */
        if (!fresh) {
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_S)) fp_sel = (fp_sel + 1) % n;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) fp_sel = (fp_sel + n - 1) % n;
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) fp_sel = (fp_sel + cols) % n;
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) fp_sel = (fp_sel + n - cols % n) % n;
            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) || ImGui::IsKeyPressed(ImGuiKey_Space)) { choose(fp_sel); }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) keep = false;
        }
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 o = ImGui::GetCursorScreenPos();
        for (int i = 0; i < n && fp_open; i++) {
            Fig &f = figs[i];
            ImVec2 p(o.x + (i % cols) * cell, o.y + (i / cols) * (img_h + name_h + UIS(8)));
            ImGui::SetCursorScreenPos(p);
            ImGui::PushID(i);
            bool clicked = ImGui::InvisibleButton("fig", ImVec2(cell - UIS(6), img_h + name_h));
            bool hot = ImGui::IsItemHovered();
            ImGui::PopID();
            if (hot && (io.MouseDelta.x != 0 || io.MouseDelta.y != 0)) fp_sel = i;      /* (a resting mouse leaves the keys' choice alone) */
            ImU32 bg = i == fp_sel ? IM_COL32(120, 84, 40, 150) : i == fp_cur ? IM_COL32(80, 70, 50, 120) : IM_COL32(40, 34, 28, 110);
            dl->AddRectFilled(p, ImVec2(p.x + cell - UIS(6), p.y + img_h + name_h), bg, UIS(4));
            if (i == fp_cur) dl->AddRect(p, ImVec2(p.x + cell - UIS(6), p.y + img_h + name_h), IM_COL32(220, 190, 120, 200), UIS(4), 0, 1.5f);
            if (f.tex && f.w && f.h) {
                float w = f.w * (float)k, h = f.h * (float)k;
                ImVec2 a(p.x + (cell - UIS(6) - w) * 0.5f, p.y + img_h - UIS(4) - h);
                dl->AddImage((ImTextureID)(intptr_t)f.tex, a, ImVec2(a.x + w, a.y + h));
            }
            ImFont *ft = ui_font_small ? ui_font_small : ImGui::GetFont();
            float fs = ft->FontSize;
            ImVec2 ts = ft->CalcTextSizeA(fs, cell - UIS(10), cell - UIS(10), f.title.c_str());
            dl->AddText(ft, fs, ImVec2(p.x + (cell - UIS(6) - ts.x) * 0.5f, p.y + img_h + UIS(2)), IM_COL32(240, 228, 200, 255),
                        f.title.c_str(), nullptr, cell - UIS(10));
            if (clicked) choose(i);
        }
        ImGui::SetCursorScreenPos(o); ImGui::Dummy(ImVec2(grid_w, grid_h));
        if (fp_open && ImGui::Button("Keep the old one (Esc)")) keep = false;
    }
    ImGui::End();
    if (!keep) fp_open = false;
    if (!fp_open) drop_textures();
}
