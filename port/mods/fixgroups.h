/* fixgroups.h - the fixes sorted by kind (fixgroups.c) */
#ifndef DL_FIXGROUPS_H
#define DL_FIXGROUPS_H
#ifdef __cplusplus
extern "C" {
#endif
#define FIX_NGROUPS 5
typedef struct { const char *name, *what; } FixGroup;
extern const FixGroup FIX_GROUPS[FIX_NGROUPS];
int fix_group(const char *id);            /* its group, by the fix's id (fixes.c FIXES[].id) */
#ifdef __cplusplus
}
#endif
#endif
