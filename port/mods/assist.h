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
    struct { int show, pct, who, opt, lead; } line[20];   /* lead: the member who would do best as leader (-1: the leader is best) */
} CardChances;
extern CardChances card_chances;
extern int assist_chances;            /* 0 off, 1 as the difficulty level allows, 2 always numbers */
void assist_set_chances(int mode);
void assist_init(void);
extern int assist_card_help;           /* show the saints and items that work on a card */
void assist_set_card_help(int on);
int assist_in_card_menu(void);
void assist_watch_cards(void);      /* keep the card menu hooks (assist_in_card_menu) on */
extern int assist_card_keys;           /* number keys pick a card's options */
extern int assist_best_leader;         /* a check the game makes on the leader is made by the member it suits best */
void assist_set_best_leader(int on);
int assist_best_at_attr(int k);         /* the able member with the highest attribute k (5: charisma); the leader on a tie */
int assist_bargainer(void);             /* who bargains: the best charisma with assist_best_leader, else the leader */
void assist_leader_restore(void);       /* the leader the player chose back, after another spoke for the party */
void assist_set_card_keys(int on);
int assist_card_key(int digit, int *gx, int *gy);
extern int pathfix_party, pathfix_enemies;
void pathfix_set(int party, int enemies);
extern int battle_speed;              /* percent, 100 = the original */
void battle_speed_set(int pct);
extern int battle_fast;               /* fast forward in a battle (Z) */
#define BATTLE_FAST_PCT 600
void battle_fast_set(int on);
extern int autopause_down, autopause_hurt;   /* autopause.c: battles pause when a member falls / is badly hurt */
void autopause_set(int down, int hurt);
extern int autopause_foes;                     /* ... when foes waiting in a castle, dungeon or mine come out to fight */
void autopause_set_foes(int on);
void autopause_init(void);
#ifdef __cplusplus
}
#endif
#endif
