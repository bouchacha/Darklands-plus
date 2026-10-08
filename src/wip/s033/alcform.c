/* place: s033 15d1:000a */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Alchemy formula viewer (overlay s033, segment 15d1): loads DARKLAND.ALC, counts the
   party's ingredients and shows one formula page with its ingredient list. */

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

struct gfx { char pad[12]; int color; };

struct ingred { char count; char pad; int item; };
struct formula {
    char name[0x50];
    int mystic;                 /* +50 "Mystical Number" */
    int x52;
    struct ingred ingr[5];      /* +54 */
};                              /* 0x68 bytes */

struct item { unsigned id; char x2[2]; unsigned char qty; char x5; };
struct invent { struct item it[64]; };          /* 0x180 bytes per character */
struct chr { char nitems; char pad[0x7f]; };    /* DGROUP 9cd3, 0x80 bytes per character */

extern volatile char D_6ef0;
extern int D_2816, D_2804, D_e7a2, D_e7a0, D_9c46;
extern int D_a67e;
extern char D_e48c[];
extern struct gfx D_2806, D_27f4;
extern unsigned char far *D_9c2b;
extern int D_e7a6[];
extern struct chr D_9cd3[];

extern char far *far F_2e38_1736[];
extern struct formula far *far F_2e38_35e4[];
extern char far *far F_2e38_0890[];
extern struct invent far F_2e38_003e[];

int printf(const char *, ...);
int sprintf(char *, const char *, ...);
unsigned _dos_findfirst(const char *, unsigned, struct find_t *);
void far exit_(int);
void *memset(void *, int, unsigned);

void far sub_root_0000_0000(int, int);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_0617_011e(char *, unsigned char far *);
unsigned far sub_root_05ba_0004(long, char *);
void far sub_root_05ba_00c2(unsigned, char *);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_058b(char *);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_0000_13a0(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_055a(struct gfx *, int, int, char *);
void far sub_root_047c_0553(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far sub_root_0265_0134(struct gfx *, char *, int, int, int, int, int, int, int, int, int, int);

int far alc_leave(int r);
void far alc_load(int a, int idx);
void far alc_draw(int a, int idx);
void far alc_key(int key);
void far alc_draw_names(int idx, struct gfx *g);
void far alc_draw_ingredients(int idx, struct gfx *g);
void far alc_draw_extra(int idx, struct gfx *g);

/* alc_view: formula viewer main loop; Esc (0x11b) leaves */
void far alc_view(int a, int idx)
{
    int key;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    alc_load(a, idx);
    alc_draw(a, idx);
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (D_6ef0 & 1) {
            key = 0x11b;
            while (D_6ef0 & 1)
                ;
        }
        sub_root_0000_0000(0x9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_0617_024a(1, "pics\\alcscrn0.pic", D_e48c);
            sub_root_047c_0568();
            sub_root_047c_058b(D_e48c);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            alc_draw(a, idx);
            sub_root_047c_056f();
            key = 0;
        }
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if (key == 0x11b)
            break;
        if (key)
            alc_key(key);
    }
    alc_leave(-1);
}

/* alc_leave: restore the saved state and free the formula file */
int far alc_leave(int r)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    sub_root_05ba_00c2((unsigned)((unsigned long)D_9c2b >> 16), "alc");
    return r;
}

/* alc_load: read DARKLAND.ALC, index its formulas and count the party's ingredients */
void far alc_load(int a, int idx)
{
    char msg[] = "alchemy file\n";
    struct find_t ff;
    unsigned char far *p;
    int i, j, n;

    sub_root_0617_024a(1, "pics\\alcscrn0.pic", D_e48c);
    if (_dos_findfirst("darkland.alc", 0, &ff)) {
        printf(msg);
        exit_(1);
    } else {
        p = (unsigned char far *)((unsigned long)sub_root_05ba_0004(ff.size, "alc mem") << 16);
        D_9c2b = p;
        sub_root_0617_011e("darkland.alc", p);
        n = *p++;
        for (i = 0; i < n; i++) {
            F_2e38_35e4[i] = (struct formula far *)p;
            p += 0x68;
        }
    }
    memset(D_e7a6, 0, 0x2e);
    for (i = 0; i < D_a67e; i++) {
        for (j = 0; j < D_9cd3[i].nitems; j++) {
            if (F_2e38_003e[i].it[j].id >= 0x48 && F_2e38_003e[i].it[j].id <= 0x5f)
                D_e7a6[F_2e38_003e[i].it[j].id - 0x48] += F_2e38_003e[i].it[j].qty;
        }
    }
    D_2806.color = 8;
    sub_root_047c_058b(D_e48c);
}

/* alc_draw: draw the page for formula idx */
void far alc_draw(int a, int idx)
{
    char buf[120];
    sub_root_047c_12c3();
    D_2806.color = 0xe;
    sprintf(buf, "%Fs", F_2e38_1736[idx]);
    sub_root_047c_055a(&D_2806, 100, 9, buf);
    sprintf(buf, "Mystical Number - %d", F_2e38_35e4[idx]->mystic);
    sub_root_047c_055a(&D_2806, 100, 0x12, buf);
    alc_draw_names(idx, &D_2806);
    alc_draw_ingredients(idx, &D_2806);
    alc_draw_extra(idx, &D_2806);
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 200, &D_27f4, 0x3c, 0);
    sub_root_047c_1293();
}

/* alc_key: key handler */
void far alc_key(int key)
{
    switch (key) {
    case 0x1000:
        alc_leave(-1);
        sub_root_0000_13a0();
    }
}

void far alc_draw_names(int idx, struct gfx *g)
{
    char buf[120];
    g->color = 8;
    sprintf(buf, "%Fs", F_2e38_1736[idx]);
    sub_root_0265_0134(g, buf, 0x5f, 0x49, 0x3a, 8, 0, 0, 0, 1, 0, 0);
    sprintf(buf, "%Fs", F_2e38_35e4[idx]->name);
    sub_root_0265_0134(g, buf, 0x5f, 0x49, 0x52, 8, 0, 0, 0, 1, 0, 0);
}

/* alc_draw_ingredients: list the five ingredients, grey when the party has too few */
void far alc_draw_ingredients(int idx, struct gfx *g)
{
    char buf[40];
    int k, y;
    g->color = 8;
    sub_root_047c_055a(g, 0xcb, 0x39, "Ingredients:");
    y = 0x41;
    for (k = 0; k < 5; k++) {
        if (F_2e38_35e4[idx]->ingr[k].count) {
            sprintf(buf, "%d %Fs (%d)", F_2e38_35e4[idx]->ingr[k].count,
                    F_2e38_0890[F_2e38_35e4[idx]->ingr[k].item],
                    D_e7a6[F_2e38_35e4[idx]->ingr[k].item - 0x48]);
            g->color = D_e7a6[F_2e38_35e4[idx]->ingr[k].item - 0x48] < F_2e38_35e4[idx]->ingr[k].count ? 7 : 8;
            sub_root_047c_055a(g, 0xcd, y, buf);
        }
        y += 8;
    }
}

void far alc_draw_extra(int idx, struct gfx *g)
{
}
