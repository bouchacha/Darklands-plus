/* place: s014 1d69:000c */
/* orig: size=04dc CONST=7db0+0002 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: CONST=7db0 */
/* s014 1d69: line of sight / line of fire.
   The segment (D_e0a5,D_e0a3)-(D_e0a9,D_e0a7) (pixel coordinates) is tested against
   axis-aligned boxes D_e1d5..D_e1d7 x D_e1d9..D_e1db with Cohen-Sutherland outcodes (04b2)
   and a clip step (0354).
   000c walks every map cell (32 x 16 pixels) in the bounding rectangle of the segment and
   tests the four wall edges of each cell and the furniture standing in it, then (unless
   a == -1) every other active combatant on the current floor, its box shrunk by the
   shooter's "height" (h/90).
   status: 3/3 written, 1 matches (04b2). */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[0xc];
    char status;                /* +14 1 = active */
    char pad15[0x44];
    char f59;                   /* +59 elevation / stance used for line of fire */
    char pad5a[0x15];
    char f6f;                   /* +6f elevation when mode 0x100 */
    char pad70[0x10];
};

struct cell {                   /* floor map cell, 4 bytes, column-major 40 per column */
    unsigned char b0;           /* low nibble: right wall type; high nibble: furniture id */
    unsigned char b1;           /* low nibble: left wall type */
    int w2;                     /* bits 0-3: top wall, bits 8-11: bottom wall (1..11 = solid) */
};
struct flr { struct cell c[1]; };

int far sub_s014_1d69_0354(void);
int far sub_s014_1d69_04b2(int, int);

extern struct cell far F_20a5_0[];      /* current floor map (level%d.flr) */
extern int D_e1d5, D_e1d7, D_e1d9, D_e1db;
extern struct flr far F_20a5_00;        /* same, via a far struct (own CONST slot) */
extern struct cbt combatants[];
extern char cbt_floor[];                   /* [13] floor of each combatant */
extern char floor_shown;                     /* floor currently shown */
extern char D_c1cf;                     /* first furniture id that blocks sight */
extern char D_d7be[][4];                /* furniture box x offset per orientation */
extern char D_d802[][4];                /* furniture box y offset per orientation */
extern char D_c12f[];                   /* furniture width */
extern char D_c11d[];                   /* furniture height */
extern char cbt_half_w[];                   /* [13] combatant half width */
extern char cbt_half_h[];                   /* [13] combatant half height */
extern int los_x0, los_y0, los_x1, los_y1;

/* 1 if the segment is blocked by a wall, furniture or (a != -1) a combatant other than a and b.
   a == -2: return 50 + index of the blocking combatant. */
int far line_blocked(int a, int b, int mode)
{
    int y1, y0, x1, x0, dy, dx, xmin, ymin, col, row, xp, yp, f, h, i, k;
    struct cbt *c;

    y1 = los_y1 / 16;
    y0 = los_y0 / 16;
    dy = abs(y0 - y1);
    x1 = los_x1 / 32;
    x0 = los_x0 / 32;
    xmin = x1 > x0 ? x0 : x1;
    ymin = y1 > y0 ? y0 : y1;
    dx = abs(x0 - x1);
    for (col = xmin; col <= dx + xmin; col++) {
        xp = col * 32 - 1;
        for (row = ymin; row <= ymin + dy; row++) {
            yp = row * 16 - 1;
            k = col * 40 + row;
            if ((F_20a5_0[k].w2 & 0xf) > 0 && (F_20a5_0[k].w2 & 0xf) < 12) {
                D_e1d5 = xp;            /* top wall */
                D_e1d7 = xp + 0x21;
                D_e1d9 = yp;
                D_e1db = yp + 2;
                if (sub_s014_1d69_0354() == 1)
                    return 1;
            }
            if ((F_20a5_0[k].w2 & 0xf00) > 0 && (F_20a5_0[k].w2 & 0xf00) < 0xc00) {
                D_e1d9 = yp + 0xf;      /* bottom wall */
                D_e1d5 = xp;
                D_e1d7 = xp + 0x21;
                D_e1db = yp + 0x11;
                if (sub_s014_1d69_0354() == 1)
                    return 1;
            }
            if ((F_20a5_0[k].b0 & 0xf) != 0 && (F_20a5_0[k].b0 & 0xf) < 12) {
                D_e1d5 = xp + 0x1f;     /* right wall */
                D_e1d7 = xp + 0x21;
                D_e1d9 = yp;
                D_e1db = yp + 0x11;
                if (sub_s014_1d69_0354() == 1)
                    return 1;
            }
            if ((F_20a5_0[k].b1 & 0xf) != 0 && (F_20a5_0[k].b1 & 0xf) < 12) {
                D_e1d5 = xp;            /* left wall */
                D_e1d7 = xp + 2;
                D_e1d9 = yp;
                D_e1db = yp + 0x11;
                if (sub_s014_1d69_0354() == 1)
                    return 1;
            }
            f = (int)(*(long far *)&F_20a5_0[k] >> 4) & 0xf;
            if (f >= D_c1cf) {
                /* furniture, box depends on its orientation (bits 4-5 of byte 3) */
                i = (((char far *)&F_20a5_00.c[k])[3] >> 4) & 3;
                D_e1d5 = D_d7be[f][i] + xp + 1;
                D_e1d9 = D_d802[f][i] + yp + 1;
                D_e1d7 = D_d7be[f][i] + D_c12f[f] + xp + 1;
                D_e1db = D_d802[f][i] + D_c11d[f] + yp + 1;
                if (sub_s014_1d69_0354() == 1)
                    return 1;
            }
        }
    }
    if (a == -1)
        return 0;
    if (mode == 0x100)
        h = combatants[a].f6f;
    else if (a == -2)
        h = 0;
    else
        h = combatants[a].f59;
    if (h >= 90)
        return 0;
    for (i = 0; i < 13; i++) {
        c = &combatants[i];
        if (c->status == 1 && cbt_floor[i] == floor_shown && a != i && b != i) {
            x0 = cbt_half_w[i] + cbt_half_w[i] * h / -90;
            y0 = cbt_half_h[i] + cbt_half_h[i] * h / -90;
            D_e1d5 = c->x - x0;
            D_e1d9 = c->y - y0;
            D_e1d7 = x0 + c->x;
            D_e1db = y0 + c->y;
            if (sub_s014_1d69_0354() == 1) {
                if (a == -2)
                    return i + 50;
                return 1;
            }
        }
    }
    return 0;
}

/* 1 if the segment intersects the box D_e1d5..D_e1db */
int far sub_s014_1d69_0354(void)
{
    int c0, c1;
    struct { int x, y; } p;

    c0 = sub_s014_1d69_04b2(los_x0, los_y0);
    c1 = sub_s014_1d69_04b2(los_x1, los_y1);
    if (c0 & c1)
        return 0;
    if (c0 == 0 || c1 == 0)
        return 1;
    if ((c1 | c0) == 12)
        return 1;
    if ((c1 | c0) == 3)
        return 1;
    if (c0 & 4) {
        p.x = (long)(los_x1 - los_x0) * (D_e1db - los_y1) / (los_y1 - los_y0) + los_x1;
        p.y = D_e1db;
    } else if (c0 & 8) {
        p.x = (long)(D_e1d9 - los_y1) * (los_x1 - los_x0) / (los_y1 - los_y0) + los_x1;
        p.y = D_e1d9;
    }
    if (sub_s014_1d69_04b2(p.x, p.y)) {
        if (c0 & 2) {
            p.y = (long)(D_e1d7 - los_x1) * (los_y1 - los_y0) / (los_x1 - los_x0) + los_y1;
            p.x = D_e1d7;
        } else if (c0 & 1) {
            p.y = (long)(los_y1 - los_y0) * (D_e1d5 - los_x1) / (los_x1 - los_x0) + los_y1;
            p.x = D_e1d5;
        }
    }
    return sub_s014_1d69_04b2(p.x, p.y) == 0;
}

/* Cohen-Sutherland outcode of (x,y) against the box */
int far sub_s014_1d69_04b2(int x, int y)
{
    int code;

    code = 0;
    if (D_e1d5 > x)
        code = 1;
    if (x > D_e1d7)
        code |= 2;
    if (y < D_e1d9)
        code |= 8;
    if (y > D_e1db)
        code |= 4;
    return code;
}
