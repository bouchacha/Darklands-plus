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
    int sel;                      /* Load: the save chosen; Save: the save this one replaces (-1 none) */
    int armed;                    /* Load: the save whose Delete was clicked once (-1 none); a second click deletes it */
    unsigned armed_t;
    char cur_file[16];            /* the save the game last wrote or read */
    unsigned seq, caret_t;
} SavesModel;

extern SavesModel saves_model;

enum { SVV_OK = 1, SVV_CANCEL, SVV_SELECT, SVV_DELETE };   /* SVV_SELECT a: row (Save: the one to replace, again: none); SVV_OK (Load: a = row); SVV_DELETE a: row */
void savesview_push(int type, int a);

int savesview_is_on(void);
void savesview_set(int on);
void savesview_init(void);
/* the dialogs (mods/saves.c calls them from its hooks of save_game / load_game): 1 go on, 0 not */
int savesview_save(char *comment, int n, char *replace, int rn);   /* replace: the save to delete once this one is written ("" none) */
int savesview_load(char *file, int n);

#ifdef __cplusplus
}
#endif
#endif
