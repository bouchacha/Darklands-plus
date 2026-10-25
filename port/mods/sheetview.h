/* sheetview.h - a member's sheet as a native screen (mods/sheetview.c runs it, ui/sheetview.cpp
 * draws it), the whole page drawn anew: the paper doll put together from the game's own
 * pictures, the things (in use first; Ready, Unequip, Drink, Drop with Undo; dragged onto a
 * member at the left to give), the attributes and skills on one page as the original has them,
 * and for the whole party the saints (with the game's statues) and the formulae. The game's own
 * routines equip, give, drop and drink. */
#ifndef DL_SHEETVIEW_H
#define DL_SHEETVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int idx, id, type, qual, count, weight, equipped, drinkable, holder, kept, potion; } SvItem;
/* (potion: the shared pack's sheet lists the member's own potions apart, between what is in use and the pack) */
/* (the shared pack, inventory.shared: the rows are the member's things in use (holder = the
   member, idx their record) and then the party's pack, every spare thing of every member by kind
   and quality (holder -1, idx -1, count the party's spares; who carries them is Darklands+'s
   business: a thing readied or drunk is handed to the member first)) */
typedef struct { int id, qual, count; char name[24]; } SvDrop;

typedef struct {
    int open;
    int who, leader, npar;
    int present[5], able[5];
    char names[5][24];
    char fullname[32];
    int female, age;
    int attr[7], attr_max[7];
    int skill[19];
    int load, capacity, load_band;
    int weapon, vital, limb, shield, missile;   /* what the doll wears and holds (item types, -1 none) */
    int shared;                        /* the things as one pack for the party */
    int nitems;
    SvItem item[200];
    /* the party, for the saints and formulae pages */
    unsigned char formula[5][66];     /* known formulae (DARKLAND.ALC order) */
    unsigned char saint[5][160];      /* known saints (DARKLAND.LST order) */
    int alch[5], intel[5], relig[5], df[5];
    int have[200];                    /* the party's count of each kind of thing (ingredients) */
    int ingr[5][23];                  /* each member's count of the 23 ingredients (things 72..94) */
    struct { short id, qual, count; } pots[64]; int npots;   /* the party's potions, by kind and quality (its grade: ui_potion_tier) */
    int pattr[5][7], pattr_max[5][7], pskill[5][19];   /* every member's, to set the shown one's against (Skills) */
    signed char saint_class[160];     /* the statue a saint is shown with (its handler's class, 0..13; -1 not known) */
    signed char saint_vmin[160];      /* the Virtue a saint asks of the one who prays (0 none) */
    int nsaints;
    /* things left behind on this sheet, to take back */
    int ndrops;
    SvDrop drops[16];
    unsigned seq;
    char note[160];
    unsigned note_t;
    int tab_step;                     /* Tab / Shift+Tab, Right / Left: to the next or the previous page (the view takes it) */
} SheetModel;

extern SheetModel sheet_model;

enum { SV_WHO = 1, SV_READY, SV_PUTAWAY, SV_GIVE, SV_DROP, SV_DRINK, SV_LEADER, SV_CLOSE, SV_UNDO,
       SV_PREADY, SV_PDRINK, SV_PDROP, SV_KEEP, SV_PRAY };
/* the pack's: SV_PREADY / SV_PDRINK item id, quality   SV_PDROP id, quality, 255 = all the party's spares
   SV_KEEP id, quality, on (kept from selling and dropping) */
/* SV_READY/PUTAWAY/DRINK item   SV_GIVE item, to member, 255 = the whole stack   SV_DROP item, 255 = the stack
   SV_PRAY member, saint (the prayer screen, as a click on a saint in the original sheet)
   SV_WHO / SV_LEADER member   SV_CLOSE   SV_UNDO: the last thing left behind taken back */

extern int sheetview_dragging;        /* the view drags a thing: a release over the party panel is a give, not a click there */

void sheetview_init(void);
int sheetview_shared(void);           /* inventory.shared (on by default; not in the Full Original preset) */
int sheetview_is_on(void);
void sheetview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
