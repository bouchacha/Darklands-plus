// battlelog.cpp - what happens in a battle, line by line. The game shows a battle only as
// figures swinging and bars that shrink; here each blow is told as it lands (a miss, a parry,
// a hit and what it cost: endurance, and strength for a real wound), who falls unconscious,
// who is killed, and harm from other causes (missiles, potions). On a wide screen the lines
// fill the left margin; otherwise the last few show over the bottom of the battlefield and
// fade. F12 > Journal > Battle log keeps the whole of the last battle.
//
// Blows come from the melee step (mods/sfx.c hooks s014 0e76:199c: the landing frame, with
// the target's endurance and strength before and after); the rest from a look at the
// combatant records every frame (DS:9c55 + 0x80 i: +0x14 status 1 up, 2 unconscious, 3 dead,
// +0x5d endurance, +0x5e strength, +0x4a target).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hooks.h"
u32 plat_ticks_ms(void);
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
extern ImFont *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
std::string ui_enemy_name(int floor, u16 b);

int ui_battle_log = 1;                      /* 0 off, 1 on */

struct BLine { u32 t; std::string text; int kind; };   /* kind: 0 the party acts, 1 an enemy acts, 2 harm to the party, 3 a fall, 4 notes */
static std::vector<BLine> lines, prev_lines;
static int known_end[13], known_str[13], known_st[13], last_st[13];   /* last_st: the last status that was not 0 (the game clears the fallen's records) */
static bool in_battle;
static int names_gen;
static std::string names[13];

#define CB(i) ((u16)(0x9c55 + 0x80 * (i)))

static std::string name_of(int i)
{
    if (i < 0 || i >= 13) return "?";
    if (!names[i].empty()) return names[i];
    std::string n;
    if (i < 5) { char b[16]; g_str(G_DGROUP, (u16)(G_MEMBER(i) + G_M_SHORTNAME), b, sizeof b); n = game_text(b, sizeof b); }
    else n = ui_enemy_name(g_rb((u16)(GV_cbt_floor + i)), CB(i));
    if (n.empty()) n = i < 5 ? "?" : "Enemy";
    if (i >= 5) {                                   /* "Bandit 2" when there are several */
        int same = 0, nth = 0;
        for (int k = 5; k < 13; k++) {
            if (!g_rb((u16)(CB(k) + 0x14)) || (s8)g_rb((u16)(CB(k) + 0x64)) <= 0) continue;
            std::string o = k == i ? n : ui_enemy_name(g_rb((u16)(GV_cbt_floor + k)), CB(k));
            if (o == n) { same++; if (k <= i) nth++; }
        }
        if (same > 1) n += " " + std::to_string(nth);
    }
    names[i] = n;
    return n;
}

std::string battlelog_name(int i) { return name_of(i); }     /* "Villager 2": as the log calls them */

static void add(int kind, const std::string &t)
{
    lines.push_back({ plat_ticks_ms(), t, kind });
    if (lines.size() > 400) lines.erase(lines.begin(), lines.begin() + 100);
    static int lg = -1; if (lg < 0) lg = getenv("DL_BATTLE_LOG") != 0;
    if (lg) fprintf(stderr, "battle: %s\n", t.c_str());
}

static void start_battle(void)
{
    if (!lines.empty()) { prev_lines = lines; }       /* the battle before, for the journal */
    lines.clear();
    for (int i = 0; i < 13; i++) {
        names[i].clear();
        known_end[i] = (s8)g_rb((u16)(CB(i) + 0x5d)); known_str[i] = (s8)g_rb((u16)(CB(i) + 0x5e)); known_st[i] = g_rb((u16)(CB(i) + 0x14));
        last_st[i] = known_st[i];
    }
    for (int i = 0; i < 13; i++) if (known_st[i]) name_of(i);    /* names (and their numbers) fixed while all are standing */
    in_battle = true;
}

static std::string cost(int de, int ds)
{
    std::string s;
    if (de > 0) s = "-" + std::to_string(de) + " End";
    if (ds > 0) s += (s.empty() ? "-" : ", -") + std::to_string(ds) + " Str";
    return s;
}

/* a blow landing (sfx.c's melee hook): i strikes t; anim: 1, 2 a hit, 6 a miss, 7 parried, other blocked */
extern "C" void battlelog_blow(int i, int t, int anim, int e0, int e1, int s0, int s1)
{
    if (!ui_battle_log || game_mode() != GM_BATTLE || i < 0 || i >= 13 || t < 0 || t >= 13 || i == t) return;
    if (!in_battle) start_battle();
    std::string a = name_of(i), d = name_of(t);
    int kind = i < 5 ? 0 : 1;
    if (anim == 1 || anim == 2) {
        int de = e0 - e1, ds = s0 - s1;
        bool back = g_rb((u16)(GV_cbt_anim + t)) == 3;       /* a heavy blow pushes the target back a step */
        if (de <= 0 && ds <= 0) add(kind, a + " hits " + d + ", the armour holds" + (back ? "; knocked back" : ""));
        else add(t < 5 ? 2 : kind, a + (ds > 0 ? " wounds " : " hits ") + d + " (" + cost(de, ds) + ")" + (back ? ", knocked back" : ""));
    }
    else if (anim == 6) add(kind, a + " misses " + d);
    else if (anim == 7) add(kind, d + " parries " + a);
    else if (anim == 14) add(kind, a + ", knocked back, misses " + d);
    else add(kind, a + " misses " + d);
    known_end[t] = e1; known_str[t] = s1;
}

/* who aims a missile weapon (bow, crossbow, handgun, sling, thrown) at combatant t, if anyone */
static int shooter_at(int t)
{
    static int missile[64]; static bool built;
    if (!built && gamedata.loaded) {
        built = true;
        for (const ItemDef &d : gamedata.items) if (d.kind == K_MISSILE && d.type >= 0 && d.type < 64) missile[d.type] = 1;
    }
    for (int j = 0; j < 13; j++) {
        u16 b = CB(j);
        if (j == t || g_rb((u16)(b + 0x14)) != 1 || (s8)g_rb((u16)(b + 0x4a)) != t) continue;
        int w = (s8)g_rb((u16)(b + 0x22));
        if (w >= 0 && w < 64 && missile[w]) return j;
    }
    return -1;
}

/* missiles: whether one hits is decided when it is loosed (missile_launch, s018 1ef8:0000):
 * a missile that will strike flies to its target (record +0x11 = 0), one that misses flies
 * on until something stops it (+0x11 = 1). The hit itself shows when the target's endurance
 * drops (battlelog_frame); one whose armour held shows when it should have landed. */
static int shot_by[13] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static u32 shot_t[13];
static std::string shot_word(int who)
{
    int w = (s8)g_rb((u16)(CB(who) + 0x22));
    return w >= 0x17 && w <= 0x1a ? "'s throw" : "'s shot";      /* throwing axe, javelin, dart, throwing knife */
}
static void launch_hook(void)               /* s018 1ef8:0000 missile_launch(who, tgt, type, d, e) */
{
    int who = (s16)rw(SS, (u16)(SP + 4)), tgt = (s16)rw(SS, (u16)(SP + 6)), type = (s16)rw(SS, (u16)(SP + 8));
    hook_call_original();
    if (unw_active || !ui_battle_log || type != 0 || who < 0 || who >= 13 || tgt < 0 || tgt >= 13 || who == tgt) return;
    int n = (s8)g_rb(GV_missile_count) - 1;
    if (n < 0 || n >= 17) return;
    u16 r = (u16)(0x944d + 0x1a * n);
    if (!in_battle) start_battle();
    if (g_rb((u16)(r + 0x11)) == 1) add(who < 5 ? 0 : 1, name_of(who) + shot_word(who) + " misses " + name_of(tgt));
    else { shot_by[tgt] = who; shot_t[tgt] = plat_ticks_ms(); }
}
extern "C" void battlelog_init(void) { hook_install(GF_missile_launch, launch_hook); }

/* every frame: falls, deaths and harm that came from no blow */
extern "C" void battlelog_frame(void)
{
    if (game_mode() != GM_BATTLE) {
        if (in_battle) {
            in_battle = false;
            if (!lines.empty()) {                     /* how it ended, from the last look at everyone */
                int up_party = 0, up_foes = 0, foes = 0;
                for (int i = 0; i < 13; i++) {
                    if (i < 5) { if (last_st[i] == 1) up_party++; }
                    else if (last_st[i]) { foes++; if (last_st[i] == 1) up_foes++; }
                }
                add(4, !up_party ? "The battle is over: the party has fallen." : foes && !up_foes ? "The battle is over: the enemies are beaten." : "The battle is over.");
            }
        }
        return;
    }
    if (!in_battle) start_battle();               /* (the names are the margin's too, log or not) */
    if (!ui_battle_log) return;
    for (int i = 0; i < 13; i++) {
        u16 b = CB(i);
        int e = (s8)g_rb((u16)(b + 0x5d)), s = (s8)g_rb((u16)(b + 0x5e)), st = g_rb((u16)(b + 0x14));
        if (known_st[i] == 0 && st) { known_end[i] = e; known_str[i] = s; known_st[i] = st; names[i].clear(); continue; }   /* joins the battle */
        if ((e < known_end[i] || s < known_str[i]) && st) {
            int de = known_end[i] - e, ds = known_str[i] - s;
            if (de < 0) de = 0;
            if (ds < 0) ds = 0;
            if (de || ds) {
                int by = shot_by[i] >= 0 ? shot_by[i] : shooter_at(i);
                shot_by[i] = -1;
                if (by >= 0) add(i < 5 ? 2 : by < 5 ? 0 : 1, name_of(by) + shot_word(by) + (ds > 0 ? " wounds " : " hits ") + name_of(i) + " (" + cost(de, ds) + ")");
                else add(i < 5 ? 2 : 4, name_of(i) + " is hurt (" + cost(de, ds) + ")");
            }
        }
        if (st != known_st[i]) {
            if (st == 3) add(i < 5 ? 3 : 4, name_of(i) + (i < 5 ? " is killed!" : " is killed"));
            else if (st == 2) add(i < 5 ? 3 : 4, name_of(i) + " falls unconscious" + (i < 5 ? "!" : ""));
            else if (st == 1 && known_st[i] == 2) add(4, name_of(i) + " comes to");
            else if (st == 5) add(i < 5 ? 3 : 4, name_of(i) + " leaves the field");
        }
        if (shot_by[i] >= 0 && plat_ticks_ms() - shot_t[i] > 2500) {      /* it struck, and nothing came through */
            int by = shot_by[i]; shot_by[i] = -1;
            if (st == 1 && by < 13) add(by < 5 ? 0 : 1, name_of(by) + shot_word(by) + " hits " + name_of(i) + ", the armour holds");
        }
        known_end[i] = e; known_str[i] = s; known_st[i] = st;
        if (st) last_st[i] = st;
    }
}

static ImU32 colour(int kind, float a)
{
    static const ImU32 C[5] = { IM_COL32(170, 205, 255, 255), IM_COL32(255, 196, 150, 255), IM_COL32(255, 140, 110, 255), IM_COL32(255, 95, 80, 255), IM_COL32(215, 205, 185, 255) };
    ImU32 c = C[kind < 0 || kind > 4 ? 4 : kind];
    return (c & 0x00ffffff) | ((ImU32)(a * 255) << 24);
}

/* the left margin in a battle: the log, newest at the bottom */
extern "C" int battlelog_margin(float x, float w, float h, float fs)
{
    if (!ui_battle_log || game_mode() != GM_BATTLE || config_get_int("original", 0)) return 0;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImFont *f = ui_font_overlay;
    float pad = fs * 0.7f, lh = fs * 1.25f, y = h - pad;
    dl->AddText(f, fs * 1.05f, ImVec2(x + pad, pad), IM_COL32(222, 178, 104, 255), "Battle");
    float top = pad + lh * 1.5f;
    for (int k = (int)lines.size() - 1; k >= 0 && y > top + lh; k--) {
        /* wrapped to the margin's width, so a long line takes two rows */
        const char *t = lines[k].text.c_str();
        ImVec2 sz = f->CalcTextSizeA(fs, FLT_MAX, w - 2 * pad, t);
        float a = k == (int)lines.size() - 1 && plat_ticks_ms() - lines[k].t < 400 ? 1.0f : 0.92f;
        y -= sz.y + fs * 0.3f;
        if (y < top) break;
        dl->AddText(f, fs, ImVec2(x + pad, y), colour(lines[k].kind, a), t, 0, w - 2 * pad);
    }
    return 1;
}

/* no margin: the last lines over the bottom of the battlefield, fading */
extern "C" void battlelog_overlay(void)
{
    if (!ui_battle_log || game_mode() != GM_BATTLE || lines.empty() || config_get_int("original", 0)) return;
    float x0, y0, x1, y1; plat_game_to_win(60, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    { extern int ui_margins_on; float l, d; plat_game_to_win(0, 0, &l, &d); if (ui_margins_on && l >= 150) return; }   /* the left margin shows them */
    ImFont *f = ui_font_overlay;
    float fs = (y1 - y0) / 200.0f * 6.2f; if (fs < 14) fs = 14;
    u32 now = plat_ticks_ms();
    const u32 SHOW = 7000, FADE = 1500;
    int n = 0;
    for (int k = (int)lines.size() - 1; k >= 0 && n < 4; k--, n++) if (now - lines[k].t > SHOW + FADE) break;
    if (!n) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float lh = fs * 1.2f, pad = fs * 0.4f;
    float y = y1 - pad - lh * n;
    for (int k = (int)lines.size() - n; k < (int)lines.size(); k++, y += lh) {
        u32 age = now - lines[k].t;
        float a = age < SHOW ? 1.0f : 1.0f - (float)(age - SHOW) / FADE;
        if (a <= 0) continue;
        const char *t = lines[k].text.c_str();
        ImVec2 sz = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        ImVec2 p(x1 - pad - sz.x, y);
        dl->AddRectFilled(ImVec2(p.x - pad, y - fs * 0.08f), ImVec2(x1 - pad * 0.5f, y + lh - fs * 0.12f), IM_COL32(16, 10, 6, (int)(150 * a)), fs * 0.25f);
        dl->AddText(f, fs, p, colour(lines[k].kind, a), t);
    }
}

/* F12 > Journal: the whole of the last battle */
extern "C" void battlelog_tab(void)
{
    if (lines.empty() && prev_lines.empty()) { ImGui::TextWrapped("Nothing yet: the log of a battle shows here, blow by blow, while it lasts and after."); return; }
    static int which = 0;
    if (!prev_lines.empty()) {
        ImGui::RadioButton("The last battle", &which, 0); ImGui::SameLine();
        ImGui::RadioButton("The one before", &which, 1);
    } else which = 0;
    const std::vector<BLine> &v = which ? prev_lines : lines;
    ImGui::TextDisabled("%d lines.", (int)v.size());
    ImGui::BeginChild("blog", ImVec2(0, 0));
    static const ImVec4 C[5] = { ImVec4(0.12f, 0.24f, 0.50f, 1), ImVec4(0.45f, 0.25f, 0.08f, 1), ImVec4(0.62f, 0.18f, 0.10f, 1), ImVec4(0.75f, 0.08f, 0.05f, 1), ImVec4(0.30f, 0.26f, 0.20f, 1) };
    for (const BLine &l : v) ImGui::TextColored(C[l.kind < 0 || l.kind > 4 ? 4 : l.kind], "%s", l.text.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}
