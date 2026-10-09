/* place: s017 1ef8:0000 */
/* orig: size=0b9a CONST=7dc4+0022 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 0/1, ALL functions written (functional draft from the original code, not matched).
 * Combat dungeon view (overlay s017, child of s014): 0000 redraws the 14 x 10 cell window of the
 * level floor around combatant who (who < 0: keep the last centre D_c866/D_c886). For each view
 * slot k (dx = D_1fc6[k], dy = D_2052[k] relative to the centre cell) it decodes the cell long
 * (byte 0: low nibble wall E side, high nibble door art; byte 1: wall S / furniture high
 * nibble; byte 2: wall N / object; byte 3: wall W + door state bits 4..5, bit 7 hidden) and the
 * floor byte (20a5:1900, low/high nibble = floor / ceiling art), chooses wall corner sprites from
 * the neighbouring cells, draws them with sub_root_047c_04ef and records what it drew in the
 * view records (20a5:4eb8, 18 bytes, 140 slots) for hit testing. Dropped objects (20a5:59cb,
 * 12-byte records, count D_c80e) of the current level are drawn on their cell. D_e22f is
 * cleared when a sprite was missing (D_e230 = 1: "all sprites loaded" check), and the
 * D_e178/D_e13d/D_e196 lists (max 15) collect animated walls/furniture/doors. */

struct vrec { int wn, we, ws, wsd, wee, door, wsn, fl, ce; };   /* 18 bytes */
struct obj { char lvl; int x, y, on; char dx, dy; char pad[4]; };   /* 12 bytes at 20a5:59cb */
struct cmb { char pad0[4]; int x, y; char pad8[0x78]; };

extern struct { long c[40][40]; } far F_20a5_0000;          /* floor cells */
extern struct { unsigned char b[40][40]; } far F_20a5_1900;  /* floor / ceiling art */
extern struct { struct vrec r[140]; } far F_20a5_4eb8;
extern struct { int v[140]; } far F_20a5_5890;               /* cell code (x << 8 | y) per slot */
extern int far F_20a5_589[];                                 /* same table, as array */
extern struct { char v[140]; } far F_20a5_363c;              /* cell x per slot (-1 none) */
extern struct { char v[140]; } far F_20a5_36c8;              /* cell y per slot */
extern struct { int v[140]; } far F_20a5_45be;               /* screen x of N/S walls */
extern struct { int v[140]; } far F_20a5_46d6;               /* screen y */
extern struct { int v[140]; } far F_20a5_4b4a;               /* screen x of floor art */
extern struct { int v[140]; } far F_20a5_415e;               /* screen y of floor art */
extern struct { int v[140]; } far F_20a5_4046;               /* screen x of E walls */
extern struct { int v[140]; } far F_20a5_491a;               /* screen x of W walls */
extern struct { int v[140]; } far F_20a5_4a32;               /* screen y of W walls */
extern struct { int v[16][2]; } far F_20a5_6eea;             /* object sprites (byte 2 hi) */
extern struct { int v[16][2]; } far F_20a5_6f2a;             /* furniture sprites */
extern struct { int v[16][2]; } far F_20a5_3474;             /* ceiling sprites */
extern struct { struct obj o[64]; } far F_20a5_59cb;

extern struct cmb D_9c55[];
extern unsigned D_a696[];
extern char D_e22f, D_e230, D_e15b, D_e15c, D_e15d, D_07d3, D_11ca, D_c866, D_c886;
extern int D_e233, D_e235, D_c26c, D_c293, D_c869, D_11c8, D_e160, D_e0ab;
extern unsigned char D_c80e, D_c12d, D_c12e, D_c17f;
extern char D_1fc6[], D_2052[];
extern int D_c220[], D_c601[], D_c24c[], D_c4dd[], D_c29b[], D_c658[], D_c26f[], D_c62c[];
extern int D_c1d0[], D_c8d5[];
extern char D_c76e[], D_c789[], D_c821[], D_c855[], D_c13f[], D_c181[];
extern char D_e178[][2], D_e13d[][2], D_e196[][2];
extern unsigned char *D_0154, *D_0168, *D_017c, *D_0190, *D_01a4;

void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_047c_0952(unsigned char *, int, int, int, int, int);
void far sub_root_047c_04ef(unsigned char *, int, int, int);
void far sub_root_047c_059e(int);
int far sub_root_047c_053e(int);
int far sub_root_0410_02e6(int far *, int, int);
void far sub_root_0410_0068(int, int, int);
void far sub_s014_14f7_3404(void);

/* low nibble of a neighbour byte, 0 when the neighbour cell is hidden (negative) or empty */
#define NB(l, sh) ((l) > 0 ? (unsigned char)((l) >> (sh)) & 0x0f : 0)

void far sub_s017_1ef8_0000(int who)
{
    long c, n, s, w, e, sw, ne, nw;
    int i, row, col, k, x, y, spr, spr2, d, t;
    unsigned char b0, b1, b2, b3, fl, ce, obj, furn, dr, s1;
    unsigned char nw0, w0, sw0, e1, s1b, nw3, ne3, n3, e2, s2;

    D_e22f = 1;
    D_07d3++;
    sub_root_047c_12c3();
    for (i = 0; i < 5; i++)
        D_a696[i] |= 8;
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
    for (i = 0; i < 5; i++)
        D_a696[i] &= ~8;
    D_e15c = D_e15b = D_e15d = 0;
    if (who >= 0) {
        D_e233 = D_9c55[who].x >> 5;
        D_e235 = D_9c55[who].y >> 4;
        D_c866 = D_e233;
        D_c886 = D_e235;
    } else {
        D_e233 = D_c866;
        D_e235 = D_c886;
    }
    D_c26c = D_e233 << 5;
    D_c293 = D_e235 << 4;
    sub_root_047c_0952(D_0154, 0, 0, 0x104, 200, 0);
    for (row = 0; row < 14; row++)
        for (col = 0; col < 10; col++) {
            k = row * 10 + col;
            x = D_1fc6[k] + D_e233;
            y = D_2052[k] + D_e235;
            F_20a5_5890.v[k] = x * 256 + y;
            c = F_20a5_0000.c[x][y];
            n = F_20a5_0000.c[x][y - 1];
            s = F_20a5_0000.c[x][y + 1];
            w = F_20a5_0000.c[x - 1][y];
            e = F_20a5_0000.c[x + 1][y];
            sw = F_20a5_0000.c[x - 1][y + 1];
            ne = F_20a5_0000.c[x + 1][y - 1];
            nw = F_20a5_0000.c[x - 1][y - 1];
            F_20a5_4eb8.r[k].wn = -1;
            F_20a5_4eb8.r[k].wsn = 0;
            F_20a5_4eb8.r[k].wee = 0;
            F_20a5_4eb8.r[k].ws = 0;
            F_20a5_4eb8.r[k].door = 0;
            F_20a5_4eb8.r[k].wsd = 0;
            F_20a5_4eb8.r[k].we = 0;
            F_20a5_36c8.v[k] = -1;
            F_20a5_363c.v[k] = -1;
            if (x <= 0 || x >= 39 || y <= 0 || y >= 39 || c < 0)
                continue;
            F_20a5_363c.v[k] = x;
            F_20a5_36c8.v[k] = y;
            b0 = (unsigned char)c;
            b1 = (unsigned char)(c >> 8);
            dr = b1 & 0x0f;
            furn = b1 >> 4;
            b2 = (unsigned char)(c >> 16) & 0x0f;
            obj = (unsigned char)(c >> 20) & 0x0f;
            b3 = (unsigned char)(c >> 24);
            fl = F_20a5_1900.b[x][y];
            ce = fl >> 4;
            fl &= 0x0f;
            s1 = (b0 >> 4) & 0x0f;
            nw0 = NB(nw, 0);
            w0 = NB(w, 0);
            sw0 = NB(sw, 0);
            e1 = NB(e, 8);
            s1b = NB(s, 8);
            nw3 = NB(nw, 24);
            ne3 = NB(ne, 24);
            n3 = NB(n, 24);
            e2 = NB(e, 16);
            s2 = NB(s, 16);
            /* north wall */
            spr = -1;
            if (b2) {
                spr = D_c220[b2];
                if (n <= 0) {
                    if (nw0 || (w > 0 && ((c & 0xf00) || w0)))
                        spr = D_c601[b2];
                    if ((c & 0x0f) || e1 || (ne > 0 && e > 0 && (ne3 || e2))) {
                        if (D_c601[b2] == spr)
                            spr = D_c24c[b2];
                        else
                            spr = D_c4dd[b2];
                    }
                } else {
                    spr = 0;
                    obj = 0;
                }
            }
            if (spr == 0) {
                if (D_e230 == 1)
                    D_e22f = 0;
            } else if (spr == -1)
                spr = 0;
            if (spr == 0)
                F_20a5_4eb8.r[k].wn = 0;
            else {
                sub_root_047c_04ef(D_0154, F_20a5_45be.v[k], F_20a5_46d6.v[k], spr);
                F_20a5_4eb8.r[k].wn = spr;
            }
            /* object standing on the north wall */
            if (obj) {
                if (F_20a5_6eea.v[obj][0] == 0 && D_e230 == 1)
                    D_e22f = 0;
                F_20a5_4eb8.r[k].we = obj;
                sub_root_047c_04ef(D_0154, F_20a5_45be.v[k] + D_c76e[obj],
                                   D_c789[obj] + F_20a5_46d6.v[k], F_20a5_6eea.v[obj][0]);
                if (obj < D_c12e && D_e15b < 15) {
                    D_e178[D_e15b][0] = k;
                    D_e178[D_e15b][1] = obj;
                    D_e15b++;
                }
            }
            /* floor and ceiling art */
            if (ce == 0)
                F_20a5_4eb8.r[k].fl = 0;
            else {
                if (D_c1d0[ce] == 0 && D_e230 == 1)
                    D_e22f = 0;
                sub_root_047c_04ef(D_0154, F_20a5_4b4a.v[k], F_20a5_415e.v[k], D_c1d0[ce]);
                F_20a5_4eb8.r[k].fl = D_c1d0[ce];
            }
            if (fl == 0)
                F_20a5_4eb8.r[k].ce = 0;
            else {
                if (F_20a5_3474.v[fl][0] == 0 && D_e230 == 1)
                    D_e22f = 0;
                sub_root_047c_04ef(D_0154, F_20a5_4b4a.v[k], F_20a5_415e.v[k], F_20a5_3474.v[fl][0]);
                F_20a5_4eb8.r[k].ce = fl;
                if (fl < D_c12d && D_e15d < 15) {
                    D_e13d[D_e15d][0] = k;
                    D_e13d[D_e15d][1] = fl;
                    D_e15d++;
                }
            }
            /* dropped objects on this cell */
            for (i = 0; i < D_c80e; i++)
                if (sub_root_0410_02e6(F_20a5_589, (F_20a5_59cb.o[i].x & 0xffe0) * 8
                                       + (F_20a5_59cb.o[i].y >> 4), 0x8c) == k
                        && F_20a5_59cb.o[i].lvl == D_11ca) {
                    t = F_20a5_59cb.o[i].y - D_c293;
                    if (F_20a5_59cb.o[i].on == 1)
                        sub_root_047c_04ef(D_0154,
                                           F_20a5_59cb.o[i].x - D_c26c - t - F_20a5_59cb.o[i].dx + 0x88,
                                           t - F_20a5_59cb.o[i].dy + 0x69, D_c869);
                }
            /* south wall */
            spr2 = -1;
            if (c & 0xf00) {
                spr2 = D_c29b[dr];
                if (w <= 0) {
                    if ((c & 0xf000000L) || s2 || sw0 || (s > 0 && sw > 0 && s1b))
                        spr2 = D_c658[dr];
                    if (nw3 || (n > 0 && (b2 || n3))) {
                        if (D_c658[dr] == spr2)
                            spr2 = D_c26f[dr];
                        else
                            spr2 = D_c62c[dr];
                    }
                } else {
                    spr2 = 0;
                    furn = 0;
                }
            }
            if (spr2 == 0) {
                if (D_e230 == 1)
                    D_e22f = 0;
            } else if (spr2 == -1)
                spr2 = 0;
            if (spr2 == 0)
                F_20a5_4eb8.r[k].ws = 0;
            else {
                sub_root_047c_04ef(D_0154, F_20a5_4b4a.v[k], F_20a5_46d6.v[k], spr2);
                F_20a5_4eb8.r[k].ws = spr2;
            }
            if (furn) {
                if (F_20a5_6f2a.v[furn][0] == 0 && D_e230 == 1)
                    D_e22f = 0;
                F_20a5_4eb8.r[k].wsd = furn;
                sub_root_047c_04ef(D_0154, F_20a5_4b4a.v[k] + D_c821[furn],
                                   D_c855[furn] + F_20a5_46d6.v[k], F_20a5_6f2a.v[furn][0]);
                if (furn < D_c17f && D_e15c < 15) {
                    D_e196[D_e15c][0] = k;
                    D_e196[D_e15c][1] = furn;
                    D_e15c++;
                }
            }
            /* door art (byte 0 high nibble, state in byte 3 bits 4..5) */
            if (s1) {
                if (D_c8d5[s1] == 0 && D_e230 == 1)
                    D_e22f = 0;
                d = ((char)b3 >> 4) & 3;
                F_20a5_4eb8.r[k].door = d * 16 + s1;
                t = s1 * 4 + d;
                sub_root_047c_04ef(D_0154, F_20a5_4b4a.v[k] + D_c13f[t],
                                   D_c181[t] + F_20a5_415e.v[k], D_c8d5[s1]);
            }
            /* east and west walls */
            if ((c & 0x0f) == 0 && e1 == 0)
                F_20a5_4eb8.r[k].wee = 0;
            else {
                if (D_c26f[b0 & 0x0f] == 0 && D_e230 == 1)
                    D_e22f = 0;
                sub_root_047c_04ef(D_0154, F_20a5_4046.v[k], F_20a5_46d6.v[k], D_c26f[b0 & 0x0f]);
                F_20a5_4eb8.r[k].wee = D_c26f[b0 & 0x0f];
            }
            if ((c & 0xf000000L) == 0 && s2 == 0)
                F_20a5_4eb8.r[k].wsn = 0;
            else {
                if (D_c24c[b3 & 0x0f] == 0 && D_e230 == 1)
                    D_e22f = 0;
                sub_root_047c_04ef(D_0154, F_20a5_491a.v[k], F_20a5_4a32.v[k], D_c24c[b3 & 0x0f]);
                F_20a5_4eb8.r[k].wsn = D_c24c[b3 & 0x0f];
            }
        }
    if (D_e22f == 0)
        D_e230 = 0;
    sub_s014_14f7_3404();
    sub_root_047c_059e(D_11c8);
    if (sub_root_047c_053e(0)) {
        D_11c8 ^= 1;
        *D_0154 ^= 1;
        *D_01a4 ^= 1;
        *D_0168 ^= 1;
        *D_0190 ^= 1;
        *D_017c ^= 1;
    }
    sub_root_047c_1293();
    D_e160 = 1;
    D_e0ab = 1;
    sub_root_0410_0068(0xc897, 0xd8f1, 8);
    D_07d3--;
}
