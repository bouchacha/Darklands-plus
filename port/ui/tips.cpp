// tips.cpp - the first words of advice, each in a window the game waits behind (ui_is_open): on the
// start screen, the way to the New Player Guide (the welcome instead, the very first time); at the
// first battle of a session with Improved combat, how the party fights now. Each shows at every
// start until "Never show again" is ticked, which a player can only do from the fourth start
// (ui.launches counts them), so a new player sees them a few times. The MT-32 window before the
// intro (ui/ui_windows.cpp) takes the same box. Not in the test sessions (DL_CTL, DL_SCRIPT)
// unless DL_TIPS; the test command utip:N opens one.
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
#include "ui.h"
int levelmap_available(void);
u32 plat_ticks_ms(void);
void ui_open_tab(int t);
}
extern ImFont *font_body, *font_head;
extern bool open_welcome;

enum { TIP_NONE, TIP_GUIDE, TIP_BATTLE };
static int open_tip, launches, battle_seen, title_seen;
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

/* "Never show again", for a window shown at every start: greyed until the fourth start */
bool tips_never_box(bool *never)
{
    bool can = launches > 3;
    if (!can) *never = false;
    ImGui::BeginDisabled(!can);
    ImGui::Checkbox(can ? "Never show again" : "Never show again (can be ticked from the fourth start)", never);
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
    if (!config_get_int("ui.welcome_seen", 0)) { open_welcome = true; return; }   /* (the first start: the welcome, which says it too) */
    if (config_get_int("tips.guide", 1)) open_tip = TIP_GUIDE;
}

extern "C" void tips_show(int which) { if (which == 3) open_welcome = true; else if (which == 1 || which == 2) open_tip = which; }   /* (utip:N) */

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

static void guide_tip(void)
{
    static bool never;
    bool done = false, guide = false;
    if (begin_tip("New to Darklands?", 620)) {
        ImGui::TextWrapped("Darklands came with a thick manual and explains little by itself. The New Player Guide sums up how it works: "
                           "the first steps, the party, money, towns, healing, alchemy, saints, battle, travel and saving, with the advice players have long given one another.");
        ImGui::Spacing();
        if (ui_pad_prompts()) ui_pad_text("It is in the Darklands+ window: {START} > New Player Guide.", true);
        else ui_hint("It is in the Darklands+ window at any time: F12 > New Player Guide. F11 lists the keys of the screen in view.");
        ImGui::Spacing();
        if (ImGui::Button("  Read the New Player Guide  ")) guide = done = true;
        ImGui::SameLine();
        if (ImGui::Button("  Not now  ") || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter)) done = true;
        tips_never_box(&never);
    }
    end_tip();
    if (done) {
        open_tip = TIP_NONE;
        if (never) config_set_int("tips.guide", 0);
        config_save();
        if (guide) ui_open_tab(UI_PAGE_GUIDE);
    }
}

static void battle_tip(void)
{
    static bool never;
    bool done = false;
    if (begin_tip("Battles, with Improved combat", 660)) {
        ImGui::PushFont(font_head); orn_septext("Your people fight on between orders"); ImGui::PopFont();
        bullet("A member with a bow, crossbow, gun or thrown weapon keeps shooting; when the target falls, at the nearest enemy in sight.");
        bullet("With no clear line (a wall, a tree or a friend in the way), a shooter steps a few paces aside to find one.");
        bullet("Shooters draw a melee weapon once an enemy is close, or to help a friend nearby who is set upon or badly hurt.");
        bullet("A member whose enemy falls turns on another close by. Parry, Berserk and Vulnerable stay as you ordered.");
        ImGui::PushFont(font_head); orn_septext("Your orders"); ImGui::PopFont();
        if (ui_pad_prompts()) ui_pad_text("{START} > New Player Guide > Battle explains the rest; F11 on a keyboard lists the keys.", true);
        bullet("Space pauses and resumes. Orders can be given while paused.");
        if (!ui_original_mode() && config_get_int("battle.quick_orders", 1)) bullet("Click a member, then an enemy (attack) or a spot (walk there).");
        else bullet("Click a member, then an enemy, a spot or a menu choice; Enter gives the order.");
        bullet("An attack on an enemy whose armor the weapon cannot pierce is aimed at the gaps in it (a Vulnerable attack).");
        std::string keys = "F11 lists every key";
        if (levelmap_available()) keys += "; Tab, the level map";
        if (config_get_int("battle.fast_key", 1)) keys += "; Z, fast forward";
        keys += "."; bullet(keys.c_str());
        ImGui::Spacing();
        ui_hint("Improved combat can be switched off in F12 > Settings > Battles.");
        ImGui::Spacing();
        if (ImGui::Button("  To battle  ") || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter)) done = true;
        tips_never_box(&never);
    }
    end_tip();
    if (done) { open_tip = TIP_NONE; if (never) config_set_int("tips.battle", 0); config_save(); }
}

/* every frame, from ui/ui.cpp */
void tips_frame(void)
{
    title_check();
    battle_check();
    if (open_tip == TIP_GUIDE) guide_tip();
    else if (open_tip == TIP_BATTLE) battle_tip();
}
