/* place: s041 1462:0000 */
/* orig: size=05cc CONST=7ee0+0002 _DATA=4dee+0066 */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* Copy-protection screen: "SELECT THE SYMBOL FOR <substance>" (overlay s041). */

extern int D_2816, D_2804, D_2812;
extern int D_e7a2, D_e7a0;
extern char D_e48c[];
extern char D_2806[];
extern char D_27f4[];
extern int D_8dd4, D_8dd6, D_8dd8, D_8dda;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_907d;
extern int D_940b;
extern char far * far F_290e_1d73[];

char *_itoa(int, char *, int);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);

int far sub_root_0000_0c70(int);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_047c_0545(void);
int far sub_root_0617_045c(char *);
void far sub_root_047c_04f8(void);
void far sub_root_047c_05a5(char *, int, int, int);
void far sub_root_047c_055a(char *, int, int, char *);
void far sub_root_0265_0004(char *, int, int, char far *);
void far sub_root_047c_0568(void);
void far sub_root_047c_058b(char *);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_047c_12c3(void);
void far sub_root_047c_0553(char *, int, int, int, int, char *, int, int);
void far sub_root_047c_1293(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_1320(int, int, int, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_05ba_00c2(int, char *);
void far sub_root_0000_13a0(void);
void far sub_root_044a_01f4(void);
void far sub_root_047c_0791(void);
void far vec_186f(void);
void far sub_root_0000_1354(void);
void far sub_root_05f8_0139(int);
void far vec_1865(void);
void far exit_(int);

int far copyprot_close(int pic);
void far copyprot_key(int key, int which);
void far copyprot_fail(void);

/* Show four random alchemical symbols and ask for the one matching a named substance.
   Clicking the right picture (or pressing its number key) lets the game continue. */
void far copy_protection(void)
{
    char path[20];
    char num[8];
    int which;
    int sym0, sym1, sym2, sym3;
    int target;
    int key;

    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    which = sub_root_0000_0c70(4);
    sym1 = sub_root_0000_0c70(20);
    sym0 = sub_root_0000_0c70(7);
    while (sym0 == sym1) {
        sym0++;
        sym0 %= 7;
    }
    sym2 = sub_root_0000_0c70(7) + 7;
    while (sym2 == sym1) {
        sym2++;
        if (sym2 >= 14)
            sym2 = 7;
    }
    sym3 = sub_root_0000_0c70(6) + 14;
    while (sym3 == sym1) {
        sym3++;
        if (sym3 >= 20)
            sym3 = 14;
    }
    switch (which) {
    case 0: target = sym0; break;
    case 1: target = sym1; break;
    case 2: target = sym2; break;
    default: target = sym3; break;
    }
    D_2812 = 14;
    D_2816 = 1;
    D_2804 = 1;
    sub_root_0617_024a(1, "pics\\bkgdalcm.pic", D_e48c);
    sub_root_047c_0545();
    strcpy(path, "pics\\copy");
    _itoa(sym0, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd4 = sub_root_0617_045c(path);
    strcpy(path, "pics\\copy");
    _itoa(sym1, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd6 = sub_root_0617_045c(path);
    strcpy(path, "pics\\copy");
    _itoa(sym2, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd8 = sub_root_0617_045c(path);
    strcpy(path, "pics\\copy");
    _itoa(sym3, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dda = sub_root_0617_045c(path);
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_2806, 0x68, 0x3f, D_8dd4);
    sub_root_047c_05a5(D_2806, 0x68, 0x72, D_8dd6);
    sub_root_047c_05a5(D_2806, 0xde, 0x3f, D_8dd8);
    sub_root_047c_05a5(D_2806, 0xde, 0x72, D_8dda);
    sub_root_047c_055a(D_2806, 0x50, 0x14, "SELECT THE SYMBOL FOR");
    sub_root_0265_0004(D_2806, 0x82, 0x1e, F_290e_1d73[target]);
    sub_root_047c_0568();
    sub_root_047c_058b(D_e48c);
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
    sub_root_047c_12c3();
    sub_root_047c_0553(D_2806, 0x3c, 0, 0x104, 0xc8, D_27f4, 0x3c, 0);
    sub_root_047c_1293();
    sub_root_047c_056f();
    sub_root_047c_1320(6, 0, 0, 0);
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if ((D_6ef0 & 1) && D_6eec > 100 && D_6eec < 200 && D_6eee > 60 && D_6eee < 100) {
            sym3 = 0;
            while (D_6ef0 & 1)
                ;
            if (which == 0)
                break;
            copyprot_fail();
        } else if ((D_6ef0 & 1) && D_6eec > 220 && D_6eec < 300 && D_6eee > 60 && D_6eee < 100) {
            sym3 = 0;
            while (D_6ef0 & 1)
                ;
            if (which == 2)
                break;
            copyprot_fail();
        } else if ((D_6ef0 & 1) && D_6eec > 100 && D_6eec < 200 && D_6eee > 110 && D_6eee < 180) {
            sym3 = 0;
            while (D_6ef0 & 1)
                ;
            if (which == 1)
                break;
            copyprot_fail();
        } else if ((D_6ef0 & 1) && D_6eec > 220 && D_6eec < 300 && D_6eee > 110 && D_6eee < 180) {
            sym3 = 0;
            while (D_6ef0 & 1)
                ;
            if (which == 3)
                break;
            copyprot_fail();
        }
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if (key) {
            copyprot_key(key, which);
            break;
        }
    }
    copyprot_close(D_8dd4);
}

/* Free the screen and restore the saved display state. */
int far copyprot_close(int pic)
{
    sub_root_05ba_00c2(pic, "b");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    while (D_6ef0 & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

/* Number keys 1-4 (top row or keypad) choose a picture; Alt-Q quits. */
void far copyprot_key(int key, int which)
{
    switch (key) {
    case 0x231: case 0x4f31:
        if (which != 0)
            copyprot_fail();
        break;
    case 0x332: case 0x5032:
        if (which != 1)
            copyprot_fail();
        break;
    case 0x433: case 0x5133:
        if (which != 2)
            copyprot_fail();
        break;
    case 0x534: case 0x4b34:
        if (which != 3)
            copyprot_fail();
        break;
    case 0x635: case 0x4c35:
        break;
    case 0x1000:
        copyprot_close(D_8dd4);
        sub_root_0000_13a0();
    case 0x4800: case 0x4b00: case 0x4d00: case 0x5000:
        copyprot_fail();
        break;
    default:
        copyprot_fail();
        break;
    }
}

/* Wrong answer: shut everything down and exit to DOS. */
void far copyprot_fail(void)
{
    sub_root_044a_01f4();
    sub_root_047c_0791();
    vec_186f();
    sub_root_0000_1354();
    if (D_907d == 1)
        sub_root_05f8_0139(D_940b);
    vec_1865();
    exit_(0);
}
