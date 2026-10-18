/* savesview.h - the game's Save and Load drawn by Darklands+ (mods/savesview.c runs them, ui/savesview.cpp
 * draws them; mods/saves.c does the saving and loading through the game's own routines). */
#ifndef DL_SAVESVIEW_H
#define DL_SAVESVIEW_H
#include "saves.h"
#ifdef __cplusplus
extern "C" {
#endif

#define SV_COMMENT_MAX 22          /* save_game's prompt takes 22 letters */
typedef struct {
    int open;
    int loading;                  /* 0 Save, 1 Load */
    char comment[48];             /* Save: what is typed */
    int fresh;                    /* the suggested comment, not yet typed over */
    int nsaves;
    SaveInfo *saves;              /* newest first */
    int sel;                      /* Load: the save chosen (-1 none) */
    char cur_file[16];            /* the save the game last wrote or read */
    unsigned seq, caret_t;
} SavesModel;

extern SavesModel saves_model;

enum { SVV_OK = 1, SVV_CANCEL, SVV_SELECT };   /* SVV_SELECT a: row; SVV_OK (Load: a = row) */
void savesview_push(int type, int a);

int savesview_is_on(void);
void savesview_set(int on);
void savesview_init(void);
/* the dialogs (mods/saves.c calls them from its hooks of save_game / load_game): 1 go on, 0 not */
int savesview_save(char *comment, int n);
int savesview_load(char *file, int n);

#ifdef __cplusplus
}
#endif
#endif
