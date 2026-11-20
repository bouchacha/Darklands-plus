// combaticons.h - the pictures for a weapon's, a shield's and an armor's numbers (combaticons.cpp): one picture a
// property, the same wherever it is shown (the item tooltips, the character sheet, shops, the loot screen, a foe in a
// battle; his design, 2026-11-19). The battle orders' sword (Attack) and shield (Parry) stay theirs; the time per blow
// is the hourglass already used for how long things last; a potion's or a saint's damage and penetration show the same
// pictures in their roundel (ui/skillicons.cpp ui_effect_icon).
#pragma once
#include "imgui.h"

enum { CI_TWO_HANDS, CI_DAMAGE, CI_PEN, CI_TIME, CI_HIT, CI_DODGE, CI_DODGE_SHOT, CI_N };
/* the picture centred at c, s pixels square, in ink */
void ui_combat_icon(ImDrawList *dl, ImVec2 c, float s, int kind, ImU32 ink);
/* an armor's tier (its hardness: 0 cloth, 1 soft, 2 stiff, 3 scale, 4 mail, 5 plate): a tunic in its make, s pixels square */
void ui_armor_icon(ImDrawList *dl, ImVec2 c, float s, int tier, ImU32 ink);
const char *ui_armor_tier_name(int tier);          /* "cloth", "soft", "stiff", "scale", "mail", "plate" */
const char *ui_armor_tier_kinds(int tier);         /* the armors of it: "padded or leather", "chain or brigandine" ... */
/* in the current ImGui window, at the text's height on the line (then SameLine); returns its width */
float ui_combat_icon_inline(int kind, ImU32 ink = 0);
float ui_armor_icon_inline(int tier, ImU32 ink = 0);
