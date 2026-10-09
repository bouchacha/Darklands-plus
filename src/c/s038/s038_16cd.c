/* place: s038 16cd:000e */
/* orig: size=05ca _DATA=4b5a+003e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Hall of fame (darkland.fam): 00b0 shows it, 0404 merges the party's score. */
/* darkland.fam: 5 records of 0x6a bytes (0x212 bytes total) */

struct res { int a; int b; };
struct fame {
    int score;          /* +0  party fame score (D_9082.v) */
    int unk2;           /* +2 */
    int seed;           /* +4  party seed (D_9c4a) */
    char name[4][25];   /* +6  member names (member record +0x11) */
};
/* party fame score; must be a struct member (not a plain int) or 6.00A flips the compares */
extern struct { int v; } D_9082;
extern char D_27f4[];
extern int D_9c4a;
extern char D_9c69[][0x80];
char *strcat();
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
int far itoa();
char *strcpy(char *, const char *);
int far sub_root_047c_055a();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_1320();
int far sub_root_0617_024a();
void far sub_s038_16cd_000e();
int far sub_s038_16cd_005e();
int far sub_s038_16cd_0084();
void far sub_s038_16cd_00b0();
void far sub_s038_16cd_0404();

void far sub_s038_16cd_000e(void)
{
    sub_root_047c_0568();
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_s038_16cd_0084();
    sub_s038_16cd_00b0();
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
    sub_s038_16cd_005e();
}

int far sub_s038_16cd_005e(void)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s038_16cd_0084(void)
{
    sub_root_0617_024a(1, "pics\\hallfame.pic", D_e48c);
    sub_root_047c_058b(D_e48c);
    D_2812 = 0;
    D_2816 = 1;
    return D_2804 = 1;
}

void far sub_s038_16cd_00b0(void)
{
    struct fame tab[5];
    char line[120];
    int y;
    int i;
    int fp;
    fread(tab, 0x212, 1, fp = fopen("darkland.fam", "rb"));
    fclose(fp);
    D_2812 = 0;
    for (y = 0x3c, i = 0; i < 4; y += 0x18, i++) {
        itoa(tab[i].score, line, 10);
        sub_root_047c_055a(0x2806, 0x28, y, line);
        line[0] = 0;
        if (tab[i].name[0][0]) {
            strcat(line, tab[i].name[0]);
            strcat(line, ",");
        }
        if (tab[i].name[1][0]) {
            strcat(line, tab[i].name[1]);
            strcat(line, ",");
        }
        if (tab[i].name[2][0]) {
            strcat(line, tab[i].name[2]);
            strcat(line, ",");
        }
        if (tab[i].name[3][0]) {
            strcat(line, tab[i].name[3]);
        }
        sub_root_047c_055a(0x2806, 0x78, y - 10, line);
    }
    itoa(D_9082.v, line, 10);
    sub_root_047c_055a(0x2806, 0x28, 0x88, line);
    if (D_9c69[0][0]) {
        strcat(line, &D_9c69[0][0x11]);
        strcat(line, ",");
    }
    if (D_9c69[1][0]) {
        strcat(line, &D_9c69[1][0x11]);
        strcat(line, ",");
    }
    if (D_9c69[2][0]) {
        strcat(line, &D_9c69[2][0x11]);
        strcat(line, ",");
    }
    if (D_9c69[3][0]) {
        strcat(line, &D_9c69[3][0x11]);
    }
    sub_root_047c_055a(0x2806, 0x78, 0x7e, line);
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_root_047c_1293();
}

/* merges the party into the hall of fame table, but never writes it back (original bug) */
void far sub_s038_16cd_0404(void)
{
    struct fame out[5];
    struct fame tab[5];
    struct fame cur;
    int fp;
    int n;
    fp = fopen("darkland.fam", "rb");
    fread(tab, 0x212, 1, fp);
    cur.score = D_9082.v;
    cur.seed = D_9c4a;
    if (D_9c69[0][0]) strcpy(cur.name[0], &D_9c69[0][0x11]); else cur.name[0][0] = 0;
    if (D_9c69[1][0]) strcpy(cur.name[1], &D_9c69[1][0x11]); else cur.name[0][1] = 0;
    if (D_9c69[2][0]) strcpy(cur.name[2], &D_9c69[2][0x11]); else cur.name[0][2] = 0;
    if (D_9c69[3][0]) strcpy(cur.name[3], &D_9c69[3][0x11]); else cur.name[0][3] = 0;
    n = 0;
    if (tab[3].score < D_9082.v) {
        if (tab[0].score > D_9082.v) out[n++] = tab[0]; else out[n++] = cur;
        if (tab[1].score > D_9082.v) out[n++] = tab[1]; else out[n++] = cur;
        if (tab[2].score > D_9082.v) out[n++] = tab[2]; else out[n++] = cur;
        if (tab[3].score > D_9082.v) out[n++] = tab[3]; else out[n++] = cur;
    }
    fclose(fp);
}
