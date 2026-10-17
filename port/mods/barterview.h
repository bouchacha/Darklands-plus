/* barterview.h - the shop screen as a native screen (mods/barterview.c runs it, ui/barterview.cpp
 * draws it). The model is what the view shows; the view asks for actions (BV_*). */
#ifndef DL_BARTERVIEW_H
#define DL_BARTERVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int id, qual, weight, price, src; } BvWare;   /* (src: the shop's ware; arrows and the like left out when hidden) */
typedef struct { int idx, id, type, qual, count, weight, price, equipped, holder, kept; } BvItem;
/* (holder: the member carrying it; -1 the party's pack (inventory.shared): every member's spares of
   that kind and quality, idx -1, sold from whoever carries them) */

typedef struct {
    int open;                 /* the shop is on screen */
    int shop, quality;        /* shop number (city_rec quality index, -1/-2 others), quality of its wares */
    char shopname[48];        /* game charset */
    int who;                  /* the member bartering (whose things are listed) */
    int payer;                /* the member holding the purse (the game's leader) */
    int npar;
    int present[5];
    char names[5][24];
    int fl, gr, pf;           /* the purse */
    long total_pf;
    int nwares;
    BvWare ware[200];
    int shared;               /* the party's pack instead of the member's things */
    int nitems;
    BvItem item[200];
    unsigned seq;             /* changes with the model */
    char note[160];           /* the last action's outcome */
    unsigned note_t;
} BarterModel;

extern BarterModel barter_model;

enum { BV_BUY = 1, BV_SELL, BV_WHO, BV_PAYER, BV_LEAVE, BV_BATCH_START, BV_BATCH_END, BV_PSELL };
/* BV_PSELL item id, quality, 255 = all the pack's spares: sold from whoever carries them */
/* BV_BUY ware n   BV_SELL item n (n = 255: the whole stack)   BV_WHO member   BV_PAYER member   BV_LEAVE
   BV_BATCH_START / BV_BATCH_END: round several sales, one word on them all at the end */

int barterview_on(void);
void barterview_set(int on);
void barterview_init(void);
void barterview_run(void);            /* from the barter screen's hook (mods/loot.c) */

#ifdef __cplusplus
}
#endif
#endif
