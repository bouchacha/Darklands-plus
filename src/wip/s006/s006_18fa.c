/* place: s006 18fa:0008 */
/* orig: size=15d4 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE (5/5 written), 0/5 match; stamped. 0008: locals renamed to the
 * hash slot order (all slots right) and its j loop written with gotos in the original block order
 * (6.00 put the room-search loop in front of the j increment); ~38 left (6.00A threads the `ok = 0`
 * arm, frame +2 for a 6.00A temp).
 * All 5 functions written from the asm + Ghidra (dec16 drafts were
 * stubs). Stairs: 0008 continues the stairs of the neighbouring level (D_db95 exits with
 * x8 = 1 up / 2 down), building a stair room (1a57 000c kind 4/5) where needed; 096e places
 * new stairs in rooms of kind D_db89 >= 3/7; 0fd4 records one stair exit (cell 9 up / 10 down,
 * D_db93 7/10 + 11 in front); 11f4 checks a stair cell; 146a checks the 3x3 neighbourhood
 * against the pattern table D_13d0.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes */

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern int _based(gen_work) *gen_strips;                  /* slots as int[5][16][5] */
extern int _based(gen_work) *D_db89;
extern unsigned char _based(gen_work) *gen_cell_walls;
extern int _based(gen_work) *gen_cell_type;
extern int _based(gen_work) *D_db91;
extern int _based(gen_work) *gen_cell_door;
extern struct gexit _based(gen_work) *gen_exits;
extern int gen_rooms[5];
extern int D_db4f[5];
extern int D_db59[5];
extern int D_db67[5];
extern int gen_exit_count[5];
extern int gen_level_lo;
extern int D_db7f;
extern int gen_level;
extern int D_db83;
extern char D_db87;
extern char D_db88;
extern unsigned D_13d0[16];

int far roll_dice(int, int);
int far sub_s004_0e76_213c(int, int, int, int);
void far sub_s005_146d_08be(int, int);
void far sub_s005_146d_0c02(int, int, int, int *, int *);
int far sub_s005_1551_0006(int, int, int, int, int, int);
int far sub_s005_1551_00bc(int, int, int, int, int);
int far sub_s005_1551_04da(int, int, int);
void far gen_add_room(int, int, int, int, int, int);
void far sub_s006_1a57_042e(int, int);
void far sub_s006_18fa_0fd4(int, int, int, int, int, int);
int far sub_s006_18fa_11f4(int, int, int, int);
int far sub_s006_18fa_146a(int, int, int);

/* continue the stairs of the neighbouring level on this one */
void far sub_s006_18fa_0008(void)
{
    int slotn;
    int ig;
    int far *slb;
    int far *ro;
    int lenc;
    int olh;
    int gapk;
    int upo;
    int kindo;
    int jk;
    int vao;
    int offl;
    struct gexit far *eb;
    int hitc;
    struct gexit far *e2a;
    int oko;
    int x0b;
    int tym;
    int y0b;
    int vbd;
    int ka;
    int x1d;
    int y1d;
    int diro;
    int wh;

    oko = 1;
    if (gen_level == 1)
        return;
    if (gen_level > 1) {
        olh = gen_level - 1;
        tym = 1;
        upo = 0;
    } else {
        olh = gen_level + 1;
        tym = 2;
        upo = 1;
    }
    jk = 0;
    goto jtest;
srch:
    if (ig >= gen_rooms[gen_level])
        goto sdone;
    ro = D_db3b + (ig + gen_level * 30) * 4;
    if (ro[0] <= eb->x0 && ro[1] >= eb->x0 && ro[2] <= eb->x2 && ro[3] >= eb->x2)
        oko = 1;
    else
        oko = 0;
    if (!oko)
        ig++;
stest:
    if (!oko)
        goto srch;
sdone:
    if (!oko)
        goto jinc;
    goto call;
jinc:
    jk++;
jtest:
    if (jk < gen_exit_count[olh]) {
        eb = gen_exits + olh * 4 + jk;
        if (eb->x8 != tym || (eb->x0a != 0 && eb->x0a != 3))
            goto jinc;
        if (D_db4f[olh] >= eb->x4) {
            oko = ig = 0;
            goto stest;
        } else {
            sub_s005_146d_0c02(olh, 0, eb->x4, &diro, &gapk);
            ro = D_db3b + olh * 120;
            switch (diro) {
            case 0:
                gapk = ro[2] - eb->x2;
                if (gapk >= 4 || (gapk >= 3 && eb->x0a == 3))
                    vbd = 1;
                else
                    vbd = 0;
                vao = 1;
                break;
            case 1:
                gapk = eb->x0 - ro[1];
                vbd = gapk >= 2 ? 1 : 0;
                vao = !eb->x0a;
                break;
            case 2:
                gapk = eb->x2 - ro[3];
                vbd = gapk >= 2 ? 1 : 0;
                vao = eb->x0a == 3 ? 1 : 0;
                break;
            case 3:
                gapk = ro[0] - eb->x0;
                if (gapk >= 4 || (gapk >= 3 && eb->x0a == 0))
                    vbd = 1;
                else
                    vbd = 0;
                vao = 1;
                break;
            default:
                gapk = -10;
                vbd = vao = 0;
                break;
            }
            if (!vao && !vbd)
                oko = 0;
            else if (vao && vbd) {
                if (gen_level < 1 || roll_dice(1, 2) > 1)
                    vao = 0;
                else
                    vbd = 0;
            }
            x1d = y1d = -1;
            y0b = x0b = 40;
            lenc = wh = offl = kindo = 0;
            if (oko && vao) {
                lenc = roll_dice(offl = 1, 2) + 1;
                wh = roll_dice(1, kindo = 4);
                wh = wh >= gapk ? wh : gapk;
            } else if (oko && vbd) {
                kindo = 5;
                lenc = roll_dice(1, 4) + 1;
                wh = roll_dice(1, offl = 2) + 1;
                wh = wh >= gapk - 1 ? wh : gapk - 1;
            }
            if (oko)
                switch (diro) {
                case 0:
                    y1d = ro[2] - offl;
                    if (eb->x0a == 0) {
                        y0b = eb->x2;
                        x0b = eb->x0 - roll_dice(1, lenc - 1) + 1;
                        x0b = x0b >= ro[0] ? x0b : ro[0];
                    } else {
                        x0b = eb->x0;
                        y0b = y1d - wh + 1;
                    }
                    x1d = x0b + lenc - 1;
                    break;
                case 1:
                    x0b = ro[1] + offl;
                    if (eb->x0a == 3) {
                        x1d = x0b + (wh >= 3 ? wh : 3) - 1;
                        y0b = eb->x2 - roll_dice(1, lenc - 1) + 1;
                        y0b = y0b >= ro[2] ? y0b : ro[2];
                    } else {
                        y0b = eb->x2;
                        x1d = x0b + wh - 1;
                        x1d = x1d >= eb->x0 + 1 ? x1d : eb->x0 + 1;
                    }
                    y1d = y0b + lenc - 1;
                    break;
                case 2:
                    y0b = ro[3] + offl;
                    if (eb->x0a == 0) {
                        y1d = y0b + (wh >= 3 ? wh : 3) - 1;
                        x0b = eb->x0 - roll_dice(1, lenc - 1) + 1;
                        x0b = x0b >= ro[0] ? x0b : ro[0];
                    } else {
                        x0b = eb->x0;
                        y1d = eb->x2 + 1 >= y0b + wh - 1 ? eb->x2 + 1 : y0b + wh - 1;
                    }
                    x1d = x0b + lenc - 1;
                    break;
                case 3:
                    x1d = ro[0] - offl;
                    if (eb->x0a == 3) {
                        x0b = eb->x0;
                        y0b = eb->x2 - roll_dice(1, lenc - 1) + 1;
                        y0b = y0b >= ro[2] ? y0b : ro[2];
                    } else {
                        y0b = eb->x2;
                        x0b = x1d - wh + 1;
                    }
                    y1d = y0b + lenc - 1;
                    break;
                }
            if (oko && (x1d < x0b || y1d < y0b))
                oko = 0;
            if (oko)
                for (ka = jk + 1, hitc = 0; ka < gen_exit_count[olh]; ka++) {
                    e2a = gen_exits + olh * 4 + ka;
                    if (e2a->x8 != tym || e2a->x4 <= D_db4f[olh])
                        continue;
                    hitc = 1;
                    if (e2a->x2 >= y0b && e2a->x2 <= y1d) {
                        if (e2a->x0 > eb->x0)
                            x1d = e2a->x0 - 2 <= x1d ? e2a->x0 - 2 : x1d;
                        else if (e2a->x0 < eb->x0)
                            x0b = (e2a->x0a == 3 ? e2a->x0 + 4 : e2a->x0 + 3) >= x0b ?
                                 (e2a->x0a == 3 ? e2a->x0 + 4 : e2a->x0 + 3) : x0b;
                    }
                    if (e2a->x0 >= x0b && e2a->x0 <= x1d) {
                        if (e2a->x2 > eb->x2)
                            y1d = e2a->x2 - 2 <= y1d ? e2a->x2 - 2 : y1d;
                        else if (e2a->x2 < eb->x2)
                            y0b = (e2a->x0a == 0 ? e2a->x2 + 4 : e2a->x2 + 3) >= y0b ?
                                 (e2a->x0a == 0 ? e2a->x2 + 4 : e2a->x2 + 3) : y0b;
                    }
                }
            hitc = 0;
            if (oko)
                for (slotn = 0; !hitc && slotn < D_db67[gen_level];) {
                    slb = gen_strips + (slotn + gen_level * 16) * 5;
                    if (slb[0] <= eb->x0 && slb[1] >= eb->x0 && slb[2] <= eb->x2 && slb[3] >= eb->x2)
                        hitc = 1;
                    else
                        hitc = 0;
                    if (!hitc)
                        slotn++;
                }
            if (oko && hitc) {
                switch (diro) {
                case 0:
                case 2:
                    x0b = slb[0] + 1 >= x0b ? slb[0] + 1 : x0b;
                    x1d = slb[1] - 1 <= x1d ? slb[1] - 1 : x1d;
                    y0b = slb[2] >= y0b ? slb[2] : y0b;
                    y1d = slb[3] <= y1d ? slb[3] : y1d;
                    break;
                case 1:
                case 3:
                    x0b = slb[0] >= x0b ? slb[0] : x0b;
                    x1d = slb[1] <= x1d ? slb[1] : x1d;
                    y0b = slb[2] + 1 >= y0b ? slb[2] + 1 : y0b;
                    y1d = slb[3] - 1 <= y1d ? slb[3] - 1 : y1d;
                    break;
                }
            } else if (oko)
                slotn = -1;
            if (oko && eb->x0a == 3 && (!vao || diro != 3))
                x1d = x0b + 2 >= x1d ? x0b + 2 : x1d;
            else if (oko && eb->x0a == 0 && (!vao || diro != 0))
                y1d = y0b + 2 >= y1d ? y0b + 2 : y1d;
            if (oko && (eb->x0 < x0b || eb->x0 > x1d || eb->x2 < y0b || eb->x2 > y1d))
                oko = 0;
            if (oko && sub_s005_1551_00bc(gen_level, x0b, x1d, y0b, y1d) >= 0)
                oko = 0;
            if (oko) {
                ka = sub_s005_1551_0006(0, tym, olh, x0b, x1d, y1d);
                if (ka < 0)
                    ka = sub_s005_1551_0006(3, tym, olh, y0b, y1d, x1d);
                if (ka >= 0)
                    oko = 0;
            }
            if (!oko) {
                for (ka = 0; !oko && ka < gen_exit_count[olh]; ka++) {
                    e2a = gen_exits + olh * 4 + ka;
                    if (e2a->x0 == eb->x0 && e2a->x2 == eb->x2)
                        oko = 1;
                    else
                        oko = 0;
                }
                if (oko) {
                    ig = eb->x4;
                    ro = D_db3b + (ig + olh * 30) * 4;
                    x0b = ro[0];
                    x1d = ro[1];
                    y0b = ro[2];
                    y1d = ro[3];
                    kindo = D_db91[ig + olh * 30];
                }
            }
            gen_add_room(kindo, x0b, x1d, y0b, y1d, diro);
            sub_s006_1a57_042e(slotn, ig = gen_rooms[gen_level] - 1);
        }
    call:
        sub_s006_18fa_0fd4(eb->x0, eb->x2, upo, ig, eb->x0a, jk);
        goto jinc;
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
    if (gen_level < 1 || gen_level >= D_db83)
        up = 0;
    else
        up = roll_dice(1, 2);
    if (gen_level == 1)
        down = roll_dice(1, 2);
    else
        down = 0;
    n7 = 0;
    for (i = 0; i < gen_rooms[gen_level]; i++)
        if (D_db89[gen_level * 30 + i] == 7)
            n7++;
    if (n7 < 2) {
        if (up > 1)
            up = 1;
        if (down > 1)
            down = 1;
    } else if (n7 == 2 && down + up == 4) {
        if (roll_dice(1, 2) < 2)
            down--;
        else
            up--;
    }
    if (gen_level == 1 && (n7 < 2 || down + up == 4 || (n7 == 2 && down + up == 3) ||
                        roll_dice(1, 5) == 1))
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
            if (i - 25 > D_db59[gen_level])
                break;
            if (i < 25) {
                if (D_db7f - gen_level == 1 || roll_dice(1, 2) > 1)
                    r = roll_dice(1, D_db4f[gen_level] + 1) - 1;
                else if (kind == 8 && i == 0 && (first || !isup))
                    r = 0;
                else
                    r = roll_dice(1, D_db59[gen_level] + 1) - 1;
            } else
                r = i - 25;
            k = D_db89[gen_level * 30 + r];
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
        rr = D_db3b + (gen_level * 30 + r) * 4;
        x = rr[0];
        y = rr[2];
        w = rr[1] - x;
        h = rr[3] - y;
        c = gen_cell_type + gen_level * 1600;
        u = gen_cell_door + gen_level * 1600;
        if (D_db91[gen_level * 30 + r] == 4) {
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
            D_db89[gen_level * 30 + r] = 0;
            continue;
        }
        alt = roll_dice(1, 2) == 1;
        ok = 0;
        for (j = 0; !ok && j < 2; j++) {
            if (alt) {
                side = 0;
                i = roll_dice(1, w) - 1;
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
                i = roll_dice(1, h) - 1;
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
            D_db89[gen_level * 30 + r] = k = first ? 6 : 1;
        else if (D_db7f - gen_level == 1 && D_db91[gen_level * 30 + r] != 3) {
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
        D_db83 = gen_level;
        D_db87 = 0;
    }
    if (down > 0 && ndown < 1 && gen_level <= gen_level_lo)
        D_db88 = 0;
}

/* record a stair at x, y in room `room` (up: 9, else 10), side 0/3; link = exit on the
 * neighbouring level or -1 */
void far sub_s006_18fa_0fd4(int x, int y, int up, int room, int side, int link)
{
    int far *c;
    struct gexit far *e;
    int _based(gen_work) *p;

    if (x < 0 || x > 39 || y < 0 || y > 39 || room < 0 || room >= gen_rooms[gen_level])
        return;
    c = gen_cell_type + (gen_level * 40 + x) * 40 + y;
    e = gen_exits + gen_level * 4 + gen_exit_count[gen_level];
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
            gen_exits[gen_level * 4 + link - 4].x6 = gen_exit_count[gen_level];
        else
            gen_exits[gen_level * 4 + link + 4].x6 = gen_exit_count[gen_level];
    }
    if (side == 0) {
        gen_cell_walls[gen_level * 1600 + x * 40 + y] |= 10;
        gen_cell_door[(gen_level * 40 + x) * 40 + y + 1] = 7;
        gen_cell_door[(gen_level * 40 + x) * 40 + y + 2] = 11;
    } else if (side == 3) {
        gen_cell_walls[gen_level * 1600 + x * 40 + y] |= 5;
        gen_cell_door[(gen_level * 40 + x) * 40 + y + 40] = 10;
        gen_cell_door[(gen_level * 40 + x) * 40 + y + 80] = 11;
    } else
        return;
    gen_exit_count[gen_level]++;
    p = D_db89 + gen_level * 30 + room;
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
    if ((!up && gen_level > 1) || (up && gen_level < 1))
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
    u = gen_cell_door + gen_level * 1600;
    o = x * 40 + y;
    c = gen_cell_type + gen_level * 1600;
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
    if (ok && sub_s006_18fa_146a(gen_level, x, y)) {
        if ((!up || c[o - 40] == 10 || c[o - 1] == 10) && (up || c[o + 40] == 9 || c[o + 1] == 9))
            ok = 0;
        else
            ok = 1;
        if (ok) {
            i = 0;
            ok = 1;
            do {
                if (i >= gen_exit_count[gen_level])
                    break;
                e = gen_exits + gen_level * 4 + i;
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
    m = gen_cell_type + l * 1600;
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
