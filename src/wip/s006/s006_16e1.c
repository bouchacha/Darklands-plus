/* place: s006 16e1:000a */
/* orig: size=0fe0 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE (2/2 written), 0/2 match; stamped. Both written from Ghidra + asm
 * (dec16 drafts were stubs). 0a28: locals renamed to 6.00A hash slot order (frame/slots right),
 * tail restructured (early `*pa = *pb = pk; return;`, cross-jumped stores); ~30 left: 6.00 keeps i
 * in AX for the exit index, 6.00A threads the pick loop's `ok = 0` arm into the loop test.
 * Outer wall around the keep: 000a extends the 4 strips (D_db65 slots 0..3 of the level) along
 * the rooms built against the keep (room kind 3, side from s006 1637 04d0) and then adds up to
 * 4 wall segments (drawn with 1a57 000c kind 8) connected by 1abf 000a; 0a28 picks the place
 * of a segment on one side of the keep (b1/b2 = usable cells next to the neighbouring rooms).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes */

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern int _based(gen_work) *gen_strips;                  /* slots as int[5][16][5] */
extern int _based(gen_work) *gen_cell_type;
extern int _based(gen_work) *D_db91;
extern struct gexit _based(gen_work) *gen_exits;
extern int keep_x0, D_db3f, D_db41, D_db43;
extern int gen_rooms[5];
extern int D_db67[5];
extern int gen_exit_count[5];
extern int D_db7f;
extern int gen_level;

int far roll_dice(int, int);
int far sub_s005_146d_0858(int, int, int);
int far sub_s006_1637_04d0(int, int);
void far gen_add_room(int, int, int, int, int, int);
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
    if (gen_level != D_db7f || gen_rooms[gen_level] <= 0)
        return;
    for (i = 0; i <= 3; i++)
        adj[i] = 0;
    for (i = 4; i <= 7; i++)
        adj[i] = -1;
    s = gen_strips + gen_level * 80;
    D_db67[gen_level] = 4;
    s[17] = s[7] = s[2] = D_db41;
    s[11] = s[1] = s[6] = D_db3f;
    s[18] = s[8] = s[13] = D_db43;
    s[10] = s[0] = s[15] = keep_x0;
    s[3] = D_db3b[0x7a];
    s[5] = D_db3b[0x79];
    s[12] = D_db3b[0x7b];
    s[16] = D_db3b[0x78];
    z = 0;
    for (i = 0; i < gen_rooms[gen_level]; i++) {
        if (!sub_s005_146d_0858(gen_level, i, 30))
            freen++;
        if (D_db91[gen_level * 30 + i] != 3)
            continue;
        side = sub_s006_1637_04d0(gen_level, i);
        adj[side] = i;
        r = D_db3b + (i + gen_level * 30) * 4;
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
        if (freen <= 0 && roll_dice(1, gen_rooms[gen_level] + 1) != 1)
            return;
        if (tries >= 6)
            return;
        n = roll_dice(1, 4 - done);
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
                    if (k <= keep_x0) {
                        i = s[16] - s[15] + 1;
                        k = roll_dice(1, i - 1) + s[15];
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
                        n = roll_dice(1, i - 1) + s[5];
                    }
                    if (adj[5] < 0) {
                        s[5] = n;
                        s[6] = n;
                        s[7] = pb + 2;
                        if (adj[6] < 0)
                            s[11] = min(n, s[11]);
                    }
                    if (pb == pa) {
                        gen_add_room(8, k, n, pa, pa + 1, 0);
                    } else {
                        i = (n + k) / 2;
                        gen_add_room(8, k, min(i + 1, n - 1), pa, pa + 1, 0);
                        gen_add_room(8, max(i - 1, k + 1), n, pb, pb + 1, 0);
                    }
                    x = adj[4];
                    y = adj[5];
                    break;
                case 1:
                    k = s[7];
                    if (k <= D_db41) {
                        i = s[3] - s[2] + 1;
                        k = roll_dice(1, i - 1) + s[2];
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
                        n = roll_dice(1, i - 1) + s[12];
                    }
                    if (adj[6] < 0) {
                        s[12] = n;
                        s[13] = n;
                        s[11] = pb - 2;
                        if (adj[7] < 0)
                            s[18] = min(n, s[18]);
                    }
                    if (pb == pa) {
                        gen_add_room(8, pa - 1, pa, k, n, 1);
                    } else {
                        i = (n + k) / 2;
                        gen_add_room(8, pa - 1, pa, k, min(i + 1, n - 1), 1);
                        gen_add_room(8, pb - 1, pb, max(i - 1, k + 1), n, 1);
                    }
                    x = adj[5];
                    y = adj[6];
                    break;
                case 2:
                    k = s[10];
                    if (k <= keep_x0) {
                        i = s[16] - s[15] + 1;
                        k = roll_dice(1, i - 1) + s[15];
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
                        n = roll_dice(1, i - 1) + s[5];
                    }
                    if (adj[6] < 0) {
                        s[5] = n;
                        s[6] = n;
                        s[8] = pb - 2;
                        if (adj[5] < 0)
                            s[1] = min(n, s[1]);
                    }
                    if (pb == pa) {
                        gen_add_room(8, k, n, pa - 1, pa, 2);
                    } else {
                        i = (n + k) / 2;
                        gen_add_room(8, k, min(i + 1, n - 1), pa - 1, pa, 2);
                        gen_add_room(8, max(i - 1, k + 1), n, pb - 1, pb, 2);
                    }
                    x = adj[7];
                    y = adj[6];
                    break;
                case 3:
                    k = s[17];
                    if (k <= D_db41) {
                        i = s[3] - s[2] + 1;
                        k = roll_dice(1, i - 1) + s[2];
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
                        n = roll_dice(1, i - 1) + s[12];
                    }
                    if (adj[7] < 0) {
                        s[12] = n;
                        s[13] = n;
                        s[10] = pb + 2;
                        if (adj[6] < 0)
                            s[8] = min(n, s[8]);
                    }
                    if (pb == pa) {
                        gen_add_room(8, pa, pa + 1, k, n, 3);
                    } else {
                        i = (n + k) / 2;
                        gen_add_room(8, pa, pa + 1, k, min(i + 1, n - 1), 3);
                        gen_add_room(8, pb, pb + 1, max(i - 1, k + 1), n, 3);
                    }
                    x = adj[4];
                    y = adj[7];
                    break;
                }
                adj[side] = 1;
                if (pb == pa) {
                    sub_s006_1abf_000a(gen_rooms[gen_level] - 1, 8, x, y);
                } else {
                    sub_s006_1abf_000a(gen_rooms[gen_level] - 2, 8, x, -1);
                    sub_s006_1abf_000a(gen_rooms[gen_level] - 1, 8, -1, y);
                }
                done++;
                tries++;
                freen = 0;
                for (i = 0; i < gen_rooms[gen_level]; i++)
                    if (!sub_s005_146d_0858(gen_level, i, 30))
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
    int far *rn;
    int rma;
    int lof;
    int far *me;
    int far *so;
    int secondg;
    int b2[40];
    int ddl;
    int b1b[40];
    struct gexit far *ea;
    int x1m;
    int coli;
    int stepl;
    int i;
    int hih;
    int ok;
    int pk;
    int ppk;
    int qk;
    int firste;
    int x2c;
    int best;
    int dk;

    so = gen_strips + gen_level * 80;
    me = gen_cell_type + gen_level * 1600;
    for (i = 0; i < 40; i++) {
        b2[i] = 0;
        b1b[i] = 0;
    }
    switch (side) {
    case 0:
        firste = 4;
        secondg = 5;
        stepl = 1;
        break;
    case 1:
        firste = 5;
        secondg = 6;
        stepl = -1;
        break;
    case 2:
        firste = 7;
        secondg = 6;
        stepl = -1;
        break;
    case 3:
        firste = 4;
        secondg = 7;
        stepl = 1;
        break;
    default:
        return;
    }
    if (side == 0 || side == 2) {
        lof = 2;
        coli = 0;
        x1m = dk = hih = 3;
        ddl = x2c = 1;
    } else {
        hih = 1;
        x2c = 3;
        x1m = dk = lof = 0;
        ddl = coli = 2;
    }
    if (adj[firste] < 0) {
        for (i = so[side * 5 + lof]; i <= so[side * 5 + hih]; i++)
            b1b[i] |= 7;
    } else {
        rma = adj[firste];
        rn = D_db3b + (rma + gen_level * 30) * 4;
        for (i = rn[lof] - 1; i <= rn[hih] + 1; i++) {
            b1b[i] |= 1;
            if (sub_s006_1abf_076e(rma, ddl, i, 1, 0))
                b1b[i] |= 2;
        }
        for (i = rn[lof]; i <= rn[hih]; i++)
            if ((b1b[i] & 1) && (b1b[i + stepl] & 1) && ((b1b[i] & 2) || (b1b[i + stepl] & 2)))
                b1b[i] |= 4;
    }
    if (adj[secondg] < 0) {
        for (i = so[side * 5 + lof]; i <= so[side * 5 + hih]; i++)
            b2[i] |= 7;
    } else {
        rma = adj[secondg];
        rn = D_db3b + (rma + gen_level * 30) * 4;
        i = 0;
        ok = 1;
        do {
            if (i >= gen_exit_count[gen_level])
                break;
            ea = gen_exits + gen_level * 4 + i;
            if (ea->x4 == rma &&
                ((dk == 3 && ea->x0a == 3 && ea->x0 == rn[0]) ||
                 (dk == 0 && ea->x0a == 0 && rn[2] == ea->x2)))
                ok = 0;
            i++;
        } while (ok);
        for (i = rn[lof] - 1; i <= rn[hih] + 1; i++) {
            if (ok || (dk == 3 && me[rn[coli] * 40 + i] != 10) ||
                (dk == 0 && me[rn[coli] + i * 40] != 10))
                b2[i] |= 1;
            if (sub_s006_1abf_076e(rma, dk, i, 1, 0))
                b2[i] |= 2;
        }
        for (i = rn[lof]; i <= rn[hih]; i++)
            if ((b2[i] & 1) && (b2[i + stepl] & 1) && ((b2[i] & 2) || (b2[i + stepl] & 2)))
                b2[i] |= 4;
    }
    best = so[side * 5 + hih] - so[side * 5 + lof] + 1;
    ok = i = 0;
    pk = lof;
    goto pt;
pb:
    if (i < 8)
        goto pr;
    pk = so[side * 5 + lof] + i - 8;
    goto pj;
pr:
    pk = roll_dice(1, best) + so[side * 5 + lof] - 1;
pj:
    if ((b1b[pk] & 4) && (b2[pk] & 4))
        ok = 1;
    else
        ok = 0;
    i++;
pt:
    if (!ok && i < 25 && i - 8 < best)
        goto pb;
    if (ok) {
        *pa = *pb = pk;
        return;
    }
    ppk = qk = -1;
    ok = i = 0;
    best = 10;
    for (; !ok && i < 40; i++)
        if (b1b[i] & 4)
            for (pk = i - 2; !ok && pk <= i + 2; pk++)
                if ((b2[pk] & 4) && abs(i - pk) < best) {
                    best = abs(i - pk);
                    ppk = i;
                    qk = pk;
                    ok = 1;
                }
    if (best < 10) {
        *pa = ppk;
        *pb = qk;
    } else
        *pa = *pb = -1;
}
