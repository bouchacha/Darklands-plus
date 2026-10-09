/* place: s006 1abf:000a */
/* orig: size=0ba6 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 4 functions written from Ghidra + asm (dec16 drafts were
 * stubs). Doors and openings between rooms: D_db93 cell codes 3/5 (door on side 0/2) and
 * 4/6 (side 1/3), 11 = stairs; D_db8d wall bits are cleared for openings.
 */

struct res { int a; int b; };

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;
extern unsigned char _based(D_db39) *D_db8d;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern int _based(D_db39) *D_db93;
extern int D_db45[5];
extern int D_db7f;
extern int D_db81;

int far sub_s004_0e76_20f6(int, int);
int far sub_s005_146d_000c(int, int, int);
void far sub_s005_146d_0066(int, int);
int far sub_s005_146d_017a(int, int, int);
int far sub_s005_146d_01d4(int, int, int *, int *, int *, int);
void far sub_s005_146d_07f4(int, int);
void far sub_s006_1abf_038e(int, int, int, int, int, int, int);
int far sub_s006_1abf_076e(int, int, int, int, int);
int far sub_s006_1abf_0b78(int, int, int, int);

/* connect room a: kind 3 = to room 0, 7 = to b, c and everything else touching, 8 = to b
 * and c unless already connected through a third room */
void far sub_s006_1abf_000a(int a, int kind, int b, int c)
{
    int i, ok, z;
    int dir, lo, hi;

    if (kind == 3) {
        if (D_db81 >= D_db7f)
            return;
        if (!sub_s005_146d_01d4(a, 0, &dir, &lo, &hi, 0))
            return;
        sub_s006_1abf_038e(a, 0, dir, lo, hi, 1, 1);
        return;
    }
    if (kind == 7) {
        if (D_db91[D_db81 * 30 + b] == 4 && D_db91[D_db81 * 30 + c] == 4)
            ok = 0;
        else
            ok = 1;
        if (!sub_s005_146d_017a(D_db81, a, b) && sub_s005_146d_01d4(a, b, &dir, &lo, &hi, 0))
            sub_s006_1abf_038e(a, b, dir, lo, hi, ok, 1);
        if (!sub_s005_146d_017a(D_db81, a, c) && sub_s005_146d_01d4(a, c, &dir, &lo, &hi, 0))
            sub_s006_1abf_038e(a, c, dir, lo, hi, ok, 1);
        for (i = 0; i < D_db45[D_db81]; i++) {
            if (i != a && b != i && c != i && !sub_s005_146d_017a(D_db81, i, b) &&
                !sub_s005_146d_017a(D_db81, i, c) && !sub_s005_146d_017a(D_db81, i, a) &&
                !sub_s005_146d_000c(D_db81, i, b) && !sub_s005_146d_000c(D_db81, i, c) &&
                !sub_s005_146d_000c(D_db81, i, a) &&
                sub_s005_146d_01d4(a, i, &dir, &lo, &hi, 0))
                sub_s006_1abf_038e(a, i, dir, lo, hi, ok, 1);
        }
        return;
    }
    if (kind != 8)
        return;
    ok = b != -1;
    if (ok) {
        z = 0;
        for (i = 0; i < D_db45[D_db81]; i++) {
            if (i != a && b != i && sub_s005_146d_017a(D_db81, i, b) &&
                sub_s005_146d_017a(D_db81, i, a) && sub_s005_146d_000c(D_db81, i, a))
                ok = 0;
            if (!ok)
                break;
        }
    }
    if (ok && sub_s005_146d_01d4(a, b, &dir, &lo, &hi, 0))
        sub_s006_1abf_038e(a, b, dir, lo, hi, 1, 0);
    ok = c != -1;
    if (ok) {
        z = 0;
        for (i = 0; i < D_db45[D_db81]; i++) {
            if (i != a && c != i && sub_s005_146d_017a(D_db81, i, c) &&
                sub_s005_146d_017a(D_db81, i, a) && sub_s005_146d_000c(D_db81, i, a))
                ok = 0;
            if (!ok)
                break;
        }
    }
    if (!ok)
        return;
    if (!sub_s005_146d_01d4(a, c, &dir, &lo, &hi, 0))
        return;
    sub_s006_1abf_038e(a, c, dir, lo, hi, 1, 0);
}

/* open side dir of room a somewhere in [lo, hi] (a door when `door` and a spot is found) */
void far sub_s006_1abf_038e(int a, int b, int dir, int lo, int hi, int door, int force)
{
    int n, tries, ok, p, i, k;
    int _based(D_db39) *o;

    n = hi - lo + 1;
    if (n <= 0)
        return;
    tries = 0;
    ok = 0;
    do {
        if (tries >= n)
            break;
        tries++;
        p = sub_s004_0e76_20f6(1, n) + lo - 1;
        ok = sub_s006_1abf_076e(a, dir, p, 1, 1);
    } while (!ok);
    if (!ok && door)
        for (p = lo; p <= hi && !(ok = sub_s006_1abf_076e(a, dir, p, 1, 1)); p++)
            ;
    if (!ok && !force)
        return;
    if (!ok && door)
        p = sub_s004_0e76_20f6(1, n) + lo - 1;
    o = D_db3b + (D_db81 * 30 + a) * 4;
    if (dir == 0) {
        i = (D_db81 * 40 + p) * 40 + o[2];
        if (D_db8f[i] < 2 || D_db8f[i] > 8)
            return;
        k = D_db8f[i - 1];
        if (k < 2 || k > 8)
            return;
        D_db8d[i] &= ~1;
        D_db8d[i - 1] &= ~4;
        if (door && ok) {
            D_db93[i] = 3;
            D_db93[i - 1] = 5;
        }
    } else if (dir == 1) {
        i = (D_db81 * 40 + o[1]) * 40 + p;
        if (D_db8f[i] < 2 || D_db8f[i] > 8)
            return;
        if (D_db8f[i + 40] < 2 || D_db8f[i + 40] > 8)
            return;
        D_db8d[i] &= ~2;
        D_db8d[i + 40] &= ~8;
        if (door && ok) {
            D_db93[i] = 4;
            D_db93[i + 40] = 6;
        }
    } else if (dir == 2) {
        i = (D_db81 * 40 + p) * 40 + o[3];
        if (D_db8f[i] < 2 || D_db8f[i] > 8)
            return;
        if (D_db8f[i + 1] < 2 || D_db8f[i + 1] > 8)
            return;
        D_db8d[i] &= ~4;
        D_db8d[i + 1] &= ~1;
        if (door && ok) {
            D_db93[i] = 5;
            D_db93[i + 1] = 3;
        }
    } else if (dir == 3) {
        i = (D_db81 * 40 + o[0]) * 40 + p;
        if (D_db8f[i] < 2 || D_db8f[i] > 8)
            return;
        k = D_db8f[i - 40];
        if (k < 2 || k > 8)
            return;
        D_db8d[i] &= ~8;
        D_db8d[i - 40] &= ~2;
        if (door && ok) {
            D_db93[i] = 6;
            D_db93[i - 40] = 4;
        }
    }
    if (!sub_s005_146d_017a(D_db81, a, b))
        sub_s005_146d_07f4(a, b);
    if ((!door || !ok) && !sub_s005_146d_000c(D_db81, a, b))
        sub_s005_146d_0066(a, b);
}

#define FLOOR(x, y) (D_db8f[(D_db81 * 40 + (x)) * 40 + (y)] >= 2 && \
                     D_db8f[(D_db81 * 40 + (x)) * 40 + (y)] <= 8)
#define FREE(x, y) (sub_s006_1abf_0b78(x, y, 0, 1) || sub_s006_1abf_0b78(x, y, 11, 1))

/* can a door/opening go on side dir of room r at position p? */
int far sub_s006_1abf_076e(int r, int dir, int p, int chk, int both)
{
    int c, x, y, x2, y2, ok;
    int _based(D_db39) *o;

    o = D_db3b + (D_db81 * 30 + r) * 4;
    switch (dir) {
    case 0:
        c = o[2];
        y2 = c - 1;
        x2 = x = p;
        y = c;
        break;
    case 1:
        c = o[1];
        x2 = c + 1;
        y = y2 = p;
        x = c;
        break;
    case 2:
        c = o[3];
        y2 = c + 1;
        x2 = x = p;
        y = c;
        break;
    case 3:
        c = o[0];
        x2 = c - 1;
        y2 = y = p;
        x = c;
        break;
    default:
        return 0;
    }
    ok = FLOOR(x, y) ? 1 : 0;
    if (!ok)
        return 0;
    if (both)
        ok = FLOOR(x2, y2) ? 1 : 0;
    if (!ok)
        return 0;
    if (!chk)
        return ok;
    switch (dir) {
    case 0:
        if (!FREE(p, c) || !FREE(p, c - 1))
            return 0;
        if (!sub_s006_1abf_0b78(p - 1, c, 3, 0) || !sub_s006_1abf_0b78(p - 1, c - 1, 5, 0) ||
            !sub_s006_1abf_0b78(p + 1, c, 3, 0) || !sub_s006_1abf_0b78(p + 1, c - 1, 5, 0))
            return 0;
        return 1;
    case 1:
        if (!FREE(c, p) || !FREE(c + 1, p))
            return 0;
        if (!sub_s006_1abf_0b78(c, p - 1, 4, 0) || !sub_s006_1abf_0b78(c + 1, p - 1, 6, 0) ||
            !sub_s006_1abf_0b78(c, p + 1, 4, 0) || !sub_s006_1abf_0b78(c + 1, p + 1, 6, 0))
            return 0;
        return 1;
    case 2:
        if (!FREE(p, c) || !FREE(p, c + 1))
            return 0;
        if (!sub_s006_1abf_0b78(p - 1, c, 5, 0) || !sub_s006_1abf_0b78(p - 1, c + 1, 3, 0) ||
            !sub_s006_1abf_0b78(p + 1, c, 5, 0) || !sub_s006_1abf_0b78(p + 1, c + 1, 3, 0))
            return 0;
        return 1;
    case 3:
        if (!FREE(c, p) || !FREE(c - 1, p))
            return 0;
        if (!sub_s006_1abf_0b78(c, p - 1, 6, 0) || !sub_s006_1abf_0b78(c - 1, p - 1, 4, 0) ||
            !sub_s006_1abf_0b78(c, p + 1, 6, 0) || !sub_s006_1abf_0b78(c - 1, p + 1, 4, 0))
            return 0;
        return 1;
    }
    return 0;
}

/* (D_db93 cell == v) == eq */
int far sub_s006_1abf_0b78(int x, int y, int v, int eq)
{
    if ((D_db93[(D_db81 * 40 + x) * 40 + y] == v) == eq)
        return 1;
    return 0;
}
