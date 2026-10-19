/* createview.h - making a character as a native screen (mods/createview.c runs it, ui/createview.cpp
 * draws it). The game's own key handler still decides every step. */
#ifndef DL_CREATEVIEW_H
#define DL_CREATEVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int years; signed char attr[6]; signed char skill[19]; signed char maxsk[19]; } CgProf;

typedef struct {
    int open;
    int state;                       /* 0 name and gender, 1 childhood, 2 attribute EPs, 3 profession, 4 skill EPs */
    char name[24], nick[12];
    int female, age;
    int ep, ep_step;
    int attr[7], attr_spent[6];
    int skill[19], skill_spent[19];
    int prof, prev, child;           /* the last profession, the one before it, the childhood */
    int row;                         /* the attribute / skill row chosen (skills: 0..20 with gaps 7 and 14) */
    int nlist, list[20];             /* states 1 and 3: what may be chosen (childhoods 0..5; professions) */
    char prof_name[37][28], child_name[6][28];
    CgProf adult[37], childhood[6];
    signed char aging[9][6];
    int taken[37];                   /* professions taken before */
    char menu[14][40];               /* the game's words for the steps */
    int nvirtue; char virtue[66][28];
    int nsaint; int saint[32];
    int typing;                      /* 1 the name is typed, 2 the short name (the keys go there) */
    char typed[24];
    unsigned seq, caret_t;
} CreateModel;

extern CreateModel create_model;

enum { CG_KEY = 1, CG_ROWPLUS, CG_ROWMINUS, CG_TYPE, CG_TYPED, CG_LEAVE };
/* CG_KEY k: the game's key handler   CG_ROWPLUS / CG_ROWMINUS row: + / - on that attribute or skill
   CG_TYPE 1 / 2: type the name / the short name   CG_TYPED: done typing   CG_LEAVE: back (Esc) */

void createview_init(void);
int createview_is_on(void);
void createview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
