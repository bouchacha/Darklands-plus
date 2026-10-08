/* place: s034 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* Calling on a saint: the prayer screen (overlay s034). */

typedef int (far *saintfn)(int, int, int);
typedef struct _iobuf FILE;

struct gfx { char pad[0xc]; int color; };
struct pchar {                      /* party member, 128 bytes */
    char used;
    char pad1[0x29];
    char name[0x25];
    char maxdf;                     /* divine favor available */
    char pad2[0x30];
};
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

extern int D_2816, D_2804, D_2800;
extern int D_e7a2, D_e7a0;
extern char D_e48c[];
extern struct gfx D_2806, D_27f4;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_907a;
extern int D_9c3d;
extern unsigned D_887a;
extern int D_887c, D_887e, D_8878;
extern int D_89ea, D_89ec;
extern int D_9c46;
extern int D_a67e;
extern char D_9419[];
extern char D_8882[360];
extern struct pchar D_9c69[];
extern saintfn far F_290e_2937[];
extern char far * far F_2e38_183e[];
extern char far * far F_290e_1d43[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
int printf(const char *, ...);
FILE *fopen(const char *, const char *);
int fgetc(FILE *);
int fseek(FILE *, long, int);
unsigned fread(void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
void exit_(int);

int far vec_18f1(int);
int far sub_root_0000_0c70(int);
void far sub_s029_0e76_0a72(int, int, int);
void far sub_root_0000_1716(int);
void far sub_root_0000_24bc(char *, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_04dc(struct gfx *, int, int, int, int, int, int);
void far sub_root_047c_05a5(struct gfx *, int, int, int);
void far sub_root_05ba_00c2(int, char *);
int far sub_s029_1367_0066(int, int, int);
void far sub_root_0000_0000(char *, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_047c_058b(char *);
void far sub_root_047c_0553(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far sub_root_0000_13a0(void);
void far sub_root_047c_055a(struct gfx *, int, int, char *);
unsigned far sub_root_0265_084e(struct gfx *, char *);
void far sub_root_0265_0134(struct gfx *, char *, int, int, int, int, int, int, int, int, int, int);
int far sub_s029_1367_0982(int);
int far sub_s029_1367_0938(int);
int far sub_root_047c_051b(int);
void far sub_root_047c_0545(void);
int far sub_root_047c_04d5(int, int, int, int, int);
void far sub_root_047c_04f8(void);
void far sub_root_047c_0952(struct gfx *, int, int, int, int, int);
void far sub_root_047c_0d62(struct gfx *, int, int, int, int, int);

int far saint_screen(int ch, int saint, int party);
int far saint_finish(int ch, int saint);
void far saint_load_bg(int ch, int saint);
void far saint_draw(int ch, int saint);
void far saint_key(int key, int ch, int saint);
void far saint_draw_df(int ch, int saint, struct gfx *g);
void far saint_draw_menu(int ch, int saint, struct gfx *g);
void far saint_draw_info(int ch, int saint, struct gfx *g);
void far saint_text_box(char *text, int h);
void far saint_party_box(char *buf);

/* Pray to a saint on behalf of the whole party. Returns -1 if cancelled, 1 on success. */
int far saint_pray_party(int ch, int saint)
{
    int r;

    D_9c3d = vec_18f1(5);
    vec_18f1(13);
    r = saint_screen(ch, saint, 1);
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    if (r == -1) {
        vec_18f1(D_9c3d);
        return -1;
    }
    if (sub_root_0000_0c70(100) <= r) {
        F_290e_2937[saint](D_89ea, D_887c, 8);
        sub_s029_0e76_0a72(ch, 6, -D_887c);
        sub_root_0000_1716(0x100);
        vec_18f1(D_9c3d);
        return 1;
    }
    if (sub_root_0000_0c70(99) < r - 66)
        sub_s029_0e76_0a72(ch, 6, -(D_887c / 2));
    sub_s029_0e76_0a72(ch, 6, -D_887c);
    sub_root_0000_1716(0x100);
    vec_18f1(D_9c3d);
    return 0;
}

/* Pray to a saint for one character. */
int far saint_pray(int ch, int saint)
{
    int r;

    if (D_907a != 1) {
        D_9c3d = vec_18f1(5);
        vec_18f1(13);
    }
    r = saint_screen(ch, saint, 0);
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    if (r == -1) {
        if (D_907a != 1 && D_9c3d != -1)
            vec_18f1(D_9c3d);
        return -1;
    }
    if (sub_root_0000_0c70(100) <= r) {
        sub_root_0000_24bc("Saint Call Successful", 100);
        F_290e_2937[saint](D_89ea, D_887c, 8);
        sub_root_0000_1716(0x100);
    } else if (sub_root_0000_0c70(99) < r - 66) {
        sub_s029_0e76_0a72(ch, 6, -(D_887c / 2));
        sub_root_0000_24bc("Saint Call Fails", 100);
    } else {
        sub_root_0000_24bc("Saint Call Fails", 100);
    }
    sub_s029_0e76_0a72(ch, 6, -D_887c);
    sub_root_0000_1716(0x100);
    if (D_907a != 1 && D_9c3d != -1)
        vec_18f1(D_9c3d);
    return 0;
}

/* The saint screen: choose how much divine favor to spend, then pray.
   Returns the success chance, or -1 when the player backs out. */
int far saint_screen(int ch, int saint, int party)
{
    char buf[120];
    int t;
    int key;
    int sel_b, sel_a;
    int row;

    sel_b = -1;
    sel_a = -1;
    D_887a = 0;
    D_89ea = ch;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    D_887e = 0;
    D_8878 = 0;
    t = F_290e_2937[saint](ch, 0, 6);
    if (party)
        D_89ec = t & 2;
    else
        D_89ec = t & 1;
    D_887c = F_290e_2937[saint](ch, 0, 2);
    if (D_9c69[ch].maxdf < D_887c)
        D_887c = 0;
    sub_root_047c_0568();
    saint_load_bg(ch, saint);
    saint_draw(ch, saint);
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (D_887e == 1 && !(D_6ef0 & 1) && D_6eec > 250 && D_6eec < 310 && D_6eee > 142 && D_6eee < 192) {
            row = (D_6eee - 142) / 9;
            if (sel_a != row) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(&D_27f4, 0xfb, (sel_a + 16) * 9, 0x3b, 9, 9, 8);
                sel_a = row;
                sub_root_047c_04dc(&D_27f4, 0xfb, (row + 16) * 9, 0x3b, 9, 8, 9);
                sub_root_047c_1293();
            }
        } else if (D_887e == 1 && (D_6ef0 & 1) && D_6eec > 250 && D_6eec < 310 && D_6eee > 142 && D_6eee < 192) {
            while (D_6ef0 & 1)
                ;
            D_89ea = row = sub_s029_1367_0066(0, 4, (D_6eee - 142) / 9);
            if (row < D_a67e && D_9c69[row].used)
                return saint_finish(ch, saint);
        } else if (!(D_6ef0 & 1) && D_6eec > 148 && D_6eec < 318 && D_6eee > 158 && D_6eee < 190) {
            row = (D_6eee - 158) / 8;
            if (sel_b != row) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(&D_27f4, 0x94, sel_b * 8 + 158, 0xa9, 8, 9, 8);
                sel_b = row;
                if (row == 2 && D_89ec && !D_887e && F_290e_2937[saint](ch, D_887c, 7)) {
                    sub_root_047c_04dc(&D_27f4, 0x94, row * 8 + 158, 0xa9, 8, 8, 9);
                    saint_party_box(buf);
                    D_887e = 1;
                } else if (row != 2 && D_887e) {
                    D_887e = 0;
                    sub_root_047c_05a5(&D_27f4, 0xfa, 0x8e, D_887a);
                    sub_root_05ba_00c2(D_887a, "b");
                    D_887a = 0;
                    sub_root_047c_04dc(&D_27f4, 0x95, row * 8 + 158, 0xa9, 8, 8, 9);
                } else {
                    sub_root_047c_04dc(&D_27f4, 0x95, row * 8 + 158, 0xa9, 8, 8, 9);
                }
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 148 && D_6eec < 318 && D_6eee > 158 && D_6eee < 190) {
            while (D_6ef0 & 1)
                ;
            switch (row = (D_6eee - 158) / 8) {
            case 0: key = 0x324d; break;
            case 1: key = 0x264c; break;
            case 2: key = 0x1950; break;
            case 3: key = 0x2044; break;
            }
        } else if (D_887e) {
            sub_root_047c_12c3();
            D_887e = 0;
            sub_root_047c_05a5(&D_27f4, 0xfa, 0x8e, D_887a);
            sub_root_05ba_00c2(D_887a, "b");
            D_887a = 0;
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eec < 60 && (t = D_6eee / 40) == ch) {
            if (D_887a)
                sub_root_05ba_00c2(D_887a, "b");
            return -1;
        }
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_047c_0568();
            saint_load_bg(ch, saint);
            saint_draw(ch, saint);
            sub_root_047c_056f();
            key = 0;
        }
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if (key == 0x11b || key == 0x2044 || key == 0x2064)
            return -1;
        if ((key == 0x1950 || key == 0x1970) && F_290e_2937[saint](ch, D_887c, 7)) {
            D_89ea = -1;
            return saint_finish(ch, saint);
        }
        if (key)
            saint_key(key, ch, saint);
    }
}

/* Leave the screen and ask the saint's handler for the success chance. */
int far saint_finish(int ch, int saint)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    if (D_887a)
        sub_root_05ba_00c2(D_887a, "b");
    return F_290e_2937[saint](ch, D_887c, 7);
}

/* Load the saint's background picture and description from darkland.snt. */
void far saint_load_bg(int ch, int saint)
{
    struct find_t ft;
    FILE *f;

    switch (F_290e_2937[saint](0, 0, 1)) {
    case 0: sub_root_0617_024a(1, "pics\\sntback0.pic", D_e48c); break;
    case 1: sub_root_0617_024a(1, "pics\\sntback1.pic", D_e48c); break;
    case 2: sub_root_0617_024a(1, "pics\\sntback2.pic", D_e48c); break;
    case 3: sub_root_0617_024a(1, "pics\\sntback3.pic", D_e48c); break;
    case 4: sub_root_0617_024a(1, "pics\\sntback4.pic", D_e48c); break;
    case 5: sub_root_0617_024a(1, "pics\\sntback5.pic", D_e48c); break;
    case 6: sub_root_0617_024a(1, "pics\\sntback6.pic", D_e48c); break;
    case 7: sub_root_0617_024a(1, "pics\\sntback7.pic", D_e48c); break;
    case 8: sub_root_0617_024a(1, "pics\\sntback8.pic", D_e48c); break;
    case 9: sub_root_0617_024a(1, "pics\\sntback9.pic", D_e48c); break;
    case 10: sub_root_0617_024a(1, "pics\\sntback0.pic", D_e48c); break;
    case 11: sub_root_0617_024a(1, "pics\\sntback0.pic", D_e48c); break;
    case 12: sub_root_0617_024a(1, "pics\\sntback0.pic", D_e48c); break;
    case 13: sub_root_0617_024a(1, "pics\\sntback0.pic", D_e48c); break;
    default: sub_root_0617_024a(1, "pics\\sntback1.pic", D_e48c); break;
    }
    if (_dos_findfirst("darkland.snt", 0, &ft) != 0) {
        printf("saint file\n");
        exit_(1);
    } else {
        f = fopen("darkland.snt", "r");
        fgetc(f);
        fseek(f, saint * sizeof(D_8882) + 1, 0);
        fread(D_8882, sizeof(D_8882), 1, f);
        fclose(f);
    }
    D_2800 = 8;
    D_2816 = 2;
    D_2804 = 2;
    sub_root_047c_058b(D_e48c);
}

/* Redraw the whole saint screen. */
void far saint_draw(int ch, int saint)
{
    sub_root_047c_12c3();
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 0xc8, &D_27f4, 0x3c, 0);
    saint_draw_df(ch, saint, &D_27f4);
    saint_draw_menu(ch, saint, &D_27f4);
    saint_draw_info(ch, saint, &D_27f4);
    sub_root_047c_1293();
}

/* Keyboard: M/+ spends more favor, L/- less, P prays for the party, Alt-Q quits. */
void far saint_key(int key, int ch, int saint)
{
    int unused;     /* never used, but the original reserves its stack slot */

    switch (key) {
    case 0x0c2d: case 0x264c: case 0x266c: case 0x4a2d:
        if (--D_887c < 0) {
            D_887c = 0;
            return;
        }
        saint_draw_df(ch, saint, &D_27f4);
        break;
    case 0x0d2b: case 0x324d: case 0x326d: case 0x4e2b:
        if (F_290e_2937[saint](ch, D_887c, 7) < 99) {
            if (++D_887c > D_9c69[ch].maxdf)
                D_887c = D_9c69[ch].maxdf;
            saint_draw_df(ch, saint, &D_27f4);
        }
        break;
    case 0x1000:
        saint_finish(ch, saint);
        sub_root_0000_13a0();
        break;
    case 0x2044: case 0x2064:
        saint_draw_menu(ch, saint, &D_27f4);
        break;
    }
}

/* Divine favor remaining and the current chance of success. */
void far saint_draw_df(int ch, int saint, struct gfx *g)
{
    char buf[120];

    sub_root_047c_12c3();
    if (D_887e) {
        D_887e = 0;
        sub_root_047c_05a5(&D_27f4, 0xfa, 0x8e, D_887a);
        sub_root_05ba_00c2(D_887a, "b");
        D_887a = 0;
    }
    sub_root_047c_0553(&D_2806, 0x9f, 0x93, 0x97, 8, &D_27f4, 0x9f, 0x93);
    g->color = 8;
    sprintf(buf, "DF Remaining: %02d", D_9c69[ch].maxdf - D_887c);
    sub_root_047c_055a(g, 0xdb, 0x93, buf);
    sprintf(buf, "Success: %d", sub_s029_1367_0066(0, 99, F_290e_2937[saint](ch, D_887c, 7)));
    sub_root_047c_055a(g, 0x9f, 0x93, buf);
    sub_root_047c_1293();
}

/* The four menu lines. */
void far saint_draw_menu(int ch, int saint, struct gfx *g)
{
    char buf[80];

    sub_root_047c_12c3();
    if (D_887e) {
        if (D_887a)
            sub_root_047c_05a5(&D_27f4, 0xfa, 0x8e, D_887a);
        if (D_887a)
            sub_root_05ba_00c2(D_887a, "b");
        D_887a = 0;
        D_887e = 0;
    }
    g->color = 8;
    sprintf(buf, "\214 M\377ore divine favor is spent");
    sub_root_047c_055a(g, 0x94, 0x9e, buf);
    sprintf(buf, "\214 L\377ess divine favor is spent");
    sub_root_047c_055a(g, 0x94, 0xa6, buf);
    g->color = F_290e_2937[saint](ch, D_887c, 7) ? 8 : 7;
    if (!D_89ec)
        sprintf(buf, "\214 P\377ray now for the entire party");
    else
        sprintf(buf, "\214 P\377ray now for ...");
    sub_root_047c_055a(g, 0x94, 0xae, buf);
    g->color = 8;
    sprintf(buf, "\214 D\377on't pray right now");
    sub_root_047c_055a(g, 0x94, 0xb6, buf);
    sub_root_047c_1293();
}

/* Saint's name, feast day and description. */
void far saint_draw_info(int ch, int saint, struct gfx *g)
{
    char buf[80];
    unsigned w;
    int day;
    unsigned x;

    sub_root_047c_12c3();
    sprintf(buf, "%Fs", F_2e38_183e[saint]);
    sub_root_0265_084e(g, buf);
    if (D_907a == 1)
        sub_root_0265_0134(g, buf, 0x9a, 0x94, 4, 8, 0, 0, 0, 1, 0, 0);
    else
        sub_root_0265_0134(g, buf, 0x9a, 0x94, 4, 8, 0, 0, 0, 1, 1, 0);
    sub_root_047c_055a(g, 0x4c, 0xad, "Saint Day");
    sub_root_047c_055a(g, 0x62, 0xb5, "is");
    day = F_290e_2937[saint](ch, 0, 0);
    sprintf(buf, "%Fs %d", F_290e_1d43[sub_s029_1367_0938(day)], sub_s029_1367_0982(day));
    w = sub_root_0265_084e(g, buf);
    x = w;
    if (w >= 49)
        x = 50;
    sub_root_047c_055a(g, (50 - x) / 2 + 75, 0xbd, buf);
    sub_root_0265_0134(g, D_8882, 0x9c, 0x94, 0x25, 8, 0, 0, 0, 1, 0, 0);
    sub_root_047c_1293();
}

/* Framed text box to the right of the menu. */
void far saint_text_box(char *text, int h)
{
    int color;
    int x2;

    color = sub_root_047c_051b(D_2804);
    if (D_887a)
        sub_root_05ba_00c2(D_887a, "b");
    sub_root_047c_0545();
    D_887a = sub_root_047c_04d5(0, 0xfa, 0x8e, h, 0x32);
    sub_root_047c_04f8();
    sub_root_047c_0952(&D_27f4, 0xfa, 0x8e, h, 0x32, 0xff);
    x2 = h + 0xf9;
    sub_root_047c_0d62(&D_27f4, 0xfa, 0x8e, x2, 0x8e, 1);
    sub_root_047c_0d62(&D_27f4, x2, 0x8e, x2, 0xbf, 1);
    sub_root_047c_0d62(&D_27f4, x2, 0xbf, 0xfa, 0xbf, 1);
    sub_root_047c_0d62(&D_27f4, 0xfa, 0xbf, 0xfa, 0x8e, 1);
    D_2800 = 8;
    sub_root_0265_0134(&D_27f4, text, h - 8, 0xff, 0x91, color, 0, 0, 0, 0, 0, 0);
}

/* List the party members' names, one per line, in a text box. */
void far saint_party_box(char *buf)
{
    char tmp[120];
    int i;

    *buf = 0;
    for (i = 0; i < D_a67e; i++) {
        if (D_9c69[i].used) {
            strcpy(tmp, D_9c69[i].name);
            tmp[strlen(tmp) + 1] = 0;
            strcat(tmp, "\n");
            strcat(buf, tmp);
        }
    }
    if (*buf)
        buf[strlen(buf) - 1] = 0;
    saint_text_box(buf, 60);
}
