/* marchview.h - Change Marching Order as a native dialog (mods/marchview.c runs it, ui/marchview.cpp
 * draws it): the members in their order, moved up and down, instead of clicking each in turn. */
#ifndef DL_MARCHVIEW_H
#define DL_MARCHVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int open;
    int n, order[5];                 /* the able members, first to last */
    char name[5][24];
    char figure[5][4];
    unsigned char pal[5][24];
    unsigned seq;
} MarchModel;

extern MarchModel march_model;

enum { MV_UP = 1, MV_DOWN, MV_DONE, MV_CANCEL, MV_MOVE };   /* MV_MOVE k, b: the k-th dragged to place b */
/* MV_UP / MV_DOWN k: the k-th in the order a place earlier / later */

void marchview_init(void);
int marchview_is_on(void);
void marchview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
