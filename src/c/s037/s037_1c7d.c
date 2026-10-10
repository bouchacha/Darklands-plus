/* place: s037 1c7d:0002 */
/* orig: size=1a96 CONST=7ec4+000e _DATA=479e+01c3 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (m0, 2026-10-09).
   0002: no `k = key` copy; key itself is used after the mouse part.
   030a: the ids loop is a user loop over a byte offset and a member pointer held in one
         struct (`struct { char *p; int k; } s;` puts p at [bp-4], k at [bp-2]; as separate
         locals 6.00A swaps them whatever the names/declaration order), k is address-taken by
         a volatile read in the ids test so `s.k = 0x10; f(.., s.k, 0x12, s.k)` keeps the 6.00
         constant-temp store; the handle is `int h[2]` (h[1] = the original's unused [bp-6])
         and n/nr share `int nn[2]`.
   09c6: q = &F_2e38_1ffc kept in its slot by a dead `&q;`, the free_buffer argument read
         through `**(unsigned far * volatile *)&q` (gives push es:[bx]), and the clears as
         separate statements `D_2870 = 0; *q = 0; D_e416 = 0;`.
   Load-game screen (pics\loadscrn.pic): list of saves\*.sav, delete,
   load (0d24 reads the whole save file).

   Save file layout (saves\NAME.sav, read by 0d24):
     char name[0x15]; char desc[0x4f]; int seed (D_9c4a); int ?; int year, month, day, hour
     (D_00e6, D_00e4, D_00e2, D_00e0); int D_906b, D_906d, D_906f; long ?; int fame (D_9082);
     int city (D_907e); D_e3d0, D_e3d2, D_a772, D_e898, D_e7d8, D_e896, D_a88d, D_9072..D_9078,
     D_a67c (ints); char D_906a (bit 7 -> D_9081 bit 0); int D_db27, D_da34; char D_c624[6];
     char D_907b, D_01bf (sound), D_01be (music); int D_a891; char ?; 36 unused ints;
     int D_a67e; int nchars; int ids[5] (party slots); char D_a668[20]; char tab[5][24];
     nchars x 0x22a-byte character records (0x80 bytes member record + far tables
     F_2e38_082c[0x14], F_2e38_07be[0x16], F_2e38_003e[0x180]);
     int nevents; nevents x 0x30-byte events; 0x5dce bytes at far D_a885; then cache.tmp. */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
extern int D_8c50;              /* cursor row */
struct font { char _p0[0xc]; int color; };
struct win { char _p0[0x10]; int f10; };
struct find_t { char reserved[21]; char attrib; unsigned wr_time; unsigned wr_date; long size; char name[13]; };
struct rect { int x0; int y0; int x1; int y1; int _8; };
typedef struct { char _p[16]; } FILE;
#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned far *)&(fp)))

extern char cur_hour[], cur_day[], cur_month[], cur_year[];
extern char opt_sound, opt_music;
extern char D_07aa, D_07bc;
extern char D_07d8;
extern int card_window2;
extern int card_window;
extern unsigned D_2870;
extern char screen_win[];
extern int screen_win_colour;
extern int D_8c4c;              /* number of saved games */
extern int screen_win_font;
extern int back_win_colour;
extern int back_win_font;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern int D_8c52;              /* first save shown */
extern int D_9072[], D_9074[], D_9076[], D_9078[];
extern unsigned combat_flags;
extern char D_9081;
extern int party_fame[];
extern char msg_buf[];
extern char game_difficulty;
extern char party_money[], party_money_gr[], party_money_pf[];
extern char D_907a;
extern char leader[];
extern int cur_loc;
extern int D_9417;
extern char D_9429, D_9433;
extern char far *city_rec;
extern char city_desc[];
extern char D_987f;
extern char D_9920, D_9934;
extern char *D_996e, *D_9982;
extern int cursor_default;
extern char party_seed[];
extern char combatants[];
extern char party[][0x80];
extern char member_shield[];
extern unsigned shield_pics[];
extern char D_a67c[];
extern char party_size[];
extern char D_a680[];
extern int cursor_current;
extern char D_a752[];
extern char D_a772[];
extern unsigned D_a776;
extern int D_a86e, D_a870, D_a872, D_a874;
extern char D_a876;
extern unsigned char far *loc_buf;
extern char prev_screen[];
extern int D_a891;
extern char cbt_floor[];
extern char move_mode;
extern char marching_order[];
extern unsigned anim_buffers[];
extern char D_da34[];
extern char D_db27[];
extern unsigned D_d6ee, D_d6f2, D_d6f9, D_d70a, D_d726, D_d729, D_d73c, D_d800;
extern char party_x[], party_y[];
extern unsigned D_e416;
extern char cur_palette[];
extern int D_e7a0;
extern int D_e7a2;
extern char saved_card[];
extern int D_e88a;
extern char saved_card2[], D_e898[];
extern char D_e89a;
extern struct { char _p[0x1dd3]; char far *s[3]; } far F_290e_0000;
extern unsigned far F_20a5_4d62;
extern char far F_20a5_763b[2];
extern unsigned far F_2e38_1ffc;
extern char far * far events[0x12c];
extern char far * far locations[];
extern char far member_palette[5][0x18];
extern char far F_2e38_3b2e[][15];
extern char far saints_known[][0x14];
extern char far formulas_known[][0x16];
extern char far inventories[][0x180];
int far _dos_close();
unsigned far _dos_findfirst(char *, unsigned, struct find_t *);
int far _dos_findnext();
int far _dos_read();
int far _dos_freemem();
int far _fstrcmp();
int far _fstrcpy();
int far _fstrcat();
int far sprintf();
int far printf();
void far exit_(int);
extern int D_8c54;              /* rows shown */
int far fopen();
int far fdopen();
int far fread();
int far fwrite();
int far fclose();
int far fseek();
long far ftell();
int far fgetc();
int far remove();
int far fcloseall();
int far longjmp();
unsigned far alloc_buffer();
void far free_buffer();
int far copy_rect_saved();
char *strcat();
int far quit_to_dos();
int far flush_keys();
int far show_difficulty();
int far text_box();
int far ask_yes_no();
int far datafile_forget();
void far F_0000_2652(int);
int far draw_text_far();
int far sub_root_047c_0066();
int far sub_root_047c_00a9();
int far sub_root_047c_040b();
int far gfx_recolor_rect();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far gfx_copy_rect();
int far gfx_text();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far gfx_draw_pic();
int far key_pressed();
int far get_key();
int far sub_root_047c_0783();
int far sub_root_047c_0791();
int far sub_root_047c_07ce();
int far sub_root_047c_08c8();
int far sub_root_047c_08f9();
int far mouse_set_cursor();
int far mouse_show();
int far mouse_hide();
int far sub_root_047c_1320();
int far mem_free();
int far pic_draw_pal();
int far pic_load();
int far sub_root_06a1_2c9e();
int far clamp_range();
int far load_saved_game();
int far order_menu_reset_vec();
int far vec_1905();
int far cards_startup_vec();
int far battle_restore_vec();
int far enemy_anim_free5_vec();
int far sub_s037_1c7d_0002();
int far sub_s037_1c7d_0218();
int far sub_s037_1c7d_024e();
void far sub_s037_1c7d_02ce();
int far sub_s037_1c7d_030a();
void far sub_s037_1c7d_06e6();
void far sub_s037_1c7d_095e();
void far load_game();
void far sub_s037_1c7d_0d24();
void far sub_s037_1c7d_19c6();
int far sub_s037_1c7d_1a60();

char D_479e[] = "Loading Game";

int far sub_s037_1c7d_0002(void)
{
    int key;
    int last;
    int row;
    last = -1;
    sub_root_047c_0568();
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_s037_1c7d_024e();
    sub_s037_1c7d_02ce();
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if (!(mouse_buttons & 1) && mouse_x > 0x44 && mouse_x < 0xc6 && mouse_y > 0x2d && mouse_y < 0x5a) {
            row = (mouse_y - 0x2d) / 15;
            if (row != last) {
                mouse_hide();
                if (last != -1)
                    gfx_recolor_rect(screen_win, 0x45, last * 15 + 0x2f, 0x80, 8, 9, 8);
                last = row;
                gfx_recolor_rect(screen_win, 0x45, row * 15 + 0x2f, 0x80, 8, 8, 9);
                mouse_show();
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0x44 && mouse_x < 0xc6 && mouse_y > 0x2d && mouse_y < 0x5a) {
            while (mouse_buttons & 1)
                ;
            switch (row = (mouse_y - 0x2d) / 15) {
            case 0:
                key = 0x264c;
                break;
            case 1:
                key = 0x2044;
                break;
            case 2:
                if (D_e89a == 1)
                    key = 0x11b;
                break;
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0xc3 && mouse_x < 0x140 && mouse_y > 0x14 && mouse_y < 0xa5) {
            while (mouse_buttons & 1)
                ;
            row = (mouse_y - 0x14) / 16;
            if (D_8c54 > row) {
                mouse_hide();
                gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x12, 0x10);
                D_8c50 = row;
                gfx_recolor_rect(screen_win, 0xc4, (row << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
                mouse_show();
            }
        }
        if (key_pressed())
            key = get_key();
        if (D_e89a == 1) {
            if (key == 0x11b || key == 0x1352 || key == 0x1372)
                return sub_s037_1c7d_0218(0);
        } else if (key == 0x11b)
            return sub_s037_1c7d_0218(0);
        if (key && sub_s037_1c7d_030a(key) == 0x11b)
            return sub_s037_1c7d_0218(1);
    }
}

int far sub_s037_1c7d_0218(int p_6)
{
    mem_free(D_e88a, "b");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    while (mouse_buttons & 1)
        ;
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s037_1c7d_024e(void)
{
    sub_root_047c_040b(1);
    sub_root_047c_040b(2);
    sub_root_047c_040b(3);
    sub_root_047c_040b(4);
    D_07d8 = 0;
    pic_draw_pal(1, "pics\\loadscrn.pic", cur_palette);
    sub_root_047c_0545();
    D_e88a = pic_load("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    set_palette(cur_palette);
    back_win_colour = 8;
    back_win_font = 1;
    screen_win_font = 1;
    D_8c4c = sub_s037_1c7d_1a60();
    D_8c52 = 0;
    return D_8c50 = 0;
}

void far sub_s037_1c7d_02ce(void)
{
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    sub_s037_1c7d_095e(screen_win);
    sub_s037_1c7d_06e6(screen_win);
    mouse_show();
}

/* button/key handler: 0x1000 quit, 0x2044/0x2064 delete, 0x264c/0x266c load,
   0x4700 home, 0x4800/0x4b00 up, 0x4d00/0x5000 down, 0x4f00 end */
int far sub_s037_1c7d_030a(int key)
{
    struct { char *p; int k; } s;
    int h[2];
    int nn[2];
    int ids[5];
    char name[20];
    switch (key) {
    case 0x1000:
        sub_s037_1c7d_0218(0);
        quit_to_dos();
        break;
    case 0x264c:
    case 0x266c:
        if (D_8c54 > D_8c50) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x42, 0x2c, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x52, 0x30, F_290e_0000.s[0]);
            sub_s037_1c7d_0d24((char far *)F_2e38_3b2e[D_8c50]);
            sub_root_06a1_2c9e("charactr.tmp", 0, h);
            _dos_read(h[0], (int far *)&nn[0], 2, &nn[1]);
            _dos_read(h[0], (int far *)ids, 10, &nn[1]);
            _dos_close(h[0]);
            for (s.p = &party[0][1], s.k = 0; s.k < 10; s.p += 0x80, s.k += 2) {
                *(unsigned *)((char *)shield_pics + s.k) = 0;
                if (*(int *)((char *)ids + *(volatile int *)&s.k) != -1) {
                    sub_root_047c_0545();
                    strcpy(name, "pics\\");
                    strcat(name, "shield");
                    name[11] = *s.p;
                    name[12] = 0;
                    strcat(name, ".pic");
                    *(unsigned *)((char *)shield_pics + s.k) = pic_load(name);
                    sub_root_047c_04f8();
                }
            }
            mouse_show();
            F_0000_2652(0x35);
            return 0x11b;
        }
        break;
    case 0x2044:
    case 0x2064:
        mouse_hide();
        gfx_recolor_rect(screen_win, 0x45, 0x2f, 0x80, 0x2d, 9, 8);
        gfx_draw_pic(screen_win, 0x41, 0x3b, D_e88a);
        screen_win_colour = 8;
        draw_text_far(screen_win, 0x52, 0x3f, F_290e_0000.s[1]);
        if (ask_yes_no("Delete this Game?")) {
            mouse_hide();
            sub_s037_1c7d_19c6((char far *)F_2e38_3b2e[D_8c50]);
            mouse_show();
            sub_s037_1c7d_06e6(screen_win);
        }
        mouse_hide();
        gfx_copy_rect(0x2806, 0x41, 0x3b, 0x82, 0xe, screen_win, 0x41, 0x3b);
        screen_win_colour = 8;
        draw_text_far(screen_win, 0x52, 0x3f, F_290e_0000.s[1]);
        gfx_recolor_rect(screen_win, 0x45, 0x3e, 0x80, 8, 8, 9);
        mouse_show();
        break;
    case 0x4700:
        D_8c50 = D_8c52 = 0;
        sub_s037_1c7d_06e6(screen_win);
        break;
    case 0x4800:
    case 0x4b00:
        s.k = 0x10;
        gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, s.k, 0x12, s.k);
        if (--D_8c50 < 0) {
            if (D_8c52) {
                D_8c52--;
                D_8c50 = 0;
                sub_s037_1c7d_06e6(screen_win);
            } else
                D_8c50 = 0;
        }
        gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
        break;
    case 0x4d00:
    case 0x5000:
        s.k = 0x10;
        gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, s.k, 0x12, s.k);
        if (++D_8c50 >= D_8c54) {
            if (D_8c4c - 9 < ++D_8c52) {
                D_8c50--;
                D_8c52--;
            } else {
                D_8c50--;
                sub_s037_1c7d_06e6(screen_win);
            }
        }
        gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
        break;
    case 0x4f00:
        D_8c52 = clamp_range(0, 9999, D_8c4c - 9);
        D_8c50 = clamp_range(0, D_8c4c, 8);
        sub_s037_1c7d_06e6(screen_win);
        break;
    }
}

/* list of saved games: "name day/month/year  fame" and the description */
void far sub_s037_1c7d_06e6(struct font *f)
{
    int skip;
    int h;
    int nr;
    int year, month, day, hour, seed, dummy;
    char name[0x15];
    char desc[0x4f];
    char path[0x1e];
    struct find_t ff;
    char line[0x52];
    mouse_hide();
    gfx_copy_rect(0x2806, 0xc3, 0x14, 0x7d, 0x91, screen_win, 0xc3, 0x14);
    D_8c54 = skip = 0;
    if (_dos_findfirst("saves\\*.sav", 0, &ff))
        return;
    do {
        if (skip++ >= D_8c52) {
            if (++D_8c54 > 9) {
                D_8c54--;
                break;
            }
            strcpy(path, "saves\\");
            strcat(path, ff.name);
            _fstrcpy((char far *)F_2e38_3b2e[D_8c54 - 1], (char far *)ff.name);
            sub_root_06a1_2c9e(path, 0, &h);
            _dos_read(h, (char far *)name, 0x15, &nr);
            _dos_read(h, (char far *)desc, 0x4f, &nr);
            _dos_read(h, (int far *)&seed, 2, &nr);
            _dos_read(h, (int far *)&dummy, 2, &nr);
            _dos_read(h, (int far *)&year, 2, &nr);
            _dos_read(h, (int far *)&month, 2, &nr);
            _dos_read(h, (int far *)&day, 2, &nr);
            _dos_read(h, (int far *)&hour, 2, &nr);
            _dos_close(h);
            sprintf(line, "%s %d/%d/%d  %d", name, year, month + 1, day, hour);
            f->color = 0x11;
            gfx_text(f, 0xc6, (D_8c54 << 4) + 7, line);
            gfx_text(f, 0xc6, (D_8c54 << 4) + 0xf, desc);
            f->color = 0x10;
            gfx_text(f, 0xc5, (D_8c54 << 4) + 6, line);
            gfx_text(f, 0xc5, (D_8c54 << 4) + 0xe, desc);
        }
    } while (!_dos_findnext(&ff));
    if (D_8c4c <= D_8c50)
        D_8c50 = D_8c54 - 1;
    gfx_recolor_rect(screen_win, 0xc4, (D_8c50 << 4) + 0x16, 0x7b, 0x10, 0x10, 0x12);
    mouse_show();
}

void far sub_s037_1c7d_095e(struct font *f)
{
    f->color = 8;
    draw_text_far(f, 0x52, 0x2f, F_290e_0000.s[0]);
    draw_text_far(f, 0x52, 0x3e, F_290e_0000.s[1]);
    if (D_e89a == 1)
        draw_text_far(f, 0x52, 0x4d, F_290e_0000.s[2]);
}

/* load: confirm, tear down the running game (free buffers), then reload and longjmp
   back into the main loop */
void far load_game(void)
{
    int i;
    unsigned far *q;
    ((struct win *)card_window)->f10 = 3;
    if (!ask_yes_no("Loading is irreversable. Proceed?"))
        return;
    vec_1905("load", "a");
    if (D_907a == 1) {
        mouse_hide();
        mouse_set_cursor(0, 0, cursor_default);
        cursor_current = cursor_default;
        datafile_forget();
        for (i = 0; i < 5; i++) {
            if (shield_pics[i]) {
                free_buffer(shield_pics[i], "SGS");
                shield_pics[i] = 0;
            }
        }
        *(long *)msg_buf = *(long *)"Seq";
        for (i = 0; i < 5; i++) {
            if (anim_buffers[i]) {
                free_buffer(anim_buffers[i], msg_buf);
                anim_buffers[i] = 0;
            }
        }
        free_buffer(F_20a5_4d62, "MiscSp");
        if (D_d6ee) {
            free_buffer(D_d6ee, "VarFrn");
            D_d6ee = 0;
        }
        if (D_d6f2)
            free_buffer(D_d6f2, "Frn");
        if (D_d6f9)
            free_buffer(D_d6f9, "FLR");
        if (D_d729)
            free_buffer(D_d729, "FFX");
        if (D_d70a)
            free_buffer(D_d70a, "NWL");
        if (D_d73c)
            free_buffer(D_d73c, "NFX");
        if (D_d726)
            free_buffer(D_d726, "WWL");
        if (D_d800)
            free_buffer(D_d800, "WFX");
        enemy_anim_free5_vec();
        F_20a5_763b[0] = 'D';
        F_20a5_763b[1] = 'G';
        for (i = 0; i < 5; i++) {
            if (party[i][0] == 1 || party[i][0] == 2) {
                *(int *)&party[i][-4] = 0;
                party[i][0x36] = -1;
                cbt_floor[i] = 0;
            }
        }
        D_907a = 0;
        D_9429 = 2;
        D_9433 = 2;
        D_9920 = 1;
        D_9934 = 1;
        D_987f = 1;
        combat_flags &= 0xfe10;
        flush_keys();
        D_07aa = 2;
        D_07bc = 2;
        move_mode = 0;
        order_menu_reset_vec();
        if (D_a876 != 'n' && D_a876 != 'N') {
            sub_root_047c_0791();
            sub_root_047c_00a9();
            sub_root_047c_08f9(D_9417);
            sprintf(msg_buf, "%csound.dlc", D_a876);
            sub_root_047c_08c8(D_9417 = sub_root_047c_07ce(msg_buf, 0));
            sub_root_047c_0783(0, 0, 0, D_a870, D_a86e, D_a872, D_a874);
            sub_root_047c_0066();
        }
        cards_startup_vec(0);
    }
    if (D_a776)
        free_buffer(D_a776, "b");
    D_a776 = 0;
    for (i = 0; i < 5; i++) {
        if (shield_pics[i])
            free_buffer(shield_pics[i], "sg");
        party[i][0] = 0;
    }
    if (D_a891 == 3) {
        fcloseall();
        if (D_2870)
            free_buffer(D_2870, "Map");
        q = &F_2e38_1ffc;
        &q;
        if (*q)
            free_buffer(**(unsigned far * volatile *)&q, "S");
        if (D_e416)
            _dos_freemem(D_e416);
        D_2870 = 0;
        *q = 0;
        D_e416 = 0;
    }
    for (i = 0; i < 0x12c; i++) {
        if (events[i]) {
            free_buffer(FP_SEG(events[i]), "event");
            events[i] = 0;
        }
    }
    sub_s037_1c7d_0002();
    longjmp(D_a752, -1);
}

char D_4919[0x30] = {0};

void far sub_s037_1c7d_0d24(char far *file)
{
    char onoffg[6];
    int dummyc;
    long dummy4o;
    char tabh[5][0x18];
    int idso[5];
    char flage;
    struct find_t ffc;
    FILE *fi;
    int je;
    char far *p;
    int zeroa;
    int hj;
    int ncharsd;
    long posb;
    char pathh[26];
    char namee[0x15];
    char desch[0x52];
    int nrh;
    char cf;
    FILE *f2b;
    int dummy2m[6];
    struct rect rj;
    int id;
    int n;

    flage = 0;
    zeroa = 0;
    _fstrcpy((char far *)D_a680, file);
    strcpy(pathh, "saves\\");
    _fstrcat((char far *)pathh, file);
    if (_dos_findfirst(pathh, 0, &ffc)) {
        printf("load");
        exit_(1);
    } else {
        sub_root_06a1_2c9e(pathh, 0, &hj);
        fi = fdopen(hj, "rb");
        fread(namee, 1, 0x15, fi);
        fread(desch, 1, 0x4f, fi);
        fread(party_seed, 2, 1, fi);
        fread(dummy2m, 2, 1, fi);
        fread(cur_year, 2, 1, fi);
        fread(cur_month, 2, 1, fi);
        fread(cur_day, 2, 1, fi);
        fread(cur_hour, 2, 1, fi);
        fread(party_money, 2, 1, fi);
        fread(party_money_gr, 2, 1, fi);
        fread(party_money_pf, 2, 1, fi);
        fread(&dummy4o, 4, 1, fi);
        fread(party_fame, 2, 1, fi);
        fread(&cur_loc, 2, 1, fi);
        fread(party_x, 2, 1, fi);
        fread(party_y, 2, 1, fi);
        fread(D_a772, 2, 1, fi);
        fread(D_e898, 2, 1, fi);
        fread(saved_card, 2, 1, fi);
        fread(saved_card2, 2, 1, fi);
        fread(prev_screen, 2, 1, fi);
        fread(D_9072, 2, 1, fi);
        fread(D_9074, 2, 1, fi);
        fread(D_9076, 2, 1, fi);
        fread(D_9078, 2, 1, fi);
        fread(D_a67c, 2, 1, fi);
        fread(&game_difficulty, 1, 1, fi);
        fread(D_db27, 2, 1, fi);
        fread(D_da34, 2, 1, fi);
        fread(marching_order, 6, 1, fi);
        fread(leader, 1, 1, fi);
        fread(&opt_music, 1, 1, fi);
        fread(&opt_sound, 1, 1, fi);
        fread(&D_a891, 2, 1, fi);
        fread(&flage, 1, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        fread(&dummyc, 2, 1, fi);
        if (game_difficulty & 0x80) {
            D_9081 |= 1;
            game_difficulty &= 0x7f;
        } else
            D_9081 &= 0xfe;
        show_difficulty(1);
        mouse_hide();
        if (D_907a == 1) {
            D_907a = 0;
            text_box(card_window, -1, 1, D_479e, card_window2, &rj);
            D_907a = 1;
        } else
            text_box(card_window, -1, 1, D_479e, card_window2, &rj);
        mouse_show();
        if (opt_music == 1) {
            strcat(onoffg, "ON");
            *D_996e = 1;
        } else {
            strcat(onoffg, "OFF");
            *D_996e = 2;
        }
        if (opt_sound == 1) {
            strcat(onoffg, "ON");
            *D_9982 = 1;
        } else {
            strcat(onoffg, "OFF");
            *D_9982 = 2;
        }
        fread(party_size, 2, 1, fi);
        fread(&ncharsd, 2, 1, fi);
        fread(idso, 10, 1, fi);
        fread(member_shield, 0x14, 1, fi);
        fread(tabh, 0x78, 1, fi);
        posb = ftell(fi);
        for (id = 0; id < ncharsd; id++) {
            for (je = 0; je < 5; je++)
                if (idso[je] == id)
                    break;
            if (je < 5) {
                fseek(fi, posb, 0);
                fseek(fi, (long)(unsigned)(idso[je] * 0x22a), 1);
                fread(&combatants[je << 7], 0x80, 1, fi);
                fseek(fi, 0L, 1);
                _dos_read(hj, (char far *)saints_known[je], 0x14, &nrh);
                _dos_read(hj, (char far *)formulas_known[je], 0x16, &nrh);
                _dos_read(hj, (char far *)inventories[je], 0x180, &nrh);
                if (party[je][1] < 'A')
                    party[je][1] = 'A';
            } else {
                fseek(fi, 0L, 1);
                _dos_read(hj, (char far *)msg_buf, 0x22a, &nrh);
            }
        }
        for (id = 0; id < 5; id++)
            for (je = 0; je < 0x18; je++)
                member_palette[id][je] = tabh[id][je];
        fread(&n, 2, 1, fi);
        for (id = 0; id < n; id++) {
            events[id] = (char far *)((unsigned long)alloc_buffer(0x30L, 0) << 16);
            fseek(fi, 0L, 1);
            _dos_read(hj, events[id], 0x30, &nrh);
        }
        fseek(fi, 0L, 1);
        _dos_read(hj, loc_buf, 0x5dce, &nrh);
        id = loc_buf[0];
        n = loc_buf[1];
        p = loc_buf + 1;
        p++;
        n = (n << 8) + id;
        for (id = 0; id < n; id++) {
            locations[id] = p;
            p += 0x3a;
        }
        load_saved_game(file);
        if (_dos_findfirst("cache.tmp", 0, &ffc)) {
            printf("cache");
            exit_(1);
        } else {
            fseek(fi, 0L, 1);
            f2b = fopen("cache.tmp", "wb");
            while (fread(&cf, 1, 1, fi))
                fwrite(&cf, 1, 1, f2b);
            fclose(f2b);
        }
        if (_dos_findfirst("darkland.dsc", 0, &ffc)) {
            printf("city");
            exit_(1);
        } else {
            f2b = fopen("darkland.dsc", "r");
            fgetc(f2b);
            fseek(f2b, (long)(cur_loc * 0x50 + 1), 0);
            fread(city_desc, 0x50, 1, f2b);
            fclose(f2b);
        }
        if (_dos_findfirst("darkland.cty", 0, &ffc)) {
            printf("city");
            exit_(1);
        } else {
            sub_root_06a1_2c9e("darkland.cty", 0, &hj);
            f2b = fdopen(hj, "r");
            fseek(f2b, (long)cur_loc * 0x26e + 1, 0);
            _dos_read(hj, city_rec, 0x26e, &id);
            fclose(f2b);
        }
        fclose(fi);
    }
    battle_restore_vec(D_a680);
    mouse_hide();
    if (D_907a == 1) {
        D_907a = 0;
        copy_rect_saved(card_window2, rj.x0, rj.y0, rj.x1, rj.y1, card_window, rj.x0, rj.y0);
        D_907a = 1;
    } else
        copy_rect_saved(card_window2, rj.x0, rj.y0, rj.x1, rj.y1, card_window, rj.x0, rj.y0);
    mouse_show();
}

/* delete a saved game (NAME.sav and NAME.bsv) unless it is the one loaded */
void far sub_s037_1c7d_19c6(char far *file)
{
    char path[26];
    if (_fstrcmp((char far *)D_a680, file)) {
        strcpy(path, "saves\\");
        _fstrcat((char far *)path, file);
        remove(path);
        path[strlen(path) - 3] = 0;
        strcat(path, "bsv");
        remove(path);
        D_8c4c--;
    }
}

int far sub_s037_1c7d_1a60(void)
{
    struct find_t ff;
    int n;
    n = 0;
    if (_dos_findfirst("saves\\*.sav", 0, &ff))
        return 0;
    do {
        n++;
    } while (!_dos_findnext(&ff));
    return n;
}
