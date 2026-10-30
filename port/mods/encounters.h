/* encounters.h - new encounters written as data (ENCOUNTERS folders of mods) */
#ifndef DL_ENCOUNTERS_H
#define DL_ENCOUNTERS_H
#ifdef __cplusplus
extern "C" {
#endif
void encounters_init(void);
int encounters_count(void);                 /* encounters loaded from the mods in use (with no errors) */
int encounters_errors(void);                /* files with errors */
const char *encounters_title(int i);
int encounters_card_active(void);           /* a new encounter's card is on the screen: 1 + its index, else 0 */
int encounters_in_street(void);             /* the one on the screen came up in a town's street */
void encounters_after_load(void);           /* a save loaded (mods/saves.c): no encounter under way */
int encounters_offered(void);               /* new encounters may come up now (setting, mods, not the original game) */
void encounters_force(int i);               /* test: the next wilderness encounter is new encounter i */
int encounters_force_pending(void);
int encounters_builtin(int i);              /* encounter i is Darklands+'s own (cut content restored), not a mod's */
int encounters_card_of(const char *id);     /* the card number of a loaded encounter, or -1 */
/* what came of an encounter's choice, while the text that tells of it is shown (else -1): rows of
   EO_ATTR / EO_ATTRMAX (member m, attribute k, change v), EO_SKILL (m, skill k, v), EO_ITEM (item k, count v),
   EO_MONEY (v pfennigs), EO_FAME (v), EO_REP (town k, v), EO_HOURS (v hours passed), EO_LOST (member m gone:
   its name, encounters_outcome_name); *final: this text ends the encounter (0 rows: nothing came of it) */
enum { EO_ATTR = 1, EO_ATTRMAX, EO_SKILL, EO_ITEM, EO_MONEY, EO_FAME, EO_REP, EO_HOURS, EO_LOST };
typedef struct { int kind, m, k; long v; } EncOutcome;
int encounters_outcome(EncOutcome *out, int max, int *final);
const char *encounters_outcome_name(int m);  /* a member's name as it was before (game charset) */
void encounters_frame(void);                 /* each frame: what came of it on the game's own cards forgets away from them */
void encounters_rehook(void);                /* mods/cardview.c switched: the game cards' hooks first again */
#ifdef __cplusplus
}
#endif
#endif
