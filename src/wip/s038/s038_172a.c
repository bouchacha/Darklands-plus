/* place: s038 172a:0008 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Fame screen: shows pics\fame.pic, reads darkland.fam (0x208 bytes) */

struct res { int a; int b; };
extern char D_27f4[];
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern char D_6ef0;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
int far fclose();
int far fopen();
int far fread();
int far sub_root_047c_0553();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_1293();
int far sub_root_047c_12c3();
int far sub_root_047c_1320();
int far sub_root_0617_024a();
void far sub_s038_172a_0008();
int far sub_s038_172a_0058();
int far sub_s038_172a_007e();
void far sub_s038_172a_00aa();

void far sub_s038_172a_0008(void)
{
    sub_root_047c_0568();
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_s038_172a_007e();
    sub_s038_172a_00aa();
    sub_root_047c_056f();
    do {
    } while (D_6ef0 & 1);
    do {
    } while (!(D_6ef0 & 3) && !sub_root_047c_073d());
    do {
    } while (D_6ef0 & 1);
    if (sub_root_047c_073d()) {
        sub_root_047c_0744();
    }
    sub_s038_172a_0058();
}

int far sub_s038_172a_0058(void)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s038_172a_007e(void)
{
    sub_root_0617_024a(1, "pics\\fame.pic", D_e48c);
    sub_root_047c_058b(D_e48c);
    D_2812 = 0;
    D_2816 = 1;
    return D_2804 = 1;
}

void far sub_s038_172a_00aa(void)
{
    char buf[0x208];
    int fp;
    fread(buf, 0x208, 1, fp = fopen("darkland.fam", "rb"));
    fclose(fp);
    D_2812 = 0;
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_root_047c_1293();
}
