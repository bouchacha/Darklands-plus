/* lodgeview.h - staying at an inn or camping as a native screen (mods/lodgeview.c runs it,
 * ui/lodgeview.cpp draws it): each member's day, what it costs, the town's teachers and their
 * fees, and days spent until the party has rested. */
#ifndef DL_LODGEVIEW_H
#define DL_LODGEVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

#define LV_TEACHERS 8

typedef struct {
    int present;                      /* able, in the party */
    char name[24];                    /* game charset */
    int act, val;                     /* the day's activity (0 relax .. 6 train) and its number */
    char detail[24];                  /* the game's word for it (a job, "potions", "50pfs") */
    char line[64];                    /* the game's own line for it ("Regain Strength (gain 2)") */
    int str, str_max, df, df_max, end, end_max;
    int can[7];                       /* each activity possible (the original's bright or dim word) */
    int skill;                        /* training: the skill (-1 none) */
    int nbrew, brew_f[4], brew_n[4];  /* alchemy: the batches set (their formula and count; the game's line says only "Make 3 potions") */
} LodgeMember;

typedef struct {
    int open;
    int mode;                         /* 0 an inn, 1 camping in the wild, 2 and 3 other stays */
    int sel;                          /* the member whose day is being chosen */
    int days;                         /* days spent here */
    int risk;                         /* camping: the risk of being caught (of 10, as the game's line says) */
    int risk_pct, risk_next_pct;      /* camping: the chance of trouble before the next day, and before the one after (with the guards set) */
    int risk_next_unguarded_pct;      /* camping: before the day after, with nobody on guard */
    int risk_add, risk_add_unguarded; /* camping: what a day adds to the risk (of 98), with the guards set and with none */
    LodgeMember m[5];
    int nteach;
    struct { int skill, fee, val, days_left, used; } t[LV_TEACHERS];   /* (used: the teacher the inn uses for that skill, teach_best_here) */
    long purse;                       /* pfennigs */
    long cost;                        /* a day: lodging + fees - earnings (the game's sum) */
    long lodging;                     /* the lodging alone */
    int resting;                      /* someone regains strength or prays */
    char place[32];                   /* the town (game charset) */
    /* time passing (the game's hourglass, drawn here instead: the original's is under this page) */
    int passing;                      /* the hours of a day are going by */
    int pass_hour, pass_day, pass_month;   /* the time shown */
    int pass_step, pass_steps;        /* three-hour steps shown, of those to go */
    int pass_n, pass_of;              /* several days at once: this one, and how many (0: until rested) */
    unsigned seq;
} LodgeModel;

extern LodgeModel lodge_model;

enum { LV_SELECT = 1, LV_ACT, LV_TEACH, LV_DAY, LV_REST, LV_LEAVE, LV_SHEET, LV_DAYS };
/* LV_SELECT m; LV_ACT a (0 relax, 1 strength, 2 pray, 3 alchemy, 4 earn, 5 guard); LV_TEACH t
   (the t-th teacher); LV_DAY one day; LV_REST days until nobody regains strength or prays;
   LV_SHEET m the member's sheet; LV_DAYS n: n days (stops when something happens or a day cannot be paid) */

void lodgeview_init(void);
int lodgeview_is_on(void);
void lodgeview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
