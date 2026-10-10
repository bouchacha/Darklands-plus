/* place: s012 1432:0000 */
/* orig: size=0a7c CONST=7d34+0002 _DATA=15ee+0015 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (m0, 2026-10-09): 5/6 functions match. 028a (battlefield cell layout): code and frame
 * size right, only stack slots differ. Original top-down: r c4 c6 c8 ca cc, d (char), row's
 * home (6.00A also stores di there once per row, never reads it), first, e (char), n.
 * m0: n/e/first in one struct `lo` (aggregates go below all scalars in 6.00A) puts n, e and
 * first in the original slots; row2 was not needed (the [bp-10h] store is row's own home);
 * `int col;` must be declared before `int row;` for `mov ax,si; add ax,di`.
 * Left: scalar order - 6.00A gives d -1, row -4, r -6, c8 -8, c6 -0a, c4 -0c, cc -0e, ca -10
 * (chars first, then by use: reordering the `cc = ca = .. = r = 0` chain reorders the c's;
 * names and declaration order have no effect). The original order equals the push order of
 * the last 07e2 call (r, c4, c6, c8, ca, cc, d, row). Struct holders for the c's push r from
 * memory instead of `push ax`; a struct can't hold row (register var with a home).
 * Earlier: renames, declaration order, chain/statement orders, volatile vars (build/agent/e6/bf*.py).
 * The V() volatile reads at the call and `r` as the zero argument reproduce 6.00's
 * memory pushes and `push ax` zero pushes.
 * Dragon cave battlefield (fixed battle map "cavedrag").
 *
 * F_20a5_0000: the battlefield map, 0x33fc bytes, loaded from/saved to level0.flr
 *   0x0000 unsigned long cell[40][40]  packed cell: bit31 = empty (init 0x80000000);
 *                                       bits 0-3, 4-7, 8-11, 16-19, 24-27 tile parts,
 *                                       bits 28-29 random variant
 *   0x1900 unsigned char hgt[40][40]   hi nibble / lo nibble (floor type / decoration)
 *   0x24c6 int list[]                  cells used, (row << 8) + col, -1 terminated
 * D_db33 (based on segment D_dadd): battlefield description record (tile set names
 *   at +1, +1a, +34, +67, +9a, tile indices behind them).
 * D_db2d (based on D_dadd): enemy groups, 0x1e bytes each, D_db05 of them.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };

struct bfmap {
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40 */
    char x1f41;
    int x1f42[5];                       /* 1f42 */
    int x1f4c;                          /* 1f4c */
    int x1f4e;                          /* 1f4e */
    char x1f50;                         /* 1f50 */
    char x1f51;                         /* 1f51 */
    char x1f52;                         /* 1f52 */
    char p1f53[0x2184 - 0x1f53];
    char x2184;                         /* 2184 */
    char p2185[0x2488 - 0x2185];
    char x2488;                         /* 2488 */
    char x2489;
    int x248a;                          /* 248a */
    char p248c[0x24c6 - 0x248c];
    int list[(0x2fb6 - 0x24c6) / 2];    /* 24c6 */
    char x2fb6;                         /* 2fb6 */
    char p2fb7[0x300c - 0x2fb7];
    char x300c;                         /* 300c */
    char p300d[0x3326 - 0x300d];
    char x3326;                         /* 3326 */
    char x3327;
    int x3328;                          /* 3328 */
    int x332a;                          /* 332a */
    int x332c;                          /* 332c */
    int x332e;                          /* 332e */
    char p3330[0x33c8 - 0x3330];
    char name[0x33fc - 0x33c8];         /* 33c8 */
};

struct bfdesc {
    char x00;
    char set0[10];                      /* 01 */
    char t0b[9];                        /* 0b */
    char p14[6];
    char set1[9];                       /* 1a */
    char x23;
    char x24;
    char t25[4];                        /* 25 */
    char p29[0x34 - 0x29];
    char set2[10];                      /* 34 */
    char t3e[2];                        /* 3e */
    char p40[0x67 - 0x40];
    char set3[10];                      /* 67 */
    char t71;                           /* 71 */
    char p72[0x9a - 0x72];
    char set4[9];                       /* 9a */
    char a3;
    char a4;
    char ta5[5];                        /* a5 */
    char aa;
    char tab[6];                        /* ab */
};

struct group {
    int pos;                            /* 00 */
    int pos2;                           /* 02 */
    int x04;                            /* 04 */
    char x06;                           /* 06 */
    char x07;                           /* 07 */
    char type[3];                       /* 08 */
    char count[3];                      /* 0b */
    char p0e[0x1e - 0x0e];
};

extern struct bfmap far floor_img;
extern _segment enemy_seg;
extern struct bfdesc _based(enemy_seg) *tile_desc;
extern struct group _based(enemy_seg) *bf_groups;
extern int group_count;

int far _fstrcpy();
int far monster_type_map();
int far roll_dice();
int far sub_s004_1262_0d0a();
int far party_lore_level();
int far sub_s012_1432_0000();
void far sub_s012_1432_0084();
void far sub_s012_1432_010a();
void far sub_s012_1432_028a();
void far sub_s012_1432_07e2(int row, int col, int *n, unsigned char hi, unsigned char lo,
    unsigned a, int p_12, unsigned b, int p_16, unsigned c, int p_1a, unsigned d, int p_1e, unsigned e, int p_22);
void far sub_s012_1432_08a8();

int far sub_s012_1432_0000(int kind, int p_8, int p_a, int p_c, int p_e, int p_10, int p_12, int p_14, int p_16, int p_18)
{
    int k;
    int t;
    if ((t = *(volatile int *)&p_8) == 0x3f || t == 0x43 || (t = p_e) == 0x3f || t == 0x43 || (t = p_14) == 0x3f)
        goto dragon;
    k = kind;
    if (t == 0x43)
dragon:
        k = 0x60;
    sub_s012_1432_0084();
    sub_s012_1432_010a(k);
    sub_s012_1432_028a(k);
    sub_s004_1262_0d0a("level0.flr", (char far *)&floor_img, sizeof(floor_img), 0);
    sub_s012_1432_08a8(k, p_8, p_a, p_c, p_e, p_10, p_12, p_14, p_16, p_18);
    return 1;
}

void far sub_s012_1432_0084(void)
{
    int i;
    int j;

    floor_img.x1f40 = 0;
    floor_img.x1f52 = 0;
    floor_img.x2184 = 0;
    floor_img.x2488 = 0;
    floor_img.x2fb6 = 0;
    floor_img.x300c = 0;
    floor_img.x3326 = 0;
    floor_img.x1f42[0] = -2;
    floor_img.x1f42[1] = -2;
    floor_img.x1f42[2] = -2;
    floor_img.x1f42[3] = -2;
    floor_img.x1f42[4] = -2;
    _fstrcpy(floor_img.name, (char far *)"");
    for (i = 0; i < 40; i++) {
        for (j = 0; j < 40; j++) {
            floor_img.cell[i][j] = 0x80000000L;
            floor_img.hgt[i][j] = 0;
        }
    }
}

#pragma optimize("a", on)
void far sub_s012_1432_010a(int kind)
{
    char name[9] = "cavedrag";

    tile_desc->x00 = 5;
    _fstrcpy((char far *)tile_desc->set0, (char far *)name);
    tile_desc->t0b[0] = 0;
    tile_desc->t0b[1] = 1;
    tile_desc->t0b[2] = 2;
    tile_desc->t0b[3] = 3;
    tile_desc->t0b[4] = 4;
    tile_desc->t0b[5] = 5;
    tile_desc->t0b[6] = 6;
    tile_desc->t0b[7] = 7;
    tile_desc->t0b[8] = 8;
    _fstrcpy((char far *)tile_desc->set1, (char far *)name);
    tile_desc->x23 = 1;
    tile_desc->t25[0] = 8;
    tile_desc->t25[1] = 9;
    tile_desc->t25[2] = 10;
    tile_desc->t25[3] = 11;
    _fstrcpy((char far *)tile_desc->set2, (char far *)name);
    tile_desc->t3e[0] = 0;
    tile_desc->t3e[1] = 1;
    _fstrcpy((char far *)tile_desc->set3, (char far *)name);
    tile_desc->t71 = 0;
    _fstrcpy((char far *)tile_desc->set4, (char far *)name);
    tile_desc->ta5[0] = 0;
    tile_desc->ta5[1] = 1;
    tile_desc->ta5[2] = 2;
    tile_desc->ta5[3] = 3;
    tile_desc->ta5[4] = 4;
    tile_desc->a3 = 5;
    tile_desc->aa = 5;
    if (kind == 0x60) {
        tile_desc->tab[0] = 10;
        tile_desc->tab[1] = 12;
        tile_desc->tab[2] = 11;
        tile_desc->tab[3] = 13;
    } else {
        tile_desc->tab[0] = 30;
        tile_desc->tab[1] = 32;
        tile_desc->tab[2] = 31;
        tile_desc->tab[3] = 33;
    }
    tile_desc->tab[4] = 20;
    tile_desc->tab[5] = 22;
}

#define V(x) (*(volatile int *)&(x))
void far sub_s012_1432_028a(int kind)
{
    int r;
    int c4;
    int c6;
    int c8;
    int ca;
    int cc;
    unsigned char d;
    int col;
    int row;
    struct { int n; char p1; unsigned char e; int first; } lo;

    *(volatile int *)&lo.n = 0;
    sub_s012_1432_07e2(3, 4, &lo.n, 7, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(3, 5, &lo.n, 5, 0, 0x0a, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(3, 6, &lo.n, 7, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(3, 7, &lo.n, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(4, 4, &lo.n, 5, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(4, 5, &lo.n, 7, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(4, 6, &lo.n, 6, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(4, 7, &lo.n, 7, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(5, 4, &lo.n, 1, 1, 0x0b, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    if (kind == 0x64)
        sub_s012_1432_07e2(5, 5, &lo.n, 1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    else
        sub_s012_1432_07e2(5, 5, &lo.n, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(5, 6, &lo.n, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(5, 7, &lo.n, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(5, 8, &lo.n, 7, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(6, 4, &lo.n, 1, 0, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(6, 5, &lo.n, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(6, 6, &lo.n, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(6, 7, &lo.n, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(6, 8, &lo.n, 7, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(7, 4, &lo.n, 2, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(7, 5, &lo.n, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(7, 6, &lo.n, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(7, 7, &lo.n, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(7, 8, &lo.n, 1, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(9, 7, &lo.n, 3, 0, 0x0c, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sub_s012_1432_07e2(0x0d, 8, &lo.n, 3, 0, 0x0c, 0, 0, 0, 1, 0, 1, 0, 0, 0);
    sub_s012_1432_07e2(0x0d, 5, &lo.n, 9, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0);
    for (row = 8; row <= 13; row++) {
        if (row < 10)
            col = 4;
        else
            col = 5;
        V(lo.first) = col;
        for (; col <= 8; col++) {
            lo.e = 0;
            r = roll_dice(1, 100);
            if ((row + col) % 2 == 0) {
                d = 3;
                if (r <= 15)
                    lo.e = 3;
                else if (r <= 30)
                    lo.e = 4;
            } else {
                int t;
                d = 4;
                if ((t = r) <= 60)
                    lo.e = 3;
                else if (t <= 75)
                    lo.e = 4;
            }
            cc = V(ca) = c8 = c6 = c4 = r = 0;
            if (lo.first == col)
                cc = 1;
            if (col == 8) {
                c8 = 1;
                c6 = 0;
            }
            if (row == 13 || (row == 9 && lo.first == col)) {
                c4 = 1;
                r = 0;
            }
            sub_s012_1432_07e2(row, col, &lo.n, d, lo.e, r, r, V(cc), V(ca), V(c8), V(c6), V(c4), V(r), r, r);
        }
    }
    floor_img.x248a = 0;
    floor_img.list[lo.n] = -1;
    floor_img.x1f4c = 0x170;
    floor_img.x1f4e = 0x68;
    floor_img.x1f50 = 6;
    floor_img.x1f51 = -1;
    floor_img.x3326 = 1;
    floor_img.x3328 = 0x1a0;
    floor_img.x332c = 0x20;
    floor_img.x332a = 0x50;
    floor_img.x332e = 0x10;
}

#pragma optimize("", on)
void far sub_s012_1432_07e2(int row, int col, int *n, unsigned char hi, unsigned char lo,
    unsigned a, int p_12, unsigned b, int p_16, unsigned c, int p_1a, unsigned d, int p_1e, unsigned e, int p_22)
{
    if (floor_img.cell[row][col] & 0x80000000L) {
        floor_img.cell[row][col] = (((((unsigned long)((roll_dice(1, 4) - 1 & 3) << 4 | (c & 15)) << 16
                | (e & 15)) << 4 | (a & 15)) << 4) | ((unsigned long)(b & 15) << 16)) | (d & 15);
        floor_img.hgt[row][col] = hi << 4 | lo;
        floor_img.list[(*n)++] = (row << 8) + col;
    }
}

#pragma optimize("a", on)
#pragma optimize("a", off)
void far sub_s012_1432_08a8(int kind, int p_8, int p_a, int n1, int p_e, int p_10, int n2, int p_14, int p_16, int n3)
{
    int t1, t2, t3;
    int a, b, c, d, e;
    int k;
    struct group far *g;

    t2 = t3 = t1 = -1;
    switch (kind) {
    case 0x60:
        t1 = monster_type_map(0, 0x3f, 1);
        n1 = 1;
        break;
    case 0x64:
        t1 = monster_type_map(0, 0x2f, 1);
        n1 = roll_dice(1, 3) + party_lore_level() / 3 + 3;
        break;
    case 0x65:
        t1 = monster_type_map(0, 0x27, 1);
        t2 = monster_type_map(0, 0x29, 2);
        if (party_lore_level() < 5)
            t3 = monster_type_map(0, 0x29, 1);
        else
            t3 = monster_type_map(0, 0x29, 3);
        n1 = roll_dice(1, 2) + 2;
        n2 = roll_dice(1, 2) + 1;
        n3 = roll_dice(1, 2);
        break;
    }
    if (kind == 0x60) {
        a = b = 5;
        c = 0;
        d = 0x18;
        e = 0x0f;
    } else {
        a = b = 6;
        d = c = 0x10;
        e = 8;
    }
    g = bf_groups + group_count;
    g->pos = (b << 5) + d;
    g->pos2 = (a << 4) + e;
    g->x06 = -1;
    g->x04 = c;
    g->x07 = 0;
    k = 0;
    if (t1 >= 0 && t1 < 12 && n1 > 0) {
        g->type[k] = t1;
        g->count[k] = n1;
        k++;
    }
    if (t2 >= 0 && t2 < 12 && n2 > 0) {
        g->type[k] = t2;
        g->count[k] = n2;
        k++;
    }
    if (t3 >= 0 && t3 < 12 && n3 > 0) {
        g->type[k] = t3;
        g->count[k] = n3;
        k++;
    }
    if (k > 0)
        group_count++;
}
