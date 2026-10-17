/* rosterview.h - the party of a new game as a native screen (mods/rosterview.c runs it,
 * ui/rosterview.cpp draws it). The game's own key handler still adds, drops, deletes, makes. */
#ifndef DL_ROSTERVIEW_H
#define DL_ROSTERVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[28], nick[12];
    int age, female;
    int attr[7], attr_max[7];        /* END STR AGL PER INT CHR DF */
    int skill[19];
    int slot;                        /* the party slot, -1 none */
    int nsaints, nformulae, nitems;
} RosterChar;

#define RV_MAX 40
typedef struct {
    int open;
    int n; RosterChar ch[RV_MAX];    /* the characters kept (charactr.tmp) */
    int sel;                         /* the one chosen (-1 none) */
    int slot_char[5];                /* the character in each party slot, -1 none */
    char figure[5][4];               /* each slot's figure ("F01" ...) */
    unsigned char pal[5][24];        /* each slot's colours (the colour keys) */
    char herald[5];                  /* each slot's herald letter (pics\shieldX.pic) */
    int party_n;
    char word[11][32];               /* the original's menu words: Create, Add, Heraldry, Image, colours 1-3, Drop, Kill, Begin, Return */
    unsigned seq;
    char note[96]; unsigned note_t;  /* a short word on the last action (the party is full ...) */
} RosterModel;

extern RosterModel roster_model;

enum { RV_SELECT = 1, RV_KEY, RV_SHEET, RV_BACK };
/* RV_SELECT i: choose character i   RV_KEY k: the game's key handler (A add, D drop, K delete, C new,
   H herald, S figure, 1/2/3 colours, B begin)   RV_SHEET slot: the member's sheet   RV_BACK: to the title */

void rosterview_init(void);
int rosterview_is_on(void);
void rosterview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
