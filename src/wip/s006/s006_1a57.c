/* place: s006 1a57:000c */
/* orig: size=067e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from Ghidra (dec16 drafts were stubs).
 * 000c adds a room (kind 2 keep, 3 tower, 4 gate house, 5 building, 6 gate path) to the
 * current level, 02a8 recomputes the wall bits of a cell, 042e cuts a room out of a free
 * strip (splitting it in two when needed).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gslot { int a[4]; int b; };

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;
extern struct gslot _based(gen_work) *gen_strips;
extern int _based(gen_work) *D_db89;
extern unsigned char _based(gen_work) *gen_cell_walls;
extern int _based(gen_work) *gen_cell_type;
extern int _based(gen_work) *D_db91;
extern int keep_x0, D_db3f, D_db41, D_db43;
extern int gen_rooms[5];
extern int D_db67[5];
extern int gen_level;

void far sub_s005_146d_0066(int, int);
int far sub_s005_146d_01d4(int, int, int *, int *, int *, int);
void far sub_s005_146d_07f4(int, int);
void far sub_s005_146d_08be(int, int);
int far sub_s005_146d_0a3a(int, int);
void far sub_s005_146d_0ae8(int, int);
void far sub_s006_1a57_02a8(int, int, int);

void far sub_s006_1a57_000c(int kind, int x0, int x1, int y0, int y1, int dir)
{
    int n, i, x, y, w, h, j, k;
    int far *r;
    int far *q;
    int far *t;

    if (x0 < 0 || x1 >= 40 || y0 < 0 || y1 >= 40 || x1 < x0 || y1 < y0 ||
        gen_rooms[gen_level] + 1 >= 30)
        return;
    n = gen_rooms[gen_level]++;
    i = gen_level * 30 + n;
    r = (int far *)(D_db3b + i * 4);
    r[0] = x0;
    r[1] = x1;
    r[2] = y0;
    r[3] = y1;
    if (kind != 6) {
        keep_x0 = keep_x0 > x0 ? x0 : keep_x0;
        D_db3f = D_db3f < x1 ? x1 : D_db3f;
        D_db41 = D_db41 > y0 ? y0 : D_db41;
        D_db43 = D_db43 < y1 ? y1 : D_db43;
    }
    if (kind == 2 || kind == 3 || kind == 5 || kind == 4) {
        q = (int far *)(D_db89 + i);
        *q = 7;
        w = x1 - x0 + 1;
        h = y1 - y0 + 1;
        if (((w < 3 && h < 3) || w == 1 || h == 1) && (kind != 4 || (dir != 0 && dir != 3)))
            *q = 0;
    }
    t = (int far *)(gen_cell_type + gen_level * 1600);
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            if (t[x * 40 + y] == 0)
                t[x * 40 + y] = kind;
    D_db91[i] = kind;
    for (y = y0; y <= y1; y++) {
        sub_s006_1a57_02a8(gen_level, x0, y);
        if (x0 < x1)
            sub_s006_1a57_02a8(gen_level, x1, y);
    }
    for (x = x0 + 1; x < x1; x++) {
        sub_s006_1a57_02a8(gen_level, x, y0);
        if (y0 < y1)
            sub_s006_1a57_02a8(gen_level, x, y1);
    }
    sub_s005_146d_07f4(n, n);
    sub_s005_146d_0066(n, n);
    if (kind == 6)
        sub_s005_146d_08be(30, n);
    for (j = 0; j < n; j++) {
        k = sub_s005_146d_01d4(j, n, &dir, &x0, &x1, 2);
        if (k)
            sub_s005_146d_07f4(j, n);
        if (k > 1)
            sub_s005_146d_0066(j, n);
        if (sub_s005_146d_0a3a(j, n))
            sub_s005_146d_0ae8(j, n);
    }
}

/* wall bits of cell (x, y): a side is walled towards solid cells and towards rooms of
 * a different kind (7 and 3 are separate, 5 and 6 connect to anything) */
void far sub_s006_1a57_02a8(int l, int x, int y)
{
    int j, v, w;
    struct {
        int X, i;
        char pad1;
        unsigned char out;
        int s;
        char pad2[3];
        unsigned char bit;
        int dy, dx;
        char pad3;
        unsigned char nb;
        unsigned char far *p;
        char pad4[2];
    } u;

    if (x < 1 || x >= 39 || y < 1 || y >= 39)
        return;
    v = gen_cell_type[u.i = (u.X = l * 40 + x) * 40 + y];
    u.out = 0;
    for (u.s = 0; u.s <= 3; u.s++) {
        switch (u.s) {
        case 0:
            u.dx = 0;
            u.dy = -1;
            u.bit = 1;
            u.nb = 4;
            break;
        case 1:
            u.dx = 1;
            u.bit = 2;
            u.dy = 0;
            u.nb = 8;
            break;
        case 2:
            u.dx = 0;
            u.bit = 4;
            u.dy = 1;
            u.nb = 1;
            break;
        case 3:
            u.dx = -1;
            u.bit = 8;
            u.dy = 0;
            u.nb = 2;
            break;
        default:
            continue;
        }
        j = (u.X + u.dx) * 40 + u.dy + y;
        w = gen_cell_type[j];
        u.p = (unsigned char far *)(gen_cell_walls + j);
        if (w > 1 && w < 9) {
            if ((!((v == 7) ^ (w == 7)) || w == 5 || v == 5) &&
                (!((v == 3) ^ (w == 3)) || w == 6 || v == 6))
                *u.p &= ~u.nb;
            else {
                u.out |= u.bit;
                *u.p |= u.nb;
            }
        } else
            u.out |= u.bit;
    }
    gen_cell_walls[u.i] = u.out;
}

/* remove room r from free strip `slot` (-1: the strip that overlaps it) */
void far sub_s006_1a57_042e(int slot, int r)
{
    int n, lo, hi, a, b, k, v, found;
    int far *o;
    struct gslot far *s;
    struct gslot far *t;

    n = D_db67[gen_level];
    if (slot < -1 || slot >= n || r < 0 || r >= gen_rooms[gen_level])
        return;
    o = D_db3b + (gen_level * 30 + r) * 4;
    if (slot == -1) {
        found = 0;
        for (slot = 0, s = gen_strips + gen_level * 16; slot < D_db67[gen_level]; slot++, s++) {
            if (o[1] < s->a[0] || s->a[1] < o[0] || o[3] < s->a[2] || s->a[3] < o[2])
                found = 0;
            else
                found = 1;
            if (found)
                break;
        }
        if (!found)
            return;
    }
    s = gen_strips + gen_level * 16 + slot;
    if (s->b == 0 || s->b == 2) {
        lo = 0;
        hi = 1;
        a = 2;
        b = 3;
    } else {
        lo = 2;
        hi = 3;
        a = 0;
        b = 1;
    }
    k = b;
    if (s->a[b] < D_db3b[gen_level * 120 + a]) {
        k = a;
        v = o[a];
        if (v > s->a[a])
            v = s->a[k];
    } else {
        v = o[b];
        if (s->a[b] > v)
            v = s->a[k];
    }
    s->a[k] = v;
    if (o[lo] <= s->a[lo]) {
        v = o[hi] + 1;
        if (v < s->a[lo])
            v = s->a[lo];
        s->a[lo] = v;
        return;
    }
    if (o[hi] < s->a[hi]) {
        t = gen_strips + gen_level * 16 + n;
        *t = *s;
        t->a[lo] = o[hi] + 1;
        s->a[hi] = o[lo] - 1;
        D_db67[gen_level]++;
        return;
    }
    v = o[lo] - 1;
    if (s->a[hi] < v)
        v = s->a[hi];
    s->a[hi] = v;
}
