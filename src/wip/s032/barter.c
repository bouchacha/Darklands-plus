/* place: s032 1462:0000 */
/* orig: size=1e0c CONST=7e66+0008 _DATA=2f18+02ea */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* e0 2026-10-09: functions renamed back to their original sub_ names (descriptive names in the comments); still 5/11.
 * Session 4: 078e down to one difference: i and y slots swapped ([bp-4]/[bp-6]); needed
 *   `#pragma optimize("a", off)`, srand as an if/else of two calls and `extern int D_9c4a;`
 *   moved before `extern int D_2800;`.  Declaration order, names, loop-init order, y as a
 *   strength-reduced i*8+0x93: no effect on the slot order.
 *   1d18: the uninitialised `town` and `size` are one variable in the original (both [bp-2]);
 *   writing them as one makes size a register variable in 6.00A (worse), so left as is.
 * qa4 2026-10-09 BEHAVIOUR FIX (QA bug 4, shops stocked far fewer items): the original reads
 *   the uninitialised [bp-2] and STORES the clamped size back into it, so from the 2nd call of
 *   the 078e loop on it holds the previous size (<= 9) and the city size - 1 is used.  The
 *   draft read [bp-2] but kept the result in DI, so the slot stayed stale and size was always 1
 *   (rarest stock table).  `volatile int size` keeps size in memory at [bp-2] like the original
 *   (frame 8 instead of 6, q not in DI: still DIFF, but same behaviour).  Verified in-game with
 *   tools/playtests/market_stock.txt.
 * m2 2026-10-09: 8/11, DATA MATCHES (stamped --wip).  CONST is 4 slots (7e66..7e6d); 7e6e is
 *   s033's first slot, not ours.  10be gets no slot once each flag test reads
 *   F_2e38_10be[D_907e]->flags itself (if/else-if chain, rarity += 3 twice; MSC CSEs the byte
 *   into the [bp-16h] temp and loads DS=2e38 for the block).  192e MATCHED: + `if (rarity <= 5)
 *   a / -100; else a / 100;` (cross-jumped pushes of the long divisor), `(int)price < 1`, decl
 *   order rarity,size,price,value,q.  0f60 MATCHED: no `id` local in the sell case (repeat the
 *   item expression: MSC's CSE temps give [bp-88h]/[bp-8ah]) + a named id in the purchase
 *   case; D_86a0 extern before D_8832.  078e MATCHED: `int y[1]` (aggregate -> lower slot).
 *   Left: 1b26 only k/D_907e-cache in di/si swapped (orig k=di); 1d18 orig keeps q in di and
 *   size in [bp-2], 6.00A never enregisters q; 0000 (big function) 6.00A gives row bx and
 *   mkey memory (orig row di, mkey si) - pragma l off, &shop, struct {hover; buf} help. */
/* data: _DATA=2f18 */
/* data: CONST=7e66 */
/* Barter screen (overlay s032): buy and sell items at a shop. */
#include "darklands/card.h"
#include "darklands/game.h"

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

extern int back_win_font, screen_win_font, back_win_colour, D_e7a2, D_e7a0;
extern char D_9920, D_9934;
extern char D_907c, leader;
extern int D_884e, D_884c, cur_loc;
extern int party_seed;
extern int screen_win_colour;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char screen_win[], back_win[], cur_palette[], D_9419[];
extern int party_money, party_money_gr, party_money_pf;
extern int party_icons_dirty;
extern struct member party[];
extern struct city far *city_rec;
extern char far *D_869c;            /* shop name */
extern long D_8868;                 /* item categories sold */
extern int D_8834;                  /* shop quality */
extern int D_8836;                  /* shop */
extern int D_86a0;                  /* shop list: first visible entry */
extern int D_8832;                  /* shop list: cursor row */
extern int D_885c;                  /* number of items for sale */
extern int D_8850;
extern int D_86a2[];                /* item types for sale */
extern int D_8838[], D_8842[], D_8852[], D_885e[];

extern char far * far F_290e_207f[];
extern struct fchar far data_file[];
extern struct itemdef far * far item_defs[];
extern unsigned char far F_290e_399b[][12];
extern struct loc far * far locations[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
void srand(unsigned);
time_t time(time_t *);
int far random_n(int);

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
int far rand_mod(int);
void far pic_draw_pal(int, char *, char *);
long far money_pfennigs(int *);
void far money_set(struct money *, int, int, int);
int far money_cmp(int *, struct money *);
void far money_add_fgp(int *, int, int, int);
void far money_sub_fgp(int *, int, int, int);
int far clamp_range(int, int, int);
int far member_attr(int, int);
int far cur_loc_query(int);
int far party_get(int);
int far member_sheet_vec(int);
int far item_take_one_vec(int, int);
int far add_item_rec(int, struct item);

int far sub_s032_1462_076c(int ret, int shop);
void far sub_s032_1462_078e(int who, char far *name, long cats);
void far sub_s032_1462_0b30(int who, char far *name, long cats);
int far sub_s032_1462_0efa(int k);
int far sub_s032_1462_0f60(int key);
void far sub_s032_1462_1770(int who);
void far sub_s032_1462_1870(int unused);
int far buy_price(int k);
int far sell_price(int k, int qual);
int far shop_has_item(int k);

extern char str_sell[], str_buy[], str_space[];

/* barter_screen (barter_screen): main loop of the barter screen for shop `shop` */
#pragma optimize("l", off)
void far barter_screen(int shop, char far *name, long cats)
{
    struct { int hover; char buf[136]; } l;
    int row;
    int mkey;

    &shop;
    l.hover = -1;
    D_9920 = 2;
    D_9934 = 2;
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    back_win_font = 1;
    screen_win_font = 1;
    D_907c = 5;
    D_884c = D_884e = leader;
    screen_win_colour = 8;
    D_869c = name ? name : F_290e_207f[shop];
    D_8868 = cats;
    if ((D_8836 = shop) < 0 || shop >= 9)
        D_8834 = 25;
    else
        D_8834 = city_rec->quality[shop];
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
    sub_s032_1462_078e(leader, D_869c, cats);
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    for (;;) {
        mkey = 0;
        if (!(mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            row = (mouse_y - 0x1b) / 8;
            if (row != l.hover) {
                mouse_hide();
                gfx_recolor_rect(screen_win, 0x50, l.hover * 8 + 0x1b, 0x96, 8, 9, 8);
                l.hover = row;
                gfx_recolor_rect(screen_win, 0x50, l.hover * 8 + 0x1b, 0x96, 8, 8, 9);
                mouse_show();
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0x50 && mouse_x < 0xe6 && mouse_y > 0x1b && mouse_y < 0x43) {
            while (mouse_buttons & 1)
                ;
            row = (mouse_y - 0x1b) / 8;
            switch (row) {
            case 0: mkey = 0x1950; break;
            case 1: mkey = 0x1f53; break;
            case 2: mkey = 0x1e41; break;
            case 3: mkey = 0x3042; break;
            case 4: mkey = 0x11b; break;
            }
        } else if (!(mouse_buttons & 1) && l.hover != -1) {
            mouse_hide();
            gfx_recolor_rect(screen_win, 0x50, l.hover * 8 + 0x1b, 0x96, 9, 9, 8);
            l.hover = -1;
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_x > 0x72 && mouse_x < 0x11c && mouse_y > 0x93 && mouse_y < 0xb3) {
            mouse_hide();
            D_8842[D_884e] = 0;
            D_885e[D_884e] = (mouse_y - 0x93) >> 3;
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_x > 0x52 && mouse_x < 0x10b && mouse_y > 0x55 && mouse_y < 0x75) {
            mouse_hide();
            D_8842[D_884e] = 1;
            D_8832 = (mouse_y - 0x55) >> 3;
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0xfa, 0x78, 9, 8);
            gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xfa, 8, 8, 9);
            mouse_show();
            while (mouse_buttons & 1)
                ;
        }
        if ((mouse_buttons & 1) && mouse_y > 0x8a && mouse_y < 0x93 && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x4800;
            D_8842[D_884e] = 0;
            screen_win_colour = 7;
            mouse_hide();
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_y > 0xb3 && mouse_y < 0xbc && mouse_x > 0x72 && mouse_x < 0x12b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x5000;
            D_8842[D_884e] = 0;
            mouse_hide();
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_y > 0x4c && mouse_y < 0x55 && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x4800;
            D_8842[D_884e] = 1;
            mouse_hide();
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_y > 0x75 && mouse_y < 0x7e && mouse_x > 0x52 && mouse_x < 0x10b) {
            while (mouse_buttons & 1)
                ;
            mkey = 0x5000;
            D_8842[D_884e] = 1;
            mouse_hide();
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            mouse_show();
        }
        if ((mouse_buttons & 1) && mouse_x < 0x3c) {
            row = mouse_y / 40;
            row = member_sheet_vec(row);
            sub_root_047c_0568();
            pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
            set_palette(cur_palette);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s032_1462_0b30(D_884e, D_869c, cats);
            sub_root_047c_056f();
            while (mouse_buttons & 1)
                ;
        }
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            sub_root_047c_0568();
            pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
            set_palette(cur_palette);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s032_1462_0b30(D_884e, D_869c, cats);
            sub_root_047c_056f();
        }
        if (key_pressed())
            mkey = get_key();
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

#pragma optimize("", on)
char str_sell[] = "\x8c S\xff" "ell an item";
char str_buy[] = "\x8c P\xff" "urchase an item";
char str_space[] = " ";

/* sub_s032_1462_076c (barter_close): restore the screen state on leaving */
int far sub_s032_1462_076c(int ret, int shop)
{
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    D_9920 = 1;
    D_9934 = 1;
    leader = D_884c;
    return ret;
}

/* sub_s032_1462_078e (draw_barter_screen): draw the screen and collect the items the shop sells */
#pragma optimize("a", off)
void far sub_s032_1462_078e(int who, char far *name, long cats)
{
    char buf[120];
    char self[10];
    char he[10];
    int y[1];
    int i;
    int k;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    set_palette(cur_palette);
    gfx_copy_rect(back_win, 0x3c, 0, 0x104, 0xc8, screen_win, 0x3c, 0);
    back_win_colour = 8;
    screen_win_colour = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", party[leader].name,
            party_money, party_money_gr, party_money_pf, money_pfennigs(&party_money));
    gfx_text(screen_win, 0x49, 9, buf);
    if (party[leader].female == 0) {
        strcpy(he, "He");
        strcpy(self, "himself");
    } else {
        strcpy(he, "She");
        strcpy(self, "herself");
    }
    if (leader != D_884e)
        sprintf(buf, "%s barters for %s to the %Fs", he, party[who].name, name);
    else
        sprintf(buf, "%s barters for %s to the %Fs", party[who].name, self, name);
    gfx_text(screen_win, 0x49, 0x12, buf);
    gfx_text(screen_win, 0x50, 0x1b, str_buy);
    screen_win_colour = 7;
    gfx_text(screen_win, 0x50, 0x23, str_sell);
    gfx_text(screen_win, 0x50, 0x2b, str_space);
    screen_win_colour = 8;
    gfx_text(screen_win, 0x50, 0x33, "\x8c B\xff" "arter for another person");
    gfx_text(screen_win, 0x50, 0x3b, "\x8c L\xff" "eave");
    sprintf(buf, "The %Fs offers...", name);
    gfx_text(screen_win, 0x7a, 0x45, buf);
    sprintf(buf, "%s currently has...", party[who].name);
    gfx_text(screen_win, 0x81, 0x83, buf);
    for (i = 0, y[0] = 0x93; y[0] < 0xb3 && i < party[who].nitems; i++, y[0] += 8) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sell_price(data_file[who].items[i].id & 0xfff, data_file[who].items[i].qual),
                item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, y[0], buf);
    }
    if (cur_loc < 0x5c && cur_loc != -1)
        srand(city_rec->seed + party_seed);
    else
        srand(party_seed + cur_loc);
    for (k = D_8850 = 0; k < 200; k++) {
        if (shop_has_item(k)) {
            if (D_8850 < 4) {
                sprintf(buf, "%5upf  %Fs  %3dq  %3dlbs", buy_price(k), item_defs[k],
                        sub_s032_1462_0efa(k), item_defs[k]->weight);
                gfx_text(screen_win, 0x55, (D_8850 - D_8852[D_884e]) * 8 + 0x55, buf);
            }
            D_86a2[D_8850] = k;
            D_8850++;
        }
    }
    srand((unsigned)time(0));
    gfx_recolor_rect(screen_win, 0x53, 0x55, 0xb8, 8, 8, 9);
    D_8838[D_884e] = party[who].nitems;
    D_885c = D_8850;
    mouse_show();
}
#pragma optimize("", on)

/* sub_s032_1462_0b30 (redraw_barter_screen): redraw the screen (other party member or after a change) */
void far sub_s032_1462_0b30(int who, char far *name, long cats)
{
    char buf[120];
    char self[10];
    char he[10];
    int i;

    pic_draw_pal(1, "pics\\buysell.pic", cur_palette);
    mouse_hide();
    gfx_copy_rect(back_win, 0x3c, 0, 0x104, 0xc8, screen_win, 0x3c, 0);
    screen_win_colour = 8;
    sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", party[leader].name,
            party_money, party_money_gr, party_money_pf, money_pfennigs(&party_money));
    gfx_text(screen_win, 0x49, 9, buf);
    if (party[leader].female == 0) {
        strcpy(he, "He");
        strcpy(self, "himself");
    } else {
        strcpy(he, "She");
        strcpy(self, "herself");
    }
    if (leader != D_884e)
        sprintf(buf, "%s barters for %s to the %Fs", he, party[who].name, name);
    else
        sprintf(buf, "%s barters for %s to the %Fs", party[who].name, self, name);
    gfx_text(screen_win, 0x4e, 0x12, buf);
    screen_win_colour = D_8842[D_884e] ? 8 : 7;
    gfx_text(screen_win, 0x50, 0x1b, str_buy);
    screen_win_colour = D_8842[D_884e] ? 7 : 8;
    gfx_text(screen_win, 0x50, 0x23, str_sell);
    screen_win_colour = 7;
    gfx_text(screen_win, 0x50, 0x2b, str_space);
    screen_win_colour = 8;
    gfx_text(screen_win, 0x50, 0x33, "\x8c B\xff" "arter for another person");
    gfx_text(screen_win, 0x50, 0x3b, "\x8c L\xff" "eave");
    sprintf(buf, "The %Fs offers...", name);
    gfx_text(screen_win, 0x7a, 0x45, buf);
    sprintf(buf, "%s has...", party[who].name);
    gfx_text(screen_win, 0x81, 0x83, buf);
    for (i = D_8852[D_884e]; i < D_8852[D_884e] + 4 && i < party[who].nitems; i++) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sell_price(data_file[who].items[i].id & 0xfff, data_file[who].items[i].qual),
                item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, (i - D_8852[D_884e]) * 8 + 0x93, buf);
    }
    for (i = D_86a0; i < D_86a0 + 4 && i < D_885c; i++) {
        sprintf(buf, "%5upf  %Fs  %3dq  %3dlbs", buy_price(D_86a2[i]), item_defs[D_86a2[i]],
                sub_s032_1462_0efa(D_86a2[i]), item_defs[D_86a2[i]]->weight);
        gfx_text(screen_win, 0x55, (i - D_86a0) * 8 + 0x55, buf);
    }
    if (D_8842[D_884e])
        gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
    else
        gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
    D_8838[D_884e] = party[who].nitems;
    mouse_show();
}

/* sub_s032_1462_0efa (item_quality): quality of the items of type k the shop offers */
int far sub_s032_1462_0efa(int k)
{
    struct itemdef far *d;

    if (item_defs[k]->type == 0x40 || item_defs[k]->type == 0x41 || item_defs[k]->type == 0x42 || item_defs[k]->cats == 0x400L ||
        item_defs[k]->type == 0x17 || item_defs[k]->type == 0x18 || item_defs[k]->type == 0x19)
        return 25;
    return (item_defs[k]->qual + D_8834) / 2;
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
        quit_to_dos();
        break;
    case 0x1950:
    case 0x1970:            /* P: purchase */
        if (D_8842[D_884e] == 0)
            break;
        if (D_885c == 0)
            break;
        money_set(&price, 0, 0, buy_price(D_86a2[D_8832 + D_86a0]));
        if (money_cmp(&party_money, &price) == -1)
            break;
        id = D_86a2[D_8832 + D_86a0];
        it.id = id;
        it.f2 = item_defs[id]->type;
        it.qual = sub_s032_1462_0efa(id);
        it.f5 = item_defs[D_86a2[D_8832 + D_86a0]]->weight;
        it.cnt = 1;
        money_sub_fgp(&party_money, 0, 0, buy_price(D_86a2[D_8832 + D_86a0]));
        gfx_copy_rect(back_win, 0x49, 9, 0xee, 9, screen_win, 0x49, 9);
        screen_win_colour = 8;
        sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", party[leader].name,
                party_money, party_money_gr, party_money_pf, money_pfennigs(&party_money));
        gfx_text(screen_win, 0x49, 9, buf);
        D_8838[D_884e] = add_item_rec(D_884e, it);
        sub_s032_1462_1770(D_884e);
        gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0x5f, 8, 8, 9);
        break;
    case 0x1f53:
    case 0x1f73:            /* S: sell */
        if (D_8842[D_884e] != 0)
            break;
        if (D_8838[D_884e] == 0)
            break;
        if (item_defs[data_file[D_884e].items[D_885e[D_884e] + D_8852[D_884e]].id]->flags & 0x80)
            break;
        money_add_fgp(&party_money, 0, 0,
                           sell_price(data_file[D_884e].items[D_885e[D_884e] + D_8852[D_884e]].id, data_file[D_884e].items[D_885e[D_884e] + D_8852[D_884e]].qual));
        gfx_copy_rect(back_win, 0x49, 9, 0xee, 9, screen_win, 0x49, 9);
        screen_win_colour = 8;
        sprintf(buf, "%s holds the purse of %dfl, %dgr, %dpf (%lupf).", party[leader].name,
                party_money, party_money_gr, party_money_pf, money_pfennigs(&party_money));
        gfx_text(screen_win, 0x49, 9, buf);
        D_8838[D_884e] = item_take_one_vec(D_884e, D_885e[D_884e] + D_8852[D_884e]);
        if (D_885e[D_884e] + D_8852[D_884e] >= D_8838[D_884e]) {
            if (D_8852[D_884e] != 0)
                D_8852[D_884e]--;
            else if (D_885e[D_884e] != 0)
                D_885e[D_884e]--;
        }
        sub_s032_1462_1770(D_884e);
        gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        break;
    case 0x2044:
    case 0x2064:            /* D */
        if (D_8842[D_884e] != 0)
            break;
        D_8838[D_884e] = item_take_one_vec(leader, D_885e[D_884e] + D_8852[D_884e]);
        sub_s032_1462_1770(D_884e);
        gfx_recolor_rect(screen_win, 0x50, D_885e[D_884e] * 8 + 0x1b, 0x5f, 8, 8, 9);
        break;
    case 0x264c:
    case 0x266c:            /* L: leave */
        return 0x11b;
    case 0x3042:
    case 0x3062:            /* B: barter for another person */
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        do {
            D_884e++;
            D_884e %= 5;
        } while (party[D_884e].present == 0);
        sub_s032_1462_0b30(D_884e, D_869c, D_8868);
        gfx_recolor_rect(screen_win, 0x50, 0x33, 0xc8, 9, 8, 9);
        break;
    case 0x4800:            /* up */
        mouse_hide();
        if (D_8842[D_884e])
            D_8832--;
        else
            D_885e[D_884e]--;
        if (D_8842[D_884e] && D_8832 < 0 && D_86a0 > 0) {
            D_8832++;
            D_86a0--;
            sub_s032_1462_1870(cur_loc);
            gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_8842[D_884e] && D_885e[D_884e] < 0 && D_8852[D_884e] > 0) {
            D_885e[D_884e]++;
            D_8852[D_884e]--;
            sub_s032_1462_1770(D_884e);
            gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_8832 < 0)
                D_8832 = 0;
            if (D_885e[D_884e] < 0)
                D_885e[D_884e] = 0;
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0x103, 0x69, 9, 8);
            if (D_8842[D_884e])
                gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xc3, 8, 8, 9);
            else
                gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        }
        mouse_show();
        break;
    case 0x5000:            /* down */
        mouse_hide();
        if (D_8842[D_884e])
            D_8832++;
        else
            D_885e[D_884e]++;
        if (D_8842[D_884e] && D_8832 >= 4 && D_86a0 + D_8832 < D_885c) {
            D_8832--;
            D_86a0++;
            sub_s032_1462_1870(cur_loc);
            gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
        } else if (!D_8842[D_884e] && D_885e[D_884e] >= 4 && D_885e[D_884e] + D_8852[D_884e] < D_8838[D_884e]) {
            D_885e[D_884e]--;
            D_8852[D_884e]++;
            sub_s032_1462_1770(D_884e);
            gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
        } else {
            if (D_8832 >= D_885c)
                D_8832--;
            if (D_8832 >= 4)
                D_8832 = 3;
            if (D_885e[D_884e] >= 4)
                D_885e[D_884e] = 3;
            gfx_recolor_rect(screen_win, 0x3c, 0x55, 0x103, 0x6e, 9, 8);
            if (D_8842[D_884e])
                gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xc8, 8, 8, 9);
            else
                gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xc8, 8, 8, 9);
        }
        mouse_show();
        break;
    case 0x5e00:
    case 0x5f00:
    case 0x6000:
    case 0x6100:
    case 0x6200:            /* Ctrl-F1..F5: choose the party member who pays */
        key = (key >> 8) - 0x5e;
        if (party[key].present == 0)
            break;
        leader = key;
        party_panel_rows(0x100);
        /* fall through */
    case 0x4b00:
    case 0x4d00:            /* left / right: switch lists */
        mouse_hide();
        if (D_8842[D_884e]) {
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 9, 8);
            gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 8, 9);
            D_8842[D_884e] = 0;
        } else {
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x23, str_sell);
            screen_win_colour = 8;
            gfx_text(screen_win, 0x50, 0x1b, str_buy);
            screen_win_colour = 7;
            gfx_text(screen_win, 0x50, 0x2b, str_space);
            gfx_recolor_rect(screen_win, 0x72, D_885e[D_884e] * 8 + 0x93, 0xb9, 8, 9, 8);
            gfx_recolor_rect(screen_win, 0x52, D_8832 * 8 + 0x55, 0xb9, 8, 8, 9);
            D_8842[D_884e] = 1;
            screen_win_colour = 8;
        }
        mouse_show();
        break;
    }
    return key;
}

/* sub_s032_1462_1770 (draw_member_items): list four of the member's items with their sell prices */
void far sub_s032_1462_1770(int who)
{
    char buf[60];
    int i;

    mouse_hide();
    screen_win_colour = 8;
    gfx_copy_rect(back_win, 0x72, 0x93, 0xb6, 0x20, screen_win, 0x72, 0x93);
    for (i = D_8852[D_884e]; i < D_8852[D_884e] + 4 && i < party[who].nitems; i++) {
        sprintf(buf, "%5upf  %Fs  (%3d) %2dq",
                sell_price(data_file[who].items[i].id & 0xfff, data_file[who].items[i].qual),
                item_defs[data_file[who].items[i].id & 0xfff],
                data_file[who].items[i].cnt, data_file[who].items[i].qual);
        gfx_text(screen_win, 0x75, (i - D_8852[D_884e]) * 8 + 0x93, buf);
    }
    mouse_show();
}

/* sub_s032_1462_1870 (draw_shop_items): list four of the items for sale with their prices */
void far sub_s032_1462_1870(int unused)
{
    char buf[120];
    int i;

    mouse_hide();
    screen_win_colour = 8;
    gfx_copy_rect(back_win, 0x52, 0x55, 0xb9, 0x20, screen_win, 0x52, 0x55);
    for (i = D_86a0; i < D_86a0 + 4 && i < D_885c; i++) {
        sprintf(buf, "%3dpf  %Fs  %3dq  %3dlbs", buy_price(D_86a2[i]), item_defs[D_86a2[i]],
                sub_s032_1462_0efa(D_86a2[i]), item_defs[D_86a2[i]]->weight);
        gfx_text(screen_win, 0x55, (i - D_86a0) * 8 + 0x55, buf);
    }
    mouse_show();
}

/* buy_price (buy_price): price in pfennigs the shop asks for an item of type k */
int far buy_price(int k)
{
    long rarity;
    long size;
    long price;
    long value;
    long q;

    value = item_defs[k]->value;
    q = sub_s032_1462_0efa(k);
    rarity = item_defs[k]->rarity;
    size = cur_loc < 0x5c && cur_loc != -1 ? city_rec->size : 2;
    if (cur_loc != -1) {
        if (locations[cur_loc]->flags & 2)
            rarity += 3;
        else if (locations[cur_loc]->flags & 0x41)
            rarity += 3;
        else if (locations[cur_loc]->flags & 8)
            q += rand_mod(10);
    }
    price = value * q * 2 / 25;
    if (cur_loc != -1) {
        if (rarity <= 5)
            price += size * price * 2 / -100;
        else
            price += size * price * 2 / 100;
    }
    price += price * (25 - member_attr(leader, 5)) / 100;
    if (cur_loc != -1)
        price += price * (clamp_range(-999, 99, cur_loc_query(0) * 50) / 100) / -100;
    price += price * (clamp_range(0, 500, party_get(4) * 100) / 1000) / -100;
    if ((int)price < 1)
        price = 1;
    return price;
}

/* sell_price (sell_price): price in pfennigs the shop pays for an item of type k and quality qual */
int far sell_price(int k, int qual)
{
    long rarity;
    long price;
    long size;
    long value;

    if (item_defs[k]->flags & 0x80)
        return 0;
    value = item_defs[k]->value / 4;
    rarity = item_defs[k]->rarity;
    size = cur_loc < 0x5c && cur_loc != -1 ? city_rec->size : 2;
    if (cur_loc != -1) {
        if (locations[cur_loc]->flags & 2)
            rarity += 3;
        else if (locations[cur_loc]->flags & 0x41)
            rarity += 3;
        else if (locations[cur_loc]->flags & 8)
            qual += rand_mod(10);
    }
    price = qual * value * 2 / 25;
    if (cur_loc != -1) {
        if (rarity <= 5)
            price += price * size * 2 / -100;
        else
            price += price * size * 2 / 100;
    }
    price += price * (25 - member_attr(leader, 5)) / -100;
    if (cur_loc != -1)
        price += price * (clamp_range(-999, 75, cur_loc_query(0) * 50) / 100) / 100;
    price += price * (clamp_range(0, 500, party_get(4) * 100) / 1000) / 100;
    if ((int)price < 1)
        price = 0;
    return price;
}

/* shop_has_item (shop_has_item): whether the shop offers items of type k today */
int far shop_has_item(int k)
{
    volatile int size;          /* MUST stay in memory at [bp-2]: see status comment */
    int r;
    int q;

    if ((item_defs[k]->cats & D_8868) && !(item_defs[k]->flags & 0x80)) {
        if (item_defs[k]->type == 0x40 || item_defs[k]->type == 0x41 || item_defs[k]->type == 0x42)
            return 1;
        size = size < 0x5c && size != -1 ? city_rec->size - 1 : 1;
        if (size > 9)
            size = 9;
        r = item_defs[k]->rarity;
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
