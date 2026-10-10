/* place: s005 1551:0006 */
/* orig: size=05ce */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 5 functions written from the asm + Ghidra (dec16 drafts were
 * stubs). Cell queries of the generator: cell index (lvl * 40 + x) * 40 + y into
 * D_db8f (int type, 0/1 = solid, 2..8 = floor kinds), D_db8d (wall bits 1/2/4/8 for sides
 * 0..3) and D_db93 (int, wall/door kinds 3..6). See s005_1432.c for the work segment.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes, 4 per level */

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;                  /* rooms: 4 ints each, 30 per level */
extern unsigned char _based(gen_work) *gen_cell_walls;
extern int _based(gen_work) *gen_cell_type;
extern int _based(gen_work) *gen_cell_door;
extern struct gexit _based(gen_work) *gen_exits;
extern int gen_rooms[5];
extern int gen_exit_count[5];
extern int gen_level_hi;
extern int gen_level_lo;
extern int gen_level;

int far sub_s005_1551_0124(int, int, int, int, int);

/* find the exit on side dir (0 or 3) of level l that touches [lo, hi] at pos */
int far sub_s005_1551_0006(int dir, int id, int l, int lo, int hi, int pos)
{
    int i, n;
    struct gexit _based(gen_work) *e;

    n = gen_exit_count[l];
    for (i = 0, e = gen_exits + l * 4; i < n; i++, e++) {
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
    int _based(gen_work) *r;

    n = gen_rooms[l];
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
    int _based(gen_work) *p;
    int w;

    if (l > gen_level_hi || l < gen_level_lo || x < 1 || x >= 39 || y < 1 || y >= 39 || dir < 0 || dir > 3)
        return -1;
    i = (l * 40 + x) * 40 + y;
    p = gen_cell_type + i;
    if (*p == 0 || *p == 1)
        return def;
    switch (dir) {
    case 0:
        if (gen_cell_walls[i] & 1)
            return 1;
        if (gen_cell_walls[i - 1] & 4)
            return 1;
        w = p[-1];
        if (w == 0 || w == 1)
            return 1;
        return 0;
    case 1:
        if (gen_cell_walls[i] & 2)
            return 1;
        if (gen_cell_walls[i + 40] & 8)
            return 1;
        if (p[40] == 0 || p[40] == 1)
            return 1;
        return 0;
    case 2:
        if (gen_cell_walls[i] & 4)
            return 1;
        if (gen_cell_walls[i + 1] & 1)
            return 1;
        if (p[1] == 0 || p[1] == 1)
            return 1;
        return 0;
    case 3:
        if (gen_cell_walls[i] & 8)
            return 1;
        if (gen_cell_walls[i - 40] & 2)
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
    int _based(gen_work) *p;
    unsigned char _based(gen_work) *b;
    int _based(gen_work) *w;

    if (l <= gen_level_hi && l >= gen_level_lo && x > 0 && x < 39 && y > 0 && y < 39 && dir >= 0 &&
        dir < 4) {
        p = gen_cell_type + (l * 40 + x) * 40 + y;
        if (*p > 1 && *p < 9) {
            b = gen_cell_walls + l * 1600;
            w = gen_cell_door + l * 1600;
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

    if (x < 1 || x >= 39 || y < 1 || y >= 39 || dir < 0 || dir > 3 ||
        (w = gen_cell_type[(gen_level * 40 + x) * 40 + y]) == 0 || w == 1)
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
    while (!sub_s005_1551_0124(gen_level, x, y, dir, 0)) {
        n++;
        y += dy;
        x += dx;
    }
    return n;
}
