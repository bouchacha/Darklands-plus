/* place: s005 1551:0006 */
/* orig: size=05ce */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 5 functions written from the asm + Ghidra (dec16 drafts were
 * stubs). Cell queries of the generator: cell index (lvl * 40 + x) * 40 + y into
 * D_db8f (int type, 0/1 = solid, 2..8 = floor kinds), D_db8d (wall bits 1/2/4/8 for sides
 * 0..3) and D_db93 (int, wall/door kinds 3..6). See s005_1432.c for the work segment.
 */

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes, 4 per level */

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms: 4 ints each, 30 per level */
extern unsigned char _based(D_db39) *D_db8d;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db93;
extern struct gexit _based(D_db39) *D_db95;
extern int D_db45[5];
extern int D_db71[5];
extern int D_db7b;
extern int D_db7d;
extern int D_db81;

int far sub_s005_1551_0124(int, int, int, int, int);

/* find the exit on side dir (0 or 3) of level l that touches [lo, hi] at pos */
int far sub_s005_1551_0006(int dir, int id, int l, int lo, int hi, int pos)
{
    int i, n;
    struct gexit _based(D_db39) *e;

    n = D_db71[l];
    for (i = 0, e = D_db95 + l * 4; i < n; i++, e++) {
        if (dir == 0 && e->x0a == 0 && e->x2 - pos == 1 && e->x0 <= hi && e->x0 >= lo &&
            (id == 0 || e->x8 == id))
            return i;
        if (dir == 3 && e->x0a == 3 && e->x0 - pos == 1 && e->x2 <= hi && e->x2 >= lo &&
            (id == 0 || e->x8 == id))
            return i;
    }
    return -1;
}

/* room of level l overlapping the rectangle */
int far sub_s005_1551_00bc(int l, int x0, int x1, int y0, int y1)
{
    int i, n;
    int _based(D_db39) *r;

    n = D_db45[l];
    for (i = 0, r = D_db3b + l * 120; i < n; i++, r += 4) {
        if (x0 <= r[1] && r[0] <= x1 && y0 <= r[3] && r[2] <= y1)
            return i;
    }
    return -1;
}

/* is the side dir of cell (x, y) blocked?  -1 = bad arguments, def for solid cells */
int far sub_s005_1551_0124(int l, int x, int y, int dir, int def)
{
    int i;
    int _based(D_db39) *p;
    int w;

    if (l > D_db7b || l < D_db7d || x < 1 || x >= 39 || y < 1 || y >= 39 || dir < 0 || dir > 3)
        return -1;
    i = (l * 40 + x) * 40 + y;
    p = D_db8f + i;
    if (*p == 0 || *p == 1)
        return def;
    switch (dir) {
    case 0:
        if (D_db8d[i] & 1)
            return 1;
        if (D_db8d[i - 1] & 4)
            return 1;
        w = p[-1];
        if (w == 0 || w == 1)
            return 1;
        return 0;
    case 1:
        if (D_db8d[i] & 2)
            return 1;
        if (D_db8d[i + 40] & 8)
            return 1;
        if (p[40] == 0 || p[40] == 1)
            return 1;
        return 0;
    case 2:
        if (D_db8d[i] & 4)
            return 1;
        if (D_db8d[i + 1] & 1)
            return 1;
        if (p[1] == 0 || p[1] == 1)
            return 1;
        return 0;
    case 3:
        if (D_db8d[i] & 8)
            return 1;
        if (D_db8d[i - 40] & 2)
            return 1;
        w = p[-40];
        if (w == 0 || w == 1)
            return 1;
        return 0;
    }
    return 1;
}

/* can a door be put on side dir of cell (x, y)? */
int far sub_s005_1551_02a2(int l, int x, int y, int dir)
{
    int _based(D_db39) *p;
    unsigned char _based(D_db39) *b;
    int _based(D_db39) *w;

    if (l <= D_db7b && l >= D_db7d && x > 0 && x < 39 && y > 0 && y < 39 && dir >= 0 &&
        dir < 4) {
        p = D_db8f + (l * 40 + x) * 40 + y;
        if (*p > 1 && *p < 9) {
            b = D_db8d + l * 1600;
            w = D_db93 + l * 1600;
            switch (dir) {
            case 0:
                if (p[-1] > 1 && p[-1] < 9 && !(b[x * 40 + y] & 1) &&
                    !(b[x * 40 + y - 1] & 4) && w[x * 40 + y] != 3 && w[x * 40 + y - 1] != 5)
                    return 1;
                break;
            case 1:
                if (p[40] < 2 || p[40] > 8)
                    return 0;
                if (b[x * 40 + y] & 2)
                    return 0;
                if (b[x * 40 + y + 40] & 8)
                    return 0;
                if (w[x * 40 + y] == 4)
                    return 0;
                if (w[x * 40 + y + 40] == 6)
                    return 0;
                return 1;
            case 2:
                if (p[1] < 2 || p[1] > 8)
                    return 0;
                if (b[x * 40 + y] & 4)
                    return 0;
                if (b[x * 40 + y + 1] & 1)
                    return 0;
                if (w[x * 40 + y] == 5)
                    return 0;
                if (w[x * 40 + y + 1] == 3)
                    return 0;
                return 1;
            case 3:
                if (p[-40] < 2 || p[-40] > 8)
                    return 0;
                if (b[x * 40 + y] & 8)
                    return 0;
                if (b[x * 40 + y - 40] & 2)
                    return 0;
                if (w[x * 40 + y] == 6)
                    return 0;
                if (w[x * 40 + y - 40] == 4)
                    return 0;
                return 1;
            }
        }
    }
    return 0;
}

/* number of open cells from (x, y) in direction dir */
int far sub_s005_1551_04da(int x, int y, int dir)
{
    int dx, dy, n;
    int w;

    if (x < 1 || x > 38 || y < 1 || y > 38 || dir < 0 || dir > 3 ||
        (w = D_db8f[(D_db81 * 40 + x) * 40 + y]) == 0 || w == 1)
        return -1;
    switch (dir) {
    case 0:
        dx = 0;
        dy = -1;
        break;
    case 1:
        dx = 1;
        dy = 0;
        break;
    case 2:
        dx = 0;
        dy = 1;
        break;
    case 3:
        dx = -1;
        dy = 0;
        break;
    default:
        return 0;
    }
    n = 0;
    while (!sub_s005_1551_0124(D_db81, x, y, dir, 0)) {
        n++;
        y += dy;
        x += dx;
    }
    return n;
}
