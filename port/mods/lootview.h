/* lootview.h - the loot screen as a native screen (mods/lootview.c runs it, ui/lootview.cpp
 * draws it): the whole pile and the member's things in two lists, Take / Put back on each row,
 * Take all and Share out, the members as tabs. The game's own G and P move the things. */
#ifndef DL_LOOTVIEW_H
#define DL_LOOTVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int id, qual, count, weight; } LvPile;
typedef struct { int idx, id, qual, count, weight, equipped, kept; } LvItem;
/* (the shared pack, inventory.shared: the right list is the party's pack, every spare thing of
   every member by kind and quality (idx -1, count the party's spares); Take puts a thing with
   whoever carries some already, else with whoever has the most free places, and Put back takes
   it from whoever has a spare one) */

typedef struct {
    int open;
    int who, npar;
    int present[5], able[5];
    char names[5][24];
    int cash_fl, cash_gr, cash_pf;   /* the cash found (already in the purse) */
    int npile;
    LvPile pile[200];
    int shared;                      /* the right list is the party's pack */
    int nitems;
    LvItem item[200];
    int room;                        /* the member shown (the party, shared) can take another kind of thing */
    int assist;                      /* Take all / Share out offered (F12 > Helpers) */
    unsigned seq;
    char note[160];
    unsigned note_t;
} LootModel;

extern LootModel loot_model;

enum { LV_TAKE = 1, LV_PUT, LV_WHO, LV_TAKEALL, LV_SHARE, LV_LEAVE };
/* LV_TAKE pile entry, n (255 the whole stack)   LV_PUT item, n   LV_WHO member   LV_TAKEALL, LV_SHARE, LV_LEAVE */

void lootview_init(void);
int lootview_is_on(void);
void lootview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
