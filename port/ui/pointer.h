/* pointer.h - the game's pointer drawn anew (pointer.cpp) */
#pragma once
#include "imgui.h"
enum { PTR_GAME = -1, PTR_POINT, PTR_GRAB };                       /* the hand: the forefinger out, or a fist */
enum { PTR_PLAIN, PTR_PAUSED, PTR_ATTACK, PTR_BUSY };                /* on its cuff: nothing, the pause, crossed blades, an hourglass */
/* the gauntlet, its fingertip at tip, k window pixels to one of the sprite's (26 x 15) */
void ui_pointer_draw(ImDrawList *dl, ImVec2 tip, float k, int pose, int badge);
