/* place: s014 1462:000a */
/* orig: size=094e CONST=7d4e+0002 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: CONST=7d4e */
/* s014 1462: movement collision test for one combatant at its current position.
   000a(c) loads the combatant's bounding box (half sizes D_c80f/D_c831 by sprite index +0x1c)
   and the 3x3 map cells around it, then
     - walls: 01f0 (individual movement, with corner slack) or 050c (D_e0a2 == 1: group /
       formation movement, plain box test) set D_e138 = 0x4000,
     - furniture: 083e sets D_e136 = 0x2000,
     - other combatants on the same floor: 0740 sets D_e11c[k] = D_01e4[k] (bit of k),
   and returns the OR of all 16 words (also kept in D_d708), or -1 when the centre cell is
   impassable (bit 31). D_e13c != 0 skips walls/furniture, D_c600 != 0 skips combatants.
   Overlap tests deliberately ignore boxes that only touch edge-to-edge.

   Map cell (long, column-major, 40 cells per column, cell = 32 x 16 pixels):
     bits 0-3 right wall, 4-7 furniture id, 8-11 left wall, 16-19 top wall, 24-27 bottom wall,
     28-29 furniture orientation, 31 impassable. Wall type 1..11 blocks.
   status: 5/5 written, 0 match. 01f0 050c 0740 083e are MSC near functions.
   m4: 000a shift temps (cx/cy) - left: k copied to BX/SI, OR order of the result; 0740 box locals in
   struct {h, w, r, x, t, y} - left: w loaded before abs(x - D_84f4); 083e under pragma e off. */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[0xc];
    char status;                /* +14 1 = active */
    char pad15[7];
    char sprite;                /* +1c sprite / size index */
    char pad1d[0x63];
};
struct flr { long c[1]; };

void far word_fill(int *, int, int);   /* word fill (ptr, value, nwords) */
void near sub_s014_1462_01f0(void);
void near sub_s014_1462_050c(void);
void near sub_s014_1462_0740(int);
void near sub_s014_1462_083e(void);

extern struct flr far floor_img;      /* current floor map */
extern struct cbt combatants[];
extern char cbt_floor[];                   /* [13] floor of each combatant */
extern char cbt_half_w[];                   /* [13] half width */
extern char cbt_half_h[];                   /* [13] half height */
extern char cbt_moving[];                   /* [13] */
extern char D_d697;                     /* */
extern char D_d6cd;                     /* */
extern int D_01e4[];                    /* [13] 1 << k */
extern char D_c11d[];                   /* furniture height */
extern char D_c12f[];                   /* furniture width */
extern char D_d7be[][4];                /* furniture box x offset per orientation */
extern char D_d802[][4];                /* furniture box y offset per orientation */
extern char D_1a40[];                   /* [9] x offset of neighbour cell: -32, 0, 32 */
extern char D_1a4a[];                   /* [9] y offset of neighbour cell: -16, 0, 16 */
extern char D_e13c;                     /* skip map tests */
extern char D_e0a2;                     /* 1 = formation movement */
extern char D_c600;                     /* skip combatant tests */
extern int D_d708;                      /* last result */
extern int D_e11c[16];                  /* [0..12] blocking combatants, [13] furniture, [14] walls */

extern int D_84f4, D_84f6;              /* centre x, y */
extern int D_84f2, D_8534;              /* half width, half height */
extern int D_8530, D_8532;              /* half width + 4 */
extern int D_84e8;                      /* half height + 8 */
extern int D_84ea;                      /* half width + 16 */
extern int D_84ec;                      /* half height + 2 */
extern int D_8524, D_8536;              /* right edge, left edge (inclusive) */
extern int D_8528, D_8538;              /* bottom edge, top edge */
extern int D_852c, D_8522;              /* left / top pixel of the centre cell */
extern int D_8520, D_8526;              /* cell left + 16 */
extern int D_852a;                      /* cell top + 15 */
extern int D_851c;                      /* cell left + 32 */
extern int D_851e, D_852e;              /* cell top + 8 */
extern int D_84ee, D_853a, D_84f0, D_853c;  /* furniture box */
extern long D_84f8[3][3];               /* neighbour cells [dy+1][dx+1] */

#pragma optimize("a", on)
int far cbt_collisions(struct cbt *c)
{
    int k;
    int x, y;

    word_fill(D_e11c, 0, 16);
    D_84f4 = x = c->x;
    D_84f6 = y = c->y;
    k = c->sprite;
    D_84f2 = cbt_half_w[k];
    D_8534 = cbt_half_h[k];
    D_8530 = D_84f2 + 4;
    D_8532 = D_84f2 + 4;
    D_84e8 = D_8534 + 8;
    D_84ea = D_84f2 + 16;
    D_84ec = D_8534 + 2;
    D_8524 = x + D_84f2 - 1;
    D_8536 = x - D_84f2 + 1;
    D_8528 = y + D_8534 - 1;
    D_8538 = y - D_8534 + 1;
    {
        int cx = x >> 5, cy = y >> 4, i;
        D_852c = cx << 5;
        D_8522 = cy << 4;
        i = cx * 40 + cy;
        D_84f8[0][0] = floor_img.c[i - 41];
        D_84f8[0][1] = floor_img.c[i - 1];
        D_84f8[0][2] = floor_img.c[i + 39];
        D_84f8[1][0] = floor_img.c[i - 40];
        D_84f8[1][1] = floor_img.c[i];
        D_84f8[1][2] = floor_img.c[i + 40];
        D_84f8[2][0] = floor_img.c[i - 39];
        D_84f8[2][1] = floor_img.c[i + 1];
        D_84f8[2][2] = floor_img.c[i + 41];
    }
    if (D_84f8[1][1] & 0x80000000L)
        return -1;
    D_8520 = D_852c + 16;
    D_8526 = D_852c + 16;
    D_852a = D_8522 + 15;
    D_851c = D_852c + 32;
    D_851e = D_8522 + 8;
    D_852e = D_8522 + 8;
    if (D_e13c == 0) {
        if (D_e0a2 == 1)
            sub_s014_1462_050c();
        else
            sub_s014_1462_01f0();
        sub_s014_1462_083e();
    }
    if (D_c600 == 0)
        sub_s014_1462_0740(k);
    return D_d708 = D_e11c[1] | D_e11c[10] | D_e11c[2] | D_e11c[11] | D_e11c[3] | D_e11c[12]
        | D_e11c[4] | D_e11c[13] | D_e11c[5] | D_e11c[14] | D_e11c[6] | D_e11c[15]
        | D_e11c[8] | D_e11c[9] | D_e11c[7] | D_e11c[0];
}

#pragma optimize("", on)

/* wall nibbles of a neighbour cell */
#define WR(n) ((int)(n) & 0xf)                  /* right wall */
#define WL(n) ((int)((n) >> 8) & 0xf)           /* left wall */
#define WT(n) ((int)((n) >> 16) & 0xf)          /* top wall */
#define WB(n) ((int)((n) >> 24) & 0xf)          /* bottom wall */

/* walls, individual movement */
void near sub_s014_1462_01f0(void)
{
    int t;

    if (((t = WT(D_84f8[1][1])) != 0 && t <= 11 && D_84f6 - D_8522 < D_84ec)
     || ((t = WB(D_84f8[1][1])) != 0 && t <= 11 && D_852a - D_84f6 < D_84ec)
     || ((t = WR(D_84f8[1][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && (D_851c - 5 < D_8524 || D_851e - D_8528 != 9))
     || ((t = WL(D_84f8[1][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532)
     || ((t = WR(D_84f8[0][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_84f6 - D_851e + 16 < D_84e8
         && (D_851c - 3 < D_8524 || D_851e - D_8538 != 8)
         && (D_851c - 5 < D_8524 || D_851e - D_8538 != 9))
     || ((t = WR(D_84f8[2][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_851e - D_84f6 + 16 < D_84e8
         && (D_851c - 3 < D_8524 || D_851e - D_8528 != -8))
     || ((t = WT(D_84f8[1][2])) != 0 && t <= 11 && D_8520 - D_84f4 + 32 < D_84ea
         && D_84f6 - D_8522 < D_84ec
         && (D_851c + 1 < D_8524 || D_851e - 6 > D_8538))
     || ((t = WB(D_84f8[1][2])) != 0 && t <= 11 && D_8526 - D_84f4 + 32 < D_84ea
         && D_852a - D_84f6 < D_84ec
         && (D_851c + 1 < D_8524 || D_851e + 6 < D_8528))
     || ((t = WL(D_84f8[0][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_84f6 - D_852e + 16 < D_84e8
         && (D_852c + 3 > D_8536 || D_852e - D_8538 != 8))
     || ((t = WL(D_84f8[2][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_852e - D_84f6 + 16 < D_84e8
         && (D_852c + 3 > D_8536 || D_852e - D_8528 != -8))
     || ((t = WB(D_84f8[1][0])) != 0 && t <= 11 && D_84f4 - D_8526 + 32 < D_84ea
         && D_852a - D_84f6 < D_84ec
         && (D_852c - 1 > D_8536 || D_852e + 6 < D_8528))
     || ((t = WT(D_84f8[1][0])) != 0 && t <= 11 && D_84f4 - D_8520 + 32 < D_84ea
         && D_84f6 - D_8522 < D_84ec
         && (D_852c - 1 > D_8536 || D_852e - 6 > D_8538)))
        D_e11c[14] = 0x4000;
}

/* walls, formation movement */
void near sub_s014_1462_050c(void)
{
    int t;

    if (((t = WT(D_84f8[1][1])) != 0 && t <= 11 && D_84f6 - D_8522 < D_84ec)
     || ((t = WB(D_84f8[1][1])) != 0 && t <= 11 && D_852a - D_84f6 < D_84ec)
     || ((t = WR(D_84f8[1][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530)
     || ((t = WL(D_84f8[1][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532)
     || ((t = WR(D_84f8[0][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_84f6 - D_8522 < D_84ec)
     || ((t = WR(D_84f8[2][1])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_852a - D_84f6 < D_84ec)
     || ((t = WT(D_84f8[1][2])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_84f6 - D_8522 < D_84ec)
     || ((t = WB(D_84f8[1][2])) != 0 && t <= 11 && D_851c - D_84f4 < D_8530
         && D_852a - D_84f6 < D_84ec)
     || ((t = WL(D_84f8[0][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_84f6 - D_8522 < D_84ec)
     || ((t = WL(D_84f8[2][1])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_852a - D_84f6 < D_84ec)
     || ((t = WB(D_84f8[1][0])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_852a - D_84f6 < D_84ec
         && (D_852c - 1 > D_8536 || D_852e + 6 < D_8528))
     || ((t = WT(D_84f8[1][0])) != 0 && t <= 11 && D_84f4 - D_852c < D_8532
         && D_84f6 - D_8522 < D_84ec
         && (D_852c - 1 > D_8536 || D_852e - 6 > D_8538)))
        D_e11c[14] = 0x4000;
}

/* other combatants on the same floor overlapping the box */
void near sub_s014_1462_0740(int self)
{
    int i;
    int b;
    struct { int h, w, r, x, t, y; } s;

    for (i = 0; i < 13; i++) {
        if (self != i && combatants[i].status == 1 && cbt_floor[self] == cbt_floor[i]
         && (D_d697 == 0 || D_d6cd == i || (D_d6cd != i && cbt_moving[i] == 0))) {
            s.x = combatants[i].x;
            s.y = combatants[i].y;
            if (abs(s.x - D_84f4) < (s.w = cbt_half_w[i]) + D_84f2
             && abs(s.y - D_84f6) < (s.h = cbt_half_h[i]) + D_8534) {
                s.r = s.w + s.x;
                b = s.h + s.y;
                s.t = s.y - s.h;
                if ((s.x - s.w != D_8524 || (D_8528 != s.t && D_8538 != b))
                 && (D_8536 != s.r || (D_8528 != s.t && b != D_8538)))
                    D_e11c[i] = D_01e4[i];
            }
        }
    }
}

/* furniture in the 3x3 cells overlapping the box */
#pragma optimize("e", off)
void near sub_s014_1462_083e(void)
{
    int i;
    long *p;
    int f, hh, fx, fy, hw, o;

    for (i = 0, p = &D_84f8[0][0]; p < &D_84f8[3][0]; i++, p++) {
        f = (int)(*p >> 4) & 0xf;
        if (f != 0) {
            hh = D_c11d[f] >> 1;
            o = (((char *)p)[3] >> 4) & 3;
            fx = D_1a40[i] + D_d7be[f][o] + D_852c;
            fy = D_1a4a[i] + D_d802[f][o] + D_8522;
            hw = D_c12f[f] >> 1;
            if (abs(fx - D_84f4) < hw + D_84f2 && abs(fy - D_84f6) < D_8534 + hh) {
                D_853a = fx + hw;
                D_84ee = fx - hw;
                D_853c = fy + hh;
                D_84f0 = fy - hh;
                if ((D_84ee != D_8524 || (D_84f0 != D_8528 && D_853c != D_8538))
                 && (D_853a != D_8536 || (D_84f0 != D_8528 && D_853c != D_8538)))
                    D_e11c[13] = 0x2000;
            }
        }
    }
}
#pragma optimize("e", on)
