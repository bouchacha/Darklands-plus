// tips.cpp - the first words of advice, each in a window the game waits behind (ui_is_open): at the
// first battle of a session with Improved combat, how the party fights now (the start screen's note
// pointing to the New Player Guide went with the guide; the welcome, the very first time, stays). Each shows at every
// start until "Never show again" is ticked, which a player can only do from the fourth start
// (ui.launches counts them; the box says how many more times it comes first), so a new player sees them a few times. The MT-32 window before the
// intro (ui/ui_windows.cpp) takes the same box. Not in the test sessions (DL_CTL, DL_SCRIPT)
// unless DL_TIPS; the test command utip:N opens one.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "imgui.h"
#include "padglyphs.h"
#include "uiinternal.h"
#include "uitabs.h"
#include "ornament.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/battlerules.h"
#include "../mods/nscreen.h"
#include "../mods/datamods.h"
#include "ui.h"
int levelmap_available(void);
u32 plat_ticks_ms(void);
void ui_open_tab(int t);
}
extern ImFont *font_body, *font_head;
extern bool open_welcome;

enum { TIP_NONE, TIP_GUIDE, TIP_BATTLE, TIP_ENCOUNTERS, TIP_VERSION };
static int open_tip, launches, battle_seen, title_seen, encounters_due, version_due;

/* a word to those who played an earlier version on what this one asks them to watch, once (ui.version_note keeps
   the version it was shown for). Each release sets its own, or "" for none (his ask, 1.5.1: stairs in a fight) */
static const char VERSION_NOTE_FOR[] = "1.5.1";
static const char VERSION_NOTE[] =
    "Stairs and ladders now work differently in a fight: instead of one member going through at a time, the whole "
    "party comes along, wherever they stand on the floor, the knocked-out carried. A party split across floors caused "
    "many bugs in the original game (16 known ones), and keeping it together was the cleanest fix.\n\nIf anything goes wrong on stairs or after a change of floor (someone left behind, "
    "walking through a wall, a fight that will not end), please make a report: F12 > About > Report a problem, and say what you were doing.";
static u32 title_at;            /* the start screen came: its note a moment later, once it is drawn (the game waits while a window is open) */

static bool quiet(void) { return (getenv("DL_CTL") || getenv("DL_SCRIPT") || getenv("DL_RESTARTED")) && !getenv("DL_TIPS"); }

/* at each start (not a test's, nor the restart after the party's end): one more */
extern "C" void tips_init(void)
{
    launches = config_get_int("ui.launches", 0);
    if (quiet()) return;
    config_set_int("ui.launches", ++launches); config_save();
}

extern "C" int tips_open(void) { return open_tip != TIP_NONE; }

/* "Never show again", for a window shown at every start: greyed until the fourth start, saying how
   many more times the window comes first */
bool tips_never_box(bool *never)
{
    bool can = launches > 3;
    if (!can) *never = false;
    ImGui::BeginDisabled(!can);
    int more = 4 - (launches > 0 ? launches : 1);
    char l[96]; snprintf(l, sizeof l, "Never show again (this will nag you %d more time%s)", more, more == 1 ? "" : "s");
    ImGui::Checkbox(can ? "Never show again" : l, never);
    ImGui::EndDisabled();
    return *never;
}

/* the start screen (mods/titleview.c, the native one or the original's): once a session */
extern "C" void tips_title_reached(void)
{
    if (title_seen) return;
    title_seen = 1;
    if (quiet()) return;
    title_at = plat_ticks_ms() | 1;
}
static void title_check(void)
{
    if (!title_at || plat_ticks_ms() - title_at < 700) return;
    title_at = 0;
    fprintf(stderr, "tips: the start screen's note\n");
    encounters_due = !config_get_int("ui.encounters_note_seen", 0);   /* (once, after the welcome if it comes: encounters_tip) */
    if (!config_get_int("ui.welcome_seen", 0)) {                  /* (the first start: the welcome, which says it too) */
        config_set("ui.version_note", VERSION_NOTE_FOR);      /* (nothing changed for a first player) */
        open_welcome = true; return;
    }
    version_due = VERSION_NOTE[0] && strcmp(config_get("ui.version_note", ""), VERSION_NOTE_FOR);
    /* (the note pointing to the New Player Guide: withdrawn with the guide, ui/ui.cpp tab_guide_all) */
}

extern "C" void tips_show(int which) { if (which == 3) open_welcome = true; else if (which == 2) open_tip = which; }   /* (utip:N: 2 the battle tips, 3 the welcome) */

static bool foes_up(void)
{
    for (int i = 5; i < 13; i++) if (g_rb((u16)(GV_combatants + i * 0x80 + 0x14)) == 1) return true;
    return false;
}

/* every frame: the first battle of the session with enemies in it, after a moment (it is drawn) */
static void battle_check(void)
{
    static u32 since;
    if (battle_seen || open_tip || quiet() || !combat_improved() || !config_get_int("tips.battle", 1)) return;
    if (game_mode() != GM_BATTLE || ns_showing() || ui_is_open() || !foes_up()) { since = 0; return; }
    u32 now = plat_ticks_ms();
    if (!since) since = now ? now : 1;
    if (now - since < 800) return;
    battle_seen = 1; open_tip = TIP_BATTLE;
    fprintf(stderr, "tips: the battle tips\n");
}

static void bullet(const char *t) { ImGui::Bullet(); ImGui::TextWrapped("%s", t); }   /* (BulletText does not wrap) */

static bool begin_tip(const char *title, float width)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 120));
    ImGui::SetNextWindowSize(ImVec2(fminf(io.DisplaySize.x * 0.92f, UIS(width)), 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowFocus();
    ImGui::PushFont(font_body);
    return ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
}
static void end_tip(void) { ImGui::End(); ImGui::PopFont(); }

static void battle_tip(void)
{
    static bool never;
    bool done = false;
    if (begin_tip("Battles, with Improved combat", 660)) {
        ImGui::PushFont(font_head); orn_septext("Your people fight on between orders"); ImGui::PopFont();
        bullet("A member with a bow, crossbow, gun or thrown weapon keeps shooting; when the target falls, at the nearest enemy in sight.");
        bullet("With no clear line (a wall, a tree or a friend in the way), a shooter steps a few paces aside to find one.");
        bullet("Shooters draw a melee weapon once an enemy is close, when nothing is left to shoot, or to help a friend nearby who is set upon or badly hurt.");
        bullet("A member whose enemy falls turns on another close by; one with no orders joins a fight beside it, unless told to hold.");
        ImGui::PushFont(font_head); orn_septext("Your orders"); ImGui::PopFont();
        if (ui_pad_prompts()) ui_pad_text("F11 on a keyboard lists the keys.", true);
        bullet("Space pauses and resumes. Orders can be given while paused.");
        if (!ui_original_mode() && config_get_int("battle.quick_orders", 1)) bullet("Click a member, then an enemy (attack) or a spot (walk there).");
        else bullet("Click a member, then an enemy, a spot or a menu choice; Enter gives the order.");
        bullet("A click on an enemy lets your member choose the attack: a Vulnerable one, aimed at the gaps, when only that gets through the enemy's armor; else a standard one. An attack you pick from the orders yourself (Std Attack, Vulnerable, Berserk, Parry) is kept.");
        std::string keys = "F11 lists every key";
        if (levelmap_available()) keys += "; Tab, the level map";
        if (config_get_int("battle.fast_key", 1)) keys += "; Z, fast forward";
        keys += "."; bullet(keys.c_str());
        ImGui::PushFont(font_head); orn_septext("The attacks"); ImGui::PopFont();
        bullet("Std Attack: blows and guard in balance.");
        bullet("Vulnerable: slow blows aimed at the gaps in the armor; they pierce more (1 to 4 more penetration), but come far less often and land a little less. For armor the weapon cannot get through.");
        bullet("Berserk: quick, sure blows, without thought of defense. To finish a foe fast.");
        bullet("Parry: mostly defends; hard to hit, striking back slower and less surely. To hold a foe off while help comes, or to spare a wounded member.");
        ImGui::Spacing();
        ui_hint("Improved combat can be switched off in F12 > Settings > Battles.");
        ImGui::Spacing();
        if (ImGui::Button("  To battle  ") || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter)) done = true;
        tips_never_box(&never);
    }
    end_tip();
    if (done) { open_tip = TIP_NONE; if (never) config_set_int("tips.battle", 0); config_save(); }
}

/* New encounters, the mod that comes with Darklands+, ticked until unticked (mods/datamods.c): said once, at the
   start screen after the welcome, with the choice to turn it off (his ask, 1.5.1). The complaint: Dragon magazine's
   review found "the repetitiveness of some encounters", and the CRPG Addict's (2019) that they become repetitive */
static void encounters_tip(void)
{
    int i = datamods_find(DATAMOD_ENCOUNTERS);
    bool done = false, off = false;
    if (i >= 0 && begin_tip("New encounters", 600)) {
        ImGui::TextWrapped("A frequent complaint about Darklands, in reviews and among players, is that its random encounters soon repeat.");
        ImGui::Spacing();
        ImGui::TextWrapped("Darklands+ comes with a mod of 160 new ones in the spirit of the original: 90 in the wild, 18 on the river boats, "
                           "34 in the towns' streets and 18 in camp. They come up among the game's own; a small seal in the corner of the card marks a new one.");
        ImGui::Spacing();
        if (ImGui::Button(datamods[i].on ? "  Play with them  " : "  Turn them on  ") || ImGui::IsKeyPressed(ImGuiKey_Enter)) done = true;
        ImGui::SameLine();
        if (ImGui::Button(datamods[i].on ? "  Turn them off  " : "  Leave them off  ")) done = off = true;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { done = true; off = !datamods[i].on; }   /* (as it stands) */
        ui_hint("Change it at any time: F12 > Mods.");
    }
    if (i >= 0) end_tip();
    if (i < 0 || done) {
        if (i >= 0 && datamods[i].on == off) { datamods_set(i, !off); fprintf(stderr, "tips: New encounters %s\n", off ? "off" : "on"); }
        open_tip = TIP_NONE; config_set_int("ui.encounters_note_seen", 1); config_save();
    }
}

static void version_tip(void)
{
    bool done = false;
    char title[48]; snprintf(title, sizeof title, "In Darklands+ %s", VERSION_NOTE_FOR);
    if (begin_tip(title, 560)) {
        ImGui::TextWrapped("%s", VERSION_NOTE);
        ImGui::Spacing();
        if (ImGui::Button("  Understood  ") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape)) done = true;
    }
    end_tip();
    if (done) { open_tip = TIP_NONE; config_set("ui.version_note", VERSION_NOTE_FOR); config_save(); }
}

/* every frame, from ui/ui.cpp */
void tips_frame(void)
{
    title_check();
    battle_check();
    if (encounters_due && !open_welcome && open_tip == TIP_NONE && !quiet()) {
        encounters_due = 0;
        if (datamods_enabled() && datamods_find(DATAMOD_ENCOUNTERS) >= 0) { open_tip = TIP_ENCOUNTERS; fprintf(stderr, "tips: the New encounters note\n"); }
    }
    if (version_due && !encounters_due && !open_welcome && open_tip == TIP_NONE && !quiet()) {
        version_due = 0; open_tip = TIP_VERSION; fprintf(stderr, "tips: the note on version %s\n", VERSION_NOTE_FOR);
    }
    if (open_tip == TIP_BATTLE) battle_tip();
    else if (open_tip == TIP_ENCOUNTERS) encounters_tip();
    else if (open_tip == TIP_VERSION) version_tip();
}
