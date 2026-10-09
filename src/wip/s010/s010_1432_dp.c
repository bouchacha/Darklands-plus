/* place: s010 1432:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=1536 */
/* status (e6, 2026-10-09): started from build/agent/gen_engine. City battlefields, same scheme as
 * s011 (up to 2 level buffers of 0x33fc bytes at D_e03f based on D_e03d, saved as level%d.flr).
 * Match: 0000 (driver: the buffer name goes through a union local -> `mov ax,1536h; push ax`
 * and the 2-byte frame), 0102 (city location -> battlefield type 1..9, nested random switches
 * falling through), 0292, 058c, 079a (tile descriptions keep/out: struct based pointer + pragma
 * "a"), 114e, 2de0. Close: 1abc (6.00 uses `loop`), 2a2c, 2526 (register load placement).
 * Not written yet (dec16 stubs with their literals): 02cc 08aa 11aa 1516 1b22 2030 26f8 2b42.
 * Level: party start at 0x1f4c/0x1f4e (+0x1f50/51), 14-byte records at 0x1f54 (count 0x1f52),
 * objects at 0x2186 (count 0x2184), group markers type 0x65 with id at x0a.
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
struct rect { int x0, x2, x4, x6; char p8[2]; };
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
extern int D_fff0;
    struct r14 r1f54[(0x2184 - 0x1f54) / 14];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char p2488[0x3326 - 0x2488];
    char n3326;                         /* 3326: raised rectangles */
    char p3327;
    struct rect r3328[21];              /* 3328 */
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

/* sub_s010_1432_08aa: not decompiled: base+index addressing */
int far sub_s010_1432_08aa(void)
{
    sprintf("icity.%03d");
    return 0;
}

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

/* sub_s010_1432_11aa: not decompiled: read of undefined dx */
void far sub_s010_1432_11aa(void) { }

/* sub_s010_1432_1516: not decompiled: mov es from non-slot */
int far sub_s010_1432_1516(void)
{
    sprintf("icity.70%d");
    return 0;
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

/* sub_s010_1432_1b22: not decompiled: dx */
int far sub_s010_1432_1b22(void)
{
    sprintf("icity.80%d");
    return 0;
}

/* sub_s010_1432_2030: not decompiled: es: with index */
void far sub_s010_1432_2030(void) { }

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
    struct rect far *p;

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
    for (i = 0, p = D_e03f->r3328; i < n; i++, p++) {
        y0 = p->x2 / 16;
        w = p->x4 / 32;
        h = p->x6 / 16;
        x0 = p->x0 / 32;
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
