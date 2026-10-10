/* place: s011 1432:0000 */
/* orig: size=2160 CONST=7d32+0002 _DATA=15a2+004b */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): ALL 17 FUNCTIONS WRITTEN. Templar fortress & tombs battlefields.
 * Match: 0000 (fortress driver), 029e, 1794, 1f5c, 1fb2 (m5: `struct { lv; name[20]; } s` puts the
 * array above the far pointer), 209e (+ 1f5c/209e need _DATA, which now lines up).
 * Close (6.00 register/CSE choices): 0484 (k*0x33fc kept in a register across the call),
 * 05c6 (hgt nibbles share the long temp's slot in 6.00), 096a (6.00 makes a CSE temp per case for
 * &lv->grp[i] / lv), 17fa, 1a2a (code identical, stack slots only: the original has two
 * separate far-segment slots ([bp-10h] es early, [bp-2] cx later) where 6.00A reuses one, o at
 * [bp-18h], lvl [bp-14h], t2 [bp-0ah], CSE temp [bp-1ah]; frame 0x1a), 1bc0, 1cb4, 1ddc, 1968, 1d94,
 * 16d2 (jump table waits for earlier lengths).
 * Level buffer (struct level, 0x33fc): cells, heights, objects (type 1 placeholder, 2 item, 5,
 * 7/8 doors, 0x65 group marker), exits at 0x2fb8 (6 x 14 bytes, count at 0x2fb6), enemy groups at
 * 0x300e (8 x 0x2c), records at 0x316e/0x3236/0x3350/0x3382, monster set name at 0x33c8.
 * D_e047 (based on D_e045) = 4 level buffers; D_db35 (based on D_dae3) = 0x2e-byte item records.
 */

struct res { int a; int b; };

struct obj {                            /* map object, 14 bytes, 55 of them at 0x2186 */
    int x0;                             /* 00: x (pixels) */
    int x2;                             /* 02: y */
    unsigned char x4;                   /* 04: width */
    unsigned char x5;                   /* 05: height */
    int type;                           /* 06: 1 placeholder, 2 item, 4, 5, 7/8 doors, 0x65 group */
    int x8;
    unsigned char x0a;
    char x0b;
    int x0c;                            /* door id / value */
};

struct t1c { char p0[9]; char a; char p0a[0x17 - 0x0a]; char b; char p18[0x1c - 0x18]; };

struct bfdesc {
    char x00;
    char set0[10];                      /* 01 */
    char t0b;                           /* 0b */
    char p0c[0x34 - 0x0c];
    char set2[10];                      /* 34 */
    char t3e;                           /* 3e */
    char p3f[0x67 - 0x3f];
    char set3[10];                      /* 67 */
    char t71;                           /* 71 */
    char p72[0x9a - 0x72];
    char set4[9];                       /* 9a */
    char a3;
    char a4;
    char ta5[2];                        /* a5 */
    char pa7[0xab - 0xa7];
    char tab;                           /* ab */
    char pac[0xb0 - 0xac];
    char tb0;                           /* b0 */
};
extern _segment D_dadd;
extern struct bfdesc _based(D_dadd) *D_db33;
int far _fstrcpy();

struct gent {                           /* 6 bytes */
    int type;
    char b, c, d, e;
};

struct grp2 {                           /* enemy group, 0x2c bytes, at level+0x300e */
    char p0[8];
    unsigned char x08;                  /* 08 */
    char x09;                           /* 09 */
    char x0a;                           /* 0a */
    unsigned char n;                    /* 0b: entries used */
    char x0c;                           /* 0c: group id (D_dff7++) */
    char p0d;
    int x0e;                            /* 0e */
    int x10;                            /* 10 */
    int x12;                            /* 12 */
    struct gent ent[4];                 /* 14 */
};

struct rec10 {                          /* 10 bytes at level+0x316e */
    int obj;                            /* 0: object index */
    char x2;
    char x3;
    char p4[2];
    char x6;
    char p7;
    char x8;
    char x9;
};

struct rec12 {                          /* 12 bytes at level+0x3236 */
    char p0[8];
    char x8[4];
};

struct rec5 {                           /* 10 bytes at level+0x3350 / 0x3382 */
    char b0, b1, b2, b3, b4;
    char p5;
    int x6;
    char p8[2];
};

struct exit {                           /* 14 bytes at level+0x2fb8 */
    char p0[6];
    char lvl;                           /* 06: room number of the other side */
    char x7;
    int x8;
    int x0a;
    char x0c;
    char x0d;                           /* 0d: exit index on the other side */
};

struct level {                          /* battle map (0x33fc bytes) */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41;
    int x1f42[5];                       /* 1f42 */
    char p1f4c[0x1f52 - 0x1f4c];
    struct t1c tbl[(0x2184 - 0x1f52) / 0x1c]; /* 1f52 */
    char p2182[0x2184 - 0x1f52 - (0x2184 - 0x1f52) / 0x1c * 0x1c];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char p2488[0x2fb6 - 0x2488];
    char nexit;                         /* 2fb6: exits used */
    char p2fb7;
    struct exit ex[6];                  /* 2fb8: exits to other rooms */
    char ngrp;                          /* 300c */
    char p300d;
    struct grp2 grp[8];                 /* 300e */
    struct rec10 r10[20];               /* 316e: indexed by object value */
    struct rec12 r12[23];               /* 3236: indexed by door/set number */
    char p334a[0x3350 - 0x334a];
    struct rec5 r7[5];                  /* 3350: type-7 doors */
    struct rec5 r8[7];                  /* 3382: type-8 doors */
    char name[9];                       /* 33c8: monster set name */
    char x33d1;                         /* 33d1 */
    char x33d2;                         /* 33d2 */
    char p33d3[0x33fc - 0x33d3];
};


struct item { char p0[0x25]; char x25; char p26[0x2e - 0x26]; };

extern _segment D_dae3;
extern struct item _based(D_dae3) *D_db35;
extern struct level far F_20a5_0000;
extern _segment D_e045;
extern struct level _based(D_e045) *D_e047;
extern int D_e043;
extern int D_e049[4];
extern char D_e051;
extern int D_dff7;
extern int D_c100;
extern int D_da34;
extern int D_db05[];
struct group {                          /* enemy group, 0x1e bytes */
    int pos;                            /* 00 */
    int pos2;                           /* 02 */
    int x04;                            /* 04 */
    char x06;                           /* 06 */
    char x07;                           /* 07 */
    char type[3];                       /* 08 */
    char count[3];                      /* 0b */
    char p0e[0x1e - 0x0e];
};
struct mon { char p0[0x0c]; char x0c; char p0d[0xd6 - 0x0d]; };
extern struct group _based(D_dadd) *D_db2d;
extern struct mon _based(D_dadd) *D_db2b;
int far sub_s004_0e76_06aa();
static char D_15a2[4] = "Mn";            /* buffer name */
static char D_15a6[] = "fortmons";
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(int, char *);
int far sprintf();
int far sub_root_05ba_0004();
int far sub_root_05d9_019a();
int far sub_s004_0e76_20dc();
int far sub_s004_0e76_20f6();
int far sub_s004_1262_0d0a();
int far sub_s011_1432_0000();
void far sub_s011_1432_029e();
void far sub_s011_1432_0484();
void far sub_s011_1432_05c6();
void far sub_s011_1432_096a();
int far sub_s011_1432_16d2();
void far sub_s011_1432_1794();
void far sub_s011_1432_17fa();
void far sub_s011_1432_1968();
void far sub_s011_1432_1d94();
void far sub_s011_1432_1a2a();
void far sub_s011_1432_1bc0();
void far sub_s011_1432_1cb4();
void far sub_s011_1432_1ddc();
void far sub_s011_1432_1f5c();
int far sub_s011_1432_1fb2();
void far sub_s011_1432_209e();

/* Templar fortress: builds 16 rooms into 4 level buffers, saving them as level%d.flr. */
int far sub_s011_1432_0000(int p_6, int *res, int p_a, int p_c, int p_e, int p_10,
    int p_12, int p_14, int p_16, int p_18)
{
    int t;

    D_e045 = (_segment)(D_e043 = alloc_buffer(0xcff0L, D_15a2));
    D_e047 = 0;
    sub_s004_0e76_20dc();
    sub_s011_1432_029e();
    D_e051 = p_12 == 1 ? 1 : 0;
    sub_s011_1432_0484(D_dff7 = 0, 1);
    sub_s011_1432_0484(1, 2);
    sub_s011_1432_1f5c(1);
    sub_s011_1432_1f5c(2);
    sub_s011_1432_0484(2, 1);
    sub_s011_1432_0484(3, 2);
    sub_s011_1432_0484(4, 0);
    sub_s011_1432_1ddc(1, 0, 0, 0);
    D_e047[2].ex[0] = D_e047[1].ex[0];
    sub_s011_1432_1f5c(1);
    sub_s011_1432_1f5c(2);
    sub_s011_1432_0484(5, 1);
    sub_s011_1432_0484(6, 2);
    sub_s011_1432_0484(7, 3);
    sub_s011_1432_1ddc(1, 0, 0, 1);
    sub_s011_1432_1ddc(2, 0, 0, 2);
    sub_s011_1432_1ddc(3, 0, 0, 3);
    sub_s011_1432_1f5c(1);
    sub_s011_1432_1f5c(2);
    sub_s011_1432_1f5c(3);
    sub_s011_1432_0484(8, 1);
    sub_s011_1432_1ddc(1, 0, 0, 4);
    sub_s011_1432_1f5c(1);
    sub_s011_1432_0484(9, 1);
    sub_s011_1432_0484(10, 2);
    sub_s011_1432_0484(11, 3);
    sub_s011_1432_1ddc(1, 0, 0, 5);
    sub_s011_1432_1ddc(2, 0, 1, 1);
    sub_s011_1432_1ddc(3, 0, 2, 1);
    sub_s011_1432_1f5c(0);
    sub_s011_1432_1f5c(2);
    sub_s011_1432_1f5c(3);
    sub_s011_1432_0484(12, 2);
    sub_s011_1432_0484(13, 3);
    sub_s011_1432_0484(14, 0);
    sub_s011_1432_1ddc(2, 0, 1, 2);
    sub_s011_1432_1ddc(3, 0, 2, 1);
    sub_s011_1432_1ddc(0, 0, 2, 2);
    sub_s011_1432_1f5c(1);
    sub_s011_1432_1f5c(2);
    sub_s011_1432_1f5c(3);
    sub_s011_1432_0484(15, 1);
    D_e047[1].ex[0] = D_e047[0].ex[0];
    sub_s011_1432_1f5c(0);
    sub_s011_1432_1f5c(1);
    free_buffer(D_e043, D_15a2);
    if (p_18 == 1)
        *res = 1;
    else
        *res = 0;
    t = D_c100;
    if (t >= 0 && t <= 15)
        *res = t;
    return 0x10;
}

#define B(o) (((char _based(D_dadd) *)D_db33)[o])

/* tile description for the fortress: 7 tile sets, all "fortmons" */
#pragma optimize("a", on)
void far sub_s011_1432_029e(void)
{
    B(0) = 6;
    _fstrcpy((char far *)&B(1), (char far *)D_15a6);
    B(0x0b) = 1;
    B(0x0c) = 2;
    B(0x0d) = 3;
    _fstrcpy((char far *)&B(0x1a), (char far *)D_15a6);
    B(0x23) = 2;
    B(0x26) = 8;
    B(0x27) = 9;
    B(0x28) = 10;
    B(0x29) = 11;
    B(0x2b) = 12;
    B(0x2a) = 13;
    _fstrcpy((char far *)&B(0x34), (char far *)D_15a6);
    B(0x3e) = 1;
    B(0x41) = 2;
    B(0x42) = 3;
    B(0x44) = 4;
    _fstrcpy((char far *)&B(0x4d), (char far *)D_15a6);
    B(0x58) = 0;
    B(0x56) = 2;
    B(0x59) = 8;
    _fstrcpy((char far *)&B(0x67), (char far *)D_15a6);
    B(0x71) = 1;
    B(0x74) = 2;
    B(0x75) = 3;
    B(0x76) = 0;
    B(0x44) = 4;
    _fstrcpy((char far *)&B(0x80), (char far *)D_15a6);
    B(0x8b) = 0;
    B(0x89) = 2;
    B(0x8c) = 6;
    B(0x8d) = 7;
    _fstrcpy((char far *)&B(0x9a), (char far *)D_15a6);
    B(0xa5) = 5;
    B(0xa6) = 8;
    B(0xa7) = 3;
    B(0xa8) = 2;
    B(0xa9) = 0;
    B(0xaa) = 1;
    B(0xab) = 15;
    B(0xac) = 17;
    B(0xad) = 7;
    B(0xa3) = 12;
    B(0xb0) = 16;
    B(0xb1) = 6;
    B(0xb2) = 10;
    B(0xb4) = 11;
    B(0xb5) = 20;
    B(0xb6) = 21;
    B(0xb7) = 22;
    B(0xb8) = 23;
    B(0xb9) = 24;
    B(0xae) = 25;
    B(0xba) = 25;
    B(0xaf) = 26;
    B(0xbb) = 26;
    B(0xbc) = 27;
    B(0xbd) = 28;
    B(0xbe) = 29;
    B(0xbf) = 30;
}
#pragma optimize("", on)

/* load room n (ifortmon.%03d) into level buffer k */
#pragma optimize("a", on)
void far sub_s011_1432_0484(int n, int k)
{
    char name[40];
    struct level far *lv;
    int i;
    int t;

    if (k < 0 || k >= 4)
        return;
    sprintf(name, "ifortmon.%03d", n);
    sub_root_05d9_019a(name, (struct level far *)(D_e047 + k));
    lv = D_e047 + k;
    lv->x1f40 = n;
    D_e049[k] = n;
    for (i = 0; i < 55; i++) {
        t = lv->obj[i].type;
        if (t != 6 && t != 0 && t != 1 && t != 2 && t != 4 && t != 5 && t != 7 && t != 8)
            break;
    }
    lv->nobj = i;
    lv->x1f42[0] = -2;
    lv->x1f42[1] = 0x12f;
    lv->x1f42[3] = 0x12f;
    lv->x1f42[4] = lv->x1f42[2] = n >= 4 ? -2 : 12;
    lv->x33d1 = 11;
    _fstrcpy(lv->name, (char far *)D_15a6);
    sub_s011_1432_05c6(n, 1, (struct level far *)(D_e047 + k));
    sub_s011_1432_096a(k);
}
#pragma optimize("", on)

/* Remap the tile nibbles of a loaded room to the fortress tile set and randomise
 * wall/floor variants (cells 1..38 x 1..38). */
void far sub_s011_1432_05c6(int n, char mode, struct level far *lv)
{
    unsigned long v;
    unsigned long t;
    unsigned long x;
    int r, c, k;
    unsigned char hi, lo;

    for (r = 1; r < 39; r++) {
        for (c = 1; c < 39; c++) {
            v = lv->cell[r][c];
            if (v & 0x80000000L)
                continue;
            t = ((unsigned)v & 0xf0) >> 4;
            if (mode) {
                switch ((int)t) {
                case 1: t = 4; break;
                case 2: t = 3; break;
                case 3: t = 1; break;
                case 4: t = 2; break;
                case 5: t = 5; break;
                case 6: t = 6; break;
                case 7: t = 14; break;
                case 8: t = 13; break;
                case 9: t = 9; break;
                case 10: t = 12; break;
                case 11: t = 8; break;
                case 12: t = 7; break;
                case 13: t = 10; break;
                case 14: t = 11; break;
                default: t = 0; break;
                }
            } else {
                switch ((int)t) {
                case 1: t = 1; break;
                case 2: t = 2; break;
                case 3: t = 7; break;
                case 10: t = 12; break;
                default: t = 0; break;
                }
            }
            lv->cell[r][c] &= ~0xf0L;
            if (t) {
                lv->cell[r][c] |= (t & 0xf) << 4;
                lv->cell[r][c] |= (unsigned long)(unsigned)((sub_s004_0e76_20f6(1, 4) - 1) & 3) << 28;
            }
            hi = lv->hgt[r][c] >> 4;
            switch (lv->hgt[r][c] & 0xf) {
            case 1: lo = 2; break;
            case 2: lo = 3; break;
            case 3: lo = 4; break;
            case 4: lo = 5; break;
            default: lo = 0; break;
            }
            switch (hi) {
            case 4:
                hi = sub_s004_0e76_20f6(1, 2) + 1;
                if (mode && n < 4 && lo == 0 && sub_s004_0e76_20f6(1, 2) == 1)
                    lo = 7;
                break;
            default:
                hi = 1;
                break;
            }
            lv->hgt[r][c] = (hi << 4) | (lo & 0xf);
            for (k = 0; k <= 24; k += 8) {
                x = (v & (0xfL << k)) >> k;
                if (x) {
                    switch (x) {
                    case 7: x = 4; break;
                    case 8: x = 5; break;
                    case 9: x = 1; break;
                    case 10: x = 6; break;
                    default: x = 1; break;
                    }
                    lv->cell[r][c] &= ~(0xfL << k);
                    lv->cell[r][c] |= (x & 0xf) << k;
                }
            }
            for (k = 12; k <= 20; k += 8) {
                t = (v & (0xfL << k)) >> k;
                if (t) {
                    switch (t) {
                    case 1: t = 2; break;
                    case 2: t = 3; break;
                    case 3: t = 1; break;
                    }
                    lv->cell[r][c] &= ~(0xfL << k);
                    lv->cell[r][c] |= (t & 0xf) << k;
                }
            }
        }
    }
}

#define BIT(n) (D_da34 & (1 << (n)))

/* Per-room setup of level buffer k: enemy groups, doors, objects, start positions. */
void far sub_s011_1432_096a(int k)
{
    struct level far *lv;
    int n;
    int a, b, c;
    struct grp2 far *g;
    struct grp2 far *g1;
    struct grp2 far *g2;
    struct grp2 far *g3;
    struct grp2 far *g8;
    struct grp2 far *g10;
    struct grp2 far *g11;

    if (k < 0 || k >= 4)
        return;
    lv = D_e047 + k;
    switch (n = lv->x1f40) {
    case 0:
        if (!BIT(n)) {
            sub_s011_1432_1a2a(lv, 0, 0x28, 1, 5, -1, 0, 0, 0, 0);
            sub_s011_1432_1a2a(lv, 1, 0x28, 1, 5, -1, 0, 0, 0, 0);
        }
    case 1:
    case 2:
    case 3:
        if (n == 0 || n == 2)
            sub_s011_1432_1a2a(lv, 2, 0x3d, 1, 5, -1, 0, 0, 1, 0);
        sub_s011_1432_1794(g = &lv->grp[0], 0x94, 0xa0, 0x23, 1);
        if (!BIT(n)) {
            g->x0c = D_dff7++;
            g->x10 = 10;
            sub_s011_1432_1794(g, 0x73, 0x98, 0x2d, 1);
            g = &lv->grp[1];
            g->x0c = D_dff7++;
            g->x10 = 12;
            sub_s011_1432_1794(g, 0x70, 0x95, 0x2d, 2);
        }
        lv->x33d2 = 0x18;
        if (n == 0 || n == 2)
            sub_s011_1432_1bc0(lv, 0, 0x0d, 0x12, 0x1e, 0x22, 0x129, 0, 0, 0, 0, 0, 0);
        else
            sub_s011_1432_1bc0(lv, 0, 0x0e, 0x10, 0x1f, 0x21, 0x129, 0, 0, 0, 0, 0, 0);
        break;
    case 4:
        g = &lv->grp[0];
        g->x0c = D_dff7++;
        g->x0e = 0x18;
        g1 = &lv->grp[1];
        g1->x0c = D_dff7++;
        g1->x0a = 5;
        g1->x0a |= 0x30;
        g1->x09 = 0x32;
        sub_s011_1432_1794(g1, 0x1b, 0x1c, 0x23, 6);
        sub_s011_1432_1794(g1, 3, 1, 0x28, 6);
        sub_s011_1432_1794(g1, 0x21, 0x40, 0x23, 0x28);
        g2 = &lv->grp[2];
        g2->x0c = D_dff7++;
        g2->x0a = 0x35;
        g2->x09 = 0x32;
        sub_s011_1432_1794(g2, 0x2c, 0x4b, 0x28, 6);
        sub_s011_1432_1794(g2, 0x35, 0x54, 0x28, 6);
        g3 = &lv->grp[3];
        g3->x0c = D_dff7++;
        g3->x0a = 0x35;
        g3->x09 = 0x32;
        sub_s011_1432_1794(g3, 0x50, 0x75, 0x23, 10);
        sub_s011_1432_1794(g3, 0x52, 0x77, 0x23, 10);
        sub_s011_1432_1794(g3, 0x4b, 0x70, 0x23, 10);
        sub_s011_1432_17fa(lv, 0, 0x88, 0x2d, 1, 0x19);
        sub_s011_1432_17fa(lv, 1, 0x84, 0x14, 1, 0x0f);
        sub_s011_1432_17fa(lv, 2, 0x84, 0x14, 1, 0x0f);
        sub_s011_1432_17fa(lv, 3, 0x8b, 0x23, 1, 0x1e);
        sub_s011_1432_17fa(lv, 4, 0x8b, 0x23, 1, 0x1e);
        sub_s011_1432_1968(lv, 0, 0x14, 1);
        sub_s011_1432_1968(lv, 0x12, 0x1e, 1);
        sub_s011_1432_1968(lv, 1, 0x1e, 1);
        sub_s011_1432_1968(lv, 0x13, 0x1e, 1);
        sub_s011_1432_1a2a(lv, 0, 0x28, 2, 6, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 1, 0x3a, 1, 3, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 2, 0x3a, 1, 4, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 3, 0x28, 2, 6, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 4, 0x3a, 1, 4, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 5, 0x28, 2, 6, -1, 0, 0, 0, 0);
        lv->x33d2 = 0x1a;
        sub_s011_1432_1bc0(lv, 0, 0x12, 0x14, 0x12, 0x14, 0x12d, 0, 0, 0x13, 0x13, 0, 0);
        break;
    case 5:
        sub_s011_1432_1a2a(lv, 0, 0x28, 2, 7, -1, 0, 0, 1, 0);
        lv->x33d2 = 0x17;
        sub_s011_1432_1cb4(lv, 0, 0x12, 0x12, 0x0f, 0x0f, 0, 0x12e, 0);
        sub_s011_1432_1bc0(lv, 0, 0x15, 0x17, 0x0e, 0x10, 0x16c, 1, 0, 0, 0, 0, 0);
        break;
    case 6:
        sub_s011_1432_1968(lv, 1, 0x14, 0);
        if (!BIT(n)) {
            sub_s011_1432_1a2a(lv, 0, 0x35, 1, 4, -1, 0, 0, 0, 0);
            sub_s011_1432_1a2a(lv, 1, 0x35, 1, 5, -1, 0, 0, 0, 0);
        }
        lv->x33d2 = 0x1d;
        sub_s011_1432_1bc0(lv, 0, 0x0c, 0x0e, 0x19, 0x1b, 0x12c, 0, 0, 0x0d, 0x1b, 0, 0);
        break;
    case 7:
        if (!BIT(n))
            sub_s011_1432_1a2a(lv, 0, 0x1e, 3, 1, 0x28, 2, 5, 0, 0);
        sub_s011_1432_1a2a(lv, 1, 0x28, 1, 4, -1, 0, 0, 1, 0);
        lv->x33d2 = 0x14;
        sub_s011_1432_1bc0(lv, 0, 0x12, 0x14, 0x1e, 0x20, 0x12b, 0, 0, 0x13, 0x1f, 0, 0);
        break;
    case 8:
        g8 = &lv->grp[0];
        g8->x0c = D_dff7++;
        g8->x10 = 3;
        g8->x0a = 0x35;
        g8->x09 = 0x32;
        g8->x0e = 10;
        sub_s011_1432_1794(g8, 10, 10, 0x2d, 1);
        sub_s011_1432_1794(g8, 0x5a, 0x7f, 0x23, 10);
        sub_s011_1432_1794(g8, 0x38, 0x61, 0x2d, 2);
        sub_s011_1432_1968(lv, 0, 0x19, 1);
        lv->tbl[1].b = 0x9e;
        lv->tbl[1].a = 0;
        sub_s011_1432_1a2a(lv, 0, 0x2d, 1, 7, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 1, 0x2d, 2, 7, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 2, 0x2f, 1, 7, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 3, 0x28, 2, 6, -1, 0, 0, 1, 0);
        lv->x33d2 = 0x1b;
        sub_s011_1432_1bc0(lv, 0, 0x20, 0x23, 0x20, 0x22, 0x130, 0, 0, 0x22, 0x21, 0, 0);
        sub_s011_1432_1bc0(lv, 1, 0x0d, 0x0d, 0x0c, 0x0c, 0x16b, 0, 0, 0x0d, 0x0c, 0, 0);
        lv->cell[13][12] &= ~0xf00L;
        lv->cell[13][12] |= 0x500L;
        return;
    case 9:
        sub_s011_1432_17fa(lv, 0, 0x8b, 0x2d, 1, 0x23);
        sub_s011_1432_17fa(lv, 1, 0x8b, 0x2d, 1, 0x23);
        sub_s011_1432_1968(lv, 2, 0x32, 0);
        sub_s011_1432_1a2a(lv, 0, 0x28, 2, 6, -1, 0, 0, 1, 0);
        if (!BIT(n))
            sub_s011_1432_1a2a(lv, 1, 0x3b, 1, 2, -1, 0, 0, 0, 0);
        sub_s011_1432_1cb4(lv, 0, 0x15, 0x15, 0x0d, 0x0d, 1, 0x133, 0);
        sub_s011_1432_1bc0(lv, 0, 2, 3, 0x1b, 0x1d, 0x135, 0, 0, 0, 0, 0, 0);
        lv->ngrp = 0;
        return;
    case 10:
        if (!BIT(n)) {
            g10 = &lv->grp[0];
            g10->x0c = D_dff7++;
            g10->x0e = 0x11;
            g10->x10 = 6;
            g10->x12 = 11;
            sub_s011_1432_1794(g10, 0x20, 0x21, 0x32, 1);
            sub_s011_1432_1794(g10, 0x3b, 0x64, 0x23, 1);
            sub_s011_1432_1794(g10, 0x42, 0x6b, 0x23, 1);
            sub_s011_1432_1794(g10, 0x40, 0x69, 0x23, 1);
        }
        sub_s011_1432_1968(lv, 1, 0x31, 0);
        sub_s011_1432_1968(lv, 2, 0x23, 1);
        if (!BIT(n)) {
            sub_s011_1432_1a2a(lv, 1, 0x3d, 1, 7, -1, 0, 0, 0, 0);
            sub_s011_1432_1a2a(lv, 2, 0x18, 2, 1, 0, 3, 6, 0, 0);
            sub_s011_1432_1a2a(lv, 3, 0x28, 2, 1, 0, 5, 5, 0, 0);
        }
        sub_s011_1432_1a2a(lv, 0, 0x28, 2, 6, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 4, 0x47, 3, 1, -1, 0, 0, 1, 1);
        lv->x1f42[0] = 0x13c;
        lv->x33d2 = 0x1c;
        sub_s011_1432_1bc0(lv, 0, 3, 4, 0x21, 0x21, 0x13b, 1, 0, 4, 0x21, 0, 0);
        sub_s011_1432_1bc0(lv, 1, 9, 0x0b, 0x22, 0x23, 0x13c, 1, 0, 0x0c, 0x23, 0, 0);
        break;
    case 11:
        if (!BIT(n)) {
            g11 = &lv->grp[0];
            g11->x0c = D_dff7++;
            g11->x10 = 12;
            g11->x0a = 0x35;
            g11->x09 = 0x32;
            sub_s011_1432_1794(g11, 0x48, 0x6d, 0x23, 6);
            sub_s011_1432_1794(g11, 0x4f, 0x74, 0x23, 3);
            sub_s011_1432_1794(g11, 0x4d, 0x72, 0x23, 3);
            sub_s011_1432_1794(g11, 0x4e, 0x73, 0x23, 3);
        }
        sub_s011_1432_17fa(lv, 0, 0x8b, 0x2d, 1, 0x19);
        sub_s011_1432_17fa(lv, 1, 0x8b, 0x2d, 1, 0x1e);
        sub_s011_1432_17fa(lv, 2, 0x8b, 0x2d, 1, 0x23);
        sub_s011_1432_1968(lv, 1, 0x0f, 0);
        if (!BIT(n)) {
            sub_s011_1432_1a2a(lv, 0, 0x2d, 2, 8, -1, 0, 0, 0, 0);
            sub_s011_1432_1a2a(lv, 1, 0x2d, 1, 5, -1, 0, 0, 0, 0);
        }
        lv->x33d2 = 0x15;
        sub_s011_1432_1bc0(lv, 0, 0x17, 0x19, 0x17, 0x19, 0x13a, 0, 0, 0x18, 0x18, 0, 0);
        break;
    case 12:
        sub_s011_1432_1968(lv, 0, 0x14, 0);
        sub_s011_1432_1968(lv, 1, 0x1e, 0);
        if (BIT(n))
            break;
        sub_s011_1432_1bc0(lv, 0, 0x10, 0x10, 0x17, 0x1a, 0x136, 0, 0, 0x10, 0x18, 0, 0);
        sub_s011_1432_1d94(lv, 0, 0x137);
        return;
    case 13:
        sub_s011_1432_1a2a(lv, 0, 0x46, 3, 7, 0x45, 2, 1, 0, 0);
        sub_s011_1432_1a2a(lv, 1, 0, 5, 4, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 2, 0, 5, 4, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 3, 0x51, 3, 1, 0x28, 2, 2, 1, 0);
        lv->x33d2 = 0x16;
        sub_s011_1432_1bc0(lv, 0, 0x1b, 0x1e, 7, 0x0d, 0x139, 0, 0, 0x1c, 8, 0, 0);
        break;
    case 14:
        a = sub_s011_1432_16d2();
        do
            b = sub_s011_1432_16d2();
        while (b == a);
        do
            c = sub_s011_1432_16d2();
        while (c == a || b == c);
        lv->grp[0].x0c = D_dff7++;
        lv->grp[0].x08 = 0x2d;
        lv->grp[0].x08 |= 0x80;
        sub_s011_1432_1794(&lv->grp[0], a, 0x9f, 0x23, 1);
        lv->grp[1].x0c = D_dff7++;
        lv->grp[1].x08 = 0xad;
        sub_s011_1432_1794(g1, b, 0x9f, 0x23, 1);
        lv->grp[2].x0c = D_dff7++;
        lv->grp[2].x08 = 0xad;
        sub_s011_1432_1794(g2, c, 0x9f, 0x23, 1);
        sub_s011_1432_1968(lv, 0, 0x1e, 0);
        sub_s011_1432_1a2a(lv, 0, 0x48, 2, 4, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 1, 0x49, 3, 6, -1, 0, 0, 0, 0);
        sub_s011_1432_1a2a(lv, 2, 0x18, 2, 1, 0x28, 2, 5, 0, 0);
        sub_s011_1432_1a2a(lv, 3, 0x37, 2, 1, -1, 0, 0, 1, 1);
        lv->x33d2 = 0x1e;
        sub_s011_1432_1bc0(lv, 0, 0x13, 0x15, 0x0c, 0x0d, lv->x1f42[0] = 0x138, 1, 1, 0x13, 8, 0, 0);
        break;
    case 15:
        lv->ngrp = 0;
        sub_s011_1432_1a2a(lv, 0, 0x37, 1, 1, -1, 0, 0, 0, 1);
        lv->x1f42[0] = 0x138;
        return;
    }
}

int far sub_s011_1432_16d2(void)
{
    switch (sub_s004_0e76_20f6(1, 0x16)) {
    case 1:
        return 0x96;
    case 2:
        return 0x97;
    case 3:
        return 0x98;
    case 4:
        return 0x99;
    case 5:
        return 0x9a;
    case 6:
        return 0x9b;
    case 7:
        return 0x9c;
    case 8:
        return 0x9d;
    case 9:
        return 0x9e;
    case 10:
        return 0x9f;
    case 11:
        return 0xa0;
    case 12:
        return 0xa1;
    case 13:
        return 0xa2;
    case 14:
        return 0xa3;
    case 15:
        return 0xa4;
    case 16:
        return 0xa5;
    case 17:
        return 0xa6;
    case 18:
        return 0xa7;
    case 19:
        return 0xa8;
    case 20:
        return 0xa9;
    case 21:
        return 0xaa;
    case 22:
        return 0xab;
    }
}

#pragma optimize("a", on)
void far sub_s011_1432_1794(struct grp2 far *g, int type, char b, char c, char d)
{
    struct gent far *e;

    if (g->n < 4) {
        e = &g->ent[g->n];
        g->n++;
        e->type = type;
        e->b = b;
        e->c = c;
        e->d = d;
        e->e = D_db35[type].x25;
    }
}
#pragma optimize("", on)

/* find the type-1 object with id k and turn it into an item/monster of type `type` */
#pragma optimize("g", off)
void far sub_s011_1432_17fa(struct level far *lv, int k, int type, char b, char c, int val)
{
    struct obj far *o;
    int i, j, old;

    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        if (o->type == 1 && *(int far *)&o->x0a == k)
            break;
    }
    if (i >= lv->nobj)
        return;
    old = o->x0c;
    o->x0c = val;
    for (j = 0; j < 4; j++)
        if (lv->r12[k].x8[j])
            lv->r12[k].x8[j] = 6;
    if (type >= 0x84 && type <= 0x99) {
        lv->r10[old].x2 = type + 0x7c;
        lv->r10[old].x3 = b;
        lv->r10[old].obj = i;
        o->type = 2;
        o->x8 = 0;
    } else {
        lv->r10[old].x8 = type;
        lv->r10[old].x9 = b;
        lv->r10[old].x6 = c;
        o->type = 4;
        o->x8 = 0;
        lv->r10[old].obj = i;
    }
}
#pragma optimize("", on)
#pragma optimize("", on)

#pragma optimize("a", on)
void far sub_s011_1432_1968(struct level far *lv, int idx, int val, char flag)
{
    struct obj far *o;
    struct obj far *p;
    int i;

    if (!flag) {
        lv->tbl[idx].b = lv->tbl[idx].a = val | 0x80;
        return;
    }
    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        if (o->type == 0 && o->x0a == idx)
            break;
    }
    if (i < lv->nobj) {
        o->x0c = val;
        lv->obj[o->x0b].x0c = val;
    }
}
#pragma optimize("", on)

/* place an enemy group (up to two monster kinds) at group marker object id+11 */
void far sub_s011_1432_1a2a(struct level far *lv, int id, int mtype, int cnt, int n1,
    int mtype2, int cnt2, int n2, char neg, char x)
{
    struct obj far *o;
    struct group far *g;
    int lvl, i, t1, t2, j;

    lvl = lv->x1f40;
    id += 11;
    for (i = 0; i < 55; i++) {
        o = &lv->obj[i];
        if (o->type == 0x65 && *(int far *)&o->x0a == id)
            break;
    }
    if (i >= 55)
        return;
    if (D_e051) {
        cnt = cnt - 1 < 1 ? 1 : cnt - 1;
        cnt2 = cnt2 - 1 < 1 ? 1 : cnt2 - 1;
    }
    t1 = sub_s004_0e76_06aa(lvl, mtype, cnt);
    D_db2b[lvl * 12 + t1].x0c = x;
    t2 = sub_s004_0e76_06aa(lvl, mtype2, cnt2);
    if (t1 < 0 && t2 < 0)
        return;
    g = &D_db2d[lvl * 10 + D_db05[lvl]];
    j = 0;
    if (t1 >= 0 && n1 > 0) {
        g->type[0] = t1;
        g->count[0] = n1;
        j = 1;
    }
    if (t2 >= 0 && n2 > 0) {
        g->type[j] = t2;
        g->count[j] = n2;
        j++;
    }
    if (j > 0) {
        g->pos = o->x0 + 16;
        g->pos2 = o->x2 + 8;
        g->x06 = o->x0c;
        g->x04 = 16;
        g->x07 = 0;
        if (neg)
            g->pos = -g->pos;
        D_db05[lvl]++;
    }
}

/* set the extent of type-7 door k (cells a..b x c..d) and its data */
#pragma optimize("z", on)
void far sub_s011_1432_1bc0(struct level far *lv, int k, int a, int b, int c, int d, int val,
    char p16, char flag, char p1a, char p1c, char p1e, char p20)
{
    struct obj far *o;
    int lvl, i;

    if (k < 0 || k >= 5)
        return;
    lvl = lv->x1f40;
    for (i = 0; i < 55; i++) {
        o = &lv->obj[i];
        if (o->type == 7 && o->x0a == k)
            break;
    }
    if (i < 55) {
        o->x0 = a << 5;
        o->x4 = (b - a + 1) << 5;
        o->x2 = c << 4;
        o->x5 = (d - c + 1) << 4;
        if (flag)
            o->x0b |= 2;
        o->x0c = val;
        lv->r7[k].b0 = p16;
        lv->r7[k].b1 = p1a;
        lv->r7[k].b2 = p1c;
        lv->r7[k].b3 = p1e;
        lv->r7[k].b4 = p20;
        if (lvl == 10 && k == 1)
            lv->r7[k].x6 -= 16;
    }
}
#pragma optimize("", on)

/* set the extent of type-8 door k */
void far sub_s011_1432_1cb4(struct level far *lv, int k, int a, int b, int c, int d,
    char hi, int val, char flag)
{
    struct obj far *o;
    int i, old;

    if (k < 0 || k >= 5)
        return;
    for (i = 0; i < 55; i++) {
        o = &lv->obj[i];
        if (o->type == 8 && o->x0a == k)
            break;
    }
    if (i < 55) {
        o->x0 = a << 5;
        o->x4 = -(a - b) + 1 << 5;
        o->x2 = c << 4;
        o->x5 = (d - c + 1) << 4;
        if (flag)
            o->x0b |= 1;
        if (hi)
            old = ((unsigned char far *)&o->x0c)[1];
        else
            old = o->x0c & 0xff;
        o->x0c = val;
        lv->r8[k].b0 = 0;
        lv->r8[k].b3 = old;
    }
}

void far sub_s011_1432_1d94(struct level far *lv, int n, int val)
{
    struct obj far *o;
    struct obj far *p;
    int i;
    int k;

    for (i = k = 0, p = lv->obj; i < 55; i++, p++) {
        *(struct obj far * volatile *)&o = p;
        if (p->type == 5) {
            if (n == k)
                break;
            k++;
        }
    }
    if (i < 55)
        o->x0c = val;
}
#pragma optimize("", on)

/* connect exit a of level buffer k1 with exit b of level buffer k2 */
#pragma optimize("a", on)
void far sub_s011_1432_1ddc(int k1, int a, int k2, int b)
{
    char c;
    char d;
    int w;

    if (k1 < 0 || k1 >= 4 || k2 < 0 || k2 >= 4 || a < 0 || a >= 6 || b < 0 || b >= 6)
        return;
    D_e047[k1].nexit = D_e047[k1].nexit < a + 1 ? a + 1 : D_e047[k1].nexit;
    D_e047[k2].nexit = D_e047[k2].nexit < b + 1 ? b + 1 : D_e047[k2].nexit;
    D_e047[k1].ex[a].lvl = D_e047[k2].x1f40;
    D_e047[k2].ex[b].lvl = D_e047[k1].x1f40;
    D_e047[k1].ex[a].x0d = b;
    D_e047[k2].ex[b].x0d = a;
    c = D_e047[k1].ex[a].x7;
    D_e047[k1].ex[a].x7 = D_e047[k2].ex[b].x7;
    D_e047[k2].ex[b].x7 = c;
    w = D_e047[k1].ex[a].x8;
    D_e047[k1].ex[a].x8 = D_e047[k2].ex[b].x8;
    D_e047[k2].ex[b].x8 = w;
    w = D_e047[k1].ex[a].x0a;
    D_e047[k1].ex[a].x0a = D_e047[k2].ex[b].x0a;
    D_e047[k2].ex[b].x0a = w;
    d = D_e047[k1].ex[a].x0c;
    D_e047[k1].ex[a].x0c = D_e047[k2].ex[b].x0c;
    D_e047[k2].ex[b].x0c = d;
}
#pragma optimize("", on)

void far sub_s011_1432_1f5c(int k)
{
    char name[0x42];

    if (k >= 0 && k < 4) {
        sprintf(name, "level%d.flr", D_e047[k].x1f40);
        sub_s004_1262_0d0a(name, (struct level far *)(D_e047 + k), 0x33fc, 0);
    }
}

int far sub_s011_1432_1fb2(int p_6, int p_8, int p_a, int count)
{
    struct { struct level far *lv; char name[20]; } s;
    int n;

    sub_s004_0e76_20dc();
    sub_s011_1432_209e();
    n = sub_s004_0e76_20f6(1, 2);
    sprintf(s.name, "imisctom.%03d", n);
    s.lv = &F_20a5_0000;
    sub_root_05d9_019a(s.name, s.lv);
    F_20a5_0000.x1f40 = 0;
    for (n = 0; n < 55 && F_20a5_0000.obj[n].type == 6; n++)
        ;
    F_20a5_0000.nobj = n;
    F_20a5_0000.x1f42[0] = -2;
    F_20a5_0000.x1f42[1] = -2;
    F_20a5_0000.x1f42[2] = -2;
    F_20a5_0000.x1f42[3] = -2;
    F_20a5_0000.x1f42[4] = -2;
    sub_s011_1432_05c6(0, 0, (*(struct level far * volatile *)&s.lv));
    for (n = 1; n <= count; n++)
        sub_s011_1432_1a2a((*(struct level far * volatile *)&s.lv), n, 0x2d, 2, 1, -1, 0, 0, 0, 0);
    sub_s004_1262_0d0a("level0.flr", (*(struct level far * volatile *)&s.lv), 0x33fc, 0);
    return 1;
}

#pragma optimize("a", on)
void far sub_s011_1432_209e(void)
{
    char keep[5] = "keep";
    char keep1[6] = "keep1";

    D_db33->x00 = 0;
    _fstrcpy((char far *)D_db33->set0, (char far *)keep);
    D_db33->t0b = 2;
    _fstrcpy((char far *)D_db33->set2, (char far *)keep1);
    D_db33->t3e = 1;
    _fstrcpy((char far *)D_db33->set3, (char far *)keep1);
    D_db33->t71 = 1;
    _fstrcpy((char far *)D_db33->set4, (char far *)keep);
    D_db33->ta5[0] = 13;
    D_db33->ta5[1] = 14;
    D_db33->tab = 18;
    D_db33->a3 = 12;
    D_db33->tb0 = 16;
}
#pragma optimize("", on)
