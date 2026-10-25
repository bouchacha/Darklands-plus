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
/* the game's rules (no state): what a shield of type 95 (small), 96 (medium) or 97 (large) and
   quality q adds to its bearer's defence against a blow (missile 0) or a shot or a throw (1); -5 for
   no shield, or a type that is none of these. The to-hit roll (s014 18bf:02a8): a blow (an order with
   bit 2: Std Attack, Vulnerable, Berserk, Parry) is decided by 18bf:0592 (melee_to_hit): 50 + 2/3 of (the attacker's combatant +0x20 - (the defender's +0x20 + this
   shield value)), with the stances' and the crowd's modifiers (and no shield against a flail); a
   shot or a throw by 02a8 itself: 50 + 2/3 of (the shooter's +0x21, less in a crowd or a cloud -
   (this shield value + twice the target's Agility, less of it under a load)). +0x20, the "melee
   rating" (weapon_ready, 18bf:0a40), is the weapon's skill + 2 per point of Strength above the
   weapon's range - 3 per point below it - 2 per point of skill below the weapon's need; +0x21, the
   "aim", the skill - 1 per point of Strength below - 2 per point of skill below. */
int gm_shield_defence(int type, int q, int missile);
/* the Agility member m brings to the defence against a shot or a throw (counted twice there): all of it while
   the load is at most 5/8 of (maximum Endurance + Strength) x 1, two thirds up to 10/8, a third up
   to 15/8, none above (s014 18bf:02a8, the to-hit roll of a shot or throw) */
int gm_member_fight_agility(int m);
int gm_member_age(int m);
int gm_member_load(int m);                        /* weight of what is carried (the sheet's load) */
int gm_member_load_band(int m);                   /* 0 light, 1 normal, 2 heavy, 3 overloaded */
/* pictures: the herald's letter (PICS\SHIELD<letter>.PIC; 0 none), the figure's code ("F01" ...:
 * PICS\<code>SHORT.PIC and the battle sprites; "" none) and the figure's own 8 colours (the
 * colour keys; 24 bytes, 6-bit RGB: what PICS\<code>SHORT.PIC's colours 0xeb..0xf2 stand for) */
char gm_member_herald_letter(int m);
void gm_member_figure_code(int m, char *out, int n);
void gm_member_palette(int m, unsigned char out[24]);
/* the equipment as the doll shows it: item types, -1 none */
typedef struct { int weapon, vital, limb, shield, missile; } GmEquip;
void gm_member_equip(int m, GmEquip *e);
/* a battle: on, the member's order letter ('-', 'A', 'M' ...; 0 none), on another floor */
int gm_in_battle(void);
char gm_member_order(int m);
int gm_member_order_place(int m);                 /* with order 'U', 'P', 'D' ...: the place action (1 door, 2 stairs, 3 a door's lock, 6 a chest's,
                                                     4 / 7 dissolve a lock, 5 open a chest, 8 leave the field, 10 halt; the game's Orders menu) */
int gm_member_elsewhere(int m);
int gm_member_selected(int m);                    /* chosen for orders (the game's highlighted frame) */

/* a member's things: record i of member m */
typedef struct { int id, type, qual, count, weight; } GmItem;
int gm_item_count(int m);                         /* records (kinds of things) */
int gm_item(int m, int i, GmItem *out);           /* 0 if none */
int gm_item_equipped(int m, int i);               /* in hand, worn or ready (the member's equipment slots) */
int gm_item_riding(int m, int i);                 /* the shared pack: the member's own mount (their best horse or mule) */
int gm_item_in_use(int m, int i);                 /* equipped, or (shared pack) the mount they ride */
int gm_is_potion(int id);                         /* a potion (items 95-116): each member's own, never the shared pack's */
int gm_is_ammo(int id);                           /* arrows, quarrels, balls (items 33-35) */
int gm_ammo_hidden(void);                         /* they never run out (assist.infinite_ammo): not shown anywhere */
int gm_member_own(int id, int type);              /* with the shared pack, each member's own (potions; ammunition and thrown weapons that run out) */
int gm_room_for(int m, int id, int qual);         /* one more fits: a free record (64 at most) or one it stacks onto */
/* the shared pack (inventory.shared): a member's spares of record i (its count, less the one in use);
   who carries a spare (id, qual) - `prefer` first, else whoever has the most - 0 if nobody */
int gm_item_spare(int m, int i);
int gm_pack_find(int prefer, int id, int qual, int *m_out, int *i_out);

/* what a member knows (far data 2e38): formulae (DARKLAND.ALC order, 3 per potion), saints
 * (DARKLAND.LST order) */
int gm_knows_formula(int m, int f);
int gm_knows_saint(int m, int s);

/* the four saints a library teaches (a monastery's or a university's, by the place it is in): the
   saint card $Learn00 (s058 18ca) draws them with srand(party seed + the place) and rand() % 135
   four times, so they are fixed for the whole game (two may be the same). Returns 4. */
int gm_library_saints(int loc, int out[4]);

/* the time of day as the game tells it, by the canonical hours: hour_band (s029 1367:0744) gives
   0-2 Matins, 3-5 Lauds, 6-8 Prime, 9-11 Terce, 12-14 Sexts, 15-17 Nones, 18-20 Vespers,
   21-23 Compline; the names are the game's own (290e:20c3, a mod's translation shows), its
   "Latins" for Lauds put right */
int gm_hour_band(int hour);
void gm_hour_name(int hour, char *out, int n);

const char *gm_attr_name(int a);                  /* "Endurance" ... "Divine Favor" (G_M_ATTR_* order) */
const char *gm_skill_name(int k);                 /* "Edged Weapons" ... "Woodwise" (G_M_SKILLS order) */
const char *gm_month_name(int m);                 /* "January" ... (from 0; taken round the year) */
const char *gm_month_short(int m);                /* "Jan" ... */
const char *gm_a_an(int n);                       /* "a" or "an" before n in figures ("an 11% chance") */
const char *gm_skill_short(int k);                /* the same, "Edged", "Impact" ... "Thrown" for the weapon skills (where room is short) */

/* item definitions (DARKLAND.LST, as the game holds it) */
void gm_itemdef_name(int id, char *out, int n);   /* game charset */
int gm_itemdef_type(int id);                       /* its type (+0x1e): a weapon's number, 0x40-0x42 arrows, quarrels, balls ... */
int gm_member_uses(int m, int id);                /* m's missile weapon looses it (its ammunition, or the thrown weapon itself) */
int gm_itemdef_weight(int id);
int gm_itemdef_no_trade(int id);                  /* flag 0x80: not sold or bought */
int gm_itemdef_cats(int id, unsigned long *cats); /* shop categories (+0x20) */

#ifdef __cplusplus
}
#endif
#endif
