// ui.cpp - the native port's own interface, drawn over the game: the Darklands+ menu
// (F12): mods, display settings, a party/inventory view. Dear ImGui on SDL_Renderer.
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "gamedata.h"

extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/mods.h"
#include "../mods/assist.h"
#include "../mods/travel.h"
#include "../mods/fixes.h"
#include "../mods/hdtext.h"
#include "../mods/hdart.h"
#include "../mods/sfx.h"
#include "ui.h"
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE;
}

static bool open_menu;
static int want_tab = -1;
extern int ui_want_subtab;
extern "C" void ui_select_tab(int t) { want_tab = t % 100; if (t >= 100) ui_want_subtab = t / 100 - 1; }   /* 100*(sub+1) + tab */
static ImFont *font_body, *font_head;
ImFont *ui_font_small, *ui_font_text;
extern "C" void tab_inventory(void);
extern "C" void tab_alchemy(void);
extern "C" void ui_overlay(void);
extern "C" void hdtext_draw(void);
extern "C" void tab_journal(void);
extern "C" void tab_travel(void);
extern "C" void journal_init(void);
extern "C" void journal_set(int on);
extern "C" int journal_enabled(void);
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
    c[ImGuiCol_Tab] = ImVec4(0.74f, 0.63f, 0.45f, 1);
    c[ImGuiCol_TabHovered] = ImVec4(0.90f, 0.80f, 0.60f, 1);
    c[ImGuiCol_TabActive] = ImVec4(0.93f, 0.87f, 0.74f, 1);
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
    ui_font_small = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, 17.0f, &fc);
    {   /* the game's text, redrawn crisp (mods/hdtext.c): baked large, drawn at any size */
        ImFontConfig tc; tc.FontDataOwnedByAtlas = false; tc.OversampleH = 2; tc.OversampleV = 2;
        static const ImWchar ranges[] = { 0x20, 0xff, 0 };
        ui_font_text = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 34.0f, &tc, ranges);
    }
    gamedata_load(game_dir);
    journal_init();
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

/* a wrapped, dimmed explanation */
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
static void hint(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrappedV(fmt, ap);
    ImGui::PopStyleColor();
    va_end(ap);
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

/* "play the original game": every change off, the player's choices kept to come back to */
static int original_mode(void) { return config_get_int("original", 0); }
static void set_original(bool on)
{
    if (on) {
        char b[512]; int n = 0;
        n += snprintf(b + n, sizeof b - n, "%d %d %d %d %d %d", assist_chances, journal_enabled(), pathfix_party, pathfix_enemies, travel_enabled, travel_fast);
        for (int i = 0; i < FIX_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", FIXES[i].on);
        for (int i = 0; i < MOD_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", MODS[i].enabled);
        config_set("original.saved", b);
        config_set("original.sound", config_get("sound.device", "fm"));
        config_set_int("original.text", hdtext_mode);
        config_set_int("original.art", hdart_on);
        config_set_int("original.sfx", sfx_mode);
        assist_set_chances(0); journal_set(0); pathfix_set(0, 0); travel_set(0, travel_fast); hdtext_set(0); hdart_set(0); sfx_set(0);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, 0);
        for (int i = 0; i < MOD_COUNT; i++) if (MODS[i].enabled) mod_enable(i, 0);
        config_set("sound.device", "game");
        config_set_int("original", 1);
    } else {
        const char *p = config_get("original.saved", "");
        char *e;
        auto next = [&](int def) { long v = strtol(p, &e, 10); if (e == p) return def; p = e; return (int)v; };
        int ch = next(2), jo = next(1), pp = next(1), pe = next(0), ta = next(1), tf = next(1);
        assist_set_chances(ch); journal_set(jo); pathfix_set(pp, pe); travel_set(ta, tf);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, next(1));
        for (int i = 0; i < MOD_COUNT; i++) if (next(0)) mod_enable(i, 1);
        config_set("sound.device", config_get("original.sound", "fm"));
        hdtext_set(config_get_int("original.text", 1));
        hdart_set(config_get_int("original.art", 1));
        sfx_set(config_get_int("original.sfx", 1));
        config_set_int("original", 0);
    }
    dirty_config = 1;
}

static void tab_helpers(void)
{
    bool orig = original_mode();
    if (ImGui::Checkbox("Play the original game", &orig)) set_original(orig);
    hint("Switches everything below off and uses the game's own sound setup; your choices come back when you untick it.");
    if (orig) { ImGui::TextWrapped("Darklands as it was in 1993 (version 483.07). Only the Darklands+ menu itself remains."); return; }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Chances of success"); ImGui::PopFont();
    ImGui::TextWrapped("Town and encounter cards show the chance of success next to each option that has one, "
                       "and who would try it. (The original shows one at a time while you hold Shift.)");
    int c = assist_chances;
    if (ImGui::RadioButton("Always, as numbers", c == 2)) c = 2;
    if (ImGui::RadioButton("As the difficulty level allows (numbers, words or nothing)", c == 1)) c = 1;
    if (ImGui::RadioButton("Off", c == 0)) c = 0;
    if (c != assist_chances) { assist_set_chances(c); dirty_config = 1; }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Battles"); ImGui::PopFont();
    bool pp = pathfix_party, pe = pathfix_enemies;
    if (ImGui::Checkbox("Better pathfinding for the party (no more getting stuck in mines)", &pp)) { pathfix_set(pp, pe); dirty_config = 1; }
    if (ImGui::Checkbox("Better pathfinding for enemies too (harder battles)", &pe)) { pathfix_set(pp, pe); dirty_config = 1; }
    hint("Moves are planned around walls and furniture with a full route search instead of the "
                        "original wall-following, which gives up on long or winding routes.");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Travel"); ImGui::PopFont();
    bool ta = travel_enabled; int tf = travel_fast;
    if (ImGui::Checkbox("Auto-travel along roads (Travel tab, or Ctrl+click on the map)", &ta)) { travel_set(ta, tf); dirty_config = 1; }
    ImGui::TextUnformatted("Pace of auto-travel:"); ImGui::SameLine();
    if (ImGui::RadioButton("as walking", tf == 0)) { travel_set(ta, 0); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("4x", tf == 1)) { travel_set(ta, 1); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("at once", tf == 2)) { travel_set(ta, 2); dirty_config = 1; }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Town journal"); ImGui::PopFont();
    bool j = journal_enabled();
    if (ImGui::Checkbox("Keep a journal of the towns the party visits", &j)) { journal_set(j); dirty_config = 1; }
    hint("Each town's shops, their wares, qualities and prices, noted on arrival (Journal tab).");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Fixes for bugs of the original"); ImGui::PopFont();
    for (int i = 0; i < FIX_COUNT; i++) {
        bool on = FIXES[i].on;
        ImGui::PushID(i);
        if (ImGui::Checkbox(FIXES[i].name, &on)) { fix_enable(i, on); dirty_config = 1; }
        ImGui::Indent(34); hint("%s", FIXES[i].desc); ImGui::Unindent(34);
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
    ImGui::PushFont(font_head); ImGui::SeparatorText("Text"); ImGui::PopFont();
    int hm = hdtext_mode;
    if (ImGui::RadioButton("Crisp text on cards (redrawn with a modern book face)", hm == 1)) { hdtext_set(1); dirty_config = 1; }
    if (ImGui::RadioButton("Crisp text everywhere (experimental: sheets, map, menus)", hm == 2)) { hdtext_set(2); dirty_config = 1; }
    if (ImGui::RadioButton("The game's own lettering", hm == 0)) { hdtext_set(0); dirty_config = 1; }
    hint("The original 7-pixel blackletter is hard to read when scaled up. Crisp text draws the same words at "
         "the window's resolution; anything it cannot match exactly is left as the game drew it.");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Pictures"); ImGui::PopFont();
    bool ha = hdart_on;
    if (ImGui::Checkbox("Use replacement art from the \"art\" folder", &ha)) { hdart_set(ha); dirty_config = 1; }
    {
        extern char game_dir[1024];
        int n = hdart_count();
        if (n) hint("%d replacement picture%s in %s%sart.", n, n == 1 ? "" : "s", game_dir, PATH_SEP);
        else hint("No replacement pictures yet (%s%sart).", game_dir, PATH_SEP);
        static char msg[600];
        if (ImGui::Button("Look again")) { hdart_rescan(); snprintf(msg, sizeof msg, "%d replacement pictures found.", hdart_count()); }
        ImGui::SameLine();
        if (ImGui::Button("Export the original pictures")) hdart_export(msg, sizeof msg);
        if (msg[0]) hint("%s", msg);
        bool sn = hdart_show_names;
        if (ImGui::Checkbox("Show the name of the picture under the mouse", &sn)) { hdart_show_names = sn; config_set_int("display.art_names", sn); dirty_config = 1; }
        hint("For artists: a file art%sNAME.png (any size; transparency where the picture has colour 0) replaces the "
             "game's picture NAME wherever it is shown, at the window's resolution. Export writes every original picture "
             "to art%soriginal as a template, in its own colours. Whatever the game draws over a picture (text, the "
             "mouse pointer, other pictures) stays on top.", PATH_SEP, PATH_SEP);
    }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Sound"); ImGui::PopFont();
    static const char *ids[] = { "fm", "adlib", "midi", "speaker", "off", "game" };
    static const char *labels[] = {
        "Sound Blaster Pro (OPL-3 FM, emulated) - recommended",
        "AdLib / Sound Blaster (OPL-2 FM, emulated)",
        "Roland MIDI through your system's synthesizer",
        "PC speaker",
        "Off",
        "As in the game's own setup (INSTALL)" };
    const char *cur = config_get("sound.device", "fm");
    for (int i = 0; i < 6; i++)
        if (ImGui::RadioButton(labels[i], !strcmp(cur, ids[i]))) { config_set("sound.device", ids[i]); dirty_config = 1; }
    hint("Takes effect after the next battle, or when the game is started again.");
    {
        ImGui::Spacing();
        bool sp = sfx_mode, hs = sfx_added_hits, sy = sfx_synth;
        extern char game_dir[1024];
        if (ImGui::Checkbox("Battle sounds from a sound pack (the \"sounds\" folder)", &sp)) { sfx_set(sp); dirty_config = 1; }
        int n = sfx_user_count();
        if (n) hint("%d sound%s in %s%ssounds; the game's own sounds for the rest.", n, n == 1 ? "" : "s", game_dir, PATH_SEP);
        else hint("No sound pack yet (%s%ssounds).", game_dir, PATH_SEP);
        if (sp) {
            ImGui::Indent(30);
            if (ImGui::Checkbox("Blows that land make a sound too (the original is silent there)", &hs)) { sfx_set_hits(hs); dirty_config = 1; }
            if (ImGui::Checkbox("Experimental: synthesized sounds where the pack has none", &sy)) { sfx_set_synth(sy); dirty_config = 1; }
            int v = sfx_volume;
            ImGui::SetNextItemWidth(260);
            if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); dirty_config = 1; }
            static char msg[400];
            if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sounds found.", sfx_user_count()); }
            ImGui::SameLine();
            if (ImGui::Button("Write the synthesized sounds as .wav")) sfx_export(msg, sizeof msg);
            if (msg[0]) hint("%s", msg);
            ImGui::Unindent(30);
        }
        if (ImGui::TreeNode("File names for a sound pack")) {
            for (int k = 0; k < SFX_COUNT; k++) hint("%s.wav or .ogg: %s", SFX_NAMES[k], SFX_DESC[k]);
            hint("Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random.");
            ImGui::TreePop();
        }
    }
    ImGui::TextWrapped("The GOG release plays battles through its Roland driver, which on a modern PC sounds like "
                       "\"random notes rather than clashing swords\". The Sound Blaster setting uses the game's own FM "
                       "drivers and digitized effects, as most players heard it in 1992.");
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
    hint("Font: EB Garamond (SIL Open Font License). Interface: Dear ImGui (MIT). "
                        "Music synthesis: your system's MIDI synthesizer.");
}

extern "C" void ui_draw(SDL_Renderer *ren)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    hdtext_draw();
    if (!open_menu) ui_overlay();
    if (open_menu) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));
        float w = fminf(io.DisplaySize.x * 0.9f, 1100), h = fminf(io.DisplaySize.y * 0.9f, 820);
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushFont(font_body);
        bool keep = true;
        if (ImGui::Begin("Darklands+", &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (ImGui::BeginTabBar("tabs")) {
                const char *names[] = { "Inventory", "Alchemy", "Journal", "Travel", "Helpers", "Mods", "Display & sound", "Item records", "About" };
                void (*fns[])(void) = { tab_inventory, tab_alchemy, tab_journal, tab_travel, tab_helpers, tab_mods, tab_display, tab_party, tab_about };
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
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
