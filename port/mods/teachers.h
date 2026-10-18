/* teachers.h - the teachers a town can offer, and the card options that bring them (assist.teachers).
 *
 * A teacher is hired for a week: the cards make it (event_new(-2, town, S, 0x28, skill, fee, 0,
 * 168 h, 0, 0)), the inn uses it ("Train or study", s036): each day a member pays the fee and
 * practice_skill (s039 1462) rolls rand(100) <= (S * 20 / 100) * 33 / 10 for +1. Nothing caps the
 * skill but 99 (the cards' "you cannot progress beyond our teacher's ability" is never shown).
 * Who brings which, from the cards' scripts (src/c, build/asm), with the town's seed
 * seed = city +0x56 + party seed:
 *   a guild's card by day, "request a meeting with the guild's masters" (option 2; when no dispute
 *   divides the guild): blacksmiths Edged and Impact, swordsmiths Edged, armorers Edged and
 *   Artifice, bowyers one of Bows / Thrown / Missile devices ((party seed + town) % 3), artificers
 *   Artifice; S = the guild's quality + seed % 15 - 5, fee 50;
 *   $Guild00 (a guild in a dispute), "inquire about special training" (option 0, a roll);
 *   $Unive00, "a teacher you can hire" (option 4, by day, a roll): Alchemy 90, Religion 70,
 *   Latin 50, Read & Write 50, Healing 110, S = seed % 15 + 45;
 *   $CityM00 (the monastery), "inquire about tutoring" (option 1, a roll): Religion 50, Latin 25,
 *   Read & Write 25, S = 50;
 *   $physi00, "beg him to allow you to be his students" (option 2, by day): Healing 60, S = 50,
 *   unless the party heals better than he does, or he takes nobody this year ((seed + year) % 3);
 *   $Alche00, "receiving additional instruction in alchemy" (option 4, by day, a roll): Alchemy,
 *   S = size * 3 + (seed + 4) % 41 (+ (seed + 9) % 11 + 10 with a university), fee S / 4 + 12.
 * Fees are pfennigs a member a day. Host-independent: the town's facts come in a TchTown. */
#ifndef DL_TEACHERS_H
#define DL_TEACHERS_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int size, buildings, seed;       /* DARKLAND.CTY +0x40, +0x5e (0x80 monastery, 0x100 university), +0x56 */
    int shopq[9];                    /* +0x62..: blacksmith, goods, swordsmith, armorer, gunsmith, bowyer, artificer, jeweler, clothmaker */
    int physician, alchemist;        /* the town has them (in this game) */
} TchTown;

enum { TCH_BLACKSMITHS, TCH_SWORDSMITHS, TCH_ARMORERS, TCH_BOWYERS, TCH_ARTIFICERS, TCH_GUILD,
       TCH_UNIVERSITY, TCH_MONASTERY, TCH_PHYSICIAN, TCH_ALCHEMIST };
#define TCH_MAXSK 5
typedef struct {
    int src;                         /* TCH_* */
    const char *who;                 /* "The blacksmiths' masters" */
    const char *ask;                 /* where and how they are asked */
    const char *cond;                /* what it takes, or when they will not ("" none) */
    int n, skill[TCH_MAXSK], fee[TCH_MAXSK];
    int strength;                    /* their ability: a lesson takes teach_chance(strength) % of days */
    char now[160];                   /* how it stands now, in the town the party is in ("" else) */
    int off;                         /* 1: not to be had now (the reason in now) */
} TchSource;

int teach_chance(int strength);                                     /* percent a day, for +1 */
int teach_town(int loc, const TchTown *t, TchSource *out, int max); /* what town loc can offer */
void teach_town_here(TchTown *t);                                   /* the current town, as the game holds it */
/* option opt of the card on show (its name, "$Black00" ...) brings teachers: 1 and *out */
int teach_card_option(const char *card, int opt, TchSource *out);

/* the teachers hired in town loc now (begun, not ended): skill, strength, fee, last day */
typedef struct { int skill, strength, fee, day, month, year; } TchHired;
int teach_hired(int loc, TchHired *out, int max);

#ifdef __cplusplus
}
#endif
#endif
