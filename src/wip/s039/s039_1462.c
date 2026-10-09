/* place: s039 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: WIP (e2, 2026-10-09): 34/43 functions match (fmatch).  Event-rules toolkit part 1:
 *   helpers used by the card scripts for timed events (F_2e38_1b2c, s029 2c4e/2e74/3b62),
 *   location flags/standings (F_2e38_10be, s029 1a8e/3878/19d0) and party skills.
 *   Not written yet: 11fc (788 bytes, start the type-4 quest: random location near the
 *   current one, kind-8/0x1c/0x2a/0x2b events, marks locations within 40/80 of it), 15c2,
 *   1712 (switch, 806 bytes), 1b0c, 1d2c.
 *   0132: frame/slot order (orig r -8, lim -6, an unused slot at -4; lim stored in the else
 *         branch although dead).
 *   026a: the original evaluates the nested call chain strictly right to left (0b64 result
 *         pushed before the 0084 subtree); 6.00A hoists the embedded assignments `d = ...`
 *         (same issue as s043 150b 1d56/203a).
 *   0504: code identical; fmatch cannot resolve the DS restore (`mov ax, DGROUP`) because
 *         this module has no _DATA segment of its own ("unresolved: segment _DATA").
 *   1c22: bodies identical; 6.00A leaves the default-equivalent cases 0..2 out of the jump
 *         table (orig table 0..11, 6.00A `sub ax,3` 3..11); tried case -1.., break cases.
 *   Numbers 9998/9999/8760 (0x270e/0x270f/0x2238) are event durations in hours, not strings.
 */

struct res { int a; int b; };
struct loc { int type; int f2; int x; int y; int f8; int fa; int fc; int fe; int f10; int rep;
             unsigned char f14; char f15, f16, f17; int f18; int f1a; int f1c; int f1e; int f20; };
extern struct loc far * far F_2e38_10be[];   /* locations (cities ...) */
struct event {                  /* timed events, F_2e38_1b2c[300] (see s029 0e76) */
    int type0;                  /* +00 */
    int f2, f4, f6, f8;
    int hour0, day0, month0, year0;     /* +0a start */
    int hour1, day1, month1, year1;     /* +12 end */
    int f1a, f1c, f1e, f20;
    int kind;                   /* +22 */
    int f24, f26, f28, f2a, f2c, f2e;
};
extern struct event far * far F_2e38_1b2c[];
extern int D_00e4;
extern long D_10be[];
extern char D_270f[];
extern char D_906a;
extern char D_907b;
struct member {                 /* party member, 0x80 bytes */
    char status;                /* 1 = able */
    char pad[0x7f];
};
extern struct member D_9c69[];
extern int D_907e;
extern int D_a67e;
extern int D_e89b;
int far sub_root_0000_0c70();
int far sub_root_0000_1716();
int far sub_s029_0e76_199c();
int far sub_s029_0e76_05ee();
int far sub_s029_0e76_01a0();
int far sub_s029_0e76_01c0();
int far sub_s029_0e76_026a();
int far sub_s029_0e76_1600();
int far sub_s029_0e76_18a8();
int far sub_s029_0e76_02ea();
int far sub_s029_0e76_0a72();
int far sub_s029_0e76_0b64();
int far sub_s029_0e76_1326();
int far sub_s029_0e76_1382();
int far sub_s029_0e76_19d0();
int far sub_s029_0e76_1a8e();
int far sub_s029_0e76_2930();
int far sub_s029_0e76_2c4e();
int far sub_s029_0e76_2e74();
int far sub_s029_0e76_32fe();
int far sub_s029_0e76_3470();
int far sub_s029_0e76_360c();
int far sub_s029_0e76_36d4();
int far sub_s029_0e76_3742();
int far sub_s029_0e76_3878();
int far sub_s029_0e76_3b62();
int far sub_s029_0e76_3e06();
int far sub_s029_1367_0066();
int far sub_s029_1367_0084();
int far sub_s029_1367_0094();
int far sub_s029_1367_086a();
int far sub_s029_1367_08ae();
int far sub_s029_1367_0938();
int far sub_s029_1367_0982();
int far sub_s029_1367_0b1c();
int far vec_1f13();
int far vec_1f77();
int far vec_1fa9();
int far vec_2067();
int far sub_s039_1462_0000();
int far sub_s039_1462_00ba();
int far sub_s039_1462_0102();
void far sub_s039_1462_0132();
void far sub_s039_1462_026a();
int far sub_s039_1462_0470();
int far sub_s039_1462_0504();
void far sub_s039_1462_0558();
int far sub_s039_1462_0a34();
void far sub_s039_1462_0a7e();
int far sub_s039_1462_0b12();
int far sub_s039_1462_0bf6();
int far sub_s039_1462_0d46();
int far sub_s039_1462_0e96();
int far sub_s039_1462_0eaa();
void far sub_s039_1462_0ebe();
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
void far sub_s039_1462_1712();
int far sub_s039_1462_1a38();
int far sub_s039_1462_1a50();
int far sub_s039_1462_1a68();
int far sub_s039_1462_1a80();
int far sub_s039_1462_1a98();
void far sub_s039_1462_1ab0();
void far sub_s039_1462_1b0c();
int far sub_s039_1462_1c22();
void far sub_s039_1462_1cd2();
void far sub_s039_1462_1ce8();
int far sub_s039_1462_1cfe();
void far sub_s039_1462_1d2c();
int far sub_s039_1462_1de6();
int far sub_s039_1462_1e06();
int far sub_s039_1462_1e90();
void far sub_s039_1462_1f00();

/* a price/amount v adjusted for the current location (D_907e): +25% or +50% depending on
 * its kind (s029 1a8e(loc, 0x20) = 1 or 2), +20% each for the location flags 0x13 and 0x12
 * (s029 3878); the result is a random value between lo and v (s029 1367 0066 clamps). */
#pragma optimize("a", on)
int far sub_s039_1462_0000(register int v, int unused, int lo)
{
    register int k;

    k = sub_s029_0e76_1a8e(D_907e, 0x20);
    if (k == 1)
        v = v * 5 / 4;
    if (k == 2)
        v = v * 6 / 4;
    if (sub_s029_0e76_3878(0x13, D_907e))
        v = v * 6 / 5;
    if (sub_s029_0e76_3878(0x12, D_907e))
        v = v * 6 / 5;
    return sub_s029_1367_0066(lo, v, sub_root_0000_0c70(v - lo));
}
#pragma optimize("", on)

/* 1 when the current location (D_907e) is hostile: its reputation (s029 199c) is -10 or
 * lower when flag 0x13 is set, -75 or lower otherwise, or flag 0x11 (s029 3878) is set. */
int far sub_s039_1462_00ba(void)
{
    register int rep;

    rep = sub_s029_0e76_199c(D_907e);
    if (!sub_s029_0e76_3878(0x11, D_907e) && rep > -0x4b &&
        (!sub_s029_0e76_3878(D_907e, 0x13) || rep > -0xa))
        return 0;
    return 1;
}

/* location flags 0x12 (+2) and 0x13 (+1) of the current location (s029 3878) */
int far sub_s039_1462_0102(void)
{
    int v_si;
    v_si = 0;
    if (sub_s029_0e76_3878(0x12, D_907e)) {
        v_si = 2;
    }
    if (sub_s029_0e76_3878(0x13, D_907e)) {
        v_si++;
    }
    return v_si;
}

/* skill practice: member who (-1 = the active member D_907b, -2 = every able member, each
 * rolling separately) improves skill k (s029 01a0/02ea, the 7 bytes at +0x57) with a chance
 * of pct * 1.5 % (mode 1) or pct * 0.5 % (-2) / pct * 3.3 % or pct * 1.1 % (one member),
 * by s029 18a8(skill, lvl + 1, (lvl + 2) * 2), clamped to 1..99. */
void far sub_s039_1462_0132(int who, int k, int mode, int pct, int lvl)
{
    int i;
    int u;
    int lim;
    int r;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        lim = (mode == 1 ? pct * 15 : pct * 5) / 10;
        for (i = 0; i < D_a67e; i++)
            if (D_9c69[i].status == 1 && sub_root_0000_0c70(100) <= lim)
                sub_s029_0e76_02ea(i, k, sub_s029_1367_0066(1, 0x63,
                    sub_s029_0e76_18a8(sub_s029_0e76_01a0(i, k), lvl + 1, (lvl + 2) * 2)));
    } else {
        *(volatile int *)&r = sub_root_0000_0c70(100);
        if ((lim = (mode == 1 ? pct * 33 : pct * 11) / 10) >= r) {
            u = sub_s029_0e76_18a8(sub_s029_0e76_01a0(who, k), lvl + 1, (lvl + 2) * 2);
            u = sub_s029_1367_0066(1, 0x63, u);
            sub_s029_0e76_02ea(who, k, u);
        }
    }
}

/* damage member who (-2 = every present member): attribute 1 (s029 05ee max, 0b64
 * current) loses d = rand(mult * max1 / 40 + 1) - 1, at least a, at most the current value;
 * attribute 0 loses rand(mult * max0 / 20 + 1) - 1, at least d and b, at most its current
 * value (s029 0a72).  Then redraws (root 1716). */
void far sub_s039_1462_026a(int who, int a, int b, int mult)
{
    int i;
    register int d;
    register int s;

    if (who == -2) {
        for (i = 0; i < D_a67e; i++)
            if (D_9c69[i].status) {
                d = sub_s029_1367_0094(d = sub_s029_1367_0084(d = sub_root_0000_0c70((d = mult * (d = sub_s029_0e76_05ee(i, 1)) / 40) + 1) - 1, a), sub_s029_0e76_0b64(i, 1));
                sub_s029_0e76_0a72(i, 1, -d);
                s = sub_s029_1367_0094(s = sub_s029_1367_0084(s = sub_s029_1367_0084(s = sub_root_0000_0c70((s = mult * (s = sub_s029_0e76_05ee(i, 0)) / 20) + 1) - 1, d), b), sub_s029_0e76_0b64(i, 0));
                s = -s;
                sub_s029_0e76_0a72(i, 0, s);
            }
    } else {
        d = sub_s029_1367_0094(d = sub_s029_1367_0084(d = sub_root_0000_0c70((d = mult * (d = sub_s029_0e76_05ee(who, 1)) / 40) + 1) - 1, a), sub_s029_0e76_0b64(who, 1));
        sub_s029_0e76_0a72(who, 1, -d);
        s = sub_s029_1367_0094(s = sub_s029_1367_0084(s = sub_s029_1367_0084(s = sub_root_0000_0c70((s = mult * (s = sub_s029_0e76_05ee(who, 0)) / 20) + 1) - 1, d), b), sub_s029_0e76_0b64(who, 0));
        s = -s;
        sub_s029_0e76_0a72(who, 0, s);
    }
    sub_root_0000_1716(0x100);
}

/* party lore level 1..10: (average of the members' best skill (s029 01c0) + the best of
 * skills 4-6 (s029 026a) + the party averages of skills 9 and 7 (s029 1600)) / 37,
 * -2 for a party of one, -1 for two, +1 for five. */
int far sub_s039_1462_0470(void)
{
    int sum;
    int n;
    register int i;
    register int v;

    n = sum = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i].status) {
            sum += sub_s029_0e76_01c0(i);
            n++;
        }
    if (n == 0)
        n = 1;
    v = sub_s029_0e76_026a() + sum / n;
    v += sub_s029_0e76_1600(9);
    v += sub_s029_0e76_1600(7);
    v /= 37;
    if (n == 1)
        v -= 2;
    else if (n == 2)
        v--;
    else if (n == 5)
        v++;
    return sub_s029_1367_0066(1, 10, v);
}

/* a random location (index 0x5c..0x19d of F_2e38_10be, starting at a random one) of the
 * given type */
int far sub_s039_1462_0504(register int type)
{
    register int i;

    i = sub_root_0000_0c70(0x141) + 0x5c;
    while (F_2e38_10be[i]->type != type)
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

    sub_s029_0e76_1a8e(loc, 0x21);
    sub_s029_0e76_1a8e(loc, 0);
    if (sub_s029_0e76_3470(2, loc) != 1 && sub_s029_0e76_1a8e(loc, 0x20) != 2) {
        sub_s029_0e76_2c4e(-2, loc, 0x5f, 8, 0, loc, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 0xc, 0x63, 2, sub_s029_1367_086a(5), 1, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 6, 0x63, 0, 0x1f8, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 7, 0x63, 0, 0x348, 0, 2);
        d = sub_root_0000_0c70(7);
        while (!sub_s029_0e76_1a8e(loc, d + 3)) {
            d++;
            d %= 7;
        }
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 0x17, 1 << (char)d, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 0x14, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 3, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 5, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 4, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 9, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 0xa, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 0x23, sub_root_0000_0c70(0x64), 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 1, 0, 0, 0x540, 0, 2);
        sub_s029_0e76_2c4e(-2, loc, 0x5a, 0x1c, 2, 0, 0, 0x540, 0, 2);
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 1, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 1, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 2, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 2, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 4, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 4, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 8, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 8, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x10, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x10, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x20, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x20, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x80, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x80, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x40, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x40, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x200, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x200, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x400, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x400, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x800, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x800, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x1000, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x1000, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x2000, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x2000, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, 0x4000, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, 0x4000, 2, 3);
        }
        if (sub_root_0000_0c70(2)) {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 1, -0x8000, 2, 3);
        } else {
            sub_s029_0e76_2e74(-2, loc, 0x1c, 2, -0x8000, 2, 3);
        }
    }
}

/* three-state flag `bit` of the current location: -1 when set in kind-0x1c group 1,
 * 1 in group 2 (s029 3742), else 0 (0a7e sets it) */
int far sub_s039_1462_0a34(int p_6)
{
    int v_di;
    int v_si;
    v_di = sub_s029_0e76_3742(2, 1, D_907e);
    v_si = sub_s029_0e76_3742(2, 2, D_907e);
    v_si &= p_6;
    if (p_6 & v_di) {
        return -1;
    }
    if (v_si) {
        return 1;
    }
    return 0;
}

/* set the three-state flag `bit` of the current location (D_907e; 0a34 reads it) in the
 * kind-0x1c events: mode -1 -> group 1, 1 -> group 2, else neither (s029 2e74: op 2 = and,
 * op 3 = or). */
void far sub_s039_1462_0a7e(register int bit, int mode)
{
    if (mode == -1) {
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 2, ~bit, 2, 2);
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 1, bit, 2, 3);
    } else if (mode == 1) {
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 1, ~bit, 2, 2);
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 2, bit, 2, 3);
    } else {
        bit = ~bit;
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 1, bit, 2, 2);
        sub_s029_0e76_2e74(-2, D_907e, 0x1c, 2, bit, 2, 2);
    }
}

/* number of the 15 location flags (0a34) that are neutral (0) */
int far sub_s039_1462_0b12(void)
{
    int v_si;
    v_si = 0;
    if (!sub_s039_1462_0a34(1)) {
        v_si = 1;
    }
    if (!sub_s039_1462_0a34(2)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(4)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(8)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x10)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x20)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x80)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x40)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x200)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x400)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x800)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x1000)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x2000)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x4000)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(-0x8000)) {
        v_si++;
    }
    return v_si;
}

/* strength of group 2: 2 per flag in group 2 + 1 per neutral flag (11 flags), doubled;
 * 1 when zero */
int far sub_s039_1462_0bf6(void)
{
    int v_si;
    v_si = 0;
    if (sub_s039_1462_0a34(1) == 1) {
        v_si = 2;
    }
    if (sub_s039_1462_0a34(2) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(4) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(8) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x10) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x20) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x80) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x40) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x200) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x400) == 1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x1000) == 1) {
        v_si++;
        v_si++;
    }
    if (!sub_s039_1462_0a34(1)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(2)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(4)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(8)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x10)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x20)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x80)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x40)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x200)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x400)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x1000)) {
        v_si++;
    }
    if (v_si) {
        return v_si << 1;
    }
    return 1;
}

/* the same for group 1 (flags == -1) */
int far sub_s039_1462_0d46(void)
{
    int v_si;
    v_si = 0;
    if (sub_s039_1462_0a34(1) == -1) {
        v_si = 2;
    }
    if (sub_s039_1462_0a34(2) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(4) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(8) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x10) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x20) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x80) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x40) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x200) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x400) == -1) {
        v_si++;
        v_si++;
    }
    if (sub_s039_1462_0a34(0x1000) == -1) {
        v_si++;
        v_si++;
    }
    if (!sub_s039_1462_0a34(1)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(2)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(4)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(8)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x10)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x20)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x80)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x40)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x200)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x400)) {
        v_si++;
    }
    if (!sub_s039_1462_0a34(0x1000)) {
        v_si++;
    }
    if (v_si) {
        return v_si << 1;
    }
    return 1;
}

/* group 1 strength in percent of group 2 */
int far sub_s039_1462_0e96(void)
{
    int v_si;
    v_si = sub_s039_1462_0bf6();
    return sub_s039_1462_0d46() * 0x64 / v_si;
}

/* group 2 strength in percent of group 1 */
int far sub_s039_1462_0eaa(void)
{
    int v_si;
    v_si = sub_s039_1462_0d46();
    return sub_s039_1462_0bf6() * 0x64 / v_si;
}

/* outcome of a fight/conflict at the current location: side a and c (+1/-1), b the result.
 * With c == 0 a chance from the strength ratio (0eaa) is computed (unused) and a random
 * number drawn.  Then time passes (s029 2930), event kind 2 of the location is cleared
 * (s029 3e06), the location gets flag 0x80 (+0x14), and the location's two standings are
 * changed by s029 19d0 depending on a/b/c and whether events 0x24/0x25 (s029 360c) hold;
 * winning without them pays a reward (1e06(3) or 1e06(1)). */
void far sub_s039_1462_0ebe(int a, int b, register int c)
{
    register int pct;

    if (c == 0) {
        if (sub_s039_1462_0eaa() >= 200)
            pct = 0;
        else if (sub_s039_1462_0eaa() >= 150)
            pct = 15;
        else if (sub_s039_1462_0eaa() >= 120)
            pct = 35;
        else if (sub_s039_1462_0eaa() <= 50)
            pct = 100;
        else if (sub_s039_1462_0eaa() <= 67)
            pct = 85;
        else if (sub_s039_1462_0eaa() <= 83)
            pct = 65;
        else
            pct = 50;
        if (sub_root_0000_0c70(100) < pct)
            pct = 0;
        else
            pct = 0;
    }
    sub_s029_0e76_2930(0x13, 0x2a0);
    sub_s029_0e76_3e06(2, D_907e);
    F_2e38_10be[D_907e]->f14 |= 0x80;
    if (c == -1 && a == -1) {
        if (b == 1) {
            if (sub_s029_0e76_360c(2, 0x24, D_907e))
                sub_s029_0e76_19d0(D_907e, -10, -30);
            else
                sub_s029_0e76_19d0(D_907e, -30, -60);
        } else if (b == -1) {
            if (sub_s029_0e76_360c(2, 0x25, D_907e))
                sub_s029_0e76_19d0(D_907e, -30, -70);
            else {
                sub_s039_1462_1e06(3);
                sub_s029_0e76_19d0(D_907e, 25, 70);
            }
        }
    } else if (c == 1 && a == 1) {
        if (b == -1) {
            if (sub_s029_0e76_360c(2, 0x25, D_907e))
                sub_s029_0e76_19d0(D_907e, -10, -30);
            else
                sub_s029_0e76_19d0(D_907e, -30, -60);
        } else if (b == 1) {
            if (sub_s029_0e76_360c(2, 0x24, D_907e))
                sub_s029_0e76_19d0(D_907e, -30, -70);
            else {
                sub_s039_1462_1e06(3);
                sub_s029_0e76_19d0(D_907e, 25, 70);
            }
        }
    } else if (c == 1 && a == -1) {
        sub_s029_0e76_2930(0x46, 0xa8);
        if (b == 1) {
            if (sub_s029_0e76_360c(2, 0x24, D_907e))
                sub_s029_0e76_19d0(D_907e, -10, -30);
            else {
                sub_s039_1462_1e06(1);
                sub_s029_0e76_19d0(D_907e, 15, 25);
            }
        } else if (b == -1) {
            if (sub_s029_0e76_360c(2, 0x25, D_907e))
                sub_s029_0e76_19d0(D_907e, -20, -60);
            else
                sub_s029_0e76_19d0(D_907e, -15, -35);
        }
    } else if (c == -1 && a == 1) {
        sub_s029_0e76_2930(0x45, 0xa8);
        if (b == -1) {
            if (sub_s029_0e76_360c(2, 0x25, D_907e))
                sub_s029_0e76_19d0(D_907e, -10, -30);
            else {
                sub_s039_1462_1e06(1);
                sub_s029_0e76_19d0(D_907e, 15, 25);
            }
        } else if (b == 1) {
            if (sub_s029_0e76_360c(2, 0x24, D_907e))
                sub_s029_0e76_19d0(D_907e, -20, -60);
            else
                sub_s029_0e76_19d0(D_907e, -15, -35);
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
    i = sub_s029_0e76_3b62(8, -1, b, -1, 3, -1);
    if (i != -1) {
        sub_s029_0e76_2c4e(who, b, 0x5f, 8, c, D_907e, 0, 9998, 0, 3);
        if (F_2e38_1b2c[i]->f2a < v)
            F_2e38_1b2c[i]->f2a = v;
        if (F_2e38_1b2c[i]->f2c < lvl)
            F_2e38_1b2c[i]->f2c = lvl;
        return 1;
    }
    ev = F_2e38_1b2c[sub_s029_0e76_2c4e(who, b, 0x5f, 8, c, D_907e, 0, 9998, 0, 3)];
    ev->f2a = v;
    ev->f2c = lvl;
    ev->f2e = x;
    sub_s029_0e76_2c4e(-2, D_e89b, 0, 0x1c, 2, 0, 0, 9998, 0, 3);
    sub_s029_0e76_2c4e(-2, D_e89b, 0, 0x1c, 3, 0, 0, 9998, 0, 3);
    sub_s029_0e76_2c4e(-2, D_e89b, 0, 0x1c, 4, 0, 0, 9998, 0, 3);
    sub_s029_0e76_2c4e(-2, D_e89b, 0, 0x2b, 0, 1, 0, 9999, 0, 0);
}
#pragma optimize("", on)

/* sub_s039_1462_11fc: not decompiled: memory via unknown si */
void far sub_s039_1462_11fc(void) { }

/* the active kind-8 event with type 4 (s029 3b62; the quest of 11fc): its f2e, f0, f1c
 * (each returns -1 when there is none) */
int far sub_s039_1462_1510(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(8, -1, -1, -1, 4, -1)) != -1)
        return F_2e38_1b2c[i]->f2e;
}

int far sub_s039_1462_153e(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(8, -1, -1, -1, 4, -1)) != -1)
        return F_2e38_1b2c[i]->type0;
}

int far sub_s039_1462_156c(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(8, -1, -1, -1, 4, -1)) != -1)
        return F_2e38_1b2c[i]->f1c;
}

/* the quest location (156c) gets type 0x62 (+0xc) and its event kind 4 is cleared */
void far sub_s039_1462_159a(void)
{
    F_2e38_10be[sub_s039_1462_156c()]->fc = 0x62;
    sub_s029_0e76_3e06(4, D_907e);
}

/* sub_s039_1462_15c2: not decompiled: es:[reg] unknown es=farslot(32470, 11832) ptr=None */
void far sub_s039_1462_15c2(void) { }

/* the same for the kind-8 event with type 12: f0, f2e */
int far sub_s039_1462_16b4(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(8, -1, -1, -1, 12, -1)) != -1)
        return F_2e38_1b2c[i]->type0;
}

int far sub_s039_1462_16e2(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(8, -1, -1, -1, 12, -1)) != -1)
        return F_2e38_1b2c[i]->f2e;
}

/* empty */
void far sub_s039_1462_1710(void)
{
}

/* sub_s039_1462_1712: not decompiled: terminator table */
void far sub_s039_1462_1712(void) { }

/* fields of timed event p_6 (F_2e38_1b2c): f1a, f1e, f1c, f2e, type0 */
int far sub_s039_1462_1a38(int p_6)
{
    return F_2e38_1b2c[p_6]->f1a;
}

int far sub_s039_1462_1a50(int p_6)
{
    return F_2e38_1b2c[p_6]->f1e;
}

int far sub_s039_1462_1a68(int p_6)
{
    return F_2e38_1b2c[p_6]->f1c;
}

int far sub_s039_1462_1a80(int p_6)
{
    return F_2e38_1b2c[p_6]->f2e;
}

int far sub_s039_1462_1a98(int p_6)
{
    return F_2e38_1b2c[p_6]->type0;
}

/* when event kind 10 of the current location holds (s029 3470): the kind-8 event of type 10
 * with f1c b gives its f2e to the active member (vec_2067) and the kind-10 event of b ends */
void far sub_s039_1462_1ab0(int b)
{
    register int i;

    if (sub_s029_0e76_3470(10, D_907e)) {
        i = sub_s029_0e76_3b62(8, -1, b, -1, 10, -1);
        vec_2067(D_907b, F_2e38_1b2c[i]->f2e);
        sub_s029_0e76_3e06(10, b);
    }
}

/* sub_s039_1462_1b0c: not decompiled: es:[reg] unknown es=farslot(32472, 11832) ptr=None */
void far sub_s039_1462_1b0c(void) { }

/* a random day of the year near the current month (D_00e4): equinoxes/solstices and
 * feast days (80, 103, 145, 162, 172, 265, 304, 360) */
int far sub_s039_1462_1c22(void)
{
    switch (D_00e4 + 1) {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        if (!sub_root_0000_0c70(2))
            return 0x91;
        return 0x67;
    case 4:
        if (!sub_root_0000_0c70(2))
            return 0xa2;
        return 0x91;
    case 5:
        if (sub_root_0000_0c70(3))
            return 0xac;
        return 0xa2;
    case 6:
    case 7:
    case 8:
        if (sub_root_0000_0c70(2))
            return 0x109;
        return 0x130;
    case 9:
        if (sub_root_0000_0c70(2))
            return 0x130;
        return 0x168;
    case 10:
    case 11:
        if (sub_root_0000_0c70(2))
            return 0x168;
        return 0x50;
    }
    if (!sub_root_0000_0c70(2))
        return 0x67;
    return 0x50;
}

/* day of month / month (s029 1367 0982/0938) of the day-of-year value of event
 * kind 1 type 4 (s029 36d4) */
void far sub_s039_1462_1cd2(void)
{
    sub_s029_1367_0982(sub_s029_0e76_36d4(1, 4));
}

void far sub_s039_1462_1ce8(void)
{
    sub_s029_1367_0938(sub_s029_0e76_36d4(1, 4));
}

/* f2a of the kind-0x1c event of type 4 with f28 1 (-1 if none) */
int far sub_s039_1462_1cfe(void)
{
    register int i;

    if ((i = sub_s029_0e76_3b62(0x1c, 4, -1, -1, 1, -1)) != -1)
        return F_2e38_1b2c[i]->f2a;
}

/* sub_s039_1462_1d2c: not decompiled: does not compile */
void far sub_s039_1462_1d2c(void) { }

/* number of present party members (of the 5 slots) */
int far sub_s039_1462_1de6(void)
{
    int i;
    int n;
    struct member *p;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i].status)
            n++;
    return n;
}

/* reward of size k (0..5: 0, 3, 10, 25, 64, 200 florins), 2/3 at difficulty D_906a 0,
 * 3/2 at 2; added to the party money (s029 1326/1382 what 4); returns the amount */
int far sub_s039_1462_1e06(int k)
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
    if (D_906a == 0)
        v = v * 2 / 3;
    else if (D_906a != 1 && D_906a == 2)
        v = v * 3 / 2;
    sub_s029_0e76_1382(4, v = sub_s029_0e76_1326(4) + v);
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
    v *= sub_s029_1367_0084(1, sub_s029_0e76_1326(4) / 200);
    v -= sub_s029_0e76_1326(4);
    sub_s029_0e76_1382(4, v);
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

    if (!sub_s029_0e76_32fe(7)) {
        x = sub_root_0000_0c70(0x20) + 0x8b;
        y = sub_root_0000_0c70(0x88);
        i = sub_s029_0e76_2c4e(-2, D_907e, 0x5f, 8, 0, 0, 0, 9999, 0, 7);
        F_2e38_1b2c[i]->f2a = y;
        F_2e38_1b2c[i]->f2c = 2;
        F_2e38_1b2c[i]->f2e = x;
        sub_s029_0e76_2c4e(-2, D_907e, 0x5f, 0x1c, 5, 0, 0, 9999, 0, 7);
    }
}
