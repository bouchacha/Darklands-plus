/* keep.h - things kept from selling and dropping, or marked to sell (keep.c) */
#ifndef DL_KEEP_H
#define DL_KEEP_H
#ifdef __cplusplus
extern "C" {
#endif
int keep_is(int id, int q);
void keep_set(int id, int q, int on);
int sell_is(int id, int q);              /* marked to sell: Sell spare gear sells it */
void keep_mark(int id, int q, int mark); /* 0 none, 1 keep, 2 sell */
#ifdef __cplusplus
}
#endif
#endif
