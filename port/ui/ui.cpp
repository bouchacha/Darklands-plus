// ui.cpp - the native port's own interface, drawn over the game: the Darklands+ window (F12)
// and its pages (the settings pages in settings.cpp), help, notices. Dear ImGui on SDL_Renderer.
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
#include "quests.h"
#include "uiinternal.h"

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
#include "../mods/lootview.h"
#include "../mods/sheetview.h"
#include "ui.h"
#include "money.h"
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
extern "C" void lodgeview_draw(void);
extern "C" { int lodgeview_is_on(void); void lodgeview_set(int on); }
extern "C" void enemyview_draw(void);
extern "C" { int enemyview_is_on(void); void enemyview_set(int on); }
extern "C" void mapmsg_draw(void);
extern "C" { int mapmsg_is_on(void); void mapmsg_set(int on); }
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
extern "C" { extern int battlescroll_on, battlescroll_rest; void battlescroll_set(int, int); extern int loot_return_thrown; extern int ammo_infinite; void ammo_set(int v); extern int thrown_infinite; void thrown_set(int v); }
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
int ui_sys_cursor;                     /* display.system_cursor: the system pointer stays over the game */
const char *UI_GAME_FONT_NAMES[UI_GAME_FONTS] = { "EB Garamond (book face)", "Alegreya (calligraphic)", "IM Fell English (old print)",
    "Atkinson Hyperlegible (easiest to read)", "MedievalSharp (medieval hand)", "Fraktur (blackletter, for the brave)",
    "Grenze Gotisch (gothic, still readable)" };
#define UI_DEFAULT_FONT 6                  /* Grenze Gotisch */
/* the text over and beside the picture (side panels, tooltips, map names, notices): the game
   text's typeface, or the menus' (display.overlay_font_follow) */
ImFont *ui_font_overlay;
void ui_overlay_font_update(void)
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
void ui_guide_tab(void);
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
SDL_Window *ui_window;
ImFont *ui_font_head;
int ui_dirty_config;
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
    ui_window = win;
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
    ui_font_head = font_head;
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
    money_glyphs_add(io.Fonts);          /* the gold, silver and copper coins after amounts (ui/money.cpp) */
    io.Fonts->Build();
    money_glyphs_paint(io.Fonts);
    open_welcome = (!config_get_int("ui.welcome_seen", 0) && !getenv("DL_SCRIPT") && !getenv("DL_CTL")) || getenv("DL_WELCOME");   /* the first start */
    {   /* the MT-32 recommendation: at each start while its ROM files are missing, until "don't remind me" */
        const char *dev = config_get("sound.device", "auto");
        open_mt32 = ((!getenv("DL_SCRIPT") && !getenv("DL_CTL")) || getenv("DL_MT32_PROMPT")) && config_get_int("sound.mt32_prompt", 1)
                    && !config_get_int("original", 0) && (!strcmp(dev, "auto") || !strcmp(dev, "mt32")) && !mt32_available();
    }
    ui_game_font = config_get_int("display.text_font", UI_DEFAULT_FONT);
    if (ui_game_font < 0 || ui_game_font >= UI_GAME_FONTS) ui_game_font = UI_DEFAULT_FONT;
    ui_overlay_font_update();
    ui_text_edge = config_get_int("display.text_edge", 2);
    { extern int ui_text_contrast; ui_text_contrast = config_get_int("display.text_contrast", 1); }
    { extern int ui_skill_icons; ui_skill_icons = config_get_int("display.skill_icons", 1); }
    { extern int ui_shop_info; ui_shop_info = config_get_int("assist.shop_info", 1); }
    { extern int ui_creation_guide; ui_creation_guide = config_get_int("assist.creation_guide", 1); }
    { extern int ui_battle_info, ui_battle_numbers, ui_battle_bars; ui_battle_info = config_get_int("battle.info", 1); ui_battle_numbers = config_get_int("battle.info_numbers", 0); ui_battle_bars = config_get_int("battle.bars", 1); }
    { extern int ui_battle_log; ui_battle_log = config_get_int("battle.log", 1); }
    ui_spell_out = config_get_int("display.spell_out", 1);
    ui_sys_cursor = config_get_int("display.system_cursor", 0);
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
    if (ui_window) { journal_init(); void battlelog_init(void); battlelog_init(); }
}

extern "C" int levelmap_is_open(void);
extern "C" int levelmap_available(void);
extern "C" void levelmap_toggle(void);
extern "C" void levelmap_frame(void);
extern "C" void levelmap_draw(void);
extern "C" int ui_is_open(void) { return open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || levelmap_is_open() || halloffame_is_open() || figpick_is_open(); }
extern "C" void ui_open_tab(int t) { open_map = false; open_goto = false; open_help = false; open_menu = true; if (t >= 0) ui_select_tab(t); }   /* (-1: the page it was on) */
extern "C" void ui_toggle(void) { open_map = false; open_goto = false; open_help = false; open_menu = !open_menu; if (!open_menu && ui_dirty_config) { config_save(); ui_dirty_config = 0; } }
/* the world map (ui/worldmap.cpp): M on the travel map, or the Travel tab */
extern "C" void ui_toggle_map(void)
{
    if (open_map) { open_map = false; return; }
    int gm = game_mode();                                   /* (battles and their loot reuse the places' memory) */
    if (gm != GM_CARD && gm != GM_MAP) { ui_notice("The world map opens on the travel map or a card, not in a battle."); return; }
    if (worldmap_open()) { open_map = true; if (open_menu) ui_toggle(); open_map = true; }
}

extern "C" int overlay_click(int x, int y), overlay_click_up(int x, int y);
extern "C" int ns_showing(void);
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
    ui_hint("Dear ImGui (MIT licence); Nuked OPL3, the Sound Blaster FM synthesis (LGPL 2.1); stb_image, stb_image_write, "
         "stb_vorbis (public domain); SDL2 (zlib licence); fonts EB Garamond, Alegreya, Atkinson Hyperlegible, IM Fell English, "
         "MedievalSharp and UnifrakturMaguntia (SIL Open Font License 1.1).");
    ui_hint("With thanks to the Darklands community: vvendigo's tools, Merle's and Joel \"Quadko\" McIntyre's documentation "
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
        ImGui::BulletText("F12 > Guide: how Darklands works, for a new player");
        ImGui::BulletText("Right click: the game's menu bar (save, load, orders in battle)");
        ImGui::BulletText("Alt+Enter: full screen   Ctrl+S: quick save   F9: quick load");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        bool done = false;
        if (ImGui::Button("Play with Darklands+")) done = true;
        ImGui::SameLine();
        if (ImGui::Button("Play the original game")) { ui_set_original(true); done = true; }
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
            if (!msg.empty()) ui_hint("%s", msg.c_str());
            ui_hint("Darklands+ also looks in Munt's ROM folder, DOSBox Staging's mt32-roms and a \"roms\" folder next to it or the game.");
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
            row("Point at a quest's picture", "the task that leads there (a portcullis or a pillory beside a town: it bars the party, or its watch wants it)");
            row("A task in the left margin", "click to open it, then Travel there");
        } else if (gm == GM_BATTLE) {
            ImGui::SeparatorText("Now: a battle");
            row("Space", "pause / go on - orders can be given while paused");
            row("Click a member", "then click an enemy, a place or a choice of the menu for the order");
            row("Ctrl + click a member", "make that member the active one");
            row("Point at anyone", "who it is and how badly hurt");
            if (levelmap_available()) row("Tab", "a map of the level as far as the party has seen it");
            { extern int ui_battle_log; if (ui_battle_log && !ui_original_mode()) row("The battle log", "every blow and wound: the left margin on a wide window, else the bottom right; F12 > Journal keeps it all"); }
        } else if (loot_active && lootview_showing()) {
            ImGui::SeparatorText("Now: the loot");
            row("Take / Put back", "on the row under the pointer: the whole stack (1: just one)");
            if (loot_model.shared) row("Take all", "everything into the party's pack");
            else { row("Take all / Share out", "everything to the member shown / to whoever has some already, else the emptiest pack (A / S)");
                   row("Names above the right list, D", "another member"); }
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
            if (sheet_model.shared) row("Ready / Unequip / Drink / Keep / Drop", "on the row under the pointer in Things: the pack is the whole party's (Drop with Shift: all of them); drag a thing in use onto a portrait to give it");
            else row("Ready / Away / Drink / Give / Drop", "on the row under the pointer in Things (Give and Drop with Shift: the whole stack)");
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
        if (config_get_int("assist.menu_click", 1) && !ui_original_mode()) row("Right click", "the menu bar: click an entry (or hold the button and let go on it); a click elsewhere or any key closes it; F10 with the keyboard");
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
    bool show = open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || levelmap_is_open() || halloffame_is_open() || figpick_is_open() || !over_game || ui_sys_cursor;
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

/* test (ucmd uicongrid): every potion in its three grades and every ingredient, large, to look at;
   again: the quests' pictures, large and at the sizes of the lists, on light and on dark */
static int icon_grid;
extern "C" void ui_icon_grid_toggle(void) { icon_grid = (icon_grid + 1) % 3; }
static void draw_icon_grid(void)
{
    if (!icon_grid) return;
    void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(244, 234, 210, 255));
    float cell = fminf(ds.x / 12.5f, ds.y / 8.5f), s = cell * 0.62f;
    if (icon_grid == 2) {
        dl->AddRectFilled(ImVec2(0, ds.y * 0.5f), ds, IM_COL32(24, 18, 12, 255));
        for (int k = 1; k < QC_N; k++) {
            float x = cell * 0.6f + (k - 1) * cell * 0.92f;
            ui_draw_quest_icon(dl, ImVec2(x, cell * 0.7f), cell * 0.85f, k);
            ui_draw_quest_icon(dl, ImVec2(x, cell * 1.7f), 32, k);
            ui_draw_quest_icon(dl, ImVec2(x, cell * 2.3f), 20, k);
            ui_draw_quest_icon(dl, ImVec2(x, ds.y * 0.5f + cell * 0.7f), cell * 0.85f, k);
            ui_draw_quest_icon(dl, ImVec2(x, ds.y * 0.5f + cell * 1.7f), 32, k);
            ui_draw_quest_icon(dl, ImVec2(x, ds.y * 0.5f + cell * 2.3f), 20, k);
        }
        return;
    }
    for (int k = 0; k < 22; k++) for (int t = 1; t <= 3; t++) {
        int col = (k % 4) * 3 + (t - 1), row = k / 4;
        ui_item_icon_tier(dl, ImVec2(cell * 0.6f + col * cell, cell * 0.6f + row * cell), s, 95 + k, t);
    }
    for (int k = 0; k < 23; k++) {
        int col = k % 12, row = 6 + k / 12;
        ui_item_icon_tier(dl, ImVec2(cell * 0.6f + col * cell, cell * 0.6f + row * cell), s, 72 + k, 0);
    }
}

extern "C" void ui_draw(SDL_Renderer *ren)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (game_mode() == GM_MAP) assist_leader_restore();     /* (back on the map: the leader the player chose) */
    if (!open_menu && !open_map && !open_goto) citynav_tick();
    if (open_goto && !goto_allowed()) open_goto = false;
    levelmap_frame();
    if (open_map) worldmap_draw();
    else if (halloffame_is_open()) halloffame_draw();
    else if (levelmap_is_open()) levelmap_draw();
    else { hdtext_draw(); partypanel_draw(); cardview_draw(); barterview_draw(); sheetview_draw(); lootview_draw(); alchemyview_draw(); cacheview_draw(); prayview_draw(); createview_draw(); rosterview_draw(); infoview_draw(); marchview_draw(); lodgeview_draw(); enemyview_draw(); mapmsg_draw(); savesview_draw(); menuview_draw(); }
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
            const char *names[] = { "Inventory", "Party", "Alchemy", "Saints", "Journal", "Saves", "Travel", "Helpers", "Mods", "Display", "Records", "About", "Editor", "Guide" };
            void (*fns[])(void) = { tab_inventory, tab_members, tab_alchemy, tab_saints, tab_journal, tab_saves, tab_travel, tab_helpers, tab_mods, tab_display, tab_party, tab_about, tab_editor, ui_guide_tab };
            ImFont *tf = font_tab;
            {   /* every label whole, with its picture: the smaller face when the window is narrow */
                float need = 0, avail = ImGui::GetContentRegionAvail().x;
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
                    if (fns[t] == tab_editor && (!config_get_int("editor.show", 0) || ui_original_mode())) continue;
                    need += font_tab->CalcTextSizeA(font_tab->FontSize, FLT_MAX, 0, names[t]).x + font_tab->FontSize * 1.25f + UIS(12) + ImGui::GetStyle().ItemInnerSpacing.x + 2;
                }
                if (need > avail && font_tab_small) tf = font_tab_small;
            }
            ImGui::PushFont(tf); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UIS(6), UIS(5)));
            if (ImGui::BeginTabBar("tabs")) {
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
                    if (fns[t] == tab_editor && (!config_get_int("editor.show", 0) || ui_original_mode())) continue;
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
    draw_icon_grid();
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
