/* battlerules.h - rules of the battles (battlerules.c): what other modules ask of them */
#ifndef DL_BATTLERULES_H
#define DL_BATTLERULES_H
#ifdef __cplusplus
extern "C" {
#endif
void battlerules_init(void);
void battlerules_test_missile(int i, int t);
/* battle.oils_manual: each weapon oil only on the weapons the manual names */
int oils_manual_on(void);
int oils_weapon(int i);                 /* the weapon type member i strikes with now (combatant +0x51, or +0x22 when shooting) */
int oils_allow(int oil, int w);         /* oil 0x69 Deadly Blade, 0x6a Strongedge, 0x6b Great Power, on weapon type w */
#ifdef __cplusplus
}
#endif
#endif
