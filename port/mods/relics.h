/* relics.h - relics as the manual has them (mods/relics.c): a prayer to a relic's saint costs its
 * bearer less Divine Favor, relics take no lasting harm, and a cathedral asks which one is given. */
#ifndef DL_RELICS_H
#define DL_RELICS_H
#ifdef __cplusplus
extern "C" {
#endif

int relics_on(void);                          /* rules.relics */
void relics_set(int on);
void relics_init(void);
void relics_frame(void);                      /* (each frame: a relic worn down is made whole again) */

int relic_saint(int code);                    /* the saint a relic belongs to (DARKLAND.LST order), -1 any saint, -2 not a relic */
int relic_virtue(int code);                   /* the Virtue its bearer needs (DARKLAND.LST +0x28) */
int relic_favor(int code);                    /* the Divine Favor it takes off a prayer (+0x2a) */
/* what member who's relic takes off a prayer to saint: 0 none; *code the relic (may be 0) */
int relic_discount(int who, int saint, int *code);

#ifdef __cplusplus
}
#endif
#endif
