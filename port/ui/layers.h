/* layers.h - who takes a click: one order for everything drawn over the game's picture (his ask,
 * 1.5.1: "obstructed clicks", a click taken by what lay under the thing clicked - the menu bar's
 * Game over a way out walked the party there).
 *
 * From the top:
 *   (a window that takes every event: the F12 window, the world map, Go to, the level map, an
 *    alert, a picker, a tip - ui_event hands it everything before any of this)
 *   UI_PANEL  Darklands+'s panels over the picture (the order strip, a way chooser, the potion tray,
 *             the stock box, the loot screen's buttons, the camp button): their buttons take the
 *             click, and the rest of the panel swallows it - the game never gets a click on a panel
 *   UI_VIEW   Darklands+'s views that run on the game's own pointer (the menu bar and its menus
 *             while open, a native screen: the character sheet, a shop, the loot screen): the
 *             game's pointer gets the click, for the view; nothing under the view does
 *   UI_FIELD  the picture itself: Darklands+'s own clicks on the field (a door, stairs, a way out)
 *             first, then the game's
 * Hints and notices draw over all of this but take nothing: a click goes through them, and a hint
 * about the field shows only while the pointer is on the field.
 *
 * Every panel over the picture says where it is as it is drawn (ui_layer_rect, or ui_cover / mclick
 * in ui/margins.cpp, which call it); the views are known from their state. The pointer's layer is
 * the topmost one under it, from this frame's rects or the last frame's (a click comes between two
 * frames). Anything new drawn over the picture follows the same: a panel registers its rect, a
 * field action asks ui_pointer_on_field() first. */
#pragma once
#ifdef __cplusplus
#include "imgui.h"
void ui_layer_rect(ImVec2 a, ImVec2 b, int layer);    /* (window coordinates, this frame) */
extern "C" {
#endif
enum { UI_FIELD = 0, UI_VIEW = 1, UI_PANEL = 2 };
int ui_layer_at(float x, float y);                    /* the topmost layer at a window point */
int ui_pointer_layer(void);                           /* at the pointer */
int ui_pointer_on_field(void);                        /* the pointer on the picture, nothing of ours above it */
#ifdef __cplusplus
}
#endif
