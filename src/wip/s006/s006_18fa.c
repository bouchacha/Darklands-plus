/* place: s006 18fa:0008 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): all 5 functions written from the asm + Ghidra (dec16 drafts were
 * stubs). Stairs: 0008 continues the stairs of the neighbouring level (D_db95 exits with
 * x8 = 1 up / 2 down), building a stair room (1a57 000c kind 4/5) where needed; 096e places
 * new stairs in rooms of kind D_db89 >= 3/7; 0fd4 records one stair exit (cell 9 up / 10 down,
 * D_db93 7/10 + 11 in front); 11f4 checks a stair cell; 146a checks the 3x3 neighbourhood
 * against the pattern table D_13d0.
 */

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes */

extern _segment D_db39;
extern int _based(D_db39) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern int _based(D_db39) *D_db65;                  /* slots as int[5][16][5] */
extern int _based(D_db39) *D_db89;
extern unsigned char _based(D_db39) *D_db8d;
extern int _based(D_db39) *D_db8f;
extern int _based(D_db39) *D_db91;
extern int _based(D_db39) *D_db93;
extern struct gexit _based(D_db39) *D_db95;
extern int D_db45[5];
extern int D_db4f[5];
extern int D_db59[5];
extern int D_db67[5];
extern int D_db71[5];
extern int D_db7d;
extern int D_db7f;
extern int D_db81;
extern int D_db83;
extern char D_db87;
extern char D_db88;
extern unsigned D_13d0[16];

int far sub_s004_0e76_20f6(int, int);
int far sub_s004_0e76_213c(int, int, int, int);
void far sub_s005_146d_08be(int, int);
void far sub_s005_146d_0c02(int, int, int, int *, int *);
int far sub_s005_1551_0006(int, int, int, int, int, int);
int far sub_s005_1551_00bc(int, int, int, int, int);
int far sub_s005_1551_04da(int, int, int);
void far sub_s006_1a57_000c(int, int, int, int, int, int);
void far sub_s006_1a57_042e(int, int);
void far sub_s006_18fa_0fd4(int, int, int, int, int, int);
int far sub_s006_18fa_11f4(int, int, int, int);
int far sub_s006_18fa_146a(int, int, int);

/* continue the stairs of the neighbouring level on this one */
void far sub_s006_18fa_0008(void)
{
    int i, slot, len, gap, ol, up, j, kind, va, off, hit, ok, ty, x0, y0, k, vb, x1, dir, y1, w;
    int far *r;
    int far *sl;
    struct gexit far *e;
    struct gexit far *e2;

    ok = 1;
    if (D_db81 == 1)
        return;
    if (D_db81 > 1) {
        ol = D_db81 - 1;
        ty = 1;
        up = 0;
    } else {
        ol = D_db81 + 1;
        ty = 2;
        up = 1;
    }
    for (j = 0; j < D_db71[ol]; j++) {
        e = D_db95 + ol * 4 + j;
        if (e->x8 != ty || (e->x0a != 0 && e->x0a != 3))
            continue;
        if (D_db4f[ol] >= e->x4) {
            i = ok = 0;
            while (!ok && i < D_db45[D_db81]) {
                r = D_db3b + (i + D_db81 * 30) * 4;
                if (r[0] <= e->x0 && r[1] >= e->x0 && r[2] <= e->x2 && r[3] >= e->x2)
                    ok = 1;
                else
                    ok = 0;
                if (!ok)
                    i++;
            }
            if (!ok)
                continue;
        } else {
            sub_s005_146d_0c02(ol, 0, e->x4, &dir, &gap);
            r = D_db3b + ol * 120;
            switch (dir) {
            case 0:
                gap = r[2] - e->x2;
                if (gap >= 4 || (gap >= 3 && e->x0a == 3))
                    vb = 1;
                else
                    vb = 0;
                va = 1;
                break;
            case 1:
                gap = e->x0 - r[1];
                vb = gap >= 2 ? 1 : 0;
                va = !e->x0a;
                break;
            case 2:
                gap = e->x2 - r[3];
                vb = gap >= 2 ? 1 : 0;
                va = e->x0a == 3 ? 1 : 0;
                break;
            case 3:
                gap = r[0] - e->x0;
                if (gap >= 4 || (gap >= 3 && e->x0a == 0))
                    vb = 1;
                else
                    vb = 0;
                va = 1;
                break;
            default:
                gap = -10;
                vb = va = 0;
                break;
            }
            if (!va && !vb)
                ok = 0;
            else if (va && vb) {
                if (D_db81 < 1 || sub_s004_0e76_20f6(1, 2) > 1)
                    va = 0;
                else
                    vb = 0;
            }
            x1 = y1 = -1;
            y0 = x0 = 40;
            len = w = off = kind = 0;
            if (ok && va) {
                len = sub_s004_0e76_20f6(off = 1, 2) + 1;
                w = sub_s004_0e76_20f6(1, kind = 4);
                w = w >= gap ? w : gap;
            } else if (ok && vb) {
                kind = 5;
                len = sub_s004_0e76_20f6(1, 4) + 1;
                w = sub_s004_0e76_20f6(1, off = 2) + 1;
                w = w >= gap - 1 ? w : gap - 1;
            }
            if (ok)
                switch (dir) {
                case 0:
                    y1 = r[2] - off;
                    if (e->x0a == 0) {
                        y0 = e->x2;
                        x0 = e->x0 - sub_s004_0e76_20f6(1, len - 1) + 1;
                        x0 = x0 >= r[0] ? x0 : r[0];
                    } else {
                        x0 = e->x0;
                        y0 = y1 - w + 1;
                    }
                    x1 = x0 + len - 1;
                    break;
                case 1:
                    x0 = r[1] + off;
                    if (e->x0a == 3) {
                        x1 = x0 + (w >= 3 ? w : 3) - 1;
                        y0 = e->x2 - sub_s004_0e76_20f6(1, len - 1) + 1;
                        y0 = y0 >= r[2] ? y0 : r[2];
                    } else {
                        y0 = e->x2;
                        x1 = x0 + w - 1;
                        x1 = x1 >= e->x0 + 1 ? x1 : e->x0 + 1;
                    }
                    y1 = y0 + len - 1;
                    break;
                case 2:
                    y0 = r[3] + off;
                    if (e->x0a == 0) {
                        y1 = y0 + (w >= 3 ? w : 3) - 1;
                        x0 = e->x0 - sub_s004_0e76_20f6(1, len - 1) + 1;
                        x0 = x0 >= r[0] ? x0 : r[0];
                    } else {
                        x0 = e->x0;
                        y1 = e->x2 + 1 >= y0 + w - 1 ? e->x2 + 1 : y0 + w - 1;
                    }
                    x1 = x0 + len - 1;
                    break;
                case 3:
                    x1 = r[0] - off;
                    if (e->x0a == 3) {
                        x0 = e->x0;
                        y0 = e->x2 - sub_s004_0e76_20f6(1, len - 1) + 1;
                        y0 = y0 >= r[2] ? y0 : r[2];
                    } else {
                        y0 = e->x2;
                        x0 = x1 - w + 1;
                    }
                    y1 = y0 + len - 1;
                    break;
                }
            if (ok && (x1 < x0 || y1 < y0))
                ok = 0;
            if (ok)
                for (k = j + 1, hit = 0; k < D_db71[ol]; k++) {
                    e2 = D_db95 + ol * 4 + k;
                    if (e2->x8 != ty || e2->x4 <= D_db4f[ol])
                        continue;
                    hit = 1;
                    if (e2->x2 >= y0 && e2->x2 <= y1) {
                        if (e2->x0 > e->x0)
                            x1 = e2->x0 - 2 <= x1 ? e2->x0 - 2 : x1;
                        else if (e2->x0 < e->x0)
                            x0 = (e2->x0a == 3 ? e2->x0 + 4 : e2->x0 + 3) >= x0 ?
                                 (e2->x0a == 3 ? e2->x0 + 4 : e2->x0 + 3) : x0;
                    }
                    if (e2->x0 >= x0 && e2->x0 <= x1) {
                        if (e2->x2 > e->x2)
                            y1 = e2->x2 - 2 <= y1 ? e2->x2 - 2 : y1;
                        else if (e2->x2 < e->x2)
                            y0 = (e2->x0a == 0 ? e2->x2 + 4 : e2->x2 + 3) >= y0 ?
                                 (e2->x0a == 0 ? e2->x2 + 4 : e2->x2 + 3) : y0;
                    }
                }
            hit = 0;
            if (ok)
                for (slot = 0; !hit && slot < D_db67[D_db81];) {
                    sl = D_db65 + (slot + D_db81 * 16) * 5;
                    if (sl[0] <= e->x0 && sl[1] >= e->x0 && sl[2] <= e->x2 && sl[3] >= e->x2)
                        hit = 1;
                    else
                        hit = 0;
                    if (!hit)
                        slot++;
                }
            if (ok && hit) {
                switch (dir) {
                case 0:
                case 2:
                    x0 = sl[0] + 1 >= x0 ? sl[0] + 1 : x0;
                    x1 = sl[1] - 1 <= x1 ? sl[1] - 1 : x1;
                    y0 = sl[2] >= y0 ? sl[2] : y0;
                    y1 = sl[3] <= y1 ? sl[3] : y1;
                    break;
                case 1:
                case 3:
                    x0 = sl[0] >= x0 ? sl[0] : x0;
                    x1 = sl[1] <= x1 ? sl[1] : x1;
                    y0 = sl[2] + 1 >= y0 ? sl[2] + 1 : y0;
                    y1 = sl[3] - 1 <= y1 ? sl[3] - 1 : y1;
                    break;
                }
            } else if (ok)
                slot = -1;
            if (ok && e->x0a == 3 && (!va || dir != 3))
                x1 = x0 + 2 >= x1 ? x0 + 2 : x1;
            else if (ok && e->x0a == 0 && (!va || dir != 0))
                y1 = y0 + 2 >= y1 ? y0 + 2 : y1;
            if (ok && (e->x0 < x0 || e->x0 > x1 || e->x2 < y0 || e->x2 > y1))
                ok = 0;
            if (ok && sub_s005_1551_00bc(D_db81, x0, x1, y0, y1) >= 0)
                ok = 0;
            if (ok) {
                k = sub_s005_1551_0006(0, ty, ol, x0, x1, y1);
                if (k < 0)
                    k = sub_s005_1551_0006(3, ty, ol, y0, y1, x1);
                if (k >= 0)
                    ok = 0;
            }
            if (!ok) {
                for (k = 0; !ok && k < D_db71[ol]; k++) {
                    e2 = D_db95 + ol * 4 + k;
                    if (e2->x0 == e->x0 && e2->x2 == e->x2)
                        ok = 1;
                    else
                        ok = 0;
                }
                if (ok) {
                    i = e->x4;
                    r = D_db3b + (i + ol * 30) * 4;
                    x0 = r[0];
                    x1 = r[1];
                    y0 = r[2];
                    y1 = r[3];
                    kind = D_db91[i + ol * 30];
                }
            }
            sub_s006_1a57_000c(kind, x0, x1, y0, y1, dir);
            sub_s006_1a57_042e(slot, i = D_db45[D_db81] - 1);
        }
        sub_s006_18fa_0fd4(e->x0, e->x2, up, i, e->x0a, j);
    }
}

/* new stairs up/down from rooms of the current level */
void far sub_s006_18fa_096e(int kind)
{
    int nup, ndown, up, down, n7, i, first, wu, wd, t, isup, r, found, min, k;
    int x, y, w, h, alt, ok, j, side;
    int far *rr;
    int far *c;
    int far *u;

    nup = 0;
    ndown = 0;
    if (!kind)
        return;
    if (D_db81 < 1 || D_db81 >= D_db83)
        up = 0;
    else
        up = sub_s004_0e76_20f6(1, 2);
    if (D_db81 == 1)
        down = sub_s004_0e76_20f6(1, 2);
    else
        down = 0;
    n7 = 0;
    for (i = 0; i < D_db45[D_db81]; i++)
        if (D_db89[D_db81 * 30 + i] == 7)
            n7++;
    if (n7 < 2) {
        if (up > 1)
            up = 1;
        if (down > 1)
            down = 1;
    } else if (n7 == 2 && down + up == 4) {
        if (sub_s004_0e76_20f6(1, 2) < 2)
            down--;
        else
            up--;
    }
    if (D_db81 == 1 && (n7 < 2 || down + up == 4 || (n7 == 2 && down + up == 3) ||
                        sub_s004_0e76_20f6(1, 5) == 1))
        first = 1;
    else
        first = 0;
    wu = up;
    wd = down;
    for (t = 0; wd + wu > 0 && t < 8; t++) {
        if (first || wd <= wu)
            isup = 1;
        else
            isup = 0;
        r = 0;
        found = 0;
        min = 7;
        for (i = 0; i < 40 && !found; i++) {
            if (i == 10)
                min = 3;
            if (i - 25 > D_db59[D_db81])
                break;
            if (i < 25) {
                if (D_db7f - D_db81 == 1 || sub_s004_0e76_20f6(1, 2) > 1)
                    r = sub_s004_0e76_20f6(1, D_db4f[D_db81] + 1) - 1;
                else if (kind == 8 && i == 0 && (first || !isup))
                    r = 0;
                else
                    r = sub_s004_0e76_20f6(1, D_db59[D_db81] + 1) - 1;
            } else
                r = i - 25;
            k = D_db89[D_db81 * 30 + r];
            if ((k >= min && (!first || k > 6) && (!isup || k != 4) && (isup || k != 5)) ||
                n7 < 1)
                found = 1;
        }
        if (!found) {
            if (!first)
                break;
            first = 0;
            continue;
        }
        rr = D_db3b + (D_db81 * 30 + r) * 4;
        x = rr[0];
        y = rr[2];
        w = rr[1] - x;
        h = rr[3] - y;
        c = D_db8f + D_db81 * 1600;
        u = D_db93 + D_db81 * 1600;
        if (D_db91[D_db81 * 30 + r] == 4) {
            if (c[(x + w + 1) * 40 + y] != 0)
                w++;
            if (c[x * 40 + h + y + 1] != 0)
                h++;
        }
        while (u[(w + x) * 40 + y] != 0 && w > 0)
            w--;
        while (u[x * 40 + h + y] != 0 && h > 0)
            h--;
        if (first) {
            w--;
            h--;
        }
        if (w < 1 && h < 1) {
            D_db89[D_db81 * 30 + r] = 0;
            continue;
        }
        alt = sub_s004_0e76_20f6(1, 2) == 1;
        ok = 0;
        for (j = 0; !ok && j < 2; j++) {
            if (alt) {
                side = 0;
                i = sub_s004_0e76_20f6(1, w) - 1;
                ok = sub_s006_18fa_11f4(i + x, y, isup, 0);
                if (!ok) {
                    i = 0;
                    while (i < w && !ok) {
                        ok = sub_s006_18fa_11f4(i + x, y, isup, 0);
                        if (!ok)
                            i++;
                    }
                }
                if (ok)
                    x += i;
            } else {
                side = 3;
                i = sub_s004_0e76_20f6(1, h) - 1;
                ok = sub_s006_18fa_11f4(x, i + y, isup, 3);
                if (!ok) {
                    i = 0;
                    while (i < h && !ok) {
                        ok = sub_s006_18fa_11f4(x, i + y, isup, 3);
                        if (!ok)
                            i++;
                    }
                }
                if (ok)
                    y += i;
            }
            if (!ok)
                alt = !alt;
        }
        if (!ok)
            D_db89[D_db81 * 30 + r] = k = first ? 6 : 1;
        else if (D_db7f - D_db81 == 1 && D_db91[D_db81 * 30 + r] != 3) {
            if (!isup)
                wd--;
            else
                wu--;
        } else if (isup && k != 4 && k != 2) {
            sub_s006_18fa_0fd4(x, y, 1, r, side, -1);
            wu--;
            nup++;
            if (first && k != 5) {
                if (side == 0 && w > 0) {
                    x++;
                    ok = sub_s006_18fa_11f4(x, y, 0, 0);
                } else if (side == 3 && h > 0) {
                    y++;
                    ok = sub_s006_18fa_11f4(x, y, 0, 3);
                } else
                    ok = 0;
                if (ok) {
                    sub_s006_18fa_0fd4(x, y, 0, r, side, -1);
                    wd--;
                    ndown++;
                }
            }
            first = 0;
        } else if (!isup && k != 5 && k != 2) {
            sub_s006_18fa_0fd4(x, y, 0, r, side, -1);
            wd--;
            ndown++;
        }
    }
    if (up > 0 && nup < 1) {
        D_db83 = D_db81;
        D_db87 = 0;
    }
    if (down > 0 && ndown < 1 && D_db81 <= D_db7d)
        D_db88 = 0;
}

/* record a stair at x, y in room `room` (up: 9, else 10), side 0/3; link = exit on the
 * neighbouring level or -1 */
void far sub_s006_18fa_0fd4(int x, int y, int up, int room, int side, int link)
{
    int far *c;
    struct gexit far *e;
    int _based(D_db39) *p;

    if (x < 0 || x > 39 || y < 0 || y > 39 || room < 0 || room >= D_db45[D_db81])
        return;
    c = D_db8f + (D_db81 * 40 + x) * 40 + y;
    e = D_db95 + D_db81 * 4 + D_db71[D_db81];
    if (!up) {
        *c = 10;
        e->x8 = 2;
    } else {
        *c = 9;
        e->x8 = 1;
    }
    e->x0 = x;
    e->x2 = y;
    e->x4 = room;
    e->x0a = side;
    e->x6 = link;
    if (link >= 0) {
        if (!up)
            D_db95[D_db81 * 4 + link - 4].x6 = D_db71[D_db81];
        else
            D_db95[D_db81 * 4 + link + 4].x6 = D_db71[D_db81];
    }
    if (side == 0) {
        D_db8d[D_db81 * 1600 + x * 40 + y] |= 10;
        D_db93[(D_db81 * 40 + x) * 40 + y + 1] = 7;
        D_db93[(D_db81 * 40 + x) * 40 + y + 2] = 11;
    } else if (side == 3) {
        D_db8d[D_db81 * 1600 + x * 40 + y] |= 5;
        D_db93[(D_db81 * 40 + x) * 40 + y + 40] = 10;
        D_db93[(D_db81 * 40 + x) * 40 + y + 80] = 11;
    } else
        return;
    D_db71[D_db81]++;
    p = D_db89 + D_db81 * 30 + room;
    switch (*p) {
    case 3:
        if (!up)
            *p = 5;
        else
            *p = 4;
        break;
    case 4:
        if (!up)
            *p = 2;
        break;
    case 5:
        if (up)
            *p = 2;
        break;
    case 6:
    case 7:
        if (!up)
            *p = 5;
        else
            *p = 4;
        break;
    }
    if ((!up && D_db81 > 1) || (up && D_db81 < 1))
        sub_s005_146d_08be(30, room);
}

/* can a stair (up or down) go at x, y facing side 0/3? */
int far sub_s006_18fa_11f4(int x, int y, int up, int side)
{
    int ok, i, o;
    int far *u;
    int far *c;
    struct gexit far *e;

    if (x < 0 || x >= 40 || y < 0 || y >= 40)
        ok = 0;
    else
        ok = 1;
    if (!ok)
        return 0;
    u = D_db93 + D_db81 * 1600;
    o = x * 40 + y;
    c = D_db8f + D_db81 * 1600;
    if (c[o] < 2 || c[o] > 8)
        ok = 0;
    else
        ok = 1;
    if (!ok)
        return 0;
    ok = u[o] == 0;
    if (!ok)
        return ok;
    if ((side == 0 && u[o + 1] == 0) || (side == 3 && u[o + 40] == 0))
        ok = 1;
    else
        ok = 0;
    if (!ok)
        return 0;
    if ((side == 3 && c[o - 40] == 0) || (side == 0 && c[o - 1] == 0))
        ok = 1;
    else
        ok = 0;
    if (!ok)
        return 0;
    if (c[o + 40] == 0 || c[o + 1] == 0)
        ok = 0;
    else
        ok = 1;
    if (!ok)
        return 0;
    if ((side == 0 && sub_s005_1551_04da(x, y, 2) > 1) ||
        (side == 3 && sub_s005_1551_04da(x, y, 1) > 1))
        ok = 1;
    else
        ok = 0;
    if (ok && sub_s006_18fa_146a(D_db81, x, y)) {
        if ((!up || c[o - 40] == 10 || c[o - 1] == 10) && (up || c[o + 40] == 9 || c[o + 1] == 9))
            ok = 0;
        else
            ok = 1;
        if (ok) {
            i = 0;
            ok = 1;
            do {
                if (i >= D_db71[D_db81])
                    break;
                e = D_db95 + D_db81 * 4 + i;
                if ((up && e->x8 == 1) || (!up && e->x8 == 2))
                    ok = sub_s004_0e76_213c(x, y, e->x0, e->x2) > 3;
                i++;
            } while (ok);
            if (ok)
                return 1;
        }
    }
    return 0;
}

/* is the 3x3 neighbourhood of x, y an allowed stair surrounding? */
int far sub_s006_18fa_146a(int l, int x, int y)
{
    int a, b, o, c;
    int far *m;

    o = x * 40 + y;
    m = D_db8f + l * 1600;
    if (m[o + 1] > 1 && m[o + 1] < 9)
        a = 0;
    else
        a = 1;
    if (m[o - 39] > 1 && m[o - 39] < 9)
        b = 0;
    else
        b = 1;
    a |= b << 1;
    if (m[o - 40] > 1 && m[o - 40] < 9)
        b = 0;
    else
        b = 1;
    a |= b << 2;
    if (m[o - 41] > 1 && m[o - 41] < 9)
        b = 0;
    else
        b = 1;
    a |= b << 3;
    if (m[o - 1] > 1 && m[o - 1] < 9)
        b = 0;
    else
        b = 1;
    if (m[o + 39] > 1 && m[o + 39] < 9)
        c = 0;
    else
        c = 1;
    b |= c << 1;
    if (m[o + 40] > 1 && m[o + 40] < 9)
        c = 0;
    else
        c = 1;
    b |= c << 2;
    if (m[o + 41] > 1 && m[o + 41] < 9)
        c = 0;
    else
        c = 1;
    return !((1 << (c << 3 | b)) & D_13d0[a]);
}
