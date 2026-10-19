// teachersui.h - the teachers a town can offer (mods/teachers.c), as Darklands+ shows them: a
// lectern beside a card's option or a place that brings one, a line a source in the margin and on
// the map, the whole of it under the pointer (ui/teachersui.cpp). Setting assist.teachers.
#pragma once
#include <string>
extern "C" {
#include "../mods/teachers.h"
}
struct ImDrawList; struct ImVec2;

bool ui_teachers_on(void);
TchTown ui_town_facts(int loc);                                   // DARKLAND.CTY, and whether this game has its physician, alchemists
int ui_town_teachers(int loc, TchSource *out, int max);           // teach_town for that town
const char *ui_teacher_short(int src);                            // "Blacksmiths", "University" ...
std::string ui_teacher_skills(const TchSource &s);                // "Edged, Impact"
std::string ui_teacher_text(const TchSource &s);                  // all of it, for a tooltip
std::string ui_teachers_hired_line(int loc);                      // "Edged (until 12 May)", "" none
void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page);   // a lectern with its book, about s across
