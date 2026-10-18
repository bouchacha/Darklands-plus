/* offices.h - whether a town's task givers have work to offer now, or when again (Go To shows it).
 *
 * From the cards' scripts. A giver (+0x1a of the task: 0 the merchants' stall, 3 the bishop, 4 the
 * foreign traders, 5 the pharmacists, 6 the Medici, 7 the Hanse, 8 the Fugger, 10 the city's lord)
 * gives a robber knight to hunt (kind 8, tag 3) or a thing to fetch (kind 8, tag 10), its +0x1e the
 * giver's town; a task done waits for its reward (kind 0x24, the giver, +0x1c the town). While one
 * runs or waits, the giver offers no other (but the lord). A refusal (and a reward paid) shuts the
 * door for 72 hours (kind 7, type0 the giver, the town); at the town hall and the fortress an
 * audience tried shuts it for a week (kind 0x3c: the chance then 1 in 100). Whether a task comes is
 * a roll each time (the game reseeds rand() every hour): known in advance is only the chance.
 *   Fugger ($fugge00 option 2): clamp(10, 50, rep + the company's standing here + fame), not below
 *     0 regard; half robber knights, half errands. Medici ($Medic00 option 2) alike. Hanse
 *     ($Hanse00 options 0 / 1): clamp(10, 50, rep / 2 + its liking / 2 + fame) or (rep + fame);
 *     three in ten robber knights.
 *   Cathedral ($Cathe00 option 4, by day; only where the town keeps a relic): clamp(1, 99, rep - 50).
 *   Town hall ($Counc00 option 0, a town with a council) / fortress ($CityF00, else): the speaker's
 *     (rep + Speak Common / 2 + Charisma) / 2, up to 70 at the hall; nothing below -40 regard.
 *   The stalls of the marketplace ($Marke00 options 0, 1, 2): about one visit in seven.
 * Host-independent (the UI only draws it). */
#ifndef DL_OFFICES_H
#define DL_OFFICES_H
#ifdef __cplusplus
extern "C" {
#endif

enum { OFS_NONE, OFS_ASK, OFS_SHUT, OFS_ON_TASK, OFS_REWARD, OFS_BARRED };
typedef struct {
    int state;                       /* OFS_* */
    int chance;                      /* OFS_ASK: percent that asking brings a task (-1: per visit, see note) */
    int hour, day, month, year;      /* OFS_SHUT: until (the end of the lock) */
    char note[200];                  /* what, and why, in words */
} OfficeStatus;

/* giver g (as above; 10 the lord: hall 1 at the town hall, 0 at the fortress) in town loc, now */
int office_status(int loc, int giver, int hall, OfficeStatus *out);

#ifdef __cplusplus
}
#endif
#endif
