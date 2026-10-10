/* place: s034 1550:0006 */
/* orig: size=0c86 CONST=7e88+000c _DATA=362c+018e */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* Party information screen: fame, date, wealth and the map of locations (overlay s034). */
#include "darklands/card.h"
#include "darklands/game.h"

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

extern int cur_loc;
extern int D_e7d6;
extern int party_x, party_y;
extern int back_win_font, screen_win_font, back_win_colour, screen_win_colour;
extern int D_e7a2, D_e7a0;
extern int cursor_default, party_icons_dirty;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char cur_palette[];
extern struct gfx back_win, screen_win;
extern int party_fame;
extern int cur_hour, cur_day, cur_month;
extern int party_money[];
extern int D_9072, D_9078;
extern struct loc far * far locations[];
extern char far * far F_290e_2183[];
extern char far * far F_290e_1f7b[];
extern char far * far bell_names[];
extern char far * far F_290e_1d43[];
extern char far * far F_290e_20e3[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *_itoa(int, char *, int);
char far *_fstrcpy(char far *, const char far *);
char far *_fstrcat(char far *, const char far *);
int abs(int);

int far nearest_city(int, int);
int far vec_1fbd(int, int, int, int);
void far mouse_set_cursor(int, int, int);
int far key_pressed(void);
int far get_key(void);
void far gfx_copy_rect(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far gfx_text(struct gfx *, int, int, char *);
int far loc_query(int, int);
void far sub_root_047c_1320(int, int, int, int);
void far pic_draw_pal(int, char *, char *);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far mouse_hide(void);
void far mouse_show(void);
void far set_palette(char *);
void far mouse_move(int, int);
int far hour_band(int);
void far draw_text_far(struct gfx *, int, int, char far *);
long far money_pfennigs(int *);
void far sub_root_047c_0545(void);
int far pic_load(char *);
void far sub_root_047c_04f8(void);
void far gfx_draw_pic(struct gfx *, int, int, int);
void far mem_free(int, char *);
int far char_width(int, char);

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
    if (cur_loc != -1 && cur_loc <= 0x5c)
        D_e7d6 = cur_loc;
    else
        D_e7d6 = nearest_city(party_x, party_y);
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    mouse_set_cursor(0, 0, cursor_default);
    party_info_bg();
    party_info_draw();
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if (mouse_x > 0xc6 && mouse_x < 0x140 && mouse_y > 0x4a && mouse_y < 0xc3) {
            x = mouse_x - 0xc6;
            y = mouse_y - 0x4a;
            loc = party_info_nearest(x - 3, y + 3);
            if (loc != last && loc != -1) {
                last = loc;
                gfx_copy_rect(&back_win, 0xc8, 0x12, 0x78, 0x34, &screen_win, 0xc8, 0x12);
                screen_win_colour = 14;
                _fstrcpy(buf, locations[loc]->name);
                gfx_text(&screen_win, 0x102 - text_width(buf) / 2, 0x22, buf);
                switch (locations[loc]->size) {
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
                gfx_text(&screen_win, 0x102 - (w = text_width(buf) / 2), 0x2c, buf);
                screen_win_colour = 14;
                strcpy(buf, "Rep: ");
                _fstrcat(buf, F_290e_2183[loc_query(loc, 1)]);
                gfx_text(&screen_win, 0x102 - (w = text_width(buf) / 2), 0x36, buf);
                sprintf(rep, "(%Fd)", locations[loc]->rep);
                gfx_text(&screen_win, 0x102 - (w = text_width(rep) / 2), 0x40, rep);
            } else if (loc == -1 && last != -1) {
                gfx_copy_rect(&back_win, 0xc8, 0x12, 0x78, 0x34, &screen_win, 0xc8, 0x12);
                last = -1;
            }
        }
        if (key_pressed())
            key = get_key();
        if ((key == 0 || key == 0xf09 || key == 0xf00) && !(mouse_buttons & 1)) {
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
    party_icons_dirty = 1;
    while (mouse_buttons & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

/* Load the screen background. */
void far party_info_bg(void)
{
    pic_draw_pal(1, "pics\\ptystats.pic", cur_palette);
    back_win_colour = 14;
    back_win_font = 1;
    screen_win_font = 1;
}

/* Draw the static parts of the screen (titles in two colours for a shadow effect). */
void far party_info_draw(void)
{
    char fame[40];
    char num[20];
    int level;
    unsigned w;

    party_info_map(&back_win);
    party_info_wealth(&back_win);
    level = party_fame / 50;
    if (level >= 10)
        level = 10;
    _fstrcpy(fame, F_290e_1f7b[level]);
    w = text_width(fame);
    back_win_colour = 0xbf;
    gfx_text(&back_win, 0x86, 6, "PARTY INFORMATION");
    gfx_text(&back_win, 0xd7, 0x13, "MAP INFORMATION");
    gfx_text(&back_win, 0x6b, 0x13, "PARTY FAME");
    gfx_text(&back_win, 0x83 - w / 2, 0x1b, fame);
    gfx_text(&back_win, 0x80, 0x23, _itoa(party_fame, num, 10));
    gfx_text(&back_win, 0x54, 0x31, "TIME");
    gfx_text(&back_win, 0x95, 0x31, "DATE");
    gfx_text(&back_win, 0x4b, 0x60, "LOCATION");
    gfx_text(&back_win, 0x4c, 0x8e, "WEALTH");
    gfx_text(&back_win, 0x89, 0x8e, "NOTES");
    gfx_text(&back_win, 0x89, 0x60, "LOCAL REP");
    back_win_colour = 5;
    gfx_text(&back_win, 0x85, 5, "PARTY INFORMATION");
    gfx_text(&back_win, 0xd6, 0x12, "MAP INFORMATION");
    gfx_text(&back_win, 0x6a, 0x12, "PARTY FAME");
    gfx_text(&back_win, 0x82 - w / 2, 0x1a, fame);
    gfx_text(&back_win, 0x7f, 0x22, _itoa(party_fame, num, 10));
    gfx_text(&back_win, 0x53, 0x30, "TIME");
    gfx_text(&back_win, 0x94, 0x30, "DATE");
    gfx_text(&back_win, 0x4a, 0x5f, "LOCATION");
    gfx_text(&back_win, 0x4b, 0x8d, "WEALTH");
    gfx_text(&back_win, 0x88, 0x8d, "NOTES");
    gfx_text(&back_win, 0x88, 0x5f, "LOCAL REP");
    sub_root_047c_0568();
    mouse_hide();
    set_palette(cur_palette);
    gfx_copy_rect(&back_win, 0x3c, 0, 0x104, 0xc8, &screen_win, 0x3c, 0);
    mouse_show();
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
        mouse_hide();
        mouse_move(116L * locations[D_e7d6]->x / 327 + 0xca,
                           106L * locations[D_e7d6]->y / 931 + 0x47);
        mouse_show();
        gfx_copy_rect(&back_win, 0xc8, 0x12, 0x7a, 0x32, &screen_win, 0xc8, 0x12);
        screen_win_colour = 14;
        _fstrcpy(buf, locations[D_e7d6]->name);
        gfx_text(&screen_win, 0x102 - text_width(buf) / 2, 0x22, buf);
        switch (locations[D_e7d6]->size) {
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
        gfx_text(&screen_win, 0x102 - (w = text_width(buf) / 2), 0x2c, buf);
        screen_win_colour = 14;
        strcpy(buf, "Rep: ");
        _fstrcat(buf, F_290e_2183[loc_query(D_e7d6, 1)]);
        gfx_text(&screen_win, 0x102 - (w = text_width(buf) / 2), 0x36, buf);
        sprintf(buf, "(%Fd)", locations[D_e7d6]->rep);
        gfx_text(&screen_win, 0x102 - (w = text_width(buf) / 2), 0x40, buf);
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

    mouse_hide();
    g->color = 14;
    _fstrcpy(name, bell_names[hour_band(cur_hour)]);
    w = text_width(name) / 2;
    draw_text_far(g, 0x60 - w, 0x40, bell_names[hour_band(cur_hour)]);
    gfx_text(g, 0x58, 0x48, _itoa(cur_hour, num, 10));
    g->color = 14;
    _fstrcpy(name, F_290e_1d43[cur_month]);
    w = text_width(name) / 2;
    draw_text_far(g, 0xa0 - w, 0x40, F_290e_1d43[cur_month]);
    gfx_text(g, 0x98, 0x48, _itoa(cur_day, num, 10));
    g->color = 14;
    if (cur_loc != -1 && cur_loc < 0x5c) {
        draw_text_far(g, 0x48, 0x6f, locations[cur_loc]->name);
    } else {
        near_ = nearest_city(party_x, party_y);
        sprintf(buf, "%Fs of ", F_290e_20e3[vec_1fbd(locations[near_]->x, locations[near_]->y, party_x, party_y)]);
        draw_text_far(g, 0x46, 0x6f, (char far *)buf);
        draw_text_far(g, 0x48, 0x78, locations[near_]->name);
    }
    g->color = 14;
    sprintf(buf, "%d Florins", party_money[0]);
    gfx_text(g, 0x43, 0x9d, buf);
    sprintf(buf, "%d Groschen", party_money[1]);
    gfx_text(g, 0x43, 0xa5, buf);
    sprintf(buf, "%d Pfenniges", party_money[2]);
    gfx_text(g, 0x43, 0xad, buf);
    sprintf(buf, "(%ld) Total", money_pfennigs(party_money));
    gfx_text(g, 0x43, 0xb5, buf);
    g->color = 14;
    sprintf(buf, "%d Florins", D_9072);
    gfx_text(g, 0x84, 0x9d, buf);
    sprintf(buf, "%d PhStone", D_9078);
    gfx_text(g, 0x84, 0xad, buf);
    g->color = 14;
    if (cur_loc != -1 && cur_loc < 0x5c) {
        draw_text_far(g, 0x84, 0x6f, F_290e_2183[loc_query(cur_loc, 1)]);
        sprintf(buf, "(%Fd)", locations[cur_loc]->rep);
        gfx_text(g, 0x8e, 0x77, buf);
    }
    mouse_show();
}

/* Mark the party's position on the small map. */
void far party_info_map(struct gfx *g)
{
    int x, y;
    int pic;

    if (cur_loc == -1) {
        x = 116L * party_x / 327;
        y = 106L * party_y / 931;
    } else {
        x = 116L * locations[cur_loc]->x / 327;
        y = 106L * locations[cur_loc]->y / 931;
    }
    sub_root_047c_0545();
    pic = pic_load("pics\\maploctr.pic");
    sub_root_047c_04f8();
    gfx_draw_pic(g, x + 0xc8, y + 0x46, pic);
    mem_free(pic, "b");
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
        if (locations[i]->type != 0)
            break;
        dx = abs(locations[i]->x - x);
        dy = abs(locations[i]->y - y) / 3;
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

    font = screen_win_font;
    w = 0;
    p = s;
    _asm {
        cmp byte ptr [si], 0
        je done
    next:
        lodsb
        push ax
        push font
        call char_width
        add sp, 4
        add di, ax
        cmp byte ptr [si], 0
        jne next
    done:
    }
    return w;
}
