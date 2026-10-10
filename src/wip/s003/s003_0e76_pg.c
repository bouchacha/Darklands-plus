/* place: s003 0e76:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Battle state save/restore (saves\<name>.bsv).
   status (e9, 2026-10-09): DRAFT.
   The file is a 0x2a-byte header D_d91b (word 0: packed size of the variable block,
   D_d91d[i]: packed size of level i, D_d93d/D_d93f and D_d941/D_d943: the two optional
   files D_1162/D_1164 (raw size, packed size)), followed by PKWARE DCL "implode" blocks
   (s003_1016, kept in assembly; implode = 1016:04c6, explode = 1016:0d07, both pascal):
   the battle variables serialised into the D_c103 buffer, then per level the .flr, .atv
   and .enm data, then the two files.  0000/006a are the memory read/write callbacks for
   (ex|im)plode, 0040/00a0 the file read/write callbacks (pascal, retf 8). */

/* pascal callbacks: the names come out upper case, which is fine (only referenced here) */
#define IMPLODE(r, w, b, t, s) ((unsigned (far pascal *)(void far *, void far *, char far *, unsigned far *, unsigned far *))sub_s003_1016_04c6)((void far *)(r), (void far *)(w), b, t, s)
#define EXPLODE(r, w, b) ((unsigned (far pascal *)(void far *, void far *, char far *))sub_s003_1016_0d07)((void far *)(r), (void far *)(w), b)

struct font { char x0[0xc]; int xc; int xe; };
struct ent { void (far *fn)(void); char x4[10]; };
struct lvl { char x0[0x2184]; char count; char x2185[7]; struct ent ent[1]; };
typedef struct _iobuf FILE;

extern struct font *D_017c;
extern char far *D_c103;
extern int D_940b;
extern char D_d4e4;
extern char D_a27a[];
extern char D_a895[];
extern char D_a896[];
extern char D_d6cc;
extern char D_9085[];
extern unsigned char D_9080;
extern char *D_1162, *D_1164;
extern int D_d90b, D_d90d;
extern char far *D_d90f;
extern char far *D_d913;
extern unsigned D_d917, D_d919;
extern unsigned D_d91b;
extern unsigned D_d91d[16];
extern int D_d93d, D_d93f, D_d941, D_d943;
extern char far F_20a5_0000[];
extern char far F_20a5_47ee[];
extern char D_07d8, D_11ca, D_907a, D_a6c1, D_c4fd, D_c600, D_c623, D_c651, D_c80e, D_c83e, D_c989, D_ca1b, D_ca29, D_ca99, D_d610, D_d661, D_d6f0, D_d6f4, D_d728, D_d73b, D_d7fe, D_d842, D_d947;
extern int D_dad7, D_07d4, D_07e6, D_160c, D_160e, D_a68f, D_a893, D_c1f0, D_c2bb, D_c925, D_c978, D_ca3d, D_ca3f, D_d662, D_d6ce, D_d945, D_d9e4, D_da34, D_da43, D_da45, D_da47, D_da49, D_da4b, D_da4d, D_da4f, D_da51, D_da53;
extern char D_a696[], D_a6b4[], D_a6c4[], D_a6d3[], D_c1c1[], D_c3cd[], D_c624[], D_c64c[], D_c678[], D_c6ef[], D_c77e[], D_c784[], D_c7a4[], D_c7cd[], D_c7fe[], D_c8b2[], D_ca57[], D_ca64[], D_d3fe[], D_d4c7[], D_d4e5[], D_d4f2[], D_d5e9[], D_d5f6[], D_d62b[], D_d68d[], D_d6fb[], D_d948[], D_d962[], D_d97c[], D_d996[], D_d9b0[], D_d9ca[], D_d9e6[], D_da36[], D_da55[], D_da6f[], D_da89[], D_daa3[], D_dabd[];
extern char far F_20a5_59c8[];
extern char far F_20a5_6238[];
extern char far F_2e38_0bb6[];
extern char far F_2e38_0c64[];

void far vec_17ed(void), vec_17f7(void), vec_1801(void), vec_180b(void);
void far vec_181f(void), vec_1815(void), vec_1833(void), vec_1829(void);
int far vec_1b2b(char *, char far *, long);
int far vec_1b35(char *, char far *);
void far vec_1b3f(int);
int far sub_s003_1016_04c6();
int far sub_s003_1016_0d07();
void far *far _fmemcpy(void far *, const void far *, unsigned);
unsigned far _dos_read(int, void far *, unsigned, unsigned *);
unsigned far _dos_write(int, const void far *, unsigned, unsigned *);
unsigned far _dos_creat(const char *, unsigned, int *);
unsigned far _dos_close(int);
unsigned far sub_root_06a1_2c9e(const char *, unsigned, int *);
int far sprintf(char *, const char *, ...);
int far printf(const char *, ...);
void far exit_(int);
FILE *far fopen(const char *, const char *);
int far fclose(FILE *);
int far remove(const char *);
char *strcpy(char *, const char *);
unsigned strlen(const char *);
void far sub_root_0000_1354(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_12c3(void);
void far sub_root_05f8_0117(int, int, int);
int far sub_root_0617_011e(char *, void far *);
int far sub_root_0617_016a(char *, void far *, int);
void far sub_s003_0e76_19ca(char *);
void far sub_s003_0e76_19e8(char *, int);

/* explode: read from the D_d90f memory block (D_d919 bytes left) */
unsigned far pascal sub_s003_0e76_0000(char far *buf, unsigned far *size)
{
    unsigned n;

    n = *size;
    if (n > D_d919)
        n = D_d919;
    D_d919 -= n;
    _fmemcpy(buf, D_d90f, n);
    D_d90f += n;
    return n;
}

/* implode/explode: read from the file D_d90b */
unsigned far pascal sub_s003_0e76_0040(char far *buf, unsigned far *size)
{
    unsigned nr;

    nr = 0;
    _dos_read(D_d90b, buf, *size, &nr);
    return nr;
}

/* explode: write to the D_d913 memory block */
#pragma optimize("a", on)
void far pascal sub_s003_0e76_006a(char far *buf, unsigned far *size)
{
    _fmemcpy(D_d913, buf, *size);
    D_d913 += *size;
    D_d917 += *size;
}
#pragma optimize("", on)

/* implode: write to the file D_d90d */
unsigned far pascal sub_s003_0e76_00a0(char far *buf, unsigned far *size)
{
    unsigned nw;

    nw = 0;
    _dos_write(D_d90d, buf, *size, &nw);
    D_d917 += *size;
    return nw;
}

/* save the battle to saves\<name>.bsv (name: "<name>.xxx") */
#pragma optimize("a", on)
#pragma optimize("c", off)
void far sub_s003_0e76_00dc(char *name)
{
    struct lvl far *lv;
    struct ent far *e;
    char far *p;
    char far *work;
    char far *map;
    unsigned *hp;
    unsigned dsize, type, nw;
    int i, k;
    FILE *f;
    char buf[30];
    char path[30];
    char fn[30];

    sub_root_05f8_0117(0, 4, D_940b);
    sub_root_05f8_0117(1, 5, D_940b);
    sub_root_05f8_0117(2, 6, D_940b);
    sub_root_05f8_0117(3, 7, D_940b);
    work = D_c103 + 0x4e20;
    if (D_d4e4 == 1) {
        sprintf(D_a27a, "level%d.flr", D_11ca);
        vec_1b2b(D_a27a, F_20a5_0000, 0x33fcL);
        sprintf(D_a27a, "level%d.atv", D_11ca);
        _fmemcpy(D_a896, F_20a5_47ee, (unsigned char)(D_a895[0] = D_d6cc) * 30);
        vec_1b2b(D_a27a, D_a895, (long)((unsigned char)D_d6cc * 30 + 1));
        D_d4e4 = 0;
    }
    D_017c->xc = 1;
    D_017c->xe = 0xff;
    sub_root_047c_12c3();
    strcpy(buf, name);
    i = strlen(buf);
    buf[i - 4] = 0;
    sprintf(path, "saves\\%s.bsv", buf);
    _dos_creat(path, 0, &D_d90d);
    *(p = D_c103) = D_907a;
    _fmemcpy((p += 1), F_20a5_6238, 0xcb2);
    _fmemcpy((p += 0xcb2), D_d6fb, 0xd);
    _fmemcpy((p += 0xd), D_c3cd, 0xe);
    *(p += 0xe) = D_c600;
    _fmemcpy((p += 1), D_c7a4, 0x28);
    _fmemcpy((p += 0x28), F_20a5_59c8, 0x870);
    *(p += 0x870) = D_c80e;
    *(p += 1) = D_c83e;
    *(p += 1) = D_d6f0;
    *(p += 1) = D_d728;
    *(p += 1) = D_d73b;
    *(p += 1) = D_d7fe;
    *(p += 1) = D_d947;
    *(int far *)(p += 1) = D_d945;
    *(p += 0x2) = D_d842;
    *(int far *)(p += 1) = D_c1f0;
    _fmemcpy((p += 0x2), D_c624, 0x6);
    _fmemcpy((p += 0x6), D_c64c, 0x5);
    _fmemcpy((p += 0x5), D_c678, 0x5);
    _fmemcpy((p += 0x5), D_c6ef, 0x5);
    _fmemcpy((p += 0x5), D_c77e, 0x5);
    _fmemcpy((p += 0x5), D_c784, 0x5);
    _fmemcpy((p += 0x5), D_c7cd, 0x5);
    _fmemcpy((p += 0x5), D_c7fe, 0x5);
    _fmemcpy((p += 0x5), D_c8b2, 0x5);
    *(p += 0x5) = D_d610;
    *(p += 1) = D_d6f4;
    *(int far *)(p += 1) = D_a68f;
    _fmemcpy((p += 0x2), D_a6d3, 0xd);
    _fmemcpy((p += 0xd), D_ca57, 0xd);
    _fmemcpy((p += 0xd), D_ca64, 0xd);
    _fmemcpy((p += 0xd), D_d3fe, 0xd);
    _fmemcpy((p += 0xd), D_d4c7, 0xd);
    _fmemcpy((p += 0xd), D_d4e5, 0xd);
    _fmemcpy((p += 0xd), D_d4f2, 0xd);
    _fmemcpy((p += 0xd), D_a6b4, 0xd);
    _fmemcpy((p += 0xd), D_d5e9, 0xd);
    _fmemcpy((p += 0xd), D_da36, 0xd);
    _fmemcpy((p += 0xd), D_d5f6, 0x1a);
    _fmemcpy((p += 0x1a), D_d62b, 0x1a);
    _fmemcpy((p += 0x1a), D_a696, 0x1a);
    _fmemcpy((p += 0x1a), D_a6c4, 0xd);
    _fmemcpy((p += 0xd), D_c1c1, 0xd);
    _fmemcpy((p += 0xd), F_2e38_0bb6, 0x40);
    *(p += 0x40) = D_a6c1;
    *(p += 1) = D_c4fd;
    *(p += 1) = D_11ca;
    *(p += 1) = D_c623;
    *(p += 1) = D_c651;
    *(int far *)(p += 1) = D_c925;
    *(int far *)(p += 0x2) = D_c978;
    *(p += 0x2) = D_c989;
    *(p += 1) = D_ca1b;
    *(p += 1) = D_ca29;
    *(int far *)(p += 1) = D_07e6;
    *(int far *)(p += 0x2) = D_ca3f;
    *(p += 0x2) = D_ca99;
    *(int far *)(p += 1) = D_a893;
    *(int far *)(p += 0x2) = D_ca3d;
    *(p += 0x2) = D_d661;
    *(int far *)(p += 1) = D_d662;
    _fmemcpy((p += 0x2), D_d68d, 0xa);
    *(int far *)(p += 0xa) = D_d6ce;
    *(int far *)(p += 0x2) = D_da34;
    _fmemcpy((p += 0x2), D_d9e6, 0x4e);
    *(int far *)(p += 0x4e) = D_d9e4;
    _fmemcpy((p += 0x2), D_d9ca, 0x1a);
    _fmemcpy((p += 0x1a), D_d9b0, 0x1a);
    _fmemcpy((p += 0x1a), D_d996, 0x1a);
    _fmemcpy((p += 0x1a), D_d97c, 0x1a);
    _fmemcpy((p += 0x1a), D_d962, 0x1a);
    _fmemcpy((p += 0x1a), D_d948, 0x1a);
    _fmemcpy((p += 0x1a), D_dabd, 0x1a);
    _fmemcpy((p += 0x1a), D_daa3, 0x1a);
    _fmemcpy((p += 0x1a), D_da89, 0x1a);
    _fmemcpy((p += 0x1a), D_da6f, 0x1a);
    _fmemcpy((p += 0x1a), D_da55, 0x1a);
    *(int far *)(p += 0x1a) = D_da53;
    *(int far *)(p += 0x2) = D_da51;
    *(int far *)(p += 0x2) = D_da4f;
    *(int far *)(p += 0x2) = D_da4d;
    *(int far *)(p += 0x2) = D_da4b;
    *(int far *)(p += 0x2) = D_da49;
    *(int far *)(p += 0x2) = D_da47;
    *(int far *)(p += 0x2) = D_c2bb;
    *(int far *)(p += 0x2) = D_da45;
    *(int far *)(p += 0x2) = D_da43;
    *(int far *)(p += 0x2) = D_07d4;
    _fmemcpy((p += 0x2), F_2e38_0c64, 0xfa);
    *(p += 0xfa) = D_07d8;
    *(int far *)(p += 1) = D_160c;
    *(int far *)(p += 0x2) = D_160e;
    p += 2;
    _dos_write(D_d90d, &D_d91b, 0x2a, &nw);
    D_d90f = D_c103;
    D_d919 = (unsigned)p;
    D_d917 = type = 0;
    dsize = 0x800;
    IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
    D_d91b = D_d917;
    i = 0;
    if (D_d661 != 0) {
        hp = D_d91d;
        map = F_20a5_0000;
        do {
            sprintf(fn, "level%d.flr", i);
            p = D_c103;
            vec_1b35(fn, map);
            _fmemcpy(p, map, 0x33fc);
            lv = (struct lvl far *)D_c103;
            for (k = 0; k < lv->count; k++) {
                e = &lv->ent[k];
                if (e->fn == vec_17ed)
                    e->fn = 0;
                else if (e->fn == vec_17f7)
                    *(long far *)&e->fn = 1;
                else if (e->fn == vec_1801)
                    *(long far *)&e->fn = 2;
                else if (e->fn == vec_180b)
                    *(long far *)&e->fn = 4;
                else if (e->fn == vec_181f)
                    *(long far *)&e->fn = 5;
                else if (e->fn == vec_1815)
                    *(long far *)&e->fn = 6;
                else if (e->fn == vec_1833)
                    *(long far *)&e->fn = 7;
                else if (e->fn == vec_1829)
                    *(long far *)&e->fn = 8;
            }
            p += 0x33fc;
            sprintf(fn, "level%d.atv", i);
            vec_1b35(fn, map);
            _fmemcpy(p, map, *map * 30 + 1);
            p += *p * 30 + 1;
            sprintf(fn, "level%d.enm", i);
            sub_root_0617_011e(fn, p);
            p += *p * 0x122 + 1;
            D_d90f = D_c103;
            D_d919 = (unsigned)p;
            D_d917 = 0;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            *hp++ = D_d917;
        } while (++i < (unsigned char)D_d661);
    }
    D_d941 = -1;
    if ((f = fopen(D_1164, "r")) != 0) {
        fclose(f);
        if ((D_d941 = sub_root_0617_011e(D_1164, D_c103)) != 0) {
            D_d90f = D_c103;
            D_d919 = D_d941;
            D_d917 = 0;
            dsize = 0x400;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            D_d943 = D_d917;
        }
    }
    D_d93d = -1;
    if ((f = fopen(D_1162, "r")) != 0) {
        fclose(f);
        if ((D_d93d = sub_root_0617_011e(D_1162, D_c103)) != 0) {
            D_d90f = D_c103;
            D_d919 = D_d93d;
            D_d917 = 0;
            dsize = 0x400;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            D_d93f = D_d917;
        }
    }
    _dos_close(D_d90d);
    sub_root_06a1_2c9e(path, 2, &D_d90d);
    _dos_write(D_d90d, &D_d91b, 0x2a, &nw);
    _dos_close(D_d90d);
    sub_root_05f8_0117(0, 0, D_940b);
    sub_root_05f8_0117(1, 1, D_940b);
    sub_root_05f8_0117(2, 2, D_940b);
    sub_root_05f8_0117(3, 3, D_940b);
    vec_1b3f(D_11ca);
    sub_root_047c_1293();
}
#pragma optimize("", on)

/* restore the battle from saves\<name>.bsv */
#pragma optimize("a", on)
#pragma optimize("c", off)
void far sub_s003_0e76_0ce8(char *name)
{
    struct lvl far *lv;
    char far *p;
    char far *work;
    unsigned nr;
    int i, k, n, r;
    char buf[30];
    char path[32];
    char fn[30];

    work = D_c103 - 0x63c0;
    D_017c->xc = 1;
    D_017c->xe = 0xff;
    strcpy(buf, name);
    i = strlen(buf);
    buf[i - 4] = 0;
    sprintf(path, "saves\\%s.bsv", buf);
    if (sub_root_06a1_2c9e(path, 0, &D_d90b) != 0)
        return;
    _dos_read(D_d90b, &D_d91b, 0x2a, &nr);
    D_d917 = 0;
    _dos_read(D_d90b, D_d90f = D_c103 + 0x4e20, D_d91b, &nr);
    if (D_d91b != nr)
        sub_s003_0e76_19ca("Vars");
    D_d919 = D_d91b;
    D_d90f = D_c103 + 0x4e20;
    D_d913 = D_c103;
    if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
        sub_s003_0e76_19e8("Vars", r);
    p = D_c103;
    D_907a = *p;
    _fmemcpy(F_20a5_6238, (p += 1), 0xcb2);
    _fmemcpy(D_d6fb, (p += 0xcb2), 0xd);
    _fmemcpy(D_c3cd, (p += 0xd), 0xe);
    D_c600 = *(p += 0xe);
    _fmemcpy(D_c7a4, (p += 1), 0x28);
    _fmemcpy(F_20a5_59c8, (p += 0x28), 0x870);
    D_c80e = *(p += 0x870);
    D_c83e = *(p += 1);
    D_d6f0 = *(p += 1);
    D_d728 = *(p += 1);
    D_d73b = *(p += 1);
    D_d7fe = *(p += 1);
    D_d947 = *(p += 1);
    D_d945 = *(int far *)(p += 1);
    D_d842 = *(p += 0x2);
    D_c1f0 = *(int far *)(p += 1);
    _fmemcpy(D_c624, (p += 0x2), 0x6);
    _fmemcpy(D_c64c, (p += 0x6), 0x5);
    _fmemcpy(D_c678, (p += 0x5), 0x5);
    _fmemcpy(D_c6ef, (p += 0x5), 0x5);
    _fmemcpy(D_c77e, (p += 0x5), 0x5);
    _fmemcpy(D_c784, (p += 0x5), 0x5);
    _fmemcpy(D_c7cd, (p += 0x5), 0x5);
    _fmemcpy(D_c7fe, (p += 0x5), 0x5);
    _fmemcpy(D_c8b2, (p += 0x5), 0x5);
    D_d610 = *(p += 0x5);
    D_d6f4 = *(p += 1);
    D_a68f = *(int far *)(p += 1);
    _fmemcpy(D_a6d3, (p += 0x2), 0xd);
    _fmemcpy(D_ca57, (p += 0xd), 0xd);
    _fmemcpy(D_ca64, (p += 0xd), 0xd);
    _fmemcpy(D_d3fe, (p += 0xd), 0xd);
    _fmemcpy(D_d4c7, (p += 0xd), 0xd);
    _fmemcpy(D_d4e5, (p += 0xd), 0xd);
    _fmemcpy(D_d4f2, (p += 0xd), 0xd);
    _fmemcpy(D_a6b4, (p += 0xd), 0xd);
    _fmemcpy(D_d5e9, (p += 0xd), 0xd);
    _fmemcpy(D_da36, (p += 0xd), 0xd);
    _fmemcpy(D_d5f6, (p += 0xd), 0x1a);
    _fmemcpy(D_d62b, (p += 0x1a), 0x1a);
    _fmemcpy(D_a696, (p += 0x1a), 0x1a);
    _fmemcpy(D_a6c4, (p += 0x1a), 0xd);
    _fmemcpy(D_c1c1, (p += 0xd), 0xd);
    _fmemcpy(F_2e38_0bb6, (p += 0xd), 0x40);
    D_a6c1 = *(p += 0x40);
    D_c4fd = *(p += 1);
    D_11ca = *(p += 1);
    D_c623 = *(p += 1);
    D_c651 = *(p += 1);
    D_c925 = *(int far *)(p += 1);
    D_c978 = *(int far *)(p += 0x2);
    D_c989 = *(p += 0x2);
    D_ca1b = *(p += 1);
    D_ca29 = *(p += 1);
    D_07e6 = *(int far *)(p += 1);
    D_ca3f = *(int far *)(p += 0x2);
    D_ca99 = *(p += 0x2);
    D_a893 = *(int far *)(p += 1);
    D_ca3d = *(int far *)(p += 0x2);
    D_d661 = *(p += 0x2);
    D_d662 = *(int far *)(p += 1);
    _fmemcpy(D_d68d, (p += 0x2), 0xa);
    D_dad7 = *(int far *)(p += 0xa);
    D_da34 = *(int far *)(p += 0x2);
    _fmemcpy(D_d9e6, (p += 0x2), 0x4e);
    D_d9e4 = *(int far *)(p += 0x4e);
    _fmemcpy(D_d9ca, (p += 0x2), 0x1a);
    _fmemcpy(D_d9b0, (p += 0x1a), 0x1a);
    _fmemcpy(D_d996, (p += 0x1a), 0x1a);
    _fmemcpy(D_d97c, (p += 0x1a), 0x1a);
    _fmemcpy(D_d962, (p += 0x1a), 0x1a);
    _fmemcpy(D_d948, (p += 0x1a), 0x1a);
    _fmemcpy(D_dabd, (p += 0x1a), 0x1a);
    _fmemcpy(D_daa3, (p += 0x1a), 0x1a);
    _fmemcpy(D_da89, (p += 0x1a), 0x1a);
    _fmemcpy(D_da6f, (p += 0x1a), 0x1a);
    _fmemcpy(D_da55, (p += 0x1a), 0x1a);
    D_da53 = *(int far *)(p += 0x1a);
    D_da51 = *(int far *)(p += 0x2);
    D_da4f = *(int far *)(p += 0x2);
    D_da4d = *(int far *)(p += 0x2);
    D_da4b = *(int far *)(p += 0x2);
    D_da49 = *(int far *)(p += 0x2);
    D_da47 = *(int far *)(p += 0x2);
    D_c2bb = *(int far *)(p += 0x2);
    D_da45 = *(int far *)(p += 0x2);
    D_da43 = *(int far *)(p += 0x2);
    D_07d4 = *(int far *)(p += 0x2);
    _fmemcpy(F_2e38_0c64, (p += 0x2), 0xfa);
    D_07d8 = *(p += 0xfa);
    D_160c = *(int far *)(p += 1);
    D_160e = *(int far *)(p += 0x2);
    p += 2;
    for (i = 0; i < (unsigned char)D_d661; i++) {
        _dos_read(D_d90b, D_d90f = D_c103 + 0x4e20, D_d91d[i], &nr);
        if (D_d91d[i] != nr)
            sub_s003_0e76_19ca("Lv");
        D_d919 = D_d91d[i];
        D_d90f = D_c103 + 0x4e20;
        D_d913 = D_c103;
        r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work);
        sprintf(D_9085, "Lv%d", i);
        if (r != 0)
            sub_s003_0e76_19e8(D_9085, r);
        sprintf(fn, "level%d.flr", i);
        p = D_c103;
        lv = (struct lvl far *)D_c103;
        for (k = 0; k < lv->count; k++) {
            switch (*(int far *)&lv->ent[k].fn) {
            case 0:
                lv->ent[k].fn = vec_17ed;
                break;
            case 1:
                lv->ent[k].fn = vec_17f7;
                break;
            case 2:
                lv->ent[k].fn = vec_1801;
                break;
            case 4:
                lv->ent[k].fn = vec_180b;
                break;
            default:
                lv->ent[k].fn = vec_181f;
                break;
            case 6:
                lv->ent[k].fn = vec_1815;
                break;
            case 7:
                lv->ent[k].fn = vec_1833;
                break;
            case 8:
                lv->ent[k].fn = vec_1829;
                break;
            }
        }
        _fmemcpy(F_20a5_0000, p, 0x33fc);
        vec_1b2b(fn, F_20a5_0000, 0x33fcL);
        sprintf(fn, "level%d.atv", i);
        p += 0x33fc;
        n = *p;
        _fmemcpy(F_20a5_0000, p, n * 30 + 1);
        vec_1b2b(fn, F_20a5_0000, (long)(n * 30 + 1));
        sprintf(fn, "level%d.enm", i);
        p += n * 30 + 1;
        n = *p;
        sub_root_0617_016a(fn, p, n * 0x122 + 1);
    }
    if (D_d941 == -1 || D_d941 == 0)
        remove(D_1164);
    else {
        _dos_read(D_d90b, D_d90f = D_c103 + 0x4e20, D_d943, &nr);
        if (D_d943 != nr)
            sub_s003_0e76_19ca("terrf");
        D_d919 = D_d943;
        D_d90f = D_c103 + 0x4e20;
        D_d913 = D_c103;
        if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
            sub_s003_0e76_19e8("terrnf", r);
        sub_root_0617_016a(D_1164, D_c103, D_d941);
    }
    if (D_d93d == -1 || D_d93d == 0)
        remove(D_1162);
    else {
        _dos_read(D_d90b, D_d90f = D_c103 + 0x4e20, D_d93f, &nr);
        if (D_d93f != nr)
            sub_s003_0e76_19ca("cf");
        D_d919 = D_d93f;
        D_d90f = D_c103 + 0x4e20;
        D_d913 = D_c103;
        if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
            sub_s003_0e76_19e8("cache", r);
        sub_root_0617_016a(D_1162, D_c103, D_d93d);
    }
    _dos_close(D_d90b);
    D_d4e4 = 0;
    vec_1b3f(D_11ca);
    D_9080 &= 0x10;
}
#pragma optimize("", on)

/* unexpected end of a block */
void far sub_s003_0e76_19ca(char *what)
{
    sub_root_0000_1354();
    printf("EOF-%s", what);
    exit_(0);
}

/* explode error */
void far sub_s003_0e76_19e8(char *what, int err)
{
    sub_root_0000_1354();
    printf("Corrupt file-%s %d", what, err);
    exit_(0);
}
