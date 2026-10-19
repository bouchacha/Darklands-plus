/* sainttexts.h - the "sainttexts" fix (mods/sainttexts.c): saints whose help in the game's code
 * departs from their text do what the text says. */
#ifndef DL_SAINTTEXTS_H
#define DL_SAINTTEXTS_H
extern int sainttexts_on;
void sainttexts_enable(int on);                      /* (fix_enable) */
int sainttexts_handles(int saint);                   /* its help is this fix's now (armourday's own hooks step aside) */
int sainttexts_step_minutes(int terrain, int minutes);   /* a step on the map, with the saints' speed (travel.c too) */
#endif
