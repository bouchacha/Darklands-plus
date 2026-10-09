/* place: s009 182c:0002 */
/* orig: size=262a _DATA=1530+0005 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): mine battlefields, part 2 (tile description, mine parameters by
 * location type p6 and mine level lv, monster group contents). All 16 functions written
 * (1260 is the 2.6 KB per-type/level table, see its comment). dec16 could decompile only 0fe6.
 * Location types: 0x0a..0x10 and 0x82..0x86 (same mines, 0x82 = 0x0b, 0x83 = 0x0d, 0x84..0x86 =
 * 0x0e..0x10), 0x66..0x69, 0x6c, 0x6d, 0x7f..0x81.
 * Globals: D_e005/D_dffd/D_e001/D_dff9 (long, 03ca: per-level parameters), D_e009 (long, 07d0),
 * D_e035 (char flag), D_db35 (0x2e-byte records based on D_dae3, x25 = default count?).
 * STATUS (m1, 2026-10-09): 7/16 match (0002 03ca 05a2 07d0 0938 10a8 1146).
 *   03ca: long ternary `? 1L : 2L` (sub dx,dx) + goto from the 0x0e lv1 copy into the 0x0d one.
 *   061a: 0x7f..0x86 cases added, chains reversed, gotos to the 0x6c/0x6d bodies; left: 0x0d lv11
 *     keeps the `*c = 12` tail (orig: 0x6c's); a goto into it changes dx tracking of `a`.
 *   07d0: random switch written out in all four places, `? 9L : 10L`.
 *   1146: #pragma optimize("a", on).
 *   0db6: code and frame right (k as a stored dead local, n as the init loop counter, no q);
 *     only its jump table differs - waits for 09b4's length (1038/1026, cross-jump puzzle).
 *   11b2: all-empty switch; 6.00 keeps the 174-byte dispatch, 6.00A deletes it (and drops
 *     empty cases even with volatile params): likely a 6.00 vs 6.00A limit -> no FILE MATCH.
 *   20a4: 6.00A keeps the last `return 6` copy / moves the labelled block; 0fe6/219e: jump tables.
 */

struct res { int a; int b; };
extern _segment D_dadd;
struct bfdesc { char b[0xc0]; };
extern struct bfdesc _based(D_dadd) *D_db33;
#define B(o) (((char _based(D_dadd) *)D_db33)[o])

struct ent {                            /* group entry, 6 bytes */
    int type;                           /* 00 */
    char x2, x3;                        /* 02, 03 */
    char n;                             /* 04 */
    char x5;                            /* 05 */
};
struct grp {                            /* level group record (0x2c bytes, see s011) */
    char p0[0x0b];
    unsigned char n;                    /* 0b: number of entries */
    char p0c[4];
    int x10;                            /* 10 */
    char p12[2];
    struct ent ent[4];                  /* 14 */
};
struct mdef { char p0[0x25]; char x25; char p26[0x2e - 0x26]; };   /* 0x2e bytes */
extern _segment D_dae3;
extern struct mdef _based(D_dae3) *D_db35;
extern long D_dff9, D_dffd, D_e001, D_e005, D_e009;
extern char D_e035;

int far _fstrcpy();
int far sub_s004_0e76_20f6();
int far sub_s009_1432_205e();
void far sub_s009_182c_0002();
void far sub_s009_182c_03ca();
void far sub_s009_182c_05a2();
void far sub_s009_182c_061a();
void far sub_s009_182c_07d0();
void far sub_s009_182c_0938();
void far sub_s009_182c_09b4();
char far sub_s009_182c_0db6();
int far sub_s009_182c_0fe6();
void far sub_s009_182c_10a8();
void far sub_s009_182c_1146();
void far sub_s009_182c_11b2();
void far sub_s009_182c_1260();
int far sub_s009_182c_1cb4();
int far sub_s009_182c_20a4();
int far sub_s009_182c_219e();

/* tile description for the mines (t = location type) */
#pragma optimize("a", on)
void far sub_s009_182c_0002(int t)
{
    char mine[5] = "mine";
    int r[2];
    int i;

    B(0) = 2;
    _fstrcpy((char far *)&B(1), (char far *)mine);
    B(0x0b) = 4;
    B(0x0c) = 5;
    B(0x0d) = 7;
    B(0x0e) = 8;
    B(0x0f) = 6;
    B(0x10) = 10;
    B(0x11) = 11;
    B(0x12) = 12;
    B(0x13) = 0;
    B(0x14) = 1;
    B(0x15) = 2;
    B(0x16) = 3;
    _fstrcpy((char far *)&B(0x1a), (char far *)mine);
    B(0x26) = 9;
    B(0x28) = 8;
    B(0x27) = 10;
    B(0x29) = 11;
    B(0x2a) = 12;
    _fstrcpy((char far *)&B(0x34), (char far *)mine);
    B(0x3e) = 0;
    B(0x3f) = 9;
    B(0x46) = 3;
    B(0x47) = -1;
    B(0x48) = 2;
    B(0x40) = 1;
    B(0x45) = 10;
    B(0x44) = 11;
    switch (t) {
    default:
        B(0x41) = 5;
        r[0] = sub_s004_0e76_20f6(1, 3);
        do
            r[1] = sub_s004_0e76_20f6(1, 3);
        while (r[1] == r[0]);
        for (i = 0; i < 2; i++) {
            switch (r[i]) {
            case 1: B(0x42 + i) = 4; break;
            case 2: B(0x42 + i) = 7; break;
            case 3: B(0x42 + i) = 8; break;
            }
        }
        break;
    case 0x0d:
    case 0x83:
        B(0x41) = 4;
        B(0x42) = 7;
        B(0x43) = 8;
        break;
    }
    _fstrcpy((char far *)&B(0x4d), (char far *)mine);
    B(0x56) = 1;
    B(0x58) = 6;
    B(0x59) = 16;
    B(0x5a) = 8;
    B(0x5b) = 10;
    B(0x5c) = 15;
    B(0x5d) = 14;
    _fstrcpy((char far *)&B(0x67), (char far *)mine);
    B(0x71) = 1;
    B(0x72) = 4;
    B(0x79) = 0;
    B(0x7a) = 10;
    B(0x7b) = 2;
    B(0x73) = 3;
    B(0x77) = 6;
    B(0x78) = 11;
    switch (t) {
    default:
        B(0x74) = 8;
        r[0] = sub_s004_0e76_20f6(1, 3);
        do
            r[1] = sub_s004_0e76_20f6(1, 3);
        while (r[1] == r[0]);
        for (i = 0; i < 2; i++) {
            switch (r[i]) {
            case 1: B(0x75 + i) = 5; break;
            case 2: B(0x75 + i) = 9; break;
            case 3: B(0x75 + i) = 7; break;
            }
        }
        break;
    case 0x0d:
    case 0x83:
        B(0x74) = 5;
        B(0x75) = 9;
        B(0x76) = 7;
        break;
    }
    _fstrcpy((char far *)&B(0x9a), (char far *)mine);
    B(0xa5) = 4;
    B(0xa6) = 3;
    B(0xa7) = 0x17;
    B(0xa9) = 0x15;
    B(0xaa) = 0x16;
    B(0xa8) = 6;
    B(0xac) = 0;
    B(0xab) = 1;
    B(0xa3) = 9;
    B(0xae) = 12;
    B(0xaf) = 5;
    B(0xb0) = 10;
    B(0xb1) = 11;
    B(0xb2) = 13;
    B(0xb3) = 2;
    switch (t) {
    default:
        B(0xad) = 2;
        break;
    case 0x0b:
    case 0x82:
        B(0xad) = 9;
        break;
    case 0x0e:
    case 0x84:
        B(0xad) = 7;
        break;
    case 0x0f:
    case 0x85:
        B(0xad) = 8;
        break;
    case 0x10:
    case 0x86:
        B(0xad) = 20;
        B(0xa3) = 9;
        break;
    }
}
#pragma optimize("", on)

/* mine parameters D_e005, D_dffd, D_e001, D_dff9 for location type p6, level lv; *out = 0/1/6 */
void far sub_s009_182c_03ca(int p6, int lv, int *out)
{
    D_dff9 = D_e001 = D_dffd = D_e005 = 9;
    if (sub_s004_0e76_20f6(1, 2) == 1)
        D_e005 = 10;
    *out = 0;
    switch (p6) {
    case 0x0a:
        D_dff9 = D_e001 = sub_s004_0e76_20f6(1, 2) == 1 ? 1L : 2L;
        break;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
        case 2:
        case 3:
            D_dff9 = D_e001 = sub_s004_0e76_20f6(1, 2) == 1 ? 1L : 2L;
            break;
        case 11:
            *out = 6;
            break;
        case 13:
            if (sub_s004_0e76_20f6(1, 2) == 1)
                D_dff9 = D_e001 = D_dffd = D_e005 = 11;
            *out = 6;
            break;
        case 14:
            D_dff9 = D_e001 = D_dffd = D_e005 = 11;
            *out = 6;
            break;
        }
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
        L0d1:
            D_dff9 = D_e001 = sub_s004_0e76_20f6(1, 2) == 1 ? 1L : 2L;
            break;
        case 2:
        case 6:
        case 7:
            break;
        case 8:
            D_dff9 = D_e001 = D_dffd = D_e005 = 11;
            break;
        case 12:
            D_dff9 = D_e001 = D_dffd = D_e005 = 3;
            *out = 1;
            break;
        default:
            *out = 6;
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 1:
            goto L0d1;
            break;
        case 2:
        case 3:
        case 4:
        case 9:
        case 13:
            break;
        case 8:
        case 12:
            D_dff9 = D_e001 = D_dffd = D_e005 = 11;
            *out = 6;
            break;
        default:
            if (sub_s004_0e76_20f6(1, 2) == 1)
                D_dff9 = D_e001 = D_dffd = D_e005 = 11;
            *out = 6;
            break;
        }
        break;
    case 0x6c:
        D_dff9 = D_e001 = D_dffd = D_e005 = 11;
        break;
    case 0x6d:
        D_dff9 = D_e001 = D_dffd = D_e005 = 3;
        break;
    }
}

/* *a, *b for location type p6, level lv */
void far sub_s009_182c_05a2(int p6, int lv, int *a, int *b)
{
    *b = 2;
    *a = 5;
    switch (p6) {
    case 0x0a:
        *b = 1;
        *a = 4;
        break;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
        case 2:
        case 3:
        case 12:
            *b = 1;
            *a = 4;
            break;
        }
        break;
    case 0x0d:
    case 0x83:
        *b = 1;
        switch (lv) {
        default:
            *a = 4;
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 1:
            *b = 1;
            *a = 4;
            break;
        }
        break;
    }
}

/* *a, *b, *c for location type p6, level lv */
void far sub_s009_182c_061a(int p6, int lv, int *a, int *b, int *c)
{
    *b = 6;
    *a = 1;
    *c = 8;
    switch (p6) {
    case 0x0a:
        *b = 4;
        break;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
        case 2:
        case 3:
            *b = 4;
            break;
        case 9:
        case 10:
            *b = 5;
            break;
        case 11:
            *a = *b = 3;
            break;
        case 13:
        case 14:
            *b = 4;
            *a = 3;
            break;
        }
        break;
    case 0x0c:
    case 0x7f:
    case 0x80:
    case 0x81:
        *b = 2;
        *a = 5;
        *c = 3;
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
            *b = 4;
            break;
        case 2:
            break;
        case 5:
        case 7:
            *b = 4;
            *a = 3;
            *c = 6;
            break;
        case 8:
            goto L6c;
        case 10:
            *b = 5;
            *c = 6;
            break;
        case 11:
            *a = *b = 3;
            *c = 12;
            break;
        case 12:
            goto L6d;
        default:
            *c = 6;
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:

        switch (lv) {
        case 1:
            *b = 4;
            break;
        case 2:
        case 3:
        case 4:
            break;
        case 6:
        case 10:
            *a = *b = 3;
            *c = 6;
            break;
        default:
            *b = 4;
            *a = 3;
            *c = 6;
            break;
        }
        break;
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
        *b = 2;
        *a = 4;
        *c = 10;
        break;
    case 0x6c:
    L6c:
        *b = 4;
        *a = 3;
        *c = 12;
        break;
    case 0x6d:
    L6d:
        *c = *b = 0;
        break;
    }
}

/* D_e009 and *out for location type p6, level lv */
void far sub_s009_182c_07d0(int p6, int lv, int *out)
{
    *out = 1;
    D_e009 = 1;
    switch (p6) {
    case 0x0a:
        switch (sub_s004_0e76_20f6(1, 3)) {
        case 2:
            D_e009 = 2;
            break;
        case 3:
            D_e009 = 3;
            *out = 2;
            break;
        }
        break;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
        case 2:
        case 3:
            switch (sub_s004_0e76_20f6(1, 3)) {
            case 2:
                D_e009 = 2;
                break;
            case 3:
                D_e009 = 3;
                *out = 2;
                break;
            }
            break;
        case 11:
        case 13:
        case 14:
            D_e009 = D_dff9 == 11 ? 9L : 10L;
            break;
        default:
            D_e009 = 5;
            *out = 4;
            break;
        }
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
            switch (sub_s004_0e76_20f6(1, 3)) {
            case 2:
                D_e009 = 2;
                break;
            case 3:
                D_e009 = 3;
                *out = 2;
                break;
            }
            break;
        case 8:
            D_e009 = 11;
            *out = 2;
            break;
        case 12:
            D_e009 = 10;
            break;
        default:
            D_e009 = 5;
            *out = 4;
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 1:
            switch (sub_s004_0e76_20f6(1, 3)) {
            case 2:
                D_e009 = 2;
                break;
            case 3:
                D_e009 = 3;
                *out = 2;
                break;
            }
            break;
        case 2:
        case 3:
        case 4:
            D_e009 = 5;
            *out = 4;
            break;
        case 9:
        case 13:
            D_e009 = 11;
            *out = 2;
            break;
        default:
            D_e009 = D_dff9 == 11 ? 9L : 10L;
            break;
        }
        break;
    case 0x6c:
        D_e009 = 11;
        *out = 2;
        break;
    case 0x6d:
        D_e009 = 10;
        break;
    }
}

/* *a, *b, *c for location type p6, level lv */
void far sub_s009_182c_0938(int p6, int lv, int *a, int *b, int *c)
{
    *c = *b = 0;
    switch (p6) {
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
        case 11:
            *a = 2;
            *b = 1;
            *c = 0x40;
            break;
        case 8:
            *a = 2;
            *b = 1;
            *c = 12;
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 9:
        case 13:
            *a = 2;
            *b = 1;
            *c = 12;
            break;
        }
        break;
    case 0x6c:
        *a = 2;
        *b = 1;
        *c = 12;
        break;
    }
}

#define S3(x, y) { *a = x; *b = y; *c = 3; return; }
#define S2(x, y) { *a = x; *b = y; return; }

/* monster kinds *a, *b (and count class *c) for location type p6, level lv, random 1..4 */
void far sub_s009_182c_09b4(int p6, int lv, int *a, int *b, int *c)
{
    int r;

    r = sub_s004_0e76_20f6(1, 4);
    switch (p6) {
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
        case 2:
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2: S3(0x18, 0x1e)
            case 3: S3(0x1d, 0x19)
            case 4: S3(0x1d, 0x1e)
            }
            break;
        case 3:
            switch (r) {
            case 1: S3(0x18, 0x1e)
            case 2: S2(0x88, 0x14)
            case 3: S2(0x8b, 0x14)
            case 4: S3(0x1d, 0x1e)
            }
            break;
        case 4:
            switch (r) {
            case 1: S3(0x1f, 0x1e)
            case 2: S3(0x1f, 0x28)
            case 3: S3(0x1f, 0x1e)
            case 4: S3(0x1f, 0x28)
            }
            break;
        case 5:
        case 6:
        case 7:
            switch (r) {
            case 1: S3(0x1f, 0x1e)
            case 2: S2(0x88, 0x14)
            case 3: S2(0x8b, 0x14)
            case 4: S3(0x1f, 0x1e)
            }
            break;
        case 8:
            switch (r) {
            case 1: S3(0x1f, 0x23)
            case 2: S2(0x88, 0x1e)
            case 3: S2(0x8b, 0x1e)
            case 4: S3(0x1f, 0x23)
            }
            break;
        case 9:
        case 10:
            switch (r) {
            case 1: S3(0x1f, 0x19)
            case 2: S3(0x1f, 0x2d)
            case 3: S3(0x1f, 0x19)
            case 4: S3(0x1f, 0x28)
            }
            break;
        case 11:
        case 14:
            S2(0x8b, 0x14)
        }
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2:
            case 3: S3(0x1d, 0x23)
            case 4: S2(0x8c, 0x14)
            }
            break;
        case 2:
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2:
            case 3: S3(0x1d, 0x1e)
            case 4: S2(0x8b, 0x14)
            }
            break;
        case 3:
        case 6:
        case 11:
            switch (r) {
            case 1: S3(0x1f, 0x19)
            case 2:
            case 3: S3(0x1f, 0x1e)
            case 4: S2(0x89, 0x14)
            }
            break;
        case 4:
            switch (r) {
            case 1: S3(0x1f, 0x1e)
            case 2:
            case 3: S3(0x1f, 0x23)
            case 4: S2(0x8c, 0x14)
            }
            break;
        case 5:
        case 10:
            switch (r) {
            case 1: S3(0x1f, 0x19)
            case 2:
            case 3: S3(0x1f, 0x1e)
            case 4: S2(0x8b, 0x14)
            }
            break;
        case 7:
            switch (r) {
            case 1: S3(0x1f, 0x19)
            case 2:
            case 3: S3(0x1f, 0x23)
            case 4: S2(0x8c, 0x14)
            }
            break;
        case 8:
            S2(0x8b, 0x14)
        case 12:
            switch (r) {
            case 1: S3(0x1e, 0x19)
            case 2:
            case 3: S3(0x1e, 0x23)
            case 4: S2(0x8c, 0x14)
            }
            break;
        }
        break;
    case 0x0e:
    case 0x84:
        if (lv == 1) {
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2:
            case 3: S3(0x1d, 0x1e)
            case 4: S3(0x18, 0x1e)
            }
        } else if (lv >= 2 && lv <= 13) {
            *a = 0x8c;
            switch (r) {
            case 1:
            case 2:
                *b = 0x14;
                return;
            case 3:
            case 4:
                *b = 0x1e;
                return;
            }
        }
        break;
    case 0x0f:
    case 0x85:
        if (lv == 1) {
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2:
            case 3: S3(0x1d, 0x1e)
            case 4: S3(0x18, 0x1e)
            }
        } else if (lv >= 2 && lv <= 13) {
            switch (r) {
            case 1: S2(0x8c, 0x14)
            case 2: S2(0x89, 0x14)
            case 3: S2(0x8b, 0x14)
            case 4: S2(0x88, 0x14)
            }
        }
        break;
    case 0x10:
    case 0x86:
        switch (lv) {
        case 1:
            switch (r) {
            case 1: S3(0x18, 0x19)
            case 2:
            case 3: S3(0x1d, 0x1e)
            case 4: S3(0x18, 0x1e)
            }
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            switch (r) {
            case 1: S3(0x18, 0x1e)
            case 2: S3(0x18, 0x2d)
            case 3: S3(0x1d, 0x1e)
            case 4: S3(0x1d, 0x2d)
            }
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            switch (r) {
            case 1: S3(0x1f, 0x1e)
            case 2: S3(0x1f, 0x2d)
            case 3: S3(0x1f, 0x1e)
            case 4: S3(0x1f, 0x2d)
            }
            break;
        case 10:
        case 11:
        case 12:
        case 13:
            switch (r) {
            case 1: S3(0x1e, 0x1e)
            case 2: S3(0x1e, 0x2d)
            case 3: S3(0x1e, 0x1e)
            case 4: S3(0x1e, 0x2d)
            }
            break;
        }
        break;
    default:
        switch (r) {
        case 2:
        case 3:
            S3(0x18, 0x19)
        default:
            S3(0x1c, 0x23)
        }
    }
}

/* fill group g (in level lv of buffer lvp) with monsters: special set pieces for some
 * levels (flag == 0), else up to 10 random kinds via sub_10a8; returns 0, 1 or 2 */
char far sub_s009_182c_0db6(char far *lvp, struct grp far *g, int p6, int lv, int flag)
{
    int idx[10];
    int a, b, k, n, i;

    if (!flag) {
        switch (p6) {
        case 0x0b:
        case 0x82:
            if (lv == 14) {
                sub_s009_1432_205e(lvp, g, 0xe4);
                return 2;
            }
            break;
        case 0x0e:
        case 0x84:
            switch (lv) {
            case 10:
                sub_s009_182c_1146(g, 0x74, 0x99, 0x2d, 3);
                sub_s009_182c_1146(g, 0x70, 0x95, 0x2d, 2);
                sub_s009_182c_1146(g, 0x66, 0x8b, 0x2d, 1);
                return 1;
            case 11:
                sub_s009_1432_205e(lvp, g, 0xe8);
                return 2;
            case 12:
                sub_s009_182c_1146(g, sub_s009_182c_0fe6(), 0x9f, 0x23, 1);
                return 1;
            }
            break;
        case 0x0f:
        case 0x85:
            switch (lv) {
            case 10:
                sub_s009_182c_1146(g, 0x73, 0x98, 0x2d, 3);
                sub_s009_182c_1146(g, 0x70, 0x95, 0x2d, 2);
                sub_s009_182c_1146(g, 0x66, 0x8b, 0x2d, 1);
                return 1;
            case 11:
                sub_s009_1432_205e(lvp, g, 0xe8);
                return 2;
            case 12:
                sub_s009_182c_1146(g, sub_s009_182c_0fe6(), 0x9f, 0x23, 1);
                return 1;
            }
            break;
        case 0x10:
        case 0x86:
            switch (lv) {
            case 10:
                sub_s009_182c_1146(g, 0x6d, 0x92, 0x2d, 3);
                sub_s009_182c_1146(g, 0x70, 0x95, 0x2d, 2);
                sub_s009_182c_1146(g, 0x66, 0x8b, 0x2d, 1);
                return 1;
            case 11:
                sub_s009_1432_205e(lvp, g, 0xe8);
                return 2;
            case 12:
                sub_s009_182c_1146(g, sub_s009_182c_0fe6(), 0x9f, 0x23, 1);
                return 1;
            }
            break;
        }
    }
    if (!D_e035 && sub_s004_0e76_20f6(1, 10) > 5)
        return 0;
    g->x10 = sub_s004_0e76_20f6(1, 10);
    for (n = 0; n < 10; n++)
        idx[n] = n;
    n = 10;
    while (g->n < 4) {
        k = sub_s004_0e76_20f6(1, n) - 1;
        switch (idx[k]) {
        case 0: a = 0x48; b = 0x6d; break;
        case 1: a = 0x4f; b = 0x74; break;
        case 2: a = 0x49; b = 0x6e; break;
        case 3: a = 0x4d; b = 0x72; break;
        case 4: a = 0x4e; b = 0x73; break;
        case 5: a = 0x50; b = 0x75; break;
        case 6: a = 0x57; b = 0x7c; break;
        case 7: a = 0x58; b = 0x7d; break;
        case 8: a = 0x59; b = 0x7e; break;
        case 9: a = 0x5a; b = 0x7f; break;
        }
        sub_s009_182c_10a8(g, p6, lv, a, b);
        idx[k] = idx[n];
        if (--n <= 0)
            break;
    }
    return 1;
}

/* random special monster kind 0x96..0xab */
int far sub_s009_182c_0fe6(void)
{
    switch (sub_s004_0e76_20f6(1, 22)) {
    case 1: return 0x96;
    case 2: return 0x97;
    case 3: return 0x98;
    case 4: return 0x99;
    case 5: return 0x9a;
    case 6: return 0x9b;
    case 7: return 0x9c;
    case 8: return 0x9d;
    case 9: return 0x9e;
    case 10: return 0x9f;
    case 11: return 0xa0;
    case 12: return 0xa1;
    case 13: return 0xa2;
    case 14: return 0xa3;
    case 15: return 0xa4;
    case 16: return 0xa5;
    case 17: return 0xa6;
    case 18: return 0xa7;
    case 19: return 0xa8;
    case 20: return 0xa9;
    case 21: return 0xaa;
    case 22: return 0xab;
    }
}

/* add a random number of monsters of kind `type` (x2 = x) to group g */
void far sub_s009_182c_10a8(struct grp far *g, int p6, int lv, int type, int x)
{
    int m, s, n;

    if (g->n >= 4)
        return;
    if (g->n == 0) {
        m = 1;
        s = 0;
    } else if (type == 0x48) {
        m = 10;
        s = 9;
    } else if (type == 0x49) {
        m = 4;
        s = 3;
    } else if (lv > 3 && (p6 == 0x0b || p6 == 0x0d)) {
        m = 8;
        s = 6;
    } else {
        m = 8;
        s = 7;
    }
    n = sub_s004_0e76_20f6(1, m) - s;
    if (n > 0)
        sub_s009_182c_1146(g, type, x, 0x23, sub_s004_0e76_20f6(1, 3) + n - 1);
}

/* append entry {type, x2, x3, n, D_db35[type].x25} to group g */
#pragma optimize("a", on)
void far sub_s009_182c_1146(struct grp far *g, int type, char x2, char x3, int n)
{
    struct ent far *e;

    if (g->n >= 4 || n <= 0)
        return;
    e = &g->ent[g->n++];
    e->type = type;
    e->x2 = x2;
    e->x3 = x3;
    e->n = n;
    e->x5 = D_db35[type].x25;
}
#pragma optimize("", on)

/* empty per-type/level switch (code removed in the original) */
void far sub_s009_182c_11b2(int p6, int lv)
{
    switch (p6) {
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1: case 2: case 3:
        case 6:
        case 11: case 12: case 13: case 14:
            break;
        }
        break;
    case 0x0c:
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
        case 11: case 12: case 13:
            break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 1:
        case 2: case 3: case 4:
        case 10: case 11: case 12: case 13:
            break;
        }
        break;
    case 0x6c:
    case 0x6d:
        break;
    }
}

#define R sub_s004_0e76_20f6

/* level layout parameters for location type p6, level lv, room kind k: *a kind, *f second kind,
 * *b, *c count, *d, *h, *e/*k flags (2.6 KB of nested switches, all arms written out) */
void far sub_s009_182c_1260(int p6, int lv, int k, int *a, int *b, int *c, int *d, char *e,
    int *f, int *g, int *h, int *i, char *j, char *l)
{
    *f = *a = -1;
    *g = *b = 1;
    *i = *h = *d = *c = 0;
    *l = *j = *e = 0;
    switch (p6) {
    case 0x0a:
        return;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1:
            switch (k) {
            case 0: *a = 0x3a; *c = R(1, 2) + 1; return;
            case 1: case 2: *a = 0x3a; *c = R(1, 2); return;
            case 3: *a = 0x3a; *c = 1; return;
            }
            break;
        case 2:
            switch (k) {
            case 0: case 1: *a = 0x3b; *c = R(1, 2); return;
            case 2: case 3: *a = 0x3b; *c = 1; return;
            }
            break;
        case 3:
            switch (k) {
            case 0: *a = 0x2c; *f = 0x30; *h = R(*c = 1, *b = 2) + 1; return;
            case 1: *a = 0x2c; *f = 0x30; *h = R(*c = 1, 2) + 1; return;
            case 2: case 3: *a = 0x30; *c = R(1, 2) + 1; return;
            case 10: *a = 0x2e; *f = 0x30; *h = R(*c = 1, 2) + 2; *l = 1; return;
            }
            break;
        case 4:
            switch (k) {
            case 0: *a = 0x39; *c = R(1, 2); return;
            case 1: case 2: case 3: *a = 0x39; *c = 1; return;
            }
            break;
        case 5:
            switch (k) {
            case 0: *a = 0x2c; *f = 0x30; *h = R(*c = 1, *b = 2) + 1; return;
            case 1: *a = 0x2c; *f = 0x30; *h = R(*c = 1, 2) + 1; return;
            case 2: case 3: *a = 0x30; *c = R(1, 2) + 1; return;
            }
            break;
        case 6:
            switch (k) {
            case 0: *f = *a = 0x2c; *h = R(*c = 1, *b = 2); return;
            case 1: *a = 0x2c; *c = R(1, 2) + 1; return;
            case 2: *f = 0x30; *h = R(1, 3) + 1; return;
            case 3: *a = 0x2c; *c = 1; return;
            }
            break;
        case 7:
            switch (k) {
            case 0: *a = 0x2c; *f = 0x2e; *h = *c = 1; return;
            case 1: *a = 0x2c; *b = 2; *c = 1; return;
            case 2: case 3: *a = 0x2c; *c = 1; return;
            }
            break;
        case 8:
            switch (k) {
            case 0: case 1: *f = *a = 0x2c; *h = R(1, *c = *b = 2) + 1; return;
            case 2: *a = 0x2c; *c = R(1, 3); return;
            case 3: *a = 0x2c; *c = R(1, 2); return;
            }
            break;
        case 9:
            switch (k) {
            case 0: *a = 0x39; *c = R(1, 2) + 1; return;
            case 1: case 2: case 3: *a = 0x39; *c = 1; return;
            }
            break;
        case 10:
            switch (k) {
            case 0: *a = 0x30; *c = R(1, 3) + 2; return;
            case 1: *a = 0x2e; *c = R(1, 2); return;
            case 2: case 3: *a = 0x2e; *c = 1; return;
            case 10: *a = 0x2e; *f = 0x30; *h = R(*c = 1, 2) + 2; *l = 1; return;
            }
            break;
        case 11:
        case 14:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x2e; *c = 1; return;
            }
            break;
        case 13:
            switch (k) {
            case 0: *a = 0x2e; *c = 1;
            case 1: *d = 0x4f7; *a = 0x2e; *c = 1; return;
            }
            break;
        }
        break;
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
            switch (k) {
            case 10: *a = 0x2c; *b = 1; *c = 3; *l = 1; return;
            case 0: case 1: case 2: case 3: *a = 0x3a; *c = R(1, 2); return;
            }
            break;
        case 2:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x39; *c = R(1, 2); return;
            }
            break;
        case 3:
            switch (k) {
            case 0: case 1: *a = 0x2e; *c = R(1, 2); return;
            case 2: case 3: *a = 0x2e; *c = 1; return;
            }
            break;
        case 4:
        case 5:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x3d; *c = 1; return;
            }
            break;
        case 6:
        case 10:
            switch (k) {
            case 0: case 1: case 2: *a = 0x2f; *c = R(1, 2); return;
            case 3: *a = 0x2f; *c = 1; return;
            }
            break;
        case 7:
            switch (k) {
            case 10: *a = 0x2c; *c = 3; *l = 1; return;
            case 0: case 1: *a = 0x3b; *c = R(1, 2); return;
            case 2: case 3: *a = 0x3b; *c = 1; return;
            }
            break;
        case 8:
            switch (k) {
            case 0: case 1: *f = *a = 0x2c; *h = R(*c = 1, *b = 2) + 1; return;
            case 2: *a = 0x2c; *c = R(1, 2) + 1; return;
            case 3: *b = 2; *f = *a = 0x2c; *h = *c = 1; return;
            }
            break;
        case 11:
            switch (k) {
            case 0: case 3: *a = 0x2e; *c = R(1, 2); return;
            case 1: case 2: *a = 0x3b; *c = R(1, 2); return;
            case 10: *a = 0x30; *c = 8; *l = 1; return;
            }
            break;
        case 12:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x30; *c = R(1, 2) + 6; return;
            }
            break;
        }
        break;
    case 0x0e:
    case 0x84:
        switch (lv) {
        case 1:
            switch (k) {
            case 0: *a = 0x2e; *c = R(1, 2); return;
            case 1: *a = 0x2e; *a = 0x30; *c = R(*c = 1, 2); return;
            case 2: *a = 0x30; *c = R(1, 2) + 2; return;
            case 3: *a = 0x30; *c = R(1, 2) + 1; return;
            }
            break;
        case 2:
        case 3:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x3d; *c = 1; return;
            }
            break;
        case 4:
            switch (k) {
            case 0: *a = 0x2e; *c = R(1, 3); return;
            case 1: case 2: case 3: *a = 0x2e; *c = R(1, 2); return;
            }
            break;
        case 5:
            switch (k) {
            case 0: *a = 0x31; *c = R(1, 2) + 1; return;
            case 1: case 2: case 3: *a = 0x31; *c = R(1, 2); return;
            }
            break;
        case 6:
            switch (k) {
            case 0: case 1: *a = 0x31; *c = R(1, 2); return;
            case 2: case 3: *a = 0x31; *c = 1; return;
            }
            break;
        case 7:
            switch (k) {
            case 0: case 1: case 2: *a = 0x2e; *c = R(1, 3); return;
            case 3: *a = 0x2e; *c = R(1, 2); return;
            }
            break;
        case 8:
            switch (k) {
            case 0: case 1: case 2: *a = 0x3d; *c = R(1, 2); return;
            case 3: *a = 0x3d; *c = 1; return;
            }
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            switch (k) {
            case 0: case 1: *a = 0x31; *c = 2; return;
            case 2: case 3: *a = 0x31; *c = 1; return;
            }
            break;
        case 13:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x31; *c = R(1, 2); return;
            case 4: *a = 0x31; *c = R(2, 2); *l = 1; return;
            }
            break;
        }
        break;
    case 0x0f:
    case 0x85:
        switch (lv) {
        case 1:
            switch (k) {
            case 0: *a = 0x2e; *c = R(1, 2); return;
            case 1: *a = 0x2e; *a = 0x30; *c = R(*c = 1, 2); return;
            case 2: *a = 0x30; *c = R(1, 2) + 2; return;
            case 3: *a = 0x30; *c = R(1, 2) + 1; return;
            }
            break;
        case 2:
        case 3:
        case 4:
        case 6:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x2f; *c = R(1, 2); return;
            }
            break;
        case 5:
        case 7:
        case 9:
            switch (k) {
            case 0: *a = 0x27; *f = 0x29; *h = R(*c = 1, 3); return;
            case 1: case 2: *a = 0x27; *f = 0x29; *h = R(*c = 1, 2); return;
            case 3: *a = 0x29; *c = R(1, 4); return;
            }
            break;
        case 8:
        case 10:
        case 12:
            switch (k) {
            case 0: case 1: case 2: *a = 0x2f; *c = R(1, 3); return;
            case 3: *a = 0x2f; *c = R(1, 2); return;
            }
            break;
        case 11:
            switch (k) {
            case 0: *a = 0x35; *c = R(1, 2); return;
            case 1: case 2: case 3: *a = 0x35; *c = 1; return;
            }
            break;
        case 13:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x35; *b = 2; *c = 1; return;
            case 4: *a = 0x35; *c = *b = 2; *l = 1; return;
            }
            break;
        }
        break;
    case 0x10:
    case 0x86:
        switch (lv) {
        case 1:
            switch (k) {
            case 0: *a = 0x2e; *c = R(1, 2); return;
            case 1: *a = 0x2e; *a = 0x30; *c = R(*c = 1, 2); return;
            case 2: *a = 0x30; *c = R(1, 2) + 2; return;
            case 3: *a = 0x30; *c = R(1, 2) + 1; return;
            }
            break;
        case 2:
        case 6:
            switch (k) {
            case 0: case 1: *a = 0x3b; *c = R(1, 2); return;
            case 2: case 3: *a = 0x3b; *c = 1; return;
            }
            break;
        case 3:
            switch (k) {
            case 0: case 1: case 2: case 3: *a = 0x39; *c = R(1, 2); return;
            }
            break;
        case 4:
            switch (k) {
            case 0: case 1: *a = 0x2d; *c = R(1, 2) + 1; return;
            case 2: case 3: *a = 0x2d; *c = R(1, 2); return;
            }
            break;
        case 5:
        case 7:
        case 8:
        case 9:
            switch (k) {
            case 0: *c = R(1, *b = 2); *f = *a = 0x2d; *h = R(1, 2); return;
            case 1: *b = 2; *f = *a = 0x2d; *h = *c = 1; return;
            case 2: case 3: *c = R(1, *b = 2) - 1; *f = *a = 0x2d; *h = 1; return;
            }
            break;
        case 10:
        case 11:
        case 12:
            switch (k) {
            case 0: *a = 0x2d; *c = R(1, *b = 2) + 1; return;
            case 1: *a = 0x2d; *c = *b = 2; return;
            case 2: case 3: *a = 0x2d; *c = R(1, *b = 2); return;
            }
            break;
        case 13:
            switch (k) {
            case 0: case 1: case 2: case 3: *f = *a = 0x2d; *h = R(*c = 1, *b = 2) - 1; return;
            case 4: *c = R(1, *b = 2); *f = *a = 0x2d; *h = R(1, 2); *l = 1; return;
            }
            break;
        }
        break;
    case 0x6c:
        switch (k) {
        case 0: case 1: case 2: case 3: case 4: *a = 0x2c; *c = 1; return;
        case 5: *a = 0x33; *c = 1; *e = 1; return;
        }
        break;
    case 0x6d:
        switch (k) {
        case 0: *a = 0x30; *c = 7; *d = 0xdd; return;
        case 1: *a = 0x19; *b = 2; *c = 1; *e = 1; return;
        }
        break;
    }
}
#undef R

/* random special item/monster for location type p6, level lv (default 0x5c) */
int far sub_s009_182c_1cb4(int p6, int lv)
{
    int v, r;

    v = 0x5c;
    if (lv < 0 || lv >= 16)
        return v;
    switch (p6) {
    case 0x0b:
    case 0x82:
        r = sub_s004_0e76_20f6(1, 12);
        switch (lv) {
        case 3:
            switch (r) {
            case 1: v = 0x5c; break;
            case 2: v = 0x71; break;
            case 3: v = 0xc5; break;
            case 4: v = 0xc6; break;
            case 5: v = 0xc7; break;
            case 6: v = 0xc8; break;
            case 7: v = 0xc9; break;
            case 8: v = 0xca; break;
            case 9: v = 0xcb; break;
            case 10: v = 0xcc; break;
            case 11: v = 0xcd; break;
            case 12: v = 0xce; break;
            }
            break;
        case 10:
            switch (r) {
            case 1: v = 0xcf; break;
            case 2: v = 0xd0; break;
            case 3: v = 0xd1; break;
            case 4: v = 0xd2; break;
            case 5: v = 0xd3; break;
            case 6: v = 0xd4; break;
            case 7: v = 0xd5; break;
            case 8: v = 0xd6; break;
            case 9: v = 0xd7; break;
            case 10: v = 0xd8; break;
            case 11: v = 0xd9; break;
            case 12: v = 0xda; break;
            }
            break;
        case 8:
            switch (r) {
            case 1: v = 0xdb; break;
            case 2: v = 0xdc; break;
            case 3: v = 0xdd; break;
            case 4: v = 0xde; break;
            case 5: v = 0xdf; break;
            case 6: v = 0xe0; break;
            case 7: v = 0xe0; break;
            case 8: v = 0xe2; break;
            case 9: return v = 0x153;
            case 10: return v = 0x154;
            case 11: return v = 0x155;
            case 12: return v = 0x156;
            }
            break;
        }
        break;
    case 0x0d:
    case 0x83:
        r = sub_s004_0e76_20f6(1, 18);
        switch (lv) {
        case 1:
            switch (r) {
            case 1: v = 0x5c; break;
            case 2: v = 0x71; break;
            case 3: v = 0xc5; break;
            case 4: v = 0xc6; break;
            case 5: v = 0xc7; break;
            case 6: v = 0xc8; break;
            case 7: v = 0xc9; break;
            case 8: v = 0xca; break;
            case 9: v = 0xcb; break;
            case 10: v = 0xcc; break;
            case 11: v = 0xcd; break;
            case 12: v = 0xce; break;
            case 13: v = 0xcf; break;
            case 14: v = 0xd0; break;
            case 15: v = 0xd1; break;
            case 16: v = 0xd2; break;
            case 17: v = 0xd3; break;
            case 18: v = 0xd4; break;
            }
            break;
        case 2:
            switch (r) {
            case 1: v = 0xd5; break;
            case 2: v = 0xd6; break;
            case 3: v = 0xd7; break;
            case 4: v = 0xd8; break;
            case 5: v = 0xd9; break;
            case 6: v = 0xda; break;
            case 7: v = 0xdb; break;
            case 8: v = 0xdc; break;
            case 9: v = 0xdd; break;
            case 10: v = 0xde; break;
            case 11: v = 0xdf; break;
            case 12: v = 0xe0; break;
            case 13: v = 0xe2; break;
            case 14: v = 0xe2; break;
            case 15: return v = 0x153;
            case 16: return v = 0x154;
            case 17: return v = 0x155;
            case 18: return v = 0x156;
            }
            break;
        case 7: v = 0xec; break;
        case 8: v = 0xee; break;
        case 11: v = 0xed; break;
        case 12: v = 0xef; break;
        }
        break;
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        r = sub_s004_0e76_20f6(1, 7);
        switch (lv) {
        case 1:
            switch (r) {
            case 1: v = 0x5c; break;
            case 2: v = 0x71; break;
            case 3: v = 0xc5; break;
            case 4: v = 0xc6; break;
            case 5: v = 0xc7; break;
            case 6: v = 0xc8; break;
            case 7: v = 0xc9; break;
            }
            break;
        case 2:
            switch (r) {
            case 1: v = 0xca; break;
            case 2: v = 0xcb; break;
            case 3: v = 0xcc; break;
            case 4: v = 0xcd; break;
            case 5: v = 0xce; break;
            case 6: v = 0xcf; break;
            case 7: v = 0xd0; break;
            }
            break;
        case 3:
            switch (r) {
            case 1: v = 0xd1; break;
            case 2: v = 0xd2; break;
            case 3: v = 0xd3; break;
            case 4: v = 0xd4; break;
            case 5: v = 0xd5; break;
            case 6: v = 0xd6; break;
            case 7: v = 0xd7; break;
            }
            break;
        case 4:
            switch (r) {
            case 1: v = 0xd8; break;
            case 2: v = 0xd9; break;
            case 3: v = 0xda; break;
            case 4: v = 0xdb; break;
            case 5: v = 0xdc; break;
            case 6: v = 0xdd; break;
            case 7: v = 0xde; break;
            }
            break;
        case 5:
            switch (sub_s004_0e76_20f6(1, 8)) {
            case 1: v = 0xdf; break;
            case 2: v = 0xe0; break;
            case 3: return v = 0x153;
            case 4: v = 0xe2; break;
            case 5: return v = 0x153;
            case 6: return v = 0x154;
            case 7: return v = 0x155;
            case 8: return v = 0x156;
            }
            break;
        default:
            return v;
        }
        break;
    }
    return v;
}

/* class of the monster groups for location type p6, level lv */
int far sub_s009_182c_20a4(int p6, int lv)
{
    switch (p6) {
    default:
        return 0;
    case 0x0b:
    case 0x82:
        switch (lv) {
        case 1: case 2: case 3: case 11: case 13: case 14:
            return 12;
        default:
            return 6;
        case 12:
            return 5;
        }
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1: case 8: case 9: case 12:
            return 12;
        case 2:
            return 6;
        default:
            return 8;
        }
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 2: case 3: case 4:
            return 6;
        }
        return 12;
    case 0x0a:
    case 0x0c:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6c:
    case 0x6d:
    case 0x7f:
    case 0x80:
    case 0x81:
        return 12;
    }
}

/* random monster group kind (2..15) for location type p6, level lv; 0 = none */
int far sub_s009_182c_219e(int p6, int lv)
{
    switch (p6) {
    case 0x0a:
        switch (sub_s004_0e76_20f6(1, 13)) {
        case 1: case 2: return 3;
        case 3: case 4: return 2;
        case 5: case 6: return 11;
        case 7: case 8: case 9: return 4;
        case 10: case 11: return 12;
        case 12: return 13;
        case 13: return 14;
        }
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
        switch (sub_s004_0e76_20f6(1, 9)) {
        case 1: case 2: return 11;
        case 3: case 4: case 5: return 4;
        case 6: case 7: return 12;
        case 8: return 13;
        case 9: return 14;
        }
    case 0x0b:
    case 0x82:
        switch (lv) {
        default:
            switch (sub_s004_0e76_20f6(1, 10)) {
            case 1: return 2;
            case 2: return 3;
            case 3: case 4: return 11;
            case 5: case 6: return 4;
            case 7: case 8: return 12;
            case 9: return 13;
            case 10: return 14;
            }
        case 1:
        case 2:
        case 3:
            switch (sub_s004_0e76_20f6(1, 2)) {
            case 1: return 2;
            case 2: return 3;
            }
        case 8:
        case 10:
            switch (sub_s004_0e76_20f6(1, 9)) {
            case 1: case 2: return 11;
            case 3: case 4: case 5: return 4;
            case 6: case 7: return 12;
            case 8: return 13;
            case 9: return 14;
            }
        case 11:
            switch (sub_s004_0e76_20f6(1, 9)) {
            case 1: return 7;
            case 2: return 15;
            case 3: return 11;
            case 4: case 5: case 6: return 4;
            case 7: return 12;
            case 8: return 13;
            case 9: return 14;
            }
        case 13:
            switch (sub_s004_0e76_20f6(1, 10)) {
            case 1: return 8;
            case 2: return 7;
            case 3: return 15;
            case 4: return 11;
            case 5: case 6: case 7: return 4;
            case 8: return 12;
            case 9: return 13;
            case 10: return 14;
            }
        case 14:
            switch (sub_s004_0e76_20f6(1, 9)) {
            case 1: return 8;
            case 2: return 7;
            case 3: return 11;
            case 4: case 5: case 6: return 4;
            case 7: return 12;
            case 8: return 13;
            case 9: return 14;
            }
        }
        break;
    case 0x0c:
    case 0x7f:
    case 0x80:
    case 0x81:
        switch (sub_s004_0e76_20f6(1, 10)) {
        case 1: return 15;
        case 2: case 3: return 11;
        case 4: case 5: case 6: return 4;
        case 7: case 8: return 12;
        case 9: return 13;
        case 10: return 14;
        }
    case 0x0d:
    case 0x83:
        switch (lv) {
        case 1:
            switch (sub_s004_0e76_20f6(1, 2)) {
            case 1: return 2;
            case 2: return 3;
            }
        case 2:
            switch (sub_s004_0e76_20f6(1, 11)) {
            case 1: case 2: return 2;
            case 3: case 4: return 3;
            case 5: return 11;
            case 6: case 7: case 8: return 4;
            case 9: return 12;
            case 10: return 13;
            case 11: return 14;
            }
        case 6:
        case 7:
            switch (sub_s004_0e76_20f6(1, 12)) {
            case 1: return 8;
            case 2: return 7;
            case 3: return 2;
            case 4: return 3;
            case 5: case 6: return 11;
            case 7: case 8: return 4;
            case 9: case 10: return 12;
            case 11: return 13;
            case 12: return 14;
            }
        case 10:
        case 11:
            switch (sub_s004_0e76_20f6(1, 14)) {
            case 1: return 7;
            case 2: return 15;
            case 3: return 2;
            case 4: return 3;
            case 5: case 6: return 11;
            case 7: case 8: return 4;
            case 9: case 10: return 12;
            case 11: return 13;
            case 12: return 14;
            }
        case 8:
            return 8;
        case 12:
            switch (sub_s004_0e76_20f6(1, 2)) {
            case 1: return 15;
            case 2: return 7;
            }
        default:
            switch (sub_s004_0e76_20f6(1, 14)) {
            case 1: return 8;
            case 2: return 7;
            case 3: return 15;
            case 4: return 2;
            case 5: return 3;
            case 6: case 7: return 11;
            case 8: case 9: case 10: return 4;
            case 11: case 12: return 12;
            case 13: return 13;
            case 14: return 14;
            }
        }
        break;
    case 0x6c:
        return 8;
    case 0x6d:
        switch (sub_s004_0e76_20f6(1, 2)) {
        case 1: return 15;
        case 2: return 7;
        }
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x84:
    case 0x85:
    case 0x86:
        switch (lv) {
        case 1:
            switch (sub_s004_0e76_20f6(1, 2)) {
            case 1: return 2;
            case 2: return 3;
            }
        case 2:
        case 3:
        case 4:
            switch (sub_s004_0e76_20f6(1, 13)) {
            case 1: case 2: return 2;
            case 3: case 4: return 3;
            case 5: case 6: return 11;
            case 7: case 8: case 9: return 4;
            case 10: case 11: return 12;
            case 12: return 13;
            case 13: return 14;
            }
        case 6:
        case 10:
            switch (sub_s004_0e76_20f6(1, 8)) {
            case 1: return 7;
            case 2: return 15;
            case 3: return 11;
            case 4: case 5: return 4;
            case 6: return 12;
            case 7: return 13;
            case 8: return 14;
            }
        default:
            switch (sub_s004_0e76_20f6(1, 10)) {
            case 1: return 8;
            case 2: return 7;
            case 3: return 15;
            case 4: return 11;
            case 5: case 6: case 7: return 4;
            case 8: return 12;
            case 9: return 13;
            case 10: return 14;
            }
        }
        break;
    }
    return 0;
}
