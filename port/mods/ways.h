/* ways.h - the members' ways of fighting between orders (mods/ways.c), and what mods/orders.c (the player's
 * clicks) shares with it */
#ifndef DL_WAYS_H
#define DL_WAYS_H
#define STD_ATTACK 0x12                         /* a combatant's order (CB_ORDER): Std Attack */
#define MISSILE    0x80                         /* Use Missile */
int orders_member_mode(int m);                  /* 1 a click on an enemy shoots, 0 fights hand to hand, -1 no missile weapon */
int orders_member_style(int m);  /* the way member m fights (styles.h ST_*) */
int orders_may_move(int i);                     /* member i may walk on its own (step aside for a line) */
int ways_melee_kind(int i, int t);              /* Std Attack, or Vulnerable where only that gets through t's armor */
int ways_near_foe(int i, int m);                /* foe m within a few paces of member i */
int ways_line_clear(int i, int j);              /* a clear line of fire from i to j, where i stands */
void ways_keep_kind(int i, int kind);           /* the kind of attack the player chose for member i's order (-1 none) */
void ways_told(int i);                          /* a click of the player's taken in member i's order */
void ways_untold(void);                         /* the order given up (Esc) */
void ways_init(void);
#endif
