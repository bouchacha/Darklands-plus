/* hdtext.h - crisp text (hdtext.c) */
#ifndef DL_HDTEXT_H
#define DL_HDTEXT_H
#ifdef __cplusplus
extern "C" {
#endif
#define HD_MAXRUNS 256
typedef struct { char text[100]; int x, y, w, h, font; unsigned rgb; } HdRun;   /* game pixels */
extern HdRun hdtext_runs[HD_MAXRUNS];
extern int hdtext_nruns, hdtext_mode;
void hdtext_prepare(unsigned *rgb, const unsigned *lut);
void hdtext_set(int mode);
#ifdef __cplusplus
}
#endif
#endif
