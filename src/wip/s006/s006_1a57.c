/* place: s006 1a57:000c */
/* orig: size=067e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from Ghidra (dec16 drafts were stubs).
 * 000c adds a room (kind 2 keep, 3 tower, 4 gate house, 5 building, 6 gate path) to the
 * current level, 02a8 recomputes the wall bits of a cell, 042e cuts a room out of a free
 * strip (splitting it in two when needed).
 */

struct res { int a; int b; };
struct gslot { int a[4]; int b; };

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;
extern struct gslot _based(D_db39) *D_db65;
extern int _based(D_db39) *D_db89;
extern unsigned char _based(D_db39) *D_db8d;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern int D_db3d, D_db3f, D_db41, D_db43;
extern int D_db45[5];
extern int D_db67[5];
extern int D_db81;

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
    int _based(D_db39) *r;
    int _based(D_db39) *t;

    if (x0 < 0 || x1 >= 40 || y0 < 0 || y1 >= 40 || x0 > x1 || y0 > y1 ||
        D_db45[D_db81] + 1 >= 30)
        return;
    n = D_db45[D_db81]++;
    i = D_db81 * 30 + n;
    r = D_db3b + i * 4;
    r[0] = x0;
    r[1] = x1;
    r[2] = y0;
    r[3] = y1;
    if (kind != 6) {
        if (x0 < D_db3d)
            D_db3d = x0;
        if (D_db3f < x1)
            D_db3f = x1;
        if (y0 < D_db41)
            D_db41 = y0;
        if (D_db43 < y1)
            D_db43 = y1;
    }
    if (kind == 2 || kind == 3 || kind == 5 || kind == 4) {
        D_db89[i] = 7;
        w = x1 - x0 + 1;
        h = y1 - y0 + 1;
        if (((w < 3 && h < 3) || w == 1 || h == 1) && (kind != 4 || (dir != 0 && dir != 3)))
            D_db89[i] = 0;
    }
    t = D_db8f + D_db81 * 1600;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            if (t[x * 40 + y] == 0)
                t[x * 40 + y] = kind;
    D_db91[i] = kind;
    for (y = y0; y <= y1; y++) {
        sub_s006_1a57_02a8(D_db81, x0, y);
        if (x0 < x1)
            sub_s006_1a57_02a8(D_db81, x1, y);
    }
    for (x = x0 + 1; x < x1; x++) {
        sub_s006_1a57_02a8(D_db81, x, y0);
        if (y0 < y1)
            sub_s006_1a57_02a8(D_db81, x, y1);
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
    int X, i, j, v, w, s, dx, dy;
    unsigned char out, bit, nb;
    unsigned char far *p;

    if (x <= 0 || x >= 39 || y <= 0 || y >= 39)
        return;
    X = l * 40 + x;
    i = X * 40 + y;
    v = D_db8f[i];
    out = 0;
    for (s = 0; s < 4; s++) {
        switch (s) {
        case 0:
            dx = 0;
            dy = -1;
            bit = 1;
            nb = 4;
            break;
        case 1:
            dx = 1;
            bit = 2;
            dy = 0;
            nb = 8;
            break;
        case 2:
            dx = 0;
            bit = 4;
            dy = 1;
            nb = 1;
            break;
        case 3:
            dx = -1;
            bit = 8;
            dy = 0;
            nb = 2;
            break;
        default:
            continue;
        }
        j = (X + dx) * 40 + dy + y;
        w = D_db8f[j];
        p = (unsigned char far *)(D_db8d + j);
        if (w < 2 || w > 8)
            out |= bit;
        else if (((w == 7) == (v == 7) || w == 5 || v == 5) &&
                 ((w == 3) == (v == 3) || w == 6 || v == 6))
            *p &= ~nb;
        else {
            out |= bit;
            *p |= nb;
        }
    }
    D_db8d[i] = out;
}

/* remove room r from free strip `slot` (-1: the strip that overlaps it) */
void far sub_s006_1a57_042e(int slot, int r)
{
    int n, lo, hi, a, b, k, v, found;
    int _based(D_db39) *o;
    struct gslot _based(D_db39) *s;
    struct gslot _based(D_db39) *t;

    n = D_db67[D_db81];
    if (slot < -1 || slot >= n || r < 0 || r >= D_db45[D_db81])
        return;
    o = D_db3b + (D_db81 * 30 + r) * 4;
    if (slot == -1) {
        found = 0;
        for (slot = 0, s = D_db65 + D_db81 * 16; slot < D_db67[D_db81]; slot++, s++) {
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
    s = D_db65 + D_db81 * 16 + slot;
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
    if (s->a[b] < D_db3b[D_db81 * 120 + a]) {
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
        t = D_db65 + D_db81 * 16 + n;
        *t = *s;
        t->a[lo] = o[hi] + 1;
        s->a[hi] = o[lo] - 1;
        D_db67[D_db81]++;
        return;
    }
    v = o[lo] - 1;
    if (s->a[hi] < v)
        v = s->a[hi];
    s->a[hi] = v;
}
