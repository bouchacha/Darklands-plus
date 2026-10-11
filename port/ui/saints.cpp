// saints.cpp - the saints (F12 > Saints): every saint's description from DARKLAND.SNT, who in
// the party knows them, and a finder by what they help with ("Strength", "fire", "heal",
// "travel on water" ...), like the saint finder of the Darklands Companion tool. Knowledge:
// saints_known, far 2e38:082c, 0x14 bytes per member, a bit per saint in DARKLAND.LST order,
// highest bit first.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
}

static bool knows(int m, int s)
{
    return (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7))) != 0;
}

static std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

extern "C" void tab_saints(void)
{
    if (gamedata.saints.empty()) { ImGui::TextWrapped("DARKLAND.LST could not be read."); return; }
    bool in_game = game_mode() != GM_OTHER;
    int size = in_game ? (s16)g_rw(G_PARTY_SIZE) : 0;
    if (size < 0 || size > 5) size = 0;
    static char find[64];
    static int known_only = 0;
    ImGui::TextWrapped("What each saint can do, and who in the party knows them. Find saints by what they help with.");
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##sfind", "Strength, fire, heal, travel, Virtue ...", find, sizeof find);
    static const char *QUICK[] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma",
                                   "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile", "Alchemy", "Religion",
                                   "Virtue", "Speak Latin", "Read/Write", "Healing", "Artifice", "Stealth", "Streetwise",
                                   "Riding", "Woodwise", "fire", "water", "heal", "reputation", "Divine Favor" };
    ImGui::SameLine();
    if (ImGui::BeginCombo("##quick", "Helps with...", ImGuiComboFlags_HeightLarge)) {
        for (const char *q : QUICK) if (ImGui::Selectable(q)) snprintf(find, sizeof find, "%s", q);
        ImGui::EndCombo();
    }
    if (in_game) {
        ImGui::RadioButton("All saints", &known_only, 0); ImGui::SameLine();
        ImGui::RadioButton("Known in the party", &known_only, 1); ImGui::SameLine();
        ImGui::RadioButton("Not yet known", &known_only, 2);
    }
    ImGui::Separator();
    std::string f = lower(find);
    int shown = 0;
    ImGui::BeginChild("saints", ImVec2(0, 0));
    for (int s = 0; s < (int)gamedata.saints.size(); s++) {
        const Saint &t = gamedata.saints[s];
        std::string who;
        int nk = 0;
        for (int m = 0; m < size; m++)
            if (knows(m, s)) {
                char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
                if (!who.empty()) who += ", ";
                who += game_text(n, sizeof n); nk++;
            }
        if (known_only == 1 && !nk) continue;
        if (known_only == 2 && nk) continue;
        if (!f.empty() && lower(t.name).find(f) == std::string::npos && lower(t.text).find(f) == std::string::npos) continue;
        shown++;
        ImGui::PushID(s);
        bool open = ImGui::CollapsingHeader(t.name.c_str(), f.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen);
        if (nk) { ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.55f); ImGui::TextDisabled("known by %s", who.c_str()); }
        if (open) { ImGui::Indent(20); ImGui::TextWrapped("%s", t.text.c_str()); ImGui::Unindent(20); ImGui::Spacing(); }
        ImGui::PopID();
    }
    if (!shown) ImGui::TextDisabled("No saint matches.");
    ImGui::EndChild();
}
