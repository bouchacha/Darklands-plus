/* place: s022 1ef8:0000 */
/* orig: size=07da CONST=7df2+0002 _DATA=2358+008f */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* w1 (2026-10-10): 051c now differs only in slot order (pragma a + 5-byte copy loop gives the
 * original `mov cx,2; rep movsw; movsb`): orig k -6, more -8, di home -0xa, cnt -0xc, counter -0xe.
 * status: wip 3/5, all functions written (03e4, 04e4, 07aa match).
 * 20a5 data: `extern int far F_20a5_1f44[];` (a far ARRAY gives `mov ax,SEG; mov es,ax`).
 * 051c blocked: 5-byte struct copy is `mov cx,2; rep movsw; movsb` (6.00A: movsw; movsw; movsb).
 * 0000 blocked: loop-pointer home store scheduled before the 13-byte struct copy, copy uses
 * `mov cx,6` first and `push ss; pop es` (6.00 vs 6.00A block-copy codegen). Combat overlay s022 (child of s014): 0000 character sheet from combat,
 * 03e4 surrender, 04e4 "%Fs%s%Fs" message about a combatant, 051c choose party order
 * (F1..F5 or click), 07aa (near) take member into the order list.
 * Combatants: 13 records of 0x80 bytes, D_9c65[0..4] party, [5..12] enemies
 * (the party records of D_9c69, based 4 bytes earlier); +4 status (1 active, 2 ?, 5 surrendered),
 * D_a6b4[i] side/kind (0x9d = ?), D_9064[5] chosen flags. */
#include "darklands/card.h"
#include "darklands/game.h"

struct s13 { char b[13]; };
struct s5 { char b[5]; };
struct gfx { char pad[0xc]; int f0c; int f0e; };
struct mouse { int f0; int f2; int f4; int f6; };
struct cmb {
    int w0;   /* +00 */
    char pad2[2];
    char status;   /* +04 */
    char pad5[1];
    char f06;   /* +06 */
    char pad7[1];
    char f08;   /* +08 */
    char pad9[2];
    char f0b;   /* +0b */
    char padc[6];
    char f12;   /* +12 */
    char pad13[27];
    char name[1];   /* +2e */
    char pad2f[11];
    char f3a;   /* +3a */
    char pad3b[6];
    char f41;   /* +41 */
    char pad42[62];
};

extern struct cmb D_9c65[];
extern unsigned char cbt_floor[];
extern char real_combat;
extern int battle_result;
extern int battle_args;
extern unsigned char D_c651;
/* original: `mov ax,SEG 20a5` = communal `int far x;` (COMDEF), which omflink rejects */
extern int far F_20a5_1f44[];  /* 20a5:1f44 */
extern char far F_286b_0212[];
extern char far F_286b_0222[];
extern char far * far F_286b_0267[];
extern struct gfx *card_window;
extern int card_window2;
extern char cbt_selected[];
extern char marching_len;
extern char marching_order[];
extern int D_c62a;
extern struct mouse *D_c4fe;
extern char D_907a;
extern char hotspot_count;
extern unsigned char hotspot_cbt[];
extern int D_d8f1[], D_d8d7[], D_d8bd[], D_d8a3[];
extern char move_mode;
extern struct s13 cbt_anim_len;
extern char D_07d3, D_07d8;
extern char msg_buf[];
extern char D_9429, D_9433, D_943d, D_9893;
extern int enemy_anim_slot, D_d67e;
extern int D_d6ee, D_d6f2, D_d6f9, D_d729, D_d70a, D_d73c, D_d726, D_d800;
extern char D_9069;
extern int D_ca53, D_ca55;
extern char D_9c4c;
extern int cursor_default, cursor_current;
extern char *D_11bc[];
extern int anim_buffers[];
extern char cbt_action[];
extern char order_mode, selected_member;
extern int order_action;
extern char D_c7cc;
extern char ai_paused;
extern int D_a693, D_a691;
extern char cbt_attack_rate[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
void *memset(void *, int, unsigned);
int far ask_yes_no(char *);
void far show_message(char *, int);
void far party_panel_frames(int);
void far mouse_hide(void);
void far mouse_show(void);
void far text_box();
int far key_pressed(void);
int far get_key(void);
struct mouse * far sub_root_047c_1320(int, int);
void far copy_rect_saved();
void far single_file_vec(void);
void far enemy_anim_free5_vec(void);
void far free_buffer(int, char *);
int far alloc_buffer(int, int, char *);
void far palette_save(int, int);
void far palette_restore(int, int);
void far sub_root_0000_2646(char *);
void far mouse_set_cursor(int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far shields_load(void);
void far shields_free(void);
void far party_panel_rows(int);
void far inventory_leave_vec(int);
void far battle_tiles_load_vec(void);
void far furniture_load_vec(void);
void far level_load_vec(void);
void far enemy_anim_alloc_vec(int);
void far cbt_set_anim(int, int);
void far weapon_ready(int, int, int);
void far sub_root_047c_04ea(int, int);
void far floor_view_draw_vec(int);
void far combat_draw(void);
void far combat_char_sheet(int);
void far do_surrender(void);
void far sub_s022_1ef8_04e4(int);
void far choose_party_order(void);
static void near sub_s022_1ef8_07aa(int, int *);

void far combat_char_sheet(int y)
{
    struct s13 save;
    char saved[13];
    char st;
    int idx, freed, i;
    char s1, s2, s3, s4;
    int w1, w2;

    idx = y / 40;
    freed = 0;
    i = 0;
    save = cbt_anim_len;
    for (; i < 13; i++)
        saved[i] = D_9c65[i].f0b;
    st = D_9c65[idx].status;
    if (st == 1) {
        if (cbt_floor[idx] == 0x9d) {
            sub_s022_1ef8_04e4(idx);
            return;
        }
        D_07d3++;
        strcpy(msg_buf, "SeqDealloc");
        s1 = D_9429;
        s2 = D_9433;
        s3 = D_943d;
        s4 = D_9893;
        D_9893 = D_943d = D_9433 = D_9429 = 2;
        w1 = enemy_anim_slot;
        w2 = D_d67e;
        enemy_anim_free5_vec();
        if (D_d6ee) {
            free_buffer(D_d6ee, "VarFrn");
            freed = 1;
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
        D_07d8 = 0;
        if (D_9069 == 0) {
            D_ca55 = alloc_buffer(0x300, 0, "PalBuff");
            D_ca53 = 0;
            palette_save(D_ca53, D_ca55);
        }
        sub_root_0000_2646("BeforeCharData");
        if (D_9c4c == 1) {
            mouse_hide();
            mouse_set_cursor(0, 0, cursor_default);
            cursor_current = cursor_default;
        }
        sub_root_047c_0568();
        shields_load();
        party_panel_rows(0x100);
        inventory_leave_vec(idx);
        sub_root_047c_1320(6, 0);
        D_c62a = 0;
        sub_root_0000_2646("AftCharData");
        sub_root_047c_0568();
        if (D_9069 == 0) {
            palette_restore(D_ca53, D_ca55);
            free_buffer(D_ca55, "PalBuff");
        }
        shields_free();
        battle_tiles_load_vec();
        if (freed == 1)
            furniture_load_vec();
        D_07d8 = 1;
        level_load_vec();
        for (i = 0; i < 6; i++)
            strcpy(D_11bc[i], "");
        for (i = 5; i < 13; i++) {
            if (D_9c65[i].status) {
                int k = D_9c65[i].f06;
                if (anim_buffers[k] == 0)
                    enemy_anim_alloc_vec(k);
                cbt_set_anim(i, D_9c65[i].f08);
            }
        }
        D_9429 = s1;
        D_9433 = s2;
        D_943d = s3;
        D_9893 = s4;
        enemy_anim_slot = w1;
        D_d67e = w2;
        cbt_anim_len = save;
        for (i = 0; i < 13; i++) {
            D_9c65[i].f0b = saved[i];
            if (D_9c65[i].status == 1 &&
                (D_9c65[i].w0 == 0x80 || (order_mode == 1 && selected_member == i && order_action == 0x80)) &&
                D_9c65[i].f12 == -1) {
                D_9c65[i].w0 = 0;
                cbt_action[i] = 0;
                D_9c65[i].f3a = -1;
                weapon_ready(i, D_9c65[i].f41, 0);
                if (order_mode == 1 && selected_member == i) {
                    order_action = -1;
                    D_c7cc = 0;
                }
            }
        }
        if (D_9c4c == 1) {
            mouse_hide();
            mouse_set_cursor(0, 0, ai_paused == 0 ? D_a693 : D_a691);
            cursor_current = cursor_default;
            sub_root_047c_04ea(0, 0);
            mouse_show();
        }
        memset(cbt_attack_rate, 0xff, 13);
        floor_view_draw_vec(-1);
        combat_draw();
        sub_root_047c_056f();
        sub_root_047c_1320(6, 0);
        D_07d3--;
    } else if (st == 2)
        sub_s022_1ef8_04e4(idx);
}

#pragma optimize("a", on)
void far do_surrender(void)
{
    int i;
    int any, other;

    if (real_combat == 1 && ask_yes_no("Do you really want to surrender?") == 1) {
        battle_result = 4;
        other = any = 0;
        battle_args = F_20a5_1f44[0];
        for (i = 5; i < 13; i++)
            if (D_9c65[i].status == 1) {
                D_c651 = cbt_floor[i];
                break;
            }
        for (i = 0; i < 5; i++) {
            if (D_9c65[i].status == 1) {
                if (cbt_floor[i] == 0x9d) {
                    any = 1;
                    battle_result = 2;
                }
                if (cbt_floor[i] != D_c651)
                    other = 1;
            }
        }
        if (any == 1) {
            battle_args = F_20a5_1f44[1];
            for (i = 0; i < 5; i++)
                if (cbt_floor[i] != 0x9d && (D_9c65[i].status == 1 || D_9c65[i].status == 2))
                    D_9c65[i].status = 5;
            return;
        }
        if (other == 1) {
            battle_result = -1;
            for (i = 0; i < 5; i++)
                if (cbt_floor[i] == D_c651 && (D_9c65[i].status == 1 || D_9c65[i].status == 2))
                    D_9c65[i].status = 5;
        }
    }
}

#pragma optimize("", on)

void far sub_s022_1ef8_04e4(int i)
{
    char buf[0x50];

    sprintf(buf, "%Fs%s%Fs", F_286b_0212, D_9c65[i].name, F_286b_0222);
    show_message(buf, 0x96);
}

#pragma optimize("a", on)
void far choose_party_order(void)
{
    char msg[0x50];
    int rect[4];
    int s1, s2;
    struct s5 saved;
    int cnt, k, more, i, j, x, y;

    s1 = card_window->f0c;
    s2 = card_window->f0e;
    for (i = 0; i < 5; i++) {
        saved.b[i] = cbt_selected[i];
        cbt_selected[i] = 0;
    }
    party_panel_frames(0x100);
    cnt = 0;
    for (i = 0; i < 5; i++)
        if (D_9c65[i].status == 1) {
            cbt_selected[i] = 1;
            cnt++;
            party_panel_frames(i);
        }
    marching_len = 0;
    for (k = 0; k < cnt - 1; k++) {
        sprintf(msg, "Select %Fs", F_286b_0267[k]);
        card_window->f0c = 1;
        card_window->f0e = 0xff;
        mouse_hide();
        text_box(card_window, -1, 1, msg, card_window2, rect);
        mouse_show();
        more = 1;
        do {
            if (key_pressed()) {
                switch (get_key()) {
                case 0x3b00:
                    sub_s022_1ef8_07aa(0, &more);
                    break;
                case 0x3c00:
                    sub_s022_1ef8_07aa(1, &more);
                    break;
                case 0x3d00:
                    sub_s022_1ef8_07aa(2, &more);
                    break;
                case 0x3e00:
                    sub_s022_1ef8_07aa(3, &more);
                    break;
                case 0x3f00:
                    sub_s022_1ef8_07aa(4, &more);
                    break;
                }
            } else {
                if (D_c62a == 0) {
                    D_c4fe = sub_root_047c_1320(6, 0);
                    D_c62a = D_c4fe->f2;
                }
                if (D_c62a > 0) {
                    y = D_c4fe->f6;
                    D_c62a = 0;
                    x = D_c4fe->f4;
                    if (x < 60) {
                        i = y / 40;
                        if (D_9c65[i].status == 1 && cbt_selected[i] == 1)
                            sub_s022_1ef8_07aa(i, &more);
                    } else if (D_907a == 1) {
                        x -= 60;
                        for (j = hotspot_count - 1; j >= 0; j--)
                            if (hotspot_cbt[j] < 5 && D_d8f1[j] <= x && D_d8d7[j] >= x &&
                                D_d8bd[j] <= y && D_d8a3[j] >= y && cbt_selected[hotspot_cbt[j]] == 1)
                                sub_s022_1ef8_07aa(hotspot_cbt[j], &more);
                    }
                }
            }
        } while (more);
        mouse_hide();
        copy_rect_saved(card_window2, rect[0], rect[1], rect[2], rect[3], card_window, rect[0], rect[1]);
        mouse_show();
    }
    for (i = 0; i < 5; i++)
        if (cbt_selected[i] == 1)
            sub_s022_1ef8_07aa(i, &more);
    for (i = 0; i < 5; i++) cbt_selected[i] = saved.b[i];
    party_panel_frames(0x100);
    if (move_mode == 2)
        single_file_vec();
    card_window->f0c = s1;
    card_window->f0e = s2;
}
#pragma optimize("a", off)

static void near sub_s022_1ef8_07aa(int i, int *more)
{
    marching_order[marching_len] = i;
    cbt_selected[i] = 0;
    party_panel_frames(i);
    marching_len++;
    *more = 0;
}
