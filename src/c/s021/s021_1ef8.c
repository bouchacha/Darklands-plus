/* place: s021 1ef8:0000 */
/* orig: size=08ec CONST=7dea+0008 _DATA=22ce+0089 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Combat overlay s021 (child of s014): skill improvement in combat and the
 * enemy information display.
 * 0000 (member, skill, mode): chance (100-v)/2 (mode 1) or /4 to raise a skill below 99,
 *      by f(D_22d2[difficulty]) + D_22ce[difficulty];
 * 0080 (member, skill, amount): "<skill> changed from X to Y" message, clamp 0..99;
 * 01e2 (row, color): show the D_9085 message box at a combatant row (-2 = whole panel);
 * 02ce: enemy info: up to two distinct enemy kinds among combatants 5..12;
 * 03fe (combatant, &pic, x): portrait "<a><b>st.pic" from einfo.cat, name, armour / weapon
 *      names and skill words, or "Natrl Armr"/"Natrl Weapn" for animals (pic name not 'E').
 * Combatant record (0x80 bytes, D_9c69[13]): +0 status, +2 kind, +0x0e weapon (-1 none),
 * +0x34 weapon quality flag, +0x37/+0x38 armour, +0x3d weapon/shield, +0x3f natural armour,
 * +0x41 armour flags, +0x48, +0x57 skills (+0x57 first, +0x5c sixth). */
#include "darklands/card.h"
#include "darklands/game.h"

struct pr { char kind; char idx; };
struct gfx { char pad[0xa]; int f0a; int f0c; char pad2[2]; int f10; };
struct mem {
    char status;   /* +00 */
    char pad1[1];
    char kind;   /* +02 */
    char pad3[11];
    char f0e;   /* +0e */
    char padf[37];
    char f34;   /* +34 */
    char pad35[2];
    char f37;   /* +37 */
    char f38;   /* +38 */
    char pad39[4];
    char f3d;   /* +3d */
    char pad3e[1];
    char f3f;   /* +3f */
    char pad40[1];
    char f41;   /* +41 */
    char pad42[6];
    char f48;   /* +48 */
    char pad49[14];
    char skill[0x29];   /* +57 */
};

char D_22ce[4] = { 0, 0, 0, 0 };
char D_22d2[3] = { 3, 4, 5 };

extern struct mem party[];
extern unsigned char game_difficulty;
extern char leader;
extern char msg_buf[];
extern struct gfx *side_win;
extern int screen_ptr;
extern int D_a626, D_a628;
extern char D_01bc;
extern volatile int mouse_buttons;
extern int D_9419;
extern char *D_1166[], *anim_file[];
extern char *quality_words[];
extern char enemy_class[];
extern char far anim_set_names[][0x14];
extern char far * far skill_names[];
extern char far * far F_286b_01f7[];
extern char far * far F_286b_01cf[];
extern char far * far F_286b_0277[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
char *_itoa();
void *memset(void *, int, unsigned);
char far *_fstrcpy(char far *, const char far *);
int far random_n(int);
int far rand_1_n(int);
int far sub_root_0410_001a(int, int, int);
void far mouse_hide(void);
void far mouse_show(void);
void far gfx_draw_pic(struct gfx *, int, int, int);
void far gfx_copy_rect();
int far font_height(int);
void far draw_text_wrapped();
int far key_pressed(void);
void far menu_bar_run(int *, int);
void far flush_keys(void);
void far free_buffer(int, char *);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far datafile_select(char *);
void far sub_root_047c_0545(void);
int far pic_load(char *);
void far sub_root_047c_04f8(void);
unsigned far text_pixel_width(struct gfx *, char *);
void far draw_text(struct gfx *, int, int, char *);
void far skill_try_improve(int, int, int);
void far skill_change_msg(int, int, int);
void far sub_s021_1ef8_01e2(int, int);
void far enemy_info(void);
void far sub_s021_1ef8_03fe(int, int *, int);

void far skill_try_improve(int m, int sk, int mode)
{
    int v, chance;

    v = party[m].skill[sk];
    if (mode == 1)
        chance = (100 - v) / 2;
    else
        chance = (100 - v) / 4;
    if (v < 99 && random_n(100) <= chance)
        skill_change_msg(m, sk, rand_1_n(D_22d2[game_difficulty]) + D_22ce[game_difficulty]);
}

void far skill_change_msg(int m, int sk, int amount)
{
    char buf[0x32];
    char num[8];

    if (amount == 0)
        return;
    if (m == -1)
        m = leader;
    _fstrcpy(buf, skill_names[sk]);
    strcat(buf, " changed from ");
    strcat(buf, _itoa(party[m].skill[sk], num, 10));
    party[m].skill[sk] = sub_root_0410_001a(party[m].skill[sk] += amount, 0, 99);
    strcat(buf, " to ");
    strcat(buf, _itoa(party[m].skill[sk], num, 10));
    strcpy(msg_buf, buf);
    if (amount > 0)
        sub_s021_1ef8_01e2(m, 15);
    else
        sub_s021_1ef8_01e2(m, 12);
}

#pragma optimize("a", on)
void far sub_s021_1ef8_01e2(int row, int color)
{
    int y;

    side_win->f0c = color;
    side_win->f0a = 0;
    mouse_hide();
    if (row == -2) {
        for (y = 0; y < 0xc6; y += 0xb)
            gfx_draw_pic((struct gfx *)screen_ptr, 3, y, D_a628);
        gfx_copy_rect(screen_ptr, 3, 1, 0x36, 0xc6, side_win, 3, 1);
        y = 0x51;
    } else {
        y = row * 40 + 1;
        gfx_draw_pic(side_win, 3, y, D_a626);
    }
    D_01bc = 1;
    draw_text_wrapped(side_win, msg_buf, 0x37, 3, y + 4, font_height(side_win->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
    mouse_show();
    while (!key_pressed() && !(mouse_buttons & 1))
        menu_bar_run(&D_9419, 0);
    while (mouse_buttons & 1)
        ;
}

#pragma optimize("", on)
void far enemy_info(void)
{
    int pics[3];
    struct pr list[2];
    int cnt, i, k, y, kind;

    memset(list, -1, 4);
    pics[2] = pics[1] = pics[0] = 0;
    cnt = 0;
    for (i = 5; i < 13; i++) {
        if (party[i].status != 0 && (kind = party[i].kind) != list[0].kind && list[1].kind != kind) {
            list[cnt].kind = kind;
            list[cnt].idx = i;
            cnt++;
            if (cnt == 2)
                break;
        }
    }
    if (cnt > 0) {
        mouse_hide();
        for (y = 0; y < 0xc6; y += 0xb)
            gfx_draw_pic(side_win, 3, y, D_a628);
        k = 0;
        for (i = 0; i < cnt; i++)
            sub_s021_1ef8_03fe(list[i].idx, &pics[i], cnt == 1 ? 50 : i * 100);
        mouse_show();
        while (!key_pressed() && !(mouse_buttons & 1))
            ;
        flush_keys();
        while (mouse_buttons & 1)
            ;
    }
    for (i = 0; i < 3; i++)
        if (pics[i])
            free_buffer(pics[i], "Einfo");
    party_panel_frames(0x100);
    party_panel_rows(0x100);
}

void far sub_s021_1ef8_03fe(int m, int *pic, int x)
{
    char name[0x3c];
    char s[0x28];
    int y, t;
    unsigned w;

    side_win->f0a = 0;
    datafile_select("einfo.cat");
    sprintf(s, "%s%sst.pic", D_1166[party[m].kind], anim_file[party[m].kind]);
    if (s[0] > 0x60)
        s[0] -= 0x20;
    sub_root_047c_0545();
    *pic = pic_load(s);
    sub_root_047c_04f8();
    gfx_draw_pic(side_win, 0xe, x, *pic);
    y = x + 0x20;
    side_win->f0c = 0xe;
    _fstrcpy(name, anim_set_names[party[m].kind]);
    while ((w = text_pixel_width(side_win, name)) > 0x36)
        name[strlen(name) - 1] = 0;
    draw_text(side_win, (0x36 - w) / 2 + 3, y, name);
    y += 8;
    side_win->f0c = 0xf;
    if (s[0] == 'E') {
        sprintf(s, "%Fs", F_286b_01f7[party[m].f37]);
        if ((unsigned char)(party[m].f41 & 0x7f) > 0)
            strcat(s, "\x8e+\x8f");
        draw_text(side_win, 4, y, s);
        y += 8;
        sprintf(s, "%Fs", F_286b_01f7[party[m].f38]);
        if ((unsigned char)(party[m].f41 & 0x7f) > 0)
            strcat(s, "\x8e+\x8f");
        draw_text(side_win, 4, y, s);
        y += 8;
        if (party[m].f48 != -1) {
            sprintf(s, "%Fs", F_286b_01cf[party[m].f48]);
            draw_text(side_win, 4, y, s);
            y += 8;
        }
        if (party[m].f3d == -1)
            t = 9;
        else
            t = party[m].f3d;
        if (t <= 0x21) {
            sprintf(s, "%Fs", F_286b_0277[t]);
            if (party[m].f34)
                strcat(s, "\x8e+\x8f");
            draw_text(side_win, 4, y, s);
            y += 8;
            if (game_difficulty != 2) {
                sprintf(s, " - %s", quality_words[sub_root_0410_001a(party[m].skill[0], 0, 100) / 20]);
                draw_text(side_win, 4, --y, s);
                y += 8;
            }
        }
        t = -1;
        if (party[m].f0e != -1)
            t = party[m].f0e;
        else if (enemy_class[m] == 2)
            t = 0x22;
        if (t != -1) {
            sprintf(s, "%Fs", F_286b_0277[t]);
            if (party[m].f34)
                strcat(s, "\x8e+\x8f");
            draw_text(side_win, 4, y, s);
            y += 8;
            if (game_difficulty != 2) {
                sprintf(s, " - %s", quality_words[sub_root_0410_001a(party[m].skill[5], 0, 100) / 20]);
                draw_text(side_win, 4, y - 1, s);
            }
        }
    } else {
        sprintf(s, "Natrl Armr");
        draw_text(side_win, 4, y, s);
        y += 8;
        if (game_difficulty != 2) {
            sprintf(s, " - %s", quality_words[sub_root_0410_001a(party[m].f3f & 0xf, 0, 6)]);
            draw_text(side_win, 4, --y, s);
            y += 8;
        }
        sprintf(s, "Natrl Weapn");
        draw_text(side_win, 4, y, s);
        y += 8;
        if (game_difficulty != 2) {
            sprintf(s, " - %s", quality_words[sub_root_0410_001a(party[m].skill[0], 0, 100) / 20]);
            draw_text(side_win, 4, y - 1, s);
        }
    }
}
