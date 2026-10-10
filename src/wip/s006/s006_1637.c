/* place: s006 1637:0006 */
/* orig: size=0572 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from Ghidra (dec16 drafts were stubs).
 * Corner towers of the keep (rooms of kind 3 at the corners, 4..7 = corner code from 04d0).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct gexit { int x0, x2, x4, x6, x8, x0a; };

extern _segment gen_work;
extern int _based(gen_work) *D_db3b;
extern int _based(gen_work) *gen_strips;                  /* slots as int[5][16][5] */
extern int _based(gen_work) *D_db91;
extern struct gexit _based(gen_work) *gen_exits;
extern int gen_rooms[5];
extern int D_db4f[5];
extern int gen_exit_count[5];
extern int gen_level;

int far roll_dice(int, int);
int far sub_s005_146d_0b9c(int, int, int);
void far gen_add_room(int, int, int, int, int, int);
void far sub_s006_1abf_000a(int, int, int, int);
void far sub_s006_1637_0366(int);
int far sub_s006_1637_04d0(int, int);

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* towers: copied from the level above (level 0), random ones (level 1), or below */
#pragma optimize("a", on)
void far sub_s006_1637_0006(int t)
{
    int i, r, n, a, b, c, d, k, x0, x1, y0, y1;
    unsigned used, bit;
    struct gexit far *e;
    int far *o;

    if (gen_level > 1) {
        for (i = 1; i <= D_db4f[gen_level - 1]; i++) {
            o = (int far *)(D_db3b + (30 + i) * 4);
            k = sub_s005_146d_0b9c(1, 0, i);
            gen_add_room(3, o[0], o[1], o[2], o[3], k);
            sub_s006_1637_0366(gen_rooms[gen_level] - 1);
            sub_s006_1abf_000a(gen_rooms[gen_level] - 1, 3, 0, 0);
        }
    } else if (gen_level < 1) {
        for (i = 0; i < gen_exit_count[gen_level + 1]; i++) {
            e = (struct gexit far *)(gen_exits + (gen_level + 1) * 4 + i);
            if (e->x8 == 2) {
                r = e->x4;
                if (D_db91[(gen_level + 1) * 30 + r] == 3) {
                    o = (int far *)(D_db3b + ((gen_level + 1) * 30 + r) * 4);
                    k = sub_s005_146d_0b9c(1, 0, r);
                    gen_add_room(3, o[0], o[1], o[2], o[3], k);
                    sub_s006_1637_0366(gen_rooms[gen_level] - 1);
                    sub_s006_1abf_000a(gen_rooms[gen_level] - 1, 3, 0, 0);
                }
            }
        }
    } else if (gen_level == 1) {
        n = roll_dice(1, 5) - 1;
        if (t == 0x27)
            n = MAX(n, 2);
        used = 0;
        o = (int far *)(D_db3b + 30 * 4);
        for (; n > 0; n--) {
            do
                { bit = roll_dice(1, 4) - 1; bit = 1 << bit; }
            while (used & bit);
            used |= bit;
            a = roll_dice(1, 2) + 2;
            b = roll_dice(1, 2) + 2;
            if (roll_dice(1, 2) > 1) {
                if (roll_dice(1, 2) > 1) {
                    d = roll_dice(1, 3) - 1;
                    c = roll_dice(1, 2) - 1;
                } else {
                    d = roll_dice(1, 2) - 1;
                    c = roll_dice(1, 3) - 1;
                }
            } else {
                c = 0;
                d = c;
            }
            switch (bit) {
            case 1:
                y1 = o[2] - 2;
                y0 = y1 - b + c + 1;
                y1 += c;
                x1 = o[0] - 2;
                x0 = x1 - a + d + 1;
                x1 += d;
                gen_add_room(3, x0, x1, y0, y1, 4);
                break;
            case 2:
                y1 = o[2] - 2;
                y0 = y1 - b + c + 1;
                y1 += c;
                x0 = o[1] - d + 2;
                x1 = x0 + a - 1;
                gen_add_room(3, x0, x1, y0, y1, 5);
                break;
            case 4:
                y0 = o[3] - c + 2;
                y1 = y0 + b - 1;
                x0 = o[1] - d + 2;
                x1 = x0 + a - 1;
                gen_add_room(3, x0, x1, y0, y1, 6);
                break;
            case 8:
                y0 = o[3] - c + 2;
                y1 = y0 + b - 1;
                x1 = o[0] - 2;
                x0 = x1 - a + d + 1;
                x1 += d;
                gen_add_room(3, x0, x1, y0, y1, 7);
                break;
            }
            sub_s006_1637_0366(gen_rooms[gen_level] - 1);
            sub_s006_1abf_000a(gen_rooms[1] - 1, 3, 0, 0);
        }
    }
}
#pragma optimize("", on)

/* shrink the free strips around the keep by tower r */
void far sub_s006_1637_0366(int r)
{
    int far *o;
    int far *s;

    o = (int far *)(D_db3b + (gen_level * 30 + r) * 4);
    s = (int far *)(gen_strips + gen_level * 80);
    switch (sub_s006_1637_04d0(gen_level, r)) {
    case 4:
        s[0] = MAX(o[1] + 1, s[0]);
        s[2] = MIN(o[2] - 1, s[2]);
        s[17] = MAX(o[3] + 1, s[17]);
        s[15] = MIN(o[0] - 1, s[15]);
        break;
    case 5:
        s[1] = MIN(s[1], o[0] - 1);
        s[2] = MIN(o[2] - 1, s[2]);
        s[7] = MAX(o[3] + 1, s[7]);
        s[6] = MAX(o[1] + 1, s[6]);
        break;
    case 6:
        s[11] = MIN(s[11], o[0] - 1);
        s[13] = MAX(o[3] + 1, s[13]);
        s[8] = MIN(o[2] - 1, s[8]);
        s[6] = MAX(o[1] + 1, s[6]);
        break;
    case 7:
        s[10] = MAX(o[1] + 1, s[10]);
        s[13] = MAX(o[3] + 1, s[13]);
        s[18] = MIN(o[2] - 1, s[18]);
        s[15] = MIN(s[15], o[0] - 1);
        break;
    }
}

/* corner of the keep (level 1 room 0) that room r is at: 4..7, 8 = none */
int far sub_s006_1637_04d0(int l, int r)
{
    int far *o;
    int far *k;

    o = (int far *)(D_db3b + (l * 30 + r) * 4);
    k = (int far *)(D_db3b + 30 * 4);
    if (o[0] < k[0] && k[2] > o[2])
        return 4;
    if (o[1] > k[1] && o[2] < k[2])
        return 5;
    if (o[0] > k[0] && o[3] > k[3])
        return 6;
    if (o[0] < k[0] && o[3] > k[3])
        return 7;
    return 8;
}
