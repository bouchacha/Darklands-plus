/* assist.h - playing aids (see assist.c) */
#ifndef DL_ASSIST_H
#define DL_ASSIST_H
#include "../runtime/cpu.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int active, n, seq;
    u32 polled;                       /* plat_ticks_ms() of the last menu poll */
    struct { int show, pct, who; } line[20];
} CardChances;
extern CardChances card_chances;
extern int assist_chances;            /* 0 off, 1 as the difficulty level allows, 2 always numbers */
void assist_set_chances(int mode);
void assist_init(void);
extern int pathfix_party, pathfix_enemies;
void pathfix_set(int party, int enemies);
extern int battle_speed;              /* percent, 100 = the original */
void battle_speed_set(int pct);
#ifdef __cplusplus
}
#endif
#endif
