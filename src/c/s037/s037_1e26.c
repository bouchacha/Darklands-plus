/* place: s037 1e26:0008 */
/* orig: size=03f0 _DATA=4962+00a3 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Start screen (pics\startscr.pic, 4 buttons): quit / default party / load / create. */

struct res { int a; int b; };
extern char D_9c69[][0x80];     /* party member records, 128 bytes */
extern int D_9c4a;              /* party random seed */
extern char D_9e69;
extern int D_a2d7[];            /* shield picture handles, one per member */
extern int D_a2df;
extern int D_a67c;
extern int D_a67e;
extern char D_a668[];
extern char D_e89a;
int far sub_root_0000_13a0();
int far sub_root_06a1_2c9e();   /* _dos_open */
int far _dos_read();
int far _dos_write();
int far _dos_close();
int far sub_s037_1462_4184(char far *);
int far sub_s037_1462_0000();
int far sub_s037_1c7d_0002();
int far vec_1adb();
char *strcpy();
char *strcat();
extern char D_27f4[];
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern int D_8cfe;
extern int D_8d00;
extern int D_8d02;
extern int D_8d04;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
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
void far sub_s037_1e26_0008();
int far sub_s037_1e26_00a6();
int far sub_s037_1e26_00dc();
void far sub_s037_1e26_014a();
int far sub_s037_1e26_0172();

void far sub_s037_1e26_0008(void)
{
    int v_si;
    sub_root_047c_0568();
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_s037_1e26_00dc();
    sub_s037_1e26_014a();
    sub_root_047c_056f();
    do {
    } while (D_6ef0 & 1);
    do {
        v_si = 0;
        if (D_6ef0 & 1 && D_6eee > 0x32 && D_6eee < 0x96) {
            do {
            } while (D_6ef0 & 1);
            switch (v_si = (D_6eee - 0x32) / 0x20) {
            default:
                v_si = 0;
                break;
            case 0:
                v_si = 0x1051;
                break;
            case 1:
                v_si = 0x2e43;
                break;
            case 2:
                v_si = 0x1454;
            }
        }
        if (!sub_root_047c_073d()) {
            continue;
        }
        v_si = sub_root_047c_0744();
    } while (!v_si || sub_s037_1e26_0172(v_si) != 0x11b);
    sub_s037_1e26_00a6();
}

int far sub_s037_1e26_00a6(void)
{
    sub_root_05ba_00c2(D_8cfe, "b");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s037_1e26_00dc(void)
{
    sub_root_0617_024a(1, "pics\\startscr.pic", D_e48c);
    sub_root_047c_0545();
    D_8cfe = sub_root_0617_045c("pics\\button1.pic");
    D_8d00 = sub_root_0617_045c("pics\\button2.pic");
    D_8d02 = sub_root_0617_045c("pics\\button3.pic");
    D_8d04 = sub_root_0617_045c("pics\\button4.pic");
    sub_root_047c_04f8();
    sub_root_047c_058b(D_e48c);
    D_2812 = 8;
    D_2816 = 1;
    return D_2804 = 1;
}

void far sub_s037_1e26_014a(void)
{
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_root_047c_1293();
}

/* start screen buttons: 0x1000 quit, 0x1454 load a saved game, 0x1051 default party,
   0x2e43 create characters. Returns 0x11b when the game should start. */
int far sub_s037_1e26_0172(int key)
{
    int i;
    int h;
    int nr;
    int ids[5];
    char name[20];
    switch (key) {
    case 0x1000:
        sub_s037_1e26_00a6();
        sub_root_0000_13a0();
        break;
    case 0x1454:
    case 0x1474:
        sub_root_047c_12c3();
        sub_root_047c_05a5(D_27f4, 0x51, 0x78, D_8d02);
        sub_root_047c_1293();
        D_e89a = 1;
        if (sub_s037_1c7d_0002()) {
            D_e89a = 0;
            return 0x11b;
        }
        D_e89a = 0;
        sub_root_047c_0568();
        sub_root_0617_024a(1, "pics\\startscr.pic", D_e48c);
        sub_root_047c_058b(D_e48c);
        sub_root_047c_12c3();
        sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
        sub_root_047c_1293();
        sub_root_047c_056f();
        break;
    case 0x1051:
    case 0x1071:
        sub_root_047c_12c3();
        sub_root_047c_05a5(D_27f4, 0x51, 0x37, D_8cfe);
        sub_root_047c_1293();
        D_9c4a = *(int far *)0x0040006cL;
        sub_s037_1462_4184("default");
        vec_1adb();
        D_9e69 = 0;
        for (i = 0; i < 4; i++) {
            sub_root_047c_0545();
            strcpy(name, "pics\\");
            strcat(name, "shield");
            name[11] = D_9c69[i][1];
            name[12] = 0;
            strcat(name, ".pic");
            D_a2d7[i] = sub_root_0617_045c(name);
            sub_root_047c_04f8();
        }
        *(volatile int *)&D_a2df = 0;
        D_a67c = 0;
        D_a67e = 4;
        sub_root_06a1_2c9e("charactr.tmp", 2, &h);
        _dos_read(h, (int far *)&i, 2, &nr);
        for (i = 0; i < 4; i++)
            ids[i] = i;
        ids[4] = -1;
        _dos_write(h, (int far *)ids, 10, &nr);
        _dos_write(h, (char far *)D_a668, 0x14, &nr);
        _dos_close(h);
        return 0x11b;
    case 0x2e43:
    case 0x2e63:
        sub_root_047c_12c3();
        sub_root_047c_05a5(D_27f4, 0x51, 0x57, D_8d00);
        sub_root_047c_1293();
        D_9c4a = *(int far *)0x0040006cL;
        if (sub_s037_1462_0000())
            return 0x11b;
        sub_root_047c_0568();
        sub_root_0617_024a(1, "pics\\startscr.pic", D_e48c);
        sub_root_047c_058b(D_e48c);
        sub_root_047c_12c3();
        sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
        sub_root_047c_1293();
        sub_root_047c_056f();
        break;
    }
}
