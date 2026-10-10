/* place: s006 168e:0008 */
/* orig: size=0532 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. both functions written from Ghidra (dec16 drafts were stubs).
 * Buildings (kind 5) in the free strips around the keep, and gate houses (kind 4) on the
 * keep walls. Slots: struct gslot {x0, x1, y0, y1, side} (see s006_15af.c).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gslot { int a[4]; int b; };

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;
extern struct gslot _based(gen_work) *gen_strips;
extern int gen_rooms[5];
extern int D_db67[5];
extern int gen_level;

int far roll_dice(int, int);
int far sub_s005_1551_0006(int, int, int, int, int, int);
void far gen_add_room(int, int, int, int, int, int);
void far sub_s006_1a57_042e(int, int);

void far sub_s006_168e_0008(void)
{
    int i, side, w, h, W, H, len, e, f, c, dx, a, b;
    struct gslot far *s;

    for (i = D_db67[gen_level] - 1; i >= 0; i--) {
        s = gen_strips + gen_level * 16 + i;
        side = s->b;
        if (side < 0 || side >= 4)
            continue;
        w = s->a[1] - s->a[0];
        W = w + 1;
        h = s->a[3] - s->a[2];
        H = h + 1;
        len = (side == 0 || side == 2) ? W : H;
        if (len <= 3 || roll_dice(1, 100) > (h + 2) / 2 * ((w + 2) / 2) * 5)
            continue;
        if (side == 0 || side == 2) {
            w--;
            if (w > 5)
                w = 5;
            e = roll_dice(1, w - 1) + 1;
            f = roll_dice(1, 2) + 1;
            c = 0;
            dx = roll_dice(1, W - e - 1) - 1;
            W = dx + s->a[0] + 1;
        } else {
            h--;
            if (h > 5)
                h = 5;
            f = roll_dice(1, h - 1) + 1;
            e = roll_dice(1, 2) + 1;
            dx = 0;
            c = roll_dice(1, H - f - 1) - 1;
            H = c + s->a[2] + 1;
        }
        if (side == 0) {
            b = s->a[3] - c - 1;
            a = b - f;
            gen_add_room(5, W, W + e - 1, a + 1, b, 0);
            if (a > s->a[2])
                a = s->a[2];
            s->a[2] = a;
        } else if (side == 1) {
            a = s->a[0] + dx + 1;
            b = a + e;
            gen_add_room(5, a, b - 1, H, H + f - 1, 1);
            if (b < s->a[1])
                b = s->a[1];
            s->a[1] = b;
        } else if (side == 2) {
            a = s->a[2] + c + 1;
            b = a + f;
            gen_add_room(5, W, W + e - 1, a, b - 1, 2);
            if (b < s->a[3])
                b = s->a[3];
            s->a[3] = b;
        } else if (side == 3) {
            b = s->a[1] - dx - 1;
            a = b - e;
            gen_add_room(5, a + 1, b, H, H + f - 1, 3);
            if (a > s->a[0])
                a = s->a[0];
            s->a[0] = a;
        }
        sub_s006_1a57_042e(i, gen_rooms[gen_level] - 1);
    }
}

void far sub_s006_168e_02b2(void)
{
    int i, side, n, base, len, a, d, p0, p1, k;
    int far *o;
    struct gslot far *s;
    struct gslot far *t;

    o = (int far *)(D_db3b + gen_level * 120);
    for (i = 0; i < D_db67[gen_level]; i++) {
        s = gen_strips + gen_level * 16 + i;
        side = s->b;
        if (side == 0 || side == 2) {
            n = s->a[1] - s->a[0];
            base = s->a[0];
        } else {
            n = s->a[3] - s->a[2];
            base = s->a[2];
        }
        len = n + 1;
        if (len <= 3 || roll_dice(1, 10) > n - 1)
            continue;
        n -= 2;
        if (n > 2)
            n = 2;
        a = roll_dice(1, n);
        d = roll_dice(1, 4);
        p0 = roll_dice(1, len - (a + 1) - 1) + base;
        p1 = p0 + a;
        t = gen_strips + gen_level * 16 + side;
        if (side == 0) {
            k = o[2] - 1;
            if (sub_s005_1551_0006(0, 0, gen_level, p0, p1, k) >= 0)
                continue;
            d = k - d;
            gen_add_room(4, p0, p1, d + 1, k, 0);
            if (d > t->a[2])
                d = t->a[2];
            t->a[2] = d;
        } else if (side == 1) {
            d = o[1] + 1 + d;
            gen_add_room(4, o[1] + 1, d - 1, p0, p1, 1);
            if (d < t->a[1])
                d = t->a[1];
            t->a[1] = d;
        } else if (side == 2) {
            d = o[3] + 1 + d;
            gen_add_room(4, p0, p1, o[3] + 1, d - 1, 2);
            if (d < t->a[3])
                d = t->a[3];
            t->a[3] = d;
        } else if (side == 3) {
            k = o[0] - 1;
            if (sub_s005_1551_0006(3, 0, gen_level, p0, p1, k) >= 0)
                continue;
            d = k - d;
            gen_add_room(4, d + 1, k, p0, p1, 3);
            if (d > t->a[0])
                d = t->a[0];
            t->a[0] = d;
        }
        sub_s006_1a57_042e(i, gen_rooms[gen_level] - 1);
    }
}
