// party.cpp - F12 > Party: the members side by side (attributes, skills, load), the best in
// each row marked, a short word on what each attribute and skill is for, and the leader, who
// acts for the party in many of the cards' checks, changed with one click (as the game's own,
// little known Ctrl+F1..F5 on the character sheet does: leader = m; party_panel_rows(0x100)).
#include <stdio.h>
#include <string.h>
#include <string>
#include "imgui.h"
#include "../mods/nscreen.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
}

std::string rules_weapons_of_skill(int k);
void ui_skill_icon_inline(int k);

static const char *ATTR[7] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma", "Divine Favor" };
static const char *ATTR_HELP[7] = {
    "Health and stamina. Wounds take it away; at 0 the character falls unconscious. Rest brings it back.",
    "Bodily strength. Weapons and armour want a certain strength (the shops' tooltips tell), and endurance and "
    "strength together are what a character can carry. Bad wounds take strength too; at 0 the character dies.",
    "Speed and deftness, of the body and of the hands.",
    "Alertness: noticing things, people and dangers.",
    "Wits and learning.",
    "Presence: how people take to the character.",
    "The saints' goodwill. Praying to a saint uses it up; it comes back with time.",
};
static const char *SKILL[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows",
    "Missile Devices", "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing",
    "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };
static const char *SKILL_HELP[19] = {
    0, 0, 0, 0, 0, 0, 0,
    "Mixing potions from the formulas a character knows: a better alchemist fails less often (F12 > Alchemy).",
    "Knowledge of the faith and its saints: learning of saints, dealings with the Church.",
    "Moral purity, valued by the saints and the Church. Sinful deeds lower it.",
    "Talking with ordinary people in their own tongue: asking, persuading, bluffing.",
    "The tongue of the Church and of scholars: priests, monks, universities.",
    "Literacy: letters, books and formulas.",
    "Tending wounds and sickness.",
    "Mechanical skill: locks, traps and devices.",
    "Moving and hiding unseen and unheard.",
    "The ways of the city streets and their underworld.",
    "Handling horses.",
    "Life in the wild: finding the way, hunting, reading tracks.",
};

const char *ui_attr_help(int a) { return a >= 0 && a < 7 ? ATTR_HELP[a] : ""; }       /* (the native sheet too) */
const char *ui_skill_help(int k) { return k >= 0 && k < 19 && SKILL_HELP[k] ? SKILL_HELP[k] : ""; }

static void set_leader_now(int m, int, int, int)
{
    g_wb(G_LEADER, (u8)m);
    u16 a[1] = { 0x100 };
    game_call_far(GF_party_panel_rows, 1, a);                   /* party_panel_rows(0x100): names, the leader's in yellow */
}

static std::string nick(int m)
{
    char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
    return game_text(n, sizeof n);
}

static int tip_frame = -1;           /* one tooltip a frame: neighbouring rows' selectables meet */
static void help_tip(const char *t)
{
    if (!t || !ImGui::IsItemHovered() || tip_frame == ImGui::GetFrameCount()) return;
    tip_frame = ImGui::GetFrameCount();
    ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 26); ImGui::TextUnformatted(t); ImGui::PopTextWrapPos(); ImGui::EndTooltip();
}

extern "C" void tab_members(void)
{
    if (game_mode() == GM_OTHER) { ImGui::TextWrapped("Start or load a game first."); return; }
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    int lead = (s8)g_rb(G_LEADER);
    int mode = game_mode();
    bool can_lead = mode == GM_CARD || mode == GM_MAP;
    ImGui::TextWrapped("On many cards the leader is the one who acts (the badges on the options name who and the chance), "
                       "so the member best at what comes next can lead first. Hover over a row for what it is for.");
    ImGuiTableFlags fl = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("members", size + 1, fl, ImVec2(0, ImGui::GetContentRegionAvail().y))) return;
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 10.5f);
    for (int m = 0; m < size; m++) ImGui::TableSetupColumn(nick(m).c_str(), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupScrollFreeze(1, 2);
    ImGui::TableHeadersRow();

    const ImVec4 best_col(0.55f, 0.25f, 0.05f, 1), low_col(0.62f, 0.18f, 0.12f, 1);
    /* the leader row */
    ImGui::TableNextRow();
    ImGui::TableNextColumn(); ImGui::TextUnformatted("Leader");
    help_tip("The leader speaks and acts for the party on many cards, and leads on the travel map. "
             "The game's own way to change it: Ctrl+F1 ... Ctrl+F5 on a character sheet.");
    for (int m = 0; m < size; m++) {
        ImGui::TableNextColumn(); ImGui::PushID(m);
        int st = g_rb((u16)(G_MEMBER(m) + G_M_STATUS));
        if (m == lead) ImGui::TextColored(best_col, "leads");
        else if (st != 1) ImGui::TextDisabled(st == 3 ? "dead" : "cannot lead");
        else {
            ImGui::BeginDisabled(!can_lead);
            if (ImGui::SmallButton("Make leader")) {
                ns_defer(set_leader_now, m, 0, 0, 0);                  /* (at the next safe point: mods/nscreen.h) */
            }
            ImGui::EndDisabled();
            if (!can_lead && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("On a card or the travel map");
        }
        ImGui::PopID();
    }

    auto section = [&](const char *t) {
        ImGui::TableNextRow(ImGuiTableRowFlags_None);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(150, 110, 60, 70));
        ImGui::TableNextColumn(); ImGui::TextUnformatted(t);
        for (int m = 0; m < size; m++) ImGui::TableNextColumn();
    };
    /* one row: v[m] shown, the highest (if they differ) marked; cur < max shown as cur/max */
    auto row = [&](const char *label, const char *tip, const int *v, const int *vmax, int icon) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (icon >= 0) ui_skill_icon_inline(icon);
        ImGui::Selectable(label, false, ImGuiSelectableFlags_SpanAllColumns); help_tip(tip);      /* the whole row answers the pointer */
        int hi = -1000, lo = 1000;
        for (int m = 0; m < size; m++) { if (v[m] > hi) hi = v[m]; if (v[m] < lo) lo = v[m]; }
        for (int m = 0; m < size; m++) {
            ImGui::TableNextColumn();
            char b[32];
            if (vmax && v[m] < vmax[m]) snprintf(b, sizeof b, "%d / %d", v[m], vmax[m]); else snprintf(b, sizeof b, "%d", v[m]);
            if (hi > lo && v[m] == hi) ImGui::TextColored(best_col, "%s  *", b);
            else if (vmax && v[m] * 2 < vmax[m]) ImGui::TextColored(low_col, "%s", b);
            else ImGui::TextUnformatted(b);
        }
    };

    section("Attributes");
    for (int a = 0; a < 7; a++) {
        int v[5], vm[5];
        for (int m = 0; m < size; m++) { v[m] = g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a)); vm[m] = g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a)); }
        row(ATTR[a], ATTR_HELP[a], v, vm, -1);
    }
    {   /* load, as the character sheet shows it (s029 0e76:013a) */
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const char *tip = "The weight carried against endurance + strength (as they are now): light up to half of it, "
                          "heavy above it, overloaded above one and a half times.";
        ImGui::Selectable("Load", false, ImGuiSelectableFlags_SpanAllColumns); help_tip(tip);
        for (int m = 0; m < size; m++) {
            ImGui::TableNextColumn();
            u16 b = G_MEMBER(m);
            int load = g_rb((u16)(b + G_M_LOAD)), cap = g_rb((u16)(b + G_M_ATTR_CUR)) + g_rb((u16)(b + G_M_ATTR_CUR + 1));
            const char *cls = load * 2 <= cap ? "light" : load <= cap ? "normal" : load * 2 <= cap * 3 ? "heavy" : "overloaded";
            if (load > cap) ImGui::TextColored(low_col, "%d / %d %s", load, cap, cls); else ImGui::Text("%d / %d %s", load, cap, cls);
        }
    }
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::TextUnformatted("Age");
        for (int m = 0; m < size; m++) { ImGui::TableNextColumn(); ImGui::Text("%d", (s16)g_rw((u16)(G_MEMBER(m) - 2))); }
    }
    section("Skills");
    for (int k = 0; k < 19; k++) {
        int v[5];
        for (int m = 0; m < size; m++) v[m] = g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k));
        std::string tip;
        if (k < 7) {
            std::string w = rules_weapons_of_skill(k);
            tip = "Attack and defence with " + (w.empty() ? std::string("these weapons") : w) + ".";
        } else tip = SKILL_HELP[k];
        row(SKILL[k], tip.c_str(), v, nullptr, k);
    }
    ImGui::EndTable();
}
