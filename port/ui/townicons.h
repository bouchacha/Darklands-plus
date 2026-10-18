// townicons.h - the pictures for what a town offers (ui/townicons.cpp): its places worth knowing
// (a university, a cathedral, an alchemist, a physician, the docks, a bank) and its notable shops
// (the swordsmith, armourer, bowyer, blacksmith, artificer), on the parchment roundel of the
// quests' pictures (ui/questicons.cpp). Shown in the margin beside a town, with what each offers
// under the pointer.
#pragma once

struct ImDrawList; struct ImVec2;

enum TownIcon {
    TI_UNIVERSITY, TI_CATHEDRAL, TI_ALCHEMIST, TI_PHYSICIAN, TI_DOCKS, TI_BANK,
    TI_SWORD, TI_ARMOUR, TI_BOW, TI_SMITH, TI_TOOLS, TI_N
};
void ui_draw_town_icon(ImDrawList *dl, ImVec2 c, float s, int icon);   // a roundel about s pixels across, centred on c
/* a kind of profession (ui/careericons.cpp): 0 arms, 1 nobility, 2 church, 3 trade, 4 learning, 5 crafts, 6 common folk */
void ui_draw_career_icon(ImDrawList *dl, ImVec2 c, float s, int kind);
bool citynav_town_offers(int loc, int icon);                          // ui/citynav.cpp: town loc has it (TI_UNIVERSITY .. TI_BANK)
void ui_town_offers(int loc, bool tables, float width = 0, bool words = false);   // ui/margins.cpp: its pictures and notable shops (ImGui rows)
