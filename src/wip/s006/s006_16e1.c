/* place: s006 16e1:000a */
/* orig: size=0fe0 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): both functions written from Ghidra + asm (dec16 drafts were stubs).
 * Outer wall around the keep: 000a extends the 4 strips (D_db65 slots 0..3 of the level) along
 * the rooms built against the keep (room kind 3, side from s006 1637 04d0) and then adds up to
 * 4 wall segments (drawn with 1a57 000c kind 8) connected by 1abf 000a; 0a28 picks the place
 * of a segment on one side of the keep (b1/b2 = usable cells next to the neighbouring rooms).
 */

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes */

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern int _based(D_db39) *D_db65;                  /* slots as int[5][16][5] */
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern struct gexit _based(D_db39) *D_db95;
extern int D_db3d, D_db3f, D_db41, D_db43;
extern int D_db45[5];
extern int D_db67[5];
extern int D_db71[5];
extern int D_db7f;
extern int D_db81;

int far sub_s004_0e76_20f6(int, int);
int far sub_s005_146d_0858(int, int, int);
int far sub_s006_1637_04d0(int, int);
void far sub_s006_1a57_000c(int, int, int, int, int, int);
void far sub_s006_1abf_000a(int, int, int, int);
int far sub_s006_1abf_076e(int, int, int, int, int);
void far sub_s006_16e1_0a28(int, int *, int *, int *);

int abs(int);
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

/* the wall around the keep: strips 0..3 hug the rooms built against it */
void far sub_s006_16e1_000a(void)
{
    int pb, tries, side, i, freen;
    int adj[8];
    int z, n, k, pa, done, y, x;
    int far *s;
    int far *r;

    tries = 0;
    freen = 0;
    done = 0;
    if (D_db81 != D_db7f || D_db45[D_db81] <= 0)
        return;
    for (i = 0; i <= 3; i++)
        adj[i] = 0;
    for (i = 4; i <= 7; i++)
        adj[i] = -1;
    s = D_db65 + D_db81 * 80;
    D_db67[D_db81] = 4;
    s[17] = s[7] = s[2] = D_db41;
    s[11] = s[1] = s[6] = D_db3f;
    s[18] = s[8] = s[13] = D_db43;
    s[10] = s[0] = s[15] = D_db3d;
    s[3] = D_db3b[0x7a];
    s[5] = D_db3b[0x79];
    s[12] = D_db3b[0x7b];
    s[16] = D_db3b[0x78];
    z = 0;
    for (i = 0; i < D_db45[D_db81]; i++) {
        if (!sub_s005_146d_0858(D_db81, i, 30))
            freen++;
        if (D_db91[D_db81 * 30 + i] != 3)
            continue;
        side = sub_s006_1637_04d0(D_db81, i);
        adj[side] = i;
        r = D_db3b + (i + D_db81 * 30) * 4;
        switch (side) {
        case 4:
            s[0] = r[1] + 1;
            s[17] = r[3] + 1;
            s[2] = max(s[2], r[2]);
            s[3] = min(s[3], r[3]);
            s[15] = max(s[15], r[0]);
            s[16] = min(r[1], s[16]);
            s[10] = max(s[10], r[0]);
            s[7] = max(r[2], s[7]);
            break;
        case 5:
            s[1] = r[0] - 1;
            s[7] = r[3] + 1;
            s[2] = max(s[2], r[2]);
            s[3] = min(s[3], r[3]);
            s[5] = max(s[5], r[0]);
            s[6] = min(r[1], s[6]);
            s[11] = min(r[1], s[11]);
            s[17] = max(r[2], s[17]);
            break;
        case 6:
            s[11] = r[0] - 1;
            s[8] = r[2] - 1;
            s[12] = max(r[2], s[12]);
            s[13] = min(r[3], s[13]);
            s[5] = max(s[5], r[0]);
            s[6] = min(r[1], s[6]);
            s[1] = min(s[1], r[1]);
            s[18] = min(s[18], r[3]);
            break;
        case 7:
            s[10] = r[1] + 1;
            s[18] = r[2] - 1;
            s[12] = max(r[2], s[12]);
            s[13] = min(r[3], s[13]);
            s[15] = max(s[15], r[0]);
            s[16] = min(r[1], s[16]);
            s[0] = max(s[0], r[0]);
            s[8] = min(r[3], s[8]);
            break;
        }
    }
    while (done < 4) {
        if (freen <= 0 && sub_s004_0e76_20f6(1, D_db45[D_db81] + 1) != 1)
            return;
        if (tries >= 6)
            return;
        n = sub_s004_0e76_20f6(1, 4 - done);
        k = 0;
        for (i = 0, side = 8; i <= 3 && n > k; i++)
            if (adj[i] == 0)
                k++;
        if (n == k) {
            side = i - 1;
            sub_s006_16e1_0a28(side, adj, &pa, &pb);
            if (pa >= 0 && pb >= 0) {
                y = x = -1;
                switch (side) {
                case 0:
                    k = s[0];
                    if (k <= D_db3d) {
                        i = s[16] - s[15] + 1;
                        k = sub_s004_0e76_20f6(1, i - 1) + s[15];
                    }
                    if (adj[4] < 0) {
                        s[15] = k;
                        s[16] = k;
                        s[17] = pa + 2;
                        if (adj[7] < 0)
                            s[10] = max(k, s[10]);
                    }
                    n = s[1];
                    if (n >= D_db3f) {
                        i = s[6] - s[5] + 1;
                        n = sub_s004_0e76_20f6(1, i - 1) + s[5];
                    }
                    if (adj[5] < 0) {
                        s[5] = n;
                        s[6] = n;
                        s[7] = pb + 2;
                        if (adj[6] < 0)
                            s[11] = min(n, s[11]);
                    }
                    if (pb == pa) {
                        sub_s006_1a57_000c(8, k, n, pa, pa + 1, 0);
                    } else {
                        i = (n + k) / 2;
                        sub_s006_1a57_000c(8, k, min(i + 1, n - 1), pa, pa + 1, 0);
                        sub_s006_1a57_000c(8, max(i - 1, k + 1), n, pb, pb + 1, 0);
                    }
                    x = adj[4];
                    y = adj[5];
                    break;
                case 1:
                    k = s[7];
                    if (k <= D_db41) {
                        i = s[3] - s[2] + 1;
                        k = sub_s004_0e76_20f6(1, i - 1) + s[2];
                    }
                    if (adj[5] < 0) {
                        s[2] = k;
                        s[3] = k;
                        s[1] = pa - 2;
                        if (adj[4] < 0)
                            s[17] = max(k, s[17]);
                    }
                    n = s[8];
                    if (n > D_db43) {
                        i = s[13] - s[12] + 1;
                        n = sub_s004_0e76_20f6(1, i - 1) + s[12];
                    }
                    if (adj[6] < 0) {
                        s[12] = n;
                        s[13] = n;
                        s[11] = pb - 2;
                        if (adj[7] < 0)
                            s[18] = min(n, s[18]);
                    }
                    if (pb == pa) {
                        sub_s006_1a57_000c(8, pa - 1, pa, k, n, 1);
                    } else {
                        i = (n + k) / 2;
                        sub_s006_1a57_000c(8, pa - 1, pa, k, min(i + 1, n - 1), 1);
                        sub_s006_1a57_000c(8, pb - 1, pb, max(i - 1, k + 1), n, 1);
                    }
                    x = adj[5];
                    y = adj[6];
                    break;
                case 2:
                    k = s[10];
                    if (k <= D_db3d) {
                        i = s[16] - s[15] + 1;
                        k = sub_s004_0e76_20f6(1, i - 1) + s[15];
                    }
                    if (adj[7] < 0) {
                        s[15] = k;
                        s[16] = k;
                        s[18] = pa - 2;
                        if (adj[4] < 0)
                            s[0] = max(k, s[0]);
                    }
                    n = s[11];
                    if (n > D_db3f) {
                        i = s[6] - s[5] + 1;
                        n = sub_s004_0e76_20f6(1, i - 1) + s[5];
                    }
                    if (adj[6] < 0) {
                        s[5] = n;
                        s[6] = n;
                        s[8] = pb - 2;
                        if (adj[5] < 0)
                            s[1] = min(n, s[1]);
                    }
                    if (pb == pa) {
                        sub_s006_1a57_000c(8, k, n, pa - 1, pa, 2);
                    } else {
                        i = (n + k) / 2;
                        sub_s006_1a57_000c(8, k, min(i + 1, n - 1), pa - 1, pa, 2);
                        sub_s006_1a57_000c(8, max(i - 1, k + 1), n, pb - 1, pb, 2);
                    }
                    x = adj[7];
                    y = adj[6];
                    break;
                case 3:
                    k = s[17];
                    if (k <= D_db41) {
                        i = s[3] - s[2] + 1;
                        k = sub_s004_0e76_20f6(1, i - 1) + s[2];
                    }
                    if (adj[4] < 0) {
                        s[2] = k;
                        s[3] = k;
                        s[0] = pa + 2;
                        if (adj[5] < 0)
                            s[7] = max(k, s[7]);
                    }
                    n = s[18];
                    if (n > D_db43) {
                        i = s[13] - s[12] + 1;
                        n = sub_s004_0e76_20f6(1, i - 1) + s[12];
                    }
                    if (adj[7] < 0) {
                        s[12] = n;
                        s[13] = n;
                        s[10] = pb + 2;
                        if (adj[6] < 0)
                            s[8] = min(n, s[8]);
                    }
                    if (pb == pa) {
                        sub_s006_1a57_000c(8, pa, pa + 1, k, n, 3);
                    } else {
                        i = (n + k) / 2;
                        sub_s006_1a57_000c(8, pa, pa + 1, k, min(i + 1, n - 1), 3);
                        sub_s006_1a57_000c(8, pb, pb + 1, max(i - 1, k + 1), n, 3);
                    }
                    x = adj[4];
                    y = adj[7];
                    break;
                }
                adj[side] = 1;
                if (pb == pa) {
                    sub_s006_1abf_000a(D_db45[D_db81] - 1, 8, x, y);
                } else {
                    sub_s006_1abf_000a(D_db45[D_db81] - 2, 8, x, -1);
                    sub_s006_1abf_000a(D_db45[D_db81] - 1, 8, -1, y);
                }
                done++;
                tries++;
                freen = 0;
                for (i = 0; i < D_db45[D_db81]; i++)
                    if (!sub_s005_146d_0858(D_db81, i, 30))
                        freen++;
                continue;
            }
            adj[side] = 1;
            done++;
        }
        tries++;
    }
}

/* place a wall segment on side `side` of the keep: *pa/*pb = its cells next to the rooms
 * on both ends (adj[first], adj[second]), -1 if there is no place */
void far sub_s006_16e1_0a28(int side, int *adj, int *pa, int *pb)
{
    int d, x2, best, first, p, q, pp, ok, i, hi, step, col, x1, dd, second, lo, rm;
    int b1[40], b2[40];
    int far *s;
    int far *m;
    int far *r;
    struct gexit far *e;

    s = D_db65 + D_db81 * 80;
    m = D_db8f + D_db81 * 1600;
    for (i = 0; i < 40; i++) {
        b2[i] = 0;
        b1[i] = 0;
    }
    switch (side) {
    case 0:
        first = 4;
        second = 5;
        step = 1;
        break;
    case 1:
        first = 5;
        second = 6;
        step = -1;
        break;
    case 2:
        first = 7;
        second = 6;
        step = -1;
        break;
    case 3:
        first = 4;
        second = 7;
        step = 1;
        break;
    default:
        return;
    }
    if (side == 0 || side == 2) {
        lo = 2;
        col = 0;
        x1 = d = hi = 3;
        dd = x2 = 1;
    } else {
        hi = 1;
        x2 = 3;
        x1 = d = lo = 0;
        dd = col = 2;
    }
    if (adj[first] < 0) {
        for (i = s[side * 5 + lo]; i <= s[side * 5 + hi]; i++)
            b1[i] |= 7;
    } else {
        rm = adj[first];
        r = D_db3b + (rm + D_db81 * 30) * 4;
        for (i = r[lo] - 1; i <= r[hi] + 1; i++) {
            b1[i] |= 1;
            if (sub_s006_1abf_076e(rm, dd, i, 1, 0))
                b1[i] |= 2;
        }
        for (i = r[lo]; i <= r[hi]; i++)
            if ((b1[i] & 1) && (b1[i + step] & 1) && ((b1[i] & 2) || (b1[i + step] & 2)))
                b1[i] |= 4;
    }
    if (adj[second] < 0) {
        for (i = s[side * 5 + lo]; i <= s[side * 5 + hi]; i++)
            b2[i] |= 7;
    } else {
        rm = adj[second];
        r = D_db3b + (rm + D_db81 * 30) * 4;
        i = 0;
        ok = 1;
        do {
            if (i >= D_db71[D_db81])
                break;
            e = D_db95 + D_db81 * 4 + i;
            if (e->x4 == rm &&
                ((d == 3 && e->x0a == 3 && e->x0 == r[0]) ||
                 (d == 0 && e->x0a == 0 && r[2] == e->x2)))
                ok = 0;
            i++;
        } while (ok);
        for (i = r[lo] - 1; i <= r[hi] + 1; i++) {
            if (ok || (d == 3 && m[r[col] * 40 + i] != 10) ||
                (d == 0 && m[r[col] + i * 40] != 10))
                b2[i] |= 1;
            if (sub_s006_1abf_076e(rm, d, i, 1, 0))
                b2[i] |= 2;
        }
        for (i = r[lo]; i <= r[hi]; i++)
            if ((b2[i] & 1) && (b2[i + step] & 1) && ((b2[i] & 2) || (b2[i + step] & 2)))
                b2[i] |= 4;
    }
    best = s[side * 5 + hi] - s[side * 5 + lo] + 1;
    i = ok = 0;
    for (p = lo; !ok && i < 25 && i - 8 < best; i++) {
        if (i < 8)
            p = sub_s004_0e76_20f6(1, best) + s[side * 5 + lo] - 1;
        else
            p = s[side * 5 + lo] + i - 8;
        if ((b1[p] & 4) && (b2[p] & 4))
            ok = 1;
        else
            ok = 0;
    }
    if (!ok) {
        q = pp = -1;
        ok = 0;
        best = 10;
        for (i = 0; !ok && i < 40; i++)
            if (b1[i] & 4)
                for (p = i - 2; !ok && p <= i + 2; p++)
                    if ((b2[p] & 4) && abs(i - p) < best) {
                        pp = i;
                        q = p;
                        ok = 1;
                        best = abs(i - p);
                    }
        if (best < 10) {
            *pa = pp;
            *pb = q;
            return;
        }
        p = -1;
    }
    *pb = p;
    *pa = p;
}
