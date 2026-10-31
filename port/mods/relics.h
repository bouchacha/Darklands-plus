/* relics.h - relics as the manual has them (mods/relics.c): a relic's bearer with the Virtue it asks
 * pays less Divine Favor for any prayer, a weapon relic is wielded only with that Virtue, relics take
 * no lasting harm, and a cathedral asks which one is given. */
#ifndef DL_RELICS_H
#define DL_RELICS_H
#ifdef __cplusplus
extern "C" {
#endif

int relics_on(void);                          /* rules.relics */
void relics_set(int on);
void relics_init(void);
void relics_frame(void);                      /* (each frame: a relic worn down is made whole again) */

int relic_is(int code);                       /* a relic (DARKLAND.LST +0x21 bit 4) */
int relic_is_weapon(int code);                /* a relic that is a weapon (St. George's Greatsword ...) */
int relic_virtue(int code);                   /* the Virtue its bearer needs (DARKLAND.LST +0x28) */
int relic_favor(int code);                    /* the Divine Favor it takes off a prayer (+0x2a) */
int relic_bearer_ok(int m, int code);         /* member m has the Virtue it asks */
int relic_member_virtue(int m);               /* (the Virtue it is set against: the member's own, s8 at +skills 9) */
/* what member who's relic takes off a prayer to saint (the one wielded or, no weapon, carried, the best): 0 none; *code the relic */
int relic_discount(int who, int saint, int *code);

#ifdef __cplusplus
}
#endif
#endif
