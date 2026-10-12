/* hdtext.h - crisp text (hdtext.c) */
#ifndef DL_HDTEXT_H
#define DL_HDTEXT_H
#ifdef __cplusplus
extern "C" {
#endif
#define HD_MAXRUNS 256
typedef struct { char text[100]; int x, y, w, h, font, center; unsigned rgb, bgrgb; int clip0, clip1, cont, flatbg; } HdRun;   /* game pixels; bgrgb: the mean colour behind; drawn in rows clip0..clip1 of h only; cont: goes on from the run before without a gap; flatbg: on a plain box */
extern HdRun hdtext_runs[HD_MAXRUNS];
extern int hdtext_nruns, hdtext_mode;
void hdtext_prepare(unsigned *rgb, const unsigned *lut, unsigned char *idx);
void hdtext_set(int mode);
void hdtext_cover(int page, int x, int y, int w, int h);   /* a blit on a page: text under it is gone */
int hdtext_game_cursor(int *x, int *y, int *w, int *h);
const unsigned *hdtext_cursor(int *x, int *y, int *w, int *h, int *stride);   /* the pointer's pixels (0xffRRGGBB, 0 clear) or 0 */
void hdtext_copy(int sp, int sx, int sy, int w, int h, int dp, int dx, int dy);   /* a rectangle copied between pages */
#ifdef __cplusplus
}
#endif
#endif
