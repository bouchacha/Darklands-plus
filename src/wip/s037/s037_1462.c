/* place: s037 1462:0000 */
/* orig: size=4664 CONST=7ea6+0008 _DATA=403c+03b1 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: ALL 20 FUNCTIONS WRITTEN; _DATA matches. 17/20 match (0348 0390 03d4 2760 2922 2a68 38aa
   3992 3a8a 3ae2 3b58 3cd6 3f30 4048 4184 43e6 4562). 4184 MATCHED (w2) with #pragma optimize("a", on);
   43e6 MATCHED (w2): pragma a + no `goto draw` (the redraw writes all three calls; 6.00 cross-jumps
   the buf call into the first one).  2760 MATCHED (w2): no y variable (`i * 8 + 0x18` written out:
   6.00's strength-reduced y gets the [bp-8] home) + locals declared n, ids, j, i, nr, h (compare
   direction `cmp [i],ax`). 3b58 MATCHED (m1) by reordering its local
   declarations (n, i, nr, ids, h, c, fp): the `cmp dx,di` operand order follows them. Character creation main menu (pics\cretscrn.pic) -
   twin of s038 1462 (its 0000/01fe/024e/0292/02f8/23ec/25ca are 0000/0348/0390/03d4/0444/
   2760/2922 here; see that file for the charactr.tmp layout), plus save game (2a68/2ace),
   party add/remove (3cd6/3b58/3f30), charactr.tmp record helpers and the text input box.
   Left:
     - 0444 (key handler, 8990/8988): ported from s038 02f8 + new cases B (begin game),
       D (drop from party), K (delete character); colour cases 1/2/3 generated from the asm
       (e4 gencol.py) - the 2nd case's fifth "A00" test cycles D_e894 (original bug, kept).
       Known 6.00 differences as in s038 (il*2 CSE in DI across calls, push di/pop di around
       inline strcpy, cross-jump copy choice), imul operand order D_e7da * h.
     - 0000: same length now (key used directly in the tail instead of a copy, pragma "a");
       only the unused 2-byte slot above key is missing (key [bp-2] vs [bp-4]), as s038 0000.
     - 2ace (save game): w2 moved `int nd;` to the top of the declarations: right length now
       (3548) and the `ixn < nd` compare direction fixed; left only the `nel > ixn` loop test
       (`mov ax,[ixn]; cmp [nel],ax; jg`, ours reversed); moving nel/ixn breaks the slots.
   _DATA notes: "Saving Game" and the 0x30-byte zero block are initialised globals; MSC
   emits a global after the literals of the function FOLLOWING its definition, so they are
   defined before 0000 and before 2ace. */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
#define FP_SEG(fp) (*((unsigned _far *)&(fp) + 1))
extern struct font *card_window;      /* current screen */
extern int card_window2;
extern int cur_hour, cur_day, cur_month, cur_year;
extern char opt_sound, opt_music;
extern int D_a53e[][9];          /* per party member, 0x12 bytes; -536 = none */
extern char game_difficulty;
extern int party_money, party_money_gr, party_money_pf, D_9072, D_9074, D_9076, D_9078, party_fame, party_seed;
extern int party_x, party_y, D_a772, D_e898, saved_card, saved_card2, prev_screen, D_a67c, D_db27, D_da34, D_a891;
extern char marching_order[6];
extern char D_a680[];            /* name of the save file */
extern char far *loc_buf;         /* location memory */
extern int D_a887;
extern char far * far events[0x12c];   /* events (0x30 bytes each) */
int far vec_1ccf();
int far vec_1cc5();
int far vec_1cbb();
int far battle_save_vec();
int far text_box();
int far mem_alloc();
int far file_load_far();
int far sub_root_06a1_12fa();
int far rect_copy_paged();
int far creat();
int far itoa();
int far fwrite();
int far exit_();
int far _fstrcpy();
long far ftell();
void far create_character();
struct find_t { char reserved[21]; char attrib; unsigned wr_time; unsigned wr_date; long size; char name[13]; };
extern char D_907a, leader;
extern int D_a65e;               /* text box background picture */
extern int D_e80a;               /* record of the character being created (0x80 bytes) */
extern unsigned char far F_2e38_3b1a[];
extern unsigned char far F_2e38_3b04[];
extern char far F_2e38_3984[];
extern char far * far locations[0x19e];   /* locations (0x3a bytes each) */
int far printf();
int far strncpy();
int far _fstrcat();
int far _dos_creat();
int far _dos_findfirst();
int far _dos_findnext();
int far remove();
int far gfx_save_rect();
int far sub_root_06a1_2824();
int far event_find();
int far event_free();
extern int cur_loc;   /* position matters: compare direction in 23ec */
struct font { char _p0[0xc]; int color; };
/* character record as stored in charactr.tmp (0x80 bytes, cf. party member records party) */
struct chr { int id; char _p2[0x12]; char status; char _p15[0x10]; char name[0x5b]; };
extern struct chr D_8b52;           /* character record buffer */
extern int D_8d06[];
extern char msg_buf[];
extern char member_shield[][4];      /* shield code per party slot ("F01", ...) */
extern char party[][0x80];
extern struct { char _p[0x1ddf]; char far *s[0x41]; } far F_290e_0000;
int far party_panel_frames();
int far party_panel_rows();
int far gfx_text();
int far _dos_close();
extern char screen_win[];
extern int screen_win_font;
extern int back_win_colour;
extern int back_win_font;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char D_9419[];
extern char D_9920;
extern char D_9934;
extern int party_icons_dirty;
extern char cur_palette[];
extern int D_e7a0;
extern int D_e7a2;
extern int D_e7da;
extern int D_e7dc;
extern int D_e88a;
extern int D_e88e;
typedef struct { char _p[16]; } FILE;
extern char D_e753, D_e754, D_e755, D_e756, D_e757, D_e758, D_e759, D_e75a, D_e75b, D_e75c,
    D_e75d, D_e75e, D_e75f, D_e760, D_e761, D_e762, D_e763, D_e764, D_e765, D_e766, D_e767,
    D_e768, D_e769, D_e76a;     /* current portrait palette (24 bytes) */
extern char D_e7e2[][4];        /* shield codes */
extern int D_e7de, D_e7e0;      /* number of shield codes, current one */
extern int D_e890, D_e892, D_e894;  /* colour indices for keys 1/2/3 */
extern int screen_win_colour;
extern unsigned shield_pics[];
extern char combatants[];
extern struct { int v; } party_size;   /* number of party members (struct: compare operand order) */
extern char far member_palette[5][0x18];   /* palette per party slot */
extern char far saints_known[][0x14];
extern char far formulas_known[][0x16];
extern char far inventories[][0x180];
FILE *far fdopen();
int far fread();
int far fclose();
int far fseek();
int far _dos_write();
int memcmp();
char *strcpy();
char *strcat();
int far quit_to_dos();
int far event_here_type();
int far event_remove();
int far equip_weight_vec();
int far create_character_vec();
int far _dos_freemem();
int far _dos_read();
int far menu_bar_run();
int far draw_text_far();
int far gfx_recolor_rect();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far gfx_copy_rect();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far gfx_draw_pic();
int far key_pressed();
int far get_key();
int far mouse_show();
int far mouse_hide();
int far sub_root_047c_1320();
int far mem_free();
int far pic_draw_pal();
int far pic_load();
int far sub_root_06a1_2c9e();
int far event_type_at();
int far slot_of_char_vec();
int far vec_20c1();
int far sub_s037_1462_0000();
int far sub_s037_1462_0348();
int far sub_s037_1462_0390();
void far sub_s037_1462_03d4();
int far sub_s037_1462_0444();
void far sub_s037_1462_2760(struct font *);
void far sub_s037_1462_2922(struct font *);
void far save_game();
void far sub_s037_1462_2ace();
void far sub_s037_1462_38aa();
void far sub_s037_1462_3992();
int far char_in_slot();
int far slot_of_char();
void far party_remove();
int far party_add();
void far party_delete();
void far sub_s037_1462_4048();
void far load_saved_game();
void far text_input();
void far sub_s037_1462_4562();
int far vec_18f1();
int far member_sheet_vec();


/* shown by 2ace */
char saving_game[] = "Saving Game";

#pragma optimize("a", on)
int far sub_s037_1462_0000(void)
{
    int key;
    int row;
    int k;
    vec_18f1(0x14);
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_root_047c_0568();
    sub_s037_1462_4562();
    D_e88e = 0;
    load_saved_game((char far *)"default");
    for (k = 0; k < 5; k++)
        party[k][0] = 0;
    sub_s037_1462_0390();
    sub_s037_1462_03d4();
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    do {
        key = 0;
        if ((mouse_buttons & 1) && mouse_x > 0x46 && mouse_x < 0xc8 && mouse_y > 0x19 && mouse_y < 0xcd) {
            while (mouse_buttons & 1)
                ;
            switch (row = (mouse_y - 0x19) / 15) {
            case 0:
                key = 0x2e43;
                break;
            case 1:
                key = 0x1e41;
                break;
            case 2:
                key = 0x2348;
                break;
            case 3:
                key = 0x1f53;
                break;
            case 4:
                key = 0x231;
                break;
            case 5:
                key = 0x332;
                break;
            case 6:
                key = 0x433;
                break;
            case 7:
                key = 0x2044;
                break;
            case 8:
                key = 0x254b;
                break;
            case 9:
                key = 0x3042;
                break;
            case 10:
                key = 0x11b;
                break;
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0xc5 && mouse_x < 0x138 && mouse_y > 0x14 && mouse_y < 0xb4) {
            while (mouse_buttons & 1)
                ;
            k = (mouse_y - 0x14) / 8;
            if (D_e7dc > k) {
                mouse_hide();
                gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x12, 0x10);
                D_e7da = k;
                gfx_recolor_rect(screen_win, 0xc6, (k << 3) + 0x16, 0x71, 8, 0x10, 0x12);
                mouse_show();
            }
        }
        /* click on a party portrait: show that member (member_sheet_vec), then redraw */
        if ((mouse_buttons & 1) && mouse_x < 0x3c && party[k = mouse_y / 0x28][0]) {
            member_sheet_vec(k);
            mouse_hide();
            sub_root_047c_0568();
            pic_draw_pal(1, "pics\\cretscrn.pic", cur_palette);
            D_e753 = member_palette[k][0];
            D_e754 = member_palette[k][1];
            D_e755 = member_palette[k][2];
            D_e756 = member_palette[k][3];
            D_e757 = member_palette[k][4];
            D_e758 = member_palette[k][5];
            D_e759 = member_palette[k][6];
            D_e75a = member_palette[k][7];
            D_e75b = member_palette[k][8];
            D_e75c = member_palette[k][9];
            D_e75d = member_palette[k][10];
            D_e75e = member_palette[k][11];
            D_e75f = member_palette[k][12];
            D_e760 = member_palette[k][13];
            D_e761 = member_palette[k][14];
            D_e762 = member_palette[k][15];
            D_e763 = member_palette[k][16];
            D_e764 = member_palette[k][17];
            D_e765 = member_palette[k][18];
            D_e766 = member_palette[k][19];
            D_e767 = member_palette[k][20];
            D_e768 = member_palette[k][21];
            D_e769 = member_palette[k][22];
            D_e76a = member_palette[k][23];
            set_palette(cur_palette);
            sub_s037_1462_03d4();
            mouse_show();
            sub_root_047c_056f();
            while (mouse_buttons & 1)
                ;
        }
        if (key_pressed())
            key = get_key();
        if (key == 0x11b || key == 0x1352 || key == 0x1372)
            return sub_s037_1462_0348(0);
    } while (!key || sub_s037_1462_0444(key) != 0x11b);
    return sub_s037_1462_0348(1);
}
#pragma optimize("", on)

int far sub_s037_1462_0348(int p_6)
{
    if (D_e88e) {
        _dos_freemem(D_e88e);
    }
    mem_free(D_e88a, "b");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    do {
    } while (mouse_buttons & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s037_1462_0390(void)
{
    pic_draw_pal(1, "pics\\cretscrn.pic", cur_palette);
    sub_root_047c_0545();
    D_e88a = pic_load("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    back_win_font = 1;
    screen_win_font = 1;
    set_palette(cur_palette);
    back_win_colour = 8;
}

void far sub_s037_1462_03d4(void)
{
    int i;
    sub_s037_1462_2922((struct font *)0x2806);
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    mouse_show();
    sub_s037_1462_2760((struct font *)screen_win);
    mouse_hide();
    for (i = 0; i < 5; i++) {
        if (party[i][0]) {
            party_panel_frames(i);
            party_panel_rows(i);
        }
    }
    mouse_show();
}

/* character-creation menu commands (key code from 0000); returns 0x11b to leave the menu */
int far sub_s037_1462_0444(register int key)
{
    int x1g;
    int fhc;
    long x2h;
    FILE *fpm;
    int nrd;
    int il;
    int hn;
    int x3l;
    int idsh[5];
    char nameh[20];
    switch (key) {
    case 0x4800: case 0x4b00: case 0x4d00:
        gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (--D_e7da < 0)
            D_e7da = D_e7dc - 1;
        gfx_recolor_rect(screen_win, 0xc6, D_e7da * hn + 0x16, 0x71, hn, 0x10, 0x12);
        gfx_copy_rect(0x2806, 0xa, 0xa5, 0x23, 0x23, screen_win, 0xa, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, member_shield[il]);
            strcat(nameh, "small.pic");
            D_e88e = pic_load(nameh);
            sub_root_047c_04f8();
            D_e753 = member_palette[il][0];
            D_e754 = member_palette[il][1];
            D_e755 = member_palette[il][2];
            D_e756 = member_palette[il][3];
            D_e757 = member_palette[il][4];
            D_e758 = member_palette[il][5];
            D_e759 = member_palette[il][6];
            D_e75a = member_palette[il][7];
            D_e75b = member_palette[il][8];
            D_e75c = member_palette[il][9];
            D_e75d = member_palette[il][10];
            D_e75e = member_palette[il][11];
            D_e75f = member_palette[il][12];
            D_e760 = member_palette[il][13];
            D_e761 = member_palette[il][14];
            D_e762 = member_palette[il][15];
            D_e763 = member_palette[il][16];
            D_e764 = member_palette[il][17];
            D_e765 = member_palette[il][18];
            D_e766 = member_palette[il][19];
            D_e767 = member_palette[il][20];
            D_e768 = member_palette[il][21];
            D_e769 = member_palette[il][22];
            D_e76a = member_palette[il][23];
            set_palette(cur_palette);
            gfx_draw_pic(screen_win, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x5000:
        gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (++D_e7da >= D_e7dc)
            D_e7da = 0;
        gfx_recolor_rect(screen_win, 0xc6, D_e7da * hn + 0x16, 0x71, hn, 0x10, 0x12);
        gfx_copy_rect(0x2806, 0xa, 0xa5, 0x23, 0x23, screen_win, 0xa, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, member_shield[il]);
            strcat(nameh, "small.pic");
            D_e88e = pic_load(nameh);
            sub_root_047c_04f8();
            D_e753 = member_palette[il][0];
            D_e754 = member_palette[il][1];
            D_e755 = member_palette[il][2];
            D_e756 = member_palette[il][3];
            D_e757 = member_palette[il][4];
            D_e758 = member_palette[il][5];
            D_e759 = member_palette[il][6];
            D_e75a = member_palette[il][7];
            D_e75b = member_palette[il][8];
            D_e75c = member_palette[il][9];
            D_e75d = member_palette[il][10];
            D_e75e = member_palette[il][11];
            D_e75f = member_palette[il][12];
            D_e760 = member_palette[il][13];
            D_e761 = member_palette[il][14];
            D_e762 = member_palette[il][15];
            D_e763 = member_palette[il][16];
            D_e764 = member_palette[il][17];
            D_e765 = member_palette[il][18];
            D_e766 = member_palette[il][19];
            D_e767 = member_palette[il][20];
            D_e768 = member_palette[il][21];
            D_e769 = member_palette[il][22];
            D_e76a = member_palette[il][23];
            set_palette(cur_palette);
            gfx_draw_pic(screen_win, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x1352: case 0x1372:
        return 0x11b;
    case 0x3042: case 0x3062:
        /* begin the game with the current party */
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x9f, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0xa3, F_290e_0000.s[4]);
            party_size.v = 0;
            sub_root_06a1_2c9e("charactr.tmp", 0, &fhc);
            fpm = fdopen(fhc, "rb");
            _dos_read(fhc, (int far *)&x1g, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            x2h = ftell(fpm);
            for (il = 0; il < 5; il++) {
                if (idsh[il] == -1)
                    party[il][0] = 0;
                else
                    party_size.v = il + 1;
            }
            fclose(fpm);
            if (party_size.v)
                return 0x11b;
            mouse_show();
            mouse_hide();
            gfx_copy_rect(0x2806, 0x43, 0x9f, 0x82, 0xe, screen_win, 0x43, 0x9f);
            mouse_show();
        }
        break;
    case 0x1e41: case 0x1e61:
        /* add the selected character to the party */
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x27, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x2b, F_290e_0000.s[0]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 4) {
                for (il = 0; il < 4; il++)
                    if (idsh[il] == -1)
                        break;
                if (il >= 4) {
                    fclose(fpm);
                    gfx_copy_rect(0x2806, 0x43, 0x27, 0x82, 0xe, screen_win, 0x43, 0x27);
                    mouse_show();
                    break;
                }
                idsh[il] = D_e7da;
                strcpy(member_shield[il], D_e7e2[0]);
                if (D_e88e)
                    _dos_freemem(D_e88e);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\shielda.pic");
                shield_pics[il] = pic_load(nameh);
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[0]);
                strcat(nameh, "small.pic");
                D_e88e = pic_load(nameh);
                sub_root_047c_04f8();
                D_e753 = 0x39;
                member_palette[il][0] = 0x39;
                D_e754 = 0x36;
                member_palette[il][1] = 0x36;
                D_e755 = 0;
                D_e759 = 0;
                D_e75c = 0;
                D_e75f = 0;
                D_e763 = 0;
                D_e764 = 0;
                D_e766 = 0;
                D_e767 = 0;
                D_e769 = 0;
                D_e76a = 0;
                member_palette[il][2] = 0;
                D_e756 = 0x3d;
                member_palette[il][3] = 0x3d;
                D_e757 = 0x38;
                member_palette[il][4] = 0x38;
                D_e758 = 0x15;
                D_e760 = 0x15;
                member_palette[il][5] = 0x15;
                member_palette[il][6] = 0;
                D_e75a = 0x18;
                member_palette[il][7] = 0x18;
                D_e75b = 0x28;
                member_palette[il][8] = 0x28;
                member_palette[il][9] = 0;
                D_e75d = 0x11;
                D_e768 = 0x11;
                member_palette[il][10] = 0x11;
                D_e75e = 0x25;
                D_e761 = 0x25;
                member_palette[il][11] = 0x25;
                member_palette[il][12] = 0;
                member_palette[il][13] = 0x15;
                member_palette[il][14] = 0x25;
                D_e762 = 0x1c;
                member_palette[il][15] = 0x1c;
                member_palette[il][16] = 0;
                member_palette[il][17] = 0;
                D_e765 = 0x17;
                member_palette[il][18] = 0x17;
                member_palette[il][19] = 0;
                member_palette[il][20] = 0;
                member_palette[il][21] = 0x11;
                member_palette[il][22] = 0;
                member_palette[il][23] = 0;
                set_palette(cur_palette);
                gfx_copy_rect(0x2806, 0xa, 0xa5, 0x24, 0x24, screen_win, 0xa, 0xa5);
                gfx_draw_pic(screen_win, 0xc, 0xa4, D_e88e);
                fseek(fpm, (long)(unsigned)(D_e7da * 0x22a), 1);
                fread(&combatants[il << 7], 0x80, 1, fpm);
                fseek(fpm, 0L, 1);
                _dos_read(fhc, (char far *)saints_known[il], 0x14, &nrd);
                _dos_read(fhc, (char far *)formulas_known[il], 0x16, &nrd);
                _dos_read(fhc, (char far *)inventories[il], 0x180, &nrd);
                party[il][0] = 1;
                equip_weight_vec(il);
                party_panel_frames(il);
                party_panel_rows(il);
            }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            mouse_show();
            mouse_hide();
            gfx_copy_rect(0x2806, 0x43, 0x27, 0x82, 0xe, screen_win, 0x43, 0x27);
            mouse_show();
            sub_s037_1462_2760((struct font *)screen_win);
        }
        break;
    case 0x2044: case 0x2064:
        /* drop the selected character from the party */
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x81, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x85, F_290e_0000.s[3]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_e7da) {
                    _dos_freemem(shield_pics[il]);
                    shield_pics[il] = 0;
                    idsh[il] = -1;
                    party[il][0] = 0;
                    party_panel_rows(il);
                    break;
                }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            mouse_show();
            mouse_hide();
            gfx_copy_rect(0x2806, 0x43, 0x81, 0x82, 0xe, screen_win, 0x43, 0x81);
            mouse_show();
            sub_s037_1462_2760((struct font *)screen_win);
        }
        break;
    case 0x2348: case 0x2368:
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x36, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x3a, F_290e_0000.s[0x40]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++party[il][1] >= 'P')
                    party[il][1] = 'A';
                _dos_freemem(shield_pics[il]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, "shield");
                nameh[11] = party[il][1];
                nameh[12] = 0;
                strcat(nameh, ".pic");
                shield_pics[il] = pic_load(nameh);
                sub_root_047c_04f8();
                party_panel_rows(il);
                fclose(fpm);
                mouse_show();
                mouse_hide();
            }
            gfx_copy_rect(0x2806, 0x43, 0x36, 0x82, 0xe, screen_win, 0x43, 0x36);
            mouse_show();
        }
        break;
    case 0x2e43: case 0x2e63:
        mouse_hide();
        gfx_draw_pic(screen_win, 0x43, 0x18, D_e88a);
        screen_win_colour = 8;
        draw_text_far(screen_win, 0x4a, 0x1c, F_290e_0000.s[1]);
        create_character();
        if (D_e88a)
            mem_free(D_e88a, "b");
        sub_s037_1462_0390();
        sub_s037_1462_03d4();
        mouse_show();
        break;
    case 0x254b: case 0x256b:
        /* delete the selected character */
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x90, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x94, F_290e_0000.s[0x13]);
            sub_root_06a1_2c9e("charactr.tmp", 0, &fhc);
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            _dos_close(fhc);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da) {
                    _dos_freemem(shield_pics[il]);
                    shield_pics[il] = 0;
                    idsh[il] = -1;
                    party[il][0] = 0;
                    party_panel_rows(il);
                    break;
                }
            sub_s037_1462_4048(D_e7da);
            if (D_e7dc <= D_e7da)
                D_e7da = D_e7dc - 1;
            mouse_show();
            mouse_hide();
            gfx_copy_rect(0x2806, 0x43, 0x90, 0x82, 0xe, screen_win, 0x43, 0x90);
            mouse_show();
            sub_s037_1462_2760((struct font *)screen_win);
        }
        break;
    case 0x1f53: case 0x1f73:
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x45, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x49, F_290e_0000.s[0x1a]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_e7e0 >= D_e7de)
                    D_e7e0 = 0;
                _dos_freemem(D_e88e);
                strcpy(member_shield[il], D_e7e2[D_e7e0]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[D_e7e0]);
                strcat(nameh, "small.pic");
                D_e88e = pic_load(nameh);
                sub_root_047c_04f8();
                gfx_copy_rect(0x2806, 0xa, 0xa5, 0x24, 0x24, screen_win, 0xa, 0xa5);
                gfx_draw_pic(screen_win, 0xc, 0xa4, D_e88e);
                fseek(fpm, -20L, 1);
                _dos_write(fhc, (char far *)member_shield, 0x14, &nrd);
                fclose(fpm);
                mouse_show();
                mouse_hide();
            }
            gfx_copy_rect(0x2806, 0x43, 0x45, 0x82, 0xe, screen_win, 0x43, 0x45);
            mouse_show();
        }
        break;
    case 0x231: case 0x4f31:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x39;
                        D_e754 = 0x36;
                        D_e755 = 0x0;
                        D_e756 = 0x3d;
                        D_e757 = 0x38;
                        D_e758 = 0x15;
                        break;
                    case 1:
                        D_e754 = 0x1c;
                        D_e755 = 0x1c;
                        D_e756 = 0x1c;
                        D_e753 = 0x1a;
                        D_e757 = 0x1a;
                        D_e758 = 0x1a;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 3:
                        D_e754 = 0x9;
                        D_e753 = 0x23;
                        D_e756 = 0x23;
                        D_e757 = 0xc;
                        D_e755 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0x23;
                        D_e755 = 0x9;
                        D_e754 = 0x21;
                        D_e756 = 0x21;
                        D_e757 = 0x1a;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e753 = 0x21;
                        D_e754 = 0x15;
                        D_e756 = 0x19;
                        D_e755 = 0x12;
                        D_e757 = 0x12;
                        D_e758 = 0xe;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e762 = 0x21;
                        D_e765 = 0x28;
                        D_e768 = 0x2d;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e762 = 0x2f;
                        D_e763 = 0x2a;
                        D_e764 = 0x12;
                        D_e765 = 0x37;
                        D_e766 = 0x32;
                        D_e767 = 0x1a;
                        D_e768 = 0x3e;
                        D_e769 = 0x3c;
                        D_e76a = 0x25;
                        break;
                    case 2:
                        D_e762 = 0x2d;
                        D_e763 = 0x1e;
                        D_e764 = 0x0;
                        D_e765 = 0x35;
                        D_e766 = 0x28;
                        D_e767 = 0x8;
                        D_e768 = 0x3e;
                        D_e769 = 0x33;
                        D_e76a = 0x12;
                        break;
                    case 3:
                        D_e762 = 0x1c;
                        D_e763 = 0xd;
                        D_e765 = 0x25;
                        D_e766 = 0x11;
                        D_e768 = 0x2d;
                        D_e769 = 0x15;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 4:
                        D_e762 = 0x0;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0xa;
                        D_e766 = 0xa;
                        D_e767 = 0xa;
                        D_e768 = 0x12;
                        D_e769 = 0x12;
                        D_e76a = 0x12;
                        break;
                    case 5:
                        D_e762 = 0x15;
                        D_e763 = 0x9;
                        D_e765 = 0x1a;
                        D_e766 = 0xb;
                        D_e768 = 0x21;
                        D_e769 = 0xd;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e756 = 0x1c;
                        D_e753 = 0x1c;
                        D_e757 = 0x13;
                        D_e754 = 0x13;
                        D_e758 = 0xa;
                        D_e755 = 0xa;
                        break;
                    case 1:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x23;
                        D_e754 = 0x23;
                        D_e755 = 0x23;
                        break;
                    case 2:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 3:
                        D_e756 = 0x1c;
                        D_e757 = 0x13;
                        D_e758 = 0xa;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 4:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x28;
                        D_e756 = 0x21;
                        D_e757 = 0xc;
                        D_e755 = 0x19;
                        D_e759 = 0x19;
                        D_e75a = 0x7;
                        D_e754 = 0x12;
                        D_e758 = 0x12;
                        D_e75b = 0x12;
                        break;
                    case 1:
                        D_e754 = 0x20;
                        D_e755 = 0x33;
                        D_e757 = 0x1c;
                        D_e758 = 0x2d;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        break;
                    case 2:
                        D_e753 = 0x1a;
                        D_e755 = 0x26;
                        D_e756 = 0x15;
                        D_e757 = 0x19;
                        D_e754 = 0x20;
                        D_e758 = 0x20;
                        D_e759 = 0x13;
                        D_e75a = 0x17;
                        D_e75b = 0x1e;
                        break;
                    case 3:
                        D_e753 = 0x2e;
                        D_e754 = 0x25;
                        D_e755 = 0x25;
                        D_e756 = 0x2a;
                        D_e757 = 0x21;
                        D_e758 = 0x21;
                        D_e759 = 0x23;
                        D_e75a = 0x19;
                        D_e75b = 0x19;
                        break;
                    case 4:
                        D_e753 = 0x33;
                        D_e754 = 0x2f;
                        D_e756 = 0x2e;
                        D_e758 = 0x19;
                        D_e755 = 0x25;
                        D_e757 = 0x25;
                        D_e759 = 0x25;
                        D_e75a = 0x1c;
                        D_e75b = 0x15;
                        break;
                    case 5:
                        D_e754 = 0x1e;
                        D_e755 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x12;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x10;
                        D_e75b = 0xb;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                }
                set_palette(cur_palette);
            }
        }
        break;
    case 0x332: case 0x5032:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        D_e75d = 0x11;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x15;
                        D_e75e = 0x25;
                        D_e761 = 0x25;
                        break;
                    case 1:
                        D_e75b = 0x0;
                        D_e75c = 0xc;
                        D_e75a = 0x7;
                        D_e75d = 0x7;
                        D_e759 = 0x6;
                        D_e75e = 0x6;
                        D_e75f = 0x8;
                        D_e760 = 0x8;
                        D_e761 = 0x5;
                        break;
                    case 2:
                        D_e75a = 0x18;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0xf;
                        D_e75d = 0x12;
                        D_e75e = 0xe;
                        D_e760 = 0x13;
                        D_e75f = 0xc;
                        D_e761 = 0xc;
                        break;
                    case 3:
                        D_e759 = 0x35;
                        D_e75a = 0x35;
                        D_e75b = 0xb;
                        D_e75c = 0x2f;
                        D_e75d = 0x2f;
                        D_e75e = 0x8;
                        D_e75f = 0x39;
                        D_e760 = 0x2e;
                        D_e761 = 0x9;
                        break;
                    case 4:
                        D_e759 = 0x31;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75d = 0x13;
                        D_e75e = 0x5;
                        D_e75c = 0x25;
                        D_e75f = 0x25;
                        D_e760 = 0x17;
                        D_e761 = 0x2;
                        break;
                    case 5:
                        D_e759 = 0x1c;
                        D_e75a = 0xa;
                        D_e75c = 0x28;
                        D_e75d = 0x10;
                        D_e75e = 0x11;
                        D_e75f = 0x21;
                        D_e75b = 0xb;
                        D_e760 = 0xb;
                        D_e761 = 0xc;
                        break;
                    }
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e762 = 0x18;
                        D_e763 = 0x11;
                        D_e764 = 0x9;
                        D_e765 = 0x15;
                        D_e766 = 0x10;
                        D_e767 = 0x8;
                        D_e768 = 0x13;
                        D_e769 = 0xe;
                        D_e76a = 0x7;
                        break;
                    case 1:
                        D_e762 = 0x12;
                        D_e763 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0xd;
                        D_e766 = 0xd;
                        D_e767 = 0xd;
                        D_e768 = 0xa;
                        D_e769 = 0xa;
                        D_e76a = 0xa;
                        break;
                    case 2:
                        D_e763 = 0x1c;
                        D_e766 = 0x15;
                        D_e769 = 0x10;
                        D_e762 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0x0;
                        D_e767 = 0x0;
                        D_e768 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 3:
                        D_e762 = 0x28;
                        D_e763 = 0x25;
                        D_e764 = 0x14;
                        D_e765 = 0x21;
                        D_e766 = 0x20;
                        D_e767 = 0x11;
                        D_e768 = 0x1c;
                        D_e769 = 0x1a;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e762 = 0x21;
                        D_e763 = 0x1e;
                        D_e764 = 0x1e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1a;
                        D_e767 = 0x1a;
                        D_e768 = 0x18;
                        D_e769 = 0x14;
                        D_e76a = 0x14;
                        break;
                    case 5:
                        D_e762 = 0x32;
                        D_e763 = 0x32;
                        D_e764 = 0x25;
                        D_e765 = 0x2a;
                        D_e766 = 0x2a;
                        D_e767 = 0x1c;
                        D_e768 = 0x23;
                        D_e769 = 0x23;
                        D_e76a = 0x15;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75f = 0x10;
                        D_e760 = 0x10;
                        D_e761 = 0x12;
                        D_e75c = 0x14;
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e759 = 0x18;
                        D_e75a = 0x18;
                        D_e75b = 0x1a;
                        D_e756 = 0x1c;
                        D_e757 = 0x1c;
                        D_e758 = 0x1e;
                        break;
                    case 1:
                        D_e75f = 0x1c;
                        D_e760 = 0x21;
                        D_e75c = 0x14;
                        D_e75d = 0x19;
                        D_e759 = 0xe;
                        D_e75a = 0xe;
                        D_e761 = 0x0;
                        D_e75e = 0x0;
                        D_e75b = 0x0;
                        D_e757 = 0x9;
                        D_e756 = 0x8;
                        D_e758 = 0x8;
                        break;
                    case 2:
                        D_e761 = 0xd;
                        D_e75d = 0x4;
                        D_e75e = 0x11;
                        D_e75a = 0xa;
                        D_e75b = 0x14;
                        D_e75f = 0x0;
                        D_e760 = 0x0;
                        D_e75c = 0x0;
                        D_e759 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x12;
                        D_e758 = 0x19;
                        break;
                    case 3:
                        D_e75f = 0x11;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e759 = 0x19;
                        D_e75a = 0x12;
                        D_e756 = 0x1e;
                        D_e75c = 0x15;
                        D_e757 = 0x15;
                        D_e761 = 0x0;
                        D_e75e = 0x0;
                        D_e75b = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e75f = 0x15;
                        D_e75c = 0x19;
                        D_e759 = 0x1c;
                        D_e756 = 0x21;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        D_e75d = 0x0;
                        D_e75e = 0x0;
                        D_e75a = 0x0;
                        D_e75b = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e75f = 0x0;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        D_e75c = 0x9;
                        D_e75d = 0x9;
                        D_e75e = 0x9;
                        D_e759 = 0xe;
                        D_e75a = 0xe;
                        D_e75b = 0xe;
                        D_e756 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x15;
                        break;
                    }
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                }
                set_palette(cur_palette);
            }
        }
        break;
    case 0x433: case 0x5133:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e762 = 0x1c;
                        D_e765 = 0x17;
                        D_e768 = 0x11;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e764 = 0x0;
                        D_e765 = 0xc;
                        D_e763 = 0x7;
                        D_e766 = 0x7;
                        D_e762 = 0x6;
                        D_e767 = 0x6;
                        D_e768 = 0x8;
                        D_e769 = 0x8;
                        D_e76a = 0x5;
                        break;
                    case 2:
                        D_e762 = 0x2b;
                        D_e763 = 0x2a;
                        D_e764 = 0x1c;
                        D_e765 = 0x23;
                        D_e766 = 0x23;
                        D_e767 = 0x12;
                        D_e768 = 0x28;
                        D_e769 = 0x25;
                        D_e76a = 0x13;
                        break;
                    case 3:
                        D_e763 = 0x2a;
                        D_e764 = 0x2a;
                        D_e766 = 0x20;
                        D_e767 = 0x21;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x25;
                        D_e76a = 0x25;
                        break;
                    case 4:
                        D_e762 = 0x15;
                        D_e763 = 0x0;
                        D_e764 = 0xc;
                        D_e765 = 0x13;
                        D_e766 = 0x8;
                        D_e767 = 0xe;
                        D_e768 = 0x11;
                        D_e769 = 0x2;
                        D_e76a = 0x18;
                        break;
                    case 5:
                        D_e762 = 0x26;
                        D_e763 = 0x28;
                        D_e764 = 0x2e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1c;
                        D_e767 = 0x25;
                        D_e768 = 0x2c;
                        D_e769 = 0x2b;
                        D_e76a = 0x33;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e753 = 0x19;
                        D_e754 = 0x12;
                        D_e755 = 0x0;
                        break;
                    case 1:
                        D_e754 = 0x18;
                        D_e753 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0xa;
                        D_e755 = 0x15;
                        break;
                    case 3:
                        D_e753 = 0x1c;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0xe;
                        D_e754 = 0xe;
                        D_e755 = 0xe;
                        break;
                    case 5:
                        D_e753 = 0x3f;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e759 = 0x2a;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75c = 0x25;
                        D_e75d = 0x18;
                        D_e75e = 0xe;
                        D_e75f = 0x1c;
                        D_e760 = 0x13;
                        D_e761 = 0xa;
                        break;
                    case 1:
                        D_e759 = 0x11;
                        D_e75a = 0x11;
                        D_e75b = 0x11;
                        D_e75c = 0xa;
                        D_e75d = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0x3;
                        D_e760 = 0x3;
                        D_e761 = 0x3;
                        break;
                    case 2:
                        D_e75a = 0x2a;
                        D_e759 = 0x9;
                        D_e75b = 0x9;
                        D_e75d = 0x25;
                        D_e760 = 0x1a;
                        D_e75c = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0xa;
                        D_e761 = 0xa;
                        break;
                    case 3:
                        D_e759 = 0x21;
                        D_e75a = 0x21;
                        D_e75b = 0x21;
                        D_e75c = 0x1c;
                        D_e75d = 0x1c;
                        D_e75e = 0x1c;
                        D_e75f = 0x18;
                        D_e760 = 0x18;
                        D_e761 = 0x18;
                        break;
                    case 4:
                        D_e759 = 0x30;
                        D_e75a = 0x2d;
                        D_e75c = 0x28;
                        D_e75d = 0x28;
                        D_e75e = 0x1c;
                        D_e75b = 0x21;
                        D_e75f = 0x21;
                        D_e760 = 0x21;
                        D_e761 = 0x18;
                        break;
                    case 5:
                        D_e759 = 0x2f;
                        D_e75a = 0x2c;
                        D_e75b = 0x18;
                        D_e75c = 0x2b;
                        D_e75e = 0x15;
                        D_e75d = 0x28;
                        D_e75f = 0x28;
                        D_e760 = 0x25;
                        D_e761 = 0x13;
                        break;
                    }
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                }
                set_palette(cur_palette);
            }
        }
        break;

    case 0x1000:
        sub_s037_1462_0348(0);
        quit_to_dos();
        break;
    }
}

/* list of the characters saved in charactr.tmp (right-hand column); '+' marks party members */
/* list of the saved characters (charactr.tmp) with '+' for party members */
void far sub_s037_1462_2760(struct font *f)
{
    int n;
    int ids[5];
    int j;
    int i;
    int nr;
    int h;
    mouse_hide();
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)member_shield, 0x14, &nr);
    gfx_copy_rect(0x2806, 0xc5, 0x14, 0x73, 0x14, screen_win, 0xc5, 0x14);
    for (i = 0; i < n; i++) {
        _dos_read(h, (char far *)&D_8b52, 0x80, &nr);
        _dos_read(h, (char far *)msg_buf, 0x1aa, &nr);
        gfx_copy_rect(0x2806, 0xc5, i * 8 + 0x18, 0x73, 0x10, screen_win, 0xc5, i * 8 + 0x18);
        for (j = 0; j < 5; j++) {
            if (i == ids[j]) {
                f->color = 0x11;
                gfx_text(f, 0xc7, i * 8 + 0x18, "+");
                f->color = 0x10;
                gfx_text(f, 0xc6, i * 8 + 0x17, "+");
            }
        }
        f->color = 0x11;
        gfx_text(f, 0xcd, i * 8 + 0x17, D_8b52.name);
        f->color = 0x10;
        gfx_text(f, 0xcc, i * 8 + 0x16, D_8b52.name);
    }
    _dos_close(h);
    if ((D_e7dc = n) <= D_e7da)
        D_e7da = n - 1;
    if (D_e7dc == 0)
        D_e7da = -1;
    else if (D_e7da == -1)
        D_e7da = 0;
    gfx_recolor_rect(f, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x10, 0x12);
    mouse_show();
}

/* the menu entries */
void far sub_s037_1462_2922(struct font *f)
{
    mouse_hide();
    f->color = 8;
    draw_text_far(f, 0x48, 0x1c, F_290e_0000.s[1]);
    draw_text_far(f, 0x48, 0x2b, F_290e_0000.s[0]);
    draw_text_far(f, 0x48, 0x3a, F_290e_0000.s[0x40]);
    draw_text_far(f, 0x48, 0x49, F_290e_0000.s[0x1a]);
    draw_text_far(f, 0x48, 0x58, F_290e_0000.s[0x3d]);
    draw_text_far(f, 0x48, 0x67, F_290e_0000.s[0x3e]);
    draw_text_far(f, 0x48, 0x76, F_290e_0000.s[0x3f]);
    draw_text_far(f, 0x48, 0x85, F_290e_0000.s[3]);
    draw_text_far(f, 0x48, 0x94, F_290e_0000.s[0x13]);
    draw_text_far(f, 0x48, 0xa3, F_290e_0000.s[4]);
    draw_text_far(f, 0x48, 0xb2, F_290e_0000.s[5]);
    mouse_show();
}

/* "Save game": ask for a comment (default "Darklands"), then save */
void far save_game(void)
{
    char comment[0x29] = "Darklands";
    if (D_907a == 1) {
        sub_root_047c_0545();
        D_a65e = pic_load("pics\\textback.pic");
        sub_root_047c_04f8();
    }
    text_input((char far *)comment, (char far *)"Save Game Comment:", 0x17);
    if (comment[0])
        sub_s037_1462_2ace(comment);
}

/* unreferenced zero block in _DATA (0x31 bytes after 2ace's strings; MSC emits a global
   after the literals of the function that follows its definition) */
char unused_42c8[0x30] = {0};

/* save the game (comment c) as saves\dksaveN.sav (first unused N): header with the
   location name, comment and global state, party/character records (charactr.tmp),
   events (events.tmp), locations, then a copy of cache.tmp */
#pragma optimize("a", on)
void far sub_s037_1462_2ace(char *c)
{
    int nd;
    int hcrb;
    int zeroo;
    long zli;
    char palb[0x78];
    char zbc;
    struct find_t ffc;
    int idso[5];
    FILE *fpi;
    int bm;
    int mb;
    int je;
    int nel;
    int vi;
    int ixn;
    int oldcola;
    struct { int x; int y; int w; int ht; } rk;
    int v7o;
    char numk[10];
    int hco;
    FILE *fp2a;
    int nrh;
    char placeb[0x16];
    char *fnfl;
    char pathh[0x1a];
    int hl;
    int kh;
    char far *pa;
    unsigned nlocd;

    zbc = 0;
    zeroo = 0;
    kh = 0;
    zli = 0;
    fnfl = "FNF:%s\n";
    if (D_907a == 1) {
        for (mb = 0; mb < 5; mb++) {
            if (party[mb][0]) {
                if (D_a53e[mb][0] != -536)
                    vec_1ccf(mb);
                if (D_a53e[mb][1] != -536)
                    vec_1cc5(mb);
                if (D_a53e[mb][2] != -536)
                    vec_1cbb(mb);
            }
        }
        game_difficulty |= 0x80;
        D_907a = 0;
        mouse_hide();
        oldcola = card_window->color;
        card_window->color = 9;
        text_box(card_window, -1, 1, saving_game, card_window2, &rk);
        card_window->color = oldcola;
        mouse_show();
        D_907a = 1;
        if (_dos_findfirst("locs.tmp", 0, &ffc)) {
            printf(fnfl, "location");
            exit_(1);
        } else {
            loc_buf = pa = (char far *)((unsigned long)mem_alloc(ffc.size, "loc memory") << 16);
            file_load_far("locs.tmp", pa);
            je = (unsigned char)*pa++;
            nlocd = (unsigned char)*pa++;
            nlocd = (nlocd << 8) + je;
            for (ixn = 0; ixn < nlocd; ixn++) {
                locations[ixn] = pa;
                pa += 0x3a;
            }
        }
        if (!_dos_findfirst("events.tmp", 0, &ffc)) {
            sub_root_06a1_2c9e("events.tmp", 0, &hl);
            fp2a = fdopen(hl, "rb");
            fread(&nel, 2, 1, fp2a);
            fseek(fp2a, 2L, 0);
            for (ixn = 0; nel > ixn; ixn++) {
                events[ixn] = (char far *)((unsigned long)mem_alloc(0x30, 0, 0) << 16);
                _dos_read(hl, events[ixn], 0x30, &je);
            }
            fclose(fp2a);
        }
    } else {
        mouse_hide();
        text_box(card_window, -1, 1, saving_game, card_window2, &rk);
        mouse_show();
    }
    for (ixn = 0; ixn < 5; ixn++)
        for (je = 0; je < 0x18; je++)
            palb[ixn * 0x18 + je] = member_palette[ixn][je];
    sub_root_06a1_2c9e("charactr.tmp", 0, &hcrb);
    _dos_read(hcrb, (int far *)&nd, 2, &nrh);
    _dos_read(hcrb, (int far *)idso, 10, &nrh);
    _dos_read(hcrb, (char far *)member_shield, 0x14, &nrh);
    party_size.v = ixn = 0;
    for (; ixn < 5; ixn++)
        if (idso[ixn] != -1)
            party_size.v = ixn + 1;
    v7o = 7;
    if (cur_loc >= 0 && cur_loc < 0x19e)
        _fstrcpy((char far *)placeb, locations[cur_loc] + 0x26);
    else
        strcpy(placeb, "Wilderness");
    do {
        strcpy(pathh, "saves\\dksave");
        itoa(kh++, numk, 10);
        strcat(pathh, numk);
        strcat(pathh, ".sav");
    } while (!_dos_findfirst(pathh, 0, &ffc));
    strcpy(D_a680, "dksave");
    itoa(--kh, numk, 10);
    strcat(D_a680, numk);
    strcat(D_a680, ".sav");
    sub_root_06a1_12fa(creat(pathh, 0x80));
    sub_root_06a1_2c9e(pathh, 2, &hl);
    fpi = fdopen(hl, "r+");
    fwrite(placeb, 1, 0x15, fpi);
    fwrite(c, 1, 0x4f, fpi);
    fwrite(&party_seed, 2, 1, fpi);
    fwrite(&v7o, 2, 1, fpi);
    fwrite(&cur_year, 2, 1, fpi);
    fwrite(&cur_month, 2, 1, fpi);
    fwrite(&cur_day, 2, 1, fpi);
    fwrite(&cur_hour, 2, 1, fpi);
    fwrite(&party_money, 2, 1, fpi);
    fwrite(&party_money_gr, 2, 1, fpi);
    fwrite(&party_money_pf, 2, 1, fpi);
    fwrite(&zli, 4, 1, fpi);
    fwrite(&party_fame, 2, 1, fpi);
    fwrite(&cur_loc, 2, 1, fpi);
    fwrite(&party_x, 2, 1, fpi);
    fwrite(&party_y, 2, 1, fpi);
    fwrite(&D_a772, 2, 1, fpi);
    fwrite(&D_e898, 2, 1, fpi);
    fwrite(&saved_card, 2, 1, fpi);
    fwrite(&saved_card2, 2, 1, fpi);
    fwrite(&prev_screen, 2, 1, fpi);
    fwrite(&D_9072, 2, 1, fpi);
    fwrite(&D_9074, 2, 1, fpi);
    fwrite(&D_9076, 2, 1, fpi);
    fwrite(&D_9078, 2, 1, fpi);
    fwrite(&D_a67c, 2, 1, fpi);
    fwrite(&game_difficulty, 1, 1, fpi);
    fwrite(&D_db27, 2, 1, fpi);
    fwrite(&D_da34, 2, 1, fpi);
    fwrite(marching_order, 6, 1, fpi);
    fwrite(&leader, 1, 1, fpi);
    fwrite(&opt_music, 1, 1, fpi);
    fwrite(&opt_sound, 1, 1, fpi);
    fwrite(&D_a891, 2, 1, fpi);
    fwrite(&zbc, 1, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&zeroo, 2, 1, fpi);
    fwrite(&party_size, 2, 1, fpi);
    fwrite(&nd, 2, 1, fpi);
    fwrite(idso, 10, 1, fpi);
    fwrite(member_shield, 0x14, 1, fpi);
    fwrite(palb, 0x78, 1, fpi);
    for (ixn = 0; ixn < nd; ixn++) {
        for (je = 0; je < 5; je++)
            if (idso[je] == ixn)
                break;
        if (je < 5) {
            if (party[je][0]) {
                fseek(fpi, 0L, 1);
                fwrite(combatants + je * 0x80, 0x80, 1, fpi);
                fseek(fpi, 0L, 1);
                _dos_write(hl, saints_known[je], 0x14, &nrh);
                _dos_write(hl, formulas_known[je], 0x16, &nrh);
                _dos_write(hl, inventories[je], 0x180, &nrh);
                _dos_read(hcrb, (char far *)msg_buf, 0x22a, &nrh);
            }
        } else {
            fseek(fpi, 0L, 1);
            _dos_read(hcrb, (char far *)msg_buf, 0x22a, &nrh);
            _dos_write(hl, (char far *)msg_buf, 0x22a, &nrh);
        }
    }
    for (ixn = 0, nel = 0; ixn < 0x12c; ixn++)
        if (events[ixn])
            nel++;
    _dos_write(hl, (int far *)&nel, 2, &nrh);
    for (ixn = 0; ixn < 0x12c; ixn++)
        if (events[ixn])
            _dos_write(hl, events[ixn], 0x30, &nrh);
    vi = 0x19e;
    _dos_write(hl, (int far *)&vi, 2, &nrh);
    for (ixn = 0; ixn < 0x19e; ixn++)
        _dos_write(hl, locations[ixn], 0x3a, &nrh);
    if (_dos_findfirst("cache.tmp", 0, &ffc)) {
        printf("cache");
        exit_(1);
    } else {
        fseek(fpi, 0L, 1);
        sub_root_06a1_2c9e("cache.tmp", 2, &hco);
        fp2a = fdopen(hco, "r+");
        while (fread(&bm, 1, 1, fp2a))
            fwrite(&bm, 1, 1, fpi);
        fclose(fp2a);
    }
    fclose(fpi);
    _dos_close(hcrb);
    if (D_907a == 1) {
        mem_free(D_a887, "Maploc");
        mem_free(D_a65e, "txtbk");
        for (ixn = 0; ixn < 0x12c; ixn++) {
            if (events[ixn]) {
                mem_free(FP_SEG(events[ixn]), "event");
                events[ixn] = 0;
            }
        }
    }
    battle_save_vec(D_a680);
    mouse_hide();
    if (D_907a == 1) {
        game_difficulty &= 0x7f;
        D_907a = 0;
        rect_copy_paged(card_window2, rk.x, rk.y, rk.w, rk.ht, card_window, rk.x, rk.y);
        D_907a = 1;
    } else
        rect_copy_paged(card_window2, rk.x, rk.y, rk.w, rk.ht, card_window, rk.x, rk.y);
    mouse_show();
}

#pragma optimize("", on)
/* append the character being created (D_e80a, 0x80 bytes, plus its saints/virtues/items)
   to charactr.tmp */
void far sub_s037_1462_38aa(void)
{
    int n;
    int nr;
    int h;
    FILE *fp;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    fseek(fp, 0L, 0);
    n++;
    _dos_write(h, (int far *)&n, 2, &nr);
    fseek(fp, (long)(unsigned)(n * 0x22a - 0x20a), 0);
    D_e80a = cur_loc;
    _dos_write(h, (char far *)&D_e80a, 0x80, &nr);
    _dos_write(h, (char far *)F_2e38_3b1a, 0x14, &nr);
    _dos_write(h, (char far *)F_2e38_3b04, 0x16, &nr);
    _dos_write(h, (char far *)F_2e38_3984, 0x180, &nr);
    fclose(fp);
}

/* write party member p back to its record in charactr.tmp */
void far sub_s037_1462_3992(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    FILE *fp;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    fseek(fp, (long)(unsigned)(p * 0x22a + 0x20), 0);
    D_e80a = cur_loc;
    _dos_write(h, (char far *)(combatants + ids[p] * 0x80), 0x80, &nr);
    _dos_write(h, saints_known[p], 0x14, &nr);
    _dos_write(h, formulas_known[p], 0x16, &nr);
    _dos_write(h, inventories[p], 0x180, &nr);
    fclose(fp);
}

/* character index in party slot p */
int far char_in_slot(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    return ids[p];
}

/* party slot of character c, -1 if not in the party */
int far slot_of_char(int c)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int i;
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    for (i = 0; i < 5; i++)
        if (ids[i] == c)
            break;
    if (i >= 5)
        return -1;
    return i;
}

/* remove party member p (record written back to charactr.tmp) */
void far party_remove(int p)
{
    int n;
    int i;
    int nr;
    int ids[5];
    int h;
    int c;
    FILE *fp;
    if (leader == p) {
        for (i = 0; i < 5; i++)
            if (party[i][0] == 1 && p != i) {
                leader = i;
                break;
            }
    }
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)member_shield, 0x14, &nr);
    fseek(fp, 0L, 0);
    _dos_write(h, (int far *)&n, 2, &nr);
    c = ids[p];
    ids[p] = -1;
    _dos_write(h, (int far *)ids, 10, &nr);
    if (party[p][0] != 5)
        party[p][0] = 0;
    *(int *)(combatants + p * 0x80) = cur_loc;
    fseek(fp, (long)(unsigned)(c * 0x22a + 0x20), 0);
    _dos_write(h, (char far *)(combatants + p * 0x80), 0x80, &nr);
    fclose(fp);
    party[p][0] = 0;
    _dos_freemem(shield_pics[p]);
    shield_pics[p] = 0;
    party_size.v = i = 0;
    for (; i < 5; i++)
        if (party[i][0])
            party_size.v = i + 1;
}

/* add character c to the party (first free slot); returns the slot or -1 */
int far party_add(int c)
{
    char name[30];
    int ids[5];
    int i;
    int nr;
    int h;
    FILE *fp;
    int slot;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&i, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)member_shield, 0x14, &nr);
    for (slot = 0; slot < 5; slot++)
        if (ids[slot] == -1)
            break;
    if (slot >= 5) {
        fclose(fp);
        return -1;
    }
    ids[slot] = c;
    fseek(fp, (long)(unsigned)(c * 0x22a), 1);
    fread(combatants + slot * 0x80, 0x80, 1, fp);
    fseek(fp, 0L, 1);
    _dos_read(h, saints_known[slot], 0x14, &nr);
    _dos_read(h, formulas_known[slot], 0x16, &nr);
    _dos_read(h, inventories[slot], 0x180, &nr);
    party[slot][0] = 1;
    sub_root_047c_0545();
    strcpy(name, "pics\\");
    strcat(name, "shield");
    name[11] = party[slot][1];
    name[12] = 0;
    strcat(name, ".pic");
    shield_pics[slot] = pic_load(name);
    sub_root_047c_04f8();
    party_panel_frames(slot);
    party_panel_rows(slot);
    fseek(fp, 2L, 0);
    _dos_write(h, (int far *)ids, 10, &nr);
    _dos_write(h, (char far *)member_shield, 0x14, &nr);
    fclose(fp);
    party_size.v = i = 0;
    for (; i < 5; i++)
        if (party[i][0])
            party_size.v = i + 1;
    return slot;
}

/* delete party member p and its character (events 0x43/0x47 of it are cancelled) */
void far party_delete(int p)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int e;
    int i;
    if ((e = event_find(0x43, p, -1, -1, -1, -1)) != -1)
        event_free(e);
    if ((e = event_find(0x47, p, -1, -1, -1, -1)) != -1)
        event_free(e);
    party[p][0] = 0;
    if (leader == p) {
        for (i = 0; i < 5; i++)
            if (party[i][0] == 1 && i != p) {
                leader = i;
                break;
            }
    }
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_close(h);
    sub_s037_1462_4048(ids[p]);
    if (shield_pics[p])
        _dos_freemem(shield_pics[p]);
    shield_pics[p] = 0;
    party_size.v = 0;
    for (i = 0; i < 5; i++)
        if (party[i][0])
            party_size.v = i + 1;
}

/* delete character c from charactr.tmp (later records move down) */
void far sub_s037_1462_4048(int c)
{
    int ids[5];
    int n;
    int nr;
    int h;
    FILE *fp;
    int i;
    sub_root_06a1_2c9e("charactr.tmp", 2, &h);
    fp = fdopen(h, "r+b");
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    fseek(fp, 0L, 0);
    n--;
    _dos_write(h, (int far *)&n, 2, &nr);
    for (i = 0; i < 5; i++) {
        if (ids[i] == c)
            ids[i] = -1;
        else if (ids[i] > c)
            ids[i]--;
    }
    _dos_write(h, (int far *)ids, 10, &nr);
    for (i = c; i < n; i++) {
        fseek(fp, (long)(unsigned)(i * 0x22a + 0x24a), 0);
        _dos_read(h, (char far *)msg_buf, 0x22a, &nr);
        fseek(fp, (long)(unsigned)(i * 0x22a + 0x20), 0);
        _dos_write(h, (char far *)msg_buf, 0x22a, &nr);
    }
    fclose(fp);
}

/* load saved game `name` (saves\name): characters to charactr.tmp, locations to locs.tmp,
   empty cache.tmp, no events.tmp */
#pragma optimize("a", on)
void far load_saved_game(char far *name)
{
    char comment[0x78];
    char path[26];
    int ids[5];
    int n;
    int v;
    int hs;
    int hc;
    int nr;
    int i;
    _dos_creat("charactr.tmp", 0, &hc);
    strcpy(path, "saves\\");
    _fstrcat((char far *)path, name);
    sub_root_06a1_2c9e(path, 0, &hs);
    _dos_read(hs, (char far *)msg_buf, 0xef, &nr);
    _dos_read(hs, (int far *)&party_size, 2, &nr);
    _dos_read(hs, (int far *)&n, 2, &nr);
    _dos_read(hs, (int far *)ids, 10, &nr);
    _dos_read(hs, (char far *)member_shield, 0x14, &nr);
    _dos_read(hs, (char far *)comment, 0x78, &nr);
    _dos_write(hc, (int far *)&n, 2, &nr);
    _dos_write(hc, (int far *)ids, 10, &nr);
    _dos_write(hc, (char far *)member_shield, 0x14, &nr);
    for (i = 0; i < n; i++) {
        _dos_read(hs, (char far *)msg_buf, 0x22a, &nr);
        _dos_write(hc, (char far *)msg_buf, 0x22a, &nr);
    }
    _dos_close(hs);
    _dos_close(hc);
    party_size.v = 0;
    for (i = 0; i < 5; i++)
        if (party[i][0])
            party_size.v = i + 1;
    _dos_creat("locs.tmp", 0, &hc);
    v = 0x19e;
    _dos_write(hc, (int far *)&v, 2, &nr);
    for (i = 0; i < 0x19e; i++)
        _dos_write(hc, locations[i], 0x3a, &nr);
    _dos_close(hc);
    _dos_creat("cache.tmp", 0, &hc);
    v = 0x63;
    _dos_write(hc, (int far *)&v, 1, &nr);
    v = 0;
    _dos_write(hc, (int far *)&v, 1, &nr);
    v = 0;
    for (i = 0; i < 0x62; i++)
        _dos_write(hc, (int far *)&v, 2, &nr);
    _dos_close(hc);
    remove("events.tmp");
}
#pragma optimize("a", off)

/* text input box: edit buf (at most max characters) under the prompt; Esc clears it */
#pragma optimize("a", on)
void far text_input(char far *buf, char far *prompt, int max)
{
    int save;
    int k;
    int i;
    screen_win_colour = 0;
    mouse_hide();
    for (i = 0; i < 0x28; i++)
        if (buf[i] == 0)
            break;
    gfx_recolor_rect(screen_win, 0x6e, 0x50, 0x9b, 0x32, 0, 5);
    sub_root_047c_0545();
    save = gfx_save_rect(0, 0x6e, 0x50, 0xa0, 0x32);
    sub_root_047c_04f8();
    gfx_draw_pic(screen_win, 0x6e, 0x50, D_a65e);
    draw_text_far(screen_win, 0x7d, 0x5a, prompt);
    draw_text_far(screen_win, 0x87, 0x64, buf);
    for (;;) {
        while (!key_pressed())
            ;
        k = get_key();
        if (k == 0x1c0d)
            break;
        if (k == 0x11b) {
            buf[0] = 0;
            break;
        }
        if (k == 0x4b00 || k == 0xe08) {
            if (i > 0)
                i--;
            buf[i] = 0;
        }
        if (max > i) {
            if ((k & 0xff) >= 0x20 && (k & 0xff) <= 0x7e) {
                buf[i] = k;
                i++;
                buf[i] = 0;
            }
            gfx_draw_pic(screen_win, 0x6e, 0x50, D_a65e);
            draw_text_far(screen_win, 0x7d, 0x5a, prompt);
            draw_text_far(screen_win, 0x87, 0x64, buf);
        }
    }
    buf[i] = 0;
    gfx_draw_pic(screen_win, 0x6e, 0x50, save);
    gfx_recolor_rect(screen_win, 0x6e, 0x50, 0x9b, 0x32, 5, 0);
    mouse_show();
    mem_free(save, "B");
    screen_win_colour = 8;
}
#pragma optimize("a", off)

/* list the shield codes from pics\F??stat.PIC, A??, C?? into D_e7e2[] (D_e7de entries) */
void far sub_s037_1462_4562(void)
{
    struct find_t ff;
    *(volatile int *)&D_e7de = 0;
    if (!_dos_findfirst("pics\\F??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (!_dos_findfirst("pics\\A??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (!_dos_findfirst("pics\\C??stat.PIC", 0, &ff)) {
        do {
            strncpy(D_e7e2[D_e7de++], ff.name, 3);
            D_e7e2[D_e7de - 1][3] = 0;
        } while (!_dos_findnext(&ff));
    }
    if (D_e7e0 >= D_e7de)
        D_e7e0 = 0;
}
