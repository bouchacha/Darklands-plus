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
/* a giver's state in a line ("Quest chance 51%", "No quests until 3 Mar" ...) and its colour, the same in Go to
   and in the world map's list; and the tooltip's words (the note, with the hour a lock ends); what the chance
   rests on is OfficeStatus.factors, shown after them in both */
ImU32 citynav_office_line(const OfficeStatus &os, int giver, char *out, size_t n);
std::string citynav_office_tip(const OfficeStatus &os);
