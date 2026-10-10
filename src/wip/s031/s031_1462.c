/* place: s031 1462:0000 */
/* orig: size=4850 CONST=7e4e+0014 _DATA=2a56+04c1 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* Inventory screen (overlay s031, main file): the "arms" screen where a party member's
   equipment and pack are shown and items are moved, used, dropped or traded.
   STATUS (m0, 2026-10-09): all 19 functions written, _DATA and CONST match; 18/19 match.
     0124 [1 diff line]: `cmp si,di` for `m == who` (ours `cmp di,si`; who is region-cached in
       si there, m is the di register var).  Tried both operand orders, (m = ..) == who,
       who - m == 0, unsigned m/casts, renaming m and who, pragmas a/c/w/z, and a dummy
       extern at every top-level position: no flip.  (Frame fixed in e1d: [bp-8] is MSC's CSE
       of D_867c, not a named k; the ph+iy temp lands at [bp-0ch] once k is gone.)
     433c MATCHED (m0): the 0x80 is a mask variable `m = 0x80; ... (m >> b)` sharing b's slot
       (6.00A keeps that store); 43b2 follows.  0124 (m0): `D_9c69[who = m]` flips the compare
       to `cmp si,di` (but moves `mov si,di` too); local decl permutations, i/key for m,
       if/else forms, `D_867d = who = m`: no effect.
     42ac matched in e1d: no `f` local (hoisted CSE temp) and a do-while loop (the for loop gave
       `mov cx,ax` instead of `mov cx,bx` in the i*3 return blocks).
     43b2: code identical; only its jump table differs (433c length).
     Notes: the box table externs are declared a few bytes early (see there) to get the
     original operand order of `x + w`; D_867e must be unsigned char or `1d44(D_867e = 0, 12)`
     pushes an immediate.  Declarations anywhere in the file can flip operand orders in
     other functions (symbol table), so re-run fmatch on the whole file after any edit. */
#include "darklands/card.h"
#include "darklands/game.h"

struct gfx { char pad[12]; int color; char pad2[4]; };     /* 0x12 bytes, screen_win/back_win */

struct box {                        /* screen rectangles, 0x24 bytes */
    char lead[2];                   /* see the note at the box table externs */
    int x, y, w, h;                 /* +00 */
    unsigned char color;            /* +08  frame colour */
    unsigned char flags;            /* +09  1: active, 2: has a list, 0x10: holds items */
    int ix, iy;                     /* +0a  icon position */
    unsigned char pw, ph;           /* +0e */
    char pic;                       /* +10  index into D_8682[] */
    char pad11;
    int nvis;                       /* +12  visible lines */
    unsigned char top[5];           /* +14  first visible line, per member */
    unsigned char sel[5];           /* +19  selected line, per member */
    unsigned char up, down, left, right;    /* +1e  neighbour boxes for the cursor keys */
};                                  /* (field offsets above are from the real record start) */

/* party member record in DGROUP (0x80 bytes) */
struct member {
    char present;               /* +00 */
    char pad01[0x0d];
    char missile_type;          /* +0e  equipped missile weapon (item type, -1 none) */
    char pad0f;
    unsigned char x10;          /* +10 */
    char full_name[0x19];       /* +11 */
    char name[0x0a];            /* +2a */
    unsigned char x34;          /* +34  two nibbles */
    char pad35[2];              /* +35  +35: encumbrance */
    char vital_type;            /* +37  vital armour */
    char limb_type;             /* +38  limb armour */
    char pad39[2];
    char vital_q;               /* +3b */
    char limb_q;                /* +3c */
    char weapon_type;           /* +3d */
    char pad3e[3];
    unsigned char x41;          /* +41  two nibbles */
    char pad42[2];
    char weapon_q;              /* +44 */
    char pad45;
    char missile_q;             /* +46 */
    char shield_q;              /* +47 */
    char shield_type;           /* +48 */
    char attr[7];               /* +49  End Str Agl Per Int Chr DF: current */
    char attr_max[7];           /* +50  maximum */
    char skill[19];             /* +57  wEdg..wMsD, Alch..R&W, Heal..WdWs */
    char nitems;                /* +6a */
    char pad6b[0x15];
};

/* item definition (DARKLAND.LST) */
struct itemdef {
    char name[0x14];
    char short_name[0x0a];
    char type;                  /* +1e */
    char pad1f;
    long flags;                 /* +20 */
    char pad24;
    char weight;                /* +25 */
    char quality;               /* +26 */
};

/* one inventory entry (6 bytes) */
struct item {
    int code;                   /* LST item code (low 12 bits) */
    unsigned char type;         /* LST item type */
    unsigned char qual;         /* quality */
    unsigned char cnt;          /* quantity */
    unsigned char weight;
};

/* character record in far data (0x180 bytes); the items run on past the record */
struct fchar {
    char pad00[0x3e];
    struct item items[53];
    char pad17c[4];
};

#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned far *)&(fp)))

extern int party_size;                       /* party size */
extern struct member party[];
extern char cur_palette[];
extern struct gfx screen_win, back_win;
extern int cursor_default;
extern int D_e7a4;
extern int party_money;
extern int party_icons_dirty;
extern char D_907a, D_907c;
extern char cbt_floor[];
extern char D_9419[];
extern unsigned char D_00e8[];            /* bit masks 0x80 >> n */                       /* number of lines in the list */
extern volatile char mouse_buttons;             /* mouse buttons */
extern volatile int mouse_x, mouse_y;               /* mouse x, y */
extern unsigned D_867a, D_8680, D_8678;
extern unsigned D_8682[];                /* brush pictures */
extern unsigned char D_867c;
extern char leader;
extern unsigned char D_867d;
extern unsigned char D_867e;
extern char D_9920, D_9934;
extern int back_win_font, screen_win_font, D_e7a2, D_e7a0, screen_win_colour;
extern unsigned char far *lst_buf;        /* darkland.lst buffer */
extern unsigned char far *loc_buf;        /* location file buffer */
extern char far *city_rec;                 /* city record */

extern char far * far events[];     /* event records (0x30 bytes each), 300 slots */
extern char far * far item_defs[];     /* item definitions */
extern char far * far virtue_names[];     /* names, 3 per entry of formulas_known */
extern char far * far saint_names[];     /* names, 8 per byte of saints_known */
extern char far * far F_290e_1e67[];     /* attribute and skill labels */
extern struct fchar far data_file[];
extern struct box far F_290e_3557[];             /* = 290e:3559, the screen's boxes */
/* The box tables (far, 290e:3559 and 290e:3829) are declared a few bytes early, with a
   `lead` in struct box: with the base symbols at the real addresses MSC 6.00A loads `w` before
   `x` in `x + w` (the operand order of two far struct fields depends on the symbol and offset);
   declared this way it matches the original. */
struct box_l2 { char lead[4]; int x, y, w, h; char pad[0x18]; };   /* only the rectangle */
extern struct box_l2 far F_290e_3825[];          /* = 290e:3829, frame boxes (4 bytes early) */
extern unsigned char far formulas_known[][22];
extern unsigned char far saints_known[][20];

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
char *itoa(int, char *, int);
int sprintf(char *, const char *, ...);
unsigned _dos_write(int, void far *, unsigned, unsigned *);
unsigned _dos_close(int);
unsigned _dos_creat(char *, unsigned, int *);
void far free_buffer(unsigned seg, char *name);

void far F_0000_2652(int);
void far world_load_vec(char *);
void far vec_1905(char *, char *);
unsigned far gfx_save_rect(int, int, int, int, int);
void far mouse_show(void);
void far mouse_hide(void);
void far sub_root_047c_0568(void);
void far sub_root_047c_0545(void);
void far sub_root_047c_04f8(void);
void far sub_root_047c_056f(void);
void far set_palette(char *);
void far mouse_set_cursor(int, int, int);
void far gfx_draw_pic(struct gfx *, int, int, unsigned);
void far gfx_copy_rect(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far gfx_line(struct gfx *, int, int, int, int, int);
void far pic_draw_pal(int, char *, char *);
void far gfx_text(struct gfx *, int, int, char *);
void far draw_text_far(struct gfx *, int, int, char far *);
int far sub_s029_0e76_013a(int);
int far sub_s029_0e76_3996(int, int);
int far clamp_range(int, int, int);
int far equipped_item_impl(int who, int slot);
void far gfx_recolor_rect(struct gfx *, int, int, int, int, int, int);
void far give_one(int from, int to, int idx, int slot);
void far give_stack(int from, int to, int idx, int slot);
void far item_move(int who, int a, int b);
void far item_take_one(int who, int idx);
void far item_remove_stack(int who, int idx);
void far item_unequip(int who, int idx, int slot);
void far quit_to_dos(void);
void far sub_s031_18e7_11b2(int who, int idx);
void far alc_view_vec(int, int);
void far vec_1fef(int, int);
void far vec_1d51(int);
void far menu_bar_run(char *, int);
int far key_pressed(void);
int far get_key(void);
void far vec_200d(int, int, int);
unsigned far pic_load(char *);
void far party_panel_frames(int);
void far party_panel_rows(int);
int far rand_mod(int);
void far sub_s029_0e76_4d24(int, int, int, int);
void far lower_skill_temp(int, int, int, int);
void far raise_attr_temp(int, int, int, int);
void far heal_attr_pct(int, int, int);

int far member_sheet(int who);
int far sub_s031_1462_08f6(int r);
void far sub_s031_1462_0950(int k);
void far sub_s031_1462_09b8(int who);
void far sub_s031_1462_0a82(int who);
void far sub_s031_1462_0b22(int who);
void far sub_s031_1462_10ae(int key, int who);
void far sub_s031_1462_1e42(int k, int who, int y);
void far sub_s031_1462_1d44(unsigned char k, int color);
void far sub_s031_1462_2518(int k, int color);
void far sub_s031_1462_264c(int k, int who);
int far sub_s031_1462_42ac(int who, int n);
int far sub_s031_1462_433c(int who, int n);
void far sub_s031_1462_25de(int k, int who);
void far sub_s031_1462_43b2(int who, int idx);
void far sub_s031_1462_2b14(int k, int who);
void far sub_s031_1462_39a0(int i, int who, struct gfx g);
void far sub_s031_1462_39c6(int i, int who, struct gfx g);

/* leave the inventory screen: save locs.tmp / events.tmp and free the world data */
int far inventory_leave(int who)
{
    int fd, fd2;
    unsigned nw;
    int i;
    int nev;

    if (party_size <= who)
        return -1;
    F_0000_2652(0x60);
    world_load_vec("locs.tmp");
    mouse_show();
    member_sheet(who);
    _dos_creat("events.tmp", 0, &fd2);
    nev = 0;
    for (i = 0; i < 300; i++)
        if (events[i])
            nev++;
    _dos_write(fd2, (void far *)&nev, 2, &nw);
    for (i = 0; i < 300; i++) {
        if (events[i]) {
            _dos_write(fd2, events[i], 0x30, &nw);
            free_buffer(FP_SEG(events[i]), "e");
            events[i] = 0;
        }
    }
    _dos_close(fd2);
    free_buffer(FP_SEG(lst_buf), "S");
    free_buffer(FP_SEG(city_rec), "C");
    free_buffer(FP_SEG(loc_buf), "M");
    sub_root_047c_0568();
}

int far member_sheet(int who)
{
    int key, m, i;

    if (party[who].present == 0 || party[who].present == 5)
        return -1;
    D_9920 = 2;
    D_9934 = 2;
    D_907c = 2;
    screen_win_colour = 0xbf;
    D_867c = 0;
    D_867d = who;
    party_panel_frames(0x100);
    party_panel_rows(0x100);
    sub_s031_1462_09b8(who);
    sub_s031_1462_1d44(D_867e = 0, 12);
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if (mouse_buttons & 1) {
            if (mouse_x > 0x3c) {
                if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= mouse_x && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= mouse_x
                        && F_290e_3557[D_867c].iy + 9 <= mouse_y && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy - 0xe >= mouse_y) {
                    sub_s031_1462_1e42(D_867c, who, mouse_y);
                } else if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= mouse_x && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= mouse_x
                        && F_290e_3557[D_867c].iy <= mouse_y && F_290e_3557[D_867c].iy + 9 >= mouse_y) {
                    while (mouse_buttons & 1) {
                        D_867c;
                        if (F_290e_3557[D_867c].sel[who] == 0) {
                            if (F_290e_3557[D_867c].top[who] != 0)
                                F_290e_3557[D_867c].top[who]--;
                            sub_s031_1462_25de(D_867c, who);
                        } else {
                            mouse_hide();
                            gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                            F_290e_3557[D_867c].sel[who]--;
                            gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                            mouse_show();
                        }
                        vec_1fef(200, 0);
                    }
                } else if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= mouse_x && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= mouse_x
                        && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy - 0xe <= mouse_y && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy >= mouse_y) {
                    while (mouse_buttons & 1) {
                        D_867c;
                        if (F_290e_3557[D_867c].sel[who] >= F_290e_3557[D_867c].nvis - 1) {
                            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4)
                                break;
                            F_290e_3557[D_867c].top[who]++;
                            sub_s031_1462_25de(D_867c, who);
                        } else {
                            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4 - 1)
                                break;
                            mouse_hide();
                            gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                            F_290e_3557[D_867c].sel[who]++;
                            gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                            mouse_show();
                        }
                        vec_1fef(200, 0);
                    }
                } else {
                    for (i = 0; i < 18; i++)
                        if (F_290e_3557[i].x <= mouse_x && F_290e_3557[i].w + F_290e_3557[i].x >= mouse_x
                                && F_290e_3557[i].y <= mouse_y && F_290e_3557[i].y + F_290e_3557[i].h >= mouse_y)
                            break;
                    if (i == 18)
                        i = 0;
                    if (D_867e && D_867a) {
                        mouse_hide();
                        if (D_867a) {
                            gfx_draw_pic(&screen_win, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        mouse_show();
                    }
                    if (D_867e && (!D_867e || D_867c == i)) {
                        sub_s031_1462_1d44(D_867c, 12);
                        D_867e = 0;
                        mouse_hide();
                        mouse_set_cursor(0, 0, cursor_default);
                        mouse_show();
                    } else {
                        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
                        sub_s031_1462_264c(i, who);
                        D_867c = i;
                    }
                }
            } else {
                m = mouse_y / 40;
                if (m == who)
                    return sub_s031_1462_08f6(-1);
                if (party[m].present == 1) {
                    if (D_907a == 1 && cbt_floor[m] == (char)0x9d)
                        vec_1d51(m);
                    else {
                        who = m;
                        D_867d = m;
                        party_panel_frames(0x100);
                        party_panel_rows(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(who);
                        sub_s031_1462_1d44(D_867c, 12);
                    }
                }
            }
            while (mouse_buttons & 1)
                ;
        }
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s031_1462_2518(who, 11);
            D_867e = 0;
            if (D_867a) {
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0a82(who);
        }
        if (key_pressed()) {
            key = get_key();
            if (key == 0x3b00 || key == 0x3c00 || key == 0x3d00 || key == 0x3e00 || key == 0x3f00) {
                m = (key >> 8) - 0x3b;
                if (m == who)
                    return sub_s031_1462_08f6(-1);
                if (party[m].present == 1) {
                    if (D_907a == 1 && cbt_floor[m] == (char)0x9d)
                        vec_1d51(m);
                    else {
                        who = m;
                        D_867d = m;
                        party_panel_frames(0x100);
                        party_panel_rows(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(m);
                        sub_s031_1462_1d44(D_867c, 12);
                    }
                }
            } else if (key == 0x1c0d && D_867c >= 12 && D_867c <= 16) {
                    m = D_867c - 12;
                    if (party[m].present == 0)
                        return;
                    if (D_907a == 1 && cbt_floor[m] == (char)0x9d)
                        vec_1d51(m);
                    else if (who != m) {
                        who = m;
                        D_867d = m;
                        party_panel_frames(0x100);
                        party_panel_rows(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(m);
                    } else
                        key = 0x11b;
            }
            if (key == 0x11b) {
                sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
                return sub_s031_1462_08f6(-1);
            }
        }
        if (key)
            sub_s031_1462_10ae(key, who);
    }
}

int far sub_s031_1462_08f6(int r)
{
    D_9920 = 1;
    D_9934 = 1;
    if (D_867a)
        free_buffer(D_867a, "b");
    D_867a = 0;
    mouse_set_cursor(0, 0, cursor_default);
    if (D_8680)
        free_buffer(D_8680, "h");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    return r;
}

void far sub_s031_1462_0950(int k)
{
    mouse_hide();
    if (D_867a) {
        gfx_draw_pic(&screen_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    mouse_set_cursor(0, 0, cursor_default);
    mouse_show();
    D_867e = 0;
}

void far sub_s031_1462_09b8(int who)
{
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    back_win_font = 1;
    screen_win_font = 1;
    sub_s031_1462_2518(who, 11);
    if (D_867a) {
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    pic_draw_pal(1, "pics\\armback.pic", cur_palette);
    sub_root_047c_0545();
    D_8680 = pic_load("pics\\handicon.pic");
    D_8678 = pic_load("pics\\handicn2.pic");
    D_8682[1] = pic_load("pics\\armbrsh1.pic");
    D_8682[2] = pic_load("pics\\armbrsh2.pic");
    D_8682[3] = pic_load("pics\\armbrsh3.pic");
    D_8682[4] = pic_load("pics\\armbrsh4.pic");
    D_8682[5] = pic_load("pics\\armbrsh5.pic");
    sub_root_047c_04f8();
    sub_s031_1462_0a82(who);
}

void far sub_s031_1462_0a82(int who)
{
    int i;

    screen_win_colour = 0xbf;
    pic_draw_pal(1, "pics\\armback.pic", cur_palette);
    sub_root_047c_0568();
    mouse_hide();
    set_palette(cur_palette);
    sub_s031_1462_0b22(who);
    gfx_copy_rect(&back_win, 0x3c, 0, 0x104, 0xc8, &screen_win, 0x3c, 0);
    for (i = 0; i < 17; i++)
        sub_s031_1462_39a0(i, who, screen_win);
    mouse_set_cursor(0, 0, cursor_default);
    mouse_show();
    sub_root_047c_056f();
}

void far sub_s031_1462_0b22(int who)
{
    unsigned pic, wp;
    int n;
    char num[8];
    char name[20];

    sub_root_047c_0545();
    pic = pic_load("pics\\armsback.pic");
    sub_root_047c_04f8();
    if (pic)
        gfx_draw_pic(&back_win, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    sub_root_047c_0545();
    switch (party[who].limb_type) {
    case 0x4c:
        pic = pic_load("pics\\pad-lims.pic");
        break;
    case 0x4d:
        pic = pic_load("pics\\pad-lims.pic");
        break;
    case 0x4e:
        pic = pic_load("pics\\leat-lim.pic");
        break;
    case 0x4f:
        pic = pic_load("pics\\studlim.pic");
        break;
    case 0x50:
        pic = pic_load("pics\\cuirblim.pic");
        break;
    case 0x51:
        pic = pic_load("pics\\scalelim.pic");
        break;
    case 0x52:
        pic = pic_load("pics\\chainlim.pic");
        break;
    case 0x53:
        pic = pic_load("pics\\briglim.pic");
        break;
    case 0x54:
        pic = pic_load("pics\\platelim.pic");
        break;
    }
    sub_root_047c_04f8();
    if (pic)
        gfx_draw_pic(&back_win, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    sub_root_047c_0545();
    switch (party[who].vital_type) {
    case 0x43:
        pic = pic_load("pics\\pad-vit.pic");
        break;
    case 0x44:
        pic = pic_load("pics\\pad-vit.pic");
        break;
    case 0x45:
        pic = pic_load("pics\\leat-vit.pic");
        break;
    case 0x46:
        pic = pic_load("pics\\studvit.pic");
        break;
    case 0x47:
        pic = pic_load("pics\\cuirbvit.pic");
        break;
    case 0x48:
        pic = pic_load("pics\\scalevit.pic");
        break;
    case 0x49:
        pic = pic_load("pics\\chainvit.pic");
        break;
    case 0x4a:
        pic = pic_load("pics\\brigvit.pic");
        break;
    case 0x4b:
        pic = pic_load("pics\\platevit.pic");
        break;
    }
    sub_root_047c_04f8();
    if (pic)
        gfx_draw_pic(&back_win, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    if (party[who].weapon_type != -1) {
        switch (party[who].weapon_type) {
        case 0:
            n = 2;
            break;
        case 1:
            n = 3;
            break;
        case 2:
            n = 4;
            break;
        case 3:
            n = 5;
            break;
        case 4:
            n = 6;
            break;
        case 5:
            n = 7;
            break;
        case 6:
            n = 8;
            break;
        case 7: n = 0; break;
        case 8:
            n = 9;
            break;
        case 9: n = 1; break;
        case 10:
            n = 11;
            break;
        case 11:
            n = 12;
            break;
        case 12:
            n = 13;
            break;
        case 13:
            n = 14;
            break;
        case 14:
            n = 15;
            break;
        case 15:
            n = 16;
            break;
        case 16:
            n = 17;
            break;
        case 17:
            n = 18;
            break;
        case 18:
            n = 19;
            break;
        case 19:
            n = 20;
            break;
        case 20:
            n = 21;
            break;
        case 21:
            n = 22;
            break;
        case 22:
            n = 24;
            break;
        case 23:
            n = 0;
            break;
        case 24:
            n = 26;
            break;
        case 25:
            n = 27;
            break;
        case 26:
            n = 1;
            break;
        case 27:
            n = 28;
            break;
        case 28:
            n = 29;
            break;
        case 29:
            n = 30;
            break;
        case 30:
            n = 31;
            break;
        case 31:
            n = 32;
            break;
        case 32:
            n = 33;
            break;
        case 33:
            n = 34;
            break;
        default:
            n = -1;
            break;
        }
        if (n != -1) {
            strcpy(name, "pics\\weapon");
            itoa(n, num, 10);
            strcat(name, num);
            strcat(name, ".pic");
            sub_root_047c_0545();
            wp = pic_load(name);
            sub_root_047c_04f8();
            if (wp)
                gfx_draw_pic(&back_win, 0x3b, 0xd, wp);
            if (wp)
                free_buffer(wp, "l");
        }
    }
    if (party[who].shield_type != -1) {
        strcpy(name, "pics\\");
        if (party[who].shield_type == 0x5f)
            strcat(name, "smallsh.pic");
        else if (party[who].shield_type == 0x60)
            strcat(name, "mediumsh.pic");
        else
            strcat(name, "mediumsh.pic");
        sub_root_047c_0545();
        pic = pic_load(name);
        sub_root_047c_04f8();
        if (pic)
            gfx_draw_pic(&back_win, 0x3b, 0xd, pic);
        if (pic)
            free_buffer(pic, "l");
    }
    if (party[who].missile_type != -1) {
        switch (party[who].missile_type) {
        case 23:
            n = 23;
            break;
        case 24:
            n = 26;
            break;
        case 25:
            n = 27;
            break;
        case 26:
            n = 1;
            break;
        case 27:
            n = 28;
            break;
        case 28:
            n = 29;
            break;
        case 29:
            n = 30;
            break;
        case 30:
            n = 31;
            break;
        case 31:
            n = 32;
            break;
        case 32:
            n = 33;
            break;
        case 33:
            n = 34;
            break;
        default:
            n = -1;
            break;
        }
        if (n != -1) {
            strcpy(name, "pics\\weapon");
            itoa(n, num, 10);
            strcat(name, num);
            strcat(name, ".pic");
            sub_root_047c_0545();
            wp = pic_load(name);
            sub_root_047c_04f8();
            if (wp)
                gfx_draw_pic(&back_win, 0x3b, 0xd, wp);
            if (wp)
                free_buffer(wp, "l");
        }
    }
}

void far sub_s031_1462_10ae(register int key, int who)
{
    int idxk, mmg, codeg;
    unsigned char ccm;

    switch (key) {
    case 0x11b:                                 /* Esc */
        if (D_867e) {
            sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, 12);
        }
        break;
    case 0x1c0d:                                /* Enter */
        if (D_867e) {
            sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, 12);
        } else
            sub_s031_1462_264c(D_867c, who);
        break;
    case 0x4b00:                                /* Left */
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
        sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].left, 12);
        break;
    case 0x4d00:                                /* Right */
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
        sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].right, 12);
        break;
    case 0x4800:                                /* Up */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].sel[who] == 0) {
                if (F_290e_3557[D_867c].top[who] == 0)
                    return;
                F_290e_3557[D_867c].top[who]--;
                sub_s031_1462_25de(D_867c, who);
            } else {
                mouse_hide();
                gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                F_290e_3557[D_867c].sel[who]--;
                gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                mouse_show();
            }
        } else {
            if (D_867e)
                sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
            sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].up, 12);
        }
        break;
    case 0x5000:                                /* Down */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            F_290e_3557[D_867c].sel[who]++;
            if (F_290e_3557[D_867c].sel[who] >= F_290e_3557[D_867c].nvis) {
                ccm = F_290e_3557[D_867c].sel[who];
                F_290e_3557[D_867c].sel[who]--;
                if (F_290e_3557[D_867c].top[who] + ccm >= D_e7a4)
                    return;
                F_290e_3557[D_867c].top[who]++;
                sub_s031_1462_25de(D_867c, who);
            } else {
                if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4) {
                    F_290e_3557[D_867c].sel[who]--;
                    return;
                }
                mouse_hide();
                gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 1, F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                gfx_recolor_rect(&screen_win, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                mouse_show();
            }
        } else {
            if (D_867e)
                sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
            sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].down, 12);
        }
        break;
    case 0x4900:                                /* PgUp */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].top[who] > 14)
                F_290e_3557[D_867c].top[who] -= 14;
            else
                F_290e_3557[D_867c].top[who] = 0;
            sub_s031_1462_25de(D_867c, who);
        }
        break;
    case 0x5100:                                /* PgDn */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].top[who] + 14 >= D_e7a4)
                return;
            F_290e_3557[D_867c].top[who] += 14;
            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4) {
                if (D_e7a4 >= 14)
                    F_290e_3557[D_867c].top[who] = D_e7a4 - 14;
                else
                    F_290e_3557[D_867c].top[who] = 0;
                F_290e_3557[D_867c].sel[who] = D_e7a4 >= 14 ? 13 : D_e7a4 - 1;
            }
            sub_s031_1462_25de(D_867c, who);
        }
        break;
    case 0x4f00:                                /* End */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            mouse_hide();
            F_290e_3557[D_867c].top[who] = D_e7a4 >= 14 ? D_e7a4 - 14 : 0;
            F_290e_3557[D_867c].sel[who] = D_e7a4 >= 14 ? 13 : D_e7a4 - 1;
            sub_s031_1462_25de(D_867c, who);
            mouse_show();
        }
        break;
    case 0x4700:                                /* Home */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            mouse_hide();
            F_290e_3557[D_867c].top[who] = 0;
            F_290e_3557[D_867c].sel[who] = 0;
            sub_s031_1462_25de(D_867c, who);
            mouse_show();
        }
        break;
    case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:   /* Ctrl-F1..F5: new leader */
        key = (char)(key >> 8) - 0x5e;
        if (key >= party_size || party[key].present != 1)
            return;
        leader = key;
        party_panel_rows(0x100);
    case 0x231: case 0x332: case 0x433: case 0x534: case 0x635:       /* 1..5: give to member */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            if (party[(key & 7) - 1].present == 0)
                return;
            if (party[(key & 7) - 1].present == 5)
                return;
            if ((idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who]) >= party[who].nitems)
                return;
            if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1))
                give_stack(who, (key & 0xf) - 1, idxk, D_867c);
            else
                give_one(who, (key & 0xf) - 1, idxk, D_867c);
            mouse_hide();
            if (D_867a) {
                gfx_draw_pic(&back_win, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            sub_root_047c_0545();
            D_867a = gfx_save_rect(0, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, F_290e_3557[D_867c].pw + 3, F_290e_3557[D_867c].ph + 0x11);
            sub_root_047c_04f8();
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, screen_win);
            mouse_show();
        } else {
            mmg = (key & 0xf) + 0xb;
            if (party[mmg].present == 1)
                return;
            if (D_907a == 1 && cbt_floor[mmg] == (char)0x9d) {
                vec_1d51(mmg);
                return;
            }
            D_867d = mmg;
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s031_1462_2518(mmg, 11);
            D_867e = 0;
            if (D_867a) {
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0a82(who);
            sub_s031_1462_1d44(D_867c, 12);
        }
        break;
    case 0x2044: case 0x2064:                   /* d: drop */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            if ((idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who]) >= party[who].nitems)
                return;
            if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1))
                item_remove_stack(who, idxk);
            else
                item_take_one(who, idxk);
            mouse_hide();
            if (D_867a)
                gfx_draw_pic(&back_win, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, screen_win);
            mouse_show();
        }
        break;
    case 0x1e41: case 0x1e61:                   /* a */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            sub_s031_18e7_11b2(who, idxk);
            mouse_hide();
            if (D_867a)
                gfx_draw_pic(&back_win, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            if (D_867c == 9) {
                if (D_867a)
                    free_buffer(D_867a, "b");
                sub_root_047c_0545();
                D_867a = gfx_save_rect(0, F_290e_3557[9].ix - 1, F_290e_3557[9].iy, F_290e_3557[9].pw + 3, F_290e_3557[9].ph + 0x11);
                sub_root_047c_04f8();
                sub_s031_1462_25de(D_867c, who);
            } else
                sub_s031_1462_264c(D_867c, who);
            mouse_hide();
            gfx_copy_rect(&back_win, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &screen_win, F_290e_3557[6].x, F_290e_3557[6].y);
            sub_s031_1462_39a0(6, who, screen_win);
            mouse_show();
        }
        break;
    case 0x1655: case 0x1675:                   /* u */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            item_unequip(who, idxk, D_867c);
            mouse_hide();
            if (D_867a)
                gfx_draw_pic(&back_win, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            if (D_867c == 9) {
                if (D_867a)
                    free_buffer(D_867a, "b");
                sub_root_047c_0545();
                D_867a = gfx_save_rect(0, F_290e_3557[9].ix - 1, F_290e_3557[9].iy, F_290e_3557[9].pw + 3, F_290e_3557[9].ph + 0x11);
                sub_root_047c_04f8();
                sub_s031_1462_25de(D_867c, who);
            } else
                sub_s031_1462_264c(D_867c, who);
            mouse_hide();
            gfx_copy_rect(&back_win, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &screen_win, F_290e_3557[6].x, F_290e_3557[6].y);
            sub_s031_1462_39a0(6, who, screen_win);
            mouse_show();
        }
        break;
    case 0x1950: case 0x1970:                   /* p */
        if (D_867c == 9 && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            codeg = data_file[who].items[idxk].code & 0xfff;
            if (!(((struct itemdef far *)item_defs[codeg])->flags & 0x800L))
                return;
            if (((struct itemdef far *)item_defs[codeg])->flags & 0x1000000L)
                return;
            sub_s031_1462_43b2(who, idxk);
            item_take_one(who, idxk);
            if (party[who].nitems < D_e7a4) {
                if (F_290e_3557[9].top[who] != 0)
                    F_290e_3557[9].top[who]--;
                else if (F_290e_3557[9].sel[who] != 0)
                    F_290e_3557[9].sel[who]--;
                D_e7a4 = party[who].nitems;
            }
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, screen_win);
        }
        break;
    case 0x1000:                                /* Alt-Q */
        sub_s031_1462_08f6(-1);
        quit_to_dos();
        break;
    default:
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        break;
    }
}

void far sub_s031_1462_1d44(unsigned char k, int color)
{
    mouse_hide();
    if (color == 0xff) {
        color = k - 12;
        party_panel_frames(color);
        party_panel_rows(color);
        sub_s031_1462_2518(D_867d, 11);
    } else {
        gfx_line(&screen_win, F_290e_3557[k].x, F_290e_3557[k].y,
                           F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y, color);
        gfx_line(&screen_win, F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y,
                           F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y + F_290e_3557[k].h, color);
        gfx_line(&screen_win, F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y + F_290e_3557[k].h,
                           F_290e_3557[k].x, F_290e_3557[k].y + F_290e_3557[k].h, color);
        gfx_line(&screen_win, F_290e_3557[k].x, F_290e_3557[k].y + F_290e_3557[k].h,
                           F_290e_3557[k].x, F_290e_3557[k].y, color);
    }
    mouse_show();
}

void far sub_s031_1462_1e42(int k, int who, int y)
{
    int shift, i;

    if (*(unsigned char far *)0x00400017L & 1 || *(unsigned char far *)0x00400017L & 2)
        shift = 1;
    else
        shift = 0;
    if ((y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] >= D_e7a4)
        return;
    mouse_hide();
    gfx_recolor_rect(&screen_win, F_290e_3557[k].ix + 8, F_290e_3557[k].iy + F_290e_3557[k].sel[who] * 8 + 9, F_290e_3557[k].pw - 0xf, 8, 9, 8);
    F_290e_3557[D_867c].sel[who] = (y - F_290e_3557[k].iy - 9) / 8;
    gfx_recolor_rect(&screen_win, F_290e_3557[k].ix + 8, F_290e_3557[k].iy + F_290e_3557[k].sel[who] * 8 + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
    if (k == 9)
        mouse_set_cursor(3, 3, D_8678);
    mouse_show();
    while (mouse_buttons & 1)
        ;
    if (mouse_x > 0x3c) {
        if (F_290e_3557[k].ix <= mouse_x && F_290e_3557[k].pw + F_290e_3557[k].ix >= mouse_x
                && F_290e_3557[k].iy <= mouse_y && F_290e_3557[k].ph + F_290e_3557[k].iy >= mouse_y) {
            if (k == 9) {
                shift = (mouse_y - F_290e_3557[k].iy - k) / 8;
                if (shift < 0)
                    shift = 0;
                if (party[who].nitems > shift) {
                    item_move(who, F_290e_3557[k].top[who] + (y - F_290e_3557[k].iy - k) / 8, F_290e_3557[k].top[who] + shift);
                    sub_s031_1462_25de(k, who);
                }
            } else if (k == 10 || k == 11) {
                if (k == 10 && (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] < D_e7a4)
                    alc_view_vec(who, sub_s031_1462_42ac(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]));
                else if (k == 11 && (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] < D_e7a4)
                    vec_200d(who, sub_s031_1462_433c(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]), 0);
                sub_s031_1462_0a82(who);
                sub_s031_1462_264c(k, who);
            }
        } else {
            for (i = 0; i <= 18; i++)
                if (F_290e_3557[i].x <= mouse_x && F_290e_3557[i].w + F_290e_3557[i].x >= mouse_x
                        && F_290e_3557[i].y <= mouse_y && F_290e_3557[i].y + F_290e_3557[i].h >= mouse_y)
                    break;
            if (i == 6) {
                sub_s031_18e7_11b2(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]);
                mouse_hide();
                if (D_867a)
                    gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                sub_s031_1462_0b22(who);
                mouse_show();
                mouse_hide();
                gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
                mouse_show();
                mouse_hide();
                if (D_867c == 9) {
                    if (D_867a)
                        free_buffer(D_867a, "b");
                    sub_root_047c_0545();
                    D_867a = gfx_save_rect(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
                    sub_root_047c_04f8();
                    sub_s031_1462_25de(D_867c, who);
                } else
                    sub_s031_1462_264c(D_867c, who);
                mouse_show();
                mouse_hide();
                gfx_copy_rect(&back_win, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &screen_win, F_290e_3557[6].x, F_290e_3557[6].y);
                sub_s031_1462_39a0(6, who, screen_win);
                mouse_show();
            }
        }
    } else if (party[mouse_y / 40].present != 0 && party[mouse_y / 40].present != 5 && (F_290e_3557[k].flags & 0x10)) {
        if (shift)
            give_stack(who, mouse_y / 40, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who], k);
        else
            give_one(who, mouse_y / 40, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who], k);
        if (k == 9) {
            mouse_hide();
            if (D_867a) {
                gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            sub_root_047c_0545();
            D_867a = gfx_save_rect(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
            sub_root_047c_04f8();
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, screen_win);
            mouse_show();
        } else if (k == 6) {
            mouse_hide();
            if (D_867a) {
                gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            gfx_copy_rect(&back_win, 0x3c, 0xd, 0x8a, 0xaa, &screen_win, 0x3c, 0xd);
            sub_s031_1462_39a0(9, who, screen_win);
            sub_s031_1462_264c(6, who);
            mouse_show();
        }
    }
    if (F_290e_3557[k].flags & 2) {
        mouse_hide();
        mouse_set_cursor(3, 3, D_8680);
        mouse_show();
    }
}

void far sub_s031_1462_2518(int k, int color)
{
    mouse_hide();
    gfx_line(&screen_win, F_290e_3825[k].x, F_290e_3825[k].y,
                       F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y, color);
    gfx_line(&screen_win, F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y,
                       F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y + F_290e_3825[k].h, color);
    gfx_line(&screen_win, F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y + F_290e_3825[k].h,
                       F_290e_3825[k].x, F_290e_3825[k].y + F_290e_3825[k].h, color);
    gfx_line(&screen_win, F_290e_3825[k].x, F_290e_3825[k].y + F_290e_3825[k].h,
                       F_290e_3825[k].x, F_290e_3825[k].y, color);
    mouse_show();
}

void far sub_s031_1462_25de(int k, int who)
{
    mouse_hide();
    sub_root_047c_0545();
    D_8682[0] = pic_load("pics\\armbrs13.pic");
    sub_root_047c_04f8();
    gfx_draw_pic(&screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[0]);
    if (D_8682[0])
        free_buffer(D_8682[0], "x");
    sub_s031_1462_2b14(k, who);
    mouse_show();
}

void far sub_s031_1462_264c(int k, int who)
{
    if (!(F_290e_3557[k].flags & 1)) {
        if (k == 0 && D_867c == k) {
            leader = who;
            party_panel_rows(0x100);
            mouse_hide();
            gfx_copy_rect(&back_win, F_290e_3557[k].x, F_290e_3557[k].y, F_290e_3557[k].w, F_290e_3557[k].h,
                               &screen_win, F_290e_3557[k].x, F_290e_3557[k].y);
            mouse_show();
            mouse_hide();
            sub_s031_1462_39a0(k, who, screen_win);
            mouse_show();
        }
        sub_s031_1462_1d44(k, 12);
        return;
    }
    mouse_hide();
    if (D_867a) {
        gfx_draw_pic(&screen_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    sub_root_047c_0545();
    D_867a = gfx_save_rect(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy,
                                F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
    sub_root_047c_04f8();
    if (F_290e_3557[k].flags & 2) {
        sub_root_047c_0545();
        D_8682[8] = pic_load("pics\\armbrsh8.pic");
        D_8682[9] = pic_load("pics\\armbrsh9.pic");
        D_8682[10] = pic_load("pics\\armbrs10.pic");
        D_8682[12] = pic_load("pics\\armbrs12.pic");
        sub_root_047c_04f8();
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x71, D_8682[8]);
        gfx_copy_rect(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x67, D_8682[9]);
        gfx_copy_rect(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x4e, D_8682[10]);
        gfx_copy_rect(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x11, D_8682[12]);
        gfx_copy_rect(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy);
        if (D_8682[8])
            free_buffer(D_8682[8], "x");
        D_8682[8] = 0;
        sub_root_047c_0545();
        D_8682[0] = pic_load("pics\\armbrs13.pic");
        sub_root_047c_04f8();
        vec_1905("full up", "a");
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[0]);
        gfx_copy_rect(&back_win, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy);
        gfx_draw_pic(&back_win, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        if (D_8682[0])
            free_buffer(D_8682[0], "x");
        D_8682[0] = 0;
    } else {
        gfx_draw_pic(&screen_win, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[F_290e_3557[k].pic]);
    }
    sub_s031_1462_2b14(k, who);
    D_867e = 1;
    if (F_290e_3557[k].flags & 2)
        mouse_set_cursor(5, 0, D_8680);
    mouse_show();
}

void far sub_s031_1462_2b14(int k, int who)
{
    register int j, n;
    int cntl, topm, bitc;
    char buff[40];

    screen_win_colour = 0xbf;
    switch (k) {
    case 3:
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x5, F_290e_1e67[0]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0xd, F_290e_1e67[1]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x15, F_290e_1e67[2]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x1d, F_290e_1e67[3]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x25, F_290e_1e67[4]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x2d, F_290e_1e67[5]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x35, F_290e_1e67[6]);
        for (j = 0; j < 7; j++) {
            sprintf(buff, "%02d/%02d", party[who].attr[j], party[who].attr_max[j]);
            gfx_text(&screen_win, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 4:
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x5, F_290e_1e67[7]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0xd, F_290e_1e67[8]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x15, F_290e_1e67[9]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x1d, F_290e_1e67[10]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x25, F_290e_1e67[11]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x2d, F_290e_1e67[12]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x35, F_290e_1e67[13]);
        for (j = 0; j < 7; j++) {
            sprintf(buff, "%02d", party[who].skill[j]);
            gfx_text(&screen_win, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 5:
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x5, F_290e_1e67[14]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0xd, F_290e_1e67[15]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x15, F_290e_1e67[16]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x1d, F_290e_1e67[17]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x25, F_290e_1e67[18]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x2d, F_290e_1e67[19]);
        for (j = 0; j < 6; j++) {
            sprintf(buff, "%02d", party[who].skill[7 + j]);
            gfx_text(&screen_win, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 6:
        if (party[who].weapon_type != -1)
            draw_text_far(&screen_win, F_290e_3557[k].ix + 4, 0x58, item_defs[equipped_item_impl(who, 0)]);
        else
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x58, "No Weapon");
        if (party[who].vital_type != -1)
            draw_text_far(&screen_win, F_290e_3557[k].ix + 4, 0x62, item_defs[equipped_item_impl(who, 1)]);
        else
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x62, "No Vitals Armor");
        if (party[who].limb_type != -1)
            draw_text_far(&screen_win, F_290e_3557[k].ix + 4, 0x6c, item_defs[equipped_item_impl(who, 2)]);
        else
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x6c, "No Limb Armor");
        if (party[who].shield_type != -1)
            draw_text_far(&screen_win, F_290e_3557[k].ix + 4, 0x76, item_defs[equipped_item_impl(who, 3)]);
        else
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x76, "No Shield");
        if (party[who].missile_type != -1)
            draw_text_far(&screen_win, F_290e_3557[k].ix + 4, 0x80, item_defs[equipped_item_impl(who, 4)]);
        else
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x80, "No Missile Weapon");
        if (sub_s029_0e76_3996(0x49, who) || sub_s029_0e76_3996(0x48, who) || sub_s029_0e76_3996(0x4a, who))
            gfx_text(&screen_win, F_290e_3557[k].ix + 5, 0x8a, "Enhc/Bless");
        break;
    case 7:
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x5, F_290e_1e67[20]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0xd, F_290e_1e67[21]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x15, F_290e_1e67[22]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x1d, F_290e_1e67[23]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x25, F_290e_1e67[24]);
        draw_text_far(&screen_win, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x2d, F_290e_1e67[25]);
        for (j = 0; j < 6; j++) {
            sprintf(buff, "%02d", party[who].skill[13 + j]);
            gfx_text(&screen_win, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 9:
        if (party[who].nitems <= 0)
            break;
        {
            screen_win_colour = 8;
            if (F_290e_3557[k].top[who] >= party[who].nitems)
                F_290e_3557[k].top[who] = clamp_range(0, 0x63, party[who].nitems - 10);
            if (F_290e_3557[k].sel[who] >= party[who].nitems)
                F_290e_3557[k].sel[who] = party[who].nitems - 1;
            for (j = 0; j < F_290e_3557[k].nvis; j++) {
                if (F_290e_3557[k].top[who] + j < party[who].nitems) {
                    sprintf(buff, "%Fs %02dq (%d)",
                            item_defs[data_file[who].items[F_290e_3557[k].top[who] + j].code & 0xfff],
                            data_file[who].items[F_290e_3557[k].top[who] + j].qual,
                            data_file[who].items[F_290e_3557[k].top[who] + j].cnt);
                    gfx_text(&screen_win, F_290e_3557[k].ix + 9, j * 8 + F_290e_3557[k].iy + 9, buff);
                }
            }
            gfx_recolor_rect(&screen_win, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
            D_e7a4 = party[who].nitems;
            screen_win_colour = 0xbf;
        }
        break;
    case 10:
        screen_win_colour = 8;
        cntl = n = j = 0;
        for (; j < 22 && n < F_290e_3557[k].nvis; j++) {
            if ((formulas_known[who][j] & 1) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                draw_text_far(&screen_win, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, virtue_names[j * 3]);
            if ((formulas_known[who][j] & 2) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                draw_text_far(&screen_win, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, virtue_names[j * 3 + 1]);
            if ((formulas_known[who][j] & 4) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                draw_text_far(&screen_win, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, virtue_names[j * 3 + 2]);
        }
        gfx_recolor_rect(&screen_win, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
        D_e7a4 = 0;
        for (j = 0; j < 22; j++) {
            if (formulas_known[who][j] & 1)
                D_e7a4++;
            if (formulas_known[who][j] & 2)
                D_e7a4++;
            if (formulas_known[who][j] & 4)
                D_e7a4++;
        }
        screen_win_colour = 0xbf;
        break;
    case 11:
        screen_win_colour = 8;
        topm = F_290e_3557[k].top[who];
        j = n = 0;
        for (; j < 17 && n - F_290e_3557[k].top[who] < F_290e_3557[k].nvis; j++) {
            for (bitc = 0; bitc < 8; bitc++) {
                if (saints_known[who][j] & D_00e8[bitc % 8]) {
                    if (F_290e_3557[k].top[who] <= n && n - F_290e_3557[k].top[who] < F_290e_3557[k].nvis)
                        draw_text_far(&screen_win, F_290e_3557[k].ix + 9, (n - topm) * 8 + F_290e_3557[k].iy + 9, saint_names[j * 8 + bitc]);
                    n++;
                }
            }
        }
        D_e7a4 = 0;
        for (j = 0; j < 17; j++)
            for (bitc = 0; bitc < 8; bitc++)
                if (saints_known[who][j] & D_00e8[bitc % 8])
                    D_e7a4++;
        gfx_recolor_rect(&screen_win, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
        screen_win_colour = 0xbf;
        break;
    }
}

void far sub_s031_1462_39a0(int i, int who, struct gfx g)
{
    sub_s031_1462_39c6(i, who, g);
}

void far sub_s031_1462_39c6(int i, int who, struct gfx g)
{
    int j, y, t;
    char buf[40];

    switch (i) {
    case 0:
        gfx_copy_rect(&back_win, F_290e_3557[0].x, F_290e_3557[0].y, F_290e_3557[0].w, F_290e_3557[0].h, &screen_win, F_290e_3557[0].x, F_290e_3557[0].y);
        gfx_text(&g, F_290e_3557[i].x + 5, 3, party[who].full_name);
        if (leader == who)
            gfx_text(&g, F_290e_3557[i].x + 0x82, F_290e_3557[i].y + 3, "Leader");
        else
            gfx_text(&g, F_290e_3557[i].x + 0x82, F_290e_3557[i].y + 3, "Not Leader");
        break;
    case 1:
        gfx_text(&g, F_290e_3557[i].x + 5, F_290e_3557[i].y + 3, "Age");
        sprintf(buf, "%02d", *(int *)((char *)&party[who] - 2));
        gfx_text(&g, F_290e_3557[i].x + 0x19, F_290e_3557[i].y + 3, buf);
        if (party[who].pad01[2] == 0)
            gfx_text(&g, F_290e_3557[i].x + 0x28, F_290e_3557[i].y + 3, "(M)");
        else
            gfx_text(&g, F_290e_3557[i].x + 0x28, F_290e_3557[i].y + 3, "(F)");
        break;
    case 2:
        gfx_copy_rect(&back_win, F_290e_3557[2].x, F_290e_3557[2].y, F_290e_3557[2].w, F_290e_3557[2].h, &screen_win, F_290e_3557[2].x, F_290e_3557[2].y);
        switch (sub_s029_0e76_013a(who)) {
        case 0:
            gfx_text(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Light Load");
            break;
        case 1:
            gfx_text(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Normal Load");
            break;
        case 2:
            gfx_text(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Heavy Load");
            break;
        case 3:
            gfx_text(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Overloaded");
            break;
        }
        sprintf(buf, "%02d/%02d", party[who].pad35[0], party[who].attr[1] + party[who].attr[0]);
        gfx_text(&g, F_290e_3557[2].x + 0x48, F_290e_3557[2].y + 4, buf);
        break;
    case 3:
        gfx_text(&g, 0xed, 0x12, "End");
        gfx_text(&g, 0xed, 0x1a, "Str");
        gfx_text(&g, 0xed, 0x22, "Agl");
        gfx_text(&g, 0xed, 0x2a, "Per");
        gfx_text(&g, 0xed, 0x32, "Int");
        gfx_text(&g, 0xed, 0x3a, "Chr");
        gfx_text(&g, 0xed, 0x42, "DF");
        for (j = 0, y = 0x12; y < 0x4a; j++, y += 8) {
            sprintf(buf, "%02d/%02d", party[who].attr[j], party[who].attr_max[j]);
            gfx_text(&g, 0xca, y, buf);
        }
        break;
    case 4:
        gfx_text(&g, 0x11d, 0x12, "wEdg");
        gfx_text(&g, 0x11d, 0x1a, "wImp");
        gfx_text(&g, 0x11d, 0x22, "wFll");
        gfx_text(&g, 0x11d, 0x2a, "wPol");
        gfx_text(&g, 0x11d, 0x32, "wThr");
        gfx_text(&g, 0x11d, 0x3a, "wBow");
        gfx_text(&g, 0x11d, 0x42, "wMsD");
        for (j = 0, y = 0x12; y < 0x4a; j++, y += 8) {
            sprintf(buf, "%02d", party[who].skill[j]);
            gfx_text(&g, 0x10c, y, buf);
        }
        break;
    case 5:
        gfx_text(&g, 0x11d, 0x55, "Alch");
        gfx_text(&g, 0x11d, 0x5d, "Relg");
        gfx_text(&g, 0x11d, 0x65, "Virt");
        gfx_text(&g, 0x11d, 0x6d, "SpkC");
        gfx_text(&g, 0x11d, 0x75, "SpkL");
        gfx_text(&g, 0x11d, 0x7d, "R&W");
        for (j = 0, y = 0x55; y < 0x85; j++, y += 8) {
            sprintf(buf, "%02d", party[who].skill[7 + j]);
            gfx_text(&g, 0x10c, y, buf);
        }
        break;
    case 6:
        gfx_copy_rect(&back_win, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &screen_win, F_290e_3557[6].x, F_290e_3557[6].y);
        if (party[who].weapon_type != -1 && (t = equipped_item_impl(who, 0)) != -1)
            draw_text_far(&g, F_290e_3557[i].x + 4, 0x58, item_defs[t] + 0x14);
        else
            gfx_text(&g, F_290e_3557[i].x + 4, 0x58, "No Weapon");
        if (party[who].vital_type != -1 && (t = equipped_item_impl(who, 1)) != -1)
            draw_text_far(&g, F_290e_3557[i].x + 4, 0x62, item_defs[t] + 0x14);
        else
            gfx_text(&g, F_290e_3557[i].x + 4, 0x62, "No V:Armor");
        if (party[who].limb_type != -1 && (t = equipped_item_impl(who, 2)) != -1)
            draw_text_far(&g, F_290e_3557[i].x + 4, 0x6c, item_defs[t] + 0x14);
        else
            gfx_text(&g, F_290e_3557[i].x + 4, 0x6c, "No L:Armor");
        if (party[who].shield_type != -1 && (t = equipped_item_impl(who, 3)) != -1)
            draw_text_far(&g, F_290e_3557[i].x + 4, 0x76, item_defs[t] + 0x14);
        else
            gfx_text(&g, F_290e_3557[i].x + 4, 0x76, "No Shield");
        if (party[who].missile_type != -1 && (t = equipped_item_impl(who, 4)) != -1)
            draw_text_far(&g, F_290e_3557[i].x + 4, 0x80, item_defs[t] + 0x14);
        else
            gfx_text(&g, F_290e_3557[i].x + 4, 0x80, "No Msl Wpn");
        sub_s031_1462_39a0(2, who, g);
        break;
    case 7:
        gfx_text(&g, 0x11d, 0x91, "Heal");
        gfx_text(&g, 0x11d, 0x99, "Artf");
        gfx_text(&g, 0x11d, 0xa1, "Stlh");
        gfx_text(&g, 0x11d, 0xa9, "StrW");
        gfx_text(&g, 0x11d, 0xb1, "Ride");
        gfx_text(&g, 0x11d, 0xb9, "WdWs");
        for (j = 0, y = 0x91; y < 0xc1; j++, y += 8) {
            sprintf(buf, "%02d", party[who].skill[13 + j]);
            gfx_text(&g, F_290e_3557[i].x + 3, y, buf);
        }
        break;
    case 17:
        gfx_text(&g, 0xed, 0x12, "End");
        gfx_text(&g, 0xed, 0x1a, "Str");
        gfx_text(&g, 0xed, 0x22, "Agl");
        gfx_text(&g, 0xed, 0x2a, "Per");
        gfx_text(&g, 0xed, 0x32, "Int");
        gfx_text(&g, 0xed, 0x3a, "Chr");
        gfx_text(&g, 0xed, 0x42, "DF");
        for (j = 0, y = 0x20; y < 0x58; j++, y += 8) {
            sprintf(buf, "%02d/%02d", party[who].attr[j], party[who].attr_max[j]);
            gfx_text(&g, 0xca, y, buf);
        }
        break;
    }
}

/* n-th set flag (3 per entry) in who's 22 flag bytes at 2e38:07be */
int far sub_s031_1462_42ac(int who, int n)
{
    int i;

    i = 0;
    do {
        if ((formulas_known[who][i] & 1) && n-- <= 0)
            return i * 3;
        if ((formulas_known[who][i] & 2) && n-- <= 0)
            return i * 3 + 1;
        if ((formulas_known[who][i] & 4) && n-- <= 0)
            return i * 3 + 2;
    } while (++i < 22);
}

/* n-th set bit in who's 17 bit bytes at 2e38:082c */
int far sub_s031_1462_433c(int who, int n)
{
    int i, b, m;

    m = 0x80;               /* mask variable: 6.00A keeps this store (mov [bp-2],80h) */
    for (i = 0; i < 17; i++) {
        b = 0;
        for (; b < 8; b++)
            if ((saints_known[who][i] & (m >> b)) && n-- <= 0)
                return i * 8 + b;
    }
}

void far sub_s031_1462_43b2(int who, int idx)
{
    int hi, lo, t, c;

    c = data_file[who].items[idx].code & 0xfff;
    switch (c) {
    case 0x69:
        hi = party[who].x34 >> 4;
        lo = party[who].x34 & 0xf;
        if (data_file[who].items[idx].qual <= 0x19)
            t = 3;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 4;
        else
            t = 5;
        sub_s029_0e76_4d24(who, c, t, 0x18);
        party[who].x34 = ((t + hi) << 4) + lo;
        break;
    case 0x6a:
        hi = party[who].x34 >> 4;
        lo = party[who].x34 & 0xf;
        if (data_file[who].items[idx].qual <= 0x19)
            t = 3;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 4;
        else
            t = 5;
        sub_s029_0e76_4d24(who, idx, t, 0x18);
        party[who].x34 = (hi << 4) + t + lo;
        break;
    case 0x6b:
        if (data_file[who].items[idx].qual <= 0x19)
            t = 0xe;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 0x16;
        else
            t = 0x1e;
        sub_s029_0e76_4d24(who, idx, t, 0x18);
        party[who].x34 = party[who].x34 % 16 + (party[who].x34 & 0xf0) + t;
        break;
    case 0x6c:
        if (data_file[who].items[idx].qual <= 0x19)
            t = 0xe;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 0x16;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = 0x1e;
        else
            t = 5;
        lower_skill_temp(who, 5, t, 0x18);
        lower_skill_temp(who, 4, t, 0x18);
        lower_skill_temp(who, 6, t, 0x18);
        break;
    case 0x6d:
        hi = party[who].x41 >> 4;
        lo = party[who].x41 & 0xf;
        if (data_file[who].items[idx].qual <= 0x19)
            t = 1;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 2;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = 3;
        else
            t = 5;
        party[who].x41 = (hi << 4) + t + lo;
        break;
    case 0x6e:
        party_money++;
        break;
    case 0x6f:
        if (data_file[who].items[idx].qual <= 0x19)
            t = rand_mod(6) + 9;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = rand_mod(7) + 0xf;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = rand_mod(8) + 0x16;
        else
            t = 5;
        raise_attr_temp(who, 3, t, 0x18);
        break;
    case 0x70:
        if (data_file[who].items[idx].qual <= 0x19)
            t = rand_mod(6) + 6;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = rand_mod(6) + 8;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = rand_mod(6) + 0xa;
        else
            t = 5;
        raise_attr_temp(who, 0, t, 0x18);
        break;
    case 0x71:
        if (data_file[who].items[idx].qual <= 0x19)
            t = rand_mod(6) + 6;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = rand_mod(6) + 8;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = rand_mod(6) + 0xa;
        else
            t = 5;
        raise_attr_temp(who, 1, t, 0x18);
        break;
    case 0x72:
        if (data_file[who].items[idx].qual <= 0x19)
            t = rand_mod(6) + 6;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = rand_mod(8) + 8;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = rand_mod(8) + 0xa;
        else
            t = 5;
        raise_attr_temp(who, 2, t, 0x18);
        break;
    case 0x73:
        if (data_file[who].items[idx].qual <= 0x19)
            t = rand_mod(6) + 9;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = rand_mod(7) + 0xf;
        else if (data_file[who].items[idx].qual <= 0x2d)
            t = rand_mod(8) + 0x16;
        else
            t = 5;
        heal_attr_pct(who, 1, data_file[who].items[idx].qual * 2);
        raise_attr_temp(who, 0, t, 0x22);
        break;
    case 0x74:
        hi = party[who].x41 >> 4;
        lo = party[who].x41 & 0xf;
        if (data_file[who].items[idx].qual <= 0x19)
            t = 0x32;
        else if (data_file[who].items[idx].qual <= 0x23)
            t = 0x46;
        else
            t = 0x5a;
        party[who].x41 = ((hi + 1) << 4) + lo;
        party[who].x10 = t;
        break;
    }
    party_panel_frames(who);
    party_panel_rows(who);
}
