/* place: s008 1432:0000 */
/* orig: size=30dc CONST=7d30+0002 _DATA=1458+0077 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=1458 */
/* status (e10, 2026-10-09): ALL 16 FUNCTIONS WRITTEN (stamped --wip). Wilderness battlefields.
 * Match (7): 0000, 009c, 294e (enemy layout; min/max macros + pragma "a"), 2d34 (pragma "a"),
 * 2d62, 2e76 (empty field; `*(volatile int far *)&F.x248a = 0`), 3002.
 * Only jump tables differ (they wait for the earlier lengths): 04ce, 1790 (enemy kinds/counts by
 * location; hash-ordered local names), 20b2 (marker ids / enemy spread), 0926 (tile set; locals
 * renamed to the hash order, x26 moved into the bucket before; left: 6.00 keeps r*40+c as one
 * CSE for cell and hgt, 6.00A only r*40 -> 1 extra instruction).
 * Close: 0172 (6.00 compares D_df89 with SI known to be 0 = `cmp [g],si`; 6.00A `cmp si,[g]`;
 * plus the cross-jumped D_df87 = 2), 1694 (6.00 returns through DX), 28a0 (first loop ok with a
 * far pointer + pragma "a"; second loop register choice), 2adc (6.00 CSE far pointer copies of
 * g), 1388 (slots/registers; p = D_df9b[i] is a dead stored local in the original).
 * 1388: 1694 -> battlefield type, 1790 -> enemies, 20b2 -> markers, then party at marker
 * D_df8d and up to five groups (D_df8f[] marker ids, D_df9b[][3] counts) via 294e/2adc.
 * Level buffer (one, 0x33fc bytes, see s010/s011 struct level): handle D_df7d, segment D_df7f,
 * based pointer D_df81. Globals: D_df83 terrain/season code (copied from D_d843), D_df85 terrain
 * class (index into the "---","gen","mtn",.. names), D_df87/D_df89/D_df8b digits of the map number.
 */
#include "darklands/card.h"
#include "darklands/game.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
struct res { int a; int b; };
struct obj {                            /* map object, 14 bytes */
    int x0, x2;
    unsigned char x4, x5;
    int type;                           /* 06: 0x65 = group marker */
    int x8;
    int x0a;                            /* 0a: marker id */
    int x0c;
};
struct level {                          /* battle map (0x33fc bytes), see s010/s011 */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41[0x1f4c - 0x1f41];
    int x1f4c;                          /* 1f4c: party start x */
    int x1f4e;                          /* 1f4e: party start y */
    char x1f50;                         /* 1f50: facing */
    char x1f51;
    char n1f52;                         /* 1f52 */
    char p1f53[0x2184 - 0x1f53];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char x2488;                         /* 2488 */
    char p2489;
    int x248a;                          /* 248a */
    char p248c[0x24c6 - 0x248c];
    int list[(0x2fb6 - 0x24c6) / 2];    /* 24c6: (row << 8) + col, -1 terminated */
    char nexit;                         /* 2fb6 */
    char p2fb7[0x300c - 0x2fb7];
    char x300c;                         /* 300c */
    char p300d[0x3326 - 0x300d];
    char n3326;                         /* 3326: raised rectangles */
    char p3327;
    int r3328[21][5];                   /* 3328: rectangles {x, y, w, h} in pixels */
    char p33fa[2];
};
extern int D_d843;
extern int D_df83;
extern char D_dfb9;
extern unsigned D_df7d;
extern _segment D_df7f;
extern struct level _based(D_df7f) *D_df81;
extern int D_c100;
extern char D_dfba;
extern int D_df85, D_df87, D_df89, D_df8b;
extern int cur_month;
extern _segment enemy_seg;
struct bfdesc { char b[0xc0]; };
extern struct bfdesc _based(enemy_seg) *tile_desc;
extern int _based(enemy_seg) *mon_template;
extern struct level far floor_img;
int far monster_type_map();
#define B(o) (((char _based(enemy_seg) *)tile_desc)[o])
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
int far _fstrcpy();
int far sprintf();
int far file_read_all();
int far roll_dice();
int far sub_s004_1262_0d0a();
int far party_lore_level();
int far sub_s008_1432_0000();
void far sub_s008_1432_009c();
void far sub_s008_1432_0172();
void far sub_s008_1432_04ce();
void far sub_s008_1432_0926();
void far wild_build();
int far sub_s008_1432_1694();
void far sub_s008_1432_1790();
void far sub_s008_1432_20b2(int, unsigned char, int, int, int, int, int, int, int, int, int);
int far sub_s008_1432_28a0();
void far sub_s008_1432_294e();
int far sub_s008_1432_2adc();
char far sub_s008_1432_2d34();
int far empty_battlefield();
void far sub_s008_1432_2e76();
void far sub_s008_1432_3002();

/* wilderness battle: build the battlefield for location type p6 into the level buffer and save
 * it as level0.flr */
#pragma optimize("a", on)
int far sub_s008_1432_0000(int p6, int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    char *m = "Wl";                     /* buffer name */

    D_df7f = (_segment)(D_df7d = alloc_buffer(0x33fcL, m));
    D_df81 = 0;
    sub_s008_1432_009c(p6);
    sub_s008_1432_0172(p6);
    sub_s008_1432_04ce(p6);
    sub_s008_1432_0926(p6);
    wild_build(p6, a, b, c, d, e, f, g, h, i);
    D_df81->nobj = 0;
    sub_s004_1262_0d0a("level0.flr", (struct level far *)D_df81, 0x33fc, 0);
    free_buffer(D_df7d, m);
    return 1;
}
#pragma optimize("", on)

void far sub_s008_1432_009c(int p_6)
{
    D_df83 = D_d843;
    D_dfb9 = 0;
    switch (p_6) {
    case 0x33:
    case 0x3f:
    case 0x40:
    case 0x41:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
    case 0x50:
    case 0x51:
    case 0x6f:
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
        D_dfb9 = 1;
        return;
    case 0x35:
        D_df83 = 0x14;
        return;
    case 0x38:
        if (D_df83 != 0x18) {
            return;
        }
        D_df83 = 0x11;
        return;
    case 0x3b:
        D_df83 = 0xe;
        return;
    case 0x42:
    case 0x43:
        D_df83 = 8;
    default:
        return;
    }
}

/* load the prepared map iwild<terrain>.<D_df87><D_df89><D_df8b> for location type p6 */
void far sub_s008_1432_0172(int p6)
{
    int nv[8];
    char *nm[8];
    char name[0x50];
    register int k;
    int t;

    nv[1] = 6;
    nv[4] = 6;
    nv[5] = 7;
    nv[6] = 2;
    nv[2] = 4;
    nv[3] = 4;
    nv[7] = 4;
    nm[0] = "---";
    nm[1] = "gen";
    nm[2] = "mtn";
    nm[3] = "msh";
    nm[4] = "spc";
    nm[5] = "sab";
    nm[6] = "gat";
    nm[7] = "wal";
    D_df87 = 1;
    k = 0;
    nv[0] = k;
    D_df89 = k;
    D_df8b = k;
    switch (p6) {
    case 0x32:
    case 0x55:
        D_df85 = k;
        D_df85 = 4;
        D_df8b = roll_dice(1, 2) + 2;
        k = D_df85;
        break;
    case 0x34:
    case 0x41:
    case 0x6f:
    case 0x70:
    case 0x71:
        D_df89 = 1;
        break;
    case 0x36:
        k = 4;
        D_df8b = 1;
        break;
    case 0x37:
        k = 4;
        D_df8b = 2;
        break;
    case 0x3a:
        D_df87 = k = 2;
        break;
    case 0x3e:
    case 0x75:
    case 0x79:
        D_df85 = k;
        D_df85 = 4;
        D_df8b = roll_dice(1, 2) + 4;
        k = D_df85;
        break;
    case 0x46:
    case 0x47:
        k = 2;
        break;
    case 0x48:
    case 0x72:
        k = 5;
        D_df8b = 1;
        break;
    case 0x49:
    case 0x4f:
    case 0x50:
        k = 5;
        D_df8b = 2;
        break;
    case 0x4a:
        k = 5;
        D_df8b = 3;
        break;
    case 0x4b:
        k = 5;
        D_df8b = 4;
        break;
    case 0x4c:
        D_df8b = k = 5;
        break;
    case 0x4d:
        k = 5;
        D_df8b = 6;
        break;
    case 0x4e:
    case 0x73:
        k = 5;
        D_df8b = 7;
        break;
    case 0x52:
    case 0x54:
        k = 7;
        break;
    case 0x53:
        k = 6;
        break;
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x74:
        D_df87 = 3;
        D_df89 = k = 1;
        break;
    }
    D_df85 = k;
    if (k == 0) {
        D_df85 = 1;
        switch (D_df83) {
        case 2:
        case 3:
        case 0x19:
            if (D_df89 == k) {
                D_df85 = 4;
                D_df8b = roll_dice(1, 2) + 2;
            }
            break;
        case 4:
        case 5:
            D_df85 = 3;
            break;
        case 7:
        case 0x10:
        case 0x11:
            D_df87 = 2;
            break;
        case 8:
        case 9:
            D_df89 = 1;
            break;
        case 0x0a:
        case 0x0b:
            if (D_df89 == k && roll_dice(1, 10) > 5)
                D_df89 = 1;
            break;
        case 0x0f:
            if (roll_dice(1, 10) > 5)
                D_df87 = 2;
            break;
        case 0x12:
            if (D_df89 == k)
                D_df85 = 2;
            else
                D_df87 = 2;
            break;
        case 0x13:
        case 0x14:
            if (D_df89 == k) {
                D_df85 = 2;
                if (roll_dice(1, 10) > 5)
                    D_df87 = 2;
            } else
                D_df87 = 2;
            break;
        case 0x15:
        case 0x16:
        case 0x17:
            if (D_df89 == k)
                D_df87 = D_df85 = 2;
            else
                D_df87 = 2;
            break;
        case 0x18:
            D_df87 = 3;
            break;
        }
    }
    if (D_df8b <= 0 && (t = D_c100) > 0) {
        D_df8b = t % 10;
        if (nv[D_df85] < D_df8b)
            D_df8b = 0;
    }
    if (D_df8b <= 0)
        D_df8b = roll_dice(1, nv[D_df85]);
    sprintf(name, "iwild%s.%03d", nm[D_df85], (D_df87 * 10 + D_df89) * 10 + D_df8b);
    file_read_all(name, (struct level far *)D_df81);
}

/* tile description for the wilderness ("out", wall set by D_df83) and random variation of the
 * map: height nibbles (hi) / floor tiles (lo) and the four wall nibbles of every cell */
#pragma optimize("a", on)
void far sub_s008_1432_04ce(int p6)
{
    union { unsigned long x; struct { char p[2]; unsigned char lo, hi; } b; } u;
    int m, r, c, k;
    unsigned char h;

    switch (cur_month) {
    default:
        B(0) = 1;
        break;
    case 0: case 1: case 2: case 10: case 11:
        B(0) = 9;
        break;
    case 3: case 8: case 9:
        B(0) = 8;
        break;
    }
    _fstrcpy((char far *)&B(1), (char far *)"out");
    switch (D_df83) {
    default:
        B(0x0b) = 0;
        B(0x0c) = 1;
        B(0x0d) = 2;
        m = 0;
        break;
    case 4:
    case 5:
        B(0x0b) = 3;
        B(0x0c) = 4;
        B(0x0d) = 5;
        m = 2;
        break;
    case 6:
    case 7:
        B(0x0b) = 6;
        B(0x0c) = 7;
        B(0x0d) = 8;
        m = 1;
        break;
    }
    B(0x0e) = 9;
    B(0x0f) = 10;
    _fstrcpy((char far *)&B(0x1a), (char far *)"out");
    B(0x26) = 0x13;
    B(0x27) = 0x14;
    B(0x28) = 0x15;
    B(0x29) = 0x16;
    B(0x2a) = 0x17;
    B(0x2b) = 0x12;
    B(0x2c) = 8;
    B(0x2d) = 9;
    B(0x2e) = 10;
    B(0x2f) = 11;
    B(0x30) = 12;
    B(0x31) = 13;
    B(0x32) = 0x10;
    B(0x33) = 0x11;
    _fstrcpy((char far *)&B(0x34), (char far *)"out");
    B(0x3e) = 0;
    if (roll_dice(1, 10) > 5)
        B(0x3f) = 1;
    else
        B(0x3f) = 2;
    B(0x40) = 3;
    B(0x41) = 4;
    B(0x42) = 5;
    _fstrcpy((char far *)&B(0x4d), (char far *)"out");
    B(0x58) = 0;
    _fstrcpy((char far *)&B(0x67), (char far *)"out");
    B(0x71) = 0;
    if (B(0x3f) == 1)
        B(0x72) = 1;
    else
        B(0x72) = 2;
    B(0x73) = 3;
    for (r = 1; r < 39; r++) {
        for (c = 1; c < 39; c++) {
            h = D_df81->hgt[r][c];
            u.b.hi = h >> 4;
            switch (h & 0xf) {
            case 1: u.b.lo = 14; break;
            case 2: u.b.lo = 15; break;
            case 3: u.b.lo = 8; break;
            case 4: u.b.lo = 9; break;
            case 5: u.b.lo = 10; break;
            case 6: u.b.lo = 11; break;
            case 7: u.b.lo = 12; break;
            case 8: u.b.lo = 13; break;
            case 9: u.b.lo = 7; break;
            default:
                u.b.lo = 0;
                if (roll_dice(1, 4) == 1) {
                    if (m == 0)
                        u.b.lo = roll_dice(1, 4) + 2;
                    else
                        u.b.lo = roll_dice(1, 5) + 1;
                }
                break;
            }
            switch (u.b.hi) {
            case 2:
                u.b.hi = 5;
                u.b.lo = 0;
                break;
            case 3:
                u.b.hi = 4;
                u.b.lo = 0;
                break;
            default:
                u.b.hi = roll_dice(1, 3);
                break;
            }
            D_df81->hgt[r][c] = (u.b.hi << 4) | (u.b.lo & 0xf);
            for (k = 0; k <= 24; k += 8) {
                u.x = (D_df81->cell[r][c] & (0xfL << k)) >> k;
                if (u.x == 0)
                    continue;
                switch ((int)u.x) {
                case 2: u.x = 3; break;
                case 3: u.x = 2; break;
                case 4: u.x = 4; break;
                case 5: u.x = 5; break;
                default: u.x = 1; break;
                }
                D_df81->cell[r][c] &= ~(0xfL << k);
                D_df81->cell[r][c] |= (u.x & 0xf) << k;
                if (k == 16) {
                    if (u.x == 4)
                        D_df81->cell[r - 1][c] |= 0x100000L;
                    else if (u.x == 5)
                        D_df81->cell[r + 1][c] |= 0x1000L;
                }
            }
        }
    }
}
#pragma optimize("", on)


/* tile set of the wilderness map: group "outgrpa/b/c", count the special tile types the prepared
 * map uses (markers 5..0x13, objects 0x66 standing for one), give each a slot in the tile table
 * of the description (tile_desc + 0xa4), shuffle the random decoration slots and then replace the
 * placeholder tile numbers of every cell by real tile slots (random thinning by terrain) */
#define TAB(i) B(0xa4 + (i))
#pragma optimize("a", on)
void far sub_s008_1432_0926(int p6)
{
    int x0ag;
    int x08;
    int x06b;
    long rotk;
    int x10h;
    int x0ed;
    int x0cf;
    int a1cm[4];
    int x14e;
    int x12g;
    int y26aa;
    int x24e;
    long vm;
    int x1ee;
    int x2ai;
    int x28b;
    int x32h;
    struct obj far *of;
    int x2ch;
    int x3aj;
    int x38c;
    int a36l[2];
    int x40k;
    int x3eg;
    int x3ci;
    int gsn;
    int x46f;
    int x44h;
    int x42j;
    int x54h;
    int x52j;
    int a50c[4];
    int x5al;
    int rh;
    int x56g;
    int x60m;
    int x5ei;
    int ch;
    int x66h;
    int x64j;
    int x62l;
    int x6cl;
    int x6an;
    char x68g;
    int x72m;
    int x70o;
    int x6ek;
    int x78h;
    int x76j;
    int kd;

    if (D_df83 == 5 || D_df83 == 4) {
        _fstrcpy((char far *)&B(0x9a), (char far *)"outgrpc");
        gsn = 3;
    } else if (D_dfb9) {
        _fstrcpy((char far *)&B(0x9a), (char far *)"outgrpb");
        gsn = 4;
    } else {
        _fstrcpy((char far *)&B(0x9a), (char far *)"outgrpa");
        gsn = 0;
    }
    x2ai = x56g = x64j = x40k = x1ee = x6ek = x42j = x46f = x14e = 0;
    switch (p6) {
    case 0x36:
        x14e++;
        break;
    case 0x39:
    case 0x41:
    case 0x6f:
    case 0x70:
    case 0x71:
        x1ee++;
        break;
    case 0x43:
        x46f++;
        break;
    }
    for (rh = 1; rh < 39; rh++)
        for (ch = 1; ch < 39; ch++) {
            vm = (D_df81->cell[rh][ch] & 0xf0) >> 4;
            if (vm == 15) {
                for (kd = 0; kd < D_df81->nobj; kd++) {
                    of = &D_df81->obj[kd];
                    if (of->type == 0x66 && of->x0 == rh << 5 && of->x2 == ch << 4)
                        break;
                }
                if (kd < D_df81->nobj)
                    vm = of->x0a;
            }
            switch ((int)vm) {
            case 5:
                x2ai++;
                break;
            case 6:
                x56g++;
                break;
            case 7:
            case 8:
                x64j++;
                break;
            case 9:
                x42j++;
                break;
            case 10:
                x6ek++;
                break;
            case 0x10:
                x46f++;
                break;
            case 0x11:
                x14e++;
                break;
            case 0x12:
                x40k++;
                break;
            case 0x13:
                x1ee++;
                break;
            }
        }
    x76j = x28b = x3aj = x0ag = x72m = x0cf = x52j = x12g = x6cl = 0;
    x3ci = 1;
    TAB(1) = 0;
    x0ed = 2;
    TAB(2) = 1;
    x78h = 3;
    TAB(3) = 2;
    x54h = 4;
    TAB(4) = 3;
    x3eg = 5;
    x66h = 15;
    if (x42j > 0) {
        x0cf = 15;
        x66h = 14;
        TAB(15) = 15;
    }
    if (x6ek > 0)
        TAB(x52j = x66h--) = 0x10;
    if (x46f > 0)
        TAB(x12g = x66h--) = 0x0b;
    if (x14e > 0)
        TAB(x6cl = x66h--) = 0x0c;
    if (x2ai > 0)
        TAB(x76j = x3eg++) = 0x12;
    if (x56g > 0)
        TAB(x28b = x3eg++) = 0x13;
    if (x64j > 0)
        TAB(x3aj = x3eg++) = 0x0a;
    if (x40k > 0)
        TAB(x0ag = x3eg++) = 0x11;
    if (x1ee > 0)
        TAB(x72m = x3eg++) = 0x0d;
    x38c = y26aa = x08 = 0;
    switch (x66h - x3eg + 1) {
    case 11:
    case 10:
        x38c++;
    case 9:
        x08++;
    case 8:
        y26aa++;
    case 7:
        x08++;
    case 6:
        x38c++;
    case 5:
        x38c++;
    case 4:
        x08++;
    case 3:
        y26aa++;
    case 2:
        x08++;
    case 1:
        x38c++;
    }
    a50c[0] = 5;
    a50c[1] = 6;
    a50c[2] = 7;
    a50c[3] = 8;
    for (rh = 3; rh > 0; rh--) {
        ch = roll_dice(1, rh + 1) - 1;
        if (ch != rh) {
            kd = a50c[rh];
            a50c[rh] = a50c[ch];
            a50c[ch] = kd;
        }
    }
    if (roll_dice(1, 2) == 1) {
        a36l[0] = 4;
        a36l[1] = 9;
    } else {
        a36l[0] = 9;
        a36l[1] = 4;
    }
    a1cm[0] = 0;
    a1cm[1] = 1;
    a1cm[2] = 2;
    for (rh = a1cm[3] = 3; rh > 1; rh--) {
        ch = roll_dice(1, rh);
        if (ch != rh) {
            kd = a1cm[rh];
            a1cm[rh] = a1cm[ch];
            a1cm[ch] = kd;
        }
    }
    if (gsn == 3)
        x62l = 0x1e;
    else if (gsn == 4)
        x62l = 0x14;
    else
        switch (D_df83) {
        case 2:
        case 3:
        case 0x19:
            if (roll_dice(1, 2) == 1)
                x62l = 0x1e;
            else
                x62l = 0x14;
            break;
        case 8:
        case 10:
        case 12:
        case 14:
        case 18:
            x62l = 0x1e;
            break;
        default:
            x62l = 0x14;
            break;
        }
    x10h = x3eg;
    for (rh = 0; x38c > rh && x66h >= x3eg; rh++) {
        TAB(x3eg) = a50c[rh];
        x3eg++;
    }
    B(0xa3) = x3eg;
    x06b = x3eg;
    for (rh = 0; y26aa > rh && x66h >= x3eg; rh++) {
        TAB(x3eg) = a36l[rh];
        x3eg++;
    }
    x6an = x3eg;
    for (rh = 0; x08 > rh && x66h >= x3eg; rh++) {
        TAB(x3eg) = a1cm[rh] + x62l;
        x3eg++;
    }
    x24e = x2ch = x60m = x70o = 10;
    x5al = x5ei = 0;
    x32h = 0x0c;
    x44h = 0x0d;
    switch (D_df83) {
    case 4:
        x2ch = 0;
        x5al = x5ei = 100;
        x32h = 0x0b;
        x44h = 0x0e;
        x60m = 0x19;
        break;
    case 0x16:
    case 0x17:
        x24e = x2ch = 0x14;
        x5al = x5ei = 0x32;
        x32h = x44h = 0x0b;
        break;
    case 0x0e:
    case 0x0f:
        x24e = 0x28;
        x2ch = 0x1e;
        x60m = 0x14;
        break;
    case 0x0c:
    case 0x0d:
        x24e = 0x50;
        x60m = x2ch = 0x32;
        x5ei = 100;
        x44h = 0x0c;
        break;
    case 6:
    case 7:
        x2ch = 0x14;
        x60m = 0x19;
        x5al = x24e = 0x46;
        x5ei = 100;
        x32h = x44h = 0x0b;
        break;
    case 0x18:
        x24e = x60m = 0x14;
        x5ei = 10;
        x44h = 0x0b;
        break;
    case 0x10:
    case 0x11:
        x24e = 0x14;
        x2ch = 8;
        x60m = 0x0f;
        x5ei = 0x16;
        x44h = 0x0c;
        break;
    }
    if (D_c100 >= 10)
        x5al = x5ei = x24e = x2ch = x60m = x70o = 0;
    D_dfba = 0;
    for (rh = 1; rh < 39; rh++)
        for (ch = 1; ch < 39; ch++) {
            x68g = 0;
            vm = (D_df81->cell[rh][ch] & 0xf0) >> 4;
            switch ((int)vm) {
            case 0x0c:
                if (roll_dice(1, 100) <= x24e)
                    vm = 0;
                else if (roll_dice(1, 100) <= x5al)
                    vm = x32h;
                break;
            case 0x0d:
                if (roll_dice(1, 100) <= x2ch)
                    vm = 0;
                else if (roll_dice(1, 100) <= x5ei)
                    vm = x44h;
                break;
            case 0x0b:
                if (roll_dice(1, 100) <= x60m)
                    vm = 0;
                break;
            case 0x0e:
                if (roll_dice(1, 100) <= x70o)
                    vm = 0;
                break;
            case 0x0f:
                for (kd = 0; kd < D_df81->nobj; kd++) {
                    of = &D_df81->obj[kd];
                    if (of->type == 0x66 && of->x0 == rh << 5 && of->x2 == ch << 4)
                        break;
                }
                if (kd < D_df81->nobj)
                    vm = of->x0a;
                if (vm == 15)
                    switch (p6) {
                    case 0x36:
                        vm = 0x11;
                        break;
                    case 0x39:
                    case 0x41:
                    case 0x6f:
                    case 0x70:
                    case 0x71:
                        vm = 0x13;
                        break;
                    case 0x43:
                        vm = 0x10;
                        break;
                    case 0x3d:
                        x68g = 7;
                        D_dfba = 1;
                        break;
                    }
                break;
            }
            rotk = (long)roll_dice(1, 4) - 1;
            switch ((int)vm) {
            default:
                vm = 0;
                break;
            case 1:
                vm = x3ci;
                break;
            case 2:
                vm = x0ed;
                break;
            case 3:
                vm = x78h;
                break;
            case 4:
                vm = x54h;
                break;
            case 5:
                vm = x76j;
                break;
            case 6:
                vm = x28b;
                break;
            case 7:
                vm = x3aj;
                rotk = 0;
                break;
            case 8:
                vm = x3aj;
                rotk = 1;
                break;
            case 9:
                vm = x0cf;
                break;
            case 10:
                vm = x52j;
                break;
            case 11:
                if (gsn == 3)
                    vm = (long)roll_dice(1, y26aa) + x06b - 1;
                else
                    vm = (long)roll_dice(1, y26aa + x38c) + x10h - 1;
                break;
            case 12:
                vm = (long)roll_dice(1, x08 - 1) + x6an;
                break;
            case 13:
                vm = x6an;
                break;
            case 14:
                vm = (long)roll_dice(1, x38c) + x10h - 1;
                break;
            case 16:
                vm = x12g;
                D_dfba = 1;
                break;
            case 17:
                vm = x6cl;
                D_dfba = 1;
                break;
            case 18:
                vm = x0ag;
                D_dfba = 1;
                break;
            case 19:
                vm = x72m;
                D_dfba = 1;
                break;
            }
            while (TAB((int)vm) < 0 && (((int *)&vm)[1] != 0 || *(int *)&vm != 0))
                vm--;
            D_df81->cell[rh][ch] &= ~0xf0L;
            D_df81->cell[rh][ch] |= vm << 4;
            D_df81->cell[rh][ch] &= ~0x30000000L;
            D_df81->cell[rh][ch] |= (rotk & 3) << 28;
            if (x68g) {
                D_df81->hgt[rh][ch] &= 0xf0;
                D_df81->hgt[rh][ch] |= x68g;
            }
        }
}
#pragma optimize("", on)

struct subgrp { char t; char x1; int x2; int x4; char x6; char x7; char x8; char p9; };
struct group {                          /* enemy group, 0x1e bytes (see s010) */
    int pos;                            /* 00 */
    int pos2;                           /* 02 */
    int x04;                            /* 04 */
    char x06;                           /* 06 */
    char x07;                           /* 07: 1 = has a sub-group */
    char type[3];                       /* 08 */
    char count[3];                      /* 0b */
    char p0e[6];
    struct subgrp sub;                  /* 14 */
};
extern struct group _based(enemy_seg) *bf_groups;
extern int group_count;
struct mon { char p0[0x0c]; char x0c; char p0d[0xd6 - 0x0d]; };
extern struct mon _based(enemy_seg) *bf_monsters;
extern int D_df99;
extern char combat_flags;
extern int D_df8d;
extern int D_df8f[5];
extern int D_df9b[5][3];
extern int D_c0fe;
int far dir_between();

/* place the party and the enemy groups: battlefield type by location (1694), adjust the enemy
 * parameters (1790), objects (20b2), party at the marker of id D_df8d, then up to five enemy
 * groups (marker ids D_df8f[], monster counts D_df9b[][3]) */
/* place the party and the enemy groups: battlefield type by location (1694), adjust the enemy
 * parameters (1790), objects (20b2), party at the marker of id D_df8d, then up to five enemy
 * groups (marker ids D_df8f[], monster counts D_df9b[][3]) */
#pragma optimize("a", on)
void far wild_build(int p6, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    char flag;
    int t, k, i, j, s, v, m, r, x;
    int px, py;
    int g1, g2, g3;
    int s16, s18, s1a;
    int *p;
    struct obj far *o;

    if (D_df89 == 1 || D_dfba != 0)
        flag = 1;
    else
        flag = 0;
    t = sub_s008_1432_1694(p6, &flag, ft1, fq1, fn1, ft2, fq2, fn2, ft3, fq3, fn3);
    sub_s008_1432_1790(p6, &ft1, &fq1, &fn1, &ft2, &fq2, &fn2, &ft3, &fq3, &fn3);
    sub_s008_1432_20b2(t, flag, ft1, fq1, fn1, ft2, fq2, fn2, ft3, fq3, fn3);
    k = sub_s008_1432_28a0(D_df8d);
    o = &D_df81->obj[k];
    D_df81->x1f4c = o->x0 + 16;
    D_df81->x1f4e = o->x2 + 8;
    D_df81->x1f50 = 0;
    D_df81->x1f51 = o->x0c;
    px = D_df81->x1f4c / 32;
    py = D_df81->x1f4e / 16;
    g1 = g2 = g3 = -1;
    if (ft1 >= 0 && fn1 > 0)
        g1 = monster_type_map(0, ft1, fq1);
    if (ft2 >= 0 && fn2 > 0)
        g2 = monster_type_map(0, ft2, fq2);
    if (ft3 >= 0 && fn3 > 0)
        g3 = monster_type_map(0, ft3, fq3);
    x = 0;
    for (i = 0; i < 5; i++) {
        p = D_df9b[i];
        s = 0;
        for (j = 0; j < 3; j++) {
            v = p[j];
            if (v < 0)
                v = 0;
            s += v;
        }
        if (s <= 0)
            continue;
        m = sub_s008_1432_28a0(D_df8f[i]);
        sub_s008_1432_294e(&s16, &s1a, &s18, p6, ft1, fn1, ft2, fn2, fq3);
        switch (s16) {
        case 1:
            s16 = g1;
            break;
        case 2:
            s16 = g2;
            break;
        case 3:
            s16 = g3;
            break;
        default:
            s16 = -1;
            break;
        }
        r = sub_s008_1432_2adc(p6, m, s1a, s18, g1, p[0], g2, p[1], g3, p[2]);
        if (r >= 0 && x == 0)
            D_df81->x1f50 = dir_between(px, py, x = bf_groups[r].pos / 32,
                bf_groups[r].pos2 / 16);
    }
    switch (p6) {
    case 0x31:
    case 0x35:
    case 0x47:
    case 0x78:
        combat_flags |= 0x40;
        break;
    }
    if (p6 == 0x55) {
        switch (D_df81->x1f50) {
        case 2:
            r = px - 1;
            j = py;
            break;
        case 4:
            j = py - 1;
            r = px;
            break;
        case 6:
            r = px + 1;
            j = py;
            break;
        default:
            j = py + 1;
            r = px;
            break;
        }
        D_df81->hgt[r][j] &= 0xf0;
        D_df81->hgt[r][j] |= 7;
    }
}
#pragma optimize("", on)

int far sub_s008_1432_1694(int p_6, int p_8, int p_a, int p_c, int p_e, int p_10, int p_12, int p_14, int p_16, int p_18)
{
    switch (p_6) {
    default:
        return p_6;
    case 0x32:
    case 0x33:
    case 0x38:
    case 0x3f:
    case 0x44:
    case 0x45:
    case 0x55:
    case 0x6f:
    case 0x70:
    case 0x76:
L_1740:
        return 0x2f;
    case 0x3a:
        *(char *)p_8 = 1;
        return 0x2e;
    case 0x3b:
        if (p_a == 0x2a) {
            goto L_1740;
        }
        if (p_a == 0x2b) {
            goto L_178b_1066;
        }
        if (p_a == 0x3a) {
            goto L_178b_1067;
        }
        goto L_1778;
    case 0x41:
        if (p_10 != 0x29) {
            goto L_1740;
        }
        goto L_1778;
    case 0x42:
        if (p_18 <= 0) {
            goto L_1740;
        }
    case 0x31:
    case 0x35:
    case 0x39:
    case 0x40:
    case 0x51:
    case 0x71:
    case 0x78:
L_1778:
        return 0x2e;
    case 0x43:
        if (p_18 > 0) {
            goto L_178b_1065;
        }
        goto L_1778;
    case 0x77:
        return 0x30;
    case 0x34:
        return p_6;
    case 0x36:
        return p_6;
    case 0x37:
        return p_6;
    case 0x3c:
        return p_6;
    case 0x3d:
        return p_6;
    case 0x3e:
        return p_6;
    case 0x46:
        return p_6;
    case 0x47:
        return p_6;
    case 0x48:
        return p_6;
    case 0x49:
        return p_6;
    case 0x4a:
        return p_6;
    case 0x4b:
        return p_6;
    case 0x4c:
        return p_6;
    case 0x4d:
        return p_6;
    case 0x4e:
        return p_6;
    case 0x4f:
        return p_6;
    case 0x50:
        return p_6;
    case 0x52:
        return p_6;
    case 0x53:
        return p_6;
    case 0x54:
        return p_6;
    case 0x56:
        return p_6;
    case 0x57:
        return p_6;
    case 0x58:
        return p_6;
    case 0x59:
        return p_6;
    case 0x5a:
        return p_6;
    case 0x5b:
        return p_6;
    case 0x5c:
        return p_6;
    case 0x5d:
        return p_6;
    case 0x5e:
        return p_6;
    case 0x5f:
        return p_6;
    case 0x60:
        return p_6;
    case 0x61:
        return p_6;
    case 0x62:
        return p_6;
    case 0x63:
        return p_6;
    case 0x64:
        return p_6;
    case 0x65:
        return p_6;
    case 0x66:
        return p_6;
    case 0x67:
        return p_6;
    case 0x68:
        return p_6;
    case 0x69:
        return p_6;
    case 0x6a:
        return p_6;
    case 0x6b:
        return p_6;
    case 0x6c:
        return p_6;
    case 0x6d:
        return p_6;
    case 0x6e:
        return p_6;
    case 0x72:
        return p_6;
    case 0x73:
        return p_6;
    case 0x74:
        return p_6;
    case 0x75:
        return p_6;
L_178b_1065:
        return p_6;
L_178b_1066:
        return p_6;
L_178b_1067:
        return p_6;
    }
}

struct member { char status; char pad[0x7f]; };
extern struct member party[];

/* the enemies for location type p6: kinds *pt1..*pt3, quality *pq1..*pq3, numbers *pn1..*pn3
 * from a random strength r and the number of able party members */
/* the enemies for location type p6: kinds *pt1..*pt3, quality *pq1..*pq3, numbers *pn1..*pn3
 * from a random strength r and the number of able party members */
/* the enemies for location type p6: kinds *pt1..*pt3, quality *pq1..*pq3, numbers *pn1..*pn3
 * from a random strength r and the number of able party members */
void far sub_s008_1432_1790(int p6, int *pt1, int *pq1, int *pn1, int *pt2, int *pq2, int *pn2,
    int *pt3, int *pq3, int *pn3)
{
    int t1n, q1e, n1b, t2o, q2e, n2b, t3o, q3f, n3c;
    int ri, cntk, i;

    t1n = t2o = t3o = -1;
    n1b = q1e = q2e = q3f = 1;
    ri = party_lore_level();
    i = cntk = n2b = n3c = 0;
    for (; i < 5; i++)
        if (party[i].status == 1)
            cntk++;
    switch (p6) {
    case 0x36:
        if (ri >= 8)
            t1n = 0x37;
        else {
            t1n = 0x35;
            if (ri > 3)
                q1e = 2;
        }
        break;
    case 0x37:
        t1n = 0x37;
        if (ri >= 7)
            q1e = 2;
        break;
    case 0x3f:
        t1n = 9;
        q1e = max(min(ri / 8 + 1, 2), 1);
        n1b = roll_dice(1, cntk) + roll_dice(1, 2) + 1;
        n1b = max(min(n1b, 7), 2);
        break;
    case 0x40:
        t1n = 0x24;
        t2o = 0x22;
        n2b = roll_dice(q2e = 1, 2) + cntk / 5 + 2;
        n2b = max(min(n2b, 5), 3);
        break;
    case 0x6f:
        t1n = 0x27;
        t2o = 0x2d;
        n2b = roll_dice(q2e = 1, n1b = 2) + 1;
        break;
    case 0x70:
        t1n = 0x35;
        n1b = roll_dice(1, 2) + cntk - 1;
        n1b = max(min(n1b, 6), 2);
        break;
    case 0x71:
        t1n = 0x27;
        n1b = 2;
        t2o = 0x29;
        q2e = max(min(ri / 6 + 1, 2), 1);
        n2b = roll_dice(1, 3) + cntk - 1;
        n2b = max(min(n2b, 6), 2);
        break;
    case 0x42:
    case 0x43:
        t1n = 4;
        q1e = roll_dice(1, 2) + ri / 6;
        q1e = max(min(q1e, 3), 1);
        t2o = 0x0b;
        q2e = roll_dice(1, 2);
        n2b = roll_dice(1, 3) + cntk / 5 + 2;
        break;
    case 0x33:
        t1n = 0x2f;
        n1b = roll_dice(1, 3) + ri / 6 + 3;
        n1b = max(min(n1b, 7), 4);
        break;
    case 0x39:
        t1n = 0x27;
        t2o = 0x29;
        q2e = max(min(ri / 3 + 1, 2), 1);
        n2b = roll_dice(1, 2) + 5;
        break;
    case 0x45:
        t1n = 0x2a;
        n1b = roll_dice(1, 3) + ri / 4 + 2;
        n1b = max(min(n1b, 7), 3);
        break;
    case 0x46:
    case 0x47:
        t1n = 0x4d;
        q1e = roll_dice(1, 2);
        t2o = 0x0d;
        q2e = roll_dice(1, 2);
        n2b = 3;
        break;
    case 0x72:
        t1n = 0x2a;
        t2o = 0x0d;
        q2e = roll_dice(1, 2);
        n2b = 3;
        break;
    case 0x48:
        t1n = 0x2a;
        n1b = 2;
        break;
    case 0x49:
        t1n = 0x29;
        q1e = roll_dice(1, 2) + 1;
        n1b = roll_dice(1, 3);
        t2o = 0x0d;
        q2e = roll_dice(1, 2);
        n2b = roll_dice(1, 2) + 2;
        break;
    case 0x4a:
        t1n = 0x29;
        q1e = 3;
        t2o = 0x0d;
        n2b = roll_dice(q2e = 1, 4) + 3;
        break;
    case 0x4b:
        t1n = 0x29;
        q1e = roll_dice(1, 2) + 1;
        n1b = roll_dice(1, 2);
        t2o = 0x0d;
        q2e = roll_dice(1, 2);
        n2b = roll_dice(1, 2) + 2;
        break;
    case 0x4c:
        t1n = 0x29;
        n1b = roll_dice(1, 2) + 1;
        t2o = 0x0d;
        q2e = roll_dice(1, 2) + 1;
        n2b = roll_dice(1, q1e = 3) + 2;
        break;
    case 0x4d:
        t1n = 0x1c;
        q1e = 2;
        t2o = 0x29;
        n2b = roll_dice(1, q2e = 3) + ri / 6 + 2;
        n2b = max(min(n2b, 6), 3);
        break;
    case 0x73:
        t1n = 0x35;
        q1e = 2;
        break;
    case 0x4e:
        t1n = 0x1c;
        q1e = 2;
        t2o = 0x29;
        q2e = 3;
        n2b = 7;
        break;
    case 0x4f:
        t1n = 0x29;
        q1e = 3;
        n1b = roll_dice(1, 2) + 1;
        t2o = 0x0d;
        if ((q3f = *pq3) > 0)
            q2e = roll_dice(1, 2) + 1;
        else
            q2e = 3;
        n2b = roll_dice(1, 2) + 3;
        break;
    case 0x50:
        t1n = 0x1c;
        q1e = 2;
        t2o = 0x29;
        if ((q3f = *pq3) > 0)
            q2e = roll_dice(1, 2) + 1;
        else
            q2e = 3;
        n2b = 7;
        break;
    case 0x51:
        t1n = 0x29;
        q1e = roll_dice(1, 2) + 1;
        n1b = roll_dice(1, 2) + 1;
        t2o = 0x0d;
        q2e = roll_dice(1, 2) + ri / 6;
        q2e = max(min(q2e, 3), 2);
        n2b = ri / 6 + roll_dice(1, 3) + 2;
        n2b = max(min(n2b, 5), 3);
        break;
    case 0x63:
        t1n = 4;
        q1e = roll_dice(1, 2) + ri / 6;
        q1e = max(min(q1e, 3), 1);
        t2o = 0x0b;
        q2e = roll_dice(1, 2);
        n2b = ri / 6 + roll_dice(1, 2) + 4;
        n2b = max(min(n2b, 7), 5);
        break;
    case 0x61:
        t2o = 0x0b;
        q2e = roll_dice(1, 2);
        n2b = roll_dice(1, 3) + ri / 6 + 2;
        n2b = max(min(n2b, 6), 3);
        t1n = 4;
        q1e = max(min(ri / 6 + q2e + 1, 4), 2);
        break;
    case 0x62:
        t1n = 0x29;
        q1e = roll_dice(1, 2) + ri / 6;
        q1e = max(min(q1e, 3), 2);
        n1b = roll_dice(1, 2) + 1;
        t2o = 0x0b;
        q2e = roll_dice(1, 2);
        n2b = ri / 6 + roll_dice(1, 3) + 2;
        n2b = max(min(n2b, 8 - n1b), 3);
        break;
    case 0x74:
        t1n = 0x4c;
        q1e = roll_dice(1, 2) + ri / 6;
        q1e = max(min(q1e, 3), 1);
        t2o = 0x0b;
        q2e = roll_dice(1, 2);
        n2b = ri / 6 + roll_dice(1, 3) + 2;
        n2b = max(min(n2b, 7), 3);
        break;
    case 0x76:
    case 0x77:
    case 0x78:
        t2o = t3o = 8;
        if (*pq3 <= 0) {
            t1n = 0x16;
            q1e = ri / 7 + 1;
            q2e = 2;
            n2b = cntk;
        } else {
            t1n = 0x12;
            q1e = max(*pq3 / -5 + 4, 1);
            q2e = 3;
            n2b = 2 - *pq3;
            n3c = max(5 - *pq3, 1);
        }
        break;
    case 0x3e:
    case 0x79:
        t2o = t3o = 8;
        if (*pq3 <= 0) {
            t1n = 0x16;
            q1e = ri / 7 + 1;
            q2e = 2;
            n2b = roll_dice(1, 3);
            q3f = 3;
            n3c = cntk;
        } else {
            t1n = 0x12;
            q2e = 3;
            n2b = 2 - *pq3;
            n3c = max(5 - *pq3, 1);
        }
        break;
    case 0x75:
        switch (*pt1) {
        default:
            t1n = 0x23;
            q1e = 3;
            break;
        case 0x24:
        case 0x26:
            t1n = 0x26;
            q1e = 2;
            break;
        }
        t2o = t3o = 0x11;
        q2e = max(*pq3 / -5 + 4, 1);
        n2b = 2 - *pq3 / 2;
        n3c = max(5 - *pq3, 1);
        break;
    default:
        return;
    }
    *pt1 = t1n;
    *pq1 = q1e;
    *pn1 = n1b;
    *pt2 = t2o;
    *pq2 = q2e;
    *pn2 = n2b;
    *pt3 = t3o;
    *pq3 = q3f;
    *pn3 = n3c;
}
/* marker ids for the party (D_df8d), the enemy groups (D_df8f[]) and the sub-group (D_df99) and
 * how the enemies (counts D_df9b[][3]) are spread over the groups, by battlefield type t; flag
 * and D_c0fe (time of day?) choose between alternative layouts */
#define R(n) roll_dice(1, n)
/* marker ids for the party (D_df8d), the enemy groups (D_df8f[]) and the sub-group (D_df99) and
 * how the enemies (counts D_df9b[][3]) are spread over the groups, by battlefield type t; flag
 * and D_c0fe (time of day?) choose between alternative layouts */
#define R(n) roll_dice(1, n)
void far sub_s008_1432_20b2(int t, unsigned char flag, int ft1, int fq1, int fn1, int ft2,
    int fq2, int fn2, int ft3, int fq3, int fn3)
{
    int i, j;

    for (i = 0; i < 5; i++) {
        D_df8f[i] = -1;
        for (j = 0; j < 3; j++)
            D_df9b[i][j] = 0;
    }
    D_df8d = 0x16;
    D_df8f[0] = 2;
    D_df8f[1] = 3;
    D_df8f[2] = 4;
    D_df8f[3] = 5;
    D_df99 = 10;
    D_df9b[0][0] = fn1;
    D_df9b[0][1] = fn2;
    D_df9b[0][2] = fn3;
    switch (t) {
    case 0x2e:
        if (flag || (D_c0fe > 0 && D_c0fe < 3) || (D_c0fe <= 0 && R(10) > 5)) {
            if (D_c0fe == 2 || R(10) > 5)
                D_df8f[0] = 3;
            D_df99 = 10;
        } else {
            D_df8d = 0x17;
            if (D_c0fe == 3 || R(10) > 5)
                D_df8f[0] = 6;
            else
                D_df8f[0] = 7;
            D_df99 = 1;
        }
        break;
    case 0x30:
        if (flag || (D_c0fe > 0 && D_c0fe < 3) || (D_c0fe <= 0 && R(10) > 5))
            D_df99 = 10;
        else {
            D_df8d = 0x17;
            D_df8f[0] = 6;
            D_df8f[1] = 7;
            D_df99 = 1;
        }
        for (j = 0; j < 3; j++)
            D_df9b[0][j] -= D_df9b[1][j] = R(D_df9b[0][j] - 1);
        break;
    case 0x2f:
        if (flag || (D_c0fe > 0 && D_c0fe < 3) || (D_c0fe <= 0 && R(10) > 5)) {
            if (D_c0fe == 2 || R(10) > 5)
                D_df8f[0] = 4;
            else
                D_df8f[0] = 5;
            D_df99 = 10;
        } else {
            D_df8d = 0x17;
            if (D_c0fe == 3 || R(10) > 5)
                D_df8f[0] = 8;
            else
                D_df8f[0] = 9;
            D_df99 = 1;
        }
        break;
    case 0x43:
        if ((D_c0fe > 0 && D_c0fe < 3) || (D_c0fe <= 0 && R(10) > 5)) {
            D_df8d = 4;
            D_df8f[0] = 2;
            return;
        }
        D_df8d = 8;
        D_df8f[0] = 6;
        break;
    case 0x3b:
        if ((D_c0fe > 0 && D_c0fe < 3) || (D_c0fe <= 0 && R(10) > 5)) {
            D_df8f[0] = 4;
            D_df8f[1] = 5;
            D_df99 = 10;
        } else {
            D_df8d = 0x17;
            D_df8f[0] = 8;
            D_df8f[1] = 9;
            D_df99 = 1;
        }
        break;
    case 0x3c:
        D_df9b[0][1] = 0;
        switch (fn2) {
        case 7:
            D_df9b[3][1]++;
        case 6:
            D_df9b[0][1]++;
        case 5:
            D_df9b[2][1]++;
        case 4:
            D_df9b[1][1]++;
        case 3:
            D_df9b[0][1]++;
        case 2:
            D_df9b[2][1]++;
        case 1:
            D_df9b[1][1]++;
        }
        break;
    case 0x34:
        for (i = 0; i < fn2; i++)
            D_df9b[R(3)][1]++;
        D_df9b[0][1] = 0;
        break;
    case 0x3e:
    case 0x75:
        if (D_c0fe >= 3 || (D_c0fe <= 0 && R(10) > 5))
            D_df8d = 0x17;
        D_df99 = 10;
        break;
    case 0x79:
        if (D_c0fe >= 3 || (D_c0fe <= 0 && R(10) > 5))
            D_df8d = 0x17;
        D_df8f[0] = 3;
        D_df99 = 10;
        break;
    case 0x46:
        D_df8d = 0x17;
        if (D_c0fe == 2 || D_c0fe == 4 || (D_c0fe <= 0 && R(10) > 5))
            D_df8f[0] = 3;
        break;
    case 0x47:
        D_df8d = 0x16;
        if (D_c0fe == 2 || D_c0fe == 4 || (D_c0fe <= 0 && R(10) > 5))
            D_df8f[0] = 3;
        break;
    case 0x48:
        D_df9b[1][0] = D_df9b[0][0] = 1;
        D_df99 = 10;
        break;
    case 0x49:
        for (i = D_df9b[0][1] = 1; i < fn2; i++)
            D_df9b[R(4) - 1][1]++;
        break;
    case 0x4a:
    case 0x4b:
        D_df9b[0][1] = 1;
        D_df9b[1][1] = 3;
        switch (fn2) {
        case 7:
            D_df9b[1][1]++;
        case 6:
            D_df9b[3][1]++;
        case 5:
            D_df9b[2][1]++;
        }
        break;
    case 0x4c:
        D_df9b[0][1] = 0;
        for (i = D_df9b[1][1] = D_df9b[0][0] = 1; i < fn1; i++)
            D_df9b[R(3)][0]++;
        for (i = 1; i < fn2; i++)
            D_df9b[R(3)][1]++;
        break;
    case 0x4d:
        D_df9b[0][1] = 0;
        for (i = D_df9b[1][1] = 1; i < fn2; i++)
            D_df9b[R(2) + 1][1]++;
        D_df99 = 10;
        break;
    case 0x4e:
        for (i = D_df9b[0][1] = 0; i < fn2; i++)
            D_df9b[R(3) + 1][1]++;
        D_df99 = 10;
        break;
    case 0x4f:
        for (i = D_df9b[0][0] = 1; i < fn1; i++)
            D_df9b[R(4) - 1][0]++;
        for (i = D_df9b[0][1] = 0; i < fn2; i++)
            D_df9b[R(4) - 1][1]++;
        if (R(10) > 5)
            D_df99 = 10;
        else
            D_df99 = 1;
        break;
    case 0x50:
        D_df9b[0][0] = 0;
        D_df9b[R(2) - 1][0] = 1;
        D_df9b[0][1] -= 2;
        D_df9b[0][1] -= D_df9b[1][1] =
            roll_dice(D_df9b[3][1] = D_df9b[2][1] = 1, D_df9b[0][1] - 1);
        if (R(10) > 5)
            D_df99 = 10;
        else
            D_df99 = 1;
        break;
    case 0x72:
        D_df99 = 10;
        break;
    case 0x53:
        if (D_c0fe == 2 || D_c0fe == 4 || (D_c0fe <= 0 && R(10) > 5))
            D_df8f[0] = 3;
        D_df99 = 10;
        break;
    case 0x54:
        D_df8d = 0x17;
        D_df8f[0] = 6;
        break;
    case 0x61:
    case 0x63:
        if (D_c0fe == 3 || D_c0fe == 4 || (D_c0fe <= 0 && R(10) > 5)) {
            D_df8d = 0x17;
            switch (R(4)) {
            case 1:
                D_df8f[0] = 6;
                break;
            case 2:
                D_df8f[1] = 7;
                break;
            case 3:
                D_df8f[2] = 8;
                break;
            case 4:
                D_df8f[3] = 9;
                break;
            }
        }
        D_df9b[0][2] = D_df9b[0][1] = 0;
        switch (fn2) {
        case 7:
            D_df9b[1][1]++;
        case 6:
            D_df9b[0][1]++;
        case 5:
            D_df9b[3][1]++;
        case 4:
            D_df9b[2][1]++;
        case 3:
            D_df9b[1][1]++;
        case 2:
            D_df9b[1][1]++;
        case 1:
            D_df9b[0][1]++;
        }
        break;
    case 0x62:
    case 0x74:
        if (D_c0fe == 1 || D_c0fe == 2 || (D_c0fe <= 0 && R(10) > 5)) {
            switch (R(4)) {
            case 1:
                D_df8f[0] = 2;
                break;
            case 2:
                D_df8f[0] = 3;
                break;
            case 3:
                D_df8f[0] = 4;
                break;
            case 4:
                D_df8f[0] = 5;
                break;
            }
        } else {
            D_df8d = 0x17;
            switch (R(4)) {
            case 1:
                D_df8f[0] = 6;
                break;
            case 2:
                D_df8f[0] = 7;
                break;
            case 3:
                D_df8f[0] = 8;
                break;
            case 4:
                D_df8f[0] = 9;
                break;
            }
        }
        break;
    }
}

#pragma optimize("a", on)
int far sub_s008_1432_28a0(int id)
{
    int cnt, i, c, n;
    struct obj far *o;

    cnt = 0;
    for (i = 0; i < D_df81->nobj; i++) {
        o = &D_df81->obj[i];
        if (o->type == 0x65 && o->x0a == id)
            cnt++;
    }
    if (cnt == 0)
        return 0;
    n = roll_dice(1, cnt);
    c = 0;
    for (i = 0; i < D_df81->nobj && n > c; i++) {
        o = &D_df81->obj[i];
        if (o->type == 0x65 && o->x0a == id)
            c++;
    }
    return i - 1;
}
#pragma optimize("", on)

#pragma optimize("a", on)
void far sub_s008_1432_294e(int *a, int *b, int *c, int t, int u, int n, int p12, int p14, int p16)
{
    *a = -1;
    *c = *b = 0;
    switch (t) {
    case 0x3b:
        switch (u) {
        case 0x2a:
            *a = 1;
            *b = 2;
            *c = n - 2;
            *c = max(min(*c, 5), 2);
            break;
        case 0x2b:
        case 0x3a:
            *b = 1;
            *c = *a = 2;
            break;
        }
        break;
    case 0x48:
        *a = 1;
        *b = 2;
        *c = 3;
        break;
    case 0x4d:
        *b = *a = 2;
        *c = 3;
        break;
    case 0x4e:
        *a = 2;
        *b = 3;
        *c = 0xe;
        break;
    case 0x4f:
        *a = 2;
        *b = 3;
        if (p16 > 0)
            *c = 4;
        else
            *c = 8;
        break;
    case 0x50:
        *a = 2;
        *b = 3;
        if (p16 > 0)
            *c = 7;
        else
            *c = 10;
        break;
    case 0x51:
        *a = 2;
        *c = roll_dice(1, *b = 3) + party_lore_level() / 4 + 4;
        *c = max(min(*c, 9), 5);
        break;
    case 0x53:
        if (p16 > 0) {
            if (p12 >= 0)
                *a = 2;
            else if (u >= 0)
                *a = 1;
            *c = p16;
            *b = p16;
        }
        break;
    case 0x72:
        *a = 1;
        *c = *b = 2;
        break;
    }
}
#pragma optimize("", on)


/* create an enemy group of up to three monster kinds at object idx; with x0a >= 0 the second
 * kind is a sub-group at a random marker of id D_df99. Returns the group number or -1 */
#pragma optimize("a", on)
int far sub_s008_1432_2adc(int p6, int idx, int x0a, char x0c, int t1, int n1, int t2, int n2,
    int t3, int n3)
{
    struct group far *g;
    struct subgrp far *h;
    int j, k;

    if (t1 < 0 && t2 < 0)
        return -1;
    if (n1 <= 0 && n2 <= 0)
        return -1;
    if (idx < 0 || idx >= 55)
        return -1;
    if ((k = D_df99) >= 0)
        k = sub_s008_1432_28a0(k);
    g = bf_groups + group_count;
    g->pos = D_df81->obj[idx].x0 + 16;
    g->pos2 = D_df81->obj[idx].x2 + 8;
    g->x06 = D_df81->obj[idx].x0c;
    g->x04 = 16;
    g->x07 = 0;
    j = 0;
    if (t1 >= 0 && t1 < 12 && n1 > 0) {
        g->type[0] = t1;
        g->count[0] = n1;
        j = 1;
        if (sub_s008_1432_2d34(p6, t1))
            bf_monsters[t1].x0c = 1;
    }
    if (t2 >= 0 && t2 < 12 && (n2 > 0 || x0a >= 0)) {
        g->type[j] = t2;
        g->count[j] = n2;
        j++;
        if (sub_s008_1432_2d34(p6, t2))
            bf_monsters[t2].x0c = 1;
        if (x0a >= 0 && k >= 0) {
            g->x07 = 1;
            g->sub.t = t2;
            h = &g->sub;
            h->x1 = 16;
            h->x2 = D_df81->obj[k].x0 + 16;
            h->x4 = D_df81->obj[k].x2 + 8;
            h->x6 = x0a;
            h->x7 = x0c;
            h->x8 = D_df81->x1f50 ^ 4;
        }
    }
    if (t3 >= 0 && t3 < 12 && n3 > 0) {
        g->type[j] = t3;
        g->count[j] = n3;
        j++;
        if (sub_s008_1432_2d34(p6, t3))
            bf_monsters[t3].x0c = 1;
    }
    if (j > 0)
        return group_count++;
    return -1;
}
#pragma optimize("", on)

#pragma optimize("a", on)
char far sub_s008_1432_2d34(int p6, int i)
{
    int t;

    switch (p6) {
    case 0x75:
        t = mon_template[i];
        if (t >= 0x1e && t <= 0x22)
            return 1;
    }
    return 0;
}
#pragma optimize("", on)

/* empty battlefield in the static map F_20a5 (all cells empty), saved as level%d.flr, enemies
 * placed by sub_3002 */
int far empty_battlefield(int p6, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    char name[0x42];
    int g1, g2, g3;
    int r, c;

    floor_img.n3326 = floor_img.x300c = floor_img.nexit = floor_img.x2488 =
        floor_img.nobj = floor_img.n1f52 = floor_img.x1f40 = 0;
    for (r = 0; r < 40; r++)
        for (c = 0; c < 40; c++) {
            floor_img.cell[r][c] = 0x80000000L;
            floor_img.hgt[r][c] = 0;
        }
    sub_s008_1432_2e76();
    sprintf(name, "level%d.flr", floor_img.x1f40);
    sub_s004_1262_0d0a(name, (struct level far *)&floor_img, 0x33fc, 0);
    g1 = g2 = g3 = -1;
    if (ft1 >= 0 && fn1 > 0)
        g1 = monster_type_map(0, ft1, fq1);
    if (ft2 >= 0 && fn2 > 0)
        g2 = monster_type_map(0, ft2, fq2);
    if (ft3 >= 0 && fn3 > 0)
        g3 = monster_type_map(0, ft3, fq3);
    sub_s008_1432_3002(p6, g1, fn1, g2, fn2, g3, fn3);
    return 1;
}

extern int gate_side;

/* empty battlefield: open 20x20 field in F_20a5 ("out" tiles), four raised border rectangles,
 * random party side gate_side (0..3) and start position */
#pragma optimize("a", on)
void far sub_s008_1432_2e76(void)
{
    int r, c, k;

    _fstrcpy((char far *)&B(1), (char far *)"out");
    floor_img.x2488 = B(0x0b) = 0;
    *(volatile int far *)&floor_img.x248a = 0;
    k = 0;
    for (r = 1; r <= 20; r++)
        for (c = 1; c <= 20; c++) {
            floor_img.list[k++] = (r << 8) + c;
            floor_img.cell[r][c] = 0;
            floor_img.hgt[r][c] |= 0x10;
        }
    floor_img.list[k] = -1;
    floor_img.n3326 = 4;
    floor_img.r3328[2][2] = floor_img.r3328[1][0] = floor_img.r3328[0][2] = 0x280;
    floor_img.r3328[2][1] = 0x140;
    floor_img.r3328[2][3] = floor_img.r3328[0][3] = floor_img.r3328[0][1] = 0x10;
    floor_img.r3328[3][1] = floor_img.r3328[3][2] = floor_img.r3328[3][0] =
        floor_img.r3328[2][0] = floor_img.r3328[1][1] = floor_img.r3328[1][2] =
        floor_img.r3328[0][0] = 0x20;
    floor_img.r3328[3][3] = floor_img.r3328[1][3] = 0x120;
    floor_img.x1f51 = -1;
    gate_side = roll_dice(1, 4) - 1;
    floor_img.x1f50 = ((char)gate_side ^ 2) << 1;
    c = r = 0;
    switch (gate_side) {
    default:
        floor_img.x1f50 = 0;
    case 0:
        c = -3;
        break;
    case 1:
        r = 3;
        break;
    case 2:
        c = 3;
        break;
    case 3:
        r = -3;
        break;
    }
    floor_img.x1f4c = (r + 10) << 5;
    floor_img.x1f4e = (c + 10) << 4;
}
#pragma optimize("", on)


/* the enemy group of the empty battlefield: start position by party side gate_side, up to three
 * monster kinds */
void far sub_s008_1432_3002(int p6, int t1, int n1, int t2, int n2, int t3, int n3)
{
    struct group far *g;
    int j;

    g = bf_groups + group_count;
    switch (gate_side) {
    case 0:
        g->pos = 0x140;
        g->pos2 = 0xd0;
        break;
    case 1:
        g->pos = 0xe0;
        g->pos2 = 0xa0;
        break;
    case 2:
        g->pos = 0x140;
        g->pos2 = 0x70;
        break;
    case 3:
        g->pos = 0x1a0;
        g->pos2 = 0xa0;
        break;
    }
    g->x04 = 0x10;
    g->x06 = 0;
    j = 0;
    if (t1 >= j && t1 < 12 && n1 > j) {
        g->type[0] = t1;
        g->count[0] = n1;
        j = 1;
    }
    if (t2 >= 0 && t2 < 12 && n2 > 0) {
        g->type[j] = t2;
        g->count[j] = n2;
        j++;
    }
    if (t3 >= 0 && t3 < 12 && n3 > 0) {
        g->type[j] = t3;
        g->count[j] = n3;
        j++;
    }
    if (j > 0)
        group_count++;
}
