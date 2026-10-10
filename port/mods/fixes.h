/* fixes.h - switchable fixes for bugs of the original game (fixes.c) */
#ifndef DL_FIXES_H
#define DL_FIXES_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { const char *id, *name, *desc; int on; } Fix;
extern Fix FIXES[];
extern int FIX_COUNT;
void fixes_init(void);
void fix_enable(int i, int on);
#ifdef __cplusplus
}
#endif
#endif
