/* alchemyview.h - the alchemy screen as a native screen (mods/alchemyview.c runs it,
 * ui/alchemyview.cpp draws it): the formulas the alchemist knows in one list, the chosen one's
 * ingredients (needed and at hand), the batches with - and +, the chance of success, Start. */
#ifndef DL_ALCHEMYVIEW_H
#define DL_ALCHEMYVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int formula, count, can; } AvRow;
typedef struct { int item, need, have; } AvIngr;

typedef struct {
    int open;
    int who;
    char name[24];
    int nrows;
    AvRow row[66];
    int sel, page;                   /* the chosen row; page: its batches are being set */
    int count, can_more;             /* batches of the chosen formula; one more can be made */
    int most;                        /* the batches of it the party's ingredients make, at most */
    int at_place;                    /* in a town, village or hideout (an inn's room): a failure can end in a mishap */
    int pct, pct1, base, stone, intel, alch, mystic, risk;   /* the chance, as the game reckons it (pct1: with one batch) */
    int ningr;
    AvIngr ingr[5];
    unsigned seq;
    char note[160];
    unsigned note_t;
} AlchemyModel;

extern AlchemyModel alchemy_model;

enum { AV_SELECT = 1, AV_MORE, AV_FEWER, AV_START, AV_LEAVE, AV_SET };   /* AV_SET: a batches */

/* the chance of n batches of the chosen formula (one roll for them all), as alchemy_draw_page and
   alchemy_brew reckon it: Intelligence + Alchemy - the mystic number + the Stone + 100 for 1 to 3
   batches, else 100 / (n / 2), within 0 and 99; 0 for none */
int alchemy_chance(const AlchemyModel *a, int n);
int alchemy_batch_part(int n);       /* that 100, or less for more batches */

void alchemyview_init(void);
int alchemyview_is_on(void);
void alchemyview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
