/* titleview.h - the start screen as a native page (mods/titleview.c runs it, ui/titleview.cpp
 * draws it): Continue (the newest save), Load, a new party, Quickstart, Settings and Quit, over
 * the game's own art. The game's own routines still load, make the party and begin. */
#ifndef DL_TITLEVIEW_H
#define DL_TITLEVIEW_H
#include "saves.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int open;
    int nsaves;
    int can_continue;                /* the newest save can be loaded straight away (the native Load) */
    SaveInfo last;                   /* the newest save */
    int busy;                        /* a choice runs (the game's own screens): the page waits */
    unsigned seq;
} TitleModel;

extern TitleModel title_model;

enum { TV_CONTINUE = 1, TV_LOAD, TV_NEW, TV_QUICK, TV_SETTINGS, TV_QUIT };

void titleview_init(void);
int titleview_is_on(void);
void titleview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
