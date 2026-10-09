/* place: s010 1432:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=1536 */
/* status (e6, 2026-10-09): started from build/agent/gen_engine; ALL 18 functions written, 9 match.
 * City battlefields, same scheme as s011 (up to 2 level buffers of 0x33fc bytes at D_e03f
 * based on D_e03d, saved as level%d.flr).
 * Match: 08aa (street battle setup: hash-ordered local names, `g1 = g2 = g3 = -1` chains
 * reversed, gotos for the shared 0x16/0x17 arms of the first switch), 0000 (driver: buffer name through a union local -> `mov ax,1536h; push ax` and the
 * 2-byte frame), 0102 (location -> battlefield type, nested random switches falling through),
 * 0292, 02cc, 058c, 079a (tile descriptions: struct based pointer + pragma "a"), 114e, 2de0.
 * Close: 11aa 1516 1b22 2030 26f8 2b42 (register/slot choices), 1abc (6.00 uses `loop`), 2a2c,
 * 2526. 02cc flips operand order when symbols are declared before it - keep new struct tags /
 * externs below it (struct exit, group, mon are declared further down for that reason).
 * Level: party start 0x1f4c/0x1f4e (+0x1f50 facing, 0x1f51), 14-byte records at 0x1f54 (count
 * 0x1f52), objects at 0x2186 (count 0x2184; group markers type 0x65 with id at x0a), 0x2488,
 * exits at 0x2fb8 (count 0x2fb6), raised rectangles at 0x3328 (count 0x3326).
 */

struct res { int a; int b; };
struct obj {                            /* map object, 14 bytes */
    int x0, x2;
    unsigned char x4, x5;
    int type;                           /* 06 */
    int x8;
    unsigned char x0a;
    char x0b;
    int x0c;
};
struct r14 {                            /* 14 bytes at level+0x1f54 */
    char p0[6];
    char x6;                            /* 06: id */
    char x7;                            /* 07 */
    char p8[4];
    char x0c;                           /* 0c: index of the partner record */
    char p0d;
};
struct level {                          /* battle map (0x33fc bytes), see s011 */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41[0x1f4c - 0x1f41];
    int x1f4c;                          /* 1f4c: party start x */
    int x1f4e;                          /* 1f4e: party start y */
    char x1f50;
    char x1f51;
    char n1f52;                         /* 1f52 */
    char p1f53;
    struct r14 r1f54[(0x2184 - 0x1f54) / 14];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char x2488;                         /* 2488 */
    char p2489[0x2fb6 - 0x2489];
    char nexit;                         /* 2fb6 */
    char p2fb7;
    char ex[6][14];                     /* 2fb8: exits (struct exit) */
    char p300c[0x3326 - 0x300c];
    char n3326;                         /* 3326: raised rectangles */
    char p3327;
    int r3328[21][5];                   /* 3328: rectangles {x, y, w, h} in pixels */
    char p33fa[2];
};
static char D_1536[] = "Ct";            /* buffer name */
extern unsigned D_e03b;
extern _segment D_e03d;
extern struct level _based(D_e03d) *D_e03f;
extern _segment D_dadd;
struct bfdesc { char b[0xc0]; };
extern struct bfdesc _based(D_dadd) *D_db33;
#define B(o) (((char _based(D_dadd) *)D_db33)[o])
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
int far _fstrcpy();
int far sprintf();
int far sub_root_05ba_0004();
int far sub_root_05ba_00c2();
int far sub_root_05d9_019a();
int far sub_s004_0e76_20f6();
int far sub_s004_1091_000a();
int far sub_s004_0e76_06aa();
int far sub_s004_1262_0d0a();
int far sub_s010_1432_0000();
int far sub_s010_1432_0102();
void far sub_s010_1432_0292();
void far sub_s010_1432_02cc();
void far sub_s010_1432_058c();
void far sub_s010_1432_079a();
int far sub_s010_1432_08aa();
int far sub_s010_1432_114e();
void far sub_s010_1432_11aa();
int far sub_s010_1432_1516();
void far sub_s010_1432_1abc();
int far sub_s010_1432_1b22();
void far sub_s010_1432_2030();
int far sub_s010_1432_2526();
void far sub_s010_1432_26f8();
int far sub_s010_1432_2a2c();
void far sub_s010_1432_2b42();
void far sub_s010_1432_2de0();

/* city battle: build the battlefield of type t = sub_0102(p_6) into the level buffers */
int far sub_s010_1432_0000(int p_6, int *res, int p_a, int p_c, int p_e, int p_10, int p_12,
    int p_14, int p_16, int p_18, int p_1a)
{
    int t;
    union { char *p; int i; } q;
    int r;

    D_e03d = (_segment)(D_e03b = alloc_buffer(0x67f8L, q.p = D_1536));
    D_e03f = 0;
    t = sub_s010_1432_0102(p_6);
    sub_s010_1432_0292(t);
    switch (t) {
    case 7:
        r = sub_s010_1432_2526(p_6, t, p_a, p_c, p_e, p_10, p_12, p_14, p_16, p_18, p_1a);
        *res = 0;
        break;
    case 8:
        r = sub_s010_1432_1516(p_6, t, p_a, p_c, p_e, p_10, p_12, p_14, p_16, p_18, p_1a);
        *res = 0;
        break;
    case 9:
        r = sub_s010_1432_1b22(p_6, t, res, p_a, p_c, p_e, p_10, p_12, p_14, p_16, p_18, p_1a);
        break;
    default:
        r = sub_s010_1432_08aa(p_6, t, p_a, p_c, p_e, p_10, p_12, p_14, p_16, p_18, p_1a);
        *res = 0;
        break;
    }
    free_buffer(D_e03b, D_1536);
    return r;
}

/* battlefield type (1..9) for city location type p; several types pick one at random */
int far sub_s010_1432_0102(int p)
{
    switch (p) {
    default:
        switch (sub_s004_0e76_20f6(1, 4)) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 3;
        case 4: return 4;
        }
    case 0x20:
        switch (sub_s004_0e76_20f6(1, 2)) {
        case 1: return 1;
        case 2: return 2;
        }
    case 0x13:
    case 0x14:
    case 0x1e:
        switch (sub_s004_0e76_20f6(1, 3)) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 3;
        }
    case 0x22:
    case 0x23:
    case 0x25:
        switch (sub_s004_0e76_20f6(1, 3)) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 4;
        }
    case 0x11:
    case 0x12:
        switch (sub_s004_0e76_20f6(1, 2)) {
        case 1: return 3;
        case 2: return 4;
        }
    case 0x18:
    case 0x19:
    case 0x1a:
        switch (sub_s004_0e76_20f6(1, 3)) {
        case 1: return 2;
        case 2: return 3;
        case 3: return 4;
        }
    case 0x1b:
    case 0x24:
    case 0x26:
        return 5;
    case 0x15:
    case 0x16:
        return 7;
    case 0x1c:
    case 0x1d:
    case 0x29:
    case 0x2a:
    case 0x6a:
        return 9;
    case 0x1f:
    case 0x21:
    case 0x2c:
    case 0x2d:
    case 0x6b:
        return 8;
    case 0x2b:
        return 6;
    }
}

/* tile description for battlefield type t */
void far sub_s010_1432_0292(int t)
{
    switch (t) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        sub_s010_1432_02cc(t);
        break;
    case 7:
        sub_s010_1432_079a(t);
        break;
    case 8:
    case 9:
        sub_s010_1432_058c(t);
        break;
    }
}

/* tile description for the street battlefields (t 1..6): floor, random wall set v */
#pragma optimize("a", on)
void far sub_s010_1432_02cc(int t)
{
    char city[5] = "city";
    char name[20];
    int v, i;

    B(0) = 3;
    _fstrcpy((char far *)&B(1), (char far *)city);
    switch (t) {
    case 1:
    case 5:
    case 6:
        switch (sub_s004_0e76_20f6(1, 2)) {
        case 1: v = 0; break;
        case 2: v = 6; break;
        }
        break;
    case 2:
    case 3:
        switch (sub_s004_0e76_20f6(1, 5)) {
        case 1: v = 0; break;
        case 2: v = 2; break;
        case 3: v = 4; break;
        case 4: v = 6; break;
        case 5: v = 8; break;
        }
        break;
    case 4:
        switch (sub_s004_0e76_20f6(1, 3)) {
        case 1: v = 2; break;
        case 2: v = 4; break;
        case 3: v = 8; break;
        }
        break;
    }
    B(0x0b) = v;
    B(0x0c) = v + 1;
    _fstrcpy((char far *)&B(0x1a), (char far *)city);
    B(0x25) = 0;
    B(0x26) = 16;
    B(0x27) = 17;
    B(0x28) = 18;
    B(0x29) = 19;
    switch (t) {
    case 1:
    case 2:
    case 3:
    case 4:
        v = sub_s004_0e76_20f6(1, 5) - 1;
        break;
    case 5:
        v = sub_s004_0e76_20f6(1, 2) + 2;
        break;
    case 6:
        v = 5;
        break;
    }
    sprintf(name, "citynw1%1d", v);
    _fstrcpy((char far *)&B(0x34), (char far *)name);
    sprintf(name, "cityww1%1d", v);
    _fstrcpy((char far *)&B(0x67), (char far *)name);
    switch (v) {
    case 1:
        B(0x3e) = 2;
        B(0x3f) = 1;
        B(0x40) = 4;
        B(0x41) = 0;
        B(0x42) = 3;
        break;
    case 3:
        B(0x3e) = 4;
        B(0x3f) = 2;
        B(0x40) = 1;
        B(0x41) = 3;
        B(0x42) = 0;
        break;
    case 4:
        B(0x3e) = 1;
        B(0x3f) = 4;
        B(0x40) = 2;
        B(0x41) = 3;
        B(0x42) = 0;
        break;
    case 5:
        B(0x45) = 6;
        B(0x46) = 7;
    case 0:
    case 2:
        B(0x3e) = 2;
        B(0x3f) = 0;
        B(0x40) = 1;
        B(0x41) = 3;
        B(0x42) = 4;
        break;
    }
    B(0x47) = 5;
    for (i = 1; i <= 10; i++)
        B(0x70 + i) = B(0x3d + i);
    _fstrcpy((char far *)&B(0x9a), (char far *)city);
    B(0xa5) = 0;
    B(0xa6) = 1;
    B(0xa7) = 2;
    B(0xa8) = 3;
    B(0xa9) = 5;
    B(0xab) = 9;
    B(0xa3) = 7;
    B(0xaa) = 4;
}
#pragma optimize("", on)

/* tile description for the keep battlefields (t 8, 9) */
#pragma optimize("a", on)
void far sub_s010_1432_058c(int t)
{
    char keep[5] = "keep";
    char keep1[6] = "keep1";

    B(0) = 0;
    _fstrcpy((char far *)&B(1), (char far *)keep);
    if (t == 9)
        B(0x0b) = 1;
    else
        B(0x0b) = 0;
    _fstrcpy((char far *)&B(0x1a), (char far *)keep);
    B(0x23) = 2;
    B(0x2a) = 10;
    B(0x2b) = 11;
    B(0x2c) = 9;
    _fstrcpy((char far *)&B(0x34), (char far *)keep1);
    if (t == 9) {
        B(0x48) = 0;
        B(0x44) = 6;
        B(0x45) = 8;
        B(0x46) = 9;
    } else
        B(0x48) = 1;
    B(0x43) = 2;
    _fstrcpy((char far *)&B(0x4d), (char far *)keep);
    if (t == 9) {
        B(0x58) = 0;
        B(0x56) = 2;
        B(0x59) = 8;
        B(0x5a) = 14;
        B(0x5b) = 10;
    }
    _fstrcpy((char far *)&B(0x67), (char far *)keep1);
    if (t == 9) {
        B(0x7b) = 0;
        B(0x77) = 6;
    } else
        B(0x7b) = 1;
    B(0x76) = 5;
    _fstrcpy((char far *)&B(0x80), (char far *)keep);
    if (t == 9) {
        B(0x8b) = 0;
        B(0x89) = 2;
    }
    _fstrcpy((char far *)&B(0x9a), (char far *)keep);
    B(0xa5) = 5;
    B(0xa6) = 3;
    B(0xa7) = 11;
    B(0xa8) = 2;
    B(0xa9) = 0;
    B(0xaa) = 1;
    B(0xab) = 8;
    B(0xac) = 9;
    B(0xad) = 17;
    B(0xae) = 16;
    B(0xaf) = 4;
    B(0xa3) = 10;
    B(0xb0) = 10;
    B(0xb1) = 12;
}
#pragma optimize("", on)

/* tile description for the outdoor battlefield (t 7) */
#pragma optimize("a", on)
void far sub_s010_1432_079a(int t)
{
    char out[4] = "out";

    B(0) = 1;
    _fstrcpy((char far *)&B(1), (char far *)out);
    B(0x0b) = 0;
    B(0x0c) = 1;
    B(0x0d) = 2;
    B(0x0e) = 9;
    _fstrcpy((char far *)&B(0x1a), (char far *)out);
    B(0x23) = 2;
    B(0x2d) = 18;
    B(0x2e) = 19;
    B(0x2f) = 20;
    B(0x30) = 21;
    B(0x31) = 22;
    B(0x32) = 23;
    _fstrcpy((char far *)&B(0x34), (char far *)out);
    B(0x48) = 3;
    _fstrcpy((char far *)&B(0x67), (char far *)out);
    B(0x7b) = 3;
    _fstrcpy((char far *)&B(0x9a), (char far *)"outgrpa");
    B(0xa5) = 7;
    B(0xa6) = 8;
    B(0xa7) = 9;
    B(0xa3) = 4;
    B(0xa8) = 30;
    B(0xa9) = 31;
    B(0xaa) = 32;
    B(0xab) = 33;
}
#pragma optimize("", on)

extern unsigned char D_9080;
int far sub_s010_1432_114e(char, char, char);

/* street battlefields (types 1..6): load prepared map icity.<t-1><nn> (random variant of the
 * type, 4/2/2/3/2/1 variants), random wall/floor variation, party start at a random group
 * marker; per location p6 the enemy groups are placed at fixed group markers.
 * v = (t-1)*100 + variant; f7a/f4/f1c: random/location flags combined by sub_114e. */
#pragma optimize("a", on)
int far sub_s010_1432_08aa(int p6, int t, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    int nvo[10];
    char namef[0x50];
    struct obj far *og;
    int nd;
    char f4g, f1cj, f7am;
    int g2f, g1g, g3f;
    int ag, bg, cg, di;
    int vh, rc, x12a;

    nvo[0] = 0;
    nvo[1] = 4;
    nvo[4] = 3;
    nvo[2] = 2;
    nvo[3] = 2;
    nvo[5] = 2;
    nvo[6] = 1;
    nvo[7] = 1;
    nvo[8] = 1;
    nvo[9] = 1;
    vh = sub_s004_0e76_20f6(1, nvo[t]) - 1;
    vh += (t - 1) * 100;
    sprintf(namef, "icity.%03d", vh);
    sub_root_05d9_019a(namef, (struct level far *)D_e03f);
    if (t == 6)
        rc = 0;
    else
        rc = sub_s004_0e76_20f6(1, 8) - 1;
    sub_s004_1091_000a((struct level far *)D_e03f, rc, 1);
    D_e03f->x1f40 = 0;
    sub_s010_1432_11aa(t);
    f7am = sub_s004_0e76_20f6(1, 2) == 1 ? 1 : 0;
    switch (p6) {
    default:
        if (f7am)
            goto l16;
    l17:
        ag = 0x17;
        break;
    case 0x1b:
    case 0x2b:
        ag = 0x15;
        break;
    case 0x1e:
        if (f7am) {
            if (vh == 1 || vh == 0xc9)
                goto l17;
        } else {
            if (vh != 0xc8)
                goto l17;
        }
    case 0x24:
    case 0x26:
    l16:
        ag = 0x16;
        break;
    case 0x20:
        if (f7am)
            goto l16;
        goto l17;
    }
    ag = sub_s010_1432_2a2c(0, ag, &nd);
    og = &D_e03f->obj[ag];
    D_e03f->x1f4c = og->x0 + 16;
    D_e03f->x1f4e = og->x2 + 8;
    D_e03f->x1f50 = 0;
    D_e03f->x1f51 = og->x0c;
    x12a = 0;
    if (p6 == 0x20) {
        x12a = fq3;
        fq3 = fq2 - 1;
    }
    g1g = g2f = g3f = -1;
    if (ft1 >= 0 && fn1 > 0)
        g1g = sub_s004_0e76_06aa(0, ft1, fq1);
    if (ft2 >= 0 && fn2 > 0)
        g2f = sub_s004_0e76_06aa(0, ft2, fq2);
    if (ft3 >= 0 && fn3 > 0)
        g3f = sub_s004_0e76_06aa(0, ft3, fq3);
    f1cj = 0;
    switch (p6) {
    case 0x11:
    case 0x13:
    case 0x15:
    case 0x18:
    case 0x1a:
    case 0x56:
        f1cj = 1;
    }
    f4g = sub_s004_0e76_20f6(1, 2) == 1 ? 1 : 0;
    ag = bg = cg = di = -1;
    switch (p6) {
    default:
        sub_s010_1432_2b42(p6, 0, 0, ag = sub_s010_1432_2a2c(0, ag = sub_s010_1432_114e(f7am, f4g, f1cj), &nd),
            -1, 0, 0, 1, 1, g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    case 0x1b:
        ag = sub_s010_1432_2a2c(0, 2, &nd);
        bg = sub_s010_1432_2a2c(0, 3, &nd);
        cg = sub_s010_1432_2a2c(0, 4, &nd);
        di = sub_s010_1432_2a2c(0, 5, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, -1, 0, 0, 1, 1, -1, 0, g2f, 1, -1, 0);
        sub_s010_1432_2b42(p6, 0, 0, bg, -1, 0, 0, 0, 1, -1, 0, g1g, 1, -1, 0);
        sub_s010_1432_2b42(p6, 0, 0, cg, di, 2, 8, 0, 1, -1, 0, g1g, fn1 - 1, g3f, fn3);
        break;
    case 0x1e:
        if (f7am) {
            switch (vh) {
            case 0: ag = 4; break;
            case 1: ag = 2; break;
            case 2: ag = 10; break;
            case 3: ag = 8; break;
            case 100: ag = 6; break;
            case 101: ag = 5; break;
            case 200: ag = 10; break;
            case 201: ag = 10; break;
            }
        } else {
            switch (vh) {
            case 0: ag = 9; break;
            case 1: ag = 2; break;
            case 2: ag = 10; break;
            case 3: ag = 3; break;
            case 100: ag = 2; break;
            case 101: ag = 8; break;
            case 200: ag = 10; break;
            case 201: ag = 10; break;
            }
        }
        sub_s010_1432_2b42(p6, 0, 0, ag = sub_s010_1432_2a2c(0, ag, &nd),
            -1, 0, 0, 1, 1, g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    case 0x20:
        if (f7am) {
            di = 11;
            switch (vh) {
            case 2: ag = 4; bg = 10; break;
            case 3: ag = 5; bg = 9; break;
            default: ag = 4; bg = 5; break;
            }
        } else {
            di = 12;
            ag = 8;
            bg = 10;
            switch (vh) {
            case 0: bg = 9; break;
            case 3: ag = 3; bg = 9; break;
            case 100: ag = 2; bg = 8; break;
            case 101: ag = 5; bg = 8; break;
            }
        }
        ag = sub_s010_1432_2a2c(0, ag, &nd);
        bg = sub_s010_1432_2a2c(0, bg, &nd);
        di = sub_s010_1432_2a2c(0, di, &nd);
        sub_s010_1432_2b42(p6, 0, 0, bg, -1, 0, 0, 0, 1, g1g, fn1 / 2, g2f, fn2 / 2,
            g3f, fn3 / 2);
        sub_s010_1432_2b42(p6, 0, 0, ag, di, 4, x12a, 1, 1, g1g, fn1 -= fn1 / 2, g2f, fn2 -= fn2 / 2,
            g3f, fn3 -= fn3 / 2);
        break;
    case 0x22:
        ag = sub_s010_1432_2a2c(0, ag = sub_s010_1432_114e(f7am, f4g, f1cj), &nd);
        if (f7am)
            bg = sub_s010_1432_2a2c(0, 11, &nd);
        else
            bg = sub_s010_1432_2a2c(0, 12, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, bg, 4, 8, 1, 1, g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    case 0x23:
        if (f7am) {
            ag = 5;
            di = 13;
            switch (vh) {
            case 0: ag = 7; break;
            case 2: ag = 4;
            case 3: di = 11; break;
            case 100: ag = 4; break;
            case 302: ag = 10; break;
            }
        } else {
            ag = 10;
            di = 14;
            switch (vh) {
            case 0: ag = 3; break;
            case 1: ag = 2; break;
            case 100: ag = 9; break;
            case 101: ag = 5; break;
            case 302: ag = 9; break;
            }
        }
        ag = sub_s010_1432_2a2c(0, ag, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, di = sub_s010_1432_2a2c(0, di, &nd), 3, fq3, 1, 1,
            g1g, fn1, g2f, fn2, -1, 0);
        break;
    case 0x24:
        ag = sub_s010_1432_2a2c(0, 6, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, di = sub_s010_1432_2a2c(0, 7, &nd), 3, fq3, 1, 1,
            g1g, fn1, g2f, fn2, -1, 0);
        break;
    case 0x25:
        if (f7am) {
            ag = 5;
            di = 13;
            switch (vh) {
            case 0: ag = 7; break;
            case 2: ag = 4;
            case 3: di = 11; break;
            case 100: ag = 4; break;
            case 302: ag = 10; break;
            }
        } else {
            ag = 10;
            di = 14;
            switch (vh) {
            case 0: ag = 3; break;
            case 1: ag = 2; break;
            case 100: ag = 9; break;
            case 101: ag = 5; break;
            case 302: ag = 9; break;
            }
        }
        ag = sub_s010_1432_2a2c(0, ag, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, di = sub_s010_1432_2a2c(0, di, &nd), 4, 8, 1, 1,
            g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    case 0x26:
        if (vh == 400)
            ag = 6;
        else
            ag = 4;
        ag = sub_s010_1432_2a2c(0, ag, &nd);
        sub_s010_1432_2b42(p6, 0, 0, ag, di = sub_s010_1432_2a2c(0, 5, &nd), 4, 12, 1, 1,
            g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    case 0x2b:
        sub_s010_1432_2b42(p6, 0, 0, ag = sub_s010_1432_2a2c(0, 2, &nd),
            -1, 0, 0, 1, 1, g1g, fn1, g2f, fn2, g3f, fn3);
        break;
    }
    switch (p6) {
    case 0x1a:
        D_9080 |= 0x40;
        break;
    }
    D_e03f->nobj = 0;
    sub_s010_1432_2de0(0);
    return 1;
}
#pragma optimize("", on)

/* direction code from three flags */
int far sub_s010_1432_114e(char a, char b, char c)
{
    if (a) {
        if (b) {
            if (c)
                return 2;
            return 4;
        }
        if (c)
            return 3;
        return 5;
    }
    if (b) {
        if (c)
            return 6;
        return 8;
    }
    if (c)
        return 7;
    return 9;
}

/* randomise a street map (type t): height nibbles, wall nibbles and floor tiles 11/12 */
void far sub_s010_1432_11aa(int t)
{
    unsigned long v;
    unsigned long x;
    long rr;
    char hi, lo;
    int pct, r, c, k, f;

    switch (t) {
    default:
        pct = 10;
        break;
    case 4:
        pct = 15;
        break;
    case 1:
    case 5:
    case 6:
        pct = 5;
        break;
    }
    for (r = 1; r < 39; r++) {
        for (c = 1; c < 39; c++) {
            v = D_e03f->cell[r][c];
            if (v & 0x80000000L)
                continue;
            if (D_e03f->hgt[r][c] & 0xf0) {
                hi = sub_s004_0e76_20f6(1, 2);
                lo = 0;
                if (sub_s004_0e76_20f6(1, 100) <= pct) {
                    switch (sub_s004_0e76_20f6(1, 5)) {
                    case 1: lo = 1; break;
                    case 2: lo = 2; break;
                    case 3: lo = 3; break;
                    case 4: lo = 4; break;
                    case 5: lo = 5; break;
                    }
                }
                D_e03f->hgt[r][c] = (lo & 0xf) | (hi << 4);
            }
            for (k = 0; k <= 24; k += 8) {
                x = (v & (0xfL << k)) >> k;
                if (x == 0)
                    continue;
                if (k == 0 || k == 24)
                    x = 10;
                else if (t == 6) {
                    switch (x) {
                    case 3: x = 4; break;
                    case 4: x = 8; break;
                    case 5: x = 9; break;
                    default:
                        switch (sub_s004_0e76_20f6(1, 3)) {
                        case 1: x = 1; break;
                        case 2: x = 2; break;
                        case 3: x = 3; break;
                        }
                        break;
                    }
                } else if (x == 3)
                    x = 4;
                else {
                    switch (sub_s004_0e76_20f6(1, 5)) {
                    case 1: x = 1; break;
                    case 2: x = 2; break;
                    case 3: x = 3; break;
                    case 4: x = 4; break;
                    case 5: x = 5; break;
                    }
                }
                D_e03f->cell[r][c] &= ~(0xfL << k);
                D_e03f->cell[r][c] |= (x & 0xf) << k;
            }
            if ((f = (int)((v & 0xf0) >> 4)) != 0) {
                D_e03f->cell[r][c] &= ~0xf0L;
                if ((v & 0x0f00000fL) == 0 && sub_s004_0e76_20f6(1, 3) > 1) {
                    switch (f) {
                    case 11:
                        switch (sub_s004_0e76_20f6(1, 6)) {
                        case 1: f = 1; break;
                        case 2: f = 2; break;
                        case 3: f = 3; break;
                        case 4: f = 4; break;
                        case 5: f = 5; break;
                        case 6: f = 6; break;
                        }
                        break;
                    case 12:
                        f = 7;
                        break;
                    }
                    D_e03f->cell[r][c] |= (unsigned long)(f & 0xf) << 4;
                    rr = sub_s004_0e76_20f6(1, 4) - 1;
                    D_e03f->cell[r][c] |= (unsigned long)(rr & 3) << 28;
                }
            }
        }
    }
}

int far sub_s004_0e76_20dc();
extern unsigned char D_da34;

/* two-level keep battle (type 8): icity.700/701, start at a group marker, enemy groups
 * depending on the location p6 (0x1f inside, 0x21, 0x2c/0x2d) */
int far sub_s010_1432_1516(int p6, int t, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    char name[20];
    struct level far *lv;
    struct obj far *o;
    char inside;
    int n, lb, id, k, i, j, rv;
    int g1, g2, g3;
    int a, b, x, y;

    for (i = 0; i < 2; i++) {
        sprintf(name, "icity.70%d", i);
        sub_root_05d9_019a(name, (struct level far *)(D_e03f + i));
        D_e03f[i].x1f40 = i;
    }
    sub_s010_1432_2030(t, 0, 1);
    inside = 0;
    switch (p6) {
    case 0x1f:
        id = 3;
        lb = 0;
        inside = 1;
        break;
    case 0x2c:
    case 0x2d:
        rv = id = sub_s004_0e76_20f6(1, 6) + 1;
        lb = 1;
        break;
    default:
        id = 2;
        lb = 0;
        break;
    }
    k = sub_s010_1432_2a2c(lb, id, &n);
    lv = D_e03f + lb;
    o = &lv->obj[k];
    lv->x1f4c = o->x0 + 16;
    lv->x1f4e = o->x2 + 8;
    lv->x1f50 = 0;
    if (inside) {
        lv->x1f51 = -1;
        for (j = 0; j < 2; j++) {
            D_e03f[j].x2488 = 0;
            for (i = 0; i < D_e03f[j].nexit; i++)
                D_e03f[j].ex[i][7] = -1;
        }
    } else
        lv->x1f51 = o->x0c;
    g3 = g2 = g1 = -1;
    switch (p6) {
    default:
        if (ft1 >= 0 && fn1 > 0)
            g1 = sub_s004_0e76_06aa(0, ft1, fq1);
        if (ft2 >= 0 && fn2 > 0)
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
        if (ft3 >= 0 && fn3 > 0)
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
        sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, 3, &n), -1, 0, 0, 1, 1,
            g1, fn1, g2, fn2, g3, fn3);
        break;
    case 0x1f:
        if (ft1 >= 0 && fn1 > 0)
            g1 = sub_s004_0e76_06aa(0, ft1, fq1);
        if (ft2 >= 0 && fn2 > 0)
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
        if (ft3 >= 0 && fn3 > 0)
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
        sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, 2, &n), -1, 0, 0, 1, 1,
            g1, fn1, g2, fn2, g3, fn3);
        break;
    case 0x21:
        sub_s004_0e76_20dc();
        if (!(D_da34 & 1)) {
            x = sub_s004_0e76_06aa(0, 0x18, 1);
            y = sub_s004_0e76_06aa(0, ft1, fq1);
            a = sub_s010_1432_2a2c(0, 3, &n);
            b = sub_s010_1432_2a2c(0, 4, &n);
            sub_s010_1432_2b42(p6, 0, 0, a, -1, 0, 0, 1, 1, x, 1, y, fn1, -1, 0);
            sub_s010_1432_2b42(p6, 0, 0, b, -1, 0, 0, 0, 1, y, 2, -1, 0, -1, 0);
        }
        if (!(D_da34 & 2)) {
            g1 = sub_s004_0e76_06aa(1, 0x1b, 1);
            y = sub_s004_0e76_06aa(1, ft2, fq2);
            x = sub_s004_0e76_06aa(1, ft3, fq3);
            a = sub_s010_1432_2a2c(1, 5, &n);
            b = sub_s010_1432_2a2c(1, 6, &n);
            sub_s010_1432_2b42(p6, 1, 0, a, -1, 0, 0, 0, 1, g1, 1, y, 2, x, 2);
            sub_s010_1432_2b42(p6, 1, 0, b, -1, 0, 0, 0, 1, -1, 0, y, 2, x, 2);
        }
        break;
    case 0x2c:
    case 0x2d:
        sub_s004_0e76_20dc();
        if (!(D_da34 & 1)) {
            g1 = sub_s004_0e76_06aa(0, ft2, fq2);
            y = sub_s004_0e76_06aa(0, ft3, fq3);
            x = sub_s010_1432_2a2c(0, 2, &n);
            b = sub_s010_1432_2a2c(0, 5, &n);
            sub_s010_1432_2b42(p6, 0, 0, x, -1, 0, 0, 0, 1, g1, 1, y, 3, -1, 0);
            sub_s010_1432_2b42(p6, 0, 0, b, -1, 0, 0, 0, 1, g1, 1, y, 3, -1, 0);
        }
        if (!(D_da34 & 2)) {
            g2 = sub_s004_0e76_06aa(1, ft1, fq1);
            sub_s004_0e76_06aa(1, ft2, fq2);
            do
                x = sub_s004_0e76_20f6(1, 6) + 1;
            while (x == rv);
            a = x;
            do
                y = sub_s004_0e76_20f6(1, 6) + 1;
            while (y == rv || a == y);
            x = sub_s010_1432_2a2c(1, a, &n);
            y = sub_s010_1432_2a2c(1, y, &n);
            sub_s010_1432_2b42(p6, 1, 0, x, -1, 0, 0, 1, 1, g1, 1, g2, 4, -1, 0);
            sub_s010_1432_2b42(p6, 1, 0, y, -1, 0, 0, 0, 1, g2, 2, -1, 0, -1, 0);
        }
        break;
    }
    for (i = 0; i < 2; i++) {
        D_e03f[i].nobj = 0;
        sub_s010_1432_2de0(i);
    }
    return 2;
}

struct subgrp { char t; char x1; int x2; int x4; char x6; char x7; char x8; char p9; };
struct group {                          /* enemy group, 0x1e bytes */
    int pos;                            /* 00 */
    int pos2;                           /* 02 */
    int x04;                            /* 04 */
    char x06;                           /* 06 */
    char x07;                           /* 07: 1 = has a sub-group */
    char type[3];                       /* 08 */
    char count[3];                      /* 0b */
    char p0e[6];
    struct subgrp sub;                  /* 14 */
};
extern struct group _based(D_dadd) *D_db2d;
extern int D_db05[];
int far sub_s004_0e76_2160();

/* clear flag x7 of the records with id `id` and of their partners */
void far sub_s010_1432_1abc(struct level far *lv, int id)
{
    struct r14 far *p;
    int i;

    for (i = 0; i < lv->n1f52; i++) {
        p = &lv->r1f54[i];
        if (p->x6 == id) {
            p->x7 = 0;
            lv->r1f54[p->x0c].x7 = 0;
        }
    }
}

struct mon { char p0[0x8c]; char x8c; char p8d[0xd6 - 0x8d]; };
extern struct mon _based(D_dadd) *D_db2b;

/* castle/keep battle (type 9): one or two levels icity.80%d, returns the number of levels,
 * *res = level the party starts in */
int far sub_s010_1432_1b22(int p6, int t, int *res, int ft1, int fq1, int fn1, int ft2, int fq2,
    int fn2, int ft3, int fq3, int fn3)
{
    char name[20];
    struct level far *lv;
    char flag;
    int first, last, id, i, k, n, a, b, s;
    int g1, g2, g3;
    int h1, h2, h3;

    flag = 0;
    first = 0;
    last = 1;
    switch (p6) {
    case 0x2a:
        id = 0x15;
        *res = 1;
        flag = 1;
        break;
    case 0x1c:
        id = 0x15;
        first = 1;
        *res = first;
        flag = 1;
        break;
    case 0x29:
        id = 0x16;
        *res = first;
        break;
    default:
        id = 0x15;
        last = 0;
        *res = 0;
        break;
    }
    for (i = first; i <= last; i++) {
        sprintf(name, "icity.80%d", i);
        sub_root_05d9_019a(name, (struct level far *)(D_e03f + i));
        D_e03f[i].x1f40 = i - first;
    }
    sub_s010_1432_2030(t, first, last);
    k = sub_s010_1432_2a2c(*res, id, &n);
    lv = D_e03f + *res;
    lv->x1f4c = lv->obj[k].x0 + 16;
    lv->x1f4e = lv->obj[k].x2 + 8;
    lv->x1f50 = 0;
    lv->x1f51 = lv->obj[k].x0c;
    if (flag)
        sub_s010_1432_1abc(lv, lv->x1f51);
    g1 = g2 = g3 = id = -1;
    switch (p6) {
    default:
        if (ft1 >= 0 && fn1 > 0)
            g1 = sub_s004_0e76_06aa(0, ft1, fq1);
        if (ft2 >= 0 && fn2 > 0)
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
        if (ft3 >= 0 && fn3 > 0)
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
        sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, 6, &n), -1, 0, 0, 1, 1,
            g1, fn1, g2, fn2, g3, fn3);
        break;
    case 0x1c:
        if (ft1 >= 0 && fn1 > 0)
            g1 = sub_s004_0e76_06aa(0, ft1, fq1);
        if (ft2 >= 0 && fn2 > 0)
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
        if (ft3 >= 0 && fn3 > 0)
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
        sub_s010_1432_2b42(p6, 1, 1, sub_s010_1432_2a2c(1, 2, &n), -1, 0, 0, 1, 4,
            g1, fn1, g2, fn2, g3, fn3);
        break;
    case 0x1d:
        if (ft1 >= 0 && fn1 > 0)
            g1 = sub_s004_0e76_06aa(0, ft1, fq1);
        if (ft2 >= 0 && fn2 > 0)
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
        if (ft3 >= 0 && fn3 > 0)
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
        switch (n) {
        case 1: a = 2; b = 5; break;
        case 2: a = 4; b = 5; break;
        case 3: a = 2; b = 3; break;
        case 4: a = 3; b = 4; break;
        default: a = id; break;
        }
        s = sub_s010_1432_2a2c(0, a, &n);
        h1 = fn1 / 2;
        h2 = fn2 / 2;
        h3 = fn3 / 2;
        sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, b, &n), -1, 0, 0, 0, 1,
            g1, h1, g2, h2, g3, h3);
        fn3 -= h3;
        fn2 -= h2;
        fn1 -= h1;
        sub_s010_1432_2b42(p6, 0, 0, s, -1, 0, 0, 1, 1, g1, fn1, g2, fn2, g3, fn3);
        break;
    case 0x29:
    case 0x2a:
        sub_s004_0e76_20dc();
        if (!(D_da34 & 1)) {
            s = sub_s004_0e76_06aa(0, 0x18, 1);
            g2 = sub_s004_0e76_06aa(0, ft2, fq2);
            g3 = sub_s004_0e76_06aa(0, ft3, fq3);
            sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, 6, &n), -1, 0, 0, 1, 2,
                s, 1, g2, fn2, g3, fn3);
        }
        if (!(D_da34 & 2)) {
            s = sub_s004_0e76_06aa(1, 0x4b, 1);
            D_db2b[12 + s].x8c = 1;
            g2 = sub_s004_0e76_06aa(1, ft2, 5);
            g3 = sub_s004_0e76_06aa(1, ft2, fq2);
            sub_s010_1432_2b42(p6, 1, 0, sub_s010_1432_2a2c(1, 3, &n), -1, 0, 0, 1, 1,
                s, 1, g2, fn2, g3, fn3);
        }
        break;
    }
    for (i = first; i <= last; i++) {
        D_e03f[i].nobj = 0;
        sub_s010_1432_2de0(i);
    }
    *res -= first;
    return last - first + 1;
}

struct exit {                           /* 14 bytes, see s011 */
    char p0[6];
    char lvl;
    char x7;
    int x8;
    int x0a;
    char x0c;
    char x0d;
};

/* randomise the keep maps in level buffers k0..k1 (type t); with two buffers, link the
 * exits of buffer 0 and buffer 1 */
void far sub_s010_1432_2030(int t, int k0, int k1)
{
    struct level far *lv;
    struct level far *lv2;
    struct exit far *e1;
    struct exit far *e2;
    unsigned long v;
    unsigned long f;
    unsigned long of;
    char two, lo, hi, cc, cd;
    int k, r, c, b, w, n, i, ww;

    two = k1 > k0 ? 1 : 0;
    for (k = k0; k <= k1; k++) {
        lv = D_e03f + k;
        for (r = 1; r < 39; r++) {
            for (c = 1; c < 39; c++) {
                v = lv->cell[r][c];
                if (v & 0x80000000L)
                    continue;
                f = ((unsigned)v & 0xf0) >> 4;
                if (f) {
                    of = f;
                    lv->cell[r][c] &= ~0xf0L;
                    switch ((int)f) {
                    case 1:
                        if (two)
                            f = 10;
                        else
                            f = 0;
                        break;
                    case 2:
                        if (two)
                            f = 9;
                        else
                            f = 0;
                        break;
                    case 3: f = 7; break;
                    case 4: f = 8; break;
                    case 5:
                    case 6:
                        if (k == 0)
                            f = 1;
                        else
                            f = 2;
                        break;
                    case 7: f = 2; break;
                    case 8: f = 3; break;
                    case 9: f = 4; break;
                    case 10: f = 11; break;
                    case 11: f = 13; break;
                    case 12: f = 12; break;
                    case 13: f = 1; break;
                    default: f = 0; break;
                    }
                    if (f) {
                        lv->cell[r][c] |= (f & 0xf) << 4;
                        lv->cell[r][c] |= (unsigned long)((long)(sub_s004_0e76_20f6(1, 4) - 1) & 3) << 28;
                    }
                }
                lo = lv->hgt[r][c] & 0xf;
                hi = lv->hgt[r][c] >> 4;
                if (hi) {
                    hi = 1;
                    switch (lo) {
                    case 1: lo = 6; break;
                    case 2: lo = 7; break;
                    case 3: lo = 8; break;
                    default: lo = 0; break;
                    }
                    lv->hgt[r][c] = (lo & 0xf) | (hi << 4);
                }
                for (b = 0; b <= 24; b += 8) {
                    if ((w = (int)((v & (0xfL << b)) >> b)) == 0)
                        continue;
                    switch (w) {
                    case 2: w = 11; break;
                    case 4:
                        if (t == 9 && b >= 16)
                            w = 8;
                        else
                            w = 6;
                        break;
                    case 5:
                        if (t == 9 && b >= 16)
                            w = 9;
                        else
                            w = 6;
                        break;
                    case 7: w = 6; break;
                    case 8:
                        if (t == 9)
                            w = 7;
                        else
                            w = 6;
                        break;
                    default: w = 0; break;
                    }
                    lv->cell[r][c] &= ~(0xfL << b);
                    lv->cell[r][c] |= (unsigned long)(w & 0xf) << b;
                    if (t == 9) {
                        n = 0;
                        switch (b) {
                        case 8:
                            if (k == 0 && sub_s004_0e76_20f6(1, 100) < 5)
                                n = 1;
                            break;
                        case 16:
                            if (of == 2)
                                n = 2;
                            else if (k == 0 && sub_s004_0e76_20f6(1, 100) < 5)
                                n = 1;
                            else if (k == 1 && sub_s004_0e76_20f6(1, 100) < 5)
                                n = 4;
                            break;
                        default:
                            continue;
                        }
                        lv->cell[r][c] &= ~(0xfL << (b + 4));
                        lv->cell[r][c] |= (unsigned long)(n & 0xf) << (b + 4);
                    }
                }
            }
        }
    }
    if (two) {
        lv = D_e03f;
        lv2 = D_e03f + 1;
        for (i = 0; i < lv->nexit; i++) {
            e2 = (struct exit far *)lv2->ex[i];
            e1 = (struct exit far *)lv->ex[i];
            e1->lvl = lv2->x1f40;
            e2->lvl = lv->x1f40;
            e2->x0d = i;
            e1->x0d = i;
            cc = e1->x7;
            e1->x7 = e2->x7;
            e2->x7 = cc;
            ww = e1->x8;
            e1->x8 = e2->x8;
            e2->x8 = ww;
            ww = e1->x0a;
            e1->x0a = e2->x0a;
            e2->x0a = ww;
            cd = e1->x0c;
            e1->x0c = e2->x0c;
            e2->x0c = cd;
        }
    }
}

/* outdoor battlefield (type 7): prepared map icity.600 + random variant, party start at a
 * random group marker, enemies at another */
int far sub_s010_1432_2526(int p6, int t, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    char a, b, c;
    int n;
    int m1, m2, m3;
    int k;
    int f1;

    sub_root_05d9_019a("icity.600", (struct level far *)D_e03f);
    k = sub_s004_0e76_20f6(1, 8) - 1;
    sub_s004_1091_000a((struct level far *)D_e03f, k, 1);
    D_e03f->x1f40 = 0;
    sub_s010_1432_26f8(t);
    *(volatile char *)&a = sub_s004_0e76_20f6(1, 2) == 1 ? 1 : 0;
    f1 = ft1;
    k = sub_s010_1432_2a2c(0, a ? 0x16 : 0x17, &n);
    D_e03f->x1f4c = D_e03f->obj[k].x0 + 16;
    D_e03f->x1f4e = D_e03f->obj[k].x2 + 8;
    D_e03f->x1f50 = 0;
    D_e03f->x1f51 = D_e03f->obj[k].x0c;
    m3 = m2 = m1 = -1;
    if (f1 >= 0 && fn1 > 0)
        m1 = sub_s004_0e76_06aa(0, f1, fq1);
    if (ft2 >= 0 && fn2 > 0)
        m2 = sub_s004_0e76_06aa(0, ft2, fq2);
    if (ft3 >= 0 && fn3 > 0)
        m3 = sub_s004_0e76_06aa(0, ft3, fq3);
    b = 0;
    if (p6 == 0x15)
        b = 1;
    c = sub_s004_0e76_20f6(1, 2) == 1 ? 1 : 0;
    sub_s010_1432_2b42(p6, 0, 0, sub_s010_1432_2a2c(0, sub_s010_1432_114e(a, c, b), &n),
        -1, 0, 0, 1, 1, m1, fn1, m2, fn2, m3, fn3);
    D_e03f->nobj = 0;
    sub_s010_1432_2de0(0);
    return 1;
}

/* randomise the outdoor map: tile variants of the floor (types 11..13), height nibbles,
 * wall nibbles, and raise the rectangles listed at 0x3326 to height 0x40 */
void far sub_s010_1432_26f8(int t)
{
    unsigned long v;
    unsigned long f;
    long rr;
    char hi, lo;
    int pa, pb, pc;
    int r, c, k, i, n, x, y, x0, y0, w, h;
    int far *p;

    pa = pb = 25;
    pc = 33;
    for (r = 1; r < 39; r++) {
        for (c = 1; c < 39; c++) {
            v = D_e03f->cell[r][c];
            if (v & 0x80000000L)
                continue;
            f = ((unsigned)v & 0xf0) >> 4;
            if (f) {
                D_e03f->cell[r][c] &= ~0xf0L;
                switch ((int)f) {
                case 11:
                    if (sub_s004_0e76_20f6(1, 100) > pa)
                        f = sub_s004_0e76_20f6(1, 3);
                    else
                        f = 0;
                    break;
                case 12:
                    if (sub_s004_0e76_20f6(1, 100) > pb)
                        f = sub_s004_0e76_20f6(1, 3) + 4;
                    else
                        f = 0;
                    break;
                case 13:
                    f = 4;
                    break;
                default:
                    f = 0;
                    break;
                }
                if (f) {
                    D_e03f->cell[r][c] |= (f & 0xf) << 4;
                    rr = sub_s004_0e76_20f6(1, 4) - 1;
                    D_e03f->cell[r][c] |= (unsigned long)(rr & 3) << 28;
                }
            }
            if (D_e03f->hgt[r][c] & 0xf0) {
                hi = sub_s004_0e76_20f6(1, 3);
                lo = 0;
                if (f == 0 && sub_s004_0e76_20f6(1, 100) <= pc)
                    lo = sub_s004_0e76_20f6(1, 5) + 9;
                D_e03f->hgt[r][c] = (lo & 0xf) | (hi << 4);
            }
            for (k = 0; k <= 24; k += 8)
                if ((v & (0xfL << k)) >> k) {
                    D_e03f->cell[r][c] &= ~(0xfL << k);
                    D_e03f->cell[r][c] |= 0xbL << k;
                }
        }
    }
    n = D_e03f->n3326;
    for (i = 0, p = D_e03f->r3328[0]; i < n; i++, p += 5) {
        y0 = p[1] / 16;
        w = p[2] / 32;
        h = p[3] / 16;
        x0 = p[0] / 32;
        for (x = x0; x < x0 + w; x++)
            for (y = y0; y < y0 + h; y++)
                D_e03f->hgt[x][y] = 0x40;
    }
}

/* pick one of the group markers (type 0x65) with id `id` in level buffer k at random:
 * *n = its rank (1..count), returns its object index (-1/0 if none) */
int far sub_s010_1432_2a2c(int k, int id, int *n)
{
    struct level far *lv;
    int cnt, i, c;

    if (k < 0 || k >= 2)
        return 0;
    cnt = 0;
    lv = D_e03f + k;
    for (i = 0; i < lv->nobj; i++)
        if (lv->obj[i].type == 0x65 && *(int far *)&lv->obj[i].x0a == id)
            cnt++;
    if (cnt == 0)
        return *n = 0;
    *n = sub_s004_0e76_20f6(1, cnt);
    c = 0;
    for (i = 0; i < lv->nobj && *n > c; i++)
        if (lv->obj[i].type == 0x65 && *(int far *)&lv->obj[i].x0a == id)
            c++;
    return i - 1;
}

/* create an enemy group of up to three monster kinds at object idx of level buffer lvl;
 * idx2 >= 0: the second kind is a separate sub-group at object idx2 */
#pragma optimize("a", on)
void far sub_s010_1432_2b42(int p6, int lvl, int lb, int idx, int idx2, char x10, char x12,
    char flag, int r, int t1, int n1, int t2, int n2, int t3, int n3)
{
    struct group far *g;
    struct level far *lv;
    struct subgrp far *h;
    int d, j;

    if (t1 < 0 && t2 < 0 && t3 < 0)
        return;
    if (n1 <= 0 && n2 <= 0 && n3 <= 0)
        return;
    if (idx < 0 || idx >= 55)
        return;
    d = lvl - lb;
    g = &D_db2d[d * 10 + D_db05[d]];
    lv = D_e03f + lvl;
    g->pos = lv->obj[idx].x0 + 16;
    g->pos2 = lv->obj[idx].x2 + 8;
    g->x06 = lv->obj[idx].x0c;
    g->x04 = r << 4;
    g->x07 = 0;
    if (flag)
        lv->x1f50 = sub_s004_0e76_2160(lv->x1f4c / 32, lv->x1f4e / 16, g->pos / 32, g->pos2 / 16);
    j = 0;
    if (t1 >= 0 && t1 < 12 && n1 > 0) {
        g->type[0] = t1;
        g->count[0] = n1;
        j = 1;
    }
    if (t2 >= 0 && t2 < 12 && (n2 > 0 || idx2 >= 0)) {
        g->type[j] = t2;
        g->count[j] = n2;
        j++;
        if (idx2 >= 0 && idx2 < 12) {
            g->x07 = 1;
            g->sub.t = t2;
            h = &g->sub;
            h->x1 = 16;
            h->x6 = x10;
            h->x7 = x12;
            h->x8 = sub_s004_0e76_2160((h->x2 = lv->obj[idx2].x0 + 16) / 32,
                (h->x4 = lv->obj[idx2].x2 + 8) / 16, lv->x1f4c / 32, lv->x1f4e / 16);
        }
    }
    if (t3 >= 0 && t3 < 12 && n3 > 0) {
        g->type[j] = t3;
        g->count[j] = n3;
        j++;
    }
    if (j > 0)
        D_db05[d]++;
}
#pragma optimize("", on)

/* save level buffer k as level%d.flr */
void far sub_s010_1432_2de0(int k)
{
    char name[0x42];

    if (k >= 0 && k < 2) {
        sprintf(name, "level%d.flr", D_e03f[k].x1f40);
        sub_s004_1262_0d0a(name, (struct level far *)(D_e03f + k), 0x33fc, 0);
    }
}
