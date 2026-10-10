/* place: s006 17df:000a */
/* orig: size=11ae */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): all 6 functions written from Ghidra + asm (dec16 drafts were
 * stubs; Ghidra timed out on 000a). Corridors between rooms (cells of kind 7): 000a picks
 * pairs of rooms that are not yet connected, 0580 (straight, rooms side by side) and 09b2
 * (L-shaped, rooms diagonal) find a free strip, 0ec4 checks it, 1112 opens the room walls.
 */

struct res { int a; int b; };

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern int _based(D_db39) *D_db93;
extern int D_db3d[4];                               /* keep rectangle x0, x1, y0, y1 */
extern int D_db45[5];
extern int D_db81;

int far sub_s004_0e76_20f6(int, int);
int far sub_s005_146d_0256(int, int, int, int, int, int, int, int, int *, int *, int *, int);
int far sub_s005_146d_017a(int, int, int);
int far sub_s005_146d_0858(int, int, int);
int far sub_s005_146d_09e0(int, int, int);
void far sub_s005_146d_0ae8(int, int);
void far sub_s005_146d_0b42(int, int);
int far sub_s005_146d_0b9c(int, int, int);
int far sub_s005_146d_0d7a(int, int, int, int);
int far sub_s005_1551_0006(int, int, int, int, int, int);
int far sub_s005_1551_00bc(int, int, int, int, int);
void far sub_s006_1a57_000c(int, int, int, int, int, int);
void far sub_s006_1abf_000a(int, int, int, int);
int far sub_s006_1abf_076e(int, int, int, int, int);
int far sub_s006_17df_0580(int, int, int, int *, int *, int *, int *, int);
int far sub_s006_17df_09b2(int, int, int, int, int, int *, int *, int *, int *, int);
int far sub_s006_17df_0d16(int, int, int, int);
int far sub_s006_17df_0ec4(int, int, int, int, int, int, int);
int far sub_s006_17df_1112(int, int, int, int, int, int);

/* corridors between rooms of the current level */
void far sub_s006_17df_000a(int kind)
{
    int cnt, a;
    long m;
    int w;
    long links[30];
    int again, typ, f, tries, i;
    long mask;
    int done, pick, k, b, x0, all, y0, x1, side, y1, pass;

    mask = 0L;
    for (i = 0; i < 30; i++)
        links[i] = 0L;
    again = pass = 1;
    for (all = 0; again && pass <= 5 && D_db45[D_db81] < 29; pass++) {
        again = 0;
        b = 0;
        for (a = 0; a < D_db45[D_db81] && D_db45[D_db81] < 29; a++) {
            k = pass * 20;
            if (sub_s005_146d_0858(D_db81, a, 30) && sub_s005_146d_0858(D_db81, a, 0))
                f = 0;
            else
                f = 1;
            if (kind == 8 && D_db81 == 0)
                f = !sub_s005_146d_0858(D_db81, a, 0);
            if (!f)
                mask |= 1L << a;
            if (!f && sub_s004_0e76_20f6(1, k) > 1)
                continue;
            if (!f || sub_s004_0e76_20f6(1, 5) == 1)
                w = 1;
            else
                w = 0;
            cnt = 0;
            for (i = 0; i < D_db45[D_db81]; i++)
                if (i != a && sub_s005_146d_09e0(D_db81, i, a) &&
                    !sub_s005_146d_017a(D_db81, i, a) &&
                    (w || !sub_s005_146d_0858(D_db81, i, a)))
                    cnt++;
            for (tries = cnt * 2, done = 0; !done && cnt > 0 && tries > 0; tries--) {
                pick = sub_s004_0e76_20f6(1, cnt);
                for (k = i = 0; i < D_db45[D_db81] && k < pick; i++)
                    if (i != a && sub_s005_146d_09e0(D_db81, i, a) &&
                        !sub_s005_146d_017a(D_db81, i, a) &&
                        (w || !sub_s005_146d_0858(D_db81, i, a)))
                        k++;
                b = i - 1;
                if (k == pick && pick) {
                    side = sub_s005_146d_0b9c(D_db81, a, b);
                    typ = sub_s005_146d_0d7a(D_db81, a, b, side);
                    if (typ <= 5 && typ >= 1) {
                        switch (side) {
                        case 0:
                            done = sub_s006_17df_0580(b, a, 2, &y0, &y1, &x0, &x1, all);
                            break;
                        case 1:
                            done = sub_s006_17df_0580(a, b, 0, &x0, &x1, &y0, &y1, all);
                            break;
                        case 2:
                            done = sub_s006_17df_0580(a, b, 2, &y0, &y1, &x0, &x1, all);
                            break;
                        case 3:
                            done = sub_s006_17df_0580(b, a, 0, &x0, &x1, &y0, &y1, all);
                            break;
                        case 4:
                            done = sub_s006_17df_09b2(a, b, typ, 0, 2, &x1, &x0, &y1, &y0, all);
                            break;
                        case 5:
                            done = sub_s006_17df_09b2(a, b, typ, 1, 2, &x0, &x1, &y1, &y0, all);
                            break;
                        case 6:
                            done = sub_s006_17df_09b2(a, b, typ, 1, 3, &x0, &x1, &y0, &y1, all);
                            break;
                        case 7:
                            done = sub_s006_17df_09b2(a, b, typ, 0, 3, &x1, &x0, &y0, &y1, all);
                            break;
                        default:
                            done = 0;
                            break;
                        }
                        if (done)
                            continue;
                    }
                }
                sub_s005_146d_0b42(a, b);
                links[a] |= 1L << b;
                cnt--;
            }
            if (!done)
                continue;
            if (f)
                again = 1;
            sub_s006_1a57_000c(7, x0, x1, y0, y1, side);
            sub_s006_1abf_000a(D_db45[D_db81] - 1, 7, a, b);
        }
        if (!again && !all) {
            for (a = 0, m = 1L; a < D_db45[D_db81] && !all; a++, m <<= 1)
                if (!(mask & m))
                    all = 1;
            if (all) {
                pass = 3;
                again = 1;
                for (i = 0; i < 30; i++)
                    for (pick = 0, m = 1L; pick < 30; pick++, m <<= 1)
                        if (links[i] & m)
                            sub_s005_146d_0ae8(i, pick);
            }
        }
    }
}

/* straight corridor between rooms a and b side by side (dir 0: along x, 2: along y) */
int far sub_s006_17df_0580(int a, int b, int dir, int *u0, int *u1, int *v0, int *v1, int flag)
{
    int q, o, p, vert, ka, kb;
    int lo, hi, n, t, once, ok, v;
    int la, lb, ha, hb;
    int far *ra;
    int far *rb;

    if (dir == 2) {
        q = 3;
        o = 0;
        vert = p = 1;
    } else if (dir == 0) {
        vert = 0;
        q = 1;
        o = 2;
        p = 3;
    } else
        return 0;
    ra = D_db3b + (D_db81 * 30 + a) * 4;
    rb = D_db3b + (D_db81 * 30 + b) * 4;
    ka = D_db91[D_db81 * 30 + a];
    kb = D_db91[D_db81 * 30 + b];
    *u0 = ra[q] + 1;
    *u1 = rb[dir] - 1;
    if (*u1 < *u0)
        return 0;
    lb = rb[o];
    la = ra[o];
    lo = (lb > la ? lb : la) - 1;
    if (lo < D_db3d[o])
        lo = D_db3d[o];
    hb = rb[p];
    ha = ra[p];
    hi = (hb < ha ? hb : ha) + 1;
    if (hi > D_db3d[p])
        hi = D_db3d[p];
    n = hi - lo + 1;
    if (n < 2 && *u0 < *u1)
        return 0;
    t = 0;
    once = 0;
    ok = 0;
    for (; t < n * 3 && !ok; t++) {
        *u0 = ra[q] + 1;
        *u1 = rb[dir] - 1;
        if (n > 2 || once || *u1 != *u0 || sub_s004_0e76_20f6(1, 2) < 2) {
            if (n * 2 - t <= 0)
                v = lo - (n * 2 - t);
            else
                v = sub_s004_0e76_20f6(1, n) + lo - 1;
            if (v > hi)
                v = hi;
            if (v < lo)
                v = lo;
            *v0 = *v1 = v;
            if (v < la) {
                if (ka == 5)
                    (*u0)--;
                else
                    *u0 = sub_s006_17df_0d16(vert, *u0 - 1, ra[dir], la);
            }
            if (*v0 > ha) {
                if (ka == 5)
                    (*u0)--;
                else
                    *u0 = sub_s006_17df_0d16(vert, *u0 - 1, ra[dir], ha);
            }
            if (*v0 < lb) {
                if (kb == 5)
                    (*u1)++;
                else
                    *u1 = sub_s006_17df_0d16(vert, *u1 + 1, rb[q], lb);
            }
            if (*v0 > hb) {
                if (kb == 5)
                    (*u1)++;
                else
                    *u1 = sub_s006_17df_0d16(vert, *u1 + 1, rb[q], hb);
            }
        } else {
            *v1 = lo + 1;
            *v0 = hi - 1;
            if (*v0 < la) {
                if (kb != 5)
                    *v0 = sub_s006_17df_0d16(!vert, *v0, lb, rb[dir]);
                if (ka != 5)
                    *v1 = sub_s006_17df_0d16(!vert, *v1, ha, ra[q]);
            } else {
                if (ka != 5)
                    *v0 = sub_s006_17df_0d16(!vert, *v0, la, ra[q]);
                if (kb != 5)
                    *v1 = sub_s006_17df_0d16(!vert, *v1, hb, rb[dir]);
            }
            once = 1;
        }
        if (!vert)
            ok = sub_s006_17df_0ec4(*u0, *u1, *v0, *v1, a, b, flag);
        else
            ok = sub_s006_17df_0ec4(*v0, *v1, *u0, *u1, a, b, flag);
    }
    if (ok)
        return 1;
    return 0;
}

/* L-shaped corridor from room a (side h, 0/1) to room b (side v, 2/3) */
int far sub_s006_17df_09b2(int a, int b, int typ, int h, int v, int *p6, int *p7, int *p8,
                           int *p9, int flag)
{
    int oh, dh, ov, dv, alt, t, ok, ka, kb;
    int far *ra;
    int far *rb;

    if (h == 0) {
        oh = 1;
        dh = -1;
    } else if (h == 1) {
        oh = 0;
        dh = h;
    } else
        return 0;
    if (v == 2) {
        ov = 3;
        dv = -1;
    } else if (v == 3) {
        ov = 2;
        dv = 1;
    } else
        return 0;
    alt = sub_s004_0e76_20f6(1, 2) == 1;
    ra = D_db3b + (D_db81 * 30 + a) * 4;
    rb = D_db3b + (D_db81 * 30 + b) * 4;
    ka = D_db91[D_db81 * 30 + a];
    kb = D_db91[D_db81 * 30 + b];
    t = 0;
    ok = 0;
    do {
        if (t > 3 || (t > 1 && typ != 1))
            break;
        if (alt && t < 2) {
            *p6 = *p7 = ra[h] + dh;
            if (ka == 5)
                *p8 = ra[v];
            else
                *p8 = sub_s006_17df_0d16(alt, ra[v], ra[ov], ra[h]);
            if (typ == 1)
                *p9 = ra[v];
            else if (kb == 5)
                *p9 = rb[ov];
            else
                *p9 = sub_s006_17df_0d16(alt, rb[ov], rb[v], rb[oh]);
        } else if (alt && t > 1) {
            *p6 = *p7 = ra[h];
            *p8 = ra[v] + dv;
            if (kb == 5)
                *p9 = rb[ov];
            else
                *p9 = sub_s006_17df_0d16(alt, rb[ov], rb[v], rb[oh]);
        } else if (!alt && t < 2) {
            *p8 = *p9 = ra[v] + dv;
            if (ka == 5)
                *p6 = ra[h];
            else
                *p6 = sub_s006_17df_0d16(alt, ra[h], ra[oh], ra[v]);
            if (typ == 1)
                *p7 = ra[h];
            else if (kb == 5)
                *p7 = rb[oh];
            else
                *p7 = sub_s006_17df_0d16(alt, rb[oh], rb[h], rb[ov]);
        } else if (!alt && t > 1) {
            *p8 = *p9 = ra[v];
            *p6 = ra[h] + dh;
            if (kb == 5)
                *p7 = rb[oh];
            else
                *p7 = sub_s006_17df_0d16(alt, rb[oh], rb[h], rb[ov]);
        }
        ok = sub_s006_17df_0ec4(*p7 < *p6 ? *p7 : *p6, *p7 < *p6 ? *p6 : *p7,
                                *p9 < *p8 ? *p9 : *p8, *p9 < *p8 ? *p8 : *p9, a, b, flag);
        if (!ok)
            alt = !alt;
        t++;
    } while (!ok);
    if (!ok)
        return 0;
    return 1;
}

/* walk from `from` towards `to` along row/column `fixed` to the first free floor cell */
int far sub_s006_17df_0d16(int vert, int from, int to, int fixed)
{
    int up, i;
    int far *c;
    int far *u;

    up = from < to;
    c = D_db8f + D_db81 * 1600;
    u = D_db93 + D_db81 * 1600;
    if (!vert) {
        if (up) {
            for (i = from; i <= to; i++)
                if (c[i * 40 + fixed] > 1 && c[i * 40 + fixed] < 9 && u[i * 40 + fixed] == 0)
                    break;
        } else {
            for (i = from; i >= to; i--)
                if (c[i * 40 + fixed] > 1 && c[i * 40 + fixed] < 9 && u[i * 40 + fixed] == 0)
                    break;
        }
    } else {
        if (up) {
            for (i = from; i <= to; i++)
                if (c[fixed * 40 + i] > 1 && c[fixed * 40 + i] < 9 && u[fixed * 40 + i] == 0)
                    break;
        } else {
            for (i = from; i >= to; i--)
                if (c[fixed * 40 + i] > 1 && c[fixed * 40 + i] < 9 && u[fixed * 40 + i] == 0)
                    break;
        }
    }
    if ((up && i > to) || (!up && i < to))
        i = from;
    return i;
}

/* can a corridor occupy x0..x1, y0..y1 between rooms ra and rb? */
int far sub_s006_17df_0ec4(int x0, int x1, int y0, int y1, int ra, int rb, int flag)
{
    unsigned bad;
    int h, w, n, k, c;

    bad = 0;
    if (x0 <= x1 && y0 <= y1 && sub_s005_1551_00bc(D_db81, x0, x1, y0, y1) < 0) {
        if (!flag) {
            h = y1 - y0 + 1;
            w = x1 - x0 + 1;
            n = h < w ? w : h;
            if (n == w) {
                for (k = 0; k < n; k++) {
                    c = D_db8f[(D_db81 * 40 + x0 + k) * 40 + y0 - 1];
                    if (c == 0 || c == 1)
                        break;
                }
                if (k >= n) {
                    if (n > 1)
                        return 0;
                    bad = 1;
                }
                for (k = 0; k < n; k++) {
                    c = D_db8f[(D_db81 * 40 + x0 + k) * 40 + y1 + 1];
                    if (c == 0 || c == 1)
                        break;
                }
                if (k >= n) {
                    if (n > 1)
                        return 0;
                    bad++;
                }
            }
            if (h == n) {
                for (k = 0; k < n; k++) {
                    c = D_db8f[(D_db81 * 40 + x0 - 1) * 40 + y0 + k];
                    if (c == 0 || c == 1)
                        break;
                }
                if (k >= n) {
                    if (n > 1)
                        return 0;
                    bad++;
                }
                for (k = 0; k < n; k++) {
                    c = D_db8f[(D_db81 * 40 + x1 + 1) * 40 + y0 + k];
                    if (c == 0 || c == 1)
                        break;
                }
                if (k >= n) {
                    if (n > 1)
                        return 0;
                    bad++;
                }
            }
            if (bad > 2)
                return 0;
        }
        if ((flag || (sub_s005_1551_0006(0, 0, D_db81, x0, x1, y1) < 0 &&
                      sub_s005_1551_0006(3, 0, D_db81, y0, y1, x1) < 0)) &&
            sub_s006_17df_1112(x0, x1, y0, y1, ra, flag) &&
            sub_s006_17df_1112(x0, x1, y0, y1, rb, flag))
            return 1;
    }
    return 0;
}

/* open the wall of room `room` where the corridor x0..x1, y0..y1 touches it */
int far sub_s006_17df_1112(int x0, int x1, int y0, int y1, int room, int flag)
{
    int dir, lo, hi, door, ok, kind;
    int far *r;

    kind = D_db91[D_db81 * 30 + room];
    r = D_db3b + (D_db81 * 30 + room) * 4;
    ok = sub_s005_146d_0256(x0, x1, y0, y1, r[0], r[1], r[2], r[3], &dir, &lo, &hi, 0);
    if (ok) {
        if (!flag && kind != 5 && kind != 7)
            door = 1;
        else
            door = 0;
        for (ok = 0; lo <= hi; lo++) {
            ok = sub_s006_1abf_076e(room, dir ^ 2, lo, door, 0);
            if (ok)
                break;
        }
    }
    return ok;
}
