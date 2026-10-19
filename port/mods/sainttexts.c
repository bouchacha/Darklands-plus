/* sainttexts.c - the "sainttexts" fix: saints whose help, in the game's code, plainly falls short
 * of or departs from their own description (DARKLAND.SNT) do what the description says.
 *
 * A saint's help is its function in s039 165c called with what 8 (mods/saintinfo.c tells the
 * whole story). For the saints below, that call is answered here instead: the same game routines
 * (raise_attr_temp, its twin for skills, heal_attr_pct, equip_bonuses, loc_reputation_adj, the
 * events), with what the text promises that the code left out or got wrong, judged one saint at a
 * time (docs/SAINTS.md says what changed and why). Anything else the function is asked (its table,
 * its chance) is the game's.
 *
 * New effects, as kind 0x4a events of the member prayed for (+0 the type, +0x1e the value; the
 * frames and hit hooks of mods/fixes.c read them):
 *   0x16e  armour quality, the armour worn (St. Crispin's with armourday; St. George, St. Michael)
 *   0x16f  armour quality, metal armour worn (scale, mail, plate: St. Dunstan)
 *   0x170  armour quality, other armour worn (padded, leather, cuir bouilli: St. Crispin)
 *   0x171  weapon quality, any weapon (St. George, St. Michael, St. Longinius)
 *   0x172  weapon quality, an edged weapon (St. Maurice)
 *   0x173  weapon quality: blades, hammers and maces, polearms, crossbows and guns (St. Eligius)
 *   0x174  fire protection (armourday's saints; St. Charity, St. Erasmus, St. Pantaleon, St. Margaret)
 *   0x175  armour thicker against missiles (St. Patrick)
 *   0x177  the party's travel faster, by that percent (St. Christopher, St. Drogo, St. Wilfrid)
 *   0x178  the same, in woods and forests (St. Eustace)
 * Armour, fire and weapon quality need armourday (its hit hook and frame): without it those saints
 * keep the game's help. Speed: the minutes of a step on the map (s030 1462:2ac8), for the party
 * when any member's runs; travel.c's plans count it too. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "fixes.h"
#include "nscreen.h"
#include "sainttexts.h"

int sainttexts_on;

/* ---------------------------------------------------------------- the game's routines */
static int rnd(int n) { return ns_call(GF_rand_mod, 1, n); }
static void attr(int who, int a, int lo, int span, int h) { int d = lo + (span > 0 ? rnd(span) : 0); ns_call(GF_raise_attr_temp, 4, who, a, d, h); }
static void attr_down(int who, int a, int lo, int span, int h) { int d = lo + (span > 0 ? rnd(span) : 0); ns_call(GF_raise_attr_temp, 4, who, a, -d, h); }
static void skill(int who, int k, int lo, int span, int h) { int d = lo + (span > 0 ? rnd(span) : 0); ns_call(GF_lower_skill_temp, 4, who, k, d, h); }
static void heal(int who, int a, int pct) { ns_call(GF_heal_attr_pct, 3, who, a, pct); }
static void lose(int who, int a, int lo, int span) { int d = lo + (span > 0 ? rnd(span) : 0); ns_call(GF_add_attr, 3, who, a, -d); }   /* (the cost: down to 1 at most) */
static void weapons(int who, int v) { ns_call(GF_equip_bonuses, 2, who, v); }
static void rep(int loc, int a, int b) { ns_call(GF_loc_reputation_adj, 3, loc, a, b); }
static int cur_loc(void) { return (s16)g_rw(G_CUR_LOC); }
static int here(void)                       /* the town the party is in, or the nearest (as the saints' code) */
{
    int l = cur_loc();
    return l == -1 ? ns_call(GF_nearest_city_impl, 2, g_rw(GV_party_x), g_rw(GV_party_y)) : l;
}
static int in_town(void) { int l = cur_loc(); return l >= 0 && l < 0x5c; }
static int present(int m) { return m >= 0 && m < 5 && g_rb(G_MEMBER(m)) != 0; }
static int armoured(int m) { return (s8)g_rb((u16)(G_MEMBER(m) + 0x37)) >= 0 || (s8)g_rb((u16)(G_MEMBER(m) + 0x38)) >= 0; }   /* (body or legs worn) */
static void record(int who, int type0, int v, int hours)
{
    ns_call(GF_event_new, 10, who, cur_loc(), 0x63, 0x4a, type0, v, 6, hours, 1, 0);
}
static int day(void) { return 19 + rand() % 3; }            /* (about a day, as the game's boosts) */
#define WEEK 168
static void each(void (*fn)(int m, int v), int v) { for (int m = 0; m < 5; m++) if (present(m)) fn(m, v); }
static void fire(int m, int v) { record(m, 0x174, v, day()); }
static void money_if_poor(void)             /* St. Alexis's: a party without florins and groschen gets 3..9 groschen */
{
    if (g_rw(G_MONEY) == 0 && g_rw((u16)(G_MONEY + 2)) == 0) g_ww((u16)(G_MONEY + 2), (u16)(rnd(7) + 3));
}
static void rep_in(const int *towns, int n, int a, int b)   /* the reputation where the party is, if it is one of these */
{
    int l = here();
    for (int i = 0; i < n; i++) if (towns[i] == l) { rep(l, a, b); return; }
}

/* ---------------------------------------------------------------- the saints */
static void christopher(int w)              /* travel speed; heals a little Strength, some Endurance */
{
    heal(w, 1, 10); heal(w, 0, 20);
    skill(w, 16, 10, 10, 0x18); skill(w, 18, 20, 20, 0x18); skill(w, 17, 25, 50, 0x18); skill(w, 5, 6, 6, 0x18);
    record(w, 0x177, 50, WEEK);
}
static void drogo(int w)                    /* the party's speed (not a week safe from ambush) */
{
    attr(w, 3, 6, 6, 0x18); skill(w, 13, 10, 10, 0x18); skill(w, 18, 20, 20, 0x18);
    record(w, 0x177, 50, WEEK);
}
static void wilfrid(int w)                  /* "may double party travel speed" */
{
    heal(w, 0, 20); skill(w, 14, 6, 6, 0x18); skill(w, 10, 20, 20, 0x18); attr(w, 5, 6, 6, 0x18);
    record(w, 0x177, 100, WEEK);
}
static void eustace(int w)                  /* travel through woods; every weapon skill but impact and flail */
{
    skill(w, 17, 10, 10, 0x18); skill(w, 18, 40, 30, 0x18);
    static const int K[5] = { 0, 3, 4, 5, 6 };
    for (int i = 0; i < 5; i++) skill(w, K[i], 15, 15, 0x18);
    record(w, 0x178, 100, WEEK);
}
static void nicholas(int w)                 /* Agility, Charisma, a tremendous Riding; reputation in Lorraine */
{
    static const int T[1] = { 0x59 };                       /* Nancy */
    rep_in(T, 1, 10, 0x14);
    attr(w, 2, 6, 6, 0x18); attr(w, 5, 6, 6, 0x18); skill(w, 17, 40, 40, 0x18);
}
static void theodore(int w)                 /* Charisma, the weapon skills, a very great Riding */
{
    weapons(w, rnd(10) + 15); skill(w, 17, 25, 25, 0x18); attr(w, 5, 6, 6, 0x18);
}
static void charity(int w)                  /* everyone's resistance to flame, the chosen one's greatly; a little Endurance */
{
    each(fire, 50); fire(w, 90); heal(w, 0, 10);
}
static void aquinas(int w)                  /* Intelligence, Latin, Read & Write, a very great Religion; reputation at monasteries and universities */
{
    skill(w, 11, 12, 15, 0x18); skill(w, 12, 15, 15, 0x18); skill(w, 8, 25, 25, 0x18); attr(w, 4, 12, 12, 0x18);
    if (in_town()) {                                       /* the town's own record (DARKLAND.CTY +0x5e: 0x80 a monastery, 0x100 a university) */
        u16 o = g_rw(GV_city_rec), sg = g_rw((u16)(GV_city_rec + 2));
        if (sg && (rw(sg, (u16)(o + 0x5e)) & 0x180)) rep(cur_loc(), 10, 0x14);
    }
}
static void francis(int w)                  /* Endurance, Perception, Charisma, Speak Common, a very great Virtue; may cost a tenth of the cash */
{
    attr(w, 0, 6, 6, 0x18); attr(w, 3, 6, 6, 0x18); attr(w, 5, 15, 15, 0x18); skill(w, 10, 10, 10, 0x18); skill(w, 9, 25, 25, 0x18);
    if (rnd(2)) {
        long pf = (long)g_rw(G_MONEY) * 240 + (long)g_rw((u16)(G_MONEY + 2)) * 12 + g_rw((u16)(G_MONEY + 4));
        pf -= pf / 10;
        g_ww(G_MONEY, (u16)(pf / 240)); g_ww((u16)(G_MONEY + 2), (u16)(pf % 240 / 12)); g_ww((u16)(G_MONEY + 4), (u16)(pf % 12));
    }
}
static void giles(int w)                    /* a little Streetwise, a tremendous Woodwise; money if needed */
{
    heal(w, 0, 40); heal(w, 1, 10);
    weapons(w, -(rnd(10) + 15)); attr_down(w, 2, 4, 4, 0x18);
    skill(w, 15, 30, 30, 0x18); skill(w, 18, 50, 40, 0x18); skill(w, 16, 5, 5, 0x18);
    money_if_poor();
}
static void stealth_up(int m, int v) { (void)v; skill(m, 15, 10, 10, 0x18); }
static void dismas(int w)                   /* Stealth for the whole party; Charisma and Speak Common too */
{
    skill(w, 18, 10, 10, 0x18); skill(w, 16, 20, 20, 0x18); skill(w, 14, 20, 20, 0x18); attr(w, 2, 10, 10, 0x18);
    each(stealth_up, 0);
    attr(w, 5, 6, 6, 0x18); skill(w, 10, 6, 6, 0x18);
}
static void apollinarius(int w) { heal(w, 1, 10); heal(w, 0, 30); }     /* heals, rather than boosts */
static void denis(int w) { heal(w, 1, 100); heal(w, 0, 10); skill(w, 10, 25, 25, 0x18); attr(w, 5, 15, 30, 0x18); }
static void lutgardis(int w)                /* her ecstasies cost Endurance */
{
    attr(w, 5, 6, 6, 0x18); attr(w, 3, 8, 8, 0x18); skill(w, 9, 6, 12, 0x18); lose(w, 0, 5, 5);
}
static void vitus(int w)                    /* the risk to lose some Endurance; the next step leads to the nearest town */
{
    attr(w, 5, 10, 10, 0x18); attr(w, 2, 12, 12, 0x18);
    ns_call(GF_event_here, 2, 0x5d, 1);
    if (rnd(2)) lose(w, 0, 3, 6);
}
static void james(int w)                    /* a small, temporary loss of Intelligence */
{
    heal(w, 0, 20); heal(w, 1, 10); skill(w, 9, 20, 15, 0x18); attr(w, 1, 4, 4, 0x18); attr(w, 5, 3, 3, 0x18); attr(w, 0, 4, 4, 0x18);
    attr_down(w, 4, 3, 3, 0x18);
}
static void margaret(int w)                 /* the flame resistance of one person's armour */
{
    if (!g_rb((u16)(G_MEMBER(w) + 0x03))) { heal(w, 0, 10); heal(w, 1, 10); }
    else {
        heal(w, 0, 100); heal(w, 1, 50);
        static const int A[6] = { 0, 5, 1, 3, 2, 4 }, K[10] = { 7, 8, 10, 11, 12, 13, 15, 14, 18, 16 };
        for (int i = 0; i < 6; i++) attr(w, A[i], 3, 4, 0x18);
        for (int i = 0; i < 10; i++) skill(w, K[i], 5, 6, 0x18);
    }
    fire(w, 90);
}
static void erasmus(int w) { attr(w, 2, 6, 6, 0x18); attr(w, 3, 6, 6, 0x18); each(fire, 50); }   /* the party's defence against fire */
static void pantaleon(int w)                /* armoured characters regain Strength and Endurance; fire hurts the party less */
{
    if (armoured(w)) { heal(w, 0, 30); heal(w, 1, 30); }
    skill(w, 7, 10, 10, 0x18); skill(w, 13, 15, 15, 0x18);
    each(fire, 50);
}
static void martin(int w)                   /* heals one who wears armour */
{
    if (armoured(w)) { heal(w, 0, 30); heal(w, 1, 30); }
    attr(w, 5, 8, 8, 0x18); attr(w, 3, 12, 8, 0x18); skill(w, 10, 25, 25, 0x18); skill(w, 17, 20, 20, 0x18);
}
static void patrick(int w)                  /* armour against missiles */
{
    static const int K[6] = { 0, 1, 3, 10, 11, 12 };
    for (int i = 0; i < 6; i++) skill(w, K[i], 15, 15, 0x18);
    attr(w, 5, 15, 10, 0x18); attr(w, 1, 12, 8, 0x18);
    record(w, 0x175, 2, day());
}
static void metal(int m, int v) { record(m, 0x16f, v, day()); }
static void dunstan(int w)                  /* the quality of the party's metal armour */
{
    weapons(w, rnd(6) + 6); attr(w, 3, 6, 6, 0x18); skill(w, 14, 25, 50, 0x18); skill(w, 16, 6, 6, 0x18); skill(w, 7, 5, 10, 0x18);
    each(metal, 10);
}
static void cloth(int m, int v) { record(m, 0x170, v, day()); }
static void crispin(int w) { (void)w; each(cloth, 10); }                 /* non-metal armour */
static void kit(int m, int v) { record(m, 0x171, v, day()); record(m, 0x16e, v, day()); }
static void george(int w)                   /* the quality of weapons and armour */
{
    attr(w, 1, 10, 5, 0x18); attr(w, 2, 5, 5, 0x18);
    static const int K[8] = { 0, 1, 2, 3, 4, 5, 6, 17 };
    for (int i = 0; i < 8; i++) skill(w, K[i], 25, 25, 0x18);
    kit(w, 10);
}
static void michael(int w)                  /* the party's weapons and armour */
{
    weapons(w, 20); skill(w, 13, 20, 20, 0x18); attr(w, 5, 12, 8, 0x18);
    rep(here(), 20, 40);
    each(kit, 10);
}
static void longinius(int w)                /* the quality of one character's weapons */
{
    for (int k = 0; k < 7; k++) skill(w, k, 10, 10, 0x18);
    record(w, 0x171, 10, day());
}
static void maurice(int w) { skill(w, 0, 20, 20, 0x18); skill(w, 7, 10, 10, 0x18); record(w, 0x172, 10, day()); }
static void eligius(int w) { skill(w, 14, 25, 50, 0x18); skill(w, 7, 8, 8, 0x18); record(w, 0x173, 10, day()); }
static void sebastian(int w)                /* a modest increase to the other weapon skills */
{
    heal(w, 0, 10); heal(w, 1, 10); skill(w, 5, 20, 20, 0x18); skill(w, 17, 10, 10, 0x18); attr(w, 2, 12, 8, 0x18);
    static const int K[6] = { 0, 1, 2, 3, 4, 6 };
    for (int i = 0; i < 6; i++) skill(w, K[i], 6, 6, 0x18);
}
static void hedwig(int w)                   /* also heals Strength and Endurance */
{
    static const int T[4] = { 0x25, 0x48, 0x49, 0x4b };    /* Silesia: Frankfurt an der Oder, Breslau, Olmütz, Teschen */
    rep_in(T, 4, 0x1e, 0x32);
    skill(w, 13, 20, 10, 0x18); heal(w, 0, 20); heal(w, 1, 20);
}
static void joseph(int w)                   /* Charisma only inside a city */
{
    skill(w, 14, 30, 30, 0x18); skill(w, 16, 30, 30, 0x18);
    if (in_town()) attr(w, 5, 6, 6, 0x18);
}
static void florian(int w)                  /* reputation in Poland too */
{
    static const int T[8] = { 0x43, 0x51, 0x52, 0x53, 0x54, 0x56, 0x2a, 0x57 };   /* Austria; Posen, Bromberg */
    rep_in(T, 8, 15, 0x19);
    weapons(w, rnd(6) + 6); skill(w, 17, 6, 6, 0x18);
    ns_call(GF_event_new, 10, -2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);   /* (water as land, for a day: the game's) */
}
static void stanislaus(int w)               /* reputation in Poland */
{
    static const int T[2] = { 0x2a, 0x57 };                 /* Posen, Bromberg */
    rep_in(T, 2, 10, 0x14);
    heal(w, 0, 20); heal(w, 1, 10); skill(w, 10, 30, 30, 0x18); skill(w, 11, 20, 20, 0x18); attr(w, 5, 6, 6, 0x18);
}
static void willibrord(int w)               /* reputation among the Frisians */
{
    static const int T[8] = { 0x00, 0x0d, 0x0e, 0x12, 0x13, 0x14, 0x15, 0x5b };   /* Groningen, Leer, Osnabrück, Zwolle, Deventer, Münster, Nymwegen, Elburg */
    rep_in(T, 8, 10, 0x14);
    skill(w, 18, 8, 8, 0x18); skill(w, 10, 20, 20, 0x18); attr(w, 5, 10, 10, 0x18);
}
static void hildegard(int w)                /* sometimes the reputation, by the Rhine */
{
    static const int T[10] = { 0x4c, 0x36, 0x35, 0x34, 0x31, 0x30, 0x1a, 0x19, 0x17, 0x16 };   /* Basel ... Xanten */
    rep_in(T, 10, 5, 15);
    attr(w, 3, 20, 40, 0x18);
}

/* the saints by their function (s039 165c), with whether their help needs armourday */
static const struct { int saint; u16 off; void (*fn)(int who); int arm; } SAINTS[] = {
    { 22, 0x13d4, christopher, 0 }, { 38, 0x1f8c, drogo, 0 }, { 130, 0x7038, wilfrid, 0 }, { 47, 0x27c2, eustace, 0 },
    { 99, 0x563a, nicholas, 0 }, { 123, 0x6ae8, theodore, 0 }, { 20, 0x125a, charity, 1 }, { 125, 0x6bb8, aquinas, 0 },
    { 52, 0x76d4, francis, 0 }, { 58, 0x307e, giles, 0 }, { 35, 0x1cf2, dismas, 0 }, { 12, 0x0b82, apollinarius, 0 },
    { 32, 0x1b02, denis, 0 }, { 89, 0x4c46, lutgardis, 0 }, { 128, 0x6e62, vitus, 0 }, { 73, 0x74ea, james, 0 },
    { 90, 0x4d1a, margaret, 1 }, { 45, 0x2628, erasmus, 1 }, { 103, 0x5a84, pantaleon, 1 }, { 93, 0x50cc, martin, 0 },
    { 104, 0x5b82, patrick, 1 }, { 39, 0x2052, dunstan, 1 }, { 28, 0x1868, crispin, 1 }, { 55, 0x2d58, george, 1 },
    { 96, 0x5334, michael, 1 }, { 86, 0x49e4, longinius, 1 }, { 95, 0x5296, maurice, 1 }, { 42, 0x23fe, eligius, 1 },
    { 117, 0x664a, sebastian, 0 }, { 64, 0x3612, hedwig, 0 }, { 79, 0x4320, joseph, 0 }, { 51, 0x2b30, florian, 0 },
    { 118, 0x671e, stanislaus, 0 }, { 133, 0x7288, willibrord, 0 }, { 68, 0x3934, hildegard, 0 },
};
#define NSAINTS (int)(sizeof SAINTS / sizeof SAINTS[0])

static int by_hook[NSAINTS];                 /* the hook point of each (runtime/hooks.c), to know which is called */

static void saint_hook(void)                 /* a saint's function (who, x, what) */
{
    int who = (s16)rw(SS, (u16)(SP + 4)), what = (s16)rw(SS, (u16)(SP + 8)), k = -1;
    for (int i = 0; i < NSAINTS; i++) if (by_hook[i] == hook_current) k = i;
    if (what != 8 || k < 0 || !present(who) || (SAINTS[k].arm && !fix_is_on("armourday"))) { hook_call_original(); return; }
    SAINTS[k].fn(who);
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix sainttexts: saint %d's help, as its text, for member %d\n", SAINTS[k].saint, who);
    AX = 0; DX = 0;
    hook_return_far(0);
}

int sainttexts_handles(int saint)
{
    if (!sainttexts_on) return 0;
    for (int i = 0; i < NSAINTS; i++) if (SAINTS[i].saint == saint) return !SAINTS[i].arm || fix_is_on("armourday");
    return 0;
}

/* ---------------------------------------------------------------- travel speed */
/* the largest running speed of the party's members (type 0x177, or 0x178 too in woods) */
static int speed_bonus(int woods)
{
    int best = 0;
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 p = g_event(i);
        if (!p) continue;
        u16 s = (u16)(p >> 16), o = (u16)p;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x4a) continue;
        int t0 = (s16)rw(s, o), v = (s16)rw(s, (u16)(o + 0x1e));
        if ((t0 == 0x177 || (woods && t0 == 0x178)) && v > best && v <= 300) {
            static const u16 now[4] = { G_CUR_HOUR, G_CUR_DAY, G_CUR_MONTH, G_CUR_YEAR };
            int running = 0;                                /* (+0x12 the end: hour, day, month, year) */
            for (int k = 3; k >= 0; k--) { int e = (s16)rw(s, (u16)(o + 0x12 + 2 * k)), n = (s16)g_rw(now[k]); if (e != n) { running = e > n; break; } }
            if (running) best = v;
        }
    }
    return best;
}
static int woods(int t) { return t == 6 || t == 7 || (t >= 12 && t <= 17); }   /* (woods, forests, pinewoods) */
int sainttexts_step_minutes(int terrain, int minutes)
{
    if (!sainttexts_on || game_mode() == GM_OTHER) return minutes;
    int b = speed_bonus(woods(terrain));
    if (!b) return minutes;
    int m = minutes * 100 / (100 + b);
    return m < 1 ? 1 : m;
}
static void step_hook(void)                 /* s030 1462:2ac8 (terrain): the minutes of one step on the map */
{
    int t = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    int m = sainttexts_step_minutes(t, (s16)AX);
    if (m != (s16)AX) { if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix sainttexts: a step on terrain %d takes %d minutes, not %d\n", t, m, (s16)AX); AX = (u16)m; }
}

/* ---------------------------------------------------------------- on and off */
int hook_point(int sect, unsigned seg, unsigned off);       /* (runtime/hooks.c: its index, hook_current while it runs) */
void sainttexts_enable(int on)
{
    sainttexts_on = on;
    for (int i = 0; i < NSAINTS; i++) {
        if (on) { hook_install(39, 0x165c, SAINTS[i].off, saint_hook); by_hook[i] = hook_point(39, 0x165c, SAINTS[i].off); }
        else hook_remove(39, 0x165c, SAINTS[i].off, saint_hook);
    }
    if (on) hook_install(30, 0x1462, 0x2ac8, step_hook); else hook_remove(30, 0x1462, 0x2ac8, step_hook);
}
