// ui.cpp - the native port's own interface, drawn over the game: the Darklands+ window (F12)
// and its pages (the settings pages in settings.cpp), help, notices. Dear ImGui on SDL_Renderer.
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <string>
#include "richtext.h"
#include "padglyphs.h"
#include "imgui.h"
#include "tablesort.h"
#include "skillicons.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "drawcache.h"
#include "gamedata.h"
#include "quests.h"
#include "uiinternal.h"
#include "uitabs.h"
#include "townicons.h"
#include "libraries.h"

#include "ui_decls.h"
extern "C" {
#include "../mods/options.h"
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
}
#include "ornament.h"

static bool open_menu, open_map, open_goto;
bool open_help, open_welcome, open_mt32;   /* (the side windows: ui/ui_windows.cpp) */
extern "C" void ui_toggle_help(void) { open_help = !open_help; }
/* "Go to" in cities (ui/citynav.cpp): G on a city card */
static bool goto_allowed(void) { return config_get_int("assist.city_goto", 1) && !config_get_int("original", 0) && citynav_available(); }
extern "C" void ui_open_goto(void) { if (goto_allowed()) { open_goto = true; open_menu = false; open_map = false; } }
extern "C" int soundpreview_active(void); extern "C" volatile int sound_music_hushed; extern int settings_pack_preview;
static bool settings_shown;                 /* the Settings tab drawn this frame (the music hushed) */
static int want_tab = -1;                  /* a page (ui/uitabs.h) to show; kept until its group's tabs have drawn it */
static int want_frames;
extern "C" void ui_select_tab(int t) { if (t % 100 == UI_PAGE_DISPLAY) t = UI_PAGE_HELPERS; want_tab = t % 100; want_frames = 0; if (t >= 100) ui_want_subtab = t / 100 - 1; }   /* 100*(sub+1) + page */
ImFont *font_body, *font_head, *font_tab, *font_tab_small;   /* (ui/ui_windows.cpp too) */
/* tabs inside a page (the Journal's, Settings' ...): in the smaller face of the window's own
   tabs, under them in the order of things; the page's text in the body face */
/* a page's own tabs: in the main tabs' face, the smaller one in a narrow window (the Journal's nine
   need about 950 points) */
static ImFont *subtab_font = nullptr;
bool ui_subtabs_begin(const char *id)
{
    subtab_font = ImGui::GetContentRegionAvail().x >= UIS(950) ? font_tab : font_tab_small;
    ImGui::PushFont(subtab_font); bool r = ImGui::BeginTabBar(id); ImGui::PopFont(); return r;
}
bool ui_subtab(const char *label, bool select)
{
    ImGui::PushFont(subtab_font ? subtab_font : font_tab_small);
    bool r = ImGui::BeginTabItem(label, nullptr, select ? ImGuiTabItemFlags_SetSelected : 0);
    ImGui::PopFont(); return r;
}

/* a page's tab with its picture (ui_page_icon) at the start of the label */
void ui_page_icon(ImDrawList *dl, ImVec2 c, float s, int page);   /* ui/skillicons.cpp */
bool ui_subtab_icon(const char *label, bool select, int page)
{
    ImGui::PushFont(subtab_font ? subtab_font : font_tab_small);
    int nsp = (int)ceilf(ImGui::GetFontSize() * 1.3f / fmaxf(1.0f, ImGui::CalcTextSize(" ").x)); if (nsp > 12) nsp = 12;
    char lab[96]; snprintf(lab, sizeof lab, "%.*s%s###%s", nsp, "            ", label, label);
    bool r = ImGui::BeginTabItem(lab, nullptr, select ? ImGuiTabItemFlags_SetSelected : 0);
    ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); float fs = ImGui::GetFontSize();
    ui_page_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + UIS(6) + fs * 0.55f, (a.y + b.y) * 0.5f), fs * 0.62f, page);
    ImGui::PopFont(); return r;
}

ImFont *ui_font_small, *ui_font_text;
float ui_scale = 1.0f;
ImFont *ui_font_game[UI_GAME_FONTS];       /* the crisp game text: EB Garamond, Alegreya, IM Fell, Atkinson, MedievalSharp, Fraktur */
ImFont *ui_font_game_hi[UI_GAME_FONTS];    /* the same baked larger on big screens (0 if not needed) */
int ui_game_font, ui_text_edge;            /* edge: 0 none, 1 shadow, 2 outline */
int ui_sys_cursor;                     /* display.system_cursor: the system pointer stays over the game */
const char *UI_GAME_FONT_NAMES[UI_GAME_FONTS] = { "EB Garamond (book face)", "Alegreya (calligraphic)", "IM Fell English (old print)",
    "Atkinson Hyperlegible (easiest to read)", "MedievalSharp (medieval hand)", "Fraktur (blackletter, for the brave)",
    "Grenze Gotisch (gothic, still readable)", "Comic Sans (we are so sorry)" };
#define UI_DEFAULT_FONT 6                  /* Grenze Gotisch */
/* the text over and beside the picture (side panels, tooltips, map names, notices): the game
   text's typeface, or the menus' (display.overlay_font_follow) */
ImFont *ui_font_overlay;
void ui_overlay_font_update(void)
{
    extern ImFont *ui_font_text;
    ui_font_overlay = config_get_int("display.overlay_font_follow", 1) && ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
}
SDL_Window *ui_window;
ImFont *ui_font_head, *ui_font_bold;
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

/* what the interface keeps of the switches (mods/options.h), read again after one changes (key) or a
   preset is applied (all of them; key 0 at the start) */
static void ui_options_reload(const char *key, int v)
{
    (void)v;
    bool all = !key || !strcmp(key, "preset");
    auto is = [&](const char *k) { return all || !strcmp(key, k); };
    { extern int ui_shop_info; ui_shop_info = config_get_int("assist.shop_info", 1); }
    { extern int ui_creation_guide; ui_creation_guide = config_get_int("assist.creation_guide", 1); }
    { extern int ui_battle_info, ui_battle_numbers; ui_battle_info = config_get_int("battle.info", 1); ui_battle_numbers = config_get_int("battle.info_numbers", 0); }
    { extern int ui_battle_log; ui_battle_log = config_get_int("battle.log", 1); }
    { extern int ui_margins_on; ui_margins_on = config_get_int("display.margins", 1); }
    if (key && (is("display.native_panel") || is("display.panel_picture"))) partypanel_init();
    if (key && is("journal.enabled")) journal_set(config_get_int("journal.enabled", 1));
    if (key && (is("display.full_names") || is("display.armour_worn")) && gamedata.loaded) gamedata_names();
    if (key) ui_dirty_config = 1;
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
    ui_font_bold = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, UIS(24.0f), &fc);   /* (the body's size: a switch's title) */
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
        const unsigned char *data[UI_GAME_FONTS] = { FONT_MEDIUM, FONT_ALEGREYA, FONT_FELL, FONT_ATKINSON, FONT_MEDIEVAL, FONT_FRAKTUR, FONT_GRENZE, FONT_COMIC };
        unsigned int size[UI_GAME_FONTS] = { FONT_MEDIUM_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_ATKINSON_SIZE, FONT_MEDIEVAL_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE, FONT_COMIC_SIZE };
        {   /* the joke typeface: the real Comic Sans where the system has it (Windows' own, or Linux's
               msttcorefonts; read from there, never shipped), else Comic Neue (OFL), which comes with
               Darklands+ and is named so */
            static unsigned char *comic; static size_t comic_n;
            const char *wd = getenv("WINDIR"); std::string p1 = std::string(wd ? wd : "C:\\Windows") + "\\Fonts\\comic.ttf";
            const char *paths[] = { p1.c_str(), "/usr/share/fonts/truetype/msttcorefonts/comic.ttf", "/usr/share/fonts/truetype/msttcorefonts/Comic_Sans_MS.ttf" };
            for (const char *pth : paths) if (!comic) { size_t n = 0; void *d = SDL_LoadFile(pth, &n); if (d && n > 1000) { comic = (unsigned char *)d; comic_n = n; } else if (d) SDL_free(d); }
            if (comic) { data[7] = comic; size[7] = (unsigned int)comic_n; }
            else UI_GAME_FONT_NAMES[7] = "Comic Neue (a Comic Sans look-alike, we are so sorry)";
        }
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
    tips_init();                         /* (the starts counted: ui/tips.cpp) */
    open_welcome = getenv("DL_WELCOME") != nullptr;   /* (the first start's welcome comes on the start screen: tips_title_reached) */
    {   /* the MT-32 recommendation: at each start while its ROM files are missing, until "don't remind me" */
        const char *dev = config_get("sound.device", "auto");
        open_mt32 = ((!getenv("DL_SCRIPT") && !getenv("DL_CTL") && !getenv("DL_RESTARTED")) || getenv("DL_MT32_PROMPT")) && config_get_int("sound.mt32_prompt", 1)
                    && !config_get_int("original", 0) && (!strcmp(dev, "auto") || !strcmp(dev, "mt32")) && !mt32_available();
    }
    ui_game_font = config_get_int("display.text_font", UI_DEFAULT_FONT);
    if (ui_game_font < 0 || ui_game_font >= UI_GAME_FONTS) ui_game_font = UI_DEFAULT_FONT;
    ui_overlay_font_update();
    ui_text_edge = config_get_int("display.text_edge", 2);
    { extern int ui_text_contrast; ui_text_contrast = config_get_int("display.text_contrast", 1); }
    { extern int ui_skill_icons; ui_skill_icons = config_get_int("display.skill_icons", 1); }
    ui_options_reload(0, 0);
    options_set_listener(ui_options_reload);
    ui_spell_out = config_get_int("display.spell_out", 1);
    ui_sys_cursor = config_get_int("display.system_cursor", 0);
    idle_rest = 1;                      /* (always: the game waits without spinning; display.idle_rest retired, DL_NO_SPIN_REST for tests) */
    gamedata_load(game_dir);
    if (getenv("DL_RESTARTED")) {
        const char *ee = getenv("DL_ERROR_EXIT");
        if (ee && atoi(ee)) ui_notice_for("Darklands stopped because of an error. Your saves are safe: Load is in the Game menu (right-click). F12 > About > Report a problem makes a report of it.", 30000);
        else ui_notice_for("The party's journey has ended. Load a saved game to go on: Load in the Game menu (right-click).", 20000);
    }
    halloffame_set_renderer(ren);
    gamepics_set_renderer(ren); partypanel_set_renderer(ren); partypanel_init();
    { void screenpics_set_renderer(SDL_Renderer *); screenpics_set_renderer(ren); }
    figures_set_renderer(ren);
    if (getenv("DL_HALL")) halloffame_open();     /* the party has just entered the Honor Roll of Glory */
    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer2_Init(ren);
    style();
    if (ui_scale != 1.0f) ImGui::GetStyle().ScaleAllSizes(ui_scale);
    ui_settings.aspect = 1;            /* always 4:3: the game was drawn for a 1992 monitor's tall pixels (the square-pixel option distorted it) */
    ui_settings.smooth = config_get_int("display.smooth", 0);
    ui_settings.integer = 0;            /* (display.integer_scale retired: three ways to scale, F12 > Settings) */
    ui_settings.scaler = config_get_int("display.scaler", 0);
}

/* what needs the game's code in place (hooks): after the game module is loaded (split.h) */
extern "C" void citynav_init(void);
extern "C" void ui_late_init(void)
{
    if (ui_window) { journal_init(); void battlelog_init(void); battlelog_init(); }
    citynav_init();                                           /* (Go to: the card texts as they are loaded) */
}

/* (third_party/imgui/imconfig.h: a failed check of ImGui's, logged once a place) */
void dl_imgui_assert(const char *expr, const char *file, int line)
{
    static const char *seen[64]; static int ns;
    for (int i = 0; i < ns; i++) if (seen[i] == expr) return;
    if (ns < 64) seen[ns++] = expr;
    fprintf(stderr, "imgui: check failed: %s (%s:%d)\n", expr, file, line);
}
extern "C" void ui_show_mt32_help(void) { open_mt32 = true; }   /* (the settings' Sound: how to get the MT-32) */
extern "C" int ui_is_open(void) { return open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || tips_open() || levelmap_is_open() || halloffame_is_open() || figpick_is_open() || ui_alert_open(); }
extern "C" void ui_open_tab(int t) { open_map = false; open_goto = false; open_help = false; open_menu = true; if (t >= 0) ui_select_tab(t); }   /* (-1: the page it was on) */
extern "C" int ui_map_open(void) { return open_map; }
extern "C" void ui_toggle(void) { open_map = false; open_goto = false; open_help = false; open_menu = !open_menu; if (!open_menu && ui_dirty_config) { config_save(); ui_dirty_config = 0; } }
/* the world map (ui/worldmap.cpp): M on the travel map or a card */
extern "C" void ui_toggle_map(void)
{
    if (open_map) { open_map = false; fprintf(stderr, "world map: closed\n"); return; }
    int gm = game_mode();                                   /* (battles and their loot reuse the places' memory) */
    if (gm != GM_CARD && gm != GM_MAP) {
        ui_notice(gm == GM_BATTLE ? "The world map can only be opened from the overland view or a card, not in a battle."
                  : gm == GM_SCREEN ? "The world map can only be opened from the overland view or a card: close this screen first."
                                    : "The world map can only be opened from the overland view or a card, once a game is under way.");
        return;
    }
    if (worldmap_open()) { open_map = true; if (open_menu) ui_toggle(); open_map = true; fprintf(stderr, "world map: open\n"); }
}

static int margin_click_taken, outside_down;
extern "C" int levelwalk_door_click(void);            /* (mods/levelwalk.c: a click on a door, out of a fight) */
extern "C" int ui_event(const SDL_Event *e)
{
    if (e->type == SDL_RENDER_TARGETS_RESET || e->type == SDL_RENDER_DEVICE_RESET) dc_reset();   /* (the kept pictures lost) */
    if (open_mt32 && !open_welcome) {       /* the MT-32 recommendation, as the welcome */
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (open_welcome || tips_open()) {      /* the first start's word of welcome, and the tips (ui/tips.cpp), take every event until closed */
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (ui_alert_open() && !open_menu) {     /* a notice to click through (ui/alerts.cpp) takes every event until it closes: the
                                               battle under it must not take its click (or the Esc / Space meant for it) */
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12) { ui_toggle(); return 1; }
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
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F11 && !e->key.repeat && !open_menu && !open_map && !open_goto) { open_help = !open_help; return 1; }
    if (open_help) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_F11)) { open_help = false; return 1; }
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
    /* Z in a battle: fast forward (mods/pathfix.c) */
    if (!open_menu && !open_map && !open_goto && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_z && !e->key.repeat
        && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_SHIFT)) && game_mode() == GM_BATTLE && !ns_showing()
        && config_get_int("battle.fast_key", 1) && !config_get_int("original", 0)) { battle_fast_set(!battle_fast); return 1; }
    if (!open_menu && !open_map && !open_goto && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_g && !e->key.repeat
        && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_SHIFT)) && goto_allowed()) { open_goto = true; return 1; }
    if (open_goto) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_g)) { open_goto = false; return 1; }
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12) { open_goto = false; ui_toggle(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (open_map) {
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE && worldmap_find_escape(ImGui::GetIO().WantTextInput)) {   /* (Esc: a name in the Find box cleared first) */
            if (ImGui::GetIO().WantTextInput) ImGui_ImplSDL2_ProcessEvent(e);
            return 1;
        }
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || (e->key.keysym.sym == SDLK_m && !ImGui::GetIO().WantTextInput))) { ui_toggle_map(); return 1; }
        ImGui_ImplSDL2_ProcessEvent(e);
        return e->type != SDL_QUIT && e->type != SDL_WINDOWEVENT;
    }
    if (!open_menu) {
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT && overlay_click(e->button.x, e->button.y)) return 1;
        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && overlay_click_up(e->button.x, e->button.y)) return 1;
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT && (margins_click(e->button.x, e->button.y) || partypanel_click(e->button.x, e->button.y) || cardview_chip_click(e->button.x, e->button.y) || levelwalk_door_click())) { margin_click_taken = 1; return 1; }
        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && margin_click_taken) { margin_click_taken = 0; return 1; }
        {   /* a right-click on a member's box in a battle: their sheet (ui/partypanel.cpp) */
            static int rtaken;
            if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_RIGHT && partypanel_rclick(e->button.x, e->button.y)) { rtaken = 1; return 1; }
            if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_RIGHT && rtaken) { rtaken = 0; return 1; }
        }
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


/* F12 > Appendices: the tables read from the game's own data (the weapons, the enemies). The New Player
   Guide's text (ui/guide.cpp, "How Darklands works") is withdrawn until each statement in it is checked in
   the game itself, not only read from the code: its first line was wrong in 1.3-RC24 */
static void tab_guide_all(void)
{
    if (!ui_subtabs_begin("guide")) return;
    if (ui_subtab("Weapons")) { rules_tab(); ImGui::EndTabItem(); }
    if (ui_subtab("Enemies")) { void bestiary_tab(void); bestiary_tab(); ImGui::EndTabItem(); }
    ImGui::EndTabBar();
}
/* F12 > Editor (with the editing tools on): the characters, and the item table for a mod */
static void tab_editor_all(void)
{
    ImGui::TextColored(ImVec4(0.62f, 0.32f, 0.08f, 1), "A deep editor, for fixing and modding:");
    ImGui::SameLine();
    ImGui::TextWrapped("it changes the saved game's own data directly, as the community's save editors do (to repair a character hit by an old bug, to cheat, or to make a mod). Nothing here is needed to play.");
    ImGui::Spacing();
    if (!ui_subtabs_begin("editor")) return;
    if (ui_subtab("Characters")) { tab_editor(); ImGui::EndTabItem(); }
    if (ui_subtab("Items (for a mod)")) { void itemedit_tab(void); itemedit_tab(); ImGui::EndTabItem(); }
    ImGui::EndTabBar();
}

/* one cursor at a time: over the game picture the game draws its own, so the system cursor
 * is hidden there; it shows with the menu or the world map open, and outside the picture */
/* a Darklands+ window over the game (the menu, the world map, a tip ...): the system's pointer
   there, and the game's hand not drawn, so the one pointer seen is where a click lands */
static bool windows_up(void)
{
    return open_menu || open_map || open_goto || open_help || open_welcome || open_mt32 || tips_open() || levelmap_is_open() || halloffame_is_open() || figpick_is_open();
}
static void ui_cursor(void)
{
    int mx, my; SDL_GetMouseState(&mx, &my);
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    bool over_game = mx >= x0 && mx < x1 && my >= y0 && my < y1;
    bool show = windows_up() || !over_game || ui_sys_cursor
                || (ns_showing() && !hdtext_live_pointer);     /* (a Darklands+ page where the game hides its pointer, the saints from a card: the system's) */
    static int shown = -1;
    if (shown != (int)show) { SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE); shown = show; }
    /* a tooltip opens past the pointer: past the game's hand (16 by 12 of its pixels, the fingertip
       the point) where the game draws it, else past the system arrow */
    ImGui::GetStyle().MouseCursorScale = show ? 1.0f : fmaxf(1.0f, (y1 - y0) / 200.0f * 1.1f);
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
extern "C" void ui_icon_grid_toggle(void) { icon_grid = (icon_grid + 1) % 5; }   /* (test: potions, quests, towns, the alchemy sheet) */
static void draw_icon_grid(void)
{
    if (!icon_grid) return;
    void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(244, 234, 210, 255));
    float cell = fminf(ds.x / 12.5f, ds.y / 8.5f), s = cell * 0.62f;
    if (icon_grid == 4) {                                   /* every potion and ingredient with its name, to show around */
        ImFont *ft = ui_font_text ? ui_font_text : ImGui::GetFont(), *fh = ui_font_head ? ui_font_head : ft;
        ImU32 ink = IM_COL32(52, 36, 22, 255), dim = IM_COL32(120, 98, 70, 255), rule = IM_COL32(150, 120, 80, 255);
        float u = ds.y / 1080.0f, fs = 24 * u, top = 124 * u, mx = 34 * u;
        dl->AddText(fh, 40 * u, ImVec2(mx, 22 * u), ink, "Darklands+: the potions and the alchemical ingredients");
        float split = ds.x * 0.62f;
        dl->AddText(fh, 28 * u, ImVec2(mx, top - 44 * u), ink, "Potions, grades 1, 2 and 3 (the gold dots on the neck)");
        dl->AddText(fh, 28 * u, ImVec2(split + 20 * u, top - 44 * u), ink, "Ingredients");
        dl->AddLine(ImVec2(split, top - 30 * u), ImVec2(split, ds.y - 24 * u), rule, 1.5f * u);
        float pw = (split - mx) / 4, ph = (ds.y - top - 20 * u) / 6, ps = fminf(pw / 3.6f, ph * 0.62f);
        for (int k = 0; k < 22; k++) {
            float x0 = mx + (k % 4) * pw, y0 = top + (k / 4) * ph;
            for (int t = 1; t <= 3; t++) ui_item_icon_tier(dl, ImVec2(x0 + ps * (0.62f + 1.08f * (t - 1)), y0 + ps * 0.62f), ps, 95 + k, t);
            const char *nm = 95 + k < (int)gamedata.items.size() ? gamedata.items[95 + k].name.c_str() : "?";
            dl->AddText(ft, fs, ImVec2(x0 + 4 * u, y0 + ps * 1.32f), ink, nm);
        }
        float iw = (ds.x - split - 40 * u) / 2, ih = (ds.y - top - 20 * u) / 12, is = fminf(ih * 0.7f, 60 * u);
        for (int k = 0; k < 23; k++) {
            float x0 = split + 24 * u + (k / 12) * iw, y0 = top + (k % 12) * ih;
            ui_item_icon_tier(dl, ImVec2(x0 + is * 0.5f, y0 + ih * 0.5f), is, 72 + k, 0);
            const char *nm = 72 + k < (int)gamedata.items.size() ? gamedata.items[72 + k].name.c_str() : "?";
            dl->AddText(ft, fs, ImVec2(x0 + is + 12 * u, y0 + ih * 0.5f - fs * 0.55f), ink, nm);
        }
        (void)dim;
        return;
    }
    if (icon_grid == 3) {                                   /* the towns' pictures, at three sizes */
        dl->AddRectFilled(ImVec2(0, ds.y * 0.5f), ds, IM_COL32(24, 18, 12, 255));
        for (int k = 0; k < TI_N; k++) {                     /* (rows of 11: on parchment above, on the dark below) */
            float x = cell * 0.6f + (k % 11) * cell * 0.92f, y = (k / 11) * cell * 1.3f;
            ui_draw_town_icon(dl, ImVec2(x, y + cell * 0.6f), cell * 0.85f, k);
            ui_draw_town_icon(dl, ImVec2(x - cell * 0.25f, y + cell * 1.15f), 32, k);
            ui_draw_town_icon(dl, ImVec2(x + cell * 0.2f, y + cell * 1.15f), 20, k);
            ui_draw_town_icon(dl, ImVec2(x, ds.y * 0.5f + y + cell * 0.6f), cell * 0.85f, k);
        }
        return;
    }
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


#include "vtrace.h"
int vt_on = -1, vt_frame, vt_last_mark, vt_time = -1;
unsigned long long ui_build_us;
extern "C" int ui_vtx, ui_cmds; int ui_vtx, ui_cmds;   /* (DL_PRESENT_TIME: the size of what the interface hands the graphics card) */
extern "C" void ui_draw(SDL_Renderer *ren)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::GetStyle().HoverDelayNormal = config_get_int("display.tooltip_delay", 500) / 1000.0f;   /* (third_party/imgui: a tooltip waits so long) */
    ImGui::NewFrame();
    if (vt_on < 0) { vt_on = getenv("DL_VTX_TRACE") != 0; vt_time = getenv("DL_UI_TIME") != 0; }
    vt_frame++;
    if (game_mode() == GM_MAP) assist_leader_restore();     /* (back on the map: the leader the player chose) */
    if (!open_menu && !open_map && !open_goto) citynav_tick();
    if (open_goto && !goto_allowed()) open_goto = false;
    levelmap_frame();
    if (game_mode() == GM_CARD || game_mode() == GM_MAP) libraries();   /* (kept for the character sheet, which cannot read them: ui/libraries.h) */
    if (open_map) {
        static int tl = -1; if (tl < 0) tl = getenv("DL_WM_TIME") != 0;
        Uint64 t0 = tl ? SDL_GetPerformanceCounter() : 0;
        worldmap_draw();
        if (tl) { static int fr; static double acc; acc += (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency(); if (++fr % 60 == 0) { fprintf(stderr, "wm: %.2f ms a frame\n", acc / 60); acc = 0; } }
    }
    else if (halloffame_is_open()) halloffame_draw();
    else if (levelmap_is_open()) levelmap_draw();
    else { VT(banner_draw()); VT(hdtext_draw()); VT(partypanel_draw()); VT(cardview_draw()); VT(barterview_draw()); VT(sheetview_draw()); VT(lootview_draw()); VT(alchemyview_draw()); VT(cacheview_draw()); VT(prayview_draw()); VT(createview_draw()); VT(rosterview_draw()); VT(infoview_draw()); VT(titleview_draw()); VT(marchview_draw()); VT(lodgeview_draw()); VT(enemyview_draw()); VT(mapmsg_draw()); VT(choose_draw()); VT(savesview_draw()); VT(menuview_draw()); }
    if (!open_menu && !open_map && !levelmap_is_open() && !halloffame_is_open()) VT(ui_overlay());
    {
        if (!windows_up()) hdtext_draw_cursor();
        else hdtext_pointer_off();
        hdtext_pointer_frame_end(); }
    { void soundpreview_tick(void); soundpreview_tick(); }   /* (mods/soundpreview.c) */
    {   /* the music quiet while the settings are open, so that what a preview plays is what is heard */
        sound_music_hushed = open_menu && settings_shown && !soundpreview_active() && !settings_pack_preview;
        { void playlist_frame(int); playlist_frame(open_menu && settings_shown); }   /* (the soundtrack's player stops with the settings: mods/playlist.c) */
    }
    { void screenpics_frame(int); screenpics_frame(open_menu || open_map || open_goto || open_help || open_welcome);
      void screenpics_map_frame(int, int); screenpics_map_frame(open_map, open_menu || open_goto || open_help || open_welcome);
      void screenpics_land_frame(int); screenpics_land_frame(open_menu || open_map || open_goto || open_help || open_welcome || levelmap_is_open()); }   /* (ui/screenpics.cpp) */
    ns_defer_frame_end(open_menu || open_map);                    /* (requests while the game is held: mods/nscreen.h) */
    if (open_menu) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));
        float w = fminf(io.DisplaySize.x * 0.95f, UIS(1100)), h = fminf(io.DisplaySize.y * 0.92f, UIS(820));
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushFont(font_body);
        bool keep = true;
        settings_shown = false;
        if (ImGui::Begin("Darklands+", &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            /* the pages (ui/uitabs.h), and the window's tabs: one a page, or a group of them with
               tabs of its own (Settings). The labels in the tabs' face, the smaller one when they
               would not all show (the whole bar in that face: ImGui measures the tabs again when
               it lays them out) */
            struct Page { const char *name; void (*fn)(void); };
            static const Page pages[UI_PAGE_N] = { { "Journal", tab_journal }, { "Saves", tab_saves },   /* (Saves, Records: no tab of their own now: the game's Save and Load, the right-click menu; the Guide and the Editor) */
                { "Settings", tab_helpers }, { "Mods", tab_mods }, { "Settings", tab_helpers }, { "Appendices", tab_guide_all },
                { "About", tab_about }, { "Editor", tab_editor_all }, { "Appendices", tab_guide_all }, { "Reconstructed Content", tab_restored } };
            struct Group { const char *name; int icon; int page[5]; int n; };
            static const Group groups[] = {      /* (one page each: no tabs under tabs) */
                { "Journal", UI_PAGE_JOURNAL, { UI_PAGE_JOURNAL }, 1 }, { "Settings", UI_PAGE_HELPERS, { UI_PAGE_HELPERS }, 1 },
                { "Reconstructed Content", UI_PAGE_RESTORED, { UI_PAGE_RESTORED }, 1 }, { "Mods", UI_PAGE_MODS, { UI_PAGE_MODS }, 1 },
                { "Editor", UI_PAGE_EDITOR, { UI_PAGE_EDITOR }, 1 }, { "Appendices", UI_PAGE_GUIDE, { UI_PAGE_GUIDE }, 1 },
                { "About", UI_PAGE_ABOUT, { UI_PAGE_ABOUT }, 1 } };
            const int NG = (int)(sizeof groups / sizeof groups[0]);
            auto shown = [](const Group &g) { return g.page[0] != UI_PAGE_EDITOR || !ui_original_mode(); };
            ImFont *tf = font_tab;
            {   /* every label whole, with its picture: the smaller face when the window is narrow */
                float need = 0, avail = ImGui::GetContentRegionAvail().x;
                for (int g = 0; g < NG; g++) if (shown(groups[g]))
                    need += font_tab->CalcTextSizeA(font_tab->FontSize, FLT_MAX, 0, groups[g].name).x + font_tab->FontSize * 1.25f + UIS(12) + ImGui::GetStyle().ItemInnerSpacing.x + 2;
                if (need > avail && font_tab_small) tf = font_tab_small;
            }
            ImGui::PushFont(tf); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UIS(6), UIS(5)));
            if (ImGui::BeginTabBar("tabs")) {
                for (int g = 0; g < NG; g++) {
                    const Group &G = groups[g];
                    if (!shown(G)) continue;
                    int want_sub = -1;
                    for (int j = 0; j < G.n; j++) if (G.page[j] == want_tab) want_sub = j;
                    /* room for its picture at the start of the label: spaces as wide as the picture */
                    char lab[64]; int nsp = (int)ceilf(ImGui::GetFontSize() * 1.25f / fmaxf(1.0f, ImGui::CalcTextSize(" ").x));
                    if (nsp > 12) nsp = 12;
                    snprintf(lab, sizeof lab, "%.*s%s###%s", nsp, "            ", G.name, G.name);
                    bool open_t = ImGui::BeginTabItem(lab, nullptr, want_sub >= 0 && want_frames == 0 ? ImGuiTabItemFlags_SetSelected : 0);
                    {   ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); float fs = ImGui::GetFontSize();
                        ui_page_icon(ImGui::GetWindowDrawList(), ImVec2(a.x + UIS(6) + fs * 0.5f, (a.y + b.y) * 0.5f), fs * 0.6f, G.icon); }
                    if (open_t && G.icon == UI_PAGE_HELPERS) settings_shown = true;
                    if (open_t) {
                        ImGui::PushFont(font_body); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UIS(8), UIS(4)));
                        /* the page in a region of its own: a long one scrolls under the tabs, not with them */
                        auto page = [&](int pg) { ImGui::BeginChild("##page", ImVec2(0, 0)); pages[pg].fn(); ImGui::EndChild(); };
                        if (G.n == 1) page(G.page[0]);
                        else if (ui_subtabs_begin(G.name)) {
                            for (int j = 0; j < G.n; j++)
                                if (ui_subtab_icon(pages[G.page[j]].name, j == want_sub, G.page[j])) { page(G.page[j]); ImGui::EndTabItem(); }
                            ImGui::EndTabBar();
                        }
                        if (want_sub >= 0) want_tab = -1;           /* (drawn, its sub-tab chosen: done) */
                        ImGui::PopStyleVar(); ImGui::PopFont();
                        ImGui::EndTabItem();
                    }
                }
                if (want_tab >= 0 && ++want_frames > 4) want_tab = -1;   /* (a group's tab takes a frame to open) */
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
    else tips_frame();                      /* (the start screen's note, the first battle's tips: ui/tips.cpp) */
    { extern int hg_active, hg_hour, hg_day, hg_month;          /* time passing under a Darklands+ page (mods/lodgeview.c) */
      if (hg_active) {
          ImDrawList *dl = ImGui::GetForegroundDrawList();
          float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
          dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(20, 12, 4, 120));
          float kx = (x1 - x0) / 320.0f, ky = (y1 - y0) / 200.0f;
          ui_draw_hourglass(dl, ImVec2(x0 + 130 * kx, y0 + 49 * ky), kx, ky, hg_hour, hg_day, hg_month);
      } }
    { if (saves_loading_cover()) {                 /* a load under way: the game's load screen hidden */
          ImDrawList *dl = ImGui::GetForegroundDrawList(); ImGuiIO &io = ImGui::GetIO();
          dl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(20, 14, 10, 255));
          const char *t = "Loading...";
          ImVec2 ts = ImGui::CalcTextSize(t);
          dl->AddText(ImVec2((io.DisplaySize.x - ts.x) * 0.5f, (io.DisplaySize.y - ts.y) * 0.5f), IM_COL32(230, 210, 170, 255), t);
      } }
    if (!open_menu && !open_map) VT(citynav_draw_cover());
    ui_icon_styles_draw();
    { void ui_pstone_sheet_draw(void); ui_pstone_sheet_draw(); }   /* (test: DL_PSTONE_SHEET) */
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
    { void repwatch_tick(void); repwatch_tick(); }
    VT(ui_cursor());
    VT(ui_alert_draw());
    { void game_dialog_draw(void); game_dialog_draw(); }
    draw_icon_grid();
    if (vt_on > 0 && vt_frame % 120 == 0) fprintf(stderr, "vtx total %d\n", vt_total());
    dc_frame_end();
    worldmap_icon_sheet_draw(); orn_test_sheet_draw(); niche_test_sheet_draw(); clock_test_sheet_draw();
    orn_windows();
    ui_rich_tip_flush();                                   /* (a shield's name under the pointer, if nothing else took the place: ui/richtext.h) */
    ui_tip_audit(); ui_panel_audit(); ui_pointer_audit();   /* (the checks: tools/uitour.py) */
    ImGui::Render();
    ui_build_us = SDL_GetPerformanceCounter();
    { ImDrawData *dd = ImGui::GetDrawData(); ui_vtx = dd->TotalVtxCount; ui_cmds = 0; for (int i = 0; i < dd->CmdListsCount; i++) ui_cmds += dd->CmdLists[i]->CmdBuffer.Size; }
    {   /* test (DL_DRAW_CHECK): every draw command's triangles, their extent: one far past the window, or not
           a number, is reported once (the inn's wash on Windows was looked for so: it was letters drawn hundreds
           of times their size, which ui/drawguard.cpp now takes out and reports) */
        static int chk = -1; if (chk < 0) chk = getenv("DL_DRAW_CHECK") != 0;
        if (chk) {
            ImDrawData *dd = ImGui::GetDrawData(); ImVec2 ds = ImGui::GetIO().DisplaySize;
            { static u32 lt; u32 t = SDL_GetTicks(); if (t - lt > 2000) { lt = t; int big = 0, bigv = 0; for (int li = 0; li < dd->CmdListsCount; li++) if (dd->CmdLists[li]->VtxBuffer.Size > bigv) { bigv = dd->CmdLists[li]->VtxBuffer.Size; big = li; }
                fprintf(stderr, "draw check: %d vertices, %d indices, %d lists; the most in list %d (%s): %d\n", dd->TotalVtxCount, dd->TotalIdxCount, dd->CmdListsCount, big, dd->CmdListsCount ? (dd->CmdLists[big]->_OwnerName ? dd->CmdLists[big]->_OwnerName : "?") : "", bigv); } }
            for (int li = 0; li < dd->CmdListsCount; li++) {
                const ImDrawList *L = dd->CmdLists[li];
                for (int ci = 0; ci < L->CmdBuffer.Size; ci++) {
                    const ImDrawCmd &c = L->CmdBuffer[ci];
                    if (c.UserCallback || !c.ElemCount) continue;
                    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f; int bad = 0, maxi = 0;
                    for (unsigned k = 0; k < c.ElemCount; k++) {
                        unsigned vi = c.VtxOffset + L->IdxBuffer[c.IdxOffset + k];
                        if ((int)vi >= L->VtxBuffer.Size) { bad = 2; break; }
                        if ((int)vi > maxi) maxi = (int)vi;
                        ImVec2 p = L->VtxBuffer[vi].pos;
                        if (!(p.x == p.x) || !(p.y == p.y)) { bad = 1; continue; }
                        x0 = fminf(x0, p.x); y0 = fminf(y0, p.y); x1 = fmaxf(x1, p.x); y1 = fmaxf(y1, p.y);
                    }
                    bool huge = x1 - x0 > ds.x * 1.5f || y1 - y0 > ds.y * 1.5f || x0 < -ds.x || y0 < -ds.y || x1 > 2 * ds.x || y1 > 2 * ds.y;
                    if (bad || huge) {
                        static int said; if (said++ < 40)
                            fprintf(stderr, "draw check: list %d (%s) cmd %d: %s, %u indices, box %.0f,%.0f - %.0f,%.0f, texture %p, clip %.0f,%.0f-%.0f,%.0f\n", li, L->_OwnerName ? L->_OwnerName : "?", ci,
                                    bad == 2 ? "an index past the vertices" : bad ? "not a number" : "far past the window", c.ElemCount, x0, y0, x1, y1, (void *)c.GetTexID(), c.ClipRect.x, c.ClipRect.y, c.ClipRect.z, c.ClipRect.w);
                    }
                }
            }
        }
    }
    { void ui_draw_guard(ImDrawData *); ui_draw_guard(ImGui::GetDrawData()); }   /* (what no screen means to draw: taken out, said in the log - ui/drawguard.cpp) */
    SDL_ClearError();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
    {   /* what the renderer refused this frame (a draw it could not make: on some graphics cards the
           screen was washed out over the inn and the camp): said in the log, each new message once */
        const char *e = SDL_GetError();
        static char last[256]; static int said;
        if (e && e[0] && strcmp(e, last) && said < 20) {
            snprintf(last, sizeof last, "%s", e); said++;
            ImDrawData *dd = ImGui::GetDrawData();
            fprintf(stderr, "renderer: %s (a frame of %d vertices, %d indices)\n", e, dd->TotalVtxCount, dd->TotalIdxCount);
        }
    }
}

/* the first start of a release: the game module is being made (runtime/prepare_run.c) */
extern "C" void ui_progress_draw(SDL_Renderer *ren, const char *step, int pct, const char *note)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::GetStyle().HoverDelayNormal = config_get_int("display.tooltip_delay", 500) / 1000.0f;   /* (third_party/imgui: a tooltip waits so long) */
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
    orn_windows();
    ImGui::Render();
    SDL_SetRenderDrawColor(ren, 20, 16, 12, 255);
    SDL_RenderClear(ren);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
    SDL_RenderPresent(ren);
}
