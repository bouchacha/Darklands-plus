/* place: s029 0e76:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* orig: size=4f1a CONST=7e18+001c _DATA=26be+00b8 */
/* status: hand port (e3). All 143 functions written; 134/143 match, _DATA and CONST slots
   match the original. Open: 013a (frame 2 vs 4: the +0x35 CSE spill goes to [bp-2], the
   original has an unused slot above it; length is right), 1260 (-2 bytes; the far-pointer
   loop in build/agent/e3/r3_1260_pform.c gets the length and frame right, only the loop
   set-up differs, but it un-cancels the +2 of 1a8e), 1446 (sum operand order:
   symbol-table state, see D_fff0/D_ffe0), 1a8e (+2: case 33 cross-jump/sum order and
   case 35 loop/field reload; case 1 solved as an if/else chain in
   build/agent/e3/r3_1a8e_case1.c), 255a (cap compares preload the current value),
   2d5c 2e74 (`cmp si,-1` on a dead register: 6.00A emits `inc si`; 2e74 needs
   #pragma optimize("a", on) for the 3144 test), 4b08 (d kept in memory, slot order),
   4650 (inline far memset: 6.00 only). 1260 and 1a8e must be fixed together (their
   length errors cancel for the jump tables of 1dfe/1fb8). Several
   matches depend on the symbol table (prototype text, local names, extern lines): re-check
   the whole file after every change (build/agent/e3/keep.py; NOPROTO=1 for splice.py;
   dperm.py on 4378/4c04 when they flip). See build/agent/e3/NOTES.md. */
/* Strategic core (s029 0e76): party member accessors (member records D_9c69[5][0x80],
   index -1 = the active member D_907b), ... */

struct res { int a; int b; };
struct F_2e38_0000_t { char _p0000[0x890]; char far * f_0890[1]; };
extern struct F_2e38_0000_t far F_2e38_0000;
struct F_2e38_0000_1_t { char _p0000[0x82c]; char f_082c[1]; };
extern struct F_2e38_0000_1_t far F_2e38_0000_1;
struct loc { int type; int f2; int x; int y; int f8; int fa; int fc; int fe; int f10; int rep;
             unsigned char f14; char f15, f16, f17; int f18; int f1a; int f1c; int f1e; int f20; };
extern struct loc far * far F_2e38_10be[];   /* locations (cities ...) */
extern char far * far F_2e38_183e[];
extern char far * far F_2e38_15ba[];
extern char far * far F_290e_1fa7[];
extern char far * far F_290e_1fe7[];
extern char far * far F_290e_2107[];
extern int (far * far F_290e_2937[])();    /* saint functions */
extern void (far * far F_290e_290f[])(int);  /* event expiry handlers */
extern char far * far F_290e_207f[];
struct event {                  /* timed events, F_2e38_1b2c[300] */
    int type0;                  /* +00 */
    int f2, f4, f6, f8;
    int hour0, day0, month0, year0;     /* +0a start */
    int hour1, day1, month1, year1;     /* +12 end */
    int f1a, f1c, f1e, f20;
    int kind;                   /* +22 */
    int f24, f26, f28, f2a, f2c, f2e;
};
extern struct event far * far F_2e38_1b2c[];
extern int D_00e0;
extern int D_00e2;
extern int D_00e4;
extern int D_e3d0;
extern int D_e3d2;
extern int D_9c4a;
extern int D_2776[];
extern int D_00f0;
extern char D_01bd;
extern int D_26bc;
extern char D_270f[];
extern char D_2238[];
extern unsigned char D_9c1d;
extern char D_907b;
extern char D_907c;
extern int D_907e;
extern int D_9082;
extern int D_9078;
extern unsigned char far F_2e38_082c[][0x14];
extern char D_9c69[][0x80];
extern char D_9c7a[][0x80];
extern char D_9c93[][0x80];
extern unsigned D_9c67[][0x40];
extern char D_9cc0[][0x80];
extern char D_9cb4[][0x80];
extern char D_9cd3[][0x80];      /* number of carried items */
struct item { int code; char f2; unsigned char quality; unsigned char qty; char f5; };
extern struct item far F_2e38_003e[][64];   /* inventories, 64 items per member */
extern char far F_2e38_075f[][0x16];
extern struct item far F_2e38_0d5e[][16];  /* 16-item lists */
extern char far F_2e38_1a96[][0x1e];       /* text slots for message substitution */
extern char D_9cb9[][0x80];
extern char D_9ca0[][0x80];
extern char D_9ca6[][0x80];
extern char D_9c9e[][0x80];
extern char D_9cb2[][0x80];
extern char D_9cb3[][0x80];
extern char D_9cb8[][0x80];
extern int D_d843;
extern int D_e3ce;
extern char D_e3d4[];
extern char D_e3e3[];
extern unsigned char D_99e6;
extern char D_9a44;
extern unsigned char D_99eb;
extern char D_991d;
int far _fstrcpy();
char *strcpy(char *, const char *);
int far sub_root_0000_0c70();
int abs(int);
int far sub_root_0000_2646();
int far sub_root_0410_001a();
int far sub_root_05ba_0004();
int far sub_root_05ba_00c2();
int far sub_root_06a1_2c9e();
int far sub_s029_1367_000a();
int far sub_s029_1367_0744();
int far sub_s029_1367_0846();
int far sub_s029_1367_08ae();
int far sub_s029_1367_09cc();
int far sub_s029_1367_0b1c();
int far vec_18bf();
typedef struct _iobuf FILE;
FILE *fdopen(int, char *);
FILE *fopen(const char *, const char *);
int fseek(FILE *, long, int);
int fclose(FILE *);
int fgetc(FILE *);
unsigned fread(void *, unsigned, unsigned, FILE *);
unsigned _dos_read(int, void far *, unsigned, unsigned *);
extern char far *D_9785;
extern char D_978b[];
extern char far * far F_290e_2137[];   /* skill names */
extern char D_9085[];
char *itoa(int, char *, int);
char *strcat(char *, const char *);
int far vec_1e91();
extern char D_907a;
extern char D_907d;
extern int D_940b;
extern int D_0bf2;
extern int D_a893;
int far vec_18f1();
int far vec_1919();
int far vec_186f();
int kbhit(void);
int getch(void);
void exit_(int);
int far random_n(int);
int far vec_1f13();
int far vec_1f8b();
extern int D_a67e;
extern int D_a67c;
extern char D_a668[5][4];
extern int D_a2d7[];
extern char D_9c55[][0x80];
unsigned _dos_write(int, void far *, unsigned, unsigned *);
int far sub_root_0617_045c();
int far sub_root_047c_0545();
int far sub_root_047c_04f8();
void far *_fmemset(void far *, int, unsigned);
int far vec_1f77();
int far vec_1f81();
int far vec_1f63();
int far vec_1f6d();
int far vec_1f09();
int far vec_1f27();
int far vec_1f1d();
void srand(unsigned);
long time(long *);
int far sub_root_047c_12c3();
int far sub_root_030b_0004();
int far sub_root_044a_01f4();
int far sub_root_047c_0791();
int far sub_root_0000_1354();
int far sub_root_05f8_0139();

struct date;
void far sub_s029_1367_09ea(struct date far *, int);
unsigned far alloc_buffer(unsigned long, char *);
#define MK_FP(s, o) ((void far *)(((unsigned long)(s) << 16) | (unsigned)(o)))

void far free_buffer(unsigned, char *);
void far sub_root_0000_24bc(char *, int);
extern int D_00e6;
#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))

int far vec_1ecd();
int far vec_1ec3();
int far vec_18b5();
int far vec_1eb9();
int far sub_root_0000_1716();

int far vec_18c9();
int far vec_1e9b();
int far vec_1ea5();
int far vec_1eaf();
int far vec_1ed7();
int far vec_1ee1();
int far vec_1eeb();
int far vec_1ef5();
int far vec_1eff();
int far vec_1f31();
int far vec_1f3b();
int far vec_1f45();
int far vec_1f4f();
int far vec_1f59();
int far vec_1f77();
int far vec_1f81();
char *far sub_s029_0e76_0000(int who, char *buf);
char *far sub_s029_0e76_003e(int who, char *buf);
int far sub_s029_0e76_007c(void);
unsigned far sub_s029_0e76_00a2(int who);
int far sub_s029_0e76_00c6(int who);
void far sub_s029_0e76_00e4(int who, int v);
void far sub_s029_0e76_010e(int who, char d);
int far sub_s029_0e76_013a(int who);
int far sub_s029_0e76_01a0(int who, int k);
int far sub_s029_0e76_01c0(int who);
int far sub_s029_0e76_021e(void);
int far sub_s029_0e76_026a(void);
void far sub_s029_0e76_02b8(int who, int k, int v);
void far sub_s029_0e76_02ea(int who, int k, int d);
int far sub_s029_0e76_05ee(int who, int k);
int far sub_s029_0e76_060e(void);
int far sub_s029_0e76_0656(void);
int far sub_s029_0e76_0690(void);
int far sub_s029_0e76_06de(int who);
int far sub_s029_0e76_072e(void);
void far sub_s029_0e76_0792(int who, int v);
int far sub_s029_0e76_088e(void);
void far sub_s029_0e76_08f2(int who, int k, int pct);
void far sub_s029_0e76_0988(int who, int k, int v);
void far sub_s029_0e76_0a72(int who, int k, int d);
int far sub_s029_0e76_0b64(int who, int k);
void far sub_s029_0e76_0b84(int who, int k, int v);
void far sub_s029_0e76_0bb6(int who, int k, int d);
int far sub_s029_0e76_0c38(int who, int k);
int far sub_s029_0e76_0c58(int who);
int far sub_s029_0e76_0c76(int who, int code);
int far sub_s029_0e76_0dd8(int who, long mask);
int far sub_s029_0e76_0f54(int who, long mask);
int far sub_s029_0e76_1128(int code);
int far sub_s029_0e76_118e(int who, int code);
int far sub_s029_0e76_11fa(int who, int k);
int far sub_s029_0e76_1260(int who, unsigned bit);
void far sub_s029_0e76_1302(int who, unsigned bit);
int far sub_s029_0e76_1326(int what);
void far sub_s029_0e76_1382(int what, int v);
int far sub_s029_0e76_1396(int k);
int far sub_s029_0e76_13de(int a, int b);
int far sub_s029_0e76_1446(int a, int b);
int far sub_s029_0e76_14a4(int k);
int far sub_s029_0e76_14ec(int k);
int far sub_s029_0e76_153a(int a, int b);
int far sub_s029_0e76_15a2(int a, int b);
int far sub_s029_0e76_1600(int k);
int far sub_s029_0e76_164a(int k);
int far sub_s029_0e76_1696(int a, int b);
int far sub_s029_0e76_16fe(int k);
int far sub_s029_0e76_174a(int k);
int far sub_s029_0e76_179c(int a, int b);
int far sub_s029_0e76_1800(int k);
int far sub_s029_0e76_184a(int a, int b);
int far sub_s029_0e76_18a8(int v, int lo, int hi);
int far sub_s029_0e76_18f2(int who);
int far sub_s029_0e76_1972(void);
int far sub_s029_0e76_199c(int i);
void far sub_s029_0e76_19b4(int i, int v);
int far sub_s029_0e76_19d0(int i, int a, int b);
int far sub_s029_0e76_1a7e();
int far sub_s029_0e76_1a8e(int loc, unsigned what);
void far sub_s029_0e76_1dfe(int loc, unsigned what, int v);
void far sub_s029_0e76_1fb8(unsigned kind, int idx, int slot);
int far sub_s029_0e76_20c6(int who, int s);
void far sub_s029_0e76_2116(int who, int s);
int far sub_s029_0e76_2164(int who, int s, int v);
int far sub_s029_0e76_2180(int a, int b);
void far sub_s029_0e76_21aa(int i, char far *s, int x, int y);
void far sub_s029_0e76_2204(int a, int b);
void far sub_s029_0e76_222a();
void far sub_s029_0e76_2246();
void far sub_s029_0e76_225c();
int far sub_s029_0e76_2278(int side, int seed, int a, int b, int c, int d, int e, int f,
                           int g, int h, int k);
int far sub_s029_0e76_23e2(void);
int far sub_s029_0e76_2434(void);
int far sub_s029_0e76_2474(void);
int far sub_s029_0e76_251a(void);
void far sub_s029_0e76_255a(int hours);
void far sub_s029_0e76_281c(int months);
int far sub_s029_0e76_2930(int k, int hours);
int far sub_s029_0e76_2a32(int a, int b, int hours);
void far sub_s029_0e76_2b36(int k, int hours);
int far sub_s029_0e76_2bce(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                           int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15,
                           int a16);
int far sub_s029_0e76_2c4e(int a, int b, int c, int d, int e, int f, int g, int hours, int x, int y);
int far sub_s029_0e76_2d5c(int a, int b, int c, int k, int e, int f, int g, int hours, int x,
                           int y);
int far sub_s029_0e76_2e74(int a, int b, int k, int e, int v, int y, int op);
void far sub_s029_0e76_3034(int a, int b, int c, int kind, int type0, int f, int g,
                            int h0, int d0, int m0, int y0, int h1, int d1, int m1, int y1,
                            int flags, int x, int y, int z, int w, int i);
void far sub_s029_0e76_30fe(int i);
int far sub_s029_0e76_3144(void);
int far sub_s029_0e76_3180(int i);
int far sub_s029_0e76_31e6(int i);
int far sub_s029_0e76_324c(int k, int i);
int far sub_s029_0e76_32aa(int k);
int far sub_s029_0e76_32ce(int k);
int far sub_s029_0e76_32fe(int v);
int far sub_s029_0e76_3350(int v);
int far sub_s029_0e76_33a2(int v, int a);
int far sub_s029_0e76_3404(int v, int a, int b);
int far sub_s029_0e76_3470(int v, int a);
int far sub_s029_0e76_34d2(int v, int a, int b);
int far sub_s029_0e76_353e(int v, int a, int b);
int far sub_s029_0e76_35aa(int v, int a);
int far sub_s029_0e76_360c(int v, int a, int b);
int far sub_s029_0e76_3676(int v);
int far sub_s029_0e76_36d4(int v, int a);
int far sub_s029_0e76_3742(int v, int a, int b);
int far sub_s029_0e76_37b8(int k);
int far sub_s029_0e76_37e2(int k, int a);
int far sub_s029_0e76_382e(int v);
int far sub_s029_0e76_3878(int k, int a);
int far sub_s029_0e76_38d2(int k, int a);
int far sub_s029_0e76_392c(int k, int a, int b);
int far sub_s029_0e76_3996(int k, int a);
int far sub_s029_0e76_39f0(int k);
int far sub_s029_0e76_3a20(int k, int a);
int far sub_s029_0e76_3a86(int k, int a);
int far sub_s029_0e76_3aec(int k, int a, int b);
int far sub_s029_0e76_3b62();
int far sub_s029_0e76_3c28(int k, int t, int a, int b, int c);
int far sub_s029_0e76_3cde();
int far sub_s029_0e76_3d92(int v, int w);
void far sub_s029_0e76_3e06(int v, int w);
void far sub_s029_0e76_3f30(void);
void far sub_s029_0e76_3fb0();
void far sub_s029_0e76_3fb2(int i);
void far sub_s029_0e76_4180(int i);
void far sub_s029_0e76_4378(int j);
void far sub_s029_0e76_4436();
void far sub_s029_0e76_4442(int i);
void far sub_s029_0e76_448a(int i);
void far sub_s029_0e76_450a();
void far sub_s029_0e76_4534(int i);
void far sub_s029_0e76_4626(void);
int far sub_s029_0e76_4650(int unused, char *name, char *shortname, char *pic, int hours,
                           int y);
void far sub_s029_0e76_4aa4();
void far sub_s029_0e76_4b08();
void far sub_s029_0e76_4c04(int who, int k, int d, int h);
void far sub_s029_0e76_4d24(int who, int a, int b, int c);
int far sub_s029_0e76_4db6(int who, int a);
int far sub_s029_0e76_4e0c(int who, int a);
void far sub_s029_0e76_4e68(int city);

/* copy member name (+0x11) to buf */
char *far sub_s029_0e76_0000(int who, char *buf)
{
    if (who == -1)
        who = D_907b;
    return strcpy(buf, D_9c7a[who]);
}

/* copy member short name (+0x2a) to buf */
char *far sub_s029_0e76_003e(int who, char *buf)
{
    if (who == -1)
        who = D_907b;
    return strcpy(buf, D_9c93[who]);
}

/* number of party members that are present (status 1) and ... ([3] == 1) */
int far sub_s029_0e76_007c(void)
{
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9c69[i][3] == 1)
            n++;
    return n;
}

/* member age in years (age kept in days at +0x7e of the previous record, i.e. [-2]) */
unsigned far sub_s029_0e76_00a2(int who)
{
    if (who == -1)
        who = D_907b;
    return D_9c67[who][0] / 365;
}

/* member value +0x35 */
int far sub_s029_0e76_00c6(int who)
{
    if (who == -1)
        who = D_907b;
    return D_9c9e[who][0];
}

/* set member value +0x35 (negative -> 0) */
void far sub_s029_0e76_00e4(int who, int v)
{
    int t;

    if (who == -1)
        who = D_907b;
    if (v >= 0)
        t = v;
    else
        t = 0;
    D_9c9e[who][0] = t;
}

/* add to member value +0x35, not below 0 */
#pragma optimize("a", on)
void far sub_s029_0e76_010e(int who, char d)
{
    if (who == -1)
        who = D_907b;
    if ((D_9c9e[who][0] += d) < 0)
        D_9c9e[who][0] = 0;
}
#pragma optimize("", on)

extern int D_ffe0;  /* unused: keeps the symbol-table state (1446/179c/184a) after 013a stopped using D_9cb3 */
/* fatigue band of a member (-1: active member): with max = +0x49 + +0x4a (the two
   attributes behind endurance) and cur = +0x35 (accumulated fatigue): 0 while cur <= max/2,
   1 up to max, 2 up to 1.5 * max, else 3.  Written without a cur local and with D_9c69 as
   the base (6.00 then keeps who in SI, indexes through BX and spills cur as a CSE). */
int far sub_s029_0e76_013a(int who)
{
    unsigned max;
    if (who == -1)
        who = D_907b;
    max = D_9c69[who][0x4a] + D_9c69[who][0x49];
    if (max / 2 >= D_9c69[who][0x35]) return 0;
    if (D_9c69[who][0x35] <= max) return 1;
    if (D_9c69[who][0x35] <= max * 3 / 2) return 2;
    return 3;
}

/* member byte +0x57 + k */
int far sub_s029_0e76_01a0(int who, int k)
{
    if (who == -1)
        who = D_907b;
    return D_9cc0[who][k];
}

/* highest of the seven skills +0x57..+0x5d of a member into D_99e6 (original bug: D_99eb is set
   to every index because the braces are missing, so it ends as 6) */
int far sub_s029_0e76_01c0(int who)
{
    char t;
    int i;
    char c;

    if (who == -1)
        who = D_907b;
    *(volatile unsigned char *)&D_99e6 = 0;
    for (i = 0; i < 7; i++) {
        if ((c = *(volatile char *)&t = D_9cc0[who][i]) > D_99e6)
            D_99e6 = c;
        D_99eb = i;
    }
    if (D_99eb == -1) {
        D_99eb = 0;
        return 0;
    }
    return D_99e6;
}

/* best of skills 0-3 over all present members: D_99e6 value, D_99eb skill, D_991d member */
int far sub_s029_0e76_021e(void)
{
    int i;
    int j;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0)
            for (j = 0; j < 4; j++)
                if (D_9cc0[i][j] > D_99e6) {
                    D_99e6 = D_9cc0[i][j];
                    D_99eb = j;
                    D_991d = i;
                }
    return D_99e6;
}

/* best of skills 4-6 over all present members */
int far sub_s029_0e76_026a(void)
{
    int i;
    int j;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0)
            for (j = 4; j < 7; j++)
                if (D_9cc0[i][j] > D_99e6) {
                    D_99e6 = D_9cc0[i][j];
                    D_99eb = j;
                    D_991d = i;
                }
    return D_99e6;
}

/* set member skill byte k (clamped to 0..99) */
void far sub_s029_0e76_02b8(int who, int k, int v)
{
    if (who == -1)
        who = D_907b;
    v = sub_root_0410_001a(v, 0, 99);
    D_9cc0[who][k] = v;
}

/* add d to skill k of member who (-2 = every present member), clamped to 0..99; when
   D_26bc is set, report "<skill> changed from X to Y" (colour 15 up, 12 down) */
void far sub_s029_0e76_02ea(int who, int k, int d)
{
    char buf[50];
    char num[8];
    int i;

    if (d == 0)
        return;
    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0; i < 5; i++) {
            if (D_9c69[i][0]) {
                _fstrcpy((char far *)buf, F_290e_2137[k]);
                strcat(buf, " changed from ");
                strcat(buf, itoa(D_9cc0[i][k], num, 10));
                D_9cc0[i][k] = sub_s029_1367_0066(0, 99, D_9cc0[i][k] + d);
                if (D_26bc) {
                    strcat(buf, " to ");
                    strcat(buf, itoa(D_9cc0[i][k], num, 10));
                    strcpy(D_9085, buf);
                    if (d > 0) vec_1e91(i, 15); else vec_1e91(i, 12);
                }
            }
        }
    } else if (D_9c69[who][0]) {
        _fstrcpy((char far *)buf, F_290e_2137[k]);
        strcat(buf, " changed from ");
        strcat(buf, itoa(D_9cc0[who][k], num, 10));
        D_9cc0[who][k] = sub_s029_1367_0066(0, 99, D_9cc0[who][k] + d);
        if (D_26bc) {
            strcat(buf, " to ");
            strcat(buf, itoa(D_9cc0[who][k], num, 10));
            strcpy(D_9085, buf);
            if (d > 0) vec_1e91(who, 15); else vec_1e91(who, 12);
        }
    }
}

/* member attribute k (+0x49 + k) */
int far sub_s029_0e76_05ee(int who, int k)
{
    if (who == -1)
        who = D_907b;
    return D_9cb2[who][k];
}

/* average effective attribute (sub_06de) of the present members -> D_99e6 */
int far sub_s029_0e76_060e(void)
{
    int i;
    int sum;
    int n;

    sum = 0;
    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            sum += sub_s029_0e76_06de(i);
            n++;
        }
    if (n == 0)
        n = 1;
    D_99e6 = sum / n;
    return D_99e6;
}

/* lowest effective attribute among present members -> D_99e6, D_991d */
int far sub_s029_0e76_0656(void)
{
    int i;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            unsigned char v;

            v = sub_s029_0e76_06de(i);
            if (D_99e6 > v) {
                D_99e6 = v;
                D_991d = i;
            }
        }
    return D_99e6;
}

/* highest effective attribute among present members -> D_99e6, D_991d */
int far sub_s029_0e76_0690(void)
{
    int i;
    int v;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            v = sub_s029_0e76_06de(i);
            if (D_99e6 < v) {
                D_99e6 = v;
                D_991d = i;
            }
        }
    return D_99e6;
}

/* attribute +0x4b scaled by the fatigue band (sub_013a): full, 2/3, 1/3, 1 */
int far sub_s029_0e76_06de(int who)
{
    int a;

    a = D_9cb4[who][0];
    switch (sub_s029_0e76_013a(who)) {
    case 0:
        return a;
    case 1:
        return a * 2 / 3;
    case 2:
        return a / 3;
    case 3:
        return 1;
    }
}

/* highest effective attribute + bonus (sub_0f54(i, 0x2000L)) among present members */
int far sub_s029_0e76_072e(void)
{
    int i;
    int v;
    int b;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            v = sub_s029_0e76_06de(i);
            if ((b = sub_s029_0e76_0f54(i, 0x2000L)) + v > D_99e6) {
                D_99e6 = v + b;
                D_991d = i;
            }
        }
    return D_99e6;
}

/* apply the 0x18-point bonuses of the equipped items: slot 0 (flags bits 0-3 -> attributes
   0, 1, 3, 2) and slot 4 (bits 4, 5, 11 -> attributes 4, 5, 6) */
void far sub_s029_0e76_0792(int who, int v)
{
    long fl;
    int s;
    int t;

    if ((s = vec_1e9b(who, 0)) != -1) {
        fl = *(long far *)(F_2e38_0000.f_0890[s] + 0x20);
        if (fl & 1)
            sub_s029_0e76_4b08(who, 0, v, 0x18);
        if (fl & 2)
            sub_s029_0e76_4b08(who, 1, v, 0x18);
        if (fl & 4)
            sub_s029_0e76_4b08(who, 3, v, 0x18);
        if (fl & 8)
            sub_s029_0e76_4b08(who, 2, v, 0x18);
    }
    if ((t = vec_1e9b(who, 4)) != -1) {
        fl = *(long far *)(F_2e38_0000.f_0890[t] + 0x20);
        if (fl & 0x10)
            sub_s029_0e76_4b08(who, 4, v, 0x18);
        if (fl & 0x20)
            sub_s029_0e76_4b08(who, 5, v, 0x18);
        if (fl & 0x8000000L)
            sub_s029_0e76_4b08(who, 6, v, 0x18);
    }
}

/* lowest effective attribute + bonus (sub_0f54(i, 0x2000L)) among status-1 members */
int far sub_s029_0e76_088e(void)
{
    int i;
    int v;
    int b;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            v = sub_s029_0e76_06de(i);
            if ((b = sub_s029_0e76_0f54(i, 0x2000L)) + v < D_99e6) {
                D_99e6 = v + b;
                D_991d = i;
            }
        }
    return D_99e6;
}

void far sub_s029_0e76_08f2(int who, int k, int pct)
{
    int i;
    char *p;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0, p = D_9cb2[0] + k; i < 5; i++, p += 0x80)
            if (D_9c69[i][0] != 0)
                *p = sub_s029_1367_0066(1, 99, *p + (p[7] - *p) * pct / 100);
        return;
    }
    D_9cb2[who][k] = sub_s029_1367_0066(1, 99, D_9cb2[who][k] + (D_9cb9[who][k] - D_9cb2[who][k]) * pct / 100);
}

/* set attribute k of member who (-2 = all present members) to v, clamped to 0..99 and to
   the attribute maximum (+0x50) plus the item bonus (4e0c) */
void far sub_s029_0e76_0988(int who, int k, int v)
{
    int i;
    char *p;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0, p = D_9cb2[0] + k; i < 5; p += 0x80, i++)
            if (D_9c69[i][0]) {
                *p = sub_root_0410_001a(*p = v, 0, 99);
                if (sub_s029_0e76_4e0c(i, k) + p[7] < *p)
                    *p = sub_s029_0e76_4e0c(i, k) + p[7];
            }
    } else {
        D_9cb2[who][k] = sub_root_0410_001a(D_9cb2[who][k] = v, 0, 99);
        if (sub_s029_0e76_4e0c(who, k) + D_9cb9[who][k] < D_9cb2[who][k])
            D_9cb2[who][k] = sub_s029_0e76_4e0c(who, k) + D_9cb9[who][k];
    }
}

/* add d to attribute k of member who (-2 = all present members), clamped to 1..99 and to
   the attribute maximum plus the item bonus */
void far sub_s029_0e76_0a72(int who, int k, int d)
{
    int i;
    char *p;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0, p = D_9cb2[0] + k; i < 5; p += 0x80, i++)
            if (D_9c69[i][0]) {
                *p = sub_root_0410_001a(*p + d, 1, 99);
                if (sub_s029_0e76_4e0c(i, k) + p[7] < *p)
                    *p = sub_s029_0e76_4e0c(i, k) + p[7];
            }
    } else if (D_9c69[who][0]) {
        D_9cb2[who][k] = sub_root_0410_001a(D_9cb2[who][k] + d, 1, 99);
        if (sub_s029_0e76_4e0c(who, k) + D_9cb9[who][k] < D_9cb2[who][k])
            D_9cb2[who][k] = sub_s029_0e76_4e0c(who, k) + D_9cb9[who][k];
    }
}

/* member skill k (+0x50 + k) */
int far sub_s029_0e76_0b64(int who, int k)
{
    if (who == -1)
        who = D_907b;
    return D_9cb9[who][k];
}

/* set member skill k; the clamp to 0..99 is computed but its result is not used (bug) */
void far sub_s029_0e76_0b84(int who, int k, int v)
{
    if (who == -1)
        who = D_907b;
    sub_root_0410_001a(v, 0, 99);
    D_9cb9[who][k] = v;
}

/* add d to skill k of a member (-2: every present member), result kept in 1..99 */
void far sub_s029_0e76_0bb6(int who, int k, int d)
{
    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (who = 0; who < 5; who++)
            if (D_9c69[who][0] != 0)
                D_9cb9[who][k] = sub_root_0410_001a(D_9cb9[who][k] + d, 1, 99);
        return;
    }
    if (D_9c69[who][0] != 0)
        D_9cb9[who][k] = sub_root_0410_001a(D_9cb9[who][k] + d, 1, 99);
}

/* member byte +0x37 + k */
int far sub_s029_0e76_0c38(int who, int k)
{
    if (who == -1)
        who = D_907b;
    return D_9ca0[who][k];
}

/* member byte +0x3d */
int far sub_s029_0e76_0c58(int who)
{
    if (who == -1)
        who = D_907b;
    return D_9ca6[who][0];
}

/* number of items of type code (low 12 bits) carried by member who (-2 = all present
   members, -1 = the active member); who >= 5 counts the 16-slot store/stash lists at
   2e38:0d5e - original bug: there the quantity is read from the party inventories
   (F_2e38_003e[who - 5]) instead of from the list itself */
int far sub_s029_0e76_0c76(int who, int code)
{
    int i;
    int j;
    int n;

    n = 0;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0 && D_9c69[i][0x6a] > 0)
                for (j = 0; j < D_9c69[i][0x6a]; j++)
                    if ((F_2e38_003e[i][j].code & 0xfff) == code)
                        n += F_2e38_003e[i][j].qty;
    } else if (who == -1) {
        for (j = 0; j < D_9cd3[D_907b][0]; j++)
            if ((F_2e38_003e[D_907b][j].code & 0xfff) == code)
                n += F_2e38_003e[D_907b][j].qty;
    } else if (who < 5) {
        for (j = 0; j < D_9cd3[who][0]; j++)
            if ((F_2e38_003e[who][j].code & 0xfff) == code)
                n += F_2e38_003e[who][j].qty;
    } else {
        who -= 5;
        for (j = 0; j < 16; j++)
            if ((F_2e38_0d5e[who][j].code & 0xfff) == code)
                n += F_2e38_003e[who][j].qty;
    }
    return n;
}

/* number of carried items whose item-type flags (+0x20) intersect mask; who -2: all present
   members, -1: the active member, 5 and up: 0 */
int far sub_s029_0e76_0dd8(int who, long mask)
{
    int i;
    int j;
    int n;

    n = 0;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0 && D_9c69[i][0x6a] > 0)
                for (j = 0; j < D_9c69[i][0x6a]; j++)
                    if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[i][j].code & 0xfff] + 0x20) & mask)
                        n += F_2e38_003e[i][j].qty;
    } else if (who == -1) {
        for (j = 0; j < D_9cd3[D_907b][0]; j++)
            if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[D_907b][j].code & 0xfff] + 0x20) & mask)
                n += F_2e38_003e[D_907b][j].qty;
    } else if (who < 5) {
        for (j = 0; j < D_9cd3[who][0]; j++)
            if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[who][j].code & 0xfff] + 0x20) & mask)
                n += F_2e38_003e[who][j].qty;
    }
    return n;
}

/* best quality (+3) among the carried items whose item-type flags intersect mask; who >= 5
   scans the first 16 items of member who - 5 */
int far sub_s029_0e76_0f54(int who, long mask)
{
    int i;
    int j;
    int n;

    n = 0;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0 && D_9c69[i][0x6a] > 0)
                for (j = 0; j < D_9c69[i][0x6a]; j++)
                    if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[i][j].code & 0xfff] + 0x20) & mask)
                        if (F_2e38_003e[i][j].quality > n)
                        n = F_2e38_003e[i][j].quality;
    } else if (who == -1) {
        for (j = 0; j < D_9cd3[D_907b][0]; j++)
            if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[D_907b][j].code & 0xfff] + 0x20) & mask)
                if (F_2e38_003e[D_907b][j].quality > n)
                        n = F_2e38_003e[D_907b][j].quality;
    } else if (who < 5) {
        for (j = 0; j < D_9cd3[who][0]; j++)
            if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[who][j].code & 0xfff] + 0x20) & mask)
                if (F_2e38_003e[who][j].quality > n)
                        n = F_2e38_003e[who][j].quality;
    } else {
        who -= 5;
        for (j = 0; j < 16; j++)
            if (*(long far *)(F_2e38_0000.f_0890[F_2e38_003e[who][j].code & 0xfff] + 0x20) & mask)
                if (F_2e38_003e[who][j].quality > n)
                    n = F_2e38_003e[who][j].quality;
    }
    return n;
}

/* total quantity of item `code` carried by the present members */
int far sub_s029_0e76_1128(int code)
{
    int i;
    int j;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0 && D_9c69[i][0x6a] > 0)
            for (j = 0; j < D_9c69[i][0x6a]; j++)
                if (F_2e38_003e[i][j].code == code)
                    n += F_2e38_003e[i][j].qty;
    return n;
}

/* does member who carry item `code` (1), or have it in the 0x75f table (2), else 0 */
int far sub_s029_0e76_118e(int who, int code)
{
    int j;

    j = 0;
    if (D_9cd3[who][0] > 0)
        do {
            if ((F_2e38_003e[who][j].code & 0xfff) == code)
                return 1;
        } while (++j < D_9cd3[who][0]);
    if (F_2e38_075f[who][code])
        return 2;
    return 0;
}

/* does member who (-2: any present member) know item type k (byte table 2e38:075f, 0x16
   bytes per member)?  1 or 0 */
int far sub_s029_0e76_11fa(int who, int k)
{
    int i;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0 && F_2e38_075f[i][k])
                return 1;
    } else if (F_2e38_075f[who][k])
        return 1;
    return 0;
}

/* test event/knowledge bit `bit` of a member (-2: any present member) */
int far sub_s029_0e76_1260(int who, unsigned bit)
{
    int mask;
    int i;

    mask = 0x80 >> (bit & 7);
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0 && (F_2e38_082c[i][bit >> 3] & mask))
                return 1;
        return 0;
    }
    if (who == -1)
        return (F_2e38_082c[D_907b][bit >> 3] & mask) != 0;
    if (who < 5)
        return (F_2e38_082c[who][bit >> 3] & mask) != 0;
}

/* set bit `bit` of a member */
void far sub_s029_0e76_1302(int who, unsigned bit)
{
    F_2e38_082c[who][bit >> 3] |= 0x80 >> (bit & 7);
}

int far sub_s029_0e76_1326(int what)
{
    int i;

    switch (what) {
    case 4:
        return D_9082;
    case 5:
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0) {
                if (D_9c69[i][0] != 1)
                    return 0;
                if (sub_s029_0e76_0dd8(i, 0x2000L) == 0)
                    return 0;
            }
        return 1;
    case 6:
        return D_9078;
    }
    return 0;
}

/* setter for the query above (only 4 -> D_9082) */
void far sub_s029_0e76_1382(int what, int v)
{
    switch (what) {
    case 4:
        D_9082 = v;
        break;
    }
}

/* lowest skill k among status-1 members -> D_99e6, D_991d */
int far sub_s029_0e76_1396(int k)
{
    int i;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_99e6 > D_9cc0[i][k]) {
            D_99e6 = D_9cc0[i][k];
            D_991d = i;
        }
    return D_99e6;
}

/* lowest of skills a and b over status-1 members -> D_99e6, D_991d */
int far sub_s029_0e76_13de(int a, int b)
{
    int i;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            if (D_99e6 > D_9cc0[i][a]) {
                D_99e6 = D_9cc0[i][a];
                D_991d = i;
            }
            if (D_99e6 > D_9cc0[i][b]) {
                D_99e6 = D_9cc0[i][b];
                D_991d = i;
            }
        }
    return D_99e6;
}

/* lowest sum of skills a + b over status-1 members */
int far sub_s029_0e76_1446(int a, int b)
{
    int i;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9cc0[i][b] + D_9cc0[i][a] < D_99e6) {
            D_99e6 = D_9cc0[i][b] + D_9cc0[i][a];
            D_991d = i;
        }
    return D_99e6;
}

/* highest skill k over status-1 members */
int far sub_s029_0e76_14a4(int k)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_99e6 < D_9cc0[i][k]) {
            D_99e6 = D_9cc0[i][k];
            D_991d = i;
        }
    return D_99e6;
}

/* highest skill k over status-1 members with flag +3 == 1 */
int far sub_s029_0e76_14ec(int k)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9c69[i][3] == 1 && D_99e6 < D_9cc0[i][k]) {
            D_99e6 = D_9cc0[i][k];
            D_991d = i;
        }
    return D_99e6;
}

/* highest of skills a and b over status-1 members */
int far sub_s029_0e76_153a(int a, int b)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            if (D_99e6 < D_9cc0[i][a]) {
                D_99e6 = D_9cc0[i][a];
                D_991d = i;
            }
            if (D_99e6 < D_9cc0[i][b]) {
                D_99e6 = D_9cc0[i][b];
                D_991d = i;
            }
        }
    return D_99e6;
}

/* highest sum of skills a + b over status-1 members */
int far sub_s029_0e76_15a2(int a, int b)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9cc0[i][a] + D_9cc0[i][b] > D_99e6) {
            D_99e6 = D_9cc0[i][a] + D_9cc0[i][b];
            D_991d = i;
        }
    return D_99e6;
}

/* average skill k over the members with status 1 (D_99e6) */
int far sub_s029_0e76_1600(int k)
{
    int i;
    int sum;
    int n;

    sum = n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1) {
            sum += D_9cc0[i][k];
            n++;
        }
    n = n ? n : 1;
    return D_99e6 = sum / n;
}

/* lowest attribute k over status-1 members -> D_9c1d (D_9a44, D_991d: member) */
int far sub_s029_0e76_164a(int k)
{
    int i;

    D_9c1d = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9cb2[i][k] < D_9c1d) {
            D_9c1d = D_9cb2[i][k];
            D_9a44 = i;
            D_991d = i;
        }
    return D_9c1d;
}

/* lowest sum of attributes a + b over status-1 members; stores 2*a (bug) */
int far sub_s029_0e76_1696(int a, int b)
{
    int i;

    D_99e6 = 199;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9cb2[i][a] + D_9cb2[i][b] < D_99e6) {
            D_99e6 = D_9cb2[i][a] * 2;
            D_9a44 = i;
            D_991d = i;
        }
    return D_99e6;
}

/* highest attribute k over status-1 members -> D_9c1d */
int far sub_s029_0e76_16fe(int k)
{
    int i;

    D_9c1d = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 1 && D_9cb2[i][k] > D_9c1d) {
            D_9c1d = D_9cb2[i][k];
            D_9a44 = i;
            D_991d = i;
        }
    return D_9c1d;
}

/* highest attribute k over present members with flag +3 == 1 -> D_9c1d */
int far sub_s029_0e76_174a(int k)
{
    int i;

    D_9c1d = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0 && D_9c69[i][3] == 1 && D_9cb2[i][k] > D_9c1d) {
            D_9c1d = D_9cb2[i][k];
            D_9a44 = i;
            D_991d = i;
        }
    return D_9c1d;
}

/* highest sum of attributes a + b over present members; stores 2*a (bug) */
int far sub_s029_0e76_179c(int a, int b)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0 && D_9cb2[i][a] + D_9cb2[i][b] > D_99e6) {
            D_99e6 = D_9cb2[i][a] * 2;
            D_991d = i;
        }
    return D_99e6;
}

/* average attribute k over the present members (D_9c1d) */
int far sub_s029_0e76_1800(int k)
{
    int i;
    int sum;
    int n;

    sum = n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0) {
            sum += D_9cb2[i][k];
            n++;
        }
    n = n ? n : 1;
    return D_9c1d = sum / n;
}

/* highest attribute a + skill b over present members */
extern int D_fff0;  /* unused: shifts the symbol table so 184a gets the original operand order */
int far sub_s029_0e76_184a(int a, int b)
{
    int i;

    D_99e6 = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0 && D_9cb2[i][a] + D_9cc0[i][b] > D_99e6) {
            D_99e6 = D_9cc0[i][b] + D_9cb2[i][a];
            D_991d = i;
        }
    return D_99e6;
}

int far sub_s029_0e76_18a8(int v, int lo, int hi)
{
    sub_s029_1367_000a(v);
    if (abs(100 - v) >= sub_root_0000_0c70(100))
        return sub_root_0000_0c70(hi - lo) + lo;
    return lo;
}

/* fatigue state of a member: 0 fresh .. 0x13, -1 exhausted ((+0x49 + +0x4a) * 5/4 vs +0x35) */
int far sub_s029_0e76_18f2(int who)
{
    int m;

    m = (D_9c69[who][0x49] + D_9c69[who][0x4a]) * 5 / 4;
    if (m >= D_9c69[who][0x35])
        return 0;
    if (D_9c69[who][0x35] <= m && D_9c69[who][0x35] >= m >> 1)
        return 9;
    if (D_9c69[who][0x35] <= m * 3 / 2 && D_9c69[who][0x35] >= m)
        return 0x13;
    return -1;
}

/* 0 if any member has status 2 or 3, else 1 */
int far sub_s029_0e76_1972(void)
{
    int i;

    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 3 || D_9c69[i][0] == 2)
            return 0;
    return 1;
}

/* location value +0x12 (reputation?) */
int far sub_s029_0e76_199c(int i)
{
    return F_2e38_10be[i]->rep;
}

void far sub_s029_0e76_19b4(int i, int v)
{
    F_2e38_10be[i]->rep = v;
}

int far sub_s029_0e76_19d0(int i, int a, int b)
{
    int r;
    int aa;
    int ab;

    r = F_2e38_10be[i]->rep;
    if (100 - abs(F_2e38_10be[i]->rep) >= sub_root_0000_0c70(100)) {
        aa = abs(a);
        ab = abs(b);
        if (b > 0)
            r += sub_root_0000_0c70(ab - aa) + a;
        else
            r -= aa + sub_root_0000_0c70(ab - aa);
        r = sub_root_0410_001a(r, -99, 99);
        F_2e38_10be[i]->rep = r;
    }
    return r;
}

int far sub_s029_0e76_1a7e(int p_6)
{
    sub_s029_0e76_1a8e(D_907e, p_6);
}

/* location/city query: what = 0 reputation of location loc, 1 reputation band (0 best ..
   5 worst), 2 city size class (1 outside a city, 2 for locations 0x5c and up), 3-11 city
   bytes (+0x63, 62, 64, 67, 65, 68, 6a, 69, 66), 12-14 and 32 location bytes (+0x15-17,
   +0x14), 15-31 city flag bits (+0x5e), 33 city seed (+0x56) + D_9c4a, 34 city word +0x46,
   35 city festival within 14 days of the start of the month (+0x6c day of year),
   37/38 location words +0x1c/+0x1a, 39 current location has +0x18 set, 40/41 city word
   +0x5a/+0x58 depending on location flag 0x80.  Other values return garbage. */
int far sub_s029_0e76_1a8e(int loc, unsigned what)
{
    int r;
    int d;
    int i;
    int t;

    switch (what) {
    case 0:
        return F_2e38_10be[loc]->rep;
    case 1:
        r = F_2e38_10be[loc]->rep;
        *(volatile int *)&i = r > -75 ? r > -40 ? r > -10 ? r > 10 ? r > 50 ? 0 : 1 : 2 : 3 : 4 : 5;
        break;
    case 2:
        t = D_907e;
        if (t == -1)
            return 1;
        if (t >= 0x5c)
            return 2;
        return *(int far *)(D_9785 + 0x40);
    case 3:
        return D_9785[0x63];
    case 4:
        return D_9785[0x62];
    case 5:
        return D_9785[0x64];
    case 6:
        return D_9785[0x67];
    case 7:
        return D_9785[0x65];
    case 8:
        return D_9785[0x68];
    case 9:
        return D_9785[0x6a];
    case 10:
        return D_9785[0x69];
    case 11:
        return D_9785[0x66];
    case 12:
        return F_2e38_10be[loc]->f15;
    case 13:
        return F_2e38_10be[loc]->f16;
    case 14:
        return F_2e38_10be[loc]->f17;
    case 15:
        return *(int far *)(D_9785 + 0x5e) & 4;
    case 16:
        return *(int far *)(D_9785 + 0x5e) & 1;
    case 17:
        return *(int far *)(D_9785 + 0x5e) & 8;
    case 18:
        return *(int far *)(D_9785 + 0x5e) & 0x800;
    case 20:
        return *(int far *)(D_9785 + 0x5e) & 0x40;
    case 23:
        return *(int far *)(D_9785 + 0x5e) & 0x400;
    case 24:
        return *(int far *)(D_9785 + 0x5e) & 2;
    case 26:
        return *(int far *)(D_9785 + 0x5e) & 0x20;
    case 27:
        return *(int far *)(D_9785 + 0x5e) & 0x100;
    case 28:
        return *(int far *)(D_9785 + 0x5e) & 0x80;
    case 29:
        return *(int far *)(D_9785 + 0x5e) & 0x10;
    case 31:
        return *(int far *)(D_9785 + 0x5e) & 0x200;
    case 32:
        return (char)F_2e38_10be[loc]->f14;
    case 33:
        t = D_907e;
        if (t == -1)
            return D_9c4a + D_e3d2 + D_e3d0;
        if (t >= 0x5c)
            return t + D_9c4a;
        return *(int far *)(D_9785 + 0x56) + D_9c4a;
    case 34:
        return *(int far *)(D_9785 + 0x46);
    case 35:
        for (d = i = 0; i < D_00e4; i++)
            d += D_2776[i];
        if (*(int far *)(D_9785 + 0x6c) > d)
            r = *(int far *)(D_9785 + 0x6c) - d;
        else
            r = d - *(int far *)(D_9785 + 0x6c);
        if (r <= 14)
            return 1;
        return 0;
    case 37:
        return F_2e38_10be[loc]->f1c;
    case 38:
        return F_2e38_10be[loc]->f1a;
    case 39:
        if (F_2e38_10be[D_907e]->f18 != -1)
            return 1;
        return 0;
    case 40:
        if (F_2e38_10be[D_907e]->f14 & 0x80)
            return *(int far *)(D_9785 + 0x5a);
        return *(int far *)(D_9785 + 0x58);
    case 41:
        if (F_2e38_10be[D_907e]->f14 & 0x80)
            return *(int far *)(D_9785 + 0x58);
        return *(int far *)(D_9785 + 0x5a);
    }
}

/* location/city setter, the counterpart of 1a8e: what = 0 reputation, 2 city word +0x40,
   3-8 and 11 city bytes, 12-14 location bytes +0x15-17, 32 sets flag bits in +0x14,
   37 records v in the three-slot list +0x1c/+0x1e/+0x20 (only while +0x20 is free) -
   original bug: instead of shifting the old entries along, every occupied slot is
   overwritten with v */
void far sub_s029_0e76_1dfe(int loc, unsigned what, int v)
{
    switch (what) {
    case 0:
        F_2e38_10be[loc]->rep = v;
        break;
    case 2:
        *(int far *)(D_9785 + 0x40) = v;
        break;
    case 3:
        D_9785[0x63] = v;
        break;
    case 4:
        D_9785[0x62] = v;
        break;
    case 5:
        D_9785[0x64] = v;
        break;
    case 6:
        D_9785[0x66] = v;
        break;
    case 7:
        D_9785[0x65] = v;
        break;
    case 8:
        D_9785[0x68] = v;
        break;
    case 11:
        D_9785[0x67] = v;
        return;
    case 12:
        F_2e38_10be[loc]->f15 = v;
        return;
    case 13:
        F_2e38_10be[loc]->f16 = v;
        return;
    case 14:
        F_2e38_10be[loc]->f17 = v;
        return;
    case 32:
        F_2e38_10be[loc]->f14 |= v;
        return;
    case 37:
        if (F_2e38_10be[loc]->f20 == 0) {
            if (F_2e38_10be[loc]->f1e)
                F_2e38_10be[loc]->f20 = v;
            if (F_2e38_10be[loc]->f1c)
                F_2e38_10be[loc]->f1e = v;
            F_2e38_10be[loc]->f1c = v;
        }
        break;
    }
}

/* copy a name into text slot `slot` (2e38:1a96, 30 bytes each): kind 0 item name, 1 and 2
   two other name tables, 3/4 time-of-day word for hour-ish value idx (1367_0744 band,
   1367_0846 text), 5 the next band (wrapping after 7), 6-8 three 290e name tables */
void far sub_s029_0e76_1fb8(unsigned kind, int idx, int slot)
{
    int t;

    switch (kind) {
    case 0:
        _fstrcpy(F_2e38_1a96[slot], F_2e38_0000.f_0890[idx]);
        break;
    case 1:
        _fstrcpy(F_2e38_1a96[slot], F_2e38_183e[idx]);
        break;
    case 2:
        _fstrcpy(F_2e38_1a96[slot], F_2e38_15ba[idx]);
        break;
    case 3:
    case 4:
        sub_s029_1367_0846(sub_s029_1367_0744(idx), F_2e38_1a96[slot]);
        return;
    case 5:
        t = sub_s029_1367_0744(idx) + 1;
        if (t > 7)
            t = 0;
        sub_s029_1367_0846(t, F_2e38_1a96[slot]);
        return;
    case 6:
        _fstrcpy(F_2e38_1a96[slot], F_290e_1fa7[idx]);
        break;
    case 7:
        _fstrcpy(F_2e38_1a96[slot], F_290e_1fe7[idx]);
        break;
    case 8:
        _fstrcpy(F_2e38_1a96[slot], F_290e_2107[idx]);
        break;
    }
}

/* invoke saint s for a member if his divine favour (+0x4f) covers the cost (mode 2) */
int far sub_s029_0e76_20c6(int who, int s)
{
    int cost;

    cost = F_290e_2937[s](who, 0, 2);
    if (D_9cb8[who][0] < cost)
        return 0;
    return F_290e_2937[s](who, cost, 7);
}

void far sub_s029_0e76_2116(int who, int s)
{
    int cost;

    cost = F_290e_2937[s](who, 0, 3);
    if (D_9cb8[who][0] < cost)
        cost = D_9cb8[who][0];
    F_290e_2937[s](who, cost, 7);
}

int far sub_s029_0e76_2164(int who, int s, int v)
{
    return F_290e_2937[s](who, v, 7);
}

int far sub_s029_0e76_2180(int a, int b)
{
    int r;

    r = vec_1eaf(a, b, 1);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
    return r;
}

/* show text s (default: string i of table 207f) */
void far sub_s029_0e76_21aa(int i, char far *s, int x, int y)
{
    char far *p;

    if (s != 0)
        p = s;
    else
        p = F_290e_207f[i];
    vec_1eff(i, p, x, y);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

void far sub_s029_0e76_2204(int a, int b)
{
    vec_1ef5(a, b);
    if (!D_907c)
        vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

void far sub_s029_0e76_222a(int p_6)
{
    vec_1eeb(p_6);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

void far sub_s029_0e76_2246(void)
{
    vec_1ee1();
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

static int pad_26e6 = 0;    /* unreferenced word between the strings at 26e6 */
void far sub_s029_0e76_225c(int p_6)
{
    vec_1ed7(p_6);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

/* run a battle: vec_18bf(0, side, encounter seed, ...) with side -1 = random of 2 and
   seed -2 = this location (D_9c4a + D_907e), -1 = random of 1000 - original bug: the
   location seed then gets D_9c4a added a second time.  Afterwards members with status 3
   are removed, everybody alive is set back to status 1, and if nobody is left the game
   ends ("death" screen, exit).  Returns D_a893. */
int far sub_s029_0e76_2278(int side, int seed, int a, int b, int c, int d, int e, int f,
                           int g, int h, int k)
{
    int i;
    int n;

    vec_18c9();
    D_907a = 1;
    if (side == -1)
        side = sub_root_0000_0c70(2);
    if (seed == -2)
        seed = D_9c4a + D_907e;
    else if (seed == -1)
        seed = sub_root_0000_0c70(1000);
    vec_18bf(0, side, seed + D_9c4a, a, b, c, d, e, f, g, h, k);
    sub_root_0000_2646("after battle");
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 3) {
            D_9c69[i][0] = 0;
            vec_18b5(i);
        }
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0 && D_9c69[i][0] != 5)
            D_9c69[i][0] = 1;
    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0)
            n++;
    if (n == 0) {
        vec_18f1(0x12);
        sub_root_047c_12c3();
        if (D_0bf2 == 0) {
            sub_root_030b_0004("death");
            if (kbhit())
                getch();
        }
        sub_root_044a_01f4();
        sub_root_047c_0791();
        vec_186f();
        sub_root_0000_1354();
        if (D_907d == 1)
            sub_root_05f8_0139(D_940b);
        exit_(0);
    }
    vec_1919();
    sub_s029_1367_05c8(0);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
    return D_a893;
}

/* members with status 5: message 0x4d with their name, then vec_1ec3; returns how many */
int far sub_s029_0e76_23e2(void)
{
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 5) {
            sub_s029_0e76_2a32(0x4d, vec_1ecd(i), 0x2238);
            vec_1ec3(i);
            n++;
        }
    sub_root_0000_1716(0x100);
    return n;
}

/* members with status 5 are removed (status 0, vec_18b5) */
int far sub_s029_0e76_2434(void)
{
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 5) {
            D_9c69[i][0] = 0;
            vec_18b5(i);
            n++;
        }
    sub_root_0000_1716(0x100);
    return n;
}

int far sub_s029_0e76_2474(void)
{
    int i;
    int n;
    int last;

    last = -1;
    if (sub_root_0000_0c70(2)) {
        n = 0;
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] == 5) {
                if (n == 0) {
                    D_9c69[i][0] = 0;
                    vec_18b5(i);
                    last = i;
                } else
                    D_9c69[i][0] = 1;
                n++;
            }
    } else {
        for (i = 4, n = 0; i >= 0; i--)
            if (D_9c69[i][0] == 5) {
                if (n == 0) {
                    D_9c69[i][0] = 0;
                    vec_18b5(i);
                    last = i;
                } else
                    D_9c69[i][0] = 1;
                n++;
            }
    }
    if (n)
        D_991d = last;
    sub_root_0000_1716(0x100);
    return n;
}

/* members with status 5 return to status 1 (vec_1eb9) */
int far sub_s029_0e76_251a(void)
{
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 5) {
            D_9c69[i][0] = 1;
            vec_1eb9(i);
            n++;
        }
    sub_root_0000_1716(0x100);
    return n;
}

/* hourly party update (called from the clock, hours = hours being advanced): pick a new
   active member if the current one is not status 1; members with status 3 are dropped;
   attribute 0 (endurance) recovers: set to max - 4 when more than 4 below the maximum,
   +1 when 3-4 below; when the day changes, attribute 6 gains clamp(1..5, skill 8 /
   (20 + rnd 5)), attribute 1 +1 when the party's best skill 13 beats rnd 150, attributes
   2, 3, 5 +1 with 11% each; attributes 6, 1, 3, 2, 5 are then capped at max + item bonus.
   If nobody is left: "death" screen and exit.  Empty member slots get event 0x43 cleared. */
void far sub_s029_0e76_255a(int hours)
{
    int i;
    int n;
    int d;
    union { int d; char t; } u;
    int e;
    int r;
    char *p;

    if (D_9c69[D_907b][0] != 1)
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] == 1) {
                D_907b = i;
                break;
            }
    for (i = 0, p = D_9cb2[0]; p < D_9cb2[5]; i++, p += 0x80) {
        if ((u.t = p[-0x49]) == 0)
            continue;
        if (u.t == 3) {
            p[-0x49] = 0;
            continue;
        }
        if ((d = p[7] - *p) > 4)
            *p = p[7] - 4;
        else if (d > 2)
            (*p)++;
        if (D_00e0 + hours >= 24) {
            p[6] += sub_s029_1367_0066(1, 5, p[0x16] / (sub_root_0000_0c70(5) + 20));
            if ((r = random_n(150)) <= sub_s029_0e76_14a4(13))
                p[1]++;
            if (random_n(100) <= 10)
                p[2]++;
            if (random_n(100) <= 10)
                p[3]++;
            if (random_n(100) <= 10)
                p[5]++;
        }
        if (p[6] > sub_s029_0e76_4e0c(i, 6) + p[13])
            p[6] = sub_s029_0e76_4e0c(i, 6) + p[13];
        if (p[1] > sub_s029_0e76_4e0c(i, 1) + p[8])
            p[1] = sub_s029_0e76_4e0c(i, 1) + p[8];
        if (p[3] > sub_s029_0e76_4e0c(i, 3) + p[10])
            p[3] = sub_s029_0e76_4e0c(i, 3) + p[10];
        if (p[2] > sub_s029_0e76_4e0c(i, 2) + p[9])
            p[2] = sub_s029_0e76_4e0c(i, 2) + p[9];
        if (p[5] > sub_s029_0e76_4e0c(i, 5) + p[12])
            p[5] = sub_s029_0e76_4e0c(i, 5) + p[12];
    }
    sub_root_0000_1716(0x100);
    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0)
            n++;
    if (n == 0) {
        sub_root_047c_12c3();
        vec_18f1(0x12);
        vec_18c9();
        if (D_0bf2 == n) {
            sub_root_030b_0004("death");
            if (kbhit())
                getch();
        }
        sub_root_044a_01f4();
        sub_root_047c_0791();
        vec_186f();
        sub_root_0000_1354();
        if (D_907d == 1)
            sub_root_05f8_0139(D_940b);
        exit_(n);
    }
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 0 && (e = sub_s029_0e76_3b62(0x43, i, -1, -1, -1, -1)) != -1)
            sub_s029_0e76_30fe(e);
    srand((unsigned)time(0L));
}

/* monthly hook (months = months elapsed so far): every member whose birth month
   ((D_9c4a + month) % 12 == member index) ages one year (+1 in the word before the record);
   every third month the nearest location of type 0x1a to the current one turns into a type 2
   location (flags cleared, +0xc = 0x93, +0xe = 0x92, +8 = 4); then without an active event
   of type 4 a 1-in-4 chance of vec_1f09, without type 12 a 1-in-3 chance of vec_1f27, and
   without type 1 always vec_1f1d */
#pragma optimize("a", on)
void far sub_s029_0e76_281c(int months)
{
    int i;
    int s;

    D_26bc = 1;
    for (i = 0; i < 5; i++)
        if (*(char *)&D_9c67[i][1] != 0 && (unsigned)(D_9c4a + D_00e4) % 12 == i)
            D_9c67[i][0]++;
    if (months % 3 == 0) {
        s = vec_1f13(F_2e38_10be[D_907e]->x, F_2e38_10be[D_907e]->y, 0x1a);
        if (D_907e != s && s != -1) {
            F_2e38_10be[s]->type = 2;
            F_2e38_10be[s]->f14 = 0;
            F_2e38_10be[s]->fc = 0x93;
            F_2e38_10be[s]->fe = 0x92;
            F_2e38_10be[s]->f8 = 4;
        }
    }
    if (!sub_s029_0e76_32fe(4) && !sub_root_0000_0c70(4))
        vec_1f09();
    if (!sub_s029_0e76_32fe(12) && !sub_root_0000_0c70(3))
        vec_1f27();
    if (!sub_s029_0e76_32fe(1))
        vec_1f1d();
}
#pragma optimize("", on)

#pragma optimize("e", off)
int far sub_s029_0e76_2930(int k, int hours)
{
    volatile struct date { int hour, day, month, year; } forever;
    int i;

    forever.hour = 23;
    forever.day = 31;
    forever.month = 12;
    forever.year = 1499;
    if ((i = sub_s029_0e76_3144()) == -1)
        return -1;
    F_2e38_1b2c[i] = MK_FP(alloc_buffer(0x30L, 0), 0);
    if (hours == 9999)
        sub_s029_0e76_3034(-2, D_907e, 0, k, 0, 100, 0, D_00e0, D_00e2, D_00e4, D_00e6,
                           forever.hour, forever.day, forever.month, forever.year, 0, 0, 0, 0, 0, i);
    else {
        sub_s029_0e76_3034(-2, D_907e, 0, k, 0, 100, 0, D_00e0, D_00e2, D_00e4, D_00e6,
                           D_00e0, D_00e2, D_00e4, D_00e6, 0, 0, 0, 0, 0, i);
        sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[i]->hour1, hours);
    }
    return D_e3ce = i;
}
#pragma optimize("", on)

/* like 2930, with two parameters: create event (-2, here, 0, a, b, 100, ...) running from
   now for `hours` hours (9999 = until 31.12.1499); returns its index */
#pragma optimize("e", off)
int far sub_s029_0e76_2a32(int a, int b, int hours)
{
    volatile struct date { int hour, day, month, year; } forever;
    int i;

    forever.hour = 23;
    forever.day = 31;
    forever.month = 12;
    forever.year = 1499;
    if ((i = sub_s029_0e76_3144()) == -1)
        return -1;
    F_2e38_1b2c[i] = MK_FP(alloc_buffer(0x30L, 0), 0);
    if (hours == 9999)
        sub_s029_0e76_3034(-2, D_907e, 0, a, b, 100, 0, D_00e0, D_00e2, D_00e4, D_00e6,
                           forever.hour, forever.day, forever.month, forever.year, 0, 0, 0, 0, 0, i);
    else {
        sub_s029_0e76_3034(-2, D_907e, 0, a, b, 100, 0, D_00e0, D_00e2, D_00e4, D_00e6,
                           D_00e0, D_00e2, D_00e4, D_00e6, 0, 0, 0, 0, 0, i);
        sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[i]->hour1, hours);
    }
    return D_e3ce = i;
}
#pragma optimize("", on)

void far sub_s029_0e76_2b36(int k, int hours)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->type0 == 0 && F_2e38_1b2c[i]->f1c == D_907e &&
            F_2e38_1b2c[i]->f1a == -2 && F_2e38_1b2c[i]->f28 == 0)
            break;
    } while (++i < 300);
    if (i < 300 && i != -1)
        sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[i]->hour1, hours);
    else
        sub_s029_0e76_2930(k, hours);
    D_e3ce = i;
}

/* create an event from 17 values (sub_3034 fills it in); returns its index (-1: table full) */
#pragma optimize("e", off)
int far sub_s029_0e76_2bce(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                           int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15,
                           int a16)
{
    int i;

    if ((i = sub_s029_0e76_3144()) == -1)
        return -1;
    F_2e38_1b2c[i] = MK_FP(alloc_buffer(0x30L, 0), 0);
    sub_s029_0e76_3034(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16,
                       0, 0, 0, i);
    return D_e3ce = i;
}
#pragma optimize("", on)

#pragma optimize("e", off)
int far sub_s029_0e76_2c4e(int a, int b, int c, int d, int e, int f, int g, int hours, int x, int y)
{
    volatile struct date { int hour, day, month, year; } forever;
    int i;

    forever.hour = 23;
    forever.day = 31;
    forever.month = 12;
    forever.year = 1499;
    if ((i = sub_s029_0e76_3144()) == -1)
        return -1;
    F_2e38_1b2c[i] = MK_FP(alloc_buffer(0x30L, 0), 0);
    if (hours == 9999)
        sub_s029_0e76_3034(a, b, c, d, e, f, g, D_00e0, D_00e2, D_00e4, D_00e6,
                           forever.hour, forever.day, forever.month, forever.year, x, y, 0, 0, 0, i);
    else {
        sub_s029_0e76_3034(a, b, c, d, e, f, g, D_00e0, D_00e2, D_00e4, D_00e6,
                           D_00e0, D_00e2, D_00e4, D_00e6, x, y, 0, 0, 0, i);
        sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[i]->hour1, hours);
    }
    return D_e3ce = i;
}
#pragma optimize("", on)

/* find the active event (kind k) with type0 e, owner f1c == b (b -1 = any), f1a == a and
   f28 == y and extend it by `hours`; if there is none, create it (a, b, c, k, e, f, g, from
   now, x, y) and let it run for `hours`.  Returns the event index (-1: table full) */
int far sub_s029_0e76_2d5c(int a, int b, int c, int k, int e, int f, int g, int hours, int x,
                           int y)
{
    int i;
    int n;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->type0 == e && (F_2e38_1b2c[i]->f1c == b || b == -1) &&
            F_2e38_1b2c[i]->f1a == a && F_2e38_1b2c[i]->f28 == y)
            break;
    } while (++i < 300);
    if (i >= 300 || (*(volatile int *)&n = i) == -1) {
        if ((n = sub_s029_0e76_3144()) == -1)
            return -1;
        F_2e38_1b2c[n] = MK_FP(alloc_buffer(0x30L, 0), 0);
        sub_s029_0e76_3034(a, b, c, k, e, f, g, D_00e0, D_00e2, D_00e4, D_00e6,
                           D_00e0, D_00e2, D_00e4, D_00e6, x, y, 0, 0, 0, n);
    }
    sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[n]->hour1, hours);
    return D_e3ce = n;
}

/* find the active event (kind k) matching type0 e, f1c b, f1a a and f28 y (-1 = any) or
   create one (lasting 999 hours) and update its value +0x1e with v: op 0 set, 1 add,
   2 and, 3 or.  Returns the event index (-1: table full) */
int far sub_s029_0e76_2e74(int a, int b, int k, int e, int v, int y, int op)
{
    int i;
    int n;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            break;
        if ((e == -1 || F_2e38_1b2c[i]->type0 == e) && (b == -1 || F_2e38_1b2c[i]->f1c == b) &&
            (a == -1 || F_2e38_1b2c[i]->f1a == a) && (y == -1 || F_2e38_1b2c[i]->f28 == y))
            break;
    } while (++i < 300);
    if (i >= 300 || (*(volatile int *)&n = i) == -1) {
        if ((n = sub_s029_0e76_3144()) == -1)
            return -1;
        F_2e38_1b2c[n] = MK_FP(alloc_buffer(0x30L, 0), 0);
        sub_s029_0e76_3034(a, b, 0, k, e, v, 0, D_00e0, D_00e2, D_00e4, D_00e6,
                           D_00e0, D_00e2, D_00e4, D_00e6, 0, y, 0, 0, 0, n);
        sub_s029_1367_09ea((struct date far *)&F_2e38_1b2c[n]->hour1, 999);
    }
    switch (op) {
    case 0:
        F_2e38_1b2c[n]->f1e = v;
        break;
    case 1:
        F_2e38_1b2c[n]->f1e += v;
        break;
    case 2:
        F_2e38_1b2c[n]->f1e &= v;
        break;
    case 3:
        F_2e38_1b2c[n]->f1e |= v;
        break;
    }
    return D_e3ce = n;
}

#pragma optimize("a", on)
void far sub_s029_0e76_3034(int a, int b, int c, int kind, int type0, int f, int g,
                            int h0, int d0, int m0, int y0, int h1, int d1, int m1, int y1,
                            int flags, int x, int y, int z, int w, int i)
{
    F_2e38_1b2c[i]->f20 = c;
    F_2e38_1b2c[i]->f1a = a;
    F_2e38_1b2c[i]->f1c = b;
    F_2e38_1b2c[i]->f1e = f;
    F_2e38_1b2c[i]->kind = kind;
    F_2e38_1b2c[i]->f24 = flags;
    F_2e38_1b2c[i]->type0 = type0;
    F_2e38_1b2c[i]->f26 = g;
    F_2e38_1b2c[i]->f28 = x;
    F_2e38_1b2c[i]->f2a = y;
    F_2e38_1b2c[i]->f2c = z;
    F_2e38_1b2c[i]->f2e = w;
    F_2e38_1b2c[i]->hour1 = h1;
    F_2e38_1b2c[i]->day1 = d1;
    F_2e38_1b2c[i]->month1 = m1;
    F_2e38_1b2c[i]->year1 = y1;
    F_2e38_1b2c[i]->hour0 = h0;
    F_2e38_1b2c[i]->day0 = d0;
    F_2e38_1b2c[i]->month0 = m0;
    F_2e38_1b2c[i]->year0 = y0;
    F_2e38_1b2c[i]->f2 = D_00e0;
    F_2e38_1b2c[i]->f4 = D_00e2;
    F_2e38_1b2c[i]->f6 = D_00e4;
    F_2e38_1b2c[i]->f8 = D_00e6;
}
#pragma optimize("", on)

/* free event i */
void far sub_s029_0e76_30fe(int i)
{
    if (F_2e38_1b2c[i])
        free_buffer(FP_SEG(F_2e38_1b2c[i]), "event");
    F_2e38_1b2c[i] = 0;
}

/* first free event slot; fatal error when all 300 are used */
int far sub_s029_0e76_3144(void)
{
    int i;

    for (i = 0; i < 300; i++)
        if (F_2e38_1b2c[i] == 0)
            return i;
    sub_root_0000_24bc("EVENT Failure", 100);
    return -1;
}

/* has event i started (start date <= now)? */
int far sub_s029_0e76_3180(int i)
{
    if (F_2e38_1b2c[i] == 0)
        return 0;
    if (F_2e38_1b2c[i]->year0 > D_00e6)
        return 0;
    if (F_2e38_1b2c[i]->year0 < D_00e6)
        return 1;
    if (F_2e38_1b2c[i]->month0 > D_00e4)
        return 0;
    if (F_2e38_1b2c[i]->month0 < D_00e4)
        return 1;
    if (F_2e38_1b2c[i]->day0 > D_00e2)
        return 0;
    if (F_2e38_1b2c[i]->day0 < D_00e2)
        return 1;
    if (F_2e38_1b2c[i]->hour0 > D_00e0)
        return 0;
    return 1;
}

/* has event i ended (end date <= now)? */
int far sub_s029_0e76_31e6(int i)
{
    if (F_2e38_1b2c[i] == 0)
        return 0;
    if (F_2e38_1b2c[i]->year1 > D_00e6)
        return 0;
    if (F_2e38_1b2c[i]->year1 < D_00e6)
        return 1;
    if (F_2e38_1b2c[i]->month1 > D_00e4)
        return 0;
    if (F_2e38_1b2c[i]->month1 < D_00e4)
        return 1;
    if (F_2e38_1b2c[i]->day1 > D_00e2)
        return 0;
    if (F_2e38_1b2c[i]->day1 < D_00e2)
        return 1;
    if (F_2e38_1b2c[i]->hour1 > D_00e0)
        return 0;
    return 1;
}

/* next event slot >= i of kind k (kind 0x1c also accepts kind 8); -1 if none */
int far sub_s029_0e76_324c(int k, int i)
{
    for (; i < 300; i++)
        if (F_2e38_1b2c[i]->kind == k || k == 0x1c && F_2e38_1b2c[i]->kind == 8)
            return D_e3ce = i;
    return -1;
}

/* no event of kind k exists (and D_00f0 is clear) */
#pragma optimize("e", off)
int far sub_s029_0e76_32aa(int k)
{
    int i;

    if (D_00f0 != 0)
        return 0;
    if ((i = sub_s029_0e76_324c(k, 0)) == -1)
        return 0;
    return 1;
}
#pragma optimize("", on)

/* the first event of kind k has started */
int far sub_s029_0e76_32ce(int k)
{
    int i;

    i = sub_s029_0e76_324c(k, 0);
    if (i != -1 && sub_s029_0e76_3180(i))
        return 1;
    return 0;
}

/* a started event of kind 8 with +0x28 == v exists */
int far sub_s029_0e76_32fe(int v)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(8, i)) == -1)
            break;
        if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f28 == v)
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x24 with +0x28 == v -> 1, else 0 */
int far sub_s029_0e76_3350(int v)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x24, i)) == -1)
            break;
        if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f28 == v)
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_33a2(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x24, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_3404(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x24, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_3470(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_34d2(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_353e(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1e == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_35aa(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_360c(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1c == b && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x8 with +0x28 == v -> its +0x1e, else 0 */
int far sub_s029_0e76_3676(int v)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && sub_s029_0e76_3180(i))
            return F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_36d4(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && sub_s029_0e76_3180(i))
            return F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
    return 0;
}

int far sub_s029_0e76_3742(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            return 0;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1c == b && sub_s029_0e76_3180(i))
            return F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
    return 0;
}

/* number of events of kind k */
int far sub_s029_0e76_37b8(int k)
{
    int i;
    int n;

    i = 0;
    n = 0;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            break;
        n++;
    } while (++i < 300);
    return n;
}

/* number of started events of kind k with +0x1c == a */
int far sub_s029_0e76_37e2(int k, int a)
{
    int i;
    int n;

    i = 0;
    n = 0;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            break;
        if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f1c == a)
            n++;
    } while (++i < 300);
    return n;
}

/* number of kind-8 events with +0x28 == v (v == -1: all) */
int far sub_s029_0e76_382e(int v)
{
    int i;
    int n;

    n = 0;
    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v || v == -1)
            n++;
    } while (++i < 300);
    return n;
}

/* started event of kind k with f1c == a -> 1 (0 while D_00f0 is set) */
int far sub_s029_0e76_3878(int k, int a)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f1c == a)
                return 1;
        } while (++i < 300);
    return 0;
}

/* started event of kind k with type0 == a -> 1 (0 while D_00f0 is set) */
int far sub_s029_0e76_38d2(int k, int a)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->type0 == a)
                return 1;
        } while (++i < 300);
    return 0;
}

/* started event of kind k with type0 == a, f1c == b -> 1 (0 while D_00f0 is set) */
int far sub_s029_0e76_392c(int k, int a, int b)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1c == b)
                return 1;
        } while (++i < 300);
    return 0;
}

/* started event of kind k with f1a == a -> 1 (0 while D_00f0 is set) */
int far sub_s029_0e76_3996(int k, int a)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f1a == a)
                return 1;
        } while (++i < 300);
    return 0;
}

/* value (+0x1e) of the first active event of kind k, 0 if there is none */
int far sub_s029_0e76_39f0(int k)
{
    int i;

    if ((i = sub_s029_0e76_324c(k, 0)) == -1)
        return 0;
    return F_2e38_1b2c[i]->f1e;
}

/* started event of kind k with f1c == a -> its +0x1e (0 while D_00f0 is set) */
int far sub_s029_0e76_3a20(int k, int a)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->f1c == a)
                return F_2e38_1b2c[i]->f1e;
        } while (++i < 300);
    return 0;
}

/* started event of kind k with type0 == a -> its +0x1e (0 while D_00f0 is set) */
int far sub_s029_0e76_3a86(int k, int a)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->type0 == a)
                return F_2e38_1b2c[i]->f1e;
        } while (++i < 300);
    return 0;
}

/* started event of kind k with type0 == a and +0x1c == b -> its +0x1e, else 0 (0 while D_00f0) */
int far sub_s029_0e76_3aec(int k, int a, int b)
{
    int i;

    i = 0;
    if (D_00f0 == 0)
        do {
            if ((i = sub_s029_0e76_324c(k, i)) == -1)
                break;
            if (sub_s029_0e76_3180(i) && F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1c == b)
                return F_2e38_1b2c[i]->f1e;
        } while (++i < 300);
    return 0;
}

/* first event of kind k matching type t, f1c a, f1a b, f28 c, f2a d (-1 = any); -1 if none */
int far sub_s029_0e76_3b62(int k, int t, int a, int b, int c, int d)
{
    int i;

    for (i = 0; i < 300; i++) {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            return -1;
        if ((t == -1 || F_2e38_1b2c[i]->type0 == t) && (a == -1 || F_2e38_1b2c[i]->f1c == a) &&
            (b == -1 || F_2e38_1b2c[i]->f1a == b) && (d == -1 || F_2e38_1b2c[i]->f2a == d) &&
            (c == -1 || F_2e38_1b2c[i]->f28 == c))
            return i;
    }
    return -1;
}

int far sub_s029_0e76_3c28(int k, int t, int a, int b, int c)
{
    int i;

    for (i = 0; i < 300; i++) {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            return -1;
        if ((t == -1 || F_2e38_1b2c[i]->type0 == t) && (a == -1 || F_2e38_1b2c[i]->f1a == a) &&
            (c == -1 || F_2e38_1b2c[i]->f2a == c) && (b == -1 || F_2e38_1b2c[i]->f28 == b))
            return F_2e38_1b2c[i]->f1c;
    }
    return -1;
}

/* remove the events of kind k matching type t, f1c a, f1a b, f28 c (-1 = any); returns the
   count only when the scan runs out of events, garbage when it reaches slot 300 */
int far sub_s029_0e76_3cde(int k, int t, int a, int b, int c)
{
    int i;
    int n;

    i = 0;
    n = i;
    do {
        if ((i = sub_s029_0e76_324c(k, i)) == -1)
            return n;
        if ((t == -1 || F_2e38_1b2c[i]->type0 == t) && (a == -1 || F_2e38_1b2c[i]->f1c == a) &&
            (b == -1 || F_2e38_1b2c[i]->f1a == b) && (c == -1 || F_2e38_1b2c[i]->f28 == c)) {
            sub_s029_0e76_30fe(i);
            n++;
        }
    } while (++i < 300);
}

int far sub_s029_0e76_3d92(int v, int w)
{
    int i;
    int n;

    i = 0;
    n = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            return n;
        if ((v == -1 || F_2e38_1b2c[i]->f28 == v) && (w == -1 || F_2e38_1b2c[i]->f1c == w)) {
            sub_s029_0e76_30fe(i);
            n++;
        }
    } while (++i < 300);
}

void far sub_s029_0e76_3e06(int v, int w)
{
    int i;
    int j;
    int found;

    i = 0;
    found = 0;
    do {
        if ((i = sub_s029_0e76_324c(8, i)) == -1)
            break;
        if ((v == -1 || F_2e38_1b2c[i]->f28 == v) && (w == -1 || F_2e38_1b2c[i]->f1c == w)) {
            found = 1;
            j = sub_s029_0e76_2c4e(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->f1e, F_2e38_1b2c[i]->f20,
                                   0x24, F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->type0, 0, D_2238, 0,
                                   F_2e38_1b2c[i]->f28);
            F_2e38_10be[F_2e38_1b2c[i]->f1e]->rep += F_2e38_1b2c[i]->f2a;
            vec_1f31(F_2e38_1b2c[i]->f2c);
            F_2e38_1b2c[j]->f2e = F_2e38_1b2c[i]->f2e;
            F_2e38_1b2c[j]->f2c = F_2e38_1b2c[i]->f1c;
        }
    } while (++i < 300);
    if (found)
        sub_s029_0e76_3d92(v, w);
}

void far sub_s029_0e76_3f30(void)
{
    int i;

    for (i = 0; i < 300; i++)
        if (sub_s029_0e76_31e6(i)) {
            if (F_2e38_1b2c[i]->f24 & 1)
                F_290e_290f[F_2e38_1b2c[i]->f26](i);
            if (F_2e38_1b2c[i]->kind == 8)
                sub_s029_0e76_3d92(F_2e38_1b2c[i]->f28, F_2e38_1b2c[i]->f1c);
            else
                sub_s029_0e76_30fe(i);
        }
}

void far sub_s029_0e76_3fb0(void)
{
}

/* alchemy event expired (event i: type0 = formula * 3 + grade, f1e = chance, f1a = member,
   f20 = number of potions, f2a/f2c): if rnd 100 >= chance the brew fails ("Alchemy
   Failure"; with 1 in 2 chance, outside cities, a follow-up event 0x51 is started whose
   length depends on whether rnd(4 - f2c) != 0); otherwise the member receives item
   0x5f + formula with quality grade * 10 + the formula's base quality (+0x26) and the
   values +0x1e/+0x25 of the item record ("Alchemy Success") */
/* compiled without optimisation (far call to 2c4e, no CSE); local names chosen for the
   6.00A hash-ordered frame */
#pragma optimize("", off)
void far sub_s029_0e76_3fb2(int i)
{
    struct item itd;
    int rn;
    int xj;
    int qb;

    if (F_2e38_1b2c[i]->f1e <= sub_root_0000_0c70(100)) {
        xj = sub_root_0000_0c70(4 - F_2e38_1b2c[i]->f2c) != 0;
        if (sub_root_0000_0c70(2) != 0 && D_907e != -1)
            sub_s029_0e76_2c4e(-2, D_907e, 0x32, 0x51, F_2e38_1b2c[i]->f2a, xj, 0,
                               xj * 0x870 + 0x2d0, 0, 0);
        sub_root_0000_24bc("Alchemy Failure", 100);
        vec_1f63(F_2e38_1b2c[i]->f1a, 7, 0, 3, 0);
    } else {
        qb = F_2e38_1b2c[i]->type0 / 3;
        rn = F_2e38_1b2c[i]->type0 % 3;
        itd.code = qb + 0x5f;
        itd.f2 = F_2e38_0000.f_0890[qb + 0x5f][0x1e];
        itd.quality = rn * 10 + F_2e38_0000.f_0890[qb + 0x5f][0x26];
        itd.f5 = F_2e38_0000.f_0890[qb + 0x5f][0x25];
        itd.qty = F_2e38_1b2c[i]->f20;
        vec_1f6d(F_2e38_1b2c[i]->f1a, itd);
        sub_root_0000_24bc("Alchemy Success", 100);
        vec_1f63(F_2e38_1b2c[i]->f1a, 7, 1, 3, 0);
    }
}
#pragma optimize("", on)

/* expiry of a type 0x1c event (i; f1c = city, f1a = owner): notes the old value of
   3742(2, 0x17, city), sets three 0x1c sub-events (4/5/3) to 2, a random value 0..99
   for 0x23, with 1 in 2 chance swaps a random bit's vec_1f59/vec_1f4f value; every seventh
   day (city +0x21 offset) picks the weaker of vec_1f3b/vec_1f45 (0x15/0x16) and may start
   a 0x1c event with that kind for 480 hours; always starts the 24-hour 0x1c/0xc event */
void far sub_s029_0e76_4180(int i)
{
    int old;
    int bit;

    *(volatile int *)&old = sub_s029_0e76_3742(2, 0x17, F_2e38_1b2c[i]->f1c);
    bit = 1 << sub_root_0000_0c70(16);
    sub_s029_0e76_2e74(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->f1c, 0x1c, 5, 0, 2, 2);
    sub_s029_0e76_2e74(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->f1c, 0x1c, 4, 0, 2, 2);
    sub_s029_0e76_2e74(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->f1c, 0x1c, 3, 0, 2, 2);
    sub_s029_0e76_2e74(-2, F_2e38_1b2c[i]->f1c, 0x1c, 0x23, sub_root_0000_0c70(100), 2, 0);
    if (sub_root_0000_0c70(2) && old != bit)
        vec_1f4f(bit, -vec_1f59(bit));
    if ((sub_s029_0e76_1a8e(F_2e38_1b2c[i]->f1c, 0x21) + D_00e2) % 7 == 0) {
        if (sub_s029_0e76_3742(2, 0, F_2e38_1b2c[i]->f1c) == -1) {
            old = vec_1f3b();
            bit = 0x15;
        } else if (sub_s029_0e76_3742(2, 0, F_2e38_1b2c[i]->f1c) == 1) {
            old = vec_1f45();
            bit = 0x16;
        } else if (vec_1f3b() > (bit = vec_1f45())) {
            old = vec_1f3b();
            bit = 0x15;
        } else {
            old = bit;
            bit = 0x16;
        }
        if (!sub_s029_0e76_360c(2, 7, F_2e38_1b2c[i]->f1c) ||
            (!sub_s029_0e76_360c(2, 6, F_2e38_1b2c[i]->f1c) && old > 0x96) || old > 0x10b)
            sub_s029_0e76_2c4e(-2, F_2e38_1b2c[i]->f1c, 0, 0x1c, bit, 0x63, 0, 0x1e0, 0, 2);
    }
    sub_s029_0e76_2c4e(-2, F_2e38_1b2c[i]->f1c, 0, 0x1c, 0xc, 0x63, 2, 0x18, 1, 2);
}

void far sub_s029_0e76_4378(int j)
{
    int i;
    int k;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f1c == F_2e38_1b2c[j]->f1c) {
            sub_s029_0e76_2c4e(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->f1e, F_2e38_1b2c[i]->f20, 0x24,
                               F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->type0, 0, D_270f, 0,
                               F_2e38_1b2c[i]->f28);
            sub_s029_0e76_19d0(F_2e38_1b2c[i]->f1e, F_2e38_1b2c[i]->f2a, F_2e38_1b2c[i]->f2a);
            F_2e38_1b2c[k]->f2e = F_2e38_1b2c[i]->f2e;
        }
    } while (++i < 300);
}

void far sub_s029_0e76_4436(void)
{
    sub_s029_0e76_2930(0x3b, 0x18);
}

/* event i expires: message 0x47 to the party if its member (type0) is present */
void far sub_s029_0e76_4442(int i)
{
    if (D_9c69[F_2e38_1b2c[i]->type0][0])
        sub_s029_0e76_2c4e(-2, D_907e, 0, 0x47, F_2e38_1b2c[i]->type0, F_2e38_1b2c[i]->type0, 0, D_270f, 0, 0);
}

/* undo an event's effect on member +0x1a (kinds 0x48 / 0x49: subtract +0x1e again) */
void far sub_s029_0e76_448a(int i)
{
    D_26bc = 1;
    if (D_9c69[F_2e38_1b2c[i]->f1a][0]) {
        if (D_01bd == 0)
            D_26bc = 0;
        switch (F_2e38_1b2c[i]->kind) {
        case 0x48:
            sub_s029_0e76_0a72(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->type0, -F_2e38_1b2c[i]->f1e);
            break;
        case 0x49:
            sub_s029_0e76_02ea(F_2e38_1b2c[i]->f1a, F_2e38_1b2c[i]->type0, -F_2e38_1b2c[i]->f1e);
            break;
        }
        D_26bc = 1;
    }
}

void far sub_s029_0e76_450a(int p_6)
{
    F_2e38_10be[F_2e38_1b2c[p_6]->f2a]->fc = 0x13d;
}

/* event i (+0x2a = location) ends: the location's +0xc becomes 0x13e, a 48-hour 0x1c/5
   event remembers the location, and a 0x1c/4 event lasts until day vec_1f81() (this year
   or next); its +0x2a gets vec_1f77(rnd 300, rnd 900, 0x15) */
void far sub_s029_0e76_4534(int i)
{
    struct date { int hour, day, month, year; } d;
    int loc;
    int day;
    int y;

    loc = F_2e38_1b2c[i]->f2a;
    F_2e38_10be[loc]->fc = 0x13e;
    F_2e38_1b2c[sub_s029_0e76_2c4e(-2, 0, 0x63, 0x1c, 5, 0, 7, 0x30, 1, 1)]->f2a = loc;
    day = vec_1f81();
    if (sub_s029_1367_09cc() < day)
        y = D_00e6;
    else
        y = D_00e6 + 1;
    sub_s029_1367_08ae(&d, y, 0, day, 0);
    i = sub_s029_0e76_2c4e(-2, 0, 0x63, 0x1c, 4, day, 8,
                           sub_s029_1367_0b1c((struct date far *)&d) * 24, 1, 1);
    F_2e38_1b2c[i]->f2a = vec_1f77(sub_root_0000_0c70(300), sub_root_0000_0c70(900), 0x15);
}

void far sub_s029_0e76_4626(void)
{
    int t;

    t = D_d843;
    if (t == 1 || t == 2)
        sub_s029_0e76_2c4e(-2, 0, 0x63, 0x4b, 0, 0, 9, 6, 1, 0);
}

/* add a new party member in the first free slot (-1 if the party is full): status 1, names,
   the default attribute maxima 37/34/31/29/32/34/99 (current = maximum), default skills,
   age 25, random portrait letter 'A'..'O', cleared flag bits and formulas, three standard
   items (vec_1f8b); appends the record to charactr.tmp (slot table of 5 file indices, the
   5 portrait names, then 0x22a bytes per character), loads pics\shield?.pic and starts the
   0x43 event for the new member */
int far sub_s029_0e76_4650(int unused, char *name, char *shortname, char *pic, int hours,
                           int y)
{
    char buf[30];
    int idx[5];
    int cnt;
    unsigned n;
    int h;
    int slot;
    int i;
    int j;
    FILE *f;

    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] == 0)
            break;
    if (i >= 5)
        return -1;
    slot = i;
    D_9c69[slot][0] = 1;
    strcpy(D_9c7a[slot], name);
    strcpy(D_9c93[slot], shortname);
    D_9c69[slot][6] = 0;
    D_9c69[slot][5] = 0;
    D_9cb9[slot][0] = 37;
    D_9cb9[slot][1] = 34;
    D_9cb9[slot][2] = 31;
    D_9cb9[slot][3] = 29;
    D_9cb9[slot][4] = 32;
    D_9cb9[slot][5] = 34;
    D_9cb9[slot][6] = 99;
    D_9cc0[slot][0] = 59;
    D_9cc0[slot][1] = 21;
    D_9cc0[slot][2] = 0;
    D_9cc0[slot][3] = 57;
    D_9cc0[slot][4] = 23;
    D_9cc0[slot][5] = 0;
    D_9cc0[slot][6] = 46;
    D_9cc0[slot][7] = 11;
    D_9cc0[slot][8] = 15;
    D_9cc0[slot][9] = 36;
    D_9cc0[slot][10] = 51;
    D_9cc0[slot][11] = 0;
    D_9cc0[slot][12] = 0;
    D_9cc0[slot][13] = 16;
    D_9cc0[slot][14] = 59;
    D_9cc0[slot][15] = 52;
    D_9cc0[slot][16] = 48;
    D_9cc0[slot][17] = 40;
    D_9cc0[slot][18] = 31;
    D_9c69[slot][0x3b] = 25;
    D_9c69[slot][0x3c] = 25;
    D_9c69[slot][0x39] = 5;
    D_9c69[slot][0x3a] = 4;
    /* the original inlines both clears (rep stosw through the CONST slot): 6.00 had a far
       memset intrinsic that 6.00A lacks */
    _fmemset(F_2e38_082c[i], 0, 0x14);
    _fmemset(&F_2e38_075f[slot][0x5f], 0, 0x16);
    D_9c67[slot][0] = 25;
    D_9c69[slot][3] = 0;
    D_9c69[slot][1] = sub_root_0000_0c70(15) + 'A';
    D_9c69[slot][7] = 0;
    D_9c69[slot][8] = slot;
    D_9c69[slot][4] = -1;
    D_9c69[slot][0x36] = -1;
    D_9c69[slot][0x35] = 10;
    D_9c69[slot][0x48] = -1;
    D_9c69[slot][0x47] = 25;
    for (j = 0; j < 7; j++)
        D_9cb2[slot][j] = D_9cb9[slot][j];
    D_9c69[slot][0x3d] = -1;
    D_9c69[slot][0x0e] = -1;
    D_9c69[slot][0x37] = -1;
    D_9c69[slot][0x38] = -1;
    D_9c69[slot][0x6a] = 0;
    vec_1f8b(slot, 0x2c, 10);
    vec_1f8b(slot, 0x33, 15);
    vec_1f8b(slot, 3, 15);
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0] != 0)
            D_a67e = i + 1;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    f = fdopen(h, "r+b");
    _dos_read(h, &cnt, 2, &n);
    _dos_read(h, idx, 10, &n);
    _dos_read(h, D_a668, 0x14, &n);
    strcpy(D_a668[slot], pic);
    idx[slot] = cnt++;
    fseek(f, 0L, 0);
    _dos_write(h, &cnt, 2, &n);
    _dos_write(h, idx, 10, &n);
    _dos_write(h, D_a668, 0x14, &n);
    fseek(f, (long)(unsigned)(cnt * 0x22a - 0x20a), 0);
    _dos_write(h, D_9c55[slot], 0x80, &n);
    _dos_write(h, F_2e38_082c[slot], 0x14, &n);
    _dos_write(h, &F_2e38_075f[slot][0x5f], 0x16, &n);
    _dos_write(h, F_2e38_003e[slot], 0x180, &n);
    fclose(f);
    sub_root_047c_0545();
    strcpy(buf, "pics\\");
    strcat(buf, "shield");
    buf[11] = D_9c69[slot][1];
    buf[12] = 0;
    strcat(buf, ".pic");
    D_a2d7[slot] = sub_root_0617_045c(buf);
    sub_root_047c_04f8();
    sub_s029_0e76_2c4e(-2, D_907e, 0, 0x43, slot, y, 5, hours, 1, 0);
    D_a67c = sub_s029_0e76_37b8(0x43);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
    sub_root_0000_1716(0x100);
}

/* close every 0x43 event whose field +0x1e is p_6: log it as a 0x47 event for the party
   (-2, lasting 9999) and remove it through 30fe.  No local copy of p_6 (a `v_di = p_6`
   copy costs a frame slot); the dead `k = 0;` is needed, without it 6.00A lays the loop
   out 6 bytes longer. */
void far sub_s029_0e76_4aa4(int p_6)
{
    int v_si;
    int k;
    k = 0;
    v_si = 0;
    do {
        v_si = sub_s029_0e76_324c(0x43, v_si);
        if (v_si == -1) {
            break;
        }
        if (F_2e38_1b2c[v_si]->f1e == p_6) {
            sub_s029_0e76_2c4e(-2, D_907e, 0, 0x47, F_2e38_1b2c[v_si]->type0, F_2e38_1b2c[v_si]->type0, 0, D_270f, 0, 0);
            sub_s029_0e76_30fe(v_si);
        }
        v_si++;
    } while (v_si < 0x12c);
}

/* lower skill k of member who (-2: every present member) by up to d points of headroom:
   d is capped at 99 - skill, applied through 02ea and recorded as a 0x49 event lasting
   6 + rnd 3 + h - 5 hours.  Original bug: the -2 loop queries and changes member -2 (who)
   instead of each present member, and the capped d carries over */
void far sub_s029_0e76_4b08(int who, int k, int d, int h)
{
    int i;
    int t;

    D_26bc = 1;
    if (who == -1)
        who = D_907b;
    if (!D_01bd)
        D_26bc = 0;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0]) {
                if ((t = 99 - sub_s029_0e76_01a0(who, k)) < d)
                    d = t;
                sub_s029_0e76_02ea(who, k, d);
                sub_s029_0e76_2c4e(who, D_907e, 0x63, 0x49, k, d, 6, sub_root_0000_0c70(3) + h - 5, 1, 0);
            }
    } else {
        if ((t = 99 - sub_s029_0e76_01a0(who, k)) >= d)
            t = d;
        sub_s029_0e76_02ea(who, k, t);
        sub_s029_0e76_2c4e(who, D_907e, 0x63, 0x49, k, t, 6, sub_root_0000_0c70(3) + h - 5, 1, 0);
    }
    D_26bc = 1;
}

/* raise attribute k of member who (-2 = every present member) by d, but only up to the gap
   by which attribute k (05ee) already exceeds its maximum (0b64), so the total boost stays
   within d; records a 0x48 event (6 + rnd 3 + h - 5 hours)
   so the change can expire.  Original bugs: in the -2 loop a member whose gap is smaller
   than d ends the whole function, and the reduced d carries over to the next member. */
void far sub_s029_0e76_4c04(int who, int k, int d, int h)
{
    int n;
    int i;
    int g;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0) {
                g = sub_s029_0e76_05ee(i, k) - sub_s029_0e76_0b64(i, k);
                if (g > 0 && d < g)
                    return;
                if (g > 0)
                    d -= g;
                sub_s029_0e76_2c4e(i, D_907e, 0x63, 0x48, k, d, 6,
                                   sub_root_0000_0c70(3) + h - 5, 1, 0);
                sub_s029_0e76_0a72(i, k, d);
            }
    } else {
        n = d;
        g = sub_s029_0e76_05ee(who, k) - sub_s029_0e76_0b64(who, k);
        if (g > 0 && n < g)
            return;
        if (g > 0)
            n -= g;
        sub_s029_0e76_2c4e(who, D_907e, 0x63, 0x48, k, n, 6, sub_root_0000_0c70(3) + h - 5, 1, 0);
        sub_s029_0e76_0a72(who, k, n);
    }
}

/* message 0x4a for a member (-2: once per present member, still with who == -2) */
void far sub_s029_0e76_4d24(int who, int a, int b, int c)
{
    int i;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] != 0)
                sub_s029_0e76_2c4e(who, D_907e, 0x63, 0x4a, a, b, 6, sub_root_0000_0c70(3) + c - 5, 1, 0);
        return;
    }
    sub_s029_0e76_2c4e(who, D_907e, 0x63, 0x4a, a, b, 6, sub_root_0000_0c70(3) + c - 5, 1, 0);
}

int far sub_s029_0e76_4db6(int who, int a)
{
    int i;
    int n;

    i = 0;
    n = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x49, i)) == -1)
            return n;
        if (F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1a == who)
            n += F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
}

/* largest +0x1e over kind-0x48 events (member who, type0 a) */
int far sub_s029_0e76_4e0c(int who, int a)
{
    int i;
    int n;

    i = 0;
    n = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x48, i)) == -1)
            return n;
        if (F_2e38_1b2c[i]->type0 == a && F_2e38_1b2c[i]->f1a == who && F_2e38_1b2c[i]->f1e > n)
            n = F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
}

void far sub_s029_0e76_4e68(int city)
{
    int h;
    unsigned n;
    FILE *f;

    sub_root_06a1_2c9e("darkland.cty", 0, &h);
    fseek(f = fdopen(h, "r"), (long)city * 0x26e + 1, 0);
    _dos_read(h, D_9785, 0x26e, &n);
    fclose(f);
    f = fopen("darkland.dsc", "r");
    fgetc(f);
    fseek(f, (long)(city * 0x50 + 1), 0);
    fread(D_978b, 0x50, 1, f);
    fclose(f);
}
