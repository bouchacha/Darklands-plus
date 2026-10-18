/* foesleft.h - the foes a place still holds that the party has not met yet (mods/foesleft.c) */
#pragma once
#define FL_FLOORS 16
typedef struct {
    int floor;                   /* the floor shown */
    int floors;                  /* floors known (0 .. floors-1) */
    int known[FL_FLOORS];        /* that floor's list is known */
    int foes[FL_FLOORS];         /* foes waiting on it, not yet met */
    int groups[FL_FLOORS];       /* in so many groups */
} FoesLeft;
int foesleft_on(void);
int foesleft_get(FoesLeft *out); /* 0 outside a battle */
void foesleft_init(void);
