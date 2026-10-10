/* place: s006 15af:0000 */
/* orig: size=0886 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from Ghidra + asm (dec16 drafts were
 * stubs). Keep level setup: room 0 of a level is the keep (outer rectangle D_db3d..D_db43),
 * D_db65 slots = the 4 strips around it {x0, x1, y0, y1, side}; D_db25 = gate side.
 * Work segment layout: see s005_1432.c.
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gslot { int a[4]; int b; };                  /* {x0, x1, y0, y1, side}, 16 per level */

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;                  /* rooms {x0, x1, y0, y1}, 30 per level */
extern struct gslot _based(gen_work) *gen_strips;
extern int _based(gen_work) *D_db89;
extern int _based(gen_work) *gen_cell_type;
extern int _based(gen_work) *D_db91;
extern int _based(gen_work) *gen_cell_door;
extern int gate_side;
extern int keep_x0, D_db3f, D_db41, D_db43;
extern int gen_rooms[5];
extern int D_db4f[5];
extern int D_db67[5];
extern int gen_level;

int far roll_dice(int, int);
void far gen_add_room(int, int, int, int, int, int);
void far sub_s006_1a57_02a8(int, int, int);
void far sub_s006_1a57_042e(int, int);

/* the keep: room 0, and the 4 strips around it */
void far sub_s006_15af_0000(void)
{
    int w, h, x0, x1, y0, y1, i;
    struct gslot _based(gen_work) *s;
    int _based(gen_work) *o;

    if (gen_level == 1) {
        w = roll_dice(1, 4) + 4;
        h = w;
        if (roll_dice(1, 2) > 1) {
            if (roll_dice(1, 2) < 2)
                h = w + roll_dice(1, 3) - 1;
            else
                w = w + roll_dice(1, 3) - 1;
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
    gen_add_room(2, x0, x1, y0, y1, -1);
    D_db67[gen_level] = 4;
    s = gen_strips + gen_level * 16;
    o = D_db3b + gen_level * 120;
    s[0].a[3] = o[2] - 1;
    s[0].a[2] = D_db41 - 1;
    s[2].a[2] = o[3] + 1;
    s[2].a[3] = D_db43 + 1;
    s[2].a[0] = s[0].a[0] = o[0] - 1;
    s[2].a[1] = s[0].a[1] = o[1] + 1;
    s[3].a[1] = o[0] - 1;
    s[3].a[0] = keep_x0 - 1;
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
    int _based(gen_work) *o;
    struct gslot _based(gen_work) *s;

    if (gen_level != 1)
        return;
    if (a == 0 || roll_dice(1, 2) > 1)
        i = 0;
    else
        i = roll_dice(1, D_db4f[1] + 1) - 1;
    D_db89[30 + i] = 3;
    if (i == 0) {
        o = D_db3b + 30 * 4;
        do {
            do {
                gate_side = roll_dice(1, 4) - 1;
                s = gen_strips + 16 + gate_side;
                if (gate_side == 0 || gate_side == 2) {
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
                mid -= roll_dice(1, 2) - 1;
        } while (mid <= lo || hi <= mid + 1);
        if (gate_side == 0 || gate_side == 2)
            gen_add_room(6, mid, mid + 1, s->a[2], s->a[3], gate_side);
        else
            gen_add_room(6, s->a[0], s->a[1], mid, mid + 1, gate_side);
        sub_s006_1a57_042e(gate_side, gen_rooms[1] - 1);
        return;
    }
    o = D_db3b + (30 + i) * 4;
    if (roll_dice(1, 2) < 2) {
        k = roll_dice(1, o[3] - o[2]) - 1 + o[2];
        if (o[0] < D_db3b[120]) {
            gate_side = 3;
            gen_add_room(6, keep_x0 - 1, o[0] - 1, k, k + 1, 3);
        } else {
            gate_side = 1;
            gen_add_room(6, o[1] + 1, D_db3f + 1, k, k + 1, 1);
        }
    } else {
        k = roll_dice(1, o[1] - o[0]) - 1 + o[0];
        if (o[2] < D_db3b[122]) {
            gate_side = 0;
            gen_add_room(6, k, k + 1, D_db41 - 1, o[2] - 1, 0);
        } else {
            gate_side = 2;
            gen_add_room(6, k, k + 1, o[3] + 1, D_db43 + 1, 2);
        }
    }
}

/* widen the outer rectangle around the gate room (kind 6) */
void far sub_s006_15af_0470(void)
{
    int one, f, e, x, y;
    int _based(gen_work) *o;
    int (_based(gen_work) *w)[40];
    int (_based(gen_work) *t)[40];

    one = gen_level == 1;
    f = 0;
    if (D_db91[30 + D_db4f[1] + 1] != 6)
        return;
    o = D_db3b + (30 + D_db4f[1] + 1) * 4;
    w = (int (_based(gen_work) *)[40])(gen_cell_door + 1600);
    if (gate_side == 0) {
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
    } else if (gate_side == 1) {
        e = o[1];
        if (one) {
            f = e <= D_db3f;
            if (f)
                o[1] = D_db3f + 1;
        } else {
            f = !(D_db3f < e && keep_x0 < D_db3f && w[e][o[2]] == 0 && w[e][o[2] + 1] == 0);
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
    } else if (gate_side == 2) {
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
    } else if (gate_side == 3) {
        e = o[0];
        if (one) {
            f = keep_x0 <= e;
            if (f)
                o[0] = keep_x0 - 1;
        } else {
            f = !(e < keep_x0 && keep_x0 < D_db3f && w[e][o[2]] == 0 && w[e][o[2] + 1] == 0);
            if (f) {
                e = o[0] - 1;
                if (e > keep_x0 - 1)
                    e = keep_x0 - 1;
                keep_x0 = o[0] = e;
            } else
                keep_x0 = o[0];
            w[e][o[2]] = 1;
            w[e][o[2] + 1] = 1;
        }
    }
    if (!f)
        return;
    t = (int (_based(gen_work) *)[40])(gen_cell_type + 1600);
    for (x = o[0]; x <= o[1]; x++)
        for (y = o[2]; y <= o[3]; y++)
            t[x][y] = 6;
    for (x = o[0]; x <= o[1]; x++)
        for (y = o[2]; y <= o[3]; y++)
            sub_s006_1a57_02a8(1, x, y);
}
