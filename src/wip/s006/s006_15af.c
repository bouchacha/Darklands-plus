/* place: s006 15af:0000 */
/* orig: size=0886 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from Ghidra + asm (dec16 drafts were
 * stubs). Keep level setup: room 0 of a level is the keep (outer rectangle D_db3d..D_db43),
 * D_db65 slots = the 4 strips around it {x0, x1, y0, y1, side}; D_db25 = gate side.
 * Work segment layout: see s005_1432.c.
 */

struct res { int a; int b; };
struct gslot { int a[4]; int b; };                  /* {x0, x1, y0, y1, side}, 16 per level */

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern struct gslot _based(D_db39) *D_db65;
extern int _based(D_db39) *D_db89;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern int _based(D_db39) *D_db93;
extern int D_db25;
extern int D_db3d, D_db3f, D_db41, D_db43;
extern int D_db45[5];
extern int D_db4f[5];
extern int D_db67[5];
extern int D_db81;

int far sub_s004_0e76_20f6(int, int);
void far sub_s006_1a57_000c(int, int, int, int, int, int);
void far sub_s006_1a57_02a8(int, int, int);
void far sub_s006_1a57_042e(int, int);

/* the keep: room 0, and the 4 strips around it */
void far sub_s006_15af_0000(void)
{
    int w, h, x0, x1, y0, y1, i;
    struct gslot _based(D_db39) *s;
    int _based(D_db39) *o;

    if (D_db81 == 1) {
        w = sub_s004_0e76_20f6(1, 4) + 4;
        h = w;
        if (sub_s004_0e76_20f6(1, 2) > 1) {
            if (sub_s004_0e76_20f6(1, 2) < 2)
                h = w + sub_s004_0e76_20f6(1, 3) - 1;
            else
                w = w + sub_s004_0e76_20f6(1, 3) - 1;
        }
        y0 = 20 - h / 2;
        x0 = 20 - w / 2;
        x1 = w + x0 - 1;
        y1 = h + y0 - 1;
    } else {
        o = D_db3b + 30 * 4;
        x0 = o[0];
        x1 = o[1];
        y0 = o[2];
        y1 = o[3];
    }
    sub_s006_1a57_000c(2, x0, x1, y0, y1, -1);
    D_db67[D_db81] = 4;
    s = D_db65 + D_db81 * 16;
    o = D_db3b + D_db81 * 120;
    s[0].a[3] = o[2] - 1;
    s[0].a[2] = D_db41 - 1;
    s[2].a[2] = o[3] + 1;
    s[2].a[3] = D_db43 + 1;
    s[2].a[0] = s[0].a[0] = o[0] - 1;
    s[2].a[1] = s[0].a[1] = o[1] + 1;
    s[3].a[1] = o[0] - 1;
    s[3].a[0] = D_db3d - 1;
    s[1].a[0] = o[1] + 1;
    s[1].a[1] = D_db3f + 1;
    s[3].a[2] = s[1].a[2] = o[2] - 1;
    s[3].a[3] = s[1].a[3] = o[3] + 1;
    for (i = 0; i < 4; i++)
        s[i].b = i;
}

/* level 1: the gate and the path to it */
void far sub_s006_15af_01cc(int a)
{
    int i, lo_i, hi_i, lo, hi, mid, k, v;
    int _based(D_db39) *o;
    struct gslot _based(D_db39) *s;

    if (D_db81 != 1)
        return;
    if (a == 0 || sub_s004_0e76_20f6(1, 2) > 1)
        i = 0;
    else
        i = sub_s004_0e76_20f6(1, D_db4f[1] + 1) - 1;
    D_db89[30 + i] = 3;
    if (i == 0) {
        o = D_db3b + 30 * 4;
        do {
            do {
                D_db25 = sub_s004_0e76_20f6(1, 4) - 1;
                s = D_db65 + 16 + D_db25;
                if (D_db25 == 0 || D_db25 == 2) {
                    lo_i = 0;
                    hi_i = 1;
                } else {
                    lo_i = 2;
                    hi_i = 3;
                }
                hi = s->a[hi_i];
                lo = s->a[lo_i];
            } while (hi - lo + 1 < 4);
            mid = (o[lo_i] + o[hi_i]) / 2;
            if ((o[hi_i] - o[lo_i] + 1) % 2 > 0)
                mid -= sub_s004_0e76_20f6(1, 2) - 1;
        } while (mid <= lo || hi <= mid + 1);
        if (D_db25 == 0 || D_db25 == 2)
            sub_s006_1a57_000c(6, mid, mid + 1, s->a[2], s->a[3], D_db25);
        else
            sub_s006_1a57_000c(6, s->a[0], s->a[1], mid, mid + 1, D_db25);
        sub_s006_1a57_042e(D_db25, D_db45[1] - 1);
        return;
    }
    o = D_db3b + (30 + i) * 4;
    if (sub_s004_0e76_20f6(1, 2) < 2) {
        k = sub_s004_0e76_20f6(1, o[3] - o[2]) - 1 + o[2];
        if (o[0] < D_db3b[120]) {
            D_db25 = 3;
            sub_s006_1a57_000c(6, D_db3d - 1, o[0] - 1, k, k + 1, 3);
        } else {
            D_db25 = 1;
            sub_s006_1a57_000c(6, o[1] + 1, D_db3f + 1, k, k + 1, 1);
        }
    } else {
        k = sub_s004_0e76_20f6(1, o[1] - o[0]) - 1 + o[0];
        if (o[2] < D_db3b[122]) {
            D_db25 = 0;
            sub_s006_1a57_000c(6, k, k + 1, D_db41 - 1, o[2] - 1, 0);
        } else {
            D_db25 = 2;
            sub_s006_1a57_000c(6, k, k + 1, o[3] + 1, D_db43 + 1, 2);
        }
    }
}

/* widen the outer rectangle around the gate room (kind 6) */
void far sub_s006_15af_0470(void)
{
    int one, f, e, x, y;
    int _based(D_db39) *o;
    int (_based(D_db39) *w)[40];
    int (_based(D_db39) *t)[40];

    one = D_db81 == 1;
    f = 0;
    if (D_db91[30 + D_db4f[1] + 1] != 6)
        return;
    o = D_db3b + (30 + D_db4f[1] + 1) * 4;
    w = (int (_based(D_db39) *)[40])(D_db93 + 1600);
    if (D_db25 == 0) {
        e = o[2];
        if (one) {
            f = D_db41 <= e;
            if (f)
                o[2] = D_db41 - 1;
        } else {
            f = !(e < D_db41 && D_db41 < D_db43 && w[o[0]][e] == 0 && w[o[0] + 1][e] == 0);
            if (f) {
                e = o[2] - 1;
                if (e > D_db41 - 1)
                    e = D_db41 - 1;
                D_db41 = o[2] = e;
            } else
                D_db41 = o[2];
            w[o[0]][e] = 1;
            w[o[0] + 1][e] = 1;
        }
    } else if (D_db25 == 1) {
        e = o[1];
        if (one) {
            f = e <= D_db3f;
            if (f)
                o[1] = D_db3f + 1;
        } else {
            f = !(D_db3f < e && D_db3d < D_db3f && w[e][o[2]] == 0 && w[e][o[2] + 1] == 0);
            if (f) {
                e = o[1] + 1;
                if (e < D_db3f + 1)
                    e = D_db3f + 1;
                D_db3f = o[1] = e;
            } else
                D_db3f = o[1];
            w[e][o[2]] = 1;
            w[e][o[2] + 1] = 1;
        }
    } else if (D_db25 == 2) {
        e = o[3];
        if (one) {
            f = e <= D_db43;
            if (f)
                o[3] = D_db43 + 1;
        } else {
            f = !(D_db43 < e && D_db41 < D_db43 && w[o[0]][e] == 0 && w[o[0] + 1][e] == 0);
            if (f) {
                e = o[3] + 1;
                if (e < D_db43 + 1)
                    e = D_db43 + 1;
                D_db43 = o[3] = e;
            } else
                D_db43 = o[3];
            w[o[0]][e] = 1;
            w[o[0] + 1][e] = 1;
        }
    } else if (D_db25 == 3) {
        e = o[0];
        if (one) {
            f = D_db3d <= e;
            if (f)
                o[0] = D_db3d - 1;
        } else {
            f = !(e < D_db3d && D_db3d < D_db3f && w[e][o[2]] == 0 && w[e][o[2] + 1] == 0);
            if (f) {
                e = o[0] - 1;
                if (e > D_db3d - 1)
                    e = D_db3d - 1;
                D_db3d = o[0] = e;
            } else
                D_db3d = o[0];
            w[e][o[2]] = 1;
            w[e][o[2] + 1] = 1;
        }
    }
    if (!f)
        return;
    t = (int (_based(D_db39) *)[40])(D_db8f + 1600);
    for (x = o[0]; x <= o[1]; x++)
        for (y = o[2]; y <= o[3]; y++)
            t[x][y] = 6;
    for (x = o[0]; x <= o[1]; x++)
        for (y = o[2]; y <= o[3]; y++)
            sub_s006_1a57_02a8(1, x, y);
}
