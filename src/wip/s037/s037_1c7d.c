/* place: s037 1c7d:0002 */
/* orig: size=1a96 CONST=7ec4+000e _DATA=479e+01c3 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: 8/11 match, data matches. 0d24 (save loader) MATCHES.
   0002: row/k share SI in 6.00A, original has row in DI, k in SI, key at [bp-2], last spilled
         to [bp-4] (same problem as s038 1462 0000).
   030a: only the 6.00 constant temp `mov ax,10h; mov [bp-2],ax; push ax; push 12h; push ax`
         (cases 0x4800/0x4d00) is left; [bp-2] is the loop's induction temp slot in 6.00A.
   09c6: q = &F_2e38_1ffc is constant-propagated by 6.00A (original keeps it at [bp-6] and
         reloads with les); frame 8 vs 6.
   Load-game screen (pics\loadscrn.pic): list of saves\*.sav, delete,
   load (0d24 reads the whole save file).

   Save file layout (saves\NAME.sav, read by 0d24):
     char name[0x15]; char desc[0x4f]; int seed (D_9c4a); int ?; int year, month, day, hour
     (D_00e6, D_00e4, D_00e2, D_00e0); int D_906b, D_906d, D_906f; long ?; int fame (D_9082);
     int city (D_907e); D_e3d0, D_e3d2, D_a772, D_e898, D_e7d8, D_e896, D_a88d, D_9072..D_9078,
     D_a67c (ints); char D_906a (bit 7 -> D_9081 bit 0); int D_db27, D_da34; char D_c624[6];
     char D_907b, D_01bf (sound), D_01be (music); int D_a891; char ?; 36 unused ints;
     int D_a67e; int nchars; int ids[5] (party slots); char D_a668[20]; char tab[5][24];
     nchars x 0x22a-byte character records (0x80 bytes member record + far tables
     F_2e38_082c[0x14], F_2e38_07be[0x16], F_2e38_003e[0x180]);
     int nevents; nevents x 0x30-byte events; 0x5dce bytes at far D_a885; then cache.tmp. */

struct res { int a; int b; };
extern int D_8c50;              /* cursor row */
struct font { char _p0[0xc]; int color; };
struct win { char _p0[0x10]; int f10; };
struct find_t { char reserved[21]; char attrib; unsigned wr_time; unsigned wr_date; long size; char name[13]; };
struct rect { int x0; int y0; int x1; int y1; int _8; };
typedef struct { char _p[16]; } FILE;
#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned far *)&(fp)))

extern char D_00e0[], D_00e2[], D_00e4[], D_00e6[];
extern char D_01be, D_01bf;
extern char D_07aa, D_07bc;
extern char D_07d8;
extern int D_0154;
extern int D_017c;
extern unsigned D_2870;
extern char D_27f4[];
extern int D_2800;
extern int D_8c4c;              /* number of saved games */
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern int D_8c52;              /* first save shown */
extern int D_9072[], D_9074[], D_9076[], D_9078[];
extern unsigned D_9080;
extern char D_9081;
extern int D_9082[];
extern char D_9085[];
extern char D_906a;
extern char D_906b[], D_906d[], D_906f[];
extern char D_907a;
extern char D_907b[];
extern int D_907e;
extern int D_9417;
extern char D_9429, D_9433;
extern char far *D_9785;
extern char D_978b[];
extern char D_987f;
extern char D_9920, D_9934;
extern char *D_996e, *D_9982;
extern int D_9c42;
extern char D_9c4a[];
extern char D_9c55[];
extern char D_9c69[][0x80];
extern char D_a668[];
extern unsigned D_a2d7[];
extern char D_a67c[];
extern char D_a67e[];
extern char D_a680[];
extern int D_a68f;
extern char D_a752[];
extern char D_a772[];
extern unsigned D_a776;
extern int D_a86e, D_a870, D_a872, D_a874;
extern char D_a876;
extern unsigned char far *D_a885;
extern char D_a88d[];
extern int D_a891;
extern char D_a6b4[];
extern char D_c102;
extern char D_c624[];
extern unsigned D_ca45[];
extern char D_da34[];
extern char D_db27[];
extern unsigned D_d6ee, D_d6f2, D_d6f9, D_d70a, D_d726, D_d729, D_d73c, D_d800;
extern char D_e3d0[], D_e3d2[];
extern unsigned D_e416;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
extern char D_e7d8[];
extern int D_e88a;
extern char D_e896[], D_e898[];
extern char D_e89a;
extern struct { char _p[0x1dd3]; char far *s[3]; } far F_290e_0000;
extern unsigned far F_20a5_4d62;
extern char far F_20a5_763b[2];
extern unsigned far F_2e38_1ffc;
extern char far * far F_2e38_1b2c[0x12c];
extern char far * far F_2e38_10be[];
extern char far F_20a5_33fc[5][0x18];
extern char far F_2e38_3b2e[][15];
extern char far F_2e38_082c[][0x14];
extern char far F_2e38_07be[][0x16];
extern char far F_2e38_003e[][0x180];
int far _dos_close();
unsigned far _dos_findfirst(char *, unsigned, struct find_t *);
int far _dos_findnext();
int far _dos_read();
int far _dos_freemem();
int far _fstrcmp();
int far _fstrcpy();
int far _fstrcat();
int far sprintf();
int far printf();
void far exit_(int);
extern int D_8c54;              /* rows shown */
int far fopen();
int far fdopen();
int far fread();
int far fwrite();
int far fclose();
int far fseek();
long far ftell();
int far fgetc();
int far remove();
int far fcloseall();
int far longjmp();
unsigned far alloc_buffer();
void far free_buffer();
int far copy_rect_saved();
char *strcat();
int far sub_root_0000_13a0();
int far sub_root_0000_1384();
int far sub_root_0000_1438();
int far sub_root_0000_23e6();
int far sub_root_0000_2522();
int far sub_root_0000_262e();
void far F_0000_2652(int);
int far sub_root_0265_0004();
int far sub_root_047c_0066();
int far sub_root_047c_00a9();
int far sub_root_047c_040b();
int far sub_root_047c_04dc();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far sub_root_047c_0553();
int far sub_root_047c_055a();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_05a5();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_0783();
int far sub_root_047c_0791();
int far sub_root_047c_07ce();
int far sub_root_047c_08c8();
int far sub_root_047c_08f9();
int far sub_root_047c_124f();
int far sub_root_047c_1293();
int far sub_root_047c_12c3();
int far sub_root_047c_1320();
int far sub_root_05ba_00c2();
int far sub_root_0617_024a();
int far sub_root_0617_045c();
int far sub_root_06a1_2c9e();
int far sub_s029_1367_0066();
int far sub_s037_1462_4184();
int far vec_18d3();
int far vec_1905();
int far vec_1919();
int far vec_1b17();
int far vec_1cb1();
int far sub_s037_1c7d_0002();
int far sub_s037_1c7d_0218();
int far sub_s037_1c7d_024e();
void far sub_s037_1c7d_02ce();
int far sub_s037_1c7d_030a();
void far sub_s037_1c7d_06e6();
void far sub_s037_1c7d_095e();
void far sub_s037_1c7d_09c6();
void far sub_s037_1c7d_0d24();
void far sub_s037_1c7d_19c6();
int far sub_s037_1c7d_1a60();

char D_479e[] = "Loading Game";

int far sub_s037_1c7d_0002(void)
{
    int key;
    int last;
    int row;
    int k;
    last = -1;
    sub_root_047c_0568();
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_s037_1c7d_024e();
    sub_s037_1c7d_02ce();
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (!(D_6ef0 & 1) && D_6eec > 0x44 && D_6eec < 0xc6 && D_6eee > 0x2d && D_6eee < 0x5a) {
            row = (D_6eee - 0x2d) / 15;
            if (row != last) {
                sub_root_047c_12c3();
                if (last != -1)
                    sub_root_047c_04dc(D_27f4, 0x45, last * 15 + 0x2f, 0x80, 8, 9, 8);
                last = row;
                sub_root_047c_04dc(D_27f4, 0x45, row * 15 + 0x2f, 0x80, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0x44 && D_6eec < 0xc6 && D_6eee > 0x2d && D_6eee < 0x5a) {
            while (D_6ef0 & 1)
                ;
            switch (row = (D_6eee - 0x2d) / 15) {
            case 0:
                key = 0x264c;
                break;
            case 1:
                key = 0x2044;
                break;
            case 2:
                if (D_e89a == 1)
                    key = 0x11b;
                break;
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0xc3 && D_6eec < 0x140 && D_6eee > 0x14 && D_6eee < 0xa5) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x14) / 16;
            if (D_8c54 > row) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x12, 0x10);
                D_8c50 = row;
                sub_root_047c_04dc(D_27f4, 0xc4, (row << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
                sub_root_047c_1293();
            }
        }
        k = key;
        if (sub_root_047c_073d())
            k = sub_root_047c_0744();
        if (D_e89a == 1) {
            if (k == 0x11b || k == 0x1352 || k == 0x1372)
                return sub_s037_1c7d_0218(0);
        } else if (k == 0x11b)
            return sub_s037_1c7d_0218(0);
        if (k && sub_s037_1c7d_030a(k) == 0x11b)
            return sub_s037_1c7d_0218(1);
    }
}

int far sub_s037_1c7d_0218(int p_6)
{
    sub_root_05ba_00c2(D_e88a, "b");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    while (D_6ef0 & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s037_1c7d_024e(void)
{
    sub_root_047c_040b(1);
    sub_root_047c_040b(2);
    sub_root_047c_040b(3);
    sub_root_047c_040b(4);
    D_07d8 = 0;
    sub_root_0617_024a(1, "pics\\loadscrn.pic", D_e48c);
    sub_root_047c_0545();
    D_e88a = sub_root_0617_045c("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    sub_root_047c_058b(D_e48c);
    D_2812 = 8;
    D_2816 = 1;
    D_2804 = 1;
    D_8c4c = sub_s037_1c7d_1a60();
    D_8c52 = 0;
    return D_8c50 = 0;
}

void far sub_s037_1c7d_02ce(void)
{
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_s037_1c7d_095e(D_27f4);
    sub_s037_1c7d_06e6(D_27f4);
    sub_root_047c_1293();
}

/* button/key handler: 0x1000 quit, 0x2044/0x2064 delete, 0x264c/0x266c load,
   0x4700 home, 0x4800/0x4b00 up, 0x4d00/0x5000 down, 0x4f00 end */
int far sub_s037_1c7d_030a(int key)
{
    int i;
    int h;
    int nr;
    int n;
    int ids[5];
    char name[20];
    switch (key) {
    case 0x1000:
        sub_s037_1c7d_0218(0);
        sub_root_0000_13a0();
        break;
    case 0x264c:
    case 0x266c:
        if (D_8c54 > D_8c50) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x42, 0x2c, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x52, 0x30, F_290e_0000.s[0]);
            sub_s037_1c7d_0d24((char far *)F_2e38_3b2e[D_8c50]);
            sub_root_06a1_2c9e("charactr.tmp", 0, &h);
            _dos_read(h, (int far *)&n, 2, &nr);
            _dos_read(h, (int far *)ids, 10, &nr);
            _dos_close(h);
            for (i = 0; i < 5; i++) {
                D_a2d7[i] = 0;
                if (ids[i] != -1) {
                    sub_root_047c_0545();
                    strcpy(name, "pics\\");
                    strcat(name, "shield");
                    name[11] = D_9c69[i][1];
                    name[12] = 0;
                    strcat(name, ".pic");
                    D_a2d7[i] = sub_root_0617_045c(name);
                    sub_root_047c_04f8();
                }
            }
            sub_root_047c_1293();
            F_0000_2652(0x35);
            return 0x11b;
        }
        break;
    case 0x2044:
    case 0x2064:
        sub_root_047c_12c3();
        sub_root_047c_04dc(D_27f4, 0x45, 0x2f, 0x80, 0x2d, 9, 8);
        sub_root_047c_05a5(D_27f4, 0x41, 0x3b, D_e88a);
        D_2800 = 8;
        sub_root_0265_0004(D_27f4, 0x52, 0x3f, F_290e_0000.s[1]);
        if (sub_root_0000_2522("Delete this Game?")) {
            sub_root_047c_12c3();
            sub_s037_1c7d_19c6((char far *)F_2e38_3b2e[D_8c50]);
            sub_root_047c_1293();
            sub_s037_1c7d_06e6(D_27f4);
        }
        sub_root_047c_12c3();
        sub_root_047c_0553(0x2806, 0x41, 0x3b, 0x82, 0xe, D_27f4, 0x41, 0x3b);
        D_2800 = 8;
        sub_root_0265_0004(D_27f4, 0x52, 0x3f, F_290e_0000.s[1]);
        sub_root_047c_04dc(D_27f4, 0x45, 0x3e, 0x80, 8, 8, 9);
        sub_root_047c_1293();
        break;
    case 0x4700:
        D_8c50 = D_8c52 = 0;
        sub_s037_1c7d_06e6(D_27f4);
        break;
    case 0x4800:
    case 0x4b00:
        i = 0x10;
        sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, i, 0x12, i);
        if (--D_8c50 < 0) {
            if (D_8c52) {
                D_8c52--;
                D_8c50 = 0;
                sub_s037_1c7d_06e6(D_27f4);
            } else
                D_8c50 = 0;
        }
        sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
        break;
    case 0x4d00:
    case 0x5000:
        i = 0x10;
        sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, i, 0x12, i);
        if (++D_8c50 >= D_8c54) {
            if (D_8c4c - 9 < ++D_8c52) {
                D_8c50--;
                D_8c52--;
            } else {
                D_8c50--;
                sub_s037_1c7d_06e6(D_27f4);
            }
        }
        sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
        break;
    case 0x4f00:
        D_8c52 = sub_s029_1367_0066(0, 9999, D_8c4c - 9);
        D_8c50 = sub_s029_1367_0066(0, D_8c4c, 8);
        sub_s037_1c7d_06e6(D_27f4);
        break;
    }
}

/* list of saved games: "name day/month/year  fame" and the description */
void far sub_s037_1c7d_06e6(struct font *f)
{
    int skip;
    int h;
    int nr;
    int year, month, day, hour, seed, dummy;
    char name[0x15];
    char desc[0x4f];
    char path[0x1e];
    struct find_t ff;
    char line[0x52];
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0xc3, 0x14, 0x7d, 0x91, D_27f4, 0xc3, 0x14);
    D_8c54 = skip = 0;
    if (_dos_findfirst("saves\\*.sav", 0, &ff))
        return;
    do {
        if (skip++ >= D_8c52) {
            if (++D_8c54 > 9) {
                D_8c54--;
                break;
            }
            strcpy(path, "saves\\");
            strcat(path, ff.name);
            _fstrcpy((char far *)F_2e38_3b2e[D_8c54 - 1], (char far *)ff.name);
            sub_root_06a1_2c9e(path, 0, &h);
            _dos_read(h, (char far *)name, 0x15, &nr);
            _dos_read(h, (char far *)desc, 0x4f, &nr);
            _dos_read(h, (int far *)&seed, 2, &nr);
            _dos_read(h, (int far *)&dummy, 2, &nr);
            _dos_read(h, (int far *)&year, 2, &nr);
            _dos_read(h, (int far *)&month, 2, &nr);
            _dos_read(h, (int far *)&day, 2, &nr);
            _dos_read(h, (int far *)&hour, 2, &nr);
            _dos_close(h);
            sprintf(line, "%s %d/%d/%d  %d", name, year, month + 1, day, hour);
            f->color = 0x11;
            sub_root_047c_055a(f, 0xc6, (D_8c54 << 4) + 7, line);
            sub_root_047c_055a(f, 0xc6, (D_8c54 << 4) + 0xf, desc);
            f->color = 0x10;
            sub_root_047c_055a(f, 0xc5, (D_8c54 << 4) + 6, line);
            sub_root_047c_055a(f, 0xc5, (D_8c54 << 4) + 0xe, desc);
        }
    } while (!_dos_findnext(&ff));
    if (D_8c4c <= D_8c50)
        D_8c50 = D_8c54 - 1;
    sub_root_047c_04dc(D_27f4, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
    sub_root_047c_1293();
}

void far sub_s037_1c7d_095e(struct font *f)
{
    f->color = 8;
    sub_root_0265_0004(f, 0x52, 0x2f, F_290e_0000.s[0]);
    sub_root_0265_0004(f, 0x52, 0x3e, F_290e_0000.s[1]);
    if (D_e89a == 1)
        sub_root_0265_0004(f, 0x52, 0x4d, F_290e_0000.s[2]);
}

/* load: confirm, tear down the running game (free buffers), then reload and longjmp
   back into the main loop */
void far sub_s037_1c7d_09c6(void)
{
    int i;
    unsigned far *q;
    ((struct win *)D_017c)->f10 = 3;
    if (!sub_root_0000_2522("Loading is irreversable. Proceed?"))
        return;
    vec_1905("load", "a");
    if (D_907a == 1) {
        sub_root_047c_12c3();
        sub_root_047c_124f(0, 0, D_9c42);
        D_a68f = D_9c42;
        sub_root_0000_262e();
        for (i = 0; i < 5; i++) {
            if (D_a2d7[i]) {
                free_buffer(D_a2d7[i], "SGS");
                D_a2d7[i] = 0;
            }
        }
        *(long *)D_9085 = *(long *)"Seq";
        for (i = 0; i < 5; i++) {
            if (D_ca45[i]) {
                free_buffer(D_ca45[i], D_9085);
                D_ca45[i] = 0;
            }
        }
        free_buffer(F_20a5_4d62, "MiscSp");
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
        vec_1cb1();
        F_20a5_763b[0] = 'D';
        F_20a5_763b[1] = 'G';
        for (i = 0; i < 5; i++) {
            if (D_9c69[i][0] == 1 || D_9c69[i][0] == 2) {
                *(int *)&D_9c69[i][-4] = 0;
                D_9c69[i][0x36] = -1;
                D_a6b4[i] = 0;
            }
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
        vec_1919(0);
    }
    if (D_a776)
        free_buffer(D_a776, "b");
    D_a776 = 0;
    for (i = 0; i < 5; i++) {
        if (D_a2d7[i])
            free_buffer(D_a2d7[i], "sg");
        D_9c69[i][0] = 0;
    }
    if (D_a891 == 3) {
        fcloseall();
        if (D_2870)
            free_buffer(D_2870, "Map");
        q = &F_2e38_1ffc;
        if (*q)
            free_buffer(*q, "S");
        if (D_e416)
            _dos_freemem(D_e416);
        D_e416 = *q = D_2870 = 0;
    }
    for (i = 0; i < 0x12c; i++) {
        if (F_2e38_1b2c[i]) {
            free_buffer(FP_SEG(F_2e38_1b2c[i]), "event");
            F_2e38_1b2c[i] = 0;
        }
    }
    sub_s037_1c7d_0002();
    longjmp(D_a752, -1);
}

char D_4919[0x30] = {0};

void far sub_s037_1c7d_0d24(char far *file)
{
    char onoffg[6];
    int dummyc;
    long dummy4o;
    char tabh[5][0x18];
    int idso[5];
    char flage;
    struct find_t ffc;
    FILE *fi;
    int je;
    char far *p;
    int zeroa;
    int hj;
    int ncharsd;
    long posb;
    char pathh[26];
    char namee[0x15];
    char desch[0x52];
    int nrh;
    char cf;
    FILE *f2b;
    int dummy2m[6];
    struct rect rj;
    int id;
    int n;

    flage = 0;
    zeroa = 0;
    _fstrcpy((char far *)D_a680, file);
    strcpy(pathh, "saves\\");
    _fstrcat((char far *)pathh, file);
    if (_dos_findfirst(pathh, 0, &ffc)) {
        printf("load");
        exit_(1);
    } else {
        sub_root_06a1_2c9e(pathh, 0, &hj);
        fi = fdopen(hj, "rb");
        fread(namee, 1, 0x15, fi);
        fread(desch, 1, 0x4f, fi);
        fread(D_9c4a, 2, 1, fi);
        fread(dummy2m, 2, 1, fi);
        fread(D_00e6, 2, 1, fi);
        fread(D_00e4, 2, 1, fi);
        fread(D_00e2, 2, 1, fi);
        fread(D_00e0, 2, 1, fi);
        fread(D_906b, 2, 1, fi);
        fread(D_906d, 2, 1, fi);
        fread(D_906f, 2, 1, fi);
        fread(&dummy4o, 4, 1, fi);
        fread(D_9082, 2, 1, fi);
        fread(&D_907e, 2, 1, fi);
        fread(D_e3d0, 2, 1, fi);
        fread(D_e3d2, 2, 1, fi);
        fread(D_a772, 2, 1, fi);
        fread(D_e898, 2, 1, fi);
        fread(D_e7d8, 2, 1, fi);
        fread(D_e896, 2, 1, fi);
        fread(D_a88d, 2, 1, fi);
        fread(D_9072, 2, 1, fi);
        fread(D_9074, 2, 1, fi);
        fread(D_9076, 2, 1, fi);
        fread(D_9078, 2, 1, fi);
        fread(D_a67c, 2, 1, fi);
        fread(&D_906a, 1, 1, fi);
        fread(D_db27, 2, 1, fi);
        fread(D_da34, 2, 1, fi);
        fread(D_c624, 6, 1, fi);
        fread(D_907b, 1, 1, fi);
        fread(&D_01bf, 1, 1, fi);
        fread(&D_01be, 1, 1, fi);
        fread(&D_a891, 2, 1, fi);
        fread(&flage, 1, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        if (D_906a & 0x80) {
            D_9081 |= 1;
            D_906a &= 0x7f;
        } else
            D_9081 &= 0xfe;
        sub_root_0000_1438(1);
        sub_root_047c_12c3();
        if (D_907a == 1) {
            D_907a = 0;
            sub_root_0000_23e6(D_017c, -1, 1, D_479e, D_0154, &rj);
            D_907a = 1;
        } else
            sub_root_0000_23e6(D_017c, -1, 1, D_479e, D_0154, &rj);
        sub_root_047c_1293();
        if (D_01bf == 1) {
            strcat(onoffg, "ON");
            *D_996e = 1;
        } else {
            strcat(onoffg, "OFF");
            *D_996e = 2;
        }
        if (D_01be == 1) {
            strcat(onoffg, "ON");
            *D_9982 = 1;
        } else {
            strcat(onoffg, "OFF");
            *D_9982 = 2;
        }
        fread(D_a67e, 2, 1, fi);
        fread(&ncharsd, 2, 1, fi);
        fread(idso, 10, 1, fi);
        fread(D_a668, 0x14, 1, fi);
        fread(tabh, 0x78, 1, fi);
        posb = ftell(fi);
        for (id = 0; id < ncharsd; id++) {
            for (je = 0; je < 5; je++)
                if (idso[je] == id)
                    break;
            if (je < 5) {
                fseek(fi, posb, 0);
                fseek(fi, (long)(unsigned)(idso[je] * 0x22a), 1);
                fread(&D_9c55[je << 7], 0x80, 1, fi);
                fseek(fi, 0L, 1);
                _dos_read(hj, (char far *)F_2e38_082c[je], 0x14, &nrh);
                _dos_read(hj, (char far *)F_2e38_07be[je], 0x16, &nrh);
                _dos_read(hj, (char far *)F_2e38_003e[je], 0x180, &nrh);
                if (D_9c69[je][1] < 'A')
                    D_9c69[je][1] = 'A';
            } else {
                fseek(fi, 0L, 1);
                _dos_read(hj, (char far *)D_9085, 0x22a, &nrh);
            }
        }
        for (id = 0; id < 5; id++)
            for (je = 0; je < 0x18; je++)
                F_20a5_33fc[id][je] = tabh[id][je];
        fread(&n, 2, 1, fi);
        for (id = 0; id < n; id++) {
            F_2e38_1b2c[id] = (char far *)((unsigned long)alloc_buffer(0x30L, 0) << 16);
            fseek(fi, 0L, 1);
            _dos_read(hj, F_2e38_1b2c[id], 0x30, &nrh);
        }
        fseek(fi, 0L, 1);
        _dos_read(hj, D_a885, 0x5dce, &nrh);
        id = D_a885[0];
        n = D_a885[1];
        p = D_a885 + 1;
        p++;
        n = (n << 8) + id;
        for (id = 0; id < n; id++) {
            F_2e38_10be[id] = p;
            p += 0x3a;
        }
        sub_s037_1462_4184(file);
        if (_dos_findfirst("cache.tmp", 0, &ffc)) {
            printf("cache");
            exit_(1);
        } else {
            fseek(fi, 0L, 1);
            f2b = fopen("cache.tmp", "wb");
            while (fread(&cf, 1, 1, fi))
                fwrite(&cf, 1, 1, f2b);
            fclose(f2b);
        }
        if (_dos_findfirst("darkland.dsc", 0, &ffc)) {
            printf("city");
            exit_(1);
        } else {
            f2b = fopen("darkland.dsc", "r");
            fgetc(f2b);
            fseek(f2b, (long)(D_907e * 0x50 + 1), 0);
            fread(D_978b, 0x50, 1, f2b);
            fclose(f2b);
        }
        if (_dos_findfirst("darkland.cty", 0, &ffc)) {
            printf("city");
            exit_(1);
        } else {
            sub_root_06a1_2c9e("darkland.cty", 0, &hj);
            f2b = fdopen(hj, "r");
            fseek(f2b, (long)D_907e * 0x26e + 1, 0);
            _dos_read(hj, D_9785, 0x26e, &id);
            fclose(f2b);
        }
        fclose(fi);
    }
    vec_1b17(D_a680);
    sub_root_047c_12c3();
    if (D_907a == 1) {
        D_907a = 0;
        copy_rect_saved(D_0154, rj.x0, rj.y0, rj.x1, rj.y1, D_017c, rj.x0, rj.y0);
        D_907a = 1;
    } else
        copy_rect_saved(D_0154, rj.x0, rj.y0, rj.x1, rj.y1, D_017c, rj.x0, rj.y0);
    sub_root_047c_1293();
}

/* delete a saved game (NAME.sav and NAME.bsv) unless it is the one loaded */
void far sub_s037_1c7d_19c6(char far *file)
{
    char path[26];
    if (_fstrcmp((char far *)D_a680, file)) {
        strcpy(path, "saves\\");
        _fstrcat((char far *)path, file);
        remove(path);
        path[strlen(path) - 3] = 0;
        strcat(path, "bsv");
        remove(path);
        D_8c4c--;
    }
}

int far sub_s037_1c7d_1a60(void)
{
    struct find_t ff;
    int n;
    n = 0;
    if (_dos_findfirst("saves\\*.sav", 0, &ff))
        return 0;
    do {
        n++;
    } while (!_dos_findnext(&ff));
    return n;
}
