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
/* what the game's own cards ask for, without the mods' encounters: how many call on saint s, how many
   offer potion id (95..116) among an option's potions; a thing nothing in the game asks for (a torch,
   candle, iron spike, lantern, grappling hook, clock; the two trophies) */
int saint_info_cards_game(int s);
int potion_cards_game(int id);
int item_unasked_game(int id);
const char *saint_info_note(int s);      /* how long the help lasts (the texts say only other spans); "" when nothing of it wears off */

#ifdef __cplusplus
}
#endif
#endif
