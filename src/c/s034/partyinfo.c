/* place: s034 1550:0006 */
/* orig: size=0c86 CONST=7e88+000c _DATA=362c+018e */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* Party information screen: fame, date, wealth and the map of locations (overlay s034). */

struct gfx { char pad[0xc]; int color; };
struct loc {                        /* map location (far, segment 2e38) */
    int type;                       /* 0 for towns */
    int pad2;
    int x, y;
    char pad8[9];
    char size;
    int rep;
    char pad14[0x12];
    char name[1];
};

extern int D_907e;
extern int D_e7d6;
extern int D_e3d0, D_e3d2;
extern int D_2816, D_2804, D_2812, D_2800;
extern int D_e7a2, D_e7a0;
extern int D_9c42, D_9c46;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_e48c[];
extern struct gfx D_2806, D_27f4;
extern int D_9082;
extern int D_00e0, D_00e2, D_00e4;
extern int D_906b[];
extern int D_9072, D_9078;
extern struct loc far * far F_2e38_10be[];
extern char far * far F_290e_2183[];
extern char far * far F_290e_1f7b[];
extern char far * far F_290e_20c3[];
extern char far * far F_290e_1d43[];
extern char far * far F_290e_20e3[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *_itoa(int, char *, int);
char far *_fstrcpy(char far *, const char far *);
char far *_fstrcat(char far *, const char far *);
int abs(int);

int far vec_1fa9(int, int);
int far vec_1fbd(int, int, int, int);
void far sub_root_047c_124f(int, int, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_047c_0553(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far sub_root_047c_055a(struct gfx *, int, int, char *);
int far sub_s029_0e76_1a8e(int, int);
void far sub_root_047c_1320(int, int, int, int);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_058b(char *);
void far sub_root_047c_1266(int, int);
int far sub_s029_1367_0744(int);
void far sub_root_0265_0004(struct gfx *, int, int, char far *);
long far sub_s029_1367_00f2(int *);
void far sub_root_047c_0545(void);
int far sub_root_0617_045c(char *);
void far sub_root_047c_04f8(void);
void far sub_root_047c_05a5(struct gfx *, int, int, int);
void far sub_root_05ba_00c2(int, char *);
int far sub_root_047c_04e3(int, char);

int far party_info_close(void);
void far party_info_bg(void);
void far party_info_draw(void);
void far party_info_key(int key);
void far party_info_wealth(struct gfx *g);
void far party_info_map(struct gfx *g);
int far party_info_nearest(int x, int y);
int far text_width(char *s);

/* The party information screen. Hovering over the map shows the nearest town;
   Tab / Shift-Tab step through the towns. */
void far party_info(void)
{
    char buf[40];
    char rep[20];
    int last;
    int w;
    int key;
    int loc;
    int x, y;

    last = -2;
    if (D_907e != -1 && D_907e <= 0x5c)
        D_e7d6 = D_907e;
    else
        D_e7d6 = vec_1fa9(D_e3d0, D_e3d2);
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_root_047c_124f(0, 0, D_9c42);
    party_info_bg();
    party_info_draw();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (D_6eec > 0xc6 && D_6eec < 0x140 && D_6eee > 0x4a && D_6eee < 0xc3) {
            x = D_6eec - 0xc6;
            y = D_6eee - 0x4a;
            loc = party_info_nearest(x - 3, y + 3);
            if (loc != last && loc != -1) {
                last = loc;
                sub_root_047c_0553(&D_2806, 0xc8, 0x12, 0x78, 0x34, &D_27f4, 0xc8, 0x12);
                D_2800 = 14;
                _fstrcpy(buf, F_2e38_10be[loc]->name);
                sub_root_047c_055a(&D_27f4, 0x102 - text_width(buf) / 2, 0x22, buf);
                switch (F_2e38_10be[loc]->size) {
                case 0: case 1: case 2: case 3: case 4:
                    strcpy(buf, "Small-Sized");
                    break;
                case 5: case 6:
                    strcpy(buf, "Moderate-Sized");
                    break;
                default:
                    strcpy(buf, "Large-Sized");
                    break;
                }
                sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(buf) / 2), 0x2c, buf);
                D_2800 = 14;
                strcpy(buf, "Rep: ");
                _fstrcat(buf, F_290e_2183[sub_s029_0e76_1a8e(loc, 1)]);
                sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(buf) / 2), 0x36, buf);
                sprintf(rep, "(%Fd)", F_2e38_10be[loc]->rep);
                sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(rep) / 2), 0x40, rep);
            } else if (loc == -1 && last != -1) {
                sub_root_047c_0553(&D_2806, 0xc8, 0x12, 0x78, 0x34, &D_27f4, 0xc8, 0x12);
                last = -1;
            }
        }
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if ((key == 0 || key == 0xf09 || key == 0xf00) && !(D_6ef0 & 1)) {
            if (key)
                party_info_key(key);
            continue;
        }
        party_info_close();
        return;
    }
}

/* Leave the screen: request a redraw of whatever is underneath. */
int far party_info_close(void)
{
    D_9c46 = 1;
    while (D_6ef0 & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

/* Load the screen background. */
void far party_info_bg(void)
{
    sub_root_0617_024a(1, "pics\\ptystats.pic", D_e48c);
    D_2812 = 14;
    D_2816 = 1;
    D_2804 = 1;
}

/* Draw the static parts of the screen (titles in two colours for a shadow effect). */
void far party_info_draw(void)
{
    char fame[40];
    char num[20];
    int level;
    unsigned w;

    party_info_map(&D_2806);
    party_info_wealth(&D_2806);
    level = D_9082 / 50;
    if (level >= 10)
        level = 10;
    _fstrcpy(fame, F_290e_1f7b[level]);
    w = text_width(fame);
    D_2812 = 0xbf;
    sub_root_047c_055a(&D_2806, 0x86, 6, "PARTY INFORMATION");
    sub_root_047c_055a(&D_2806, 0xd7, 0x13, "MAP INFORMATION");
    sub_root_047c_055a(&D_2806, 0x6b, 0x13, "PARTY FAME");
    sub_root_047c_055a(&D_2806, 0x83 - w / 2, 0x1b, fame);
    sub_root_047c_055a(&D_2806, 0x80, 0x23, _itoa(D_9082, num, 10));
    sub_root_047c_055a(&D_2806, 0x54, 0x31, "TIME");
    sub_root_047c_055a(&D_2806, 0x95, 0x31, "DATE");
    sub_root_047c_055a(&D_2806, 0x4b, 0x60, "LOCATION");
    sub_root_047c_055a(&D_2806, 0x4c, 0x8e, "WEALTH");
    sub_root_047c_055a(&D_2806, 0x89, 0x8e, "NOTES");
    sub_root_047c_055a(&D_2806, 0x89, 0x60, "LOCAL REP");
    D_2812 = 5;
    sub_root_047c_055a(&D_2806, 0x85, 5, "PARTY INFORMATION");
    sub_root_047c_055a(&D_2806, 0xd6, 0x12, "MAP INFORMATION");
    sub_root_047c_055a(&D_2806, 0x6a, 0x12, "PARTY FAME");
    sub_root_047c_055a(&D_2806, 0x82 - w / 2, 0x1a, fame);
    sub_root_047c_055a(&D_2806, 0x7f, 0x22, _itoa(D_9082, num, 10));
    sub_root_047c_055a(&D_2806, 0x53, 0x30, "TIME");
    sub_root_047c_055a(&D_2806, 0x94, 0x30, "DATE");
    sub_root_047c_055a(&D_2806, 0x4a, 0x5f, "LOCATION");
    sub_root_047c_055a(&D_2806, 0x4b, 0x8d, "WEALTH");
    sub_root_047c_055a(&D_2806, 0x88, 0x8d, "NOTES");
    sub_root_047c_055a(&D_2806, 0x88, 0x5f, "LOCAL REP");
    sub_root_047c_0568();
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 0xc8, &D_27f4, 0x3c, 0);
    sub_root_047c_1293();
    sub_root_047c_056f();
}

/* Tab / Shift-Tab: mark the next / previous town on the map and describe it. */
void far party_info_key(int key)
{
    char buf[40];
    int w;

    switch (key) {
    case 0xf00:
        if ((D_e7d6 -= 2) < 0)
            D_e7d6 = 0x5b;
    case 0xf09:
        sub_root_047c_12c3();
        sub_root_047c_1266(116L * F_2e38_10be[D_e7d6]->x / 327 + 0xca,
                           106L * F_2e38_10be[D_e7d6]->y / 931 + 0x47);
        sub_root_047c_1293();
        sub_root_047c_0553(&D_2806, 0xc8, 0x12, 0x7a, 0x32, &D_27f4, 0xc8, 0x12);
        D_2800 = 14;
        _fstrcpy(buf, F_2e38_10be[D_e7d6]->name);
        sub_root_047c_055a(&D_27f4, 0x102 - text_width(buf) / 2, 0x22, buf);
        switch (F_2e38_10be[D_e7d6]->size) {
        case 0: case 1: case 2: case 3: case 4:
            strcpy(buf, "Small-Sized");
            break;
        case 5: case 6:
            strcpy(buf, "Moderate-Sized");
            break;
        default:
            strcpy(buf, "Large-Sized");
            break;
        }
        sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(buf) / 2), 0x2c, buf);
        D_2800 = 14;
        strcpy(buf, "Rep: ");
        _fstrcat(buf, F_290e_2183[sub_s029_0e76_1a8e(D_e7d6, 1)]);
        sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(buf) / 2), 0x36, buf);
        sprintf(buf, "(%Fd)", F_2e38_10be[D_e7d6]->rep);
        sub_root_047c_055a(&D_27f4, 0x102 - (w = text_width(buf) / 2), 0x40, buf);
        if (++D_e7d6 >= 0x5c)
            D_e7d6 = 0;
    }
}

/* Date, location and wealth panel. */
void far party_info_wealth(struct gfx *g)
{
    char buf[80];
    char name[40];
    char num[10];
    int w;
    int near_;

    sub_root_047c_12c3();
    g->color = 14;
    _fstrcpy(name, F_290e_20c3[sub_s029_1367_0744(D_00e0)]);
    w = text_width(name) / 2;
    sub_root_0265_0004(g, 0x60 - w, 0x40, F_290e_20c3[sub_s029_1367_0744(D_00e0)]);
    sub_root_047c_055a(g, 0x58, 0x48, _itoa(D_00e0, num, 10));
    g->color = 14;
    _fstrcpy(name, F_290e_1d43[D_00e4]);
    w = text_width(name) / 2;
    sub_root_0265_0004(g, 0xa0 - w, 0x40, F_290e_1d43[D_00e4]);
    sub_root_047c_055a(g, 0x98, 0x48, _itoa(D_00e2, num, 10));
    g->color = 14;
    if (D_907e != -1 && D_907e < 0x5c) {
        sub_root_0265_0004(g, 0x48, 0x6f, F_2e38_10be[D_907e]->name);
    } else {
        near_ = vec_1fa9(D_e3d0, D_e3d2);
        sprintf(buf, "%Fs of ", F_290e_20e3[vec_1fbd(F_2e38_10be[near_]->x, F_2e38_10be[near_]->y, D_e3d0, D_e3d2)]);
        sub_root_0265_0004(g, 0x46, 0x6f, (char far *)buf);
        sub_root_0265_0004(g, 0x48, 0x78, F_2e38_10be[near_]->name);
    }
    g->color = 14;
    sprintf(buf, "%d Florins", D_906b[0]);
    sub_root_047c_055a(g, 0x43, 0x9d, buf);
    sprintf(buf, "%d Groschen", D_906b[1]);
    sub_root_047c_055a(g, 0x43, 0xa5, buf);
    sprintf(buf, "%d Pfenniges", D_906b[2]);
    sub_root_047c_055a(g, 0x43, 0xad, buf);
    sprintf(buf, "(%ld) Total", sub_s029_1367_00f2(D_906b));
    sub_root_047c_055a(g, 0x43, 0xb5, buf);
    g->color = 14;
    sprintf(buf, "%d Florins", D_9072);
    sub_root_047c_055a(g, 0x84, 0x9d, buf);
    sprintf(buf, "%d PhStone", D_9078);
    sub_root_047c_055a(g, 0x84, 0xad, buf);
    g->color = 14;
    if (D_907e != -1 && D_907e < 0x5c) {
        sub_root_0265_0004(g, 0x84, 0x6f, F_290e_2183[sub_s029_0e76_1a8e(D_907e, 1)]);
        sprintf(buf, "(%Fd)", F_2e38_10be[D_907e]->rep);
        sub_root_047c_055a(g, 0x8e, 0x77, buf);
    }
    sub_root_047c_1293();
}

/* Mark the party's position on the small map. */
void far party_info_map(struct gfx *g)
{
    int x, y;
    int pic;

    if (D_907e == -1) {
        x = 116L * D_e3d0 / 327;
        y = 106L * D_e3d2 / 931;
    } else {
        x = 116L * F_2e38_10be[D_907e]->x / 327;
        y = 106L * F_2e38_10be[D_907e]->y / 931;
    }
    sub_root_047c_0545();
    pic = sub_root_0617_045c("pics\\maploctr.pic");
    sub_root_047c_04f8();
    sub_root_047c_05a5(g, x + 0xc8, y + 0x46, pic);
    sub_root_05ba_00c2(pic, "b");
}

/* Town nearest to a point on the small map, or -1 if none is close. */
int far party_info_nearest(int x, int y)
{
    int best, bestd, i, dx, dy, d;

    best = -1;
    bestd = 9999;
    x = (long)x * 327 / 118;
    y = (long)y * 931 / 106;
    for (i = 0; i < 414; i++) {
        if (F_2e38_10be[i]->type != 0)
            break;
        dx = abs(F_2e38_10be[i]->x - x);
        dy = abs(F_2e38_10be[i]->y - y) / 3;
        if (dy <= dx)
            d = dy / 2 + dx;
        else
            d = dx / 2 + dy;
        if (d < 10 && bestd > d) {
            bestd = d;
            best = i;
        }
    }
    return best;
}

/* Width in pixels of a string in the current font. */
int far text_width(char *s)
{
    int font;
    register char *p;
    register int w;

    font = D_2804;
    w = 0;
    p = s;
    _asm {
        cmp byte ptr [si], 0
        je done
    next:
        lodsb
        push ax
        push font
        call sub_root_047c_04e3
        add sp, 4
        add di, ax
        cmp byte ptr [si], 0
        jne next
    done:
    }
    return w;
}
