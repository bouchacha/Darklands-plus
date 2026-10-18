/* enemyview.h - the battle's Enemy Info (E, the Orders menu) as a native page (mods/enemyview.c
 * runs it, ui/enemyview.cpp draws it): every kind of foe on the field with its picture, how many
 * and how they fare, its armour, shield and weapons, and what the party's weapons do against it. */
#ifndef DL_ENEMYVIEW_H
#define DL_ENEMYVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

#define EV_KINDS 8

typedef struct {
    int kind;                         /* the animation set (combatant +0x16) */
    int first;                        /* a combatant of it (5..12) */
    int count, able;                  /* on the field; still fighting */
    int idx[8], nidx;
    char name[24];                    /* anim_set_names[kind], game charset */
    char pic[24];                     /* "EINFO.CAT|E00SWST.PIC" */
    int human;                        /* armour and weapons (else natural ones) */
    int vital, limb, shield, melee, missile;   /* types, -1 none */
    int armour_magic, weapon_magic;
    int melee_skill, missile_skill, natural;   /* the words' indices (quality_words) */
} EnemyKind;

typedef struct {
    int open;
    int n;
    EnemyKind k[EV_KINDS];
    int difficulty;                   /* 2 expert: no words for skill */
    char quality[7][16];              /* the game's words, "very poor" .. "superb" */
    unsigned seq;
} EnemyModel;

extern EnemyModel enemy_model;

void enemyview_init(void);
int enemyview_is_on(void);
void enemyview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
