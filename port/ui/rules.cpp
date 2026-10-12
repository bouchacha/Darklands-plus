// rules.cpp - game rules as data. The weapon numbers live in tables of the game's code (far
// segment 20a5, read by the battle module s014: skill used, time for a blow, hands and
// penetration, damage, skill and strength wanted, reload time); here they are listed in the
// menu (F12 > Records > Weapons) and changed by a mod folder's rules.ini:
//
//   [weapon Short Sword]        (an item name of DARKLAND.LST, or [weapon 3] by number)
//   damage = 9
//   penetration = 4
//   time = 40                   (time for a blow; lower is quicker)
//   reload = 0                  (missile weapons: time to reload / draw)
//   hands = 1
//   skill = edged               (edged, impact, flails, polearms, thrown, bows, missile devices)
//   skill_wanted = 16
//   strength = 13-28            (or strength_min / strength_max)
//
// The game's own numbers are kept and put back when the mod is switched off.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/datamods.h"
extern char game_dir[1024];
}

#define NW 63
struct Field { const char *key; u16 off; int shift, mask; };
static const Field FIELDS[] = {
    { "skill", 0x75ac, 0, 0xff }, { "time", 0x7669, 0, 0xff }, { "hands", 0x76a8, 4, 0x0f }, { "penetration", 0x76a8, 0, 0x0f },
    { "damage", 0x76e7, 0, 0xff }, { "skill_wanted", 0x7726, 0, 0xff }, { "strength_min", 0x7765, 0, 0xff },
    { "strength_max", 0x77a4, 0, 0xff }, { "reload", 0x77e3, 0, 0xff },
};
static const int NF = sizeof FIELDS / sizeof FIELDS[0];
static const char *SKILLS[7] = { "edged", "impact", "flails", "polearms", "thrown", "bows", "missile devices" };
static const u16 TABLES[] = { 0x75ac, 0x7669, 0x76a8, 0x76e7, 0x7726, 0x7765, 0x77a4, 0x77e3, 0x7822 };
static unsigned char orig[sizeof TABLES / sizeof TABLES[0]][NW];
static bool have_orig;

static int get(int w, const Field &f) { return rb(G_FAR20A5, (u16)(f.off + w)) >> f.shift & f.mask; }
static void set(int w, const Field &f, int v)
{
    u8 b = rb(G_FAR20A5, (u16)(f.off + w));
    b = (u8)((b & ~(f.mask << f.shift)) | ((v & f.mask) << f.shift));
    wb(G_FAR20A5, (u16)(f.off + w), b);
}

/* the weapon type's name: the first item of that type in DARKLAND.LST (relics share it) */
static std::string weapon_name(int w)
{
    for (const ItemDef &d : gamedata.items)
        if (d.type == w && !d.name.empty() && w <= 33 && (d.kind == K_WEAPON || d.kind == K_MISSILE)) return d.name;
    return "";
}

/* the weapons that use skill k (0 edged ... 6 missile devices) by the tables as they are now */
std::string rules_weapons_of_skill(int k)
{
    std::string s;
    for (int w = 0; w < NW; w++) {
        if (get(w, FIELDS[0]) != k) continue;
        std::string n = weapon_name(w);
        if (n.empty() || (", " + s + ",").find(", " + n + ",") != std::string::npos) continue;
        if (!s.empty()) s += ", ";
        s += n;
    }
    return s;
}
static int weapon_by_name(const std::string &n)
{
    char *e; long v = strtol(n.c_str(), &e, 10);
    if (*n.c_str() && !*e) return v >= 0 && v < NW ? (int)v : -1;
    for (const ItemDef &d : gamedata.items)
        if (!strcasecmp(d.name.c_str(), n.c_str()) && d.type >= 0 && d.type <= 33) return d.type;
    return -1;
}

static std::string trim(std::string s)
{
    size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

static int apply_file(const char *path, std::string &log)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    char line[300]; int w = -1, changed = 0, ln = 0;
    while (fgets(line, sizeof line, f)) {
        ln++;
        std::string s = trim(line);
        if (s.empty() || s[0] == '#' || s[0] == ';') continue;
        if (s[0] == '[') {
            size_t e = s.find(']');
            std::string sec = trim(s.substr(1, e == std::string::npos ? std::string::npos : e - 1));
            w = -1;
            if (!strncasecmp(sec.c_str(), "weapon", 6)) w = weapon_by_name(trim(sec.substr(6)));
            if (w < 0) log += "rules.ini line " + std::to_string(ln) + ": unknown section " + sec + "\n";
            continue;
        }
        size_t eq = s.find('=');
        if (w < 0 || eq == std::string::npos) continue;
        std::string k = trim(s.substr(0, eq)), v = trim(s.substr(eq + 1));
        for (char &c : k) c = (char)tolower((unsigned char)c);
        if (k == "strength") {
            int a = 0, b = 0;
            if (sscanf(v.c_str(), "%d-%d", &a, &b) == 2) { set(w, FIELDS[6], a); set(w, FIELDS[7], b); changed++; }
            else if (sscanf(v.c_str(), "%d", &a) == 1) { set(w, FIELDS[6], a); changed++; }
            continue;
        }
        if (k == "skill") {
            int idx = -1;
            for (int i = 0; i < 7; i++) if (!strncasecmp(v.c_str(), SKILLS[i], strlen(SKILLS[i]) > 4 ? 4 : strlen(SKILLS[i]))) idx = i;
            if (idx < 0 && isdigit((unsigned char)v[0])) idx = atoi(v.c_str());
            if (idx >= 0 && idx < 7) { set(w, FIELDS[0], idx); changed++; }
            continue;
        }
        bool ok = false;
        for (int i = 0; i < NF; i++) if (k == FIELDS[i].key) { set(w, FIELDS[i], atoi(v.c_str())); changed++; ok = true; }
        if (!ok) log += "rules.ini line " + std::to_string(ln) + ": unknown field " + k + "\n";
    }
    fclose(f);
    return changed;
}

static std::string rules_log;
static int rules_changed;
static bool want_apply;
static unsigned char expect[sizeof TABLES / sizeof TABLES[0]][NW];
#define NT (sizeof TABLES / sizeof TABLES[0])

/* asked for at start and when mods change; done by rules_frame once the game has set up
 * its tables (they are filled in after the program starts) */
extern "C" void rules_apply(void) { want_apply = true; }

static void apply_now(void)
{
    if (!gamedata.loaded) gamedata_load(game_dir);
    for (size_t t = 0; t < NT; t++) for (int w = 0; w < NW; w++) wb(G_FAR20A5, (u16)(TABLES[t] + w), orig[t][w]);
    rules_log.clear(); rules_changed = 0;
    for (int i = 0; i < datamods_n && datamods_enabled(); i++) {
        if (!datamods[i].on) continue;
        char p[1300];
        snprintf(p, sizeof p, "%s/rules.ini", datamods[i].path);
        rules_changed += apply_file(p, rules_log);
    }
    if (!rules_log.empty()) fprintf(stderr, "%s", rules_log.c_str());
    for (size_t t = 0; t < NT; t++) for (int w = 0; w < NW; w++) expect[t][w] = rb(G_FAR20A5, (u16)(TABLES[t] + w));
}

/* every frame: the game's numbers are kept once it runs; the mods' put in, and again should
 * the game ever fill its tables anew */
extern "C" void rules_frame(void)
{
    if (game_mode() == GM_OTHER && !have_orig) return;
    if (!have_orig) {
        for (size_t t = 0; t < NT; t++) for (int w = 0; w < NW; w++) orig[t][w] = rb(G_FAR20A5, (u16)(TABLES[t] + w));
        have_orig = true; want_apply = true;
    }
    if (!want_apply) {
        bool same = true;
        for (size_t t = 0; t < NT && same; t++) for (int w = 0; w < NW; w++) if (rb(G_FAR20A5, (u16)(TABLES[t] + w)) != expect[t][w]) { same = false; break; }
        if (same) return;
        if (!rules_changed) { for (size_t t = 0; t < NT; t++) for (int w = 0; w < NW; w++) expect[t][w] = rb(G_FAR20A5, (u16)(TABLES[t] + w)); return; }
    }
    want_apply = false;
    apply_now();
}

/* the numbers as a rules.ini (a start for a mod) */
static bool write_rules(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    fprintf(f, "# Weapon numbers (F12 > Records > Weapons). Change what you like and delete the rest;\n"
               "# the game's own numbers stay for anything not named here.\n"
               "# skill: edged, impact, flails, polearms, thrown, bows, missile devices. time: for a blow (lower is quicker).\n"
               "# strength = min-max: below min each point costs 1 attack and 3 defence; above max blows hit harder.\n\n");
    for (int w = 0; w <= 33; w++) {
        std::string n = weapon_name(w);
        if (n.empty()) continue;
        fprintf(f, "[weapon %s]\n", n.c_str());
        int sk = get(w, FIELDS[0]);
        fprintf(f, "skill = %s\n", sk < 7 ? SKILLS[sk] : std::to_string(sk).c_str());
        fprintf(f, "damage = %d\npenetration = %d\nhands = %d\ntime = %d\n", get(w, FIELDS[4]), get(w, FIELDS[3]), get(w, FIELDS[2]) >= 2 ? 2 : 1, get(w, FIELDS[1]));
        if (get(w, FIELDS[8])) fprintf(f, "reload = %d\n", get(w, FIELDS[8]));
        fprintf(f, "skill_wanted = %d\nstrength = %d-%d\n\n", get(w, FIELDS[5]), get(w, FIELDS[6]), get(w, FIELDS[7]));
    }
    fclose(f);
    return true;
}

/* F12 > Records > Weapons */
void rules_tab(void)
{
    ImGui::TextWrapped("The battle numbers of every weapon, as the game uses them now%s.", rules_changed ? " (changed by a mod)" : "");
    static const char *SK[7] = { "Edged", "Impact", "Flails", "Polearms", "Thrown", "Bows", "Missile dev." };
    if (ImGui::BeginTable("weapons", 9, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit,
                          ImVec2(0, ImGui::GetContentRegionAvail().y - 110))) {
        const char *H[9] = { "Weapon", "Skill", "Hands", "Damage", "Penetr.", "Time", "Reload", "Skill wanted", "Strength" };
        for (int i = 0; i < 9; i++) ImGui::TableSetupColumn(H[i], i == 0 ? ImGuiTableColumnFlags_WidthStretch : 0);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        for (int w = 0; w <= 33; w++) {
            std::string n = weapon_name(w);
            if (n.empty()) continue;
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(n.c_str());
            int sk = get(w, FIELDS[0]);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(sk < 7 ? SK[sk] : "?");
            ImGui::TableNextColumn(); ImGui::Text("%d", get(w, FIELDS[2]) >= 2 ? 2 : 1);
            ImGui::TableNextColumn(); ImGui::Text("%d", get(w, FIELDS[4]));
            ImGui::TableNextColumn(); ImGui::Text("%d", get(w, FIELDS[3]));
            ImGui::TableNextColumn(); ImGui::Text("%d", get(w, FIELDS[1]));
            ImGui::TableNextColumn(); if (get(w, FIELDS[8])) ImGui::Text("%d", get(w, FIELDS[8]));
            ImGui::TableNextColumn(); ImGui::Text("%d", get(w, FIELDS[5]));
            ImGui::TableNextColumn(); ImGui::Text("%d-%d", get(w, FIELDS[6]), get(w, FIELDS[7]));
        }
        ImGui::EndTable();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrapped("Time: for a blow, lower is quicker. Each point of skill short of what a weapon wants costs 2 attack and 2 defence; "
                       "below its strength range each point costs 1 attack and 3 defence and blows hit softer, above it blows hit harder.");
    ImGui::PopStyleColor();
    static char modname[64] = "My weapons"; static std::string msg;
    ImGui::SetNextItemWidth(UIS(220)); ImGui::InputText("##rulesmod", modname, sizeof modname); ImGui::SameLine();
    if (ImGui::Button("Write these numbers into a mod folder (rules.ini)") && modname[0]) {
        char w[1300]; int r = datamods_new(modname, w, sizeof w);
        std::string p = std::string(w) + "/rules.ini";
        if (r < 0) msg = "Could not make the mod folder.";
        else if (write_rules(p.c_str())) msg = "Written: " + p + ". Edit it, then tick the mod in the Mods tab.";
        else msg = "Could not write " + p + ".";
    }
    if (!msg.empty()) ImGui::TextWrapped("%s", msg.c_str());
    if (!rules_log.empty()) ImGui::TextColored(ImVec4(0.6f, 0.15f, 0.1f, 1), "%s", rules_log.c_str());
}
