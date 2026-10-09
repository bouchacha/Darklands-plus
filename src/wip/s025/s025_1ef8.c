/* place: s025 1ef8:0000 */
/* orig: size=0ddc CONST=7d1c+000c _DATA=2600+0073 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 6/7. 0000 decoded fully but blocked: the original keeps SI live across inline
 * block copies with push si/push di (6.00); 6.00A splits the register variable instead. Combat overlay s025 (child of s014): enemy placement and animation buffers.
 * 0000 places the enemies of the level (level%d.enm) into the combatant records 5..12;
 * 09a6 (kind, who) makes sure the animation buffers of an enemy kind are loaded (frees and
 * reloads all when a bigger one is needed); 0a84 (kind) allocates the "W"/"C"/"D" buffers with
 * the sizes from tacanim.db (14-byte records: name, W, C, D sizes, 4 frame counts);
 * 0be4/0c02 free the buffers of kinds 5 and 6; 0cfc enemy side setup; 0d3c reads the
 * four frame counts of a combatant's kind. */

typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;
struct anim { char name[4]; unsigned w, c, d; char n[4]; };
struct mem { char status; char pad1; char kind; char pad3; char f04; char pad5[0x7b]; };
struct s6 { char b[6]; };
struct s6i { int a; char p[4]; };
struct grp { char type; char n; int x; int y; char count; char pad7; char facing; char pad9; };
struct enc { int x; int y; int n; char pad6; char flag; char types[3]; char counts[3];
    int fl; int gr; int pf; struct grp grp; };
struct pal { unsigned char off; char rgb[0x30]; unsigned char a, b, c, d; };
struct money { int fl; int gr; int pf; };
struct bits { unsigned lo : 9; unsigned mid : 4; unsigned hi : 2; unsigned top : 1; };
struct ab { char a; char b; };
struct cbt {
    char pad0[4];
    int x;   /* +04 */
    int y;   /* +06 */
    char pad8[6];
    char * info;   /* +0e */
    int w10;   /* +10 */
    char pad12[2];
    char status;   /* +14 */
    char pad15[1];
    char kind;   /* +16 */
    char pad17[1];
    char f18;   /* +18 */
    char f19;   /* +19 */
    char f1a;   /* +1a */
    char f1b;   /* +1b */
    char idx;   /* +1c */
    char pad1d[5];
    char f22;   /* +22 */
    char pad23[39];
    char f4a;   /* +4a */
    char pad4b[6];
    char f51;   /* +51 */
    char pad52[44];
    char nitems;   /* +7e */
    char pad7f[1];
};
struct etype {
    struct cbt head;
    char name[4];   /* +80 */
    char name2[0x17];   /* +84 */
    char f9b;   /* +9b */
    char pad9c[2];
    int f9e;   /* +9e */
    int fa0;   /* +a0 */
    int fa2;   /* +a2 */
    int fa4;   /* +a4 */
    char fa6;   /* +a6 */
    unsigned char fa7[3];   /* +a7 */
    char faa;   /* +aa */
    char fab;   /* +ab */
    int fac;   /* +ac */
    char fae[10];   /* +ae */
    char fb8[10];   /* +b8 */
    struct s6 fc2[16];   /* +c2 */
};

#define MK_FP(s, o) ((void far *)(((unsigned long)(s) << 16) | (unsigned)(o)))

extern struct cbt D_9c55[];
extern struct etype D_b899;
extern int D_e0ea, D_d6f7;
extern char D_17b2[];
extern int D_940b;
extern char D_9069;
extern unsigned D_ca53, D_ca55;
extern char D_9085[];
extern int D_c28d[], D_c23e[], D_c927[], D_d645[], D_d664[];
extern char D_c2bf[], D_c1f3[], D_c1c1[], D_d1d7[], D_d680[];
extern char D_d37c[][10], D_d411[][10], D_c98a[][10];
extern struct s6 far F_2e38_0bde[][16];
extern char far F_2e38_0bd8[][0x14];
extern struct grp far F_2e38_0bf6;
extern struct money far F_2e38_0bb0;
extern struct s6i far F_20a5_35ee[];
extern char far F_20a5_763b, far F_20a5_763c;
extern int D_e0a5, D_e0a3, D_e0a9, D_e0a7;
extern struct bits D_a696[];
extern char D_ca64[], D_d4e5[], D_d4f2[], D_d5e9[], D_d4c7[], D_a6d3[], D_a6c4[];
extern struct ab D_d70c[];
extern char D_ca57[], D_d5c6[];
extern char D_1182[], D_118e[], D_119a[], D_11a6[];
extern char D_c80f[], D_c831[];
extern char D_d6eb;
extern char D_c21a;
extern char D_c908[], D_c96b[];
extern char D_cab4[][0x90];
extern char D_9631, D_d4e4;
extern int D_da34;
extern char *D_1174[];
extern char D_261c[], D_2628[];

extern struct mem D_9c69[];
extern unsigned D_ca45[];
extern int D_ca75[], D_ca8b[];
extern int D_ca4d;
extern char *D_1166[];
extern char *D_11b2[], *D_11b6[], *D_11ba[];
extern int D_c200[];
extern int D_d65f, D_d67e;
extern char D_c3cc;
extern char D_d6f0;
extern char D_11ca;
extern char D_a6b4[];
extern int D_c3cd;
extern int D_07d4, D_da4f;

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
FILE *fopen(const char *, const char *);
unsigned fread(void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
int stricmp(const char *, const char *);
int far alloc_buffer(unsigned, int, char *);
void far free_buffer(int, char *);
void far sub_s014_0e76_38b6(int, int);
int far vec_1e05(int, int *);
int abs(int);
int fseek(FILE *, long, int);
void far *_fmemcpy(void far *, const void far *, unsigned);
int far random_n(int);
void far sub_root_05f8_0117(int, int, int);
void far sub_root_0617_011e(char *, char far *);
void far sub_root_0410_031e(unsigned, unsigned);
void far sub_root_0410_0364(unsigned, unsigned);
int far sub_s014_1462_000a(struct cbt *);
int far sub_s014_1d69_000c(int, int, int);
void far sub_s014_18bf_0a40(int, int, int);
void far sub_root_047c_0362(int, int, int, int);
void far sub_root_047c_03da(int);
int far sub_s025_1ef8_0000(int, struct enc far *, int);
void far sub_s025_1ef8_09a6(int, int);
void far sub_s025_1ef8_0a84(int);
void far sub_s025_1ef8_0be4(void);
void far sub_s025_1ef8_0c02(int);
void far sub_s025_1ef8_0cfc(void);
void far sub_s025_1ef8_0d3c(int, char *, char *, char *, char *);

int far sub_s025_1ef8_0000(int mode, struct enc far *enc, int who)
{
    int x0, y0, n, ngroups, facing, dir, quad, dx, dy, adx, ady;
    char far *types;
    char far *counts;
    struct pal far *pals;
    struct etype *rec;
    FILE *fp;
    int g, j, k, slot, si, tries, ret, placed, w, p, pi;
    int w2, w1, basex, basey, rx, ry;
    char a, b;
    unsigned char palidx[3], pb[3], pc[3], pd[3];

    ret = 0;
    placed = 0;
    rec = &D_b899;
    pals = (struct pal far *)MK_FP(D_d6f7, D_e0ea + 0x31b);
    switch (mode) {
    case 0:
        x0 = enc->x;
        y0 = enc->y;
        n = enc->n;
        types = enc->types;
        counts = enc->counts;
        ngroups = 3;
        dx = (D_9c55[who].x - x0) / 2;
        dy = D_9c55[who].y - y0;
        adx = abs(dx);
        ady = abs(dy);
        if (adx != 0)
            facing = ady * 4 / adx;
        else
            facing = 20;
        if (facing == 0)
            dir = 0;
        else if (facing >= 2 && facing < 8)
            dir = 1;
        else
            dir = 2;
        if (dx >= 0 && dy < 0)
            quad = 0;
        else if (dx >= 0 && dy >= 0)
            quad = 1;
        else if (dx < 0 && dy >= 0)
            quad = 2;
        else
            quad = 3;
        facing = D_17b2[quad * 3 + dir];
        break;
    case 1:
        x0 = F_2e38_0bf6.x;
        y0 = F_2e38_0bf6.y;
        n = F_2e38_0bf6.n;
        ngroups = 1;
        types = &F_2e38_0bf6.type;
        counts = &F_2e38_0bf6.count;
        facing = F_2e38_0bf6.facing;
        break;
    }
    sub_root_05f8_0117(0, 8, D_940b);
    sub_root_0617_011e("enemypal.dat", (char far *)pals);
    if (D_9069 == 0) {
        D_ca55 = D_d6f7;
        D_ca53 = 0;
        sub_root_0410_031e(D_ca53, D_ca55);
    }
    sprintf(D_9085, "level%d.enm", D_11ca);
    fp = fopen(D_9085, "rb");
    for (g = 0; g < ngroups; g++) {
        if (types[g] == -1)
            continue;
        fseek(fp, (long)types[g] * 0x122 + 1, 0);
        fread(&D_b899, 0x122, 1, fp);
        for (j = 0; j < counts[g]; j++) {
            si = -1;
            for (k = 5; k <= 12; k++)
                if (D_9c55[k].status == 0) {
                    si = k;
                    break;
                }
            if (si != -1) {
                if (rec->fa0 == D_d65f)
                    slot = 5;
                else if (rec->fa0 == D_d67e)
                    slot = 6;
                else if (D_d65f == -1) {
                    slot = 5;
                    D_d65f = rec->fa0;
                } else if (D_d67e == -1) {
                    slot = 6;
                    D_d67e = rec->fa0;
                } else
                    si = -1;
            }
            if (si == -1)
                continue;
            D_a6b4[si] = D_11ca;
            D_c200[si] = rec->fa2;
            D_9c55[si] = rec->head;
            D_9c55[si].f19 = D_9c55[si].f1a = facing;
            D_9c55[si].kind = slot;
            D_9c55[si].idx = si;
            D_9c55[si].f4a = -1;
            D_c28d[slot] = types[g];
            if (mode == 0)
                D_c2bf[si] = rec->fab;
            else
                D_c2bf[si] = 0;
            D_c23e[slot] = rec->fa4;
            D_c1f3[si] = rec->faa;
            D_c1c1[si] = rec->fa6;
            strcpy(D_1166[slot], rec->name);
            _fstrcpy(F_2e38_0bd8[slot], rec->name2);
            for (k = 0; k < D_9c55[si].nitems; k++)
                F_2e38_0bde[si][k] = rec->fc2[k];
            D_c927[si] = rec->fac;
            D_d1d7[si] = 0;
            for (k = 0; k < 10; k++) {
                if (k != 2)
                    D_d1d7[si] += rec->fae[k];
                D_d37c[si][k] = rec->fae[k];
                D_d411[si][k] = rec->fb8[k];
            }
            if (rec->f9e == 0)
                w = D_9c55[si].f51;
            else {
                w = D_9c55[si].f22;
                D_d680[si] = -1;
                if (D_9c55[si].kind == 5) {
                    if (rec->fa2 == D_d67e)
                        D_d680[si] = 6;
                } else if (D_9c55[si].kind == 6 && rec->fa2 == D_d65f)
                    D_d680[si] = 5;
            }
            if (rec->fab != 2)
                sub_s014_18bf_0a40(si, w, rec->f9e);
            else {
                D_1174[D_9c55[si].kind][0] = 'P';
                D_1174[D_9c55[si].kind][1] = 'T';
            }
            D_9c55[si].status = 1;
            D_e0a5 = x0;
            D_e0a3 = y0;
            w2 = n * 2;
            w1 = n;
            p = rec->f9b + facing / 2 * 3;
            a = D_261c[p];
            b = D_2628[p];
            basex = D_e0a5 - (a < 0 ? w2 : 0);
            basey = D_e0a3 - (b < 0 ? w1 : 0);
            rx = w2 << (a & 1);
            ry = w1 << (b & 1);
            D_a696[si].lo = 0;
            D_a6c4[si] = D_a6d3[si] = D_d70c[si].b = D_d70c[si].a = D_d4c7[si] = D_d5e9[si] =
                D_d4f2[si] = D_d4e5[si] = D_ca64[si] = 0;
            D_9c55[si].w10 = 0;
            F_20a5_35ee[si].a = 0;
            D_d5c6[si] = D_ca57[si] = 7;
            sub_s025_1ef8_0d3c(si, &D_1182[slot], &D_118e[slot], &D_119a[slot], &D_11a6[slot]);
            D_c80f[si] = D_119a[slot];
            D_c831[si] = D_11a6[slot];
            D_a696[si].hi = 3;
            for (tries = 0; tries <= 500; tries++) {
                if (tries == 0) {
                    D_9c55[si].x = basex;
                    D_9c55[si].y = basey;
                } else {
                    D_9c55[si].x = random_n(rx) + basex;
                    D_9c55[si].y = random_n(ry) + basey;
                }
                if (sub_s014_1462_000a(&D_9c55[si]) == 0) {
                    D_e0a9 = D_9c55[si].x;
                    D_e0a7 = D_9c55[si].y;
                    if (sub_s014_1d69_000c(-1, 0, 0) == 0)
                        break;
                }
            }
            if (tries > 500)
                D_9c55[si].status = 0;
            else {
                placed = 1;
                if (D_9069 == 0)
                    for (p = 0; p < 3; p++) {
                        pi = rec->fa7[p];
                        if (pi == 0xff) {
                            palidx[p] = -1;
                            continue;
                        }
                        _fmemcpy(*(char far **)&D_ca53 + pals[pi].off, pals[pi].rgb, 0x30);
                        palidx[p] = pals[pi].a;
                        pb[p] = pals[pi].b;
                        pc[p] = pals[pi].c;
                        pd[p] = pals[pi].d;
                    }
                if (mode == 0 && enc->flag == 1) {
                    D_d6eb = 1;
                    F_2e38_0bf6 = enc->grp;
                }
                D_9c55[si].f18 = 5;
                if (D_ca45[slot] == 0) {
                    sub_root_05f8_0117(0, 4, D_940b);
                    sub_root_05f8_0117(0, 5, D_940b);
                    sub_root_05f8_0117(0, 6, D_940b);
                    sub_root_05f8_0117(0, 7, D_940b);
                    sub_s025_1ef8_09a6(slot, si);
                    sub_root_05f8_0117(0, 8, D_940b);
                    sub_root_05f8_0117(1, 1, D_940b);
                    sub_root_05f8_0117(2, 2, D_940b);
                    sub_root_05f8_0117(3, 3, D_940b);
                }
                if (D_c21a && si == 5) {
                    if (D_c21a == 'I') {
                        F_20a5_763b = 'D';
                        F_20a5_763c = '7';
                    } else {
                        F_20a5_763b = 'C';
                        F_20a5_763c = 'D';
                    }
                }
                D_c96b[si] = D_c908[slot];
                for (p = 0; p <= 9; p++)
                    D_c98a[si][p] = p;
                D_9c55[si].info = D_cab4[D_9c55[si].kind];
                D_9c55[si].f1b = 0;
            }
            D_d645[si] = D_9c55[si].x;
            D_d664[si] = D_9c55[si].y;
        }
    }
    fclose(fp);
    if (placed == 1) {
        ret = 1;
        D_9631 = 1;
        D_d4e4 = 1;
        D_da34 |= 1 << D_11ca;
        F_2e38_0bb0.pf += enc->pf;
        if (F_2e38_0bb0.pf >= 12) {
            F_2e38_0bb0.gr += F_2e38_0bb0.pf / 12;
            F_2e38_0bb0.pf %= 12;
        }
        F_2e38_0bb0.gr += enc->gr;
        if (F_2e38_0bb0.gr >= 20) {
            F_2e38_0bb0.fl += F_2e38_0bb0.gr / 20;
            F_2e38_0bb0.gr %= 20;
        }
        F_2e38_0bb0.fl += enc->fl;
        if (D_9069 == 0) {
            sub_root_0410_0364(D_ca53, D_ca55);
            for (g = 0; g < 3; g++)
                if (palidx[g] != 0xff) {
                    sub_root_047c_0362(pd[g], pc[g], palidx[g], pb[g]);
                    sub_root_047c_03da(pd[g]);
                }
        }
    }
    sub_root_05f8_0117(0, 0, D_940b);
    return ret;
}

char D_261c[12] = { -15, -15, -15, -15, 0, -16, -15, -15, -15, -15, -16, 0 };
char D_2628[12] = { -15, -16, 0, -15, -15, -15, -15, 0, -16, -15, -15, -15 };

#pragma optimize("a", on)
void far sub_s025_1ef8_09a6(int kind, int who)
{
    int need, k, t;

    need = 0;
    for (k = kind + 1; k <= 6; k++)
        if (D_ca45[k] != 0 && D_ca45[kind] < D_ca45[k])
            need = 1;
    if (need == 0) {
        sub_s025_1ef8_0a84(kind);
        sub_s014_0e76_38b6(who, 5);
        return;
    }
    sub_s025_1ef8_0be4();
    for (k = 5; k < 13; k++) {
        if (D_9c69[k].status) {
            t = D_9c69[k].kind;
            if (D_ca45[t] == 0) {
                sub_s025_1ef8_0a84(t);
                if (t == 5)
                    D_d65f = D_c200[k];
                else if (t == 6)
                    D_d67e = D_c200[k];
            }
            D_c3cc = 1;
            sub_s014_0e76_38b6(k, D_9c69[k].f04);
            D_c3cc = 0;
        }
    }
}

#pragma optimize("", on)
void far sub_s025_1ef8_0a84(int kind)
{
    char name[6];
    struct anim rec;
    FILE *fp;

    strcpy(name, D_1166[kind]);
    fp = fopen("tacanim.db", "rb");
    while (!(fp->_flag & 0x10)) {
        fread(&rec, 14, 1, fp);
        if (stricmp(rec.name, name) == 0)
            break;
    }
    fclose(fp);
    if (kind >= 5) {
        sprintf(name, "W %s", D_1166[kind]);
        D_ca45[kind] = alloc_buffer(rec.w, 0, name);
        sprintf(name, "C %s", D_1166[kind]);
        D_ca75[kind] = alloc_buffer(rec.c, 0, name);
        sprintf(name, "D %s", D_1166[kind]);
        D_ca8b[kind] = alloc_buffer(rec.d, 0, name);
    } else if (rec.w < rec.c) {
        sprintf(name, "C %s", D_1166[kind]);
        D_ca4d = alloc_buffer(rec.c, 0, name);
    } else {
        sprintf(name, "W %s", D_1166[kind]);
        D_ca4d = alloc_buffer(rec.w, 0, name);
    }
}

void far sub_s025_1ef8_0be4(void)
{
    int k;

    for (k = 5; k <= 6; k++)
        sub_s025_1ef8_0c02(k);
    D_d65f = -1;
    D_d67e = -1;
}

void far sub_s025_1ef8_0c02(int kind)
{
    char tag[8];

    strcpy(tag, "EB");
    if (D_ca45[kind]) {
        free_buffer(D_ca45[kind], tag);
        strcpy(D_11b2[kind], "");
        D_ca45[kind] = 0;
        if (D_ca75[kind]) {
            strcat(tag, "t");
            free_buffer(D_ca75[kind], tag);
            D_ca75[kind] = 0;
            strcpy(D_11b6[kind], "");
        }
        if (D_ca8b[kind]) {
            strcat(tag, "d");
            free_buffer(D_ca8b[kind], tag);
            D_ca8b[kind] = 0;
            strcpy(D_11ba[kind], "");
        }
    }
}

void far sub_s025_1ef8_0cfc(void)
{
    int i;

    D_d6f0 = 1;
    for (i = 5; i < 13; i++)
        if (D_9c69[i].status) {
            D_a6b4[i] = D_11ca;
            if (!vec_1e05(i, &D_c3cd))
                D_9c69[i].status = 0;
        }
    D_da4f = D_07d4;
}

void far sub_s025_1ef8_0d3c(int who, char *a, char *b, char *c, char *d)
{
    char name[4];
    struct anim rec;
    FILE *fp;

    strcpy(name, D_1166[D_9c69[who].kind]);
    fp = fopen("tacanim.db", "rb");
    while (!(fp->_flag & 0x10)) {
        fread(&rec, 14, 1, fp);
        if (stricmp(rec.name, name) == 0)
            break;
    }
    fclose(fp);
    *a = rec.n[0];
    *b = rec.n[1];
    *c = rec.n[2];
    *d = rec.n[3];
}
