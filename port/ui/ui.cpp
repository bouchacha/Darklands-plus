// ui.cpp - the native port's own interface, drawn over the game: the Darklands+ menu
// (F12): mods, display settings, a party/inventory view. Dear ImGui on SDL_Renderer.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/mods.h"
#include "ui.h"
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE;
}

static bool open_menu;
static int want_tab = -1;
extern "C" void ui_select_tab(int t) { want_tab = t; }
static ImFont *font_body, *font_head;
static SDL_Window *g_win;
static int dirty_config;
UiSettings ui_settings = { 1, 0, 0 };

static void style(void)
{
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowRounding = 6; s.FrameRounding = 4; s.GrabRounding = 4; s.TabRounding = 4;
    s.WindowPadding = ImVec2(18, 14); s.ItemSpacing = ImVec2(10, 8); s.FramePadding = ImVec2(8, 4);
    ImVec4 *c = s.Colors;
    ImVec4 ink(0.20f, 0.12f, 0.06f, 1), parch(0.93f, 0.87f, 0.74f, 0.97f), dark(0.45f, 0.18f, 0.10f, 1);
    c[ImGuiCol_Text] = ink;
    c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.38f, 0.30f, 1);
    c[ImGuiCol_WindowBg] = parch;
    c[ImGuiCol_PopupBg] = parch;
    c[ImGuiCol_ChildBg] = ImVec4(0.88f, 0.81f, 0.66f, 0.6f);
    c[ImGuiCol_Border] = ImVec4(0.42f, 0.30f, 0.16f, 0.8f);
    c[ImGuiCol_TitleBg] = c[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.66f, 0.46f, 1);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.36f, 0.16f, 0.09f, 0.7f);
    c[ImGuiCol_FrameBg] = ImVec4(0.98f, 0.94f, 0.84f, 1);
    c[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 0.97f, 0.88f, 1);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.95f, 0.88f, 0.72f, 1);
    c[ImGuiCol_CheckMark] = dark;
    c[ImGuiCol_Button] = ImVec4(0.55f, 0.24f, 0.13f, 1);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.66f, 0.30f, 0.16f, 1);
    c[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.18f, 0.10f, 1);
    c[ImGuiCol_Tab] = ImVec4(0.78f, 0.68f, 0.50f, 1);
    c[ImGuiCol_TabHovered] = ImVec4(0.66f, 0.30f, 0.16f, 1);
    c[ImGuiCol_TabActive] = ImVec4(0.55f, 0.24f, 0.13f, 1);
    c[ImGuiCol_Header] = ImVec4(0.80f, 0.70f, 0.52f, 1);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.75f, 0.56f, 1);
    c[ImGuiCol_HeaderActive] = ImVec4(0.75f, 0.64f, 0.46f, 1);
    c[ImGuiCol_Separator] = ImVec4(0.55f, 0.42f, 0.25f, 1);
    c[ImGuiCol_TableHeaderBg] = ImVec4(0.80f, 0.70f, 0.52f, 1);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(0.0f, 0.0f, 0.0f, 0.05f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0.05f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.42f, 0.25f, 0.8f);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.45f);
}

extern "C" void ui_init(SDL_Window *win, SDL_Renderer *ren)
{
    g_win = win;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFontConfig fc; fc.FontDataOwnedByAtlas = false;
    font_body = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 24.0f, &fc);
    font_head = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, 32.0f, &fc);
    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer2_Init(ren);
    style();
    ui_settings.aspect = config_get_int("display.aspect_4_3", 1);
    ui_settings.smooth = config_get_int("display.smooth", 0);
    ui_settings.integer = config_get_int("display.integer_scale", 0);
}

extern "C" int ui_is_open(void) { return open_menu; }
extern "C" void ui_toggle(void) { open_menu = !open_menu; if (!open_menu && dirty_config) { config_save(); dirty_config = 0; } }

extern "C" int ui_event(const SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12 && !(e->key.keysym.mod & KMOD_CTRL)) { ui_toggle(); return 1; }
    if (!open_menu) return 0;
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE) { ui_toggle(); return 1; }
    ImGui_ImplSDL2_ProcessEvent(e);
    return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
}

/* ------------------------------------------------------------------ tabs */
static void tab_mods(void)
{
    ImGui::TextWrapped("Changes to the game, switched on and off here. They take effect at once "
                       "(\"new game\" mods when the next game starts).");
    ImGui::Spacing();
    const char *cat = "";
    for (int i = 0; i < MOD_COUNT; i++) {
        Mod &m = MODS[i];
        if (strcmp(cat, m.category)) {
            cat = m.category;
            ImGui::PushFont(font_head); ImGui::SeparatorText(cat); ImGui::PopFont();
        }
        bool on = m.enabled;
        ImGui::PushID(i);
        if (ImGui::Checkbox(m.name, &on)) { mod_enable(i, on); dirty_config = 1; }
        ImGui::Indent(34);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextWrapped("%s%s", m.desc, m.needs_new_game ? "  (New game.)" : "");
        ImGui::PopStyleColor();
        ImGui::Unindent(34);
        ImGui::PopID();
    }
}

static void tab_display(void)
{
    bool a = ui_settings.aspect, s = ui_settings.smooth, n = ui_settings.integer;
    if (ImGui::Checkbox("4:3 picture, as on a 1992 monitor", &a)) { ui_settings.aspect = a; config_set_int("display.aspect_4_3", a); dirty_config = 1; }
    if (ImGui::Checkbox("Smooth scaling", &s)) { ui_settings.smooth = s; config_set_int("display.smooth", s); dirty_config = 1; }
    if (ImGui::Checkbox("Whole-number scaling (sharpest pixels)", &n)) { ui_settings.integer = n; config_set_int("display.integer_scale", n); dirty_config = 1; }
    bool fs = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(g_win, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

static void item_name(int id, char *out, int n)
{
    u32 d = g_itemdef(id);
    if (!d || id <= 0) { snprintf(out, (size_t)n, "#%d", id); return; }
    g_str((u16)(d >> 16), (u16)d, out, n);
}

static void tab_party(void)
{
    ImGui::TextWrapped("The party's possessions, read live from the game.");
    if (ImGui::BeginTabBar("members")) {
        for (int m = 0; m < 5; m++) {
            char name[32];
            g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), name, sizeof name);
            if (!name[0]) continue;
            if (ImGui::BeginTabItem(name)) {
                int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                if (n > 64) n = 64;
                if (ImGui::BeginTable("inv", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                                      ImVec2(0, 360))) {
                    ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("id"); ImGui::TableSetupColumn("type"); ImGui::TableSetupColumn("quality");
                    ImGui::TableSetupColumn("count"); ImGui::TableSetupColumn("weight");
                    ImGui::TableHeadersRow();
                    for (int i = 0; i < n; i++) {
                        u16 r = G_ITEM(m, i);
                        int id = rw(G_FAR2E38, r);
                        char nm[48]; item_name(id, nm, sizeof nm);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn(); ImGui::TextUnformatted(nm);
                        ImGui::TableNextColumn(); ImGui::Text("%d", id);
                        for (int k = 2; k < 6; k++) { ImGui::TableNextColumn(); ImGui::Text("%d", rb(G_FAR2E38, (u16)(r + k))); }
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

static void tab_about(void)
{
    ImGui::TextWrapped("Darklands (c) 1992 MicroProse. This is the original game, recompiled to run natively, "
                       "with the original game files from your copy.");
    ImGui::Spacing();
    ImGui::TextWrapped("F12 opens and closes this menu (Esc closes it). The game is paused while it is open.");
    ImGui::TextWrapped("Alt+Enter: full screen.  Ctrl+F12: screenshot.");
    ImGui::Spacing();
    ImGui::TextDisabled("Font: EB Garamond (SIL Open Font License). Interface: Dear ImGui (MIT). "
                        "Music synthesis: your system's MIDI synthesizer.");
}

extern "C" void ui_draw(SDL_Renderer *ren)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (open_menu) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));
        float w = fminf(io.DisplaySize.x * 0.8f, 900), h = fminf(io.DisplaySize.y * 0.85f, 700);
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushFont(font_body);
        bool keep = true;
        if (ImGui::Begin("Darklands+", &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (ImGui::BeginTabBar("tabs")) {
                const char *names[4] = { "Mods", "Party", "Display", "About" };
                void (*fns[4])(void) = { tab_mods, tab_party, tab_display, tab_about };
                for (int t = 0; t < 4; t++) {
                    if (ImGui::BeginTabItem(names[t], nullptr, want_tab == t ? ImGuiTabItemFlags_SetSelected : 0)) { fns[t](); ImGui::EndTabItem(); }
                }
                want_tab = -1;
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        ImGui::PopFont();
        if (!keep) ui_toggle();
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
}
