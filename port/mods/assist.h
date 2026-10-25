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
    struct { int show, pct, who, opt, lead; u32 reads; int pick, dep; signed char per[5]; } line[20];   /* lead: the member who would do best as leader (-1: the leader is best);
                                                                         reads: what the chance depends on (CC_SKILL(k) bits 0-18, CC_ATTR(a) 19-25, CC_REP, CC_FAME);
                                                                         pick: the player picks who speaks (what comes of it is theirs), per: each member's chance (-1: not able);
                                                                         dep: the chance is the speaker's (per[] filled: the card shows them all under the pointer, to pick one) */
} CardChances;
extern CardChances card_chances;
#define CC_SKILL(k) (1u << (k))
#define CC_ATTR(a)  (1u << (19 + (a)))
#define CC_REP      (1u << 26)
#define CC_FAME     (1u << 27)
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
void assist_pick_speaker(int opt, int m); /* member m speaks for the party on option opt, chosen next (a line with pick) */
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
