/* keep.h - the player's marks (keep.c): things kept from selling and dropping, or marked to sell;
 * formulae first or last in the alchemy screen's list */
#ifndef DL_KEEP_H
#define DL_KEEP_H
#ifdef __cplusplus
extern "C" {
#endif
int keep_is(int id, int q);
void keep_set(int id, int q, int on);
int sell_is(int id, int q);              /* marked to sell: Sell spare gear sells it */
void keep_mark(int id, int q, int mark); /* 0 none, 1 keep, 2 sell */
enum { FMARK_NONE, FMARK_FIRST, FMARK_LAST };
int formula_mark(int f);                 /* a formula's place in the alchemy list: FMARK_FIRST (starred), FMARK_LAST (put aside) */
void formula_mark_set(int f, int mark);
#ifdef __cplusplus
}
#endif
#endif
