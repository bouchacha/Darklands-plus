/* citynav.h - the town quest givers shown by Go to and the world map. */
#pragma once
#include <vector>
#include "../mods/offices.h"

#include <string>
#include "imgui.h"

struct TownOffice {
    const char *name;
    int icon;                   /* TownIcon (townicons.h) */
    int giver;                  /* the quest giver (mods/offices.h office_status) */
    OfficeStatus status;
};

bool citynav_work_enabled(void);
std::vector<TownOffice> citynav_town_offices(int town);
/* a giver's state and its colour (the tile shortens the trading line), and its tooltip's words */
ImU32 citynav_office_line(const OfficeStatus &os, int giver, char *out, size_t n, bool tile = false);
std::string citynav_office_tip(const OfficeStatus &os);
