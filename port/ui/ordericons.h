// ordericons.h - the pictures on the battle's order buttons (ui/ordericons.cpp)
#pragma once

struct ImDrawList; struct ImVec2;

enum OrderIcon {
    OI_NONE = -1,
    OI_RESUME, OI_DONE, OI_INFO, OI_WALK, OI_FLEE, OI_HALT, OI_GROUP, OI_FILE, OI_DOOR, OI_STAIRS, OI_CHEST,
    OI_PICK, OI_DISSOLVE, OI_DISARM, OI_SURRENDER, OI_LOOT, OI_EXIT, OI_CANCEL,
    OI_ATTACK, OI_VULNERABLE, OI_BERSERK, OI_PARRY, OI_MISSILE, OI_FAST, OI_N
};
void ui_draw_order_icon(ImDrawList *dl, ImVec2 c, float s, int icon);   // about s pixels across, centred on c
int ui_order_icon_for(const char *name);                               // the picture for a battle menu entry's name, or OI_NONE
void ui_draw_style_icon(ImDrawList *dl, ImVec2 c, float s, int style); // a member's way of fighting (mods/styles.h ST_*)
