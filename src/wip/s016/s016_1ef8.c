/* place: s016 1ef8:0000 */
/* orig: size=1730 CONST=7dba+0002 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s016: combat command keys - party movement modes (group / individual / single file),
   active-member selection, stairs/portal commands, command letter display.
   status: all 59 functions written; 56/59 match. Blockers (MSC 6.00 vs 6.00A): 0000 switch
   layout (6.00: tree spine inline, bodies in source order), 0da6 placement of the shared
   rebuild block (switch(D_c629) form: worse); 15be differs only in its jump table (absolute
   offsets, follows from the earlier lengths). Notes: build/agent/e7/NOTES.md
   0bd6/0cdc MATCHED (m0): the 5-byte clear is a fill loop `for (i = 0; i < 5; i++)
   D_a6d3[i] = 0;` under #pragma optimize("a") (gives `mov cx,2; rep stosw; stosb`), and the
   following party loop runs on a char pointer to f4a initialised BEFORE the clear
   (`k = 0; p = &D_9c55[0].f4a; <clear>; for (; p < &D_9c55[5].f4a; k++, p += sizeof(struct cbt))`,
   f10 as `*(int *)(p - 0x3a)`), which reproduces 6.00's memory homes of k/p around the clear. */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    int tx;                     /* +08 move target x */
    int ty;                     /* +0a move target y */
    char padc[4];
    int f10;                    /* +10 condition bits */
    char pad12[2];
    char status;                /* +14 1 = active */
    char pad15[5];
    char f1a;                   /* +1a facing */
    char pad1b[0x2f];
    char f4a;                   /* +4a leader in single file (-1 = none) */
    char pad4b[0x35];
};
struct res { int a; int b; };
struct pf {                     /* cbt_flags[13]: per-combatant flags */
    unsigned lo : 5;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned mid : 5;
    unsigned b12 : 1;           /* "F" instead of "W" */
    unsigned hi : 3;
};
struct q16 { int w0; char f2; char name[4]; char f7; char f8; char f9; char pada[12]; };
struct win { char pad0[0xc]; int f0c; char pade[2]; int f10; };
struct r2c { char b[0x2c]; };
struct r0e { char b[0xe]; };

extern struct cbt combatants[];
extern char D_064a[];
extern char D_0880;
extern char D_0881;
extern char D_9631;
extern char D_9645;
extern char D_9659;
extern char D_97dd;
extern char D_97f1;
extern char D_9805;
extern char D_9819;
extern char D_9841;
extern char D_9c26;
extern char D_9c2f;
extern char D_9c3c;
extern char D_9c41;
extern char D_a394;
extern char party_floor;
extern char D_c8b2[];
extern char D_ca30[];
extern char D_d73b;
extern char D_e079;
int far quit_to_dos();
int far do_surrender_vec();
int far enemy_info_vec();
int far combat_char_sheet_vec();
#define KBFLAGS (*(char far *)0x00400017L)
extern struct pf cbt_flags[];
extern struct q16 D_9a47[];
extern struct win *D_0168;
extern int D_0190;
extern char D_074a[];
extern char D_0768[];
extern char D_0788[];
extern char D_07aa;
extern char D_07bc;
extern int combat_clock;
extern char D_07d3;
extern char D_11b1;
extern char floor_shown;
extern char D_17b0;
extern char leader;
extern char msg_buf[];
extern char cbt_selected[];
extern char D_9609;
extern char D_966d;
extern char D_9681;
extern char D_9695;
extern char D_96a9;
extern char D_96bd;
extern char D_96d1;
extern char D_96e5;
extern char D_96f9;
extern char D_970d;
extern char D_9749;
extern char D_982d;
extern char ai_paused;
extern int order_action;
extern char selected_member;
extern char order_mode;
extern char cbt_floor[];
extern char real_combat;
extern int D_a6c2;
extern int D_a6d1;
extern char cbt_moving[];
extern char move_mode;
extern char D_c1ce;
extern char D_c1f2;
extern char marching_order[];
extern char marching_len;
extern unsigned char D_c64c[];
extern int D_c652;
extern unsigned char D_c77e[];
extern unsigned char D_c784[];
extern char D_c7cc;
extern char D_c7cd;
extern char D_c866;
extern char D_c886;
extern char cbt_action[];
extern char D_ca99;
extern char cbt_attack_rate[];
extern char D_d505[][8];
extern char D_d610;
extern char D_d6ea;
extern char D_d6ed;
extern char D_d6f1;
extern char D_d6fb[];
extern char D_d7fe;
extern char cursor_target;
extern unsigned char hotspot_cbt[];
extern unsigned char D_d855[];
extern int D_d86f[];
extern int D_d889[];
extern int D_d97c[];
extern char D_e07b;
extern int D_e09d;
extern int D_e09f;
extern char hotspot_count;
extern int D_e1bc;
extern char D_e1c8;
extern char D_e1ca;
extern char far F_286b_0231[];
extern char far cbt_paths[][0xfa];
extern struct r2c far F_20a5_3016[];
extern struct r0e far F_20a5_1f5b[];

void *memset(void *, int, unsigned);
char *strcpy(char *, const char *);
char far * far _fstrcpy(char far *, const char far *);
int far random_n(int);
int far hot_keys();
int far party_panel_frames();
int far party_panel_rows();
int far show_message();
int far draw_text();
int far font_height();
int far gfx_copy_rect();
int far gfx_draw_pic();
int far key_pressed();
int far get_key();
int far cbt_cancel_action();
int far cbt_forget_target();
int far cbt_set_anim();
int far order_queue_clear();
int far cursor_normal();
int far path_find();
int far path_give();
int far cbt_anim_set();
int far consume_slot();
int far order_mode_enter();
int far order_mode_leave();
int far order_menu_reset();
int far level_switch_vec();
int far floor_view_draw_vec();
int far take_exit_vec();
int far chest_open_vec();
int far door_toggle_vec();
int far chest_disarm_vec();
int far lock_pick_vec(void far *);

void far combat_keys();
void far member_activate();
void far sub_s016_1ef8_0a2c();
void far sub_s016_1ef8_0aba();
void far sub_s016_1ef8_0b92();
void far sub_s016_1ef8_0bd6();
void far sub_s016_1ef8_0cdc();
void far marching_order_rebuild();
void far sub_s016_1ef8_0e2e();
void far single_file();
void far sub_s016_1ef8_0f76();
void far sub_s016_1ef8_101e();
void far sub_s016_1ef8_1072();
void far sub_s016_1ef8_10b2();
void far sub_s016_1ef8_1182();
void far sub_s016_1ef8_1242();
void far sub_s016_1ef8_130a();
void far sub_s016_1ef8_1314();
void far sub_s016_1ef8_131e();
void far sub_s016_1ef8_1328();
void far sub_s016_1ef8_1332();
void far sub_s016_1ef8_133e();
void far sub_s016_1ef8_1364();
void far sub_s016_1ef8_136e();
void far sub_s016_1ef8_137a();
void far sub_s016_1ef8_1384();
void far sub_s016_1ef8_138e();
void far sub_s016_1ef8_1398();
void far sub_s016_1ef8_13a2();
void far sub_s016_1ef8_13ac();
void far sub_s016_1ef8_13b6();
void far sub_s016_1ef8_13c0();
void far sub_s016_1ef8_13ca();
void far sub_s016_1ef8_13d4();
void far sub_s016_1ef8_1436();
void far sub_s016_1ef8_1466();
void far sub_s016_1ef8_1470();
void far sub_s016_1ef8_147a();
void far sub_s016_1ef8_1484();
void far sub_s016_1ef8_148e();
void far sub_s016_1ef8_1498();
void far sub_s016_1ef8_14a2();
void far sub_s016_1ef8_14ac();
void far sub_s016_1ef8_14b6();
void far sub_s016_1ef8_14c0();
void far sub_s016_1ef8_14ca();
void far sub_s016_1ef8_14d4();
void far sub_s016_1ef8_14de();
void far sub_s016_1ef8_14e8();
void far sub_s016_1ef8_14f2();
void far sub_s016_1ef8_14fc();
void far sub_s016_1ef8_1506();
void far sub_s016_1ef8_1510();
void far sub_s016_1ef8_151a();
void far sub_s016_1ef8_1524();
void far sub_s016_1ef8_152e();
int far sub_s016_1ef8_156e();
void far sub_s016_1ef8_1590();
void far sub_s016_1ef8_15be();

/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
void far combat_keys(void)
{
    int k;
    int i;

    k = 0;
    if (key_pressed()) {
        hot_keys();
        if (key_pressed())
            k = get_key();
    } else if (D_0880 < 0 && D_0881 < 0)
        k = 0x4700;
    else if (D_0880 < 0 && D_0881 > 0)
        k = 0x4f00;
    else if (D_0880 > 0 && D_0881 < 0)
        k = 0x4900;
    else if (D_0880 > 0 && D_0881 > 0)
        k = 0x5100;
    else if (D_0880 < 0)
        k = 0x4b00;
    else if (D_0880 > 0)
        k = 0x4d00;
    else if (D_0881 < 0)
        k = 0x4800;
    else if (D_0881 > 0)
        k = 0x5000;

    if (party_floor == floor_shown)
        goto main;
    /* party split over two levels: only some commands */
    switch (k) {
    case 0x4800: case 0x4838: case 0x5000: case 0x5032: case 0x4b00: case 0x4b34:
    case 0x4d00: case 0x4d36: case 0x4700: case 0x4737: case 0x4f00: case 0x4f31:
    case 0x5100: case 0x5133: case 0x4900: case 0x4939:
        show_message(D_064a, 0x96);
        return;
    case 0x011b: case 0x0231: case 0x0332: case 0x0433: case 0x0534: case 0x0635:
    case 0x1c0d: case 0x3920: case 0x3b00: case 0x3c00: case 0x3d00: case 0x3e00:
    case 0x3f00: case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:
main:
            switch (k) {
            case 0x4800:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866--;
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0;
                }
                break;
            case 0x5000:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866++;
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x44;
                }
                break;
            case 0x4b00:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866--;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x66;
                }
                break;
            case 0x4d00:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866++;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x22;
                }
                break;
            case 0x4700:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866 -= 2;
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x77;
                }
                break;
            case 0x4f00:
                if (KBFLAGS & 3) {
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x55;
                }
                break;
            case 0x5100:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866 += 2;
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x33;
                }
                break;
            case 0x4900:
                if (KBFLAGS & 3) {
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (cbt_floor[D_17b0] == floor_shown && D_17b0 < 5 && move_mode != 1) {
                    D_e079 = 1;
                    combatants[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x11;
                }
                break;
            case 0x4838: D_c866--; D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x5032: D_c866++; D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x4b34: D_c866--; sub_s016_1ef8_0b92(); break;
            case 0x4d36: D_c866++; sub_s016_1ef8_0b92(); break;
            case 0x4737: D_c866 -= 2; D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x4f31: D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x5133: D_c866 += 2; D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x4939: D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x3b00: combat_char_sheet_vec(10); break;
            case 0x3c00: combat_char_sheet_vec(0x32); break;
            case 0x3d00: combat_char_sheet_vec(0x5a); break;
            case 0x3e00: combat_char_sheet_vec(0x82); break;
            case 0x3f00: combat_char_sheet_vec(0xaa); break;
            case 0x5e00: member_activate(0); break;
            case 0x5f00: member_activate(1); break;
            case 0x6000: member_activate(2); break;
            case 0x6100: member_activate(3); break;
            case 0x6200: member_activate(4); break;
            case 0x0231: sub_s016_1ef8_0a2c(0); break;
            case 0x0332: sub_s016_1ef8_0a2c(1); break;
            case 0x0433: sub_s016_1ef8_0a2c(2); break;
            case 0x0534: sub_s016_1ef8_0a2c(3); break;
            case 0x0635: sub_s016_1ef8_0a2c(4); break;
            case 0x1e41:
            case 0x1e61:
                if (D_97f1 == 1 && selected_member != -1)
                    sub_s016_1ef8_130a();
                break;
            case 0x3042:
            case 0x3062:
                if (D_9819 == 1 && selected_member != -1)
                    sub_s016_1ef8_131e();
                break;
            case 0x2044:
            case 0x2064:
                sub_s016_1ef8_0f76();
                break;
            case 0x1245:
            case 0x1265:
                if (D_9631 == 1)
                    enemy_info_vec();
                break;
            case 0x2146:
            case 0x2166:
                if (ai_paused == 1 && D_9659 == 1 && selected_member != -1)
                    sub_s016_1ef8_133e();
                break;
            case 0x2247:
            case 0x2267:
                if (D_9681 == 1)
                    sub_s016_1ef8_0bd6();
                break;
            case 0x2348:
            case 0x2368:
                sub_s016_1ef8_101e();
                break;
            case 0x324d:
            case 0x326d:
                if (D_9841 == 1)
                    sub_s016_1ef8_136e();
                break;
            case 0x184f:
            case 0x186f:
                sub_s016_1ef8_1072();
                break;
            case 0x1950:
            case 0x1970:
                sub_s016_1ef8_10b2();
                break;
            case 0x1051:
            case 0x1071:
                if (D_9695 == 1)
                    sub_s016_1ef8_0cdc();
                break;
            case 0x1f53:
            case 0x1f73:
                do_surrender_vec();
                break;
            case 0x1454:
            case 0x1474:
                if (D_97dd == 1) {
                    D_9c26 = 1;
                    D_9c2f = 2;
                    D_9c3c = D_9c41 = -1;
                    D_a394 = 3;
                }
                break;
            case 0x1655:
            case 0x1675:
                sub_s016_1ef8_1182();
                break;
            case 0x2f56:
            case 0x2f76:
                if (D_9805 == 1 && selected_member != -1)
                    sub_s016_1ef8_1314();
                break;
            case 0x1157:
            case 0x1177:
                if (ai_paused == 1 && D_9645 == 1 && selected_member != -1)
                    sub_s016_1ef8_1332();
                break;
            case 0x2d58:
            case 0x2d78:
                sub_s016_1ef8_1242();
                break;
            case 0x1800:
                for (i = 0; i < 13; i++) {
                    combatants[i].f10 = 0;
                    cbt_moving[i] = 0;
                }
                real_combat = 0;
                order_menu_reset();
                break;
            case 0x1c0d:
                if (D_d7fe == 1 && move_mode == 1)
                    D_d73b = 1;
                else if (move_mode == 0)
                    order_mode_leave();
                break;
            case 0x3920:
                if (ai_paused == 0) {
                    ai_paused = 1;
                    D_07d3++;
                    cursor_normal();
                    D_9609 = 1;
                } else
                    sub_s016_1ef8_1436();
                break;
            case 0x011b:
                if (ai_paused == 1)
                    sub_s016_1ef8_13d4();
                break;
            }
        break;
    case 0x1655:
    case 0x1675:
        if (ai_paused == 1) {
            if (D_96bd == 1) {
                D_c77e[selected_member] = D_c8b2[selected_member];
                sub_s016_1ef8_1384();
            }
            return;
        }
        if (D_c8b2[leader] != -1)
            take_exit_vec(D_c8b2[leader], leader);
        return;
    case 0x1000:
        quit_to_dos();
        return;
    }
}

/* make party member i the active one (no-op for absent / unconscious / fled members) */
void far member_activate(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */

    if (i < 5 && (u.t = *(volatile char *)&u.t = combatants[i].status) != 0 && cbt_floor[i] != -99 && u.t != 3 && u.t != 5) {
        D_11b1 = 1;
        D_17b0 = i;
        leader = i;
        if (cbt_floor[i] != floor_shown) {
            level_switch_vec(cbt_floor[i]);
            floor_shown = cbt_floor[i];
            D_e07b = 1;
        }
        party_panel_rows(0x100);
        order_menu_reset();
    }
}

#pragma optimize("a", on)
void far sub_s016_1ef8_0a2c(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */

    if (i < 5 && (u.t = *(volatile char *)&u.t = combatants[i].status) != 0 && cbt_floor[i] != -99 && u.t != 3 && u.t != 5) {
        if (selected_member != -1)
            order_mode_leave();
        D_11b1 = 1;
        D_17b0 = i;
        D_d6ea = cbt_flags[i].b12;
        if (cbt_floor[i] != floor_shown) {
            level_switch_vec(cbt_floor[i]);
            floor_shown = cbt_floor[i];
            floor_view_draw_vec(i);
        }
        order_menu_reset();
        D_d6f1 = 1;
        D_d6ed = i;
    }
}
#pragma optimize("", on)

void far sub_s016_1ef8_0aba(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */
    int k;

    if (i < 5 && (u.t = *(volatile char *)&u.t = combatants[i].status) != 0 && cbt_floor[i] != -99 && u.t != 3 && u.t != 5) {
        D_11b1 = 1;
        D_17b0 = i;
        if (cbt_floor[i] != floor_shown) {
            level_switch_vec(cbt_floor[i]);
            floor_shown = cbt_floor[i];
            D_e07b = 1;
        }
        selected_member = i;
        for (k = 0; k < hotspot_count; k++) {
            if (hotspot_cbt[k] == i) {
                D_e09f = D_d505[cbt_anim_set(selected_member)][combatants[selected_member].f1a] + D_d889[k] - 4;
                D_e09d = (D_d855[k] >> 1) + D_d86f[k] - 0xd;
                break;
            }
        }
        order_mode_enter();
    }
}

void far sub_s016_1ef8_0b92(void)
{
    if (D_c866 < 1) {
        D_c866 = 1;
    } else {
        if (D_c866 > 0x27) {
            D_c866 = 0x27;
        }
    }
    if (D_c886 < 1) {
        D_c886 = 1;
    } else {
        if (D_c886 > 0x27) {
            D_c886 = 0x27;
        }
    }
    D_11b1 = 0;
    floor_view_draw_vec(-1);
}

/* toggle "travel as a group" */
#pragma optimize("a", on)
void far sub_s016_1ef8_0bd6(void)
{
    char msg[80];
    int k;
    int i;
    char *p;

    move_mode = move_mode != 1 ? 1 : 0;
    if (move_mode == 1) {
        strcpy(msg, D_074a);
        D_07bc = 2;
        D_07aa = 1;
        D_d7fe = 1;
        marching_order_rebuild(1);
        k = 0;
        p = &combatants[0].f4a;
        for (i = 0; i < 5; i++)
            cbt_moving[i] = 0;
        for (; p < &combatants[5].f4a; k++, p += sizeof(struct cbt)) {
            *p = -1;
            order_queue_clear(k);
            *(int *)(p - 0x3a) = 0;
        }
    } else {
        strcpy(msg, D_0768);
        D_07aa = 2;
        D_07bc = 2;
        D_d7fe = 0;
        if (D_d610 == 0)
            D_9681 = 2;
        p = &combatants[0].f4a;
        for (i = 0; i < 5; i++)
            cbt_moving[i] = 0;
        for (; p < &combatants[5].f4a; p += sizeof(struct cbt)) {
            *p = -1;
            *(int *)(p - 0x3a) = 0;
        }
    }
    show_message(msg, 0x3c);
    order_menu_reset();
    if (ai_paused == 1)
        D_9609 = 1;
}
#pragma optimize("", on)

/* toggle "travel in single file" */
#pragma optimize("a", on)
void far sub_s016_1ef8_0cdc(void)
{
    char msg[80];
    int i;
    char *p;

    D_d7fe = 0;
    move_mode = move_mode != 2 ? 2 : 0;
    if (move_mode == 2) {
        strcpy(msg, D_0788);
        D_07aa = 2;
        D_07bc = 1;
        marching_order_rebuild(1);
        single_file();
    } else {
        strcpy(msg, D_0768);
        D_07aa = 2;
        D_07bc = 2;
        if (D_d610 == 0)
            D_9695 = 2;
        p = &combatants[0].f4a;
        for (i = 0; i < 5; i++)
            cbt_moving[i] = 0;
        for (; p < &combatants[5].f4a; p += sizeof(struct cbt)) {
            *p = -1;
            *(int *)(p - 0x3a) = 0;
        }
    }
    show_message(msg, 0x3c);
    order_menu_reset();
    if (ai_paused == 1)
        D_9609 = 1;
}
#pragma optimize("", on)

/* rebuild the marching order marching_order unless it still lists exactly the active members */
void far marching_order_rebuild(int p)
{
    int ok;
    int i;
    int j;
    int found;

    if (marching_len == 0) {
rebuild:
        sub_s016_1ef8_0e2e(p);
        return;
    } else {
        ok = 1;
        for (j = 0; j < marching_len; j++)
            if (combatants[marching_order[j]].status != 1)
                ok = 0;
        if (!ok)
            goto rebuild;
        for (i = 0; i < 5; i++) {
            if (combatants[i].status == 1) {
                found = 0;
                for (j = 0; j < marching_len; j++) {
                    if (marching_order[j] == i) {
                        found = 1;
                        break;
                    }
                }
                if (!found)
                    goto rebuild;
            }
        }
    }
}

void far sub_s016_1ef8_0e2e(int p)
{
    char msg[80];
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (combatants[i].status == 1)
            marching_order[n++] = i;
    marching_len = n;
    if (p == 1) {
        _fstrcpy(msg, F_286b_0231);
        show_message(msg, 0x3c);
    }
}

/* single file: every member follows the previous one in the marching order */
void far single_file(void)
{
    int k;

    member_activate(marching_order[0]);
    *(volatile char far *)cbt_paths[marching_order[0]] = -1;   /* clear the path */
    D_d6fb[marching_order[0]] = 0;
    combatants[marching_order[0]].f4a = -1;
    order_queue_clear(marching_order[0]);
    combatants[marching_order[0]].f10 = 0;
    for (k = 1; k < marching_len; k++) {
        order_queue_clear(marching_order[k]);
        combatants[marching_order[k]].f10 = 0;
        combatants[marching_order[k]].f4a = k - 1;
        combatants[marching_order[k]].tx = combatants[marching_order[k - 1]].x;
        combatants[marching_order[k]].ty = combatants[marching_order[k - 1]].y;
        if (path_find(marching_order[k]) == 1) {
            path_give(marching_order[k]);
            D_d6fb[marching_order[k]] = 0;
            D_d97c[marching_order[k]] = random_n(0x3c) + combat_clock + 0x1e;
            cbt_moving[marching_order[k]] = 1;
            combatants[marching_order[k]].f10 = 1;
        }
    }
}

void far sub_s016_1ef8_0f76(void)
{
    if (ai_paused == 1) {
        if (D_96f9 == 1) {
            sub_s016_1ef8_13c0();
            return;
        }
        if (D_970d != 1) {
            return;
        }
        sub_s016_1ef8_13b6();
        return;
    }
    if (D_96f9 == 1) {
        show_message(0x590, 0x96);
        consume_slot(selected_member, (unsigned char)D_c7cd);
        if (!D_ca99) {
            goto L_0ffd;
        }
        door_toggle_vec((unsigned char)D_c64c[leader], leader);
        goto L_1006;
    }
    if (D_970d != 1) {
        goto L_1014;
    }
    if (chest_disarm_vec(leader) != 1) {
        goto L_1014;
    }
    goto L_0ffd;
    goto L_0ffd;
L_0ffd:
    chest_open_vec(leader);
L_1006:
    floor_view_draw_vec(-1);
    goto L_1014;
    goto L_1014;
L_1014:
    cbt_cancel_action(leader);
}

void far sub_s016_1ef8_101e(void)
{
    int i;

    if (D_966d == 1) {
        if (move_mode == 0) {
            sub_s016_1ef8_1364();
            return;
        }
        for (i = 0; i < 5; i++) {
            if (combatants[i].status == 1) {
                combatants[i].f10 = 0;
                cbt_moving[i] = 0;
                party_panel_rows(i);
                cbt_set_anim(i, 5);
                cbt_action[i] = 0;
            }
        }
    }
}

void far sub_s016_1ef8_1072(void)
{
    if (D_96d1 == 1) {
        if (ai_paused == 1) {
            if (selected_member == -1) {
                return;
            }
            sub_s016_1ef8_13a2();
            return;
        }
        chest_open_vec(leader);
        floor_view_draw_vec(-1);
        cbt_cancel_action(leader);
    }
}

void far sub_s016_1ef8_10b2(void)
{
    if (ai_paused == 1) {
        if (D_982d == 1 && selected_member != -1) {
            sub_s016_1ef8_1328();
            return;
        }
        if (D_96e5 == 1) {
            if (D_ca99 == 0) {
                sub_s016_1ef8_13ac();
                return;
            }
            sub_s016_1ef8_1398();
            return;
        }
    } else if (D_96e5 == 1) {
        D_e1ca = leader;
        if (D_ca99 == 0) {
            if (lock_pick_vec(&F_20a5_3016[D_c784[leader]]) != 1)
                goto done;
            chest_open_vec(leader);
        } else {
            if (lock_pick_vec(&F_20a5_1f5b[D_c64c[leader]]) != 1)
                goto done;
            door_toggle_vec(D_c64c[leader], leader);
        }
        floor_view_draw_vec(-1);
done:
        cbt_cancel_action(leader);
    }
}

void far sub_s016_1ef8_1182(void)
{
    int i;

    if (ai_paused == 1) {
        if (selected_member != -1) {
            if (D_96bd == 1) {
                sub_s016_1ef8_1384();
                return;
            }
            if (D_96a9 == 1) {
                sub_s016_1ef8_137a();
                return;
            }
        }
    } else {
        if (D_96bd == 1) {
            if (move_mode == 0) {
                take_exit_vec(D_c77e[leader], leader);
            } else {
                for (i = 0; i < 5; i++)
                    if (combatants[i].status == 1)
                        take_exit_vec(D_c77e[leader], i);
            }
        } else if (D_96a9 == 1) {
            door_toggle_vec(D_c64c[leader], leader);
            floor_view_draw_vec(-1);
        }
        cbt_cancel_action(leader);
    }
}

void far sub_s016_1ef8_1242(void)
{
    int i;

    if (D_9749 == 1) {
        if (ai_paused == 1) {
            if (selected_member != -1) {
                sub_s016_1ef8_138e();
                return;
            }
        } else if (move_mode == 0) {
            cbt_floor[leader] = -99;
            cbt_flags[leader].b5 = 0;
            cbt_flags[leader].b6 = 1;
            cbt_cancel_action(leader);
            party_panel_rows(leader);
            cbt_forget_target(leader);
            floor_view_draw_vec(-1);
        } else if (real_combat == 0) {
            for (i = 0; i < 5; i++) {
                if (combatants[i].status == 1) {
                    cbt_floor[i] = -99;
                    cbt_flags[i].b5 = 0;
                    cbt_flags[i].b6 = 1;
                    cbt_forget_target(i);
                    cbt_cancel_action(i);
                }
            }
            party_panel_rows(0x100);
        }
    }
}

void far sub_s016_1ef8_130a(void)
{
    sub_s016_1ef8_156e(0x12);
}

void far sub_s016_1ef8_1314(void)
{
    sub_s016_1ef8_156e(0xa);
}

void far sub_s016_1ef8_131e(void)
{
    sub_s016_1ef8_156e(6);
}

void far sub_s016_1ef8_1328(void)
{
    sub_s016_1ef8_156e(0x22);
}

void far sub_s016_1ef8_1332(void)
{
    order_action = 1;
    sub_s016_1ef8_15be();
}

void far sub_s016_1ef8_133e(void)
{
    order_action = 1;
    cbt_flags[selected_member].b12 = 1;
    sub_s016_1ef8_15be();
    cbt_flags[selected_member].b12 = 1;
}

void far sub_s016_1ef8_1364(void)
{
    sub_s016_1ef8_1590(0xa);
}

void far sub_s016_1ef8_136e(void)
{
    sub_s016_1ef8_156e(0x80);
}

void far sub_s016_1ef8_137a(void)
{
    sub_s016_1ef8_1590(1);
}

void far sub_s016_1ef8_1384(void)
{
    sub_s016_1ef8_1590(2);
}

void far sub_s016_1ef8_138e(void)
{
    sub_s016_1ef8_1590(8);
}

void far sub_s016_1ef8_1398(void)
{
    sub_s016_1ef8_1590(3);
}

void far sub_s016_1ef8_13a2(void)
{
    sub_s016_1ef8_1590(5);
}

void far sub_s016_1ef8_13ac(void)
{
    sub_s016_1ef8_1590(6);
}

void far sub_s016_1ef8_13b6(void)
{
    sub_s016_1ef8_1590(4);
}

void far sub_s016_1ef8_13c0(void)
{
    sub_s016_1ef8_1590(7);
}

void far sub_s016_1ef8_13ca(void)
{
    sub_s016_1ef8_1590(9);
}

void far sub_s016_1ef8_13d4(void)
{
    order_action = -1;
    D_c652 = 0;
    cbt_flags[selected_member].b12 = D_d6ea;
    *(volatile char *)&order_mode = 0;
    cbt_selected[selected_member] = D_c7cc = 0;
    party_panel_frames(selected_member);
    party_panel_rows(0x100);
    D_e1bc = 0;
    cursor_target = selected_member = -1;
    order_menu_reset();
}

void far sub_s016_1ef8_1436(void)
{
    if (selected_member != -1) {
        order_mode_leave();
    }
    ai_paused = 0;
    cursor_normal();
    D_07d3--;
    D_9609 = 2;
    memset(cbt_attack_rate, 0xff, 0xd);
}

void far sub_s016_1ef8_1466(void)
{
    sub_s016_1ef8_152e(0);
}

void far sub_s016_1ef8_1470(void)
{
    sub_s016_1ef8_152e(1);
}

void far sub_s016_1ef8_147a(void)
{
    sub_s016_1ef8_152e(2);
}

void far sub_s016_1ef8_1484(void)
{
    sub_s016_1ef8_152e(3);
}

void far sub_s016_1ef8_148e(void)
{
    sub_s016_1ef8_152e(4);
}

void far sub_s016_1ef8_1498(void)
{
    sub_s016_1ef8_152e(5);
}

void far sub_s016_1ef8_14a2(void)
{
    sub_s016_1ef8_152e(6);
}

void far sub_s016_1ef8_14ac(void)
{
    sub_s016_1ef8_152e(7);
}

void far sub_s016_1ef8_14b6(void)
{
    sub_s016_1ef8_152e(8);
}

void far sub_s016_1ef8_14c0(void)
{
    sub_s016_1ef8_152e(9);
}

void far sub_s016_1ef8_14ca(void)
{
    sub_s016_1ef8_152e(0xa);
}

void far sub_s016_1ef8_14d4(void)
{
    sub_s016_1ef8_152e(0xb);
}

void far sub_s016_1ef8_14de(void)
{
    sub_s016_1ef8_152e(0xc);
}

void far sub_s016_1ef8_14e8(void)
{
    sub_s016_1ef8_152e(0xd);
}

void far sub_s016_1ef8_14f2(void)
{
    sub_s016_1ef8_152e(0xe);
}

void far sub_s016_1ef8_14fc(void)
{
    sub_s016_1ef8_152e(0xf);
}

void far sub_s016_1ef8_1506(void)
{
    sub_s016_1ef8_152e(0x10);
}

void far sub_s016_1ef8_1510(void)
{
    sub_s016_1ef8_152e(0x11);
}

void far sub_s016_1ef8_151a(void)
{
    sub_s016_1ef8_152e(0x12);
}

void far sub_s016_1ef8_1524(void)
{
    sub_s016_1ef8_152e(0x13);
}

void far sub_s016_1ef8_152e(int k)
{
    cbt_flags[selected_member].b12 = 0;
    order_action = 0x100;
    D_c1ce = D_9a47[k].f9;
    D_c1f2 = D_9a47[k].f8;
    D_e1c8 = D_c1ce == 4 ? 1 : 0;
    sub_s016_1ef8_15be();
}

int far sub_s016_1ef8_156e(int p_6)
{
    order_action = p_6;
    if (cursor_target < -1) {
        cursor_target = -1;
        D_e1bc = 0;
    }
    sub_s016_1ef8_15be();
}

void far sub_s016_1ef8_1590(int v)
{
    order_action = 0x400;
    D_a6d1 = v;
    if (cursor_target < -1) {
        cursor_target = -1;
        D_e1bc = 0;
    }
    sub_s016_1ef8_15be();
    D_e1ca = selected_member;
}

/* show the letter of the current command next to the active member's portrait */
#pragma optimize("a", on)
void far sub_s016_1ef8_15be(void)
{
    unsigned t;
    int s;

    t = order_action;
    if (t != 0x400) {
        switch (t) {
        case 1:
            msg_buf[0] = cbt_flags[selected_member].b12 ? 'F' : 'W';
            break;
        case 6:
            msg_buf[0] = 'B';
            break;
        case 10:
            msg_buf[0] = 'V';
            break;
        case 0x12:
            msg_buf[0] = 'A';
            break;
        case 0x22: msg_buf[0] = 'P'; break;
        case 0x40:
            msg_buf[0] = 'R';
            break;
        case 0x80:
            msg_buf[0] = 'M';
            break;
        case 0x100:
            msg_buf[0] = 'T';
            break;
        }
    } else {
        switch (D_a6d1) {
        case 1:
        case 2:
            msg_buf[0] = 'U';
            break;
        case 3:
        case 6:
            msg_buf[0] = 'P';
            break;
        case 4:
        case 7:
            msg_buf[0] = 'D';
            break;
        case 5:
            msg_buf[0] = 'O';
            break;
        case 8:
            msg_buf[0] = 'X';
            break;
        case 10:
            msg_buf[0] = 'H';
            break;
        }
    }
    msg_buf[1] = 0;
    gfx_draw_pic(D_0168, 6, selected_member * 40 + 0x1f, D_a6c2);
    s = D_0168->f0c;
    D_0168->f0c = 0xff;
    draw_text(D_0168, 6, selected_member * 40 + 0x1f, msg_buf);
    D_0168->f0c = s;
    gfx_copy_rect(D_0168, 6, selected_member * 40 + 0x1f, 10, font_height(D_0168->f10),
                       D_0190, 6, selected_member * 40 + 0x1f);
    cbt_flags[selected_member].b12 = 0;
}
#pragma optimize("", on)
