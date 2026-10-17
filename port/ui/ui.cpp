// ui.cpp - the native port's own interface, drawn over the game: the Darklands+ menu
// (F12): mods, display settings, a party/inventory view. Dear ImGui on SDL_Renderer.
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "imgui.h"
#include "tablesort.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "gamedata.h"

extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/mods.h"
#include "../mods/assist.h"
#include "../mods/cardview.h"
#include "../mods/travel.h"
#include "../mods/fixes.h"
#include "../mods/saves.h"
#include "../mods/hdtext.h"
#include "../mods/musicpack.h"
#include "../mods/hdart.h"
#include "../mods/sfx.h"
#include "../mods/datamods.h"
#include "ui.h"
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[], FONT_GRENZE[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE;
}

extern "C" { int mt32_available(void); int mt32_is_open(void); void plat_restart(void); }
static bool open_menu, open_map, open_goto, open_help, open_welcome, open_mt32;
static void welcome_window(void);
extern "C" int orders_click_attack;
static void help_window(void);
extern "C" void ui_toggle_help(void) { open_help = !open_help; }
extern "C" int citynav_window(void);
extern "C" int citynav_available(void);
extern "C" void citynav_tick(void);
extern "C" void citynav_go(int dest);
/* "Go to" in cities (ui/citynav.cpp): G on a city card */
static bool goto_allowed(void) { return config_get_int("assist.city_goto", 1) && !config_get_int("original", 0) && citynav_available(); }
extern "C" void ui_open_goto(void) { if (goto_allowed()) { open_goto = true; open_menu = false; open_map = false; } }
extern "C" int worldmap_open(void);
extern "C" void worldmap_draw(void);
void worldmap_set_renderer(SDL_Renderer *r);
static int want_tab = -1;
extern int ui_want_subtab;
extern "C" void ui_select_tab(int t) { want_tab = t % 100; if (t >= 100) ui_want_subtab = t / 100 - 1; }   /* 100*(sub+1) + tab */
static ImFont *font_body, *font_head, *font_tab, *font_tab_small;
extern "C" int idle_rest;                  /* runtime/bios.c: rest while the game only waits */
extern "C" int halloffame_is_open(void);
extern "C" void halloffame_open(void);
extern "C" void halloffame_draw(void);
extern "C" void cardview_draw(void);
extern "C" void barterview_draw(void);
extern "C" void sheetview_draw(void);
extern "C" void lootview_draw(void);
extern "C" void menuview_draw(void);
extern "C" void savesview_draw(void);
extern "C" void cacheview_draw(void);
extern "C" void prayview_draw(void);
extern "C" void createview_draw(void);
extern "C" void rosterview_draw(void);
extern "C" void infoview_draw(void);
extern "C" void marchview_draw(void);
extern "C" { int marchview_is_on(void); void marchview_set(int on); }
extern "C" { int infoview_is_on(void); void infoview_set(int on); }
extern "C" { int rosterview_is_on(void); void rosterview_set(int on); }
extern "C" { int createview_is_on(void); void createview_set(int on); }
extern "C" { int prayview_is_on(void); void prayview_set(int on); }
extern "C" { int cacheview_is_on(void); void cacheview_set(int on); }
extern "C" { int savesview_is_on(void); void savesview_set(int on); }
extern "C" { int menuview_is_on(void); void menuview_set(int on); }
extern "C" void ns_defer_frame_end(int game_held);
extern "C" void alchemyview_draw(void);
extern "C" { int alchemyview_showing(void); int alchemyview_is_on(void); void alchemyview_set(int on); }
extern "C" { int partypanel_is_on(void); void partypanel_set(int on); int partypanel_picture(void); void partypanel_set_picture(int v); void partypanel_draw(void); void partypanel_set_renderer(SDL_Renderer *r); void partypanel_init(void); }
void gamepics_set_renderer(SDL_Renderer *r);
extern "C" { int lootview_showing(void); int lootview_is_on(void); void lootview_set(int on); }
extern "C" { int sheetview_showing(void); int sheetview_is_on(void); void sheetview_set(int on); }
extern "C" { int barterview_is_on(void); void barterview_set(int on); int barterview_showing(void); }
extern "C" { extern int battlescroll_on, battlescroll_rest; void battlescroll_set(int, int); extern int loot_return_thrown; extern int ammo_infinite; void ammo_set(int v); }
extern "C" void halloffame_close(void);
extern "C" int halloffame_record(void);
void halloffame_set_renderer(SDL_Renderer *r);
void figures_set_renderer(SDL_Renderer *r);
extern "C" int figpick_is_open(void);
extern "C" void figpick_window(void);

ImFont *ui_font_small, *ui_font_text;
float ui_scale = 1.0f;
ImFont *ui_font_game[UI_GAME_FONTS];       /* the crisp game text: EB Garamond, Alegreya, IM Fell, Atkinson, MedievalSharp, Fraktur */
ImFont *ui_font_game_hi[UI_GAME_FONTS];    /* the same baked larger on big screens (0 if not needed) */
int ui_game_font, ui_text_edge;            /* edge: 0 none, 1 shadow, 2 outline */
extern int ui_spell_out;                   /* overlay.cpp: character sheet names in full */
static int sys_cursor;                     /* display.system_cursor: the system pointer stays over the game */
const char *UI_GAME_FONT_NAMES[UI_GAME_FONTS] = { "EB Garamond (book face)", "Alegreya (calligraphic)", "IM Fell English (old print)",
    "Atkinson Hyperlegible (easiest to read)", "MedievalSharp (medieval hand)", "Fraktur (blackletter, for the brave)",
    "Grenze Gotisch (gothic, still readable)" };
#define UI_DEFAULT_FONT 6                  /* Grenze Gotisch */
/* the text over and beside the picture (side panels, tooltips, map names, notices): the game
   text's typeface, or the menus' (display.overlay_font_follow) */
ImFont *ui_font_overlay;
static void overlay_font_update(void)
{
    extern ImFont *ui_font_text;
    ui_font_overlay = config_get_int("display.overlay_font_follow", 1) && ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
}
extern "C" void tab_inventory(void);
extern "C" void tab_members(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
extern "C" void tab_alchemy(void);
extern "C" void ui_overlay(void);
extern "C" void hdtext_draw(void);
extern "C" void hdtext_draw_cursor(void);
extern "C" void hdtext_pointer_off(void);
extern "C" void hdtext_pointer_frame_end(void);
void ui_tab_icon(ImDrawList *dl, ImVec2 c, float s, int tab);
extern "C" void tab_journal(void);
extern "C" void tab_travel(void);
extern "C" void tab_saints(void);
extern "C" void tab_saves(void);
extern "C" void ui_draw_notice(void);
extern "C" void ui_notice(const char *msg);
extern "C" void rules_apply(void);
extern "C" void ui_notice_for(const char *msg, unsigned ms);
extern "C" void tab_editor(void);
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
    ImVec4 ink(0.20f, 0.12f, 0.06f, 1), parch(0.93f, 0.87f, 0.74f, 1.0f), dark(0.45f, 0.18f, 0.10f, 1);   /* opaque: the game's text showed through faintly */
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
    c[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.24f, 0.13f, 1);
    c[ImGuiCol_SliderGrabActive] = dark;
    c[ImGuiCol_Button] = ImVec4(0.78f, 0.66f, 0.47f, 1);           /* tan with ink letters: readable */
    c[ImGuiCol_ButtonHovered] = ImVec4(0.88f, 0.77f, 0.57f, 1);
    c[ImGuiCol_ButtonActive] = ImVec4(0.68f, 0.54f, 0.36f, 1);
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
    worldmap_set_renderer(ren);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;     /* the cursor is ours (ui_cursor) */
    {   /* the menus' size: display.ui_scale in percent, else on Windows the system's scale
           (125 %, 150 % ... of 96 dpi; the window is DPI-aware, see plat_init), else 100 % */
        int pct = config_get_int("display.ui_scale", 0);
        float k = 1.0f;
        if (pct >= 75 && pct <= 300) k = pct / 100.0f;
#ifdef _WIN32
        else {
            float ddpi = 0; int di = SDL_GetWindowDisplayIndex(win);
            if (SDL_GetDisplayDPI(di < 0 ? 0 : di, &ddpi, 0, 0) == 0 && ddpi > 0) k = ddpi / 96.0f;
            if (k < 1) k = 1; if (k > 3) k = 3;
        }
#endif
        ui_scale = roundf(k * 4) / 4;
        if (ui_scale != 1.0f) fprintf(stderr, "ui: menus at %.0f %%\n", ui_scale * 100);
    }
    ImFontConfig fc; fc.FontDataOwnedByAtlas = false;
    font_body = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, UIS(24.0f), &fc);
    font_head = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, UIS(32.0f), &fc);
    font_tab = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, UIS(20.0f), &fc);
    font_tab_small = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, UIS(16.5f), &fc);   /* the tabs in a narrow window */
    ui_font_small = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, UIS(17.0f), &fc);
    {   /* the game's text, redrawn crisp (mods/hdtext.c): baked at the size it takes on this screen
           when the window fills it (a line of the game's text is 6 of its 200 rows; 34 pixels at
           most screens, up to 90 on a 4K screen, where text baked smaller turned soft) */
        float bake = 34.0f;
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(win) < 0 ? 0 : SDL_GetWindowDisplayIndex(win), &dm) == 0 && dm.h > 0) {
            float need = dm.h / 200.0f * 6 * 1.18f * 1.05f;
            if (need > bake) bake = need > 90 ? 90 : need;
        }
        ImFontConfig tc; tc.FontDataOwnedByAtlas = false; tc.OversampleH = 2; tc.OversampleV = 2;
        static const ImWchar ranges[] = { 0x20, 0xff, 0 };
        ui_font_text = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 34.0f, &tc, ranges);
        const unsigned char *data[UI_GAME_FONTS] = { FONT_MEDIUM, FONT_ALEGREYA, FONT_FELL, FONT_ATKINSON, FONT_MEDIEVAL, FONT_FRAKTUR, FONT_GRENZE };
        const unsigned int size[UI_GAME_FONTS] = { FONT_MEDIUM_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_ATKINSON_SIZE, FONT_MEDIEVAL_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE };
        ui_font_game[0] = ui_font_text;
        for (int i = 1; i < UI_GAME_FONTS; i++) ui_font_game[i] = io.Fonts->AddFontFromMemoryTTF((void *)data[i], (int)size[i], 34.0f, &tc, ranges);
        if (bake > 44) {      /* big screens: a second, larger set for the game's text in a big window */
            ImFontConfig hc = tc; hc.OversampleH = 1; hc.OversampleV = 1;
            for (int i = 0; i < UI_GAME_FONTS; i++) ui_font_game_hi[i] = io.Fonts->AddFontFromMemoryTTF((void *)data[i], (int)size[i], bake, &hc, ranges);
            fprintf(stderr, "ui: game text also baked at %.0f pixels (screen %dx%d)\n", bake, dm.w, dm.h);
        }
    }
    open_welcome = (!config_get_int("ui.welcome_seen", 0) && !getenv("DL_SCRIPT") && !getenv("DL_CTL")) || getenv("DL_WELCOME");   /* the first start */
    {   /* the MT-32 recommendation: at each start while its ROM files are missing, until "don't remind me" */
        const char *dev = config_get("sound.device", "auto");
        open_mt32 = ((!getenv("DL_SCRIPT") && !getenv("DL_CTL")) || getenv("DL_MT32_PROMPT")) && config_get_int("sound.mt32_prompt", 1)
                    && !config_get_int("original", 0) && (!strcmp(dev, "auto") || !strcmp(dev, "mt32")) && !mt32_available();
    }
    ui_game_font = config_get_int("display.text_font", UI_DEFAULT_FONT);
    if (ui_game_font < 0 || ui_game_font >= UI_GAME_FONTS) ui_game_font = UI_DEFAULT_FONT;
    overlay_font_update();
    ui_text_edge = config_get_int("display.text_edge", 2);
    { extern int ui_text_contrast; ui_text_contrast = config_get_int("display.text_contrast", 1); }
    { extern int ui_skill_icons; ui_skill_icons = config_get_int("display.skill_icons", 1); }
    { extern int ui_shop_info; ui_shop_info = config_get_int("assist.shop_info", 1); }
    { extern int ui_creation_guide; ui_creation_guide = config_get_int("assist.creation_guide", 1); }
    { extern int ui_battle_info, ui_battle_numbers; ui_battle_info = config_get_int("battle.info", 1); ui_battle_numbers = config_get_int("battle.info_numbers", 0); }
    { extern int ui_battle_log; ui_battle_log = config_get_int("battle.log", 1); }
    ui_spell_out = config_get_int("display.spell_out", 1);
    sys_cursor = config_get_int("display.system_cursor", 0);
    idle_rest = config_get_int("display.idle_rest", 1);
    { extern int ui_margins_on; ui_margins_on = config_get_int("display.margins", 1); }
    gamedata_load(game_dir);
    if (getenv("DL_RESTARTED")) {
        const char *ee = getenv("DL_ERROR_EXIT");
        if (ee && atoi(ee)) ui_notice_for("Darklands stopped on an error (darklands_port.log in the game folder tells which). Your saved games are kept: load the last one (autosaves are made on arrival and before battles): F12 > Saves, or Load in the game.", 30000);
        else ui_notice_for("The party's journey has ended. Load a saved game to go on: F12 > Saves, or Load in the game.", 20000);
    }
    halloffame_set_renderer(ren);
    gamepics_set_renderer(ren); partypanel_set_renderer(ren); partypanel_init();
    figures_set_renderer(ren);
    if (getenv("DL_HALL")) halloffame_open();     /* the party has just entered the Honor Roll of Glory */
    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer2_Init(ren);
    style();
    if (ui_scale != 1.0f) ImGui::GetStyle().ScaleAllSizes(ui_scale);
    ui_settings.aspect = 1;            /* always 4:3: the game was drawn for a 1992 monitor's tall pixels (the square-pixel option distorted it) */
    ui_settings.smooth = config_get_int("display.smooth", 0);
    ui_settings.integer = config_get_int("display.integer_scale", 0);
    ui_settings.scaler = config_get_int("display.scaler", 0);
}

/* what needs the game's code in place (hooks): after the game module is loaded (split.h) */
extern "C" void ui_late_init(void)
{
    if (g_win) { journal_init(); void battlelog_init(void); battlelog_init(); }
}

extern "C" int levelmap_is_open(void);
extern "C" int levelmap_available(void);
extern "C" void levelmap_toggle(void);
extern "C" void levelmap_frame(void);
extern "C" void levelmap_draw(void);
extern "C" int ui_is_open(void) { return open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || levelmap_is_open() || halloffame_is_open() || figpick_is_open(); }
extern "C" void ui_toggle(void) { open_map = false; open_goto = false; open_help = false; open_menu = !open_menu; if (!open_menu && dirty_config) { config_save(); dirty_config = 0; } }
/* the world map (ui/worldmap.cpp): M on the travel map, or the Travel tab */
extern "C" void ui_toggle_map(void)
{
    if (open_map) { open_map = false; return; }
    if (worldmap_open()) { open_map = true; if (open_menu) ui_toggle(); open_map = true; }
}

extern "C" int overlay_click(int x, int y), overlay_click_up(int x, int y);
extern "C" int margins_click(int x, int y);           /* ui/margins.cpp: the battle's order buttons */
static int margin_click_taken, outside_down;
extern "C" int plat_win_outside(int wx, int wy);
extern "C" { extern int loot_active, barter_active; int loot_enabled(void); }
extern "C" int plat_controller_connected(void);
extern "C" int ui_event(const SDL_Event *e)
{
    if (open_mt32 && !open_welcome) {       /* the MT-32 recommendation, as the welcome */
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (open_welcome) {                     /* the first start's word of welcome takes every event until closed */
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (figpick_is_open()) {                /* the figure picker (ui/figures.cpp) takes every event until it closes */
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12 && !(e->key.keysym.mod & KMOD_CTRL)) { ui_toggle(); return 1; }
    if (!open_menu && !open_map && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_m && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_SHIFT))
        && (game_mode() == GM_MAP || game_mode() == GM_CARD) && config_get_int("map.key_m", 1)) { ui_toggle_map(); return 1; }
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F1 && !e->key.repeat && !open_menu && !open_map && !open_goto) { open_help = !open_help; return 1; }
    if (open_help) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_F1)) { open_help = false; return 1; }
        if (e->type == SDL_MOUSEBUTTONDOWN) { open_help = false; return 1; }
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (halloffame_is_open()) {             /* the Honor Roll of Glory takes every event until closed */
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12) { halloffame_close(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    /* Tab in a battle or dungeon: the level map (ui/levelmap.cpp) */
    if (!open_menu && !open_map && !open_goto && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_TAB && !e->key.repeat
        && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) && (levelmap_is_open() || levelmap_available())) { levelmap_toggle(); return 1; }
    if (levelmap_is_open()) {
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE) { levelmap_toggle(); return 1; }
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12) { levelmap_toggle(); ui_toggle(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (!open_menu && !open_map && !open_goto && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_g && !e->key.repeat
        && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_SHIFT)) && goto_allowed()) { open_goto = true; return 1; }
    if (open_goto) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_g)) { open_goto = false; return 1; }
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12) { open_goto = false; ui_toggle(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (open_map) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || (e->key.keysym.sym == SDLK_m && !ImGui::GetIO().WantTextInput))) { ui_toggle_map(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (!open_menu) {
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT && overlay_click(e->button.x, e->button.y)) return 1;
        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && overlay_click_up(e->button.x, e->button.y)) return 1;
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT && margins_click(e->button.x, e->button.y)) { margin_click_taken = 1; return 1; }
        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && margin_click_taken) { margin_click_taken = 0; return 1; }
        /* a left click beside the picture (on the margins) is not the game's: its pointer waits at
           the picture's edge, where a click would walk the party or the fighters there */
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT && game_mode() != GM_OTHER && plat_win_outside(e->button.x, e->button.y)) { outside_down = 1; return 1; }
        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && outside_down) { outside_down = 0; return 1; }
        return 0;
    }
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
extern "C" int encounters_count(void);
extern "C" int encounters_errors(void);
static void tab_mods(void)
{
    ImGui::PushFont(font_head); ImGui::SeparatorText("Mod folders"); ImGui::PopFont();
    {
        static char msg[1400];
        char md[1100]; datamods_folder(md, sizeof md);
        extern char dlp_user_dir[];
        ImGui::TextWrapped("A mod is a folder in %s" PATH_SEP "mods with files laid out like the game folder: "
                           "while it is ticked the game reads them instead of its own (the game's files are never changed).", dlp_user_dir);
        if (!datamods_n) hint("No mods found yet.");
        for (int i = 0; i < datamods_n; i++) {
            ImGui::PushID(1000 + i);
            bool on = datamods[i].on != 0;
            if (ImGui::Checkbox(datamods[i].title, &on)) { datamods_set(i, on); dirty_config = 1; }
            ImGui::Indent(UIS(34));
            if (datamods[i].desc[0]) hint("%s", datamods[i].desc);
            { std::string fp = datamods[i].path; for (char &c : fp) if (c == '/' || c == '\\') c = PATH_SEP[0]; hint("folder: %s", fp.c_str()); }
            ImGui::Unindent(UIS(34));
            ImGui::PopID();
        }
        {   /* mods with ENCOUNTERS folders (mods/encounters.c) */
            int ne = encounters_count(), nerr = encounters_errors();
            if (ne || nerr) {
                ImGui::Spacing();
                ImGui::TextWrapped("New encounters in use: %d%s", ne, nerr ? "" : ".");
                if (nerr) { ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.6f, 0.4f, 1), "(%d file%s with errors left out: see darklands_port.log)", nerr, nerr == 1 ? "" : "s"); }
                ImGui::Indent(UIS(34));
                int share = config_get_int("encounters.share", 100);
                ImGui::SetNextItemWidth(UIS(220));
                if (ImGui::SliderInt("How often they come up##encshare", &share, 0, 300, "%d %%")) { config_set_int("encounters.share", share); dirty_config = 1; }
                hint("100 %%: each counts as one more entry (or as many as its weight) of the game's own table of encounters for the terrain.");
                bool seal = config_get_int("encounters.seal", 1) != 0;
                if (ImGui::Checkbox("Mark them with a seal in the corner of the card", &seal)) { config_set_int("encounters.seal", seal); dirty_config = 1; }
                ImGui::Unindent(UIS(34));
            }
        }
        if (ImGui::Button("Look for mods again")) { datamods_scan(); hdart_rescan(); msg[0] = 0; }
        ImGui::SameLine();
        static char newname[64] = "My mod";
        ImGui::SetNextItemWidth(UIS(200)); ImGui::InputText("##newmod", newname, sizeof newname);
        ImGui::SameLine();
        if (ImGui::Button("Make a new mod folder") && newname[0]) {
            char w[1300]; int r = datamods_new(newname, w, sizeof w);
            snprintf(msg, sizeof msg, r > 0 ? "Made %s (with a mod.txt that explains what goes where)." : r == 0 ? "%s is there already." : "Could not make %s.", w);
        }
        if (ImGui::Button("Export the card texts")) {
            char w[1300]; int r = datamods_export_cards(w, sizeof w);
            if (r >= 0) snprintf(msg, sizeof msg, "%d card files written to %s as text. Copy the ones you change into your mod's MSGFILES folder.", r, w);
            else snprintf(msg, sizeof msg, "Could not read MSGFILES.");
        }
        ImGui::SameLine(); hint("every town and encounter text, editable, with its options");
        { extern int ui_collect_texts; bool ct = ui_collect_texts != 0;
          if (ImGui::Checkbox("Collect the texts shown (for a translation: mods" PATH_SEP "_export" PATH_SEP "texts_seen.txt)", &ct)) ui_collect_texts = ct; }
        hint("A mod's text.txt says lines differently, one per line: Purchase an item => Kaufen (menus the game writes from its program; needs crisp text).");
        if (msg[0]) ImGui::TextWrapped("%s", msg);
        hint("Mods are applied in alphabetical order of their folders: where two change the same file, the later one wins. "
             "Single files inside MSGFILES and the .CAT catalogues can be replaced by a folder of that name holding just those files. "
             "Card texts and pictures change at once; the item list, saints, cities and the like when the game starts.");
    }
    ImGui::PushFont(font_head); ImGui::SeparatorText("Built-in changes"); ImGui::PopFont();
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
        ImGui::Indent(UIS(34));
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextWrapped("%s%s", m.desc, m.needs_new_game ? "  (New game.)" : "");
        ImGui::PopStyleColor();
        ImGui::Unindent(UIS(34));
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
        config_set_int("original.battle_speed", battle_speed);
        battle_speed_set(100);
        config_set_int("original.card_help", assist_card_help);
        assist_set_card_help(0);
        config_set_int("original.card_keys", assist_card_keys);
        assist_set_card_keys(0);
        config_set_int("original.pause_down", autopause_down); config_set_int("original.pause_hurt", autopause_hurt);
        autopause_set(0, 0);
        { extern int ui_margins_on; config_set_int("original.margins", ui_margins_on); ui_margins_on = 0; config_set_int("display.margins", 0); }
        for (int i = 0; i < FIX_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", FIXES[i].on);
        for (int i = 0; i < MOD_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", MODS[i].enabled);
        config_set("original.saved", b);
        config_set("original.sound", config_get("sound.device", "auto"));
        config_set_int("original.text", hdtext_mode);
        config_set_int("original.art", hdart_on);
        config_set_int("original.sfx", sfx_mode);
        config_set_int("original.saves", saves_on);
        saves_enable(0);
        assist_set_chances(0); journal_set(0); pathfix_set(0, 0); travel_set(0, travel_fast); hdtext_set(0); hdart_set(0); sfx_set(0);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, 0);
        for (int i = 0; i < MOD_COUNT; i++) if (MODS[i].enabled) mod_enable(i, 0);
        config_set("sound.device", "game");
        config_set_int("original", 1);
        datamods_scan(); hdart_rescan(); rules_apply(); sfx_reload(); musicpack_reload();
    } else {
        const char *p = config_get("original.saved", "");
        char *e;
        auto next = [&](int def) { long v = strtol(p, &e, 10); if (e == p) return def; p = e; return (int)v; };
        int ch = next(2), jo = next(1), pp = next(1), pe = next(0), ta = next(1), tf = next(1);
        assist_set_chances(ch); journal_set(jo); pathfix_set(pp, pe); travel_set(ta, tf);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, next(1));
        for (int i = 0; i < MOD_COUNT; i++) if (next(0)) mod_enable(i, 1);
        config_set("sound.device", config_get("original.sound", "auto"));
        hdtext_set(config_get_int("original.text", 2));
        hdart_set(config_get_int("original.art", 1));
        sfx_set(config_get_int("original.sfx", 1));
        saves_enable(config_get_int("original.saves", 1));
        battle_speed_set(config_get_int("original.battle_speed", 100));
        assist_set_card_help(config_get_int("original.card_help", 1));
        assist_set_card_keys(config_get_int("original.card_keys", 1));
        autopause_set(config_get_int("original.pause_down", 1), config_get_int("original.pause_hurt", 0));
        { extern int ui_margins_on; ui_margins_on = config_get_int("original.margins", 1); config_set_int("display.margins", ui_margins_on); }
        config_set_int("original", 0);
        datamods_scan(); hdart_rescan(); rules_apply(); sfx_reload(); musicpack_reload();
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

    ImGui::PushFont(font_head); ImGui::SeparatorText("Cards, screens and lists"); ImGui::PopFont();
    { bool pp = partypanel_is_on() != 0;
      if (ImGui::Checkbox("The party panel drawn by Darklands+: each member's Endurance, Strength and Divine Favor as labelled bars with their numbers", &pp)) { partypanel_set(pp); dirty_config = 1; }
      ImGui::Indent(UIS(34));
      hint("Clicks on it still go to the game (a member's sheet; orders in a battle). A portrait of your own: put NAME.png (or .jpg) in a folder named portraits in the Darklands+ folder, NAME being the member's name. The battle figure is the little one the game shows beside a battle, in the colours chosen for the member.");
      int pic = partypanel_picture();
      static const char *PICS[] = { "Your portrait, else the battle figure, else the herald", "The battle figure, else the herald", "The herald", "No picture" };
      ImGui::SetNextItemWidth(UIS(360));
      if (ImGui::Combo("Picture##ppic", &pic, PICS, 4)) { partypanel_set_picture(pic); dirty_config = 1; }
      ImGui::Unindent(UIS(34)); }
    { bool nm = menuview_is_on() != 0;
      if (ImGui::Checkbox("The game's menus (Game, Orders, Attack, Party ...) drawn by Darklands+: a right click or F10 opens them, a click chooses", &nm)) { menuview_set(nm); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("They stay open until you choose or click elsewhere, instead of only while the right button is held (letting go of it over an entry still chooses that entry). The entries and what they do are the game's own. Off: the game's own menu bar."); ImGui::Unindent(UIS(34)); }
    { bool nc2 = cacheview_is_on() != 0;
      if (ImGui::Checkbox("Store rooms drawn by Darklands+: everything stored and everything the member carries in two lists", &nc2)) { cacheview_set(nc2); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Take and Put on each row (one or the whole stack), Put away for everything not in use. The storing is the game's own. Off: the game's screen, four things at a time."); ImGui::Unindent(UIS(34)); }
    { bool ncr = createview_is_on() != 0;
      if (ImGui::Checkbox("Making a character drawn by Darklands+: each step on one page, with what a childhood or profession would bring", &ncr)) { createview_set(ncr); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The attributes after it (and the aging past thirty), each skill's gain and room, the virtues and saints learned. The rules are the game's own. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool nro = rosterview_is_on() != 0;
      if (ImGui::Checkbox("Choosing the party of a new game drawn by Darklands+: the four places with their figures, the characters kept, the chosen one's numbers", &nro)) { rosterview_set(nro); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Click to choose, double-click to add or drop; the figure, herald and colours beside the chosen member. Adding, dropping and deleting are the game's own. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool wf = config_get_int("party.new_member_looks", 1) != 0;
      ImGui::Indent(UIS(34));
      if (ImGui::Checkbox("Each member added gets a look of their own", &wf)) { config_set_int("party.new_member_looks", wf); dirty_config = 1; }
      hint("The game gives everyone added the same figure (the tall man) in the same colours, so they are hard to tell apart in a battle. This gives a woman the woman figure and each member the next colours (Figure and Colours change them as before)."); ImGui::Unindent(UIS(34)); }
    { bool ni = infoview_is_on() != 0;
      if (ImGui::Checkbox("Party Info drawn by Darklands+: fame and its title, the time, where, the wealth, and the reputation in every city", &ni)) { infoview_set(ni); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The original names one town at a time under the pointer on its little map; this lists all of them, by reputation or by name. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool nmo = marchview_is_on() != 0;
      if (ImGui::Checkbox("Change Marching Order as a list drawn by Darklands+: move members up and down, then Done", &nmo)) { marchview_set(nmo); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Instead of clicking each member in turn. The order is the game's own (single file follows it). Off: the game's way."); ImGui::Unindent(UIS(34)); }
    { bool np = prayview_is_on() != 0;
      if (ImGui::Checkbox("Calling on a saint drawn by Darklands+: the statue, what the saint does, the favor to spend and the chance", &np)) { prayview_set(np); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The chance and the favor are the saint's own rules. Off: the game's prayer screen."); ImGui::Unindent(UIS(34)); }
    { bool nv = savesview_is_on() != 0;
      if (ImGui::Checkbox("The game's Save and Load drawn by Darklands+: the saves listed with where, when and who", &nv)) { savesview_set(nv); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Save asks for the comment on a page that lists the saves there are; Load lists them all to choose from (no question first). The saving and loading are the game's own. Off: the game's comment prompt and its list of nine."); ImGui::Unindent(UIS(34)); }
    { bool nc = cardview_on != 0;
      if (ImGui::Checkbox("Cards drawn by Darklands+: their text in the chosen typeface, the options as rows to click, chances beside them", &nc)) { cardview_set(nc); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The game still decides everything on a card; only the text and the choosing are new. An option that uses an item or calls on a saint lists, under it, who can. Off: the game draws its cards itself (with the crisp text over them)."); ImGui::Unindent(UIS(34)); }
    { bool ns = barterview_is_on() != 0;
      if (ImGui::Checkbox("Shops drawn by Darklands+: all the wares and all your things in two lists, Buy and Sell on each, what is in use marked", &ns)) { barterview_set(ns); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The merchant's stock and prices are the game's own. The bottom of the screen describes whatever the pointer is on (no tooltips over the lists). Off: the game's own shop screen."); ImGui::Unindent(UIS(34)); }
    { bool sv = sheetview_is_on() != 0;
      if (ImGui::Checkbox("Character sheets drawn by Darklands+: the paper doll, the things with Ready, Unequip, Drink and Drop, the skills on one page, the party's saints and formulae", &sv)) { sheetview_set(sv); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Drag a thing onto a member in the party panel to give it (Shift: one of a stack); a drop can be undone while the sheet is open. Equipping, giving, dropping and drinking are the game's own. Off: the game's own sheet."); ImGui::Unindent(UIS(34)); }
    { bool lv = lootview_is_on() != 0;
      if (ImGui::Checkbox("The loot drawn by Darklands+: the whole pile and each member's things in two lists, a whole stack in one click", &lv)) { lootview_set(lv); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The game's own G and P still move every thing. Off: the game's own loot screen."); ImGui::Unindent(UIS(34)); }
    { bool av = alchemyview_is_on() != 0;
      if (ImGui::Checkbox("Alchemy drawn by Darklands+: every known formula in one list, its ingredients at hand, batches with - and +, the chance of success", &av)) { alchemyview_set(av); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The chance and the mixing are the game's own. Off: the game's own alchemy screen."); ImGui::Unindent(UIS(34)); }
    bool ch = assist_card_help;
    if (ImGui::Checkbox("Show which saints and items work on a card, and who knows or carries them", &ch)) { assist_set_card_help(ch); dirty_config = 1; }
    ImGui::Indent(UIS(34)); hint("Like the Darklands Companion: the saints a card accepts are listed even when nobody in the party knows them yet."); ImGui::Unindent(UIS(34));
    { extern int ui_shop_info; bool si = ui_shop_info != 0;
      if (ImGui::Checkbox("Tooltips: what the item, saint or formula under the mouse is good for (shops, loot, character sheet, saints, formulas)", &si)) { ui_shop_info = si; config_set_int("assist.shop_info", si); dirty_config = 1; } }
    { bool rt = config_get_int("assist.return_thrown", 1) != 0;
      if (ImGui::Checkbox("After a battle, javelins, throwing axes, knives and darts go back to whoever threw them", &rt)) { config_set_int("assist.return_thrown", rt); loot_return_thrown = rt; dirty_config = 1; } }
    { bool ia = ammo_infinite != 0;
      if (ImGui::Checkbox("Arrows, quarrels and balls never run out (one in the pack is enough to shoot)", &ia)) { ammo_set(ia); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("The party's shots take nothing from the pack and leave nothing to pick up after the battle. Enemies still shoot and lose theirs. Off: each shot uses one, and the game leaves 7 in 10 arrows and quarrels to be found in the loot."); ImGui::Unindent(UIS(34)); }
    { bool la = config_get_int("assist.loot_all", 1) != 0;
      if (ImGui::Checkbox("Loot and shops: take the whole pile (A) or share it out (S); sell a whole stack (A), buy ten (T)", &la)) { config_set_int("assist.loot_all", la); dirty_config = 1; } }
    { extern int ui_creation_guide; bool cg = ui_creation_guide != 0;
      if (ImGui::Checkbox("Character creation: a short guide to each step in the empty column on the left", &cg)) { ui_creation_guide = cg; config_set_int("assist.creation_guide", cg); dirty_config = 1; } }
    { bool mf = config_get_int("party.more_figures", 1) != 0;
      if (ImGui::Checkbox("Character creation: Select Character Image also offers the enemies' human figures (sergeant, monk, knight, Templar ... 14 more)", &mf)) { config_set_int("party.more_figures", mf); dirty_config = 1; }
      ImGui::Indent(UIS(34)); hint("Their own walking, fighting and dying animations, with colours of their own for the 1st/2nd/3rd color keys. A figure without an animation for the weapon in hand swings the nearest one it has. Saves keep the figure; the original game cannot show these figures."); ImGui::Unindent(UIS(34));
      if (mf) { ImGui::Indent(UIS(34)); bool fp = config_get_int("party.figure_picker", 1) != 0;
        if (ImGui::Checkbox("Select Character Image shows them all at once to pick from (else it steps to the next, as in the original)", &fp)) { config_set_int("party.figure_picker", fp); dirty_config = 1; }
        ImGui::Unindent(UIS(34)); } }
    { bool ff = config_get_int("assist.fast_fades", 1) != 0;
      if (ImGui::Checkbox("Cards show their options at once (no slow fade-in)", &ff)) { config_set_int("assist.fast_fades", ff); dirty_config = 1; } }
    bool ck = assist_card_keys;
    if (ImGui::Checkbox("Number keys 1-9 pick a card's options", &ck)) { assist_set_card_keys(ck); dirty_config = 1; }
    { bool wl = config_get_int("assist.wheel_lists", 1) != 0;
      if (ImGui::Checkbox("The mouse wheel moves in the game's lists (equipment, shops ...)", &wl)) { config_set_int("assist.wheel_lists", wl); dirty_config = 1; } }
    { bool gp = config_get_int("input.gamepad", 1) != 0;
      if (ImGui::Checkbox("Game controller: the left stick moves the pointer, A clicks, B right-clicks (F1 lists the rest)", &gp)) { config_set_int("input.gamepad", gp); dirty_config = 1; }
      hint(plat_controller_connected() ? "A controller is connected." : "No controller connected now."); }
    { bool mc = config_get_int("assist.menu_click", 1) != 0;
      if (ImGui::Checkbox("A right click opens the menu bar (the next click chooses; Esc closes it)", &mc)) { config_set_int("assist.menu_click", mc); dirty_config = 1; }
      if (ImGui::IsItemHovered()) ImGui::SetTooltip("The game shows its menus only while the right button is held down, and takes the entry under the pointer when\nit is let go. That still works; this adds the click. F10 opens the menus from the keyboard (arrows, Enter, Esc)."); }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Battles"); ImGui::PopFont();
    { bool d = autopause_down, h = autopause_hurt;
      if (ImGui::Checkbox("Pause the battle when someone in the party falls", &d)) { autopause_set(d, h); dirty_config = 1; }
      if (ImGui::Checkbox("Pause the battle when someone is badly hurt (endurance below a quarter)", &h)) { autopause_set(d, h); dirty_config = 1; } }
    { extern int ui_battle_info, ui_battle_numbers; bool bi = ui_battle_info != 0, bn = ui_battle_numbers != 0;
    { bool ca = orders_click_attack;
      if (ImGui::Checkbox("In a battle, a click on an enemy sends the selected member at it (Std Attack), even before an attack is chosen", &ca)) { orders_click_attack = ca; config_set_int("battle.click_attack", ca); dirty_config = 1; }
      { bool sp = battlescroll_on != 0, sr = battlescroll_rest != 0;
        if (ImGui::Checkbox("Battle map: scroll at a steady pace (Shift at the edge no longer flies to the end of the field)", &sp)) { battlescroll_set(sp, sr); dirty_config = 1; }
        if (sp) { ImGui::SameLine(); if (ImGui::Checkbox("the pointer resting on the edge scrolls, no Shift needed", &sr)) { battlescroll_set(sp, sr); dirty_config = 1; } } } }
      if (ImGui::Checkbox("Pointing at someone in a battle shows who it is, how hurt, attack, defence and weapon", &bi)) { ui_battle_info = bi; config_set_int("battle.info", bi); dirty_config = 1; }
      { extern int ui_battle_log; bool bl = ui_battle_log != 0;
        if (ImGui::Checkbox("Battle log: every blow, parry, wound and fall, in the left margin on a wide screen, else over the field (F12 > Journal keeps it)", &bl)) { ui_battle_log = bl; config_set_int("battle.log", bl); dirty_config = 1; } }
      if (bi) { ImGui::SameLine(); if (ImGui::Checkbox("enemies' endurance as numbers", &bn)) { ui_battle_numbers = bn; config_set_int("battle.info_numbers", bn); dirty_config = 1; } } }
    { bool lm = config_get_int("assist.level_map", 1) != 0, la = config_get_int("assist.level_map_all", 0) != 0;
      if (ImGui::Checkbox("Tab in battles, castles, mines and dungeons: a map of the level as far as the party has seen it", &lm)) { config_set_int("assist.level_map", lm); dirty_config = 1; }
      if (lm) { ImGui::SameLine(); if (ImGui::Checkbox("the whole level", &la)) { config_set_int("assist.level_map_all", la); dirty_config = 1; } } }
    bool pp = pathfix_party, pe = pathfix_enemies;
    if (ImGui::Checkbox("Better pathfinding for the party (no more getting stuck in mines)", &pp)) { pathfix_set(pp, pe); dirty_config = 1; }
    if (ImGui::Checkbox("Better pathfinding for enemies too (harder battles)", &pe)) { pathfix_set(pp, pe); dirty_config = 1; }
    hint("Moves are planned around walls and furniture with a full route search instead of the "
                        "original wall-following, which gives up on long or winding routes.");
    {
        ImGui::TextUnformatted("Speed of battles and dungeons:"); ImGui::SameLine();
        static const int SPD[4] = { 100, 150, 200, 300 }; static const char *SPN[4] = { "as the original", "1.5x", "2x", "3x" };
        for (int i = 0; i < 4; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(SPN[i], battle_speed == SPD[i])) { battle_speed_set(SPD[i]); dirty_config = 1; }
        }
        hint("Walking (also through dungeons, castles and mines), attacks and spells all go faster; pausing (Space) works as before.");
    }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Travel"); ImGui::PopFont();
    bool ta = travel_enabled; int tf = travel_fast;
    if (ImGui::Checkbox("Auto-travel along roads (Travel tab, or Ctrl+click on the map)", &ta)) { travel_set(ta, tf); dirty_config = 1; }
    bool cg = config_get_int("assist.city_goto", 1);
    if (ImGui::Checkbox("Go to in cities: G on a city's card lists its places; the party walks there through the menus", &cg)) { config_set_int("assist.city_goto", cg); dirty_config = 1; }
    { bool mn = config_get_int("map.names", 1) != 0;
      if (ImGui::Checkbox("Names of the towns and places on the travel map", &mn)) { config_set_int("map.names", mn); dirty_config = 1; } }
    {   /* the game's rate of encounters on the road (mods/travel.c) */
        static const int P[] = { 25, 50, 100, 200 };
        static const char *N[] = { "rare (a quarter)", "fewer (half)", "as the game", "more (double)" };
        int cur = config_get_int("travel.encounters", 100), sel = 2;
        for (int i = 0; i < 4; i++) if (P[i] == cur) sel = i;
        ImGui::SetNextItemWidth(UIS(200));
        if (ImGui::Combo("encounters on the road", &sel, N, 4)) { config_set_int("travel.encounters", P[sel]); dirty_config = 1; }
    }
    ImGui::TextUnformatted("Pace of auto-travel:"); ImGui::SameLine();
    if (ImGui::RadioButton("as walking", tf == 0)) { travel_set(ta, 0); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("4x", tf == 1)) { travel_set(ta, 1); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("at once", tf == 2)) { travel_set(ta, 2); dirty_config = 1; }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Town journal"); ImGui::PopFont();
    bool j = journal_enabled();
    if (ImGui::Checkbox("Keep a journal of the towns the party visits", &j)) { journal_set(j); dirty_config = 1; }
    hint("Each town's shops, their wares, qualities and prices, noted on arrival (Journal tab).");
    if (j) {
        bool ch = config_get_int("journal.chronicle", 1) != 0;
        if (ImGui::Checkbox("Keep a chronicle of every text the cards show", &ch)) { config_set_int("journal.chronicle", ch); dirty_config = 1; }
        hint("Word for word, with the date, the place and the choice made: a clue read weeks ago can be found again (Journal > Chronicle).");
    }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Editor"); ImGui::PopFont();
    bool ed = config_get_int("editor.show", 0);
    if (ImGui::Checkbox("Show the character editor (Editor tab)", &ed)) { config_set_int("editor.show", ed); dirty_config = 1; }
    hint("Change money, names, attributes, skills, saints, formulas, items, and a character's colours, shield and "
         "figure, as the community's save-game editors do. For repairing characters hit by old bugs, or for cheating.");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Fixes for bugs of the original"); ImGui::PopFont();
    for (int i = 0; i < FIX_COUNT; i++) {
        bool on = FIXES[i].on;
        ImGui::PushID(i);
        if (ImGui::Checkbox(FIXES[i].name, &on)) { fix_enable(i, on); dirty_config = 1; }
        ImGui::Indent(UIS(34)); hint("%s", FIXES[i].desc); ImGui::Unindent(UIS(34));
        ImGui::PopID();
    }
}

extern "C" { int mpu_out_count(void); const char *mpu_out_name(int i); int mt32_families(void); int mt32_switch_pending(void); int mt32_switch(void); const char *mt32_status(void); void mt32_rescan(void); int mt32_dir_count(void); const char *mt32_dir(int); extern int sound_music_volume; }
static void tab_display(void)
{
    bool s = ui_settings.smooth, n = ui_settings.integer;   /* (the picture is always 4:3, as on a 1992 monitor) */
    if (ImGui::Checkbox("Smooth scaling", &s)) { ui_settings.smooth = s; config_set_int("display.smooth", s); dirty_config = 1; }
    if (ImGui::Checkbox("Whole-number scaling (sharpest pixels)", &n)) { ui_settings.integer = n; config_set_int("display.integer_scale", n); dirty_config = 1; }
    {
        ImGui::TextUnformatted("Pixel edges:"); ImGui::SameLine();
        static const char *SC[3] = { "as drawn", "smoothed (Scale2x)", "smoothed more (Scale4x)" };
        for (int i = 0; i < 3; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(SC[i], ui_settings.scaler == i)) { ui_settings.scaler = i; config_set_int("display.scaler", i); dirty_config = 1; }
        }
        hint("Rounds off the stair-steps of diagonal edges in the pictures and figures without blurring them "
             "(the EPX/Scale2x rule: a pixel's corner takes its neighbours' colour where two of them agree). "
             "Scale4x needs a graphics card: without one (a virtual machine, say) the game slows down.");
    }
    { extern int ui_margins_on; bool mg = ui_margins_on != 0;
      if (ImGui::Checkbox("Wide screens: date, place, purse, tasks, the town and effects in the side margins (in battles: the log and everyone's condition)", &mg)) { ui_margins_on = mg; config_set_int("display.margins", mg); dirty_config = 1; } }
    {   /* grows with the picture; this scales it */
        static const int P[] = { 70, 80, 90, 100, 115, 130, 150, 175, 200 };
        int cur = config_get_int("display.text_size", 100), sel = 3;
        for (int i = 0; i < 9; i++) if (P[i] == cur) sel = i;
        char lab[16]; snprintf(lab, sizeof lab, "%d %%", P[sel]);
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::BeginCombo("text size in the margins, tooltips and on the map", lab)) {
            for (int i = 0; i < 9; i++) { char t[16]; snprintf(t, sizeof t, "%d %%", P[i]); if (ImGui::Selectable(t, i == sel)) { config_set_int("display.text_size", P[i]); dirty_config = 1; } }
            ImGui::EndCombo();
        }
    }
    { bool sc = sys_cursor != 0;
      if (ImGui::Checkbox("Show the system mouse pointer over the game too", &sc)) { sys_cursor = sc; config_set_int("display.system_cursor", sc); dirty_config = 1; } }
    {   /* the menus' size (fonts are baked at start-up) */
        static const int P[] = { 0, 100, 125, 150, 175, 200, 250 };
        static const char *N[] = { "automatic (the system's scale)", "100 %", "125 %", "150 %", "175 %", "200 %", "250 %" };
        int cur = config_get_int("display.ui_scale", 0), sel = 0;
        for (int i = 0; i < 7; i++) if (P[i] == cur) sel = i;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::Combo("size of these menus (after a restart)", &sel, N, 7)) { config_set_int("display.ui_scale", P[sel]); dirty_config = 1; }
    }
    { bool ir = idle_rest != 0;
      if (ImGui::Checkbox("Let the computer rest while the game waits for a click or a key (less CPU, quieter fans)", &ir)) { idle_rest = ir; config_set_int("display.idle_rest", ir); dirty_config = 1; } }
    { bool pu = config_get_int("display.pause_unfocused", 1) != 0;
      if (ImGui::Checkbox("Pause the game while its window is in the background", &pu)) { config_set_int("display.pause_unfocused", pu); dirty_config = 1; } }
    { bool po = config_get_int("display.play_opening", 0);
      if (ImGui::Checkbox("Play the opening (logo and animations) at every start; otherwise only the first time", &po)) { config_set_int("display.play_opening", po); dirty_config = 1; } }
    bool fs = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(g_win, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    ImGui::PushFont(font_head); ImGui::SeparatorText("Text"); ImGui::PopFont();
    int hm = hdtext_mode;
    if (ImGui::RadioButton("Crisp text on story cards only", hm == 1)) { hdtext_set(1); dirty_config = 1; }
    if (ImGui::RadioButton("Crisp text everywhere (cards, party, sheets, menus, shops)", hm == 2)) { hdtext_set(2); dirty_config = 1; }
    if (ImGui::RadioButton("The game's own lettering", hm == 0)) { hdtext_set(0); dirty_config = 1; }
    if (hm) {
        ImGui::SetNextItemWidth(UIS(380));
        if (ImGui::BeginCombo("Typeface", UI_GAME_FONT_NAMES[ui_game_font])) {
            for (int i = 0; i < UI_GAME_FONTS; i++) {
                ImGui::PushFont(ui_font_game[i]);
                if (ImGui::Selectable(UI_GAME_FONT_NAMES[i], i == ui_game_font)) { ui_game_font = i; config_set_int("display.text_font", i); overlay_font_update(); dirty_config = 1; }
                ImGui::PopFont();
            }
            ImGui::EndCombo();
        }
        { bool of = config_get_int("display.overlay_font_follow", 1) != 0;
          if (ImGui::Checkbox("The same typeface for the side panels, tooltips and names on the maps", &of)) { config_set_int("display.overlay_font_follow", of); overlay_font_update(); dirty_config = 1; } }
        { bool so = ui_spell_out != 0;
          if (ImGui::Checkbox("Spell out the character sheet's abbreviations (wPol -> Polearm Wpns, R&W -> Read & Write)", &so)) { ui_spell_out = so; config_set_int("display.spell_out", so); dirty_config = 1; } }
        ImGui::TextUnformatted("Letter edges:"); ImGui::SameLine();
        static const char *E[3] = { "plain", "shadow", "outline" };
        for (int i = 0; i < 3; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(E[i], ui_text_edge == i)) { ui_text_edge = i; config_set_int("display.text_edge", i); dirty_config = 1; }
        }
        { extern int ui_skill_icons; bool si = ui_skill_icons != 0;
          if (ImGui::Checkbox("Small pictures beside the skills of the character sheet", &si)) { ui_skill_icons = si; config_set_int("display.skill_icons", si); dirty_config = 1; } }
        { extern int ui_text_contrast; bool tc = ui_text_contrast != 0;
          if (ImGui::Checkbox("Darken (or lighten) faint text against its background", &tc)) { ui_text_contrast = tc; config_set_int("display.text_contrast", tc); dirty_config = 1; } }
    }
    hint("The original 7-pixel blackletter is hard to read when scaled up. Crisp text draws the same words at "
         "the window's resolution; anything it cannot match exactly is left as the game drew it.");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Pictures"); ImGui::PopFont();
    bool ha = hdart_on;
    if (ImGui::Checkbox("Use replacement art from the \"art\" folder", &ha)) { hdart_set(ha); dirty_config = 1; }
    {
        extern char game_dir[1024];
        int n = hdart_count();
        extern char dlp_user_dir[];
        if (n) hint("%d replacement picture%s in %s%sart.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
        else hint("No replacement pictures yet (%s%sart).", dlp_user_dir, PATH_SEP);
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
        hint("More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the fifteen of the game "
             "(Heraldry, when making a party). A character with one of them needs this version of the game to be loaded.", PATH_SEP);
    }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Sound"); ImGui::PopFont();
    static const char *ids[] = { "auto", "mt32", "fm", "adlib", "midi", "speaker", "off", "game" };
    static const char *labels[] = {
        "Automatic: the Roland MT-32 when its ROM files are found, otherwise the Sound Blaster - recommended",
        "Roland MT-32 / LAPC-I, emulated (Munt; needs the MT-32 or CM-32L ROM files)",
        "Sound Blaster Pro (OPL-3 FM, emulated)",
        "AdLib / Sound Blaster (OPL-2 FM, emulated)",
        "Roland MIDI to your system's MIDI device (a real MT-32, or Munt installed as a device)",
        "PC speaker",
        "Off",
        "As in the game's own setup (INSTALL)" };
    const char *cur = config_get("sound.device", "auto");
    for (int i = 0; i < 8; i++)
        if (ImGui::RadioButton(labels[i], !strcmp(cur, ids[i]))) { config_set("sound.device", ids[i]); dirty_config = 1; }
    hint("Takes effect after the next battle, or when the game is started again.");
    if (!strcmp(cur, "midi")) {                     /* which MIDI output */
        ImGui::Indent(UIS(30));
        int n = mpu_out_count();
        const char *out = config_get("sound.midi_out", "");
        if (!n) hint("No MIDI outputs on this system (Windows lists them here).");
        else {
            ImGui::SetNextItemWidth(UIS(420));
            if (ImGui::BeginCombo("MIDI output", out[0] ? out : "Windows' default")) {
                if (ImGui::Selectable("Windows' default", !out[0])) { config_set("sound.midi_out", ""); dirty_config = 1; }
                for (int i = 0; i < n; i++) {
                    const char *nm = mpu_out_name(i);
                    if (nm[0] && ImGui::Selectable(nm, !strcmp(nm, out))) { config_set("sound.midi_out", nm); dirty_config = 1; }
                }
                ImGui::EndCombo();
            }
            hint("A real MT-32 on a MIDI interface, or a synthesizer installed as a MIDI device (Munt's mt32emu driver, "
                 "a Sound Canvas ...). The game's Roland driver loads its instruments when it starts, so the choice takes "
                 "effect after the next battle, or at the next start.");
        }
        ImGui::Unindent(UIS(30));
    }
    {
        int v = sound_music_volume;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::SliderInt("Volume of the music", &v, 0, 150, "%d%%")) { sound_music_volume = v; config_set_int("sound.music_volume", v); dirty_config = 1; }
    }
    if (!strcmp(cur, "auto") || !strcmp(cur, "mt32")) {
        ImGui::Indent(UIS(30));
        hint("%s", mt32_status());
        static char romdir[1024]; static bool init;
        if (!init) { snprintf(romdir, sizeof romdir, "%s", config_get("sound.mt32_roms", "")); init = true; }
        ImGui::SetNextItemWidth(UIS(420));
        if (ImGui::InputTextWithHint("##mt32roms", "the folder with the ROM files, if they are elsewhere", romdir, sizeof romdir)) { config_set("sound.mt32_roms", romdir); dirty_config = 1; }
        ImGui::SameLine();
        if (ImGui::Button("Look again##mt32")) { mt32_rescan(); mt32_switch(); }
        if (mt32_families() == 3) {                /* both: which one */
            const char *cur = config_get("sound.mt32_model", "mt32");
            ImGui::TextUnformatted("Both found, play on the");
            ImGui::SameLine(); if (ImGui::RadioButton("CM-32L / LAPC-I (its extra sound effects turn the battle sounds into chimes)", !strcmp(cur, "cm32l"))) { config_set("sound.mt32_model", "cm32l"); mt32_rescan(); mt32_switch(); dirty_config = 1; }
            ImGui::SameLine(); if (ImGui::RadioButton("MT-32 (recommended: battle sounds as blows and grunts)", !strcmp(cur, "mt32"))) { config_set("sound.mt32_model", "mt32"); mt32_rescan(); mt32_switch(); dirty_config = 1; }
        }
        {   bool lcd = config_get_int("sound.mt32_lcd", 1);
            if (ImGui::Checkbox("Show the synthesizer's display when the game writes to it", &lcd)) { config_set_int("sound.mt32_lcd", lcd); dirty_config = 1; } }
        if (ImGui::TreeNode("About the MT-32")) {
            ImGui::TextWrapped("Darklands was composed for the Roland MT-32 and LAPC-I: its Roland driver loads its own "
                               "instruments into the synthesizer and plays the music and the battle sounds with them. Munt's "
                               "emulator of the MT-32, CM-32L and LAPC-I is built into Darklands+, but it needs the synthesizer's "
                               "two ROM images (control and PCM), which belong to Roland and cannot come with Darklands+. If you "
                               "use Munt or DOSBox with an MT-32, you already have them: Darklands+ finds them by their contents "
                               "(any file names) in these folders, a CM-32L / LAPC-I pair first:");
            for (int i = 0; i < mt32_dir_count(); i++) hint("%s", mt32_dir(i));
            ImGui::TreePop();
        }
        ImGui::Unindent(UIS(30));
    }
    {
        ImGui::Spacing();
        bool sp = sfx_mode, hs = sfx_added_hits, sy = sfx_synth;
        extern char game_dir[1024];
        if (ImGui::Checkbox("Battle sounds from a sound pack (the \"sounds\" folder)", &sp)) { sfx_set(sp); dirty_config = 1; }
        int n = sfx_user_count();
        extern char dlp_user_dir[];
        if (n) hint("%d sound%s in %s%ssounds; the game's own sounds for the rest.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
        else hint("No sound pack in use: put one into %s%ssounds, or tick a sound mod in F12 > Mods (\"Battle sounds\" comes with Darklands+).", dlp_user_dir, PATH_SEP);
        if (sp) {
            ImGui::Indent(UIS(30));
            if (ImGui::Checkbox("Blows that land make a sound too (the original is silent there)", &hs)) { sfx_set_hits(hs); dirty_config = 1; }
            if (ImGui::Checkbox("Experimental: synthesized sounds where the pack has none", &sy)) { sfx_set_synth(sy); dirty_config = 1; }
            int v = sfx_volume;
            ImGui::SetNextItemWidth(UIS(260));
            if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); dirty_config = 1; }
            static char msg[400];
            if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sounds found.", sfx_user_count()); }
            ImGui::SameLine();
            if (ImGui::Button("Write the synthesized sounds as .wav")) sfx_export(msg, sizeof msg);
            if (msg[0]) hint("%s", msg);
            ImGui::Unindent(UIS(30));
        }
        if (ImGui::TreeNode("File names for a sound pack")) {
            for (int k = 0; k < SFX_COUNT; k++) hint("%s.wav or .ogg: %s", SFX_NAMES[k], SFX_DESC[k]);
            hint("Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random.");
            ImGui::TreePop();
        }
    }
    {   /* music packs (mods/musicpack.c) */
        ImGui::Spacing();
        bool mp = musicpack_on;
        extern char dlp_user_dir[];
        if (ImGui::Checkbox("Music from a music pack (recordings in a \"music\" folder)", &mp)) { musicpack_set(mp); dirty_config = 1; }
        int n = musicpack_count();
        if (n) {
            const char *f0 = ""; for (int k = 0; k < 60 && !f0[0]; k++) f0 = musicpack_from(k);
            hint("%d piece%s in the pack (%s); the synthesizer plays the others.", n, n == 1 ? "" : "s", strcmp(f0, "music") ? f0 : "the music folder");
        }
        else hint("No music pack: a mod's music folder, or %s%smusic, with a recording for each piece named by its number "
                  "(10.ogg, \"11 market.ogg\", .ogg or .wav). Darklands+ comes with none.", dlp_user_dir, PATH_SEP);
        int fp = 0, now = musicpack_now(&fp);
        if (now >= 0) {
            const char *d = musicpack_desc(now);
            hint("The last piece started: %d%s%s%s, from the %s.", now, d[0] ? " (" : "", d, d[0] ? ")" : "", fp ? "music pack" : "synthesizer");
        }
        if (ImGui::TreeNode("The pieces")) {
            hint("A recording replaces a piece when its file name starts with the number (up to four for one number, "
                 "played in turn: \"27 a.ogg\", \"27 b.ogg\" ...). The pieces that loop in the game "
                 "loop in the pack (the whole file again), the others play once. While a piece of the pack is on, the "
                 "synthesizer is silent; a piece the pack lacks is played by the synthesizer.");
            for (int k = 8; k <= 47; k++) {
                const char *f = musicpack_file(k), *d = musicpack_desc(k);
                const char *b = f + strlen(f); while (b > f && b[-1] != '/' && b[-1] != '\\') b--;
                hint("%2d  %s%s%s", k, d, d[0] ? ", " : "", musicpack_loops(k) ? "loops" : "plays once");
                if (f[0]) { ImGui::SameLine(); int nv = musicpack_versions(k); if (nv > 1) ImGui::Text("  %s and %d more, in turn", b, nv - 1); else ImGui::Text("  %s", b); }
            }
            hint("The pieces without a name are heard elsewhere in towns and on the road; the line above names each piece as it starts.");
            if (ImGui::SmallButton("Look again##music")) musicpack_reload();
            ImGui::TreePop();
        }
    }
    ImGui::TextWrapped("The GOG release plays everything through its Roland driver, which on a General MIDI synthesizer "
                       "sounds like \"random notes rather than clashing swords\": it is meant for an MT-32. With the MT-32's "
                       "ROM files Darklands+ plays it as composed; without them, the Sound Blaster setting uses the game's "
                       "own FM drivers and digitized effects, as most players heard it in 1992.");
}

static void item_name(int id, char *out, int n)
{
    u32 d = g_itemdef(id);
    if (!d || id <= 0) { snprintf(out, (size_t)n, "#%d", id); return; }
    g_str((u16)(d >> 16), (u16)d, out, n);
}

void rules_tab(void);
static void tab_party(void)
{
    if (!ImGui::BeginTabBar("records")) return;
    if (ImGui::BeginTabItem("Weapons")) { rules_tab(); ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("Items")) { void itemedit_tab(void); itemedit_tab(); ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("Enemies")) { void bestiary_tab(void); bestiary_tab(); ImGui::EndTabItem(); }
    if (!ImGui::BeginTabItem("The party's items")) { ImGui::EndTabBar(); return; }
    ImGui::TextWrapped("The party's possessions, read live from the game.");
    if (ImGui::BeginTabBar("members")) {
        for (int m = 0; m < 5; m++) {
            char name[32];
            g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), name, sizeof name);
            if (!name[0]) continue;
            if (ImGui::BeginTabItem(name)) {
                int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                if (n > 64) n = 64;
                if (ImGui::BeginTable("inv", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
                                      ImVec2(0, UIS(360)))) {
                    ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("id"); ImGui::TableSetupColumn("type"); ImGui::TableSetupColumn("quality");
                    ImGui::TableSetupColumn("count"); ImGui::TableSetupColumn("weight");
                    ImGui::TableHeadersRow();
                    std::vector<int> rows; for (int i = 0; i < n; i++) rows.push_back(i);
                    static TableSort ts[5];
                    auto key = [&](int i, int c) -> SortKey {
                        u16 r = G_ITEM(m, i); int id = rw(G_FAR2E38, r);
                        if (c == 0) { char nm[48]; item_name(id, nm, sizeof nm); return nm; }
                        if (c == 1) return id;
                        return (int)rb(G_FAR2E38, (u16)(r + c));
                    };
                    for (int i : table_order(ts[m], rows, key)) {
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
    ImGui::EndTabItem();
    ImGui::EndTabBar();
}

extern "C" const char dlp_version[];
static void tab_about(void)
{
    ImGui::PushFont(font_head); ImGui::Text("Darklands+"); ImGui::PopFont();
    ImGui::SameLine(); ImGui::TextDisabled("build %s", dlp_version);
    ImGui::TextWrapped("Darklands (c) 1992 MicroProse. This is the original game, recompiled to run natively, with the "
                       "game files of your own copy; everything Darklands+ adds can be switched off (Helpers > Play the original game).");
    ImGui::Spacing();
    ImGui::TextWrapped("F1: the keys of the moment.  F12: this menu (the game waits while it is open).  Alt+Enter: full screen.  "
                       "Ctrl+F12: screenshot.  Ctrl+S: quick save.  F9 or Ctrl+L: quick load.  M: world map.  G: go to (in cities).");
    ImGui::PushFont(font_head); ImGui::SeparatorText("Files"); ImGui::PopFont();
    {   extern char dos_shadow[];
        if (dos_shadow[0]) ImGui::BulletText("saved games, and every other file the game writes: %s (the game folder cannot be "
                                             "written; the saved games in its SAVES folder can still be loaded)", dos_shadow);
        else ImGui::BulletText("saved games: the SAVES folder of the game folder, as in the original"); }
    {   extern char dlp_data_dir[];
        bool own = strcmp(dlp_data_dir, game_dir) != 0;    /* the game folder could not be written */
        ImGui::BulletText("the game: %s", game_dir);
        ImGui::BulletText("settings: darklands_port.ini in %s", own ? dlp_data_dir : "the game folder");
        extern char dlp_user_dir[];
        ImGui::BulletText("mods, replacement art and sounds: the mods, art and sounds folders in %s", own ? dlp_user_dir : "the game folder");
        ImGui::BulletText("screenshots (Ctrl+F12, the window as you see it): the screenshots folder in %s", own ? dlp_data_dir : "the game folder");
        ImGui::BulletText("a log of the session: darklands_port.log (Windows) in %s, to send along with a report of a problem", own ? dlp_data_dir : "the game folder");
#ifdef DL_RTSPLIT
        ImGui::BulletText("the game's program, made from your DARKLAND.EXE on the first start: %s (delete it to have it made again)",
#ifdef _WIN32
                          "darklands_game.dll");
#else
                          "libdarklands_game.so");
#endif
#endif
    }
    ImGui::PushFont(font_head); ImGui::SeparatorText("Made with"); ImGui::PopFont();
    hint("Dear ImGui (MIT licence); Nuked OPL3, the Sound Blaster FM synthesis (LGPL 2.1); stb_image, stb_image_write, "
         "stb_vorbis (public domain); SDL2 (zlib licence); fonts EB Garamond, Alegreya, Atkinson Hyperlegible, IM Fell English, "
         "MedievalSharp and UnifrakturMaguntia (SIL Open Font License 1.1).");
    hint("With thanks to the Darklands community: vvendigo's tools, Merle's and Joel \"Quadko\" McIntyre's documentation "
         "of the file formats, and the Darklands Companion.");
}

/* F1: the keys and clicks of the moment (the game's own and Darklands+'s) */
/* the first start: what Darklands+ is, the keys that matter, and the choice to play the
 * original game untouched (the purists' switch), once */
static void welcome_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));
    float w = fminf(io.DisplaySize.x * 0.9f, UIS(760));
    ImGui::SetNextWindowSize(ImVec2(w, 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    if (ImGui::Begin("Welcome to Darklands+", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        extern char dos_shadow[];
        if (dos_shadow[0])
            ImGui::TextWrapped("This is Darklands (MicroProse, 1992) running natively, with the game files of your own copy. "
                               "Windows does not let programs change the game folder where it is, so your saved games go to %s "
                               "(the ones already in the game folder can be loaded as always).", dos_shadow);
        else
            ImGui::TextWrapped("This is Darklands (MicroProse, 1992) running natively, with the game files of your own copy. "
                               "Your saved games stay in the game folder as always.");
        ImGui::Spacing();
        ImGui::TextWrapped("Darklands+ adds what players have long wished for: crisp text, chances of success on the cards, "
                           "a party and inventory screen, a journal of quests and towns, a world map, a battle log, quick saves "
                           "and fixes for old bugs. Each of them can be switched off.");
        ImGui::Spacing();
        ImGui::PushFont(font_head); ImGui::SeparatorText("Keys"); ImGui::PopFont();
        ImGui::BulletText("F12: the Darklands+ menu (the game waits while it is open)");
        ImGui::BulletText("F1: the keys and clicks of the moment, the game's hidden ones too");
        ImGui::BulletText("Right click: the game's menu bar (save, load, orders in battle)");
        ImGui::BulletText("Alt+Enter: full screen   Ctrl+S: quick save   F9: quick load");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        bool done = false;
        if (ImGui::Button("Play with Darklands+")) done = true;
        ImGui::SameLine();
        if (ImGui::Button("Play the original game")) { set_original(true); done = true; }
        ImGui::TextDisabled("Either can be changed at any time: F12 > Helpers > Play the original game.");
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape)) done = true;
        if (done) { open_welcome = false; config_set_int("ui.welcome_seen", 1); config_save(); }
    }
    ImGui::End();
    ImGui::PopFont();
}

/* Darklands was made for the Roland MT-32: say so, and help find the ROM files */
extern "C" int plat_choose_folder(const char *title, char *out, int n);
static void mt32_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));
    float w = fminf(io.DisplaySize.x * 0.92f, UIS(780));
    ImGui::SetNextWindowSize(ImVec2(w, 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    if (ImGui::Begin("Darklands sounds best on a Roland MT-32", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextWrapped("Darklands was composed for the Roland MT-32 (and its sibling the LAPC-I). The game loads its own "
                           "instruments into it: the music is played as written, with flutes, strings and horns, and the battles "
                           "sound like swords and shields.");
        ImGui::Spacing();
        ImGui::TextWrapped("Every other sound setting is a stand-in. The Sound Blaster plays simpler FM versions of the music and "
                           "only rough imitations of the battle sounds; a General MIDI synthesizer such as Windows' own plays the "
                           "wrong instruments altogether.");
        ImGui::Spacing();
        ImGui::TextWrapped("Darklands+ has the MT-32 built in (Munt's emulator). It only needs the synthesizer's two ROM files, "
                           "control and PCM, of an MT-32 or a CM-32L (CM-32L is best). They belong to Roland, so they cannot come "
                           "with this download. If you play other games with Munt or DOSBox and an MT-32, you have them already: "
                           "show Darklands+ the folder.");
        ImGui::Spacing();
        ImGui::Separator();
        static char romdir[1024]; static bool init; static std::string msg;
        if (!init) { snprintf(romdir, sizeof romdir, "%s", config_get("sound.mt32_roms", "")); init = true; }
        bool have = mt32_available();
        if (have) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.18f, 0.40f, 0.16f, 1)); ImGui::TextWrapped("%s", mt32_status()); ImGui::PopStyleColor(); }
        else {
#ifdef _WIN32
            if (ImGui::Button("Choose the folder with the ROM files...")) {
                char d[1024];
                if (plat_choose_folder("The folder with the MT-32 or CM-32L ROM files (control and PCM):", d, sizeof d)) {
                    snprintf(romdir, sizeof romdir, "%s", d); config_set("sound.mt32_roms", romdir); config_save(); mt32_rescan();
                    msg = mt32_available() ? "" : "No MT-32 or CM-32L ROM files in that folder.";
                }
            }
            ImGui::SameLine(); ImGui::TextDisabled("or type it:");
#endif
            ImGui::SetNextItemWidth(UIS(380));
            if (ImGui::InputTextWithHint("##mt32dir", "C:\\path\\to\\the\\roms", romdir, sizeof romdir)) config_set("sound.mt32_roms", romdir);
            ImGui::SameLine();
            if (ImGui::Button("Look again")) { config_save(); mt32_rescan(); msg = mt32_available() ? "" : "Not found there yet."; }
            if (!msg.empty()) hint("%s", msg.c_str());
            hint("Darklands+ also looks in Munt's ROM folder, DOSBox Staging's mt32-roms and a \"roms\" folder next to it or the game.");
        }
        ImGui::Spacing();
        bool done = false;
        if (have && !mt32_is_open()) {
            if (ImGui::Button("Start again with the MT-32")) { config_save(); plat_restart(); }
            ImGui::SameLine();
        }
        if (ImGui::Button(have ? "Close" : "Not now")) done = true;
        if (!have) {
            ImGui::SameLine();
            if (ImGui::Button("Don't remind me again")) { config_set_int("sound.mt32_prompt", 0); done = true; }
        }
        ImGui::TextDisabled("F12 > Display > Sound has all of this too.");
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) done = true;
        if (done) { open_mt32 = false; config_save(); }
    }
    ImGui::End();
    ImGui::PopFont();
}

static void help_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 100));
    float w = fminf(io.DisplaySize.x * 0.92f, UIS(900));
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, io.DisplaySize.y * 0.94f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    if (ImGui::Begin("Keys and clicks (F1)", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetWindowFontScale(fminf(1.0f, fmaxf(0.7f, io.DisplaySize.y / 1000.0f)));
        auto row = [](const char *k, const char *d) { ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "%s", k); ImGui::SameLine(UIS(230)); ImGui::TextWrapped("%s", d); };
        int gm = game_mode();
        if (gm == GM_CARD) {
            ImGui::SeparatorText("Now: a card");
            row("Click an option", "or press its number, 1 to 9");
            row("Shift over an option", "the game's own chance of success (the badges show them all)");
            row("G", "go to a place in this city");
            row("M", "world map");
        } else if (gm == GM_MAP) {
            ImGui::SeparatorText("Now: the travel map");
            row("Click", "walk towards that spot");
            row("Ctrl + click", "travel there along the roads (any key or click stops)");
            row("M", "world map: find a place, double-click to travel there");
        } else if (gm == GM_BATTLE) {
            ImGui::SeparatorText("Now: a battle");
            row("Space", "pause / go on - orders can be given while paused");
            row("Click a member", "then click an enemy, a place or a choice of the menu for the order");
            row("Ctrl + click a member", "make that member the active one");
            row("Point at anyone", "who it is and how badly hurt");
            if (levelmap_available()) row("Tab", "a map of the level as far as the party has seen it");
            { extern int ui_battle_log; if (ui_battle_log && !original_mode()) row("The battle log", "every blow and wound: the left margin on a wide window, else the bottom right; F12 > Journal keeps it all"); }
        } else if (loot_active && lootview_showing()) {
            ImGui::SeparatorText("Now: the loot");
            row("Take / Put back", "on the row under the pointer: the whole stack (1: just one)");
            row("Take all / Share out", "everything to the member shown / to whoever has some already, else the emptiest pack (A / S)");
            row("Names above the right list, D", "another member");
            row("L, Esc or Leave", "leave (what is left stays behind)");
        } else if (loot_active) {
            ImGui::SeparatorText("Now: the loot");
            row("G / P", "get the chosen item from the pile / put one back (the cursor's list)");
            row("D", "hand out to the next member");
            if (loot_enabled()) { row("A", "take the whole pile for the member shown"); row("S", "share the pile out: each thing to whoever has some already, else the emptiest pack"); }
            row("L or Esc", "leave (what is left stays behind)");
        } else if (barter_active && barterview_showing()) {
            ImGui::SeparatorText("Now: a shop");
            row("Buy / Sell", "on the row under the pointer; with Shift: ten / the whole stack");
            row("Names above your list", "whose things are listed (B: the next member); right-click: that member holds the purse");
            row("Wheel, Up, Down", "scroll the list under the pointer");
            row("L, Esc or Leave", "leave");
        } else if (barter_active) {
            ImGui::SeparatorText("Now: a shop");
            row("P / S", "buy the merchant's chosen item / sell your chosen item (one at a time)");
            if (loot_enabled()) { row("T", "buy ten of the merchant's chosen item (while the money lasts)"); row("A", "sell the whole stack of your chosen item"); }
            row("B", "barter for the next member");
            row("L or Esc", "leave");
        } else if (sheetview_showing()) {
            ImGui::SeparatorText("Now: a character sheet");
            row("Ready / Away / Drink / Give / Drop", "on the row under the pointer in Things (Give and Drop with Shift: the whole stack)");
            row("Portraits, F1 ... F5", "another member's sheet; this member's own portrait, Esc or Close: back");
            row("Ctrl+F1 ... Ctrl+F5", "make member 1 ... 5 the leader (or Make leader)");
            row("Wheel, Up, Down", "scroll the list");
        } else if (hdart_on_screen("ARMBACK")) {        /* the character sheet (s031's keys) */
            ImGui::SeparatorText("Now: a character sheet");
            row("Ctrl+F1 ... Ctrl+F5", "make member 1 ... 5 the leader (or F12 > Party)");
            row("Up, Down, PgUp, PgDn", "move in a list (the mouse wheel too)");
            row("1 ... 5", "in the equipment list: give the chosen item to member 1 ... 5 (with Shift: all of them)");
            row("A", "in the equipment list: ready or wear the chosen item");
            row("D", "in the equipment list: drop the chosen item (with Shift: all of them)");
        }
        ImGui::SeparatorText("The game's own keys");
        if (config_get_int("assist.menu_click", 1) && !original_mode()) row("Right click", "the menu bar: click an entry (or hold the button and let go on it); a click elsewhere or any key closes it; F10 with the keyboard");
        else row("Right mouse button", "the menu bar, while it is held: let go on an entry (F10 with the keyboard)");
        row("Alt+S / Alt+L", "save / load (with a comment, nine saves)");
        row("Alt+O", "order of the party");
        row("Alt+D", "difficulty    Alt+M music    Alt+F sound effects");
        row("Alt+C", "messages about saints' changes on / off");
        row("F6", "the party's information");
        row("Alt+Q", "quit to the desktop");
        ImGui::SeparatorText("Darklands+");
        row("F12", "the Darklands+ menu (pauses the game)");
        row("Ctrl+S / F9 or Ctrl+L", "quick save / quick load");
        row("Alt+Enter", "full screen    Ctrl+F12 screenshot");
        { if (plat_controller_connected() && config_get_int("input.gamepad", 1)) {
            ImGui::SeparatorText("The controller");
            row("Left stick", "the pointer (left trigger held: slowly)    right stick: the wheel");
            row("A / B", "click / right click (the menu bar)");
            row("X / Y", "Enter / Esc");
            row("D-pad", "arrow keys    LB: Tab (level map)    RB: Space (pause)");
            row("Start / Back", "F12 (this menu) / M (world map)");
            row("Right trigger", "held: Ctrl (on the travel map: Ctrl + click travels there)    left stick pressed: F1");
          } }
        ImGui::TextDisabled("F1, Esc or a click closes this.");
    }
    ImGui::End();
    ImGui::PopFont();
}

/* one cursor at a time: over the game picture the game draws its own, so the system cursor
 * is hidden there; it shows with the menu or the world map open, and outside the picture */
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
static void ui_cursor(void)
{
    int mx, my; SDL_GetMouseState(&mx, &my);
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    bool over_game = mx >= x0 && mx < x1 && my >= y0 && my < y1;
    bool show = open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || levelmap_is_open() || halloffame_is_open() || figpick_is_open() || !over_game || sys_cursor;
    static int shown = -1;
    if (shown != (int)show) { SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE); shown = show; }
    if (show) {
        static SDL_Cursor *arrow; if (!arrow) arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        ImGuiMouseCursor c = ImGui::GetMouseCursor();
        static SDL_Cursor *cur[ImGuiMouseCursor_COUNT];
        static const SDL_SystemCursor map[ImGuiMouseCursor_COUNT] = { SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_IBEAM, SDL_SYSTEM_CURSOR_SIZEALL,
            SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_HAND, SDL_SYSTEM_CURSOR_NO };
        if (c >= 0 && c < ImGuiMouseCursor_COUNT) { if (!cur[c]) cur[c] = SDL_CreateSystemCursor(map[c]); SDL_SetCursor(cur[c]); }
        else SDL_SetCursor(arrow);
    }
}

extern "C" void ui_draw(SDL_Renderer *ren)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (!open_menu && !open_map && !open_goto) citynav_tick();
    if (open_goto && !goto_allowed()) open_goto = false;
    levelmap_frame();
    if (open_map) worldmap_draw();
    else if (halloffame_is_open()) halloffame_draw();
    else if (levelmap_is_open()) levelmap_draw();
    else { hdtext_draw(); partypanel_draw(); cardview_draw(); barterview_draw(); sheetview_draw(); lootview_draw(); alchemyview_draw(); cacheview_draw(); prayview_draw(); createview_draw(); rosterview_draw(); infoview_draw(); marchview_draw(); savesview_draw(); menuview_draw(); }
    if (!open_menu && !open_map && !levelmap_is_open() && !halloffame_is_open()) ui_overlay();
    {
        if (!open_menu && !open_map && !open_goto && !open_help && !levelmap_is_open() && !halloffame_is_open() && !figpick_is_open()) hdtext_draw_cursor();
        else hdtext_pointer_off();
        hdtext_pointer_frame_end(); }
    ns_defer_frame_end(open_menu || open_map);                    /* (requests while the game is held: mods/nscreen.h) */
    if (open_menu) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));
        float w = fminf(io.DisplaySize.x * 0.95f, UIS(1100)), h = fminf(io.DisplaySize.y * 0.92f, UIS(820));
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushFont(font_body);
        bool keep = true;
        if (ImGui::Begin("Darklands+", &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            /* the labels a little smaller than the text, so that all of them show in a 960 pixel
               window (the whole tab bar in that face: ImGui measures the tabs again when it lays
               them out) */
            const char *names[] = { "Inventory", "Party", "Alchemy", "Saints", "Journal", "Saves", "Travel", "Helpers", "Mods", "Display", "Records", "About", "Editor" };
            void (*fns[])(void) = { tab_inventory, tab_members, tab_alchemy, tab_saints, tab_journal, tab_saves, tab_travel, tab_helpers, tab_mods, tab_display, tab_party, tab_about, tab_editor };
            ImFont *tf = font_tab;
            {   /* every label whole, with its picture: the smaller face when the window is narrow */
                float need = 0, avail = ImGui::GetContentRegionAvail().x;
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
                    if (fns[t] == tab_editor && (!config_get_int("editor.show", 0) || original_mode())) continue;
                    need += font_tab->CalcTextSizeA(font_tab->FontSize, FLT_MAX, 0, names[t]).x + font_tab->FontSize * 1.25f + UIS(12) + ImGui::GetStyle().ItemInnerSpacing.x + 2;
                }
                if (need > avail && font_tab_small) tf = font_tab_small;
            }
            ImGui::PushFont(tf); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UIS(6), UIS(5)));
            if (ImGui::BeginTabBar("tabs")) {
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
                    if (fns[t] == tab_editor && (!config_get_int("editor.show", 0) || original_mode())) continue;
                    /* room for its picture at the start of the label: spaces as wide as the picture */
                    char lab[64]; int nsp = (int)ceilf(ImGui::GetFontSize() * 1.25f / fmaxf(1.0f, ImGui::CalcTextSize(" ").x));
                    if (nsp > 12) nsp = 12;
                    snprintf(lab, sizeof lab, "%.*s%s###%s", nsp, "            ", names[t], names[t]);
                    bool open_t = ImGui::BeginTabItem(lab, nullptr, want_tab == t ? ImGuiTabItemFlags_SetSelected : 0);
                    {   ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); float fs = ImGui::GetFontSize();
                        ui_tab_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + UIS(6) + fs * 0.5f, (a.y + b.y) * 0.5f), fs * 0.6f, t); }
                    if (open_t) {
                        ImGui::PushFont(font_body); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UIS(8), UIS(4)));
                        fns[t]();
                        ImGui::PopStyleVar(); ImGui::PopFont();
                        ImGui::EndTabItem();
                    }
                }
                want_tab = -1;
                ImGui::EndTabBar();
            }
            ImGui::PopStyleVar(); ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopFont();
        if (!keep) ui_toggle();
    }
    if (open_goto) { ImGui::PushFont(font_body); if (!citynav_window()) open_goto = false; ImGui::PopFont(); }
    figpick_window();
    if (open_help) help_window();
    if (open_welcome) welcome_window();
    else if (open_mt32) mt32_window();
    ui_draw_notice();
    if (getenv("DL_ICON_SHEET")) {                 /* test: every item's icon */
        ImDrawList *dl = ImGui::GetForegroundDrawList(); ImGuiIO &io = ImGui::GetIO();
        dl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(40, 30, 20, 255));
        int first = atoi(getenv("DL_ICON_SHEET")), cols = 10; float cw = io.DisplaySize.x / cols, chh = 64;
        for (int i = first; i < (int)gamedata.items.size() && i < first + 110; i++) {
            int k = i - first; ImVec2 p((k % cols) * cw, (k / cols) * chh);
            ui_item_icon(dl, ImVec2(p.x + cw / 2, p.y + 24), 40, i);
            std::string n = game_text(gamedata.items[i].name.c_str(), 64);
            dl->AddText(ui_font_small, 13, ImVec2(p.x + 4, p.y + 46), IM_COL32(240, 230, 210, 255), n.c_str());
        }
    }
    ui_cursor();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
}

/* the first start of a release: the game module is being made (runtime/prepare_run.c) */
extern "C" void ui_progress_draw(SDL_Renderer *ren, const char *step, int pct, const char *note)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 30, 0));
    ImGui::Begin("Darklands+##prepare", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextWrapped("Darklands+ is preparing the game from your own DARKLAND.EXE. This happens only once and takes a minute or two.");
    ImGui::Spacing();
    ImGui::TextUnformatted(step && *step ? step : "Starting");
    ImGui::ProgressBar(pct / 100.0f, ImVec2(-1, 0));
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", note && *note ? note : "Nothing from the game is included in the Darklands+ download; it is built here from your own copy.");
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::Render();
    SDL_SetRenderDrawColor(ren, 20, 16, 12, 255);
    SDL_RenderClear(ren);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
    SDL_RenderPresent(ren);
}
