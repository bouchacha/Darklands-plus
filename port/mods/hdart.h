/* hdart.h - replacement art (hdart.c) */
#ifndef DL_HDART_H
#define DL_HDART_H
#ifdef __cplusplus
extern "C" {
#endif
#define HDART_MAXDRAWS 160
/* draw picture pic's part (sx, sy, w, h) over the game pixels (x, y, w, h), colour * mod/255 */
typedef struct { int pic, sx, sy, x, y, w, h; unsigned char mod[3]; } HdDraw;
extern HdDraw hdart_draws[HDART_MAXDRAWS];
extern int hdart_ndraws, hdart_on, hdart_generation;
void hdart_prepare(unsigned *rgb, const unsigned char *idx, const unsigned *lut);
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
extern int hdart_show_names;
long hdart_pic_encode(const unsigned char *px, int w, int h, unsigned char **out);   /* a PIC file of the pixels (malloc'd) */
#ifdef __cplusplus
}
#endif
#endif
