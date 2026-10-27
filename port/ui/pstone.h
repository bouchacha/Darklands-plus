/* pstone.h - the Philosopher's Stone, as the alchemists pictured it (pstone.cpp) */
#pragma once
#include "imgui.h"
/* drawn at c, s across, in look 0 (the philosophers' egg), 1 (the stone itself) or 2 (the squared
   circle); quality q > 0 with its number, -q without, 0 none yet; t the time (its glow) */
void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t);
int ui_pstone_look(void);                   /* the look set (display.pstone_look) */
const char *ui_pstone_look_name(int k);
const char *ui_pstone_stage_name(int q);    /* the stage of the Great Work quality q has reached */
