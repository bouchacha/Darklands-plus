/* battlerules.h - rules of the battles (battlerules.c): what other modules ask of them */
#ifndef DL_BATTLERULES_H
#define DL_BATTLERULES_H
#ifdef __cplusplus
extern "C" {
#endif
int combat_improved(void);     /* battle.improved (and not the original game) */
void battlerules_init(void);
void battlerules_test_missile(int i, int t);
#ifdef __cplusplus
}
#endif
#endif
