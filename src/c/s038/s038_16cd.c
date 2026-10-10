/* place: s038 16cd:000e */
/* orig: size=05ca _DATA=4b5a+003e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH. Hall of fame (darkland.fam): 00b0 shows it, 0404 merges the party's score. */
/* darkland.fam: 5 records of 0x6a bytes (0x212 bytes total) */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
struct fame {
    int score;          /* +0  party fame score (party_fame.v) */
    int unk2;           /* +2 */
    int seed;           /* +4  party seed (party_seed) */
    char name[4][25];   /* +6  member names (member record +0x11) */
};
/* party fame score; must be a struct member (not a plain int) or 6.00A flips the compares */
extern struct { int v; } party_fame;
extern char screen_win[];
extern int party_seed;
extern char party[][0x80];
char *strcat();
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
int far itoa();
char *strcpy(char *, const char *);
int far gfx_text();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far key_pressed();
int far get_key();
int far sub_root_047c_1320();
int far pic_draw_pal();
void far sub_s038_16cd_000e();
int far sub_s038_16cd_005e();
int far sub_s038_16cd_0084();
void far sub_s038_16cd_00b0();
void far sub_s038_16cd_0404();

void far sub_s038_16cd_000e(void)
{
    sub_root_047c_0568();
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_s038_16cd_0084();
    sub_s038_16cd_00b0();
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
    sub_s038_16cd_005e();
}

int far sub_s038_16cd_005e(void)
{
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    do {
    } while (mouse_buttons & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s038_16cd_0084(void)
{
    pic_draw_pal(1, "pics\\hallfame.pic", cur_palette);
    set_palette(cur_palette);
    back_win_colour = 0;
    back_win_font = 1;
    return screen_win_font = 1;
}

void far sub_s038_16cd_00b0(void)
{
    struct fame tab[5];
    char line[120];
    int y;
    int i;
    int fp;
    fread(tab, 0x212, 1, fp = fopen("darkland.fam", "rb"));
    fclose(fp);
    back_win_colour = 0;
    for (y = 0x3c, i = 0; i < 4; y += 0x18, i++) {
        itoa(tab[i].score, line, 10);
        gfx_text(0x2806, 0x28, y, line);
        line[0] = 0;
        if (tab[i].name[0][0]) {
            strcat(line, tab[i].name[0]);
            strcat(line, ",");
        }
        if (tab[i].name[1][0]) {
            strcat(line, tab[i].name[1]);
            strcat(line, ",");
        }
        if (tab[i].name[2][0]) {
            strcat(line, tab[i].name[2]);
            strcat(line, ",");
        }
        if (tab[i].name[3][0]) {
            strcat(line, tab[i].name[3]);
        }
        gfx_text(0x2806, 0x78, y - 10, line);
    }
    itoa(party_fame.v, line, 10);
    gfx_text(0x2806, 0x28, 0x88, line);
    if (party[0][0]) {
        strcat(line, &party[0][0x11]);
        strcat(line, ",");
    }
    if (party[1][0]) {
        strcat(line, &party[1][0x11]);
        strcat(line, ",");
    }
    if (party[2][0]) {
        strcat(line, &party[2][0x11]);
        strcat(line, ",");
    }
    if (party[3][0]) {
        strcat(line, &party[3][0x11]);
    }
    gfx_text(0x2806, 0x78, 0x7e, line);
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    mouse_show();
}

/* merges the party into the hall of fame table, but never writes it back (original bug) */
void far sub_s038_16cd_0404(void)
{
    struct fame out[5];
    struct fame tab[5];
    struct fame cur;
    int fp;
    int n;
    fp = fopen("darkland.fam", "rb");
    fread(tab, 0x212, 1, fp);
    cur.score = party_fame.v;
    cur.seed = party_seed;
    if (party[0][0]) strcpy(cur.name[0], &party[0][0x11]); else cur.name[0][0] = 0;
    if (party[1][0]) strcpy(cur.name[1], &party[1][0x11]); else cur.name[0][1] = 0;
    if (party[2][0]) strcpy(cur.name[2], &party[2][0x11]); else cur.name[0][2] = 0;
    if (party[3][0]) strcpy(cur.name[3], &party[3][0x11]); else cur.name[0][3] = 0;
    n = 0;
    if (tab[3].score < party_fame.v) {
        if (tab[0].score > party_fame.v) out[n++] = tab[0]; else out[n++] = cur;
        if (tab[1].score > party_fame.v) out[n++] = tab[1]; else out[n++] = cur;
        if (tab[2].score > party_fame.v) out[n++] = tab[2]; else out[n++] = cur;
        if (tab[3].score > party_fame.v) out[n++] = tab[3]; else out[n++] = cur;
    }
    fclose(fp);
}
