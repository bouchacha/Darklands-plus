// overlay.h - what the overlay's files share (overlay.cpp draws the layer over the running game and
// calls the others; battleoverlay.cpp: the battle's bars, the pointer's info, the fast-forward badge).
#pragma once
#include "imgui.h"

ImDrawList *ui_overlay_list(void);       /* the overlay layer: under tooltips and windows, over the picture */
ImVec2 ui_g2w(float gx, float gy);        /* a game pixel (320x200) in window pixels */
void ui_draw_battle_bars(void);
void ui_draw_battle_info(void);
void ui_draw_battle_fast(void);
