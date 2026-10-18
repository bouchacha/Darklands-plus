/* mapmsg.h - the travel map's messages in the cards' style (mods/mapmsg.c runs it, ui/mapmsg.cpp
 * draws it). */
#ifndef DL_MAPMSG_H
#define DL_MAPMSG_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int open;
    char text[408];                   /* game charset; \n starts a paragraph */
    unsigned seq;
} MapMsgModel;

extern MapMsgModel mapmsg_model;

void mapmsg_init(void);
int mapmsg_is_on(void);
void mapmsg_set(int on);

#ifdef __cplusplus
}
#endif
#endif
