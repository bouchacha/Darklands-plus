/* place: s015 1ef8:0000 */
/* orig: size=1160 CONST=7db8+0002 */
/* data: CONST=7db8 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* QA2 fix (2026-10-09): CONST was stamped at 7cf8 (data search hit another module's slot); the original code uses 7db8: hint added, re-stamped CONST=7db8+0002. */
/* status: wip 4/7 (0000, 0046, 00dc, 01a8). 01c6/0708/0c0e: dead induction-variable home
 * stores and SI/DI choice of the 6.00 compiler (`mov [bp-4],0`, `mov [bp-2],3Ch; mov si,[bp-2]`). Combat overlay s015 (child of s014): debug/statistics screens.
 * 0000 tracks the smallest free DOS memory seen (D_1fc2, bytes); 0046 shows it;
 * 00dc debug screen loop (ESC leaves, keys 1..6 page through 3 tables of 10 combatants,
 * starting at 0 or 3); 01a8 row colour (odd rows 9, even 15); 01c6 combatant record dump
 * (D_9c55, labels D_1f2c[26]); 0708 attributes/skills (D_9cb2, labels D_1f60[25]);
 * 0c0e skills and far table F_20a5_6234[k][D_d6fb[k]] (labels D_1f92[24]). */
#include "darklands/card.h"
#include "darklands/game.h"

struct gfx { char f00; char pad[0xb]; int f0c; };

extern unsigned long D_1fc2;
extern int D_0190;
extern char msg_buf[];
extern struct gfx *side_win;
extern struct gfx *card_window2, *card_window;
extern int D_11c8;
extern int D_17b0;
extern char *D_1f2c[], *D_1f60[], *D_1f92[];
extern char combatants[][0x80], member_attrs[][0x80], D_9cc0[][0x80];
extern char cbt_anim[], cbt_attack_rate[], cbt_action[], D_caa7[], D_d3fe[], cbt_moving[], D_d41b[], cbt_floor[];
extern unsigned char D_d6fb[];
extern unsigned char far F_20a5_6234[][0xfa];

unsigned _dos_allocmem(unsigned, unsigned *);
void far gfx_fill_rect();
void far ltoa_commas();
void far draw_text();
void far sub_root_0410_0302(int);
void far mouse_hide(void);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far mouse_show(void);
void far sub_root_047c_059e(int);
int far gfx_page_segment(int);
int far get_key(void);
void far quit_to_dos(void);
void far floor_view_draw_vec(int);
void far combat_draw(void);
void far itoa_commas();
void far debug_mem_track(void);
void far sub_s015_1ef8_0046(void);
void far sub_s015_1ef8_00dc(void);
void far sub_s015_1ef8_01a8(int);
void far sub_s015_1ef8_01c6(int);
void far sub_s015_1ef8_0708(int);
void far sub_s015_1ef8_0c0e(int);

void far debug_mem_track(void)
{
    unsigned seg;
    unsigned long sz;

    _dos_allocmem(0x7530, &seg);
    sz = (unsigned long)seg << 4;
    if (sz < D_1fc2)
        D_1fc2 = sz;
}

void far sub_s015_1ef8_0046(void)
{
    gfx_fill_rect(D_0190, 0, 0, 0x3c, 0x3c, 0);
    ltoa_commas(D_1fc2, msg_buf);
    draw_text(D_0190, 3, 3, msg_buf);
    ltoa_commas(0x9b080L - D_1fc2, msg_buf);
    draw_text(D_0190, 3, 0xd, msg_buf);
    sub_root_0410_0302(200);
    mouse_hide();
    party_panel_frames(0x100);
    party_panel_rows(0x100);
    mouse_show();
}

void far sub_s015_1ef8_00dc(void)
{
    int run;

    if (D_11c8 == 0) {
        sub_root_047c_059e(D_11c8);
        if (gfx_page_segment(0)) {
            D_11c8 ^= 1;
            card_window2->f00 ^= 1;
            card_window->f00 ^= 1;
        }
    }
    run = 1;
    do {
        if (run == 1) {
            sub_s015_1ef8_01c6(0);
            run++;
        }
        switch (get_key()) {
        case 0x11b:
            run = 0;
            break;
        case 0x231:
            sub_s015_1ef8_01c6(0);
            break;
        case 0x332:
            sub_s015_1ef8_01c6(3);
            break;
        case 0x433:
            sub_s015_1ef8_0708(0);
            break;
        case 0x534:
            sub_s015_1ef8_0708(3);
            break;
        case 0x635:
            sub_s015_1ef8_0c0e(0);
            break;
        case 0x736:
            sub_s015_1ef8_0c0e(3);
            break;
        case 0x1000:
            quit_to_dos();
            break;
        }
    } while (run);
    side_win->f0c = 15;
    floor_view_draw_vec(D_17b0);
    combat_draw();
}

void far sub_s015_1ef8_01a8(int row)
{
    side_win->f0c = (row & 1) == 1 ? 9 : 15;
}

void far sub_s015_1ef8_01c6(int first)
{
    int i, y, k, x, j;

    gfx_fill_rect(side_win, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 26; i++) {
        sub_s015_1ef8_01a8(i + 1);
        draw_text(side_win, 0, y, D_1f2c[i]);
        y += 7;
    }
    *(volatile int *)&j = 0;
    for (k = first, x = 0x3c; k < first + 10; k++) {
        itoa_commas(k, msg_buf);
        sub_s015_1ef8_01a8(0);
        draw_text(side_win, x, 0, msg_buf);
        itoa_commas(*(int *)&combatants[k][0], msg_buf);
        sub_s015_1ef8_01a8(7);
        draw_text(side_win, x, 7, msg_buf);
        itoa_commas(*(int *)&combatants[k][2], msg_buf);
        sub_s015_1ef8_01a8(0x0e);
        draw_text(side_win, x, 0x0e, msg_buf);
        itoa_commas(*(int *)&combatants[k][4], msg_buf);
        sub_s015_1ef8_01a8(0x15);
        draw_text(side_win, x, 0x15, msg_buf);
        itoa_commas(*(int *)&combatants[k][6], msg_buf);
        sub_s015_1ef8_01a8(0x1c);
        draw_text(side_win, x, 0x1c, msg_buf);
        itoa_commas(*(int *)&combatants[k][8], msg_buf);
        sub_s015_1ef8_01a8(0x23);
        draw_text(side_win, x, 0x23, msg_buf);
        itoa_commas(*(int *)&combatants[k][0x0a], msg_buf);
        sub_s015_1ef8_01a8(0x2a);
        draw_text(side_win, x, 0x2a, msg_buf);
        itoa_commas(*(int *)&combatants[k][0x10], msg_buf);
        sub_s015_1ef8_01a8(0x31);
        draw_text(side_win, x, 0x31, msg_buf);
        itoa_commas(combatants[k][0x14], msg_buf);
        sub_s015_1ef8_01a8(0x38);
        draw_text(side_win, x, 0x38, msg_buf);
        itoa_commas(combatants[k][0x18], msg_buf);
        sub_s015_1ef8_01a8(0x3f);
        draw_text(side_win, x, 0x3f, msg_buf);
        itoa_commas(combatants[k][0x19], msg_buf);
        sub_s015_1ef8_01a8(0x46);
        draw_text(side_win, x, 0x46, msg_buf);
        itoa_commas(combatants[k][0x1a], msg_buf);
        sub_s015_1ef8_01a8(0x4d);
        draw_text(side_win, x, 0x4d, msg_buf);
        itoa_commas(combatants[k][0x1b], msg_buf);
        sub_s015_1ef8_01a8(0x54);
        draw_text(side_win, x, 0x54, msg_buf);
        itoa_commas(combatants[k][0x4a], msg_buf);
        sub_s015_1ef8_01a8(0x5b);
        draw_text(side_win, x, 0x5b, msg_buf);
        itoa_commas((unsigned char)combatants[k][0x1f], msg_buf);
        sub_s015_1ef8_01a8(0x62);
        draw_text(side_win, x, 0x62, msg_buf);
        itoa_commas(combatants[k][0x22], msg_buf);
        sub_s015_1ef8_01a8(0x69);
        draw_text(side_win, x, 0x69, msg_buf);
        itoa_commas(cbt_anim[k], msg_buf);
        sub_s015_1ef8_01a8(0x70);
        draw_text(side_win, x, 0x70, msg_buf);
        itoa_commas(cbt_attack_rate[k], msg_buf);
        sub_s015_1ef8_01a8(0x77);
        draw_text(side_win, x, 0x77, msg_buf);
        itoa_commas(cbt_action[k], msg_buf);
        sub_s015_1ef8_01a8(0x7e);
        draw_text(side_win, x, 0x7e, msg_buf);
        itoa_commas(D_caa7[k], msg_buf);
        sub_s015_1ef8_01a8(0x85);
        draw_text(side_win, x, 0x85, msg_buf);
        itoa_commas(D_d3fe[k], msg_buf);
        sub_s015_1ef8_01a8(0x8c);
        draw_text(side_win, x, 0x8c, msg_buf);
        itoa_commas((unsigned char)cbt_moving[k], msg_buf);
        sub_s015_1ef8_01a8(0x93);
        draw_text(side_win, x, 0x93, msg_buf);
        itoa_commas(combatants[k][0x57], msg_buf);
        sub_s015_1ef8_01a8(0x9a);
        draw_text(side_win, x, 0x9a, msg_buf);
        itoa_commas(combatants[k][0x56], msg_buf);
        sub_s015_1ef8_01a8(0x0a1);
        draw_text(side_win, x, 0x0a1, msg_buf);
        itoa_commas(D_d41b[k], msg_buf);
        sub_s015_1ef8_01a8(0x0a8);
        draw_text(side_win, x, 0x0a8, msg_buf);
        itoa_commas((unsigned char)combatants[k][0x20], msg_buf);
        sub_s015_1ef8_01a8(0x0af);
        draw_text(side_win, x, 0x0af, msg_buf);
        itoa_commas((unsigned char)combatants[k][0x21], msg_buf);
        sub_s015_1ef8_01a8(0x0b6);
        draw_text(side_win, x, 0x0b6, msg_buf);
        x += 0x19;
    }
}

void far sub_s015_1ef8_0708(int first)
{
    int i, y, k, x, j;

    gfx_fill_rect(side_win, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 25; i++) {
        sub_s015_1ef8_01a8(i + 1);
        draw_text(side_win, 0, y, D_1f60[i]);
        y += 7;
    }
    *(volatile int *)&j = 0;
    for (k = first, x = 0x3c; k < first + 10; k++) {
        itoa_commas(k, msg_buf);
        sub_s015_1ef8_01a8(0);
        draw_text(side_win, x, 0, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(7);
        draw_text(side_win, x, 7, msg_buf);
        itoa_commas(member_attrs[k][7], msg_buf);
        sub_s015_1ef8_01a8(0x0e);
        draw_text(side_win, x, 0x0e, msg_buf);
        itoa_commas(member_attrs[k][1], msg_buf);
        sub_s015_1ef8_01a8(0x15);
        draw_text(side_win, x, 0x15, msg_buf);
        itoa_commas(member_attrs[k][8], msg_buf);
        sub_s015_1ef8_01a8(0x1c);
        draw_text(side_win, x, 0x1c, msg_buf);
        itoa_commas(member_attrs[k][2], msg_buf);
        sub_s015_1ef8_01a8(0x23);
        draw_text(side_win, x, 0x23, msg_buf);
        itoa_commas(member_attrs[k][3], msg_buf);
        sub_s015_1ef8_01a8(0x2a);
        draw_text(side_win, x, 0x2a, msg_buf);
        itoa_commas(member_attrs[k][4], msg_buf);
        sub_s015_1ef8_01a8(0x31);
        draw_text(side_win, x, 0x31, msg_buf);
        itoa_commas(member_attrs[k][5], msg_buf);
        sub_s015_1ef8_01a8(0x38);
        draw_text(side_win, x, 0x38, msg_buf);
        itoa_commas(member_attrs[k][6], msg_buf);
        sub_s015_1ef8_01a8(0x3f);
        draw_text(side_win, x, 0x3f, msg_buf);
        itoa_commas(cbt_floor[k], msg_buf);
        sub_s015_1ef8_01a8(0x46);
        draw_text(side_win, x, 0x46, msg_buf);
        itoa_commas(0, msg_buf);
        sub_s015_1ef8_01a8(0x4d);
        draw_text(side_win, x, 0x4d, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x54);
        draw_text(side_win, x, 0x54, msg_buf);
        itoa_commas((unsigned char)member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x5b);
        draw_text(side_win, x, 0x5b, msg_buf);
        itoa_commas((unsigned char)member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x62);
        draw_text(side_win, x, 0x62, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x69);
        draw_text(side_win, x, 0x69, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x70);
        draw_text(side_win, x, 0x70, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x77);
        draw_text(side_win, x, 0x77, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x7e);
        draw_text(side_win, x, 0x7e, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x85);
        draw_text(side_win, x, 0x85, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x8c);
        draw_text(side_win, x, 0x8c, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x93);
        draw_text(side_win, x, 0x93, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x9a);
        draw_text(side_win, x, 0x9a, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x0a1);
        draw_text(side_win, x, 0x0a1, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x0a8);
        draw_text(side_win, x, 0x0a8, msg_buf);
        itoa_commas(member_attrs[k][0], msg_buf);
        sub_s015_1ef8_01a8(0x0af);
        draw_text(side_win, x, 0x0af, msg_buf);
        x += 0x19;
    }
}

void far sub_s015_1ef8_0c0e(int first)
{
    int i, y, k, x;

    gfx_fill_rect(side_win, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 24; i++) {
        sub_s015_1ef8_01a8(i + 1);
        draw_text(side_win, 0, y, D_1f92[i]);
        y += 7;
    }
    for (k = first, x = 0x3c; k < first + 10; k++) {
        itoa_commas(k, msg_buf);
        sub_s015_1ef8_01a8(0);
        draw_text(side_win, x, 0, msg_buf);
        itoa_commas(D_9cc0[k][0], msg_buf);
        sub_s015_1ef8_01a8(7);
        draw_text(side_win, x, 7, msg_buf);
        itoa_commas(D_9cc0[k][1], msg_buf);
        sub_s015_1ef8_01a8(0x0e);
        draw_text(side_win, x, 0x0e, msg_buf);
        itoa_commas(D_9cc0[k][2], msg_buf);
        sub_s015_1ef8_01a8(0x15);
        draw_text(side_win, x, 0x15, msg_buf);
        itoa_commas(D_9cc0[k][3], msg_buf);
        sub_s015_1ef8_01a8(0x1c);
        draw_text(side_win, x, 0x1c, msg_buf);
        itoa_commas(D_9cc0[k][4], msg_buf);
        sub_s015_1ef8_01a8(0x23);
        draw_text(side_win, x, 0x23, msg_buf);
        itoa_commas(D_9cc0[k][5], msg_buf);
        sub_s015_1ef8_01a8(0x2a);
        draw_text(side_win, x, 0x2a, msg_buf);
        itoa_commas(D_9cc0[k][6], msg_buf);
        sub_s015_1ef8_01a8(0x31);
        draw_text(side_win, x, 0x31, msg_buf);
        itoa_commas(D_9cc0[k][0x11], msg_buf);
        sub_s015_1ef8_01a8(0x38);
        draw_text(side_win, x, 0x38, msg_buf);
        itoa_commas(D_9cc0[k][0x12], msg_buf);
        sub_s015_1ef8_01a8(0x3f);
        draw_text(side_win, x, 0x3f, msg_buf);
        itoa_commas(D_9cc0[k][0x10], msg_buf);
        sub_s015_1ef8_01a8(0x46);
        draw_text(side_win, x, 0x46, msg_buf);
        itoa_commas(D_9cc0[k][0x0d], msg_buf);
        sub_s015_1ef8_01a8(0x4d);
        draw_text(side_win, x, 0x4d, msg_buf);
        itoa_commas(D_9cc0[k][0x0f], msg_buf);
        sub_s015_1ef8_01a8(0x54);
        draw_text(side_win, x, 0x54, msg_buf);
        itoa_commas(D_9cc0[k][0x0e], msg_buf);
        sub_s015_1ef8_01a8(0x5b);
        draw_text(side_win, x, 0x5b, msg_buf);
        itoa_commas(D_9cc0[k][7], msg_buf);
        sub_s015_1ef8_01a8(0x62);
        draw_text(side_win, x, 0x62, msg_buf);
        itoa_commas(D_9cc0[k][9], msg_buf);
        sub_s015_1ef8_01a8(0x69);
        draw_text(side_win, x, 0x69, msg_buf);
        itoa_commas(D_9cc0[k][0x0b], msg_buf);
        sub_s015_1ef8_01a8(0x70);
        draw_text(side_win, x, 0x70, msg_buf);
        itoa_commas(D_9cc0[k][0x0a], msg_buf);
        sub_s015_1ef8_01a8(0x77);
        draw_text(side_win, x, 0x77, msg_buf);
        itoa_commas(D_9cc0[k][0x0c], msg_buf);
        sub_s015_1ef8_01a8(0x7e);
        draw_text(side_win, x, 0x7e, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 0], msg_buf);
        sub_s015_1ef8_01a8(0x85);
        draw_text(side_win, x, 0x85, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 1], msg_buf);
        sub_s015_1ef8_01a8(0x8c);
        draw_text(side_win, x, 0x8c, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 2], msg_buf);
        sub_s015_1ef8_01a8(0x93);
        draw_text(side_win, x, 0x93, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 3], msg_buf);
        sub_s015_1ef8_01a8(0x9a);
        draw_text(side_win, x, 0x9a, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 4], msg_buf);
        sub_s015_1ef8_01a8(0x0a1);
        draw_text(side_win, x, 0x0a1, msg_buf);
        itoa_commas(F_20a5_6234[k][D_d6fb[k] + 5], msg_buf);
        sub_s015_1ef8_01a8(0x0a8);
        draw_text(side_win, x, 0x0a8, msg_buf);
        x += 0x19;
    }
}

