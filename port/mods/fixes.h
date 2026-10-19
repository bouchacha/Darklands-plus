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
void potion_bonus(int m, int *dmg, int *pen);
int fix_is_on(const char *id);
int fixes_running(int m, int type0);             /* the largest running kind 0x4a event of member m of that type (0 none), a battle's too */                  /* the fix is on (and the original game not asked for) */
int fixes_cards_down(void);                       /* a battle (and its loot) has closed the cards: the event table holds only what the battle made */
/* oilweapons: each weapon oil only with the weapons the manual names */
int oils_weapon(int i);                         /* the weapon type member i strikes with now (combatant +0x51, or +0x22 when shooting) */
int oils_allow(int oil, int w);                 /* oil 0x69 Deadly Blade, 0x6a Strongedge, 0x6b Great Power, on weapon type w (1: in force) */
void fixes_alchemy_blast_odds(int risk, int *num, int *den);   /* alchrisk: of a failed brew's accidents (at a place), num in den are explosions */
int fixes_debt_note_office(int aux);            /* debtnote: the office a debt-note errand's giver names (1 the Medici, 2 the Fugger) */
void fixes_card_opts(void);                     /* cardopts: a card's opt_on[] put right before its options are shown */   /* weapon potions still running for member m */
#ifdef __cplusplus
}
#endif
#endif
