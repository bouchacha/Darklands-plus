/* saves.h - quick save / load, autosaves, loading from the menu (saves.c) */
#ifndef DL_SAVES_H
#define DL_SAVES_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    char file[16];                  /* DKSAVE3.SAV */
    char place[22], comment[80];
    int year, month, day, hour;     /* game time (month from 0, day from 1) */
    int florins, groschen, pfennigs, fame;   /* the purse (unsigned words in the save) */
    char members[5][12]; int nmembers;
    long long mtime;                /* when it was written (host clock) */
    int battle;                     /* saved during a battle (a .bsv beside it) */
} SaveInfo;
extern int saves_on, saves_auto_arrive, saves_auto_battle, saves_keep_quick, saves_keep_auto;
void saves_init(void);
void saves_enable(int on);
void saves_frame(void);
int saves_key(int sym, int ctrl);                 /* Ctrl+S, F9, Ctrl+L; 1 if taken */
void saves_quick_save(void);
void saves_quick_load(void);
int saves_load(const char *file);                 /* at the game's next chance */
void saves_title_load(const char *file);          /* the start screen's load screen, about to open, reads it */
int saves_list(SaveInfo **out);                   /* newest first; free() */
int saves_read(const char *file, SaveInfo *si);
int saves_delete(const char *file);               /* not the one loaded; 0 if refused */
int saves_ironman(void);                          /* the running game is an Ironman game (one save, kept by the game) */
#ifdef __cplusplus
}
#endif
#endif
