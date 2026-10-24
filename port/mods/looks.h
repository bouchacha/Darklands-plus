/* looks.h - more figures for the party (looks.c): the human enemies' figures */
#ifndef DL_LOOKS_H
#define DL_LOOKS_H
#ifdef __cplusplus
extern "C" {
#endif
void looks_init(void);
int looks_offered(void);                    /* Select Character Image offers them (setting, not in the original game) */
int looks_count(void);                      /* the extra figures */
const char *looks_code(int i);              /* "E05" */
const char *looks_title(int i);             /* "Gray-haired fighter" */
int looks_usable(int i);                    /* its sprites and colours are there */
int looks_is_extra(const char *code);
const char *looks_title_of(const char *code);   /* any figure's name ("" unknown) */
void looks_set_figure(int m, const char *code); /* party slot m's figure, with that figure's colours */
const char *looks_virtual_file(const char *dosname);    /* a picture made for them, or 0 */
void looks_sprite_loaded(const char *name, unsigned short frames, int slot);
int looks_picker_on(void);                  /* Select Character Image opens the picker (party.figure_picker) */
int looks_list(char out[][4], int max);     /* the figures it offers, in order */
void looks_member_figure(int m, char *code);
void looks_pick(const char *code);          /* the picker's choice (sends S to the game, which takes it) */
void looks_pick_next(const char *code);     /* the figure the next S takes (the caller sends the S) */
int looks_thumb(const char *code, int m, unsigned char *rgba, int maxw, int maxh, int *w, int *h);   /* m in that figure, RGBA */
int looks_weapons(const char *code, char out[][3], int max);   /* the weapons it has its own animations for ("SW", "BW" ...); -1: all (the game's own figures) */
#ifdef __cplusplus
}
#endif
#endif
