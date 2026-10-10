/* place: s038 172a:0008 */
/* orig: size=0102 _DATA=4b98+001e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Fame screen: shows pics\fame.pic, reads darkland.fam (0x208 bytes) */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
extern char screen_win[];
extern int screen_win_font;
extern int back_win_colour;
extern int back_win_font;
extern char mouse_buttons;
extern char cur_palette[];
extern int D_e7a0;
extern int D_e7a2;
int far fclose();
int far fopen();
int far fread();
int far gfx_copy_rect();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far key_pressed();
int far get_key();
int far mouse_show();
int far mouse_hide();
int far sub_root_047c_1320();
int far pic_draw_pal();
void far sub_s038_172a_0008();
int far sub_s038_172a_0058();
int far sub_s038_172a_007e();
void far sub_s038_172a_00aa();

void far sub_s038_172a_0008(void)
{
    sub_root_047c_0568();
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_s038_172a_007e();
    sub_s038_172a_00aa();
    sub_root_047c_056f();
    do {
    } while (mouse_buttons & 1);
    do {
    } while (!(mouse_buttons & 3) && !key_pressed());
    do {
    } while (mouse_buttons & 1);
    if (key_pressed()) {
        get_key();
    }
    sub_s038_172a_0058();
}

int far sub_s038_172a_0058(void)
{
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    do {
    } while (mouse_buttons & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s038_172a_007e(void)
{
    pic_draw_pal(1, "pics\\fame.pic", cur_palette);
    set_palette(cur_palette);
    back_win_colour = 0;
    back_win_font = 1;
    return screen_win_font = 1;
}

void far sub_s038_172a_00aa(void)
{
    char buf[0x208];
    int fp;
    fread(buf, 0x208, 1, fp = fopen("darkland.fam", "rb"));
    fclose(fp);
    back_win_colour = 0;
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    mouse_show();
}
