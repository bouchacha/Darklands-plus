/* place: s029 0e76:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: hand port in progress (e3), started from tools/dec16.py output. 84/143 match;
   about 25 functions are still empty stubs (02ea, 0792, 0988, 0a72, 0c76, 0dd8, 0f54, 1a8e,
   1dfe, 1fb8, 2278, 255a, 281c, 2a32, 2d5c, 2e74, 3fb2, 4180, 4534, 4650,
   4c04 ...). Several matches depend on declaration positions (symbol-table order) and flip when
   lines are added above them: re-check with build/agent/e3/s029_0e76_best.c as reference.
   Drafts that do not match yet: build/agent/e3/b22.c (3c28), b23.c (0dd8).
   See build/agent/e3/NOTES.md. */
/* Strategic core (s029 0e76): party member accessors (member records D_9c69[5][0x80],
   index -1 = the active member D_907b), ... */

struct res { int a; int b; };
struct F_2e38_0000_t { char _p0000[0x890]; char far * f_0890[1]; };
extern struct F_2e38_0000_t far F_2e38_0000;
struct F_2e38_0000_1_t { char _p0000[0x82c]; char f_082c[1]; };
extern struct F_2e38_0000_1_t far F_2e38_0000_1;
struct loc { int type; int f2; int x; int y; int f8; int fa; int fc; int fe; int f10; int rep; };
extern struct loc far * far F_2e38_10be[];   /* locations (cities ...) */
struct F_2e38_0000_3_t { char _p0000[0x183e]; char far * f_183e[1]; };
extern struct F_2e38_0000_3_t far F_2e38_0000_3;
struct F_2e38_0000_4_t { char _p0000[0x15ba]; char far * f_15ba[1]; };
extern struct F_2e38_0000_4_t far F_2e38_0000_4;
struct F_290e_0000_1_t { char _p0000[0x1fa7]; char far * f_1fa7[1]; };
extern struct F_290e_0000_1_t far F_290e_0000_1;
struct F_290e_0000_2_t { char _p0000[0x1fe7]; char far * f_1fe7[1]; };
extern struct F_290e_0000_2_t far F_290e_0000_2;
struct F_290e_0000_3_t { char _p0000[0x2107]; char far * f_2107[1]; };
extern struct F_290e_0000_3_t far F_290e_0000_3;
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
struct item { int code; int f2; unsigned char qty; char f5; };
extern struct item far F_2e38_003e[][64];   /* inventories, 64 items per member */
extern char far F_2e38_075f[][0x16];
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
void far sub_s029_0e76_02ea();
int far sub_s029_0e76_05ee(int who, int k);
int far sub_s029_0e76_060e(void);
int far sub_s029_0e76_0656(void);
int far sub_s029_0e76_0690(void);
int far sub_s029_0e76_06de(int who);
int far sub_s029_0e76_072e(void);
void far sub_s029_0e76_0792();
int far sub_s029_0e76_088e(void);
void far sub_s029_0e76_08f2(int who, int k, int pct);
void far sub_s029_0e76_0988();
void far sub_s029_0e76_0a72();
int far sub_s029_0e76_0b64(int who, int k);
void far sub_s029_0e76_0b84(int who, int k, int v);
void far sub_s029_0e76_0bb6(int who, int k, int d);
int far sub_s029_0e76_0c38(int who, int k);
int far sub_s029_0e76_0c58(int who);
void far sub_s029_0e76_0c76();
int far sub_s029_0e76_0dd8();
int far sub_s029_0e76_0f54();
int far sub_s029_0e76_1128(int code);
int far sub_s029_0e76_118e(int who, int code);
void far sub_s029_0e76_11fa();
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
void far sub_s029_0e76_1600();
int far sub_s029_0e76_164a(int k);
int far sub_s029_0e76_1696(int a, int b);
int far sub_s029_0e76_16fe(int k);
int far sub_s029_0e76_174a(int k);
int far sub_s029_0e76_179c(int a, int b);
void far sub_s029_0e76_1800();
int far sub_s029_0e76_184a(int a, int b);
int far sub_s029_0e76_18a8(int v, int lo, int hi);
int far sub_s029_0e76_18f2(int who);
int far sub_s029_0e76_1972(void);
int far sub_s029_0e76_199c(int i);
void far sub_s029_0e76_19b4(int i, int v);
int far sub_s029_0e76_19d0(int i, int a, int b);
int far sub_s029_0e76_1a7e();
void far sub_s029_0e76_1a8e();
void far sub_s029_0e76_1dfe();
void far sub_s029_0e76_1fb8();
int far sub_s029_0e76_20c6(int who, int s);
void far sub_s029_0e76_2116(int who, int s);
int far sub_s029_0e76_2164(int who, int s, int v);
int far sub_s029_0e76_2180(int a, int b);
void far sub_s029_0e76_21aa(int i, char far *s, int x, int y);
void far sub_s029_0e76_2204(int a, int b);
void far sub_s029_0e76_222a();
void far sub_s029_0e76_2246();
void far sub_s029_0e76_225c();
void far sub_s029_0e76_2278();
int far sub_s029_0e76_23e2(void);
int far sub_s029_0e76_2434(void);
int far sub_s029_0e76_2474(void);
int far sub_s029_0e76_251a(void);
void far sub_s029_0e76_255a();
void far sub_s029_0e76_281c();
int far sub_s029_0e76_2930(int k, int hours);
void far sub_s029_0e76_2a32();
void far sub_s029_0e76_2b36(int k, int hours);
int far sub_s029_0e76_2bce(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                           int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15,
                           int a16);
int far sub_s029_0e76_2c4e(int a, int b, int c, int d, int e, int f, int g, int hours, int x, int y);
void far sub_s029_0e76_2d5c();
void far sub_s029_0e76_2e74();
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
void far sub_s029_0e76_39f0();
int far sub_s029_0e76_3a20(int k, int a);
int far sub_s029_0e76_3a86(int k, int a);
int far sub_s029_0e76_3aec(int k, int a, int b);
int far sub_s029_0e76_3b62();
void far sub_s029_0e76_3c28();
int far sub_s029_0e76_3cde();
int far sub_s029_0e76_3d92(int v, int w);
void far sub_s029_0e76_3e06(int v, int w);
void far sub_s029_0e76_3f30(void);
void far sub_s029_0e76_3fb0();
void far sub_s029_0e76_3fb2();
void far sub_s029_0e76_4180();
void far sub_s029_0e76_4378(int j);
void far sub_s029_0e76_4436();
void far sub_s029_0e76_4442(int i);
void far sub_s029_0e76_448a(int i);
void far sub_s029_0e76_450a();
void far sub_s029_0e76_4534();
void far sub_s029_0e76_4626(void);
void far sub_s029_0e76_4650();
void far sub_s029_0e76_4aa4();
void far sub_s029_0e76_4b08();
void far sub_s029_0e76_4c04();
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

/* band of +0x35 against (+0x49 + +0x4a): 0 up to half, 1 up to full, 2 up to 1.5x, else 3 */
int far sub_s029_0e76_013a(int who)
{
    unsigned max;
    unsigned cur;

    if (who == -1)
        who = D_907b;
    max = D_9cb3[who][0] + D_9cb2[who][0];
    cur = D_9c9e[who][0];
    if (max / 2 >= cur)
        return 0;
    if (cur <= max)
        return 1;
    if (cur <= max * 3 / 2)
        return 2;
    return 3;
}

/* member byte +0x57 + k */
int far sub_s029_0e76_01a0(int who, int k)
{
    if (who == -1)
        who = D_907b;
    return D_9cc0[who][k];
}

int far sub_s029_0e76_01c0(int who)
{
    char t[2];
    int i;

    if (who == -1)
        who = D_907b;
    *(volatile unsigned char *)&D_99e6 = 0;
    for (i = 0; i < 7; i++) {
        *(volatile char *)t = D_9cc0[who][i];
        if (t[0] > D_99e6)
            D_99e6 = t[0];
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

/* sub_s029_0e76_02ea: not decompiled: unhandled instruction repne scasb al, byte ptr es:[di] */
void far sub_s029_0e76_02ea(void) { }

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

/* sub_s029_0e76_0792: not decompiled: es:[reg] unknown es=farslot(32282, 11832) ptr=None */
void far sub_s029_0e76_0792(void) { }

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

/* sub_s029_0e76_0988: not decompiled: memory via unknown si */
void far sub_s029_0e76_0988(void) { }

/* sub_s029_0e76_0a72: not decompiled: memory via unknown si */
void far sub_s029_0e76_0a72(void) { }

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

/* sub_s029_0e76_0c76: not decompiled: memory via unknown si */
void far sub_s029_0e76_0c76(void) { }

/* sub_s029_0e76_0dd8: not decompiled: memory via unknown si */
int far sub_s029_0e76_0dd8(int who, long mask) { }

/* sub_s029_0e76_0f54: not decompiled: memory via unknown si */
int far sub_s029_0e76_0f54(int who, long mask) { }

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
    char cnt[2];

    j = 0;
    if ((cnt[0] = D_9cd3[who][0]) > 0)
        do {
            if ((F_2e38_003e[who][j].code & 0xfff) == code)
                return 1;
        } while (++j < cnt[0]);
    if (F_2e38_075f[who][code])
        return 2;
    return 0;
}

/* sub_s029_0e76_11fa: not decompiled: memory via unknown bx */
void far sub_s029_0e76_11fa(void) { }

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

/* sub_s029_0e76_1600: not decompiled: memory via unknown di */
void far sub_s029_0e76_1600(void) { }

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

/* sub_s029_0e76_1800: not decompiled: memory via unknown di */
void far sub_s029_0e76_1800(void) { }

/* highest attribute a + skill b over present members */
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
    int cur;

    m = (D_9cb2[who][0] + D_9cb3[who][0]) * 5 / 4;
    *(volatile int *)&cur = D_9c9e[who][0];
    if (m >= cur)
        return 0;
    if (cur <= m && cur >= m >> 1)
        return 9;
    if (cur <= m * 3 / 2 && cur >= m)
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

/* sub_s029_0e76_1a8e: not decompiled: read of undefined bx */
void far sub_s029_0e76_1a8e(void) { }

/* sub_s029_0e76_1dfe: not decompiled: es:[reg] unknown es=farslot(32288, 11832) ptr=None */
void far sub_s029_0e76_1dfe(void) { }

/* sub_s029_0e76_1fb8: not decompiled: terminator table */
void far sub_s029_0e76_1fb8(void) { }

/* invoke saint s for a member if his divine favour (+0x4f) covers the cost (mode 2) */
int far sub_s029_0e76_20c6(int who, int s)
{
    int cost;

    cost = F_290e_2937[s](who, 0, 2);
    if (D_9cb8[who][0] < cost)
        return 0;
    return F_290e_2937[s](who, cost, 7);
}

/* invoke saint s with the cost (mode 3) limited by the divine favour */
void far sub_s029_0e76_2116(int who, int s)
{
    int cost;
    int dfv;

    cost = F_290e_2937[s](who, 0, 3);
    if ((dfv = D_9cb8[who][0]) < cost)
        cost = dfv;
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

void far sub_s029_0e76_225c(int p_6)
{
    vec_1ed7(p_6);
    vec_1ea5(D_e3d4, (char far *)D_e3e3);
}

/* sub_s029_0e76_2278: not decompiled: memory via unknown si */
void far sub_s029_0e76_2278(void) { }

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

/* sub_s029_0e76_255a: not decompiled: memory via unknown bx */
void far sub_s029_0e76_255a(void) { }

/* sub_s029_0e76_281c: not decompiled: memory via unknown di */
void far sub_s029_0e76_281c(void) { }

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

/* sub_s029_0e76_2a32: not decompiled: cannot print farslot */
void far sub_s029_0e76_2a32(void) { }

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

/* sub_s029_0e76_2d5c: not decompiled: far struct array with element size 4 / access 2 */
void far sub_s029_0e76_2d5c(void) { }

/* sub_s029_0e76_2e74: not decompiled: es:[reg] unknown es=farslot(32304, 11832) ptr=None */
void far sub_s029_0e76_2e74(void) { }

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
    struct event far *e;

    if ((e = F_2e38_1b2c[i]) == 0)
        return 0;
    if (e->year0 > D_00e6)
        return 0;
    if (e->year0 < D_00e6)
        return 1;
    if (e->month0 > D_00e4)
        return 0;
    if (e->month0 < D_00e4)
        return 1;
    if (e->day0 > D_00e2)
        return 0;
    if (e->day0 < D_00e2)
        return 1;
    if (e->hour0 > D_00e0)
        return 0;
    return 1;
}

/* has event i ended (end date <= now)? */
int far sub_s029_0e76_31e6(int i)
{
    struct event far *e;

    if ((e = F_2e38_1b2c[i]) == 0)
        return 0;
    if (e->year1 > D_00e6)
        return 0;
    if (e->year1 < D_00e6)
        return 1;
    if (e->month1 > D_00e4)
        return 0;
    if (e->month1 < D_00e4)
        return 1;
    if (e->day1 > D_00e2)
        return 0;
    if (e->day1 < D_00e2)
        return 1;
    if (e->hour1 > D_00e0)
        return 0;
    return 1;
}

/* find the next event of kind k from index i (kind 0x1c also matches kind 8) -> D_e3ce */
int far sub_s029_0e76_324c(int k, int i)
{
    int t;

    for (; i < 300; i++) {
        t = F_2e38_1b2c[i]->kind;
        if (t == k || k == 0x1c && t == 8)
            return D_e3ce = i;
    }
    return -1;
}

/* no event of kind k exists (and D_00f0 is clear) */
int far sub_s029_0e76_32aa(int k)
{
    if (D_00f0 != 0)
        return 0;
    if (sub_s029_0e76_324c(k, 0) == -1)
        return 0;
    return 1;
}

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

/* started event of kind 0x24 with +0x28 == v, f1c == a -> 1, else 0 */
int far sub_s029_0e76_33a2(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x24, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x24 with +0x28 == v, f1c == b, f1a == a -> 1, else 0 */
int far sub_s029_0e76_3404(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x24, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x8 with +0x28 == v, f1c == a -> 1, else 0 */
int far sub_s029_0e76_3470(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x8 with +0x28 == v, f1c == b, f1a == a -> 1, else 0 */
int far sub_s029_0e76_34d2(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1c == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x8 with +0x28 == v, f1e == b, f1a == a -> 1, else 0 */
int far sub_s029_0e76_353e(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x8, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->f1e == b && F_2e38_1b2c[i]->f1a == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x1c with +0x28 == v, type0 == a -> 1, else 0 */
int far sub_s029_0e76_35aa(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && sub_s029_0e76_3180(i))
            return 1;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x1c with +0x28 == v, type0 == a, f1c == b -> 1, else 0 */
int far sub_s029_0e76_360c(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            break;
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

/* started event of kind 0x1c with +0x28 == v, type0 == a -> its +0x1e, else 0 */
int far sub_s029_0e76_36d4(int v, int a)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            break;
        if (F_2e38_1b2c[i]->f28 == v && F_2e38_1b2c[i]->type0 == a && sub_s029_0e76_3180(i))
            return F_2e38_1b2c[i]->f1e;
    } while (++i < 300);
    return 0;
}

/* started event of kind 0x1c with +0x28 == v, type0 == a, f1c == b -> its +0x1e, else 0 */
int far sub_s029_0e76_3742(int v, int a, int b)
{
    int i;

    i = 0;
    do {
        if ((i = sub_s029_0e76_324c(0x1c, i)) == -1)
            break;
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

/* sub_s029_0e76_39f0: not decompiled: es:[reg] unknown es=farslot(32304, 11832) ptr=None */
void far sub_s029_0e76_39f0(void) { }

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

int far sub_s029_0e76_3b62(int p_6, int p_8, int p_a, int p_c, int p_e, int p_10)
{
    int v_di;
    int v_si;
    v_si = 0;
    v_di = p_8;
    do {
        v_si = sub_s029_0e76_324c(p_6, v_si);
        if (v_si == -1) {
            goto L_3c20;
        }
        if ((v_di == -1 || F_2e38_1b2c[v_si]->type0 == v_di) && (p_a == -1 || F_2e38_1b2c[v_si]->f1c == p_a) && (p_c == -1 || F_2e38_1b2c[v_si]->f1a == p_c) && (p_10 == -1 || F_2e38_1b2c[v_si]->f2a == p_10) && (p_e == -1 || F_2e38_1b2c[v_si]->f28 == p_e)) {
            goto L_3c1a;
        }
        v_si++;
    } while (v_si < 0x12c);
    goto L_3c20;
L_3c1a:
    return v_si;
L_3c20:
    return -1;
}

/* sub_s029_0e76_3c28: not decompiled: es:[reg] unknown es=farslot(32304, 11832) ptr=None */
void far sub_s029_0e76_3c28(void) { }

int far sub_s029_0e76_3cde(int p_6, int p_8, int p_a, int p_c, int p_e)
{
    int l_2;
    int v_di;
    int v_si;
    v_si = 0;
    l_2 = v_si;
    v_di = p_8;
    do {
        v_si = sub_s029_0e76_324c(p_6, v_si);
        if (v_si == -1) {
            goto L_3d8a;
        }
        if ((v_di == -1 || F_2e38_1b2c[v_si]->type0 == v_di) && (p_a == -1 || F_2e38_1b2c[v_si]->f1c == p_a) && (p_c == -1 || F_2e38_1b2c[v_si]->f1a == p_c) && (p_e == -1 || F_2e38_1b2c[v_si]->f28 == p_e)) {
            sub_s029_0e76_30fe(v_si);
            l_2++;
        }
        v_si++;
    } while (v_si < 0x12c);
    return;
L_3d8a:
    return l_2;
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

/* sub_s029_0e76_3fb2: not decompiled: 16-bit read of partial register ax */
void far sub_s029_0e76_3fb2(void) { }

/* sub_s029_0e76_4180: not decompiled: cannot print farslot */
void far sub_s029_0e76_4180(void) { }

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

/* event i: message 0x47 about member type0 when that member is present */
void far sub_s029_0e76_4442(int i)
{
    int w;

    w = F_2e38_1b2c[i]->type0;
    if (D_9c69[w][0])
        sub_s029_0e76_2c4e(-2, D_907e, 0, 0x47, w, w, 0, D_270f, 0, 0);
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

/* sub_s029_0e76_4534: not decompiled: es:[reg] unknown es=farslot(32304, 11832) ptr=None */
void far sub_s029_0e76_4534(void) { }

void far sub_s029_0e76_4626(void)
{
    int t;

    t = D_d843;
    if (t == 1 || t == 2)
        sub_s029_0e76_2c4e(-2, 0, 0x63, 0x4b, 0, 0, 9, 6, 1, 0);
}

/* sub_s029_0e76_4650: not decompiled: memory via unknown bx */
void far sub_s029_0e76_4650(void) { }

void far sub_s029_0e76_4aa4(int p_6)
{
    int v_di;
    int v_si;
    v_si = 0;
    v_di = p_6;
    do {
        v_si = sub_s029_0e76_324c(0x43, v_si);
        if (v_si == -1) {
            break;
        }
        if (F_2e38_1b2c[v_si]->f1e == v_di) {
            sub_s029_0e76_2c4e(-2, D_907e, 0, 0x47, F_2e38_1b2c[v_si]->type0, F_2e38_1b2c[v_si]->type0, 0, D_270f, 0, 0);
            sub_s029_0e76_30fe(v_si);
        }
        v_si++;
    } while (v_si < 0x12c);
}

void far sub_s029_0e76_4b08(int p_6, int p_8, int p_a, int p_c)
{
    int l_4;
    int v_di;
    int v_si;
    v_di = p_6;
    D_26bc = 1;
    if (v_di == -1) {
        v_di = D_907b;
    }
    if (!D_01bd) {
        D_26bc = 0;
    }
    if (v_di == -2) {
        l_4 = -0x6397;
        p_6 = v_di;
        do {
            if (*(char *)l_4) {
                if (-(sub_s029_0e76_01a0(v_di, p_8) - 0x63) < p_a) {
                    p_a = -(sub_s029_0e76_01a0(v_di, p_8) - 0x63);
                }
                sub_s029_0e76_02ea(v_di, p_8);
                sub_s029_0e76_2c4e(v_di, D_907e, 0x63, 0x49, p_8, 6, sub_root_0000_0c70(3) + p_c - 5, 1, 0);
            }
            l_4 += 0x80;
        } while ((unsigned)l_4 < -0x6117);
    } else {
        if (-(sub_s029_0e76_01a0(v_di, p_8) - 0x63) >= p_a) {
            v_si = p_a;
        } else {
            v_si = -(sub_s029_0e76_01a0(v_di, p_8) - 0x63);
        }
        sub_s029_0e76_02ea(v_di, p_8, v_si);
        sub_s029_0e76_2c4e(v_di, D_907e, 0x63, 0x49, p_8, v_si, 6, sub_root_0000_0c70(3) + p_c - 5, 1, 0);
    }
    D_26bc = 1;
}

/* sub_s029_0e76_4c04: not decompiled: dx */
void far sub_s029_0e76_4c04(void) { }

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
