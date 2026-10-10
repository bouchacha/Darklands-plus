/* place: s035 1462:0000 */
/* orig: size=17c0 CONST=7e94+0008 _DATA=37ba+0272 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* data: _DATA=37ba */
/* data: CONST=7e94 */
/* Item cache screen (overlay s035): get / put items between a party member and a
   cache stored in the temporary file "cache.tmp". */
#include "darklands/card.h"
#include "darklands/game.h"

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

extern int back_win_font, screen_win_font, D_e7a2, D_e7a0;
extern char D_907c, leader;
extern int D_89f2, D_89ee, D_89f8, cur_loc;
extern volatile int screen_win_colour;          /* text colour */
extern volatile int mouse_buttons, mouse_x, mouse_y;   /* mouse buttons, x, y */
extern char screen_win[], back_win[], cur_palette[], D_9419[];
extern int party_money, party_money_gr, party_money_pf;
extern unsigned D_89f6;
extern int D_89f0;
extern int party_size;
extern struct member party[];

extern struct loc far * far locations[];
extern struct fchar far data_file[];
extern struct itemdef far * far item_defs[];
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
void far mouse_hide(void);
void far mouse_show(void);
void far gfx_recolor_rect(char *, int, int, int, int, int, int);
void far gfx_text(char *, int, int, char *);
void far gfx_copy_rect(char *, int, int, int, int, char *, int, int);
void far set_palette(char *);
int far key_pressed(void);
int far get_key(void);
void far menu_bar_run(char *, int);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far quit_to_dos(void);
void far pic_draw_pal(int, char *, char *);
void far file_load_far(char *, void far *);
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
unsigned far sub_root_06a1_2c9e(char *, unsigned, int *);   /* _dos_open */
int far member_sheet_vec(int);
int far item_take_one_vec(int, int);
int far add_item_rec(int, struct item);

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
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    D_907c = 5;
    D_89ee = D_89f2 = leader;
    screen_win_colour = 8;
    D_89f8 = arg;
    sub_root_047c_0568();
    draw_cache_screen(leader, arg);
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    for (;;) {
        mkey = 0;
        if (!(mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            row = (mouse_y - 0x1b) / 8;
            if (row != hover) {
                mouse_hide();
                gfx_recolor_rect(screen_win, 0x50, hover * 8 + 0x1b, 0x96, 8, 9, 8);
                hover = row;
                gfx_recolor_rect(screen_win, 0x50, row * 8 + 0x1b, 0x96, 8, 8, 9);
                mouse_show();
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            while (mouse_buttons & 1)
                ;
            row = (mouse_y - 0x1b) / 8;
            switch (row) {
            case 0: mkey = 0x2247; break;
            case 1: mkey = 0x1950; break;
            case 2: mkey = 0x2e43; break;
            case 3: mkey = 0x11b; break;
            }
        } else if (!(mouse_buttons & 1) && hover != -1) {
            mouse_hide();
            gfx_recolor_rect(screen_win, 0x50, hover * 8 + 0x1b, 0x96, 8, 9, 8);
            hover = -1;
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_x > 0x72 && mouse_x < 0x11c && mouse_y > 0x93 && mouse_y < 0xb3) {
            mouse_hide();
            D_37c4[D_89f2] = 0;
            D_37e2[D_89f2] = (mouse_y - 0x93) >> 3;
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x1b, str_get);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_put);
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_x > 0x52 && mouse_x < 0x10b && mouse_y > 0x55 && mouse_y < 0x75) {
            mouse_hide();
            D_37c4[D_89f2] = 1;
            D_37ec[D_89f2] = (mouse_y - 0x55) >> 3;
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_put);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_get);
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x8a && mouse_y < 0x93 && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x4800;
            D_37c4[D_89f2] = 0;
        }
        if ((mouse_buttons & 1) && mouse_y > 0xb3 && mouse_y < 0xbc && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x5000;
            D_37c4[D_89f2] = 0;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x4c && mouse_y < 0x55 && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x4800;
            D_37c4[D_89f2] = 1;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x75 && mouse_y < 0x7e && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x5000;
            D_37c4[D_89f2] = 1;
        }
        if ((mouse_buttons & 1) && mouse_x < 0x3c) {
            row = mouse_y / 40;
            while (row != -1) {
                row = member_sheet_vec(row);
                party_panel_frames(0x100);
                party_panel_rows(0x100);
            }
            redraw_cache_screen(D_89f2, arg);
            while (mouse_buttons & 1)
                ;
        }
        menu_bar_run(D_9419, 0);
        if (key_pressed())
            mkey = get_key();
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
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    if (D_89f6 != 0)
        cache_save(locations[cur_loc]->cache);
    else
        locations[cur_loc]->cache = -1;
    leader = D_89ee;
    return ret;
}

/* draw_cache_screen: draw the whole screen and load the cache contents */
void far draw_cache_screen(int who, int unused)
{
    char buf[120];
    int y;
    int i;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    set_palette(cur_palette);
    gfx_copy_rect(back_win, 0x3c, 0, 0x104, 0xc8, screen_win, 0x3c, 0);
    screen_win_colour = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf.", party[leader].name, party_money, party_money_gr, party_money_pf);
    gfx_text(screen_win, 0x49, 9, buf);
    screen_win_colour = 7;
    gfx_text(screen_win, 0x50, 0x1b, str_get);
    screen_win_colour = 8;
    gfx_text(screen_win, 0x50, 0x23, str_put);
    gfx_text(screen_win, 0x50, 0x2b, "\x8c C\xff" "ache another person's items");
    gfx_text(screen_win, 0x50, 0x33, "\x8c L\xff" "eave");
    sprintf(buf, "The cache contains...");
    gfx_text(screen_win, 0x6b, 0x45, buf);
    sprintf(buf, "%s currently has...", party[who].name);
    gfx_text(screen_win, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < party[who].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, y, buf);
    }
    cache_load(locations[cur_loc]->cache);
    for (i = 0; i < D_89f6 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        gfx_text(screen_win, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    gfx_recolor_rect(screen_win, 0x73, 0x93, 0xb7, 8, 8, 9);
    D_37ba[D_89f2] = party[who].nitems;
    D_89f0 = D_89f6;
    screen_win_colour = 8;
    mouse_show();
}

/* redraw_cache_screen: redraw the screen for another party member */
void far redraw_cache_screen(int who, int unused)
{
    char buf[120];
    int y;
    int i;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    set_palette(cur_palette);
    gfx_copy_rect(back_win, 0x3c, 0, 0x104, 0xc8, screen_win, 0x3c, 0);
    screen_win_colour = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf.", party[leader].name, party_money, party_money_gr, party_money_pf);
    gfx_text(screen_win, 0x49, 9, buf);
    screen_win_colour = D_37c4[D_89f2] ? 8 : 7;
    gfx_text(screen_win, 0x50, 0x1b, str_get);
    screen_win_colour = D_37c4[D_89f2] ? 7 : 8;
    gfx_text(screen_win, 0x50, 0x23, str_put);
    screen_win_colour = D_37c4[D_89f2] ? 8 : 7;
    screen_win_colour = 8;
    gfx_text(screen_win, 0x50, 0x2b, "\x8c C\xff" "ache another person's items");
    gfx_text(screen_win, 0x50, 0x33, "\x8c L\xff" "eave");
    sprintf(buf, "The cache contains...");
    gfx_text(screen_win, 0x57, 0x45, buf);
    sprintf(buf, "%s has...", party[who].name);
    gfx_text(screen_win, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < party[who].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, y, buf);
    }
    for (i = 0; i < D_89f6 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        gfx_text(screen_win, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    if (D_37c4[D_89f2])
        gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb7, 8, 8, 9);
    else
        gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb7, 8, 8, 9);
    D_37ba[D_89f2] = party[who].nitems;
    D_89f0 = D_89f6;
    screen_win_colour = 8;
    mouse_show();
}

/* cache_key: handle one key press on the cache screen */
void far cache_key(int key)
{
    struct item it;

    switch (key) {
    case 0x1000:
        cache_close(-1);
        quit_to_dos();
        break;
    case 0x1950:
    case 0x1970:            /* P: put an item into the cache */
        if (D_37c4[D_89f2] != 0)
            break;
        if (party[D_89f2].nitems == 0)
            break;
        cache_add(data_file[D_89f2].items[D_37e2[D_89f2] + D_37ce[D_89f2]].id & 0xfff, D_89f2,
                  data_file[D_89f2].items[D_37e2[D_89f2] + D_37ce[D_89f2]].qual);
        D_37ba[D_89f2] = item_take_one_vec(D_89f2, D_37e2[D_89f2] + D_37ce[D_89f2]);
        if (D_37e2[D_89f2] + D_37ce[D_89f2] >= D_37ba[D_89f2]) {
            if (D_37ce[D_89f2] != 0)
                D_37ce[D_89f2]--;
            else if (D_37e2[D_89f2] != 0)
                D_37e2[D_89f2]--;
        }
        draw_cache_items(cur_loc);
        draw_member_items(D_89f2);
        gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        break;
    case 0x2044:
    case 0x2064:            /* D */
        if (D_37c4[D_89f2] != 0)
            break;
        D_37ba[D_89f2] = item_take_one_vec(leader, D_37e2[D_89f2] + D_37ce[D_89f2]);
        draw_member_items(D_89f2);
        gfx_recolor_rect(screen_win, 0x50, D_37e2[D_89f2] * 8 + 0x1b, 0x5f, 8, 8, 9);
        break;
    case 0x2247:
    case 0x2267:            /* G: get an item from the cache */
        if (D_37c4[D_89f2] == 0)
            break;
        if (D_89f0 == 0)
            break;
        it.id = F_2e38_37f4[D_37d8[D_89f2] + D_37ec[D_89f2]].id;
        it.f2 = item_defs[it.id]->f1e;
        it.qual = F_2e38_37f4[D_37d8[D_89f2] + D_37ec[D_89f2]].qual;
        it.f5 = item_defs[it.id]->f25;
        it.cnt = 1;
        D_37ba[D_89f2] = add_item_rec(D_89f2, it);
        cache_take(D_37d8[D_89f2] + D_37ec[D_89f2]);
        if (D_37d8[D_89f2] + D_37ec[D_89f2] >= D_89f0) {
            if (D_37d8[D_89f2] != 0)
                D_37d8[D_89f2]--;
            else if (D_37ec[D_89f2] != 0)
                D_37ec[D_89f2]--;
        }
        draw_cache_items(cur_loc);
        draw_member_items(D_89f2);
        gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0x5f, 8, 8, 9);
        break;
    case 0x2e43:
    case 0x2e63:            /* C: cache another person's items */
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        do {
            D_89f2++;
            D_89f2 %= party_size;
        } while (party[D_89f2].present == 0);
        redraw_cache_screen(D_89f2, D_89f8);
        gfx_recolor_rect(screen_win, 0x50, 0x33, 0xc8, 9, 8, 9);
        break;
    case 0x4800:            /* up */
        mouse_hide();
        if (D_37c4[D_89f2])
            D_37ec[D_89f2]--;
        else
            D_37e2[D_89f2]--;
        if (D_37c4[D_89f2] && D_37ec[D_89f2] < 0 && D_37d8[D_89f2] > 0) {
            D_37ec[D_89f2]++;
            D_37d8[D_89f2]--;
            draw_cache_items(cur_loc);
            gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_37c4[D_89f2] && D_37e2[D_89f2] < 0 && D_37ce[D_89f2] > 0) {
            D_37e2[D_89f2]++;
            D_37ce[D_89f2]--;
            draw_member_items(D_89f2);
            gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_37ec[D_89f2] < 0)
                D_37ec[D_89f2] = 0;
            if (D_37e2[D_89f2] < 0)
                D_37e2[D_89f2] = 0;
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (D_37c4[D_89f2])
                gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xc3, 8, 8, 9);
            else
                gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        mouse_show();
        break;
    case 0x4b00:
    case 0x4d00:            /* left / right: switch lists */
        mouse_hide();
        if (D_37c4[D_89f2]) {
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x1b, str_get);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_put);
            gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 9, 8);
            gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
            D_37c4[D_89f2] = 0;
        } else {
            if (D_89f0 == 0)
                break;
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_put);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_get);
            gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 9, 8);
            gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
            D_37c4[D_89f2] = 1;
        }
        mouse_show();
        break;
    case 0x5000:            /* down */
        mouse_hide();
        if (D_37c4[D_89f2])
            D_37ec[D_89f2]++;
        else
            D_37e2[D_89f2]++;
        if (D_37c4[D_89f2] && D_37ec[D_89f2] >= 4 && D_37d8[D_89f2] + D_37ec[D_89f2] < D_89f0) {
            D_37ec[D_89f2]--;
            D_37d8[D_89f2]++;
            draw_cache_items(cur_loc);
            gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_37c4[D_89f2] && D_37e2[D_89f2] >= 4 && D_37ce[D_89f2] + D_37e2[D_89f2] < D_37ba[D_89f2]) {
            D_37e2[D_89f2]--;
            D_37ce[D_89f2]++;
            draw_member_items(D_89f2);
            gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_37ec[D_89f2] >= D_89f0)
                D_37ec[D_89f2]--;
            if (D_37ba[D_89f2] <= D_37e2[D_89f2])
                D_37e2[D_89f2]--;
            if (D_37ec[D_89f2] >= 4)
                D_37ec[D_89f2] = 3;
            if (D_37e2[D_89f2] >= 4)
                D_37e2[D_89f2] = 3;
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (D_37c4[D_89f2])
                gfx_recolor_rect(screen_win, 0x52, D_37ec[D_89f2] * 8 + 0x55, 0xc8, 8, 8, 9);
            else
                gfx_recolor_rect(screen_win, 0x72, D_37e2[D_89f2] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        mouse_show();
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

    mouse_hide();
    gfx_copy_rect(back_win, 0x72, 0x93, 0xb6, 0x20, screen_win, 0x72, 0x93);
    for (i = D_37ce[D_89f2]; i < D_37ce[D_89f2] + 4 && i < party[who].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2dq", item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, (i - D_37ce[D_89f2]) * 8 + 0x93, buf);
    }
    mouse_show();
}

/* draw_cache_items: list four of the cache's entries */
void far draw_cache_items(int unused)
{
    char buf[120];
    int i;

    mouse_hide();
    gfx_copy_rect(back_win, 0x52, 0x55, 0xb6, 0x20, screen_win, 0x52, 0x55);
    for (i = D_37d8[D_89f2]; i < D_37d8[D_89f2] + 4 && i < D_89f0; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[F_2e38_37f4[i].id],
                F_2e38_37f4[i].cnt, F_2e38_37f4[i].qual);
        gfx_text(screen_win, 0x55, (i - D_37d8[D_89f2]) * 8 + 0x55, buf);
    }
    mouse_show();
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
    file_load_far("cache.tmp", buf);
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
        locations[cur_loc]->cache = ncache;
    } else {
        p = buf = q = MK_FP(alloc_buffer(ft.size, "cache mem"), 0);
        file_load_far("cache.tmp", buf);
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
