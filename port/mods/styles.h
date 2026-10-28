/* styles.h - each party member's way of fighting (styles.c): what they do in a battle until the player
 * gives them an order, chosen on their box in the party panel; mods/ways.c carries it out. */
#ifndef DL_STYLES_H
#define DL_STYLES_H
#ifdef __cplusplus
extern "C" {
#endif
/* the numbers are kept in the settings (battle.style.<name>): never renumber */
enum { ST_OFF, ST_ARCHER, ST_SKIRMISHER, ST_FOOTMAN, ST_CHARGER, ST_BERSERKER, ST_BODYGUARD, ST_HOLDFAST, ST_N };
/* the order shown in the chooser */
extern const int STYLE_ORDER[ST_N];
int style_of(int m);                    /* member m's (0..4) way: chosen, or as their weapons suggest */
int style_chosen(int m);                /* chosen for this character (-1: none, the suggested one) */
int style_suggested(int m);             /* a missile weapon in hand: Skirmisher; else Man-at-arms (ST_FOOTMAN) */
void style_set(int m, int st);          /* -1: back to the suggested one */
const char *style_name(int st);
const char *style_brief(int st);        /* one line: what the member does */
void style_test_set(int m, int st);     /* (tests: member m's way for this session only; -1 none) */
#ifdef __cplusplus
}
#endif
#endif
