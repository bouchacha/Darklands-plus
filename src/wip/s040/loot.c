/* place: s040 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* e0 2026-10-09: functions renamed back to their original sub_ names (descriptive names in the comments); still 8/10. */
/* data: _DATA=4bbc */ /* data: CONST=7edc */
/* Loot distribution: share found items and cash between the party (overlay s040). */
/*
 * STATUS (e0, 2026-10-09 s3): 9/10 functions match; 01c8 matched by reading cash[0] for
 *   sub_s029_1367_0130 through a cast (`*(unsigned far *)cash`), which 6.00A does not
 *   constant-propagate.
 * Old STATUS (2026-10-08): 8/10 functions match (check: python3 -I tools/fmatch.py src/wip/s040/loot.c).
 *   sub_s040_1462_0000 (0000) now matches: the buffer pointer is built once from alloc_buffer's
 *     result (p = (far *)((unsigned long)alloc_buffer(..) << 16); D_a889 = p;) and the two
 *     skipped header bytes are dead reads (st = *buf++; st = *buf++;).
 * Still different:
 *   sub_s040_1462_01c8 (01c8): after "cash[0] = 0" the original re-reads es:[si] when pushing cash[0]
 *     for sub_s029_1367_0130; 6.00A (with /Oa, needed for the rep stosw init loops) keeps the
 *     constant 0 across the two far calls.  Tried *cash, cash[0] as clamp argument, D_8dcc[0],
 *     volatile casts/globals (ignored); `int volatile far *cash` fixes the push but then the
 *     store moves before the first argument push (original: between pushes).
 *     #pragma optimize("a", off) fixes the push but loses the rep stosw loops.
 *   sub_s040_1462_0c22 (0c22): (a) in the 'D' loop the original indexes D_a6b4 with bx and D_9c69 with dx
 *     (mine: the other way round); (b) cross-jumping in the Up/Down cases keeps different copies
 *     of the shared tails (original: Up's pile-cursor draw and inv-scroll tail, Down's
 *     pile-scroll tail; 6.00A picks the opposite ones).  Tried if/else orientation, goto/label
 *     placements, several loop shapes for 'D' (do/while, (x+1)%5, split status tests).
 * These look like optimizer differences between the game's MSC 6.00 and 6.00A.
 */

struct gfx { char pad[0xc]; int color; };
struct invitem {                    /* inventory or loot entry, 6 bytes */
    int type;                       /* item definition (low 12 bits in inventories) */
    unsigned char kind;
    unsigned char quality;
    unsigned char count;
    unsigned char f5;
};
struct itemdef {                    /* from darkland.lst, 46 bytes */
    char name[0x1e];
    char kind;
    char pad1f[6];
    char f25;
    char pad26[8];
};
struct lst { int hdr; struct itemdef defs[1]; };
struct pchar {                      /* party member, 128 bytes */
    char status;                    /* 0 = empty slot, 3 = dead */
    char pad1[0x29];
    char name[0x40];
    char nitems;
    char pad6b[0x15];
};
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

extern int D_2816, D_2804, D_2800, D_2812;
extern int D_e7a2, D_e7a0;
extern char D_e48c[];
extern struct gfx D_2806, D_27f4;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_907b;
extern int D_907e;
extern char D_9419[];
extern int D_906b[];
extern int D_a67e;
extern int D_a6b4[];
extern struct pchar D_9c69[];
extern unsigned char far *D_a889;
extern unsigned D_8dd0;
extern struct invitem far *D_8db6;     /* the loot */
extern int D_8dba;
extern int D_8dbc;                     /* number of loot entries */
extern int D_8dc4;                     /* party member receiving */
extern unsigned char far *D_8dc8;      /* caller's loot count */
extern int far *D_8dcc;                /* cash found: florins, groschen, pfennigs */
extern struct itemdef far * far F_2e38_0890[];
extern struct invitem far F_2e38_003e[][64];

int sprintf(char *, const char *, ...);
int printf(const char *, ...);
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
void exit_(int);

unsigned far alloc_buffer(unsigned long size, char *name);
void far free_buffer(unsigned seg, char *name);
void far sub_root_0617_011e(char *, unsigned char far *);
int far sub_s029_1367_0066(int, int, int);
void far sub_s029_1367_0130(int *, int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_047c_04dc(struct gfx *, int, int, int, int, int, int);
void far sub_root_047c_055a(struct gfx *, int, int, char *);
void far sub_root_047c_0553(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far sub_root_047c_058b(char *);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_0000_0000(char *, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_0000_13a0(void);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
int far vec_2003(int, int);
int far vec_1f6d(int, struct invitem);
void far vec_1d51(int);

extern int n_items[], side[], inv_top[], pile_top[], inv_sel[], pile_sel[];
void far sub_s040_1462_01c8(struct invitem far *items, unsigned char *count, int far *cash);
int far sub_s040_1462_06c8(int r);
void far sub_s040_1462_06e2(int ch);
void far sub_s040_1462_0952(int ch);
void far sub_s040_1462_0c22(int key);
void far sub_s040_1462_1242(int type, int ch, int quality);
void far sub_s040_1462_12ec(int idx);
void far sub_s040_1462_1372(int ch);
void far sub_s040_1462_145e(int unused);

/* Entry: load the item list, fix up the qualities of the found items and run the screen. */
void far sub_s040_1462_0000(struct invitem far *items, unsigned char *count, int far *cash)
{
    char *msg;
    unsigned seg;
    unsigned char far *buf;
    struct find_t ft;
    char st;
    struct lst far *p;
    int i;

    msg = "Could not open %s file\n";
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    if (D_9c69[D_907b].status == 3 || D_9c69[D_907b].status == 0) {
        for (i = 0; i < 5; i++) {
            if (D_9c69[i].status != 3 && D_9c69[D_907b].status != 0) {
                D_907b = i;
                break;
            }
        }
    }
    if (_dos_findfirst("darkland.lst", 0, &ft) != 0) {
        printf(msg, "list");
        exit_(1);
    } else {
        buf = (unsigned char far *)((unsigned long)alloc_buffer(ft.size, "Item memory") << 16);
        D_a889 = buf;
        sub_root_0617_011e("darkland.lst", buf);
        D_8dd0 = *buf++;
        st = *buf++;
        st = *buf++;
        for (i = 0; i < D_8dd0; i++) {
            F_2e38_0890[i] = (struct itemdef far *)buf;
            buf += sizeof(struct itemdef);
        }
    }
    for (i = 0; i < *count; i++) {
        if (items[i].count == 0)
            items[i].count = 1;
        if (items[i].kind <= 0x61) {
            if (items[i].kind == 0x40 || items[i].kind == 0x41 || items[i].kind == 0x42 || items[i].kind == 0x17 || items[i].kind == 0x18)
                items[i].quality = 25;
            else if (items[i].kind == 0x19 || items[i].kind == 0x1a || items[i].kind == 0x17 || items[i].kind == 0x18)
                items[i].quality = sub_s029_1367_0066(1, 99, items[i].quality);
            else
                items[i].quality = sub_s029_1367_0066(1, 99, items[i].quality - 10);
        }
    }
    sub_root_047c_1293();
    sub_s040_1462_01c8(items, count, cash);
    free_buffer(((unsigned *)&D_a889)[1], "Saint");
    sub_root_047c_0568();
}

int n_items[5] = {0};        /* items carried by each member */
int side[5] = {1, 1, 1, 1, 1};   /* 1 = cursor in the loot pile, 0 = in the inventory */
int inv_top[5] = {0};
int pile_top[5] = {0};
int inv_sel[5] = {0};
int pile_sel[5] = {0};
char menu_get[] = "\214 G\377et an item from pile of loot";
char menu_put[] = "\214 P\377ut an item into the pile of loot";
char menu_dist[] = "\214 D\377istribute to a different person";
char menu_leave[] = "\214 L\377eave";

/* The loot screen's main loop. */
void far sub_s040_1462_01c8(struct invitem far *items, unsigned char *count, int far *cash)
{
    int key;
    int sel;
    int i;
    int row;

    sel = -1;
    D_8dbc = 0;
    for (i = 0; i < 5; i++) {
        n_items[i] = 0;
        side[i] = 1;
        inv_top[i] = 0;
        pile_top[i] = 0;
        inv_sel[i] = 0;
        pile_sel[i] = 0;
    }
    cash[0] = 0;
    cash[1] = sub_s029_1367_0066(0, 10, cash[1]);
    cash[2] = sub_s029_1367_0066(0, 10, cash[2]);
    D_8db6 = items;
    D_8dc8 = count;
    D_8dcc = cash;
    D_8dc4 = D_907b;
    D_8dba = D_907b;
    D_2800 = 8;
    sub_s029_1367_0130(D_906b, *(unsigned far *)cash, cash[1], cash[2]);
    sub_root_047c_0568();
    sub_s040_1462_06e2(D_907b);
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (!(D_6ef0 & 1) && D_6eec > 0x50 && D_6eec < 0xe6 && D_6eee > 0x1b && D_6eee < 0x43) {
            row = (D_6eee - 0x1b) / 8;
            if (sel != row) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(&D_27f4, 0x50, sel * 8 + 0x1b, 0x96, 8, 9, 8);
                sel = row;
                sub_root_047c_04dc(&D_27f4, 0x50, row * 8 + 0x1b, 0x96, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0x50 && D_6eec < 0xe6 && D_6eee > 0x1b && D_6eee < 0x43) {
            while (D_6ef0 & 1)
                ;
            switch (row = (D_6eee - 0x1b) / 8) {
            case 0: key = 0x2247; break;
            case 1: key = 0x1950; break;
            case 2: key = 0x2044; break;
            case 3: key = 0x11b; break;
            }
        } else if (!(D_6ef0 & 1) && sel != -1) {
            sub_root_047c_12c3();
            sub_root_047c_04dc(&D_27f4, 0x50, sel * 8 + 0x1b, 0x96, 8, 9, 8);
            sel = -1;
            sub_root_047c_1293();
        }
        if ((D_6ef0 & 1) && D_6eec > 0x72 && D_6eec < 0x11c && D_6eee > 0x93 && D_6eee < 0xb3) {
            sub_root_047c_12c3();
            side[D_8dc4] = 0;
            inv_sel[D_8dc4] = (D_6eee - 0x93) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
            D_2800 = 8;
            sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
            sub_root_047c_04dc(&D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eec > 0x52 && D_6eec < 0x10b && D_6eee > 0x55 && D_6eee < 0x75) {
            sub_root_047c_12c3();
            side[D_8dc4] = 1;
            pile_sel[D_8dc4] = (D_6eee - 0x55) >> 3;
            D_2800 = 7;
            sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
            D_2800 = 8;
            sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
            sub_root_047c_04dc(&D_27f4, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            sub_root_047c_1293();
            while (D_6ef0 & 1)
                ;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x8a && D_6eee < 0x93 && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            key = 0x4800;
            side[D_8dc4] = 0;
        }
        if ((D_6ef0 & 1) && D_6eee > 0xb3 && D_6eee < 0xbc && D_6eec > 0x72 && D_6eec < 0x12b) {
            while (D_6ef0 & 1)
                ;
            key = 0x5000;
            side[D_8dc4] = 0;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x4c && D_6eee < 0x55 && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            key = 0x4800;
            side[D_8dc4] = 1;
        }
        if ((D_6ef0 & 1) && D_6eee > 0x75 && D_6eee < 0x7e && D_6eec > 0x52 && D_6eec < 0x10b) {
            while (D_6ef0 & 1)
                ;
            key = 0x5000;
            side[D_8dc4] = 1;
        }
        sub_root_0000_0000(D_9419, 0);
        if (sub_root_047c_073d())
            key = sub_root_047c_0744();
        if (key == 0x11b || key == 0x264c || key == 0x266c) {
            sub_s040_1462_06c8(-1);
            return;
        }
        if (key)
            sub_s040_1462_0c22(key);
    }
}

/* Restore the display state and the selected party member. */
int far sub_s040_1462_06c8(int r)
{
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    D_907b = D_8dba;
    return r;
}

/* First drawing of the screen. */
void far sub_s040_1462_06e2(int ch)
{
    char buf[120];
    int i;
    int y;
    struct invitem far *it;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 0xc8, &D_27f4, 0x3c, 0);
    D_2800 = 8;
    D_2812 = 8;
    D_2804 = 1;
    D_2816 = 1;
    sprintf(buf, "The party finds %dfl, %dgr, %dpf in cash.", D_8dcc[0], D_8dcc[1], D_8dcc[2]);
    sub_root_047c_055a(&D_27f4, 0x49, 9, buf);
    D_2800 = 8;
    sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
    D_2800 = 7;
    sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
    D_2800 = 8;
    sub_root_047c_055a(&D_27f4, 0x50, 0x2b, menu_dist);
    sub_root_047c_055a(&D_27f4, 0x50, 0x33, menu_leave);
    sprintf(buf, "The loot contains...");
    sub_root_047c_055a(&D_27f4, 0x6b, 0x45, buf);
    sprintf(buf, "%s currently has...", D_9c69[ch].name);
    sub_root_047c_055a(&D_27f4, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < D_9c69[ch].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_003e[ch][i].type & 0xfff]->name,
                F_2e38_003e[ch][i].count, F_2e38_003e[ch][i].quality);
        sub_root_047c_055a(&D_27f4, 0x75, y, buf);
    }
    for (i = 0; i < *D_8dc8 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        sub_root_047c_055a(&D_27f4, 0x55, (i - inv_top[D_8dc4]) * 8 + 0x55, buf);
    }
    sub_root_047c_04dc(&D_27f4, 0x53, 0x55, 0xfa, 8, 8, 9);
    n_items[D_8dc4] = D_9c69[ch].nitems;
    D_8dbc = *D_8dc8;
    D_2800 = 8;
    sub_root_047c_1293();
}

/* Redraw everything after switching to another party member. */
void far sub_s040_1462_0952(int ch)
{
    char buf[120];
    int i;

    sub_root_0617_024a(1, "pics\\buysell.pic", D_e48c);
    sub_root_047c_12c3();
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 0xc8, &D_27f4, 0x3c, 0);
    sprintf(buf, "The party finds %dfl, %dgr, %dpf in cash.", D_8dcc[0], D_8dcc[1], D_8dcc[2]);
    sub_root_047c_055a(&D_27f4, 0x49, 9, buf);
    D_2800 = side[D_8dc4] ? 8 : 7;
    sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
    D_2800 = side[D_8dc4] ? 7 : 8;
    sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
    D_2800 = 8;
    sub_root_047c_055a(&D_27f4, 0x50, 0x2b, menu_dist);
    sub_root_047c_055a(&D_27f4, 0x50, 0x33, menu_leave);
    sprintf(buf, "The loot contains...");
    sub_root_047c_055a(&D_27f4, 0x57, 0x45, buf);
    sprintf(buf, "%s has...", D_9c69[ch].name);
    sub_root_047c_055a(&D_27f4, 0x77, 0x83, buf);
    for (i = inv_top[D_8dc4]; i < inv_top[D_8dc4] + 4 && i < D_9c69[ch].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_003e[ch][i].type & 0xfff]->name,
                F_2e38_003e[ch][i].count, F_2e38_003e[ch][i].quality);
        sub_root_047c_055a(&D_27f4, 0x75, (i - inv_top[D_8dc4]) * 8 + 0x93, buf);
    }
    for (i = pile_top[D_8dc4]; i < pile_top[D_8dc4] + 4 && i < D_8dbc; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        sub_root_047c_055a(&D_27f4, 0x55, (i - pile_top[D_8dc4]) * 8 + 0x55, buf);
    }
    if (side[D_8dc4])
        sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
    else
        sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb7, 8, 8, 9);
    n_items[D_8dc4] = D_9c69[ch].nitems;
    sub_root_047c_1293();
}

/* Keyboard commands. */
void far sub_s040_1462_0c22(int key)
{
    struct invitem it;

    switch (key) {
    case 0x1000:
        sub_s040_1462_06c8(-1);
        sub_root_0000_13a0();
        return;
    case 0x1950: case 0x1970:
        if (side[D_8dc4] || !n_items[D_8dc4])
            return;
        sub_s040_1462_1242(F_2e38_003e[D_8dc4][inv_top[D_8dc4] + inv_sel[D_8dc4]].type & 0xfff, D_8dc4,
                 F_2e38_003e[D_8dc4][inv_top[D_8dc4] + inv_sel[D_8dc4]].quality);
        n_items[D_8dc4] = vec_2003(D_8dc4, inv_top[D_8dc4] + inv_sel[D_8dc4]);
        if (inv_top[D_8dc4] + inv_sel[D_8dc4] + 1 >= n_items[D_8dc4]) {
            if (inv_top[D_8dc4])
                inv_top[D_8dc4]--;
            else if (inv_sel[D_8dc4])
                inv_sel[D_8dc4]--;
        }
        sub_s040_1462_145e(D_907e);
        sub_s040_1462_1372(D_8dc4);
        sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        return;
    case 0x2044: case 0x2064:
        sub_root_0000_1588(0x100);
        sub_root_0000_1716(0x100);
        for (;;) {
            D_8dc4++;
            D_8dc4 %= 5;
            if (D_8dc4 >= D_a67e)
                continue;
            if (D_a6b4[D_8dc4] == -99) {
                vec_1d51(D_8dc4);
                continue;
            }
            if (D_9c69[D_8dc4].status != 0 && D_9c69[D_8dc4].status != 3)
                break;
        }
        sub_s040_1462_0952(D_8dc4);
        sub_root_047c_04dc(&D_27f4, 0x50, 0x2b, 0xc8, 9, 8, 9);
        return;
    case 0x2247: case 0x2267:
        if (!side[D_8dc4] || !D_8dbc)
            return;
        it.type = D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type;
        it.kind = F_2e38_0890[D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type]->kind;
        it.quality = D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].quality;
        it.f5 = F_2e38_0890[D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type]->f25;
        it.count = 1;
        n_items[D_8dc4] = vec_1f6d(D_8dc4, it);
        sub_s040_1462_12ec(pile_top[D_8dc4] + pile_sel[D_8dc4]);
        if (pile_top[D_8dc4] + pile_sel[D_8dc4] + 1 > D_8dbc) {
            if (pile_top[D_8dc4])
                pile_top[D_8dc4]--;
            else if (pile_sel[D_8dc4])
                pile_sel[D_8dc4]--;
        }
        sub_s040_1462_145e(D_907e);
        sub_s040_1462_1372(D_8dc4);
        sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        return;
    case 0x4800:
        sub_root_047c_12c3();
        if (side[D_8dc4])
            pile_sel[D_8dc4]--;
        else
            inv_sel[D_8dc4]--;
        if (side[D_8dc4] && pile_sel[D_8dc4] < 0 && pile_top[D_8dc4] > 0) {
            pile_sel[D_8dc4]++;
            pile_top[D_8dc4]--;
            sub_s040_1462_145e(D_907e);
            sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        } else if (!side[D_8dc4] && inv_sel[D_8dc4] < 0 && inv_top[D_8dc4] > 0) {
            inv_sel[D_8dc4]++;
            inv_top[D_8dc4]--;
            sub_s040_1462_1372(D_8dc4);
            sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (pile_sel[D_8dc4] < 0)
                pile_sel[D_8dc4] = 0;
            if (inv_sel[D_8dc4] < 0)
                inv_sel[D_8dc4] = 0;
            sub_root_047c_04dc(&D_27f4, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (side[D_8dc4])
                sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            else
                sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        break;
    case 0x4b00: case 0x4d00:
        sub_root_047c_12c3();
        if (side[D_8dc4]) {
            D_2800 = 7;
            sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
            D_2800 = 8;
            sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
            sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 9, 8);
            sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xfa, 8, 8, 9);
            side[D_8dc4] = 0;
        } else {
            if (!D_8dbc)
                return;
            D_2800 = 7;
            sub_root_047c_055a(&D_27f4, 0x50, 0x23, menu_put);
            D_2800 = 8;
            sub_root_047c_055a(&D_27f4, 0x50, 0x1b, menu_get);
            sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 9, 8);
            sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            side[D_8dc4] = 1;
        }
        break;
    case 0x5000:
        sub_root_047c_12c3();
        if (side[D_8dc4])
            pile_sel[D_8dc4]++;
        else
            inv_sel[D_8dc4]++;
        if (side[D_8dc4] && pile_sel[D_8dc4] >= 4 && pile_top[D_8dc4] + pile_sel[D_8dc4] < D_8dbc) {
            pile_sel[D_8dc4]--;
            pile_top[D_8dc4]++;
            sub_s040_1462_145e(D_907e);
            sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        } else if (!side[D_8dc4] && inv_sel[D_8dc4] >= 4 && inv_top[D_8dc4] + inv_sel[D_8dc4] < n_items[D_8dc4]) {
            inv_sel[D_8dc4]--;
            inv_top[D_8dc4]++;
            sub_s040_1462_1372(D_8dc4);
            sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (pile_sel[D_8dc4] >= 4)
                pile_sel[D_8dc4] = 3;
            if (inv_sel[D_8dc4] >= 4)
                inv_sel[D_8dc4] = 3;
            sub_root_047c_04dc(&D_27f4, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (side[D_8dc4])
                sub_root_047c_04dc(&D_27f4, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            else
                sub_root_047c_04dc(&D_27f4, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        break;
    default:
        return;
    }
    sub_root_047c_1293();
}

/* Put an item into the loot pile, stacking it with an identical one if possible. */
void far sub_s040_1462_1242(int type, int ch, int quality)
{
    int i;

    for (i = 0; i < *D_8dc8; i++) {
        if (D_8db6[i].type == type && D_8db6[i].quality == quality && D_8db6[i].count < 255) {
            D_8db6[i].count++;
            return;
        }
    }
    if (*D_8dc8 < 45) {
        (*D_8dc8)++;
        D_8dbc = *D_8dc8;
        D_8db6[i].type = type;
        D_8db6[i].quality = quality;
        D_8db6[i].count = 1;
    }
}

/* Take one item off a loot pile entry, closing the gap when it runs out. */
void far sub_s040_1462_12ec(int idx)
{
    int i;

    if (--D_8db6[idx].count == 0) {
        for (i = idx; i < *D_8dc8; i++) {
            D_8db6[i].type = D_8db6[i + 1].type;
            D_8db6[i].quality = D_8db6[i + 1].quality;
            D_8db6[i].count = D_8db6[i + 1].count;
        }
        (*D_8dc8)--;
        D_8dbc = *D_8dc8;
    }
}

/* The member's inventory list. */
void far sub_s040_1462_1372(int ch)
{
    char buf[60];
    int i;

    sub_root_047c_12c3();
    D_2800 = 8;
    sub_root_047c_0553(&D_2806, 0x72, 0x93, 0xb6, 0x20, &D_27f4, 0x72, 0x93);
    for (i = inv_top[D_8dc4]; i < inv_top[D_8dc4] + 4 && i < D_9c69[ch].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[F_2e38_003e[ch][i].type & 0xfff]->name,
                F_2e38_003e[ch][i].count, F_2e38_003e[ch][i].quality);
        sub_root_047c_055a(&D_27f4, 0x75, (i - inv_top[D_8dc4]) * 8 + 0x93, buf);
    }
    sub_root_047c_1293();
}

/* The loot pile list. */
void far sub_s040_1462_145e(int unused)
{
    char buf[120];
    int i;

    sub_root_047c_12c3();
    D_2800 = 8;
    sub_root_047c_0553(&D_2806, 0x52, 0x55, 0xb6, 0x20, &D_27f4, 0x52, 0x55);
    for (i = pile_top[D_8dc4]; i < pile_top[D_8dc4] + 4 && i < D_8dbc; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", F_2e38_0890[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        sub_root_047c_055a(&D_27f4, 0x55, (i - pile_top[D_8dc4]) * 8 + 0x55, buf);
    }
    sub_root_047c_1293();
}
