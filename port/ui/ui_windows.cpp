// ui_windows.cpp - the interface's side windows: About, the welcome on the first start, the news
// of a new version, getting the MT-32 working, and F11's keys and clicks for the screen in view.
// Split from ui/ui.cpp, which opens them (its flags open_help, open_welcome, open_mt32, open_news).
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <string>
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

extern "C" int saves_loading_cover(void);   /* mods/saves.c */
void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);   /* ui/hourglass.cpp */
extern "C" void banner_draw(void);                                   /* ui/banner.cpp */
extern "C" void worldmap_icon_sheet_draw(void); extern "C" void orn_test_sheet_draw(void); extern "C" void niche_test_sheet_draw(void); extern "C" void clock_test_sheet_draw(void);                      /* ui/worldmap.cpp (a test) */
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
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[], FONT_GRENZE[], FONT_COMIC[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE, FONT_COMIC_SIZE;
}
#include "ornament.h"

extern bool open_help, open_welcome, open_mt32, open_news;
#include "ui_decls.h"
extern ImFont *ui_font_small, *ui_font_text, *ui_font_overlay, *ui_font_head, *ui_font_bold;
extern ImFont *ui_font_game[UI_GAME_FONTS], *ui_font_game_hi[UI_GAME_FONTS];
extern int ui_game_font, ui_text_edge, ui_sys_cursor, ui_dirty_config;
extern float ui_scale;
extern SDL_Window *ui_window;
extern ImFont *font_body, *font_head, *font_tab, *font_tab_small;   /* (ui/ui.cpp) */

extern "C" const char dlp_version[];
void tab_about(void)
{
    ImGui::PushFont(font_head); ImGui::Text("Darklands+"); ImGui::PopFont();
    ImGui::SameLine(); ImGui::TextDisabled("build %s", dlp_version);
    ImGui::TextWrapped("Darklands (c) 1992 MicroProse. This is the original game, recompiled to run natively, with the "
                       "game files of your own copy; everything Darklands+ adds can be switched off (F12 > Settings: the Full Original preset is the game as it was).");
    ImGui::Spacing();
    ImGui::TextWrapped("F11: the keys for the current screen.  F12: this menu (the game waits while it is open).  Alt+Enter: full screen.  "
                       "Ctrl+F12: screenshot.  Ctrl+S: quick save.  F9 or Ctrl+L: quick load.  M: world map.  G: go to (in cities).");
    ImGui::PushFont(font_head); orn_septext("Files"); ImGui::PopFont();
    {   extern char dos_shadow[];
        if (dos_shadow[0]) ImGui::BulletText("saved games, and every other file the game writes: %s (Darklands+ cannot write to the game "
                                             "folder; the saved games in its SAVES folder can still be loaded)", dos_shadow);
        else ImGui::BulletText("saved games: the SAVES folder of the game folder, as in the original"); }
    {   extern char dlp_data_dir[];
        bool own = strcmp(dlp_data_dir, game_dir) != 0;    /* the game folder could not be written */
        ImGui::BulletText("the game: %s", game_dir);
        ImGui::BulletText("settings: darklands_port.ini in %s", own ? dlp_data_dir : "the game folder");
        extern char dlp_user_dir[];
        ImGui::BulletText("mods, replacement art and sounds: the mods, art and sounds folders in %s", own ? dlp_user_dir : "the game folder");
        ImGui::BulletText("screenshots (Ctrl+F12, the window as you see it): the screenshots folder in %s", own ? dlp_data_dir : "the game folder");
        ImGui::BulletText("the session log: darklands_port.log (Windows) in %s; send it along when you report a problem", own ? dlp_data_dir : "the game folder");
#ifdef DL_RTSPLIT
        ImGui::BulletText("the game's program, made from your DARKLAND.EXE on the first start: %s (delete it to have it made again)",
#ifdef _WIN32
                          "darklands_game.dll");
#else
                          "libdarklands_game.so");
#endif
#endif
    }
    ImGui::PushFont(font_head); orn_septext("Made with"); ImGui::PopFont();
    ui_hint("Dear ImGui (MIT license); Nuked OPL3, the Sound Blaster FM synthesis (LGPL 2.1); Munt's libmt32emu, the Roland MT-32 (LGPL 2.1); stb_image, stb_image_write, "
         "stb_vorbis (public domain); SDL2 (zlib license); fonts EB Garamond, Alegreya, Atkinson Hyperlegible, IM Fell English, "
         "MedievalSharp, UnifrakturMaguntia, Grenze Gotisch and Comic Neue (SIL Open Font License 1.1).");
    ui_hint("Darklands was designed by Arnold Hendrick and made by MicroProse; everything here stands on their game.");
    ui_hint("With thanks to the Darklands community: vvendigo's tools, Merle's and Joel \"Quadko\" McIntyre's documentation "
         "of the file formats, the Darklands Companion, and Arkana Mechanika (Darklands Restoration) for bug reports.");
}

/* F11: the keys and clicks of the moment (F1 ... F5 are the game's: a member's sheet) (the game's own and Darklands+'s) */
/* the first start: what Darklands+ is, the keys that matter, and the choice to play the
 * original game untouched (the purists' switch), once */
void welcome_window(void)
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
                               "Windows does not let programs write to the game folder where it is installed, so your saved games go to %s "
                               "(the ones already in the game folder can be loaded as always).", dos_shadow);
        else
            ImGui::TextWrapped("This is Darklands (MicroProse, 1992) running natively, with the game files of your own copy. "
                               "Your saved games stay in the game folder as always.");
        ImGui::Spacing();
        ImGui::TextWrapped("Darklands+ adds what players have long wished for (crisp text, chances on the cards, a character sheet "
                           "for the whole party, journals, a world map, a battle log, quick saves, fixes for old bugs), each of which can be switched off.");
        bool done = false;
        {   /* how much of Darklands+: the presets (mods/options.h), the recommended one chosen */
            ImGui::PushFont(font_head); orn_septext("How would you like to play?"); ImGui::PopFont();
            static int pick = -1;
            if (pick < 0) { pick = preset_current(); if (pick < 0) pick = PRESET_PLUS; }
            for (int p = 0; p < PRESET_COUNT; p++) {
                ImGui::PushID(p);
                std::string nm = std::string(PRESETS[p].name) + (p == PRESET_PLUS ? " (recommended)" : "");
                if (ImGui::RadioButton("##p", pick == p)) pick = p;
                ImGui::SameLine();
                if (ui_title_desc(nm.c_str(), PRESETS[p].desc)) pick = p;
                ImGui::PopID();
            }
            ImGui::TextColored(ImVec4(0.18f, 0.40f, 0.16f, 1), "Saved games stay compatible with the original game whichever you choose.");
            if (ui_pad_prompts()) ui_pad_text("Change it at any time, or turn single features on or off: {START} > Settings. {LSTICK} moves the pointer, {A} clicks.", true);
            else ImGui::TextDisabled("Change it at any time, or turn single features on or off: F12 > Settings.");
            ImGui::Spacing();
            if (ImGui::Button("  Play  ") || ImGui::IsKeyPressed(ImGuiKey_Enter)) { if (pick != preset_current()) preset_apply(pick); done = true; }
        }
        ImGui::Spacing();
        ImGui::PushFont(font_head); orn_septext("Keys"); ImGui::PopFont();
        ImGui::BulletText("F12: the Darklands+ menu (the game waits while it is open)");
        ImGui::BulletText("F11: the keys and clicks for the current screen, including the game's hidden ones");
        ImGui::BulletText("F12 > New Player Guide: how Darklands works, for a new player");
        ImGui::BulletText("Click a party member on the left: their character sheet, with the whole party's saints and formulae");
        ImGui::BulletText("M: the world map, to find a place and travel there");
        ImGui::BulletText("Right-click: the game's menu bar (save, load, orders in battle)");
        ImGui::BulletText("Alt+Enter: full screen   Ctrl+S: quick save   F9: quick load");
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) done = true;
        if (done) { open_welcome = false; config_set_int("ui.welcome_seen", 1); if (!news_key().empty()) config_set("ui.news_seen", news_key().c_str()); config_save(); }
    }
    ImGui::End();
    ImGui::PopFont();
}

/* what is new in this version (its first two numbers: "1.2" from "1.2-preview35"; "" for a build
   that is no release), shown once to a player of an earlier one */
extern "C" const char dlp_release[];
std::string news_key(void)
{
    std::string r = dlp_release;
    size_t a = r.find('.'); if (a == std::string::npos) return "";
    size_t b = a + 1; while (b < r.size() && isdigit((unsigned char)r[b])) b++;
    return r.substr(0, b);
}
void news_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));
    float w = fminf(io.DisplaySize.x * 0.9f, UIS(760));
    ImGui::SetNextWindowSize(ImVec2(w, 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    std::string title = "New in Darklands+ " + news_key();
    if (ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        static const char *NEWS[] = {
            "City politics, restored (experimental, off by default): the campaign MicroProse wrote and never switched on. Take the rebels' side "
            "or the rulers' in a city in turmoil, sway the guilds, march on the town hall or arrest the ringleaders. F12 > Reconstructed Content.",
            "More of what MicroProse left unused, on a page of its own (F12 > Reconstructed Content, experimental, all off by default): cities preparing for war or "
            "under siege, \"Fehde!\", soldiers of a private war waiting in ambush on the road, and a town's hospice.",
            "With a controller, the screens show its buttons, drawn as an Xbox, PlayStation or Nintendo controller has them.",
            "M: the world map. Choose a place on it to see what it offers, with a Travel there button at the top.",
            "Calling on a saint says why the chance is zero: too little Virtue, or too little Divine Favor.",
            "Your saved games are copied into \"Darklands+ save backups\" the first time each new version starts.",
            "Click a party member on the left: the character sheet, the one place for the party's items, skills, saints and formulae." };
        for (const char *t : NEWS) { ImGui::Bullet(); ImGui::TextWrapped("%s", t); }
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Every change can be switched off: F12 > Settings. F12 > New Player Guide explains the game; F11 lists the keys for the current screen.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        bool done = ImGui::Button("  Back to the game  ") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape);
        if (done) { open_news = false; config_set("ui.news_seen", news_key().c_str()); config_save(); }
    }
    ImGui::End();
    ImGui::PopFont();
}

/* Darklands was made for the Roland MT-32: say so, and help find the ROM files */
extern "C" int plat_choose_folder(const char *title, char *out, int n);
void mt32_window(void)
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
                           "control and PCM, of an MT-32 or a CM-32L (either works; a CM-32L turns the battle sounds into chimes, so with both, play on the MT-32). They belong to Roland, so they cannot come "
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
            if (ImGui::Button("Look again")) { config_save(); mt32_rescan(); msg = mt32_available() ? "" : "No ROM files found there yet."; }
            if (!msg.empty()) ui_hint("%s", msg.c_str());
            ui_hint("Darklands+ also looks in Munt's ROM folder, DOSBox Staging's mt32-roms folder, and a \"roms\" folder in the Darklands+ folder or the game folder.");
        }
        ImGui::Spacing();
        bool done = false;
        if (have && !mt32_is_open()) {
            if (ImGui::Button("Restart with the MT-32")) { config_save(); plat_restart(); }
            ImGui::SameLine();
        }
        if (ImGui::Button(have ? "Close" : "Not now")) done = true;
        if (!have) {
            ImGui::SameLine();
            if (ImGui::Button("Don't remind me again")) { config_set_int("sound.mt32_prompt", 0); done = true; }
        }
        ImGui::TextDisabled("F12 > Settings has all of this too.");
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) done = true;
        if (done) { open_mt32 = false; config_save(); }
    }
    ImGui::End();
    ImGui::PopFont();
}

void help_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 100));
    float w = fminf(io.DisplaySize.x * 0.92f, UIS(900));
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, io.DisplaySize.y * 0.94f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushFont(font_body);
    if (ImGui::Begin("Keys and clicks (F11)", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetWindowFontScale(fminf(1.0f, fmaxf(0.7f, io.DisplaySize.y / 1000.0f)));
        auto row = [](const char *k, const char *d) {     /* (a label too long for its column wraps within it) */
            ImGui::BeginGroup();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + UIS(222));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.18f, 0.10f, 1)); ImGui::TextWrapped("%s", k); ImGui::PopStyleColor();
            ImGui::PopTextWrapPos();
            ImGui::EndGroup();
            ImGui::SameLine(UIS(230)); ImGui::TextWrapped("%s", d);
        };
        int gm = game_mode();
        int sv_loading = 0;
        if (savesview_showing(&sv_loading)) {
            orn_septext(sv_loading ? "Now: loading a game" : "Now: saving the game");
            if (sv_loading) { row("Double-click a save", "load it (or click it, then Load)"); row("Up, Down, Enter", "choose a save, load it"); }
            else { row("Type", "a name for the save (22 letters at most; Backspace deletes one)"); row("Enter or Save", "save it"); }
            row("Esc or Cancel", "back to the game as it was");
        } else if (lodgeview_showing()) {          /* (a stay: the game counts it as a card) */
            orn_septext(lodgeview_camping() ? "Now: a camp" : "Now: a stay at the inn");
            row("A member, or 1 ... 5", "choose the member whose day you are planning");
            row("J R P A E G T", "Regain Endurance (Just Relax), Regain Strength, Pray, Alchemy work, Earn a little money, Guard the camp, Train or study (the original's keys)");
            row("S or A day", "one day passes; A week: seven days; Until rested: members short of Strength who only relax switch to Regain Strength, then days pass until nobody is regaining Strength or praying");
            row("L, Esc or Leave", "leave");
        } else if (gm == GM_CARD) {
            orn_septext("Now: a card");
            row("Click an option", "or press its number, 1 to 9");
            row("Hold Shift over an option", "the chance of success as the original game shows it (the badges show every option's chance)");
            row("G", "go to a place in this city");
            row("M", "world map");
            row("Town pictures", "(in the right margin) what this town has that most towns lack, and its notable shops: point at one to see what it offers");
        } else if (gm == GM_MAP) {
            orn_septext("Now: the overland view");
            row("Click", "walk towards that spot");
            row("Ctrl+click", "travel there along the roads (any key or click stops)");
            row("M", "world map: find a place, double-click to travel there");
            row("Point at a quest's picture", "the quest that leads there (a portcullis or a pillory beside a town: the town bars the party, or its watch wants the party)");
            row("A quest in the left margin", "click to open it, then Travel there");
            row("Town pictures", "(in the right margin, near a town) what that town has that most towns lack, and its notable shops (point at one to see what it offers)");
        } else if (gm == GM_BATTLE) {
            orn_septext("Now: a battle");
            row("Space", "pause / resume; orders can be given while paused");
            if (!ui_original_mode() && config_get_int("battle.quick_orders", 1)) {
                row("Click a member", "then click an enemy (attack) or a spot (walk there): that click gives the order");
                row("1 to 5", "select a member; Esc: cancel the order being given");
                if (config_get_int("battle.panel_select", 1)) row("A member's box at the left", "click: select them (double-click: and move the view to them); right-click: their sheet");
                if (config_get_int("battle.volley_toggle", 1)) row("Bow or sword on a member's box", "that member shoots or fights hand to hand when you click an enemy (click it to switch)");
                row("The right margin's buttons", "the other orders (Vulnerable, Berserk, Parry, Use Missile, Halt ...); for a member already fighting, an attack is given against that enemy at once");
            } else
                row("Click a member", "then click an enemy, a spot or a menu choice; Enter gives the order");
            row("Ctrl+click a member", "make that member the active one: the view follows them (scrolling at an edge stops it)");
            row("Point at anyone", "who it is and how badly hurt");
            if (levelmap_available()) row("Tab", "a map of the level as far as the party has seen it");
            if (!ui_original_mode() && config_get_int("battle.fast_key", 1)) row("Z", "fast forward (six times the pace) / back to the chosen speed");
            { extern int ui_battle_log; if (ui_battle_log && !ui_original_mode()) row("The battle log", "every blow and wound: the left margin on a wide window, else the bottom right; F12 > Journal keeps it all"); }
        } else if (loot_active && lootview_showing()) {
            orn_septext("Now: the loot");
            row("Take / Put back", "on the row under the pointer: the whole stack (1: just one)");
            if (loot_model.shared) { row("Take all", "everything into the party's pack"); row("Potions", "go to the member shown above the right list (each member's own); the rest into the party's pack"); }
            else { row("Take all / Share out", "everything to the member shown / to whoever has some already, else whoever has the most room (A / S)");
                   row("Names above the right list, D", "switch to another member"); }
            row("L, Esc or Leave", "leave (what is left stays behind)");
        } else if (loot_active) {
            orn_septext("Now: the loot");
            row("G / P", "get the chosen item from the pile / put one back (the cursor's list)");
            row("D", "hand out to the next member");
            if (loot_enabled()) { row("A", "take the whole pile for the member shown"); row("S", "share the pile out: each item to whoever has some already, else whoever has the most room"); }
            row("L or Esc", "leave (what is left stays behind)");
        } else if (barter_active && barterview_showing()) {
            orn_septext("Now: a shop");
            row("Buy / Sell", "on the row under the pointer; with Shift: ten / the whole stack");
            row("Names above your list", "whose items are listed (B: the next member); right-click a name: that member holds the purse");
            row("Wheel, Up, Down", "scroll the list under the pointer");
            row("L, Esc or Leave", "leave");
        } else if (barter_active) {
            orn_septext("Now: a shop");
            row("P / S", "buy the merchant's chosen item / sell your chosen item (one at a time)");
            if (loot_enabled()) { row("T", "buy ten of the merchant's chosen item (while the money lasts)"); row("A", "sell the whole stack of your chosen item"); }
            row("B", "barter for the next member");
            row("L or Esc", "leave");
        } else if (cacheview_showing()) {
            orn_septext("Now: the store room");
            row("Take / Put", "on the row under the pointer: the whole stack (1: just one)");
            row("Put away", "every item not equipped (with the party's pack, Darklands+: except kept items and potions)");
            if (cacheview_shared_pack()) row("Potions", "a potion taken goes to the member shown above the right list; the rest into the party's pack");
            if (cacheview_every_inn()) row("The store room", "the same at every inn (F12 > Settings)");
            row("L, Esc or Leave", "leave");
        } else if (sheetview_showing()) {
            orn_septext("Now: a character sheet");
            if (sheet_model.shared) { row("Ready / Unequip / Drink / Keep / Drop", "on the row under the pointer on the Items page: with the shared pack (Darklands+) the spare items are the whole party's, and Ready first hands the item to this member (Drop with Shift: all of them)");
                                      row("Potions", "each member's own: drag one onto someone at the left to give it");
                                      row("Keep", "marks an item never to be sold or dropped (a small seal on its row)");
                                      row("The load", "only equipped items count toward the load, as in the original"); }
            else row("Ready / Unequip / Drink / Give / Drop", "on the row under the pointer on the Items page (Give and Drop with Shift: the whole stack)");
            row("Items, Skills, Saints, Formulae", "the pages at the top (Tab or Right: next page; Shift+Tab or Left: previous page)");
            row("Portraits, F1 ... F5", "another member's sheet; this member's own portrait, Esc or Close: back");
            row("Ctrl+F1 ... Ctrl+F5", "make member 1 ... 5 the leader (or Make leader)");
            row("Wheel, Up, Down", "scroll the list");
        } else if (hdart_on_screen("ARMBACK")) {        /* the character sheet (s031's keys) */
            orn_septext("Now: a character sheet");
            row("Ctrl+F1 ... Ctrl+F5", "make member 1 ... 5 the leader (or Make leader at the top)");
            row("Up, Down, PgUp, PgDn", "move in a list (the mouse wheel too)");
            row("1 ... 5", "in the equipment list: give the chosen item to member 1 ... 5 (with Shift: all of them)");
            row("A", "in the equipment list: ready or wear the chosen item");
            row("D", "in the equipment list: drop the chosen item (with Shift: all of them)");
        }
        orn_septext("Keys of the original game");
        if (!ui_original_mode()) row("Right-click", "the menu bar: click an entry (or hold the button and let go on it); a click elsewhere or any key closes it; F10 with the keyboard");
        else row("Right mouse button", "the menu bar, while it is held: let go on an entry (F10 with the keyboard)");
        row("Alt+S / Alt+L", "save / load (with a comment, nine saves)");
        row("Alt+O", "order of the party");
        row("Alt+D", "difficulty    Alt+M music    Alt+F sound effects");
        row("Alt+C", "messages about saints' changes on / off");
        row("F6", "the party's information");
        row("Alt+Q", "quit to the desktop");
        orn_septext("Darklands+");
        row("F12", "the Darklands+ menu (pauses the game)");
        row("Ctrl+S / F9 or Ctrl+L", "quick save / quick load");
        row("Alt+Enter", "full screen    Ctrl+F12 screenshot");
        { if ((plat_controller_connected() || ui_pad_prompts()) ) {
            /* the buttons drawn as the controller in use has them (padglyphs.h), else named */
            orn_septext("The controller");
            auto prow = [](const char *keys, const char *d) {     /* keys: "{A} {B}" */
                float x0 = ImGui::GetCursorPosX();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.18f, 0.10f, 1)); ui_pad_text(keys); ImGui::PopStyleColor();
                (void)x0; ImGui::SameLine(UIS(230)); ImGui::TextWrapped("%s", d);
            };
            prow("{LSTICK}", "moves the pointer (slowly while the left trigger is held)");
            prow("{RSTICK}", "the mouse wheel; pressed: Z (fast forward in a battle)");
            prow("{A} {B}", "click / right-click (the menu bar)");
            prow("{X} {Y}", "Enter / Esc");
            prow("{DPAD}", "the arrow keys");
            prow("{LB} {RB}", "Tab (the level map) / Space (pause)");
            prow("{START} {BACK}", "F12 (this menu) / M (the world map)");
            prow("{RT}", "held: Ctrl (in the overland view: Ctrl+click travels there)");
            prow("{LS}", "pressed: F11 (this list)");
          } }
        ImGui::TextDisabled("F11, Esc or a click closes this.");
    }
    ImGui::End();
    ImGui::PopFont();
}

