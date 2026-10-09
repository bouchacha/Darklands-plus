/* place: s021 1ef8:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Combat overlay s021 (child of s014): skill improvement in combat and the
 * enemy information display.
 * 0000 (member, skill, mode): chance (100-v)/2 (mode 1) or /4 to raise a skill below 99,
 *      by f(D_22d2[difficulty]) + D_22ce[difficulty];
 * 0080 (member, skill, amount): "<skill> changed from X to Y" message, clamp 0..99;
 * 01e2 (row, color): show the D_9085 message box at a combatant row (-2 = whole panel);
 * 02ce: enemy info: up to two distinct enemy kinds among combatants 5..12;
 * 03fe (combatant, &pic, x): portrait "<a><b>st.pic" from einfo.cat, name, armour / weapon
 *      names and skill words, or "Natrl Armr"/"Natrl Weapn" for animals (pic name not 'E').
 * Combatant record (0x80 bytes, D_9c69[13]): +0 status, +2 kind, +0x0e weapon (-1 none),
 * +0x34 weapon quality flag, +0x37/+0x38 armour, +0x3d weapon/shield, +0x3f natural armour,
 * +0x41 armour flags, +0x48, +0x57 skills (+0x57 first, +0x5c sixth). */

struct pr { char kind; char idx; };
struct gfx { char pad[0xa]; int f0a; int f0c; char pad2[2]; int f10; };
struct mem {
    char status;   /* +00 */
    char pad1[1];
    char kind;   /* +02 */
    char pad3[11];
    char f0e;   /* +0e */
    char padf[37];
    char f34;   /* +34 */
    char pad35[2];
    char f37;   /* +37 */
    char f38;   /* +38 */
    char pad39[4];
    char f3d;   /* +3d */
    char pad3e[1];
    char f3f;   /* +3f */
    char pad40[1];
    char f41;   /* +41 */
    char pad42[6];
    char f48;   /* +48 */
    char pad49[14];
    char skill[0x29];   /* +57 */
};

char D_22ce[4] = { 0, 0, 0, 0 };
char D_22d2[3] = { 3, 4, 5 };

extern struct mem D_9c69[];
extern unsigned char D_906a;
extern char D_907b;
extern char D_9085[];
extern struct gfx *D_0104;
extern int D_0118;
extern int D_a626, D_a628;
extern char D_01bc;
extern volatile int D_6ef0;
extern int D_9419;
extern char *D_1166[], *D_1174[];
extern char *D_4fce[];
extern char D_c2bf[];
extern char far F_2e38_0bd8[][0x14];
extern char far * far F_290e_2137[];
extern char far * far F_286b_01f7[];
extern char far * far F_286b_01cf[];
extern char far * far F_286b_0277[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
char *_itoa();
void *memset(void *, int, unsigned);
char far *_fstrcpy(char far *, const char far *);
int far random_n(int);
int far sub_root_0000_0c8c(int);
int far sub_root_0410_001a(int, int, int);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_05a5(struct gfx *, int, int, int);
void far sub_root_047c_0553();
int far sub_root_047c_051b(int);
void far sub_root_0265_0134();
int far sub_root_047c_073d(void);
void far sub_root_0000_0000(int *, int);
void far sub_root_0000_1384(void);
void far free_buffer(int, char *);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_0000_25f2(char *);
void far sub_root_047c_0545(void);
int far sub_root_0617_045c(char *);
void far sub_root_047c_04f8(void);
unsigned far sub_root_0265_084e(struct gfx *, char *);
void far sub_root_0265_006e(struct gfx *, int, int, char *);
void far sub_s021_1ef8_0000(int, int, int);
void far sub_s021_1ef8_0080(int, int, int);
void far sub_s021_1ef8_01e2(int, int);
void far sub_s021_1ef8_02ce(void);
void far sub_s021_1ef8_03fe(int, int *, int);

void far sub_s021_1ef8_0000(int m, int sk, int mode)
{
    int v, chance;

    v = D_9c69[m].skill[sk];
    if (mode == 1)
        chance = (100 - v) / 2;
    else
        chance = (100 - v) / 4;
    if (v < 99 && random_n(100) <= chance)
        sub_s021_1ef8_0080(m, sk, sub_root_0000_0c8c(D_22d2[D_906a]) + D_22ce[D_906a]);
}

void far sub_s021_1ef8_0080(int m, int sk, int amount)
{
    char buf[0x32];
    char num[8];

    if (amount == 0)
        return;
    if (m == -1)
        m = D_907b;
    _fstrcpy(buf, F_290e_2137[sk]);
    strcat(buf, " changed from ");
    strcat(buf, _itoa(D_9c69[m].skill[sk], num, 10));
    D_9c69[m].skill[sk] = sub_root_0410_001a(D_9c69[m].skill[sk] += amount, 0, 99);
    strcat(buf, " to ");
    strcat(buf, _itoa(D_9c69[m].skill[sk], num, 10));
    strcpy(D_9085, buf);
    if (amount > 0)
        sub_s021_1ef8_01e2(m, 15);
    else
        sub_s021_1ef8_01e2(m, 12);
}

#pragma optimize("a", on)
void far sub_s021_1ef8_01e2(int row, int color)
{
    int y;

    D_0104->f0c = color;
    D_0104->f0a = 0;
    sub_root_047c_12c3();
    if (row == -2) {
        for (y = 0; y < 0xc6; y += 0xb)
            sub_root_047c_05a5((struct gfx *)D_0118, 3, y, D_a628);
        sub_root_047c_0553(D_0118, 3, 1, 0x36, 0xc6, D_0104, 3, 1);
        y = 0x51;
    } else {
        y = row * 40 + 1;
        sub_root_047c_05a5(D_0104, 3, y, D_a626);
    }
    D_01bc = 1;
    sub_root_0265_0134(D_0104, D_9085, 0x37, 3, y + 4, sub_root_047c_051b(D_0104->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
    sub_root_047c_1293();
    while (!sub_root_047c_073d() && !(D_6ef0 & 1))
        sub_root_0000_0000(&D_9419, 0);
    while (D_6ef0 & 1)
        ;
}

#pragma optimize("", on)
void far sub_s021_1ef8_02ce(void)
{
    int pics[3];
    struct pr list[2];
    int cnt, i, k, y, kind;

    memset(list, -1, 4);
    pics[2] = pics[1] = pics[0] = 0;
    cnt = 0;
    for (i = 5; i < 13; i++) {
        if (D_9c69[i].status != 0 && (kind = D_9c69[i].kind) != list[0].kind && list[1].kind != kind) {
            list[cnt].kind = kind;
            list[cnt].idx = i;
            cnt++;
            if (cnt == 2)
                break;
        }
    }
    if (cnt > 0) {
        sub_root_047c_12c3();
        for (y = 0; y < 0xc6; y += 0xb)
            sub_root_047c_05a5(D_0104, 3, y, D_a628);
        k = 0;
        for (i = 0; i < cnt; i++)
            sub_s021_1ef8_03fe(list[i].idx, &pics[i], cnt == 1 ? 50 : i * 100);
        sub_root_047c_1293();
        while (!sub_root_047c_073d() && !(D_6ef0 & 1))
            ;
        sub_root_0000_1384();
        while (D_6ef0 & 1)
            ;
    }
    for (i = 0; i < 3; i++)
        if (pics[i])
            free_buffer(pics[i], "Einfo");
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
}

void far sub_s021_1ef8_03fe(int m, int *pic, int x)
{
    char name[0x3c];
    char s[0x28];
    int y, t;
    unsigned w;

    D_0104->f0a = 0;
    sub_root_0000_25f2("einfo.cat");
    sprintf(s, "%s%sst.pic", D_1166[D_9c69[m].kind], D_1174[D_9c69[m].kind]);
    if (s[0] > 0x60)
        s[0] -= 0x20;
    sub_root_047c_0545();
    *pic = sub_root_0617_045c(s);
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_0104, 0xe, x, *pic);
    y = x + 0x20;
    D_0104->f0c = 0xe;
    _fstrcpy(name, F_2e38_0bd8[D_9c69[m].kind]);
    while ((w = sub_root_0265_084e(D_0104, name)) > 0x36)
        name[strlen(name) - 1] = 0;
    sub_root_0265_006e(D_0104, (0x36 - w) / 2 + 3, y, name);
    y += 8;
    D_0104->f0c = 0xf;
    if (s[0] == 'E') {
        sprintf(s, "%Fs", F_286b_01f7[D_9c69[m].f37]);
        if ((unsigned char)(D_9c69[m].f41 & 0x7f) > 0)
            strcat(s, "\x8e+\x8f");
        sub_root_0265_006e(D_0104, 4, y, s);
        y += 8;
        sprintf(s, "%Fs", F_286b_01f7[D_9c69[m].f38]);
        if ((unsigned char)(D_9c69[m].f41 & 0x7f) > 0)
            strcat(s, "\x8e+\x8f");
        sub_root_0265_006e(D_0104, 4, y, s);
        y += 8;
        if (D_9c69[m].f48 != -1) {
            sprintf(s, "%Fs", F_286b_01cf[D_9c69[m].f48]);
            sub_root_0265_006e(D_0104, 4, y, s);
            y += 8;
        }
        if (D_9c69[m].f3d == -1)
            t = 9;
        else
            t = D_9c69[m].f3d;
        if (t <= 0x21) {
            sprintf(s, "%Fs", F_286b_0277[t]);
            if (D_9c69[m].f34)
                strcat(s, "\x8e+\x8f");
            sub_root_0265_006e(D_0104, 4, y, s);
            y += 8;
            if (D_906a != 2) {
                sprintf(s, " - %s", D_4fce[sub_root_0410_001a(D_9c69[m].skill[0], 0, 100) / 20]);
                sub_root_0265_006e(D_0104, 4, --y, s);
                y += 8;
            }
        }
        t = -1;
        if (D_9c69[m].f0e != -1)
            t = D_9c69[m].f0e;
        else if (D_c2bf[m] == 2)
            t = 0x22;
        if (t != -1) {
            sprintf(s, "%Fs", F_286b_0277[t]);
            if (D_9c69[m].f34)
                strcat(s, "\x8e+\x8f");
            sub_root_0265_006e(D_0104, 4, y, s);
            y += 8;
            if (D_906a != 2) {
                sprintf(s, " - %s", D_4fce[sub_root_0410_001a(D_9c69[m].skill[5], 0, 100) / 20]);
                sub_root_0265_006e(D_0104, 4, y - 1, s);
            }
        }
    } else {
        sprintf(s, "Natrl Armr");
        sub_root_0265_006e(D_0104, 4, y, s);
        y += 8;
        if (D_906a != 2) {
            sprintf(s, " - %s", D_4fce[sub_root_0410_001a(D_9c69[m].f3f & 0xf, 0, 6)]);
            sub_root_0265_006e(D_0104, 4, --y, s);
            y += 8;
        }
        sprintf(s, "Natrl Weapn");
        sub_root_0265_006e(D_0104, 4, y, s);
        y += 8;
        if (D_906a != 2) {
            sprintf(s, " - %s", D_4fce[sub_root_0410_001a(D_9c69[m].skill[0], 0, 100) / 20]);
            sub_root_0265_006e(D_0104, 4, y - 1, s);
        }
    }
}
