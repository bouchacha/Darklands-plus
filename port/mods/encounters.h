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
int encounters_offered(void);               /* new encounters may come up now (setting, mods, not the original game) */
void encounters_force(int i);               /* test: the next wilderness encounter is new encounter i */
int encounters_force_pending(void);
int encounters_builtin(int i);              /* encounter i is Darklands+'s own (cut content restored), not a mod's */
int encounters_card_of(const char *id);     /* the card number of a loaded encounter, or -1 */
#ifdef __cplusplus
}
#endif
#endif
