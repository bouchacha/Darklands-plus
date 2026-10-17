/* cacheview.h - the store room (a town's cache of the party's things) as a native screen
 * (mods/cacheview.c runs it, ui/cacheview.cpp draws it): what is stored and what the member
 * carries in two lists, Take / Put on each row (one or the whole stack), Put away everything not
 * in use, the members as tabs. The game's own G and P move the things. */
#ifndef DL_CACHEVIEW_H
#define DL_CACHEVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int id, qual, count, weight; } CvEntry;
typedef struct { int idx, id, qual, count, weight, equipped, kept, potion; } CvItem;
/* (the shared pack, inventory.shared: the right list is the party's pack, every spare thing of
   every member by kind and quality (idx -1, count the party's spares); Take puts a thing with
   whoever carries some already, else with whoever has the most free places; Put stores it from
   whoever has a spare one; Put away stores all the party's spares but the kept ones) */

typedef struct {
    int open;
    int who, npar;
    int present[5], able[5];
    char names[5][24];
    char place[24];
    int nstore;
    CvEntry store[256];
    int shared;                      /* the right list is the party's pack */
    int nitems;
    CvItem item[200];
    int room;                        /* the member shown can take another kind of thing */
    unsigned seq;
    char note[160];
    unsigned note_t;
} CacheModel;

extern CacheModel cache_model;

enum { CV_TAKE = 1, CV_PUT, CV_WHO, CV_PUTAWAY, CV_LEAVE };
/* CV_TAKE entry, n (255 the whole stack)   CV_PUT item, n   CV_WHO member   CV_PUTAWAY (all not in use)   CV_LEAVE */

void cacheview_init(void);
int cacheview_is_on(void);
void cacheview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
