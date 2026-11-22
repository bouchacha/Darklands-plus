/* layers.h - one order for everything drawn over the game's picture, the same for the pointer and for
 * the eye: what is drawn on top takes the click and the hover, and nothing under it reacts (his asks,
 * 1.5.1: "obstructed clicks", a click taken by what lay under the thing clicked - the menu bar's Game
 * over a way out walked the party there; RC15: a hint about the field over the potion tray, and the
 * potion's tooltip not shown).
 *
 * From the top (in brackets: where it is drawn):
 *   the pointer: the game's hand over the picture, else the system's arrow [ImGui's foreground list]
 *   tooltips, of the topmost thing under the pointer only [ImGui's tooltip windows]
 *   the passing notices: ui_notice, the controller's, the MT-32's; they take nothing [UI_TIER_TOAST]
 *   UI_MODAL  a window that takes every event (the F12 window, the world map, Go to, the level map, an
 *             alert, a picker, a tip, the Save page's options - ui_event hands it everything first - and
 *             the game's own message over a page): everything under it sees no pointer at all, no hover
 *             and no tooltip (ui_draw hides ImGui's pointer from it, NsDraw::begin the game's) [ImGui's
 *             windows; its shade in UI_TIER_DIM]
 *   UI_PANEL  Darklands+'s panels over the picture (the margins, the party's boxes, the order strip, the
 *             potion tray, a way chooser, the stock box, the loot screen's buttons, the camp button):
 *             their buttons take the click, and the rest of the panel swallows it - the game never gets a
 *             click on a panel [UI_TIER_PANEL]
 *   UI_FIELD  the picture, with Darklands+'s marks on it, which take nothing (the fighters' bars, the foe
 *             arrow, the route, the chances on a card; a hint or a foe's box about the pointer, placed
 *             clear of the panels, and under them where it cannot be; shown only while the pointer is on
 *             the field) [UI_TIER_FIELD], and a page of ours over it (the town walk's, the hours passing)
 *             [UI_TIER_COVER]: Darklands+'s own clicks on the field (a door, stairs, a way out) first, then
 *             the game's
 *   UI_VIEW   Darklands+'s views that run on the game's own pointer, over the whole picture (the menu bar
 *             and its menus while open, a native screen: the character sheet, a shop, the loot screen): the
 *             game's pointer gets the click, for the view; nothing under the view does. The field's marks
 *             do not show while one is up [ImGui's background list, over the game's picture]
 *
 * Every panel over the picture says where it is as it is drawn (ui_layer_rect, or ui_cover / mclick in
 * ui/margins.cpp, which call it); the views and the windows are known from their state. The pointer's
 * layer is the topmost one under it, from this frame's rects or the last frame's (a click comes between
 * two frames). Anything new drawn over the picture follows the same: a panel registers its rect and draws
 * in its tier (ui_tier for its window, ui_tier_list for lines and shapes), a field action asks
 * ui_pointer_on_field() first. ImGui's foreground list is for the pointer, the load's cover, the game's
 * message and tests only: it lies over the tooltips. */
#pragma once
#ifdef __cplusplus
#include "imgui.h"
void ui_layer_rect(ImVec2 a, ImVec2 b, int layer);    /* (window coordinates, this frame) */
bool ui_layer_clear(ImVec2 a, ImVec2 b);              /* no panel of ours over any of a-b */
/* drawing: every window of these tiers is put under every other window, in this order */
enum { UI_TIER_COVER, UI_TIER_FIELD, UI_TIER_PANEL, UI_TIER_DIM, UI_TIER_TOAST };
void ui_tier(int tier);                               /* the window just begun is in that tier */
ImDrawList *ui_tier_list(int tier);                   /* lines and shapes in that tier (under its windows) */
void ui_tiers_stack(void);                            /* (ui_draw, before Render) */
extern "C" {
#endif
enum { UI_FIELD = 0, UI_VIEW = 1, UI_PANEL = 2, UI_MODAL = 3 };
int ui_layer_at(float x, float y);                    /* the topmost layer at a window point */
int ui_pointer_layer(void);                           /* at the pointer */
int ui_pointer_on_field(void);                        /* the pointer on the picture, nothing of ours above it */
int ui_modal_up(void);                                /* a window that takes every event is up */
#ifdef __cplusplus
}
#endif
