/* savecheck.h - read-only checks of the original save and battle-file layouts. */
#ifndef DL_SAVECHECK_H
#define DL_SAVECHECK_H
#include <stdio.h>

/* A null result means the file can be read. The caller owns and closes the stream. */
const char *savecheck_sav(FILE *file);
const char *savecheck_bsv(FILE *file);
long savecheck_sav_tail(FILE *file);
#endif
