/* place: s030 1462:0000 */
/* orig: size=305e CONST=7e3e+0010 _DATA=2872+01c8 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* Overland travel map (overlay s030): the screen where the party travels
   across the map of medieval Germany.  Note /Oa (assume no aliasing): it is
   what makes MSC address DGROUP through ss: while DS may hold a far segment.
   STATUS (e1d, 2026-10-09): all 22 functions written; 16/22 match (0000 00a8 02c0 1c4e 1f30
     20d2 271a 27aa 2842 28f2 29bc 29fa 2ac8 2bb0 2d44 2dd6); DATA and CONST match.
     1f30 matched (e1d): the second loop reuses r as its counter and k holds r's old value
       (`for (k = r, r = 0; ...) ICONS[k - 1 + r]`), so k no longer shares r's slot.  Adding a
       4th local instead also matched 1f30 but flipped 29fa/2dd6 (symbol count).
     16c6 (e1d): removing xa and writing `(x + (cond ? 16 : 24))` twice gets the cx CSE and
       the frame (14 lines, but the sbb form of the ternary, and it flips 2ac8 2d44 2dd6);
       the branchy `cond ? x + 16 : x + 24` is not CSE'd by 6.00A.  A union `tl` {v, xa}
       gives the right frame but xa stays in memory.
     25be: renames, pragmas a/c/g/l/e/w/t, register hints, a D_2866 copy: si/bx stay swapped.
     2dd6 matched with `unsigned v` (flips `cmp dx,si`); fragile: other declarations flip it back.
     1c76: code identical; its jump table entries are absolute and stay off until the functions
       before it (0c44 -4, 1300 +2, 16c6 +4 bytes) have the right length.
     1dd4 (e4, 2026-10-09): `rows = -(d - 0x83);` (or `0x83u - d`) stops 6.00A from CSE-ing
       0x83 - d with the 3108 size argument and reproduces the original call/reload code
       (frame 0x12, d at [bp-12h]); left: loop temps/first/rows slots (orig first -8, rows
       -0ah; ours one slot higher). NOT applied: it flips 2ac8/2d44/2dd6 (dummy locals in
       1dd4 did not compensate).
     25be (e4): `col = D_2866; col = (... ? col + 16 : col + 24) / 16;` gives the original
       `mov si,[2866h]` start but keeps col in si to the loop and flips 3 other functions.
     (older notes below)  [n] = fdskip.py lines.
     02c0 matched after renaming its locals (hash order) and choosing one-line vs two-line
       `case ..: return K;` arms (build/agent/e1/greedy_lines.py) - see e1 NOTES.
     20d2 matched with hash-ordered names + unused gap arrays + the position of
       `extern int D_286e;` (decides `D_286e + row` operand order; moving it changes 0c44/16c6).
     1c76: only the jump table differs (relocation; case 4 written on one line, last).
     25be [17]: speed/x in bx/si (ours si/bx).
     16c6: slots fixed by renaming; `xa` (x+16/x+24 column) lives in cx in the original,
       ours [bp-18]; operand orders of `global + local` sums depend on extern positions.
     0c44: locals renamed to hash order; left: 6.00 reloads col/k after storing them,
       D_e3f8 kept in cx, operand orders.
     1300, 1dd4: shape right; 6.00A shares stack slots between locals with disjoint
       lifetimes; original has unused slots (try unused arrays + hash names, as in 20d2). */
#include "darklands/card.h"
#include "darklands/game.h"

typedef struct { int x, y; } XY;

struct city {                       /* DARKLAND.CTY record (far) */
    char short_name[32];
    char full_name[32];
    int  city_size;                 /* 0x40 */
    int  entry_x, entry_y;          /* 0x42 */
    int  exit_x, exit_y;            /* 0x46: party position after leaving */
};

struct location {                   /* DARKLAND.LOC record, 0x3a bytes */
    int  icon;                      /* 0x00 */
    int  unk02;
    int  x, y;                      /* 0x04 */
    int  unk08, unk0a;
    int  menu;                      /* 0x0c */
    int  unk0e;
    char pad10[4];
    unsigned char flags;            /* 0x14 */
    char pad15[2];
    char unk17;                     /* 0x17 */
};

/* far data in segment 2e38 */
struct far2e38 {
    char pad0[0x10be];
    struct location far *loc[414];  /* 2e38:10be  all locations; 0..91 are cities */
};
extern struct far2e38 far data_file;
extern int D_2866, D_2868;          /* party marker screen offset */

extern int cur_loc;                  /* current location index, -1 = wilderness */
extern struct city far *city_rec;     /* current city record */
extern int party_x, party_y;          /* party map x, y */
extern int next_card;                  /* next card/menu id */

extern int cur_month;
extern int D_2860;
extern unsigned D_2870;             /* map buffer segment */
extern int D_775a;                  /* errno */
extern int D_8672;
extern int D_9c3d, cursor_default;
extern int D_a891;
extern int D_e3f2;
extern struct _iobuf *D_e3f4;       /* DARKLAND.MAP file */
extern int D_e3f6, D_e3f8;          /* map width, height */
extern unsigned D_e416;
extern int D_e488;

struct _iobuf *fopen(const char *, const char *);
int printf(const char *, ...);
int fcloseall(void);
unsigned _dos_freemem(unsigned);

int far event_started(int);
void far event_here(int, int);
int far party_get(int);
void far copyprot_run_vec(void);
int far vec_18f1(int);
int far rand_mod(int);
void far mouse_hide(void);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far mouse_set_cursor(int, int, int);
void far mem_free(unsigned, char *);

void far sub_s030_1462_1c4e(struct _iobuf *);
void far sub_s030_1462_1c76(struct _iobuf *);
void far sub_s030_1462_1f30(void);
void far sub_s030_1462_20d2(int, int);
int far overland_loop(void);
#define BIOS_KBFLAGS ((unsigned char far *)0x00400017L)
int far overland_open(int, int);

extern unsigned far F_2e38_1ffc[];

struct party_slot { char present; char pad[127]; };   /* 128-byte records at DGROUP 9c69 */
extern struct party_slot party[];
extern int mouse_x, mouse_y;          /* mouse x, y */
extern unsigned char mouse_buttons;        /* mouse buttons */
extern int screen_win[];                /* status-line window */
extern int screen_win_colour;                  /* its text colour */
extern int D_9419[];
extern int party_icons_dirty;                  /* party changed: reload icons */
extern int cur_palette[];                /* palette */
extern signed char D_0880, D_0881, D_0882;   /* joystick-ish x, y, button */
extern signed char D_e3fa, D_e3fb;
extern char city_desc[];               /* city description (DARKLAND.DSC record) */
extern int D_d843;                  /* terrain type under the party */

int sprintf(char *, const char *, ...);
struct _iobuf *fdopen(int, const char *);
int fseek(struct _iobuf *, long, int);
int fclose(struct _iobuf *);
int fgetc(struct _iobuf *);
unsigned fread(void *, unsigned, unsigned, struct _iobuf *);
unsigned far sub_root_06a1_2c9e(const char *, unsigned, int *);   /* _dos_open */
unsigned _dos_read(int, void far *, unsigned, unsigned *);

void far member_sheet_vec(int);
extern int D_286e;                  /* map view: top row */
int far vec_1fe5(void);
int far party_lore_level(void);
void far set_palette(int *);
void far gfx_text(int *, int, int, char *);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far menu_bar_run(int *, int);
void far pic_draw_pal(int, char *, int *);
int far key_pressed(void);
int far get_key(void);
int far ev8_at(int, int);
int far loc_fact_value_any(int, int);
int far is_daylight(void);
int far ev8_exists(int);
void far event_update(int, int, int, int, int, int, int);
int far loc_fact_any(int, int);
void far sub_s030_1767_0046(long);
void far sub_s030_1462_2dd6(void);
int far sub_s030_1462_0c44(int);

extern int cur_day;
extern int D_8676, D_290e, D_2910;
extern char leader;
void far vec_1ff9(int);
void far free_buffer(unsigned, char *);
void far delay_ticks(long);
void far quit_to_dos(void);
int far sub_s030_1462_1300(int, int);
void far sub_s030_1462_16c6(int, int, int, int);
int far sub_s030_1462_25be(void);
int far nearest_city_impl(int, int);
int far sub_s030_1462_2ac8(int);
int far sub_s030_1462_2d44(int, int);
void far map_message(int);
void far event_remove(int, int, int, int, int);
int abs(int);
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
void far exit_(int);
int far gfx_save_rect(int, int, int, int, int);
void far sub_root_047c_04f8(void);
void far sub_root_047c_0545(void);
void far gfx_fill_rect(int *, int, int, int, int, int);
void far gfx_line(int *, int, int, int, int, int);
int far font_height(int);
void far draw_text_wrapped(int *, char *, int, int, int, int, int, int, int, int, int, int);
void far gfx_draw_pic(int *, int, int, unsigned);
void far mouse_show(void);
extern int screen_win_font;
#define ICONS ((unsigned (far *)[16])F_2e38_1ffc)
extern unsigned far F_2e38_2798[][8];  /* character icons */
extern unsigned D_e3fc;
extern unsigned D_e400[8];
void far pic_draw_0(int, char *);
unsigned far pic_load(char *);
#define G F_2e38_239c
extern int D_866a, cur_hour;
extern int D_286a;                  /* map view: left column */
extern int D_286c;                  /* map view: party row in view */
extern int D_284e[];
extern int D_282a[];
extern signed char far F_290e_38db[][16], far F_290e_391b[][16], far F_290e_395b[][16];
void far sub_root_047c_04ea(int, int);
void far gfx_draw_sprite(int *, int, int, unsigned);
void far gfx_copy_rect(int *, int, int, int, int, int *, int, int);
extern int party_size;
extern unsigned char far F_2e38_239c[][19];   /* terrain grid around the party */
struct micon { unsigned y, x, icon, frame; int flag; };    /* map icon list, far 2e38:2838 */
extern struct micon far F_2e38_2838[];
extern unsigned char far *D_866c;   /* map rows buffer (0xd2 bytes per row) */
extern long D_2862;
unsigned far alloc_buffer(unsigned long, char *);
void far sub_root_06a1_3108(void far *, void far *, unsigned);
void far sub_s030_1462_1dd4(struct _iobuf *, int, int);
void far sub_s030_1767_00d2(int *);
void far advance_hours(int);
int far event_at(int, int);
int far event_value_at(int, int);
#define D_e48c_b ((unsigned char *)cur_palette)

/* overlay entry: place the party, then run the overland map */
int far overland_map(void)
{
    int x, y;
    if (cur_loc < 92 && cur_loc != -1) {
        x = party_x = city_rec->exit_x;
        y = party_y = city_rec->exit_y;
    } else if (cur_loc != -1) {
        x = party_x = data_file.loc[cur_loc]->x;
        y = party_y = data_file.loc[cur_loc]->y;
    } else {
        x = party_x;
        y = party_y;
    }
    if (event_started(0x47)) {
        return next_card = 0xf1;
    }
    if (!event_started(0x63)) {
        copyprot_run_vec();
        event_here(0x63, 0x3e80);
    }
    return overland_open(x, y);
}

/* open DARKLAND.MAP, centre the view on (x, y), run the map loop */
int far overland_open(int x, int y)
{
    int row;
    int result;


    mouse_hide();
    D_9c3d = vec_18f1(5);
    if (rand_mod(2))
        vec_18f1(0x14);
    else
        vec_18f1(0x13);
    D_e3f2 = party_get(5);
    switch (cur_month) {
    case 0: case 1: case 2:
        D_2860 = 0xf;
        break;
    case 3:
        D_2860 = 6;
        break;
    case 8: case 9:
        D_2860 = 6;
        break;
    case 10: case 11:
        D_2860 = 0xf;
        break;
    default:
        D_2860 = 0x68;
    }
    D_a891 = 3;
    D_8672 = 0;
    D_e488 = 0;
    if ((D_e3f4 = fopen("darkland.map", "rb")) == 0)
        printf("Error %d: Could not open map file\n", D_775a);
    sub_s030_1462_1c4e(D_e3f4);

    /* vertical: top row and party row within the view */
    row = y / 100 - 1;
    if (row < 0)
        row = 0;
    D_286e = row * 100;
    D_286c = y - D_286e - 16;
    D_2868 = 0x40;
    while (D_286c > 0x53) {
        D_286e += 24;
        D_286c = y - D_286e - 16;
    }
    if (D_e3f8 - D_286e - 0x30 < D_286c) {
        D_2868 = (D_286e - D_e3f8 + D_286c + 0x40) * 4;
        D_286e = D_e3f8 - 0x83;
        D_286c = 0x53;
    }
    if (D_286c < 0)
        D_286c = 0;

    /* horizontal */
    D_2866 = 0x82;
    D_286a = x - 8;
    if (D_286a > D_e3f6 - 0x11) {
        D_2866 = (D_286a - D_e3f6) * 16 + 0x192;
        D_286a = D_e3f6 - 0x11;
    } else if (D_286a <= 0) {
        D_2866 = 0x82 - D_286a;
        D_286a = 0;
    }

    sub_s030_1462_1c76(D_e3f4);
    sub_root_047c_0568();
    sub_s030_1462_1f30();
    mouse_set_cursor(5, 0, cursor_default);
    sub_s030_1462_20d2(D_286a, D_286c);
    sub_root_047c_056f();
    result = overland_loop();
    fcloseall();
    mem_free(D_2870, "Map");
    if (F_2e38_1ffc[0])
        mem_free(F_2e38_1ffc[0], "S");
    if (D_e416)
        _dos_freemem(D_e416);
    D_2870 = 0;
    F_2e38_1ffc[0] = 0;
    D_e416 = 0;
    return result;
}

/* the overland main loop: returns the next card/menu id */
int far overland_loop(void)
{
    register int key;
    int fdf;
    struct _iobuf *fpk;
    int delayc;
    long posa;
    int sel;
    unsigned nreadk;
    int colh, rowo;
    char bufk[40];
    int unusede[5];
    int mxe, mye;

    delayc = 30;
    cur_loc = -1;
    mxe = D_2866;
    mye = D_2868;
    for (;;) {
        if (mouse_buttons & 1) {
            if (mouse_x > 0x3c) {
                mye = mouse_y - 0x15;
                mxe = mouse_x - 0x41;
            } else {
                /* click on a party portrait */
                mxe = 0;
                sel = mouse_y / 40;
                if (party[sel].present) {
                    mem_free(F_2e38_1ffc[0], "S");
                    F_2e38_1ffc[0] = 0;
                    member_sheet_vec(sel);
                    sub_root_047c_0568();
                    sub_s030_1462_1f30();
                    sub_s030_1462_2dd6();
                    set_palette(cur_palette);
                    party_panel_frames(0x100);
                    party_panel_rows(0x100);
                    sub_s030_1462_20d2(D_286a, D_286c);
                    sub_root_047c_056f();
                }
            }
        }
        if ((mouse_buttons & 2) && ((*BIOS_KBFLAGS & 2) || (*BIOS_KBFLAGS & 1))) {
            /* right button with shift: show map coordinates */
            screen_win_colour = 0xf;
            rowo = (D_2868 + 11) / 4;
            if ((rowo + D_286e + D_286c - 1) % 2)
                colh = (D_2866 + 9) / 16;
            else
                colh = (D_2866 + 17) / 16;
            rowo += D_286e + D_286c;
            colh += D_286a;
            sprintf(bufk, "character X:%02d Y:%02d", colh, rowo);
            gfx_text(screen_win, 0x58, 0x1f, bufk);
        }
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            pic_draw_pal(1, "pics\\mapicons.pic", cur_palette);
            sub_root_047c_0568();
            sub_s030_1462_2dd6();
            set_palette(cur_palette);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s030_1462_20d2(D_286a, D_286c);
            sub_root_047c_056f();
        }

        /* direction towards the mouse target, as a keypad scan code */
        if (mxe == 0)
            key = 0x4c35;
        else if (D_2866 > mxe && D_2868 > mye)
            key = 0x4700;
        else if (D_2866 > mxe && D_2868 < mye)
            key = 0x4f00;
        else if (D_2866 < mxe && D_2868 > mye)
            key = 0x4900;
        else if (D_2866 < mxe && D_2868 < mye)
            key = 0x5100;
        else if (D_2866 > mxe)
            key = 0x4b00;
        else if (D_2866 < mxe)
            key = 0x4d00;
        else if (D_2868 > mye)
            key = 0x4800;
        else if (D_2868 < mye)
            key = 0x5000;
        else {
            key = 0x4c35;
            mxe = 0;
        }

        /* keyboard / joystick override the mouse */
        if (key_pressed() || D_0880 || D_0881 || D_0882) {
            mxe = 0;
            key = get_key();
            if (key_pressed())
                key = get_key();
            else if ((D_e3fa < -60 && D_e3fb < -60) || (D_0880 < 0 && D_0881 < 0))
                key = 0x4700;
            else if ((D_e3fa < -60 && D_e3fb > 60) || (D_0880 < 0 && D_0881 > 0))
                key = 0x4f00;
            else if ((D_e3fa > 60 && D_e3fb < -60) || (D_0880 > 0 && D_0881 < 0))
                key = 0x4900;
            else if ((D_e3fa > 60 && D_e3fb > 60) || (D_0880 > 0 && D_0881 > 0))
                key = 0x5100;
            else if (D_e3fa < -60 || D_0880 < 0)
                key = 0x4b00;
            else if (D_e3fa > 60 || D_0880 > 0)
                key = 0x4d00;
            else if (D_e3fb < -60 || D_0881 < 0)
                key = 0x4800;
            else if (D_e3fb > 60 || D_0881 > 0)
                key = 0x5000;
            else if (D_0882)
                key = 0x1c0d;
            D_0880 = D_0881 = D_0882 = 0;
        }

        sel = sub_s030_1462_0c44(key);
        if (sel >= 0) {
            if (sel < 92) {
                /* arrived at a city: load its CTY and DSC records */
                sub_root_06a1_2c9e("darkland.cty", 0, &fdf);
                fpk = fdopen(fdf, "r");
                posa = (long)sel * 0x26e;
                fseek(fpk, posa + 1, 0);
                _dos_read(fdf, city_rec, 0x26e, &nreadk);
                fclose(fpk);
                fpk = fopen("darkland.dsc", "r");
                fgetc(fpk);
                fseek(fpk, (long)(sel * 0x50 + 1), 0);
                fread(city_desc, 0x50, 1, fpk);
                fclose(fpk);
            }
            if (sel == 1000) {
                /* a day passed in the wilderness: random events */
                if (event_started(0x5f))
                    return rand_mod(2) ? 0x16a : 0x169;
                if (cur_loc != -1 && ev8_at(4, cur_loc)) {
                    if (loc_fact_value_any(4, 1) % 2 == 0)
                        return 0x97;
                    switch (vec_1fe5()) {
                    case 0:
                        if (loc_fact_value_any(4, 5) == 0)
                            return 0x97;
                        return 0xa1;
                    case 1:
                        return 0x9e;
                    case 2:
                        return 0x9b;
                    case 3:
                        return 0x9c;
                    }
                } else if (!is_daylight() && !rand_mod(3) && ev8_exists(7)) {
                    event_update(-2, -1, 0x1c, 5, 1, 7, 1);
                    sel = loc_fact_value_any(7, 5);
                    switch (sel) {
                    case 2:
                        return 0xa8;
                    case 3:
                        return loc_fact_any(7, 1) ? 0xa9 : 0xa8;
                    case 6: case 9:
                        return loc_fact_any(7, 2) ? 0x41 : 0xa9;
                    case 1:
                        return 0x41;
                    default:
                        if (sel % 3 == 0 && !loc_fact_any(7, 2))
                            return 0xa9;
                        return 0x41;
                    }
                } else if (!is_daylight() && party_lore_level() > 4 && loc_fact_any(1, 2)) {
                    return 0x126;
                } else {
                    switch (D_d843) {
                    case 1: case 2: case 3:
                        break;
                    case 4:
                        switch (rand_mod(9)) {
                        case 0: case 1:
                            return 0x112;
                        case 2: case 3: case 4:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        case 5: case 6: return 0x116;
                        default: return 0x11b;
                        }
                        break;
                    case 5:
                        switch (rand_mod(0x11)) {
                        case 0: case 1:
                            return 0x112;
                        case 2: case 3: case 4:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        case 5: case 6:
                            return 0x116;
                        case 7: case 8:
                            return 0xa0;
                        case 9: case 10:
                            return 0x102;
                        case 11: case 12:
                            return 0x103;
                        case 13:
                            return 0x111;
                        default: return 0x122;
                        }
                        break;
                    case 6: case 7:
                        switch (rand_mod(0x11)) {
                        case 0: case 1: return 0x112;
                        case 2: case 3:
                            return 0xa5;
                        case 4: case 5:
                            return 0x107;
                        case 6: case 7: return 0xa0;
                        case 8: case 9: return 0x102;
                        case 10: case 11: return 0x103;
                        case 12: return 0x111;
                        case 13:
                            return 0x118;
                        default: return 0x122;
                        }
                        break;
                    case 8: case 9:
                        switch (rand_mod(0x1a)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0x102;
                        case 4: case 5: return 0x103;
                        case 6: case 7: case 8:
                            return 0x106;
                        case 9: case 10: return 0x107;
                        case 11: case 12: case 13: return 0x7f;
                        case 14: case 15: case 16: case 17: return 0x104;
                        case 18: case 19: return 0x105;
                        case 20: case 21: case 22: return 0x108;
                        case 23: case 24:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        default:
                            return 0x24;
                        }
                        break;
                    case 10: case 11:
                        switch (rand_mod(0x11)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: case 5: return 0x102;
                        case 6: case 7: return 0x103;
                        case 8: case 9: case 10: return 0x106;
                        case 11: case 12: return 0x107;
                        case 13: return 0x112;
                        case 14: case 15:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        default: return 0x24;
                        }
                        break;
                    case 12: case 13: case 14: case 15:
                        switch (rand_mod(0x10)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: case 5: return 0x102;
                        case 6: case 7: return 0x103;
                        case 8: return 0x113;
                        case 9: return 0x112;
                        case 10: return 0x122;
                        case 11: case 12:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        case 13: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 16: case 17:
                        switch (rand_mod(0x10)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: return 0x111;
                        case 5: return 0x113;
                        case 6: case 7: return 0x112;
                        case 8: case 9: return 0x122;
                        case 10: case 11: case 12:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        case 13: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 18: case 19: case 20: case 21:
                        switch (rand_mod(0xf)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0x102;
                        case 4: case 5: return 0x103;
                        case 6: return 0x111;
                        case 7: case 8: return 0x112;
                        case 9: case 10: case 11:
                            if (cur_month < 3 || cur_month > 9) return 0x117;
                            break;
                        case 12: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 22: case 23:
                        switch (rand_mod(9)) {
                        case 0: case 1: case 2: return 0xf0;
                        case 3: return 0x111;
                        default:
                            if (cur_month < 4 || cur_month > 8) return 0x117;
                            break;
                        }
                        break;
                    case 24:
                        switch (rand_mod(0x1d)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa6;
                        case 4: case 5: return 0xa7;
                        case 6: return 0x3d;
                        case 7: return 0xc3;
                        case 8: case 9: return 0x102;
                        case 10: case 11: return 0x103;
                        case 12: case 13: case 14: return 0x106;
                        case 15: case 16: return 0x107;
                        case 17: case 18: return 0x7f;
                        case 19: case 20: case 21: case 22: return 0x104;
                        case 23: case 24: return 0x105;
                        case 25: return 0x163;
                        case 26: return 0x108;
                        default: return 0x24;
                        }
                        break;
                    }
                }
            } else if (sel == 2000) {
                return data_file.loc[cur_loc]->unk0e;
            } else if (sel != 9999) {
                return data_file.loc[cur_loc]->menu;
            } else {
                mxe = 0;
            }
        }
        sub_s030_1767_0046((long)delayc);
    }
}

int far sub_s030_1462_0c44(register int key)
{
    int oel, kf, cold, rowk, oad, occ, oxo, oyo;

    rowk = (D_2868 + 11) / 4;
    cold = ((rowk + D_286e + D_286c - 1) % 2 ? D_2866 + 16 : D_2866 + 24) / 16;
    party_x = D_286a + cold - 1;
    party_y = rowk + D_286e + D_286c - 1;
    oxo = D_2866;
    oyo = D_2868;
    switch (key) {
    case 0x4b00:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        D_8676 = 7;
        break;
    case 0x4d00:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        D_8676 = 3;
        break;
    case 0x4800:
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 1;
        break;
    case 0x5000:
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 5;
        break;
    case 0x4700:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 8;
        break;
    case 0x4900:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 2;
        break;
    case 0x5100:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 4;
        break;
    case 0x4f00:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 6;
        break;
    case 0x4c35:
        D_8676 = 0;
        break;
    case 0x3b00: case 0x3c00: case 0x3d00: case 0x3e00: case 0x3f00:
        kf = (key >> 8) - 0x3b;
        member_sheet_vec(kf);
        pic_draw_pal(1, "pics\\mapicons.pic", cur_palette);
        sub_root_047c_0568();
    redraw:
        sub_s030_1462_2dd6();
        set_palette(cur_palette);
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        sub_s030_1462_20d2(D_286a, D_286c);
        sub_root_047c_056f();
        break;
    case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:
        kf = (key >> 8) - 0x5e;
        if (kf < party_size && party[kf].present == 1) {
            leader = kf;
            party_panel_rows(0x100);
        }
        break;
    case 0x2e43: case 0x2e63: case 0x4200:
        free_buffer(F_2e38_1ffc[0], "S");
        F_2e38_1ffc[0] = 0;
        vec_1ff9(sub_s030_1462_25be());
        if (event_started(0x5f))
            return 1000;
        sub_root_047c_0568();
        sub_s030_1462_1f30();
        goto redraw;
    case 0x1e41: case 0x1e61: case 0x4100:
        D_2910 = 0;
        D_a891 = 1;
        goto test;
        do {
            if (mouse_buttons & 3)
                break;
            delay_ticks(75L);
            kf = sub_s030_1462_1300(D_2866, D_2868);
            D_a891 = 3;
            if (kf != -1)
                return kf;
            sub_s030_1462_16c6(D_2866, D_2868, D_2910++, 9);
            if (D_2910 >= 8)
                D_2910 = 0;
            delay_ticks(75L);
        test:;
        } while (!key_pressed());
        D_a891 = 3;
        break;
    case 0x1000:
        free_buffer(D_2870, "M");
        free_buffer(F_2e38_1ffc[0], "S");
        if (D_e416)
            _dos_freemem(D_e416);
        D_2870 = 0;
        F_2e38_1ffc[0] = 0;
        D_e416 = 0;
        quit_to_dos();
        break;
    }
    if (D_290e > 1)
        D_290e = 0;
    if (D_8676 != 0) {
        kf = sub_s030_1462_1300(D_2866, D_2868);
        if (kf >= 0) {
            D_2866 = oxo;
            D_2868 = oyo;
            D_8676 = 0;
            return kf;
        }
    }
    occ = D_286c;
    oad = D_286a;
    oel = D_286e;
    if (D_2868 < 20 || D_2868 > 170) {
        oyo = D_2868 - 0x5c;
        if (D_2868 < 20)
            D_286c -= 23;
        else
            D_286c += 23;
        if (D_e3f8 - D_286e - 0x30 <= D_286c) {
            D_286e = D_e3f8 - 0x83;
            D_286c = D_e3f8 - D_286e - 0x30;
            D_2868 += (occ - D_286c) * 5;
        } else if (D_286c > 0x53) {
            if (D_286e + 0xc4 >= D_e3f8)
                D_286e = D_e3f8 - 0x83;
            else
                D_286e += 0x41;
            if (oel != D_286e) {
                sub_s030_1462_1dd4(D_e3f4, D_286e, oel);
                D_286c += oel - D_286e;
                D_2868 += D_2868 < 20 ? 92 : -92;
            }
            if (D_286c > 0x53)
                D_286c = 0x53;
        } else if (D_286c <= 1) {
            if (D_286e <= 0x41)
                D_286e = 0;
            else
                D_286e -= 0x41;
            if (oel != D_286e) {
                sub_s030_1462_1dd4(D_e3f4, D_286e, oel);
                D_286c += oel - D_286e;
                D_2868 += D_2868 < 20 ? 92 : -92;
            }
            if (D_286c <= 1) {
                D_286c = 1;
                D_2868 += (occ - 1) * 4;
            }
        } else {
            D_2868 += D_2868 < 20 ? 92 : -92;
        }
    }
    if (D_2866 < 20 || D_2866 > 0xe2) {
        oxo = D_2866 - 0x80;
        if (D_2866 < 20)
            D_286a -= 8;
        else
            D_286a += 8;
        if (D_e3f6 - 0x11 < D_286a) {
            D_286a = D_e3f6 - 0x11;
            D_2866 += (oad - D_286a) * 16;
        } else if (D_286a <= 0) {
            D_286a = 0;
            D_2866 += oad * 16;
        } else if (D_2866 < 20)
            D_2866 += 0x80;
        else
            D_2866 -= 0x80;
    }
    if (occ != D_286c || D_286a != oad)
        sub_s030_1462_20d2(D_286a, D_286c);
    sub_s030_1462_16c6(D_2866, D_2868, D_2910, D_8676);
    if (key)
        D_2910++;
    if (D_2910 >= 8)
        D_2910 = 0;
    return -1;
}

int D_290e = 0;
int D_2910 = 0;

int far sub_s030_1462_1300(int x, int y)
{
    int row, col, t, tt, r, i, best, found, dx, dy, d, m, w;
    unsigned char c;

    if (event_started(0x5d)) {
        advance_hours(1);
        return cur_loc = nearest_city_impl(party_x, party_y);
    }
    row = (y + 11) / 4;
    col = ((row + D_286e + D_286c - 1) % 2 ? x + 16 : x + 24) / 16;
    c = G[row][col];
    t = c >= 50 ? 28 : c;
    if ((t == 1 || t == 2) && !event_started(0x4b) && D_d843 != 1 && D_d843 != 2)
        return 9999;
    D_d843 = t;
    D_8672 += sub_s030_1462_2ac8(t);
    if (D_8672 >= 60) {
        advance_hours(1);
        gfx_draw_pic(screen_win, 289, 0, D_e400[cur_hour / 3]);
        D_8672 -= 60;
        m = D_2860;
        sub_s030_1462_2dd6();
        if (m != D_2860)
            sub_s030_1462_20d2(D_286a, D_286c);
    }
    tt = t;
    if (t >= 25 && t != 27 && t != 25) {
        r = t == 29 ? 4 : 1;
        for (i = 0; i < 414; i++) {
            if (D_286a - r + col - 1 <= data_file.loc[i]->x
                    && D_286a + col + r + 1 > data_file.loc[i]->x
                    && row - r + D_286e + D_286c - 1 <= data_file.loc[i]->y
                    && row + D_286e + D_286c + r + 1 > data_file.loc[i]->y
                    && data_file.loc[i]->menu != 0x62)
                return cur_loc = i;
        }
    }
    best = 100;
    found = -1;
    for (i = 0; i < 414; i++) {
        if ((-1 - data_file.loc[i]->unk08) / 2 + D_286a + col > data_file.loc[i]->x)
            continue;
        if ((data_file.loc[i]->unk08 + 1) / 2 + D_286a + col <= data_file.loc[i]->x)
            continue;
        if (row - data_file.loc[i]->unk08 + D_286e + D_286c - 1 > data_file.loc[i]->y)
            continue;
        if (data_file.loc[i]->unk08 + row + D_286e + D_286c + 1 <= data_file.loc[i]->y)
            continue;
        dx = data_file.loc[i]->x - col - D_286a;
        if (dx < 0)
            dx = -dx;
        dy = data_file.loc[i]->y - D_286e - D_286c - row;
        if (dy < 0)
            dy = -dy;
        d = dy <= dx ? dy / 2 + dx : dx / 2 + dy;
        if (d < best) {
            if (found != i && (m = event_value_at(0x2b, i)) != 0) {
                event_remove(0x2b, -1, i, -1, -1);
                map_message(m);
            }
            found = i;
            best = d;
        }
    }
    if (found != -1 && data_file.loc[found]->icon == 15)
        vec_18f1(0x18);
    t = tt;
    if (t == 1 || t == 2)
        return -1;
    if (D_e488 < 10 && rand_mod(500) <= 9)
        D_e488++;
    w = sub_s030_1462_2d44(t, found);
    if ((D_e488 + 1) * w < rand_mod(1000))
        return -1;
    party_x = D_286a + col - 1;
    party_y = row + D_286e + D_286c - 1;
    cur_loc = found;
    if (found != -1 && data_file.loc[found]->unk0e != 0x62 && rand_mod(100) <= 60)
        return 2000;
    return 1000;
}

void far sub_s030_1462_16c6(int x, int y, int frame, int dir)
{
    register int cc, r;
    register int xa;
    int tl, dyd, oddk, pyj, maskh, fxg, fyg, coli, row, pxa, cg;

    gfx_fill_rect(D_282a, 0, pyj = 0, 0x37, 0x32, D_2860);
    fyg = (y + 11) % 4;
    row = (y + 11) / 4;
    xa = (row + D_286e + D_286c - 1) % 2 ? x + 16 : x + 24;
    coli = xa / 16;
    fxg = xa % 16;
    tl = G[row][coli] > 50 ? 28 : G[row][coli];
    D_d843 = tl;
    switch (tl) {
    case 16: case 17: case 18: case 19:
        dyd = F_290e_391b[fyg][fxg];
        break;
    case 20: case 21: case 22: case 23:
        dyd = F_290e_38db[fyg][fxg];
        break;
    case 3: case 4: case 25:
        dyd = F_290e_395b[fyg][fxg];
        break;
    default:
        dyd = 0;
        break;
    }
    for (r = row - 6; r <= row + 3; r++) {
        oddk = (D_286e + D_286c + r - 1) % 2;
        if (oddk)
            pxa = 8;
        else
            pxa = 0;
        for (cc = coli - 1; cc <= coli + 2; cc++) {
            maskh = 0;
            cg = G[r][cc];
            if (cg == 1 || cg == 2 || cg == 3 || cg == 27 || cg == 25) {
                if (G[r - 1][cc - 1 + oddk] == 1 || G[r - 1][cc - 1 + oddk] == 2 || G[r - 1][cc - 1 + oddk] == 3 || G[r - 1][cc - 1 + oddk] == 25 || G[r - 1][cc - 1 + oddk] == 27)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 1 || G[r - 1][cc + oddk] == 2 || G[r - 1][cc + oddk] == 3 || G[r - 1][cc + oddk] == 25 || G[r - 1][cc + oddk] == 27)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 1 || G[r + 1][cc - 1 + oddk] == 2 || G[r + 1][cc - 1 + oddk] == 3 || G[r + 1][cc - 1 + oddk] == 25 || G[r + 1][cc - 1 + oddk] == 27)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 1 || G[r + 1][cc + oddk] == 2 || G[r + 1][cc + oddk] == 3 || G[r + 1][cc + oddk] == 25 || G[r + 1][cc + oddk] == 27)
                    maskh |= 8;
                gfx_draw_sprite(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg == 4 || cg == 5 || cg == 6 || cg == 7) {
                if (G[r - 1][cc - 1 + oddk] == 4 || G[r - 1][cc - 1 + oddk] == 5 || G[r - 1][cc - 1 + oddk] == 6 || G[r - 1][cc - 1 + oddk] == 7)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 4 || G[r - 1][cc + oddk] == 5 || G[r - 1][cc + oddk] == 6 || G[r - 1][cc + oddk] == 7)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 4 || G[r + 1][cc - 1 + oddk] == 5 || G[r + 1][cc - 1 + oddk] == 6 || G[r + 1][cc - 1 + oddk] == 7)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 4 || G[r + 1][cc + oddk] == 5 || G[r + 1][cc + oddk] == 6 || G[r + 1][cc + oddk] == 7)
                    maskh |= 8;
                gfx_draw_sprite(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg == 24) {
                if (G[r - 1][cc - 1 + oddk] == 24 || G[r - 1][cc - 1 + oddk] >= 25 || G[r - 1][cc - 1 + oddk] == 27)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 24 || G[r - 1][cc + oddk] >= 25 || G[r - 1][cc + oddk] == 27)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 24 || G[r + 1][cc - 1 + oddk] >= 25 || G[r + 1][cc - 1 + oddk] == 27)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 24 || G[r + 1][cc + oddk] >= 25 || G[r + 1][cc + oddk] == 27)
                    maskh |= 8;
                gfx_draw_sprite(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg >= 50) {
                gfx_draw_sprite(D_282a, pxa, pyj, F_2e38_1ffc[cg + 382]);
            } else if (cg != 0) {
                if (G[r - 1][cc - 1 + oddk] == cg)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == cg)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == cg)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == cg)
                    maskh |= 8;
                gfx_draw_sprite(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            }
            pxa += 16;
        }
        if ((row == r && tl <= 9) || (row - r == 1 && fyg == 0 && tl >= 20)
                || (row == r && tl >= 15 && fyg != 0) || (row - r == 1 && tl >= 9 && tl <= 19))
            gfx_draw_sprite(D_282a, (row + D_286e + D_286c - 1) % 2 * 8 + fxg + 10, fyg - dyd + 18,
                               F_2e38_2798[dir][frame]);
        pyj += 4;
    }
    mouse_hide();
    gfx_copy_rect(D_282a, ((row + D_286e + D_286c - 1) % 2 + 1) * 8 + fxg, fyg - dyd + 14, 17, 21,
                       screen_win, x + 52, y - dyd + 5);
    mouse_show();
}

void far sub_s030_1462_1c4e(struct _iobuf *f)
{
    fread(&D_e3f6, 4, 1, f);
    sub_s030_1767_00d2(&D_e3f6);
    sub_s030_1767_00d2(&D_e3f8);
}

void far sub_s030_1462_1c76(struct _iobuf *f)
{
    int k, n;

    D_2870 = alloc_buffer(30000L, "map");
    D_866c = (unsigned char far *)((unsigned long)D_2870 << 16);
    sub_s030_1462_1dd4(f, D_286e, -1);
    for (k = 92, n = 0; k < 414; k++) {
        if (data_file.loc[k]->menu == 0x62 || data_file.loc[k]->icon == 15)
            continue;
        F_2e38_2838[n].x = data_file.loc[k]->x;
        F_2e38_2838[n].y = data_file.loc[k]->y;
        switch (data_file.loc[k]->icon) {
        default:
            F_2e38_2838[n].icon = 0x1c;
            break;
        case 3:
            F_2e38_2838[n].icon = 0x33;
            break;
        case 5: case 15: case 19: case 24:
            F_2e38_2838[n].icon = 0x38;
            break;
        case 6:
            F_2e38_2838[n].icon = 0x37;
            break;
        case 8: case 10:
            F_2e38_2838[n].icon = 0x1d;
            F_2e38_2838[n].frame = k % 16;
            break;
        case 9:
            F_2e38_2838[n].icon = 0x3e;
            break;
        case 13:
            F_2e38_2838[n].icon = 0x39;
            break;
        case 16:
            F_2e38_2838[n].icon = 0x36;
            break;
        case 17:
            F_2e38_2838[n].icon = 0x3b;
            break;
        case 18:
            F_2e38_2838[n].icon = 0x35;
            break;
        case 20:
            F_2e38_2838[n].icon = 0x3c;
            break;
        case 21:
            F_2e38_2838[n].icon = 0x3d;
            break;
        case 22:
            F_2e38_2838[n].icon = 0x3f;
            break;
        case 23:
            F_2e38_2838[n].icon = 0x40;
            break;
        case 26:
            F_2e38_2838[n].icon = 0x3a;
            break;
        case 4: F_2e38_2838[n].icon = 0x39; break;
        }
        F_2e38_2838[n].flag = 0;
        n++;
    }
    F_2e38_2838[n].flag = -1;
}

void far sub_s030_1462_1dd4(struct _iobuf *f, int top, int old)
{
    unsigned off;
    unsigned first, rows;
    unsigned r;
    long pos;
    unsigned x, w;
    unsigned d;

    if (old == top)
        return;
    if (old == -1) {
        first = 0;
        rows = 0x83;
        off = top << 2;
    } else if (old < top) {
        d = top - old;
        sub_root_06a1_3108(D_866c, D_866c + d * 0xd2, (0x83 - d) * 0xd2);
        first = d;
        rows = 0x83 - d;
        off = (d + top) << 2;
    } else {
        old -= top;
        sub_root_06a1_3108(D_866c + old * 0xd2, D_866c, old * 0xd2);
        first = 0;
        rows = old;
        off = top << 2;
    }
    fseek(f, (long)(off + 4), 0);
    fread(&pos, 4, 1, f);
    fseek(f, pos, 0);
    for (r = first; r < first + rows; r++) {
        x = 0;
        w = 0;
        while (w < D_e3f6) {
            D_866c[r * 0xd2 + x] = fgetc(f);
            w += D_866c[r * 0xd2 + x] >> 5;
            x++;
            if ((long)x > D_2862)
                D_2862 = x;
        }
    }
}

void far sub_s030_1462_1f30(void)
{
    int r, c, k;

    sub_root_047c_0545();
    pic_draw_pal(1, "pics\\mapicons.pic", cur_palette);
    for (r = 1; r < 16; r++)
        for (c = 0; c < 16; c++)
            ICONS[r - 1][c] = gfx_save_rect(1, c * 16, r * 12, 16, 12);
    pic_draw_0(1, "pics\\mapicon2.pic");
    for (k = r, r = 0; r < 16; r++)
        for (c = 0; c < 16; c++)
            ICONS[k - 1 + r][c] = gfx_save_rect(1, c * 16, r * 12, 16, 12);
    pic_draw_0(1, "pics\\charicon.pic");
    for (r = 0; r < 10; r++)
        for (c = 0; c < 8; c++)
            F_2e38_2798[r][c] = gfx_save_rect(1, c * 14 + 1, r * 14 + 1, 13, 13);
    D_e3fc = pic_load("pics\\mapheada.pic");
    D_e400[0] = pic_load("pics\\mapbrs00.pic");
    D_e400[1] = pic_load("pics\\mapbrs03.pic");
    D_e400[2] = pic_load("pics\\mapbrs06.pic");
    D_e400[3] = pic_load("pics\\mapbrs09.pic");
    D_e400[4] = pic_load("pics\\mapbrs12.pic");
    D_e400[5] = pic_load("pics\\mapbrs15.pic");
    D_e400[6] = pic_load("pics\\mapbrs18.pic");
    D_e400[7] = pic_load("pics\\mapbrs21.pic");
    sub_root_047c_04f8();
    sub_root_047c_0568();
    sub_s030_1462_2dd6();
    set_palette(cur_palette);
    party_panel_frames(0x100);
    party_panel_rows(0x100);
}

void far sub_s030_1462_20d2(int col, int row)
{
    register int x, y;
    int pyg, kh, outl, maskf, te, oddc, tope, pxo;
    unsigned char cntl, valc;
    int gam[6], gbb[1];

    if (col == 0)
        col = 1;
    if (row == 0)
        row = 1;
    pyg = 0;
    D_866a = 0;
    sub_root_047c_04ea(1, D_2860);
    for (y = 0; y < 50; y++) {
        x = 0;
        kh = 0;
        outl = 0;
        while (col + 18 > kh) {
            te = D_866c[(row + y - 1) * 0xd2 + x];
            valc = te & 0x1f;
            cntl = te >> 5;
            goto test;
            do {
                if (outl >= 19)
                    break;
                if (kh++ >= col - 1) {
                    G[y][outl] = valc;
                    outl++;
                }
            test:;
            } while (cntl--);
            x++;
        }
    }
    for (y = 0, tope = row + D_286e - 1; F_2e38_2838[y].flag != -1; y++) {
        if (F_2e38_2838[y].y >= tope && F_2e38_2838[y].x >= col - 1 && F_2e38_2838[y].x <= col + 18
                && F_2e38_2838[y].y <= tope + 50)
            G[F_2e38_2838[y].y - tope][F_2e38_2838[y].x - col + 1] = F_2e38_2838[y].icon;
    }
    for (y = 1; y <= 48; y++) {
        oddc = (row + D_286e + y - 1) % 2;
        if (oddc)
            pxo = 8;
        else
            pxo = 0;
        for (x = 1; x <= 17; x++) {
            maskf = 0;
            te = G[y][x];
            if (te == 1 || te == 2 || te == 3 || te == 27 || te == 25) {
                if (G[y - 1][x - 1 + oddc] == 1 || G[y - 1][x - 1 + oddc] == 2 || G[y - 1][x - 1 + oddc] == 3 || G[y - 1][x - 1 + oddc] == 25 || G[y - 1][x - 1 + oddc] == 27)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 1 || G[y - 1][x + oddc] == 2 || G[y - 1][x + oddc] == 3 || G[y - 1][x + oddc] == 25 || G[y - 1][x + oddc] == 27)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 1 || G[y + 1][x - 1 + oddc] == 2 || G[y + 1][x - 1 + oddc] == 3 || G[y + 1][x - 1 + oddc] == 25 || G[y + 1][x - 1 + oddc] == 27)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 1 || G[y + 1][x + oddc] == 2 || G[y + 1][x + oddc] == 3 || G[y + 1][x + oddc] == 25 || G[y + 1][x + oddc] == 27)
                    maskf |= 8;
                gfx_draw_sprite(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te == 4 || te == 5 || te == 6 || te == 7) {
                if (G[y - 1][x - 1 + oddc] == 4 || G[y - 1][x - 1 + oddc] == 5 || G[y - 1][x - 1 + oddc] == 6 || G[y - 1][x - 1 + oddc] == 7)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 4 || G[y - 1][x + oddc] == 5 || G[y - 1][x + oddc] == 6 || G[y - 1][x + oddc] == 7)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 4 || G[y + 1][x - 1 + oddc] == 5 || G[y + 1][x - 1 + oddc] == 6 || G[y + 1][x - 1 + oddc] == 7)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 4 || G[y + 1][x + oddc] == 5 || G[y + 1][x + oddc] == 6 || G[y + 1][x + oddc] == 7)
                    maskf |= 8;
                gfx_draw_sprite(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te == 24) {
                if (G[y - 1][x - 1 + oddc] == 24 || G[y - 1][x - 1 + oddc] >= 25 || G[y - 1][x - 1 + oddc] == 27)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 24 || G[y - 1][x + oddc] >= 25 || G[y - 1][x + oddc] == 27)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 24 || G[y + 1][x - 1 + oddc] >= 25 || G[y + 1][x - 1 + oddc] == 27)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 24 || G[y + 1][x + oddc] >= 25 || G[y + 1][x + oddc] == 27)
                    maskf |= 8;
                gfx_draw_sprite(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te >= 50) {
                gfx_draw_sprite(D_284e, pxo, pyg, F_2e38_1ffc[te + 382]);
            } else if (te != 0) {
                if (G[y - 1][x - 1 + oddc] == te)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == te)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == te)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == te)
                    maskf |= 8;
                gfx_draw_sprite(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            }
            pxo += 16;
        }
        pyg += 4;
    }
    mouse_hide();
    gfx_draw_pic(screen_win, 60, 0, D_e3fc);
    gfx_draw_pic(screen_win, 289, 0, D_e400[cur_hour / 3]);
    gfx_copy_rect(D_284e, 8, 8, 260, 184, screen_win, 60, 16);
    mouse_show();
}

int far sub_s030_1462_25be(void)
{
    int res, row, col, speed, i, j;

    res = 0;
    row = (D_2868 + 11) / 4;
    col = ((row + D_286e + D_286c - 1) % 2 ? D_2866 + 16 : D_2866 + 24) / 16;
    speed = party_size * 3;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (F_2e38_239c[row + j][col + i] == 0x1d) {
                i = i > 5 ? i - 5 : abs(i - 5);
                j = j > 5 ? j - 5 : abs(j - 5);
                if (j > i)
                    j = i;
                res = j;
                i = 99;
                break;
            }
        }
    }
    if (res)
        speed += abs(res - 6);
    if (F_2e38_239c[row][col] == 5 || F_2e38_239c[row][col] == 4 || F_2e38_239c[row][col] == 15 || F_2e38_239c[row][col] == 19)
        speed--;
    else if (F_2e38_239c[row][col] == 16 || F_2e38_239c[row][col] == 20)
        speed -= 2;
    else if (F_2e38_239c[row][col] == 17 || F_2e38_239c[row][col] == 21)
        speed -= 3;
    else if (F_2e38_239c[row][col] == 22)
        speed -= 5;
    else if (F_2e38_239c[row][col] == 23)
        speed -= 6;
    if (F_2e38_239c[row][col] == 24)
        speed++;
    if (F_2e38_239c[row][col] == 8 || F_2e38_239c[row][col] == 9)
        speed += 2;
    return speed;
}

int far nearest_city_impl(int x, int y)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 92; i++) {
        if (data_file.loc[i]->x == x && data_file.loc[i]->y == y && data_file.loc[i]->icon == 0)
            continue;
        dx = abs(data_file.loc[i]->x - x);
        dy = abs(data_file.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    return best;
}

int far sub_s030_1462_27aa(int x, int y)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 92; i++) {
        if (data_file.loc[i]->x == x && data_file.loc[i]->y == y && data_file.loc[i]->unk17
                && data_file.loc[i]->icon == 0)
            continue;
        dx = abs(data_file.loc[i]->x - x);
        dy = abs(data_file.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    return best;
}

int far nearest_loc_type_impl(int x, int y, int icon)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 414; i++) {
        if (data_file.loc[i]->icon != icon)
            continue;
        if (data_file.loc[i]->x == x && data_file.loc[i]->y == y)
            continue;
        if (data_file.loc[i]->flags & 4)
            continue;
        dx = abs(data_file.loc[i]->x - x);
        dy = abs(data_file.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    if (min == 9999 && cur_loc != -1)
        return cur_loc;
    return best;
}

int far sub_s030_1462_28f2(int x, int y, int icon)
{
    int best, r, max, i, dx, dy, d;

    best = -1;
    r = rand_mod(100);
    max = 0;
    for (i = 0; i < 414; i++) {
        if (data_file.loc[i]->icon != icon)
            continue;
        if (data_file.loc[i]->x == x && data_file.loc[i]->y == y)
            continue;
        dx = abs(data_file.loc[i]->x - x);
        dy = abs(data_file.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d > max && d < r + 100) {
            max = d;
            best = i;
        }
    }
    if (best != -1)
        return best;
    return nearest_loc_type_impl(x, y, icon);
}

int far sub_s030_1462_29fa(int, int, int, int);
int far sub_s030_1462_29bc(int k)
{
    return sub_s030_1462_29fa(data_file.loc[cur_loc]->x, data_file.loc[cur_loc]->y,
                              data_file.loc[k]->x, data_file.loc[k]->y);
}

int far sub_s030_1462_29fa(int x1, int y1, int x2, int y2)
{
    int dx, dy;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1) / 3;
    if (x2 <= x1) {
        if (y2 <= y1) {
            if (dx / 2 >= dy)
                return 6;
            if (dy / 2 >= dx)
                return 0;
            return 7;
        } else {
            if (dx / 2 >= dy)
                return 6;
            if (dy / 2 >= dx)
                return 4;
            return 5;
        }
    } else {
        if (y2 <= y1) {
            if (dx / 2 >= dy)
                return 2;
            if (dy / 2 >= dx)
                return 0;
            return 1;
        } else {
            if (dx / 2 >= dy)
                return 2;
            if (dy / 2 >= dx)
                return 4;
            return 3;
        }
    }
}

int far sub_s030_1462_2ac8(int p_6)
{
    int v_di;
    int v_si;
    v_di = 9;
    switch (p_6) {
    case 1:
    case 2:
    case 0x10:
        v_di = 0x28;
        break;
    case 3:
    case 0x13:
        v_di = 0x1e;
        break;
    case 4:
    case 5:
        v_di = 0x32;
        break;
    case 6:
        v_di = 0x11;
        break;
    case 7:
        v_di = 0xd;
        break;
    case 9:
        v_di = 0xb;
        break;
    case 0xa:
        v_di = 0xa;
        break;
    case 0xb:
        v_di = 0xc;
        break;
    case 0xc:
        v_di = 0xe;
        break;
    case 0xd:
        v_di = 0x10;
        break;
    case 0xe:
        v_di = 0x12;
        break;
    case 0xf:
        v_di = 0x1b;
        break;
    case 0x11:
        v_di = 0x3c;
        break;
    case 0x12:
        v_di = 0x14;
        break;
    case 0x14:
        v_di = 0x2d;
        break;
    case 0x15:
        v_di = 0x43;
        break;
    case 0x16:
        v_di = 0x62;
        break;
    case 0x17:
        v_di = 0xc5;
        break;
    case 0x18:
        v_di = 6;
    }
    if (v_di >= 0x5a0) {
        v_si = (unsigned)v_di / 0x5a0;
        v_di += v_si * -0x5a0;
        do {
            advance_hours(1);
            v_si--;
        } while (v_si);
    }
    return v_di;
}

#pragma check_stack(on)
void far map_message(int msg)
{
    char *fmt;
    struct find_t ft;
    char buf[400];
    struct _iobuf *f;
    unsigned seg;

    fmt = "Could not open %s file\n";
    while (mouse_buttons & 1)
        ;
    if (_dos_findfirst("darkland.msg", 0, &ft)) {
        printf(fmt, "map message description");
        exit_(1);
    } else {
        f = fopen("darkland.msg", "r");
        fgetc(f);
        fseek(f, (long)(msg * 400 + 1), 0);
        fread(buf, 400, 1, f);
        fclose(f);
    }
    mouse_hide();
    sub_root_047c_0545();
    seg = gfx_save_rect(0, 100, 50, 180, 115);
    sub_root_047c_04f8();
    gfx_fill_rect(screen_win, 100, 50, 180, 115, 7);
    gfx_line(screen_win, 100, 50, 279, 50, 8);
    gfx_line(screen_win, 279, 50, 279, 164, 8);
    gfx_line(screen_win, 279, 164, 100, 164, 8);
    gfx_line(screen_win, 100, 164, 100, 50, 8);
    screen_win_colour = 0;
    draw_text_wrapped(screen_win, buf, 170, 105, 55, font_height(screen_win_font), 0, 0, 0, 1, 0, 0);
    fmt = (char *)seg;
    while (!key_pressed() && !(mouse_buttons & 1) && !(mouse_buttons & 2))
        ;
    gfx_draw_pic(screen_win, 100, 50, seg);
    mouse_show();
    _dos_freemem(seg);
}
#pragma check_stack(off)

int far sub_s030_1462_2d44(int type, int loc)
{
    int r;

    r = 1;
    switch (type) {
    case 4: case 5: case 16: case 17: case 20: case 21:
        r = 3;
        break;
    case 6: case 7: case 14: case 15: case 18: case 19: case 22: case 23:
        r = 2;
        break;
    }
    if (loc >= 0 && loc <= 0x19e) {
        if (event_at(0x2a, loc))
            r += event_value_at(0x2a, loc);
        else
            r += data_file.loc[loc]->unk0a;
    }
    return r;
}

#pragma optimize("a", off)
void far sub_s030_1462_2dd6(void)
{
    int changed, old;
    unsigned v;                     /* unsigned: gives `cmp si,dx` (int: `cmp dx,si`) */

    changed = 0;
    old = D_2860;
    switch (cur_month) {
    case 0: case 1: case 2: case 11:
        if (D_e48c_b[0xde] != 0x2e)
            changed = 1;
        D_e48c_b[0xeb] = 0x30;
        D_e48c_b[0xdf] = 0x34;
        D_e48c_b[0xe8] = 0x34;
        D_e48c_b[0xef] = 0x34;
        D_e48c_b[0xf0] = 0x2A;
        D_e48c_b[0xf1] = 0x28;
        D_e48c_b[0xf2] = 0x31;
        D_e48c_b[0xde] = 0x2E;
        D_e48c_b[0xed] = 0x2E;
        D_e48c_b[0x10b] = 0x2E;
        D_e48c_b[0xee] = 0x2C;
        D_e48c_b[0x10c] = 0x2C;
        D_e48c_b[0xea] = 0x32;
        D_e48c_b[0x10d] = 0x32;
        D_e48c_b[0xe9] = 0x3C;
        D_e48c_b[0x142] = 0x3C;
        D_e48c_b[0x144] = 0x3C;
        D_e48c_b[0x14a] = 0x3C;
        D_e48c_b[0xec] = 0x38;
        D_e48c_b[0x111] = 0x38;
        D_e48c_b[0x13b] = 0x38;
        D_e48c_b[0x145] = 0x38;
        D_e48c_b[0x14b] = 0x38;
        D_e48c_b[0x1f2] = 0x38;
        D_e48c_b[0x1f3] = 0x38;
        D_e48c_b[0x216] = 0x38;
        D_e48c_b[0x112] = 0x33;
        D_e48c_b[0x13c] = 0x33;
        D_e48c_b[0x217] = 0x33;
        D_e48c_b[0x141] = 0x3B;
        D_e48c_b[0x22b] = 0x3B;
        D_e48c_b[0x146] = 0x3A;
        D_e48c_b[0x14c] = 0x3A;
        D_e48c_b[0x22c] = 0x3A;
        D_e48c_b[0x113] = 0x3F;
        D_e48c_b[0x13d] = 0x3F;
        D_e48c_b[0x143] = 0x3F;
        D_e48c_b[0x1f4] = 0x3F;
        D_e48c_b[0x218] = 0x3F;
        D_e48c_b[0x22d] = 0x3F;
        D_e48c_b[0x2be] = 0x25;
        D_e48c_b[0x2bf] = 0x2F;
        D_e48c_b[0xe0] = 0x37;
        D_e48c_b[0xe7] = 0x37;
        D_e48c_b[0x2c0] = 0x37;
        v = 15;
        goto done;
    case 3:
        if (D_e48c_b[0x135] != 0x3f)
            changed = 1;
        D_e48c_b[0x135] = 0x3F;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        break;
    case 8:
        v = 0x68;
        if (cur_day > 2)
            D_e48c_b[0x135] = 0x3f;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        if (cur_day > 4)
            D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        if (cur_day > 6)
            v = 6;
        goto done;
    case 9:
        if (D_e48c_b[0x135] != 0x3f)
            changed = 1;
        D_e48c_b[0x135] = 0x3F;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        v = 6;
        goto done;
    case 10:
        v = 6;
        D_e48c_b[0xde] = 0x2E;
        D_e48c_b[0xe0] = 0x37;
        D_e48c_b[0xe7] = 0x37;
        D_e48c_b[0xdf] = 0x34;
        D_e48c_b[0xe8] = 0x34;
        D_e48c_b[0xe9] = 0x3C;
        D_e48c_b[0xea] = 0x32;
        D_e48c_b[0xeb] = 0x30;
        D_e48c_b[0xec] = 0x38;
        if (cur_day > 2) {
            if (D_e48c_b[0xed] != 0x2e)
                changed = 1;
            D_e48c_b[0xef] = 0x34;
            D_e48c_b[0xf0] = 0x2A;
            D_e48c_b[0xf1] = 0x28;
            D_e48c_b[0xf2] = 0x31;
            D_e48c_b[0xed] = 0x2E;
            D_e48c_b[0x10b] = 0x2E;
            D_e48c_b[0xee] = 0x2C;
            D_e48c_b[0x10c] = 0x2C;
            D_e48c_b[0x10d] = 0x32;
            D_e48c_b[0x111] = 0x38;
            D_e48c_b[0x112] = 0x33;
            D_e48c_b[0x113] = 0x3F;
        }
        if (cur_day > 3) {
            if (D_e48c_b[0x13b] != 0x38)
                changed = 1;
            D_e48c_b[0x142] = 0x3C;
            D_e48c_b[0x144] = 0x3C;
            D_e48c_b[0x14a] = 0x3C;
            D_e48c_b[0x13b] = 0x38;
            D_e48c_b[0x145] = 0x38;
            D_e48c_b[0x14b] = 0x38;
            D_e48c_b[0x1f2] = 0x38;
            D_e48c_b[0x1f3] = 0x38;
            D_e48c_b[0x216] = 0x38;
            D_e48c_b[0x13c] = 0x33;
            D_e48c_b[0x217] = 0x33;
            D_e48c_b[0x141] = 0x3B;
            D_e48c_b[0x22b] = 0x3B;
            D_e48c_b[0x146] = 0x3A;
            D_e48c_b[0x14c] = 0x3A;
            D_e48c_b[0x22c] = 0x3A;
            D_e48c_b[0x13d] = 0x3F;
            D_e48c_b[0x143] = 0x3F;
            D_e48c_b[0x1f4] = 0x3F;
            D_e48c_b[0x218] = 0x3F;
            D_e48c_b[0x22d] = 0x3F;
            D_e48c_b[0x2be] = 0x25;
            D_e48c_b[0x2bf] = 0x2F;
            D_e48c_b[0x2c0] = 0x37;
        }
        if (cur_day > 4)
            v = 15;
        goto done;
    }
    v = 0x68;
done:
    D_2860 = v;
    if (old != v || changed)
        set_palette(cur_palette);
}
#pragma optimize("", on)
