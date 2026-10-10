/* place: s014 19cd:0006 */
/* orig: size=379a CONST=7da2+000e _DATA=1bca+006c */
/* data: CONST=7da2 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* QA2 fix (2026-10-09): D_c291 is a POINTER to the near loot copy (s002: D_c291 = &D_a895); it was declared 'struct loot D_c291[]', so 34a0 and 2e50 copied the loot list over DGROUP c291.. and wrote that to the loot file. Re-stamped (data unchanged). */
/* s014 19cd: COMBAT USER INTERFACE (mouse orders, location menus, loot piles).
   Screen -> map: the battle view is an oblique projection,
     map_y = mouse_y - F_20a5_41f4 + D_c293
     map_x = (mouse_y - F_20a5_41f4) - F_20a5_4be0 + mouse_x - 16 + D_c26c.
   Hot spots of the visible combatants (built by the renderer, count D_e0a1):
     D_d848[k] combatant, rect D_d8f1/D_d8d7 (x0/x1) x D_d8bd/D_d8a3 (y0/y1),
     cursor anchor D_d889/D_d86f, size D_d862/D_d855.
   Order mode: D_a6b3 = 1 while a party member D_a6b2 is selected; D_a6b0 = chosen action
   (-1 none, 1 walk, 0x80 missile, 0x100 spell/item, 0x400 cancel...), D_d845 = what the cursor
   points at (>= 0 combatant, -2 free ground, -3 blocked ground, -1 nothing); cursor box
   D_e1c2/D_e1be/D_e1b8/D_e1ba with colour D_e1bc.
     0006  mouse handler (clicks on members, ground, group moves in formation D_c102 1/2)
     069c  enter order mode for D_a6b2; 0738 leave it and apply the order
     0ad4  hover feedback in order mode; 0dcc click in order mode
     11ec/164a/1714  group move: place the marching order D_c624[] in formation around the
           clicked point (formation tables D_1b9a.. by direction)
     1764  set the cursor box; 17f6 track it; 19ee initial cursor of an order
     26a4  per tick: which location (door/stairs/chest/region/trigger) each member stands in
           (1ade 1db8 1e92 209a: flag bits in D_a696[i], leader's menu entries), triggers
           (216a: far handlers in floor+0x218c), doors bumped by enemies open (vec_1d6f)
     2312/23f6/24a8/255a  trap/trigger handlers (pit "Aaaahh!", alarms, "discovers a trap")
     28a2/29b8  build the order menu for the selected member
     2d08/2e50/3204/3258/33c0/34a0  loot piles (items of the fallen in a file, cache screen)
     36f0/3766  shield picture load / free
   w1 (2026-10-10): 13 match; 36f0/3766: struct i5 copies written as int copy loops + pragma a (rep movsw).
   status: 34 of 34 functions written, 10 match (1714 1764 1d5e 209a 216a 3204 2d08 069c 1db8 28a2; see e8 NOTES).
   28a2 (m4): if/else instead of ternaries (both the D_9645 one and the strcpy sources).
   1db8 (m4): box locals x1/y0/y1 in a struct {k, y1, y0, x1}, '? 1 : 2' result, D_11ca declared
   after D_c623 (moved down: cmp direction).
   069c: bit-field read ((struct a696bf *)D_a696)[i].b12 and a volatile store to D_a6b3.
   m4: 2d08 has ONE int ofs (frame 6, no ofs[1] store); _fmemcpy is now prototyped, so the near
   D_c291 is widened with DS (orig 'push ds') - without it only 2 bytes were pushed (behaviour bug). */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    int tx;                     /* +08 */
    int ty;                     /* +0a */
    char padc[4];
    int f10;                    /* +10 action */
    char pad12[2];
    char status;                /* +14 */
    char pad15;
    char kind;                  /* +16 */
    char pad17[2];
    char f19;                   /* +19 facing */
    char f1a;                   /* +1a */
    char f1b;                   /* +1b */
    char pad1c[2];
    char f1e;                   /* +1e */
    char pad1f[3];
    char weapon;                /* +22 */
    char pad23[0x1b];
    char name[0xc];             /* +3e */
    char target;                /* +4a */
    char pad4b[6];
    char f51;                   /* +51 default weapon */
    char pad52[8];
    char f5a;                   /* +5a */
    char pad5b[5];
    char f60;                   /* +60 perception (trap discovery) */
    char pad61[0x1d];
    char nitems;                /* +7e */
    char pad7f;
};

struct rect14 { int x; int y; unsigned char w; unsigned char h; char f6; char f7;
                unsigned char f8; unsigned char f9; char pad[4]; };       /* doors, 14 bytes */
struct trig { int x; int y; unsigned char w; unsigned char h;
              void (far *fn)(int, int, int, int); int a; int b; };   /* 14 bytes */
struct region { int x; int y; unsigned char w; unsigned char h; char f6; char f7;
                unsigned char lock; unsigned char acid; char pad[0x22]; };  /* 0x2c bytes */
struct zone { int x; int y; int w; int h; char pad[2]; };                  /* 10 bytes */
struct pile { int ofs; char cnt; char level; int x; int y; int vis; char dx; char dy; }; /* 12 bytes */
struct loot { int id; char a; char b; char cnt; char pad; };               /* 6 bytes */

/* far data in segment 20a5 (current floor and combat tables) */
/* the floor image 20a5:0000 is ONE far variable in the original source (every access goes
   through CONST slot 7daa, e.g. 23f6/24a8 reach 3350 and 2186 through it) */
struct sd10 { char on; char pad[4]; char c; int a; int b; };  /* 3350 furniture removal, 10 bytes */
struct wr10 { char on; char pad[3]; int a; int b; char c; char pad2; }; /* 3382 wall removal */
struct tp12 { unsigned char col[4]; unsigned char row[4]; unsigned char f[4]; }; /* 3236 trap map patches */
struct flr {
    long cell[1600];                    /* 0000 map cells */
    unsigned char h[1600];              /* 1900 height/flag bytes [40][40] */
    char p1f40[0x1f52 - 0x1f40];
    char ndoors;                        /* 1f52 */
    char p1f53;
    struct rect14 doors[40];            /* 1f54 */
    char ntrig;                         /* 2184 */
    char p2185;
    struct trig trig[50];               /* 2186 */
    char p2442[0x2fb6 - 0x2442];
    char nstairs;                       /* 2fb6 */
    char p2fb7;
    struct rect14 stairs[6];            /* 2fb8 */
    char nreg;                          /* 300c */
    char p300d;
    struct region reg[12];              /* 300e */
    char p321e[0x3236 - 0x321e];
    struct tp12 tp[20];                 /* 3236 */
    char nzone;                         /* 3326 */
    char p3327;
    struct zone zone[4];                /* 3328 */
    struct sd10 sd[5];                  /* 3350 */
    struct wr10 wr[7];                  /* 3382 */
    char p33c6[0x33d3 - 0x33c6];
    char open[40];                      /* 33d3 door states */
};
extern struct flr far F_20a5_000;
#define F_20a5_0000 (F_20a5_000.cell)
#define F_20a5_1900 (F_20a5_000.h)
#define F_20a5_1f52 (F_20a5_000.ndoors)
#define F_20a5_1f54 (F_20a5_000.doors)
#define F_20a5_2184 (F_20a5_000.ntrig)
#define F_20a5_2186 (F_20a5_000.trig)
#define F_20a5_2fb6 (F_20a5_000.nstairs)
#define F_20a5_2fb8 (F_20a5_000.stairs)
#define F_20a5_300c (F_20a5_000.nreg)
#define F_20a5_300e (F_20a5_000.reg)
#define F_20a5_3236 (F_20a5_000.tp)
#define F_20a5_3326 (F_20a5_000.nzone)
#define F_20a5_3328 (F_20a5_000.zone)
#define F_20a5_3350 (F_20a5_000.sd)
#define F_20a5_3382 (F_20a5_000.wr)
#define F_20a5_33d3 (F_20a5_000.open)
extern int far F_20a5_41f4;             /* view origin y */
extern int far F_20a5_4be0;             /* view origin x */
extern struct pile far loot_piles[];   /* loot piles (count loot_pile_count): offset in the loot file */
extern struct loot far F_20a5_3954[];   /* items of the fallen (count D_c783) */
extern char far cbt_paths[13][250];   /* paths */
extern char far F_20a5_7900[13][13];    /* melee reach */
extern struct { int a, b, c; } far F_2e38_0bb0;
extern unsigned char far F_2e38_0003[][0x180]; /* party inventories (+0x3e..) */

extern struct cbt combatants[];
extern int D_e118;                      /* path length */
extern int enemy_anim_slot, D_d67e;              /* enemy animation slots */
extern struct cbt D_d73e;               /* scratch record */
extern char hotspot_cbt[];                   /* hot spot -> combatant */
extern int D_d8f1[], D_d8d7[], D_d8bd[], D_d8a3[]; /* hot spot rects */
extern int D_d889[], D_d86f[];          /* cursor anchor */
extern unsigned char D_d862[], D_d855[];/* cursor size */
extern char hotspot_count;                     /* hot spot count */
extern int mouse_x, mouse_y;              /* mouse x, y */
extern unsigned char mouse_buttons;            /* mouse buttons */
extern char D_d728;                     /* button pressed */
#define KBFLAGS (*(unsigned char far *)0x00400017L)  /* BIOS keyboard flags (4 = Ctrl) */
extern int D_9409;
extern char move_mode;                     /* movement mode */
extern unsigned D_da4b;                 /* group-cursor timeout */
extern unsigned combat_clock;                 /* clock */
extern char cursor_target;                     /* cursor target */
extern int D_c941, D_c987, D_ca19, D_ca2a;   /* group cursor colours */
extern int D_e1c0;
extern char marching_order[];                   /* marching order */
extern char marching_len;                     /* its length */
extern char floor_shown;                     /* shown floor (decl position decides cmp order in 1db8) */
extern char order_mode, selected_member;             /* order mode, selected member */
extern int order_action;                      /* chosen action */
extern int D_a6d1;
extern char D_c7cc, D_c762, D_e1c8;
extern int D_e09f, D_e09d, D_e1c6, D_e1c4;   /* member cursor */
extern int D_d742, D_d744;              /* = D_d73e.x / y */
extern int D_c26c, D_c293;              /* view scroll */
extern char D_d788;                     /* = D_d73e.target */
extern char D_d757;                     /* = D_d73e.f19 */
extern char D_d7fe;
extern int D_e1b4, D_e1b6;              /* clicked map point */
extern int D_c62a;                      /* pending mouse event */
extern int *D_c4fe;                     /* mouse event record */
extern char D_d73b;
extern char cbt_moving[];
extern char D_d6fb[];
extern char party_floor;                     /* party floor */
extern int los_x0, los_y0, los_x1, los_y1;
extern int D_c2bd, D_c242, D_c244, D_c28f, D_c4db, D_c621;   /* cursor colours */
extern int D_e1c2, D_e1be, D_e1b8, D_e1ba, D_e1bc;
extern char D_c600;
extern int D_c652, D_c67d;              /* order destination */
extern char real_combat;
extern char D_17b0;
extern char cbt_action[], cbt_anim[], D_ca30[];
extern int D_d97c[];
extern int D_c867;
extern char cbt_special[];                   /* [13] */
extern unsigned char D_d6ea;
extern unsigned char cbt_flags[][2];       /* [13] location flags */
extern char *anim_file[];
extern char D_c1ce, D_c1f2, D_ca0c[], D_c97a[];
extern int cbt_engage_time[];
extern char cbt_selected[];                   /* [13] selected flag */
extern char ai_paused;
extern char D_07d3;
extern char D_9609;
extern char D_6f9c_[];
extern char cbt_half_w[], cbt_half_h[];
extern char D_d505[][8];                /* sprite x offsets */
extern char D_d5d3[], D_d5de[];
extern char D_1b9a[8], D_1ba2[8], D_1baa[8], D_1bb2[8], D_1bba[8], D_1bc2[8]; /* formation */
extern char D_179c[];
extern char D_17b2[][3];

void far party_panel_frames(int);
void far party_panel_rows(int);
void far show_message(char *, int);      /* message box */
int far sub_root_047c_1320(int, int);
unsigned far distance3(int, int, int);
int far dir_sub(int, int);          /* direction - n (mod 8) */
int far dir_add(int, int);          /* direction + n (mod 8) */
void far order_queue(int, int);
void far effect_start(int, int, int);
void far sub_s014_0e76_4694(void);
void far cursor_normal(void);
int far path_find(int);
void far path_give(int);
void far path_save(void);
void far path_give_saved(int);
int far cbt_collisions(struct cbt *);
int far cbt_anim_set(int);
void far combat_draw(void);
void far weapon_ready(int, int, int);
int far can_fire(int);
int far line_blocked(int, int, int);
void far member_activate_vec(int);                 /* s016 09bc: Ctrl-click on a member */
void far vec_1c39(int);                 /* s016 0a2c: click on a member */
void far floor_view_draw_vec(int);                 /* s017 0000: redraw (combatant or -1) */
void far door_set_vec(int, int, int);       /* s023 04ac: door state */
void far vec_1c4d(int, int);            /* s023 0aea: remove furniture */
void far vec_1c57(int, int);            /* s023 0b6e: remove wall */
void far vec_1da1(int);                 /* s019 0c80: spell/effect menu of a member */
void far enemy_anim_free5_vec(void);                /* s025 0be4: free enemy animations */
void far loot_distribute_vec(unsigned, unsigned, unsigned char *, unsigned, unsigned, int); /* s040: loot screen */
void far battle_tiles_load_vec(void);                /* s002 0e76_0c8c: restore combat screen */
void far furniture_load_vec(void);                /* s024 07a2: reload furniture */
void far enemy_anim_alloc_vec(int);                 /* s025 0a84: reload an enemy animation set */

void far order_mode_enter(void);
void far order_mode_leave(void);
void far sub_s014_19cd_0ad4(int, int);
void far sub_s014_19cd_0dcc(int, int);
int far sub_s014_19cd_11ec(int, int, int);
int far sub_s014_19cd_164a(int, int, int, int, int);
int far sub_s014_19cd_1714(int);
void far sub_s014_19cd_1764(int, int, int, int, int, int, int);
void far sub_s014_19cd_19ee(void);
void near sub_s014_19cd_1ade(int);
void near sub_s014_19cd_1db8(int);
void near sub_s014_19cd_1e92(int);
void near sub_s014_19cd_209a(int);
void near sub_s014_19cd_216a(int);
void near sub_s014_19cd_2232(int);
void far order_menu_reset(void);
void far order_menu_build(void);

/* m := combatant of the front-most hot spot under (x, y), or 99 */
#define TOPMOST(m, x, y) { int k_, by_ = 0; m = 99; \
    for (k_ = hotspot_count - 1; k_ >= 0; k_--) \
        if (HIT(k_, x, y) && by_ < combatants[(unsigned char)hotspot_cbt[k_]].y) { \
            m = (unsigned char)hotspot_cbt[k_]; by_ = combatants[m].y; } }
#define MAPY(my) ((my) - F_20a5_41f4)
#define HIT(k, x, y) (D_d8f1[k] <= (x) && (x) <= D_d8d7[k] && D_d8bd[k] <= (y) && (y) <= D_d8a3[k])

/* member cursor around the selected member, using hot spot k */
#define MEMBER_CURSOR(k) \
    D_e09f = D_d505[cbt_anim_set(selected_member)][combatants[selected_member].f1a] + D_d889[k] - 4; \
    D_e09d = (D_d5d3[cbt_anim_set(selected_member)] >> 1) + D_d86f[k] - 13

/* mouse handler */
void far combat_mouse(void)
{
    int mx, my;
    int k, i, n, r;
    int c1, c2, w1, h1, w2, h2, dx1, dy1, dx2, dy2, col1, col2, lead;
    int ex, ey, ry, rx;

    my = mouse_y;
    mx = mouse_x - 60;
    if (D_9409 != 0)
        return;
    if (move_mode != 0 && D_da4b < combat_clock)
        cursor_target = -1;
    if (move_mode == 1) {
        col1 = D_c941; dx1 = 11; dx2 = 11; dy1 = 25; D_e1c0 = 25; dy2 = 25;
        w1 = 0x16; w2 = 0x16; h1 = 0x1d; h2 = 0x1d; col2 = D_c987;
        lead = marching_order[0];
    } else if (move_mode == 2) {
        col1 = D_ca19; dx1 = 7; dx2 = 7; dy1 = 28; D_e1c0 = 28; dy2 = 28;
        w1 = 0xe; w2 = 0xe; h1 = 0x1f; h2 = 0x1f; col2 = D_ca2a;
        lead = marching_order[0];
    }
    /* hover */
    if ((D_d728 == 1 || (mouse_buttons & 1) != 0) && mx >= 0 && mx < 0x104 && (KBFLAGS & 4) == 0) {
        if (move_mode == 0) {
            if (order_mode == 0) {
                for (k = hotspot_count - 1; k >= 0; k--)
                    if (hotspot_cbt[k] < 5 && HIT(k, mx, my)) {
                        selected_member = hotspot_cbt[k];
                        MEMBER_CURSOR(k);
                        D_e1c6 = 8;
                        D_e1c4 = 0x1a;
                        order_action = -1;
                        D_c7cc = 0;
                        sub_s014_19cd_19ee();
                        break;
                    }
                if (k < 0)
                    selected_member = -1;
            } else
                sub_s014_19cd_0ad4(mx, my);
        } else {
            /* group cursor */
            D_d73e = combatants[marching_order[0]];
            ry = MAPY(my);
            D_d742 = ry - F_20a5_4be0 + mx - 16 + D_c26c;
            D_d744 = ry + D_c293;
            D_d788 = -1;
            D_d757 = 0x14;
            D_da4b = D_d7fe == 0 ? combat_clock + 60 : combat_clock + 1800;
            D_e1b4 = D_d742;
            D_e1b6 = D_d744;
            if (cbt_collisions(&D_d73e) == 0) {
                cursor_target = -2;
                sub_s014_19cd_1764(mx - dx1, my - dy1, w1, h1, col1, -1, 0);
            } else {
                cursor_target = -3;
                sub_s014_19cd_1764(mx - dx2, my - dy2, w2, h2, col2, -1, 0);
            }
        }
    }
    /* click */
    if (D_c62a == 0) {
        D_c4fe = (int *)sub_root_047c_1320(6, 0);
        D_c62a = D_c4fe[1];
    }
    if (D_c62a <= 0 && D_d73b != 1)
        return;
    if (D_d73b == 1) {
        if (move_mode != 0)
            D_da4b = combat_clock + 60;
        D_d73b = 0;
        ey = mouse_y;
        ex = mouse_x;
    } else {
        ex = D_c4fe[2];
        ey = D_c4fe[3];
    }
    ex -= 60;
    if (ex < 0)
        return;
    D_c62a = 0;
    if ((KBFLAGS & 4) != 0) {
        /* Ctrl-click on a member */
        for (k = hotspot_count - 1; k >= 0; k--)
            if (hotspot_cbt[k] < 5 && HIT(k, ex, ey)) {
                member_activate_vec(hotspot_cbt[k]);
                return;
            }
        return;
    }
    if (move_mode == 0) {
        if (order_mode == 0) {
            selected_member = -1;
            for (k = hotspot_count - 1; k >= 0; k--)
                if (hotspot_cbt[k] < 5 && HIT(k, ex, ey)) {
                    selected_member = hotspot_cbt[k];
                    MEMBER_CURSOR(k);
                    order_mode_enter();
                    return;
                }
        } else
            sub_s014_19cd_0dcc(ex, ey);
        return;
    }
    /* group move */
    ry = MAPY(ey);
    rx = ry - F_20a5_4be0 + ex - 16;
    D_da4b = combat_clock + 60;
    for (i = 0; i < 5; i++)
        if (combatants[i].status == 1)
            cbt_moving[i] = 1;
    if (sub_s014_19cd_11ec(lead, rx + D_c26c, ry + D_c293) != 1) {
        for (i = 0; i < 5; i++)
            if (combatants[i].status == 1)
                cbt_moving[i] = 0;
        cursor_target = -3;
        sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
        sub_s014_0e76_4694();
        return;
    }
    if (move_mode != 1) {
        /* single file: only the leader paths, the others follow */
        combatants[marching_order[0]].tx = rx + D_c26c;
        combatants[marching_order[0]].ty = ry + D_c293;
        if (path_find(marching_order[0]) == 1) {
            path_give(marching_order[0]);
            cursor_target = -2;
            sub_s014_19cd_1764(ex - dx1, ey - dy1, w1, h1, col1, -1, 0);
            combatants[marching_order[0]].f10 = 1;
            combatants[marching_order[0]].target = -1;
            D_d6fb[marching_order[0]] = 0;
            return;
        }
        cursor_target = -3;
        sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
        sub_s014_0e76_4694();
        cbt_moving[marching_order[0]] = 0;
        return;
    }
    for (k = 0; k < marching_len; k++) {
        n = marching_order[k];
        if (combatants[n].status != 1)
            continue;
        if (path_find(n) != 1) {
            cursor_target = -3;
            sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
            sub_s014_0e76_4694();
            for (i = 0; i < 5; i++)
                if (combatants[i].status == 1)
                    cbt_moving[i] = 0;
            return;
        }
        path_give(n);
        cursor_target = -2;
        sub_s014_19cd_1764(ex - dx1, ey - dy1, w1, h1, col1, -1, 0);
        combatants[n].f10 = 1;
        D_d6fb[n] = 0;
    }
}

/* enter order mode for selected_member */
struct a696bf { unsigned lo:5, b5:1, b6:1, mid:5, b12:1, hi:3; };

void far order_mode_enter(void)
{
    D_e1c6 = 8;
    D_e1c4 = 0x1a;
    *(volatile char *)&order_mode = 1;
    cbt_selected[selected_member] = 1;
    party_panel_frames(selected_member);
    party_panel_rows(0x100);
    D_d7fe = 1;
    D_d6ea = ((struct a696bf *)cbt_flags)[selected_member].b12;
    order_action = -1;
    D_a6d1 = -1;
    D_c7cc = 0;
    D_e1bc = 0;
    D_c652 = 0;
    sub_s014_19cd_19ee();
    if (real_combat == 0)
        sub_s014_19cd_2232(selected_member);
    order_menu_build();
    if (ai_paused == 0) {
        ai_paused = 1;                     /* pause the AI while giving orders */
        cursor_normal();
        D_c867 = combat_clock;
        D_07d3++;
        D_9609 = 1;
    }
}

/* leave order mode and apply the order */
void far order_mode_leave(void)
{
    int dx, dy, i;
    char *n;

    dy = D_c67d;
    dx = D_c652;
    D_d7fe = 0;
    if (order_action == -1 && D_c7cc == 0 && D_c652 == 0)
        goto out;
    i = selected_member;
    if (order_action == 0x400) {
        combatants[i].f10 = 0;
        cbt_special[i] = D_a6d1;
        cbt_flags[i][0] &= 0xef;
        goto out;
    }
    if (cursor_target == -3 || D_e1bc == D_c2bd)
        goto out;
    if (D_c652 != 0) {
        combatants[i].tx = D_e1b4 = D_c652;
        combatants[i].ty = D_e1b6 = dy;
    }
    if (order_action == -1)
        order_action = combatants[i].f10;
    if ((order_action & 0x1be) == 0x1be)
        cbt_flags[i][0] &= 0xef;
    if (cursor_target < 1) {
        if (cursor_target != -2)
            goto out;
        if (order_action != 0x100) {
            if (real_combat == 1 && party_floor != floor_shown)
                show_message((char *)0x64a, 0x96);   /* "USE STAIRS-your only option..." */
            else {
                if (D_c7cc == 1 || dx != 0) {
                    path_give_saved(i);
                    D_d6fb[i] = 0;
                }
                D_17b0 = i;
                combatants[i].tx = D_c652;
                combatants[i].ty = D_c67d;
                combatants[i].target = -1;
                cbt_moving[i] = 1;
                combatants[i].f10 = 1;
                cbt_action[i] = 0;
                cbt_anim[i] = 0;
                D_ca30[i] = -1;
                D_d97c[i] = D_c867 + 30;
            }
            goto out;
        }
        /* cast / use at a spot */
        cbt_moving[i] = 0;
        n = anim_file[combatants[i].kind];
        n[0] = 'P';
        n[1] = 'T';
        D_ca0c[i] = D_c1ce;
        D_c97a[i] = D_c1f2;
        combatants[i].f10 = 0x100;
        combatants[i].tx = dx;
        combatants[i].ty = dy;
        combatants[i].target = -1;
        cbt_engage_time[i] = D_c867;
    } else if (order_action == 0x80 || order_action == 0x100) {
        if (order_action == 0x80) {
            /* fire a missile at cursor_target */
            cbt_moving[i] = 0;
            weapon_ready(i, combatants[i].weapon, 0x80);
            combatants[i].target = cursor_target;
            combatants[i].f10 = 0x80;
            cbt_flags[i][0] |= 2;
            cbt_engage_time[i] = D_c867;
        } else {
            cbt_moving[i] = 0;
            n = anim_file[combatants[i].kind];
            n[0] = 'P';
            n[1] = 'T';
            D_ca0c[i] = D_c1ce;
            D_c97a[i] = D_c1f2;
            combatants[i].f10 = 0x100;
            if (cursor_target == -2) {
                combatants[i].tx = dx;
                combatants[i].ty = dy;
                combatants[i].target = -1;
            } else
                combatants[i].target = cursor_target;
            cbt_engage_time[i] = D_c867;
        }
    } else {
        /* melee attack on cursor_target */
        if ((D_c7cc == 1 || dx != 0) && D_c762 == 0)
            path_give_saved(i);
        weapon_ready(i, combatants[i].f51, 0);
        combatants[i].target = cursor_target;
        if (D_c7cc == 0) {
            if ((order_action & 2) != 2)
                goto out;
            combatants[i].f10 = order_action;
        } else if (D_c762 != 1) {
            cbt_moving[i] = 1;
            D_d6fb[i] = 0;
            combatants[i].f10 = 1;
            D_17b0 = i;
            if ((order_action & 2) == 2)
                order_queue(i, order_action);
            goto out;
        } else
            combatants[i].f10 = order_action;
    }
    cbt_action[i] = 7;
out:
    cbt_selected[selected_member] = 0;
    order_mode = 0;
    party_panel_frames(selected_member);
    party_panel_rows(0x100);
    order_action = -1;
    D_e1bc = 0;
    selected_member = -1;
    cursor_target = -1;
    order_menu_reset();
}

/* order mode: hover feedback at (x, y) */
void far sub_s014_19cd_0ad4(int x, int y)
{
    int k, f, col, h, w, m;

    TOPMOST(m, x, y);
    if (m < 5)
        return;                         /* over a party member */
    cursor_target = -1;
    f = combatants[selected_member].f10;
    k = hotspot_count - 1;
    if (order_action == 1 || (order_action == -1 && (f == 1 || f == 0)))
        k = -1;
    else
        for (; k >= 0; k--)
            if ((unsigned char)hotspot_cbt[k] > 4 && HIT(k, x, y)) {
                cursor_target = hotspot_cbt[k];
                col = D_c242;
                if (order_action == 0x80 || order_action == 0x100
                 || (order_action == -1 && (f == 0x80 || f == 0x100))) {
                    los_x0 = combatants[selected_member].x;
                    los_y0 = combatants[selected_member].y;
                    los_x1 = combatants[cursor_target].x;
                    los_y1 = combatants[cursor_target].y;
                    if (line_blocked(selected_member, cursor_target, 0x80) != 0)
                        col = D_c2bd;   /* no line of fire */
                }
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], col, 0, cursor_target);
                break;
            }
    if (k < 0 && ((order_action == -1 && f != 0x1082) || order_action == 1 || (order_action == 0x100 && D_e1c8 == 0))) {
        D_d73e = combatants[selected_member];
        h = MAPY(y);
        D_d742 = h - F_20a5_4be0 + x + D_c26c - 16;
        D_d744 = h + D_c293;
        D_d788 = -1;
        D_d757 = 0x14;
        if (order_action == 0x100)
            D_c600 = 1;
        D_e1b4 = D_d742;
        D_e1b6 = D_d744;
        if (cbt_collisions(&D_d73e) == 0) {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c28f;
            cursor_target = -2;
            if (order_action == 0x100 && D_e1c8 == 0) {
                los_x0 = combatants[selected_member].x;
                los_y0 = combatants[selected_member].y;
                los_x1 = D_d742;
                los_y1 = D_d744;
                h = 0xf;
                D_e1c0 = 0xf;
                w = 0x15;
                col = D_c4db;
                if (line_blocked(selected_member, selected_member, 0x100) == 1) {
                    col = D_c621;
                    cursor_target = -3;
                }
            }
        } else {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c244;
            cursor_target = -3;
        }
        sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
        if (order_action == 0x100)
            D_c600 = 0;
    }
    if (cursor_target == -1)
        D_e1bc = 0;
}

/* order mode: click at (x, y) */
void far sub_s014_19cd_0dcc(int x, int y)
{
    int k, f, col, h, w, m, i;

    TOPMOST(m, x, y);
    if (m < 5) {
        if (selected_member != -1)
            order_mode_leave();
        vec_1c39(m);
        return;
    }
    cursor_target = -1;
    D_e1bc = 0;
    if (party_floor != floor_shown) {
        show_message((char *)0x64a, 0x96);
        return;
    }
    D_c762 = 0;
    i = selected_member;
    f = combatants[i].f10;
    k = hotspot_count - 1;
    if (order_action == 1 || (order_action == -1 && (f == 1 || f == 0)))
        k = -1;
    else
        for (; k >= 0; k--)
            if ((unsigned char)hotspot_cbt[k] >= 5 && HIT(k, x, y)) {
                D_d73e = combatants[i];
                cursor_target = hotspot_cbt[k];
                combatants[i].tx = D_e1b4 = combatants[cursor_target].x;
                combatants[i].ty = D_e1b6 = combatants[cursor_target].y;
                combatants[i].target = cursor_target;
                if (order_action == 0x100 || order_action == 0x80) {
                    los_x0 = combatants[i].x;
                    los_y0 = combatants[i].y;
                    los_x1 = combatants[i].tx;
                    los_y1 = combatants[i].ty;
                    if (line_blocked(i, cursor_target, 0x100) == 1) {
                        sub_s014_0e76_4694();
                        col = D_c2bd;
                    } else {
                        D_c7cc = 1;
                        col = D_c242;
                    }
                } else {
                    if (F_20a5_7900[i][cursor_target] == 0) {
                        if (path_find(i) != 1) {
                            sub_s014_0e76_4694();
                            col = D_c2bd;
                            goto show;
                        }
                        path_save();
                    } else
                        D_c762 = 1;
                    D_c7cc = 1;
                    col = D_c242;
                }
show:
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], col, 0, cursor_target);
                combatants[selected_member] = D_d73e;
                break;
            }
    if (cursor_target == -3 || k >= 0)
        return;
    if (!((order_action == -1 && f != 0x1082) || order_action == 1 || (order_action == 0x100 && D_e1c8 == 0)))
        return;
    /* click on the ground */
    D_d73e = combatants[i];
    D_e1b6 = MAPY(y);
    combatants[i].tx = D_e1b4 = D_e1b6 - F_20a5_4be0 + x + D_c26c - 16;
    combatants[i].ty = D_e1b6 = D_e1b6 + D_c293;
    combatants[i].target = -1;
    combatants[i].f19 = 0x14;
    if (order_action == 0x100 && (D_c600 = 1, D_e1c8 == 0)) {
        los_x0 = combatants[i].x;
        los_y0 = combatants[i].y;
        los_x1 = combatants[i].tx;
        los_y1 = combatants[i].ty;
        h = 0xf;
        D_e1c0 = 0xf;
        w = 0x15;
        col = D_c4db;
        if (line_blocked(i, i, 0x100) == 1) {
            col = D_c621;
            goto blocked;
        }
    } else {
        if (path_find(i) != 1) {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c244;
blocked:
            cursor_target = -3;
            sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
            sub_s014_0e76_4694();
            goto restore;
        }
        path_save();
        h = 0x1a;
        D_e1c0 = 0x1a;
        w = 0x1b;
        col = D_c28f;
    }
    cursor_target = -2;
    sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
    D_c652 = combatants[selected_member].tx;
    D_c67d = combatants[selected_member].ty;
restore:
    if (order_action == 0x100)
        D_c600 = 0;
    combatants[selected_member] = D_d73e;
}

extern struct item6 { int id; unsigned char f2; unsigned char f3; unsigned char cnt; char f5; }
    far inventories[][64];              /* party inventories */
extern char far activations[][30];      /* activation records */
extern unsigned char activation_count;            /* their count */
extern char D_c6ef[], D_c678[], D_c64c[], D_c7cd[], D_c77e[], D_c8b2[], D_c784[], D_c7fe[];
extern char cbt_floor[];                   /* [13] floor */
extern char D_c81c[];                   /* [13] loot pile under the combatant */
extern char loot_pile_count;                     /* loot piles */
extern char D_c865;
extern char leader;                     /* leader */
extern char D_96a9, D_96bd, D_96d1, D_96e5, D_96f9, D_970d, D_9735, D_9749; /* menu enables */
extern int D_970b;                      /* menu text */
extern char D_ca99;
extern char floor_dirty, D_ca29;
extern int battle_result;                      /* battle result */
extern int battle_args, D_da4d, D_d9e4;
extern int D_c925, D_c978;
extern char D_c989, D_ca1b;
extern char D_0570[];                   /* " discovers a trap!" */
void far trap_spell_vec(int, int, int, int);  /* trap handlers in root 09c0 */
void far trap_missile_vec(int, int, int, int);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);

#define INSIDE(r, px, py) ((r).x <= (px) && (px) < (r).x + (r).w && (r).y <= (py) && (py) < (r).y + (r).h)

/* group move of the formation led by `lead` to (px, py); 1 ok */
int far sub_s014_19cd_11ec(int lead, int px, int py)
{
    int sy[13], sx[13];
    int lo, hi, k, r, dir, ox, oy, d, adx, ady, ratio, slope, quad, last, a1, a2;
    char far *path;

    if (lead < 5) {
        lo = 0;
        hi = 5;
    } else {
        lo = 5;
        hi = 13;
    }
    for (k = lo; k < hi; k++) {
        sx[k] = combatants[k].x;
        sy[k] = combatants[k].y;
    }
    combatants[lead].tx = px;
    combatants[lead].ty = py;
    if (path_find(lead) != 1)
        return 0;
    r = 1;
    if (move_mode == 1) {
        /* formation: facing from the direction of the move */
        if ((int)distance3((px - combatants[lead].x) / 2, py - combatants[lead].y, 0) < 49) {
            d = (px - combatants[lead].x) / 2;
            k = py - combatants[lead].y;
            adx = abs(d);
            ady = abs(k);
            ratio = adx == 0 ? 20 : ady * 4 / adx;
            if (ratio == 0)
                slope = 0;
            else if (ratio < 2 || ratio > 7)
                slope = 2;
            else
                slope = 1;
            if (d >= 0 && k < 0)
                quad = 0;
            else if (d >= 0 && k >= 0)
                quad = 1;
            else if (d < 0 && k >= 0)
                quad = 2;
            else
                quad = 3;
            dir = D_17b2[quad][slope];
        } else {
            /* facing along the end of the leader's path */
            path_give(lead);
            path = cbt_paths[lead];
            if (path[D_e118 - 1] == -1)
                D_e118--;
            last = D_179c[(unsigned char)path[D_e118 - 1] >> 4];
            a1 = dir_add(D_179c[(unsigned char)path[D_e118 - 2] >> 4], last) & 0xff;
            a2 = dir_add(D_179c[(unsigned char)path[D_e118 - 3] >> 4], last) & 0xff;
            dir = dir_sub(last, (a1 * 2 + 1) / 3) & 0xff;
        }
        ox = 0;
        oy = 0;
        /* standing in a doorway, on stairs etc.: shift the formation by half a rank */
        for (k = 0; k < F_20a5_1f52; k++)
            if (F_20a5_1f54[k].x <= px && px <= F_20a5_1f54[k].w + F_20a5_1f54[k].x
             && F_20a5_1f54[k].y <= py && py <= F_20a5_1f54[k].h + F_20a5_1f54[k].y)
                goto shift;
        for (k = 0; k < F_20a5_2fb6; k++)
            if (F_20a5_2fb8[k].x <= px && px <= F_20a5_2fb8[k].w + F_20a5_2fb8[k].x
             && F_20a5_2fb8[k].y <= py && py <= F_20a5_2fb8[k].h + F_20a5_2fb8[k].y)
                goto shift;
        for (k = 0; k < F_20a5_300c; k++)
            if (F_20a5_300e[k].x <= px && px <= F_20a5_300e[k].w + F_20a5_300e[k].x
             && F_20a5_300e[k].y <= py && py <= F_20a5_300e[k].h + F_20a5_300e[k].y)
                goto shift;
        for (k = 0; k < F_20a5_3326; k++)
            if (F_20a5_3328[k].x <= px && px <= F_20a5_3328[k].w + F_20a5_3328[k].x
             && F_20a5_3328[k].y <= py && py <= F_20a5_3328[k].h + F_20a5_3328[k].y)
                goto shift;
        goto place;
shift:
        ox = D_1ba2[dir] / 2;
        oy = D_1bba[dir] / 2;
place:
        los_y0 = py;
        oy += py;
        los_x0 = px;
        ox += px;
        r = sub_s014_19cd_164a(ox, oy, dir, lo, hi);
        if (r == 0)
            r = sub_s014_19cd_164a(ox, oy, dir_add(dir, 1) & 0xff, lo, hi);
        if (r == 0)
            r = sub_s014_19cd_164a(ox, oy, dir_sub(dir, 1) & 0xff, lo, hi);
    }
    /* the placed positions become destinations */
    for (k = lo; k < hi; k++) {
        combatants[k].tx = combatants[k].x;
        combatants[k].ty = combatants[k].y;
        combatants[k].x = sx[k];
        combatants[k].y = sy[k];
    }
    if (r == 0)
        for (k = lo; k < hi; k++)
            cbt_moving[k] = 0;
    return r;
}

/* place the marching order in formation `dir` around (px, py); 1 if every spot is free */
#pragma optimize("a", on)
#pragma optimize("l", off)
int far sub_s014_19cd_164a(int px, int py, int dir, int lo, int hi)
{
    int k, row, m;

    for (k = 0; k < marching_len; k++) {
        row = (k - lo) >> 1;
        m = marching_order[k];
        combatants[m].x = D_1b9a[dir] + row * D_1baa[dir] + px + ((k & 1) == 1 ? D_1ba2[dir] : 0);
        combatants[m].y = D_1bb2[dir] + row * D_1bc2[dir] + ((k & 1) == 1 ? D_1bba[dir] : 0) + py;
        if (sub_s014_19cd_1714(m) == 0)
            return 0;
    }
    return 1;
}
#pragma optimize("a", off)
#pragma optimize("l", on)

/* 1 if combatant i's (trial) position is free and visible from the clicked point */
int far sub_s014_19cd_1714(int i)
{
    if (cbt_collisions(&combatants[i]) == 0) {
        los_x1 = combatants[i].x;
        los_y1 = combatants[i].y;
        if (line_blocked(-1, 0, 0) == 0)
            return 1;
    }
    return 0;
}

/* cursor box: free (mode -1) or around combatant `who` */
void far sub_s014_19cd_1764(int x, int y, int w, int h, int col, int mode, int who)
{
    if (mode == -1) {
        D_e1c2 = x;
        D_e1be = y;
        D_e1b8 = w;
        D_e1ba = h;
    } else {
        D_e1c2 = D_d505[cbt_anim_set(who)][combatants[who].f1a] + x - 5;
        D_e1be = D_d5d3[cbt_anim_set(who)] - D_d5de[cbt_anim_set(who)] + y - 5;
        D_e1b8 = 10;
        D_e1ba = 10;
    }
    D_e1bc = col;
}

/* keep the cursors on the moving combatants */
#pragma optimize("g", off)
void far sub_s014_19cd_17f6(void)
{
    int k, f, col;

    if (order_mode == 1) {
        for (k = hotspot_count - 1; k >= 0; k--)
            if (hotspot_cbt[k] == selected_member) {
                MEMBER_CURSOR(k);
                D_e1c6 = 8;
                D_e1c4 = 0x1a;
                break;
            }
    }
    if (cursor_target >= 0) {
        for (k = hotspot_count - 1; k >= 0; k--)
            if (hotspot_cbt[k] == cursor_target)
                break;
        if (k >= 0) {
            col = D_e1bc;
            f = combatants[selected_member].f10;
            if (order_action == 0x80 || order_action == 0x100 || (order_action == -1 && (f == 0x80 || f == 0x100))) {
                los_x0 = combatants[selected_member].x;
                los_y0 = combatants[selected_member].y;
                los_x1 = combatants[cursor_target].x;
                los_y1 = combatants[cursor_target].y;
                col = line_blocked(selected_member, cursor_target, 0x80) == 0 ? D_c242 : D_c2bd;
            }
            D_e1be = D_d86f[k];
            sub_s014_19cd_1764(D_d889[k], D_e1be, D_d862[k], D_d855[k], col, 0, cursor_target);
        } else
            D_e1bc = 0;
    }
}
#pragma optimize("g", on)

/* clip the member cursor to the view (0..0x103 x 0..0xc7) */
void far sub_s014_19cd_1966(void)
{
    int x, w, h;

    x = D_e09f;
    w = D_e1c6;
    if (x < 0) {
        if ((w += x) < 0)
            w = 0;
        x = 0;
    }
    if (x > 0x103)
        w = 0;
    D_e1c6 = w;
    D_e09f = x;
    if (D_e1c6 + D_e09f - 1 > 0x103 && (D_e1c6 = 0x104 - D_e09f) < 0)
        D_e1c6 = 0;
    h = D_e1c4;
    if (D_e09d < 0) {
        if ((h += D_e09d) < 0)
            h = 0;
        D_e09d = 0;
    }
    if (D_e09d > 0xc7)
        h = 0;
    D_e1c4 = h;
    if (D_e09d + h - 1 > 0xc7 && (D_e1c4 = 0xc8 - D_e09d) < 0)
        D_e1c4 = 0;
}

/* initial cursor of the selected member's current order */
void far sub_s014_19cd_19ee(void)
{
    int k, w, col;
    struct cbt *c;

    c = &combatants[selected_member];
    if (c->target == -1) {
        if (cbt_moving[selected_member] != 1 && c->f10 != 0x100 && c->f10 != 1) {
            cursor_target = -1;
            D_e1bc = 0;
            return;
        }
        if (c->f10 == 0x100) {
            D_e1c0 = 0xf;
            w = 0x15;
            col = D_c4db;
        } else {
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c28f;
        }
        D_e1b4 = c->tx;
        D_e1b6 = c->ty;
        cursor_target = -2;
        sub_s014_19cd_1764(0, 0, 0xb, w, col, -1, 0);
    } else {
        for (k = 0; k < hotspot_count; k++)
            if (c->target == (unsigned char)hotspot_cbt[k]) {
                cursor_target = hotspot_cbt[k];
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], D_c242, 0, cursor_target);
                return;
            }
    }
}

/* best lockpick (item 99) of member i -> D_c7cd[i] */
#define FIND_PICK(i) { int n_; unsigned char b_ = 100; D_c7cd[i] = -1; \
    for (n_ = 0; n_ < combatants[i].nitems; n_++) \
        if (inventories[i][n_].id == 99 && inventories[i][n_].f3 < b_) { \
            D_c7cd[i] = n_; b_ = inventories[i][n_].f3; } }

#define FIND_PICK2(i) { int n_; int b_ = 100; D_c7cd[i] = -1; \
    for (n_ = 0; n_ < combatants[i].nitems; n_++) \
        if (inventories[i][n_].id == 99 && inventories[i][n_].f3 < b_) { \
            D_c7cd[i] = n_; b_ = inventories[i][n_].f3; } }

/* door under combatant i (flag 2) and the leader's door menu entries */
void near sub_s014_19cd_1ade(int i)
{
    int k;
    int v;
    unsigned long c;
    int f;
    char lock;
    int x0, x1, y0, y1;

    for (k = 0; k < F_20a5_1f52; k++) {
        if ((x0 = F_20a5_1f54[k].x) <= 0)
            continue;
        x1 = F_20a5_1f54[k].w + x0;
        y0 = F_20a5_1f54[k].y;
        y1 = y0 + F_20a5_1f54[k].h;
        if (combatants[i].x >= x0 && combatants[i].x < x1 && combatants[i].y >= y0 && combatants[i].y < y1) {
            c = F_20a5_0000[(F_20a5_1f54[k].f8 & 0x3f) * 40 + (F_20a5_1f54[k].f9 & 0x3f)];
            if ((F_20a5_1f54[k].f8 & 0x80) == 0x80)
                D_c6ef[i] = 0;
            else if ((F_20a5_1f54[k].f8 & 0x40) == 0x40)
                D_c6ef[i] = 1;
            else if ((F_20a5_1f54[k].f9 & 0x80) == 0x80)
                D_c6ef[i] = 2;
            else if ((F_20a5_1f54[k].f9 & 0x40) == 0x40)
                D_c6ef[i] = 3;
            switch ((unsigned char)D_c6ef[i]) {        /* wall of the door cell on that side */
            case 0: v = (int)(c >> 16) & 0xf; break;
            case 1: v = (int)c & 0xf; break;
            case 2: v = (int)(c >> 24) & 0xf; break;
            case 3: v = (int)(c >> 8) & 0xf; break;
            }
            cbt_flags[i][0] |= 2;
            if (v < 8)
                D_c678[i] = 1;
            else
                D_c678[i] = 0;   /* door open */
            D_c64c[i] = k;
            break;
        }
        cbt_flags[i][0] &= 0xfd;
    }
    if (leader != i || ai_paused != 0)
        return;
    f = *(int *)cbt_flags[i];
    if ((f & 2) == 2) {
        lock = F_20a5_1f54[(unsigned char)D_c64c[i]].f7;
        if (lock != 0 && real_combat == 0) {
            D_96a9 = 2;
            D_96f9 = 2;
            D_96e5 = 1;
            D_ca99 = 1;
            FIND_PICK2(i);
            if (D_c7cd[i] == -1)
                return;
            D_96f9 = 1;
            return;
        }
        if (lock != 0)
            return;
        D_96a9 = 1;
    } else {
        D_96a9 = 2;
        if ((f & 4) != 0)
            return;
    }
    D_96f9 = 2;
    D_96e5 = 2;
}

/* remove activation record k */
struct atv30 { char b[30]; };
void far sub_s014_19cd_1d5e(int k)
{
    int i;

    if (activation_count != 0) {
        for (i = k; i < activation_count - 1; i++)
            ((struct atv30 far *)activations)[i] = ((struct atv30 far *)activations)[i + 1];
        activation_count--;
    }
}

/* stairs under combatant i (flag 1) */
struct bit0 { unsigned b0:1; };

void near sub_s014_19cd_1db8(int i)
{
    int k, x1, y0, y1;

    for (k = 0; k < F_20a5_2fb6; k++) {
        x1 = F_20a5_2fb8[k].w + F_20a5_2fb8[k].x;
        y0 = F_20a5_2fb8[k].y;
        y1 = y0 + F_20a5_2fb8[k].h;
        if (combatants[i].x >= F_20a5_2fb8[k].x && combatants[i].x < x1
         && combatants[i].y >= y0 && combatants[i].y < y1) {
            D_c77e[i] = k;
            cbt_flags[i][0] |= 1;
            break;
        }
        cbt_flags[i][0] &= 0xfe;
    }
    if (leader == i && ai_paused == 0) {
        if (party_floor != floor_shown) {
            D_96bd = 1;
            D_c77e[i] = D_c8b2[i];
            return;
        }
        D_96bd = ((struct bit0 *)cbt_flags[i])->b0 == 1 ? 1 : 2;
    }
}

/* special region (chest, lock...) under combatant i (flag 4) */
void near sub_s014_19cd_1e92(int i)
{
    int k;
    struct region far *r;

    for (k = 0; k < F_20a5_300c; k++) {
        if (F_20a5_300e[k].x <= 0)
            continue;
        if (INSIDE(F_20a5_300e[k], combatants[i].x, combatants[i].y)) {
            D_c784[i] = k;
            cbt_flags[i][0] |= 4;
            break;
        }
        cbt_flags[i][0] &= 0xfb;
    }
    if (leader != i || ai_paused != 0)
        return;
    if ((cbt_flags[i][0] & 4) == 4) {
        D_96d1 = 2;
        D_96e5 = 2;
        D_96f9 = 2;
        D_970d = 2;
        r = &F_20a5_300e[(unsigned char)D_c784[i]];
        if ((r->lock & 0x7f) == 0 || real_combat != 0) {
            if (r->acid == 0 || real_combat != 0) {
                if ((r->lock & 0x7f) == 0 && r->acid == 0 && real_combat == 0)
                    D_96d1 = 1;         /* open chest */
            } else {
                D_970d = 1;             /* dissolve lock: strength of the acid */
                switch (r->acid) {
                case 0x50: D_970b = 0x628; break;
                case 0x14: D_970b = 0x5f2; break;
                case 0x32: D_970b = 0x60c; break;
                }
            }
        } else {
            D_96e5 = 1;                 /* pick lock */
            D_ca99 = 0;
            FIND_PICK(i);
            if (D_c7cd[i] != -1)
                D_96f9 = 1;
        }
    } else {
        D_96d1 = 2;
        if ((cbt_flags[i][0] & 2) == 0) {
            D_96e5 = 2;
            D_96f9 = 2;
            D_970d = 2;
        }
    }
}

/* exit zone under combatant i (flag 0x20) */
void near sub_s014_19cd_209a(int i)
{
    int k, x0, x1, y0, y1;

    for (k = 0; k < F_20a5_3326; k++) {
        if ((x0 = F_20a5_3328[k].x) <= 0)
            continue;
        x1 = F_20a5_3328[k].w + x0;
        y0 = F_20a5_3328[k].y;
        y1 = y0 + F_20a5_3328[k].h;
        if (cbt_floor[i] == floor_shown && combatants[i].x >= x0 && combatants[i].x < x1
         && combatants[i].y >= y0 && combatants[i].y < y1) {
            D_c7fe[i] = k;
            cbt_flags[i][0] |= 0x20;
            break;
        }
        cbt_flags[i][0] &= 0xdf;
    }
    if (leader == i && ai_paused == 0)
        D_9749 = (cbt_flags[i][0] & 0x20) == 0x20 ? 1 : 2;
}

/* trigger areas: call the handler of every area combatant i stands in
   (enemies only spring the two trap handlers trap_missile_vec / trap_spell_vec) */
void near sub_s014_19cd_216a(int i)
{
    int k, x0, x1, y0, y1;

    for (k = 0; k < F_20a5_2184; k++) {
        x0 = F_20a5_2186[k].x;
        x1 = x0 + F_20a5_2186[k].w;
        y0 = F_20a5_2186[k].y;
        y1 = y0 + F_20a5_2186[k].h;
        if (x0 > 0 && combatants[i].x >= x0 && combatants[i].x < x1 && combatants[i].y >= y0 && combatants[i].y < y1
         && (i < 5 || i >= 5 && (F_20a5_2186[k].fn == trap_missile_vec || F_20a5_2186[k].fn == trap_spell_vec)))
            (*F_20a5_2186[k].fn)(i, k, F_20a5_2186[k].a, F_20a5_2186[k].b);
    }
}

/* loot pile next to combatant i */
void near sub_s014_19cd_2232(int i)
{
    int x, y, k;

    D_c81c[i] = -1;
    x = combatants[i].x;
    y = combatants[i].y;
    for (k = 0; k < (unsigned char)loot_pile_count; k++)
        if (loot_piles[k].level == floor_shown && loot_piles[k].cnt != 0
         && x - 16 <= loot_piles[k].x && loot_piles[k].x < x + 16
         && y - 16 <= loot_piles[k].y && loot_piles[k].y < y + 16) {
            D_c81c[i] = k;
            break;
        }
    D_c865 = D_c81c[i] != -1;
    if (leader == i && ai_paused == 0)
        D_9735 = D_c865 && real_combat == 0 ? 1 : 2;
}

/* trigger: pit / trapdoor - fires when the whole party stands in area k */
#pragma optimize("a", on)
#pragma optimize("e", off)
void far sub_s014_19cd_2312(int i, int k, int n, int b)
{
    int m;

    for (m = 0; m < 5; m++)
        if (combatants[m].status == 1 && !INSIDE(F_20a5_2186[k], combatants[m].x, combatants[m].y))
            return;
    F_20a5_2186[k].x = -1;
    floor_dirty = 1;
    effect_start(F_20a5_3350[n].a, F_20a5_3350[n].b, F_20a5_3350[n].c);
    D_ca29 = 1;
    battle_result = 6;
    battle_args = b;
    show_message("Aaaahh!", 60);
}
#pragma optimize("a", off)
#pragma optimize("e", on)

/* trigger: remove furniture n (alarm) */
void far sub_s014_19cd_23f6(int i, int k, unsigned char n, int b)
{
    /* orig keeps the far address of the record in di/[bp-2] (es from slot 7daa): 6.00 CSE of
       a far address, 6.00A only keeps the offset */
    if (real_combat != 1) {
        battle_args = b;
        if (F_20a5_3350[n].on == 1)
            vec_1c4d(n, k);
        battle_result = 7;
        if (D_ca29 == 0) {
            D_da4d = combat_clock;
            D_d9e4 = combat_clock + 1000;
            D_ca29 = 1;
        }
        effect_start(F_20a5_3350[n].a, F_20a5_3350[n].b, F_20a5_3350[n].c);
        D_c925 = F_20a5_2186[k].x;
        D_c978 = F_20a5_2186[k].y;
        D_c989 = F_20a5_2186[k].w;
        D_ca1b = F_20a5_2186[k].h;
    }
}

/* trigger: remove wall n */
void far sub_s014_19cd_24a8(int i, int k, unsigned char n, int b)
{
    if (real_combat != 1) {
        battle_args = b;
        if (F_20a5_3382[n].on == 1)
            vec_1c57(n, k);
        battle_result = 7;
        if (D_ca29 == 0) {
            D_da4d = combat_clock;
            D_d9e4 = combat_clock + 1000;
            D_ca29 = 1;
        }
        effect_start(F_20a5_3382[n].a, F_20a5_3382[n].b, F_20a5_3382[n].c);
        D_c925 = F_20a5_2186[k].x;
        D_c978 = F_20a5_2186[k].y;
        D_c989 = F_20a5_2186[k].w;
        D_ca1b = F_20a5_2186[k].h;
    }
}

/* trigger: hidden trap - member i with perception above `skill` discovers it and the trap
   is drawn into the map (4 patches of trap n) */
#pragma optimize("a", on)
void far sub_s014_19cd_255a(int i, int k, int n, int skill)
{
    char msg[60];
    int m, sh;
    unsigned v, f;
    long far *cell;

    if (combatants[i].f60 <= skill)
        return;
    F_20a5_2186[k].x = -F_20a5_2186[k].x;
    for (m = 0; m < 4; m++) {
        v = F_20a5_3236[n].col[m];
        if (v == 0)
            continue;
        if (F_20a5_3236[n].f[m] < 0x80)
            F_20a5_1900[F_20a5_3236[n].row[m] + v * 40] |= F_20a5_3236[n].f[m];
        else {
            *(volatile int *)&sh = (v & 0x80) == 0 ? 0xc : 0x14;
            cell = &F_20a5_0000[F_20a5_3236[n].row[m] + (v & 0x3f) * 40];
            *cell |= (long)(F_20a5_3236[n].f[m] & 0x7f) << sh;
        }
    }
    strcpy(msg, combatants[i].name);
    strcat(msg, D_0570);
    show_message(msg, 150);
    floor_view_draw_vec(-1);
    for (m = 0; m < 5; m++)
        cbt_moving[m] = 0;
}
#pragma optimize("a", off)

/* per tick: locations of the party (door / stairs / region / exit flags) and triggers */
void far party_locations(void)
{
    int i, k, m;
    unsigned char old, was;

    for (i = 0; i < 5; i++) {
        if (combatants[i].status != 1)
            continue;
        was = (cbt_flags[i][0] & 2) >> 1;
        sub_s014_19cd_1ade(i);
        if (was == 0 && (cbt_flags[i][0] & 2) == 2) {
            cbt_flags[i][0] &= 0xda;
            goto entered;
        }
        if (was == 1 && (cbt_flags[i][0] & 2) == 0) {
            cbt_flags[i][0] &= 0xda;
            goto left;
        }
        old = cbt_flags[i][0];
        sub_s014_19cd_1db8(i);
        if ((old & 1) == 0 && (cbt_flags[i][0] & 1) == 1) {
            cbt_flags[i][0] &= 0xd9;
            goto entered;
        }
        if ((old & 1) == 1 && (cbt_flags[i][0] & 1) == 0) {
            cbt_flags[i][0] &= 0xd9;
            goto left;
        }
        was = (cbt_flags[i][0] & 4) >> 2;
        sub_s014_19cd_1e92(i);
        if (was == 0 && (cbt_flags[i][0] & 4) == 4) {
            cbt_flags[i][0] &= 0xdc;
            goto entered;
        }
        if (was == 1 && (cbt_flags[i][0] & 4) == 0) {
            cbt_flags[i][0] &= 0xdc;
            goto left;
        }
        was = (cbt_flags[i][0] & 0x20) >> 5;
        sub_s014_19cd_209a(i);
        if (was == 0 && (cbt_flags[i][0] & 0x20) == 0x20) {
            cbt_flags[i][0] &= 0xf8;
            goto entered;
        }
        if (was == 1 && (cbt_flags[i][0] & 0x20) == 0) {
            cbt_flags[i][0] &= 0xf8;
            goto left;
        }
        sub_s014_19cd_216a(i);
        cbt_flags[i][0] &= 0xf7;
        continue;
entered:
        cbt_flags[i][0] |= 8;
left:
        party_panel_rows(i);
    }
    for (i = 5; i < 13; i++)
        if (combatants[i].status == 1)
            sub_s014_19cd_216a(i);
    /* enemies open closed, unlocked doors they walk into */
    for (k = 0; k < F_20a5_1f52; k++)
        if (F_20a5_33d3[k] == 0 && F_20a5_1f54[k].f7 == 0 && F_20a5_1f54[k].x > 0)
            for (m = 5; m < 13; m++)
                if (combatants[m].status == 1 && INSIDE(F_20a5_1f54[k], combatants[m].x, combatants[m].y)) {
                    door_set_vec(k, 1, 1);
                    floor_view_draw_vec(-1);
                    break;
                }
}

extern char D_06ab[], D_06bb[], D_06cd[], D_06df[], D_06f1[], D_0727[];   /* menu labels */
extern char D_073e[], D_0744[];         /* "(Ldr)", "(All)" */
extern char D_9433, D_961d, D_9645, D_9659, D_966d, D_975d; /* menu enables */
extern char D_97dd, D_97f1, D_9805, D_9819, D_982d, D_9841;
struct menu { char on; char pad[0x13]; };
#define D_97dd_ ((struct menu *)&D_97dd)  /* the 6 combat action entries */
extern char D_c107[];                   /* menu title */
extern char *D_11f2;
extern char D_d7ff;
extern struct mod { char pad[9]; char f9; char pad2[8]; } D_a532[];
extern char D_1b98;                     /* a loot cache exists */
extern int D_d846;
extern struct loot far *D_c654;         /* loot list buffer */
extern unsigned D_c67f;                 /* entries in it */
extern unsigned char D_c783;            /* items of the fallen this fight */
extern unsigned char D_c83e;
extern char *D_1162;                    /* loot file name */
extern char far *load_buf;                /* loot file image */
extern struct loot *loot_list;   /* POINTER to the near copy of the list (set by s002: &msg_file), not an array */
void far *_fmemcpy(void far *, const void far *, unsigned);
extern int D_c7a4[20];
extern char cbt_anim_len[13];
extern char msg_buf[];
extern char D_9429, D_943d, D_9893;
extern int D_d6ee, D_d6f2, D_d6f9, D_d729, D_d70a, D_d73c, D_d726, D_d800;  /* level buffers */
extern char D_07d8;
extern char D_9069, D_9c4c;
extern int D_ca55, D_ca53, cursor_default, cursor_current, D_a691, D_a693;
extern int D_9920, D_9934;
extern char *D_11bc[6];
extern int anim_buffers[];
extern int shield_pics[5];                   /* shield pictures of the party */
extern int D_e1cb[5];
struct i5 { int v[5]; };
struct b13 { char b[13]; };
int far _alloc_buffer(unsigned, int, char *);
void far _free_buffer(int, char *);
void far palette_save(int, int);  /* save palette */
void far palette_restore(int, int);  /* restore palette */
void far ems_map_page(int, int, int);
extern int ems_frame_handle;
void far mouse_hide(void);
void far mouse_set_cursor(int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_04ea(int, int);
void far mouse_show(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_0545(void);
void far sub_root_047c_04f8(void);
int far file_load_far(char *, char far *);      /* load a file, returns its size */
void far file_save(char *, char far *, int); /* write a file */
int far pic_load(char *);                   /* load a picture */
void far sub_root_06a1_3108(char far *, char far *, int); /* far memmove */
void far cbt_set_anim(int, int);
void far loot_screen(struct loot far *, unsigned *, void far *, int);
int far sub_s014_19cd_3204(struct loot *, int);
void far sub_s014_19cd_3258(char *, int *, int, int);
void far sub_s014_19cd_33c0(struct loot far *, int);
void far shields_load(void);
void far shields_free(void);
typedef struct _iobuf FILE;
FILE *fopen(const char *, const char *);
int fseek(FILE *, long, int);
long ftell(FILE *);
unsigned fwrite(const void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
#define CURSOR_BUSY() ems_map_page(3, 10, ems_frame_handle)
#define CURSOR_IDLE() ems_map_page(3, 3, ems_frame_handle)

/* reset the order menu */
void far order_menu_reset(void)
{
    struct menu *m;

    D_9433 = 2;
    for (m = D_97dd_; m < D_97dd_ + 6; m++)
        m->on = 2;
    D_9609 = 2;
    D_961d = 2;
    if (party_floor != floor_shown)
        D_9645 = 2;
    else
        D_9645 = 1;
    strcpy(D_06ab + 8, D_073e);
    D_9659 = 2;
    D_96a9 = 2;
    D_96bd = 2;
    if (move_mode == 0)
        strcpy(D_06bb + 10, D_073e);
    else
        strcpy(D_06bb + 10, D_0744);
    strcpy(D_06cd + 10, D_073e);
    strcpy(D_06df + 9, D_073e);
    strcpy(D_06f1 + 13, D_073e);
    D_970b = 0x60c;
    D_96d1 = 2;
    D_96e5 = 2;
    D_96f9 = 2;
    D_970d = 2;
    D_9735 = 2;
    D_9749 = 2;
    if (move_mode == 0)
        strcpy(D_0727 + 16, D_073e);
    else
        strcpy(D_0727 + 16, D_0744);
    D_975d = 2;
}

/* build the order menu of the selected member */
void far order_menu_build(void)
{
    struct menu *m;
    int a;
    unsigned char fl, lock;
    struct region far *r;

    order_menu_reset();
    D_06ab[8] = 0;
    D_06bb[10] = 0;
    D_06cd[10] = 0;
    D_06df[9] = 0;
    D_06f1[13] = 0;
    D_0727[16] = 0;
    D_970b = 0x5c6;
    D_9659 = 1;
    D_966d = 1;
    D_9609 = 1;
    D_975d = 1;
    a = selected_member;
    if (party_floor != floor_shown) {
        D_96bd = 1;
        D_9645 = 2;
        D_9659 = 2;
        D_966d = 2;
        D_c77e[a] = D_c8b2[a];
        return;
    }
    D_9645 = 1;
    strcpy(D_c107, "\x02");
    strcat(D_c107, combatants[a].name);
    strcat(D_c107, D_11f2);
    D_961d = 1;
    D_9735 = D_c865 == 1 && real_combat == 0 ? 1 : 2;
    if (real_combat == 1) {
        D_9433 = 1;
        for (m = D_97dd_; m < D_97dd_ + 6; m++)
            m->on = 1;
        D_d7ff = 1;
        vec_1da1(a);
        a = selected_member;
        if (D_a532[a].f9 == 1) {
            D_9645 = 2;
            D_97dd = 2;
            D_97f1 = 2;
            D_9805 = 2;
            D_9819 = 2;
            D_982d = 2;
            D_9841 = 2;
            return;
        }
        if (combatants[a].weapon == -1 || can_fire(a) == 0)
            D_9841 = 2;                 /* no missile weapon / ammo: no "fire" */
    }
    a = selected_member;
    fl = cbt_flags[a][0];
    if ((fl & 2) == 2) {
        lock = F_20a5_1f54[(unsigned char)D_c64c[a]].f7;
        if (lock == 0 || real_combat != 0)
            D_96a9 = 1;
        else {
            D_ca99 = 1;
            D_96e5 = 1;
            if (lock > 0x7f) {
                FIND_PICK(a);
                if (D_c7cd[a] != -1)
                    D_96f9 = 1;
            }
        }
        D_d7ff = 2;
    }
    if ((fl & 1) == 1) {
        D_d7ff = 4;
        D_96bd = 1;
    }
    if ((fl & 0x20) == 0x20)
        D_9749 = 1;
    if ((fl & 4) == 4 && real_combat == 0) {
        r = &F_20a5_300e[(unsigned char)D_c784[a]];
        if ((r->lock & 0x7f) == 0) {
            if (r->acid == 0)
                D_96d1 = 1;
            else {
                D_970d = 1;
                switch (r->acid) {
                case 0x50: D_970b = 0x5dc; break;
                case 0x14: D_970b = 0x5b2; break;
                case 0x32: D_970b = 0x5c6; break;
                }
            }
        } else {
            D_96e5 = 1;
            D_ca99 = 0;
            if (r->lock > 0x7f) {
                FIND_PICK(a);
                if (D_c7cd[a] != -1)
                    D_96f9 = 1;
            }
        }
    }
}

/* open the loot pile next to combatant i */
void far loot_pile_open(int i)
{
    int ofs;
    char cnt;
    int k;

    if (D_1b98 != 1)
        return;
    if (D_c81c[i] < 0) {
        cnt = -D_c81c[i];
        ofs = D_d846;
    } else {
        ofs = loot_piles[D_c81c[i]].ofs;
        cnt = loot_piles[D_c81c[i]].cnt;
    }
    if (cnt == 0)
        return;
    D_07d3++;
    sub_s014_19cd_3258(&cnt, &ofs, 1, 0);
    loot_screen(D_c654, &D_c67f, &F_2e38_0bb0, i);
    F_2e38_0bb0.a = 0;
    F_2e38_0bb0.b = 0;
    F_2e38_0bb0.c = 0;
    if (loot_pile_count != 0) {
        if (D_c67f != 0) {
            CURSOR_BUSY();
            _fmemcpy(loot_list, D_c654, D_c67f * 6);
            CURSOR_IDLE();
            ofs = sub_s014_19cd_3204(loot_list, D_c67f);
        }
        for (k = 0; k < (unsigned char)loot_pile_count; k++)
            if (loot_piles[k].ofs == -1) {
                loot_piles[k].cnt = D_c67f;
                loot_piles[k].ofs = ofs;
            }
    }
    floor_view_draw_vec(D_17b0);
    combat_draw();
    D_07d3--;
}

/* loot screen: free the combat buffers, run the s040 screen, restore everything */
void far loot_screen(struct loot far *list, unsigned *cnt, void far *x, int who)
{
    char kinds[14];
    struct b13 seq;
    int save65f, save67e;
    int reload;
    char s9429, s9433, s943d, s9893;
    unsigned seg;
    struct loot far *tmp;
    int k;

    D_07d3++;
    reload = 0;
    seq = *(struct b13 *)cbt_anim_len;
    for (k = 0; k < 13; k++)
        kinds[k] = combatants[k].f1b;
    strcpy(msg_buf, "Seq dealloc");
    s9429 = D_9429;
    s9433 = D_9433;
    s943d = D_943d;
    s9893 = D_9893;
    D_9429 = 2;
    D_9433 = 2;
    D_943d = 2;
    D_9893 = 2;
    save65f = enemy_anim_slot;
    save67e = D_d67e;
    enemy_anim_free5_vec();
    if (D_d6ee != 0) {
        _free_buffer(D_d6ee, "VarFrn");
        reload = 1;
    }
    if (D_d6f2 != 0)
        _free_buffer(D_d6f2, "Frn");
    if (D_d6f9 != 0)
        _free_buffer(D_d6f9, "FLR");
    if (D_d729 != 0)
        _free_buffer(D_d729, "FFX");
    if (D_d70a != 0)
        _free_buffer(D_d70a, "NWL");
    if (D_d73c != 0)
        _free_buffer(D_d73c, "NFX");
    if (D_d726 != 0)
        _free_buffer(D_d726, "WWL");
    if (D_d800 != 0)
        _free_buffer(D_d800, "WFX");
    D_07d8 = 0;
    if (D_9069 == 0) {
        D_ca55 = _alloc_buffer(0x300, 0, "Pal Buf");
        D_ca53 = 0;
        palette_save(0, D_ca55);
    }
    seg = _alloc_buffer(6000, 0, "loot");
    tmp = (struct loot far *)((unsigned long)seg << 16);
    CURSOR_BUSY();
    _fmemcpy(tmp, list, *(unsigned char *)cnt * 6);
    CURSOR_IDLE();
    if (D_9c4c == 1) {
        mouse_hide();
        mouse_set_cursor(0, 0, cursor_default);
        cursor_current = cursor_default;
    }
    shields_load();
    sub_root_047c_0568();
    party_panel_rows(0x100);
    D_9920 = 2;
    D_9934 = 2;
    loot_distribute_vec(0, seg, (unsigned char *)cnt, (unsigned)(unsigned long)x, (unsigned)((unsigned long)x >> 16), who);
    D_9934 = 1;
    sub_root_047c_1320(6, 0);
    D_c62a = 0;
    if (D_9069 == 0) {
        palette_restore(D_ca53, D_ca55);
        _free_buffer(D_ca55, "Pal Buf");
    }
    CURSOR_BUSY();
    _fmemcpy(list, tmp, *(unsigned char *)cnt * 6);
    CURSOR_IDLE();
    _free_buffer(seg, "loot");
    shields_free();
    battle_tiles_load_vec();
    if (reload == 1)
        furniture_load_vec();
    if (D_9c4c == 1) {
        mouse_hide();
        mouse_set_cursor(0, 0, ai_paused == 0 ? D_a693 : D_a691);
        cursor_current = cursor_default;
        sub_root_047c_04ea(0, 0);
        mouse_show();
    }
    D_07d8 = 1;
    for (k = 0; k < 6; k++)
        strcpy(D_11bc[k], "");
    for (k = 5; k < 13; k++)
        if (combatants[k].status != 0) {
            if (anim_buffers[combatants[k].kind] == 0)
                enemy_anim_alloc_vec(combatants[k].kind);
            cbt_set_anim(k, ((char *)&combatants[k])[0x18]);
        }
    D_9429 = s9429;
    D_9433 = s9433;
    D_943d = s943d;
    D_9893 = s9893;
    enemy_anim_slot = save65f;
    D_d67e = save67e;
    *(struct b13 *)cbt_anim_len = seq;
    for (k = 0; k < 13; k++)
        combatants[k].f1b = kinds[k];
    D_07d3--;
    sub_root_047c_056f();
}

/* append n loot records to the loot file; returns their offset */
int far sub_s014_19cd_3204(struct loot *buf, int n)
{
    FILE *f;
    int pos;

    f = fopen(D_1162, "ab");
    fseek(f, 0L, 2);
    pos = (int)ftell(f);
    fwrite(buf, 6, n, f);
    fclose(f);
    return pos;
}

/* take the piles at file offsets ofs[0..n-1] (cnt[] records each) out of the loot file
   into the current loot list (m entries) */
void far sub_s014_19cd_3258(char *cnt, int *ofs, int n, int m)
{
    int size, k, j, c;
    char far *p;
    int *q;

    D_c67f = m;
    CURSOR_BUSY();
    size = file_load_far(D_1162, load_buf);
    for (k = 0; k < n; k++) {
        if (ofs[k] == -1)
            continue;
        p = load_buf + ofs[k];
        c = (unsigned char)cnt[k];
        sub_s014_19cd_33c0((struct loot far *)p, c);
        sub_root_06a1_3108(p, p + c * 6, size - c * 6 - ofs[k]);
        for (j = k; j < n; j++)
            if (ofs[k] < ofs[j])
                ofs[j] -= c * 6;
        for (q = D_c7a4; q < D_c7a4 + 20; q++) {
            if (ofs[k] == *q)
                *q = -1;
            if (ofs[k] < *q)
                *q -= c * 6;
        }
        for (j = 0; j < (unsigned char)loot_pile_count; j++) {
            if (loot_piles[j].ofs == ofs[k])
                loot_piles[j].ofs = -1;
            if (ofs[k] < loot_piles[j].ofs)
                loot_piles[j].ofs -= c * 6;
        }
        size -= c * 6;
    }
    file_save(D_1162, load_buf, size);
    CURSOR_IDLE();
}

/* merge c loot records into the list (same item and quality: add the counts) */
void far sub_s014_19cd_33c0(struct loot far *p, int c)
{
    int k;
    struct loot far *q;

    CURSOR_BUSY();
    for (; c > 0; c--, p++) {
        for (k = 0, q = D_c654; k < D_c67f; k++, q++)
            if (p->id == q->id && q->a == p->a && p->b == q->b) {
                D_c654[k].cnt += p->cnt;
                break;
            }
        if (k >= D_c67f) {
            q = D_c654 + D_c67f++;
            *q = *p;
        }
    }
    CURSOR_IDLE();
}

/* after a fight: write the items of the fallen and the new piles to the loot file */
void far loot_after_fight(void)
{
    int ofs[180];
    char cnt[180];
    unsigned char idx[180];
    int n, a, b, k, o;
    struct loot far *d;
    struct loot far *s;

    n = 0;
    for (k = D_c83e; k < (unsigned char)loot_pile_count; k++)
        if (loot_piles[k].ofs != -2) {
            ofs[n] = loot_piles[k].ofs;
            idx[n] = k;
            cnt[n] = loot_piles[k].cnt;
            n++;
        }
    for (a = 0; a < n - 1; a++)
        for (b = a + 1; b < n; b++)
            if (ofs[a] == ofs[b]) {
                loot_piles[idx[b]].ofs = -1;
                ofs[b] = -1;
            }
    CURSOR_BUSY();
    for (k = 0, s = F_20a5_3954, d = D_c654; k < D_c783; k++, s++, d++)
        *d = *s;
    CURSOR_IDLE();
    D_c67f = D_c783;
    if (n != 0)
        sub_s014_19cd_3258(cnt, ofs, n, D_c67f);
    D_c81c[leader] = 0;
    if (n < 1 && D_c783 == 0)
        D_1b98 = 0;
    else {
        D_1b98 = 1;
        CURSOR_BUSY();
        _fmemcpy(loot_list, D_c654, D_c67f * 6);
        CURSOR_IDLE();
        o = sub_s014_19cd_3204(loot_list, D_c67f);
        for (k = 0; k < (unsigned char)loot_pile_count; k++)
            if (loot_piles[k].ofs == -1) {
                loot_piles[k].cnt = D_c67f;
                loot_piles[k].ofs = o;
                D_c81c[leader] = k;
                break;
            }
        if ((unsigned char)loot_pile_count == k) {
            D_c81c[leader] = -D_c783;
            D_d846 = o;
        }
    }
    D_c83e = loot_pile_count;
    D_c783 = 0;
}

/* load the shield pictures of the party ("pics\shielda.pic", letter = record +0x15) */
#pragma optimize("a", on)
void far shields_load(void)
{
    char name[32];
    int k;
    char *c;
    int *p;

    for (k = 0; k < 5; k++) D_e1cb[k] = shield_pics[k];
    sub_root_047c_0545();
    for (c = &combatants[0].status, p = shield_pics; p < shield_pics + 5; c += 0x80, p++) {
        *p = 0;
        if (*c != 0) {
            strcpy(name, "pics\\shielda.pic");
            name[11] = c[1];
            *p = pic_load(name);
        }
    }
    sub_root_047c_04f8();
}
#pragma optimize("a", off)

/* free the shield pictures */
#pragma optimize("a", on)
void far shields_free(void)
{
    int k;

    for (k = 0; k < 5; k++)
        if (shield_pics[k] != 0) {
            _free_buffer(shield_pics[k], "RSM");
            break;
        }
    for (k = 0; k < 5; k++) shield_pics[k] = D_e1cb[k];
}
#pragma optimize("a", off)
