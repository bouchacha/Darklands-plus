/* place: s040 1462:0000 */
/* orig: size=1524 CONST=7edc+0004 _DATA=4bbc+0231 */
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
 *   Session 5: line layout of the shared tails (one-line calls, `}`/`else` joined, 255 combos)
 *     and case order: no effect on which copy cross-jumping keeps.
 * These look like optimizer differences between the game's MSC 6.00 and 6.00A.
 */
#include "darklands/card.h"
#include "darklands/game.h"

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

extern int back_win_font, screen_win_font, screen_win_colour, back_win_colour;
extern int D_e7a2, D_e7a0;
extern char cur_palette[];
extern struct gfx back_win, screen_win;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char leader;
extern int cur_loc;
extern char D_9419[];
extern int party_money[];
extern int party_size;
extern int cbt_floor[];
extern struct pchar party[];
extern unsigned char far *lst_buf;
extern unsigned D_8dd0;
extern struct invitem far *D_8db6;     /* the loot */
extern int D_8dba;
extern int D_8dbc;                     /* number of loot entries */
extern int D_8dc4;                     /* party member receiving */
extern unsigned char far *D_8dc8;      /* caller's loot count */
extern int far *D_8dcc;                /* cash found: florins, groschen, pfennigs */
extern struct itemdef far * far item_defs[];
extern struct invitem far inventories[][64];

int sprintf(char *, const char *, ...);
int printf(const char *, ...);
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
void exit_(int);

unsigned far alloc_buffer(unsigned long size, char *name);
void far free_buffer(unsigned seg, char *name);
void far file_load_far(char *, unsigned char far *);
int far clamp_range(int, int, int);
void far money_add_fgp(int *, int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far mouse_hide(void);
void far mouse_show(void);
void far gfx_recolor_rect(struct gfx *, int, int, int, int, int, int);
void far gfx_text(struct gfx *, int, int, char *);
void far gfx_copy_rect(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far set_palette(char *);
void far pic_draw_pal(int, char *, char *);
void far menu_bar_run(char *, int);
int far key_pressed(void);
int far get_key(void);
void far quit_to_dos(void);
void far party_panel_frames(int);
void far party_panel_rows(int);
int far item_take_one_vec(int, int);
int far add_item_rec(int, struct invitem);
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
void far loot_distribute(struct invitem far *items, unsigned char *count, int far *cash)
{
    char *msg;
    unsigned seg;
    unsigned char far *buf;
    struct find_t ft;
    char st;
    struct lst far *p;
    int i;

    msg = "Could not open %s file\n";
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    if (party[leader].status == 3 || party[leader].status == 0) {
        for (i = 0; i < 5; i++) {
            if (party[i].status != 3 && party[leader].status != 0) {
                leader = i;
                break;
            }
        }
    }
    if (_dos_findfirst("darkland.lst", 0, &ft) != 0) {
        printf(msg, "list");
        exit_(1);
    } else {
        buf = (unsigned char far *)((unsigned long)alloc_buffer(ft.size, "Item memory") << 16);
        lst_buf = buf;
        file_load_far("darkland.lst", buf);
        D_8dd0 = *buf++;
        st = *buf++;
        st = *buf++;
        for (i = 0; i < D_8dd0; i++) {
            item_defs[i] = (struct itemdef far *)buf;
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
                items[i].quality = clamp_range(1, 99, items[i].quality);
            else
                items[i].quality = clamp_range(1, 99, items[i].quality - 10);
        }
    }
    mouse_show();
    sub_s040_1462_01c8(items, count, cash);
    free_buffer(((unsigned *)&lst_buf)[1], "Saint");
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
    cash[1] = clamp_range(0, 10, cash[1]);
    cash[2] = clamp_range(0, 10, cash[2]);
    D_8db6 = items;
    D_8dc8 = count;
    D_8dcc = cash;
    D_8dc4 = leader;
    D_8dba = leader;
    screen_win_colour = 8;
    money_add_fgp(party_money, *(unsigned far *)cash, cash[1], cash[2]);
    sub_root_047c_0568();
    sub_s040_1462_06e2(leader);
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if (!(mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            row = (mouse_y - 0x1b) / 8;
            if (sel != row) {
                mouse_hide();
                gfx_recolor_rect(&screen_win, 0x50, sel * 8 + 0x1b, 0x96, 8, 9, 8);
                sel = row;
                gfx_recolor_rect(&screen_win, 0x50, row * 8 + 0x1b, 0x96, 8, 8, 9);
                mouse_show();
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            while (mouse_buttons & 1)
                ;
            switch (row = (mouse_y - 0x1b) / 8) {
            case 0: key = 0x2247; break;
            case 1: key = 0x1950; break;
            case 2: key = 0x2044; break;
            case 3: key = 0x11b; break;
            }
        } else if (!(mouse_buttons & 1) && sel != -1) {
            mouse_hide();
            gfx_recolor_rect(&screen_win, 0x50, sel * 8 + 0x1b, 0x96, 8, 9, 8);
            sel = -1;
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_x > 0x72 && mouse_x < 0x11c && mouse_y > 0x93 && mouse_y < 0xb3) {
            mouse_hide();
            side[D_8dc4] = 0;
            inv_sel[D_8dc4] = (mouse_y - 0x93) >> 3;
            screen_win_colour = 7;
            gfx_text(&screen_win, 0x50, 0x1b, menu_get);
            screen_win_colour = 8;
            gfx_text(&screen_win, 0x50, 0x23, menu_put);
            gfx_recolor_rect(&screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_x > 0x52 && mouse_x < 0x10b && mouse_y > 0x55 && mouse_y < 0x75) {
            mouse_hide();
            side[D_8dc4] = 1;
            pile_sel[D_8dc4] = (mouse_y - 0x55) >> 3;
            screen_win_colour = 7;
            gfx_text(&screen_win, 0x50, 0x23, menu_put);
            screen_win_colour = 8;
            gfx_text(&screen_win, 0x50, 0x1b, menu_get);
            gfx_recolor_rect(&screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x8a && mouse_y < 0x93 && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            key = 0x4800;
            side[D_8dc4] = 0;
        }
        if ((mouse_buttons & 1) && mouse_y > 0xb3 && mouse_y < 0xbc && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            key = 0x5000;
            side[D_8dc4] = 0;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x4c && mouse_y < 0x55 && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            key = 0x4800;
            side[D_8dc4] = 1;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x75 && mouse_y < 0x7e && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            key = 0x5000;
            side[D_8dc4] = 1;
        }
        menu_bar_run(D_9419, 0);
        if (key_pressed())
            key = get_key();
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
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    leader = D_8dba;
    return r;
}

/* First drawing of the screen. */
void far sub_s040_1462_06e2(int ch)
{
    char buf[120];
    int i;
    int y;
    struct invitem far *it;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    set_palette(cur_palette);
    gfx_copy_rect(&back_win, 0x3c, 0, 0x104, 0xc8, &screen_win, 0x3c, 0);
    screen_win_colour = 8;
    back_win_colour = 8;
    screen_win_font = 1;
    back_win_font = 1;
    sprintf(buf, "The party finds %dfl, %dgr, %dpf in cash.", D_8dcc[0], D_8dcc[1], D_8dcc[2]);
    gfx_text(&screen_win, 0x49, 9, buf);
    screen_win_colour = 8;
    gfx_text(&screen_win, 0x50, 0x1b, menu_get);
    screen_win_colour = 7;
    gfx_text(&screen_win, 0x50, 0x23, menu_put);
    screen_win_colour = 8;
    gfx_text(&screen_win, 0x50, 0x2b, menu_dist);
    gfx_text(&screen_win, 0x50, 0x33, menu_leave);
    sprintf(buf, "The loot contains...");
    gfx_text(&screen_win, 0x6b, 0x45, buf);
    sprintf(buf, "%s currently has...", party[ch].name);
    gfx_text(&screen_win, 0x77, 0x83, buf);
    for (i = 0, y = 0x93; y < 0xb3 && i < party[ch].nitems; i++, y += 8) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[inventories[ch][i].type & 0xfff]->name,
                inventories[ch][i].count, inventories[ch][i].quality);
        gfx_text(&screen_win, 0x75, y, buf);
    }
    for (i = 0; i < *D_8dc8 && i < 4; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        gfx_text(&screen_win, 0x55, (i - inv_top[D_8dc4]) * 8 + 0x55, buf);
    }
    gfx_recolor_rect(&screen_win, 0x53, 0x55, 0xfa, 8, 8, 9);
    n_items[D_8dc4] = party[ch].nitems;
    D_8dbc = *D_8dc8;
    screen_win_colour = 8;
    mouse_show();
}

/* Redraw everything after switching to another party member. */
void far sub_s040_1462_0952(int ch)
{
    char buf[120];
    int i;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    gfx_copy_rect(&back_win, 0x3c, 0, 0x104, 0xc8, &screen_win, 0x3c, 0);
    sprintf(buf, "The party finds %dfl, %dgr, %dpf in cash.", D_8dcc[0], D_8dcc[1], D_8dcc[2]);
    gfx_text(&screen_win, 0x49, 9, buf);
    screen_win_colour = side[D_8dc4] ? 8 : 7;
    gfx_text(&screen_win, 0x50, 0x1b, menu_get);
    screen_win_colour = side[D_8dc4] ? 7 : 8;
    gfx_text(&screen_win, 0x50, 0x23, menu_put);
    screen_win_colour = 8;
    gfx_text(&screen_win, 0x50, 0x2b, menu_dist);
    gfx_text(&screen_win, 0x50, 0x33, menu_leave);
    sprintf(buf, "The loot contains...");
    gfx_text(&screen_win, 0x57, 0x45, buf);
    sprintf(buf, "%s has...", party[ch].name);
    gfx_text(&screen_win, 0x77, 0x83, buf);
    for (i = inv_top[D_8dc4]; i < inv_top[D_8dc4] + 4 && i < party[ch].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[inventories[ch][i].type & 0xfff]->name,
                inventories[ch][i].count, inventories[ch][i].quality);
        gfx_text(&screen_win, 0x75, (i - inv_top[D_8dc4]) * 8 + 0x93, buf);
    }
    for (i = pile_top[D_8dc4]; i < pile_top[D_8dc4] + 4 && i < D_8dbc; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        gfx_text(&screen_win, 0x55, (i - pile_top[D_8dc4]) * 8 + 0x55, buf);
    }
    if (side[D_8dc4])
        gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
    else
        gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb7, 8, 8, 9);
    n_items[D_8dc4] = party[ch].nitems;
    mouse_show();
}

/* Keyboard commands. */
void far sub_s040_1462_0c22(int key)
{
    struct invitem it;

    switch (key) {
    case 0x1000:
        sub_s040_1462_06c8(-1);
        quit_to_dos();
        return;
    case 0x1950: case 0x1970:
        if (side[D_8dc4] || !n_items[D_8dc4])
            return;
        sub_s040_1462_1242(inventories[D_8dc4][inv_top[D_8dc4] + inv_sel[D_8dc4]].type & 0xfff, D_8dc4,
                 inventories[D_8dc4][inv_top[D_8dc4] + inv_sel[D_8dc4]].quality);
        n_items[D_8dc4] = item_take_one_vec(D_8dc4, inv_top[D_8dc4] + inv_sel[D_8dc4]);
        if (inv_top[D_8dc4] + inv_sel[D_8dc4] + 1 >= n_items[D_8dc4]) {
            if (inv_top[D_8dc4])
                inv_top[D_8dc4]--;
            else if (inv_sel[D_8dc4])
                inv_sel[D_8dc4]--;
        }
        sub_s040_1462_145e(cur_loc);
        sub_s040_1462_1372(D_8dc4);
        gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        return;
    case 0x2044: case 0x2064:
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        for (;;) {
            D_8dc4++;
            D_8dc4 %= 5;
            if (D_8dc4 >= party_size)
                continue;
            if (cbt_floor[D_8dc4] == -99) {
                vec_1d51(D_8dc4);
                continue;
            }
            if (party[D_8dc4].status != 0 && party[D_8dc4].status != 3)
                break;
        }
        sub_s040_1462_0952(D_8dc4);
        gfx_recolor_rect(&screen_win, 0x50, 0x2b, 0xc8, 9, 8, 9);
        return;
    case 0x2247: case 0x2267:
        if (!side[D_8dc4] || !D_8dbc)
            return;
        it.type = D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type;
        it.kind = item_defs[D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type]->kind;
        it.quality = D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].quality;
        it.f5 = item_defs[D_8db6[pile_top[D_8dc4] + pile_sel[D_8dc4]].type]->f25;
        it.count = 1;
        n_items[D_8dc4] = add_item_rec(D_8dc4, it);
        sub_s040_1462_12ec(pile_top[D_8dc4] + pile_sel[D_8dc4]);
        if (pile_top[D_8dc4] + pile_sel[D_8dc4] + 1 > D_8dbc) {
            if (pile_top[D_8dc4])
                pile_top[D_8dc4]--;
            else if (pile_sel[D_8dc4])
                pile_sel[D_8dc4]--;
        }
        sub_s040_1462_145e(cur_loc);
        sub_s040_1462_1372(D_8dc4);
        gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        return;
    case 0x4800:
        mouse_hide();
        if (side[D_8dc4])
            pile_sel[D_8dc4]--;
        else
            inv_sel[D_8dc4]--;
        if (side[D_8dc4] && pile_sel[D_8dc4] < 0 && pile_top[D_8dc4] > 0) {
            pile_sel[D_8dc4]++;
            pile_top[D_8dc4]--;
            sub_s040_1462_145e(cur_loc);
            gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        } else if (!side[D_8dc4] && inv_sel[D_8dc4] < 0 && inv_top[D_8dc4] > 0) {
            inv_sel[D_8dc4]++;
            inv_top[D_8dc4]--;
            sub_s040_1462_1372(D_8dc4);
            gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (pile_sel[D_8dc4] < 0)
                pile_sel[D_8dc4] = 0;
            if (inv_sel[D_8dc4] < 0)
                inv_sel[D_8dc4] = 0;
            gfx_recolor_rect(&screen_win, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (side[D_8dc4])
                gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            else
                gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        break;
    case 0x4b00: case 0x4d00:
        mouse_hide();
        if (side[D_8dc4]) {
            screen_win_colour = 7;
            gfx_text(&screen_win, 0x50, 0x1b, menu_get);
            screen_win_colour = 8;
            gfx_text(&screen_win, 0x50, 0x23, menu_put);
            gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 9, 8);
            gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xfa, 8, 8, 9);
            side[D_8dc4] = 0;
        } else {
            if (!D_8dbc)
                return;
            screen_win_colour = 7;
            gfx_text(&screen_win, 0x50, 0x23, menu_put);
            screen_win_colour = 8;
            gfx_text(&screen_win, 0x50, 0x1b, menu_get);
            gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 9, 8);
            gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            side[D_8dc4] = 1;
        }
        break;
    case 0x5000:
        mouse_hide();
        if (side[D_8dc4])
            pile_sel[D_8dc4]++;
        else
            inv_sel[D_8dc4]++;
        if (side[D_8dc4] && pile_sel[D_8dc4] >= 4 && pile_top[D_8dc4] + pile_sel[D_8dc4] < D_8dbc) {
            pile_sel[D_8dc4]--;
            pile_top[D_8dc4]++;
            sub_s040_1462_145e(cur_loc);
            gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
        } else if (!side[D_8dc4] && inv_sel[D_8dc4] >= 4 && inv_top[D_8dc4] + inv_sel[D_8dc4] < n_items[D_8dc4]) {
            inv_sel[D_8dc4]--;
            inv_top[D_8dc4]++;
            sub_s040_1462_1372(D_8dc4);
            gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (pile_sel[D_8dc4] >= 4)
                pile_sel[D_8dc4] = 3;
            if (inv_sel[D_8dc4] >= 4)
                inv_sel[D_8dc4] = 3;
            gfx_recolor_rect(&screen_win, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (side[D_8dc4])
                gfx_recolor_rect(&screen_win, 0x52, pile_sel[D_8dc4] * 8 + 0x55, 0xfa, 8, 8, 9);
            else
                gfx_recolor_rect(&screen_win, 0x72, inv_sel[D_8dc4] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        break;
    default:
        return;
    }
    mouse_show();
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

    mouse_hide();
    screen_win_colour = 8;
    gfx_copy_rect(&back_win, 0x72, 0x93, 0xb6, 0x20, &screen_win, 0x72, 0x93);
    for (i = inv_top[D_8dc4]; i < inv_top[D_8dc4] + 4 && i < party[ch].nitems; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[inventories[ch][i].type & 0xfff]->name,
                inventories[ch][i].count, inventories[ch][i].quality);
        gfx_text(&screen_win, 0x75, (i - inv_top[D_8dc4]) * 8 + 0x93, buf);
    }
    mouse_show();
}

/* The loot pile list. */
void far sub_s040_1462_145e(int unused)
{
    char buf[120];
    int i;

    mouse_hide();
    screen_win_colour = 8;
    gfx_copy_rect(&back_win, 0x52, 0x55, 0xb6, 0x20, &screen_win, 0x52, 0x55);
    for (i = pile_top[D_8dc4]; i < pile_top[D_8dc4] + 4 && i < D_8dbc; i++) {
        sprintf(buf, "%Fs  (%3d) %2d-Qual", item_defs[D_8db6[i].type]->name,
                D_8db6[i].count, D_8db6[i].quality);
        gfx_text(&screen_win, 0x55, (i - pile_top[D_8dc4]) * 8 + 0x55, buf);
    }
    mouse_show();
}
