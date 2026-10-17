/* keep.h - things kept from selling and dropping (keep.c) */
#ifndef DL_KEEP_H
#define DL_KEEP_H
#ifdef __cplusplus
extern "C" {
#endif
int keep_is(int id, int q);
void keep_set(int id, int q, int on);
#ifdef __cplusplus
}
#endif
#endif
