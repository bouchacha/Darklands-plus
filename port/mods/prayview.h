/* prayview.h - calling on a saint (the prayer screen) as a native screen (mods/prayview.c runs
 * it, ui/prayview.cpp draws it). The saint's own handler gives the chance and the default
 * divine favor, as in the original. */
#ifndef DL_PRAYVIEW_H
#define DL_PRAYVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int open;
    int who, saint, party;           /* who prays; the saint (DARKLAND.LST order); for the whole party */
    int choose;                      /* the saint helps one member: who is chosen */
    int df, maxdf, chance;           /* the favor to spend, what the member has, the chance (0..99) */
    int why, virtue, vmin, xmin;     /* no chance: 1 the member's Virtue is below the saint's minimum (vmin), 2 less favor than its least (xmin), 0 another reason */
    int cls;                         /* the statue (the handler's class) */
    char day[32];                    /* the saint's day */
    int present[5];
    char names[5][24];
    unsigned seq;
} PrayModel;

extern PrayModel pray_model;

enum { PV_MORE = 1, PV_LESS, PV_PRAY, PV_CANCEL };   /* PV_PRAY a: the member prayed for (-1 as P) */

void prayview_init(void);
int prayview_is_on(void);
void prayview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
