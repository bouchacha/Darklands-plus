/* place: s037 1462:0000 */
/* orig: size=4664 CONST=7ea6+0008 _DATA=403c+03b1 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: ALL 20 FUNCTIONS WRITTEN; _DATA matches. 14/20 match (0348 0390 03d4 2922 2a68 38aa
   3992 3a8a 3ae2 3b58 3cd6 3f30 4048 4562). 3b58 MATCHED (m1) by reordering its local
   declarations (n, i, nr, ids, h, c, fp): the `cmp dx,di` operand order follows them. Character creation main menu (pics\cretscrn.pic) -
   twin of s038 1462 (its 0000/01fe/024e/0292/02f8/23ec/25ca are 0000/0348/0390/03d4/0444/
   2760/2922 here; see that file for the charactr.tmp layout), plus save game (2a68/2ace),
   party add/remove (3cd6/3b58/3f30), charactr.tmp record helpers and the text input box.
   Left:
     - 0444 (key handler, 8990/8988): ported from s038 02f8 + new cases B (begin game),
       D (drop from party), K (delete character); colour cases 1/2/3 generated from the asm
       (e4 gencol.py) - the 2nd case's fifth "A00" test cycles D_e894 (original bug, kept).
       Known 6.00 differences as in s038 (il*2 CSE in DI across calls, push di/pop di around
       inline strcpy, cross-jump copy choice), imul operand order D_e7da * h.
     - 0000: same length now (key used directly in the tail instead of a copy, pragma "a");
       only the unused 2-byte slot above key is missing (key [bp-2] vs [bp-4]), as s038 0000.
     - 2ace (save game, 3550/3548): `n > i` loop compares (`mov ax,i; cmp [n],ax`) come out
       reversed; needs #pragma optimize("a") for D_017c kept in BX.
     - 2760: slot/register layout of the list loop (6.00 temp [bp-2] = &ids).
     - 4184: 6.00 turned the unused-index copy loop into a `dec si; jne` countdown.
     - 43e6: 6.00 keeps i in SI and FP_OFF(buf) in DI; 6.00A spills i to [bp-2].
   _DATA notes: "Saving Game" and the 0x30-byte zero block are initialised globals; MSC
   emits a global after the literals of the function FOLLOWING its definition, so they are
   defined before 0000 and before 2ace. */

struct res { int a; int b; };
#define FP_SEG(fp) (*((unsigned _far *)&(fp) + 1))
extern struct font *D_017c;      /* current screen */
extern int D_0154;
extern int D_00e0, D_00e2, D_00e4, D_00e6;
extern char D_01be, D_01bf;
extern int D_a53e[][9];          /* per party member, 0x12 bytes; -536 = none */
extern char D_906a;
extern int D_906b, D_906d, D_906f, D_9072, D_9074, D_9076, D_9078, D_9082, D_9c4a;
extern int D_e3d0, D_e3d2, D_a772, D_e898, D_e7d8, D_e896, D_a88d, D_a67c, D_db27, D_da34, D_a891;
extern char D_c624[6];
extern char D_a680[];            /* name of the save file */
extern char far *D_a885;         /* location memory */
extern int D_a887;
extern char far * far F_2e38_1b2c[0x12c];   /* events (0x30 bytes each) */
int far vec_1ccf();
int far vec_1cc5();
int far vec_1cbb();
int far vec_1b21();
int far sub_root_0000_23e6();
int far sub_root_05ba_0004();
int far sub_root_0617_011e();
int far sub_root_06a1_12fa();
int far sub_root_03ec_000e();
int far creat();
int far itoa();
int far fwrite();
int far exit_();
int far _fstrcpy();
long far ftell();
void far sub_s037_18c8_0004();
struct find_t { char reserved[21]; char attrib; unsigned wr_time; unsigned wr_date; long size; char name[13]; };
extern char D_907a, D_907b;
extern int D_a65e;               /* text box background picture */
extern int D_e80a;               /* record of the character being created (0x80 bytes) */
extern unsigned char far F_2e38_3b1a[];
extern unsigned char far F_2e38_3b04[];
extern char far F_2e38_3984[];
extern char far * far F_2e38_10be[0x19e];   /* locations (0x3a bytes each) */
int far printf();
int far strncpy();
int far _fstrcat();
int far _dos_creat();
int far _dos_findfirst();
int far _dos_findnext();
int far remove();
int far sub_root_047c_04d5();
int far sub_root_06a1_2824();
int far sub_s029_0e76_3b62();
int far sub_s029_0e76_30fe();
extern int D_907e;   /* position matters: compare direction in 23ec */
struct font { char _p0[0xc]; int color; };
/* character record as stored in charactr.tmp (0x80 bytes, cf. party member records D_9c69) */
struct chr { int id; char _p2[0x12]; char status; char _p15[0x10]; char name[0x5b]; };
extern struct chr D_8b52;           /* character record buffer */
extern int D_8d06[];
extern char D_9085[];
extern char D_a668[][4];      /* shield code per party slot ("F01", ...) */
extern char D_9c69[][0x80];
extern struct { char _p[0x1ddf]; char far *s[0x41]; } far F_290e_0000;
int far sub_root_0000_1588();
int far sub_root_0000_1716();
int far sub_root_047c_055a();
int far _dos_close();
extern char D_27f4[];
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_9419[];
extern char D_9920;
extern char D_9934;
extern int D_9c46;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
extern int D_e7da;
extern int D_e7dc;
extern int D_e88a;
extern int D_e88e;
typedef struct { char _p[16]; } FILE;
extern char D_e753, D_e754, D_e755, D_e756, D_e757, D_e758, D_e759, D_e75a, D_e75b, D_e75c,
    D_e75d, D_e75e, D_e75f, D_e760, D_e761, D_e762, D_e763, D_e764, D_e765, D_e766, D_e767,
    D_e768, D_e769, D_e76a;     /* current portrait palette (24 bytes) */
extern char D_e7e2[][4];        /* shield codes */
extern int D_e7de, D_e7e0;      /* number of shield codes, current one */
extern int D_e890, D_e892, D_e894;  /* colour indices for keys 1/2/3 */
extern int D_2800;
extern unsigned D_a2d7[];
extern char D_9c55[];
extern struct { int v; } D_a67e;   /* number of party members (struct: compare operand order) */
extern char far F_20a5_33fc[5][0x18];   /* palette per party slot */
extern char far F_2e38_082c[][0x14];
extern char far F_2e38_07be[][0x16];
extern char far F_2e38_003e[][0x180];
FILE *far fdopen();
int far fread();
int far fclose();
int far fseek();
int far _dos_write();
int memcmp();
char *strcpy();
char *strcat();
int far sub_root_0000_13a0();
int far sub_s029_0e76_2a32();
int far sub_s029_0e76_3cde();
int far vec_2049();
int far vec_20cb();
int far _dos_freemem();
int far _dos_read();
int far sub_root_0000_0000();
int far sub_root_0265_0004();
int far sub_root_047c_04dc();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far sub_root_047c_0553();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_05a5();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_1293();
int far sub_root_047c_12c3();
int far sub_root_047c_1320();
int far sub_root_05ba_00c2();
int far sub_root_0617_024a();
int far sub_root_0617_045c();
int far sub_root_06a1_2c9e();
int far sub_s029_0e76_392c();
int far vec_20ad();
int far vec_20c1();
int far sub_s037_1462_0000();
int far sub_s037_1462_0348();
int far sub_s037_1462_0390();
void far sub_s037_1462_03d4();
int far sub_s037_1462_0444();
void far sub_s037_1462_2760(struct font *);
void far sub_s037_1462_2922(struct font *);
void far sub_s037_1462_2a68();
void far sub_s037_1462_2ace();
void far sub_s037_1462_38aa();
void far sub_s037_1462_3992();
int far sub_s037_1462_3a8a();
int far sub_s037_1462_3ae2();
void far sub_s037_1462_3b58();
int far sub_s037_1462_3cd6();
void far sub_s037_1462_3f30();
void far sub_s037_1462_4048();
void far sub_s037_1462_4184();
void far sub_s037_1462_43e6();
void far sub_s037_1462_4562();
int far vec_18f1();
int far vec_1fdb();


/* shown by 2ace */
char saving_game[] = "Saving Game";

#pragma optimize("a", on)
int far sub_s037_1462_0000(void)
{
    int key;
    int row;
    int k;
    vec_18f1(0x14);
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_root_047c_0568();
    sub_s037_1462_4562();
    D_e88e = 0;
    sub_s037_1462_4184((char far *)"default");
    for (k = 0; k < 5; k++)
        D_9c69[k][0] = 0;
    sub_s037_1462_0390();
    sub_s037_1462_03d4();
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    do {
        key = 0;
        if ((D_6ef0 & 1) && D_6eec > 0x46 && D_6eec < 0xc8 && D_6eee > 0x19 && D_6eee < 0xcd) {
            while (D_6ef0 & 1)
                ;
            switch (row = (D_6eee - 0x19) / 15) {
            case 0:
                key = 0x2e43;
                break;
            case 1:
                key = 0x1e41;
                break;
            case 2:
                key = 0x2348;
                break;
            case 3:
                key = 0x1f53;
                break;
            case 4:
                key = 0x231;
                break;
            case 5:
                key = 0x332;
                break;
            case 6:
                key = 0x433;
                break;
            case 7:
                key = 0x2044;
                break;
            case 8:
                key = 0x254b;
                break;
            case 9:
                key = 0x3042;
                break;
            case 10:
                key = 0x11b;
                break;
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0xc5 && D_6eec < 0x138 && D_6eee > 0x14 && D_6eee < 0xb4) {
            while (D_6ef0 & 1)
                ;
            k = (D_6eee - 0x14) / 8;
            if (D_e7dc > k) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x12, 0x10);
                D_e7da = k;
                sub_root_047c_04dc(D_27f4, 0xc6, (k << 3) + 0x16, 0x71, 8, 0x10, 0x12);
                sub_root_047c_1293();
            }
        }
        /* click on a party portrait: show that member (vec_1fdb), then redraw */
        if ((D_6ef0 & 1) && D_6eec < 0x3c && D_9c69[k = D_6eee / 0x28][0]) {
            vec_1fdb(k);
            sub_root_047c_12c3();
            sub_root_047c_0568();
            sub_root_0617_024a(1, "pics\\cretscrn.pic", D_e48c);
            D_e753 = F_20a5_33fc[k][0];
            D_e754 = F_20a5_33fc[k][1];
            D_e755 = F_20a5_33fc[k][2];
            D_e756 = F_20a5_33fc[k][3];
            D_e757 = F_20a5_33fc[k][4];
            D_e758 = F_20a5_33fc[k][5];
            D_e759 = F_20a5_33fc[k][6];
            D_e75a = F_20a5_33fc[k][7];
            D_e75b = F_20a5_33fc[k][8];
            D_e75c = F_20a5_33fc[k][9];
            D_e75d = F_20a5_33fc[k][10];
            D_e75e = F_20a5_33fc[k][11];
            D_e75f = F_20a5_33fc[k][12];
            D_e760 = F_20a5_33fc[k][13];
            D_e761 = F_20a5_33fc[k][14];
            D_e762 = F_20a5_33fc[k][15];
            D_e763 = F_20a5_33fc[k][16];
            D_e764 = F_20a5_33fc[k][17];
            D_e765 = F_20a5_33fc[k][18];
            D_e766 = F_20a5_33fc[k][19];
            D_e767 = F_20a5_33fc[k][20];
            D_e768 = F_20a5_33fc[k][21];
            D_e769 = F_20a5_33fc[k][22];
            D_e76a = F_20a5_33fc[k][23];
            sub_root_047c_058b(D_e48c);
            sub_s037_1462_03d4();
            sub_root_047c_1293();
            sub_root_047c_056f();
            while (D_6ef0 & 1)
                ;
        }
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if (key == 0x11b || key == 0x1352 || key == 0x1372)
            return sub_s037_1462_0348(0);
    } while (!key || sub_s037_1462_0444(key) != 0x11b);
    return sub_s037_1462_0348(1);
}
#pragma optimize("", on)

int far sub_s037_1462_0348(int p_6)
{
    if (D_e88e) {
        _dos_freemem(D_e88e);
    }
    sub_root_05ba_00c2(D_e88a, "b");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s037_1462_0390(void)
{
    sub_root_0617_024a(1, "pics\\cretscrn.pic", D_e48c);
    sub_root_047c_0545();
    D_e88a = sub_root_0617_045c("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    D_2816 = 1;
    D_2804 = 1;
    sub_root_047c_058b(D_e48c);
    D_2812 = 8;
}

void far sub_s037_1462_03d4(void)
{
    int i;
    sub_s037_1462_2922((struct font *)0x2806);
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_root_047c_1293();
    sub_s037_1462_2760((struct font *)D_27f4);
    sub_root_047c_12c3();
    for (i = 0; i < 5; i++) {
        if (D_9c69[i][0]) {
            sub_root_0000_1588(i);
            sub_root_0000_1716(i);
        }
    }
    sub_root_047c_1293();
}

/* character-creation menu commands (key code from 0000); returns 0x11b to leave the menu */
int far sub_s037_1462_0444(register int key)
{
    int x1g;
    int fhc;
    long x2h;
    FILE *fpm;
    int nrd;
    int il;
    int hn;
    int x3l;
    int idsh[5];
    char nameh[20];
    switch (key) {
    case 0x4800: case 0x4b00: case 0x4d00:
        sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (--D_e7da < 0)
            D_e7da = D_e7dc - 1;
        sub_root_047c_04dc(D_27f4, 0xc6, D_e7da * hn + 0x16, 0x71, hn, 0x10, 0x12);
        sub_root_047c_0553(0x2806, 0xa, 0xa5, 0x23, 0x23, D_27f4, 0xa, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_a668[il]);
            strcat(nameh, "small.pic");
            D_e88e = sub_root_0617_045c(nameh);
            sub_root_047c_04f8();
            D_e753 = F_20a5_33fc[il][0];
            D_e754 = F_20a5_33fc[il][1];
            D_e755 = F_20a5_33fc[il][2];
            D_e756 = F_20a5_33fc[il][3];
            D_e757 = F_20a5_33fc[il][4];
            D_e758 = F_20a5_33fc[il][5];
            D_e759 = F_20a5_33fc[il][6];
            D_e75a = F_20a5_33fc[il][7];
            D_e75b = F_20a5_33fc[il][8];
            D_e75c = F_20a5_33fc[il][9];
            D_e75d = F_20a5_33fc[il][10];
            D_e75e = F_20a5_33fc[il][11];
            D_e75f = F_20a5_33fc[il][12];
            D_e760 = F_20a5_33fc[il][13];
            D_e761 = F_20a5_33fc[il][14];
            D_e762 = F_20a5_33fc[il][15];
            D_e763 = F_20a5_33fc[il][16];
            D_e764 = F_20a5_33fc[il][17];
            D_e765 = F_20a5_33fc[il][18];
            D_e766 = F_20a5_33fc[il][19];
            D_e767 = F_20a5_33fc[il][20];
            D_e768 = F_20a5_33fc[il][21];
            D_e769 = F_20a5_33fc[il][22];
            D_e76a = F_20a5_33fc[il][23];
            sub_root_047c_058b(D_e48c);
            sub_root_047c_05a5(D_27f4, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x5000:
        sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (++D_e7da >= D_e7dc)
            D_e7da = 0;
        sub_root_047c_04dc(D_27f4, 0xc6, D_e7da * hn + 0x16, 0x71, hn, 0x10, 0x12);
        sub_root_047c_0553(0x2806, 0xa, 0xa5, 0x23, 0x23, D_27f4, 0xa, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_a668[il]);
            strcat(nameh, "small.pic");
            D_e88e = sub_root_0617_045c(nameh);
            sub_root_047c_04f8();
            D_e753 = F_20a5_33fc[il][0];
            D_e754 = F_20a5_33fc[il][1];
            D_e755 = F_20a5_33fc[il][2];
            D_e756 = F_20a5_33fc[il][3];
            D_e757 = F_20a5_33fc[il][4];
            D_e758 = F_20a5_33fc[il][5];
            D_e759 = F_20a5_33fc[il][6];
            D_e75a = F_20a5_33fc[il][7];
            D_e75b = F_20a5_33fc[il][8];
            D_e75c = F_20a5_33fc[il][9];
            D_e75d = F_20a5_33fc[il][10];
            D_e75e = F_20a5_33fc[il][11];
            D_e75f = F_20a5_33fc[il][12];
            D_e760 = F_20a5_33fc[il][13];
            D_e761 = F_20a5_33fc[il][14];
            D_e762 = F_20a5_33fc[il][15];
            D_e763 = F_20a5_33fc[il][16];
            D_e764 = F_20a5_33fc[il][17];
            D_e765 = F_20a5_33fc[il][18];
            D_e766 = F_20a5_33fc[il][19];
            D_e767 = F_20a5_33fc[il][20];
            D_e768 = F_20a5_33fc[il][21];
            D_e769 = F_20a5_33fc[il][22];
            D_e76a = F_20a5_33fc[il][23];
            sub_root_047c_058b(D_e48c);
            sub_root_047c_05a5(D_27f4, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x1352: case 0x1372:
        return 0x11b;
    case 0x3042: case 0x3062:
        /* begin the game with the current party */
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x9f, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0xa3, F_290e_0000.s[4]);
            D_a67e.v = 0;
            sub_root_06a1_2c9e("charactr.tmp", 0, &fhc);
            fpm = fdopen(fhc, "rb");
            _dos_read(fhc, (int far *)&x1g, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            x2h = ftell(fpm);
            for (il = 0; il < 5; il++) {
                if (idsh[il] == -1)
                    D_9c69[il][0] = 0;
                else
                    D_a67e.v = il + 1;
            }
            fclose(fpm);
            if (D_a67e.v)
                return 0x11b;
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_root_047c_0553(0x2806, 0x43, 0x9f, 0x82, 0xe, D_27f4, 0x43, 0x9f);
            sub_root_047c_1293();
        }
        break;
    case 0x1e41: case 0x1e61:
        /* add the selected character to the party */
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x27, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x2b, F_290e_0000.s[0]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 4) {
                for (il = 0; il < 4; il++)
                    if (idsh[il] == -1)
                        break;
                if (il >= 4) {
                    fclose(fpm);
                    sub_root_047c_0553(0x2806, 0x43, 0x27, 0x82, 0xe, D_27f4, 0x43, 0x27);
                    sub_root_047c_1293();
                    break;
                }
                idsh[il] = D_e7da;
                strcpy(D_a668[il], D_e7e2[0]);
                if (D_e88e)
                    _dos_freemem(D_e88e);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\shielda.pic");
                D_a2d7[il] = sub_root_0617_045c(nameh);
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[0]);
                strcat(nameh, "small.pic");
                D_e88e = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                D_e753 = 0x39;
                F_20a5_33fc[il][0] = 0x39;
                D_e754 = 0x36;
                F_20a5_33fc[il][1] = 0x36;
                D_e755 = 0;
                D_e759 = 0;
                D_e75c = 0;
                D_e75f = 0;
                D_e763 = 0;
                D_e764 = 0;
                D_e766 = 0;
                D_e767 = 0;
                D_e769 = 0;
                D_e76a = 0;
                F_20a5_33fc[il][2] = 0;
                D_e756 = 0x3d;
                F_20a5_33fc[il][3] = 0x3d;
                D_e757 = 0x38;
                F_20a5_33fc[il][4] = 0x38;
                D_e758 = 0x15;
                D_e760 = 0x15;
                F_20a5_33fc[il][5] = 0x15;
                F_20a5_33fc[il][6] = 0;
                D_e75a = 0x18;
                F_20a5_33fc[il][7] = 0x18;
                D_e75b = 0x28;
                F_20a5_33fc[il][8] = 0x28;
                F_20a5_33fc[il][9] = 0;
                D_e75d = 0x11;
                D_e768 = 0x11;
                F_20a5_33fc[il][10] = 0x11;
                D_e75e = 0x25;
                D_e761 = 0x25;
                F_20a5_33fc[il][11] = 0x25;
                F_20a5_33fc[il][12] = 0;
                F_20a5_33fc[il][13] = 0x15;
                F_20a5_33fc[il][14] = 0x25;
                D_e762 = 0x1c;
                F_20a5_33fc[il][15] = 0x1c;
                F_20a5_33fc[il][16] = 0;
                F_20a5_33fc[il][17] = 0;
                D_e765 = 0x17;
                F_20a5_33fc[il][18] = 0x17;
                F_20a5_33fc[il][19] = 0;
                F_20a5_33fc[il][20] = 0;
                F_20a5_33fc[il][21] = 0x11;
                F_20a5_33fc[il][22] = 0;
                F_20a5_33fc[il][23] = 0;
                sub_root_047c_058b(D_e48c);
                sub_root_047c_0553(0x2806, 0xa, 0xa5, 0x24, 0x24, D_27f4, 0xa, 0xa5);
                sub_root_047c_05a5(D_27f4, 0xc, 0xa4, D_e88e);
                fseek(fpm, (long)(unsigned)(D_e7da * 0x22a), 1);
                fread(&D_9c55[il << 7], 0x80, 1, fpm);
                fseek(fpm, 0L, 1);
                _dos_read(fhc, (char far *)F_2e38_082c[il], 0x14, &nrd);
                _dos_read(fhc, (char far *)F_2e38_07be[il], 0x16, &nrd);
                _dos_read(fhc, (char far *)F_2e38_003e[il], 0x180, &nrd);
                D_9c69[il][0] = 1;
                vec_2049(il);
                sub_root_0000_1588(il);
                sub_root_0000_1716(il);
            }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_root_047c_0553(0x2806, 0x43, 0x27, 0x82, 0xe, D_27f4, 0x43, 0x27);
            sub_root_047c_1293();
            sub_s037_1462_2760((struct font *)D_27f4);
        }
        break;
    case 0x2044: case 0x2064:
        /* drop the selected character from the party */
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x81, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x85, F_290e_0000.s[3]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_e7da) {
                    _dos_freemem(D_a2d7[il]);
                    D_a2d7[il] = 0;
                    idsh[il] = -1;
                    D_9c69[il][0] = 0;
                    sub_root_0000_1716(il);
                    break;
                }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_root_047c_0553(0x2806, 0x43, 0x81, 0x82, 0xe, D_27f4, 0x43, 0x81);
            sub_root_047c_1293();
            sub_s037_1462_2760((struct font *)D_27f4);
        }
        break;
    case 0x2348: case 0x2368:
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x36, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x3a, F_290e_0000.s[0x40]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_9c69[il][1] >= 'P')
                    D_9c69[il][1] = 'A';
                _dos_freemem(D_a2d7[il]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, "shield");
                nameh[11] = D_9c69[il][1];
                nameh[12] = 0;
                strcat(nameh, ".pic");
                D_a2d7[il] = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                sub_root_0000_1716(il);
                fclose(fpm);
                sub_root_047c_1293();
                sub_root_047c_12c3();
            }
            sub_root_047c_0553(0x2806, 0x43, 0x36, 0x82, 0xe, D_27f4, 0x43, 0x36);
            sub_root_047c_1293();
        }
        break;
    case 0x2e43: case 0x2e63:
        sub_root_047c_12c3();
        sub_root_047c_05a5(D_27f4, 0x43, 0x18, D_e88a);
        D_2800 = 8;
        sub_root_0265_0004(D_27f4, 0x4a, 0x1c, F_290e_0000.s[1]);
        sub_s037_18c8_0004();
        if (D_e88a)
            sub_root_05ba_00c2(D_e88a, "b");
        sub_s037_1462_0390();
        sub_s037_1462_03d4();
        sub_root_047c_1293();
        break;
    case 0x254b: case 0x256b:
        /* delete the selected character */
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x90, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x94, F_290e_0000.s[0x13]);
            sub_root_06a1_2c9e("charactr.tmp", 0, &fhc);
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            _dos_close(fhc);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da) {
                    _dos_freemem(D_a2d7[il]);
                    D_a2d7[il] = 0;
                    idsh[il] = -1;
                    D_9c69[il][0] = 0;
                    sub_root_0000_1716(il);
                    break;
                }
            sub_s037_1462_4048(D_e7da);
            if (D_e7dc <= D_e7da)
                D_e7da = D_e7dc - 1;
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_root_047c_0553(0x2806, 0x43, 0x90, 0x82, 0xe, D_27f4, 0x43, 0x90);
            sub_root_047c_1293();
            sub_s037_1462_2760((struct font *)D_27f4);
        }
        break;
    case 0x1f53: case 0x1f73:
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x45, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x49, F_290e_0000.s[0x1a]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_e7e0 >= D_e7de)
                    D_e7e0 = 0;
                _dos_freemem(D_e88e);
                strcpy(D_a668[il], D_e7e2[D_e7e0]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[D_e7e0]);
                strcat(nameh, "small.pic");
                D_e88e = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                sub_root_047c_0553(0x2806, 0xa, 0xa5, 0x24, 0x24, D_27f4, 0xa, 0xa5);
                sub_root_047c_05a5(D_27f4, 0xc, 0xa4, D_e88e);
                fseek(fpm, -20L, 1);
                _dos_write(fhc, (char far *)D_a668, 0x14, &nrd);
                fclose(fpm);
                sub_root_047c_1293();
                sub_root_047c_12c3();
            }
            sub_root_047c_0553(0x2806, 0x43, 0x45, 0x82, 0xe, D_27f4, 0x43, 0x45);
            sub_root_047c_1293();
        }
        break;
    case 0x231: case 0x4f31:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x39;
                        D_e754 = 0x36;
                        D_e755 = 0x0;
                        D_e756 = 0x3d;
                        D_e757 = 0x38;
                        D_e758 = 0x15;
                        break;
                    case 1:
                        D_e754 = 0x1c;
                        D_e755 = 0x1c;
                        D_e756 = 0x1c;
                        D_e753 = 0x1a;
                        D_e757 = 0x1a;
                        D_e758 = 0x1a;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 3:
                        D_e754 = 0x9;
                        D_e753 = 0x23;
                        D_e756 = 0x23;
                        D_e757 = 0xc;
                        D_e755 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0x23;
                        D_e755 = 0x9;
                        D_e754 = 0x21;
                        D_e756 = 0x21;
                        D_e757 = 0x1a;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e753 = 0x21;
                        D_e754 = 0x15;
                        D_e756 = 0x19;
                        D_e755 = 0x12;
                        D_e757 = 0x12;
                        D_e758 = 0xe;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e762 = 0x21;
                        D_e765 = 0x28;
                        D_e768 = 0x2d;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e762 = 0x2f;
                        D_e763 = 0x2a;
                        D_e764 = 0x12;
                        D_e765 = 0x37;
                        D_e766 = 0x32;
                        D_e767 = 0x1a;
                        D_e768 = 0x3e;
                        D_e769 = 0x3c;
                        D_e76a = 0x25;
                        break;
                    case 2:
                        D_e762 = 0x2d;
                        D_e763 = 0x1e;
                        D_e764 = 0x0;
                        D_e765 = 0x35;
                        D_e766 = 0x28;
                        D_e767 = 0x8;
                        D_e768 = 0x3e;
                        D_e769 = 0x33;
                        D_e76a = 0x12;
                        break;
                    case 3:
                        D_e762 = 0x1c;
                        D_e763 = 0xd;
                        D_e765 = 0x25;
                        D_e766 = 0x11;
                        D_e768 = 0x2d;
                        D_e769 = 0x15;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 4:
                        D_e762 = 0x0;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0xa;
                        D_e766 = 0xa;
                        D_e767 = 0xa;
                        D_e768 = 0x12;
                        D_e769 = 0x12;
                        D_e76a = 0x12;
                        break;
                    case 5:
                        D_e762 = 0x15;
                        D_e763 = 0x9;
                        D_e765 = 0x1a;
                        D_e766 = 0xb;
                        D_e768 = 0x21;
                        D_e769 = 0xd;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e756 = 0x1c;
                        D_e753 = 0x1c;
                        D_e757 = 0x13;
                        D_e754 = 0x13;
                        D_e758 = 0xa;
                        D_e755 = 0xa;
                        break;
                    case 1:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x23;
                        D_e754 = 0x23;
                        D_e755 = 0x23;
                        break;
                    case 2:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 3:
                        D_e756 = 0x1c;
                        D_e757 = 0x13;
                        D_e758 = 0xa;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 4:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x28;
                        D_e756 = 0x21;
                        D_e757 = 0xc;
                        D_e755 = 0x19;
                        D_e759 = 0x19;
                        D_e75a = 0x7;
                        D_e754 = 0x12;
                        D_e758 = 0x12;
                        D_e75b = 0x12;
                        break;
                    case 1:
                        D_e754 = 0x20;
                        D_e755 = 0x33;
                        D_e757 = 0x1c;
                        D_e758 = 0x2d;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        break;
                    case 2:
                        D_e753 = 0x1a;
                        D_e755 = 0x26;
                        D_e756 = 0x15;
                        D_e757 = 0x19;
                        D_e754 = 0x20;
                        D_e758 = 0x20;
                        D_e759 = 0x13;
                        D_e75a = 0x17;
                        D_e75b = 0x1e;
                        break;
                    case 3:
                        D_e753 = 0x2e;
                        D_e754 = 0x25;
                        D_e755 = 0x25;
                        D_e756 = 0x2a;
                        D_e757 = 0x21;
                        D_e758 = 0x21;
                        D_e759 = 0x23;
                        D_e75a = 0x19;
                        D_e75b = 0x19;
                        break;
                    case 4:
                        D_e753 = 0x33;
                        D_e754 = 0x2f;
                        D_e756 = 0x2e;
                        D_e758 = 0x19;
                        D_e755 = 0x25;
                        D_e757 = 0x25;
                        D_e759 = 0x25;
                        D_e75a = 0x1c;
                        D_e75b = 0x15;
                        break;
                    case 5:
                        D_e754 = 0x1e;
                        D_e755 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x12;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x10;
                        D_e75b = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;
    case 0x332: case 0x5032:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        D_e75d = 0x11;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x15;
                        D_e75e = 0x25;
                        D_e761 = 0x25;
                        break;
                    case 1:
                        D_e75b = 0x0;
                        D_e75c = 0xc;
                        D_e75a = 0x7;
                        D_e75d = 0x7;
                        D_e759 = 0x6;
                        D_e75e = 0x6;
                        D_e75f = 0x8;
                        D_e760 = 0x8;
                        D_e761 = 0x5;
                        break;
                    case 2:
                        D_e75a = 0x18;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0xf;
                        D_e75d = 0x12;
                        D_e75e = 0xe;
                        D_e760 = 0x13;
                        D_e75f = 0xc;
                        D_e761 = 0xc;
                        break;
                    case 3:
                        D_e759 = 0x35;
                        D_e75a = 0x35;
                        D_e75b = 0xb;
                        D_e75c = 0x2f;
                        D_e75d = 0x2f;
                        D_e75e = 0x8;
                        D_e75f = 0x39;
                        D_e760 = 0x2e;
                        D_e761 = 0x9;
                        break;
                    case 4:
                        D_e759 = 0x31;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75d = 0x13;
                        D_e75e = 0x5;
                        D_e75c = 0x25;
                        D_e75f = 0x25;
                        D_e760 = 0x17;
                        D_e761 = 0x2;
                        break;
                    case 5:
                        D_e759 = 0x1c;
                        D_e75a = 0xa;
                        D_e75c = 0x28;
                        D_e75d = 0x10;
                        D_e75e = 0x11;
                        D_e75f = 0x21;
                        D_e75b = 0xb;
                        D_e760 = 0xb;
                        D_e761 = 0xc;
                        break;
                    }
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e762 = 0x18;
                        D_e763 = 0x11;
                        D_e764 = 0x9;
                        D_e765 = 0x15;
                        D_e766 = 0x10;
                        D_e767 = 0x8;
                        D_e768 = 0x13;
                        D_e769 = 0xe;
                        D_e76a = 0x7;
                        break;
                    case 1:
                        D_e762 = 0x12;
                        D_e763 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0xd;
                        D_e766 = 0xd;
                        D_e767 = 0xd;
                        D_e768 = 0xa;
                        D_e769 = 0xa;
                        D_e76a = 0xa;
                        break;
                    case 2:
                        D_e763 = 0x1c;
                        D_e766 = 0x15;
                        D_e769 = 0x10;
                        D_e762 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0x0;
                        D_e767 = 0x0;
                        D_e768 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 3:
                        D_e762 = 0x28;
                        D_e763 = 0x25;
                        D_e764 = 0x14;
                        D_e765 = 0x21;
                        D_e766 = 0x20;
                        D_e767 = 0x11;
                        D_e768 = 0x1c;
                        D_e769 = 0x1a;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e762 = 0x21;
                        D_e763 = 0x1e;
                        D_e764 = 0x1e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1a;
                        D_e767 = 0x1a;
                        D_e768 = 0x18;
                        D_e769 = 0x14;
                        D_e76a = 0x14;
                        break;
                    case 5:
                        D_e762 = 0x32;
                        D_e763 = 0x32;
                        D_e764 = 0x25;
                        D_e765 = 0x2a;
                        D_e766 = 0x2a;
                        D_e767 = 0x1c;
                        D_e768 = 0x23;
                        D_e769 = 0x23;
                        D_e76a = 0x15;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75f = 0x10;
                        D_e760 = 0x10;
                        D_e761 = 0x12;
                        D_e75c = 0x14;
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e759 = 0x18;
                        D_e75a = 0x18;
                        D_e75b = 0x1a;
                        D_e756 = 0x1c;
                        D_e757 = 0x1c;
                        D_e758 = 0x1e;
                        break;
                    case 1:
                        D_e75f = 0x1c;
                        D_e760 = 0x21;
                        D_e75c = 0x14;
                        D_e75d = 0x19;
                        D_e759 = 0xe;
                        D_e75a = 0xe;
                        D_e761 = 0x0;
                        D_e75e = 0x0;
                        D_e75b = 0x0;
                        D_e757 = 0x9;
                        D_e756 = 0x8;
                        D_e758 = 0x8;
                        break;
                    case 2:
                        D_e761 = 0xd;
                        D_e75d = 0x4;
                        D_e75e = 0x11;
                        D_e75a = 0xa;
                        D_e75b = 0x14;
                        D_e75f = 0x0;
                        D_e760 = 0x0;
                        D_e75c = 0x0;
                        D_e759 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x12;
                        D_e758 = 0x19;
                        break;
                    case 3:
                        D_e75f = 0x11;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e759 = 0x19;
                        D_e75a = 0x12;
                        D_e756 = 0x1e;
                        D_e75c = 0x15;
                        D_e757 = 0x15;
                        D_e761 = 0x0;
                        D_e75e = 0x0;
                        D_e75b = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e75f = 0x15;
                        D_e75c = 0x19;
                        D_e759 = 0x1c;
                        D_e756 = 0x21;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        D_e75d = 0x0;
                        D_e75e = 0x0;
                        D_e75a = 0x0;
                        D_e75b = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e75f = 0x0;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        D_e75c = 0x9;
                        D_e75d = 0x9;
                        D_e75e = 0x9;
                        D_e759 = 0xe;
                        D_e75a = 0xe;
                        D_e75b = 0xe;
                        D_e756 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x15;
                        break;
                    }
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;
    case 0x433: case 0x5133:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e762 = 0x1c;
                        D_e765 = 0x17;
                        D_e768 = 0x11;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e764 = 0x0;
                        D_e765 = 0xc;
                        D_e763 = 0x7;
                        D_e766 = 0x7;
                        D_e762 = 0x6;
                        D_e767 = 0x6;
                        D_e768 = 0x8;
                        D_e769 = 0x8;
                        D_e76a = 0x5;
                        break;
                    case 2:
                        D_e762 = 0x2b;
                        D_e763 = 0x2a;
                        D_e764 = 0x1c;
                        D_e765 = 0x23;
                        D_e766 = 0x23;
                        D_e767 = 0x12;
                        D_e768 = 0x28;
                        D_e769 = 0x25;
                        D_e76a = 0x13;
                        break;
                    case 3:
                        D_e763 = 0x2a;
                        D_e764 = 0x2a;
                        D_e766 = 0x20;
                        D_e767 = 0x21;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x25;
                        D_e76a = 0x25;
                        break;
                    case 4:
                        D_e762 = 0x15;
                        D_e763 = 0x0;
                        D_e764 = 0xc;
                        D_e765 = 0x13;
                        D_e766 = 0x8;
                        D_e767 = 0xe;
                        D_e768 = 0x11;
                        D_e769 = 0x2;
                        D_e76a = 0x18;
                        break;
                    case 5:
                        D_e762 = 0x26;
                        D_e763 = 0x28;
                        D_e764 = 0x2e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1c;
                        D_e767 = 0x25;
                        D_e768 = 0x2c;
                        D_e769 = 0x2b;
                        D_e76a = 0x33;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e753 = 0x19;
                        D_e754 = 0x12;
                        D_e755 = 0x0;
                        break;
                    case 1:
                        D_e754 = 0x18;
                        D_e753 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0xa;
                        D_e755 = 0x15;
                        break;
                    case 3:
                        D_e753 = 0x1c;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0xe;
                        D_e754 = 0xe;
                        D_e755 = 0xe;
                        break;
                    case 5:
                        D_e753 = 0x3f;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e759 = 0x2a;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75c = 0x25;
                        D_e75d = 0x18;
                        D_e75e = 0xe;
                        D_e75f = 0x1c;
                        D_e760 = 0x13;
                        D_e761 = 0xa;
                        break;
                    case 1:
                        D_e759 = 0x11;
                        D_e75a = 0x11;
                        D_e75b = 0x11;
                        D_e75c = 0xa;
                        D_e75d = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0x3;
                        D_e760 = 0x3;
                        D_e761 = 0x3;
                        break;
                    case 2:
                        D_e75a = 0x2a;
                        D_e759 = 0x9;
                        D_e75b = 0x9;
                        D_e75d = 0x25;
                        D_e760 = 0x1a;
                        D_e75c = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0xa;
                        D_e761 = 0xa;
                        break;
                    case 3:
                        D_e759 = 0x21;
                        D_e75a = 0x21;
                        D_e75b = 0x21;
                        D_e75c = 0x1c;
                        D_e75d = 0x1c;
                        D_e75e = 0x1c;
                        D_e75f = 0x18;
                        D_e760 = 0x18;
                        D_e761 = 0x18;
                        break;
                    case 4:
                        D_e759 = 0x30;
                        D_e75a = 0x2d;
                        D_e75c = 0x28;
                        D_e75d = 0x28;
                        D_e75e = 0x1c;
                        D_e75b = 0x21;
                        D_e75f = 0x21;
                        D_e760 = 0x21;
                        D_e761 = 0x18;
                        break;
                    case 5:
                        D_e759 = 0x2f;
                        D_e75a = 0x2c;
                        D_e75b = 0x18;
                        D_e75c = 0x2b;
                        D_e75e = 0x15;
                        D_e75d = 0x28;
                        D_e75f = 0x28;
                        D_e760 = 0x25;
                        D_e761 = 0x13;
                        break;
                    }
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;

    case 0x1000:
        sub_s037_1462_0348(0);
        sub_root_0000_13a0();
        break;
    }
}

/* list of the characters saved in charactr.tmp (right-hand column); '+' marks party members */
/* list of the saved characters (charactr.tmp) with '+' for party members */
void far sub_s037_1462_2760(struct font *f)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int i;
    int y;
    int j;
    sub_root_047c_12c3();
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)D_a668, 0x14, &nr);
    sub_root_047c_0553(0x2806, 0xc5, 0x14, 0x73, 0x14, D_27f4, 0xc5, 0x14);
    for (i = 0, y = 0x18; i < n; i++, y += 8) {
        _dos_read(h, (char far *)&D_8b52, 0x80, &nr);
        _dos_read(h, (char far *)D_9085, 0x1aa, &nr);
        sub_root_047c_0553(0x2806, 0xc5, y, 0x73, 0x10, D_27f4, 0xc5, y);
        for (j = 0; j < 5; j++) {
            if (i == ids[j]) {
                f->color = 0x11;
                sub_root_047c_055a(f, 0xc7, y, "+");
                f->color = 0x10;
                sub_root_047c_055a(f, 0xc6, y - 1, "+");
            }
        }
        f->color = 0x11;
        sub_root_047c_055a(f, 0xcd, y - 1, D_8b52.name);
        f->color = 0x10;
        sub_root_047c_055a(f, 0xcc, y - 2, D_8b52.name);
    }
    _dos_close(h);
    if ((D_e7dc = n) <= D_e7da)
        D_e7da = n - 1;
    if (D_e7dc == 0)
        D_e7da = -1;
    else if (D_e7da == -1)
        D_e7da = 0;
    sub_root_047c_04dc(f, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x10, 0x12);
    sub_root_047c_1293();
}

/* the menu entries */
void far sub_s037_1462_2922(struct font *f)
{
    sub_root_047c_12c3();
    f->color = 8;
    sub_root_0265_0004(f, 0x48, 0x1c, F_290e_0000.s[1]);
    sub_root_0265_0004(f, 0x48, 0x2b, F_290e_0000.s[0]);
    sub_root_0265_0004(f, 0x48, 0x3a, F_290e_0000.s[0x40]);
    sub_root_0265_0004(f, 0x48, 0x49, F_290e_0000.s[0x1a]);
    sub_root_0265_0004(f, 0x48, 0x58, F_290e_0000.s[0x3d]);
    sub_root_0265_0004(f, 0x48, 0x67, F_290e_0000.s[0x3e]);
    sub_root_0265_0004(f, 0x48, 0x76, F_290e_0000.s[0x3f]);
    sub_root_0265_0004(f, 0x48, 0x85, F_290e_0000.s[3]);
    sub_root_0265_0004(f, 0x48, 0x94, F_290e_0000.s[0x13]);
    sub_root_0265_0004(f, 0x48, 0xa3, F_290e_0000.s[4]);
    sub_root_0265_0004(f, 0x48, 0xb2, F_290e_0000.s[5]);
    sub_root_047c_1293();
}

/* "Save game": ask for a comment (default "Darklands"), then save */
void far sub_s037_1462_2a68(void)
{
    char comment[0x29] = "Darklands";
    if (D_907a == 1) {
        sub_root_047c_0545();
        D_a65e = sub_root_0617_045c("pics\\textback.pic");
        sub_root_047c_04f8();
    }
    sub_s037_1462_43e6((char far *)comment, (char far *)"Save Game Comment:", 0x17);
    if (comment[0])
        sub_s037_1462_2ace(comment);
}

/* unreferenced zero block in _DATA (0x31 bytes after 2ace's strings; MSC emits a global
   after the literals of the function that follows its definition) */
char unused_42c8[0x30] = {0};

/* save the game (comment c) as saves\dksaveN.sav (first unused N): header with the
   location name, comment and global state, party/character records (charactr.tmp),
   events (events.tmp), locations, then a copy of cache.tmp */
#pragma optimize("a", on)
void far sub_s037_1462_2ace(char *c)
{
    int hcrb;
    int zeroo;
    long zli;
    char palb[0x78];
    char zbc;
    struct find_t ffc;
    int idso[5];
    FILE *fpi;
    int bm;
    int mb;
    int je;
    int nel;
    int vi;
    int ixn;
    int oldcola;
    struct { int x; int y; int w; int ht; } rk;
    int v7o;
    char numk[10];
    int hco;
    FILE *fp2a;
    int nrh;
    char placeb[0x16];
    char *fnfl;
    char pathh[0x1a];
    int hl;
    int kh;
    int nd;
    char far *pa;
    unsigned nlocd;

    zbc = 0;
    zeroo = 0;
    kh = 0;
    zli = 0;
    fnfl = "FNF:%s\n";
    if (D_907a == 1) {
        for (mb = 0; mb < 5; mb++) {
            if (D_9c69[mb][0]) {
                if (D_a53e[mb][0] != -536)
                    vec_1ccf(mb);
                if (D_a53e[mb][1] != -536)
                    vec_1cc5(mb);
                if (D_a53e[mb][2] != -536)
                    vec_1cbb(mb);
            }
        }
        D_906a |= 0x80;
        D_907a = 0;
        sub_root_047c_12c3();
        oldcola = D_017c->color;
        D_017c->color = 9;
        sub_root_0000_23e6(D_017c, -1, 1, saving_game, D_0154, &rk);
        D_017c->color = oldcola;
        sub_root_047c_1293();
        D_907a = 1;
        if (_dos_findfirst("locs.tmp", 0, &ffc)) {
            printf(fnfl, "location");
            exit_(1);
        } else {
            D_a885 = pa = (char far *)((unsigned long)sub_root_05ba_0004(ffc.size, "loc memory") << 16);
            sub_root_0617_011e("locs.tmp", pa);
            je = (unsigned char)*pa++;
            nlocd = (unsigned char)*pa++;
            nlocd = (nlocd << 8) + je;
            for (ixn = 0; ixn < nlocd; ixn++) {
                F_2e38_10be[ixn] = pa;
                pa += 0x3a;
            }
        }
        if (!_dos_findfirst("events.tmp", 0, &ffc)) {
            sub_root_06a1_2c9e("events.tmp", 0, &hl);
            fp2a = fdopen(hl, "rb");
            fread(&nel, 2, 1, fp2a);
            fseek(fp2a, 2L, 0);
            for (ixn = 0; nel > ixn; ixn++) {
                F_2e38_1b2c[ixn] = (char far *)((unsigned long)sub_root_05ba_0004(0x30, 0, 0) << 16);
                _dos_read(hl, F_2e38_1b2c[ixn], 0x30, &je);
            }
            fclose(fp2a);
        }
    } else {
        sub_root_047c_12c3();
        sub_root_0000_23e6(D_017c, -1, 1, saving_game, D_0154, &rk);
        sub_root_047c_1293();
    }
    for (ixn = 0; ixn < 5; ixn++)
        for (je = 0; je < 0x18; je++)
            palb[ixn * 0x18 + je] = F_20a5_33fc[ixn][je];
    sub_root_06a1_2c9e("charactr.tmp", 0, &hcrb);
    _dos_read(hcrb, (int far *)&nd, 2, &nrh);
    _dos_read(hcrb, (int far *)idso, 10, &nrh);
    _dos_read(hcrb, (char far *)D_a668, 0x14, &nrh);
    D_a67e.v = ixn = 0;
    for (; ixn < 5; ixn++)
        if (idso[ixn] != -1)
            D_a67e.v = ixn + 1;
    v7o = 7;
    if (D_907e >= 0 && D_907e < 0x19e)
        _fstrcpy((char far *)placeb, F_2e38_10be[D_907e] + 0x26);
    else
        strcpy(placeb, "Wilderness");
    do {
        strcpy(pathh, "saves\\dksave");
        itoa(kh++, numk, 10);
        strcat(pathh, numk);
        strcat(pathh, ".sav");
    } while (!_dos_findfirst(pathh, 0, &ffc));
    strcpy(D_a680, "dksave");
    itoa(--kh, numk, 10);
    strcat(D_a680, numk);
    strcat(D_a680, ".sav");
    sub_root_06a1_12fa(creat(pathh, 0x80));
    sub_root_06a1_2c9e(pathh, 2, &hl);
    fpi = fdopen(hl, "r+");
    fwrite(placeb, 1, 0x15, fpi);
    fwrite(c, 1, 0x4f, fpi);
    fwrite(&D_9c4a, 2, 1, fpi);
    fwrite(&v7o, 2, 1, fpi);
    fwrite(&D_00e6, 2, 1, fpi);
    fwrite(&D_00e4, 2, 1, fpi);
    fwrite(&D_00e2, 2, 1, fpi);
    fwrite(&D_00e0, 2, 1, fpi);
    fwrite(&D_906b, 2, 1, fpi);
    fwrite(&D_906d, 2, 1, fpi);
    fwrite(&D_906f, 2, 1, fpi);
    fwrite(&zli, 4, 1, fpi);
    fwrite(&D_9082, 2, 1, fpi);
    fwrite(&D_907e, 2, 1, fpi);
    fwrite(&D_e3d0, 2, 1, fpi);
    fwrite(&D_e3d2, 2, 1, fpi);
    fwrite(&D_a772, 2, 1, fpi);
    fwrite(&D_e898, 2, 1, fpi);
    fwrite(&D_e7d8, 2, 1, fpi);
    fwrite(&D_e896, 2, 1, fpi);
    fwrite(&D_a88d, 2, 1, fpi);
    fwrite(&D_9072, 2, 1, fpi);
    fwrite(&D_9074, 2, 1, fpi);
    fwrite(&D_9076, 2, 1, fpi);
    fwrite(&D_9078, 2, 1, fpi);
    fwrite(&D_a67c, 2, 1, fpi);
    fwrite(&D_906a, 1, 1, fpi);
    fwrite(&D_db27, 2, 1, fpi);
    fwrite(&D_da34, 2, 1, fpi);
    fwrite(D_c624, 6, 1, fpi);
    fwrite(&D_907b, 1, 1, fpi);
    fwrite(&D_01bf, 1, 1, fpi);
    fwrite(&D_01be, 1, 1, fpi);
    fwrite(&D_a891, 2, 1, fpi);
    fwrite(&zbc, 1, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&D_a67e, 2, 1, fpi);
    fwrite(&nd, 2, 1, fpi);
    fwrite(idso, 10, 1, fpi);
    fwrite(D_a668, 0x14, 1, fpi);
    fwrite(palb, 0x78, 1, fpi);
    for (ixn = 0; ixn < nd; ixn++) {
        for (je = 0; je < 5; je++)
            if (idso[je] == ixn)
                break;
        if (je < 5) {
            if (D_9c69[je][0]) {
                fseek(fpi, 0L, 1);
                fwrite(D_9c55 + je * 0x80, 0x80, 1, fpi);
                fseek(fpi, 0L, 1);
                _dos_write(hl, F_2e38_082c[je], 0x14, &nrh);
                _dos_write(hl, F_2e38_07be[je], 0x16, &nrh);
                _dos_write(hl, F_2e38_003e[je], 0x180, &nrh);
                _dos_read(hcrb, (char far *)D_9085, 0x22a, &nrh);
            }
        } else {
            fseek(fpi, 0L, 1);
            _dos_read(hcrb, (char far *)D_9085, 0x22a, &nrh);
            _dos_write(hl, (char far *)D_9085, 0x22a, &nrh);
        }
    }
    for (ixn = 0, nel = 0; ixn < 0x12c; ixn++)
        if (F_2e38_1b2c[ixn])
            nel++;
    _dos_write(hl, (int far *)&nel, 2, &nrh);
    for (ixn = 0; ixn < 0x12c; ixn++)
        if (F_2e38_1b2c[ixn])
            _dos_write(hl, F_2e38_1b2c[ixn], 0x30, &nrh);
    vi = 0x19e;
    _dos_write(hl, (int far *)&vi, 2, &nrh);
    for (ixn = 0; ixn < 0x19e; ixn++)
        _dos_write(hl, F_2e38_10be[ixn], 0x3a, &nrh);
    if (_dos_findfirst("cache.tmp", 0, &ffc)) {
        printf("cache");
        exit_(1);
    } else {
        fseek(fpi, 0L, 1);
        sub_root_06a1_2c9e("cache.tmp", 2, &hco);
        fp2a = fdopen(hco, "r+");
        while (fread(&bm, 1, 1, fp2a))
            fwrite(&bm, 1, 1, fpi);
        fclose(fp2a);
    }
    fclose(fpi);
    _dos_close(hcrb);
    if (D_907a == 1) {
        sub_root_05ba_00c2(D_a887, "Maploc");
        sub_root_05ba_00c2(D_a65e, "txtbk");
        for (ixn = 0; ixn < 0x12c; ixn++) {
            if (F_2e38_1b2c[ixn]) {
                sub_root_05ba_00c2(FP_SEG(F_2e38_1b2c[ixn]), "event");
                F_2e38_1b2c[ixn] = 0;
            }
        }
    }
    vec_1b21(D_a680);
    sub_root_047c_12c3();
    if (D_907a == 1) {
        D_906a &= 0x7f;
        D_907a = 0;
        sub_root_03ec_000e(D_0154, rk.x, rk.y, rk.w, rk.ht, D_017c, rk.x, rk.y);
        D_907a = 1;
    } else
        sub_root_03ec_000e(D_0154, rk.x, rk.y, rk.w, rk.ht, D_017c, rk.x, rk.y);
    sub_root_047c_1293();
}

#pragma optimize("", on)
/* append the character being created (D_e80a, 0x80 bytes, plus its saints/virtues/items)
   to charactr.tmp */
void far sub_s037_1462_38aa(void)
{
    int n;
    int nr;
    int h;
    FILE *fp;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    fseek(fp, 0L, 0);
    n++;
    _dos_write(h, (int far *)&n, 2, &nr);
    fseek(fp, (long)(unsigned)(n * 0x22a - 0x20a), 0);
    D_e80a = D_907e;
    _dos_write(h, (char far *)&D_e80a, 0x80, &nr);
    _dos_write(h, (char far *)F_2e38_3b1a, 0x14, &nr);
    _dos_write(h, (char far *)F_2e38_3b04, 0x16, &nr);
    _dos_write(h, (char far *)F_2e38_3984, 0x180, &nr);
    fclose(fp);
}

/* write party member p back to its record in charactr.tmp */
void far sub_s037_1462_3992(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    FILE *fp;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    fseek(fp, (long)(unsigned)(p * 0x22a + 0x20), 0);
    D_e80a = D_907e;
    _dos_write(h, (char far *)(D_9c55 + ids[p] * 0x80), 0x80, &nr);
    _dos_write(h, F_2e38_082c[p], 0x14, &nr);
    _dos_write(h, F_2e38_07be[p], 0x16, &nr);
    _dos_write(h, F_2e38_003e[p], 0x180, &nr);
    fclose(fp);
}

/* character index in party slot p */
int far sub_s037_1462_3a8a(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    return ids[p];
}

/* party slot of character c, -1 if not in the party */
int far sub_s037_1462_3ae2(int c)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int i;
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    for (i = 0; i < 5; i++)
        if (ids[i] == c)
            break;
    if (i >= 5)
        return -1;
    return i;
}

/* remove party member p (record written back to charactr.tmp) */
void far sub_s037_1462_3b58(int p)
{
    int n;
    int i;
    int nr;
    int ids[5];
    int h;
    int c;
    FILE *fp;
    if (D_907b == p) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] == 1 && p != i) {
                D_907b = i;
                break;
            }
    }
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)D_a668, 0x14, &nr);
    fseek(fp, 0L, 0);
    _dos_write(h, (int far *)&n, 2, &nr);
    c = ids[p];
    ids[p] = -1;
    _dos_write(h, (int far *)ids, 10, &nr);
    if (D_9c69[p][0] != 5)
        D_9c69[p][0] = 0;
    *(int *)(D_9c55 + p * 0x80) = D_907e;
    fseek(fp, (long)(unsigned)(c * 0x22a + 0x20), 0);
    _dos_write(h, (char far *)(D_9c55 + p * 0x80), 0x80, &nr);
    fclose(fp);
    D_9c69[p][0] = 0;
    _dos_freemem(D_a2d7[p]);
    D_a2d7[p] = 0;
    D_a67e.v = i = 0;
    for (; i < 5; i++)
        if (D_9c69[i][0])
            D_a67e.v = i + 1;
}

/* add character c to the party (first free slot); returns the slot or -1 */
int far sub_s037_1462_3cd6(int c)
{
    char name[30];
    int ids[5];
    int i;
    int nr;
    int h;
    FILE *fp;
    int slot;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&i, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)D_a668, 0x14, &nr);
    for (slot = 0; slot < 5; slot++)
        if (ids[slot] == -1)
            break;
    if (slot >= 5) {
        fclose(fp);
        return -1;
    }
    ids[slot] = c;
    fseek(fp, (long)(unsigned)(c * 0x22a), 1);
    fread(D_9c55 + slot * 0x80, 0x80, 1, fp);
    fseek(fp, 0L, 1);
    _dos_read(h, F_2e38_082c[slot], 0x14, &nr);
    _dos_read(h, F_2e38_07be[slot], 0x16, &nr);
    _dos_read(h, F_2e38_003e[slot], 0x180, &nr);
    D_9c69[slot][0] = 1;
    sub_root_047c_0545();
    strcpy(name, "pics\\");
    strcat(name, "shield");
    name[11] = D_9c69[slot][1];
    name[12] = 0;
    strcat(name, ".pic");
    D_a2d7[slot] = sub_root_0617_045c(name);
    sub_root_047c_04f8();
    sub_root_0000_1588(slot);
    sub_root_0000_1716(slot);
    fseek(fp, 2L, 0);
    _dos_write(h, (int far *)ids, 10, &nr);
    _dos_write(h, (char far *)D_a668, 0x14, &nr);
    fclose(fp);
    D_a67e.v = i = 0;
    for (; i < 5; i++)
        if (D_9c69[i][0])
            D_a67e.v = i + 1;
    return slot;
}

/* delete party member p and its character (events 0x43/0x47 of it are cancelled) */
void far sub_s037_1462_3f30(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int e;
    int i;
    if ((e = sub_s029_0e76_3b62(0x43, p, -1, -1, -1, -1)) != -1)
        sub_s029_0e76_30fe(e);
    if ((e = sub_s029_0e76_3b62(0x47, p, -1, -1, -1, -1)) != -1)
        sub_s029_0e76_30fe(e);
    D_9c69[p][0] = 0;
    if (D_907b == p) {
        for (i = 0; i < 5; i++)
            if (D_9c69[i][0] == 1 && i != p) {
                D_907b = i;
                break;
            }
    }
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    sub_s037_1462_4048(ids[p]);
    if (D_a2d7[p])
        _dos_freemem(D_a2d7[p]);
    D_a2d7[p] = 0;
    D_a67e.v = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0])
            D_a67e.v = i + 1;
}

/* delete character c from charactr.tmp (later records move down) */
void far sub_s037_1462_4048(int c)
{
    int ids[5];
    int n;
    int nr;
    int h;
    FILE *fp;
    int i;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    fseek(fp, 0L, 0);
    n--;
    _dos_write(h, (int far *)&n, 2, &nr);
    for (i = 0; i < 5; i++) {
        if (ids[i] == c)
            ids[i] = -1;
        else if (ids[i] > c)
            ids[i]--;
    }
    _dos_write(h, (int far *)ids, 10, &nr);
    for (i = c; i < n; i++) {
        fseek(fp, (long)(unsigned)(i * 0x22a + 0x24a), 0);
        _dos_read(h, (char far *)D_9085, 0x22a, &nr);
        fseek(fp, (long)(unsigned)(i * 0x22a + 0x20), 0);
        _dos_write(h, (char far *)D_9085, 0x22a, &nr);
    }
    fclose(fp);
}

/* load saved game `name` (saves\name): characters to charactr.tmp, locations to locs.tmp,
   empty cache.tmp, no events.tmp */
void far sub_s037_1462_4184(char far *name)
{
    char comment[0x78];
    char path[26];
    int ids[5];
    int n;
    int v;
    int hs;
    int hc;
    int nr;
    int i;
    _dos_creat("charactr.tmp", 0, &hc);
    strcpy(path, "saves\\");
    _fstrcat((char far *)path, name);
    sub_root_06a1_2c9e(path, 0, &hs);
    _dos_read(hs, (char far *)D_9085, 0xef, &nr);
    _dos_read(hs, (int far *)&D_a67e, 2, &nr);
    _dos_read(hs, (int far *)&n, 2, &nr);
    _dos_read(hs, (int far *)ids, 10, &nr);
    _dos_read(hs, (char far *)D_a668, 0x14, &nr);
    _dos_read(hs, (char far *)comment, 0x78, &nr);
    _dos_write(hc, (int far *)&n, 2, &nr);
    _dos_write(hc, (int far *)ids, 10, &nr);
    _dos_write(hc, (char far *)D_a668, 0x14, &nr);
    for (i = 0; i < n; i++) {
        _dos_read(hs, (char far *)D_9085, 0x22a, &nr);
        _dos_write(hc, (char far *)D_9085, 0x22a, &nr);
    }
    _dos_close(hs);
    _dos_close(hc);
    D_a67e.v = 0;
    for (i = 0; i < 5; i++)
        if (D_9c69[i][0])
            D_a67e.v = i + 1;
    _dos_creat("locs.tmp", 0, &hc);
    v = 0x19e;
    _dos_write(hc, (int far *)&v, 2, &nr);
    for (i = 0; i < 0x19e; i++)
        _dos_write(hc, F_2e38_10be[i], 0x3a, &nr);
    _dos_close(hc);
    _dos_creat("cache.tmp", 0, &hc);
    v = 0x63;
    _dos_write(hc, (int far *)&v, 1, &nr);
    v = 0;
    _dos_write(hc, (int far *)&v, 1, &nr);
    v = 0;
    for (i = 0; i < 0x62; i++)
        _dos_write(hc, (int far *)&v, 2, &nr);
    _dos_close(hc);
    remove("events.tmp");
}

/* text input box: edit buf (at most max characters) under the prompt; Esc clears it */
void far sub_s037_1462_43e6(char far *buf, char far *prompt, int max)
{
    int save;
    int k;
    int i;
    D_2800 = 0;
    sub_root_047c_12c3();
    for (i = 0; i < 0x28; i++)
        if (buf[i] == 0)
            break;
    sub_root_047c_04dc(D_27f4, 0x6e, 0x50, 0x9b, 0x32, 0, 5);
    sub_root_047c_0545();
    save = sub_root_047c_04d5(0, 0x6e, 0x50, 0xa0, 0x32);
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_27f4, 0x6e, 0x50, D_a65e);
    sub_root_0265_0004(D_27f4, 0x7d, 0x5a, prompt);
draw:
    sub_root_0265_0004(D_27f4, 0x87, 0x64, buf);
    for (;;) {
        while (!sub_root_047c_073d())
            ;
        k = sub_root_047c_0744();
        if (k == 0x1c0d)
            break;
        if (k == 0x11b) {
            buf[0] = 0;
            break;
        }
        if (k == 0x4b00 || k == 0xe08) {
            if (i > 0)
                i--;
            buf[i] = 0;
        }
        if (max > i) {
            if ((k & 0xff) >= 0x20 && (k & 0xff) <= 0x7e) {
                buf[i] = k;
                i++;
                buf[i] = 0;
            }
            sub_root_047c_05a5(D_27f4, 0x6e, 0x50, D_a65e);
            sub_root_0265_0004(D_27f4, 0x7d, 0x5a, prompt);
            goto draw;
        }
    }
    buf[i] = 0;
    sub_root_047c_05a5(D_27f4, 0x6e, 0x50, save);
    sub_root_047c_04dc(D_27f4, 0x6e, 0x50, 0x9b, 0x32, 5, 0);
    sub_root_047c_1293();
    sub_root_05ba_00c2(save, "B");
    D_2800 = 8;
}

/* list the shield codes from pics\F??stat.PIC, A??, C?? into D_e7e2[] (D_e7de entries) */
void far sub_s037_1462_4562(void)
{
    struct find_t ff;
    *(volatile int *)&D_e7de = 0;
    if (!_dos_findfirst("pics\\F??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (!_dos_findfirst("pics\\A??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (!_dos_findfirst("pics\\C??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (D_e7e0 >= D_e7de)
        D_e7e0 = 0;
}
