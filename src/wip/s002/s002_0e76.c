/* place: s002 0e76:0000 */
/* orig: size=1aa6 CONST=7d22+0006 _DATA=0eb0+0411 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* compat600-off: sub_s002_0e76_1830 */
/* data: _DATA=0eb0 */
/* data: CONST=7d22 */
/* status (e6, 2026-10-09): ALL 10 FUNCTIONS WRITTEN; _DATA and CONST line up (hints above).
 * Match: 1750, 17ea, 1830 (m1: compat600-off keeps its stosb memsets), 1a7a (m1: the second
 * clear is a byte fill loop under pragma a), 08fe (battlegr.img loader: pragma a + "el" off, locals renamed for 6.00A's name-hash
 * slot order). Close: 0000 (k*i loop operand order, `sub ax,ax; push ax;
 * push ax`), 0c8c (6.00 keeps &rec[t] in a stack temp across _fmemcpy), 0806 (two stack slots
 * swapped), 1762 (m1: it is strcpy(*p, r[i].name) - names copied INTO the table; left: the loop pointer init
 * goes to SI instead of DI), 168a (strength-reduced D_ca45 pointer).
 * Data: tables D_114a (name strings: party names, cfile.dat, terrain.fil, portrait codes),
 * D_1182, D_11b2, D_11c8.. are defined here and emitted after 0000's literals.
 * Battle entry 0000(mode, bfld, rseed, 3 x (type, quality, number)) -> D_ca3d.
 */

struct res { int a; int b; };

/* ---- module data (DGROUP 0x0eb0..): the initialisers' strings come first, the variables
 * themselves are emitted after the literals of the next function (0000). ---- */
extern char D_a895[];
extern int far *D_9060;
extern char D_a876, D_907a, D_d661, D_9069, D_c102, D_07aa, D_07bc;
extern unsigned char D_c7dd;
extern unsigned D_9080;
extern int D_9417, D_a870, D_a86e, D_a872, D_a874, D_d6ce, D_c1f0, D_d843, D_00e4, D_d662;
extern int D_c21d, D_c240, D_c291, D_a68f, D_9c42, D_a893, D_ca3d;
extern char D_9429, D_9433, D_9609, D_961d, D_9631, D_9645, D_9681, D_9695, D_96a9, D_96bd;
extern char D_96d1, D_96e5, D_96f9, D_970d, D_9721, D_9735, D_9749, D_975d, D_987f, D_9920, D_9934;
extern char D_d493[13], D_c2cc[256], D_c3db[256], D_c500[256], D_a6b4[5];
extern int D_d645[13], D_d664[13], D_d611[13];
extern char far *D_ca53;
extern unsigned D_d6ee, D_d6f2, D_d6f9, D_d729, D_d70a, D_d73c, D_d726, D_d800;
int far fprintf();
int far _strdate();
int far _strtime();
int far remove();
int far vec_1aef();
int far vec_1af9();
int far vec_1b03();
int far vec_18d3();
int far vec_186f();
int far sub_root_0000_1384();
int far sub_root_0000_24bc();
int far sub_root_0000_262e();
int far sub_root_0410_031e();
int far sub_root_0410_0364();
int far sub_root_047c_0362();
int far sub_root_047c_04ea();
int far sub_root_047c_0584();
int far sub_root_047c_124f();
char *strcat(char *, char *);
#define FP_SEG(p) ((unsigned)((unsigned long)(void far *)(p) >> 16))
char *D_114a[] = {
    "", D_a895,
    "1234567890", "2         ", "3         ", "4         ", "5         ",
    "6         ", "7         ", "8         ", "9         ", "0         ",
    "cfile.dat", "terrain.fil",
    "f60", "f01", "a00", "c00", "c00", "e00", "e00",
    "xx", "xx", "xx", "xx", "xx", "xx", "xx"
};
char D_1182[48] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
char *D_11b2[] = {
    "xxxxxxxx", "xxxxxxxx", "xxxxxxxx", "xxxxxxxx", "xxxxxxxx", "xxxxxxxx",
    "xxxxxxxx", "xxxxxxxx", "xxxxxxxx", "xxxxxxxx", "xxxxxxxx"
};
int D_11c8 = 1;
char D_11ca[2] = { -1, 0 };
char D_11cc[16] = { 5, 5, 1, 1, 5, 5, 1, 1, 10, 10, 1, 1, 10, 10, 1, 1 };
char D_11dc[2] = { -1, 0 };
unsigned char D_11de[10] = { 0xff, 0xfe, 0xee, 0xb6, 0xaa, 0x94, 0x88, 0x80, 0, 0 };
char *D_11e8 = "Palette Buffer";
char D_11ea[8] = "tac.txt";
char *D_11f2 = " finished";
extern char D_12a6[];
extern char D_4276[];
extern int D_c943;
extern int D_c945;
extern int D_c947;
extern int D_c949;
extern int D_c94b;
extern int D_c94d;
extern int D_c94f;
extern int D_c951;
extern int D_c953;
extern int D_c955;
extern int D_c957;
extern int D_c959;
extern int D_c95b;
extern int D_c95d;
extern int D_c95f;
extern int D_c961;
extern int D_c963;
extern int D_c965;
extern int D_c967;
extern int D_c969;
typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;
FILE *far fopen();
struct animrec { char name[4]; unsigned size; char p6[8]; };
struct lstrec { char p0[0x14]; char name[9]; char x1d; char p1e[0x2e - 0x1e]; };
struct member { char x0; char p1; char x2; char p3[0x7d]; };
extern struct member D_9c69[5];
extern char D_9085[];
extern unsigned D_ca45[7];
extern char D_c80e;
extern char D_c83e;
struct rec6 { int x0; char p2[4]; };
struct rec26 { char p0[0x0e]; int x0e; char p10[0x26 - 0x10]; };
struct c4 { char c[4]; };
struct tac {                            /* tactical data at 20a5:0000 */
    char p0[0x33fc];
    char x33fc[0x78];                   /* 33fc */
    struct { unsigned a, b; } x3474[16]; /* 3474: floor effect tiles */
    struct rec26 *x34b4[0x9d];          /* 34b4: -> D_a895 records */
    struct rec6 x35ee[13];              /* 35ee */
    char p363c[0x3dc0 - 0x363c];
    unsigned x3dc0[0x2b];               /* 3dc0 */
    char p3e16[0x4d62 - 0x3e16];
    unsigned x4d62[0x2b];               /* 4d62: sprite segments (first = buffer) */
    char p4db8[0x6eea - 0x4db8];
    struct { unsigned a, b; } x6eea[16]; /* 6eea: north wall effect tiles */
    struct { unsigned a, b; } x6f2a[16]; /* 6f2a: west wall effect tiles */
};
struct party {                          /* 128-byte party/combatant record (0x9c59 - 10) */
    int xm10;                           /* -0a */
    int xm8;                            /* -08 */
    char pm6[6];
    char *x0;                           /* 00: -> 0x90-byte record at 0xcab4 */
    int x2;
    char p4[2];
    char x6;                            /* 06 (= D_9c69 x0) */
    char p7;
    char x8;                            /* 08 */
    char p9[3];
    char xc;                            /* 0c */
    char pd;
    char xe;                            /* 0e */
    char pf[0x3c - 0xf];
    char x3c;                           /* 3c */
    char p3d[0x80 - 0x3d - 10];
};
#define D_9c63 D_9c59
extern struct party D_9c59[13];
extern char D_cab4[5][0x90];
extern unsigned D_c1d0[16], D_c220[16], D_c24c[16], D_c26f[16], D_c29b[16], D_c4dd[16];
extern unsigned D_c601[16], D_c62c[16], D_c658[16], D_c8d5[16];
struct tdesc {                          /* tile description (D_9085) */
    char x00;
    char set0[9];                       /* 01: floor set */
    signed char t0a[16];                /* 0a: [1..15] tile numbers, -1 unused */
    char set1[9];                       /* 1a: floor effects */
    char x23;
    signed char t24[16];
    char set2[9];                       /* 34: north walls */
    signed char t3d[16];
    char set3[9];                       /* 4d: north wall effects */
    char x56;
    signed char t57[16];
    char set4[9];                       /* 67: west walls */
    signed char t70[16];
    char set5[9];                       /* 80: west wall effects */
    char x89;
    signed char t8a[16];
    char set6[9];                       /* 9a: furniture */
    char xa3;
    signed char ta4[16];
    signed char nvar;                   /* b4 */
    signed char tb5[16];
};
struct flcrec { unsigned size, off; };
struct ffcrec { unsigned size, off, x4; };
struct nwcrec { unsigned size, off, x4, x6, x8; };
struct nfcrec { unsigned size, off, x4; char x6, x7, x8, x9; };
struct frcrec { int w[16]; char x20, x21; unsigned size; int x24; unsigned off; };
extern char D_c12d, D_c12e, D_c17f, D_c8b7, D_c907, D_c1cf;
extern char D_c7de[16], D_c7ee[16], D_c76e[16], D_c789[16], D_c876[16], D_c887[16], D_c821[16], D_c855[16];
extern char D_c12f[16], D_c11d[16];
extern char D_d7be[16][4], D_d802[16][4], D_c13f[16][4], D_c181[16][4];
extern unsigned char D_9081;
extern char D_d6f0, D_d6f4, D_a6c1, D_c600, D_ca29;
extern int D_ca3d, D_d846, D_d65f, D_d67e;
extern char D_c26e, D_c21f, D_07d3, D_c865, D_d4e4, D_c783, D_a695, D_a6b2, D_d845;
extern char D_c21a, D_9084, D_1b98, D_c180, D_a33d, D_a395, D_a531, D_d6eb, D_c3cc;
extern unsigned D_d68d[5];
extern unsigned D_ca75[7], D_ca8b[7];
extern char D_ca57[13], D_ca64[13], D_d4c7[13], D_d4e5[13], D_d4f2[13], D_d5e9[13], D_a6d3[13], D_a6c4[13];
extern unsigned D_a696[13];
extern struct { char a, b; } D_d70c[13];
extern char D_d5b9[13];
extern int D_c927[13], D_d8f1[13], D_d8d7[13], D_d8bd[13], D_d8a3[13], D_d889[13], D_d86f[13];
extern char D_d862[13], D_d855[13], D_d848[13];
void *memset(void *, int, unsigned);
extern char far F_20a5_763b[];
extern struct tac far F_20a5_0000;
extern char far F_2e38_0bb6[];
extern char far F_20a5_4276[];
extern char far F_20a5_59c8[];
extern void far *D_c103;
extern unsigned D_d72b[8], D_ca2c[2], D_ca41[2], D_ca71[2], D_cea4[4], D_ca83[2];
extern unsigned D_d1be[8], D_d40b[8], D_d428[8], D_d4a0[8], D_d4b1[8], D_d4d4[8];
extern unsigned D_c21b, D_c242, D_c244, D_c28f, D_c2bd, D_c941, D_c987, D_ca19, D_ca2a;
extern unsigned D_a693, D_a691, D_d4ff, D_d501, D_d5b7, D_d503, D_d5b5, D_a2e5;
extern unsigned D_d4c1, D_d4c3, D_d4c5, D_c869, D_c4db, D_c621, D_ceac;
extern char D_d438, D_d4b0;
#define MK_FP(s, o) ((void far *)(((unsigned long)(s) << 16) | (unsigned)(o)))
int far _fmemcpy();
unsigned far alloc_buffer(unsigned long, char *);
char *strcpy(char *, char *);
int far _fmemset();
int far fclose();
int far fread();
int far fseek();
int far sprintf();
int far stricmp();
void *memset(void *, int, unsigned);
int far sub_root_0000_25f2();
int far sub_root_0000_2646();
int far sub_root_047c_0066();
int far sub_root_047c_00a9();
int far sub_root_047c_0568();
int far sub_root_047c_0783();
int far sub_root_047c_0791();
int far sub_root_047c_07ce();
int far sub_root_047c_08c8();
int far sub_root_047c_08f9();
int far sub_root_047c_12c3();
int far sub_root_05ba_0004();
int far sub_root_05d9_019a();
int far sub_root_0617_011e();
int far sub_root_06a1_2052();
#define rewind sub_root_06a1_2052
int far vec_1adb();
int far vec_1b0d();
int far sub_root_047c_0545();
int far sub_root_047c_04f8();
unsigned far sub_root_0617_045c();
void far free_buffer(unsigned, char *);
extern unsigned D_a2d7[6];
int far sub_s002_0e76_0000();
void far sub_s002_0e76_0806();
void far sub_s002_0e76_08fe();
void far sub_s002_0e76_0c8c();
void far sub_s002_0e76_168a();
void far sub_s002_0e76_1750();
void far sub_s002_0e76_1762();
int far sub_s002_0e76_17ea();
void far sub_s002_0e76_1830();
void far sub_s002_0e76_1a7a();

/* battle entry: set up the tactical engine for one battle, run it, clean up.
 * mode 0..3, bfld = battlefield type, rseed = random seed (0: BIOS tick count),
 * three enemy groups (type, quality, number). Returns D_ca3d (battle result). */
int far sub_s002_0e76_0000(int mode, int bfld, unsigned rseed, int ft1, int fq1, int fn1,
    int ft2, int fq2, int fn2, int ft3, int fq3, int fn3)
{
    char tbufc[16];
    char dbufb[16];
    FILE *fl;
    int kf;
    int ig;

    D_9060 = (int far *)&D_11c8;
    sub_root_0000_2646("tac start");
    sub_root_047c_0568();
    sub_root_047c_12c3();
    if (D_a876 != 'n' && D_a876 != 'N') {
        sub_root_047c_0791();
        sub_root_047c_00a9();
        sub_root_047c_08f9(D_9417);
        sprintf(D_9085, "%csound.dlb", D_a876);
        sub_root_047c_08c8(D_9417 = sub_root_047c_07ce(D_9085, 0));
        sub_root_047c_0783(0, 0, 0, D_a870, D_a86e, D_a872, D_a874);
        sub_root_047c_0066();
    }
    D_907a = 1;
    if (!(D_9080 & 0x100)) {
        switch (mode) {
        case 0:
            D_9080 &= ~2;
            D_9080 |= 0x10;
            sub_s002_0e76_1a7a();
            break;
        case 1:
            D_9080 |= 2;
            D_9080 &= ~0x10;
            break;
        case 2:
            D_9080 &= ~0x12;
            D_9080 |= 0x20;
            break;
        case 3:
            D_9080 &= ~0x12;
            D_9080 |= 0x20;
            D_11ca[0] = bfld;
            break;
        }
    } else {
        D_9080 |= 2;
        D_9080 &= ~0x10;
    }
    sub_s002_0e76_1830();
    if (D_11dc[1] == 1) {
        vec_1adb();
        if (rseed == 0)
            rseed = *(unsigned far *)MK_FP(0x40, 0x6c);
        D_9080 |= 1;
    }
    D_d6ce = rseed;
    D_c1f0 = bfld;
    if (mode == 0) {
        fclose(fl = fopen(D_11ea, "w"));
        fl = fopen(D_11ea, "w");
    } else
        fl = fopen(D_11ea, "a");
    _strdate(dbufb);
    _strtime(tbufc);
    fprintf(fl, "%s %s\n", dbufb, tbufc);
    fprintf(fl, "Tac params are\n\tmode: %d\n\tbfldtype: %d\n\trseed: %u\n\tftype1: %d\tfqual1: %d\tfnum1: %d\n\tftype2: %d\tfqual2: %d\tfnum2: %d\n\tftype3: %d\tfqual3: %d\tfnum3: %d\n\tterrn: %d\tmnth: %d\n\n",
        mode, bfld, rseed, ft1, fq1, fn1, ft2, fq2, fn2, ft3, fq3, fn3, D_d843, D_00e4);
    fclose(fl);
    if ((D_9080 & 0x10) == 0x10)
        D_d661 = vec_1aef(bfld, rseed, &D_d662, ft1, fq1, fn1, ft2, fq2, fn2, ft3, fq3, fn3);
    sub_s002_0e76_1750();
    sub_s002_0e76_17ea();
    D_c21d = 4;
    D_c240 = 8;
    D_c291 = (int)D_a895;
    D_9429 = 1;
    D_9433 = 2;
    D_9609 = 2;
    D_961d = 2;
    D_9631 = 2;
    D_9645 = 2;
    D_9681 = 2;
    D_9695 = 2;
    D_96a9 = 2;
    D_96bd = 2;
    D_96d1 = 2;
    D_96e5 = 2;
    D_96f9 = 2;
    D_970d = 2;
    D_9721 = 2;
    D_9735 = 2;
    D_9749 = 2;
    D_975d = 2;
    D_987f = 2;
    memset(D_d493, 0xff, 13);
    for (ig = 0; ig < 0x100; ig++) {
        D_c2cc[ig] = ig;
        D_c3db[ig] = ig;
        D_c500[ig] = ig;
    }
    sub_s002_0e76_0806();
    sub_s002_0e76_08fe();
    sub_root_047c_12c3();
    sub_root_047c_124f(0, 0, D_a693);
    D_a68f = D_9c42;
    sub_s002_0e76_168a();
    sub_s002_0e76_0c8c();
    sub_s002_0e76_1762();
    for (ig = 0; ig < 11; ig++)
        strcpy(D_11b2[ig], "");
    if ((D_9080 & 0x10) == 0x10)
        remove(D_114a[12]);
    for (ig = 5; ig < 13; ig++)
        D_9c63[ig].x6 = 0;
    kf = 0x26;
    for (ig = 0; ig < 0x9d; ig++)
        F_20a5_0000.x34b4[ig] = (struct rec26 *)(D_a895 + kf * ig);
    F_20a5_0000.x34b4[3]->x0e = 0;
    sub_root_047c_04ea(0, 0);
    for (ig = 0; ig < 13; ig++) {
        D_d645[ig] = D_9c63[ig].xm10;
        D_d664[ig] = D_9c63[ig].xm8;
        D_d611[ig] = D_9c63[ig].xc;
    }
    sub_root_047c_0584(0x1c0);
    if (D_9069 == 0) {
        D_ca53 = (char far *)((long)alloc_buffer(0x300L, D_11e8) << 16);
        sub_root_0410_031e(D_ca53);
        _fmemset(D_ca53 + 0x30, 0xff, 0x2d0);
        _fmemset(D_ca53 + 0x27, 0x31, 3);
        sub_root_0617_011e("comnclrs.dat", D_c103);
        _fmemcpy(D_ca53 + 0x30, D_c103, 0x30);
        _fmemcpy(D_ca53 + 0x168, (char far *)D_c103 + 0x30, 0x84);
        _fmemcpy(D_ca53 + 0x2d9, (char far *)D_c103 + 0xb4, 0x24);
        _fmemcpy(D_ca53 + 0xf0, F_20a5_0000.x33fc, 0x78);
        sub_root_0617_011e("bkgndpal.dat", D_c103);
        _fmemcpy(D_ca53 + 0x1ec, (char far *)D_c103 + 0xd5 * D_c7dd, 0xd5);
        sub_root_0410_0364(D_ca53);
        free_buffer(FP_SEG(D_ca53), D_11e8);
    }
    sub_root_047c_0362(1, 4, 0x10, 0x15);
    sub_root_047c_0362(2, 6, 0x18, 0x1e);
    sub_root_047c_0362(4, 5, 0x16, 0x17);
    vec_1b03();
    sub_root_047c_12c3();
    sub_root_047c_124f(0, 0, D_9c42);
    D_a68f = D_9c42;
    sub_root_0000_262e();
    strcpy(D_9085, "BattleResults - ");
    switch (D_a893) {
    case 0:
        strcat(D_9085, "victory");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case 2:
        strcat(D_9085, "retreat");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case 3:
        strcat(D_9085, "defeat");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case 4:
        strcat(D_9085, "surrender");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case 5:
        strcat(D_9085, "all dead");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case 8:
        strcat(D_9085, "leave");
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    case -1:
    case 6:
    case 7:
        break;
    default:
        sub_root_0000_24bc(D_9085, 0x96);
        break;
    }
    if (D_a893 == 8)
        D_a893 = 2;
    vec_1af9();
    for (ig = 0; ig < 5; ig++)
        if (D_a2d7[ig]) {
            free_buffer(D_a2d7[ig], "SGS");
            D_a2d7[ig] = 0;
        }
    *(struct c4 *)D_9085 = *(struct c4 *)"Seq";
    for (ig = 0; ig < 5; ig++)
        if (D_ca45[ig]) {
            free_buffer(D_ca45[ig], D_9085);
            D_ca45[ig] = 0;
        }
    free_buffer(F_20a5_0000.x4d62[0], "MiscSp");
    if (D_d6ee) {
        free_buffer(D_d6ee, "VarFrn");
        D_d6ee = 0;
    }
    if (D_d6f2)
        free_buffer(D_d6f2, "Frn");
    if (D_d6f9)
        free_buffer(D_d6f9, "FLR");
    if (D_d729)
        free_buffer(D_d729, "FFX");
    if (D_d70a)
        free_buffer(D_d70a, "NWL");
    if (D_d73c)
        free_buffer(D_d73c, "NFX");
    if (D_d726)
        free_buffer(D_d726, "WWL");
    if (D_d800)
        free_buffer(D_d800, "WFX");
    F_20a5_763b[0] = 'D';
    F_20a5_763b[1] = 'G';
    for (ig = 0; ig < 5; ig++)
        if (D_9c63[ig].x6 == 1 || D_9c63[ig].x6 == 2) {
            D_9c63[ig].x2 = 0;
            D_9c63[ig].x3c = 0xff;
            D_a6b4[ig] = 0;
        }
    D_907a = 0;
    D_9429 = 2;
    D_9433 = 2;
    D_9920 = 1;
    D_9934 = 1;
    D_987f = 1;
    D_9080 &= 0xfe10;
    sub_root_0000_1384();
    D_07aa = 2;
    D_07bc = 2;
    D_c102 = 0;
    vec_18d3();
    if (D_a876 != 'n' && D_a876 != 'N') {
        sub_root_047c_0791();
        sub_root_047c_00a9();
        sub_root_047c_08f9(D_9417);
        sprintf(D_9085, "%csound.dlc", D_a876);
        sub_root_047c_08c8(D_9417 = sub_root_047c_07ce(D_9085, 0));
        sub_root_047c_0783(0, 0, 0, D_a870, D_a86e, D_a872, D_a874);
        sub_root_047c_0066();
    }
    if (D_11dc[1] == 1)
        vec_186f();
    return D_ca3d;
}

/* load the short portraits of the party members and lay them out on the battle screen */
void far sub_s002_0e76_0806(void)
{
    int i, j;
    unsigned v;
    int y;

    *(volatile int *)&y = 0;
    for (i = 0; i < 5; i++) {
        D_a2d7[i] = 0;
        if (D_9c69[i].x0) {
            sub_root_047c_0545();
            sprintf(D_9085, "pics\\%sshort.pic", D_114a[14 + i]);
            D_a2d7[i] = sub_root_0617_045c(D_9085);
            sub_root_047c_04f8();
        }
    }
    sub_root_047c_0545();
    D_a2d7[5] = sub_root_0617_045c("pics\\a00short.pic");
    sub_root_047c_04f8();
    for (i = 0; i < 5; i++) {
        if ((v = D_a2d7[i]) != 0) {
            for (j = i + 1; j < 6; j++)
                if (D_a2d7[j]) {
                    y = ((D_a2d7[j] - v) << 4) - 0x14;
                    break;
                }
            vec_1b0d(D_9c69[i].x2, y, v);
        }
    }
    free_buffer(D_a2d7[5], "LSGI");
}

/* load battlegr.img: a buffer of battle graphics with a table of offsets in front;
 * every offset is relocated by the buffer's segment and stored in the sprite tables. */
#pragma optimize("a", on)
#pragma optimize("el", off)
void far sub_s002_0e76_08fe(void)
{
    int far *pb;
    unsigned sizef;
    unsigned basei;
    char far *dste;
    int ij;

    sub_root_0617_011e("battlegr.img", pb = (int far *)D_c103);
    sizef = *pb++;
    basei = F_20a5_0000.x4d62[0] = alloc_buffer((unsigned long)sizef, "MiscSp");
    ((unsigned *)&dste)[1] = basei;
    ((unsigned *)&dste)[0] = 0;
    for (ij = 0; ij < 0x2b; ij++)
        F_20a5_0000.x4d62[ij] = *pb++ + basei;
    for (ij = 0; ij < 0x2b; ij++)
        F_20a5_0000.x3dc0[ij] = *pb++ + basei;
    for (ij = 0; ij < 8; ij++)
        D_d72b[ij] = *pb++ + basei;
    D_c21b = *pb++ + basei;
    D_c242 = *pb++ + basei;
    D_c244 = *pb++ + basei;
    D_c28f = *pb++ + basei;
    D_c2bd = *pb++ + basei;
    D_c941 = *pb++ + basei;
    D_c987 = *pb++ + basei;
    D_ca19 = *pb++ + basei;
    D_ca2a = *pb++ + basei;
    for (ij = 0; ij < 2; ij++)
        D_ca2c[ij] = *pb++ + basei;
    for (ij = 0; ij < 2; ij++)
        D_ca41[ij] = *pb++ + basei;
    for (ij = 0; ij < 2; ij++)
        D_ca71[ij] = *pb++ + basei;
    D_d438 = 0;
    for (ij = 0; ij < 4; ij++)
        D_cea4[ij] = *pb++ + basei;
    D_ceac = 0;
    D_a693 = *pb++ + basei;
    D_a691 = *pb++ + basei;
    for (ij = 0; ij < 8; ij++) {
        D_d1be[ij] = *pb++ + basei;
        D_d40b[ij] = *pb++ + basei;
        D_d428[ij] = *pb++ + basei;
        D_d4a0[ij] = *pb++ + basei;
        D_d4b1[ij] = *pb++ + basei;
        D_d4d4[ij] = *pb++ + basei;
    }
    D_d4ff = *pb++ + basei;
    D_d501 = *pb++ + basei;
    D_d5b7 = *pb++ + basei;
    D_d503 = ((D_cea4[0] - D_ca2c[0]) << 4) - 4;
    D_d5b5 = ((D_a2e5 - D_cea4[0]) << 4) - 4;
    for (ij = 0; ij < 10; ij++) {
        D_d4c1 = *pb++ + basei;
        D_d4c3 = *pb++ + basei;
        D_d4c5 = *pb++ + basei;
    }
    for (ij = 0; ij < 2; ij++)
        D_ca83[ij] = *pb++ + basei;
    D_c869 = *pb++ + basei;
    D_c4db = *pb++ + basei;
    D_c621 = *pb++ + basei;
    _fmemcpy(dste, pb, sizef);
    D_d438 = 0;
    D_d4b0 = 2;
}
#pragma optimize("", on)

/* load the battlefield tiles: the tile description (D_114a[13]) names up to seven tile sets
 * (floor .flc, floor effects .ffc, north walls .nwc, north wall effects .nfc, west walls
 * .wwc, west wall effects .wfc, furniture .frc); each set file holds a directory of
 * records {size, offset, ...} for its tiles, the used ones are copied into one buffer. */
void far sub_s002_0e76_0c8c(void)
{
    char namek[80];
    struct tdesc *dg;
    struct flcrec far *flcc;
    struct ffcrec far *ffch;
    struct nfcrec far *nfck;
    struct nfcrec far *wfcj;
    struct nwcrec far *nwcl;
    struct nwcrec far *wwco;
    struct frcrec far *frcf;
    struct nwcrec far *rk;
    struct frcrec far *qm;
    char far *dsth;
    unsigned sizen;
    int tb, jk;
    register int i;

    sub_root_0617_011e(D_114a[13], (char far *)D_9085);
    D_c12d = 2;
    D_c12e = 8;
    D_c17f = 8;
    sub_root_0000_25f2("bc");
    flcc = D_c103;
    ffch = D_c103;
    nfck = D_c103;
    wfcj = D_c103;
    nwcl = D_c103;
    wwco = D_c103;
    frcf = D_c103;
    dg = (struct tdesc *)D_9085;
    D_c7dd = dg->x00;
    sizen = D_d6f9 = 0;
    if (dg->set0[0]) {
        D_c8b7 = stricmp(dg->set0, "keep") == 0;
        sprintf(namek, "%s.flc", dg->set0);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t0a[i] != -1)
                sizen += flcc[dg->t0a[i]].size;
        ((unsigned *)&dsth)[1] = D_d6f9 = alloc_buffer((unsigned long)sizen, "Flr");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            D_c1d0[i] = 0;
            if ((tb = dg->t0a[i]) != -1) {
                _fmemcpy(dsth, (char far *)D_c103 + flcc[tb].off, flcc[tb].size);
                D_c1d0[i] = FP_SEG(dsth);
                ((unsigned *)&dsth)[1] += flcc[tb].size >> 4;
            }
        }
    }
    sizen = D_d729 = 0;
    if (dg->set1[0]) {
        D_c12d = dg->x23;
        sprintf(namek, "%s.ffc", dg->set1);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t24[i] != -1)
                sizen += ffch[dg->t24[i]].size;
        ((unsigned *)&dsth)[1] = D_d729 = alloc_buffer((unsigned long)sizen, "ffx");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            F_20a5_0000.x3474[i].a = 0;
            if ((tb = dg->t24[i]) != -1) {
                _fmemcpy(dsth, (char far *)D_c103 + ffch[tb].off, ffch[tb].size);
                F_20a5_0000.x3474[i].a = FP_SEG(dsth);
                if (ffch[tb].x4 == 0)
                    F_20a5_0000.x3474[i].b = 0;
                else
                    F_20a5_0000.x3474[i].b = ((ffch[tb].x4 - ffch[tb].off) >> 4) + F_20a5_0000.x3474[i].a;
                ((unsigned *)&dsth)[1] += ffch[tb].size >> 4;
            }
        }
    }
    sizen = D_d70a = 0;
    if (dg->set2[0]) {
        sprintf(namek, "%s.nwc", dg->set2);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t3d[i] != -1)
                sizen += nwcl[dg->t3d[i]].size;
        ((unsigned *)&dsth)[1] = D_d70a = alloc_buffer((unsigned long)sizen, "nwl");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            D_c24c[i] = D_c601[i] = D_c4dd[i] = D_c220[i] = 0;
            if ((tb = dg->t3d[i]) != -1) {
                rk = &nwcl[tb];
                _fmemcpy(dsth, (char far *)D_c103 + rk->off, rk->size);
                D_c220[i] = FP_SEG(dsth);
                D_c4dd[i] = ((rk->x4 - rk->off) >> 4) + FP_SEG(dsth);
                D_c601[i] = D_c4dd[i] + ((rk->x6 - rk->x4) >> 4);
                D_c24c[i] = ((rk->x8 - rk->x6) >> 4) + D_c601[i];
                ((unsigned *)&dsth)[1] += rk->size >> 4;
            }
        }
    }
    sizen = D_d73c = 0;
    if (dg->set3[0]) {
        D_c12e = dg->x56;
        sprintf(namek, "%s.nfc", dg->set3);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t57[i] != -1)
                sizen += nfck[dg->t57[i]].size;
        ((unsigned *)&dsth)[1] = D_d73c = alloc_buffer((unsigned long)sizen, "NFX");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            F_20a5_0000.x6eea[i].a = 0;
            if ((tb = dg->t57[i]) != -1) {
                _fmemcpy(dsth, (char far *)D_c103 + nfck[tb].off, nfck[tb].size);
                F_20a5_0000.x6eea[i].a = FP_SEG(dsth);
                D_c7de[i] = nfck[tb].x6;
                D_c7ee[i] = nfck[tb].x7;
                D_c76e[i] = nfck[tb].x8;
                D_c789[i] = nfck[tb].x9;
                if (nfck[tb].x4 == 0)
                    F_20a5_0000.x6eea[i].b = 0;
                else
                    F_20a5_0000.x6eea[i].b = ((nfck[tb].x4 - nfck[tb].off) >> 4) + F_20a5_0000.x6eea[i].a;
                ((unsigned *)&dsth)[1] += nfck[tb].size >> 4;
            }
        }
    }
    sizen = D_d726 = 0;
    if (dg->set4[0]) {
        sprintf(namek, "%s.wwc", dg->set4);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t70[i] != -1)
                sizen += wwco[dg->t70[i]].size;
        ((unsigned *)&dsth)[1] = D_d726 = alloc_buffer((unsigned long)sizen, "wwl");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            D_c26f[i] = D_c658[i] = D_c62c[i] = D_c29b[i] = 0;
            if ((tb = dg->t70[i]) != -1) {
                rk = &wwco[tb];
                _fmemcpy(dsth, (char far *)D_c103 + rk->off, rk->size);
                D_c29b[i] = FP_SEG(dsth);
                D_c62c[i] = ((rk->x4 - rk->off) >> 4) + FP_SEG(dsth);
                D_c658[i] = D_c62c[i] + ((rk->x6 - rk->x4) >> 4);
                D_c26f[i] = ((rk->x8 - rk->x6) >> 4) + D_c658[i];
                ((unsigned *)&dsth)[1] += rk->size >> 4;
            }
        }
    }
    sizen = D_d800 = 0;
    if (dg->set5[0]) {
        D_c17f = dg->x89;
        sprintf(namek, "%s.wfc", dg->set5);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->t8a[i] != -1)
                sizen += wfcj[dg->t8a[i]].size;
        ((unsigned *)&dsth)[1] = D_d800 = alloc_buffer((unsigned long)sizen, "WFX");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            F_20a5_0000.x6f2a[i].a = 0;
            if ((tb = dg->t8a[i]) != -1) {
                _fmemcpy(dsth, (char far *)D_c103 + wfcj[tb].off, wfcj[tb].size);
                F_20a5_0000.x6f2a[i].a = FP_SEG(dsth);
                D_c876[i] = wfcj[tb].x6;
                D_c887[i] = wfcj[tb].x7;
                D_c821[i] = wfcj[tb].x8;
                D_c855[i] = wfcj[tb].x9;
                if (wfcj[tb].x4 == 0)
                    F_20a5_0000.x6f2a[i].b = 0;
                else
                    F_20a5_0000.x6f2a[i].b = ((wfcj[tb].x4 - wfcj[tb].off) >> 4) + F_20a5_0000.x6f2a[i].a;
                ((unsigned *)&dsth)[1] += wfcj[tb].size >> 4;
            }
        }
    }
    sizen = D_d6f2 = 0;
    D_c907 = 0;
    D_c1cf = dg->xa3;
    if (dg->set6[0]) {
        sprintf(namek, "%s.frc", dg->set6);
        sub_root_05d9_019a(namek, D_c103);
        for (i = 1; i < 16; i++)
            if (dg->ta4[i] != -1)
                sizen += frcf[dg->ta4[i]].size;
        ((unsigned *)&dsth)[1] = D_d6f2 = alloc_buffer((unsigned long)sizen, "frn");
        ((unsigned *)&dsth)[0] = 0;
        for (i = 1; i < 16; i++) {
            D_c8d5[i] = 0;
            if ((tb = dg->ta4[i]) != -1) {
                qm = &frcf[tb];
                _fmemcpy(dsth, (char far *)D_c103 + qm->off, qm->size);
                D_c8d5[i] = FP_SEG(dsth);
                D_c12f[i] = qm->x20;
                D_c11d[i] = qm->x21;
                if (qm->x24 == 1)
                    D_c907 = 1;
                for (jk = 0; jk < 4; jk++) {
                    D_d7be[i][jk] = frcf[tb].w[jk];
                    D_d802[i][jk] = frcf[tb].w[jk + 4];
                    D_c13f[i][jk] = frcf[tb].w[jk + 8];
                    D_c181[i][jk] = frcf[tb].w[jk + 12];
                }
                ((unsigned *)&dsth)[1] += frcf[tb].size >> 4;
            }
        }
    }
    D_d6ee = 0;
    if (dg->nvar) {
        sizen = 0;
        for (i = 0; i < dg->nvar; i++)
            if (frcf[dg->tb5[i]].size > sizen)
                sizen = frcf[dg->tb5[i]].size;
        D_d6ee = alloc_buffer((unsigned long)sizen, "varfrn");
    }
}

/* allocate the animation buffers of the party members (sizes from tacanim.db) */
void far sub_s002_0e76_168a(void)
{
    struct animrec r;
    FILE *f;
    unsigned size;
    int i;

    f = fopen("tacanim.db", "rb");
    for (i = 0; i < 5; i++) {
        if (D_9c69[i].x0 == 0)
            continue;
        rewind(f);
        size = 0;
        while (!(f->_flag & 0x10)) {
            fread(&r, 14, 1, f);
            if (stricmp(r.name, D_114a[14 + i]) == 0) {
                size = r.size;
                break;
            }
        }
        sprintf(D_9085, "%s anim buf", D_114a[14 + i]);
        D_ca45[i] = alloc_buffer((unsigned long)size, D_9085);
    }
    fclose(f);
}

void far sub_s002_0e76_1750(void)
{
    sub_root_0617_011e("hitseq.dta", (char far *)F_20a5_4276);
}

/* read the party records from darkland.lst and put the default names in */
void far sub_s002_0e76_1762(void)
{
    struct lstrec r[10];
    FILE *f;
    char **p;
    int i;

    fseek(f = fopen("darkland.lst", "rb"), 0x1115L, 0);
    fread(r, 0x2e, 10, f);
    fclose(f);
    for (i = 0, p = &D_114a[2]; p < &D_114a[12]; p++, i++) {
        r[i].x1d = 0;
        strcpy(*p, r[i].name);
    }
}

int far sub_s002_0e76_17ea(void)
{
    D_c949 = -0x3748;
    D_c94f = -0x3748;
    D_c957 = -0x3748;
    D_c965 = -0x3748;
    D_c943 = -0x370b;
    D_c945 = -0x370b;
    D_c94b = -0x370b;
    D_c951 = -0x370b;
    D_c955 = -0x370b;
    D_c95d = -0x370b;
    D_c95f = -0x370b;
    D_c963 = -0x370b;
    D_c967 = -0x370b;
    D_c947 = -0x36ed;
    D_c94d = -0x36ed;
    D_c953 = -0x36ed;
    D_c959 = -0x36ed;
    D_c95b = -0x36ed;
    D_c961 = -0x36ed;
    return D_c969 = -0x36ed;
}

/* reset the battle state */
#pragma optimize("a", on)
#pragma optimize("el", off)
void far sub_s002_0e76_1830(void)
{
    int i;

    memset(D_c1d0, 0, 16);
    _fmemset(F_20a5_0000.x3474, 0, 0x20);
    memset(D_c220, 0, 16);
    memset(D_c24c, 0, 16);
    memset(D_c26f, 0, 16);
    memset(D_c29b, 0, 16);
    memset(D_c4dd, 0, 16);
    memset(D_c601, 0, 16);
    memset(D_c62c, 0, 16);
    memset(D_c658, 0, 16);
    _fmemset(F_20a5_0000.x6eea, 0, 0x20);
    _fmemset(F_20a5_0000.x6f2a, 0, 0x20);
    memset(D_c8d5, 0, 16);
    if (!(*(volatile unsigned char *)&D_9081 & 1)) {
        D_d6f0 = 1;
        D_d6f4 = 0;
        D_a6c1 = 0;
        D_c600 = 0;
        D_ca29 = 0;
        D_ca3d = -1;
    }
    D_c26e = 1;
    D_c21f = 3;
    D_d846 = 0;
    D_11c8 = 1;
    D_a695 = D_c783 = D_d4e4 = D_c865 = D_07d3 = 0;
    *(volatile char *)&D_a6b2 = 0xff;
    D_d65f = -1;
    D_d67e = -1;
    D_d845 = 0x32;
    D_c21a = 0;
    D_9084 = 0;
    D_1b98 = 0;
    D_c180 = 0;
    D_a33d = 0;
    D_a395 = 0;
    D_a531 = 0;
    D_d6eb = 0;
    D_c3cc = 0;
    for (i = 5; i < 13; i++)
        D_9c63[i].x6 = 0;
    for (i = 0; i < 5; i++) {
        D_9c63[i].x8 = i;
        D_9c63[i].x0 = (char *)&D_cab4[i];
        if (!(*(volatile unsigned char *)&D_9081 & 1))
            D_d68d[i] &= 0xff80;
    }
    for (i = 0; i < 7; i++) {
        D_ca8b[i] = D_ca75[i] = D_ca45[i] = 0;
    }
    for (i = 0; i < 13; i++) {
        if (!(*(volatile unsigned char *)&D_9081 & 1)) {
            D_ca57[i] = 7;
            D_ca64[i] = 0;
            D_d4c7[i] = 0;
            D_d4e5[i] = 0;
            D_d4f2[i] = 0;
            *(volatile char *)&D_d5e9[i] = 0;
            D_a696[i] &= 0x6e00;
            D_a6d3[i] = 0;
            D_a6c4[i] = 0;
        }
        D_9c63[i].xe = i;
        D_d70c[i].a = 0;
        D_d70c[i].b = 0;
        D_9c63[i].x3c = 0xff;
        D_d5b9[i] = 0xff;
        D_c927[i] = 0;
        D_9c63[i].x2 = 0;
        F_20a5_0000.x35ee[i].x0 = 0;
        D_d8f1[i] = 0;
        D_d8d7[i] = 0;
        D_d8bd[i] = 0;
        D_d8a3[i] = 0;
        D_d889[i] = 0;
        D_d86f[i] = 0;
        D_d862[i] = 0;
        D_d855[i] = 0;
        D_d848[i] = 0;
    }
}
#pragma optimize("", on)

#pragma optimize("a", on)
void far sub_s002_0e76_1a7a(void)
{
    int i;

    D_c83e = D_c80e = 0;
    _fmemset((char far *)F_20a5_59c8, 0, 0x870);
    for (i = 0; i < 0x40; i++) F_2e38_0bb6[i] = 0;
}
#pragma optimize("", on)
