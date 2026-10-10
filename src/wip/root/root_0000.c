/* place: root 0000:0000 */
/* orig: size=2652 _DATA=020a+0074 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: hand port (e3 rounds 3-4), every function written; 27/38 match (fmatch counts 38
   because 0000:2652 - sub_root_0265_0002 - belongs to this file, see below).
   Matching: 0bc2..0caa, 1080..1408 group, 14f6, 1588, 2286, 24bc.., 2648.  Open:
   - 0000 (480/468), 1438 (6.00 keeps the constant 2 in AL), 23e6 (m5: locals in one struct
     {tx, bx, bh, bw, by, ty} = original slots; left: 6.00 pushes `by` from AX before the r->x
     store, 6.00A pushes it from memory after);
   - 14f6 matches only in some symbol-table states (cmp [si+6],ax and the h + y operand
     order flip when lines above change): the dummy D_ffe0 in front of it selects one;
     re-check it after every edit above it;
   - 1716 MATCHED (m5): all memory locals in one struct in the original slot order
     (c, pad, name[40], save, pic, i, m, y, bits - first member lowest);
     18fa: only its jump table, i.e. the length of an earlier function;
   - 06c2/07d4/0984/0dd2/01d4 (classified 'asm' in functions.json, but they are plain MSC
     code: inline strlen with loop, enter/leave) written from the asm, lengths within a
     few bytes; 06c2 frame 8 vs 4 (two slots the old compiler kept), push [si] vs our
     reuse of DX; the if/else colour stores need #pragma optimize("a", on);
   - 1a9c (status column) cannot match: 6.00 saves SI/DI around the inline strcpy/strcat
     (known 6.00 vs 6.00A limit); written for documentation.
   2648/2652: 2648 calls 0000:2652 with push cs / call near, and the other callers reach
   2652 as 0000:2652 (lcall 0, 2652h), so the empty function at 2652 is the LAST function
   of this file, not of root 0265 (text.c, which really starts at 0265:0004).  Defined
   here as sub_root_0265_0002, 2648 matches; build/analysis/srcfiles.json still assigns
   it to the 0265 file (fmatch: 'len 2/0').  root 0000 and root 0265 are NOT one file:
   0000 calls the text functions with far calls.
   Game shell: menu bar handling, hot keys (0caa), "Game Paused" box, options toggles (sound
   effects / music / saintly changes / difficulty), "Quit to DOS? y/n", text boxes, party
   panel (1588 frames, 1716 rows, 18fa status letter, 1a9c bars). */

/* screen/window descriptor (D_0154 = off-screen copy, D_017c = visible screen) */
#include "darklands/card.h"
#include "darklands/game.h"
struct win {
    int f00;
    int w;          /* +0x02 */
    int f04;
    int x1;         /* +0x06 last column */
    int y1;         /* +0x08 last row */
    int f0a;
    int f0c;        /* +0x0c text colour */
    int f0e;        /* +0x0e background colour */
    int f10;        /* +0x10 font */
};
struct rect { int x; int y; int w; int h; };
struct res { int a; int b; };

/* menu bar entry (10 bytes), its items (0x14 bytes) and sub-items (0x16 bytes) */
struct subitem {
    char *name;                 /* +0x00 */
    char enabled;               /* +0x02 */
    char key[4];                /* +0x03 shortcut text */
    unsigned char count;        /* +0x07 quantity shown in front (>= 2) */
    char pad08[0x0a];
    void (far *fn)(void);       /* +0x12 */
};
struct item {
    char *name;                 /* +0x00 */
    char enabled;               /* +0x02 */
    char key[0x0b];             /* +0x03 shortcut text, right column */
    void (far *fn)(void);       /* +0x0e */
    struct subitem *sub;        /* +0x12 */
};
struct menu {
    char *name;                 /* +0x00 */
    int x0;                     /* +0x02 */
    int x1;                     /* +0x04 */
    char has_items;             /* +0x06 */
    char pad07;
    struct item *items;         /* +0x08 */
};

extern char far data_file[];
extern int D_000c[];
extern int D_000e[];
extern struct win *card_window2;
extern struct win *card_window;
extern char member_attrs[][0x80];
extern char D_9cb3[][0x80];
extern char D_9cb8[][0x80];
extern char D_9cb9[][0x80];
extern char D_9cba[][0x80];
extern char D_9cbf[][0x80];
extern char D_9c9f[][0x80];
extern char D_01bc;
extern char *D_01a8;
extern char *D_01aa;
extern char *D_01ac;
extern char *D_01ae;
extern char *D_01b0;
extern char real_combat;
extern char cbt_moving[];
extern char party[][0x80];
extern char D_9c93[][0x80];
extern int D_9c65[][0x40];
extern char leader;
extern int mouse_x;
extern int shield_pics[];
extern int D_a2e5;
extern int D_a626;
extern char D_a3d8[];
extern int cbt_flags[];
extern int order_action;
extern char selected_member;
extern char order_mode;
extern char cbt_floor[];
extern int D_a6c2;
extern unsigned char cbt_special[];
extern int D_a6d1;
extern struct win *D_0168;
extern struct win *D_0190;
extern int D_01e4[];
extern char cbt_selected[];
extern int D_a2e9;
extern int D_a33b;
extern int D_a33e;
extern int D_a392;
extern int D_a3d6;
extern int D_a3e5;
extern char *D_01a6;
extern char *D_01b2;
extern char *D_01b4;
extern char *D_01b6;
extern char *D_01b8;
extern char *D_01ba;
extern unsigned char opt_saint_msgs;
extern unsigned char opt_sound;
extern unsigned char opt_music;
extern int D_0204;
extern int D_0206;
extern char D_07d3;
extern char exit_requested;
extern unsigned char mouse_buttons;
extern char game_difficulty;
extern char D_907a;
extern char D_907d;
extern int D_9409;
extern int ems_frame_handle;
extern int D_940d;
extern int D_940f;
extern int D_9411;
extern int D_9413;
extern int D_9415;
extern int D_944b;
extern struct menu D_9419[4];
extern int D_9783;
extern int D_9789;
extern int D_987b;
extern char D_987f;
extern char D_9893;
extern char D_9920;
extern char D_9934;
extern char *D_995a;
extern char *D_996e;
extern char *D_9982;
extern int D_99e7;
extern int D_99e9;
extern int D_9a45;
extern char *D_99ec;
extern char *D_9a02;
extern char *D_9a18;
extern char D_9c26;
extern int D_9c27;
extern int D_9c29;
extern char D_9c2f;
extern char D_9c3c;
extern int D_9c3f;
extern char D_9c41;
extern int cursor_default;
extern int D_9c15[];
extern int D_9c1e;
extern int D_9c20;
extern unsigned char D_9c22[];
extern char D_a394;
extern int mouse_y;
extern int D_9c44;
extern int D_9c48;
extern int D_9c51;
extern int D_9c53;
extern int D_9c3a;
extern char D_9c4c;
extern int D_a2e3;
extern int D_a2e7;
extern int D_a340;
extern int D_a660;
extern int D_a662;
extern int D_a664;
extern int D_a666;
extern int cursor_current;
extern int D_a691;
extern int D_a693;
extern char ai_paused;

int far _fstrcpy(char far *, char far *);
int far _fstricmp(char far *, char far *);
void far exit_(int);
int far getch(void);
int far int86(int, void *, void *);
int far rand(void);
int far remove(char *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
void far copy_rect_saved(struct win *, int, int, int, int, struct win *, int, int);
void far draw_text(struct win *, int, int, char *);
void far draw_text_wrapped(struct win *, char *, int, int, int, int, int, int, int, int, int, int);
long far measure_text_wrapped(struct win *, char *, int, int, int, int, int, int, int, int, int);
void far itoa_commas(int, char *);
int far text_pixel_width(struct win *, char *);
void far sub_root_0410_0302(int);
void far sub_root_044a_0122(void);
void far sub_root_044a_01f4(void);
void far sub_root_047c_00a9(void);
int far font_height(int);
void far gfx_recolor_rect(struct win *, int, int, int, int, int, int);
void far gfx_copy_rect(struct win *, int, int, int, int, struct win *, int, int);
void far gfx_draw_pic(struct win *, int, int, int);
int far key_pressed(void);
int far get_key(void);
void far sound_play(int);
void far sub_root_047c_0791(void);
void far gfx_fill_rect(struct win *, int, int, int, int);
void far gfx_line(struct win *, int, int, int, int, int);
void far sub_root_047c_123f(void);
void far mouse_set_cursor(int, int, int);
void far mouse_show(void);
void far mouse_hide(void);
void far sub_root_05f8_0139(int);
void far file_load_far(char *, char far *);
void far catalog_open(char *);
void far catalog_close(void);
void far choose_party_order_vec(void);
void far save_game_vec(void);
void far load_game_vec(void);
void far party_info_screen_vec(void);
void far video_restore_vec(void);
void far vec_186f(void);

void far menu_bar_run(struct menu *m, int once);
void far menu_bar_track(struct menu *m);
void far menu_bar_draw(struct menu *m);
void far menu_dropdown_open(struct item *items, int x, int y);
void far menu_submenu_open(struct subitem *subs, int x, int y);
void far menu_dropdown_close(void);
void far menu_submenu_close(void);
int far rand_mod(int n);
int far rand_1_n(int n);
void far hot_keys(void);
void far sub_root_0000_0dd2(void);
void far sub_root_0000_1080(void);
void far toggle_saint_msgs(void);
void far toggle_music(void);
void far toggle_sound(void);
void far exit_if_requested(void);
void far game_shutdown(void);
void far flush_keys(void);
void far quit_to_dos(void);
void far set_difficulty_basic(void);
void far set_difficulty_standard(void);
void far set_difficulty_expert(void);
void far show_difficulty(int show);
int far copy_rect_clipped(struct win *src, int x, int y, int w, int h, struct win *dst,
                           int dx, int dy);
void far party_panel_frames(int which);
void far party_panel_rows(int which);
void far member_status_letter(int i, struct win *w);
void far member_bars(int i);
void far draw_frame_box(struct win *w, int x, int y, int cw, int ch, int bg);
void far text_box(struct win *w, int y, int save, char *text, struct win *dst,
                            struct rect *r);
void far show_message(char *text, int ticks);
int far ask_yes_no(char *question);
void far sub_root_0000_25f0(void);
void far datafile_select(char *name);
void far datafile_forget(void);
void far sub_root_0000_2646(void);
void far sub_root_0000_2648(void);
void far sub_root_0265_0002(int);

/* menu bar loop: runs until neither the mouse button (mouse_buttons bit 1) nor menu mode (D_9c26)
   is active; once != 0 only restores the screen colours.  While active it draws (06c2) and
   tracks (01d4) the bar; on release it closes the open dropdown (0bc2) and calls the chosen
   item's far handler (+0x0e) or sub-item handler (+0x12). */
void far menu_bar_run(struct menu *m, int once)
{
    int first;
    int c0c;
    int c10;
    int loop;
    int active;
    int bar;
    int item;
    int sub;
    int sel;
    struct item *it;
    struct subitem *si;

    first = 0;
    c0c = card_window->f0c;
    c10 = card_window->f10;
    loop = card_window->f10 = 1;
    if (once == 0) {
        if (key_pressed())
            hot_keys();
        if (D_07d3 == 0) {
            D_07d3 = 1;
            first = 1;
        }
        do {
            if (D_9c26 == 1 && (key_pressed() || (mouse_buttons & 2)))
                sub_root_0000_0dd2();
            if ((mouse_buttons & 2) || D_9c26 == 1)
                active = 1;
            else
                active = loop = 0;
            if (active == 1) {
                D_a2e3 = active;
                if (cursor_default != cursor_current && D_907a == 1 && D_9c4c == 1) {
                    mouse_hide();
                    mouse_set_cursor(0, 0, ai_paused == 0 ? D_a693 : D_a691);
                    cursor_current = cursor_default;
                    mouse_show();
                }
                menu_bar_draw(m);
                menu_bar_track(m);
            } else {
                D_a2e3 = active;
                if (active == 0 && D_9409 == 1) {
                    D_9409 = loop = 0;
                    bar = D_a2e7;
                    item = D_0204;
                    sub = D_0206;
                    sel = D_a340;
                    menu_dropdown_close();
                    if (D_944b == 1) {
                        D_944b = 0;
                        it = &m[bar].items[item];
                        if (it->enabled == 1 && D_9413 > item) {
                            card_window->f0c = c0c;
                            card_window->f10 = c10;
                            (*it->fn)();
                        }
                    }
                    if (D_9783 == 1) {
                        D_9783 = 0;
                        si = &m[bar].items[sel].sub[sub];
                        if (si->enabled == 1 && D_9415 > sub) {
                            card_window->f0c = c0c;
                            card_window->f10 = c10;
                            (*si->fn)();
                        }
                    }
                }
            }
        } while (loop);
    }
    card_window->f0c = c0c;
    card_window->f10 = c10;
    if (--first == 0)
        D_07d3 = 0;
}

/* menu tracking for one frame.  Position: the mouse (mouse_x - 0x3c, mouse_y) or, in keyboard
   menu mode, a position derived from the bar/item/sub-item cursor (D_a394 counts down the
   steps of opening a menu from the keyboard).  On the bar (y < 12) it highlights the entry
   under the cursor and opens its dropdown (07d4); inside the open dropdown it highlights
   the item row (D_0204, D_944b) and opens its sub-menu (0984); inside the sub-menu it
   highlights the sub-item row (D_0206, D_9783); elsewhere it removes the highlights */
void far menu_bar_track(struct menu *menus)
{
    int j;
    int y;
    int x;
    int row;
    int mx;
    int my;
    int sx;
    int iy;
    int sy;
    struct menu *m;
    struct item *it;

    if (D_9c26 == 0) {
        mx = mouse_x;
        my = mouse_y;
    } else {
        if (D_a394 == 3) {
            D_a394--;
        } else if (D_a394 == 2) {
            D_a394--;
            D_9c3c = 0;
        } else if (D_a394 == 1) {
            D_a394--;
            D_9c41 = 0;
        }
        sx = D_9c41 == -1 ? 0 : D_9c22[D_9c2f];
        mx = sx + D_9c15[D_9c2f] + 0x3c;
        iy = D_9c3c == -1 ? 0 : (D_9c3c << 3) + 0xc;
        sy = D_9c41 == -1 ? 0 : D_9c41 << 3;
        my = iy + sy + 4;
    }
    if (my < 0xc) {
        y = my;
        D_944b = 0;
        x = mx - 0x3c;
        for (j = 0, m = menus; j < D_9411; j++, m++) {
            if (x < m->x0 || m->x1 < x)
                continue;
            if (D_a2e7 != j) {
                mouse_hide();
                if (D_a2e7 >= 0 && D_a2e7 < D_9411)
                    gfx_recolor_rect(card_window, D_9c1e, 0, D_9c20 - D_9c1e + 1, 0xc, 9, 0);
                if (D_940f == 1) {
                    menu_submenu_close();
                    D_a340 = -0x63;
                }
                if (D_940d == 1) {
                    copy_rect_saved(card_window2, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, card_window,
                                    D_9789, 0xc);
                    D_940d = 0;
                    D_0204 = -1;
                }
                D_a2e7 = j;
                D_9c1e = m->x0;
                D_9c20 = m->x1;
                gfx_recolor_rect(card_window, D_9c1e, 0, D_9c20 - D_9c1e + 1, 0xc, 0, 9);
                if (menus[D_a2e7].has_items == 1)
                    menu_dropdown_open(menus[D_a2e7].items, menus[D_a2e7].x0, y);
                mouse_show();
            } else if (D_0204 >= 0 && D_9413 > D_0204) {
                mouse_hide();
                gfx_recolor_rect(card_window, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
                mouse_show();
                D_0204 = -1;
            }
        }
        return;
    }
    y = my;
    if (D_940d == 1 && (x = mx - 0x3c) >= D_9789 && x <= D_987b && my >= 0xc && D_99e9 >= my) {
        D_944b = 1;
        if ((row = (my - 0xc) / 8) != D_0204) {
            mouse_hide();
            if (D_0204 >= 0 && D_0204 < D_9413)
                gfx_recolor_rect(card_window, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
            if (D_940f == 1) {
                menu_submenu_close();
                D_a340 = -0x63;
            }
            if ((D_0204 = row) >= 0 && D_0204 < D_9413)
                gfx_recolor_rect(card_window, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 0, 9);
            it = &menus[D_a2e7].items[D_0204];
            if (it->sub != 0 && D_940f == 0 && it->enabled == 1) {
                menu_submenu_open(it->sub, D_987b + 1, (D_0204 << 3) + 0xc);
                D_a340 = D_0204;
            }
            mouse_show();
            return;
        }
        if (D_0206 != -0x63) {
            gfx_recolor_rect(card_window, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9, 0);
            D_0206 = -0x63;
        }
        return;
    }
    if (D_940f == 1 && (x = mx - 0x3c) >= D_9c29 && x <= D_9c3a && D_9c27 <= my &&
        D_9c44 >= my) {
        D_9783 = 1;
        if ((row = (my - 0xc) / 8) - D_a340 != D_0206) {
            mouse_hide();
            if (D_0206 >= 0 && D_0206 < D_9415)
                gfx_recolor_rect(card_window, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9,
                                   0);
            if ((D_0206 = row - D_a340) >= 0 && D_0206 < D_9415)
                gfx_recolor_rect(card_window, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 0,
                                   9);
            mouse_show();
        }
        return;
    }
    if (D_944b == 1) {
        mouse_hide();
        gfx_recolor_rect(card_window, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
        mouse_show();
        D_944b = 0;
        D_0204 = -1;
        return;
    }
    if (D_9783 == 1) {
        mouse_hide();
        gfx_recolor_rect(card_window, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9, 0);
        mouse_show();
        D_9783 = 0;
        D_0206 = -0x63;
    }
}

/* draw the menu bar: save the bar area to the off-screen page, clear it and print the
   titles left to right (grey when the entry has no items), recording each title's x range */
#pragma optimize("a", on)
void far menu_bar_draw(struct menu *m)
{
    int x;
    int w;

    if (D_9409 == 0) {
        x = 3;
        mouse_hide();
        gfx_copy_rect(card_window, 0, 0, 0x104, 0xc, card_window2, 0, 0);
        copy_rect_saved(card_window, 0, 0, 0x104, 0xc, card_window2, 0, 0);
        draw_frame_box(card_window, 0, 0, 0x104, 0xc, 0x80);
        D_9411 = 0;
        for (; strlen(m->name) != 0; m++) {
            w = text_pixel_width(card_window, m->name);
            m->x0 = x;
            if (m->has_items == 1)
                card_window->f0c = 0;
            else
                card_window->f0c = 7;
            draw_text(card_window, x + 4, 3, m->name);
            x += w + 8;
            m->x1 = x - 1;
            D_9411++;
        }
        mouse_show();
        D_940d = 0;
        D_940f = 0;
        D_9409 = 1;
        D_a2e7 = -1;
    }
}
#pragma optimize("", on)

/* open the dropdown for items at x: size it to the widest name and shortcut column, save
   the area under it, draw the frame and the items (grey when disabled) and highlight the
   item under y */
void far menu_dropdown_open(struct item *items, int x, int y)
{
    int j;
    int wname;
    int wkey;
    int w;
    struct item *it;
    int yy;

    D_9413 = 0;
    wname = 0;
    wkey = 0;
    j = 0;
    for (it = items; strlen(it->name) != 0; it++) {
        if ((w = text_pixel_width(card_window, it->name)) > wname)
            wname = w;
        if ((w = text_pixel_width(card_window, it->key)) > wkey)
            wkey = w;
        D_9413++;
    }
    D_9789 = x;
    D_987b = x + wkey + wname + 9;
    D_9a45 = D_9789 + wname + 6;
    D_99e7 = D_987b - D_9789 + 1;
    D_99e9 = (D_9413 << 3) + 0xe;
    copy_rect_saved(card_window, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, card_window2, D_9789, 0xc);
    draw_frame_box(card_window, D_9789, 0xb, D_99e7, (D_9413 << 3) + 4, 0x80);
    for (j = 0, it = items, yy = 0xe; j < D_9413; j++, it++, yy += 8) {
        if (it->enabled == 1)
            card_window->f0c = 0;
        else
            card_window->f0c = 7;
        draw_text(card_window, x + 3, yy, it->name);
        draw_text(card_window, D_9a45, yy, it->key);
    }
    if ((D_0204 = (y - 0xe) / 8) >= 0 && D_0204 < D_9413)
        gfx_recolor_rect(card_window, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 0, 9);
    D_940d = 1;
}

/* open the sub-menu for subs at (x, y): like 07d4 with a third, centred column for item
   counts of 2 and more */
void far menu_submenu_open(struct subitem *subs, int x, int y)
{
    char num[16];
    int wkey;
    int wnum;
    int wname;
    int j;
    unsigned char *pc;
    int w;
    struct subitem *s;
    int yy;

    D_9415 = 0;
    wnum = 0;
    wname = 0;
    wkey = 0;
    j = 0;
    for (s = subs; strlen(s->name) != 0; s++) {
        if (s->count >= 2) {
            itoa_commas(s->count, num);
            if ((w = text_pixel_width(card_window, num)) > wnum)
                wnum = w;
        }
        if ((w = text_pixel_width(card_window, s->name)) > wname)
            wname = w;
        if ((w = text_pixel_width(card_window, s->key)) > wkey)
            wkey = w;
        D_9415++;
    }
    D_9c51 = wnum ? wnum + 3 : 0;
    D_9c29 = x - 1;
    D_9c3a = D_9c29 + wkey + D_9c51 + wname + 9;
    D_9c53 = D_9c29 + D_9c51 + 3;
    D_9c48 = D_9c29 + D_9c51 + wname + 6;
    D_9c3f = D_9c3a - D_9c29 + 1;
    D_9c27 = y - 1;
    D_9c44 = D_9c27 + (D_9415 << 3) - 1;
    copy_rect_saved(card_window, D_9c29, D_9c27, D_9c3f, (D_9415 << 3) + 4, card_window2, D_9c29 + 1,
                    D_9c27 + 1);
    draw_frame_box(card_window, D_9c29, D_9c27, D_9c3f, (D_9415 << 3) + 4, 0x80);
    for (j = 0, s = subs, yy = 0; j < D_9415; j++, s++, yy += 8) {
        card_window->f0c = s->enabled == 1 ? 0 : 7;
        if (s->count > 1) {
            itoa_commas(s->count, num);
            w = text_pixel_width(card_window, num);
            draw_text(card_window, (D_9c51 - w) / 2 + D_9c29 + 3, D_9c27 + yy + 3, num);
        }
        draw_text(card_window, D_9c53, D_9c27 + yy + 3, s->name);
        draw_text(card_window, D_9c48, D_9c27 + yy + 3, s->key);
    }
    D_0206 = -0x63;
    D_940f = 1;
}

/* close the open dropdown: restore the item highlight and the menu bar from the off-screen
   copy card_window2 */
void far menu_dropdown_close(void)
{
    mouse_hide();
    menu_submenu_close();
    if (D_940d == 1) {
        copy_rect_saved(card_window2, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, card_window, D_9789, 0xc);
        D_0204 = -1;
    }
    copy_rect_saved(card_window2, 0, 0, 0x104, 0xc, card_window, 0, 0);
    D_a2e7 = -1;
    mouse_show();
}

/* close the open sub-menu */
void far menu_submenu_close(void)
{
    if (D_940f == 1) {
        copy_rect_saved(card_window2, D_9c29 + 1, D_9c27 + 1, D_9c3f, (D_9415 << 3) + 4, card_window,
                        D_9c29, D_9c27);
        D_0206 = -0x63;
        D_940f = 0;
    }
}

/* random number 0 .. n-1 (n at least 2) */
int far rand_mod(int n)
{
    int v_si;
    v_si = n;
    if (v_si <= 1) {
        v_si = 2;
    }
    return rand() % v_si;
}

/* random number 1 .. n (n at least 2) */
int far rand_1_n(int n)
{
    int v_si;
    v_si = n;
    if (v_si <= 1) {
        v_si = 2;
    }
    return rand() % v_si + 1;
}

/* hot keys (BIOS scan codes): Alt-Q quit, Alt-O/Alt-F/Alt-L/Alt-J/Alt-? optional handlers
   (choose_party_order_vec..185b, enabled by D_9893/D_9920/D_9934/D_987f), Alt-P pause, Alt-D cycle
   difficulty, Alt-F sound effects, Alt-C saintly changes, Alt-M music, F10 menu mode */
void far hot_keys(void)
{
    switch (key_pressed()) {
    case 0x1000:
        get_key();
        quit_to_dos();
        return;
    case 0x1800:
        get_key();
        if (D_9893 != 1) {
            return;
        }
        choose_party_order_vec();
        return;
    case 0x1900:
        get_key();
        sub_root_0000_1080();
        return;
    case 0x1f00:
        get_key();
        if (D_9920 != 1) {
            return;
        }
        save_game_vec();
        return;
    case 0x2000:
        get_key();
        if (!game_difficulty) {
            set_difficulty_standard();
            return;
        }
        if (game_difficulty == 1) {
            set_difficulty_expert();
            return;
        }
        if (game_difficulty != 2) {
            return;
        }
        set_difficulty_basic();
        return;
    case 0x2100:
        get_key();
        toggle_sound();
        return;
    case 0x2600:
        get_key();
        if (D_9934 != 1) {
            return;
        }
        load_game_vec();
        return;
    case 0x2e00:
        get_key();
        toggle_saint_msgs();
        return;
    case 0x3200:
        get_key();
        toggle_music();
        return;
    case 0x4000:
        get_key();
        if (D_987f != 1) {
            return;
        }
        party_info_screen_vec();
        return;
    case 0x4400:
        get_key();
        D_9c26 = 1;
        D_9c2f = 0;
        D_9c3c = -1;
        D_9c41 = -1;
    default:
        return;
    }
}

/* keyboard menu navigation (menu mode D_9c26): Esc/F10 or a mouse click leave it, Enter
   selects, Alt-Q quits, the cursor keys move between bar entries (D_9c2f, skipping entries
   whose has_items is 2), items (D_9c3c, -1 = none, only enabled ones) and sub-items
   (D_9c41, -1 = none) of the bar D_9419 */
void far sub_root_0000_0dd2(void)
{
    int b;
    struct item *items;
    int n;
    int j;
    struct subitem *subs;

    if (mouse_buttons & 2) {
        if (key_pressed())
            get_key();
        D_9c26 = 0;
        D_944b = 0;
        D_9783 = 0;
        return;
    }
    switch (get_key()) {
    case 0x011b:
    case 0x4400:
        D_9c26 = 0;
        D_944b = 0;
        D_9783 = 0;
        break;
    case 0x1000:
        quit_to_dos();
        break;
    case 0x1c0d:
        D_9c26 = 0;
        break;
    case 0x4800:
        if (D_9c41 != -1) {
            if (D_9c41 > 0)
                D_9c41--;
        } else if (D_9c3c != -1) {
            while (D_9c3c != -1) {
                D_9c3c--;
                if (D_9419[D_9c2f].items[D_9c3c].enabled == 1)
                    break;
            }
        }
        break;
    case 0x4b00:
        if (D_9c41 != -1) {
            D_9c41 = -1;
            return;
        }
        if (D_9c2f > 0) {
            D_9c3c = -1;
            D_9c41 = -1;
            while (D_9c2f != 0) {
                D_9c2f--;
                if (D_9419[D_9c2f].has_items == 1)
                    return;
            }
        }
        break;
    case 0x4d00:
        b = D_9c2f;
        if (D_9419[b].items[D_9c3c].sub != 0) {
            D_9c41 = 0;
            return;
        }
        if (D_9c2f < 3) {
            for (j = b; j < 3; j++)
                if (D_9419[j + 1].has_items != 2)
                    break;
            if (j < 3 && D_9419[j + 1].has_items == 1) {
                D_9c2f = j + 1;
                D_9c3c = -1;
                D_9c41 = -1;
                return;
            }
        }
        break;
    case 0x5000:
        if (D_9c41 != -1) {
            subs = D_9419[D_9c2f].items[D_9c3c].sub;
            if (strlen(subs[D_9c41 + 1].name) != 0) {
                D_9c41++;
                return;
            }
            break;
        }
        n = D_9c3c;
        items = D_9419[D_9c2f].items;
        if (strlen(items[n + 1].name) != 0) {
            while (strlen(items[n + 1].name) != 0 && items[n + 1].enabled == 2)
                n++;
            if (strlen(items[n + 1].name) != 0 && items[n + 1].enabled == 1)
                D_9c3c = n + 1;
        }
        break;
    }
}

/* "Game Paused / Press a key to resume" until a key is pressed */
#pragma optimize("a", on)
void far sub_root_0000_1080(void)
{
    struct rect r2;
    struct rect r1;
    int first;

    first = 0;
    if (!D_07d3) {
        D_07d3 = 1;
        first = 1;
    }
    card_window->f0c = 1;
    card_window->f0e = 0xff;
    sound_play(2);
    mouse_hide();
    text_box(card_window, 0x32, 1, "Game Paused", card_window2, &r1);
    text_box(card_window, -1, 1, "Press a key to resume", card_window2, &r2);
    mouse_show();
    getch();
    mouse_hide();
    copy_rect_saved(card_window2, r1.x, r1.y, r1.w, r1.h, card_window, r1.x, r1.y);
    copy_rect_saved(card_window2, r2.x, r2.y, r2.w, r2.h, card_window, r2.x, r2.y);
    mouse_show();
    sound_play(3);
    if (--first == 0)
        D_07d3 = 0;
}
#pragma optimize("", on)

/* toggle "Show saintly changes" (opt_saint_msgs), tick its menu entry, report "... ON/OFF" */
void far toggle_saint_msgs(void)
{
    char buf[80];

    strcpy(buf, D_01ba);
    if ((opt_saint_msgs ^= 1) == 1) {
        strcat(buf, "ON");
        *D_995a = 1;
    } else {
        strcat(buf, "OFF");
        *D_995a = 2;
    }
    show_message(buf, 0x3c);
}

/* toggle music (opt_music; off also stops it through 047c_078a(1)) */
void far toggle_music(void)
{
    char buf[80];

    strcpy(buf, D_01b8);
    if ((opt_music ^= 1) == 1) {
        strcat(buf, "ON");
        *D_996e = 1;
    } else {
        strcat(buf, "OFF");
        *D_996e = 2;
        sound_play(1);
    }
    show_message(buf, 0x3c);
}

/* toggle sound effects (opt_sound) */
void far toggle_sound(void)
{
    char buf[80];

    strcpy(buf, D_01b6);
    if ((opt_sound ^= 1) == 1) {
        strcat(buf, "ON");
        *D_9982 = 1;
    } else {
        strcat(buf, "OFF");
        *D_9982 = 2;
    }
    show_message(buf, 0x3c);
}

/* exit to DOS when exit_requested is set */
void far exit_if_requested(void)
{
    if (exit_requested) {
        game_shutdown();
        sub_root_044a_0122();
        exit_(0);
    }
}

/* shut down the sound/timer and return to text mode 3 */
void far game_shutdown(void)
{
    union { struct { char al; char ah; } h; int x[7]; } r;

    sub_root_044a_0122();
    if (D_9c4c == 1) {
        sub_root_047c_123f();
    }
    sub_root_047c_00a9();
    r.h.ah = 0;
    r.h.al = 3;
    int86(0x10, &r, &r);
}

/* flush the key buffer */
void far flush_keys(void)
{
    if (key_pressed()) {
        do {
            get_key();
        } while (key_pressed());
    }
}

/* "Quit to DOS? y/n": on yes shut everything down, delete the temporary save "dk.tmp"
   and exit */
void far quit_to_dos(void)
{
    char *z[2];

    if (ask_yes_no(D_01b2) == 1) {
        sub_root_044a_01f4();
        sub_root_047c_0791();
        vec_186f();
        game_shutdown();
        if (D_907d == 1) {
            sub_root_05f8_0139(ems_frame_handle);
        }
        video_restore_vec();
        *(char * volatile *)&z[0] = 0;
        file_load_far(D_01a6, (char far *)z[0]);
        remove(D_01a6);
        exit_(0);
    }
}

/* difficulty setters (Alt-D cycles 1 -> 2 -> 0 -> 1) */
void far set_difficulty_basic(void)
{
    game_difficulty = 0;
    show_difficulty(1);
}

void far set_difficulty_standard(void)
{
    game_difficulty = 1;
    show_difficulty(1);
}

void far set_difficulty_expert(void)
{
    game_difficulty = 2;
    show_difficulty(1);
}

/* (6.00 keeps the 2 in AL for the second and third store - known 6.00A limit)
   tick the difficulty menu entry (D_99ec Basic, D_9a02 Standard, D_9a18 Expert) and, when
   show == 1, report "Difficulty is now set to ..." */
#pragma optimize("a", on)
void far show_difficulty(int show)
{
    char buf[80];

    strcpy(buf, D_01b4);
    switch (game_difficulty & 0x7f) {
    case 0:
        *D_99ec = 1;
        *D_9a02 = 2;
        *D_9a18 = 2;
        strcat(buf, "Basic");
        break;
    case 1:
        *D_9a02 = 1;
        *D_99ec = 2;
        *D_9a18 = 2;
        strcat(buf, "Standard");
        break;
    case 2:
        *D_99ec = 2;
        *D_9a02 = 2;
        *D_9a18 = 1;
        strcat(buf, "Expert");
        break;
    }
    if (show == 1)
        show_message(buf, 0x3c);
}

/* m6: 14f4 is a separate empty function (a lone retf after 1438); 1438 needs pragma "a" (keeps 2 in AL). */
void far sub_root_0000_14f4(void)
{
}
#pragma optimize("a", off)

/* unused extern: symbol-table knob for the operand order in 14f6 */
extern int D_ffe0;

/* clip the rectangle (x, y, w, h) of src against dst (dst->x1 +6, dst->y1 +8; negative
   source coordinates move the target origin to 0) and copy it (047c_0553); 0 when nothing
   is left */
int far copy_rect_clipped(struct win *src, int x, int y, int w, int h, struct win *dst,
                           int dx, int dy)
{
    if (x < 0) {
        w += x;
        x = 0;
        dx = 0;
        if (w <= 0)
            return 0;
    }
    if (x + w > dst->x1) {
        w = dst->x1 - x + 1;
        if (w <= 0)
            return 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
        dy = 0;
        if (h <= 0)
            return 0;
    }
    if (y + h > dst->y1) {
        h = dst->y1 - y + 1;
        if (h <= 0)
            return 0;
    }
    gfx_copy_rect(src, x, y, w, h, dst, dx, dy);
    return 1;
}

/* redraw the five party-member slot frames (rows of 0x28 pixels on the pages D_0168 and
   D_0190) whose bits are set in D_01e4[which] (0x100: all five); cbt_selected[i] picks the
   normal (0) or highlighted (1) frame pieces */
void far party_panel_frames(int which)
{
    unsigned bits;
    int y;
    int i;

    if (which == 0x100)
        bits = 0x1f;
    else
        bits = D_01e4[which];
    i = 0;
    y = 0;
    do {
        mouse_hide();
        if ((bits & 1) == 1) {
            if (cbt_selected[i] == 0) {
                gfx_draw_pic(D_0168, 0, y, D_a2e9);
                gfx_draw_pic(D_0168, 0x39, y, D_a2e9);
                gfx_draw_pic(D_0168, 3, y, D_a3d6);
                gfx_draw_pic(D_0168, 3, y + 0x27, D_a3e5);
                gfx_draw_pic(D_0190, 0, y, D_a2e9);
                gfx_draw_pic(D_0190, 0x39, y, D_a2e9);
                gfx_draw_pic(D_0190, 3, y, D_a3d6);
                gfx_draw_pic(D_0190, 3, y + 0x27, D_a3e5);
            } else if (cbt_selected[i] == 1) {
                gfx_draw_pic(D_0168, 0, y, D_a33b);
                gfx_draw_pic(D_0168, 0x39, y, D_a33b);
                gfx_draw_pic(D_0168, 3, y, D_a33e);
                gfx_draw_pic(D_0168, 3, y + 0x27, D_a392);
                gfx_draw_pic(D_0190, 0, y, D_a33b);
                gfx_draw_pic(D_0190, 0x39, y, D_a33b);
                gfx_draw_pic(D_0190, 3, y, D_a33e);
                gfx_draw_pic(D_0190, 3, y + 0x27, D_a392);
            }
        }
        mouse_show();
        bits >>= 1;
        i++;
    } while ((y += 0x28) < 200);
}

/* redraw the party panel rows selected by D_01e4[which] (0x100: all five): portrait frame,
   name (grey when the member is out of action or someone else is acting, yellow for the
   current leader leader), the status letter (1a9c/18fa) and a cross over a dead member's
   portrait; then copy the row to the screen page D_0190 */
#pragma optimize("a", on)
void far party_panel_rows(int which)
{
    struct { char c; char pad; char name[40]; int save; int *pic; int i; char *m; int y; int bits; } s;
    int k;

    if (which == 0x100)
        s.bits = 0x1f;
    else
        s.bits = D_01e4[which];
    s.save = D_0168->f0c;
    for (s.i = 0, s.y = 1, s.m = party[0], s.pic = shield_pics; s.pic < shield_pics + 5;
         s.bits >>= 1, s.y += 0x28, s.m += 0x80, s.i++, s.pic++) {
        if ((s.bits & 1) == 1) {
            gfx_draw_pic(D_0168, 3, s.y, D_a626);
            if ((s.c = s.m[0]) != 0) {
                if (s.c == 3)
                    gfx_draw_pic(D_0168, 6, s.y + 8, D_a2e5);
                else
                    gfx_draw_pic(D_0168, 6, s.y + 8, *s.pic);
                strcpy(s.name, s.m + 0x2a);
                if (order_mode == 1 && selected_member != s.i || cbt_floor[s.i] == (char)0x9d ||
                    (s.c = s.m[0]) == 3 || s.c == 5)
                    D_0168->f0c = 7;
                else if (leader == s.i)
                    D_0168->f0c = 0x0e;
                else
                    D_0168->f0c = 0x0f;
                draw_text(D_0168, 4, s.y + 1, s.name);
                member_bars(s.i);
                member_status_letter(s.i, D_0168);
            }
            if (s.m[0] == 3) {
                D_0168->w = 3;
                D_0168->f04 = s.y;
                D_0168->x1 = 0x35;
                D_0168->y1 = 0x26;
                for (k = 5; k < 0x69; k += 10)
                    gfx_line(D_0168, k, 0, 0, k, 7);
                D_0168->w = 0;
                D_0168->f04 = 0;
                D_0168->x1 = 0x13f;
                D_0168->y1 = 0xc7;
            }
            if (mouse_x < 0x2d)
                mouse_hide();
            copy_rect_clipped(D_0168, 3, s.y, 0x36, 0x26, D_0190, 3, s.y);
            mouse_show();
        }
    }
    D_0168->f0c = s.save;
}
#pragma optimize("", on)

/* status letter of member i in its panel row: '-' idle, 'W'/'F' (walk/fight), 'B', 'V', 'A',
   'P', 'R', 'M', 'T' for action modes, 'U', 'P', 'D', 'O', 'X' for combat orders (mode
   0x400), 'b'/'e' for the special states in D_a3d8 */
void far member_status_letter(int i, struct win *w)
{
    char s[2];
    int st;
    int t;

    if (D_907a == 1 && party[i][0] != 5) {
        if (D_a3d8[i] == 0) {
            if (ai_paused == 1 && order_mode == 1 && selected_member == i && (t = order_action) != -1)
                st = t;
            else
                st = D_9c65[i][0];
            if (st != 0x400) {
                switch (st) {
                case 0:
                    s[0] = '-';
                    break;
                case 1:
                    s[0] = (cbt_flags[i] & 0x1000) ? 'F' : 'W';
                    break;
                case 6:
                    s[0] = 'B';
                    break;
                case 0x0a:
                    s[0] = 'V';
                    break;
                case 0x12:
                    s[0] = 'A';
                    break;
                case 0x22: s[0] = 'P'; break;
                case 0x40:
                    s[0] = 'R';
                    break;
                case 0x80:
                    s[0] = 'M';
                    break;
                case 0x100:
                    s[0] = 'T';
                    break;
                }
            } else {
                if (ai_paused == 1 && order_mode == 1 && selected_member == i && (t = D_a6d1) != -1)
                    st = t;
                else
                    st = cbt_special[i];
                switch (st) {
                case 1:
                case 2:
                    s[0] = 'U';
                    break;
                case 3:
                case 6:
                    s[0] = 'P';
                    break;
                case 4:
                case 7:
                    s[0] = 'D';
                    break;
                case 5:
                    s[0] = 'O';
                    break;
                case 8:
                    s[0] = 'X';
                    break;
                }
            }
        } else {
            switch (D_a3d8[i]) {
            case 1:
                s[0] = 'b';
                break;
            case 2:
                s[0] = 'e';
                break;
            }
        }
        s[1] = 0;
        gfx_draw_pic(w, 6, i * 0x28 + 0x1f, D_a6c2);
        draw_text(w, 6, i * 0x28 + 0x1f, s);
    }
}

/* status column of party row i: for an unconscious member (state 5) "???? ???? ????";
   otherwise either a status icon text (D_01a8..D_01b0 by the flag bits of cbt_flags[i]) or
   three vertical bars for endurance, strength and divine favour (current * 16 / maximum,
   "+" above a bar when the current value exceeds the maximum) with the current values
   printed beside them; while the member carries someone (D_9c65 mode bits 0x1be, slot
   +0x36) the carried member's bars are drawn as well */
void far member_bars(int i)
{
    long ext;
    int top;
    char *msg;
    int dy;
    int bot;
    char buf[40];
    int j;
    int bars;
    char c;
    int q;
    int len;

    bars = 0;
    if (party[i][0] == 5) {
        strcpy(buf, "???? ???? ????");
        ext = measure_text_wrapped(D_0168, buf, 0x1c, 0x16, i * 0x28 + 9,
                                 font_height(D_0168->f10), 0, 0, 0, 0, 0);
        if ((dy = (i * 0x28 + 0x25 - (int)(ext >> 16)) / 2) < -1)
            dy = -1;
        D_01bc = 1;
        D_0168->f0c = 7;
        draw_text_wrapped(D_0168, buf, 0x1c, 0x16, i * 0x28 + dy + 9,
                           font_height(D_0168->f10), 0, 0, 0, 0, 0, 0);
        D_01bc = 0;
        return;
    }
    top = i * 0x28 + 0xc;
    bot = i * 0x28 + 0x1b;
    c = member_attrs[i][0];
    q = (c << 4) / D_9cb9[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if ((cbt_flags[i] & 8) == 8 && real_combat == 0) {
        if ((cbt_flags[i] & 2) == 2)
            msg = D_01a8;
        else if ((cbt_flags[i] & 1) == 1)
            msg = D_01aa;
        else if ((cbt_flags[i] & 4) == 4)
            msg = D_01ac;
        else if ((cbt_flags[i] & 0x20) == 0x20)
            msg = D_01ae;
        else if ((cbt_flags[i] & 0x40) == 0x40)
            msg = D_01b0;
        else
            goto plain;
        ext = measure_text_wrapped(D_0168, msg, 0x24, 0x13, i * 0x28 + 9,
                                 font_height(D_0168->f10), 0, 0, 0, 0, 0);
        if ((dy = (i * 0x28 + 0x1f - (int)(ext >> 16)) / 2) < -1)
            dy = -1;
        D_01bc = 1;
        draw_text_wrapped(D_0168, msg, 0x24, 0x13, i * 0x28 + dy + 9,
                           font_height(D_0168->f10), 0, 0, 0, 0, 0, 0);
        D_01bc = 0;
    } else {
plain:
        bars = 1;
        if (len < 0)
            len = 0;
        if (member_attrs[i][0] > D_9cb9[i][0])
            draw_text(D_0168, 0x14, top - 4, "+");
        if (len != 0) {
            gfx_line(D_0168, 0x15, top, 0x15, top + len - 1, 0x0f);
            gfx_line(D_0168, 0x16, top, 0x16, top + len - 1, 0x0f);
        }
        if (len != 16) {
            gfx_line(D_0168, 0x15, top + len, 0x15, bot, 9);
            gfx_line(D_0168, 0x16, top + len, 0x16, bot, 9);
        }
    }
    itoa_commas(member_attrs[i][0], buf);
    strcat(buf, " ");
    D_0168->x1 = 0x38;
    draw_text(D_0168, 0x12, i * 0x28 + 0x1f, buf);

    c = D_9cb3[i][0];
    q = (c << 4) / D_9cba[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if (bars == 1) {
        if (len < 0)
            len = 0;
        if (D_9cb3[i][0] > D_9cba[i][0])
            draw_text(D_0168, 0x22, top - 4, "+");
        if (len != 0) {
            gfx_line(D_0168, 0x23, top, 0x23, top + len - 1, 0x0f);
            gfx_line(D_0168, 0x24, top, 0x24, top + len - 1, 0x0f);
        }
        if (len != 16) {
            gfx_line(D_0168, 0x23, top + len, 0x23, bot, 2);
            gfx_line(D_0168, 0x24, top + len, 0x24, bot, 2);
        }
    }
    itoa_commas(D_9cb3[i][0], buf);
    strcat(buf, " ");
    draw_text(D_0168, 0x20, i * 0x28 + 0x1f, buf);

    c = D_9cb8[i][0];
    q = (c << 4) / D_9cbf[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if (bars == 1) {
        if (len < 0)
            len = 0;
        if (D_9cbf[i][0] < D_9cb8[i][0])
            draw_text(D_0168, 0x2f, top - 4, "+");
        if (len != 0) {
            gfx_line(D_0168, 0x30, top, 0x30, top + len - 1, 0x0f);
            gfx_line(D_0168, 0x31, top, 0x31, top + len - 1, 0x0f);
        }
        if (len != 16) {
            gfx_line(D_0168, 0x30, top + len, 0x30, bot, 0x0e);
            gfx_line(D_0168, 0x31, top + len, 0x31, bot, 0x0e);
        }
    }
    itoa_commas(D_9cb8[i][0], buf);
    strcat(buf, " ");
    draw_text(D_0168, 0x2d, i * 0x28 + 0x1f, buf);

    if ((D_9c65[i][0] & 0x1be) && cbt_moving[i] == 0 && (j = D_9c9f[i][0]) != -1) {
        c = member_attrs[j][0];
        q = (c << 4) / D_9cb9[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (member_attrs[j][0] > D_9cb9[j][0])
                draw_text(D_0168, 0x18, top - 4, "+");
            if (len != 0)
                gfx_line(D_0168, 0x19, top, 0x19, top + len - 1, 0x0f);
            if (len != 16)
                gfx_line(D_0168, 0x19, top + len, 0x19, bot, 9);
        }
        c = D_9cb3[j][0];
        q = (c << 4) / D_9cba[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (D_9cb3[j][0] > D_9cba[j][0])
                draw_text(D_0168, 0x26, top - 4, "+");
            if (len != 0)
                gfx_line(D_0168, 0x27, top, 0x27, top + len - 1, 0x0f);
            if (len != 16)
                gfx_line(D_0168, 0x27, top + len, 0x27, bot, 2);
        }
        c = D_9cb8[j][0];
        q = (c << 4) / D_9cbf[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (D_9cbf[j][0] < D_9cb8[j][0])
                draw_text(D_0168, 0x33, top - 4, "+");
            if (len != 0)
                gfx_line(D_0168, 0x34, top, 0x34, top + len - 1, 0x0f);
            if (len != 16)
                gfx_line(D_0168, 0x34, top + len, 0x34, bot, 0x0e);
        }
    }
    D_0168->x1 = 0x13f;
}

/* framed box at x,y with a cw x ch interior: dark (0x86) outer and light (0x82) inner
   bevel lines, corner pieces D_a660..D_a666 */
void far draw_frame_box(struct win *w, int x, int y, int cw, int ch, int bg)
{
    int x1;
    int y1;
    int x2;
    int y2;

    gfx_fill_rect(w, x, y, cw, ch, bg);
    x1 = x + cw - 1;
    y1 = y + ch - 1;
    gfx_line(w, x, y, x1, y, 0x86);
    gfx_line(w, x1, y, x1, y1, 0x86);
    gfx_line(w, x1, y1, x, y1, 0x86);
    gfx_line(w, x, y1, x, y, 0x86);
    x2 = x1 - 1;
    y2 = y1 - 1;
    gfx_line(w, x + 1, y + 1, x2, y + 1, 0x82);
    gfx_line(w, x2, y + 1, x2, y2, 0x82);
    gfx_line(w, x2, y2, x + 1, y2, 0x82);
    gfx_line(w, x + 1, y2, x + 1, y + 1, 0x82);
    gfx_draw_pic(w, x, y, D_a660);
    gfx_draw_pic(w, x, y1 - 3, D_a662);
    gfx_draw_pic(w, x1 - 3, y, D_a664);
    gfx_draw_pic(w, x1 - 3, y1 - 3, D_a666);
}

/* text box: centre text horizontally (320 wide) at text row y (-1: vertically centred),
   frame it (2286) and print it; with save != 0 first save the background into dst and
   return the saved rectangle in *r */
void far text_box(struct win *w, int y, int save, char *text, struct win *dst,
                            struct rect *r)
{
    struct { int tx; int bx; int bh; int bw; int by; int ty; } s;
    int tw;

    if (y != -1)
        s.ty = y;
    else
        s.ty = -(font_height(w->f10) / 2 - 100);
    tw = text_pixel_width(w, text);
    s.bw = tw + 10;
    s.bh = font_height(w->f10) + 7;
    s.tx = (0x140 - tw - w->w) / 2;
    s.bx = s.tx - 5;
    s.by = s.ty - 4;
    if (save) {
        r->y = s.by;
        r->w = s.bw;
        r->h = s.bh;
        r->x = s.bx;
        copy_rect_saved(w, s.bx, s.by, s.bw, s.bh, dst, s.bx, s.by);
    }
    draw_frame_box(w, s.bx, s.by, s.bw, s.bh, w->f0e);
    draw_text(w, s.tx, s.ty, text);
}

/* message box for `ticks` timer ticks (0410_0302), then restore the screen */
#pragma optimize("a", on)
void far show_message(char *text, int ticks)
{
    struct rect r;

    card_window->f0c = 1;
    card_window->f0e = 0xff;
    mouse_hide();
    text_box(card_window, -1, 1, text, card_window2, &r);
    sub_root_0410_0302(ticks);
    copy_rect_saved(card_window2, r.x, r.y, r.w, r.h, card_window, r.x, r.y);
    mouse_show();
}
#pragma optimize("", on)

/* "<question> y/n" box: waits for Y or N (either case), returns 1 for yes */
#pragma optimize("a", on)
int far ask_yes_no(char *question)
{
    char buf[120];
    struct rect r;
    char c;
    int yes;

    card_window->f0c = 1;
    card_window->f0e = 0xff;
    strcpy(buf, question);
    strcat(buf, " y/n");
    mouse_hide();
    text_box(card_window, -1, 1, buf, card_window2, &r);
    for (;;) {
        c = getch();
        if (c >= 0x60)
            c -= 0x20;
        if (c == 'Y') {
            yes = 1;
            break;
        }
        if (c == 'N') {
            yes = 0;
            break;
        }
    }
    copy_rect_saved(card_window2, r.x, r.y, r.w, r.h, card_window, r.x, r.y);
    mouse_show();
    return yes;
}
#pragma optimize("", on)

void far sub_root_0000_25f0(void)
{
}

/* select data file `name` (kept in data_file) unless it is already the current one */
void far datafile_select(char *name)
{
    if (_fstricmp(name, data_file)) {
        catalog_close();
        catalog_open(name);
        _fstrcpy(data_file, name);
    }
}

/* forget the current data file (data_file = "") */
void far datafile_forget(void)
{
    catalog_close();
    _fstrcpy(data_file, "");
}

void far sub_root_0000_2646(void)
{
}

/* calls the empty function at 0000:2652 (below) */
void far sub_root_0000_2648(void)
{
    sub_root_0265_0002(0);
}

/* 0000:2652: empty function called by 2648 */
void far sub_root_0265_0002(int a)
{
}
