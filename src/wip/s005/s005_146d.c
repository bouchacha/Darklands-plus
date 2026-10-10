/* place: s005 146d:000c */
/* orig: size=0e3a */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 15 functions written from the asm + Ghidra (dec16 drafts
 * were stubs). Room graph of the generator: conn[lvl][a][b] bits (1 = linked, 2 = ?,
 * 4 = reachable (transitive, room 30 = outside), 8 = door marker, high nibble = direction
 * code from 0c02), rooms {x0, x1, y0, y1}. See s005_1432.c for the work segment.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };

extern _segment gen_work;
extern int (_based(gen_work) *D_db3b)[4];                 /* room rects, [lvl * 30 + i] */
extern unsigned char (_based(gen_work) *D_db8b)[31][31];  /* conn[lvl][a][b] */
extern int (_based(gen_work) *gen_cell_type)[40][40];            /* cell types [lvl][x][y] */
extern int gen_rooms[5];
extern int gen_level_hi;
extern int gen_level_lo;
extern int gen_level;

int far sub_s005_1551_0124(int, int, int, int, int);
int far sub_s005_1551_02a2(int, int, int, int);
int far sub_s005_146d_000c(int, int, int);
void far sub_s005_146d_08be(int, int);
int far sub_s005_146d_0256(int, int, int, int, int, int, int, int, int *, int *, int *, int);
void far sub_s005_146d_0c02(int, int, int, int *, int *);
int far sub_s005_146d_0d7a(int, int, int, int);

#pragma optimize("eglc", off)
int far sub_s005_146d_000c(register int l, int a, int b)
{
    int n;
    register int aa;

    if (l <= gen_level_hi && l >= gen_level_lo) {
        aa = a;
        n = gen_rooms[l];
        if (aa < n && aa >= 0 && b < *(volatile int *)&n && b >= 0)
            return D_db8b[l][aa][b] & 1;
    }
    return 0;
}
#pragma optimize("eglc", on)

/* link rooms a and b and everything linked to them */
#pragma optimize("a", on)
void far sub_s005_146d_0066(int a, int b)
{
    int i, j;
    unsigned char (far *m)[31];

    if (gen_rooms[gen_level] > a && a >= 0 && gen_rooms[gen_level] > b && b >= 0) {
        m = D_db8b[gen_level];
        if (b == a) {
            m[a][b] |= 5;
            return;
        }
        m[a][b] |= 1;
        m[b][a] |= 1;
        for (i = 0; i <= 30; i++) {
            if (gen_rooms[gen_level] > i && sub_s005_146d_000c(gen_level, i, a)) {
                for (j = 0; j <= 30; j++) {
                    if (gen_rooms[gen_level] > j && sub_s005_146d_000c(gen_level, j, b)) {
                        m[i][j] |= 1;
                        m[j][i] |= 1;
                    }
                }
            }
        }
        sub_s005_146d_08be(a, b);
    }
}
#pragma optimize("a", off)

#pragma optimize("eglc", off)
/* m6: unused dummy externs - their count shifts MSC's symbol table and fixes operand order (add/cmp) */
extern int D_ffd0;
extern int D_ffd1;
int far sub_s005_146d_017a(register int l, int a, int b)
{
    int n;
    register int aa;

    if (l <= gen_level_hi && l >= gen_level_lo) {
        aa = a;
        n = gen_rooms[l];
        if (aa < n && aa >= 0 && b < *(volatile int *)&n && b >= 0)
            return D_db8b[l][aa][b] & 2;
    }
    return 0;
}
#pragma optimize("eglc", on)

int far sub_s005_146d_01d4(int a, int b, int *dir, int *lo, int *hi, int mode)
{
    int *ra, *rb;

    if (gen_rooms[gen_level] > a && a >= 0 && gen_rooms[gen_level] > b && b >= 0) {
        ra = D_db3b[gen_level * 30 + a];
        rb = D_db3b[gen_level * 30 + b];
        return sub_s005_146d_0256(ra[0], ra[1], ra[2], ra[3], rb[0], rb[1], rb[2], rb[3],
                                  dir, lo, hi, mode);
    }
    return 0;
}

/* where do rectangles a and b touch?  *dir = side of a (0 = y-, 1 = x+, 2 = y+, 3 = x-),
 * [*lo, *hi] = shared stretch; mode 0: geometry only, 1: passable cells, 2+: also check
 * for walls (returns 2 when one is found); 3 = overlapping */
int far sub_s005_146d_0256(int ax0, int ax1, int ay0, int ay1, int bx0, int bx1, int by0,
                           int by1, int *dir, int *lo, int *hi, int mode)
{
    int i, l, h, k;

    *dir = 7;
    if (ax1 < ax0 || ay1 < ay0 || bx1 < bx0 || by1 < by0)
        return 0;
    if (by1 - ay0 == -1 && bx1 >= ax0 && bx0 <= ax1) {
        l = bx0 < ax0 ? ax0 : bx0;
        h = bx1 > ax1 ? ax1 : bx1;
        if (mode == 0 && h >= l) {
            *lo = l;
            *hi = h;
            *dir = 0;
            return 1;
        }
        k = ay0;
        while (h >= l && (sub_s005_1551_0124(gen_level, l, k, 0, 1) ||
                          gen_cell_type[gen_level][l][k] <= 1 || gen_cell_type[gen_level][l][k] >= 9))
            l++;
        while (h >= l && (sub_s005_1551_0124(gen_level, h, k, 0, 1) ||
                          gen_cell_type[gen_level][h][k] <= 1 || gen_cell_type[gen_level][h][k] >= 9))
            h--;
        if (h >= l) {
            *lo = l;
            *hi = h;
            *dir = 0;
            if (mode == 1)
                return 1;
            if (mode > 1) {
                for (i = l; i <= h; i++)
                    if (sub_s005_1551_02a2(gen_level, i, k, 0))
                        return 2;
                return 1;
            }
        }
    }
    if (bx0 - ax1 == 1 && ay0 <= by1 && by0 <= ay1) {
        l = ay0 < by0 ? by0 : ay0;
        h = ay1 > by1 ? by1 : ay1;
        if (mode == 0 && h >= l) {
            *lo = l;
            *hi = h;
            *dir = 1;
            return 1;
        }
        k = ax1;
        while (h >= l && (sub_s005_1551_0124(gen_level, k, l, 1, 1) ||
                          gen_cell_type[gen_level][k][l] <= 1 || gen_cell_type[gen_level][k][l] >= 9))
            l++;
        while (h >= l && (sub_s005_1551_0124(gen_level, k, h, 1, 1) ||
                          gen_cell_type[gen_level][k][h] <= 1 || gen_cell_type[gen_level][k][h] >= 9))
            h--;
        if (h >= l) {
            *lo = l;
            *hi = h;
            *dir = 1;
            if (mode == 1)
                return 1;
            if (mode > 1) {
                for (i = l; i <= h; i++)
                    if (sub_s005_1551_02a2(gen_level, k, i, 1))
                        return 2;
                return 1;
            }
        }
    }
    if (by0 - ay1 == 1 && ax0 <= bx1 && bx0 <= ax1) {
        l = bx0 < ax0 ? ax0 : bx0;
        h = bx1 > ax1 ? ax1 : bx1;
        if (mode == 0 && h >= l) {
            *lo = l;
            *hi = h;
            *dir = 2;
            return 1;
        }
        k = ay1;
        while (h >= l && (sub_s005_1551_0124(gen_level, l, k, 2, 1) ||
                          gen_cell_type[gen_level][l][k] <= 1 || gen_cell_type[gen_level][l][k] >= 9))
            l++;
        while (h >= l && (sub_s005_1551_0124(gen_level, h, k, 2, 1) ||
                          gen_cell_type[gen_level][h][k] <= 1 || gen_cell_type[gen_level][h][k] >= 9))
            h--;
        if (h >= l) {
            *lo = l;
            *hi = h;
            *dir = 2;
            if (mode == 1)
                return 1;
            if (mode > 1) {
                for (i = l; i <= h; i++)
                    if (sub_s005_1551_02a2(gen_level, i, k, 2))
                        return 2;
                return 1;
            }
        }
    }
    if (bx1 - ax0 == -1 && ay0 <= by1 && by0 <= ay1) {
        l = ay0 < by0 ? by0 : ay0;
        h = ay1 > by1 ? by1 : ay1;
        if (mode == 0 && h >= l) {
            *lo = l;
            *hi = h;
            *dir = 3;
            return 1;
        }
        k = ax0;
        while (h >= l && (sub_s005_1551_0124(gen_level, k, l, 3, 1) ||
                          gen_cell_type[gen_level][k][l] <= 1 || gen_cell_type[gen_level][k][l] >= 9))
            l++;
        while (h >= l && (sub_s005_1551_0124(gen_level, k, h, 3, 1) ||
                          gen_cell_type[gen_level][k][h] <= 1 || gen_cell_type[gen_level][k][h] >= 9))
            h--;
        if (h >= l) {
            *lo = l;
            *hi = h;
            *dir = 3;
            if (mode == 1)
                return 1;
            if (mode > 1) {
                for (i = l; i <= h; i++)
                    if (sub_s005_1551_02a2(gen_level, k, i, 3))
                        return 2;
                return 1;
            }
        }
    }
    if (mode != 0 && bx1 >= ax0 && bx0 <= ax1 && by1 >= ay0 && by0 <= ay1)
        return 3;
    return 0;
}

#pragma optimize("a", on)
void far sub_s005_146d_07f4(int a, int b)
{
    int n;

    *(volatile int *)&n = gen_rooms[gen_level];
    if (a < gen_rooms[gen_level] && a >= 0 && b < gen_rooms[gen_level] && b >= 0) {
        D_db8b[gen_level][a][b] |= 2;
        D_db8b[gen_level][b][a] |= 2;
        sub_s005_146d_08be(a, b);
    }
}
#pragma optimize("a", off)

#pragma optimize("eglc", off)
/* m6: unused dummy externs - their count shifts MSC's symbol table and fixes operand order (add/cmp) */
extern int D_ffd0;
extern int D_ffd1;
extern int D_ffd2;
extern int D_ffd3;
extern int D_ffd4;
extern int D_ffd5;
extern int D_ffd6;
extern int D_ffd7;
extern int D_ffd8;
int far sub_s005_146d_0858(int l, int a, int b)
{
    register int aa;
    register int ll;
    int n;

    ll = l;
    if (ll <= gen_level_hi && ll >= gen_level_lo && (aa = a) >= 0) {
        n = gen_rooms[ll];
        if ((aa < n || aa == 30) && b >= 0 && (b < *(volatile int *)&n || b == 30))
            return D_db8b[ll][aa][b] & 4;
    }
    return 0;
}
#pragma optimize("eglc", on)

/* mark a and b (room 30 = outside) as mutually reachable, transitively */
#pragma optimize("a", on)
void far sub_s005_146d_08be(int a, int b)
{
    int i, j;
    unsigned char (far *m)[31];

    if (a >= 0 && (a < gen_rooms[gen_level] || a == 30) && b >= 0 &&
        (b < gen_rooms[gen_level] || b == 30)) {
        m = D_db8b[gen_level];
        if (b == a) {
            m[a][b] |= 4;
            return;
        }
        m[a][b] |= 4;
        m[b][a] |= 4;
        for (i = 0; i <= 30; i++) {
            if ((gen_rooms[gen_level] > i || i == 30) && sub_s005_146d_0858(gen_level, i, a)) {
                for (j = 0; j <= 30; j++) {
                    if ((gen_rooms[gen_level] > j || j == 30) && sub_s005_146d_0858(gen_level, j, b)) {
                        m[i][j] |= 4;
                        m[j][i] |= 4;
                    }
                }
            }
        }
    }
}
#pragma optimize("a", off)

#pragma optimize("eglc", off)
/* m6: unused dummy externs - their count shifts MSC's symbol table and fixes operand order (add/cmp) */
extern int D_fe90;
extern int D_fe91;
int far sub_s005_146d_09e0(register int l, int a, int b)
{
    int n;
    register int aa;

    if (l <= gen_level_hi && l >= gen_level_lo) {
        aa = a;
        aa = a;
    n = gen_rooms[l];
        if (aa < n && aa >= 0 && b < *(volatile int *)&n && b >= 0)
            return D_db8b[l][aa][b] & 8;
    }
    return 0;
}
#pragma optimize("eglc", on)

/* store the direction from a to b; returns 1 when it is usable for a door */
#pragma optimize("a", on)
int far sub_s005_146d_0a3a(int a, int b)
{
    int d, g;
    int n;

    n = gen_rooms[gen_level];
    if (a < n && a >= 0 && b < *(volatile int *)&n && b >= 0) {
        sub_s005_146d_0c02(gen_level, a, b, &d, &g);
        D_db8b[gen_level][a][b] |= d << 4;
        D_db8b[gen_level][b][a] |= (d ^ 2) << 4;
        if (g <= 5 && g >= 1 && d < 8 && d >= 0 && (d <= 3 || g <= 2))
            return 1;
    }
    return 0;
}
#pragma optimize("a", off)

void far sub_s005_146d_0ae8(int a, int b)
{
    int n;

    n = gen_rooms[gen_level];
    if (a < n && a >= 0 && b < *(volatile int *)&n && b >= 0) {
        D_db8b[gen_level][a][b] |= 8;
        D_db8b[gen_level][b][a] |= 8;
    }
}

void far sub_s005_146d_0b42(int a, int b)
{
    int n;

    n = gen_rooms[gen_level];
    if (a < n && a >= 0 && b < *(volatile int *)&n && b >= 0) {
        D_db8b[gen_level][a][b] &= ~8;
        D_db8b[gen_level][b][a] &= ~8;
    }
}

/* m6: unused dummy externs - their count shifts MSC's symbol table and fixes operand order (add/cmp) */
extern int D_fea0;
extern int D_fea1;
extern int D_fea2;
int far sub_s005_146d_0b9c(int l, int a, int b)
{
    int n;

    if (l > gen_level_hi || l < gen_level_lo)
        return -1;
    n = gen_rooms[l];
    if (a < n && a >= 0 && b < *(volatile int *)&n && b >= 0)
        return ((char)D_db8b[l][a][b] >> 4) & 15;
    return 0;
}

/* direction (0..3 sides, 4..7 diagonals, 8 none) and gap from room a to room b */
void far sub_s005_146d_0c02(int l, int a, int b, int *dir, int *gap)
{
    int d0, d1, d2, d3;
    int m13, m02, t;

    if (gen_rooms[l] > a && a >= 0 && gen_rooms[l] > b && b >= 0) {
        d0 = sub_s005_146d_0d7a(l, a, b, 0);
        d1 = sub_s005_146d_0d7a(l, a, b, 1);
        d2 = sub_s005_146d_0d7a(l, a, b, 2);
        d3 = sub_s005_146d_0d7a(l, a, b, 3);
        m13 = d3 < d1 ? d1 : d3;
        m02 = d2 < d0 ? d0 : d2;
        *gap = m13 < m02 ? m02 : m13;
        *dir = 8;
        if (d0 > 0) {
            t = d0 > 2 ? 2 : d0;
            if (t > m13) {
                *dir = 0;
                return;
            }
        }
        if (d1 > 0) {
            t = d1 > 2 ? 2 : d1;
            if (t > m02) {
                *dir = 1;
                return;
            }
        }
        if (d2 > 0) {
            t = d2 > 2 ? 2 : d2;
            if (t > m13) {
                *dir = 2;
                return;
            }
        }
        if (d3 > 0) {
            t = d3 > 2 ? 2 : d3;
            if (t > m02) {
                *dir = 3;
                return;
            }
        }
        if (d0 > 0 && d3 > 0) {
            *dir = 4;
            return;
        }
        if (d0 > 0 && d1 > 0) {
            *dir = 5;
            return;
        }
        if (d2 > 0 && d1 > 0) {
            *dir = 6;
            return;
        }
        if (d2 > 0 && d3 > 0)
            *dir = 7;
    }
}

/* distance between rooms a and b in direction dir */
int far sub_s005_146d_0d7a(int l, int a, int b, int dir)
{
    int n;
    int (_based(gen_work) *r)[4];

    n = gen_rooms[l];
    if (n > a && a >= 0 && n > b && b >= 0) {
        r = D_db3b + l * 30;
        switch (dir) {
        case 0: case 4: case 5:
            return r[a][2] - r[b][3];
        case 1:
            return r[b][0] - r[a][1];
        case 2: case 6: case 7:
            return r[b][2] - r[a][3];
        case 3:
            return r[a][0] - r[b][1];
        }
    }
    return 0;
}
