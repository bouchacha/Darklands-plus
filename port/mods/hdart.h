/* hdart.h - replacement art (hdart.c) */
#ifndef DL_HDART_H
#define DL_HDART_H
#ifdef __cplusplus
extern "C" {
#endif
#define HDART_MAXDRAWS 1024          /* (a battlefield whose scenery is all replaced: some hundreds) */
/* draw picture pic's part (sx, sy, w, h) over the game pixels (x, y, w, h), colour * mod/255 */
typedef struct { int pic, sx, sy, x, y, w, h; unsigned char mod[3]; } HdDraw;
extern HdDraw hdart_draws[HDART_MAXDRAWS];
extern int hdart_ndraws, hdart_on, hdart_generation;
void hdart_prepare(unsigned *rgb, const unsigned char *idx, const unsigned *lut);
void *hdart_begin_screen(void);             /* keep the current page regions and sprite handles, NULL on failure */
void hdart_end_screen(void *saved);         /* after the game's pages are restored; accepts NULL */
const char *hdart_file(int pic);
const char *hdart_name(int pic);
void hdart_size(int pic, int *w, int *h);
int hdart_count(void);
int hdart_seen(void);
void hdart_rescan(void);
int hdart_export(char *msg, int msgn);
void hdart_set(int on);
int hdart_pick(int x, int y, int *px, int *py, int *pw, int *ph);   /* picture under a screen pixel, -1 none */
int hdart_on_screen(const char *name);   /* that picture is on the screen (tracked pictures only) */
int hdart_has_art(int pic);
const char *hdart_art_file(const char *key);   /* the replacement file named key (art\KEY.png, a mod's art too), 0 if none */
extern int hdart_show_names;
long hdart_pic_encode(const unsigned char *px, int w, int h, unsigned char **out);   /* a PIC file of the pixels (malloc'd) */
#ifdef __cplusplus
}
#endif
#endif
