/* place: s041 1462:0000 */
/* orig: size=05cc CONST=7ee0+0002 _DATA=4dee+0066 */
/* flags: /AM /G2 /Gs /Ox /Zp1 /Oa */
/* Copy-protection screen: "SELECT THE SYMBOL FOR <substance>" (overlay s041). */
#include "darklands/card.h"
#include "darklands/game.h"

extern int back_win_font, screen_win_font, back_win_colour;
extern int D_e7a2, D_e7a0;
extern char cur_palette[];
extern char back_win[];
extern char screen_win[];
extern int D_8dd4, D_8dd6, D_8dd8, D_8dda;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char D_907d;
extern int ems_frame_handle;
extern char far * far F_290e_1d73[];

char *_itoa(int, char *, int);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);

int far rand_mod(int);
void far pic_draw_pal(int, char *, char *);
void far sub_root_047c_0545(void);
int far pic_load(char *);
void far sub_root_047c_04f8(void);
void far gfx_draw_pic(char *, int, int, int);
void far gfx_text(char *, int, int, char *);
void far draw_text_far(char *, int, int, char far *);
void far sub_root_047c_0568(void);
void far set_palette(char *);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far mouse_hide(void);
void far gfx_copy_rect(char *, int, int, int, int, char *, int, int);
void far mouse_show(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_1320(int, int, int, int);
int far key_pressed(void);
int far get_key(void);
void far mem_free(int, char *);
void far quit_to_dos(void);
void far sub_root_044a_01f4(void);
void far sub_root_047c_0791(void);
void far vec_186f(void);
void far game_shutdown(void);
void far sub_root_05f8_0139(int);
void far video_restore_vec(void);
void far exit_(int);

int far copyprot_close(int pic);
void far copyprot_key(int key, int which);
void far copyprot_fail(void);

/* Show four random alchemical symbols and ask for the one matching a named substance.
   Clicking the right picture (or pressing its number key) lets the game continue. */
void far copy_protection(void)
{
    char path[20];
    char num[8];
    int which;
    int sym0, sym1, sym2, sym3;
    int target;
    int key;

    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    which = rand_mod(4);
    sym1 = rand_mod(20);
    sym0 = rand_mod(7);
    while (sym0 == sym1) {
        sym0++;
        sym0 %= 7;
    }
    sym2 = rand_mod(7) + 7;
    while (sym2 == sym1) {
        sym2++;
        if (sym2 >= 14)
            sym2 = 7;
    }
    sym3 = rand_mod(6) + 14;
    while (sym3 == sym1) {
        sym3++;
        if (sym3 >= 20)
            sym3 = 14;
    }
    switch (which) {
    case 0: target = sym0; break;
    case 1: target = sym1; break;
    case 2: target = sym2; break;
    default: target = sym3; break;
    }
    back_win_colour = 14;
    back_win_font = 1;
    screen_win_font = 1;
    pic_draw_pal(1, "pics\\bkgdalcm.pic", cur_palette);
    sub_root_047c_0545();
    strcpy(path, "pics\\copy");
    _itoa(sym0, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd4 = pic_load(path);
    strcpy(path, "pics\\copy");
    _itoa(sym1, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd6 = pic_load(path);
    strcpy(path, "pics\\copy");
    _itoa(sym2, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dd8 = pic_load(path);
    strcpy(path, "pics\\copy");
    _itoa(sym3, num, 10);
    strcat(path, num);
    strcat(path, ".pic");
    D_8dda = pic_load(path);
    sub_root_047c_04f8();
    gfx_draw_pic(back_win, 0x68, 0x3f, D_8dd4);
    gfx_draw_pic(back_win, 0x68, 0x72, D_8dd6);
    gfx_draw_pic(back_win, 0xde, 0x3f, D_8dd8);
    gfx_draw_pic(back_win, 0xde, 0x72, D_8dda);
    gfx_text(back_win, 0x50, 0x14, "SELECT THE SYMBOL FOR");
    draw_text_far(back_win, 0x82, 0x1e, F_290e_1d73[target]);
    sub_root_047c_0568();
    set_palette(cur_palette);
    party_panel_frames(0x100);
    party_panel_rows(0x100);
    mouse_hide();
    gfx_copy_rect(back_win, 0x3c, 0, 0x104, 0xc8, screen_win, 0x3c, 0);
    mouse_show();
    sub_root_047c_056f();
    sub_root_047c_1320(6, 0, 0, 0);
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if ((mouse_buttons & 1) && mouse_x > 100 && mouse_x < 200 && mouse_y > 60 && mouse_y < 100) {
            sym3 = 0;
            while (mouse_buttons & 1)
                ;
            if (which == 0)
                break;
            copyprot_fail();
        } else if ((mouse_buttons & 1) && mouse_x > 220 && mouse_x < 300 && mouse_y > 60 && mouse_y < 100) {
            sym3 = 0;
            while (mouse_buttons & 1)
                ;
            if (which == 2)
                break;
            copyprot_fail();
        } else if ((mouse_buttons & 1) && mouse_x > 100 && mouse_x < 200 && mouse_y > 110 && mouse_y < 180) {
            sym3 = 0;
            while (mouse_buttons & 1)
                ;
            if (which == 1)
                break;
            copyprot_fail();
        } else if ((mouse_buttons & 1) && mouse_x > 220 && mouse_x < 300 && mouse_y > 110 && mouse_y < 180) {
            sym3 = 0;
            while (mouse_buttons & 1)
                ;
            if (which == 3)
                break;
            copyprot_fail();
        }
        if (key_pressed())
            key = get_key();
        if (key) {
            copyprot_key(key, which);
            break;
        }
    }
    copyprot_close(D_8dd4);
}

/* Free the screen and restore the saved display state. */
int far copyprot_close(int pic)
{
    mem_free(pic, "b");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    while (mouse_buttons & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

/* Number keys 1-4 (top row or keypad) choose a picture; Alt-Q quits. */
void far copyprot_key(int key, int which)
{
    switch (key) {
    case 0x231: case 0x4f31:
        if (which != 0)
            copyprot_fail();
        break;
    case 0x332: case 0x5032:
        if (which != 1)
            copyprot_fail();
        break;
    case 0x433: case 0x5133:
        if (which != 2)
            copyprot_fail();
        break;
    case 0x534: case 0x4b34:
        if (which != 3)
            copyprot_fail();
        break;
    case 0x635: case 0x4c35:
        break;
    case 0x1000:
        copyprot_close(D_8dd4);
        quit_to_dos();
    case 0x4800: case 0x4b00: case 0x4d00: case 0x5000:
        copyprot_fail();
        break;
    default:
        copyprot_fail();
        break;
    }
}

/* Wrong answer: shut everything down and exit to DOS. */
void far copyprot_fail(void)
{
    sub_root_044a_01f4();
    sub_root_047c_0791();
    vec_186f();
    game_shutdown();
    if (D_907d == 1)
        sub_root_05f8_0139(ems_frame_handle);
    video_restore_vec();
    exit_(0);
}
