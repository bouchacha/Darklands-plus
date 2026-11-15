/* citynav.h - the town quest givers shown by Go to and the world map. */
#pragma once
#include <vector>
#include "../mods/offices.h"

struct TownOffice {
    const char *name;
    int icon;                   /* TownIcon (townicons.h) */
    OfficeStatus status;
};

bool citynav_work_enabled(void);
std::vector<TownOffice> citynav_town_offices(int town);
