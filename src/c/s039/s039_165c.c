/* place: s039 165c:0000 */
/* orig: size=79d4 CONST=7eda+0002 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (e2, 2026-10-09): 139/139 functions (fmatch; while build/symbols.json comes
 *   from a non-identical build, check with build/agent/e2/fm.py, which re-derives the sub_
 *   addresses from their names).
 * The saints.  Not event rules: each of the 138 functions after 0000 describes one saint and
 * is called through a table (no direct callers) as
 *     int saint(int who, int x, int what)
 *   what 0..6: the saint's table entry t[what]:
 *       t0 day of the year (0..365, probably the feast day), t1 a class 0..9, t2 the minimum
 *       x, t3 an upper limit (99 for most), t4 the minimum virtue, t5 the base chance,
 *       t6 1 (3 for 125a);
 *   what 7: the chance that member `who` reaches the saint: 0000(t5, virtue of who, t4, x,
 *       t2, 0) (virtue = member byte +0x60 = skill 9; x is supplied by the caller,
 *       probably the Religion skill);
 *   what 8: the saint's help for member `who`, mostly temporary boosts lasting 24 hours:
 *       s029 4c04(who, attr, d, 24) attributes, 4b08(who, skill, d, 24) skills, 08f2(who,
 *       attr, pct) heals an attribute by pct % of the damage, 19d0(loc, a, b) raises the
 *       standing in the current location (or the one at the party's map position
 *       D_e3d0/D_e3d2 via vec_1fa9 when D_907e is -1), 0792(who, v), 2c4e timed events;
 *       returns 0.
 *   Attribute/skill numbers as in the member record (attrs +0x49 + k, skills +0x57 + k);
 *   in Darklands' order probably attrs 0 End, 1 Str, 2 Agl, 3 Per, 4 Int, 5 Chr, 6 DF and
 *   skills 0-6 weapons, 7 Alchemy, 8 Religion, 9 Virtue, 10 Latin, 11 Reading, 12 Healing,
 *   13 Artifice, 14 Stealth, 15 Streetwise, 16 Riding, 17 Woodwise.
 * Matching notes: the table is filled by single stores (`int t[7]; t[0] = ..`), `what` is a
 *   register parameter; the location ternary needs `#pragma optimize("a", on)` where the
 *   original keeps D_907e in DX; 2208 needs `for (flag = i = 0, p = D_9c69; ...)`; 0000's
 *   operand order (p_6 + p_10 + p_c) depends on MSC's symbol table: the 10 unused
 *   prototypes X_0..X_9 after the file's own prototypes reproduce it.
 */
#include "darklands/card.h"
#include "darklands/game.h"
struct member {                 /* party member, 0x80 bytes */
    char status;                /* +00: 0 = empty slot */
    char f01, f02;
    char f03;                   /* +03 */
    char pad04[0xc];
    char f10;                   /* +10 */
    char pad11[0x2a];
    char f3b, f3c;              /* +3b, +3c */
    char pad3d[4];
    char f41;                   /* +41 */
    char pad42[0x1e];
    char f60;                   /* +60: skill 9 (Virtue) = +0x57 + 9 */
    char pad61[0x1f];
};
struct money { int fl; int gr; int pf; };
extern struct money party_money;     /* party money */
extern int D_4bb6, D_4bb8;
extern int cur_loc;              /* current location (-1: none) */
extern int party_x, party_y;      /* party map position */
extern unsigned char far saints_known[][0x14];  /* per member: 160 bits */
extern struct member party[];
int far rand_mod();
int far party_count_able();
int far member_skill();
int far set_skill();
int far add_skill();
int far member_attr();
int far equip_bonuses();
int far heal_attr_pct();
int far set_attr();
int far add_attr();
int far member_attr_max();
int far count_items_flags();
int far loc_reputation_adj();
int far event_here();
int far event_new();
int far lower_skill_temp();
int far raise_attr_temp();

int far money_sub_fgp();
int far nearest_city();
int far add_item();
long far money_pfennigs();
int far sub_s039_165c_0000();
int far sub_s039_165c_0034();
int far sub_s039_165c_0190();
int far sub_s039_165c_024c();
int far sub_s039_165c_030c();
int far sub_s039_165c_038e();
int far sub_s039_165c_047e();
int far sub_s039_165c_0536();
int far sub_s039_165c_0604();
int far sub_s039_165c_06a2();
int far sub_s039_165c_0792();
int far sub_s039_165c_0898();
int far sub_s039_165c_09a2();
int far sub_s039_165c_0b82();
int far sub_s039_165c_0c20();
int far sub_s039_165c_0cbe();
int far sub_s039_165c_0d5c();
int far sub_s039_165c_0eb6();
int far sub_s039_165c_0f6e();
int far sub_s039_165c_1024();
int far sub_s039_165c_10ca();
int far sub_s039_165c_125a();
int far sub_s039_165c_133a();
int far sub_s039_165c_13d4();
int far sub_s039_165c_14a8();
int far sub_s039_165c_152a();
int far sub_s039_165c_15e2();
int far sub_s039_165c_16b6();
int far sub_s039_165c_17a4();
int far sub_s039_165c_1868();
int far sub_s039_165c_18ea();
int far sub_s039_165c_1988();
int far sub_s039_165c_1a18();
int far sub_s039_165c_1b02();
int far sub_s039_165c_1bac();
int far sub_s039_165c_1c54();
int far sub_s039_165c_1cf2();
int far sub_s039_165c_1de2();
int far sub_s039_165c_1eb6();
int far sub_s039_165c_1f8c();
int far sub_s039_165c_2052();
int far sub_s039_165c_2142();
int far sub_s039_165c_2208();
int far sub_s039_165c_23fe();
int far sub_s039_165c_249a();
int far sub_s039_165c_2538();
int far sub_s039_165c_2628();
int far sub_s039_165c_26d6();
int far sub_s039_165c_27c2();
int far sub_s039_165c_28b0();
int far sub_s039_165c_296a();
int far sub_s039_165c_2a24();
int far sub_s039_165c_2b30();
int far sub_s039_165c_2c2a();
int far sub_s039_165c_2cc8();
int far sub_s039_165c_2d58();
int far sub_s039_165c_2ece();
int far sub_s039_165c_2f82();
int far sub_s039_165c_307e();
int far sub_s039_165c_3186();
int far sub_s039_165c_3274();
int far sub_s039_165c_3320();
int far sub_s039_165c_33d6();
int far sub_s039_165c_353e();
int far sub_s039_165c_3612();
int far sub_s039_165c_36d6();
int far sub_s039_165c_3816();
int far sub_s039_165c_38b4();
int far sub_s039_165c_3934();
int far sub_s039_165c_39b4();
int far sub_s039_165c_3abc();
int far sub_s039_165c_3c32();
int far sub_s039_165c_3cec();
int far sub_s039_165c_3da4();
int far sub_s039_165c_3e0a();
int far sub_s039_165c_3eec();
int far sub_s039_165c_3fc2();
int far sub_s039_165c_41fa();
int far sub_s039_165c_4320();
int far sub_s039_165c_43d8();
int far sub_s039_165c_4608();
int far sub_s039_165c_46c2();
int far sub_s039_165c_47b0();
int far sub_s039_165c_489c();
int far sub_s039_165c_4962();
int far sub_s039_165c_49e4();
int far sub_s039_165c_4b0a();
int far sub_s039_165c_4ba8();
int far sub_s039_165c_4c46();
int far sub_s039_165c_4d1a();
int far sub_s039_165c_4f76();
int far sub_s039_165c_502e();
int far sub_s039_165c_50cc();
int far sub_s039_165c_51ba();
int far sub_s039_165c_5296();
int far sub_s039_165c_5334();
int far sub_s039_165c_540a();
int far sub_s039_165c_54c4();
int far sub_s039_165c_563a();
int far sub_s039_165c_5724();
int far sub_s039_165c_57de();
int far sub_s039_165c_5952();
int far sub_s039_165c_5a84();
int far sub_s039_165c_5b82();
int far sub_s039_165c_5cca();
int far sub_s039_165c_5d6c();
int far sub_s039_165c_5e5c();
int far sub_s039_165c_5ede();
int far sub_s039_165c_6008();
int far sub_s039_165c_6122();
int far sub_s039_165c_61aa();
int far sub_s039_165c_627e();
int far sub_s039_165c_6338();
int far sub_s039_165c_642c();
int far sub_s039_165c_64f4();
int far sub_s039_165c_65ac();
int far sub_s039_165c_664a();
int far sub_s039_165c_671e();
int far sub_s039_165c_67f2();
int far sub_s039_165c_68f4();
int far sub_s039_165c_69ac();
int far sub_s039_165c_6a4a();
int far sub_s039_165c_6ae8();
int far sub_s039_165c_6bb8();
int far sub_s039_165c_6ca6();
int far sub_s039_165c_6d92();
int far sub_s039_165c_6e62();
int far sub_s039_165c_6f0c();
int far sub_s039_165c_7038();
int far sub_s039_165c_710a();
int far sub_s039_165c_71ce();
int far sub_s039_165c_7288();
int far sub_s039_165c_7342();
int far sub_s039_165c_7432();
int far sub_s039_165c_74ea();
int far sub_s039_165c_75d8();
int far sub_s039_165c_76d4();
int far sub_s039_165c_7812();
int far sub_s039_165c_78ca();
/* unused: their number fixes the operand order in 0000 */
int far X_0();
int far X_1();
int far X_2();
int far X_3();
int far X_4();
int far X_5();
int far X_6();
int far X_7();
int far X_8();
int far X_9();


/* chance (percent) of reaching a saint: 0 unless p_8 >= p_a (virtue >= minimum) and
 * p_c >= p_e (x >= minimum), else (p_8 - p_a) / 2 - p_e + p_6 + p_10 + p_c (base p_6,
 * bonus p_10) */
int far sub_s039_165c_0000(int p_6, int p_8, int p_a, int p_c, int p_e, int p_10)
{
    if (p_a <= p_8 && p_e <= p_c) {
        return (p_8 - p_a) / 2 - p_e + p_6 + p_10 + p_c;
    }
    return 0;
}

/* saint: day 335, class 2, x 15 (t3 99), virtue 43, base 25; attr 0 +8..15; attr 5 +6..11; skills 0,1,2,3,4,5,6 +15..29 */
int far sub_s039_165c_0034(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x14f;
    t[1] = 2;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x2b;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x2b, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 36, class 9, x 20 (t3 80), virtue 24, base 25; attr 0 +6..11 (+12..23 when member byte +03 is set), attr 5 +6..11 */
int far sub_s039_165c_0190(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x24;
    t[1] = 9;
    t[2] = 0x14;
    t[3] = 0x50;
    t[4] = 0x18;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x18, x, 0x14, 0);
    case 8:
        if (party[who].f03 == 0)
            raise_attr_temp(who, 0, rand_mod(6) + 6, 0x18);
        else
            raise_attr_temp(who, 0, rand_mod(12) + 12, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 21, class 9, x 10 (t3 99), virtue 19, base 25; only while 007c() > 0 (also the chance): local standing +10/+20 (19d0), attr 5 +10..29 */
int far sub_s039_165c_024c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x15;
    t[1] = 9;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x13;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        if (party_count_able())
            return sub_s039_165c_0000(0x19, party[who].f60, 0x13, x, 10, 0);
        return 0;
    case 8:
        if (party_count_able()) {
            what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
            loc_reputation_adj(what, 10, 0x14);
            raise_attr_temp(who, 5, rand_mod(0x14) + 10, 0x18);
        }
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 31, class 0, x 15 (t3 99), virtue 23, base 25; skill 18 +25..29 */
int far sub_s039_165c_030c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x1f;
    t[1] = 0;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x17;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x17, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(5) + 0x19, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 319, class 6, x 25 (t3 99), virtue 39, base 60; attr 4 +8..15; attr 3 +6..11; skill 7 +30..44; skills 11,12 +15..29 */
int far sub_s039_165c_038e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x13f;
    t[1] = 6;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x27;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x27, x, 0x19, 0);
    case 8:
        raise_attr_temp(who, 4, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 7, rand_mod(0xf) + 0x1e, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 139, class 1, x 15 (t3 99), virtue 44, base 65; attr 4 +6..11; skills 11,12 +20..39 */
int far sub_s039_165c_047e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x8b;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x2c;
    t[5] = 0x41;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x2c, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 4, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0x14) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 168, class 2, x 10 (t3 99), virtue 28, base 40; local standing +10/+20, attr 5 +6..11; a party without florins and groschen gets 3..9 groschen */
int far sub_s039_165c_0536(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa8;
    t[1] = 2;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x1c;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x1c, x, 0xa, 0);
    case 8:
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        loc_reputation_adj(what, 10, 0x14);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        if (party_money.fl == 0 && party_money.gr == 0)
            party_money.gr = rand_mod(7) + 3;
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 334, class 7, x 5 (t3 50), virtue 14, base 20; attr 0 +6..11; attr 5 +8..15 */
int far sub_s039_165c_0604(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x14e;
    t[1] = 7;
    t[2] = 5;
    t[3] = 0x32;
    t[4] = 0xe;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0xe, x, 5, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 231, class 7, x 20 (t3 99), virtue 37, base 15; skills 0,1,3,4,5 +15..29 */
int far sub_s039_165c_06a2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xe7;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x25;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x25, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 17, class 7, x 20 (t3 99), virtue 35, base 35; attr 0 +4..9; attr 1 +5..10; attr 5 +1..10; skills 10,11 +10..19; skill 12 +20..39 */
int far sub_s039_165c_0792(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x11;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x23;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x23, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(6) + 4, 0x18);
        raise_attr_temp(who, 1, rand_mod(6) + 5, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 1, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0x14) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 164, class 5, x 10 (t3 99), virtue 59, base 65; attr 3 +6..11; attr 1 +4..7; attr 5 +15..24; skill 10 +20..29; skills 11,1 +10..19 */
int far sub_s039_165c_0898(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa4;
    t[1] = 5;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x3b;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x3b, x, 0xa, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xf, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0x14, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 199, class 1, x 15 (t3 99), virtue 18, base 25; attr 3 +6..11; attr 1,4 +4..7; skills 0,1,2,3,4,5,6 +10..19; skills 16,11 +6..11; skills 12,17 +8..15 */
int far sub_s039_165c_09a2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xc7;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x12;
    t[5] = 0x19;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x12, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 4, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 8, class 7, x 20 (t3 50), virtue 78, base 70; attr 0 +8..23 */
int far sub_s039_165c_0b82(int who, int x, register int what)
{
    int t[7];

    t[0] = 8;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x32;
    t[4] = 0x4e;
    t[5] = 0x46;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x46, party[who].f60, 0x4e, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 1, rand_mod(3) + 1 + 1, 0x18);
        raise_attr_temp(who, 0, rand_mod(0x10) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 338, class 9, x 20 (t3 50), virtue 32, base 40; skill 14 +15..29 (5 h); skill 6 +20..39 (10 h) */
int far sub_s039_165c_0c20(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x152;
    t[1] = 9;
    t[2] = 0x14;
    t[3] = 0x32;
    t[4] = 0x20;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x20, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0xe, rand_mod(0xf) + 0xf, 5);
        lower_skill_temp(who, 6, rand_mod(0x14) + 0x14, 0xa);
        return 0;
    }
    return t[what];
}

/* saint: day 30, class 9, x 15 (t3 60), virtue 21, base 20; attr 4 +4..8; attr 3 +3..6 */
int far sub_s039_165c_0cbe(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x1e;
    t[1] = 9;
    t[2] = 0xf;
    t[3] = 0x3c;
    t[4] = 0x15;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x15, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 4, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 3, rand_mod(4) + 3, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 156, class 6, x 10 (t3 90), virtue 25, base 10; attr 5 +8..15; skills 0,1,2,3,4,5,6 +10..19; skill 10 +8..15 */
int far sub_s039_165c_0d5c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x9c;
    t[1] = 6;
    t[3] = 0x5a;
    t[4] = 0x19;
    t[2] = 0xa;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x19, x, 0xa, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 329, class 9, x 30 (t3 70), virtue 46, base 30; attr 5,4 +8..15; skill 10 +40..99 */
int far sub_s039_165c_0eb6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x149;
    t[1] = 9;
    t[3] = 0x46;
    t[4] = 0x2e;
    t[2] = 0x1e;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x2e, x, 0x1e, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 4, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x3c) + 0x28, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 120, class 9, x 10 (t3 99), virtue 33, base 25; attr 5,3 +1..15; skill 10 +10..29 */
int far sub_s039_165c_0f6e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x78;
    t[1] = 9;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x21;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x21, x, 0xa, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(0xf) + 1, 0x18);
        raise_attr_temp(who, 3, rand_mod(0xf) + 1, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 327, class 9, x 25 (t3 99), virtue 41, base 50; attr 5 +3..10; local standing +20/+60 when who carries an item of type flag 0x20000000 (0dd8) */
int far sub_s039_165c_1024(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x147;
    t[1] = 9;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x29;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x29, x, 0x19, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(8) + 3, 0x18);
        if (count_items_flags(who, 0x20000000L))
            loc_reputation_adj(cur_loc, 0x14, 0x3c);
        return 0;
    }
    return t[what];
}

/* saint: day 15, class 0, x 15 (t3 99), virtue 23, base 25; attr 3 +3..5; attr 1 +6..11; attr 4 +3..5; skills 0,1,2,3,4,5,6,11 +6..11 */
int far sub_s039_165c_10ca(int who, int x, register int what)
{
    int t[7];

    t[1] = 0;
    t[0] = 0xf;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x17;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x17, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(3) + 3, 0x18);
        raise_attr_temp(who, 1, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 4, rand_mod(3) + 3, 0x18);
        lower_skill_temp(who, 0, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 1, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 2, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 4, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 5, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 6, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 213, class 9, x 10 (t3 50), virtue 40, base 50, t6 3; local standing +10/+20, skill 2 +15..29, attr 5 +6..11, high nibble of member byte +41 := 0xa */
int far sub_s039_165c_125a(int who, int x, register int what)
{
    int t[7];
    register int loc;
    register int v;

    t[0] = 0xd5;
    t[1] = 9;
    t[2] = 0xa;
    t[4] = 0x28;
    t[3] = 0x32;
    t[5] = 0x32;
    t[6] = 3;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x28, x, 0xa, 0);
    case 8:
        loc = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        loc_reputation_adj(loc, 10, 0x14);
        lower_skill_temp(who, 2, rand_mod(15) + 15, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        party[who].f41 = (party[who].f41 & 0xf) - 0x60;
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 205, class 8, x 50 (t3 99), virtue 54, base 50; attr 2 +8..15; attr 5 +1..10 */
int far sub_s039_165c_133a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xcd;
    t[1] = 8;
    t[3] = 0x63;
    t[4] = 0x36;
    t[2] = 0x32;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x36, x, 0x32, 0);
    case 8:
        raise_attr_temp(who, 2, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 1, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 207, class 7, x 15 (t3 99), virtue 31, base 25; skill 16 +10..19; skill 18 +20..39; skill 17 +25..74; skill 5 +6..11 */
int far sub_s039_165c_13d4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xcf;
    t[1] = 7;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x1f;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x1f, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 0x10, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0x32) + 0x19, 0x18);
        lower_skill_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 224, class 8, x 30 (t3 70), virtue 33, base 20; heal attr 0 100%; heal attr 1 100% */
int far sub_s039_165c_14a8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xe0;
    t[1] = 8;
    t[2] = 0x1e;
    t[3] = 0x46;
    t[4] = 0x21;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x21, x, 0x1e, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x64);
        return 0;
    }
    return t[what];
}

/* saint: day 155, class 8, x 10 (t3 99), virtue 22, base 30; skill 13 +10..19; attr 5 +6..11; heal attr 0 25%; heal attr 1 50% */
int far sub_s039_165c_152a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x9b;
    t[1] = 8;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x16;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x16, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0x19);
        heal_attr_pct(who, 1, 0x32);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 329, class 0, x 15 (t3 99), virtue 34, base 55; skill 10 +30..89; skill 11 +6..11; skill 12 +10..19; attr 5 +8..15 */
int far sub_s039_165c_15e2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x149;
    t[1] = 0;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x22;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x22, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0x3c) + 0x1e, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 161, class 0, x 20 (t3 99), virtue 36, base 60; attr 0 +8..15; attr 1 +6..13; attr 5 +4..7; 0792(rnd(0xa) + 0xa) */
int far sub_s039_165c_16b6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa1;
    t[1] = 0;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x24;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x24, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 1, rand_mod(8) + 6, 0x18);
        raise_attr_temp(who, 5, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 3, -(rand_mod(5) - 0xfffb), 0x18);
        equip_bonuses(who, rand_mod(0xa) + 0xa);
        return 0;
    }
    return t[what];
}

/* saint: day 270, class 7, x 15 (t3 75), virtue 15, base 30; heal attr 0/1 10%, skill 13 +40..79, attr 3 +4..7, teaches a saint (saints_known[who][3] bit 1) */
int far sub_s039_165c_17a4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x10e;
    t[1] = 7;
    t[3] = 0x4b;
    t[2] = 0xf;
    t[4] = 0xf;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0xf, x, 0xf, 0);
    case 8:
        heal_attr_pct(who, 0, 10);
        heal_attr_pct(who, 1, 10);
        lower_skill_temp(who, 13, rand_mod(0x28) + 0x28, 0x18);
        raise_attr_temp(who, 3, rand_mod(4) + 4, 0x18);
        saints_known[who][3] |= 2;
        return 0;
    }
    return t[what];
}

/* saint: day 299, class 7, x 25 (t3 99), virtue 28, base 35; every present member: bytes +3b and +3c +10 */
int far sub_s039_165c_1868(int who, int x, register int what)
{
    int t[7];
    int i;

    t[0] = 0x12b;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x1c;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x1c, x, 0x19, 0);
    case 8:
        for (i = 0; i < 5; i++)
            if (party[i].status) {
                party[i].f3b += 10;
                party[i].f3c += 10;
            }
        return 0;
    }
    return t[what];
}

/* saint: day 270, class 7, x 25 (t3 99), virtue 54, base 50; skill 7 +5..9; attr 4 +5..9 */
int far sub_s039_165c_18ea(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x10e;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x36;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x36, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 7, rand_mod(5) + 5, 0x18);
        raise_attr_temp(who, 4, rand_mod(5) + 5, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 270, class 7, x 25 (t3 80), virtue 30, base 40; teaches a saint (saints_known[who][3] bit 4), heal attr 0 100% and attr 1 30% */
int far sub_s039_165c_1988(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x10e;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x50;
    t[4] = 0x1e;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x1e, x, 0x19, 0);
    case 8:
        saints_known[who][3] |= 0x10;
        heal_attr_pct(who, 0, 100);
        heal_attr_pct(who, 1, 0x1e);
        return 0;
    }
    return t[what];
}

/* saint: day 145, class 0, x 10 (t3 75), virtue 30, base 25; attr 4 +4..7; attr 5 +1..6; attr 3 +6..11; skill 17 +6..11; 0792(rnd(8) + 8) */
int far sub_s039_165c_1a18(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x91;
    t[1] = 0;
    t[2] = 0xa;
    t[3] = 0x4b;
    t[4] = 0x1e;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x1e, x, 0xa, 0);
    case 8:
        raise_attr_temp(who, 4, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 1, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        equip_bonuses(who, rand_mod(8) + 8);
        lower_skill_temp(who, 0x11, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 283, class 1, x 25 (t3 99), virtue 38, base 20; skill 10 +25..49; attr 5 +15..44; heal attr 1 100% */
int far sub_s039_165c_1b02(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x11b;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x26;
    t[5] = 0x14;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x26, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 1, 0x64);
        lower_skill_temp(who, 0xa, rand_mod(0x19) + 0x19, 0x18);
        raise_attr_temp(who, 5, rand_mod(0x1e) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 96, class 0, x 20 (t3 60), virtue 57, base 45; attr 1 +5..14; heal attr 0 30%; 0792(rnd(0x28) + 0x14) */
int far sub_s039_165c_1bac(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x60;
    t[1] = 0;
    t[2] = 0x14;
    t[3] = 0x3c;
    t[4] = 0x39;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x39, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 1, rand_mod(0xa) + 5, 0x18);
        equip_bonuses(who, rand_mod(0x28) + 0x14);
        heal_attr_pct(who, 0, 0x1e);
        return 0;
    }
    return t[what];
}

/* saint: day 17, class 8, x 20 (t3 65), virtue 26, base 10; attr 5 +15..29; heal attr 0 100%; heal attr 1 20% */
int far sub_s039_165c_1c54(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x11;
    t[1] = 8;
    t[2] = 0x14;
    t[3] = 0x41;
    t[4] = 0x1a;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x1a, x, 0x14, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x14);
        raise_attr_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 85, class 7, x 30 (t3 99), virtue 53, base 45; skill 18 +10..19; skills 16,14 +20..39; attr 2 +10..19; skill 15 +10..19 */
int far sub_s039_165c_1cf2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x55;
    t[1] = 7;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x35;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x35, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xf, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 221, class 6, x 20 (t3 99), virtue 29, base 15; skills 10,11 +10..19; skill 12 +20..39; attr 4 +10..19 */
int far sub_s039_165c_1de2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xdd;
    t[1] = 6;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x1d;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x1d, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 304, class 8, x 20 (t3 99), virtue 52, base 50; attr 5 +10..19, attr 3 +6..11, skill 13 +10..19; item 3 (add_item) unless who carries an item of flag 1 */
int far sub_s039_165c_1eb6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x130;
    t[1] = 8;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x34;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x34, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(10) + 10, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 13, rand_mod(10) + 10, 0x18);
        if (!count_items_flags(who, 1L))
            add_item(who, 3);
        return 0;
    }
    return t[what];
}

/* saint: day 107, class 4, x 15 (t3 99), virtue 59, base 60; attr 3 +6..11; skill 13 +10..19; skill 18 +20..39 */
int far sub_s039_165c_1f8c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x6b;
    t[1] = 4;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x3b;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x3b, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0x14) + 0x14, 0x18);
        event_here(0x4f, 0xa8);
        return 0;
    }
    return t[what];
}

/* saint: day 140, class 0, x 30 (t3 60), virtue 45, base 60; 0792(6..11), attr 3 +6..11, skills 14 +25..74, 16 +6..11, 7 +5..14; member byte +41 +1 */
int far sub_s039_165c_2052(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x8c;
    t[1] = 0;
    t[2] = 0x1e;
    t[4] = 0x2d;
    t[3] = 0x3c;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x2d, x, 0x1e, 0);
    case 8:
        equip_bonuses(who, rand_mod(6) + 6);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 14, rand_mod(0x32) + 0x19, 0x18);
        lower_skill_temp(who, 16, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 7, rand_mod(10) + 5, 0x18);
        party[who].f41++;
        return 0;
    }
    return t[what];
}

/* saint: day 136, class 8, x 20 (t3 75), virtue 21, base 10; attr 2 +4..7; skill 15 +15..29; skill 13 +10..19; heal attr 0 30% */
int far sub_s039_165c_2142(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x88;
    t[1] = 8;
    t[2] = 0x14;
    t[3] = 0x4b;
    t[4] = 0x15;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x15, x, 0x14, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        raise_attr_temp(who, 2, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0xf, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 287, class 0, x 20 (t3 99), virtue 5, base 10; attr 0/4/3 +4..7/+6..11/+8..15, skill 17 +8..15, skills 0-6 +6..11; every present member gets skill 9 (virtue) >= 20, then the party loses a quarter of its money (s029 1367 0180) */
int far sub_s039_165c_2208(int who, int x, register int what)
{
    int t[7];
    int i;
    struct member *p;
    int flag;
    register int v;

    t[0] = 0x11f;
    t[1] = 0;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 5;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 5, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 4, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 3, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 17, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 1, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 2, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 3, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 4, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 5, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 6, rand_mod(6) + 6, 0x18);
        for (flag = i = 0, p = party; p < party + 5; i++, p++)
            if (p->status) {
                if (member_skill(i, 9) < 0x14)
                    set_skill(i, 9, 0x14);
                flag = 1;
            }
        if (flag)
            money_sub_fgp(&party_money, 0, 0, (unsigned)money_pfennigs(&party_money) >> 2);
        return 0;
    }
    return t[what];
}

/* saint: day 336, class 1, x 10 (t3 60), virtue 29, base 25; skill 14 +25..74; skill 7 +8..15 */
int far sub_s039_165c_23fe(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x150;
    t[2] = 0xa;
    t[3] = 0x3c;
    t[4] = 0x1d;
    t[5] = 0x19;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x1d, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0xe, rand_mod(0x32) + 0x19, 0x18);
        lower_skill_temp(who, 7, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 221, class 6, x 20 (t3 99), virtue 31, base 55; skill 10 +20..39; attr 5 +15..29 */
int far sub_s039_165c_249a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xdd;
    t[1] = 6;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x1f;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x1f, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 312, class 6, x 15 (t3 99), virtue 40, base 30; attr 5 +5..9; skill 17 +10..19; skill 10 +6..11; skill 18 +10..19; kind-0x4b event 9 (24 h) */
int far sub_s039_165c_2538(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x138;
    t[1] = 6;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x28;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x28, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(5) + 5, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0xa) + 0xa, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}

/* saint: day 154, class 7, x 15 (t3 50), virtue 32, base 40; attr 2,3 +6..11; member +41 changed */
int far sub_s039_165c_2628(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x9a;
    t[1] = 7;
    t[2] = 0xf;
    t[3] = 0x32;
    t[4] = 0x20;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x20, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 2, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        party[who].f41 = (party[who].f41 & 0xf) - 0x60;
        return 0;
    }
    return t[what];
}

/* saint: day 139, class 4, x 20 (t3 99), virtue 42, base 25; attr 1 +8..23; attr 5 +4..7; skills 17,10 +6..11; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_26d6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x8b;
    t[1] = 4;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x2a;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x2a, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 1, rand_mod(0x10) + 8, 0x18);
        raise_attr_temp(who, 5, rand_mod(4) + 4, 0x18);
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        lower_skill_temp(who, 0x11, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 264, class 7, x 30 (t3 99), virtue 41, base 30; skill 17 +10..19; skill 18 +40..69; skills 0,4,5 +6..11 */
int far sub_s039_165c_27c2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x108;
    t[1] = 7;
    t[3] = 0x63;
    t[4] = 0x29;
    t[2] = 0x1e;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x29, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0x11, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0x1e) + 0x28, 0x18);
        lower_skill_temp(who, 0, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 4, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 14, class 7, x 25 (t3 99), virtue 37, base 30; skill 15 +40..119; skill 18 +10..19; attr 2 +8..15 */
int far sub_s039_165c_28b0(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xe;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x25;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x25, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 0xf, rand_mod(0x50) + 0x28, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 2, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 269, class 0, x 30 (t3 99), virtue 17, base 45; skill 11 +10..19; attr 5 +8..15; attr 3 +6..11 */
int far sub_s039_165c_296a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x10d;
    t[1] = 0;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x11;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x11, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0xb, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 254, class 0, x 40 (t3 99), virtue 24, base 35; skill 16 +4..7; skills 10,12 +6..11; attr 2 +8..15; attr 5 +6..11; kind-0x4b event 9 (24 h) */
int far sub_s039_165c_2a24(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xfe;
    t[1] = 0;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x18;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x18, x, 0x28, 0);
    case 8:
        lower_skill_temp(who, 0x10, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 2, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 125, class 7, x 25 (t3 99), virtue 17, base 35; local standing +15/+25 at locations 0x43, 0x51-0x54, 0x56; 0792(6..11), skill 17 +6..11, a party-wide kind-0x4b event 9 (24 h) */
int far sub_s039_165c_2b30(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x7d;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x11;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x11, x, 0x19, 0);
    case 8:
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        switch (what) {
        case 0x43:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x56:
            loc_reputation_adj(what, 15, 0x19);
        }
        equip_bonuses(who, rand_mod(6) + 6);
        lower_skill_temp(who, 17, rand_mod(6) + 6, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 273, class 3, x 30 (t3 75), virtue 56, base 45; attr 4,3 +10..29 */
int far sub_s039_165c_2c2a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x111;
    t[1] = 3;
    t[2] = 0x1e;
    t[3] = 0x4b;
    t[4] = 0x38;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x38, x, 0x1e, 0);
    case 8:
        raise_attr_temp(who, 4, rand_mod(0x14) + 0xa, 0x18);
        raise_attr_temp(who, 3, rand_mod(0x14) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 3, class 9, x 40 (t3 99), virtue 73, base 75; heal attr 0 30%; heal attr 1 20% */
int far sub_s039_165c_2cc8(int who, int x, register int what)
{
    int t[7];

    t[0] = 3;
    t[1] = 9;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x49;
    t[5] = 0x4b;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x4b, party[who].f60, 0x49, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0x14);
        event_here(0x4f, 0xa8);
        return 0;
    }
    return t[what];
}

/* saint: day 114, class 6, x 40 (t3 99), virtue 23, base 10; attr 1 +10..14; attr 2 +5..9; skills 0,1,2,3,4,5,6,17 +25..49 */
int far sub_s039_165c_2d58(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x72;
    t[1] = 6;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x17;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x17, x, 0x28, 0);
    case 8:
        raise_attr_temp(who, 1, rand_mod(5) + 0xa, 0x18);
        raise_attr_temp(who, 2, rand_mod(5) + 5, 0x18);
        lower_skill_temp(who, 0, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 1, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 2, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 3, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 4, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 5, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 6, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0x19) + 0x19, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 5, class 4, x 40 (t3 75), virtue 34, base 50; skill 13 +20..39; heal attr 0 30%; heal attr 1 20%; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_2ece(int who, int x, register int what)
{
    int t[7];

    t[0] = 5;
    t[1] = 4;
    t[2] = 0x28;
    t[3] = 0x4b;
    t[4] = 0x22;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x22, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0x14);
        lower_skill_temp(who, 0xd, rand_mod(0x14) + 0x14, 0x18);
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        return 0;
    }
    return t[what];
}

/* saint: day 77, class 8, x 35 (t3 99), virtue 26, base 30; skill 17 +5..9; skills 18,11,12 +15..29; attr 5 +6..11 */
int far sub_s039_165c_2f82(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x4d;
    t[1] = 8;
    t[2] = 0x23;
    t[3] = 0x63;
    t[4] = 0x1a;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x1a, x, 0x23, 0);
    case 8:
        lower_skill_temp(who, 0x11, rand_mod(5) + 5, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0xf) + 0xf, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        event_here(0x5d, 1);
        return 0;
    }
    return t[what];
}

/* saint: day 245, class 1, x 35 (t3 99), virtue 45, base 15; skill 15 +30..59; skill 18 +5..9; skill 16 +50..89; heal attr 0 40%; heal attr 1 10%; 0792(-(rnd(0xa) - 0xfff1)) */
int far sub_s039_165c_307e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xf5;
    t[2] = 0x23;
    t[3] = 0x63;
    t[4] = 0x2d;
    t[5] = 0xf;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x2d, x, 0x23, 0);
    case 8:
        heal_attr_pct(who, 0, 0x28);
        heal_attr_pct(who, 1, 0xa);
        equip_bonuses(who, -(rand_mod(0xa) - 0xfff1));
        raise_attr_temp(who, 2, -(rand_mod(4) - 0xfffc), 0x18);
        lower_skill_temp(who, 0xf, rand_mod(0x1e) + 0x1e, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(5) + 5, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(0x28) + 0x32, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 125, class 5, x 15 (t3 99), virtue 64, base 60; skill 7 +40..59; skill 14 +10..19; attr 4 +5..9; attr 3 +15..24 */
int far sub_s039_165c_3186(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x7d;
    t[1] = 5;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x40;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x40, x, 0xf, 0);
    case 8:
        add_attr(who, 1, -1);
        add_skill(who, 9, -1);
        lower_skill_temp(who, 7, rand_mod(0x14) + 0x28, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 4, rand_mod(5) + 5, 0x18);
        raise_attr_temp(who, 3, rand_mod(0xa) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 125, class 4, x 15 (t3 99), virtue 39, base 55; skill 11 +20..39; skill 12 +25..49; heal attr 0 30% */
int far sub_s039_165c_3274(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x7d;
    t[1] = 4;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x27;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x27, x, 0xf, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        lower_skill_temp(who, 0xb, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0x19) + 0x19, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 311, class 1, x 25 (t3 99), virtue 55, base 40; skill 10 +15..29; skill 14 +10..19; heal attr 0 20% */
int far sub_s039_165c_3320(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x137;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x37;
    t[5] = 0x28;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x37, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        lower_skill_temp(who, 0xa, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0xa) + 0xa, 0x18);
        event_here(0x4f, 0xa8);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 158, class 6, x 25 (t3 50), virtue 44, base 55; local standing +20/+40 at locations 4-7 and 10-12; skills 0-6 +15..29 */
int far sub_s039_165c_33d6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x9e;
    t[1] = 6;
    t[2] = 0x19;
    t[3] = 0x32;
    t[4] = 0x2c;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x2c, x, 0x19, 0);
    case 8:
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        switch (what) {
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
        case 11:
        case 12:
            loc_reputation_adj(what, 0x14, 0x28);
        }
        lower_skill_temp(who, 0, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 1, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 2, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 3, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 4, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 5, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 6, rand_mod(15) + 15, 0x18);
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 321, class 2, x 30 (t3 99), virtue 74, base 80; attr 5 +8..15; skill 7 +10..19; skill 14 +20..39; skill 10 +10..19 */
int far sub_s039_165c_353e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x141;
    t[1] = 2;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x4a;
    t[5] = 0x50;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x50, party[who].f60, 0x4a, x, 0x1e, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 7, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 289, class 8, x 15 (t3 75), virtue 16, base 10; local standing +30/+50 at locations 0x25, 0x48, 0x49, 0x4b; skill 13 +20..29 */
int far sub_s039_165c_3612(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x121;
    t[1] = 8;
    t[2] = 0xf;
    t[3] = 0x4b;
    t[4] = 0x10;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x10, x, 0xf, 0);
    case 8:
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        switch (what) {
        case 0x25:
        case 0x48:
        case 0x49:
        case 0x4b:
            loc_reputation_adj(what, 0x1e, 0x32);
        }
        lower_skill_temp(who, 13, rand_mod(10) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 194, class 6, x 30 (t3 99), virtue 61, base 75; skills 0,1,2,3,4,5,6 +10..19; attr 4 +8..23 */
int far sub_s039_165c_36d6(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xc2;
    t[1] = 6;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x3d;
    t[5] = 0x4b;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x4b, party[who].f60, 0x3d, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 4, rand_mod(0x10) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 75, class 6, x 20 (t3 99), virtue 30, base 35; attr 5 +10..19; attr 1 +4..7 */
int far sub_s039_165c_3816(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x4b;
    t[1] = 6;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x1e;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x1e, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 168, class 1, x 15 (t3 99), virtue 47, base 40; attr 3 +15..29 */
int far sub_s039_165c_38b4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa8;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x2f;
    t[5] = 0x28;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x2f, x, 0xf, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 260, class 8, x 25 (t3 99), virtue 13, base 25; attr 3 +20..59 */
int far sub_s039_165c_3934(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x104;
    t[1] = 8;
    t[3] = 0x63;
    t[4] = 0xd;
    t[2] = 0x19;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0xd, x, 0x19, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(0x28) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 307, class 1, x 50 (t3 99), virtue 34, base 50; skills 3,4,5 +15..29; skill 6 +10..19; skill 15 +25..49; skill 18 +40..79 */
int far sub_s039_165c_39b4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x133;
    t[3] = 0x63;
    t[4] = 0x22;
    t[2] = 0x32;
    t[5] = 0x32;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x22, x, 0x32, 0);
    case 8:
        lower_skill_temp(who, 3, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xf, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0x28) + 0x28, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 311, class 0, x 20 (t3 99), virtue 63, base 65; attr 5 +15..29; skills 0,1,2,3,4,5,6,10 +10..19; skill 17 +5..9 */
int far sub_s039_165c_3abc(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x137;
    t[1] = 0;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x3f;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x3f, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(5) + 5, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 135, class 4, x 10 (t3 99), virtue 21, base 35; skill 16 +10..19; skill 10 +5..9; attr 5 +8..15 */
int far sub_s039_165c_3c32(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x87;
    t[1] = 4;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x15;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x15, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0x10, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(5) + 5, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 15, class 8, x 10 (t3 99), virtue 85, base 70; skill 13 +20..39; attr 5 +8..15; heal attr 0 50%; heal attr 1 100% */
int far sub_s039_165c_3cec(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xf;
    t[1] = 8;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x55;
    t[5] = 0x46;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x46, party[who].f60, 0x55, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0x32);
        heal_attr_pct(who, 1, 0x64);
        lower_skill_temp(who, 0xd, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 292, class 7, x 35 (t3 75), virtue 66, base 50; no effect */
int far sub_s039_165c_3da4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x124;
    t[1] = 7;
    t[2] = 0x23;
    t[3] = 0x4b;
    t[4] = 0x42;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x42, x, 0x23, 0);
    case 8:
        return 0;
    }
    return t[what];
}

/* saint: day 294, class 0, x 10 (t3 99), virtue 36, base 15; member byte +03 clear: heal attr 0 30%, attr 1 10%; set: heal 100%/20%, attr 0 +5..9, attr 1 +4..7 */
int far sub_s039_165c_3e0a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x126;
    t[1] = 0;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x24;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x24, x, 0xa, 0);
    case 8:
        if (party[who].f03 == 0) {
            heal_attr_pct(who, 0, 0x1e);
            heal_attr_pct(who, 1, 10);
        } else {
            heal_attr_pct(who, 0, 100);
            heal_attr_pct(who, 1, 0x14);
            raise_attr_temp(who, 0, rand_mod(5) + 5, 0x18);
            raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        }
        return 0;
    }
    return t[what];
}

/* saint: day 256, class 2, x 10 (t3 50), virtue 37, base 25; skills 10,11 +30..59; attr 5 +8..15 */
int far sub_s039_165c_3eec(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x100;
    t[1] = 2;
    t[2] = 0xa;
    t[3] = 0x32;
    t[4] = 0x25;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x25, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0x1e) + 0x1e, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0x1e) + 0x1e, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, -(rand_mod(6) - 0xfffa), 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 89, class 6, x 40 (t3 99), virtue 20, base 30; skills 7, 8, 10-16 and 18 + rnd 4 + skill 9 (virtue) / 5 + 1 */
int far sub_s039_165c_3fc2(int who, int x, register int what)
{
    int t[7];
    register int v;

    t[0] = 0x59;
    t[1] = 6;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x14;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x14, x, 0x28, 0);
    case 8:
        lower_skill_temp(who, 7, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 8, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 10, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 11, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 12, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 13, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 15, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 14, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 18, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        lower_skill_temp(who, 16, rand_mod(4) + member_skill(who, 9) / 5 + 1, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 136, class 4, x 15 (t3 60), virtue 22, base 30; skills 10 +20..39, 16 +4..7, attrs 5 +8..15, 4 +6..11; standing +15/+25 at location 0x46 (passes an uninitialised local as the location), else +10/+20 at 0x44-0x47, 0x49, 0x4a */
int far sub_s039_165c_41fa(int who, int x, register int what)
{
    int t[7];
    int u;

    t[0] = 0x88;
    t[1] = 4;
    t[2] = 0xf;
    t[3] = 0x3c;
    t[4] = 0x16;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x16, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 10, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 16, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 4, rand_mod(6) + 6, 0x18);
        if (cur_loc == 0x46)
            loc_reputation_adj(u, 15, 0x19);
        else {
            what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
            switch (what) {
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
            case 0x49:
            case 0x4a:
                loc_reputation_adj(what, 10, 0x14);
            }
        }
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 78, class 7, x 35 (t3 99), virtue 27, base 35; skills 14,16 +30..59; attr 5 +6..11 */
int far sub_s039_165c_4320(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x4e;
    t[1] = 7;
    t[3] = 0x63;
    t[4] = 0x1b;
    t[2] = 0x23;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x1b, x, 0x23, 0);
    case 8:
        lower_skill_temp(who, 0xe, rand_mod(0x1e) + 0x1e, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(0x1e) + 0x1e, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 301, class 7, x 20 (t3 90), virtue 15, base 5; attr 0,5,1,3,2,4 +4..8; skills 7,8,10,11,12,13,15,14,18,16 +6..11; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_43d8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x12d;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x5a;
    t[4] = 0xf;
    t[5] = 5;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(5, party[who].f60, 0xf, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 0, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 5, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 1, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 3, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 2, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 4, rand_mod(5) + 4, 0x18);
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        lower_skill_temp(who, 7, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 8, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xf, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 43, class 6, x 40 (t3 99), virtue 61, base 75; skill 18 +20..39; skill 17 +6..11; kind-0x4b event 9 (24 h) */
int far sub_s039_165c_4608(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x2b;
    t[1] = 6;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x3d;
    t[5] = 0x4b;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x4b, party[who].f60, 0x3d, x, 0x28, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(6) + 6, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}

/* saint: day 69, class 0, x 20 (t3 99), virtue 31, base 25; attr 5 +6..11; skill 10 +15..29; skill 17 +4..7; skill 13 +10..19; heal attr 0 40%; heal attr 1 20% */
int far sub_s039_165c_46c2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x45;
    t[1] = 0;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x1f;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x1f, x, 0x14, 0);
    case 8:
        heal_attr_pct(who, 0, 0x28);
        heal_attr_pct(who, 1, 0x14);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 178, class 4, x 25 (t3 99), virtue 51, base 60; skill 10 +6..11; skill 17 +10..19; attr 1 +4..7; attr 2 +3..5; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_47b0(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xb2;
    t[1] = 4;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x33;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x33, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 2, rand_mod(3) + 3, 0x18);
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        return 0;
    }
    return t[what];
}

/* saint: day 222, class 7, x 30 (t3 99), virtue 20, base 10; skills 10,16 +10..19; attr 5 +12..19; heal attr 0 100% */
int far sub_s039_165c_489c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xde;
    t[1] = 7;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x14;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x14, x, 0x1e, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 351, class 7, x 50 (t3 99), virtue 80, base 60; heal attr 0 30%; heal attr 1 100% */
int far sub_s039_165c_4962(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x15f;
    t[1] = 7;
    t[2] = 0x32;
    t[3] = 0x63;
    t[4] = 0x50;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x50, x, 0x32, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0x64);
        return 0;
    }
    return t[what];
}

/* saint: day 74, class 7, x 30 (t3 99), virtue 49, base 45; skills 0,1,2,3,4,5,6 +10..19 */
int far sub_s039_165c_49e4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x4a;
    t[1] = 7;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x31;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x31, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 347, class 8, x 40 (t3 99), virtue 77, base 65; attr 3 +6..11; heal attr 0 100%; heal attr 1 100% */
int far sub_s039_165c_4b0a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x15b;
    t[1] = 8;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x4d;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x4d, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x64);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 291, class 7, x 20 (t3 99), virtue 17, base 35; skill 13 +20..39; heal attr 0 50%; heal attr 1 30% */
int far sub_s039_165c_4ba8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x123;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x11;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x11, x, 0x14, 0);
    case 8:
        heal_attr_pct(who, 0, 0x32);
        heal_attr_pct(who, 1, 0x1e);
        lower_skill_temp(who, 0xd, rand_mod(0x14) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 167, class 9, x 35 (t3 99), virtue 60, base 50; attr 5 +6..11; attr 3 +8..15; skill 9 +6..17; attr 0 +5..9 */
int far sub_s039_165c_4c46(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa7;
    t[1] = 9;
    t[2] = 0x23;
    t[3] = 0x63;
    t[4] = 0x3c;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x3c, x, 0x23, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 3, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 9, rand_mod(0xc) + 6, 0x18);
        raise_attr_temp(who, 0, rand_mod(5) + 5, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 201, class 9, x 25 (t3 75), virtue 46, base 45; member byte +03 clear: heal attr 0/1 10%; set: heal 100%/50%, attrs 0-5 +3..6, skills 7, 8, 10-15, 18, 16 +5..10 */
int far sub_s039_165c_4d1a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xc9;
    t[1] = 9;
    t[2] = 0x19;
    t[3] = 0x4b;
    t[4] = 0x2e;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x2e, x, 0x19, 0);
    case 8:
        if (party[who].f03 == 0) {
            heal_attr_pct(who, 0, 10);
            heal_attr_pct(who, 1, 10);
        } else {
            heal_attr_pct(who, 0, 100);
            heal_attr_pct(who, 1, 0x32);
            raise_attr_temp(who, 0, rand_mod(4) + 3, 0x18);
            raise_attr_temp(who, 5, rand_mod(4) + 3, 0x18);
            raise_attr_temp(who, 1, rand_mod(4) + 3, 0x18);
            raise_attr_temp(who, 3, rand_mod(4) + 3, 0x18);
            raise_attr_temp(who, 2, rand_mod(4) + 3, 0x18);
            raise_attr_temp(who, 4, rand_mod(4) + 3, 0x18);
            lower_skill_temp(who, 7, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 8, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 10, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 11, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 12, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 13, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 15, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 14, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 18, rand_mod(6) + 5, 0x18);
            lower_skill_temp(who, 16, rand_mod(6) + 5, 0x18);
        }
        return 0;
    }
    return t[what];
}

/* saint: day 53, class 9, x 25 (t3 99), virtue 52, base 10; skill 13 +20..39; skill 10 +10..19; heal attr 0 100%; heal attr 1 40% */
int far sub_s039_165c_4f76(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x35;
    t[1] = 9;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x34;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x34, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x28);
        lower_skill_temp(who, 0xd, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 280, class 7, x 25 (t3 99), virtue 35, base 40; skill 12 +15..29; attr 2 +12..23 */
int far sub_s039_165c_502e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x118;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x23;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x23, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 0xc, rand_mod(0xf) + 0xf, 0x18);
        raise_attr_temp(who, 2, rand_mod(0xc) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 315, class 7, x 25 (t3 99), virtue 69, base 65; attr 5 +8..15; attr 3 +12..19; skill 10 +25..49; skill 17 +20..39; heal attr 0 30%; heal attr 1 30% */
int far sub_s039_165c_50cc(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x13b;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x45;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x45, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0x1e);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, rand_mod(8) + 0xc, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x19) + 0x19, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0x14) + 0x14, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 264, class 7, x 20 (t3 80), virtue 26, base 25; skills 11 +15..29, 12 +25..49, attr 4 +8..15; D_4bb6 and D_4bb8 +20..39 */
int far sub_s039_165c_51ba(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x108;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x50;
    t[4] = 0x1a;
    t[5] = 0x19;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x19, party[who].f60, 0x1a, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 11, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 12, rand_mod(0x19) + 0x19, 0x18);
        raise_attr_temp(who, 4, rand_mod(8) + 8, 0x18);
        D_4bb6 += rand_mod(0x14) + 0x14;
        D_4bb8 += rand_mod(0x14) + 0x14;
        return 0;
    }
    return t[what];
}

/* saint: day 265, class 7, x 30 (t3 50), virtue 81, base 70; skill 0 +20..39; skill 7 +10..19 */
int far sub_s039_165c_5296(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x109;
    t[1] = 7;
    t[2] = 0x1e;
    t[3] = 0x32;
    t[4] = 0x51;
    t[5] = 0x46;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x46, party[who].f60, 0x51, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 7, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 272, class 3, x 25 (t3 99), virtue 72, base 55; 0792(20), skill 13 +20..39, attr 5 +12..19, local standing +20/+40 */
int far sub_s039_165c_5334(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x110;
    t[1] = 3;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x48;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x48, x, 0x19, 0);
    case 8:
        equip_bonuses(who, 0x14);
        lower_skill_temp(who, 13, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 12, 0x18);
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        loc_reputation_adj(what, 0x14, 0x28);
        return 0;
    }
    return t[what];
}

/* saint: day 54, class 8, x 35 (t3 99), virtue 71, base 70; skill 13 +20..39; attr 5 +10..19; kind-0x4b event 9 (24 h) */
int far sub_s039_165c_540a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x36;
    t[1] = 8;
    t[2] = 0x23;
    t[3] = 0x63;
    t[4] = 0x47;
    t[5] = 0x46;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x46, party[who].f60, 0x47, x, 0x23, 0);
    case 8:
        lower_skill_temp(who, 0xd, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}

/* saint: day 240, class 5, x 10 (t3 99), virtue 66, base 65; skills 0,1,2,3,4,5,6 +15..29; skill 15 +20..39; skills 18,16 +10..19 */
int far sub_s039_165c_54c4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xf0;
    t[1] = 5;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x42;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x42, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xf, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0x10, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

#pragma optimize("a", on)
/* saint: day 340, class 1, x 25 (t3 99), virtue 49, base 55; local standing +10/+20, skill 2 +15..29, attr 5 +6..11, attr 0 +4..8 */
int far sub_s039_165c_563a(int who, int x, register int what)
{
    int t[7];
    register int loc;
    register int v;

    t[0] = 0x154;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x31;
    t[5] = 0x37;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x31, x, 0x19, 0);
    case 8:
        loc = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        loc_reputation_adj(loc, 10, 0x14);
        lower_skill_temp(who, 2, rand_mod(15) + 15, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 0, rand_mod(5) + 4, 0x18);
        return 0;
    }
    return t[what];
}
#pragma optimize("", on)

/* saint: day 347, class 9, x 20 (t3 99), virtue 25, base 40; attr 3 +6..11; local standing +15/+25 at locations 0x36 and 0x4c */
int far sub_s039_165c_5724(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x15b;
    t[1] = 9;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x19;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x19, x, 0x14, 0);
    case 8:
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        switch (what) {
        case 0x36:
        case 0x4c:
            loc_reputation_adj(what, 15, 0x19);
        }
        return 0;
    }
    return t[what];
}

/* saint: day 181, class 1, x 30 (t3 99), virtue 47, base 60; skills 0,1,2,3,4,5,6 +8..22; skill 10 +8..15; attr 3 +10..19; attr 5 +4..8 */
int far sub_s039_165c_57de(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xb5;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x2f;
    t[5] = 0x3c;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x2f, x, 0x1e, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(5) + 4, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 210, class 4, x 15 (t3 99), virtue 18, base 20; skills 0,1,2,3,4,5,6 +8..22; heal attr 0 30% */
int far sub_s039_165c_5952(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xd2;
    t[1] = 4;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x12;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x12, x, 0xf, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        lower_skill_temp(who, 0, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 1, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 2, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 3, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 4, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 5, rand_mod(0xf) + 8, 0x18);
        lower_skill_temp(who, 6, rand_mod(0xf) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 208, class 2, x 25 (t3 99), virtue 56, base 20; heal attr 0/1 30%, skill 7 +10..19, skill 13 +15..29, attr 5 +6..11, kind-0x4b event 9 (24 h), high nibble of member byte +41 := 0xa */
int far sub_s039_165c_5a84(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xd0;
    t[1] = 2;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x38;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x38, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0x1e);
        lower_skill_temp(who, 7, rand_mod(10) + 10, 0x18);
        lower_skill_temp(who, 13, rand_mod(15) + 15, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        party[who].f41 = (party[who].f41 & 0xf) - 0x60;
        return 0;
    }
    return t[what];
}

/* saint: day 76, class 0, x 25 (t3 99), virtue 22, base 15; skills 0, 1, 3, 10-12 +15..29, attr 5 +15..24, attr 1 +12..19, member byte +41 +2 */
int far sub_s039_165c_5b82(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x4c;
    t[1] = 0;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x16;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x16, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 0, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 1, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 3, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 10, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 11, rand_mod(15) + 15, 0x18);
        lower_skill_temp(who, 12, rand_mod(15) + 15, 0x18);
        raise_attr_temp(who, 5, rand_mod(10) + 15, 0x18);
        raise_attr_temp(who, 1, rand_mod(8) + 12, 0x18);
        party[who].f41 += 2;
        return 0;
    }
    return t[what];
}

/* saint: day 66, class 7, x 20 (t3 80), virtue 70, base 60; heal attr 0/1 100%, attr 4 lowered by half its maximum (0b64) */
int far sub_s039_165c_5cca(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x42;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x50;
    t[4] = 0x46;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x46, x, 0x14, 0);
    case 8:
        heal_attr_pct(who, 0, 100);
        heal_attr_pct(who, 1, 100);
        raise_attr_temp(who, 4, -(member_attr_max(who, 4) / 2), 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 206, class 7, x 27 (t3 52), virtue 19, base 35; skills 10,11 +12..26; skill 12 +15..29; skill 13 +10..19; attr 4 +12..23 */
int far sub_s039_165c_5d6c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xce;
    t[1] = 7;
    t[2] = 0x1b;
    t[3] = 0x34;
    t[4] = 0x13;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x13, x, 0x1b, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0xf) + 0xc, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xf) + 0xc, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 4, rand_mod(0xc) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 66, class 8, x 40 (t3 99), virtue 16, base 10; attr 5 +15..29 */
int far sub_s039_165c_5e5c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x42;
    t[1] = 8;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x10;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x10, x, 0x28, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 180, class 7, x 25 (t3 75), virtue 58, base 55; skills 10, 11, 13 +10..19, skill 0 +15..29, attr 5 +8..15, attr 1 +12..19, attr 3 lowered by half its maximum */
int far sub_s039_165c_5ede(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xb4;
    t[1] = 7;
    t[2] = 0x19;
    t[3] = 0x4b;
    t[4] = 0x3a;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x3a, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 10, rand_mod(10) + 10, 0x18);
        lower_skill_temp(who, 11, rand_mod(10) + 10, 0x18);
        lower_skill_temp(who, 13, rand_mod(10) + 10, 0x18);
        lower_skill_temp(who, 0, rand_mod(15) + 15, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 1, rand_mod(8) + 12, 0x18);
        raise_attr_temp(who, 3, -(member_attr_max(who, 3) / 2), 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 1, class 2, x 25 (t3 99), virtue 62, base 50; skills 7 +10..19, 15 +40..79, 17 +10..19, attrs 4/3 +6..11; local standing -1/-5 */
int far sub_s039_165c_6008(int who, int x, register int what)
{
    int t[7];

    t[1] = 2;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x3e;
    t[5] = 0x32;
    t[0] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x3e, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 7, rand_mod(10) + 10, 0x18);
        lower_skill_temp(who, 15, rand_mod(40) + 40, 0x18);
        lower_skill_temp(who, 17, rand_mod(10) + 10, 0x18);
        raise_attr_temp(who, 4, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        loc_reputation_adj(what, -1, -5);
        return 0;
    }
    return t[what];
}

/* saint: day 54, class 7, x 10 (t3 75), virtue 19, base 20; every present member: member byte +41 := low nibble + 0xb0, byte +10 := 0x5a */
int far sub_s039_165c_6122(int who, int x, register int what)
{
    int t[7];
    int i;

    t[0] = 0x36;
    t[1] = 7;
    t[2] = 0xa;
    t[3] = 0x4b;
    t[4] = 0x13;
    t[5] = 0x14;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x14, party[who].f60, 0x13, x, 0xa, 0);
    case 8:
        for (i = 0; i < 5; i++)
            if (party[i].status) {
                party[i].f41 = (party[i].f41 & 0xf) - 0x50;
                party[i].f10 = 0x5a;
            }
        return 0;
    }
    return t[what];
}

/* saint: day 272, class 3, x 40 (t3 60), virtue 75, base 70; skill 13 +50..139; attr 2 +8..15; attr 3 +6..11; heal attr 0 100%; heal attr 1 100% */
int far sub_s039_165c_61aa(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x110;
    t[1] = 3;
    t[2] = 0x28;
    t[3] = 0x3c;
    t[4] = 0x4b;
    t[5] = 0x46;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x46, party[who].f60, 0x4b, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x64);
        lower_skill_temp(who, 0xd, rand_mod(0x5a) + 0x32, 0x18);
        raise_attr_temp(who, 2, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 7, class 5, x 10 (t3 99), virtue 67, base 50; skill 12 +30..89; attr 4 +15..29; kind-0x4b event 9 (24 h) */
int far sub_s039_165c_627e(int who, int x, register int what)
{
    int t[7];

    t[0] = 7;
    t[1] = 5;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x43;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x43, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0xc, rand_mod(0x3c) + 0x1e, 0x18);
        raise_attr_temp(who, 4, rand_mod(0xf) + 0xf, 0x18);
        event_new(-2, 0, 0x63, 0x4b, 0, 0, 9, 0x18, 1, 0);
        return 0;
    }
    return t[what];
}

/* saint: day 181, class 5, x 10 (t3 99), virtue 29, base 35; skills 7 +20..39, 12 +30..59, attr 4 +12..19; attrs 0 and 1 raised to at least 9 (0988) */
int far sub_s039_165c_6338(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xb5;
    t[1] = 5;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x1d;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x1d, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 7, rand_mod(20) + 20, 0x18);
        lower_skill_temp(who, 12, rand_mod(30) + 30, 0x18);
        raise_attr_temp(who, 4, rand_mod(8) + 12, 0x18);
        if (member_attr(who, 0) < 9)
            set_attr(who, 0, 9);
        if (member_attr(who, 1) < 9)
            set_attr(who, 1, 9);
        return 0;
    }
    return t[what];
}

/* saint: day 7, class 6, x 25 (t3 99), virtue 27, base 45; skill 7 +6..17; skill 14 +35..69; heal attr 0 30% */
int far sub_s039_165c_642c(int who, int x, register int what)
{
    int t[7];

    t[0] = 7;
    t[1] = 6;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x1b;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x1b, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        raise_attr_temp(who, 5, -(rand_mod(6) - 0xfffa), 0x18);
        lower_skill_temp(who, 7, rand_mod(0xc) + 6, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0x23) + 0x23, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 228, class 5, x 40 (t3 90), virtue 19, base 45; skill 13 +10..19; skill 14 +8..15; heal attr 0 20%; heal attr 1 20% */
int far sub_s039_165c_64f4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xe4;
    t[1] = 5;
    t[2] = 0x28;
    t[3] = 0x5a;
    t[4] = 0x13;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x13, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        heal_attr_pct(who, 1, 0x14);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 92, class 2, x 25 (t3 99), virtue 64, base 50; attr 5 +10..19; heal attr 0 100%; heal attr 1 100% */
int far sub_s039_165c_65ac(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x5c;
    t[1] = 2;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x40;
    t[5] = 0x32;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x32, party[who].f60, 0x40, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x64);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 20, class 7, x 15 (t3 90), virtue 28, base 10; skill 5 +20..39; skill 17 +10..19; attr 2 +12..19; heal attr 0 10%; heal attr 1 10% */
int far sub_s039_165c_664a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x14;
    t[1] = 7;
    t[2] = 0xf;
    t[3] = 0x5a;
    t[4] = 0x1c;
    t[5] = 0xa;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xa, party[who].f60, 0x1c, x, 0xf, 0);
    case 8:
        heal_attr_pct(who, 0, 0xa);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 5, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 2, rand_mod(8) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 91, class 4, x 40 (t3 99), virtue 42, base 65; skill 10 +30..59; skill 11 +20..39; attr 5 +6..11; heal attr 0 20%; heal attr 1 10% */
int far sub_s039_165c_671e(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x5b;
    t[1] = 4;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x2a;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x2a, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 0xa, rand_mod(0x1e) + 0x1e, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 228, class 6, x 20 (t3 99), virtue 27, base 30; 0792(15..24); local standing +15/+25 only at location 0x55; skills 10 +6..11, 17 +10..19, attrs 5 +6..11, 4 +8..15 */
int far sub_s039_165c_67f2(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xe4;
    t[1] = 6;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x1b;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x1b, x, 0x14, 0);
    case 8:
        equip_bonuses(who, rand_mod(10) + 15);
        if (cur_loc == 0x55)
            loc_reputation_adj(cur_loc, 15, 0x19);
        lower_skill_temp(who, 10, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 17, rand_mod(10) + 10, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 4, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 121, class 4, x 30 (t3 99), virtue 25, base 65; skill 10 +25..49; attr 5 +10..19; heal attr 0 20%; heal attr 1 10% */
int far sub_s039_165c_68f4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x79;
    t[1] = 4;
    t[2] = 0x1e;
    t[3] = 0x63;
    t[4] = 0x19;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x19, x, 0x1e, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 0xa, rand_mod(0x19) + 0x19, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 284, class 7, x 40 (t3 99), virtue 50, base 30; attr 5 +6..11; heal attr 0 40%; heal attr 1 20% */
int far sub_s039_165c_69ac(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x11c;
    t[1] = 7;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x32;
    t[5] = 0x1e;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x1e, party[who].f60, 0x32, x, 0x28, 0);
    case 8:
        heal_attr_pct(who, 0, 0x28);
        heal_attr_pct(who, 1, 0x14);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 140, class 7, x 10 (t3 99), virtue 62, base 65; skill 13 +30..59; heal attr 0 40%; heal attr 1 20% */
int far sub_s039_165c_6a4a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x8c;
    t[1] = 7;
    t[2] = 0xa;
    t[3] = 0x63;
    t[4] = 0x3e;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x3e, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0x28);
        heal_attr_pct(who, 1, 0x14);
        lower_skill_temp(who, 0xd, rand_mod(0x1e) + 0x1e, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 313, class 2, x 25 (t3 99), virtue 38, base 40; skill 2 +15..29; attr 5 +6..11; attr 0 +4..8; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_6ae8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x139;
    t[1] = 2;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x26;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x26, x, 0x19, 0);
    case 8:
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        lower_skill_temp(who, 2, rand_mod(0xf) + 0xf, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 0, rand_mod(5) + 4, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 28, class 1, x 15 (t3 99), virtue 68, base 65; skills 10,11 +12..26; skill 12 +15..29; skill 13 +10..19; attr 4 +12..23 */
int far sub_s039_165c_6bb8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x1c;
    t[2] = 0xf;
    t[3] = 0x63;
    t[4] = 0x44;
    t[5] = 0x41;
    t[1] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x44, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(0xf) + 0xc, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xf) + 0xc, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0xf) + 0xf, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 4, rand_mod(0xc) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 45, class 7, x 10 (t3 50), virtue 48, base 45; skills 15,13,10 +6..11; attr 5 +10..19; heal attr 0 10%; heal attr 1 10% */
int far sub_s039_165c_6ca6(int who, int x, register int what)
{
    int t[7];

    t[1] = 7;
    t[2] = 0xa;
    t[3] = 0x32;
    t[4] = 0x30;
    t[0] = 0x2d;
    t[5] = 0x2d;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x2d, party[who].f60, 0x30, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0xa);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 0xf, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 202, class 7, x 30 (t3 80), virtue 65, base 40; skill 10 +10..19; attr 5 +8..15; heal attr 0 100%; heal attr 1 20%; 0792(rnd(0xa) + 0xf) */
int far sub_s039_165c_6d92(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xca;
    t[1] = 7;
    t[2] = 0x1e;
    t[3] = 0x50;
    t[4] = 0x41;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x41, x, 0x1e, 0);
    case 8:
        heal_attr_pct(who, 0, 0x64);
        heal_attr_pct(who, 1, 0x14);
        equip_bonuses(who, rand_mod(0xa) + 0xf);
        lower_skill_temp(who, 0xa, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 166, class 6, x 40 (t3 99), virtue 48, base 65; attr 5 +10..19; attr 2 +12..23 */
int far sub_s039_165c_6e62(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xa6;
    t[1] = 6;
    t[2] = 0x28;
    t[3] = 0x63;
    t[4] = 0x30;
    t[5] = 0x41;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x41, party[who].f60, 0x30, x, 0x28, 0);
    case 8:
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 2, rand_mod(0xc) + 0xc, 0x18);
        event_here(0x5d, 1);
        return 0;
    }
    return t[what];
}

/* saint: day 269, class 4, x 35 (t3 99), virtue 44, base 60; skill 17 +8..15, 0792(8..15), attrs 3 +6..11, 4 +8..13, 1 +6..11; local standing +15/+25 at locations 0x44-0x47, 0x49, 0x4a */
int far sub_s039_165c_6f0c(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x10d;
    t[1] = 4;
    t[2] = 0x23;
    t[3] = 0x63;
    t[4] = 0x2c;
    t[5] = 0x3c;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x3c, party[who].f60, 0x2c, x, 0x23, 0);
    case 8:
        lower_skill_temp(who, 17, rand_mod(8) + 8, 0x18);
        equip_bonuses(who, rand_mod(8) + 8);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 4, rand_mod(6) + 8, 0x18);
        raise_attr_temp(who, 1, rand_mod(6) + 6, 0x18);
        what = cur_loc == -1 ? nearest_city(party_x, party_y) : cur_loc;
        switch (what) {
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x49:
        case 0x4a:
            loc_reputation_adj(what, 15, 0x19);
        }
        return 0;
    }
    return t[what];
}

/* saint: day 295, class 0, x 15 (t3 65), virtue 25, base 15; skill 14 +6..11; skill 10 +20..39; attr 5 +6..11; heal attr 0 20% */
int far sub_s039_165c_7038(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x127;
    t[1] = 0;
    t[3] = 0x41;
    t[4] = 0x19;
    t[2] = 0xf;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x19, x, 0xf, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        lower_skill_temp(who, 0xe, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(6) + 6, 0x18);
        event_here(0x4f, 0xa8);
        return 0;
    }
    return t[what];
}

/* saint: day 312, class 7, x 20 (t3 99), virtue 35, base 35; skill 18 +8..15; skill 17 +10..19; attr 2 +8..15 */
int far sub_s039_165c_710a(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x138;
    t[1] = 7;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x23;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x23, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(0xa) + 0xa, 0x18);
        raise_attr_temp(who, 2, rand_mod(8) + 8, 0x18);
        event_here(0x4f, 0xa8);
        return 0;
    }
    return t[what];
}

/* saint: day 157, class 6, x 25 (t3 99), virtue 50, base 55; skill 18 +6..11; skill 10 +25..49; attr 5 +10..19 */
int far sub_s039_165c_71ce(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x9d;
    t[1] = 6;
    t[2] = 0x19;
    t[3] = 0x63;
    t[4] = 0x32;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x32, x, 0x19, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x19) + 0x19, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 311, class 6, x 15 (t3 50), virtue 43, base 55; skill 18 +8..15; skill 10 +20..39; attr 5 +10..19 */
int far sub_s039_165c_7288(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x137;
    t[1] = 6;
    t[2] = 0xf;
    t[3] = 0x32;
    t[4] = 0x2b;
    t[5] = 0x37;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x37, party[who].f60, 0x2b, x, 0xf, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xa) + 0xa, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 273, class 6, x 20 (t3 99), virtue 24, base 35; skills 18,17 +6..11; skill 10 +20..39; attr 5 +8..15; attr 4 +6..11 */
int far sub_s039_165c_7342(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x111;
    t[1] = 6;
    t[2] = 0x14;
    t[3] = 0x63;
    t[4] = 0x18;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x18, x, 0x14, 0);
    case 8:
        lower_skill_temp(who, 0x12, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0x11, rand_mod(6) + 6, 0x18);
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0x14, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 4, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 117, class 8, x 10 (t3 65), virtue 23, base 15; skill 16 +6..11; attr 3 +6..11; heal attr 0 20%; heal attr 1 5% */
int far sub_s039_165c_7432(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x75;
    t[1] = 8;
    t[2] = 0xa;
    t[3] = 0x41;
    t[4] = 0x17;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x17, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        heal_attr_pct(who, 1, 5);
        lower_skill_temp(who, 0x10, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 206, class 7, x 12 (t3 50), virtue 25, base 40; skill 9 +20..34; attr 1 +4..7; attr 5 +3..5; attr 0 +4..7; heal attr 0 20%; heal attr 1 10% */
int far sub_s039_165c_74ea(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xce;
    t[1] = 7;
    t[2] = 0xc;
    t[3] = 0x32;
    t[4] = 0x19;
    t[5] = 0x28;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x28, party[who].f60, 0x19, x, 0xc, 0);
    case 8:
        heal_attr_pct(who, 0, 0x14);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 9, rand_mod(0xf) + 0x14, 0x18);
        raise_attr_temp(who, 1, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 5, rand_mod(3) + 3, 0x18);
        raise_attr_temp(who, 0, rand_mod(4) + 4, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 206, class 7, x 27 (t3 52), virtue 19, base 35; skill 10 +20..39; skill 11 +10..19; skill 12 +40..79; attr 5 +8..15; attr 0 +5..9; heal attr 1 10% */
int far sub_s039_165c_75d8(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xce;
    t[1] = 7;
    t[2] = 0x1b;
    t[3] = 0x34;
    t[4] = 0x13;
    t[5] = 0x23;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x23, party[who].f60, 0x13, x, 0x1b, 0);
    case 8:
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 0xa, rand_mod(0x14) + 0x14, 0x18);
        lower_skill_temp(who, 0xb, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xc, rand_mod(0x28) + 0x28, 0x18);
        raise_attr_temp(who, 5, rand_mod(8) + 8, 0x18);
        raise_attr_temp(who, 0, rand_mod(5) + 5, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 274, class 5, x 10 (t3 60), virtue 10, base 1; skill 10 +4..7; skill 13 +10..19; skill 14 +12..23; skill 18 +4..7; attr 3 +6..11; attr 5 +15..29; attr 0 +6..11; heal attr 0 30%; heal attr 1 10% */
int far sub_s039_165c_76d4(int who, int x, register int what)
{
    int t[7];

    t[0] = 0x112;
    t[1] = 5;
    t[3] = 0x3c;
    t[2] = 0xa;
    t[4] = 0xa;
    t[5] = 1;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(1, party[who].f60, 0xa, x, 0xa, 0);
    case 8:
        heal_attr_pct(who, 0, 0x1e);
        heal_attr_pct(who, 1, 0xa);
        lower_skill_temp(who, 0xa, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0xc) + 0xc, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 3, rand_mod(6) + 6, 0x18);
        raise_attr_temp(who, 5, rand_mod(0xf) + 0xf, 0x18);
        raise_attr_temp(who, 0, rand_mod(6) + 6, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 244, class 5, x 25 (t3 60), virtue 31, base 23; skill 8 +8..15; skill 13 +12..23; heal attr 0 50%; heal attr 1 100% */
int far sub_s039_165c_7812(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xf4;
    t[1] = 5;
    t[2] = 0x19;
    t[3] = 0x3c;
    t[4] = 0x1f;
    t[5] = 0x17;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0x17, party[who].f60, 0x1f, x, 0x19, 0);
    case 8:
        heal_attr_pct(who, 0, 0x32);
        heal_attr_pct(who, 1, 0x64);
        lower_skill_temp(who, 8, rand_mod(8) + 8, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xc) + 0xc, 0x18);
        return 0;
    }
    return t[what];
}

/* saint: day 182, class 7, x 10 (t3 65), virtue 24, base 15; skill 10 +4..7; skill 13 +10..19; skill 14 +12..23; skill 18 +4..7; attr 1,0 +4..8 */
int far sub_s039_165c_78ca(int who, int x, register int what)
{
    int t[7];

    t[0] = 0xb6;
    t[1] = 7;
    t[2] = 0xa;
    t[3] = 0x41;
    t[4] = 0x18;
    t[5] = 0xf;
    t[6] = 1;
    switch (what) {
    case 7:
        return sub_s039_165c_0000(0xf, party[who].f60, 0x18, x, 0xa, 0);
    case 8:
        lower_skill_temp(who, 0xa, rand_mod(4) + 4, 0x18);
        lower_skill_temp(who, 0xd, rand_mod(0xa) + 0xa, 0x18);
        lower_skill_temp(who, 0xe, rand_mod(0xc) + 0xc, 0x18);
        lower_skill_temp(who, 0x12, rand_mod(4) + 4, 0x18);
        raise_attr_temp(who, 1, rand_mod(5) + 4, 0x18);
        raise_attr_temp(who, 0, rand_mod(5) + 4, 0x18);
        return 0;
    }
    return t[what];
}
