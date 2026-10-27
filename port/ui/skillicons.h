// skillicons.h - the small pictures drawn with ImGui shapes for the skills, the attributes, a piece of
// equipment and the F12 window's tabs (skillicons.cpp; the equipment's in inventory.cpp).
#pragma once
#include "imgui.h"

/* c is the picture's middle. In the heraldic look a shield is wider and taller than s (UI_SHIELD_W, _H
   times s): text beside one starts UI_SHIELD_W * s / 2 from c, and a little more */
#define UI_SHIELD_W 1.25f
#define UI_SHIELD_H 1.44f
void ui_skill_icon(ImDrawList *dl, ImVec2 c, float s, int k);     /* skill k (0..18) at c, size s */
void ui_skill_icon_inline(int k);                                 /* in the flow of a window's text */
void ui_attr_icon(ImDrawList *dl, ImVec2 c, float s, int k);      /* attribute k (0 End .. 6 DF) */
void ui_age_icon(ImDrawList *dl, ImVec2 c, float s);              /* a member's age: an hourglass on the attributes' sable */
void ui_standing_icon(ImDrawList *dl, ImVec2 c, float s, int k);  /* the party's standing: 0 the town's opinion (reputation), 1 fame */
void ui_tab_icon(ImDrawList *dl, ImVec2 c, float s, int tab);     /* the F12 window's tab */
void ui_equip_icon(ImDrawList *dl, ImVec2 c, float s, int type);  /* an equipment type (LST type), inventory.cpp */
