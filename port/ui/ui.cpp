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
#include "../mods/saves.h"
#include "../mods/hdtext.h"
#include "../mods/hdart.h"
#include "../mods/sfx.h"
#include "../mods/datamods.h"
#include "ui.h"
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE;
}

static bool open_menu, open_map, open_goto, open_help;
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
static ImFont *font_body, *font_head, *font_tab;
ImFont *ui_font_small, *ui_font_text;
ImFont *ui_font_game[UI_GAME_FONTS];       /* the crisp game text: EB Garamond, Alegreya, IM Fell, Atkinson, MedievalSharp, Fraktur */
int ui_game_font, ui_text_edge;            /* edge: 0 none, 1 shadow, 2 outline */
extern int ui_spell_out;                   /* overlay.cpp: character sheet names in full */
static int sys_cursor;                     /* display.system_cursor: the system pointer stays over the game */
const char *UI_GAME_FONT_NAMES[UI_GAME_FONTS] = { "EB Garamond (book face)", "Alegreya (calligraphic)", "IM Fell English (17th-century print)",
    "Atkinson Hyperlegible (easiest to read)", "MedievalSharp (medieval hand)", "Fraktur (blackletter, for the brave)" };
extern "C" void tab_inventory(void);
extern "C" void tab_members(void);
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
extern "C" void tab_alchemy(void);
extern "C" void ui_overlay(void);
extern "C" void hdtext_draw(void);
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
    ImFontConfig fc; fc.FontDataOwnedByAtlas = false;
    font_body = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 24.0f, &fc);
    font_head = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, 32.0f, &fc);
    font_tab = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 20.0f, &fc);
    ui_font_small = io.Fonts->AddFontFromMemoryTTF((void *)FONT_SEMIBOLD, (int)FONT_SEMIBOLD_SIZE, 17.0f, &fc);
    {   /* the game's text, redrawn crisp (mods/hdtext.c): baked large, drawn at any size */
        ImFontConfig tc; tc.FontDataOwnedByAtlas = false; tc.OversampleH = 2; tc.OversampleV = 2;
        static const ImWchar ranges[] = { 0x20, 0xff, 0 };
        ui_font_text = io.Fonts->AddFontFromMemoryTTF((void *)FONT_MEDIUM, (int)FONT_MEDIUM_SIZE, 34.0f, &tc, ranges);
        const unsigned char *data[UI_GAME_FONTS] = { FONT_MEDIUM, FONT_ALEGREYA, FONT_FELL, FONT_ATKINSON, FONT_MEDIEVAL, FONT_FRAKTUR };
        const unsigned int size[UI_GAME_FONTS] = { FONT_MEDIUM_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_ATKINSON_SIZE, FONT_MEDIEVAL_SIZE, FONT_FRAKTUR_SIZE };
        ui_font_game[0] = ui_font_text;
        for (int i = 1; i < UI_GAME_FONTS; i++) ui_font_game[i] = io.Fonts->AddFontFromMemoryTTF((void *)data[i], (int)size[i], 34.0f, &tc, ranges);
    }
    ui_game_font = config_get_int("display.text_font", 0);
    if (ui_game_font < 0 || ui_game_font >= UI_GAME_FONTS) ui_game_font = 0;
    ui_text_edge = config_get_int("display.text_edge", 2);
    { extern int ui_text_contrast; ui_text_contrast = config_get_int("display.text_contrast", 1); }
    { extern int ui_skill_icons; ui_skill_icons = config_get_int("display.skill_icons", 1); }
    { extern int ui_shop_info; ui_shop_info = config_get_int("assist.shop_info", 1); }
    { extern int ui_creation_guide; ui_creation_guide = config_get_int("assist.creation_guide", 1); }
    { extern int ui_battle_info, ui_battle_numbers; ui_battle_info = config_get_int("battle.info", 1); ui_battle_numbers = config_get_int("battle.info_numbers", 0); }
    ui_spell_out = config_get_int("display.spell_out", 1);
    sys_cursor = config_get_int("display.system_cursor", 0);
    { extern int ui_margins_on; ui_margins_on = config_get_int("display.margins", 1); }
    gamedata_load(game_dir);
    journal_init();
    if (getenv("DL_RESTARTED")) ui_notice_for("The party's journey has ended. Load a saved game to go on: F12 > Saves, or Load in the game.", 20000);
    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer2_Init(ren);
    style();
    ui_settings.aspect = config_get_int("display.aspect_4_3", 1);
    ui_settings.smooth = config_get_int("display.smooth", 0);
    ui_settings.integer = config_get_int("display.integer_scale", 0);
    ui_settings.scaler = config_get_int("display.scaler", 0);
}

extern "C" int ui_is_open(void) { return open_menu || open_map || open_goto || open_help; }
extern "C" void ui_toggle(void) { open_map = false; open_goto = false; open_help = false; open_menu = !open_menu; if (!open_menu && dirty_config) { config_save(); dirty_config = 0; } }
/* the world map (ui/worldmap.cpp): M on the travel map, or the Travel tab */
extern "C" void ui_toggle_map(void)
{
    if (open_map) { open_map = false; return; }
    if (worldmap_open()) { open_map = true; if (open_menu) ui_toggle(); open_map = true; }
}

extern "C" int ui_event(const SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F12 && !(e->key.keysym.mod & KMOD_CTRL)) { ui_toggle(); return 1; }
    if (!open_menu && !open_map && e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_m && !(e->key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_SHIFT))
        && (game_mode() == GM_MAP || game_mode() == GM_CARD) && config_get_int("map.key_m", 1)) { ui_toggle_map(); return 1; }
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F1 && !e->key.repeat && !open_menu && !open_map && !open_goto) { open_help = !open_help; return 1; }
    if (open_help) {
        if (e->type == SDL_KEYDOWN && (e->key.keysym.sym == SDLK_ESCAPE || e->key.keysym.sym == SDLK_F1)) { open_help = false; return 1; }
        if (e->type == SDL_MOUSEBUTTONDOWN) { open_help = false; return 1; }
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
    ImGui::PushFont(font_head); ImGui::SeparatorText("Mod folders"); ImGui::PopFont();
    {
        static char msg[1400];
        char md[1100]; datamods_folder(md, sizeof md);
        ImGui::TextWrapped("A mod is a folder in %s" PATH_SEP "mods with files laid out like the game folder: "
                           "while it is ticked the game reads them instead of its own (the game's files are never changed).", game_dir);
        if (!datamods_n) hint("No mods found yet.");
        for (int i = 0; i < datamods_n; i++) {
            ImGui::PushID(1000 + i);
            bool on = datamods[i].on != 0;
            if (ImGui::Checkbox(datamods[i].title, &on)) { datamods_set(i, on); dirty_config = 1; }
            ImGui::Indent(34);
            if (datamods[i].desc[0]) hint("%s", datamods[i].desc);
            hint("folder: mods" PATH_SEP "%s", datamods[i].folder);
            ImGui::Unindent(34);
            ImGui::PopID();
        }
        if (ImGui::Button("Look for mods again")) { datamods_scan(); hdart_rescan(); msg[0] = 0; }
        ImGui::SameLine();
        static char newname[64] = "My mod";
        ImGui::SetNextItemWidth(200); ImGui::InputText("##newmod", newname, sizeof newname);
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
        config_set("original.sound", config_get("sound.device", "fm"));
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
        datamods_scan(); hdart_rescan(); rules_apply();
    } else {
        const char *p = config_get("original.saved", "");
        char *e;
        auto next = [&](int def) { long v = strtol(p, &e, 10); if (e == p) return def; p = e; return (int)v; };
        int ch = next(2), jo = next(1), pp = next(1), pe = next(0), ta = next(1), tf = next(1);
        assist_set_chances(ch); journal_set(jo); pathfix_set(pp, pe); travel_set(ta, tf);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, next(1));
        for (int i = 0; i < MOD_COUNT; i++) if (next(0)) mod_enable(i, 1);
        config_set("sound.device", config_get("original.sound", "fm"));
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
        datamods_scan(); hdart_rescan(); rules_apply();
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
    bool ch = assist_card_help;
    if (ImGui::Checkbox("Show which saints and items work on a card, and who knows or carries them", &ch)) { assist_set_card_help(ch); dirty_config = 1; }
    { extern int ui_shop_info; bool si = ui_shop_info != 0;
      if (ImGui::Checkbox("In shops: what the item under the mouse is good for, and who in the party could use it", &si)) { ui_shop_info = si; config_set_int("assist.shop_info", si); dirty_config = 1; } }
    { extern int ui_creation_guide; bool cg = ui_creation_guide != 0;
      if (ImGui::Checkbox("Character creation: a short guide to each step in the empty column on the left", &cg)) { ui_creation_guide = cg; config_set_int("assist.creation_guide", cg); dirty_config = 1; } }
    bool ck = assist_card_keys;
    if (ImGui::Checkbox("Number keys 1-9 pick a card's options", &ck)) { assist_set_card_keys(ck); dirty_config = 1; }
    { bool wl = config_get_int("assist.wheel_lists", 1) != 0;
      if (ImGui::Checkbox("The mouse wheel moves in the game's lists (equipment, shops ...)", &wl)) { config_set_int("assist.wheel_lists", wl); dirty_config = 1; } }
    hint("Like the Darklands Companion: the saints a card accepts are listed even when nobody in the party knows them yet.");

    ImGui::PushFont(font_head); ImGui::SeparatorText("Battles"); ImGui::PopFont();
    { bool d = autopause_down, h = autopause_hurt;
      if (ImGui::Checkbox("Pause the battle when someone in the party falls", &d)) { autopause_set(d, h); dirty_config = 1; }
      if (ImGui::Checkbox("Pause the battle when someone is badly hurt (endurance below a quarter)", &h)) { autopause_set(d, h); dirty_config = 1; } }
    { extern int ui_battle_info, ui_battle_numbers; bool bi = ui_battle_info != 0, bn = ui_battle_numbers != 0;
      if (ImGui::Checkbox("Pointing at someone in a battle shows who it is, how hurt, attack, defence and weapon", &bi)) { ui_battle_info = bi; config_set_int("battle.info", bi); dirty_config = 1; }
      if (bi) { ImGui::SameLine(); if (ImGui::Checkbox("enemies' endurance as numbers", &bn)) { ui_battle_numbers = bn; config_set_int("battle.info_numbers", bn); dirty_config = 1; } } }
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
    ImGui::TextUnformatted("Pace of auto-travel:"); ImGui::SameLine();
    if (ImGui::RadioButton("as walking", tf == 0)) { travel_set(ta, 0); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("4x", tf == 1)) { travel_set(ta, 1); dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("at once", tf == 2)) { travel_set(ta, 2); dirty_config = 1; }

    ImGui::PushFont(font_head); ImGui::SeparatorText("Town journal"); ImGui::PopFont();
    bool j = journal_enabled();
    if (ImGui::Checkbox("Keep a journal of the towns the party visits", &j)) { journal_set(j); dirty_config = 1; }
    hint("Each town's shops, their wares, qualities and prices, noted on arrival (Journal tab).");

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
      if (ImGui::Checkbox("Wide screens: date, place, purse, tasks and effects in the side margins", &mg)) { ui_margins_on = mg; config_set_int("display.margins", mg); dirty_config = 1; } }
    { bool sc = sys_cursor != 0;
      if (ImGui::Checkbox("Show the system mouse pointer over the game too", &sc)) { sys_cursor = sc; config_set_int("display.system_cursor", sc); dirty_config = 1; } }
    { bool pu = config_get_int("display.pause_unfocused", 1) != 0;
      if (ImGui::Checkbox("Pause the game while its window is in the background", &pu)) { config_set_int("display.pause_unfocused", pu); dirty_config = 1; } }
    bool fs = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(g_win, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    ImGui::PushFont(font_head); ImGui::SeparatorText("Text"); ImGui::PopFont();
    int hm = hdtext_mode;
    if (ImGui::RadioButton("Crisp text on story cards only", hm == 1)) { hdtext_set(1); dirty_config = 1; }
    if (ImGui::RadioButton("Crisp text everywhere (cards, party, sheets, menus, shops)", hm == 2)) { hdtext_set(2); dirty_config = 1; }
    if (ImGui::RadioButton("The game's own lettering", hm == 0)) { hdtext_set(0); dirty_config = 1; }
    if (hm) {
        ImGui::SetNextItemWidth(380);
        if (ImGui::BeginCombo("Typeface", UI_GAME_FONT_NAMES[ui_game_font])) {
            for (int i = 0; i < UI_GAME_FONTS; i++) {
                ImGui::PushFont(ui_font_game[i]);
                if (ImGui::Selectable(UI_GAME_FONT_NAMES[i], i == ui_game_font)) { ui_game_font = i; config_set_int("display.text_font", i); dirty_config = 1; }
                ImGui::PopFont();
            }
            ImGui::EndCombo();
        }
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
        hint("More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the fifteen of the game "
             "(Heraldry, when making a party). A character with one of them needs this version of the game to be loaded.", PATH_SEP);
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

void rules_tab(void);
static void tab_party(void)
{
    if (!ImGui::BeginTabBar("records")) return;
    if (ImGui::BeginTabItem("Weapons")) { rules_tab(); ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("Items")) { void itemedit_tab(void); itemedit_tab(); ImGui::EndTabItem(); }
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
    ImGui::EndTabItem();
    ImGui::EndTabBar();
}

static void tab_about(void)
{
    ImGui::TextWrapped("Darklands (c) 1992 MicroProse. This is the original game, recompiled to run natively, "
                       "with the original game files from your copy.");
    ImGui::Spacing();
    ImGui::TextWrapped("F12 opens and closes this menu (Esc closes it). The game is paused while it is open.");
    ImGui::TextWrapped("Alt+Enter: full screen.  Ctrl+F12: screenshot.  Ctrl+S: quick save.  F9 or Ctrl+L: quick load.  M on the travel map: world map.");
    ImGui::Spacing();
    hint("Font: EB Garamond (SIL Open Font License). Interface: Dear ImGui (MIT). "
                        "Music synthesis: your system's MIDI synthesizer.");
}

/* F1: the keys and clicks of the moment (the game's own and Darklands+'s) */
static void help_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 100));
    float w = fminf(io.DisplaySize.x * 0.92f, 900);
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, io.DisplaySize.y * 0.94f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    if (ImGui::Begin("Keys and clicks (F1)", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetWindowFontScale(fminf(1.0f, fmaxf(0.7f, io.DisplaySize.y / 1000.0f)));
        auto row = [](const char *k, const char *d) { ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "%s", k); ImGui::SameLine(230); ImGui::TextWrapped("%s", d); };
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
        } else if (hdart_on_screen("ARMBACK")) {        /* the character sheet (s031's keys) */
            ImGui::SeparatorText("Now: a character sheet");
            row("Ctrl+F1 ... Ctrl+F5", "make member 1 ... 5 the leader (or F12 > Party)");
            row("Up, Down, PgUp, PgDn", "move in a list (the mouse wheel too)");
            row("1 ... 5", "in the equipment list: give the chosen item to member 1 ... 5 (with Shift: all of them)");
            row("A", "in the equipment list: ready or wear the chosen item");
            row("D", "in the equipment list: drop the chosen item (with Shift: all of them)");
        }
        ImGui::SeparatorText("The game's own keys");
        row("Right mouse button", "the menu bar (F10 with the keyboard)");
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
    bool show = open_menu || open_map || open_goto || open_help || !over_game || sys_cursor;
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
    if (open_map) worldmap_draw();
    else hdtext_draw();
    if (!open_menu && !open_map) ui_overlay();
    if (open_menu) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));
        float w = fminf(io.DisplaySize.x * 0.95f, 1100), h = fminf(io.DisplaySize.y * 0.92f, 820);
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushFont(font_body);
        bool keep = true;
        if (ImGui::Begin("Darklands+", &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            /* the labels a little smaller than the text, so that all of them show in a 960 pixel
               window (the whole tab bar in that face: ImGui measures the tabs again when it lays
               them out) */
            ImGui::PushFont(font_tab); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 5));
            if (ImGui::BeginTabBar("tabs")) {
                const char *names[] = { "Inventory", "Party", "Alchemy", "Saints", "Journal", "Saves", "Travel", "Helpers", "Mods", "Display", "Records", "About", "Editor" };
                void (*fns[])(void) = { tab_inventory, tab_members, tab_alchemy, tab_saints, tab_journal, tab_saves, tab_travel, tab_helpers, tab_mods, tab_display, tab_party, tab_about, tab_editor };
                for (int t = 0; t < (int)(sizeof fns / sizeof fns[0]); t++) {
                    if (fns[t] == tab_editor && (!config_get_int("editor.show", 0) || original_mode())) continue;
                    if (ImGui::BeginTabItem(names[t], nullptr, want_tab == t ? ImGuiTabItemFlags_SetSelected : 0)) {
                        ImGui::PushFont(font_body); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
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
    if (open_help) help_window();
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
