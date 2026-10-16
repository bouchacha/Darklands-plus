/* sheetview.h - a member's sheet and pack as a native screen (mods/sheetview.c runs it,
 * ui/sheetview.cpp draws it): the attributes, what is in use, the load, and the whole pack in
 * one list with Ready / Put away / Drink / Give / Drop on each row; skills, formulae and saints
 * in the other tabs. The game's own routines equip, give, drop and drink. */
#ifndef DL_SHEETVIEW_H
#define DL_SHEETVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int idx, id, type, qual, count, weight, equipped, drinkable; } SvItem;

typedef struct {
    int open;
    int who, leader, npar;
    int present[5], able[5];
    char names[5][24];
    char fullname[32];
    int female, age;
    int attr[7], attr_max[7];
    int skill[19];
    int load, capacity;
    int nitems;
    SvItem item[64];
    unsigned char formulas[22];   /* known formulae, bit f % 3 of byte f / 3 (DARKLAND.ALC order) */
    unsigned char saints[20];     /* known saints, bit 0x80 >> (s & 7) of byte s >> 3 */
    unsigned seq;
    char note[160];
    unsigned note_t;
} SheetModel;

extern SheetModel sheet_model;

enum { SV_WHO = 1, SV_READY, SV_PUTAWAY, SV_GIVE, SV_DROP, SV_DRINK, SV_LEADER, SV_CLOSE };
/* SV_READY/PUTAWAY/DRINK item   SV_GIVE item, to member, 255 = the whole stack   SV_DROP item, 255 = the stack
   SV_WHO / SV_LEADER member   SV_CLOSE */

void sheetview_init(void);
int sheetview_is_on(void);
void sheetview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
