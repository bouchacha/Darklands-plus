// teachersui.h - the teachers a town can offer (mods/teachers.c), as Darklands+ shows them: a
// lectern beside a card's option or a place that brings one, a line a source in the margin and on
// the map, the whole of it under the pointer (ui/teachersui.cpp). Setting assist.teachers.
#pragma once
#include <string>
#include <vector>
extern "C" {
#include "../mods/teachers.h"
}
struct ImDrawList; struct ImVec2;

bool ui_teachers_on(void);
TchTown ui_town_facts(int loc);                                   // DARKLAND.CTY, and whether this game has its physician, alchemists
int ui_town_teachers(int loc, TchSource *out, int max);           // teach_town for that town
const char *ui_teacher_short(int src);                            // "Blacksmiths", "University" ...
std::string ui_teacher_skills(const TchSource &s);                // "{Edged} {Impact}": their shields (ui/richtext.h)
std::string ui_teacher_text(const TchSource &s);                  // all of it, for a tooltip
std::string ui_teachers_hired_line(int loc);                      // "{Edged} (until 12 May)", "" none
void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page);   // a lectern with its book, about s across
const char *ui_skill_short(int k);                                // "Edged", "Missile Devices", "Read & Write" ...
int ui_teacher_icon(int src);                                     // the place's town icon (TI_*): the smithy, the university ...
/* who would learn skill k from a teacher who teaches to `to` - the one rule wherever a teacher shows dim when nobody
   would (the margin, the map, Find teachers): a fighting skill if some member's best weapon of that kind (hand to hand:
   Edged, Impact, Flails, Polearms; at range: Thrown, Bows, Missile Devices) is below it; a skill each member needs
   (Religion, Virtue, Stealth, Riding) each member below it; a skill the party needs once (the others) the party's best,
   if below it. out (may be 0): the members and their skill now (for a party's skill, its best); whether anyone would */
struct TchLearner { int m; int v; bool party; };   /* (party: the best of a skill one member does for the party) */
bool ui_teach_learners(int k, int to, std::vector<TchLearner> *out);
bool ui_teach_each(int k);                                        // each member needs it (else one does it for the party)
/* the same in words for a list (rich text): who would learn, with their skill now ("{@0:38} {@2:44}", or "{@1:47} for
   the party"); if nobody would, the skills that count all the same; *anyone as above */
std::string ui_teach_who(int k, int to, bool *anyone);
