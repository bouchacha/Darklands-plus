/* place: s024 1ef8:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 0/4, all functions written (functional drafts; 08f2 closest: one extra frame slot).
 * Combat level data (overlay s024, child of s014):
 * 0000 debug dump of every level's activation (.atv) and floor (.flr) records to tac.txt,
 *      then relocates the loaded floor of the current level (wall lists, trap handlers),
 * 059a switch to level n: save the current level's .flr/.atv if dirty (D_d4e4), load the new
 *      ones, mark the walls of opened doors (bit mask F_2e38_0bb6[level]),
 * 07a2 load "<name>.frc" (furniture/animation set) for the level,
 * 08f2 place combatant `who` around an activation spot (13 offsets per direction, then random
 *      tries, clearing the 3x3 floor cells after 500 tries).
 * Far data: segment 20a5 holds the current level's floor (.flr image, 0x33fc bytes at 20a5:0000)
 * and the activation records (0x1e bytes at 20a5:47ee); 2e38 holds per-level door masks. */

typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;

/* activation record (level%d.atv), 0x1e bytes */
struct atv {
    int strtx, strty, sprd;     /* +00 */
    char rm;                    /* +06 */
    char reenf;                 /* +07 AllowReenf */
    char type[3];               /* +08 */
    char num[3];                /* +0b */
    int f, g, pf;               /* +0e */
    char rtype, rsprd;          /* +14 */
    int rx, ry;                 /* +16 */
    char rrate, rmax, rdirn;    /* +1a */
    char pad;
};
struct atvtab { struct atv r[30]; };

struct item6 { char b[6]; };
struct trig { char n; unsigned char idx; char pad[7]; struct item6 it[5]; char pad2[5]; };   /* 0x2c */
struct xy10 { int x, y; char pad[6]; };
struct handler { union { int type; void (far *fn)(void); } u; char pad[10]; };    /* 0x0e */

/* level floor image (level%d.flr), lives at 20a5:0000 */
struct flr {
    char pad0[0x1f42];
    int v, s, r, d, e;          /* +1f42 */
    int f1f4c, f1f4e;           /* +1f4c */
    char f1f50;                 /* +1f50 */
    char door;                  /* +1f51 */
    char pad1f52[0x2184 - 0x1f52];
    char nhandler;              /* +2184 */
    char pad2185[7];
    struct handler h[50];       /* +218c */
    char pad[0x2488 - 0x218c - 50 * 0x0e];
    char nlist;                 /* +2488 */
    char pad2489;
    int list[64];               /* +248a */
    char pad250a[0x300c - 0x250a];
    char ntrig;                 /* +300c */
    char pad300d[0x0c];
    struct trig t[8];           /* +3019 */
    char pad30[0x3356 - 0x3019 - 8 * 0x2c];
    struct xy10 furn[5];        /* +3356 */
    struct xy10 wall[7];        /* +3388 */
    char pad33ce[3];
    char f33d1;                 /* +33d1 */
    char f33d2;                 /* +33d2 */
};
struct cell { char b0, b1, b2, b3; };
struct frc {
    int a[4];                   /* +00 */
    int b[4];                   /* +08 */
    int c[4];                   /* +10 */
    int d[4];                   /* +18 */
    char f20, f21;
    int size;                   /* +22 */
    int f24;                    /* +24 */
    int off;                    /* +26 */
};
struct cmb {
    char pad0[4];
    int x, y;                   /* +04 */
    char pad8[0x11];
    char f19, f1a;              /* +19 */
    char pad1b[0x65];
};
struct spot { char pad[8]; int x, y; unsigned char dir; };

extern struct atvtab far F_20a5_47ee;
extern struct flr far F_20a5_0000;
extern struct { long m[16]; } far F_2e38_0bb6;
extern struct { int w[1]; } far F_20a5_000;
extern struct cell far F_20a5_0[][40];
extern char far F_2e38_0c00[];
extern struct { char b[14]; } far F_20a5_1f60[];
extern char far F_20a5_33c8[];
extern char far F_20a5_33d3[];
extern char D_11ca;
extern unsigned char D_a895, D_d6cc;
extern char D_a896[];
extern char *D_c291;
extern int D_c7a4[];
extern char D_e230, D_07d3, D_d4e4;
extern int D_d6ee;
extern char far *D_c103;
extern int D_c8d5[];
extern char D_c12f[], D_c11d[], D_c907;
extern char D_d7be[][4], D_d802[][4], D_c13f[][4], D_c181[][4];
extern struct cmb D_9c55[];
extern int D_e0a3, D_e0a5, D_e0a7, D_e0a9;
extern int D_d645[], D_d664[];

FILE *fopen(const char *, const char *);
int fclose(FILE *);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);
void far *_fmemcpy(void far *, const void far *, unsigned);
void far *_fmemset(void far *, int, unsigned);
int far random_n(int);
void far vec_1b35(char *, char far *);
void far vec_1b2b(char *, char far *, long);
void far vec_1df1(int);
void far vec_17ed(void), vec_17f7(void), vec_1801(void), vec_180b(void);
void far vec_181f(void), vec_1815(void), vec_1833(void), vec_1829(void);
int far sub_s014_19cd_3204(char *, int);
void far sub_s014_0e76_4188(int, int, int);
void far sub_root_0000_25f2(char *);
void far sub_root_05d9_019a(char *, char far *);
int far sub_root_0410_004c(int, int);
int far sub_s014_1462_000a(struct cmb *);
int far sub_s014_1d69_000c(int, int, int);
void far sub_s024_1ef8_0000(int, int);
void far sub_s024_1ef8_059a(int);
void far sub_s024_1ef8_07a2(void);
int far sub_s024_1ef8_08f2(int, struct spot *);

void far sub_s024_1ef8_0000(int n, int cur)
{
    char name[0x42];
    FILE *fp;
    char far *base;
    int lvl, i, j;
    struct atv far *a;
    struct trig far *t;

    base = (char far *)&F_20a5_0000 + 0x24c6;
    D_11ca = cur;
    for (lvl = 0; lvl < n; lvl++) {
        fp = fopen("tac.txt", "a");
        fprintf(fp, "Enemy Activation Record for level %d\n", lvl);
        sprintf(name, "level%d.atv", lvl);
        vec_1b35(name, (char far *)&D_a895);
        _fmemcpy(&F_20a5_47ee, D_a896, (D_d6cc = D_a895) * 30);
        fprintf(fp, "\tNumActivationSpots: %d\n", D_d6cc);
        for (i = 0; i < D_d6cc; i++) {
            fprintf(fp, "\tstrtx: %d\n", F_20a5_47ee.r[i].strtx);
            fprintf(fp, "\tstrty: %d\n", F_20a5_47ee.r[i].strty);
            fprintf(fp, "\tsprd: %d\n", F_20a5_47ee.r[i].sprd);
            fprintf(fp, "\trm: %d\n", F_20a5_47ee.r[i].rm);
            fprintf(fp, "\tAllowReenf: %d\n", F_20a5_47ee.r[i].reenf);
            fprintf(fp, "\ttype[0]: %d\n", F_20a5_47ee.r[i].type[0]);
            fprintf(fp, "\ttype[1]: %d\n", F_20a5_47ee.r[i].type[1]);
            fprintf(fp, "\ttype[2]: %d\n", F_20a5_47ee.r[i].type[2]);
            fprintf(fp, "\tnum/type[0]: %d\n", F_20a5_47ee.r[i].num[0]);
            fprintf(fp, "\tnum/type[1]: %d\n", F_20a5_47ee.r[i].num[1]);
            fprintf(fp, "\tnum/type[2]: %d\n", F_20a5_47ee.r[i].num[2]);
            fprintf(fp, "\tF: %d\tG: %d Pf: %d\n", F_20a5_47ee.r[i].f, F_20a5_47ee.r[i].g,
                    F_20a5_47ee.r[i].pf);
            if (F_20a5_47ee.r[i].reenf) {
                fprintf(fp, "\t\tRtype: %d\n", F_20a5_47ee.r[i].rtype);
                fprintf(fp, "\t\tRsprd: %d\n", F_20a5_47ee.r[i].rsprd);
                fprintf(fp, "\t\tRx: %d\n", F_20a5_47ee.r[i].rx);
                fprintf(fp, "\t\tRy: %d\n", F_20a5_47ee.r[i].ry);
                fprintf(fp, "\t\tRrate: %d\n", F_20a5_47ee.r[i].rrate);
                fprintf(fp, "\t\tRmax: %d\n", F_20a5_47ee.r[i].rmax);
                fprintf(fp, "\t\tRdirn: %d\n", F_20a5_47ee.r[i].rdirn);
            }
        }
        fclose(fp);
        sprintf(name, "level%d.flr", lvl);
        vec_1b35(name, (char far *)&F_20a5_0000);
        _fmemset(F_20a5_33d3, 0, 0x28);
        fp = fopen("tac.txt", "a");
        fprintf(fp, "Level Record:%d\n", lvl);
        fprintf(fp, "V:%d S:%d R:%d D%d E:%d\n", F_20a5_0000.v, F_20a5_0000.s, F_20a5_0000.r,
                F_20a5_0000.d, F_20a5_0000.e);
        fprintf(fp, "FurnRemove\n");
        for (i = 0; i < 5; i++)
            fprintf(fp, "x:%d y:%d\n", F_20a5_0000.furn[i].x, F_20a5_0000.furn[i].y);
        fprintf(fp, "WallRemove\n");
        for (i = 0; i < 7; i++)
            fprintf(fp, "x:%d y:%d\n", F_20a5_0000.wall[i].x, F_20a5_0000.wall[i].y);
        fclose(fp);
        vec_1df1(lvl);
        for (j = 0; j < F_20a5_0000.ntrig; j++) {
            t = &F_20a5_0000.t[j];
            if (t->n)
                for (i = 0; i < (unsigned char)t->n; i++)
                    ((struct item6 *)D_c291)[i] = F_20a5_0000.t[j].it[i];
            D_c7a4[F_20a5_0000.t[j].idx] = sub_s014_19cd_3204(D_c291, (unsigned char)t->n);
        }
        for (i = 0; i < F_20a5_0000.nlist; i++)
            F_20a5_0000.list[i] += (int)base;
        for (i = 0; i < F_20a5_0000.nhandler; i++)
            switch (F_20a5_0000.h[i].u.type) {
            case 0:
                F_20a5_0000.h[i].u.fn = vec_17ed;
                break;
            case 1:
                F_20a5_0000.h[i].u.fn = vec_17f7;
                break;
            case 2:
                F_20a5_0000.h[i].u.fn = vec_1801;
                break;
            case 4:
                F_20a5_0000.h[i].u.fn = vec_180b;
                break;
            case 5:
                F_20a5_0000.h[i].u.fn = vec_181f;
                break;
            case 6:
                F_20a5_0000.h[i].u.fn = vec_1815;
                break;
            case 7:
                F_20a5_0000.h[i].u.fn = vec_1833;
                break;
            case 8:
                F_20a5_0000.h[i].u.fn = vec_1829;
                break;
            }
        vec_1b2b(name, (char far *)&F_20a5_0000, 0x33fcL);
        if (lvl == cur) {
            char door = F_20a5_0000.door;
            if (door >= 0)
                F_2e38_0bb6.m[D_11ca] |= 1L << door;
            sub_s014_0e76_4188(F_20a5_0000.f1f4c, F_20a5_0000.f1f4e, (unsigned char)F_20a5_0000.f1f50);
        }
    }
}

void far sub_s024_1ef8_059a(int level)
{
    char name[0x42];
    long mask;
    int i, k, j;

    if (level == -1)
        return;
    D_e230 = 1;
    D_07d3++;
    if (D_d4e4 == 1) {
        sprintf(name, "level%d.flr", D_11ca);
        vec_1b2b(name, (char far *)&F_20a5_0000, 0x33fcL);
        sprintf(name, "level%d.atv", D_11ca);
        _fmemcpy(D_a896, &F_20a5_47ee, (D_a895 = D_d6cc) * 30);
        vec_1b2b(name, (char far *)&D_a895, (unsigned long)D_d6cc * 30 + 1);
    }
    D_d4e4 = 0;
    sprintf(name, "level%d.flr", level);
    vec_1b35(name, (char far *)&F_20a5_0000);
    _fmemset(F_2e38_0c00, 0, 0x28);
    for (i = 0; i < 40; i++)
        if (F_2e38_0c00[i] == 0 && F_20a5_1f60[i].b[0] != -1)
            F_2e38_0c00[F_20a5_1f60[i].b[0]] = 1;
    D_11ca = level;
    mask = F_2e38_0bb6.m[level];
    for (k = 0; k < F_20a5_0000.nlist; k++) {
        if (!(mask & 1)) {
            j = 0;
            if (F_20a5_000.w[F_20a5_0000.list[k] / 2] != -1)
                for (j = 0; *(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[k] + j * 2) != -1; j++) {
                    int v = *(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[k] + j * 2);
                    F_20a5_0[v >> 8][v & 0xff].b3 |= 0x80;
                }
        }
        mask >>= 1;
    }
    sprintf(name, "level%d.atv", level);
    vec_1b35(name, (char far *)&D_a895);
    _fmemcpy(&F_20a5_47ee, D_a896, (D_d6cc = D_a895) * 30);
    if (D_d6ee)
        sub_s024_1ef8_07a2();
    D_07d3--;
}

void far sub_s024_1ef8_07a2(void)
{
    char name[0x1c];
    struct frc far *f;
    union { char far *p; struct { int off, seg; } w; } dst;
    int k, t, j;

    f = (struct frc far *)D_c103;
    sub_root_0000_25f2("bc");
    sprintf(name, "%Fs.frc", F_20a5_33c8);
    sub_root_05d9_019a(name, D_c103);
    k = F_20a5_0000.f33d2;
    dst.w.seg = D_c8d5[F_20a5_0000.f33d1] = D_d6ee;
    dst.w.off = 0;
    _fmemcpy(dst.p, D_c103 + f[k].off, f[k].size);
    t = F_20a5_0000.f33d1;
    D_c12f[t] = f[k].f20;
    D_c11d[t] = f[k].f21;
    if (f[k].f24 == 1)
        D_c907 = 1;
    for (j = 0; j < 4; j++) {
        D_d7be[t][j] = f[k].a[j];
        D_d802[t][j] = f[k].b[j];
        D_c13f[t][j] = f[k].c[j];
        D_c181[t][j] = f[k].d[j];
    }
}

int far sub_s024_1ef8_08f2(int who, struct spot *a)
{
    int dx[13], dy[13];
    int ox, oy, i, done;
    register int x, y;

    ox = D_9c55[who].x;
    oy = D_9c55[who].y;
    D_9c55[who].f19 = D_9c55[who].f1a = sub_root_0410_004c(a->dir, 4);
    switch (a->dir) {
    case 0:
        dy[3] = dy[4] = dy[5] = -8;
        dy[6] = dy[7] = dy[8] = -16;
        dy[9] = dy[10] = dy[11] = -24;
        dy[12] = -32;
        dx[0] = dx[3] = dx[6] = dx[9] = dy[0] = dy[1] = dy[2] = 0;
        dx[1] = dx[4] = dx[7] = dx[10] = -12;
        dx[2] = dx[5] = dx[8] = dx[11] = dx[12] = 12;
        break;
    case 2:
        dy[1] = dy[4] = dy[7] = dy[10] = -8;
        dy[2] = dy[5] = dy[8] = dy[11] = dy[12] = 8;
        dx[0] = dx[1] = dx[2] = dy[0] = dy[3] = dy[6] = dy[9] = 0;
        dx[3] = dx[4] = dx[5] = 12;
        dx[6] = dx[7] = dx[8] = 24;
        dx[9] = dx[10] = dx[11] = 36;
        dx[12] = 48;
        break;
    case 4:
        dy[3] = dy[4] = dy[5] = 8;
        dy[6] = dy[7] = dy[8] = 16;
        dy[9] = dy[10] = dy[11] = 24;
        dy[12] = 32;
        dx[0] = dx[3] = dx[6] = dx[9] = dy[0] = dy[1] = dy[2] = 0;
        dx[1] = dx[4] = dx[7] = dx[10] = 12;
        dx[2] = dx[5] = dx[8] = dx[11] = dx[12] = -12;
        break;
    case 6:
        dy[1] = dy[4] = dy[7] = dy[10] = 8;
        dy[2] = dy[5] = dy[8] = dy[11] = dy[12] = -8;
        dx[0] = dx[1] = dx[2] = dy[0] = dy[3] = dy[6] = dy[9] = 0;
        dx[3] = dx[4] = dx[5] = -12;
        dx[6] = dx[7] = dx[8] = -24;
        dx[9] = dx[10] = dx[11] = -36;
        dx[12] = -48;
        break;
    }
    if (who < 5) {
        for (i = 0; i < 13; i++) {
            D_9c55[who].x = dx[i] + a->x;
            D_9c55[who].y = dy[i] + a->y;
            if (sub_s014_1462_000a(&D_9c55[who]) == 0)
                goto ok;
        }
        D_e0a5 = a->x;
        D_e0a3 = a->y;
        done = i = 0;
        for (;;) {
            D_9c55[who].x = random_n(48) + D_e0a5;
            D_9c55[who].y = random_n(24) + D_e0a3;
            if (sub_s014_1462_000a(&D_9c55[who]) == 0) {
                D_e0a9 = D_9c55[who].x;
                D_e0a7 = D_9c55[who].y;
                if (sub_s014_1d69_000c(-1, 0, 0) == 0)
                    goto ok;
            }
            if (++i > 500 && done == 0) {
                x = a->x >> 5;
                y = a->y >> 4;
                F_20a5_0[x - 1][y - 1].b0 &= 0x0f;
                F_20a5_0[x - 1][y].b0 &= 0x0f;
                F_20a5_0[x - 1][y + 1].b0 &= 0x0f;
                F_20a5_0[x][y - 1].b0 &= 0x0f;
                F_20a5_0[x][y].b0 &= 0x0f;
                F_20a5_0[x][y + 1].b0 &= 0x0f;
                F_20a5_0[x + 1][y - 1].b0 &= 0x0f;
                F_20a5_0[x + 1][y].b0 &= 0x0f;
                F_20a5_0[x + 1][y + 1].b0 &= 0x0f;
                done = 1;
                D_d4e4 = 1;
                i = 0;
                continue;
            }
            if (i > 500)
                break;
        }
    } else {
        for (i = 12; i > 0; i--) {
            D_9c55[who].x = dx[i] + a->x;
            D_9c55[who].y = dy[i] + a->y;
            if (sub_s014_1462_000a(&D_9c55[who]) == 0)
                goto ok;
        }
    }
    D_9c55[who].x = ox;
    D_9c55[who].y = oy;
    return 0;
ok:
    D_d645[who] = D_9c55[who].x;
    D_d664[who] = D_9c55[who].y;
    return 1;
}
