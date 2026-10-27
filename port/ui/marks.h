/* marks.h - small signs drawn with ImGui shapes (marks.cpp), at c, r its half size in window pixels:
 * the alchemy screen's (a formula starred or put aside, the mystic number's pentacle, the part of the
 * chance the batches give, what may come of a brew: the ingredients lost, a stink, an explosion);
 * on the icon sheets (uiconstyles:2) with the other families */
#pragma once
#include "imgui.h"
void ui_mark_star(ImDrawList *dl, ImVec2 c, float r, ImU32 fill, ImU32 line, float th);   /* first in a list (fill 0: outline only) */
void ui_mark_aside(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float th);               /* put aside: last in a list */
void ui_mark_pentacle(ImDrawList *dl, ImVec2 c, float r, ImU32 col, float th);            /* a formula's mystic number */
void ui_mark_falling_bars(ImDrawList *dl, ImVec2 c, float r, ImU32 col);                  /* fewer batches, a better chance */
void ui_mark_blast(ImDrawList *dl, ImVec2 c, float r);                                    /* an explosion */
void ui_mark_stink(ImDrawList *dl, ImVec2 c, float r, float th);                          /* a stink */
void ui_mark_lost(ImDrawList *dl, ImVec2 c, float r, int item, float th);                 /* the ingredients lost (item: one of them) */
/* how much, as a rank's chevrons: n (1..3) stacked on a dark badge, the lowest one's middle at base; hw the
   chevrons' half width, th their stroke (pixels). Gold pointing up; down (a cost, a loss): red pointing down.
   A potion's grade (ui/alchemyicons.cpp); a saint's help where the pictures are not shields (ui_icon_rank). Returns the badge's half width */
float ui_mark_chevrons(ImDrawList *dl, ImVec2 base, float hw, float th, int n, bool down);
