/* place: s037 1e26:0008 */
/* orig: size=03f0 _DATA=4962+00a3 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Start screen (pics\startscr.pic, 4 buttons): quit / default party / load / create. */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
extern char party[][0x80];     /* party member records, 128 bytes */
extern int party_seed;              /* party random seed */
extern char D_9e69;
extern int shield_pics[];            /* shield picture handles, one per member */
extern int D_a2df;
extern int D_a67c;
extern int party_size;
extern char member_shield[];
extern char D_e89a;
int far quit_to_dos();
int far sub_root_06a1_2c9e();   /* _dos_open */
int far _dos_read();
int far _dos_write();
int far _dos_close();
int far load_saved_game(char far *);
int far sub_s037_1462_0000();
int far sub_s037_1c7d_0002();
int far party_default_vec();
char *strcpy();
char *strcat();
extern char screen_win[];
extern int screen_win_font;
extern int back_win_colour;
extern int back_win_font;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern int D_8cfe;
extern int D_8d00;
extern int D_8d02;
extern int D_8d04;
extern char cur_palette[];
extern int D_e7a0;
extern int D_e7a2;
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
void far sub_s037_1e26_0008();
int far sub_s037_1e26_00a6();
int far sub_s037_1e26_00dc();
void far sub_s037_1e26_014a();
int far sub_s037_1e26_0172();

void far sub_s037_1e26_0008(void)
{
    int v_si;
    sub_root_047c_0568();
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_s037_1e26_00dc();
    sub_s037_1e26_014a();
    sub_root_047c_056f();
    do {
    } while (mouse_buttons & 1);
    do {
        v_si = 0;
        if (mouse_buttons & 1 && mouse_y > 0x32 && mouse_y < 0x96) {
            do {
            } while (mouse_buttons & 1);
            switch (v_si = (mouse_y - 0x32) / 0x20) {
            default:
                v_si = 0;
                break;
            case 0:
                v_si = 0x1051;
                break;
            case 1:
                v_si = 0x2e43;
                break;
            case 2:
                v_si = 0x1454;
            }
        }
        if (!key_pressed()) {
            continue;
        }
        v_si = get_key();
    } while (!v_si || sub_s037_1e26_0172(v_si) != 0x11b);
    sub_s037_1e26_00a6();
}

int far sub_s037_1e26_00a6(void)
{
    mem_free(D_8cfe, "b");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    do {
    } while (mouse_buttons & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s037_1e26_00dc(void)
{
    pic_draw_pal(1, "pics\\startscr.pic", cur_palette);
    sub_root_047c_0545();
    D_8cfe = pic_load("pics\\button1.pic");
    D_8d00 = pic_load("pics\\button2.pic");
    D_8d02 = pic_load("pics\\button3.pic");
    D_8d04 = pic_load("pics\\button4.pic");
    sub_root_047c_04f8();
    set_palette(cur_palette);
    back_win_colour = 8;
    back_win_font = 1;
    return screen_win_font = 1;
}

void far sub_s037_1e26_014a(void)
{
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    mouse_show();
}

/* start screen buttons: 0x1000 quit, 0x1454 load a saved game, 0x1051 default party,
   0x2e43 create characters. Returns 0x11b when the game should start. */
int far sub_s037_1e26_0172(int key)
{
    int i;
    int h;
    int nr;
    int ids[5];
    char name[20];
    switch (key) {
    case 0x1000:
        sub_s037_1e26_00a6();
        quit_to_dos();
        break;
    case 0x1454:
    case 0x1474:
        mouse_hide();
        gfx_draw_pic(screen_win, 0x51, 0x78, D_8d02);
        mouse_show();
        D_e89a = 1;
        if (sub_s037_1c7d_0002()) {
            D_e89a = 0;
            return 0x11b;
        }
        D_e89a = 0;
        sub_root_047c_0568();
        pic_draw_pal(1, "pics\\startscr.pic", cur_palette);
        set_palette(cur_palette);
        mouse_hide();
        gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
        mouse_show();
        sub_root_047c_056f();
        break;
    case 0x1051:
    case 0x1071:
        mouse_hide();
        gfx_draw_pic(screen_win, 0x51, 0x37, D_8cfe);
        mouse_show();
        party_seed = *(int far *)0x0040006cL;
        load_saved_game("default");
        party_default_vec();
        D_9e69 = 0;
        for (i = 0; i < 4; i++) {
            sub_root_047c_0545();
            strcpy(name, "pics\\");
            strcat(name, "shield");
            name[11] = party[i][1];
            name[12] = 0;
            strcat(name, ".pic");
            shield_pics[i] = pic_load(name);
            sub_root_047c_04f8();
        }
        *(volatile int *)&D_a2df = 0;
        D_a67c = 0;
        party_size = 4;
        sub_root_06a1_2c9e("charactr.tmp", 2, &h);
        _dos_read(h, (int far *)&i, 2, &nr);
        for (i = 0; i < 4; i++)
            ids[i] = i;
        ids[4] = -1;
        _dos_write(h, (int far *)ids, 10, &nr);
        _dos_write(h, (char far *)member_shield, 0x14, &nr);
        _dos_close(h);
        return 0x11b;
    case 0x2e43:
    case 0x2e63:
        mouse_hide();
        gfx_draw_pic(screen_win, 0x51, 0x57, D_8d00);
        mouse_show();
        party_seed = *(int far *)0x0040006cL;
        if (sub_s037_1462_0000())
            return 0x11b;
        sub_root_047c_0568();
        pic_draw_pal(1, "pics\\startscr.pic", cur_palette);
        set_palette(cur_palette);
        mouse_hide();
        gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
        mouse_show();
        sub_root_047c_056f();
        break;
    }
}
