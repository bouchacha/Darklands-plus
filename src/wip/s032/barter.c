/* place: s032 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* e0 2026-10-09: functions renamed back to their original sub_ names (descriptive names in the comments); still 5/11.
 * Session 4: 078e down to one difference: i and y slots swapped ([bp-4]/[bp-6]); needed
 *   `#pragma optimize("a", off)`, srand as an if/else of two calls and `extern int D_9c4a;`
 *   moved before `extern int D_2800;`.  Declaration order, names, loop-init order, y as a
 *   strength-reduced i*8+0x93: no effect on the slot order.
 *   1d18: the uninitialised `town` and `size` are one variable in the original (both [bp-2]);
 *   writing them as one makes size a register variable in 6.00A (worse), so left as is.
 *   CONST: the original reaches F_2e38_10be (192e/1b26) through DS=2e38 (no CONST slot) and
 *   has an unused 28f5 slot at 7e6e; ours gives 10be a slot. */
/* data: _DATA=2f18 */
/* data: CONST=7e66 */
/* Barter screen (overlay s032): buy and sell items at a shop. */

typedef long time_t;

/* party member record in DGROUP (0x80 bytes) */
struct member {
    char present;               /* +00 */
    char pad01[2];
    char female;                /* +03 */
    char pad04[0x26];
    char name[0x40];            /* +2a */
    char nitems;                /* +6a */
    char pad6b[0x15];
};

/* one inventory entry (6 bytes) */
struct item {
    int id;                     /* low 12 bits: item type */
    char f2;
    unsigned char qual;
    unsigned char cnt;
    char f5;
};

/* character record in far data (0x180 bytes) */
struct fchar {
    char pad00[0x3e];
    struct item items[53];
    char pad17c[4];
};

/* item definition */
struct itemdef {
    char name[0x1e];
    int type;                   /* +1e */
    long cats;                  /* +20: shop categories */
    unsigned char flags;        /* +24: 0x80 = cannot be traded */
    char weight;                /* +25 */
    char qual;                  /* +26 */
    char rarity;                /* +27 */
    char pad28[4];
    int value;                  /* +2c */
};

struct city { char pad00[0x40]; int size; char pad42[0x14]; int seed; char pad58[0x0a]; char quality[9]; };
struct loc { char pad[0x14]; unsigned char flags; };
struct money { int fl, gr, pf; };

extern int D_2816, D_2804, D_2812, D_e7a2, D_e7a0;
extern char D_9920, D_9934;
extern char D_907c, D_907b;
extern int D_884e, D_884c, D_907e;
extern int D_9c4a;
extern int D_2800;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_27f4[], D_2806[], D_e48c[], D_9419[];
extern int D_906b, D_906d, D_906f;
extern int D_9c46;
extern struct member D_9c69[];
extern struct city far *D_9785;
extern char far *D_869c;            /* shop name */
extern long D_8868;                 /* item categories sold */
extern int D_8834;                  /* shop quality */
extern int D_8836;                  /* shop */
extern int D_8832;                  /* shop list: cursor row */
extern int D_86a0;                  /* shop list: first visible entry */
extern int D_885c;                  /* number of items for sale */
extern int D_8850;
extern int D_86a2[];                /* item types for sale */
extern int D_8838[], D_8842[], D_8852[], D_885e[];

extern char far * far F_290e_207f[];
extern struct fchar far F_2e38_0000[];
extern struct itemdef far * far F_2e38_0890[];
extern unsigned char far F_290e_399b[][12];
extern struct loc far * far F_2e38_10be[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
void srand(unsigned);
time_t time(time_t *);
int far random_n(int);

void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_04dc(char *, int, int, int, int, int, int);
void far sub_root_047c_055a(char *, int, int, char *);
void far sub_root_047c_0553(char *, int, int, int, int, char *, int, int);
void far sub_root_047c_058b(char *);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_0000_0000(char *, int);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_0000_13a0(void);
int far sub_root_0000_0c70(int);
void far sub_root_0617_024a(int, char *, char *);
long far sub_s029_1367_00f2(int *);
void far sub_s029_1367_00a4(struct money *, int, int, int);
int far sub_s029_1367_02d4(int *, struct money *);
void far sub_s029_1367_0130(int *, int, int, int);
void far sub_s029_1367_0180(int *, int, int, int);
int far sub_s029_1367_0066(int, int, int);
int far sub_s029_0e76_05ee(int, int);
int far sub_s029_0e76_1a7e(int);
int far sub_s029_0e76_1326(int);
int far vec_1fdb(int);
int far vec_2003(int, int);
int far vec_1f6d(int, struct item);

int far sub_s032_1462_076c(int ret, int shop);
void far sub_s032_1462_078e(int who, char far *name, long cats);
void far sub_s032_1462_0b30(int who, char far *name, long cats);
int far sub_s032_1462_0efa(int k);
int far sub_s032_1462_0f60(int key);
void far sub_s032_1462_1770(int who);
void far sub_s032_1462_1870(int unused);
int far sub_s032_1462_192e(int k);
int far sub_s032_1462_1b26(int k, int qual);
int far sub_s032_1462_1d18(int k);

extern char str_sell[], str_buy[], str_space[];

/* sub_s032_1462_0000 (barter_screen): main loop of the barter screen for shop `shop` */
void far sub_s032_1462_0000(int shop, char far *name, long cats)
{
    char buf[136];
    int hover;
    int row;
    int mkey;

    hover = -1;
    D_9920 = 2;
    D_9934 = 2;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    D_2816 = 1;
    D_2804 = 1;
    D_907c = 5;
    D_884c = D_884e = D_907b;
    D_2800 = 8;
    D_869c = name == 0 ? F_290e_207f[shop] : name;
    D_8868 = cats;
    if ((D_8836 = shop) < 0 || shop >= 9)
        D_8834 = 25;
    else
        D_8834 = D_9785->quality[shop];
    if (shop == -2)
        D_8834 = 10;
    for (row = 0; row < 5; row++) {
        D_8842[row] = 1;
        D_8838[row] = 0;
        D_885c = 0;
        D_8852[row] = 0;
        D_86a0 = 0;
        D_885e[row] = 0;
        D_8832 = 0;
    }
    sub_root_047c_0568();
    sub_s032_1462_078e(D_907b, D_869c, cats);
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        mkey = 0;
        if (!(D_6ef0 & 1) && D_6eec > 0x50 && D_6eec < 0xe6 && D_6eee > 0x1b && D_6eee < 0x43) {
            row = (D_6eee - 0x1b) / 8;
            if (row != hover) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(D_27f4, 0x50, hover * 8 + 0x1b, 0x96, 8, 9, 8);
                hover = row;
                sub_root_047c_04dc(D_27f4, 0x50, hover * 8 + 0x1b, 0x96, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0x50 && D_6eec < 0xe6 && D_6eee > 0x1b && D_6eee < 0x43) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x1b) / 8;
            switch (row) {
            case 0: mkey = 0x1950; break;
            case 1: mkey = 0x1f53; break;
            case 2: mkey = 0x1e41; break;
            case 3: mkey = 0x3042; break;
            case 4: mkey = 0x11b; break;
            }
        } else if (!(D_6ef0 & 1) && hover != -1) {
            sub_root_047c_12c3();
            sub_root_047c_04dc(D_27f4, 0x50, hover * 8 + 0x1b, 0x96, 9, 9, 8);
            hover = -1;
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eec > 0x72 && D_6eec < 0x11c && D_6eee > 0x93 && D_6eee < 0xb3) {
            sub_root_047c_12c3();
            D_8842[D_884e] = 0;
            D_885e[D_884e] = (D_6eee - 0x93) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eec > 0x52 && D_6eec < 0x10b && D_6eee > 0x55 && D_6eee < 0x75) {
            sub_root_047c_12c3();
            D_8842[D_884e] = 1;
            D_8832 = (D_6eee - 0x55) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x8a && D_6eee < 0x93 && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x4800;
            D_8842[D_884e] = 0;
            D_2800 = 7;
            sub_root_047c_12c3();
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eee > 0xb3 && D_6eee < 0xbc && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x5000;
            D_8842[D_884e] = 0;
            sub_root_047c_12c3();
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eee > 0x4c && D_6eee < 0x55 && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x4800;
            D_8842[D_884e] = 1;
            sub_root_047c_12c3();
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eee > 0x75 && D_6eee < 0x7e && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x5000;
            D_8842[D_884e] = 1;
            sub_root_047c_12c3();
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eec < 0x3c) {
            row = D_6eee / 40;
            row = vec_1fdb(row);
            sub_root_047c_0568();
            sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
            sub_root_047c_058b(D_e48c);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s032_1462_0b30(D_884e, D_869c, cats);
            sub_root_047c_056f();
            while (D_6ef0 & 1)
                ;
        }
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_047c_0568();
            sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
            sub_root_047c_058b(D_e48c);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s032_1462_0b30(D_884e, D_869c, cats);
            sub_root_047c_056f();
        }
        if (sub_root_047c_073d())
            mkey = sub_root_047c_0744();
        if (mkey == 0x11b || mkey == 0x264c || mkey == 0x266c) {
            sub_s032_1462_076c(-1, shop);
            return;
        }
        if (mkey && sub_s032_1462_0f60(mkey) == 0x11b) {
            sub_s032_1462_076c(-1, shop);
            return;
        }
    }
}

char str_sell[] = "\x8c S\xff" "ell an item";
char str_buy[] = "\x8c P\xff" "urchase an item";
char str_space[] = " ";

/* sub_s032_1462_076c (barter_close): restore the screen state on leaving */
int far sub_s032_1462_076c(int ret, int shop)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    D_9920 = 1;
    D_9934 = 1;
    D_907b = D_884c;
    return ret;
}

/* sub_s032_1462_078e (draw_barter_screen): draw the screen and collect the items the shop sells */
#pragma optimize("a", off)
void far sub_s032_1462_078e(int who, char far *name, long cats)
{
    char buf[120];
    char self[10];
    char he[10];
    int y;
    int i;
    int k;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_root_047c_0553(D_2806, 0x3c, 0, 0x104, 0xc8, D_27f4, 0x3c, 0);
    D_2812 = 8;
    D_2800 = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", D_9c69[D_907b].name,
            D_906b, D_906d, D_906f, sub_s029_1367_00f2(&D_906b));
    sub_root_047c_055a(D_27f4, 0x49, 9, buf);
    if (D_9c69[D_907b].female == 0) {
        strcpy(he, "He");
        strcpy(self, "himself");
    } else {
        strcpy(he, "She");
        strcpy(self, "herself");
    }
    if (D_907b != D_884e)
        sprintf(buf, "%s barters for %s to the %Fs", he, D_9c69[who].name, name);
    else
        sprintf(buf, "%s barters for %s to the %Fs", D_9c69[who].name, self, name);
    sub_root_047c_055a(D_27f4, 0x49, 0x12, buf);
    sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
    D_2800 = 7;
    sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
    sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
    D_2800 = 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x33, "\x8c B\xff" "arter for another person");
    sub_root_047c_055a(D_27f4, 0x50, 0x3b, "\x8c L\xff" "eave");
    sprintf(buf, "The %Fs offers...", name);
    sub_root_047c_055a(D_27f4, 0x7a, 0x45, buf);
    sprintf(buf, "%s currently has...", D_9c69[who].name);
    sub_root_047c_055a(D_27f4, 0x81, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < D_9c69[who].nitems; i++, y += 8) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sub_s032_1462_1b26(F_2e38_0000[who].items[i].id & 0xfff, F_2e38_0000[who].items[i].qual),
                F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, y, buf);
    }
    if (D_907e < 0x5c && D_907e != -1)
        srand(D_9785->seed + D_9c4a);
    else
        srand(D_9c4a + D_907e);
    for (k = D_8850 = 0; k < 200; k++) {
        if (sub_s032_1462_1d18(k)) {
            if (D_8850 < 4) {
                sprintf(buf, "%5upf  %Fs  %3dq  %3dlbs", sub_s032_1462_192e(k), F_2e38_0890[k],
                        sub_s032_1462_0efa(k), F_2e38_0890[k]->weight);
                sub_root_047c_055a(D_27f4, 0x55, (D_8850 - D_8852[D_884e]) * 8 + 0x55, buf);
            }
            D_86a2[D_8850] = k;
            D_8850++;
        }
    }
    srand((unsigned)time(0));
    sub_root_047c_04dc(D_27f4, 0x53, 0x55, 0xb8, 8, 8, 9);
    D_8838[D_884e] = D_9c69[who].nitems;
    D_885c = D_8850;
    sub_root_047c_1293();
}
#pragma optimize("", on)

/* sub_s032_1462_0b30 (redraw_barter_screen): redraw the screen (other party member or after a change) */
void far sub_s032_1462_0b30(int who, char far *name, long cats)
{
    char buf[120];
    char self[10];
    char he[10];
    int i;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_0553(D_2806, 0x3c, 0, 0x104, 0xc8, D_27f4, 0x3c, 0);
    D_2800 = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", D_9c69[D_907b].name,
            D_906b, D_906d, D_906f, sub_s029_1367_00f2(&D_906b));
    sub_root_047c_055a(D_27f4, 0x49, 9, buf);
    if (D_9c69[D_907b].female == 0) {
        strcpy(he, "He");
        strcpy(self, "himself");
    } else {
        strcpy(he, "She");
        strcpy(self, "herself");
    }
    if (D_907b != D_884e)
        sprintf(buf, "%s barters for %s to the %Fs", he, D_9c69[who].name, name);
    else
        sprintf(buf, "%s barters for %s to the %Fs", D_9c69[who].name, self, name);
    sub_root_047c_055a(D_27f4, 0x4e, 0x12, buf);
    D_2800 = D_8842[D_884e] ? 8 : 7;
    sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
    D_2800 = D_8842[D_884e] ? 7 : 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
    D_2800 = 7;
    sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
    D_2800 = 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x33, "\x8c B\xff" "arter for another person");
    sub_root_047c_055a(D_27f4, 0x50, 0x3b, "\x8c L\xff" "eave");
    sprintf(buf, "The %Fs offers...", name);
    sub_root_047c_055a(D_27f4, 0x7a, 0x45, buf);
    sprintf(buf, "%s has...", D_9c69[who].name);
    sub_root_047c_055a(D_27f4, 0x81, 0x83, buf);
    for (i = D_8852[D_884e]; i < D_8852[D_884e] + 4 && i < D_9c69[who].nitems; i++) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sub_s032_1462_1b26(F_2e38_0000[who].items[i].id & 0xfff, F_2e38_0000[who].items[i].qual),
                F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, (i - D_8852[D_884e]) * 8 + 0x93, buf);
    }
    for (i = D_86a0; i < D_86a0 + 4 && i < D_885c; i++) {
        sprintf(buf, "%5upf  %Fs  %3dq  %3dlbs", sub_s032_1462_192e(D_86a2[i]), F_2e38_0890[D_86a2[i]],
                sub_s032_1462_0efa(D_86a2[i]), F_2e38_0890[D_86a2[i]]->weight);
        sub_root_047c_055a(D_27f4, 0x55, (i - D_86a0) * 8 + 0x55, buf);
    }
    if (D_8842[D_884e])
        sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
    else
        sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
    D_8838[D_884e] = D_9c69[who].nitems;
    sub_root_047c_1293();
}

/* sub_s032_1462_0efa (item_quality): quality of the items of type k the shop offers */
int far sub_s032_1462_0efa(int k)
{
    struct itemdef far *d;

    if (F_2e38_0890[k]->type == 0x40 || F_2e38_0890[k]->type == 0x41 || F_2e38_0890[k]->type == 0x42 || F_2e38_0890[k]->cats == 0x400L ||
        F_2e38_0890[k]->type == 0x17 || F_2e38_0890[k]->type == 0x18 || F_2e38_0890[k]->type == 0x19)
        return 25;
    return (F_2e38_0890[k]->qual + D_8834) / 2;
}

/* sub_s032_1462_0f60 (barter_key): handle one key press; returns the key (0x11b = leave) */
int far sub_s032_1462_0f60(int key)
{
    char buf[120];
    struct money price;
    struct item it;
    int id;

    switch (key) {
    case 0x1000:
        sub_s032_1462_076c(-1, 0);
        sub_root_0000_13a0();
        break;
    case 0x1950:
    case 0x1970:            /* P: purchase */
        if (D_8842[D_884e] == 0)
            break;
        if (D_885c == 0)
            break;
        sub_s029_1367_00a4(&price, 0, 0, sub_s032_1462_192e(D_86a2[D_8832 + D_86a0]));
        if (sub_s029_1367_02d4(&D_906b, &price) == -1)
            break;
        it.id = D_86a2[D_8832 + D_86a0];
        it.f2 = F_2e38_0890[it.id]->type;
        it.qual = sub_s032_1462_0efa(it.id);
        it.f5 = F_2e38_0890[D_86a2[D_8832 + D_86a0]]->weight;
        it.cnt = 1;
        sub_s029_1367_0180(&D_906b, 0, 0, sub_s032_1462_192e(D_86a2[D_8832 + D_86a0]));
        sub_root_047c_0553(D_2806, 0x49, 9, 0xee, 9, D_27f4, 0x49, 9);
        D_2800 = 8;
        sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", D_9c69[D_907b].name,
                D_906b, D_906d, D_906f, sub_s029_1367_00f2(&D_906b));
        sub_root_047c_055a(D_27f4, 0x49, 9, buf);
        D_8838[D_884e] = vec_1f6d(D_884e, it);
        sub_s032_1462_1770(D_884e);
        sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0x5f, 8, 8, 9);
        break;
    case 0x1f53:
    case 0x1f73:            /* S: sell */
        if (D_8842[D_884e] != 0)
            break;
        if (D_8838[D_884e] == 0)
            break;
        id = F_2e38_0000[D_884e].items[D_885e[D_884e] + D_8852[D_884e]].id;
        if (F_2e38_0890[id]->flags & 0x80)
            break;
        sub_s029_1367_0130(&D_906b, 0, 0,
                           sub_s032_1462_1b26(id, F_2e38_0000[D_884e].items[D_885e[D_884e] + D_8852[D_884e]].qual));
        sub_root_047c_0553(D_2806, 0x49, 9, 0xee, 9, D_27f4, 0x49, 9);
        D_2800 = 8;
        sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", D_9c69[D_907b].name,
                D_906b, D_906d, D_906f, sub_s029_1367_00f2(&D_906b));
        sub_root_047c_055a(D_27f4, 0x49, 9, buf);
        D_8838[D_884e] = vec_2003(D_884e, D_885e[D_884e] + D_8852[D_884e]);
        if (D_885e[D_884e] + D_8852[D_884e] >= D_8838[D_884e]) {
            if (D_8852[D_884e] != 0)
                D_8852[D_884e]--;
            else if (D_885e[D_884e] != 0)
                D_885e[D_884e]--;
        }
        sub_s032_1462_1770(D_884e);
        sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        break;
    case 0x2044:
    case 0x2064:            /* D */
        if (D_8842[D_884e] != 0)
            break;
        D_8838[D_884e] = vec_2003(D_907b, D_885e[D_884e] + D_8852[D_884e]);
        sub_s032_1462_1770(D_884e);
        sub_root_047c_04dc(D_27f4, 0x50, D_885e[D_884e] * 8 + 0x1b, 0x5f, 8, 8, 9);
        break;
    case 0x264c:
    case 0x266c:            /* L: leave */
        return 0x11b;
    case 0x3042:
    case 0x3062:            /* B: barter for another person */
        sub_root_0000_1588(0x100);
        sub_root_0000_1716(0x100);
        do {
            D_884e++;
            D_884e %= 5;
        } while (D_9c69[D_884e].present == 0);
        sub_s032_1462_0b30(D_884e, D_869c, D_8868);
        sub_root_047c_04dc(D_27f4, 0x50, 0x33, 0xc8, 9, 8, 9);
        break;
    case 0x4800:            /* up */
        sub_root_047c_12c3();
        if (D_8842[D_884e])
            D_8832--;
        else
            D_885e[D_884e]--;
        if (D_8842[D_884e] && D_8832 < 0 && D_86a0 > 0) {
            D_8832++;
            D_86a0--;
            sub_s032_1462_1870(D_907e);
            sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_8842[D_884e] && D_885e[D_884e] < 0 && D_8852[D_884e] > 0) {
            D_885e[D_884e]++;
            D_8852[D_884e]--;
            sub_s032_1462_1770(D_884e);
            sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_8832 < 0)
                D_8832 = 0;
            if (D_885e[D_884e] < 0)
                D_885e[D_884e] = 0;
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (D_8842[D_884e])
                sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xc3, 8, 8, 9);
            else
                sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        sub_root_047c_1293();
        break;
    case 0x5000:            /* down */
        sub_root_047c_12c3();
        if (D_8842[D_884e])
            D_8832++;
        else
            D_885e[D_884e]++;
        if (D_8842[D_884e] && D_8832 >= 4 && D_86a0 + D_8832 < D_885c) {
            D_8832--;
            D_86a0++;
            sub_s032_1462_1870(D_907e);
            sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_8842[D_884e] && D_885e[D_884e] >= 4 && D_885e[D_884e] + D_8852[D_884e] < D_8838[D_884e]) {
            D_885e[D_884e]--;
            D_8852[D_884e]++;
            sub_s032_1462_1770(D_884e);
            sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_8832 >= D_885c)
                D_8832--;
            if (D_8832 >= 4)
                D_8832 = 3;
            if (D_885e[D_884e] >= 4)
                D_885e[D_884e] = 3;
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (D_8842[D_884e])
                sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xc8, 8, 8, 9);
            else
                sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        sub_root_047c_1293();
        break;
    case 0x5e00:
    case 0x5f00:
    case 0x6000:
    case 0x6100:
    case 0x6200:            /* Ctrl-F1..F5: choose the party member who pays */
        key = (key >> 8) - 0x5e;
        if (D_9c69[key].present == 0)
            break;
        D_907b = key;
        sub_root_0000_1716(0x100);
        /* fall through */
    case 0x4b00:
    case 0x4d00:            /* left / right: switch lists */
        sub_root_047c_12c3();
        if (D_8842[D_884e]) {
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
            D_8842[D_884e] = 0;
        } else {
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_sell);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_buy);
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x2b, str_space);
            sub_root_047c_04dc(D_27f4, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
            D_8842[D_884e] = 1;
            D_2800 = 8;
        }
        sub_root_047c_1293();
        break;
    }
    return key;
}

/* sub_s032_1462_1770 (draw_member_items): list four of the member's items with their sell prices */
void far sub_s032_1462_1770(int who)
{
    char buf[60];
    int i;

    sub_root_047c_12c3();
    D_2800 = 8;
    sub_root_047c_0553(D_2806, 0x72, 0x93, 0xb6, 0x20, D_27f4, 0x72, 0x93);
    for (i = D_8852[D_884e]; i < D_8852[D_884e] + 4 && i < D_9c69[who].nitems; i++) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sub_s032_1462_1b26(F_2e38_0000[who].items[i].id & 0xfff, F_2e38_0000[who].items[i].qual),
                F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, (i - D_8852[D_884e]) * 8 + 0x93, buf);
    }
    sub_root_047c_1293();
}

/* sub_s032_1462_1870 (draw_shop_items): list four of the items for sale with their prices */
void far sub_s032_1462_1870(int unused)
{
    char buf[120];
    int i;

    sub_root_047c_12c3();
    D_2800 = 8;
    sub_root_047c_0553(D_2806, 0x52, 0x55, 0xb9, 0x20, D_27f4, 0x52, 0x55);
    for (i = D_86a0; i < D_86a0 + 4 && i < D_885c; i++) {
        sprintf(buf, "%3dpf  %Fs  %3dq  %3dlbs", sub_s032_1462_192e(D_86a2[i]), F_2e38_0890[D_86a2[i]],
                sub_s032_1462_0efa(D_86a2[i]), F_2e38_0890[D_86a2[i]]->weight);
        sub_root_047c_055a(D_27f4, 0x55, (i - D_86a0) * 8 + 0x55, buf);
    }
    sub_root_047c_1293();
}

/* sub_s032_1462_192e (buy_price): price in pfennigs the shop asks for an item of type k */
int far sub_s032_1462_192e(int k)
{
    unsigned char flags;
    long price;
    long rarity;
    long q;
    long size;
    long value;

    value = F_2e38_0890[k]->value;
    q = sub_s032_1462_0efa(k);
    rarity = F_2e38_0890[k]->rarity;
    size = D_907e < 0x5c && D_907e != -1 ? D_9785->size : 2;
    if (D_907e != -1) {
        flags = F_2e38_10be[D_907e]->flags;
        if ((flags & 2) || (flags & 0x41))
            rarity += 3;
        else if (flags & 8)
            q += sub_root_0000_0c70(10);
    }
    price = q * value * 2 / 25;
    if (D_907e != -1)
        price += price * size * 2 / (rarity > 5 ? 100 : -100);
    price += price * (25 - sub_s029_0e76_05ee(D_907b, 5)) / 100;
    if (D_907e != -1)
        price += price * (sub_s029_1367_0066(-999, 99, sub_s029_0e76_1a7e(0) * 50) / 100) / -100;
    price += price * (sub_s029_1367_0066(0, 500, sub_s029_0e76_1326(4) * 100) / 1000) / -100;
    if (price < 1)
        price = 1;
    return price;
}

/* sub_s032_1462_1b26 (sell_price): price in pfennigs the shop pays for an item of type k and quality qual */
int far sub_s032_1462_1b26(int k, int qual)
{
    unsigned char flags;
    long price;
    long rarity;
    long size;
    long value;
    struct itemdef far *d;

    d = F_2e38_0890[k];
    if (d->flags & 0x80)
        return 0;
    value = d->value / 4;
    rarity = d->rarity;
    size = D_907e < 0x5c && D_907e != -1 ? D_9785->size : 2;
    if (D_907e != -1) {
        flags = F_2e38_10be[D_907e]->flags;
        if ((flags & 2) || (flags & 0x41))
            rarity += 3;
        else if (flags & 8)
            qual += sub_root_0000_0c70(10);
    }
    price = qual * value * 2 / 25;
    if (D_907e != -1)
        price += size * price * 2 / (rarity > 5 ? 100 : -100);
    price += price * (25 - sub_s029_0e76_05ee(D_907b, 5)) / -100;
    if (D_907e != -1)
        price += price * (sub_s029_1367_0066(-999, 75, sub_s029_0e76_1a7e(0) * 50) / 100) / 100;
    price += price * (sub_s029_1367_0066(0, 500, sub_s029_0e76_1326(4) * 100) / 1000) / 100;
    if (price < 1)
        price = 0;
    return price;
}

/* sub_s032_1462_1d18 (shop_has_item): whether the shop offers items of type k today */
int far sub_s032_1462_1d18(int k)
{
    int size;
    int town;
    int r;
    int q;

    if ((F_2e38_0890[k]->cats & D_8868) && !(F_2e38_0890[k]->flags & 0x80)) {
        if (F_2e38_0890[k]->type == 0x40 || F_2e38_0890[k]->type == 0x41 || F_2e38_0890[k]->type == 0x42)
            return 1;
        size = town < 0x5c && town != -1 ? D_9785->size - 1 : 1;
        if (size > 9)
            size = 9;
        r = F_2e38_0890[k]->rarity;
        q = sub_s032_1462_0efa(k);
        if (q < 0x17)
            r++;
        else if (q > 0x1d)
            r -= 2;
        else if (q > 0x1a)
            r--;
        if (r < 0)
            r = 0;
        else if (r > 11)
            r = 11;
        q = F_290e_399b[size][r];
        if (random_n(100) <= q)
            return 1;
    }
    return 0;
}
