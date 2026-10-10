/* place: s009 1432:0000 */
/* orig: size=3fa2 _DATA=14d0+0060 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=14d0 */
/* status (e10, 2026-10-09): mine battlefields, part 1 (the other source file of s009, see
 * s009_182c.c). ALL 32 FUNCTIONS WRITTEN from the asm (dec16 managed only 01e0 07e4 0af8 0cfa);
 * no DATA DIFFERS, stamped --wip. 12 match: 0000 019e (m1: fill loops under pragma a) 01e0 0732 0a18 0af8 0cfa 12b2 18fc 3f54 3f6c
 * 3f8a. Close: 0398 and 1948 (stack slot order only), 0d90 (4 lines, compare operand order),
 * 2252 (AX known-zero reuse, 6.00), 0990 (6.00 keeps D_e037 offset/segment in AX/CX across the
 * store), 3a90 (frame 1 word larger; 6.00 keeps s in SI and uses `sub ax,ax; inc ax`), 07e4 (6.00
 * constant-propagates k). The big generators (1af0..37b4) are written but far off. A mine is built from up to 16 sections ("iminegen.%03d"
 * random sections, "iminspec.%03d" special ones) loaded into 4 level slots of the 0xcff0-byte
 * buffer D_e037 (based on D_db37); D_e033 (based on D_dfc3) holds a 40x40 flag map per level
 * (0x80/0x40/0x20/0x10 walls, 2 = occupied, 1 = marked). D_dfd5[section] = slot,
 * D_dfcd[slot] = section, D_dfc5[slot] = number of exits the section file had.
 */

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

struct res { int a; int b; };
struct obj {                            /* map object, 14 bytes */
    int x0, x2;                         /* 00: position (pixels) */
    unsigned char x4, x5;               /* 04: size */
    int type;                           /* 06: 1 item, 2/4 chest, 5 exit, 6, 7 door, 8 wall gap */
    int x8;
    int x0a;                            /* 0a: record index */
    int x0c;                            /* 0c: value */
};
struct r14 {                            /* 14 bytes at level+0x1f54 */
    int x0;
    char p2[6];
    unsigned char x8, x9;               /* 08: cell row / column (+ 0x80/0x40 wall side flags) */
    char p0a[4];
};
struct exit {                           /* 14 bytes at level+0x2fb8 */
    int x0, x2;                         /* 00: position (pixels) */
    unsigned char x4, x5;               /* 04: size */
    char x6;                            /* 06: level number of the other side */
    char x7;
    int x8, x0a;                        /* 08: arrival position on the other side */
    char x0c;                           /* 0c: direction */
    char x0d;                           /* 0d: exit index on the other side */
};
struct grp2 {                           /* group record, 0x2c bytes at level+0x300e */
    int x0, x2;
    unsigned char x4, x5, x6, x7;
    unsigned char x08;
    char x09;
    char x0a;
    unsigned char n;                    /* 0b */
    char x0c;                           /* 0c: group id */
    char p0d;
    int x0e;
    int x10;
    int x12;
    char ent[24];
};
struct rec10 {                          /* 10 bytes at level+0x316e */
    int x0;
    char x2, x3;
    char p4[2];
    char x6;
    char p7;
    char x8, x9;
};
struct rec12 { char p0[8]; char x8[4]; };   /* 12 bytes at level+0x3236 */
struct r3350 { char b0, b1, b2, b3, b4, b5; int x6; int x8; };     /* 10 bytes */
struct r3382 { char b0, b1, b2, b3; int x4; int x6; char b8, b9; };  /* 10 bytes */
struct level {                          /* battle map (0x33fc bytes) */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41;
    int x1f42, x1f44, x1f46, x1f48, x1f4a; /* 1f42 */
    int x1f4c, x1f4e;                   /* 1f4c: party start */
    char x1f50, x1f51;
    char n1f52;                         /* 1f52 */
    char p1f53;
    struct r14 r1f54[40];               /* 1f54 */
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char x2488;                         /* 2488 */
    char p2489;
    int x248a[30];                      /* 248a */
    int list[0x578];                    /* 24c6 */
    char nexit;                         /* 2fb6 */
    char p2fb7;
    struct exit exit[6];                /* 2fb8 */
    char n300c;                         /* 300c */
    char p300d;
    struct grp2 grp[8];                 /* 300e */
    struct rec10 rec10[20];             /* 316e */
    struct rec12 rec12[20];             /* 3236 */
    char n3326;                         /* 3326 */
    char p3327;
    int r3328[3][5];                    /* 3328: rectangles {x, y, w, h} */
    int x3346, x3348;                   /* 3346 */
    char p334a[6];
    struct r3350 r3350[5];              /* 3350 */
    struct r3382 r3382[5];              /* 3382 */
    char p33b4[0x33fc - 0x33b4];
};
struct fmap { unsigned char m[40][40]; };
struct subgrp { char t; char x1; int x2; int x4; char x6; char x7; char x8; char p9; };
struct group {                          /* enemy group, 0x1e bytes (see s010) */
    int pos, pos2;
    int x04;
    char x06;
    char x07;
    char type[3];
    char count[3];
    char p0e[6];
    struct subgrp sub;
};
struct mon { char p0[0x0c]; char x0c; char p0d[0xd6 - 0x0d]; };
struct member { char status; char pad[0x7f]; };

extern unsigned D_dfbd, D_dfc1;
extern _segment D_db37, D_dfc3;
extern struct level _based(D_db37) *D_e037;
extern struct fmap _based(D_dfc3) *D_e033;
extern _segment D_dadd;
extern struct group _based(D_dadd) *D_db2d;
extern struct mon _based(D_dadd) *D_db2b;
extern int D_db05[];
extern int D_dff7;
extern char D_e035;
extern unsigned char D_e036;
extern int D_dfc5[4], D_dfcd[4], D_dfd5[16];
extern char D_e00d[21];
extern char D_e022[16];
extern int D_c100, D_c0fe;
extern char D_9080;
extern int D_da34;
extern char D_11dd;
extern int D_13d0[];
extern long D_dff9, D_dffd, D_e001, D_e005, D_e009;
extern struct member D_9c69[];
extern char D_77c8[];

unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
int far sprintf();
int far fprintf();
void exit_(int);
void *memset(void *, int, unsigned);
int far sub_root_05d9_019a();
int far sub_s004_0e76_06aa();
int far sub_s004_0e76_20dc();
int far sub_s004_0e76_20f6();
int far sub_s004_1091_000a();
unsigned char far sub_s004_1091_17e0(unsigned char, int);
int far sub_s004_1262_0d0a();
void far sub_s009_182c_0002();
void far sub_s009_182c_03ca();
int far sub_s009_182c_05a2();
void far sub_s009_182c_061a();
void far sub_s009_182c_07d0();
void far sub_s009_182c_0938();
void far sub_s009_182c_09b4();
char far sub_s009_182c_0db6();
void far sub_s009_182c_1260();
int far sub_s009_182c_1cb4();
int far sub_s009_182c_20a4();
int far sub_s009_182c_219e();
int far vec_1c1b();
int far sub_s009_1432_0000();
void far sub_s009_1432_019e(void);
int far sub_s009_1432_01e0();
int far sub_s009_1432_0218();
int far sub_s009_1432_0398();
int far sub_s009_1432_05e4();
int far sub_s009_1432_0732();
int far sub_s009_1432_07e4();
int far sub_s009_1432_0990();
int far sub_s009_1432_0a18();
int far sub_s009_1432_0af8();
int far sub_s009_1432_0cfa();
void far sub_s009_1432_0d90(int, int, int, int *, char);
int far sub_s009_1432_12b2();
void far sub_s009_1432_1358();
void far sub_s009_1432_18fc();
void far sub_s009_1432_1948();
void far sub_s009_1432_1af0();
void far sub_s009_1432_1da4();
void far sub_s009_1432_205e();
void far sub_s009_1432_2252();
void far sub_s009_1432_23d6(int, int, int, int, int, int, int, int, char, int, int, char);
void far sub_s009_1432_26cc();
void far sub_s009_1432_2964();
void far sub_s009_1432_2c98();
void far sub_s009_1432_37b4();
int far sub_s009_1432_3a90(int, int, char);
void far sub_s009_1432_3bea();
void far sub_s009_1432_3e1c();
char far sub_s009_1432_3f54();
char far sub_s009_1432_3f6c();
char far sub_s009_1432_3f8a();

#define SLOT(sec) (((sec) >= 0 && (sec) < 16) ? D_dfd5[sec] : -1)
#define R(n) sub_s004_0e76_20f6(1, n)

/* mine battle for location type t: allocate the four level slots and the flag maps, build the
 * mine; returns the number of levels */
int far sub_s009_1432_0000(int t, int a, int b, int c, int d, int e, int f, int g, int h)
{
    char *m1 = "Mn";
    char *m2 = "Tl";
    int r;

    D_db37 = (_segment)(D_dfbd = alloc_buffer(0xcff0L, m1));
    D_e037 = 0;
    D_dfc3 = (_segment)(D_dfc1 = alloc_buffer(0x6400L, m2));
    D_e033 = 0;
    D_dff7 = 0;
    D_e035 = sub_s009_1432_3f8a(t);
    D_e036 = 0;
    switch (t) {
    case 0x87:
        t = 0x0b;
        break;
    case 0x88:
        t = 0x0d;
        break;
    case 0x89:
        t = 0x10;
        break;
    }
    sub_s009_1432_019e();
    sub_s009_182c_0002(t);
    switch (t) {
    default:
        r = sub_s009_1432_01e0(t);
        break;
    case 0x0b:
        r = sub_s009_1432_0398(t);
        break;
    case 0x0c:
    case 0x7f:
    case 0x80:
    case 0x81:
        r = sub_s009_1432_05e4(t, a, b, c);
        break;
    case 0x0d:
        r = sub_s009_1432_07e4(t);
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
        r = sub_s009_1432_0af8(t);
        break;
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
        r = sub_s009_1432_0218(t);
        break;
    case 0x6c:
    case 0x6d:
        r = sub_s009_1432_0990(t);
        break;
    case 0x82:
        r = sub_s009_1432_0732(t, a, b, c, g, h);
        break;
    case 0x83:
        r = sub_s009_1432_0a18(t, g, h);
        break;
    case 0x84:
    case 0x85:
    case 0x86:
        r = sub_s009_1432_0cfa(t, g, h);
        break;
    }
    free_buffer(D_dfbd, m1);
    free_buffer(D_dfc1, m2);
    return r;
}

/* clear the section bookkeeping */
#pragma optimize("a", on)
void far sub_s009_1432_019e(void)
{
    int i;

    for (i = 0; i < 21; i++) D_e00d[i] = 0;
    for (i = 0; i < 4; i++) D_dfcd[i] = -1;
    for (i = 0; i < 4; i++) D_dfc5[i] = 0;
    for (i = 0; i < 16; i++) D_dfd5[i] = -1;
    for (i = 0; i < 16; i++) D_e022[i] = 0;
}
#pragma optimize("", on)

/* one-section mine */
int far sub_s009_1432_01e0(int t)
{
    int r;

    sub_s009_1432_0d90(t, 0, 0, &r, 1);
    sub_s009_1432_37b4(t, 0, -1, -1);
    sub_s009_1432_3e1c(t, 0);
    return 1;
}

/* small mines 0x66..0x69: one section with one or two groups */
int far sub_s009_1432_0218(int t)
{
    int g, r, n, s;

    sub_s009_1432_0d90(t, 0, 0, &g, 0);
    r = vec_1c1b();
    for (g = 0; g < 5; g++)
        D_9c69[g];
    s = R((r + 1) / 2) + 1;
    switch (t) {
    case 0x66:
        sub_s009_1432_23d6(t, g = sub_s004_0e76_06aa(0, 0x3b, 1), s, 0, -1, 0, 0, 0, 0, 0, -1,
            -1);
        break;
    case 0x67:
        n = R(4) + 2;
        g = sub_s004_0e76_06aa(0, 0x12, max(min(r, 5), 1));
        sub_s009_1432_23d6(t, g, 1, 0, sub_s004_0e76_06aa(0, 7, s), n, 0, 0, 0, 0, -1, -1);
        break;
    case 0x68:
        sub_s009_1432_23d6(t, g = sub_s004_0e76_06aa(0, 0x3a, 1), s, 0, -1, 0, 0, 0, 0, 0, -1,
            -1);
        break;
    case 0x69:
        n = s / 2;
        s -= n;
        sub_s009_1432_23d6(t, g = sub_s004_0e76_06aa(0, 0x31, 1), s, 0, -1, 0, 0, 0, 0, 0, -1,
            -1);
        sub_s009_1432_23d6(t, g, n, 0, -1, 0, 0, 0, 0, 1, -1, -1);
        break;
    }
    D_e037->r3382[0].b1 = D_e037->r3382[0].x4 = 0;
    sub_s009_1432_3e1c(t, 0);
    return 1;
}

/* mine 0x0b: sections 1..14 */
int far sub_s009_1432_0398(int t)
{
    int a, b, c, k, r, i, j, x;

    r = vec_1c1b();
    sub_s004_0e76_20dc();
    sub_s009_1432_0d90(t, 3, 0, &a, 1);
    sub_s009_1432_0d90(t, 4, k = 1, &b, 0);
    sub_s009_1432_37b4(t, 1, 2, 3);
    sub_s009_1432_2964(t, 1, 2, 3);
    sub_s009_1432_2964(t, 2, 3, -1);
    sub_s009_1432_2c98(t, 3, 4, -1, a);
    sub_s009_1432_3e1c(t, 1);
    sub_s009_1432_3e1c(t, 2);
    sub_s009_1432_3e1c(t, 3);
    j = -1;
    for (i = 7; i <= 7; i++) {
        if (j >= 0)
            break;
        sub_s009_1432_0d90(t, i, ++k, &b, 0);
        sub_s009_1432_2964(t, 4, i, -1);
        j = i;
    }
    if (j < 0) {
        sub_s009_1432_0d90(t, 8, ++k, &a, 1);
        sub_s009_1432_2964(t, 4, 8, -1);
        j = 8;
    }
    sub_s009_1432_0d90(t, 10, ++k, &c, 1);
    sub_s009_1432_2964(t, 4, 10, -1);
    *(volatile int *)&x = 10;
    sub_s009_1432_3e1c(t, 4);
    for (i = j + 1; i <= 8; i++) {
        if (i == 8 || R(10) <= r) {
            sub_s009_1432_0d90(t, i, ++k, &a, i == 8);
            sub_s009_1432_2964(t, j, i, -1);
            sub_s009_1432_3e1c(t, j);
            j = i;
        }
    }
    sub_s009_1432_2c98(t, 8, -1, -1, a);
    sub_s009_1432_3e1c(t, 8);
    sub_s009_1432_0d90(t, 0x0d, ++k, &b, 0);
    sub_s009_1432_0d90(t, 0x0e, ++k, &b, 0);
    sub_s009_1432_2c98(t, 10, 11, 13, c);
    sub_s009_1432_2964(t, 11, 13, -1);
    sub_s009_1432_2964(t, 13, 14, -1);
    sub_s009_1432_3e1c(t, 10);
    sub_s009_1432_3e1c(t, 11);
    sub_s009_1432_3e1c(t, 13);
    sub_s009_1432_3e1c(t, 14);
    return k + 1;
}

/* mine 0x0c / lairs 0x7f..0x81: one section, monsters (kind ft, quality fq, number fn) spread
 * over up to five groups */
int far sub_s009_1432_05e4(int t, int ft, int fq, int fn)
{
    int r, n, e, q, rem, g, i, off, x, c;
    int j, k;

    r = vec_1c1b();
    switch (t) {
    case 0x7f:
        ft = 0x2a;
        fq = 1;
        n = r * 4 + 2;
        break;
    case 0x80:
        ft = 0x2f;
        fq = 1;
        n = r * 2 + 1;
        break;
    case 0x81:
        ft = 0x34;
        fq = 1;
        n = r * 3 / 2;
        break;
    default:
        n = fn;
        break;
    }
    if (ft == 0x34) {
        x = (r - 1) / 3 + 2;
        e = n - x;
        n = x;
    } else if (n > 8) {
        e = n - 8;
        n = 8;
    } else
        e = 0;
    q = n / 5;
    rem = n % 5;
    sub_s009_1432_0d90(t, 0, 0, &g, 0);
    g = sub_s004_0e76_06aa(0, ft, fq);
    j = k = 0;
    for (i = 0, off = 0; off < 50; i++, off += 10) {
        c = q;
        if (rem > i)
            c++;
        if (c <= 0)
            break;
        x = (off == 0 && e > 0) ? e * 10 + 1 : 0;
        sub_s009_1432_23d6(t, g, c, x, -1, 0, 0, 0, 0, i, -1, -1);
        D_e037->r3382[i].b1 = D_e037->r3382[i].x4 = 0;
    }
    sub_s009_1432_3e1c(t, 0);
    return 1;
}

/* mine 0x82: single section lv (12 = the 0x0c layout) */
int far sub_s009_1432_0732(int t, int a, int b, int c, int lv, int p)
{
    char flag;

    if (lv == 12)
        return sub_s009_1432_05e4(lv, a, b, c);
    if (lv < 1 || lv > 14)
        lv = 1;
    flag = (lv == 3 || lv == 8 || lv == 10) ? 1 : 0;
    sub_s009_1432_0d90(t, lv, 0, &p, flag);
    sub_s009_1432_37b4(t, lv, -1, -1);
    sub_s009_1432_2964(t, lv, -1, -1);
    if (lv == 4)
        sub_s009_1432_2964(t, lv, -1, -1);
    if (flag)
        sub_s009_1432_2c98(t, lv, -1, -1, p);
    sub_s009_1432_3e1c(t, lv);
    return 1;
}

/* mine 0x0d */
int far sub_s009_1432_07e4(int t)
{
    int a, b, c, d, e, k;

    vec_1c1b();
    sub_s004_0e76_20dc();
    sub_s009_1432_0d90(t, 1, 0, &a, 1);
    sub_s009_1432_0d90(t, 2, 1, &b, 0);
    sub_s009_1432_0d90(t, 3, 2, &c, 0);
    sub_s009_1432_37b4(t, 1, -1, -1);
    sub_s009_1432_2c98(t, 1, 2, -1, a);
    sub_s009_1432_2c98(t, 2, 3, -1, b);
    sub_s009_1432_3e1c(t, 1);
    sub_s009_1432_3e1c(t, 2);
    sub_s009_1432_0d90(t, 7, 3, &a, 0);
    sub_s009_1432_2964(t, 3, 6, 7);
    k = 4;
    sub_s009_1432_0d90(t, 0x0b, k, &b, 0);
    sub_s009_1432_2964(t, 3, 0x0a, d = 0x0b);
    sub_s009_1432_3e1c(t, 3);
    sub_s009_1432_0d90(t, 5, ++k, &c, 0);
    sub_s009_1432_2964(t, 7, 5, -1);
    sub_s009_1432_2964(t, 0x0b, 5, -1);
    sub_s009_1432_3e1c(t, 5);
    sub_s009_1432_0d90(t, 8, ++k, &d, 1);
    sub_s009_1432_2c98(t, 7, 8, -1, a);
    sub_s009_1432_2c98(t, 8, -1, -1, d);
    sub_s009_1432_3e1c(t, 7);
    sub_s009_1432_3e1c(t, 8);
    sub_s009_1432_0d90(t, 0x0c, ++k, &e, 1);
    sub_s009_1432_2c98(t, 0x0b, 0x0c, -1, b);
    sub_s009_1432_2c98(t, 0x0c, -1, -1, e);
    sub_s009_1432_3e1c(t, 0x0b);
    sub_s009_1432_3e1c(t, 0x0c);
    return k + 1;
}

/* special one-section mines 0x6c/0x6d (0x6d: floor tile 7 under the first enemy group) */
int far sub_s009_1432_0990(int t)
{
    int r, x, y;

    sub_s009_1432_0d90(t, 0, 0, &r, 0);
    if (t == 0x6d) {
        x = D_db2d->pos / 32;
        y = D_db2d->pos2 / 16;
        D_e037->cell[x][y] &= ~0xf0L;
        D_e037->cell[x][y] |= 0x70;
    }
    sub_s009_1432_3e1c(t, 0);
    return 1;
}

/* mine 0x83: single section lv */
int far sub_s009_1432_0a18(int t, int lv, int p)
{
    char flag;

    if (lv == 9)
        return sub_s009_1432_0990(0x6c);
    if (lv == 13)
        return sub_s009_1432_0990(0x6d);
    if (lv < 1 || lv > 13)
        lv = 1;
    flag = (lv == 1 || lv == 8 || lv == 12) ? 1 : 0;
    sub_s009_1432_0d90(t, lv, 0, &p, flag);
    sub_s009_1432_37b4(t, lv, -1, -1);
    sub_s009_1432_2964(t, lv, -1, -1);
    if (lv == 4 || lv == 6 || lv == 7 || lv == 10 || lv == 11)
        sub_s009_1432_2964(t, lv, -1, -1);
    if (lv == 1 || lv == 2 || lv == 7 || lv == 8 || lv == 11 || lv == 12)
        sub_s009_1432_2c98(t, lv, -1, -1, p);
    sub_s009_1432_3e1c(t, lv);
    return 1;
}

/* mines 0x0e..0x10 */
int far sub_s009_1432_0af8(int t)
{
    int a, b, c;

    vec_1c1b();
    sub_s004_0e76_20dc();
    sub_s009_1432_0d90(t, 1, 0, &a, 1);
    sub_s009_1432_0d90(t, 2, 1, &c, 0);
    sub_s009_1432_37b4(t, 1, -1, -1);
    sub_s009_1432_2c98(t, 1, 2, -1, a);
    sub_s009_1432_3e1c(t, 1);
    sub_s009_1432_0d90(t, 0x0a, 2, &b, 0);
    sub_s009_1432_2c98(t, 2, 6, 0x0a, c);
    sub_s009_1432_2964(t, 6, 0x0a, -1);
    sub_s009_1432_3e1c(t, 6);
    sub_s009_1432_3e1c(t, 0x0a);
    sub_s009_1432_0d90(t, 3, 3, &a, 0);
    sub_s009_1432_2964(t, 2, 3, -1);
    sub_s009_1432_0d90(t, 0x0b, 4, &b, 0);
    sub_s009_1432_2c98(t, 3, 7, 0x0b, a);
    sub_s009_1432_2964(t, 7, 0x0b, -1);
    sub_s009_1432_3e1c(t, 2);
    sub_s009_1432_3e1c(t, 7);
    sub_s009_1432_3e1c(t, 0x0b);
    sub_s009_1432_0d90(t, 4, 5, &a, 0);
    sub_s009_1432_2964(t, 3, 4, -1);
    sub_s009_1432_0d90(t, 0x0c, 6, &b, 0);
    sub_s009_1432_2c98(t, 4, 8, 0x0c, a);
    sub_s009_1432_2964(t, 8, 0x0c, -1);
    sub_s009_1432_3e1c(t, 3);
    sub_s009_1432_3e1c(t, 8);
    sub_s009_1432_3e1c(t, 0x0c);
    sub_s009_1432_0d90(t, 5, 7, &a, 0);
    sub_s009_1432_2964(t, 4, 5, -1);
    sub_s009_1432_0d90(t, 0x0d, 8, &b, 1);
    sub_s009_1432_2c98(t, 5, 9, 0x0d, a);
    sub_s009_1432_2964(t, 9, 0x0d, -1);
    sub_s009_1432_3e1c(t, 4);
    sub_s009_1432_3e1c(t, 5);
    sub_s009_1432_3e1c(t, 9);
    sub_s009_1432_3e1c(t, 0x0d);
    return 9;
}

/* mines 0x84..0x86: single section lv */
int far sub_s009_1432_0cfa(int t, int lv, int p)
{
    if (lv < 1 || lv > 0x0d)
        lv = 1;
    sub_s009_1432_0d90(t, lv, 0, &p, lv == 1 || lv == 0x0d);
    sub_s009_1432_37b4(t, lv, -1, -1);
    sub_s009_1432_2964(t, lv, -1, -1);
    if (lv >= 2 && lv <= 5)
        sub_s009_1432_2964(t, lv, -1, -1);
    if (lv >= 1 && lv <= 5)
        sub_s009_1432_2c98(t, lv, -1, -1, p);
    sub_s009_1432_3e1c(t, lv);
    return 1;
}

/* load section sec of mine t as level lvno into a free slot: a special map (iminspec.nnn) or a
 * random unused general one (iminegen.nnn), rotated by *rot (random if < 0 or not a
 * fixed-layout mine), then fill in walls, heights, objects, groups */
#pragma optimize("a", on)
void far sub_s009_1432_0d90(int t, int sec, int lvno, int *rot, char special)
{
    int slotn;
    int cntl;
    char specg;
    char exf;
    char namec[16];
    int x0m;
    int xn;
    int y0n;
    int yo;
    int kn;
    int j;
    struct level far *lvi;
    int numl;

    slotn = -1;
    if (sec >= 0 && sec < 16)
        slotn = sub_s009_1432_12b2(sec);
    if (slotn < 0 || slotn >= 4)
        return;
    exf = sub_s009_1432_3f6c(t);
    switch (t) {
    case 0x82:
        t = 0x0b;
        break;
    case 0x83:
        t = 0x0d;
        break;
    case 0x84:
        t = 0x0e;
        break;
    case 0x85:
        t = 0x0f;
        break;
    case 0x86:
        t = 0x10;
        break;
    }
    numl = -100;
    specg = 0;
    if (exf && D_c100 >= 0)
        numl = -D_c100;
    else if (D_e035)
        numl = 13;
    switch (t) {
    case 0x0a:
        if (D_c100 >= 0)
            numl = -D_c100;
        break;
    case 0x0b:
        switch (sec) {
        case 13:
            numl = 0;
            break;
        case 14:
            numl = 9;
            break;
        }
        break;
    case 0x0c:
    case 0x7f:
    case 0x80:
    case 0x81:
        numl = 1;
        break;
    case 0x6c:
        numl = 2;
        break;
    case 0x6d:
        numl = 3;
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
        switch (sec) {
        case 10:
            numl = 10;
            break;
        case 11:
            numl = 11;
            break;
        case 12:
            numl = 12;
            break;
        case 13:
            numl = 4;
            break;
        }
        break;
    case 0x66:
        numl = 5;
        break;
    case 0x67:
        numl = 6;
        break;
    case 0x68:
        numl = 7;
        break;
    case 0x69:
        numl = 8;
        break;
    }
    if (numl >= 0) {
        sprintf(namec, "iminspec.%03d", numl);
        kn = -numl;
        specg = 1;
    } else {
        if (numl < 0 && numl > -100)
            kn = -numl;
        else {
            for (kn = cntl = 0; kn < 21; kn++)
                if (D_e00d[kn] == 0)
                    cntl++;
            if (cntl <= 0)
                cntl = 21;
            j = R(cntl);
            for (cntl = kn = 0; kn < 21; kn++) {
                if (cntl >= j)
                    goto found;
                if (D_e00d[kn] == 0)
                    cntl++;
            }
found:;
        }
        sprintf(namec, "iminegen.%03d", kn - 1);
        D_e00d[kn - 1] = 1;
    }
    sub_root_05d9_019a(namec, (struct level far *)(D_e037 + slotn));
    lvi = (struct level far *)(D_e037 + slotn);
    if (specg)
        D_e036 |= 1 << slotn;
    if (exf && *rot >= 0)
        *rot %= 8;
    else
        *rot = R(8) - 1;
    sub_s004_1091_000a((struct level far *)(D_e037 + slotn), *rot, 1);
    for (kn = 0; lvi->n300c > kn; kn++)
        lvi->grp[kn].x0c = sub_s004_1091_17e0(lvi->grp[kn].x0c, *rot);
    D_e037[slotn].x3346 = kn - 1;
    D_e037[slotn].x3348 = *rot;
    lvi->x1f40 = lvno;
    D_dfc5[slotn] = lvi->nexit;
    lvi->nexit = 0;
    if (!sub_s009_1432_3f54(t))
        lvi->n3326 = 0;
    kn = 0;
    do {
        cntl = lvi->obj[kn].type;
        if (cntl != 6 && cntl != 0 && cntl != 1 && cntl != 2 && cntl != 4)
            break;
    } while (++kn < 55);
    lvi->nobj = kn;
    lvi->x1f42 = -2;
    lvi->x1f48 = lvi->x1f44 = 0x2e;
    lvi->x1f4a = lvi->x1f46 = 0x0c;
    for (kn = 0; kn < 40; kn++)
        for (j = 0; j < 40; j++)
            D_e033[lvno].m[kn][j] = 0xff;
    sub_s009_1432_1358(t, sec);
    sub_s009_1432_18fc(t, sec);
    sub_s009_1432_1948(t, sec);
    sub_s009_1432_1da4(t, sec);
    if (special || D_e035)
        sub_s009_1432_26cc(t, sec);
    sub_s009_1432_1af0(t, sec);
    sub_s009_1432_2252(t, sec);
    if (lvi->x1f4c > 0 && lvi->x1f4e > 0) {
        y0n = lvi->x1f4e / 16;
        x0m = lvi->x1f4c / 32;
        for (xn = x0m - 1; xn <= x0m + 1; xn++)
            for (yo = y0n - 1; yo <= y0n + 1; yo++)
                D_e033[lvi->x1f40].m[xn][yo] |= 2;
    }
}
#pragma optimize("", on)

/* level slot (0..3) for section sec: the one it already has or a free one */
int far sub_s009_1432_12b2(int sec)
{
    int s, i;

    s = -1;
    if (sec >= 0 && sec < 16)
        s = D_dfd5[sec];
    if (s >= 0 && s < 4) {
        D_dfcd[s] = sec;
        return s;
    }
    for (i = 0; i < 4; i++)
        if (D_dfcd[i] == sec)
            return D_dfd5[sec] = i;
    for (i = 0; i < 4; i++)
        if (D_dfcd[i] < 0 || D_dfcd[i] >= 17) {
            D_dfcd[i] = sec;
            D_dfd5[sec] = i;
            D_e022[sec] = 1;
            return i;
        }
    fprintf((void *)D_77c8, "ERROR: no slot in which to store mine section %d\n", sec);
    exit_(1);
}

/* walls of section sec: random wall types (03ca/061a/07d0/0938 give the mine parameters), runs
 * of solid walls, heights and floor variation; marks the flag map */
void far sub_s009_1432_1358(int t, int sec)
{
    long tmpo;
    long rotk;
    int sloto;
    int a0ao;
    char cold[40];
    int a0el;
    char b38f;
    int a3ek;
    long e3ci;
    int a42n;
    int a40;
    int u44i;
    int a48j;
    int a46l;
    int rf;
    long v4ck;
    int lvnoj;
    long v56i;
    int cg;
    struct level far *lvi;
    long v5ao;
    int u60a;
    char b64a;
    char rowe;
    int runi;
    long v68j;
    long cure;

    sloto = -1;
    if (sec >= 0 && sec < 16)
        sloto = D_dfd5[sec];
    if (sloto < 0 || sloto >= 4)
        return;
    lvi = (struct level far *)(D_e037 + sloto);
    lvnoj = lvi->x1f40;
    sub_s009_182c_03ca(t, sec, &runi);
    sub_s009_182c_061a(t, sec, &a42n, &a48j, &a40);
    sub_s009_182c_07d0(t, sec, &a0ao);
    sub_s009_182c_0938(t, sec, &a3ek, &a46l, &a0el);
    for (cg = 1; cg < 40; cg++)
        cold[cg] = 0;
    for (rf = 38; rf > 0; rf--) {
        rowe = 0;
        for (cg = 38; cg > 0; cg--) {
            cure = lvi->cell[rf][cg];
            if (cure & 0x80000000L)
                continue;
            D_e033[lvnoj].m[rf][cg] = 0;
            e3ci = (cure >> 16) & 0xf;
            if (e3ci != 0) {
                if (runi > 1) {
                    if (!cold[cg] && R(runi) == 1)
                        cold[cg] = 1;
                    else if (cold[cg] && R(4) == 1)
                        cold[cg] = 0;
                }
                if (cold[cg])
                    e3ci = 3;
                else
                    e3ci = R(2) == 1 ? D_dff9 : D_dffd;
                lvi->cell[rf][cg] &= ~0xf0000L;
                lvi->cell[rf][cg] |= (e3ci & 0xf) << 16;
                if (!cold[cg] && a48j > 0 && R(a40) == 1) {
                    b64a = R(a48j) + a42n - 1;
                    lvi->cell[rf][cg] |= (long)((b64a & 0xf) << 4) << 16;
                }
            }
            v5ao = cure & 0xf;
            if (v5ao != 0) {
                tmpo = lvi->cell[rf + 1][cg];
                if (!(tmpo & 0x80000000L))
                    v5ao = (tmpo >> 8) & 0xf;
                else
                    v5ao = R(2) == 1 ? D_e001 : D_e005;
                lvi->cell[rf][cg] &= ~0xfL;
                lvi->cell[rf][cg] |= v5ao & 0xf;
            }
            v4ck = (cure >> 24) & 0xf;
            if (v4ck != 0) {
                tmpo = lvi->cell[rf][cg + 1];
                if (!(tmpo & 0x80000000L))
                    v4ck = (tmpo >> 16) & 0xf;
                else
                    v4ck = R(2) == 1 ? D_dff9 : D_dffd;
                lvi->cell[rf][cg] &= ~0xf000000L;
                lvi->cell[rf][cg] |= (v4ck & 0xf) << 24;
            }
            v68j = (cure >> 8) & 0xf;
            if (v68j != 0) {
                if (runi > 1) {
                    if (!rowe && R(runi) == 1)
                        rowe = 1;
                    else if (rowe && R(4) == 1)
                        rowe = 0;
                }
                if (rowe)
                    v68j = 3;
                else
                    v68j = R(2) == 1 ? D_e001 : D_e005;
                lvi->cell[rf][cg] &= ~0xf00L;
                lvi->cell[rf][cg] |= (v68j & 0xf) << 8;
            }
            b64a = 0;
            b38f = lvi->hgt[rf][cg] >> 4;
            if (b38f) {
                b38f = R(a0ao) + (char)D_e009 - 1;
                if (e3ci == 3 && v68j == 3)
                    b64a = 5;
                else if (e3ci == 3)
                    b64a = 3;
                else if (v68j == 3)
                    b64a = 4;
                else if (a46l > 0 && R(a0el) == 1)
                    b64a = R(a46l) + a3ek - 1;
                lvi->hgt[rf][cg] = (b64a & 0xf) | (b38f << 4);
            }
            v56i = (cure & 0xf0) >> 4;
            if (v56i) {
                v56i = 9;
                lvi->cell[rf][cg] &= ~0xf0L;
                lvi->cell[rf][cg] |= 0x90;
                rotk = R(4) - 1;
                lvi->cell[rf][cg] |= (rotk & 3) << 28;
            }
            if (e3ci)
                D_e033[lvnoj].m[rf][cg] |= 0x80;
            if (v5ao)
                D_e033[lvnoj].m[rf][cg] |= 0x40;
            if (v4ck)
                D_e033[lvnoj].m[rf][cg] |= 0x20;
            if (v68j)
                D_e033[lvnoj].m[rf][cg] |= 0x10;
            if (v56i)
                D_e033[lvnoj].m[rf][cg] |= 1;
        }
    }
}

/* section 1: other start area */
void far sub_s009_1432_18fc(int t, int sec)
{
    int slot;
    struct level far *lv;

    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot >= 0 && slot < 4) {
        lv = (struct level far *)(D_e037 + slot);
        if (sec == 1)
            lv->x1f4a = lv->x1f46 = 12;
    }
}

#pragma optimize("a", on)
void far sub_s009_1432_1948(int t, int sec)
{
    int slot, i, sh, a1a, a18;
    long v;
    struct level far *lv;
    struct r14 far *p;
    struct obj far *o;

    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    lv = (struct level far *)(D_e037 + slot);
    sub_s009_182c_05a2(t, sec, &a1a, &a18);
    for (i = 0; i < lv->n1f52; i++) {
        p = &lv->r1f54[i];
        if (p->x8 & 0x80)
            sh = 16;
        else if (p->x8 & 0x40)
            sh = 0;
        else if (p->x9 & 0x80)
            sh = 24;
        else
            sh = (p->x9 & 0x40) ? 8 : 0;
        if (p->x0 >= 0)
            v = R(a18) + a1a - 1;
        else
            v = sh >= 8 ? D_e001 : D_dff9;
        lv->cell[p->x8 & 0x3f][p->x9 & 0x3f] |= v << sh;
        D_e033[lv->x1f40].m[p->x8 & 0x3f][p->x9 & 0x3f] |= 2;
    }
    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        if (o->type == 1)
            o->x0c = R(12) + sec + 15;
    }
}
#pragma optimize("", on)

/* chests and items of section sec */
void far sub_s009_1432_1af0(int t, int sec)
{
    int slot, i, j, c, k, id, a10, a14, a12, x, y, x0, x1, y0, y1;
    struct level far *lv;
    struct obj far *o;
    struct rec12 far *q;

    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    lv = (struct level far *)(D_e037 + slot);
    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        switch (o->type) {
        case 1:
            o->x0c = R(10) + sec + 20;
            q = &lv->rec12[o->x0a];
            for (j = 0; j < 4; j++)
                if (q->x8[j])
                    q->x8[j] = 6;
            break;
        case 2:
        case 4:
            sub_s009_182c_09b4(t, sec, &a10, &a14, &a12);
            c = o->x0c;
            k = o->x0a;
            id = lv->rec10[k].x0;
            if (a10 >= 0x84 && a10 <= 0x99) {
                if (k >= 0) {
                    lv->rec10[k].x0 = id;
                    lv->rec10[k].x2 = a10 + 0x7c;
                    lv->rec10[k].x3 = a14;
                }
                o->type = 2;
                o->x8 = 0;
                o->x0c = -1;
            } else {
                if (a12 > 3)
                    a12 = 3;
                if (k >= 0) {
                    lv->rec10[k].x0 = id;
                    lv->rec10[k].x8 = a10;
                    lv->rec10[k].x9 = a14;
                    lv->rec10[k].x6 = a12;
                }
                if (c >= 0) {
                    lv->rec10[c].x0 = id;
                    lv->rec10[c].x8 = a10;
                    lv->rec10[c].x9 = a14;
                    lv->rec10[c].x6 = a12;
                }
                o->type = 4;
                o->x8 = 0;
            }
            y0 = o->x2 / 16;
            y1 = y0 + (o->x5 >> 4) - 1;
            x0 = o->x0 / 32;
            x1 = x0 + (o->x4 >> 5) - 1;
            for (x = x0; x <= x1; x++)
                for (y = y0; y <= y1; y++)
                    D_e033[lv->x1f40].m[x][y] |= 2;
            break;
        }
    }
}

/* groups of section sec: keep those 182c_0db6 accepts (compacting the list), give them ids and
 * random contents */
void far sub_s009_1432_1da4(int t, int sec)
{
    int slot, n, i, x, y, x0, x1, y0, y1, m;
    char r;
    struct level far *lv;
    struct grp2 far *s;
    struct grp2 far *d;

    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    i = n = 0;
    lv = (struct level far *)(D_e037 + slot);
    if (lv->n300c > 0) {
        s = d = &lv->grp[0];
        do {
            if (12 - lv->x1f40 > 20 - D_dff7)
                break;
            if (n != i) {
                d->x0 = s->x0;
                d->x2 = s->x2;
                d->x4 = s->x4;
                d->x5 = s->x5;
                d->x6 = s->x6;
                d->x7 = s->x7;
            }
            r = sub_s009_182c_0db6(lv, d, t, sec, n);
            if (r) {
                lv->cell[d->x6][d->x7] |= 0x50;
                x0 = d->x0 / 32;
                x1 = x0 + (d->x4 >> 5) - 1;
                y0 = d->x2 / 16;
                y1 = y0 + (d->x5 >> 4) - 1;
                D_e033[lv->x1f40].m[d->x6][d->x7] |= 1;
                for (x = x0; x <= x1; x++)
                    for (y = y0; y <= y1; y++)
                        D_e033[lv->x1f40].m[x][y] |= 2;
                if (r == 1) {
                    d->x0c = D_dff7++;
                    switch (R(3)) {
                    case 1:
                        d->x0a = 5;
                        d->x0a = 0x25;
                        m = sub_s004_0e76_20f6(2, 2) + sec / 4 - 3;
                        m = max(min(m, 2), 0);
                        d->x09 = m * 30 + 20;
                        break;
                    case 2:
                        d->x08 = (sec + 2) * 5;
                        d->x08 |= 0x80;
                        break;
                    }
                }
                d++;
                n++;
            }
            s++;
        } while (++i < lv->n300c);
    }
    lv->n300c = n;
}

/* add a door object for exit record e (id) to level lv and a free 0x3350 record */
void far sub_s009_1432_205e(struct level far *lv, struct exit far *e, int id)
{
    int k, j, x, y, xx, yy;
    struct r3350 far *p;
    struct obj far *o;

    for (k = 0, p = &lv->r3350[0]; k < 5; k++, p++)
        if (!(p->x6 > 0 && p->x6 < 0x500 && p->x8 > 0 && p->x8 < 0x280))
            break;
    if (k >= 5)
        return;
    for (j = 54; j > lv->nobj; j--)
        lv->obj[j] = lv->obj[j - 1];
    o = &lv->obj[lv->nobj];
    lv->nobj++;
    o->x0 = e->x0;
    o->x4 = e->x4;
    o->x2 = e->x2;
    o->x5 = e->x5;
    o->type = 7;
    o->x8 = 0;
    o->x0c = id;
    o->x0a = k;
    p->b0 = 0;
    p->x6 = e->x6 * 32 + 16;
    p->x8 = e->x7 * 16 + 8;
    switch (p->b5 = e->x0c) {
    case 0:
        p->x8 += 0x18;
        break;
    case 2:
        p->x6 -= 0x30;
        break;
    case 4:
        p->x8 -= 0x18;
        break;
    case 6:
        p->x6 += 0x30;
        break;
    }
    y = p->x8 / 16 + 1;
    x = p->x6 / 32 + 1;
    for (xx = x - 1; xx <= x + 1; xx++)
        for (yy = y - 1; yy <= y + 1; yy++)
            D_e033[lv->x1f40].m[xx][yy] |= 2;
}

/* the six enemy groups of section sec (182c_1260 gives their contents) */
void far sub_s009_1432_2252(int t, int sec)
{
    int slot, lvno, i, g1, g2;
    int a24, a22, a20, a1e, a1c, a1a, a18, a16;
    char b0f, b0e, b0d;
    struct level far *lv;

    if (sub_s009_1432_3f54(t))
        return;
    if (t == 0x0c || t == 0x7f || t == 0x80 || t == 0x81)
        return;
    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    lv = (struct level far *)(D_e037 + slot);
    lvno = lv->x1f40;
    for (i = 0; i < 6; i++) {
        sub_s009_182c_1260(t, sec, i, &a24, &a22, &a20, &a1e, &b0f, &a1c, &a1a, &a18, &a16,
            &b0e, &b0d);
        if ((g1 = sub_s004_0e76_06aa(lvno, a24, a22)) >= 0)
            D_db2b[lvno * 12 + g1].x0c = b0f;
        if ((g2 = sub_s004_0e76_06aa(lvno, a1c, a1a)) >= 0)
            D_db2b[lvno * 12 + g2].x0c = b0e;
        sub_s009_1432_23d6(t, g1, a20, a1e, g2, a18, a16, sec, b0d, i, -1, -1);
        if (i < 5)
            lv->r3382[i].b1 = lv->r3382[i].x4 = 0;
    }
}

/* create an enemy group in level of section sec: kinds t1/t2 with counts n1/n2 (x1/x2 > 0: a
 * sub-group, x % 10 / x / 10); position from marker 6 (mine 0x6c), from (pos, pos2) or from the
 * 0x3382 record pos */
void far sub_s009_1432_23d6(int t, int t1, int n1, int x1, int t2, int n2, int x2, int sec,
    char flag, int pos, int pos2, char dir)
{
    int slot, off, lvno, k, x, y, j;
    struct level far *lv;
    struct group far *g;
    struct subgrp far *h;
    struct r3382 far *r;

    if (t1 < 0 && t2 < 0)
        return;
    if (pos < 0)
        return;
    if (pos >= 6 && pos2 < 0)
        return;
    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    off = slot * 0x33fc;
    lv = (struct level far *)(D_e037 + slot);
    lvno = lv->x1f40;
    g = D_db2d + lvno * 10 + D_db05[lvno];
    if (t == 0x6c && pos >= 5) {
        for (k = 0; k < 55; k++)
            if (lv->obj[k].type == 0x65 && lv->obj[k].x0a == 6)
                break;
        if (k >= 55)
            return;
        g->pos = lv->obj[k].x0 + 16;
        g->pos2 = lv->obj[k].x2 + 8;
        g->x06 = lv->obj[k].x0c;
    } else if (pos2 > 0) {
        g->pos = pos;
        g->pos2 = pos2;
        g->x06 = dir;
    } else {
        if (pos >= 5)
            return;
        r = &((struct level far *)((char _based(D_db37) *)D_e037 + off))->r3382[pos];
        g->pos = r->x4;
        g->pos2 = r->x6;
        g->x06 = r->b3;
    }
    x = g->pos / 32;
    y = g->pos2 / 16;
    if (flag && !sub_s009_1432_3f6c(t))
        g->pos = -g->pos;
    D_e033[lvno].m[x][y] |= 2;
    g->x04 = 16;
    if (flag)
        g->x04 = 32;
    g->x07 = 0;
    j = 0;
    if (t1 >= 0 && (n1 > 0 || x1 > 0)) {
        g->type[0] = t1;
        g->count[0] = n1;
        j = 1;
        if (x1 > 0) {
            g->x07 = 1;
            g->sub.t = t1;
            h = &g->sub;
            h->x1 = g->x04;
            h->x2 = g->pos;
            h->x4 = g->pos2;
            h->x6 = x1 % 10;
            h->x7 = x1 / 10;
            h->x8 = 4;
        }
    }
    if (t2 >= 0 && (n2 > 0 || x2 > 0)) {
        g->type[j] = t2;
        g->count[j] = n2;
        j++;
        if (x2 > 0) {
            g->x07 = 1;
            g->sub.t = t2;
            h = &g->sub;
            h->x1 = g->x04;
            h->x2 = g->pos;
            h->x4 = g->pos2;
            h->x6 = x2 % 10;
            h->x7 = x2 / 10;
            h->x8 = 4;
        }
    }
    if (j > 0)
        D_db05[lvno]++;
}

/* special mines: turn the exit placeholders (type 5) into real objects (doors on section 13 of
 * mines 0x0e..0x10 / 0x84..0x86) */
void far sub_s009_1432_26cc(int t, int sec)
{
    int slot, n, i, x, y, x0, x1, y0, y1;
    struct level far *lv;
    struct obj far *o;
    struct obj far *p;
    struct obj tmp;
    struct r3350 far *r;

    slot = -1;
    if (sec >= 0 && sec < 16)
        slot = D_dfd5[sec];
    if (slot < 0 || slot >= 4)
        return;
    if (D_11dd && !sub_s009_1432_3f6c(t))
        return;
    lv = (struct level far *)(D_e037 + slot);
    n = lv->nobj;
    for (i = n; i < 55; i++) {
        o = &lv->obj[i];
        if (o->type != 5)
            continue;
        if (n != i) {
            tmp = *o;
            *o = lv->obj[n];
            lv->obj[n] = tmp;
        }
        p = &lv->obj[n];
        p->x0c = 0x30;
        if (((t >= 0x0e && t <= 0x10) || (t >= 0x84 && t <= 0x86)) && sec == 13) {
            p->type = 7;
            p->x8 = 0;
            p->x0c = 0xe9;
        }
        lv->nobj = ++n;
        x0 = p->x0 / 32;
        x1 = x0 + (p->x4 >> 5) - 1;
        y0 = p->x2 / 16;
        y1 = y0 + (p->x5 >> 4) - 1;
        for (x = x0; x <= x1; x++)
            for (y = y0; y <= y1; y++)
                D_e033[lv->x1f40].m[x][y] |= 2;
        r = &lv->r3350[p->x0a];
        y0 = r->x8 / 16;
        x0 = r->x6 / 32;
        for (x = x0 - 1; x <= x0 + 1; x++)
            for (y = y0 - 1; y <= y0 + 1; y++)
                D_e033[lv->x1f40].m[x][y] |= 2;
    }
}

/* connect section a with section b (or c if b has no slot) through an exit on each side */
void far sub_s009_1432_2964(int t, int a, int b, int c)
{
    int sa, sb, bs, ea, eb, x, y, xx, yy;
    char t7, tc;
    int ti;
    struct level far *lva;
    struct level far *lvb;
    struct exit far *pa;
    struct exit far *pb;

    sa = -1;
    if (a >= 0 && a < 16)
        sa = D_dfd5[a];
    sb = -1;
    if (b >= 0 && b < 16) {
        bs = b;
        sb = D_dfd5[b];
    }
    if (sb < 0 || sb >= 4) {
        if (c >= 0 && c < 16) {
            bs = c;
            sb = D_dfd5[c];
        }
    }
    if (sa < 0 || sa >= 4 || sb >= 4)
        return;
    lva = (struct level far *)(D_e037 + sa);
    ea = sub_s009_1432_3a90(t, a, 1);
    pa = &lva->exit[ea];
    x = pa->x0 / 32 + 1;
    y = pa->x2 / 16 + 1;
    lva->cell[x][y] |= 0x10;
    for (xx = x - 1; xx <= x + 1; xx++)
        for (yy = y - 1; yy <= y + 1; yy++)
            D_e033[lva->x1f40].m[xx][yy] |= 2;
    D_e033[lva->x1f40].m[x][y] |= 1;
    if (sb < 0)
        return;
    lvb = (struct level far *)(D_e037 + sb);
    pa->x6 = lvb->x1f40;
    eb = sub_s009_1432_3a90(t, bs, 1);
    pb = &lvb->exit[eb];
    pb->x6 = lva->x1f40;
    pa->x0d = eb;
    pb->x0d = ea;
    t7 = pa->x7;
    pa->x7 = pb->x7;
    pb->x7 = t7;
    ti = pa->x8;
    pa->x8 = pb->x8;
    pb->x8 = ti;
    ti = pa->x0a;
    pa->x0a = pb->x0a;
    pb->x0a = ti;
    tc = pa->x0c;
    pa->x0c = pb->x0c;
    pb->x0c = tc;
    x = pb->x0 / 32 + 1;
    y = pb->x2 / 16 + 1;
    lvb->cell[x][y] |= 0xa0;
    for (xx = x - 1; xx <= x + 1; xx++)
        for (yy = y - 1; yy <= y + 1; yy++)
            D_e033[lvb->x1f40].m[xx][yy] |= 2;
    D_e033[lvb->x1f40].m[x][y] |= 1;
}

/* open a passage from section sec through one of its wall-gap objects (type 8): carve the wall
 * cell, add it to the 0x248a/0x24c6 lists, make an exit to section b (or c, or a dead end with
 * a guard group for some sections) */
void far sub_s009_1432_2c98(int t, int sec, int b, int c, int rot)
{
    int sa, sb, bs, lvno, i, j, n, k, m, mode, x, y, xx, yy, q, e, eb, px, py, dir;
    char ok;
    long w, v;
    struct level far *lv;
    struct level far *lvb;
    struct obj far *o;
    struct obj tmp;
    struct r3382 far *r;
    struct exit far *p;
    struct exit far *pb;

    sa = -1;
    if (sec >= 0 && sec < 16)
        sa = D_dfd5[sec];
    sb = -1;
    if (b >= 0 && b < 16) {
        bs = b;
        sb = D_dfd5[b];
    }
    if (sb < 0 || sb >= 4) {
        if (c >= 0 && c < 16) {
            bs = c;
            sb = D_dfd5[c];
        }
    }
    if (sa < 0 || sa >= 4 || sb >= 4)
        return;
    lv = (struct level far *)(D_e037 + sa);
    lvno = lv->x1f40;
    for (i = 0; i < 55; i++)
        if (lv->obj[i].type == 8)
            break;
    for (j = i + 1; j < 55; j++)
        if (lv->obj[j].type != 8)
            break;
    n = j - i;
    if (n <= 0)
        return;
    k = 0;
    ok = 0;
    while (!ok && k < 15) {
        if (k >= 4)
            m = k - 4;
        else
            m = R(n) - 1;
        m = min(n - 1, m);
        m += i;
        o = &lv->obj[m];
        dir = sub_s004_1091_17e0((unsigned char)o->x0a, rot);
        if (o->type == 8 && (dir == 0 || dir == 6))
            ok = 1;
        else
            ok = 0;
        k++;
    }
    if (o->type != 8)
        return;
    mode = 1;
    if (t == 0x0d) {
        if (sec >= 7 && sec <= 8)
            mode = 2;
        else if (sec >= 11 && sec <= 12)
            mode = 3;
    }
    if (D_11dd)
        mode = 0;
    switch (mode) {
    case 0:
        w = 0;
        break;
    case 2:
        w = 6;
        break;
    case 3:
        w = 5;
        break;
    default:
        w = 8;
        break;
    }
    w &= 0xf;
    x = o->x0 / 32;
    y = o->x2 / 16;
    for (xx = x - 1; xx <= x + 1; xx++)
        for (yy = y - 1; yy <= y + 1; yy++)
            D_e033[lvno].m[xx][yy] |= 2;
    lv->hgt[x][y] &= 0xf0;
    v = D_e001 | (D_e001 << 8) | (D_dff9 << 16) | (D_dff9 << 24) | 0x10;
    switch (dir) {
    case 0:
        lv->cell[x][y] &= ~0xf0000L;
        v &= ~0xf0000L;
        if (mode) {
            lv->cell[x][y] |= w << 16;
            v |= w << 24;
        }
        lv->cell[x][y] &= ~0xf00000L;
        y--;
        break;
    case 2:
        lv->cell[x][y] &= ~0xfL;
        v &= ~0xf00L;
        if (mode) {
            lv->cell[x][y] |= w;
            v |= w << 8;
        }
        x++;
        break;
    case 4:
        lv->cell[x][y] &= ~0xf000000L;
        v &= ~0xf0000L;
        if (mode) {
            lv->cell[x][y] |= w << 24;
            v |= w << 16;
        }
        y++;
        break;
    case 6:
        lv->cell[x][y] &= ~0xf00L;
        v &= ~0xfL;
        if (mode) {
            lv->cell[x][y] |= w << 8;
            v |= w;
        }
        x--;
        break;
    }
    D_e033[lvno].m[x][y] |= 1;
    lv->cell[x][y] = v;
    lv->hgt[x][y] |= (char)D_e009 << 4;
    lv->hgt[x][y] &= 0xf0;
    if (lv->x2488 < 30) {
        q = lv->x248a[lv->x2488] / 2;
        while (q < 0x578 && lv->list[q] != -1)
            q++;
        lv->x248a[lv->x2488] = ++q * 2;
        lv->x2488++;
        lv->list[q] = (x << 8) + y;
        lv->list[++q] = -1;
    }
    tmp = lv->obj[m];
    for (q = m; q > lv->nobj; q--)
        lv->obj[q] = lv->obj[q - 1];
    lv->obj[lv->nobj] = tmp;
    o = &lv->obj[lv->nobj];
    lv->nobj++;
    for (q = 0; q < 5; q++)
        if (lv->r3382[q].b1 == 0)
            break;
    q = min(q, 4);
    r = &lv->r3382[q];
    if (mode == 0) {
        o->type = 6;
        o->x8 = 0;
        o->x0a = lv->x2488 - 1;
    } else if (mode <= 3) {
        o->type = 8;
        o->x8 = 0;
        o->x0a = q;
        o->x0c = sub_s009_182c_1cb4(t, sec);
        r->b0 = 0;
        r->b1 = x;
        r->b2 = y;
        switch (dir) {
        case 0:
            r->b2 |= 0x80;
            break;
        case 2:
            r->b2 |= 0x40;
            break;
        case 4:
            r->b1 |= 0x80;
            break;
        case 6:
            r->b1 |= 0x40;
            break;
        }
        r->b3 = lv->x2488 - 1;
    }
    if (((t == 0x0b || t == 0x82) && (sec == 3 || sec == 10)) ||
        ((t == 0x0d || t == 0x83) && (sec == 1 || sec == 7 || sec == 11))) {
        int a58, a6e, a50, a1a, a5e, a76, a56, a22, g1, g2;
        char b42, b4e, b54;

        sub_s009_182c_1260(t, sec, 10, &a58, &a6e, &a50, &a1a, &b42, &a5e, &a76, &a56, &a22,
            &b4e, &b54);
        if ((g1 = sub_s004_0e76_06aa(lvno, a58, a6e)) >= 0)
            D_db2b[lvno * 12 + g1].x0c = b42;
        if ((g2 = sub_s004_0e76_06aa(lvno, a5e, a76)) >= 0)
            D_db2b[lvno * 12 + g2].x0c = b4e;
        px = o->x0 + 16;
        py = o->x2 + 8;
        sub_s009_1432_23d6(t, g1, a50, a1a, g2, a56, a22, sec, b54, px, py,
            lv->x2488 - 1);
    }
    for (q = 5; q > lv->nexit; q--)
        lv->exit[q] = lv->exit[q - 1];
    D_dfc5[sa] = min(D_dfc5[sa] + 1, 6);
    e = lv->nexit;
    lv->nexit++;
    p = &lv->exit[e];
    p->x0 = x * 32 + 4;
    p->x2 = y * 16 + 2;
    p->x4 = 0x18;
    p->x5 = 0x0c;
    px = x * 32 + 16;
    py = y * 16 + 8;
    switch (dir) {
    case 0:
        p->x5 += 6;
        py += 0x20;
        break;
    case 2:
        p->x0 -= 12;
        p->x4 += 12;
        px -= 0x40;
        break;
    case 4:
        p->x2 -= 6;
        p->x5 += 6;
        py -= 0x20;
        break;
    case 6:
        p->x4 += 12;
        px += 0x40;
        break;
    }
    r->x6 = py;
    r->b8 = dir;
    r->x4 = px;
    xx = px / 32 + 1;
    yy = py / 16 + 1;
    for (i = xx - 1; i <= xx + 1; i++)
        for (j = yy - 1; j <= yy + 1; j++)
            D_e033[lvno].m[i][j] |= 2;
    if (sb < 0) {
        p->x6 = lv->x1f40;
        p->x0d = e;
        p->x7 = lv->x2488 - 1;
        p->x8 = r->x4;
        p->x0a = r->x6;
        p->x0c = dir;
        return;
    }
    eb = sub_s009_1432_3a90(t, bs, 1);
    lvb = (struct level far *)(D_e037 + sb);
    pb = &lvb->exit[eb];
    p->x6 = lvb->x1f40;
    pb->x6 = lv->x1f40;
    p->x0d = eb;
    pb->x0d = e;
    p->x7 = pb->x7;
    pb->x7 = lv->x2488 - 1;
    p->x8 = pb->x8;
    pb->x8 = r->x4;
    p->x0a = pb->x0a;
    pb->x0a = r->x6;
    p->x0c = pb->x0c;
    pb->x0c = dir;
    x = pb->x0 / 32 + 1;
    y = pb->x2 / 16 + 1;
    lvb->cell[x][y] |= 0xa0;
    for (xx = x - 1; xx <= x + 1; xx++)
        for (yy = y - 1; yy <= y + 1; yy++)
            D_e033[lvb->x1f40].m[xx][yy] |= 2;
    D_e033[lvb->x1f40].m[x][y] |= 1;
}

/* party entrance: an exit of section sec (or b, c) becomes the start rectangle and position */
void far sub_s009_1432_37b4(int t, int sec, int b, int c)
{
    int s, u, e, x, y, x0, x1, y0, y1;
    struct level far *lv;
    struct exit far *p;
    int far *q;

    u = -1;
    s = -1;
    if (sec >= 0 && sec < 16) {
        u = sec;
        s = D_dfd5[sec];
    }
    if (s < 0 || s >= 4) {
        if (b >= 0 && b < 16) {
            u = b;
            s = D_dfd5[b];
        }
    }
    if (s < 0 || s >= 4) {
        if (c >= 0 && c < 16) {
            u = c;
            s = D_dfd5[c];
        }
    }
    if (u < 0 || s < 0 || s >= 4)
        return;
    lv = (struct level far *)(D_e037 + s);
    e = sub_s009_1432_3a90(t, u, 0);
    p = &lv->exit[e];
    q = lv->r3328[0];
    q[0] = p->x0;
    q[1] = p->x2;
    q[2] = p->x4;
    q[3] = p->x5;
    lv->n3326 = 1;
    x = p->x0 / 32 + 1;
    y = p->x2 / 16 + 1;
    lv->cell[x][y] |= 0xa0;
    lv->x1f4c = p->x8;
    lv->x1f4e = p->x0a;
    lv->x1f51 = p->x7;
    lv->x1f50 = p->x0c ^ 4;
    D_e033[lv->x1f40].m[x][y] |= 1;
    x0 = q[0] / 32;
    x1 = x0 + q[2] / 32 - 1;
    y0 = q[1] / 16;
    y1 = y0 + q[3] / 16 - 1;
    for (x = x0; x <= x1; x++)
        for (y = y0; y <= y1; y++)
            D_e033[lv->x1f40].m[x][y] |= 2;
    y0 = lv->x1f4e / 16;
    x0 = lv->x1f4c / 32;
    for (x = x0 - 1; x <= x0 + 1; x++)
        for (y = y0 - 1; y <= y0 + 1; y++)
            D_e033[lv->x1f40].m[x][y] |= 2;
}

/* pick one of the section's original exits (the fixed one for some mines) and move it to the
 * end of the used exits (add) or of the unused ones; returns its index */
int far sub_s009_1432_3a90(int t, int sec, char add)
{
    int s, cnt, k, n, x;
    struct level far *lv;
    int *p;
    struct exit tmp;

    s = -1;
    if (sec >= 0 && sec < 16)
        s = D_dfd5[sec];
    if (s < 0 || s >= 4)
        return 0;
    lv = (struct level far *)(D_e037 + s);
    p = &D_dfc5[s];
    cnt = *p - lv->nexit;
    k = -1;
    switch (t) {
    case 0x0b:
        switch (sec) {
        case 13:
            k = 1;
            break;
        }
        break;
    case 0x82:
        if (sec == 13) {
            k = 1;
            break;
        }
    case 0x0a:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
        if ((x = D_c0fe) >= 0 && cnt > x)
            k = x;
        break;
    }
    if (k < 0)
        k = R(cnt) + lv->nexit - 1;
    if (add)
        n = lv->nexit++;
    else
        n = --*p;
    if (n != k) {
        tmp = lv->exit[k];
        lv->exit[k] = lv->exit[n];
        lv->exit[n] = tmp;
    }
    return n;
}

/* random floor decoration (182c_219e) on free cells whose neighbourhood allows it (D_13d0) */
void far sub_s009_1432_3bea(int t, int sec)
{
    int s, n, lvno, r, c;
    struct level far *lv;
    unsigned char _based(D_dfc3) *m;

    s = -1;
    if (sec >= 0 && sec < 16)
        s = D_dfd5[sec];
    if (s < 0 || s >= 4)
        return;
    if ((t == 0x0b || t == 0x82) && sec == 13)
        return;
    if ((n = sub_s009_182c_20a4(t, sec)) <= 0)
        return;
    lv = (struct level far *)(D_e037 + s);
    lvno = lv->x1f40;
    for (r = 1; r < 39; r++)
        for (c = 1; c < 39; c++) {
            if (lv->cell[r][c] & 0x80000000L)
                continue;
            if (D_e033[lvno].m[r][c] & 3)
                continue;
            if (lv->cell[r][c] & 0xf0)
                continue;
            if (R(n) != 1)
                continue;
            {
                int m1, m2;

                m1 = 0;
                m = &D_e033[lvno].m[r][c];
                if (m[1] & 0x81)
                    m1 = 1;
                if (m[-0x27] & 0xc1)
                    m1 |= 2;
                if (m[-0x28] & 0x41)
                    m1 |= 4;
                if (m[-0x29] & 0x61)
                    m1 |= 8;
                m2 = 0;
                if (m[-1] & 0x21)
                    m2 = 1;
                if (m[0x27] & 0x31)
                    m2 |= 2;
                if (m[0x28] & 0x11)
                    m2 |= 4;
                if (m[0x29] & 0x91)
                    m2 |= 8;
                if (D_13d0[m1] & (1 << m2))
                    continue;
            }
            {
                long w;

                w = sub_s009_182c_219e(t, sec);
                if (w == 0)
                    continue;
                lv->cell[r][c] |= (w & 0xf) << 4;
                lv->cell[r][c] |= (long)((R(4) - 1) & 3) << 28;
                D_e033[lvno].m[r][c] |= 1;
                lv->hgt[r][c] &= 0xf0;
            }
        }
}

/* finish section sec: decoration, clear the groups when the party has been there (D_da34),
 * save as level%d.flr and free the slot */
void far sub_s009_1432_3e1c(int t, int sec)
{
    int s, off, lvno, i;
    char name[0x42];
    struct level far *lv;

    s = -1;
    if (sec >= 0 && sec < 16)
        s = D_dfd5[sec];
    if (s < 0 || s >= 4)
        return;
    off = s * 0x33fc;
    lv = (struct level far *)(D_e037 + s);
    lvno = lv->x1f40;
    sub_s009_1432_3bea(t, sec);
    if ((D_9080 & 1) && (D_da34 & (1 << lvno)) && !(D_e036 & (1 << s))) {
        D_db05[lvno] = 0;
        for (i = 0; i < lv->n300c; i++) {
            lv->grp[i].n = 0;
            lv->grp[i].x10 = 0;
        }
    }
    sprintf(name, "level%d.flr", lv->x1f40);
    sub_s004_1262_0d0a(name, (struct level far *)((char _based(D_db37) *)D_e037 + off), 0x33fc,
        0);
    D_dfc5[s] = 0;
    D_dfcd[s] = -1;
    D_dfd5[sec] = -1;
    D_e036 &= ~(1 << s);
}

/* small mines 0x66..0x69 */
char far sub_s009_1432_3f54(int t)
{
    switch (t) {
    default:
        return 0;
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
        return 1;
    }
}

/* mines with a fixed layout (0x0a, 0x82..0x86) */
char far sub_s009_1432_3f6c(int t)
{
    switch (t) {
    default:
        return 0;
    case 0x0a:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
        return 1;
    }
}

/* 0x87..0x89: mines 0x0b/0x0d/0x10 with special sections */
char far sub_s009_1432_3f8a(int t)
{
    switch (t) {
    default:
        return 0;
    case 0x87:
    case 0x88:
    case 0x89:
        return 1;
    }
}
