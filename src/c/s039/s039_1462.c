/* place: s039 1462:0000 */
/* orig: size=1fa0 CONST=7ed6+0004 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (e2, 2026-10-09): 43/43 functions match (fmatch).
 *   Event-rules toolkit part 1: helpers used by the card scripts for timed events
 *   (F_2e38_1b2c, s029 2c4e/2e74/3b62), quests (11fc type 4, 15c2 type 12, 1712 type 10,
 *   1b0c type 1), location flags/standings (F_2e38_10be, s029 1a8e/3878/19d0), party skills
 *   and prices.
 *   026a: NOT a 6.00A limit after all: the current value (s029 0b64) is fetched into a
 *         temporary BEFORE the nested min/max chain (`m[0] = 0b64(..);`), the max (05ee) in
 *         a statement of its own.  The frame (enter 8, slot -6 unused, temp at -8) needs the
 *         temporary to be `int m[2]` using m[0].
 *   1712: locals declared `ter ev v k` (ter at -8, ev at -6).
 *   1c22: cases 0..2 get their own copy of the default tail AND the same code follows the
 *         switch (`if (rand(2)) return 0x50; return 0x67;` both times): 6.00A keeps the
 *         full 0..11 table and cross-jumping merges the two copies into one block that is
 *         both the table target and the range-check target.
 *   0132 matched with a 3-int aggregate local (roll, limit, unused third word) + pragma a.
 *   Numbers 9998/9999/8760 (0x270e/0x270f/0x2238) are event durations in hours, not strings.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct loc { int type; int f2; int x; int y; int f8; int fa; int fc; int fe; int f10; int rep;
             unsigned char f14; char f15, f16, f17; int f18; int f1a; int f1c; int f1e; int f20; };
extern struct loc far * far locations[];   /* locations (cities ...) */
struct event {                  /* timed events, events[300] (see s029 0e76) */
    int type0;                  /* +00 */
    int f2, f4, f6, f8;
    int hour0, day0, month0, year0;     /* +0a start */
    int hour1, day1, month1, year1;     /* +12 end */
    int f1a, f1c, f1e, f20;
    int kind;                   /* +22 */
    int f24, f26, f28, f2a, f2c, f2e;
};
extern struct event far * far events[];
extern int cur_month;
extern int cur_year;
struct date { int hour; int day; int month; int year; };
extern long D_10be[];
extern char D_270f[];
extern char game_difficulty;
extern char leader;
struct member {                 /* party member, 0x80 bytes */
    char status;                /* 1 = able */
    char pad[0x7f];
};
extern struct member party[];
extern int cur_loc;
extern int party_size;
extern int var_loc;
int far rand_mod();
int far party_panel_rows();
int far loc_reputation();
int far member_attr();
int far member_skill();
int far member_best_skill();
int far party_best_skill_hi();
int far party_avg_skill();
int far improve_roll();
int far add_skill();
int far add_attr();
int far member_attr_max();
int far party_get();
int far party_set();
int far loc_reputation_adj();
int far loc_query();
int far event_here();
int far event_new();
int far event_update();
int far ev8_exists();
int far ev8_at();
int far loc_fact();
int far loc_fact_value_any();
int far loc_fact_value();
int far event_at();
int far event_find();
int far ev8_resolve();
int far clamp_range();
int far imax();
int far imin();
int far hours_until();
int far date_set();
int far month_of_yday();
int far mday_of_yday();
int far days_until();
int far vec_1f13();
int far nearest_loc_type();
int far nearest_city();
int far add_item();
int far sub_s039_1462_0000();
int far loc_hostile_impl();
int far sub_s039_1462_0102();
void far practice_skill_impl();
void far injure_impl();
int far party_lore_level_impl();
int far sub_s039_1462_0504();
void far sub_s039_1462_0558();
int far loc_side_get_impl();
void far loc_side_set_impl();
int far sub_s039_1462_0b12();
int far side2_strength_impl();
int far side1_strength_impl();
int far side1_percent_impl();
int far side2_percent_impl();
void far conflict_outcome_impl();
int far sub_s039_1462_10b6();
void far sub_s039_1462_11fc();
int far sub_s039_1462_1510();
int far sub_s039_1462_153e();
int far sub_s039_1462_156c();
void far sub_s039_1462_159a();
void far sub_s039_1462_15c2();
int far sub_s039_1462_16b4();
int far sub_s039_1462_16e2();
void far sub_s039_1462_1710();
int far sub_s039_1462_1712();
int far sub_s039_1462_1a38();
int far sub_s039_1462_1a50();
int far event_loc_of_impl();
int far event_f2e_of_impl();
int far event_type_of_impl();
void far sub_s039_1462_1ab0();
void far sub_s039_1462_1b0c();
int far sub_s039_1462_1c22();
void far sub_s039_1462_1cd2();
void far sub_s039_1462_1ce8();
int far sub_s039_1462_1cfe();
int far sub_s039_1462_1d2c();
int far party_present_count_impl();
int far give_reward_impl();
int far sub_s039_1462_1e90();
void far sub_s039_1462_1f00();

/* a price/amount v adjusted for the current location (cur_loc): +25% or +50% depending on
 * its kind (s029 1a8e(loc, 0x20) = 1 or 2), +20% each for the location flags 0x13 and 0x12
 * (s029 3878); the result is a random value between lo and v (s029 1367 0066 clamps). */
#pragma optimize("a", on)
int far sub_s039_1462_0000(register int v, int unused, int lo)
{
    register int k;

    k = loc_query(cur_loc, 0x20);
    if (k == 1)
        v = v * 5 / 4;
    if (k == 2)
        v = v * 6 / 4;
    if (event_at(0x13, cur_loc))
        v = v * 6 / 5;
    if (event_at(0x12, cur_loc))
        v = v * 6 / 5;
    return clamp_range(lo, v, rand_mod(v - lo));
}
#pragma optimize("", on)

/* 1 when the current location (cur_loc) is hostile: its reputation (s029 199c) is -10 or
 * lower when flag 0x13 is set, -75 or lower otherwise, or flag 0x11 (s029 3878) is set. */
int far loc_hostile_impl(void)
{
    register int rep;

    rep = loc_reputation(cur_loc);
    if (!event_at(0x11, cur_loc) && rep > -0x4b &&
        (!event_at(cur_loc, 0x13) || rep > -0xa))
        return 0;
    return 1;
}

/* location flags 0x12 (+2) and 0x13 (+1) of the current location (s029 3878) */
int far sub_s039_1462_0102(void)
{
    int v_si;
    v_si = 0;
    if (event_at(0x12, cur_loc)) {
        v_si = 2;
    }
    if (event_at(0x13, cur_loc)) {
        v_si++;
    }
    return v_si;
}

/* skill practice: member who (-1 = the active member leader, -2 = every able member, each
 * rolling separately) improves skill k (s029 01a0/02ea, the 7 bytes at +0x57) with a chance
 * of pct * 1.5 % (mode 1) or pct * 0.5 % (-2) / pct * 3.3 % or pct * 1.1 % (one member),
 * by s029 18a8(skill, lvl + 1, (lvl + 2) * 2), clamped to 1..99. */
#pragma optimize("a", on)
void far practice_skill_impl(int who, int k, int mode, int pct, int lvl)
{
    int i;
    int u;
    struct { int a; int b; int c; } t;   /* roll, limit, unused: the slot layout -8/-6/-4 */

    if (who == -1)
        who = leader;
    if (who == -2) {
        t.b = (mode == 1 ? pct * 15 : pct * 5) / 10;
        for (i = 0; i < party_size; i++)
            if (party[i].status == 1 && rand_mod(100) <= t.b)
                add_skill(i, k, clamp_range(1, 0x63,
                    improve_roll(member_skill(i, k), lvl + 1, (lvl + 2) * 2)));
    } else {
        *(volatile int *)&t.a = rand_mod(100);
        if ((t.b = (mode == 1 ? pct * 33 : pct * 11) / 10) >= t.a) {
            u = improve_roll(member_skill(who, k), lvl + 1, (lvl + 2) * 2);
            u = clamp_range(1, 0x63, u);
            add_skill(who, k, u);
        }
    }
}
#pragma optimize("", on)

/* damage member who (-2 = every present member): attribute 1 (s029 05ee max, 0b64
 * current) loses d = rand(mult * max1 / 40 + 1) - 1, at least a, at most the current value;
 * attribute 0 loses rand(mult * max0 / 20 + 1) - 1, at least d and b, at most its current
 * value (s029 0a72).  Then redraws (root 1716). */
void far injure_impl(int who, int a, int b, int mult)
{
    int i;
    register int d;
    register int s;
    int m[2];

    if (who == -2) {
        for (i = 0; i < party_size; i++)
            if (party[i].status) {
                d = member_attr(i, 1);
                m[0] = member_attr_max(i, 1);
                d = imin(d = imax(d = rand_mod((d = mult * d / 40) + 1) - 1, a), m[0]);
                add_attr(i, 1, -d);
                s = member_attr(i, 0);
                m[0] = member_attr_max(i, 0);
                s = imin(s = imax(s = imax(s = rand_mod((s = mult * s / 20) + 1) - 1, d), b), m[0]);
                s = -s;
                add_attr(i, 0, s);
            }
    } else {
        d = member_attr(who, 1);
        m[0] = member_attr_max(who, 1);
        d = imin(d = imax(d = rand_mod((d = mult * d / 40) + 1) - 1, a), m[0]);
        add_attr(who, 1, -d);
        s = member_attr(who, 0);
        m[0] = member_attr_max(who, 0);
        s = imin(s = imax(s = imax(s = rand_mod((s = mult * s / 20) + 1) - 1, d), b), m[0]);
        s = -s;
        add_attr(who, 0, s);
    }
    party_panel_rows(0x100);
}

/* party lore level 1..10: (average of the members' best skill (s029 01c0) + the best of
 * skills 4-6 (s029 026a) + the party averages of skills 9 and 7 (s029 1600)) / 37,
 * -2 for a party of one, -1 for two, +1 for five. */
int far party_lore_level_impl(void)
{
    int sum;
    int n;
    register int i;
    register int v;

    n = sum = 0;
    for (i = 0; i < 5; i++)
        if (party[i].status) {
            sum += member_best_skill(i);
            n++;
        }
    if (n == 0)
        n = 1;
    v = party_best_skill_hi() + sum / n;
    v += party_avg_skill(9);
    v += party_avg_skill(7);
    v /= 37;
    if (n == 1)
        v -= 2;
    else if (n == 2)
        v--;
    else if (n == 5)
        v++;
    return clamp_range(1, 10, v);
}

/* a random location (index 0x5c..0x19d of locations, starting at a random one) of the
 * given type */
int far sub_s039_1462_0504(register int type)
{
    register int i;

    i = rand_mod(0x141) + 0x5c;
    while (locations[i]->type != type)
        if (++i >= 0x19e)
            i = 0x5c;
    return i;
}

/* set up location loc (a city: s029 1a8e(loc, 0x21) and (loc, 0) refresh it) unless event
 * kind 2 (s029 3470) holds or its kind (1a8e 0x20) is 2: timed events of kind 0x5f/0x5a
 * (s029 2c4e: 56 days, 21 days, 35 days, until 5 o'clock), one random weekday (a day d with
 * 1a8e(loc, d + 3) set: bit 1 << d of value 0x17), and for each of the 15 bits a coin flip
 * between kind-0x1c groups 1 and 2 (s029 2e74 op 3 = or). */
void far sub_s039_1462_0558(register int loc)
{
    register int d;

    loc_query(loc, 0x21);
    loc_query(loc, 0);
    if (ev8_at(2, loc) != 1 && loc_query(loc, 0x20) != 2) {
        event_new(-2, loc, 0x5f, 8, 0, loc, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 0xc, 0x63, 2, hours_until(5), 1, 2);
        event_new(-2, loc, 0x5a, 0x1c, 6, 0x63, 0, 0x1f8, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 7, 0x63, 0, 0x348, 0, 2);
        d = rand_mod(7);
        while (!loc_query(loc, d + 3)) {
            d++;
            d %= 7;
        }
        event_new(-2, loc, 0x5a, 0x1c, 0x17, 1 << (char)d, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 0x14, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 3, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 5, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 4, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 9, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 0xa, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 0x23, rand_mod(0x64), 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 1, 0, 0, 0x540, 0, 2);
        event_new(-2, loc, 0x5a, 0x1c, 2, 0, 0, 0x540, 0, 2);
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 1, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 1, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 2, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 2, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 4, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 4, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 8, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 8, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x10, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x10, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x20, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x20, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x80, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x80, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x40, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x40, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x200, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x200, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x400, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x400, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x800, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x800, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x1000, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x1000, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x2000, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x2000, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, 0x4000, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, 0x4000, 2, 3);
        }
        if (rand_mod(2)) {
            event_update(-2, loc, 0x1c, 1, -0x8000, 2, 3);
        } else {
            event_update(-2, loc, 0x1c, 2, -0x8000, 2, 3);
        }
    }
}

/* three-state flag `bit` of the current location: -1 when set in kind-0x1c group 1,
 * 1 in group 2 (s029 3742), else 0 (0a7e sets it) */
int far loc_side_get_impl(int p_6)
{
    int v_di;
    int v_si;
    v_di = loc_fact_value(2, 1, cur_loc);
    v_si = loc_fact_value(2, 2, cur_loc);
    v_si &= p_6;
    if (p_6 & v_di) {
        return -1;
    }
    if (v_si) {
        return 1;
    }
    return 0;
}

/* set the three-state flag `bit` of the current location (cur_loc; 0a34 reads it) in the
 * kind-0x1c events: mode -1 -> group 1, 1 -> group 2, else neither (s029 2e74: op 2 = and,
 * op 3 = or). */
void far loc_side_set_impl(register int bit, int mode)
{
    if (mode == -1) {
        event_update(-2, cur_loc, 0x1c, 2, ~bit, 2, 2);
        event_update(-2, cur_loc, 0x1c, 1, bit, 2, 3);
    } else if (mode == 1) {
        event_update(-2, cur_loc, 0x1c, 1, ~bit, 2, 2);
        event_update(-2, cur_loc, 0x1c, 2, bit, 2, 3);
    } else {
        bit = ~bit;
        event_update(-2, cur_loc, 0x1c, 1, bit, 2, 2);
        event_update(-2, cur_loc, 0x1c, 2, bit, 2, 2);
    }
}

/* number of the 15 location flags (0a34) that are neutral (0) */
int far sub_s039_1462_0b12(void)
{
    int v_si;
    v_si = 0;
    if (!loc_side_get_impl(1)) {
        v_si = 1;
    }
    if (!loc_side_get_impl(2)) {
        v_si++;
    }
    if (!loc_side_get_impl(4)) {
        v_si++;
    }
    if (!loc_side_get_impl(8)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x10)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x20)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x80)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x40)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x200)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x400)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x800)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x1000)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x2000)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x4000)) {
        v_si++;
    }
    if (!loc_side_get_impl(-0x8000)) {
        v_si++;
    }
    return v_si;
}

/* strength of group 2: 2 per flag in group 2 + 1 per neutral flag (11 flags), doubled;
 * 1 when zero */
int far side2_strength_impl(void)
{
    int v_si;
    v_si = 0;
    if (loc_side_get_impl(1) == 1) {
        v_si = 2;
    }
    if (loc_side_get_impl(2) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(4) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(8) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x10) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x20) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x80) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x40) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x200) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x400) == 1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x1000) == 1) {
        v_si++;
        v_si++;
    }
    if (!loc_side_get_impl(1)) {
        v_si++;
    }
    if (!loc_side_get_impl(2)) {
        v_si++;
    }
    if (!loc_side_get_impl(4)) {
        v_si++;
    }
    if (!loc_side_get_impl(8)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x10)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x20)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x80)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x40)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x200)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x400)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x1000)) {
        v_si++;
    }
    if (v_si) {
        return v_si << 1;
    }
    return 1;
}

/* the same for group 1 (flags == -1) */
int far side1_strength_impl(void)
{
    int v_si;
    v_si = 0;
    if (loc_side_get_impl(1) == -1) {
        v_si = 2;
    }
    if (loc_side_get_impl(2) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(4) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(8) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x10) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x20) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x80) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x40) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x200) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x400) == -1) {
        v_si++;
        v_si++;
    }
    if (loc_side_get_impl(0x1000) == -1) {
        v_si++;
        v_si++;
    }
    if (!loc_side_get_impl(1)) {
        v_si++;
    }
    if (!loc_side_get_impl(2)) {
        v_si++;
    }
    if (!loc_side_get_impl(4)) {
        v_si++;
    }
    if (!loc_side_get_impl(8)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x10)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x20)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x80)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x40)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x200)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x400)) {
        v_si++;
    }
    if (!loc_side_get_impl(0x1000)) {
        v_si++;
    }
    if (v_si) {
        return v_si << 1;
    }
    return 1;
}

/* group 1 strength in percent of group 2 */
int far side1_percent_impl(void)
{
    int v_si;
    v_si = side2_strength_impl();
    return side1_strength_impl() * 0x64 / v_si;
}

/* group 2 strength in percent of group 1 */
int far side2_percent_impl(void)
{
    int v_si;
    v_si = side1_strength_impl();
    return side2_strength_impl() * 0x64 / v_si;
}

/* outcome of a fight/conflict at the current location: side a and c (+1/-1), b the result.
 * With c == 0 a chance from the strength ratio (0eaa) is computed (unused) and a random
 * number drawn.  Then time passes (s029 2930), event kind 2 of the location is cleared
 * (s029 3e06), the location gets flag 0x80 (+0x14), and the location's two standings are
 * changed by s029 19d0 depending on a/b/c and whether events 0x24/0x25 (s029 360c) hold;
 * winning without them pays a reward (1e06(3) or 1e06(1)). */
void far conflict_outcome_impl(int a, int b, register int c)
{
    register int pct;

    if (c == 0) {
        if (side2_percent_impl() >= 200)
            pct = 0;
        else if (side2_percent_impl() >= 150)
            pct = 15;
        else if (side2_percent_impl() >= 120)
            pct = 35;
        else if (side2_percent_impl() <= 50)
            pct = 100;
        else if (side2_percent_impl() <= 67)
            pct = 85;
        else if (side2_percent_impl() <= 83)
            pct = 65;
        else
            pct = 50;
        if (rand_mod(100) < pct)
            pct = 0;
        else
            pct = 0;
    }
    event_here(0x13, 0x2a0);
    ev8_resolve(2, cur_loc);
    locations[cur_loc]->f14 |= 0x80;
    if (c == -1 && a == -1) {
        if (b == 1) {
            if (loc_fact(2, 0x24, cur_loc))
                loc_reputation_adj(cur_loc, -10, -30);
            else
                loc_reputation_adj(cur_loc, -30, -60);
        } else if (b == -1) {
            if (loc_fact(2, 0x25, cur_loc))
                loc_reputation_adj(cur_loc, -30, -70);
            else {
                give_reward_impl(3);
                loc_reputation_adj(cur_loc, 25, 70);
            }
        }
    } else if (c == 1 && a == 1) {
        if (b == -1) {
            if (loc_fact(2, 0x25, cur_loc))
                loc_reputation_adj(cur_loc, -10, -30);
            else
                loc_reputation_adj(cur_loc, -30, -60);
        } else if (b == 1) {
            if (loc_fact(2, 0x24, cur_loc))
                loc_reputation_adj(cur_loc, -30, -70);
            else {
                give_reward_impl(3);
                loc_reputation_adj(cur_loc, 25, 70);
            }
        }
    } else if (c == 1 && a == -1) {
        event_here(0x46, 0xa8);
        if (b == 1) {
            if (loc_fact(2, 0x24, cur_loc))
                loc_reputation_adj(cur_loc, -10, -30);
            else {
                give_reward_impl(1);
                loc_reputation_adj(cur_loc, 15, 25);
            }
        } else if (b == -1) {
            if (loc_fact(2, 0x25, cur_loc))
                loc_reputation_adj(cur_loc, -20, -60);
            else
                loc_reputation_adj(cur_loc, -15, -35);
        }
    } else if (c == -1 && a == 1) {
        event_here(0x45, 0xa8);
        if (b == -1) {
            if (loc_fact(2, 0x25, cur_loc))
                loc_reputation_adj(cur_loc, -10, -30);
            else {
                give_reward_impl(1);
                loc_reputation_adj(cur_loc, 15, 25);
            }
        } else if (b == 1) {
            if (loc_fact(2, 0x24, cur_loc))
                loc_reputation_adj(cur_loc, -20, -60);
            else
                loc_reputation_adj(cur_loc, -15, -35);
        }
    }
}

#pragma optimize("a", on)
int far sub_s039_1462_10b6(int who, int b, int c, int v, int lvl, int x)
{
    register int i;
    struct event far *ev;

    if (lvl >= 4)
        lvl = 4;
    i = event_find(8, -1, b, -1, 3, -1);
    if (i != -1) {
        event_new(who, b, 0x5f, 8, c, cur_loc, 0, 9998, 0, 3);
        if (events[i]->f2a < v)
            events[i]->f2a = v;
        if (events[i]->f2c < lvl)
            events[i]->f2c = lvl;
        return 1;
    }
    ev = events[event_new(who, b, 0x5f, 8, c, cur_loc, 0, 9998, 0, 3)];
    ev->f2a = v;
    ev->f2c = lvl;
    ev->f2e = x;
    event_new(-2, var_loc, 0, 0x1c, 2, 0, 0, 9998, 0, 3);
    event_new(-2, var_loc, 0, 0x1c, 3, 0, 0, 9998, 0, 3);
    event_new(-2, var_loc, 0, 0x1c, 4, 0, 0, 9998, 0, 3);
    event_new(-2, var_loc, 0, 0x2b, 0, 1, 0, 9999, 0, 0);
}
#pragma optimize("", on)

/* start the type-4 quest unless one is active (s029 3b62 kind 8 type 4): a location di near
 * the current one (nearest_loc_type; anywhere when cur_loc is -1) gets a random site type (r = rand 4:
 * 0x9d/0x9b/0x9e/0x9c), a kind-8 event (type 0x5f, 8760 hours, f28 r, f2a 30..49, f2c 4,
 * f2e 0..15) and kind-0x2a/0x2b/0x1c events; every type-8 location within 40/80 of it becomes
 * type 9 with site 0x88 and flag 0x20 (the first one site 0x9d when r == 0).  Without such a
 * location and r == 0 the event's type0 becomes 1..3. */
void far sub_s039_1462_11fc(void)
{
    register int loc;
    register int ev;
    int r;
    int n;
    int i;

    if (event_find(8, -1, -1, -1, 4, -1) != -1)
        return;
    if (cur_loc == -1)
        loc = nearest_loc_type(rand_mod(300), rand_mod(900), 15);
    else
        loc = nearest_loc_type(locations[cur_loc]->x, locations[cur_loc]->y, 15);
    switch (r = rand_mod(4)) {
    case 0:
        locations[loc]->fc = 0x9d;
        break;
    case 1:
        locations[loc]->fc = 0x9b;
        break;
    case 2:
        locations[loc]->fc = 0x9e;
        break;
    default:
        locations[loc]->fc = 0x9c;
        break;
    }
    ev = event_new(-2, loc, 0x5f, 8, r,
        nearest_city(locations[loc]->x, locations[loc]->y), 0, 8760, 0, 4);
    events[ev]->f2a = rand_mod(20) + 30;
    events[ev]->f2c = 4;
    events[ev]->f2e = rand_mod(16);
    event_new(-2, loc, 0, 0x2a, 0, 10, 0, 8760, 0, 0);
    event_new(-2, loc, 0, 0x2b, 0, 2, 0, 9999, 0, 0);
    event_new(-2, loc, 0, 0x1c, 1, 0, 0, 8760, 0, 4);
    event_new(-2, loc, 0, 0x1c, 3, 0, 0, 8760, 0, 4);
    event_new(-2, loc, 0, 0x1c, 4, 0, 0, 8760, 0, 4);
    event_new(-2, loc, 0, 0x1c, 7, 0, 0, 8760, 0, 4);
    event_new(-2, loc, 0, 0x1c, 6, 0, 0, 8760, 0, 4);
    n = 0;
    for (i = 0x5c; i < 0x19e; i++)
        if (locations[loc]->x - 40 <= locations[i]->x &&
            locations[loc]->x + 40 > locations[i]->x &&
            locations[loc]->y - 80 <= locations[i]->y &&
            locations[loc]->y + 80 > locations[i]->y &&
            locations[i]->type == 8) {
            if (n++ == 0 && r == 0)
                locations[i]->fc = 0x9d;
            else {
                locations[i]->fc = 0x88;
                locations[i]->f14 |= 0x20;
                locations[i]->type = 9;
            }
        }
    if (n == 0 && r == 0)
        events[ev]->type0 = rand_mod(3) + 1;
}

/* the active kind-8 event with type 4 (s029 3b62; the quest of 11fc): its f2e, f0, f1c
 * (each returns -1 when there is none) */
int far sub_s039_1462_1510(void)
{
    register int i;

    if ((i = event_find(8, -1, -1, -1, 4, -1)) != -1)
        return events[i]->f2e;
}

int far sub_s039_1462_153e(void)
{
    register int i;

    if ((i = event_find(8, -1, -1, -1, 4, -1)) != -1)
        return events[i]->type0;
}

int far sub_s039_1462_156c(void)
{
    register int i;

    if ((i = event_find(8, -1, -1, -1, 4, -1)) != -1)
        return events[i]->f1c;
}

/* the quest location (156c) gets type 0x62 (+0xc) and its event kind 4 is cleared */
void far sub_s039_1462_159a(void)
{
    locations[sub_s039_1462_156c()]->fc = 0x62;
    ev8_resolve(4, cur_loc);
}

/* start the type-12 quest unless one is active: a location near the current one (nearest_loc_type;
 * anywhere east when cur_loc is -1) gets site type 0x2d and a kind-8 event (type 0x5f, 8760
 * hours, f28 rand 3, f2a 40..49, f2c 3, f2e 0..37) */
void far sub_s039_1462_15c2(void)
{
    register int loc;
    register int ev;
    int r;

    if (event_find(8, -1, -1, -1, 12, -1) != -1)
        return;
    if (cur_loc == -1)
        loc = nearest_loc_type(rand_mod(300), rand_mod(600) + 300, 6);
    else
        loc = nearest_loc_type(locations[cur_loc]->x, locations[cur_loc]->y, 6);
    r = rand_mod(3);
    locations[loc]->fc = 0x2d;
    ev = event_new(-2, loc, 0x5f, 8, r, 0x63, 0, 8760, 0, 12);
    events[ev]->f2a = rand_mod(10) + 40;
    events[ev]->f2c = 3;
    events[ev]->f2e = rand_mod(0x26);
}

/* the same for the kind-8 event with type 12: f0, f2e */
int far sub_s039_1462_16b4(void)
{
    register int i;

    if ((i = event_find(8, -1, -1, -1, 12, -1)) != -1)
        return events[i]->type0;
}

int far sub_s039_1462_16e2(void)
{
    register int i;

    if ((i = event_find(8, -1, -1, -1, 12, -1)) != -1)
        return events[i]->f2e;
}

/* empty */
void far sub_s039_1462_1710(void)
{
}

/* start a kind-8 event of type 10 (9999 hours) for region kind t (0..10; 1, 2, 9 and others
 * leave k/v uninitialised): a random k (0..11) with its value v (0x78..0x8a) by t, then a
 * terrain from k (0x10/5/0x11, 0x12, 0xd/0/0x12/0x13, ...); var_loc becomes a location of that
 * terrain near the current one (vec_1f13); the event (f28 k, f2a 10..19, f2c 2, f2e v) is
 * returned. */
int far sub_s039_1462_1712(int t)
{
    int ter;
    int ev;
    int v;
    int k;

    switch (t) {
    case 3:
        switch (rand_mod(5)) {
        case 0: k = 0; v = 0x81; break;
        case 1: k = 1; v = 0x82; break;
        case 2: k = 2; v = 0x83; break;
        case 3: k = 3; v = 0x84; break;
        default: k = 6; v = 0x7f; break;
        }
        break;
    case 7:
        switch (rand_mod(10)) {
        case 0: k = 0; v = 0x81; break;
        case 1: k = 1; v = 0x82; break;
        case 2: k = 3; v = 0x84; break;
        case 3: k = 4; v = 0x85; break;
        case 4: k = 6; v = 0x7f; break;
        case 5: k = 7; v = 0x87; break;
        case 6: k = 8; v = 0x88; break;
        case 7: k = 9; v = 0x89; break;
        case 8: k = 10; v = 0x8a; break;
        default: k = 11; v = 0x78; break;
        }
        break;
    case 6:
    case 8:
        switch (rand_mod(9)) {
        case 0: k = 0; v = 0x81; break;
        case 1: k = 1; v = 0x82; break;
        case 2: k = 3; v = 0x84; break;
        case 3: k = 4; v = 0x85; break;
        case 4: k = 7; v = 0x87; break;
        case 5: k = 8; v = 0x88; break;
        case 6: k = 9; v = 0x89; break;
        case 7: k = 10; v = 0x8a; break;
        default: k = 11; v = 0x78; break;
        }
        break;
    case 10:
        switch (rand_mod(8)) {
        case 0: k = 0; v = 0x81; break;
        case 1: k = 1; v = 0x82; break;
        case 2: k = 3; v = 0x84; break;
        case 3: k = 4; v = 0x85; break;
        case 4: k = 6; v = 0x7f; break;
        case 5: k = 7; v = 0x87; break;
        case 6: k = 8; v = 0x88; break;
        default: k = 9; v = 0x89; break;
        }
        break;
    case 0:
    case 4:
        switch (rand_mod(7)) {
        case 0: k = 0; v = 0x81; break;
        case 1: k = 1; v = 0x82; break;
        case 2: k = 3; v = 0x84; break;
        case 3: k = 4; v = 0x85; break;
        case 4: k = 7; v = 0x87; break;
        case 5: k = 8; v = 0x88; break;
        default: k = 9; v = 0x89; break;
        }
        break;
    case 5:
        switch (rand_mod(6)) {
        case 0: k = 3; v = 0x84; break;
        case 1: k = 4; v = 0x85; break;
        case 2: k = 5; v = 0x86; break;
        case 3: k = 7; v = 0x87; break;
        case 4: k = 8; v = 0x88; break;
        default: k = 9; v = 0x89; break;
        }
        break;
    }
    switch (k) {
    case 0:
    case 1:
        switch (rand_mod(3)) {
        case 0: ter = 0x10; break;
        case 1: ter = 5; break;
        case 2: ter = 0x11; break;
        }
        break;
    case 2:
        ter = 0x12;
        break;
    case 3:
        switch (rand_mod(4)) {
        case 0: ter = 0xd; break;
        case 1: ter = 0; break;
        case 2: ter = 0x12; break;
        case 3: ter = 0x13; break;
        }
        break;
    case 4:
        if (rand_mod(2))
            ter = 0xd;
        else
            ter = 0x14;
        break;
    case 5:
        ter = 0x13;
        break;
    case 6:
    case 9:
    case 10:
    case 11:
        ter = 0;
        break;
    case 7:
        switch (rand_mod(6)) {
        case 0: ter = 0x10; break;
        case 1: ter = 0x11; break;
        case 2: ter = 5; break;
        case 3: ter = 0x12; break;
        case 4: ter = 0x13; break;
        case 5: ter = 0xd; break;
        }
        break;
    case 8:
        switch (rand_mod(3)) {
        case 0: ter = 0xd; break;
        case 1: ter = 0x14; break;
        case 2: ter = 0x13; break;
        }
        break;
    }
    var_loc = vec_1f13(locations[cur_loc]->x, locations[cur_loc]->y, ter);
    ev = event_new(t, var_loc, 0x5f, 8, k, cur_loc, 0, 9999, 0, 10);
    events[ev]->f2a = rand_mod(10) + 10;
    events[ev]->f2c = 2;
    events[ev]->f2e = v;
    return ev;
}

/* fields of timed event p_6 (events): f1a, f1e, f1c, f2e, type0 */
int far sub_s039_1462_1a38(int p_6)
{
    return events[p_6]->f1a;
}

int far sub_s039_1462_1a50(int p_6)
{
    return events[p_6]->f1e;
}

int far event_loc_of_impl(int p_6)
{
    return events[p_6]->f1c;
}

int far event_f2e_of_impl(int p_6)
{
    return events[p_6]->f2e;
}

int far event_type_of_impl(int p_6)
{
    return events[p_6]->type0;
}

/* when event kind 10 of the current location holds (s029 3470): the kind-8 event of type 10
 * with f1c b gives its f2e to the active member (add_item) and the kind-10 event of b ends */
void far sub_s039_1462_1ab0(int b)
{
    register int i;

    if (ev8_at(10, cur_loc)) {
        i = event_find(8, -1, b, -1, 10, -1);
        add_item(leader, events[i]->f2e);
        ev8_resolve(10, b);
    }
}

/* start the type-1 event unless one is active: a kind-8 event (type 0x5f, 9999 hours, f28 1,
 * f2a 60..69, f2c 6, f2e 0), a kind-0x1c event 4 lasting until feast day `day` (1c22; days
 * until it via s029 1367 08ae/0b1c, times 24 hours) whose f2a is a random location
 * (nearest_loc_type), and a kind-0x1c event 2 */
void far sub_s039_1462_1b0c(void)
{
    register int ev;
    register int day;
    struct date d;

    if (event_find(8, -1, -1, -1, 1, -1) != -1)
        return;
    ev = event_new(-2, 0, 0x5f, 8, 0, 0x63, 0, 9999, 0, 1);
    events[ev]->f2a = rand_mod(10) + 60;
    events[ev]->f2c = 6;
    events[ev]->f2e = 0;
    day = sub_s039_1462_1c22();
    date_set(&d, cur_year, 0, day, 0);
    ev = event_new(-2, 0, 0x63, 0x1c, 4, day, 8,
        days_until((struct date far *)&d) * 24, 1, 1);
    events[ev]->f2a = nearest_loc_type(rand_mod(300), rand_mod(900), 0x15);
    event_new(-2, 0, 0x5f, 0x1c, 2, 0, 0, 9999, 0, 1);
}

/* a random day of the year near the current month (cur_month): equinoxes/solstices and
 * feast days (80, 103, 145, 162, 172, 265, 304, 360) */
int far sub_s039_1462_1c22(void)
{
    switch (cur_month + 1) {
    case 0:
    case 1:
    case 2:
        if (rand_mod(2))
            return 0x50;
        return 0x67;
    case 3:
        if (!rand_mod(2))
            return 0x91;
        return 0x67;
    case 4:
        if (!rand_mod(2))
            return 0xa2;
        return 0x91;
    case 5:
        if (rand_mod(3))
            return 0xac;
        return 0xa2;
    case 6:
    case 7:
    case 8:
        if (rand_mod(2))
            return 0x109;
        return 0x130;
    case 9:
        if (rand_mod(2))
            return 0x130;
        return 0x168;
    case 10:
    case 11:
        if (rand_mod(2))
            return 0x168;
        return 0x50;
    }
    if (rand_mod(2))
        return 0x50;
    return 0x67;
}

/* day of month / month (s029 1367 0982/0938) of the day-of-year value of event
 * kind 1 type 4 (s029 36d4) */
void far sub_s039_1462_1cd2(void)
{
    mday_of_yday(loc_fact_value_any(1, 4));
}

void far sub_s039_1462_1ce8(void)
{
    month_of_yday(loc_fact_value_any(1, 4));
}

/* f2a of the kind-0x1c event of type 4 with f28 1 (-1 if none) */
int far sub_s039_1462_1cfe(void)
{
    register int i;

    if ((i = event_find(0x1c, 4, -1, -1, 1, -1)) != -1)
        return events[i]->f2a;
}

/* price of v + 1 for the party: by reputation rep (<= -10: 4/3, >= 50: 1/2, >= 10: 7/10),
 * at least 1, times the party size (1de6); 8/3 in a location of kind 2 (s029 1a8e 0x20),
 * 5/3 in kind 8; clamped to 1..1000 */
int far sub_s039_1462_1d2c(int rep, register int v)
{
    v++;
    if (rep <= -10)
        v = v * 4 / 3;
    else if (rep >= 50)
        v = v / 2;
    else if (rep >= 10)
        v = v * 7 / 10;
    if (v < 1)
        v = 1;
    v *= party_present_count_impl();
    if (cur_loc != -1 && loc_query(cur_loc, 0x20) == 2)
        v = v * 8 / 3;
    if ((cur_loc != -1 || cur_loc >= 0x5c) && loc_query(cur_loc, 0x20) == 8)
        v = v * 5 / 3;
    return clamp_range(1, 1000, v);
}

/* number of present party members (of the 5 slots) */
int far party_present_count_impl(void)
{
    int i;
    int n;
    struct member *p;

    n = 0;
    for (i = 0; i < 5; i++)
        if (party[i].status)
            n++;
    return n;
}

/* reward of size k (0..5: 0, 3, 10, 25, 64, 200 florins), 2/3 at difficulty game_difficulty 0,
 * 3/2 at 2; added to the party money (s029 1326/1382 what 4); returns the amount */
int far give_reward_impl(int k)
{
    register int v;

    if (k == 0)
        v = 0;
    else if (k == 1)
        v = 3;
    else if (k == 2)
        v = 10;
    else if (k == 3)
        v = 25;
    else if (k == 4)
        v = 64;
    else
        v = 200;
    if (game_difficulty == 0)
        v = v * 2 / 3;
    else if (game_difficulty != 1 && game_difficulty == 2)
        v = v * 3 / 2;
    party_set(4, v = party_get(4) + v);
    return v;
}

/* pay for a service of size k (0, 1, 2, 4, 15 times max(1, money / 200); other k: an
 * uninitialised factor): sets the money (s029 1382 what 4) to cost - money (sic) and
 * returns that value */
int far sub_s039_1462_1e90(int k)
{
    register int v;

    if (k == 0)
        v = 0;
    else if (k == 1)
        v = k;
    else if (k == 2)
        v = k;
    else if (k == 3)
        v = 4;
    else if (k == 4)
        v = 15;
    v *= imax(1, party_get(4) / 200);
    v -= party_get(4);
    party_set(4, v);
    return v;
}

/* unless a kind-8 event of value 7 exists (s029 32fe): start one (type 0x5f, 9999 hours,
 * f28 7) at the current location with f2a = random 0..135, f2c = 2, f2e = 139..170, and its
 * kind-0x1c event 5 */
void far sub_s039_1462_1f00(void)
{
    register int x;
    register int y;
    int i;

    if (!ev8_exists(7)) {
        x = rand_mod(0x20) + 0x8b;
        y = rand_mod(0x88);
        i = event_new(-2, cur_loc, 0x5f, 8, 0, 0, 0, 9999, 0, 7);
        events[i]->f2a = y;
        events[i]->f2c = 2;
        events[i]->f2e = x;
        event_new(-2, cur_loc, 0x5f, 0x1c, 5, 0, 0, 9999, 0, 7);
    }
}
