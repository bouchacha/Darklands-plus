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
int fix_is_on(const char *id);                  /* the fix is on (and the original game not asked for) */
int fixes_debt_note_office(int aux);            /* debtnote: the office a debt-note errand's giver names (1 the Medici, 2 the Fugger) */
void fixes_card_opts(void);                     /* cardopts: a card's opt_on[] put right before its options are shown */   /* weapon potions still running for member m */
#ifdef __cplusplus
}
#endif
#endif
