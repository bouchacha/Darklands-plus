/* styles.h - each party member's way of fighting (styles.c): what they do in a battle until the player
 * gives them an order, chosen on their box in the party panel; mods/ways.c carries it out. */
#ifndef DL_STYLES_H
#define DL_STYLES_H
#ifdef __cplusplus
extern "C" {
#endif
/* the numbers are kept in the settings (battle.style.<name>): never renumber. Four ways to choose and
   Manual (ST_OFF); 5 and 6 were 1.4-RC4's Berserker and Bodyguard (read as Charger and Man-at-arms).
   ST_HELD is no way to choose: one the player sent somewhere or halted holds there for the rest of
   the fight (mods/ways.c); a 7 kept in the settings (RC4's Holdfast) is read as Archer */
enum { ST_OFF, ST_ARCHER, ST_SKIRMISHER, ST_FOOTMAN, ST_CHARGER, ST_HELD = 7, ST_N };
#define STYLE_CHOICES 5
extern const int STYLE_ORDER[STYLE_CHOICES];   /* the ways in the chooser, in its order */
int style_of(int m);                    /* member m's (0..4) way: chosen, or as their weapons suggest */
int style_chosen(int m);                /* chosen for this character (-1: none, the suggested one) */
int style_suggested(int m);             /* a missile weapon in hand: Skirmisher; else Man-at-arms (ST_FOOTMAN) */
void style_set(int m, int st);          /* -1: back to the suggested one */
const char *style_name(int st);
const char *style_brief(int st);        /* one line: what the member does */
extern const char *STYLE_NOTE;          /* what a way is not: no bonus or penalty, an order takes over */
void style_test_set(int m, int st);     /* (tests: member m's way for this session only; -1 none) */
#ifdef __cplusplus
}
#endif
#endif
