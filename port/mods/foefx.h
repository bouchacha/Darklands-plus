/* foefx.h - what a thrown potion (or a trap's) is doing to each fighter now, and for how long (foefx.c) */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
enum { FX_SKILLS, FX_WEAKENED, FX_BLIND };
typedef struct {
    int what;        /* FX_SKILLS weapon skills lowered; FX_WEAKENED Agility, Perception and weapon skills lowered;
                        FX_BLIND blinded (parries only, cannot shoot or throw) */
    int potion;      /* the potion that did it (item 95 ...), -1 not known */
    int left;        /* ticks left of the battle's clock (about 60 a second) */
    int total;       /* ticks it lasts in all, 0 not known */
    int pct;         /* FX_SKILLS, FX_WEAKENED: how much lower, in hundredths of what it was */
} FoeFx;
int foefx_of(int i, FoeFx *out, int max);   /* combatant i (0-12): its effects now, how many */
#ifdef __cplusplus
}
#endif
