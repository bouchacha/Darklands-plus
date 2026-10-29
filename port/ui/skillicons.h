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
/* one key for every shield: 0..6 the attributes (ATTR_*), 7..25 the skills (UI_KEY_SKILL + SKILL_*), the town's opinion, fame,
   a member's age (the hourglass) */
enum { UI_KEY_SKILL = 7, UI_KEY_REP = 26, UI_KEY_FAME = 27, UI_KEY_AGE = 28, UI_KEYS = 29 };
void ui_key_icon(ImDrawList *dl, ImVec2 c, float s, int key);     /* the shield of that key at c, size s */
const char *ui_key_name(int key);                                 /* its name ("Strength", "Edged Weapons", "Reputation", "Fame") */
int ui_key_of(const char *name, int len);                         /* the key of a name (the sheet's, the short weapon names, "Latin"); -1 */
/* a shield and a value after it ("[Strength] 30 of 35"): left_mid its left end at the line's middle, s the
   shield's size, the value in font f at size fs; returns the width (pixels) */
float ui_shield_value(ImDrawList *dl, ImVec2 left_mid, float s, int key, const char *value, ImFont *f, float fs, ImU32 col);
void ui_dim_since(ImDrawList *dl, int v0, float k);               /* what was drawn into dl since vertex v0, faded to alpha k */
void ui_darken_since(ImDrawList *dl, int v0, float k);            /* the same darkened to k, kept opaque: on a dark ground, a shield's layers (its shadow, the parts
                                                                      of its charge) do not show through each other as a fade lets them */
/* what is at work that is not an attribute or a skill (ui/effects.h EI_*), on a roundel; src 1 a potion's, 2 a saint's (a mark
   at its foot), 0 not shown */
void ui_effect_icon(ImDrawList *dl, ImVec2 c, float s, int what, int src);
extern int ui_icon_rank;                                          /* for the next shield: 1..3 chevronels on its field (how much), -1..-3 reversed in red (what it costs); 0 none */
void ui_tab_icon(ImDrawList *dl, ImVec2 c, float s, int tab);     /* the F12 window's tab */
void ui_equip_icon(ImDrawList *dl, ImVec2 c, float s, int type);  /* an equipment type (LST type), inventory.cpp */
