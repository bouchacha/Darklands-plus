/* gamemodel.h - the game's state by name: one place that knows where the party, the members,
 * their things and their equipment are kept, for the native screens and the rest of
 * Darklands+ (instead of addresses written out in every file). Reading only, unless a
 * function says otherwise; changes go through the game's own routines. */
#ifndef DL_GAMEMODEL_H
#define DL_GAMEMODEL_H
#include "../runtime/cpu.h"
#ifdef __cplusplus
extern "C" {
#endif

/* the party */
int gm_party_size(void);
int gm_leader(void);                              /* who holds the purse / leads */
int gm_member_present(int m);                     /* in the party (record in use) */
int gm_member_status(int m);                      /* 0 none, 1 able, 2 unconscious, 3 dead, 5 away */
void gm_member_name(int m, char *out, int n);     /* short name, game charset */
void gm_member_fullname(int m, char *out, int n);
int gm_member_female(int m);
void gm_money(int *fl, int *gr, int *pf);

/* attributes (0 END, 1 STR, 2 AGL, 3 PER, 4 INT, 5 CHR, 6 DF), skills (19, DARKLAND.LST order) */
void gm_member_attr(int m, int k, int *cur, int *max);
int gm_member_skill(int m, int k);
int gm_member_age(int m);
int gm_member_load(int m);                        /* weight of what is carried (the sheet's load) */
int gm_member_load_band(int m);                   /* 0 light, 1 normal, 2 heavy, 3 overloaded */
/* pictures: the figure (PICS\SHIELD<letter>.PIC, a battle look chosen at the start) and the
 * herald (PICS\<code>STAT.PIC); "" if none */
char gm_member_figure(int m);
void gm_member_herald(int m, char *out, int n);
/* the equipment as the doll shows it: item types, -1 none */
typedef struct { int weapon, vital, limb, shield, missile; } GmEquip;
void gm_member_equip(int m, GmEquip *e);
/* a battle: on, the member's order letter ('-', 'A', 'M' ...; 0 none), on another floor */
int gm_in_battle(void);
char gm_member_order(int m);
int gm_member_elsewhere(int m);
int gm_member_selected(int m);                    /* chosen for orders (the game's highlighted frame) */

/* a member's things: record i of member m */
typedef struct { int id, type, qual, count, weight; } GmItem;
int gm_item_count(int m);                         /* records (kinds of things) */
int gm_item(int m, int i, GmItem *out);           /* 0 if none */
int gm_item_equipped(int m, int i);               /* in hand, worn or ready (the member's equipment slots) */
int gm_room_for(int m, int id, int qual);         /* one more fits: a free record (64 at most) or one it stacks onto */

/* what a member knows (far data 2e38): formulae (DARKLAND.ALC order, 3 per potion), saints
 * (DARKLAND.LST order) */
int gm_knows_formula(int m, int f);
int gm_knows_saint(int m, int s);

/* item definitions (DARKLAND.LST, as the game holds it) */
void gm_itemdef_name(int id, char *out, int n);   /* game charset */
int gm_itemdef_weight(int id);
int gm_itemdef_no_trade(int id);                  /* flag 0x80: not sold or bought */
int gm_itemdef_cats(int id, unsigned long *cats); /* shop categories (+0x20) */

#ifdef __cplusplus
}
#endif
#endif
