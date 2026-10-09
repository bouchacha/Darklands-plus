/* place: s008 1432:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=1458 */
/* status (e6, 2026-10-09): wilderness battlefields. Started from build/agent/gen_engine (dec16
 * only managed 009c/1694). 3 match: 0000 (driver; buffer name is a local `char *m = "Wl"`, pragma
 * "a"), 009c, 2d62 (empty map in F_20a5 + enemies). Written, close: 0172 (prepared map
 * iwild<terrain>.nnn; compare operand order + cross-jumped D_df87 = 2), 04ce (tile description
 * "out" + random height/wall variation; only its jump-table base differs, waits for 0172's
 * length), 1694 (dec16), 28a0 (random group marker, like s010 2a2c), 2d34.
 * NOT yet written (stubs with their string literals so _DATA lines up): 0926 1388 1790 20b2
 * 294e 2adc 2e76 3002.
 * Level buffer (one, 0x33fc bytes, see s010/s011 struct level): handle D_df7d, segment D_df7f,
 * based pointer D_df81. Globals: D_df83 terrain/season code (copied from D_d843), D_df85 terrain
 * class (index into the "---","gen","mtn",.. names), D_df87/D_df89/D_df8b digits of the map number.
 */

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
    char p1f41[0x1f52 - 0x1f41];
    char n1f52;                         /* 1f52 */
    char p1f53[0x2184 - 0x1f53];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char x2488;                         /* 2488 */
    char p2489[0x2fb6 - 0x2489];
    char nexit;                         /* 2fb6 */
    char p2fb7[0x300c - 0x2fb7];
    char x300c;                         /* 300c */
    char p300d[0x3326 - 0x300d];
    char n3326;                         /* 3326 */
    char p3327[0x33fc - 0x3327];
};
extern int D_d843;
extern int D_df83;
extern char D_dfb9;
extern unsigned D_df7d;
extern _segment D_df7f;
extern struct level _based(D_df7f) *D_df81;
extern int D_df85, D_df87, D_df89, D_df8b;
extern int D_c100;
extern int D_00e4;
extern _segment D_dadd;
struct bfdesc { char b[0xc0]; };
extern struct bfdesc _based(D_dadd) *D_db33;
extern int _based(D_dadd) *D_db29;
extern struct level far F_20a5_0000;
int far sub_s004_0e76_06aa();
#define B(o) (((char _based(D_dadd) *)D_db33)[o])
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
int far _fstrcpy();
int far sprintf();
int far sub_root_05d9_019a();
int far sub_s004_0e76_20f6();
int far sub_s004_1262_0d0a();
int far vec_1c1b();
int far sub_s008_1432_0000();
void far sub_s008_1432_009c();
void far sub_s008_1432_0172();
void far sub_s008_1432_04ce();
void far sub_s008_1432_0926();
void far sub_s008_1432_1388();
int far sub_s008_1432_1694();
void far sub_s008_1432_1790();
void far sub_s008_1432_20b2();
int far sub_s008_1432_28a0();
void far sub_s008_1432_294e();
void far sub_s008_1432_2adc();
char far sub_s008_1432_2d34();
int far sub_s008_1432_2d62();
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
    sub_s008_1432_1388(p6, a, b, c, d, e, f, g, h, i);
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
        D_df8b = sub_s004_0e76_20f6(1, 2) + 2;
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
        D_df8b = sub_s004_0e76_20f6(1, 2) + 4;
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
                D_df8b = sub_s004_0e76_20f6(1, 2) + 2;
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
            if (D_df89 == k && sub_s004_0e76_20f6(1, 10) > 5)
                D_df89 = 1;
            break;
        case 0x0f:
            if (sub_s004_0e76_20f6(1, 10) > 5)
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
                if (sub_s004_0e76_20f6(1, 10) > 5)
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
        D_df8b = sub_s004_0e76_20f6(1, nv[D_df85]);
    sprintf(name, "iwild%s.%03d", nm[D_df85], (D_df87 * 10 + D_df89) * 10 + D_df8b);
    sub_root_05d9_019a(name, (struct level far *)D_df81);
}

/* tile description for the wilderness ("out", wall set by D_df83) and random variation of the
 * map: height nibbles (hi) / floor tiles (lo) and the four wall nibbles of every cell */
#pragma optimize("a", on)
void far sub_s008_1432_04ce(int p6)
{
    union { unsigned long x; struct { char p[2]; unsigned char lo, hi; } b; } u;
    int m, r, c, k;
    unsigned char h;

    switch (D_00e4) {
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
    if (sub_s004_0e76_20f6(1, 10) > 5)
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
                if (sub_s004_0e76_20f6(1, 4) == 1) {
                    if (m == 0)
                        u.b.lo = sub_s004_0e76_20f6(1, 4) + 2;
                    else
                        u.b.lo = sub_s004_0e76_20f6(1, 5) + 1;
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
                u.b.hi = sub_s004_0e76_20f6(1, 3);
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

/* sub_s008_1432_0926: not written yet (stub with its literals) */
void far sub_s008_1432_0926(void)
{
    _fstrcpy("outgrpc");
    _fstrcpy("outgrpb");
    _fstrcpy("outgrpa");
}

/* sub_s008_1432_1388: not decompiled: 16-bit read of partial register ax */
void far sub_s008_1432_1388(void) { }

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

/* sub_s008_1432_1790: not decompiled: ax */
void far sub_s008_1432_1790(void) { }

/* sub_s008_1432_20b2: not decompiled: memory via unknown si */
void far sub_s008_1432_20b2(void) { }

/* pick one of the group markers (type 0x65) with id `id` at random; returns its object index */
int far sub_s008_1432_28a0(int id)
{
    int cnt, i, c, n;

    cnt = 0;
    for (i = 0; i < D_df81->nobj; i++)
        if (D_df81->obj[i].type == 0x65 && D_df81->obj[i].x0a == id)
            cnt++;
    if (cnt == 0)
        return 0;
    n = sub_s004_0e76_20f6(1, cnt);
    c = 0;
    for (i = 0; i < D_df81->nobj && n > c; i++)
        if (D_df81->obj[i].type == 0x65 && D_df81->obj[i].x0a == id)
            c++;
    return i - 1;
}

/* sub_s008_1432_294e: not decompiled: memory via unknown si */
void far sub_s008_1432_294e(void) { }

/* sub_s008_1432_2adc: not decompiled: mov es from non-slot */
void far sub_s008_1432_2adc(void) { }

/* location 0x75 and monster record i of kind 0x1e..0x22 */
char far sub_s008_1432_2d34(int p6, int i)
{
    int t;

    if (p6 == 0x75) {
        t = D_db29[i];
        if (t >= 0x1e && t <= 0x22)
            return 1;
    }
    return 0;
}

/* empty battlefield in the static map F_20a5 (all cells empty), saved as level%d.flr, enemies
 * placed by sub_3002 */
int far sub_s008_1432_2d62(int p6, int ft1, int fq1, int fn1, int ft2, int fq2, int fn2,
    int ft3, int fq3, int fn3)
{
    char name[0x42];
    int g1, g2, g3;
    int r, c;

    F_20a5_0000.n3326 = F_20a5_0000.x300c = F_20a5_0000.nexit = F_20a5_0000.x2488 =
        F_20a5_0000.nobj = F_20a5_0000.n1f52 = F_20a5_0000.x1f40 = 0;
    for (r = 0; r < 40; r++)
        for (c = 0; c < 40; c++) {
            F_20a5_0000.cell[r][c] = 0x80000000L;
            F_20a5_0000.hgt[r][c] = 0;
        }
    sub_s008_1432_2e76();
    sprintf(name, "level%d.flr", F_20a5_0000.x1f40);
    sub_s004_1262_0d0a(name, (struct level far *)&F_20a5_0000, 0x33fc, 0);
    g1 = g2 = g3 = -1;
    if (ft1 >= 0 && fn1 > 0)
        g1 = sub_s004_0e76_06aa(0, ft1, fq1);
    if (ft2 >= 0 && fn2 > 0)
        g2 = sub_s004_0e76_06aa(0, ft2, fq2);
    if (ft3 >= 0 && fn3 > 0)
        g3 = sub_s004_0e76_06aa(0, ft3, fq3);
    sub_s008_1432_3002(p6, g1, fn1, g2, fn2, g3, fn3);
    return 1;
}

/* sub_s008_1432_2e76: not written yet (stub with its literal) */
void far sub_s008_1432_2e76(void)
{
    _fstrcpy("out");
}

/* sub_s008_1432_3002: not decompiled: es:[reg] unknown es=g(56029,) ptr=None */
void far sub_s008_1432_3002(void) { }
