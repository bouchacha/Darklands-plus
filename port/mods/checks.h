/* checks.h - things that must never happen, logged as "check:" lines (mods/checks.c) */
#ifndef DL_CHECKS_H
#define DL_CHECKS_H
enum { CK_MENU, CK_PARTY, CK_ITEMS, CK_MONEY, CK_LOC, CK_EVENTS, CK_TIME, CK_ENCOUNTERS };
void check_fail(int kind, const char *fmt, ...);   /* once a session for each kind, with where it was seen */
void checks_frame(void);                           /* each frame */
void checks_map_step(int result, int moved);       /* each step on the map (mods/travel.c) */
#endif
