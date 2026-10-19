/* saintinfo.h - what each saint's help does in the game's code (mods/saintinfo.c), beside the
 * description the game shows, and where that description promises otherwise. Host-independent. */
#ifndef DL_SAINTINFO_H
#define DL_SAINTINFO_H
#ifdef __cplusplus
extern "C" {
#endif

#define SAINT_INFO_N 136
const char *saint_info_does(int s);      /* the help as the code gives it (with the fixes as they are now) */
const char *saint_info_unlike(int s);    /* where the description promises otherwise; "" none */
int saint_info_cards(int s);             /* how many of the encounters' card scripts let a member call on it */
const char *saint_info_note(void);       /* said once on a screen of saints: how long the help lasts (the texts say only other spans) */

#ifdef __cplusplus
}
#endif
#endif
