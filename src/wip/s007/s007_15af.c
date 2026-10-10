/* place: s007 15af:0000 */
/* orig: size=43b8 CONST=7d2e+0002 _DATA=1440+0017 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. 5/34 match (025a 30e6 3694 3b92 3d2e; m5: 025a = #pragma optimize("a", on), a far
 * `int far *o` room pointer, and if/else arms ordered as the original (`area > 0x32` -> 2/5 first,
 * `20f6(1,2) != 0` -> 8/9 first)). Written from the asm + Ghidra (dec16 drafts were stubs).
 * Castle/keep battlefields: turns the generator work area (s005, segment D_db39) into level
 * buffers (F_20a5_0000, struct level below) and saves them as level%d.flr.
 */

struct res { int a; int b; };

struct r14 {                            /* 14 bytes at level+0x1f54: passages between parts */
    int x0, x2;                         /* pixel position */
    unsigned char x4, x5;               /* size (0x20, 0x10) */
    char x6;                            /* component */
    char x7;
    unsigned char x8, x9;               /* cell x / y, 0x80/0x40 = side flags */
    char x0a, x0b;
    char x0c;                           /* partner record or -1 */
    char p0d;
};
struct level {                          /* battle map (0x33fc bytes), see s010/s011 */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41;
    int x1f42[5];                       /* 1f42 */
    int x1f4c, x1f4e;                   /* 1f4c: party start (pixels) */
    char x1f50;                         /* 1f50: party facing */
    char x1f51;                         /* 1f51: party's part */
    char n1f52;                         /* 1f52 */
    char p1f53;
    struct r14 r14[(0x2184 - 0x1f54) / 14]; /* 1f54 */
    char nobj;                          /* 2184 */
    char p2185[0x2488 - 0x2185];
    char ncomp;                         /* 2488: connected parts */
    char p2489;
    int comp[30];                       /* 248a: byte offset of each part's list in list[] */
    int list[0x578];                    /* 24c6: cells (x << 8) + y, -1 after each part */
    char p2f96[0x2fb6 - 0x2f96];
    char nexit;                         /* 2fb6 */
    char p2fb7[0x300c - 0x2fb7];
    char ngrp;                          /* 300c */
    char p300d[0x3326 - 0x300d];
    char n3326;                         /* 3326 */
    char p3327[0x33fc - 0x3327];
};
extern struct level far F_20a5_0000;
#define F F_20a5_0000
#define CB(x, y, k) (((unsigned char far *)&F_20a5_0000.cell[x][y])[k])

struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes, 4 per level (s005) */
struct xrec {                           /* 14 bytes at D_5158: level exits */
    int x0, x2;
    char x4, x5, x6, x7;
    int x8, x0a;
    char x0c, x0d;
};

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms: {x0, x1, y0, y1}, 30 per level */
extern unsigned char _based(D_db39) *D_db8d;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;                  /* room kind [lvl * 30 + i] */
extern int _based(D_db39) *D_db93;
extern struct gexit _based(D_db39) *D_db95;
extern _segment D_dadd;
extern char _based(D_dadd) *D_db33;                 /* tile descriptions */
extern int D_db25;                                  /* keep entrance side */
extern int D_db3d, D_db3f, D_db41, D_db43;
extern int D_db45[5];
extern int D_db4f[5];
extern int D_db59[5];
extern int D_db7f;
extern int D_db63;
extern int D_db71[5];
extern int D_db7b;
extern int D_db7d;
extern int D_db9b[5];                               /* parts per level */
extern int D_dbbb[5][30];                           /* part of each room */
extern char D_5156;
extern struct xrec D_5158[];
extern char D_54c6;
extern int D_54c8, D_54ca, D_54cc, D_54ce;

int far sprintf(char *, const char *, ...);
char far *_fstrcpy(char far *, const char far *);
void *memset(void *, int, unsigned);
int far sub_s004_0e76_20f6(int, int);
int far sub_s004_0e76_06aa(int, int, int);
int far sub_s004_1262_0d0a(char *, void far *, long);
int far sub_s005_146d_000c(int, int, int);
int far sub_s005_1551_00bc(int, int, int, int, int);

void far sub_s007_15af_0188(int);
void far sub_s007_15af_025a(void);
void far sub_s007_15af_051e(int);
void far sub_s007_15af_0698(int, int);
void far sub_s007_15af_0b3e(int, int, int, int, long *, long *, long *, long *, long *, long *, long *);
void far sub_s007_15af_10a6(int);
void far sub_s007_15af_1234(int);
void far sub_s007_15af_16ba(int);
void far sub_s007_15af_18f8(int, int *, int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_1a5e(int, int *, int *);
void far sub_s007_15af_1bfe(int, int *, int *, int *, int *, int *, int *, int *, int *, int *);
void far sub_s007_15af_1d78(int, int, int, int *, int *);
char far sub_s007_15af_3372(int, int, int);
void far sub_s007_15af_224e(int, int, int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_23d4(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_2964(int, int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_2a44(int, int, int, int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_2cdc(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
char far sub_s007_15af_30e6(int, int, int, int);
void far sub_s007_15af_316c(int, int, int, int, int, int, int, int, int, int, int);
void far sub_s007_15af_345a(int, int, int *);
void far sub_s007_15af_3f8a(int, int, int, int, long);

/* build and save all levels of a castle/keep battlefield */
void far sub_s007_15af_0000(int *pn, int t, int p3, int p4, int p5, int p6, int p7, int p8,
                            int p9, int p10, int p11)
{
    int l, i, j;
    unsigned long far *c;
    int side;
    int sv;
    char buf[66];

    sv = 0;
    for (l = D_db7d; l <= D_db7b; l++)
        sub_s007_15af_0188(l);
    sub_s007_15af_025a();
    sub_s007_15af_1a5e(t, pn, &side);
    sub_s007_15af_1bfe(t, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11);
    for (l = D_db7d; l <= D_db7b; l++) {
        F.x1f40 = l - D_db7d;
        F.n3326 = F.ngrp = F.nexit = F.ncomp = F.nobj = F.n1f52 = 0;
        F.x1f42[4] = F.x1f42[3] = F.x1f42[2] = F.x1f42[1] = F.x1f42[0] = -2;
        c = F.cell[0];
        for (i = 0; i < 1600; i += 40)
            for (j = 0; j < 40; j++) {
                *c++ = 0x80000000L;
                F.hgt[0][i + j] = 0;
            }
        sub_s007_15af_051e(l);
        sub_s007_15af_0698(t, l);
        if (l == 1)
            sub_s007_15af_10a6(l);
        sub_s007_15af_1234(l);
        sub_s007_15af_16ba(l);
        sub_s007_15af_18f8(l, pn, t, side, p3, p4, p5, p6, p7, p8, p9, p10, p11);
        sub_s007_15af_345a(t, l, &sv);
        sprintf(buf, "level%d.flr", F.x1f40);
        sub_s004_1262_0d0a(buf, &F_20a5_0000, 0x33fcL);
    }
}

/* split the rooms of level l into connected parts */
void far sub_s007_15af_0188(int l)
{
    int i, j;
    int seen[30];

    D_db9b[l] = 0;
    for (i = 0; i < 30; i++)
        D_dbbb[l][i] = -1;
    memset(seen, 0, sizeof seen);
    for (i = 0; i < D_db45[l]; i++) {
        if (seen[i] == 0) {
            for (j = 0; j < D_db45[l]; j++) {
                if (seen[j] == 0 && sub_s005_146d_000c(l, i, j)) {
                    seen[j] = 1;
                    D_dbbb[l][j] = D_db9b[l];
                }
            }
            D_db9b[l]++;
        }
    }
}

/* tile descriptions of keeps */
#pragma optimize("a", on)
void far sub_s007_15af_025a(void)
{
    char k[5] = "keep";
    char k1[6] = "keep1";
    int r;
    int area;
    int far *o;

    r = sub_s004_0e76_20f6(1, 2);
    o = D_db3b + 30 * 4;
    area = (o[3] - o[2] + 1) * (o[1] - o[0] + 1);
    D_db33[0] = 0;
    _fstrcpy((char far *)(D_db33 + 1), k);
    if (r == 1)
        D_db33[0x0b] = 1;
    else
        D_db33[0x0b] = 0;
    _fstrcpy((char far *)(D_db33 + 0x1a), k);
    D_db33[0x23] = 1;
    D_db33[0x25] = 10;
    D_db33[0x26] = 11;
    D_db33[0x28] = 9;
    D_db33[0x27] = 8;
    _fstrcpy((char far *)(D_db33 + 0x34), k1);
    if (r == 1)
        D_db33[0x3e] = 0;
    else
        D_db33[0x3e] = 1;
    D_db33[0x3f] = 10;
    if (area > 0x32)
        D_db33[0x41] = 2;
    else
        D_db33[0x41] = 6;
    D_db33[0x42] = 7;
    if (sub_s004_0e76_20f6(1, 2) != 0) {
        D_db33[0x45] = 8;
        D_db33[0x46] = 9;
    } else {
        D_db33[0x45] = 3;
        D_db33[0x46] = 4;
    }
    _fstrcpy((char far *)(D_db33 + 0x4d), k);
    D_db33[0x58] = 0;
    D_db33[0x56] = 2;
    D_db33[0x59] = 8;
    D_db33[0x5b] = 14;
    D_db33[0x5a] = 10;
    D_db33[0x5c] = 11;
    D_db33[0x5d] = 12;
    _fstrcpy((char far *)(D_db33 + 0x67), k1);
    if (r == 1)
        D_db33[0x71] = 0;
    else
        D_db33[0x71] = 1;
    D_db33[0x72] = 10;
    D_db33[0x73] = 2;
    if (area > 0x32)
        D_db33[0x74] = 5;
    else
        D_db33[0x74] = 6;
    D_db33[0x75] = 3;
    D_db33[0x78] = 7;
    D_db33[0x79] = 8;
    _fstrcpy((char far *)(D_db33 + 0x80), k);
    D_db33[0x8b] = 0;
    D_db33[0x89] = 2;
    D_db33[0x8c] = 6;
    _fstrcpy((char far *)(D_db33 + 0x9a), k);
    D_db33[0xa5] = 13;
    D_db33[0xa6] = 5;
    D_db33[0xa7] = 8;
    D_db33[0xa8] = 9;
    D_db33[0xa9] = 0;
    D_db33[0xaa] = 1;
    D_db33[0xab] = 3;
    D_db33[0xac] = 11;
    D_db33[0xad] = 2;
    D_db33[0xae] = 17;
    D_db33[0xaf] = 7;
    D_db33[0xb0] = 16;
    D_db33[0xb1] = 6;
    D_db33[0xa3] = 12;
    D_db33[0xb2] = 12;
    D_db33[0xb3] = 4;
}
#pragma optimize("", on)

/* cell lists of the parts; clears the cells of all rooms */
void far sub_s007_15af_051e(int l)
{
    int k, c, i, x, y;
    int _based(D_db39) *r;

    F.ncomp = D_db9b[l];
    k = 0;
    for (c = 0; c < D_db9b[l]; c++) {
        if (k + 1 >= 0x578)
            break;
        F.comp[c] = k * 2;
        for (i = 0, r = D_db3b + l * 120; i < D_db45[l]; i++, r += 4) {
            if (k + 1 >= 0x578)
                break;
            if (D_dbbb[l][i] == c) {
                for (x = r[0]; x <= r[1]; x++)
                    for (y = r[2]; y <= r[3]; y++) {
                        F.list[k++] = ((unsigned char)x << 8) + y;
                        F.cell[x][y] = 0;
                    }
            }
        }
        F.list[k++] = -1;
    }
}

/* walls: open the exits, then let 0b3e decorate every room cell */
void far sub_s007_15af_0698(int t, int l)
{
    int i, x, y, r, idx, v;
    struct gexit _based(D_db39) *e;
    int _based(D_db39) *o;
    long a, b, c, d, w, f, g;
    unsigned long far *p;

    for (i = 0; i < D_db71[l]; i++) {
        e = D_db95 + l * 4 + i;
        x = e->x0;
        y = e->x2;
        if (e->x0a == 0) {
            r = sub_s005_1551_00bc(l, x - 1, x - 1, y, y);
            idx = (l * 40 + x) * 40 + y;
            v = D_db8f[idx - 40];
            if (v > 1 && v < 9 && sub_s005_146d_000c(l, r, e->x4))
                D_db8d[idx] &= ~8;
            r = sub_s005_1551_00bc(l, x + 1, x + 1, y, y);
            v = D_db8f[idx + 40];
            if (v > 1 && v < 9 && sub_s005_146d_000c(l, r, e->x4))
                D_db8d[idx] &= ~2;
        } else if (e->x0a == 3) {
            r = sub_s005_1551_00bc(l, x, x, y + 1, y + 1);
            idx = (l * 40 + x) * 40 + y;
            v = D_db8f[idx + 1];
            if (v > 1 && v < 9 && sub_s005_146d_000c(l, r, e->x4))
                D_db8d[idx] &= ~4;
            r = sub_s005_1551_00bc(l, x, x, y - 1, y - 1);
            v = D_db8f[idx - 1];
            if (v > 1 && v < 9 && sub_s005_146d_000c(l, r, e->x4))
                D_db8d[idx] &= ~1;
        }
    }
    for (r = 0; r < D_db45[l]; r++) {
        o = D_db3b + (l * 30 + r) * 4;
        for (x = o[0]; x <= o[1]; x++) {
            for (y = o[2]; y <= o[3]; y++) {
                p = &F.cell[x][y];
                F.hgt[x][y] |= 0x10;
                sub_s007_15af_0b3e(l, r, x, y, &a, &b, &c, &d, &w, &f, &g);
                if (a)
                    *p |= (a & 15) << 16;
                if (c)
                    *p |= c & 15;
                if (b)
                    *p |= (b & 15) << 24;
                if (d)
                    *p |= (d & 15) << 8;
                if (w)
                    *p |= (w & 15) << 20;
                if (f)
                    *p |= (f & 15) << 12;
                if (g)
                    sub_s007_15af_3f8a(l, r, x, y, g);
            }
        }
        if (D_db91[l * 30 + r] == 7) {
            if (o[0] < o[1]) {
                x = o[1] + sub_s004_0e76_20f6(1, o[1] - o[0] + 1) - 1;
                y = o[2];
                if (D_db8d[(l * 40 + x) * 40 + y] & 1) {
                    CB(x, y, 2) &= 0x0f;
                    CB(x, y, 2) |= 0x10;
                }
            } else if (o[2] < o[3]) {
                y = o[3] + sub_s004_0e76_20f6(1, o[3] - o[2] + 1) - 1;
                x = o[0];
                if (D_db8d[(l * 40 + x) * 40 + y] & 8) {
                    CB(x, y, 1) &= 0x0f;
                    CB(x, y, 1) |= 0x10;
                }
            }
        }
    }
}

/* wall pieces of cell (x, y) of room r: sides s0..s3 (1 = wall, 2 = outer wall,
 * 3 = ...), e = decoration on side 0, f = on side 3, g = object (3f8a) */
void far sub_s007_15af_0b3e(int l, int r, int x, int y, long *s0, long *s2, long *s1,
                            long *s3, long *e, long *f, long *g)
{
    int k, t;
    unsigned char w;
    int far *o;

    t = D_db91[l * 30 + r];
    w = D_db8d[(l * 40 + x) * 40 + y];
    o = (int far *)(D_db3b + (l * 30 + r) * 4);
    *e = *f = *g = *s0 = *s1 = *s2 = *s3 = 0;
    if (w & 1)
        *s0 = 1;
    if (w & 2)
        *s1 = 1;
    if (w & 4)
        *s2 = 1;
    if (w & 8)
        *s3 = 1;
    switch (t) {
    case 2:
        if (*s0 && l > 1 && o[2] <= D_db41 + 1 && sub_s004_0e76_20f6(1, 6) == 1)
            *e = 6;
        else if (*s0 && l > 1 && sub_s004_0e76_20f6(1, 16) == 1)
            *e = 1;
        else if (*s0 && sub_s004_0e76_20f6(1, 4) == 1) {
            switch (sub_s004_0e76_20f6(1, 16)) {
            case 1: case 2: case 3: case 4:
                *e = 1;
                break;
            case 5: case 6: case 7: case 8:
                *e = 3;
                break;
            case 9: case 10: case 11: case 12:
                *e = 4;
                break;
            case 13: case 14:
                *g = 14;
                break;
            case 15: case 16:
                *e = 5;
                break;
            }
        }
        if (*s3 && !*s0 && sub_s004_0e76_20f6(1, 4) == 1) {
            switch (sub_s004_0e76_20f6(1, 16)) {
            case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
                *f = 1;
                break;
            case 11: case 12:
                *g = 15;
                break;
            case 13: case 14: case 15: case 16:
                *s3 = 3;
                break;
            }
        }
        break;
    case 3:
    case 5:
        if (l > 1 && *s0 && sub_s004_0e76_20f6(1, 4) == 1) {
            if (o[2] <= D_db41 + 1 && sub_s004_0e76_20f6(1, 5) < 4)
                *e = 6;
            else
                *e = 1;
        }
        if (*s3 && !*s0 && sub_s004_0e76_20f6(1, 4) == 1)
            *f = 1;
        break;
    case 4:
        if (l > 1 && *s0 && o[2] <= D_db41 + 1 && sub_s004_0e76_20f6(1, 4) == 1)
            *e = 6;
        break;
    case 6:
        if (*s0 && (D_db25 == 1 || D_db25 == 3) && (x + y) % 2 == 0) {
            switch (sub_s004_0e76_20f6(1, 4)) {
            case 1:
                *e = 1;
                break;
            case 2:
                *e = 3;
                break;
            case 3:
                *e = 4;
                break;
            case 4:
                *e = 5;
                break;
            }
        }
        if (*s3 && (D_db25 == 0 || D_db25 == 2) && (x + y) % 2 == 0)
            *f = 1;
        break;
    case 7:
        if (l > 1 && *s0 && o[2] <= D_db41 + 1 && sub_s004_0e76_20f6(1, 8) == 1)
            *e = 6;
        else if (*s0 && sub_s004_0e76_20f6(1, 8) == 1) {
            switch (sub_s004_0e76_20f6(1, 4)) {
            case 1:
                *e = 1;
                break;
            case 2:
                *e = 3;
                break;
            case 3:
                *e = 4;
                break;
            case 4:
                *e = 5;
                break;
            }
        }
        if (*s3 && !*s0 && sub_s004_0e76_20f6(1, 8) == 1)
            *f = 1;
        break;
    case 8:
        if (*s0) {
            k = sub_s005_1551_00bc(l, x, x, y - 1, y - 1);
            if (k < 0 || D_db91[l * 30 + k] == 8)
                *s0 = 2;
        }
        if (*s1) {
            k = sub_s005_1551_00bc(l, x + 1, x + 1, y, y);
            if (k < 0 || D_db91[l * 30 + k] == 8)
                *s1 = 2;
        }
        if (*s2) {
            k = sub_s005_1551_00bc(l, x, x, y + 1, y + 1);
            if (k < 0 || D_db91[l * 30 + k] == 8)
                *s2 = 2;
        }
        if (*s3) {
            k = sub_s005_1551_00bc(l, x - 1, x - 1, y, y);
            if (k < 0 || D_db91[l * 30 + k] == 8)
                *s3 = 2;
        }
        break;
    }
    if (l == 0) {
        if (*e != 1)
            *e = 0;
        if (*f != 1)
            *f = 0;
        *g = 0;
    }
}

/* level 1: the gate of the keep (room D_db63) */
void far sub_s007_15af_10a6(int l)
{
    int x0, x1, y0, y1;
    int _based(D_db39) *o;

    if (l == 1 && D_db63 >= 0 && D_db91[30 + D_db63] == 6) {
        D_54c6 = 1;
        o = D_db3b + (30 + D_db63) * 4;
        y0 = o[2];
        y1 = o[3];
        x0 = o[0];
        x1 = o[1];
        if (D_db25 == 0) {
            CB(x0, y0, 2) &= 0xf0;
            CB(x0, y0, 2) |= 8;
            CB(x1, y0, 2) &= 0xf0;
            CB(x1, y0, 2) |= 9;
            y1 = y0;
        } else if (D_db25 == 1) {
            CB(x1, y0, 0) &= 0xf0;
            CB(x1, y0, 0) |= 9;
            CB(x1, y1, 0) &= 0xf0;
            CB(x1, y1, 0) |= 8;
            x0 = x1;
        } else if (D_db25 == 2) {
            CB(x0, y1, 3) &= 0xf0;
            CB(x0, y1, 3) |= 8;
            CB(x1, y1, 3) &= 0xf0;
            CB(x1, y1, 3) |= 9;
            y0 = y1;
        } else if (D_db25 == 3) {
            CB(x0, y0, 1) &= 0xf0;
            CB(x0, y0, 1) |= 9;
            CB(x0, y1, 1) &= 0xf0;
            CB(x0, y1, 1) |= 8;
            x1 = x0;
        }
        D_54c8 = x0 * 32 + 1;
        D_54cc = (x1 - x0) * 32 + 31;
        D_54ca = y0 * 16 + 1;
        D_54ce = (y1 - y0) * 16 + 15;
    }
}

/* passages through walls between different parts (wall kinds 3 and 6) */
void far sub_s007_15af_1234(int l)
{
    int n, x, y, a, b, va, vb;
    unsigned kind;
    unsigned char sz;
    int w, h;
    int _based(D_db39) *o;
    struct r14 far *q;

    n = F.n1f52;
    if (l == 0) {
        kind = 5;
        o = D_db3b + 30 * 4;
        w = o[1] - o[0] + 1;
        h = o[3] - o[2] + 1;
        if (w < h)
            w = h;
        sz = ((w + 5) | 0x40) << 1;
    } else {
        kind = 4;
        sz = 0;
    }
    for (x = D_db3d; x <= D_db3f; x++) {
        for (y = D_db41; y <= D_db43; y++) {
            if (D_db93[(l * 40 + x) * 40 + y] == 3) {
                b = sub_s005_1551_00bc(l, x, x, y, y);
                a = sub_s005_1551_00bc(l, x, x, y - 1, y - 1);
                va = a >= 0 && a < D_db45[l];
                vb = b >= 0 && b < D_db45[l];
                if (D_dbbb[l][a] != D_dbbb[l][b] && va && vb) {
                    CB(x, y, 2) &= 0xf0;
                    *(unsigned far *)&CB(x, y, 2) |= kind;
                    CB(x, y - 1, 3) &= 0xf0;
                    *(unsigned far *)&CB(x, y - 1, 2) |= kind << 8;
                    q = &F.r14[n];
                    q->x0 = x * 32;
                    q->x2 = y * 16;
                    q->x4 = 0x20;
                    q->x5 = 0x10;
                    q->x6 = D_dbbb[l][a];
                    q->x7 = sz;
                    q->x8 = x | 0x80;
                    q->x9 = y;
                    q->x0b = q->x0a = 0;
                    q->x0c = vb ? n + 1 : -1;
                    q++;
                    q->x0 = x * 32;
                    q->x2 = y * 16 - 16;
                    q->x4 = 0x20;
                    q->x5 = 0x10;
                    q->x6 = D_dbbb[l][b];
                    q->x7 = sz;
                    q->x8 = x;
                    q->x9 = (y - 1) | 0x80;
                    q->x0b = q->x0a = 0;
                    q->x0c = va ? n : -1;
                    n += 2;
                }
            } else if (D_db93[(l * 40 + x) * 40 + y] == 6) {
                b = sub_s005_1551_00bc(l, x, x, y, y);
                a = sub_s005_1551_00bc(l, x - 1, x - 1, y, y);
                va = a >= 0 && a < D_db45[l];
                vb = b >= 0 && b < D_db45[l];
                if (D_dbbb[l][a] != D_dbbb[l][b] && va && vb) {
                    CB(x, y, 1) &= 0xf0;
                    *(unsigned far *)&CB(x, y, 0) |= kind << 8;
                    CB(x - 1, y, 0) &= 0xf0;
                    *(unsigned far *)&CB(x - 1, y, 0) |= kind;
                    q = &F.r14[n];
                    q->x0 = x * 32;
                    q->x2 = y * 16;
                    q->x4 = 0x20;
                    q->x5 = 0x10;
                    q->x6 = D_dbbb[l][a];
                    q->x7 = sz;
                    q->x8 = x;
                    q->x9 = y | 0x40;
                    q->x0b = q->x0a = 0;
                    q->x0c = vb ? n + 1 : -1;
                    q++;
                    q->x0 = x * 32 - 32;
                    q->x2 = y * 16;
                    q->x4 = 0x20;
                    q->x5 = 0x10;
                    q->x6 = D_dbbb[l][b];
                    q->x7 = sz;
                    q->x8 = (x - 1) | 0x40;
                    q->x9 = y;
                    q->x0b = q->x0a = 0;
                    q->x0c = va ? n : -1;
                    n += 2;
                }
            }
        }
    }
    F.n1f52 = n;
}

/* stairs between levels: exit records at D_5158 */
void far sub_s007_15af_16ba(int l)
{
    int i, n, x, y;
    struct gexit _based(D_db39) *e;
    struct xrec *q;
    unsigned char c;

    n = D_5156;
    q = &D_5158[n];
    for (i = 0; i < D_db71[l]; i++, q++, n++) {
        e = D_db95 + l * 4 + i;
        if (e->x8 == 1) {
            q->x6 = F.x1f40 + 1;
            q->x7 = D_dbbb[l + 1][D_db95[(l + 1) * 4 + e->x6].x4];
        } else {
            q->x6 = F.x1f40 - 1;
            q->x7 = D_dbbb[l - 1][D_db95[(l - 1) * 4 + e->x6].x4];
        }
        q->x0d = e->x6;
        x = e->x0;
        y = e->x2;
        if (e->x0a == 0) {
            q->x2 = (y + 1) * 16;
            q->x0 = x * 32;
            q->x8 = x * 32 + 16;
            q->x0a = (y + 1) * 16 + 0x18;
            q->x0c = 0;
            CB(x, y, 2) &= 0x0f;
            if (e->x8 == 1)
                c = 12;
            else {
                c = 10;
                CB(x, y, 2) |= 0x20;
            }
        } else if (e->x0a == 3) {
            q->x2 = y * 16;
            q->x0 = (x + 1) * 32;
            q->x8 = (x + 1) * 32 + 0x30;
            q->x0a = y * 16 + 8;
            q->x0c = 6;
            CB(x, y, 1) = 0;
            CB(x, y, 1) |= 1;
            if (e->x8 == 1)
                c = 13;
            else {
                c = 11;
                CB(x, y, 1) |= 0x20;
            }
        } else
            break;
        CB(x, y, 0) &= 0x0f;
        CB(x, y, 0) |= c << 4;
        q->x4 = 0x20;
        q->x5 = 0x10;
    }
    D_5156 = n;
}

/* contents of level l by battlefield type t */
void far sub_s007_15af_18f8(int l, int *pn, int t, int side, int p5, int p6, int p7, int p8,
                            int p9, int p10, int p11, int p12, int p13)
{
    int a, b;

    a = 0;
    b = 0;
    if (*pn == l)
        sub_s007_15af_1d78(l, t, *pn, &a, &b);
    switch (t) {
    case 8:
        sub_s007_15af_2964(l, *pn, 8, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        break;
    case 0:
        sub_s007_15af_224e(l, *pn, 0, b, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        break;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 0x7a:
        sub_s007_15af_23d4(l, *pn, side, t, a, b, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        break;
    case 0x27: case 0x28:
        sub_s007_15af_2a44(l, *pn, t, a, b, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        break;
    case 0x7b: case 0x7c: case 0x7d: case 0x7e:
        sub_s007_15af_2cdc(l, *pn, side, t, a, b, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        break;
    }
}

/* level of the party (*pn) and second level (*lv) by battlefield type */
void far sub_s007_15af_1a5e(int t, int *pn, int *lv)
{
    int n, m;

    n = D_db7b - D_db7d + 1;
    *lv = -1;
    switch (t) {
    case 0:
        *pn = 1;
        break;
    case 1: case 2: case 4: case 0x7a:
        m = D_db7f - 1 > D_db7b ? D_db7b : D_db7f - 1;
        *pn = *lv = sub_s004_0e76_20f6(1, m - 1) + 1;
        break;
    case 3:
        *pn = *lv = 2;
        break;
    case 5:
        *pn = 1;
        do
            *lv = sub_s004_0e76_20f6(1, D_db7b - D_db7d) + D_db7d - 1;
        while (*lv == 1 && n > 1 && sub_s004_0e76_20f6(1, 4) > 1);
        break;
    case 6: case 7:
        *pn = *lv = 1;
        break;
    case 8:
        *pn = D_db7d >= 0 ? D_db7d : 0;
        break;
    case 0x27:
        *pn = D_db7b;
        *lv = 2;
        break;
    case 0x28:
        *pn = 1;
        *lv = 2;
        break;
    case 0x7b:
        *pn = sub_s004_0e76_20f6(1, D_db7b - 1) + 1;
        m = D_db7f - 1 > D_db7b ? D_db7b : D_db7f - 1;
        *lv = sub_s004_0e76_20f6(1, m - D_db7d + 1) + D_db7d - 1;
        break;
    case 0x7c: case 0x7d:
        m = D_db7f - 1 > D_db7b ? D_db7b : D_db7f - 1;
        *pn = 1;
        *lv = sub_s004_0e76_20f6(1, m - D_db7d + 1) + D_db7d - 1;
        break;
    case 0x7e:
        *pn = 1;
        *lv = sub_s004_0e76_20f6(1, 2);
        break;
    }
}

struct member { char p[0x80]; };
extern struct member D_9c69[5];
int far vec_1c1b(void);

/* enemy groups for the battlefield type: (type, sub, count) x 3 */
void far sub_s007_15af_1bfe(int t, int *a, int *b, int *c, int *d, int *e, int *f, int *g,
                            int *h, int *i)
{
    int a_, b_, d_, e_, f_, g_, h_;
    int v;
    struct member *m;

    g_ = -1;
    h_ = 1;
    d_ = 0;
    f_ = 0;
    v = vec_1c1b();
    for (m = D_9c69; m < &D_9c69[5]; m++)
        ;
    switch (t) {
    case 1: case 2: case 4: case 5: case 6: case 7: case 0x7a:
        if (*a == 0x24 || *a == 0x26) {
            a_ = 0x26;
            b_ = 2;
        } else {
            a_ = 0x23;
            b_ = 3;
        }
        g_ = d_ = 0x12;
        e_ = 4;
        h_ = 3;
        break;
    case 3:
        a_ = 0x16;
        b_ = v / 7 + 1;
        g_ = d_ = 0x12;
        e_ = 4;
        h_ = 3;
        break;
    case 8:
        a_ = 0;
        b_ = *f;
        d_ = 3;
        e_ = 2;
        f_ = b_;
        break;
    case 0x7b: case 0x7c: case 0x7d: case 0x7e:
        a_ = 0x50;
        b_ = (v + 2) / 3;
        if (b_ > 3)
            b_ = 3;
        if (b_ < 1)
            b_ = 1;
        d_ = 0x4f;
        e_ = (v + 4) / 2;
        if (e_ > 5)
            e_ = 5;
        if (e_ < 2)
            e_ = 2;
        f_ = (v + 6) / 2;
        break;
    default:
        return;
    }
    *a = a_;
    *b = b_;
    *c = 1;
    *d = d_;
    *e = e_;
    *f = f_;
    *g = g_;
    *h = h_;
    *i = 0;
}

/* party start: room *room, side *dir; sets the level's start position */
void far sub_s007_15af_1d78(int l, int t, int pn, int *room, int *dir)
{
    int i, best, area, ok, k;
    int w, h, px, py, xa, xb, ya, yb;
    int far *r;

    *dir = -1;
    switch (t) {
    case 0: case 5: case 0x28: case 0x7d: case 0x7e:
        *room = D_db63;
        *dir = D_db25;
        break;
    case 1: case 2: case 0x7a:
        *room = -1;
        for (i = D_db4f[l] + 1; D_db91[l * 30 + i] != 5 && D_db59[l] >= i && D_db45[l] > i; i++)
            ;
        for (; D_db59[l] >= i && D_db45[l] > i; i++) {
            if (D_db91[l * 30 + i] == 5) {
                r = (int far *)(D_db3b + (l * 30 + i) * 4);
                if (-(r[0] - r[1]) >= 2 || r[3] - r[2] >= 2) {
                    *room = i;
                    break;
                }
            }
        }
        if (*room <= 0)
            *room = sub_s004_0e76_20f6(1, D_db4f[l]);
        break;
    case 3: case 7: case 8:
        *room = 0;
        break;
    case 4:
        *room = sub_s004_0e76_20f6(1, D_db4f[l] + 1) - 1;
        break;
    case 6:
        best = -1;
        area = 0;
        for (i = 1; D_db59[l] > i; i++) {
            r = (int far *)(D_db3b + (l * 30 + i) * 4);
            if ((r[3] - r[2] + 1) * (r[1] - r[0] + 1) >= area) {
                area = (r[3] - r[2] + 1) * (r[1] - r[0] + 1);
                best = i;
            }
        }
        if (best > 0)
            *room = best;
        else if (D_db91[l * 30] == 2)
            *room = sub_s004_0e76_20f6(1, D_db4f[l]);
        else
            *room = sub_s004_0e76_20f6(1, D_db4f[l] + 1) - 1;
        break;
    case 0x27:
        best = -1;
        area = 0;
        for (i = 0; D_db45[l] > i; i++) {
            r = (int far *)(D_db3b + (l * 30 + i) * 4);
            if ((r[3] - r[2] + 1) * (r[1] - r[0] + 1) >= area) {
                area = (r[3] - r[2] + 1) * (r[1] - r[0] + 1);
                best = i;
            }
        }
        if (best < 0)
            *room = 0;
        else
            *room = best;
        break;
    case 0x7b:
        if (pn == D_db7f)
            *room = sub_s004_0e76_20f6(1, D_db45[l]) - 1;
        else
            *room = sub_s004_0e76_20f6(1, D_db59[l]);
        break;
    case 0x7c:
        do
            *room = sub_s004_0e76_20f6(1, D_db59[l]);
        while (*room == D_db63);
        break;
    }
    *room = *room < 0 ? 0 : *room;
    if (*dir < 0) {
        i = 0;
        ok = 0;
        while (!ok && i < 10) {
            if (i >= 6)
                k = i - 6;
            else
                k = sub_s004_0e76_20f6(1, 4) - 1;
            *dir = k;
            ok = sub_s007_15af_3372(l, *room, k);
            i++;
        }
        if (!ok)
            *dir = sub_s004_0e76_20f6(1, 4) - 1;
    }
    F.x1f51 = D_dbbb[l][*room];
    r = (int far *)(D_db3b + (l * 30 + *room) * 4);
    w = r[1] - r[0] + 1;
    h = r[3] - r[2] + 1;
    px = w * 16;
    py = h * 8;
    xa = r[0];
    xb = r[1];
    ya = r[2];
    yb = r[3];
    switch (*dir) {
    case 0:
        F.x1f50 = 4;
        py = 16;
        yb = ya + 1;
        k = (w - 2) / 2;
        xa = k + r[0];
        xb = r[1] - k;
        break;
    case 1:
        F.x1f50 = 6;
        px = (w - 1) * 32;
        xa = xb - 1;
        k = (h - 2) / 2;
        ya = k + r[2];
        yb = r[3] - k;
        break;
    case 2:
        F.x1f50 = 0;
        py = (h - 1) * 16;
        ya = yb - 1;
        k = (w - 2) / 2;
        xa = k + r[0];
        xb = r[1] - k;
        break;
    case 3:
        F.x1f50 = 2;
        xb = xa + 1;
        px = 32;
        k = (h - 2) / 2;
        ya = k + r[2];
        yb = r[3] - k;
        break;
    }
    F.x1f4c = r[0] * 32 + px;
    F.x1f4e = r[2] * 16 + py;
    for (px = xa; px <= xb; px++)
        for (py = ya; py <= yb; py++)
            if (D_db93[(l * 40 + px) * 40 + py] == 0)
                D_db93[(l * 40 + px) * 40 + py] = 0x10;
}

/* pixel position of a group in room i: the centre, or next to the party's side */
#define GPOS(o, px, py, pn, l, i, room, dir) \
    px = (o[0] + o[1] + 1) * 16; \
    py = (o[2] + o[3] + 1) * 8; \
    if (pn == l && i == room) { \
        if (dir == 0) \
            py = o[3] << 4; \
        else if (dir == 1) \
            px = (o[0] + 1) << 5; \
        else if (dir == 2) \
            py = (o[2] + 1) << 4; \
        else if (dir == 3) \
            px = o[1] << 5; \
    }

/* type 0: enemies on level 1 next to the keep's gate */
void far sub_s007_15af_224e(int l, int pn, int t, int dir, int p5, int p6, int p7, int p8,
                            int p9, int p10, int p11, int p12, int p13)
{
    int lv, i, m1, m2, m3, px, py;
    int far *o;

    if (pn != l)
        return;
    lv = l - D_db7d;
    m3 = m2 = m1 = -1;
    if (sub_s007_15af_30e6(t, l, 1, -1))
        m1 = sub_s004_0e76_06aa(lv, p5, p6);
    if (sub_s007_15af_30e6(t, l, 2, -1))
        m2 = sub_s004_0e76_06aa(lv, p8, p9);
    if (sub_s007_15af_30e6(t, l, 3, -1))
        m3 = sub_s004_0e76_06aa(lv, p11, p12);
    if (m1 < 0 && m2 < 0 && m3 < 0)
        return;
    for (i = 0; i < D_db45[1]; i++)
        if (sub_s005_146d_000c(1, i, D_db63))
            break;
    if (i >= D_db45[1])
        i = D_db63;
    o = (int far *)(D_db3b + (30 + i) * 4);
    px = (o[1] + o[0] + 1) * 16;
    py = (o[2] + o[3] + 1) * 8;
    if (dir == 0)
        py = o[3] << 4;
    else if (dir == 1)
        px = (o[0] + 1) << 5;
    else if (dir == 2)
        py = (o[2] + 1) << 4;
    else if (dir == 3)
        px = o[1] << 5;
    sub_s007_15af_316c(l, px, py, 0x10, i, m1, p7, m2, p10, m3, p13);
}

struct mrec { char p0[0x0c]; char x0c; char p0d[0xd6 - 0x0d]; };   /* 0xd6-byte monster */
extern struct mrec _based(D_dadd) *D_db2b;

/* types 1..7, 0x7a: one group per room */
void far sub_s007_15af_23d4(int l, int pn, int side, int t, int room, int dir, int p7, int p8,
                            int p9, int p10, int p11, int p12, int p13, int p14, int p15)
{
    int lv, i, j, k, boss, px, py;
    int m1, m2, m3, g1, g2, g3, n2, n3;
    char has[30];
    int far *o;

    m3 = m2 = m1 = -1;
    lv = l - D_db7d;
    if (sub_s007_15af_30e6(t, l, 1, side))
        m1 = sub_s004_0e76_06aa(lv, p7, p8);
    if (sub_s007_15af_30e6(t, l, 2, side))
        m2 = sub_s004_0e76_06aa(lv, p10, p11);
    if (sub_s007_15af_30e6(t, l, 3, side))
        m3 = sub_s004_0e76_06aa(lv, p13, p14);
    if (m1 < 0 && m2 < 0 && m3 < 0)
        return;
    boss = -1;
    if (side == l) {
        switch (t) {
        case 3: case 0x7a:
            boss = room;
            break;
        case 1: case 2: case 4: case 5:
            do
                boss = sub_s004_0e76_20f6(1, D_db59[l] + 1) - 1;
            while (pn == l && boss == room && D_db59[l] > 1);
            break;
        case 6: case 7:
            boss = 0;
            break;
        }
    }
    if (m1 >= 0)
        D_db2b[l * 12 + m1].x0c = 1;
    for (i = 0; i <= D_db59[l]; i++)
        has[i] = 0;
    for (j = 0; j <= D_db71[l]; j++)
        has[D_db95[l * 4 + j].x4] = 1;
    for (i = 0; i <= D_db59[l]; i++) {
        g3 = g2 = g1 = -1;
        n3 = 0;
        n2 = 0;
        if (boss == i)
            g1 = m1;
        switch (t) {
        case 0x7a:
            if (boss == i)
                break;
        case 1:
            if (i == room && i >= 1)
                break;
        case 2:
            k = D_db91[l * 30 + i];
            if (k == 2) {
                n2 = sub_s004_0e76_20f6(1, 3) - 1;
                n3 = sub_s004_0e76_20f6(1, 2) - 1;
            } else if (k == 3) {
                n2 = sub_s004_0e76_20f6(1, 3) - 2;
                n3 = sub_s004_0e76_20f6(1, 2) - 1;
                if (l == D_db7f && i == 0 && n3 < 1)
                    n3 = 1;
            } else if ((l != 0 && l != D_db7f) || sub_s004_0e76_20f6(1, 3) < 2) {
                n2 = sub_s004_0e76_20f6(1, 5) == 1;
                if (sub_s004_0e76_20f6(1, 4) == 1)
                    n2 = sub_s004_0e76_20f6(1, 2);
            }
            break;
        case 3: case 6: case 7:
            if (pn == l) {
                if (i == room) {
                    n2 = sub_s004_0e76_20f6(1, 3) + 1;
                    n3 = sub_s004_0e76_20f6(1, 2);
                }
                if (boss == i)
                    n3 += sub_s004_0e76_20f6(1, 2);
                if (D_db91[l * 30 + i] == 3)
                    n3 = sub_s004_0e76_20f6(1, 2) - 1;
            } else {
                k = D_db91[l * 30 + i];
                if (k == 2) {
                    g2 = m2;
                    n2 = sub_s004_0e76_20f6(1, 3) + 1;
                } else if (k == 3) {
                    g3 = m3;
                    j = 1;
                    n3 = sub_s004_0e76_20f6(j, 2);
                }
            }
            break;
        case 4: case 5:
            j = 1;
            if (t == 5) {
                if (l == 1)
                    j = 2;
                else if (D_db7f - l == 1)
                    j = 0;
            }
            k = D_db91[l * 30 + i];
            if (k == 2) {
                n2 = sub_s004_0e76_20f6(j, 3);
                if (has[i])
                    n2++;
            } else if (k == 3) {
                if (has[i] || boss == i || sub_s004_0e76_20f6(1, 2) == 1)
                    n3 = sub_s004_0e76_20f6(j, 2);
            } else if (has[i] || boss == i) {
                n3 = sub_s004_0e76_20f6(j, 2);
                n2 = sub_s004_0e76_20f6(j, 2) - j;
            }
            break;
        }
        if (n2 > 0)
            g2 = m2;
        if (n3 > 0)
            g3 = m3;
        o = (int far *)(D_db3b + (l * 30 + i) * 4);
        GPOS(o, px, py, pn, l, i, room, dir)
        sub_s007_15af_316c(l, px, py, 0x10, i, g1, boss == i, g2, n2, g3, n3);
    }
}

/* type 8: one camp group on level l */
void far sub_s007_15af_2964(int l, int pn, int t, int p5, int p6, int p7, int p8, int p9,
                            int p10, int p11, int p12, int p13)
{
    int lv, m1, m2;
    int _based(D_db39) *o;

    if (D_db91[l * 30] == 2) {
        lv = l - D_db7d;
        m1 = -1;
        if (sub_s007_15af_30e6(t, l, 1, -1))
            m1 = sub_s004_0e76_06aa(lv, p5, p6);
        if (sub_s007_15af_30e6(t, l, 2, -1))
            m2 = sub_s004_0e76_06aa(lv, p8, p9);
        else
            m2 = -1;
        if (m1 >= 0 || m2 >= 0) {
            o = D_db3b + l * 120;
            sub_s007_15af_316c(l, (o[1] + o[0] + 1) * 16, (o[2] + o[3] + 1) * 8, 0x10,
                               D_dbbb[l][0], m1, 1, m2, sub_s004_0e76_20f6(1, p10) + 1, -1, 0);
        }
    }
}

/* types 0x27/0x28: guards at the stairs */
void far sub_s007_15af_2a44(int l, int pn, int t, int room, int dir, int p6, int p7, int p8,
                            int p9, int p10, int p11, int p12, int p13, int p14)
{
    int lv, n, i, want, px, py;
    int m1, m2, m3, a1, a2, a3;
    int ex[2];
    int *q;
    int far *o;

    m3 = m2 = m1 = -1;
    lv = l - D_db7d;
    switch (l) {
    case 1:
        m1 = sub_s004_0e76_06aa(lv, 0x18, 1);
        m2 = sub_s004_0e76_06aa(lv, p9, p10);
        m3 = sub_s004_0e76_06aa(lv, p12, p13);
        break;
    case 2:
        m1 = sub_s004_0e76_06aa(lv, 0x18, 1);
        m2 = sub_s004_0e76_06aa(lv, p9, 5);
        m3 = sub_s004_0e76_06aa(lv, p9, p10);
        break;
    case 3:
        if (D_db7f > l) {
            m2 = sub_s004_0e76_06aa(lv, p9, 5);
            m3 = sub_s004_0e76_06aa(lv, p9, p10);
        }
        break;
    default:
        return;
    }
    if (m1 < 0 && m2 < 0 && m3 < 0)
        return;
    n = 0;
    ex[1] = -1;
    ex[0] = -1;
    if (l == 1)
        want = 1;
    else if (l == 3)
        want = 2;
    else
        want = 0;
    if (want) {
        for (i = 0, q = ex; i < D_db71[l]; i++) {
            if (D_db95[l * 4 + i].x8 == want) {
                n++;
                if (q < &ex[2])
                    *q++ = i;
            }
        }
    }
    for (i = 0; i <= D_db59[l]; i++) {
        a3 = 0;
        a2 = 0;
        a1 = 0;
        if (l == 1) {
            if (ex[0] == i) {
                if (n != 1) {
                    a3 = 1;
                    a1 = 1;
                    a2 = 2;
                } else {
                    a1 = 1;
                    a2 = 4;
                    a3 = 3;
                }
            } else if (ex[1] == i) {
                a1 = 0;
                a3 = 2;
                a2 = 2;
            }
        } else if (l == 2) {
            if (i != 0)
                continue;
            a1 = 1;
            a2 = 3;
            a3 = 4;
        } else if (l == 3 && D_db7f > 3) {
            if (ex[0] == i || ex[1] == i) {
                a1 = 0;
                a3 = 2;
                a2 = 2;
            }
        } else
            continue;
        if (a1 || a2 || a3) {
            o = (int far *)(D_db3b + (l * 30 + i) * 4);
            GPOS(o, px, py, pn, l, i, room, dir)
            sub_s007_15af_316c(l, px, py, 0x10, i, m1, a1, m2, a2, m3, a3);
        }
    }
}

extern int D_db4f[5];
extern int D_db59[5];

/* types 0x7b..0x7e: enemies spread over the rooms */
void far sub_s007_15af_2cdc(int l, int pn, int side, int t, int room, int dir, int p7, int p8,
                            int p9, int p10, int p11, int p12, int p13, int p14, int p15)
{
    int lv, i, k, cnt, boss, gate, px, py, m1, m2, g1, g2, n2;
    int far *o;
    int per[30];

    m2 = -1;
    m1 = -1;
    lv = l - D_db7d;
    if (sub_s007_15af_30e6(t, l, 1, side))
        m1 = sub_s004_0e76_06aa(lv, p7, p8);
    if (sub_s007_15af_30e6(t, l, 2, side))
        m2 = sub_s004_0e76_06aa(lv, p10, p11);
    if (m1 < 0 && m2 < 0)
        return;
    gate = -2;
    if (l == 1) {
        for (i = 0; i < D_db45[1] && !sub_s005_146d_000c(1, i, D_db63); i++)
            ;
        if (i < D_db45[1])
            gate = i;
    }
    boss = -1;
    if (side == l && t > 0x7a) {
        if (t == 0x7c || t == 0x7d)
            boss = sub_s004_0e76_20f6(1, D_db59[l] + 1) - 1;
        else if (t == 0x7e) {
            if (l == 1 && sub_s004_0e76_20f6(1, 2) == 1)
                boss = gate;
            else
                boss = D_db95[l * 4].x4;
        }
    }
    if (m1 >= 0)
        D_db2b[l * 12 + m1].x0c = 1;
    for (i = 0; i < 30; i++)
        per[i] = 0;
    if (t == 0x7e) {
        if (D_db7f == l)
            for (i = 0; i < p12; i++)
                per[sub_s004_0e76_20f6(1, D_db4f[l]) - 1]++;
        if (l < D_db7b - 1) {
            cnt = 1;
            for (i = 0; i < D_db71[l]; i++)
                if (D_db95[l * 4 + i].x8 == 1 && D_db95[l * 4 + i].x4 != 0)
                    cnt++;
            k = (p12 + 2) / 3;
            per[0] += k;
            k = p12 - k;
            for (i = 0; i < D_db71[l]; i++) {
                if (D_db95[l * 4 + i].x8 == 1 && D_db95[l * 4 + i].x4 != 0) {
                    if (cnt == 2)
                        per[D_db95[l * 4 + i].x4] += k;
                    else {
                        per[D_db95[l * 4 + i].x4] += k / 2;
                        k -= k / 2;
                    }
                }
            }
        }
        if (l == 1)
            per[gate] += p12;
    } else if (D_db7f == l)
        per[0]++;
    else
        for (i = 0; i < p12; i++)
            per[sub_s004_0e76_20f6(1, D_db59[l]) - 1]++;
    for (i = 0; i <= D_db59[l]; i++) {
        if (per[i] > 0 || boss == i) {
            g2 = -1;
            g1 = -1;
            n2 = 0;
            if (boss == i)
                g1 = m1;
            if (per[i] > 0) {
                g2 = m2;
                n2 = per[i];
            }
            o = (int far *)(D_db3b + (l * 30 + i) * 4);
            GPOS(o, px, py, pn, l, i, room, dir)
            sub_s007_15af_316c(l, px, py, 0x10, i, g1, boss == i, g2, n2, -1, 0);
        }
    }
}

/* may group k (1..3) of type t be placed on level l? */
char far sub_s007_15af_30e6(int t, int l, int k, int side)
{
    if (l < D_db7d || l > D_db7b || k < 1 || k > 3)
        return 0;
    switch (t) {
    case 0:
        if (l == 1)
            return 1;
        break;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 0x7a:
        if (k == 3)
            return 1;
        if (k == 2)
            return 1;
        if (k != 1)
            break;
        if (l == side)
            return 1;
        break;
    case 8:
        if (k >= 3)
            break;
        if (D_db91[l * 30] == 2)
            return 1;
        break;
    case 0x7b: case 0x7c: case 0x7d: case 0x7e:
        if (k == 2)
            return 1;
        if (k != 1)
            break;
        if (l == side)
            return 1;
        break;
    }
    return 0;
}

struct group {                          /* enemy group, 0x1e bytes (D_db2d) */
    int x0, x2, x4;                     /* pixel position, radius */
    char x6, x7;                        /* part */
    char type[3];                       /* 08 */
    char cnt[3];                        /* 0b */
    char p0e[0x1e - 0x0e];
};
extern struct group _based(D_dadd) *D_db2d;
extern int D_db05[5];
extern unsigned D_da34;

/* add an enemy group (up to 3 kinds m1..m3 with counts n1..n3) */
void far sub_s007_15af_316c(int l, int px, int py, int rad, int room, int m1, int n1, int m2,
                            int n2, int m3, int n3)
{
    int lv, k, i;
    struct group far *g;
    int _based(D_db39) *c;

    if (m1 < 0 && m2 < 0 && m3 < 0)
        return;
    if (m1 < 0)
        n1 = 0;
    if (m2 < 0)
        n2 = 0;
    if (m3 < 0)
        n3 = 0;
    i = l * 30 + room;
    if (D_db91[i] != 2) {
        if (n1 > 4)
            n1 = 4;
        if (n2 > 4 - n1)
            n2 = 4 - n1;
        if (n2 < 0)
            n2 = 0;
        if (n3 > 4 - n1 - n2)
            n3 = 4 - n1 - n2;
        if (n3 < 0)
            n3 = 0;
        if (n1 < 1)
            m1 = -1;
        if (n2 < 1)
            m2 = -1;
        if (n3 < 1)
            m3 = -1;
    }
    if (m1 < 0 && m2 < 0 && m3 < 0)
        return;
    if (D_da34 & (1 << l)) {
        if (n1 > 0)
            n1 = (n1 + 1) / 2;
        if (n2 > 0)
            n2 = (n2 + 1) / 2;
        if (n3 > 0)
            n3 = (n3 + 1) / 2;
    }
    lv = l - D_db7d;
    g = (struct group far *)(D_db2d + lv * 10 + D_db05[lv]);
    k = 0;
    if (m1 >= 0 && n1 > 0) {
        g->type[0] = m1;
        g->cnt[0] = n1;
        k = 1;
    }
    if (m2 >= 0 && n2 > 0) {
        g->type[k] = m2;
        g->cnt[k] = n2;
        k++;
    }
    if (m3 >= 0 && n3 > 0) {
        g->type[k] = m3;
        g->cnt[k] = n3;
        k++;
    }
    if (k) {
        g->x0 = px;
        g->x2 = py;
        g->x4 = rad;
        g->x6 = D_dbbb[l][room];
        g->x7 = 0;
        D_db05[lv]++;
        c = D_db93 + (l * 40 + px / 32) * 40 + py / 16;
        if (*c == 0)
            *c = 14;
    }
}

/* is side dir of room r free of stairs (cell kinds 9/10)? */
char far sub_s007_15af_3372(int l, int r, int dir)
{
    int x, y, x1, dx, dy, v;
    int _based(D_db39) *o;

    o = D_db3b + (l * 30 + r) * 4;
    x = o[0];
    y = o[2];
    x1 = o[1];
    dy = 0;
    dx = 0;
    if (dir == 0)
        dx = 1;
    else if (dir == 1) {
        dy = 1;
        x = x1;
    } else if (dir == 2) {
        dx = 1;
        y = o[3];
    } else if (dir == 3)
        dy = 1;
    for (; x <= x1 && y <= o[3]; x += dx, y += dy) {
        v = D_db8f[l * 1600 + x * 40 + y];
        if (v == 9 || v == 10)
            return 0;
    }
    return 1;
}

char far sub_s007_15af_3b92(int, int);
char far sub_s007_15af_3bce(int, int, int);
char far sub_s007_15af_3d2e(int, int, int);
char far sub_s007_15af_3da0(int, int, int, int);
long far sub_s007_15af_3e3e(int, int, int, char);
void far sub_s007_15af_3694(int, int, int far *);
void far sub_s007_15af_36e2(int, int, int far *);
void far sub_s007_15af_39ba(int, int, int far *);
void far sub_s007_15af_3aac(int, int, int far *);
void far sub_s007_15af_4076(int, int, int, int, int *);
extern char D_51ac;

/* furniture: stairs cells, then random objects in every room */
void far sub_s007_15af_345a(int t, int l, int *cnt)
{
    int i, r, x, y, v, kind, placed;
    long w;
    struct gexit _based(D_db39) *e;
    int far *o;
    char force;

    for (i = 0; i < D_db71[l]; i++) {
        e = D_db95 + l * 4 + i;
        if (e->x0a == 0)
            D_db93[(l * 40 + e->x0) * 40 + e->x2 + 2] = 11;
        else if (e->x0a == 3)
            D_db93[(l * 40 + e->x0) * 40 + e->x2 + 80] = 11;
    }
    v = sub_s004_0e76_20f6(1, 3);
    for (r = 0; r < D_db45[l]; r++) {
        o = (int far *)(D_db3b + (l * 30 + r) * 4);
        kind = D_db91[l * 30 + r];
        if (kind == 2)
            sub_s007_15af_3694(t, l, o);
        placed = 0;
        for (x = o[0]; x <= o[1]; x++) {
            for (y = o[2]; y <= o[3]; y++) {
                if (!sub_s007_15af_3b92(l, kind) || !sub_s007_15af_3bce(l, x, y))
                    continue;
                if (*cnt == 0 && v - 1 > 0)
                    force = 1;
                else
                    force = 0;
                w = sub_s007_15af_3e3e(t, l, kind, force);
                if (w == 5) {
                    if (v - 1 <= D_51ac || *cnt > 19 || placed > 0 || o[3] == y ||
                        !sub_s007_15af_3d2e(l, x, y))
                        continue;
                    sub_s007_15af_4076(t, l, x, y, cnt);
                    placed++;
                } else if (w == 9 && t == 8)
                    w = 0;
                sub_s007_15af_3f8a(l, r, x, y, w);
            }
        }
    }
}

/* furnish a hall (kind 2) */
void far sub_s007_15af_3694(int t, int l, int far *o)
{
    if (l > 0)
        sub_s007_15af_36e2(t, l, o);
    if (D_db7d == l && t != 0)
        sub_s007_15af_39ba(t, l, o);
    if (l > 0)
        sub_s007_15af_3aac(t, l, o);
}

/* a table with benches in the middle of the hall */
void far sub_s007_15af_36e2(int t, int l, int far *o)
{
    int x0, y0, h, w, k, x, y, n, x1, y1;
    long kind;
    long side;
    int _based(D_db39) *c;

    w = o[1] - o[0];
    h = o[3] - o[2];
    k = w - 2;
    x0 = k / 2 + o[0];
    if (k % 2 == 1 && sub_s004_0e76_20f6(1, 2) == 1)
        x0++;
    y0 = o[2];
    k = h - 2;
    y0 = k / 2 + y0;
    if (k % 2 == 1 && sub_s004_0e76_20f6(1, 2) == 1)
        y0++;
    for (x = x0; x <= x0 + 2; x++)
        for (y = y0; y <= y0 + 2; y++)
            if (!sub_s007_15af_3bce(l, x, y))
                return;
    kind = 8;
    n = 1;
    if (t != 8 && sub_s004_0e76_20f6(1, 4) > 1) {
        kind = 9;
        if (sub_s004_0e76_20f6(1, 2) == 1) {
            n = 2;
            if ((h + 1) * (w + 1) > 0x32 && sub_s004_0e76_20f6(1, 2) == 1)
                n = 3;
        }
    }
    x1 = x0 + 1;
    if (n == 1 || (n == 2 && sub_s004_0e76_20f6(1, 2) == 1))
        y0++;
    y1 = y0 + n;
    for (y = y0; y < y1; y++)
        if (sub_s007_15af_3bce(l, x1, y))
            sub_s007_15af_3f8a(l, 0, x1, y, kind);
    for (x = x0; x <= x0 + 2; x += 2) {
        for (y = y0; y < y1; y++) {
            if (sub_s007_15af_3bce(l, x, y)) {
                if (sub_s004_0e76_20f6(1, 4) == 1)
                    side = 2;
                else if ((o[2] + o[3]) / 2 < y)
                    side = 3;
                else
                    side = 4;
                sub_s007_15af_3f8a(l, 0, x, y, side);
            }
        }
    }
    y = y0 - 1;
    if (sub_s007_15af_3bce(l, x1, y))
        sub_s007_15af_3f8a(l, 0, x1, y, 4L);
    if (sub_s007_15af_3bce(l, x1, y1))
        sub_s007_15af_3f8a(l, 0, x1, y1, 3L);
    for (x = x0; x <= x0 + 2; x++)
        for (k = y; k <= y1; k++) {
            c = D_db93 + (l * 40 + x) * 40 + k;
            if (*c == 0)
                *c = 13;
        }
}

/* a throne (object 1) somewhere in the hall of the lowest level */
void far sub_s007_15af_39ba(int t, int l, int far *o)
{
    int i, x, y, a, b;
    int _based(D_db39) *c;

    for (i = 0; i < 50; i++) {
        x = sub_s004_0e76_20f6(1, o[1] - o[0] + 1) + o[0] - 1;
        y = sub_s004_0e76_20f6(1, o[3] - o[2] + 1) + o[2] - 1;
        if (sub_s007_15af_3bce(l, x, y))
            break;
    }
    if (i < 50) {
        sub_s007_15af_3f8a(l, 0, x, y, 1L);
        for (a = x - 1; a <= x + 1; a++)
            for (b = y - 1; b <= y + 1; b++) {
                c = D_db93 + (l * 40 + a) * 40 + b;
                if (*c == 0)
                    *c = 13;
            }
    }
}

/* a two-cell height feature (hgt 2/1) in the hall */
void far sub_s007_15af_3aac(int t, int l, int far *o)
{
    int i, x, y;
    int _based(D_db39) *c;

    for (i = 0; i < 50; i++) {
        x = sub_s004_0e76_20f6(1, o[1] - o[0] + 1) + o[0] - 1;
        y = sub_s004_0e76_20f6(1, o[3] - o[2] + 1) + o[2] - 1;
        if (sub_s007_15af_3bce(l, x, y) && sub_s007_15af_3bce(l, x, y + 1))
            break;
    }
    if (i < 50) {
        F.hgt[x][y] &= 0xf0;
        F.hgt[x][y] |= 2;
        F.hgt[x][y + 1] &= 0xf0;
        F.hgt[x][y + 1] |= 1;
        c = D_db93 + (l * 40 + x) * 40 + y;
        if (c[0] == 0)
            c[0] = 15;
        if (c[1] == 0)
            c[1] = 15;
    }
}

/* chance of an object in a cell of a room of this kind */
char far sub_s007_15af_3b92(int l, int kind)
{
    int n;

    if (l == 0) {
        if (kind == 2)
            n = 20;
        else
            n = 0;
    } else {
        switch (kind) {
        case 2: case 3: case 4: case 5:
            n = 6;
            break;
        default:
            n = 0;
            break;
        }
    }
    return sub_s004_0e76_20f6(1, n) == 1;
}

extern unsigned D_13d0[16];             /* allowed wall patterns */

/* is cell (x, y) free for an object (no blocked passage around it)? */
char far sub_s007_15af_3bce(int l, int x, int y)
{
    int a, b;
    int _based(D_db39) *p;

    p = D_db8f + l * 1600;
    if (p[x * 40 + y] > 1 && p[x * 40 + y] < 9 && D_db93[(l * 40 + x) * 40 + y] == 0) {
        a = !sub_s007_15af_3da0(l, x, y + 1, 2);
        a |= !sub_s007_15af_3da0(l, x - 1, y + 1, 7) << 1;
        a |= !sub_s007_15af_3da0(l, x - 1, y, 3) << 2;
        a |= !sub_s007_15af_3da0(l, x - 1, y - 1, 4) << 3;
        b = !sub_s007_15af_3da0(l, x, y - 1, 0);
        b |= !sub_s007_15af_3da0(l, x + 1, y - 1, 5) << 1;
        b |= !sub_s007_15af_3da0(l, x + 1, y, 1) << 2;
        b |= !sub_s007_15af_3da0(l, x + 1, y + 1, 6) << 3;
        return ((1 << b) & D_13d0[a]) == 0;
    }
    return 0;
}

/* is the 3x3 block around (x, y) unused? */
char far sub_s007_15af_3d2e(int l, int x, int y)
{
    int i, j;

    for (i = x - 1; i <= x + 1; i++)
        for (j = y - 1; j <= y + 1; j++)
            if (D_db93[(l * 40 + i) * 40 + j] != 0)
                return 0;
    return 1;
}

/* is the cell open towards direction k (0..3 sides, 4..7 corners)? */
char far sub_s007_15af_3da0(int l, int x, int y, int k)
{
    int i, v;
    unsigned char b;

    i = (l * 40 + x) * 40 + y;
    v = D_db8f[i];
    if (v <= 1 || v >= 9)
        return 0;
    b = D_db8d[i];
    switch (k) {
    case 0:
        b &= 4;
        break;
    case 1:
        b &= 8;
        break;
    case 2:
        b &= 1;
        break;
    case 3:
        b &= 2;
        break;
    case 4:
        b &= 6;
        break;
    case 5:
        b &= 12;
        break;
    case 6:
        b &= 9;
        break;
    case 7:
        b &= 3;
        break;
    }
    if (b == 0)
        return 1;
    return 0;
}

/* random object for a room of this kind (5 = chest) */
long far sub_s007_15af_3e3e(int t, int l, int kind, char force)
{
    long r;

    r = 0;
    if (l == 0) {
        if (kind == 2) {
            switch (sub_s004_0e76_20f6(1, 3)) {
            case 1:
                r = 5;
                break;
            case 2:
                r = 2;
                break;
            case 3:
                r = 7;
                break;
            }
        }
    } else {
        switch (kind) {
        case 2:
            switch (sub_s004_0e76_20f6(1, 3)) {
            case 1:
                r = 5;
                break;
            case 2:
                r = 7;
                break;
            case 3:
                r = 2;
                break;
            }
            break;
        case 3:
            switch (sub_s004_0e76_20f6(1, 4)) {
            case 1:
                r = 5;
                break;
            case 2:
                r = 7;
                break;
            case 3:
                r = 2;
                break;
            case 4:
                r = 3;
                break;
            case 5:
                r = 4;
                break;
            }
            break;
        case 4:
            switch (sub_s004_0e76_20f6(1, 6)) {
            case 1:
                r = 5;
                break;
            case 2:
                r = 7;
                break;
            case 3:
                r = 2;
                break;
            case 4:
                r = 3;
                break;
            case 5:
                r = 4;
                break;
            case 6:
                r = 8;
                break;
            }
            break;
        case 5:
            switch (sub_s004_0e76_20f6(1, 7)) {
            case 1:
                r = 5;
                break;
            case 2:
                r = 7;
                break;
            case 3:
                r = 2;
                break;
            case 4:
                r = 3;
                break;
            case 5:
                r = 4;
                break;
            case 6:
                r = 8;
                break;
            case 7:
                r = 9;
                break;
            }
            break;
        }
    }
    if (r && force)
        r = 5;
    if (t == 0 && r == 9)
        r = 8;
    return r;
}

/* put object v on cell (x, y) of room r */
void far sub_s007_15af_3f8a(int l, int r, int x, int y, long v)
{
    int _based(D_db39) *o;
    int _based(D_db39) *p;

    if (v >= 3 && v <= 4) {
        o = D_db3b + (l * 30 + r) * 4;
        if (y - o[2] < 2)
            v = 4;
        else if (o[3] - y < 2)
            v = 3;
    }
    F.hgt[x][y] &= 0xf0;
    CB(x, y, 0) &= 0x0f;
    F.cell[x][y] |= (v & 15) << 4;
    F.cell[x][y] |= (long)(unsigned char)(((sub_s004_0e76_20f6(1, 4) - 1) & 3) << 4) << 24;
    p = D_db8f + (l * 40 + x) * 40 + y;
    if (*p > 1 && *p < 9)
        *p = 11;
}

struct gent { int type; char x2, x3, n, x5; };     /* 6 bytes */
struct grp2 {                           /* treasure/guard group, 0x2c bytes at level+0x300e */
    int x0, x2;
    unsigned char x4, x5;
    char x6, x7;
    char x08, x09, x0a;
    unsigned char n;                    /* 0b */
    char x0c, p0d;
    int x0e, x10, x12;
    struct gent ent[4];                 /* 14 */
};
struct item { char p0[0x25]; char x25; char p26[0x2e - 0x26]; };
extern _segment D_dae3;
extern struct item _based(D_dae3) *D_db35;
void far sub_s007_15af_42dc(int, struct grp2 far *, char, int);

/* a chest with contents at (x, y) */
void far sub_s007_15af_4076(int t, int l, int x, int y, int *cnt)
{
    char far *ng;
    struct grp2 far *g;
    int i, j, n, r, w, h;
    int _based(D_db39) *c;
    int _based(D_db39) *o;
    int perm[17];

    ng = (char far *)&F + 0x300c;
    if (*ng >= 8)
        return;
    g = (struct grp2 far *)((char far *)&F + 0x300e) + (*ng)++;
    g->x0c = *cnt;
    (*cnt)++;
    g->x0 = x * 32 - 16;
    g->x2 = y * 16 - 8;
    g->x4 = 0x40;
    g->x5 = 0x20;
    g->x6 = x;
    g->x7 = y;
    for (i = x - 1; i <= x + 1; i++)
        for (j = y - 1; j <= y + 1; j++) {
            c = D_db93 + (l * 40 + i) * 40 + j;
            if (*c == 0)
                *c = 12;
        }
    o = D_db3b + 30 * 4;
    w = o[1] - o[0] + 1;
    h = o[3] - o[2] + 1;
    if (w < h)
        w = h;
    g->x08 = (w + 5) << 1;
    g->x08 |= 0x80;
    g->x0e = sub_s004_0e76_20f6(1, 3) - 1;
    g->x10 = sub_s004_0e76_20f6(1, 6) - 1;
    for (i = 0; i < 16; i++)
        perm[i] = i;
    for (n = 16; n > 0; n--) {
        if (g->n >= 4)
            return;
        r = sub_s004_0e76_20f6(1, n);
        if (r != n) {
            i = perm[r];
            perm[r] = perm[n];
            perm[n] = i;
        }
        switch (r) {
        case 1: sub_s007_15af_42dc(5, g, 2, 4); break;
        case 2: sub_s007_15af_42dc(5, g, 3, 5); break;
        case 3: sub_s007_15af_42dc(5, g, 7, 0); break;
        case 4: sub_s007_15af_42dc(5, g, 14, 14); break;
        case 5: sub_s007_15af_42dc(4, g, 0x45, 0x26); break;
        case 6: sub_s007_15af_42dc(4, g, 0x44, 0x25); break;
        case 7: sub_s007_15af_42dc(6, g, 0x47, 0x28); break;
        case 8: sub_s007_15af_42dc(6, g, 0x46, 0x27); break;
        case 9: sub_s007_15af_42dc(8, g, 0x48, 0x29); break;
        case 10: sub_s007_15af_42dc(8, g, 0x4a, 0x2b); break;
        case 11: sub_s007_15af_42dc(4, g, 0x4e, 0x2f); break;
        case 12: sub_s007_15af_42dc(4, g, 0x4d, 0x2e); break;
        case 13: sub_s007_15af_42dc(6, g, 0x50, 0x31); break;
        case 14: sub_s007_15af_42dc(6, g, 0x4f, 0x30); break;
        case 15: sub_s007_15af_42dc(8, g, 0x51, 0x32); break;
        case 16: sub_s007_15af_42dc(8, g, 0x53, 0x34); break;
        }
    }
}

/* add item type to the group with a 1/chance chance (always for the first) */
void far sub_s007_15af_42dc(int chance, struct grp2 far *g, char x, int type)
{
    struct gent far *e;
    int i;

    if (g->n < 4 && (g->n == 0 || sub_s004_0e76_20f6(1, chance) == 1)) {
        g->x08++;
        e = &g->ent[g->n++];
        e->type = type;
        e->x2 = x;
        e->x3 = 0x23;
        for (i = 0; i < 3; i++) {
            if (sub_s004_0e76_20f6(1, 8) != 1)
                break;
            e->x3 += 10;
            g->x08++;
        }
        e->n = 1;
        for (i = 0; i < 3; i++) {
            if (sub_s004_0e76_20f6(1, chance) != 1)
                break;
            e->n++;
            g->x08++;
        }
        e->x5 = D_db35[e->type].x25;
    }
}
