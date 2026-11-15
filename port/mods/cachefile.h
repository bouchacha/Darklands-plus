/* cachefile.h - preserve inn stores when CACHE.TMP is rewritten. */
#ifndef DL_CACHEFILE_H
#define DL_CACHEFILE_H

void cachefile_init(void);
/* The open table replaces destination and source is emptied in one write.
 * 0 means the fix is off, 1 success, -1 a failed write with the old file kept. */
int cachefile_merge(int destination, int source);

#endif
