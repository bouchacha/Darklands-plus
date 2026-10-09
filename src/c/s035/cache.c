/* place: s035 1462:0000 */
/* orig: size=17c0 CONST=7e94+0008 _DATA=37ba+0272 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* data: _DATA=37ba */
/* data: CONST=7e94 */
/* Item cache screen (overlay s035): get / put items between a party member and a
   cache stored in the temporary file "cache.tmp". */

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};
typedef struct _iobuf FILE;
#define FP_SEG(fp) (*((unsigned _far *)&(fp) + 1))
#define MK_FP(seg, off) ((unsigned char far *)(((unsigned long)(seg) << 16) | (off)))

/* party member record in DGROUP (0x80 bytes) */
struct member {
    char present;               /* +00 */
    char pad01[0x29];
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
    char f1e;
    char pad1f[6];
    char f25;
};

/* cache entry */
struct cent {
    int id;
    char qual;
    char cnt;
};

struct loc { char pad[0x18]; int cache; };

extern int D_2816, D_2804, D_e7a2, D_e7a0;
extern char D_907c, D_907b;
extern int D_89f2, D_89ee, D_89f8, D_907e;
extern volatile int D_2800;          /* text colour */
extern volatile int D_6ef0, D_6eec, D_6eee;   /* mouse buttons, x, y */
extern char D_27f4[], D_2806[], D_e48c[], D_9419[];
extern int D_906b, D_906d, D_906f;
extern unsigned D_89f6;
extern int D_89f0;
extern int D_a67e;
extern struct member D_9c69[];

extern struct loc far * far F_2e38_10be[];
extern struct fchar far F_2e38_0000[];
extern struct itemdef far * far F_2e38_0890[];
extern struct cent far F_2e38_37f4[];

/* Module data (DGROUP 37ba..382a), per party member [5].  D_37e2/D_37ce are declared
   first: MSC orders the operands of D_37ce[x] + D_37e2[x] by declaration order. */
static int D_37e2[5], D_37ce[5];
static int D_37ba[5] = {0};     /* number of items the member carries */
static int D_37c4[5] = {0};     /* active list: 0 = member's items, 1 = cache */
static int D_37ce[5] = {0};     /* member list: first visible item */
static int D_37d8[5] = {0};     /* cache list: first visible entry */
static int D_37e2[5] = {0};     /* member list: cursor row */
static int D_37ec[5] = {0};     /* cache list: cursor row */
static char str_get[] = "\x8c G\xff" "et an item from cache";
static char str_put[] = "\x8c P\xff" "ut an item into cache";

int sprintf(char *, const char *, ...);
int printf(const char *, ...);
void far exit_(int);
int _dos_findfirst(char *, unsigned, struct find_t *);
unsigned _dos_write(int, void far *, unsigned, unsigned *);
FILE *fdopen(int, char *);
int fgetc(FILE *);
int fputc(int, FILE *);
int fseek(FILE *, long, int);
long ftell(FILE *);
int fclose(FILE *);

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
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_0617_011e(char *, void far *);
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
unsigned far sub_root_06a1_2c9e(char *, unsigned, int *);   /* _dos_open */
int far vec_1fdb(int);
int far vec_2003(int, int);
int far vec_1f6d(int, struct item);

int far cache_close(int ret);
void far draw_cache_screen(int who, int unused);
void far redraw_cache_screen(int who, int unused);
void far cache_key(int key);
void far cache_add(int id, int who, int qual);
void far cache_take(unsigned idx);
void far draw_member_items(int who);
void far draw_cache_items(int unused);
void far cache_load(int handle);
void far cache_save(int handle);

/* cache_screen: main loop of the item cache screen */
void far cache_screen(int arg)
{
    int mkey;
    int hover;
    int row;

    hover = -1;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    D_907c = 5;
    D_89ee = D_89f2 = D_907b;
    D_2800 = 8;
    D_89f8 = arg;
    sub_root_047c_0568();
    draw_cache_screen(D_907b, arg);
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
                sub_root_047c_04dc(D_27f4, 0x50, row * 8 + 0x1b, 0x96, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0x50 && D_6eec < 0xe6 && D_6eee > 0x1b && D_6eee < 0x43) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x1b) / 8;
            switch (row) {
            case 0: mkey = 0x2247; break;
            case 1: mkey = 0x1950; break;
            case 2: mkey = 0x2e43; break;
            case 3: mkey = 0x11b; break;
            }
        } else if (!(D_6ef0 & 1) && hover != -1) {
            sub_root_047c_12c3();
            sub_root_047c_04dc(D_27f4, 0x50, hover * 8 + 0x1b, 0x96, 8, 9, 8);
            hover = -1;
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eec > 0x72 && D_6eec < 0x11c && D_6eee > 0x93 && D_6eee < 0xb3) {
            sub_root_047c_12c3();
            D_37c4[D_89f2] = 0;
            D_37e2[D_89f2] = (D_6eee - 0x93) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eec > 0x52 && D_6eec < 0x10b && D_6eee > 0x55 && D_6eee < 0x75) {
            sub_root_047c_12c3();
            D_37c4[D_89f2] = 1;
            D_37ec[D_89f2] = (D_6eee - 0x55) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x8a && D_6eee < 0x93 && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x4800;
            D_37c4[D_89f2] = 0;
        }
        if ((D_6ef0 & 1) && D_6eee > 0xb3 && D_6eee < 0xbc && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x5000;
            D_37c4[D_89f2] = 0;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x4c && D_6eee < 0x55 && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x4800;
            D_37c4[D_89f2] = 1;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x75 && D_6eee < 0x7e && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            mkey = 0x5000;
            D_37c4[D_89f2] = 1;
        }
        if ((D_6ef0 & 1) && D_6eec < 0x3c) {
            row = D_6eee / 40;
            while (row != -1) {
                row = vec_1fdb(row);
                sub_root_0000_1588(0x100);
                sub_root_0000_1716(0x100);
            }
            redraw_cache_screen(D_89f2, arg);
            while (D_6ef0 & 1)
                ;
        }
        sub_root_0000_0000(D_9419, 0);
        if (sub_root_047c_073d())
            mkey = sub_root_047c_0744();
        if (mkey == 0x11b)
            break;
        if (mkey)
            cache_key(mkey);
    }
    cache_close(-1);
}

/* cache_close: write the cache back (or mark the location as having none) */
int far cache_close(int ret)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    if (D_89f6 != 0)
        cache_save(F_2e38_10be[D_907e]->cache);
    else
        F_2e38_10be[D_907e]->cache = -1;
    D_907b = D_89ee;
    return ret;
}

/* draw_cache_screen: draw the whole screen and load the cache contents */
void far draw_cache_screen(int who, int unused)
{
    char buf[120];
    int y;
    int i;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_root_047c_0553(D_2806, 0x3c, 0, 0x104, 0xc8, D_27f4, 0x3c, 0);
    D_2800 = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf.", D_9c69[D_907b].name, D_906b, D_906d, D_906f);
    sub_root_047c_055a(D_27f4, 0x49, 9, buf);
    D_2800 = 7;
    sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
    D_2800 = 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
    sub_root_047c_055a(D_27f4, 0x50, 0x2b, "\x8c C\xff" "ache another person's items");
    sub_root_047c_055a(D_27f4, 0x50, 0x33, "\x8c L\xff" "eave");
    sprintf(buf, "The cache contains...");
    sub_root_047c_055a(D_27f4, 0x6b, 0x45, buf);
    sprintf(buf, "%s currently has...", D_9c69[who].name);
    sub_root_047c_055a(D_27f4, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < D_9c69[who].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, y, buf);
    }
    cache_load(F_2e38_10be[D_907e]->cache);
    for (i = 0; i < D_89f6 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        sub_root_047c_055a(D_27f4, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    sub_root_047c_04dc(D_27f4, 0x73, 0x93, 0xb7, 8, 8, 9);
    D_37ba[D_89f2] = D_9c69[who].nitems;
    D_89f0 = D_89f6;
    D_2800 = 8;
    sub_root_047c_1293();
}

/* redraw_cache_screen: redraw the screen for another party member */
void far redraw_cache_screen(int who, int unused)
{
    char buf[120];
    int y;
    int i;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_root_047c_0553(D_2806, 0x3c, 0, 0x104, 0xc8, D_27f4, 0x3c, 0);
    D_2800 = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf.", D_9c69[D_907b].name, D_906b, D_906d, D_906f);
    sub_root_047c_055a(D_27f4, 0x49, 9, buf);
    D_2800 = D_37c4[D_89f2] ? 8 : 7;
    sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
    D_2800 = D_37c4[D_89f2] ? 7 : 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
    D_2800 = D_37c4[D_89f2] ? 8 : 7;
    D_2800 = 8;
    sub_root_047c_055a(D_27f4, 0x50, 0x2b, "\x8c C\xff" "ache another person's items");
    sub_root_047c_055a(D_27f4, 0x50, 0x33, "\x8c L\xff" "eave");
    sprintf(buf, "The cache contains...");
    sub_root_047c_055a(D_27f4, 0x57, 0x45, buf);
    sprintf(buf, "%s has...", D_9c69[who].name);
    sub_root_047c_055a(D_27f4, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < D_9c69[who].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, y, buf);
    }
    for (i = 0; i < D_89f6 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        sub_root_047c_055a(D_27f4, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    if (D_37c4[D_89f2])
        sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb7, 8, 8, 9);
    else
        sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb7, 8, 8, 9);
    D_37ba[D_89f2] = D_9c69[who].nitems;
    D_89f0 = D_89f6;
    D_2800 = 8;
    sub_root_047c_1293();
}

/* cache_key: handle one key press on the cache screen */
void far cache_key(int key)
{
    struct item it;

    switch (key) {
    case 0x1000:
        cache_close(-1);
        sub_root_0000_13a0();
        break;
    case 0x1950:
    case 0x1970:            /* P: put an item into the cache */
        if (D_37c4[D_89f2] != 0)
            break;
        if (D_9c69[D_89f2].nitems == 0)
            break;
        cache_add(F_2e38_0000[D_89f2].items[D_37e2[D_89f2] + D_37ce[D_89f2]].id & 0xfff, D_89f2,
                  F_2e38_0000[D_89f2].items[D_37e2[D_89f2] + D_37ce[D_89f2]].qual);
        D_37ba[D_89f2] = vec_2003(D_89f2, D_37e2[D_89f2] + D_37ce[D_89f2]);
        if (D_37e2[D_89f2] + D_37ce[D_89f2] >= D_37ba[D_89f2]) {
            if (D_37ce[D_89f2] != 0)
                D_37ce[D_89f2]--;
            else if (D_37e2[D_89f2] != 0)
                D_37e2[D_89f2]--;
        }
        draw_cache_items(D_907e);
        draw_member_items(D_89f2);
        sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        break;
    case 0x2044:
    case 0x2064:            /* D */
        if (D_37c4[D_89f2] != 0)
            break;
        D_37ba[D_89f2] = vec_2003(D_907b, D_37e2[D_89f2] + D_37ce[D_89f2]);
        draw_member_items(D_89f2);
        sub_root_047c_04dc(D_27f4, 0x50, D_37e2[D_89f2] * 8 + 0x1b, 0x5f, 8, 8, 9);
        break;
    case 0x2247:
    case 0x2267:            /* G: get an item from the cache */
        if (D_37c4[D_89f2] == 0)
            break;
        if (D_89f0 == 0)
            break;
        it.id = F_2e38_37f4[D_37d8[D_89f2] + D_37ec[D_89f2]].id;
        it.f2 = F_2e38_0890[it.id]->f1e;
        it.qual = F_2e38_37f4[D_37d8[D_89f2] + D_37ec[D_89f2]].qual;
        it.f5 = F_2e38_0890[it.id]->f25;
        it.cnt = 1;
        D_37ba[D_89f2] = vec_1f6d(D_89f2, it);
        cache_take(D_37d8[D_89f2] + D_37ec[D_89f2]);
        if (D_37d8[D_89f2] + D_37ec[D_89f2] >= D_89f0) {
            if (D_37d8[D_89f2] != 0)
                D_37d8[D_89f2]--;
            else if (D_37ec[D_89f2] != 0)
                D_37ec[D_89f2]--;
        }
        draw_cache_items(D_907e);
        draw_member_items(D_89f2);
        sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0x5f, 8, 8, 9);
        break;
    case 0x2e43:
    case 0x2e63:            /* C: cache another person's items */
        sub_root_0000_1588(0x100);
        sub_root_0000_1716(0x100);
        do {
            D_89f2++;
            D_89f2 %= D_a67e;
        } while (D_9c69[D_89f2].present == 0);
        redraw_cache_screen(D_89f2, D_89f8);
        sub_root_047c_04dc(D_27f4, 0x50, 0x33, 0xc8, 9, 8, 9);
        break;
    case 0x4800:            /* up */
        sub_root_047c_12c3();
        if (D_37c4[D_89f2])
            D_37ec[D_89f2]--;
        else
            D_37e2[D_89f2]--;
        if (D_37c4[D_89f2] && D_37ec[D_89f2] < 0 && D_37d8[D_89f2] > 0) {
            D_37ec[D_89f2]++;
            D_37d8[D_89f2]--;
            draw_cache_items(D_907e);
            sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_37c4[D_89f2] && D_37e2[D_89f2] < 0 && D_37ce[D_89f2] > 0) {
            D_37e2[D_89f2]++;
            D_37ce[D_89f2]--;
            draw_member_items(D_89f2);
            sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_37ec[D_89f2] < 0)
                D_37ec[D_89f2] = 0;
            if (D_37e2[D_89f2] < 0)
                D_37e2[D_89f2] = 0;
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (D_37c4[D_89f2])
                sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xc3, 8, 8, 9);
            else
                sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        sub_root_047c_1293();
        break;
    case 0x4b00:
    case 0x4d00:            /* left / right: switch lists */
        sub_root_047c_12c3();
        if (D_37c4[D_89f2]) {
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
            sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
            D_37c4[D_89f2] = 0;
        } else {
            if (D_89f0 == 0)
                break;
            D_2800 = 7;
            sub_root_047c_055a(D_27f4, 0x50, 0x23, str_put);
            D_2800 = 8;
            sub_root_047c_055a(D_27f4, 0x50, 0x1b, str_get);
            sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 9, 8);
            sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
            D_37c4[D_89f2] = 1;
        }
        sub_root_047c_1293();
        break;
    case 0x5000:            /* down */
        sub_root_047c_12c3();
        if (D_37c4[D_89f2])
            D_37ec[D_89f2]++;
        else
            D_37e2[D_89f2]++;
        if (D_37c4[D_89f2] && D_37ec[D_89f2] >= 4 && D_37d8[D_89f2] + D_37ec[D_89f2] < D_89f0) {
            D_37ec[D_89f2]--;
            D_37d8[D_89f2]++;
            draw_cache_items(D_907e);
            sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_37c4[D_89f2] && D_37e2[D_89f2] >= 4 && D_37ce[D_89f2] + D_37e2[D_89f2] < D_37ba[D_89f2]) {
            D_37e2[D_89f2]--;
            D_37ce[D_89f2]++;
            draw_member_items(D_89f2);
            sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_37ec[D_89f2] >= D_89f0)
                D_37ec[D_89f2]--;
            if (D_37ba[D_89f2] <= D_37e2[D_89f2])
                D_37e2[D_89f2]--;
            if (D_37ec[D_89f2] >= 4)
                D_37ec[D_89f2] = 3;
            if (D_37e2[D_89f2] >= 4)
                D_37e2[D_89f2] = 3;
            sub_root_047c_04dc(D_27f4, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (D_37c4[D_89f2])
                sub_root_047c_04dc(D_27f4, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xc8, 8, 8, 9);
            else
                sub_root_047c_04dc(D_27f4, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        sub_root_047c_1293();
        break;
    }
}

/* cache_add: add one item to the cache (stack onto an equal entry if possible) */
void far cache_add(int id, int who, int qual)
{
    unsigned i;

    for (i = 0; i < D_89f6; i++) {
        if (F_2e38_37f4[i].id == id && F_2e38_37f4[i].qual == qual) {
            F_2e38_37f4[i].cnt++;
            return;
        }
    }
    D_89f0 = ++D_89f6;
    F_2e38_37f4[i].id = id;
    F_2e38_37f4[i].qual = qual;
    F_2e38_37f4[i].cnt = 1;
}

/* cache_take: remove one item from cache entry idx */
void far cache_take(unsigned idx)
{
    unsigned i;

    if (--F_2e38_37f4[idx].cnt == 0) {
        for (i = idx; i < D_89f6; i++) {
            F_2e38_37f4[i].id = F_2e38_37f4[i + 1].id;
            F_2e38_37f4[i].qual = F_2e38_37f4[i + 1].qual;
            F_2e38_37f4[i].cnt = F_2e38_37f4[i + 1].cnt;
        }
        D_89f0 = --D_89f6;
    }
}

/* draw_member_items: list four of the member's items */
void far draw_member_items(int who)
{
    char buf[60];
    int i;

    sub_root_047c_12c3();
    sub_root_047c_0553(D_2806, 0x72, 0x93, 0xb6, 0x20, D_27f4, 0x72, 0x93);
    for (i = D_37ce[D_89f2]; i < D_37ce[D_89f2] + 4 && i < D_9c69[who].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2dq", F_2e38_0890[F_2e38_0000[who].items[i].id & 0xfff],
                F_2e38_0000[who].items[i].cnt, F_2e38_0000[who].items[i].qual);
        sub_root_047c_055a(D_27f4, 0x75, (i - D_37ce[D_89f2]) * 8 + 0x93, buf);
    }
    sub_root_047c_1293();
}

/* draw_cache_items: list four of the cache's entries */
void far draw_cache_items(int unused)
{
    char buf[120];
    int i;

    sub_root_047c_12c3();
    sub_root_047c_0553(D_2806, 0x52, 0x55, 0xb6, 0x20, D_27f4, 0x52, 0x55);
    for (i = D_37d8[D_89f2]; i < D_37d8[D_89f2] + 4 && i < D_89f0; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        sub_root_047c_055a(D_27f4, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    sub_root_047c_1293();
}

/* cache_load: read cache number `handle` from cache.tmp into the cache table */
void far cache_load(int handle)
{
    struct find_t ft;
    unsigned char far *buf;
    int i;
    unsigned char far *p;
    unsigned char far *q;

    if (handle == -1) {
        D_89f6 = 0;
        return;
    }
    if (_dos_findfirst("cache.tmp", 0, &ft)) {
        printf("No cache file\n");
        exit_(1);
        return;
    }
    p = buf = q = MK_FP(alloc_buffer(ft.size, "cache mem"), 0);
    sub_root_0617_011e("cache.tmp", buf);
    /* header: byte 0, byte 1 = number of caches, then a 16-bit offset per cache */
    q++;
    q += handle * 2 - 1;
    p += *q;
    p += *++q << 8;
    D_89f6 = *p++;
    for (i = 0; i < D_89f6; i++) {
        F_2e38_37f4[i].id = *p++;
        F_2e38_37f4[i].id += *p++ << 8;
        F_2e38_37f4[i].qual = *p++;
        F_2e38_37f4[i].cnt = *p++;
    }
    free_buffer(FP_SEG(buf), "Cache");
}

/* cache_save: write the cache table back into cache.tmp; handle -1 appends it as a new cache
   and stores the new handle in the current location */
void far cache_save(int handle)
{
    struct find_t ft;
    long pos;
    int k;
    FILE *fp;
    unsigned ncache;
    unsigned i;
    int fh;
    unsigned n;
    unsigned nw;
    char far *p;
    unsigned char far *buf;
    unsigned char far *q;

    if (_dos_findfirst("cache.tmp", 0, &ft)) {
        printf("No cache file\n");
        exit_(1);
    } else if (handle == -1) {
        sub_root_06a1_2c9e("cache.tmp", 2, &fh);
        fp = fdopen(fh, "r+");
        fgetc(fp);
        ncache = fgetc(fp);
        fseek(fp, 0L, 2);
        pos = ftell(fp);
        fputc(D_89f0, fp);
        fseek(fp, 0L, 2);
        _dos_write(fh, F_2e38_37f4, D_89f0 << 2, &nw);
        fseek(fp, (long)((ncache + 1) * 2), 0);
        fputc((int)pos, fp);
        fputc((int)(pos >> 8), fp);
        fseek(fp, 1L, 0);
        fputc(++ncache, fp);
        fclose(fp);
        F_2e38_10be[D_907e]->cache = ncache;
    } else {
        p = buf = q = MK_FP(alloc_buffer(ft.size, "cache mem"), 0);
        sub_root_0617_011e("cache.tmp", buf);
        sub_root_06a1_2c9e("cache.tmp", 1, &fh);
        fp = fdopen(fh, "wb");
        n = *q++;
        ncache = *q++;
        p += (char)*q;
        fputc(n, fp);
        fputc(ncache, fp);
        for (i = 0; i < n; i++)
            fputc(0, fp);
        for (i = 0; i < ncache; i++) {
            if (handle - i == 1) {
                pos = ftell(fp);
                fseek(fp, (long)((i + 1) * 2), 0);
                fputc((int)pos, fp);
                fputc((int)(pos >> 8), fp);
                fseek(fp, pos, 0);
                fputc(D_89f0, fp);
                fseek(fp, 0L, 1);
                _dos_write(fh, F_2e38_37f4, D_89f0 << 2, &nw);
                k = *p++;
                p += k << 2;
            } else {
                k = *p;
                pos = ftell(fp);
                fseek(fp, (long)((i + 1) * 2), 0);
                fputc((int)pos, fp);
                fputc((int)(pos >> 8), fp);
                fseek(fp, pos, 0);
                _dos_write(fh, p, (k << 2) + 1, &nw);
                p += (k << 2) + 1;
            }
        }
        free_buffer(FP_SEG(buf), "Cache");
        fclose(fp);
    }
}
