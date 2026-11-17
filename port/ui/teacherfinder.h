#pragma once
#include "worldmap_draw.h"

/* The map holds the game clock. Look up the towns and their offers once when opening the finder. */
void teacher_finder_start(int member, int skill);
bool teacher_finder_close_combo();
int teacher_finder_window(bool &open, const std::vector<Place> &places, float scale, float pad);
int worldmap_land_minutes(int loc);
