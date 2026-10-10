/* place: s014 0e76:0000 */
/* orig: size=481a CONST=7d36+0012 _DATA=1776+014c */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* QA2 fix (2026-10-09): garbage loot after battle. (1) 'extern struct item6 far F_2e38_003e[][64], F_2e38_0dbe[][16];' - in MSC far binds to the declarator, so the enemies' item table 0dbe was a NEAR array: a dead enemy's items were copied from DS:0bde+i*0x60. Now far on each declarator. (2) same for F_2e38_0bfd (reinforcement count, written to DS:0bfd); now one far array F_2e38_0bfc[2] (one CONST slot as in the original). (3) D_c291 is a pointer (s002 sets it to &D_a895), it was declared as an array (copies went to DS:c291). Re-stamped (data unchanged). */
/* s014 0e76: TACTICAL COMBAT MAIN LOOP AND ACTION STATE MACHINE.
     0000  combat entry: load the level (s020/s024), place the party, open/close doors and
           triggers carried over (D_9080 flags), ready weapons, then loop:
             08fe end-of-battle test -> 0694 (save floor/activations, skill raises) and return
             real combat starts when D_d6f4 (alarm) fires: D_a6c1 = 1 ("battle mode")
             clock D_07d4 (rebased at 50000), 45ae (non-combat idle), 3166 input,
             4442 (enemies come into view -> stop / pause), vec_1c43 (s018 enemy turn),
             vec_1c89 (s015), 0f58 one tick
     0f58  one tick: AI (1db7_0008), locations (19cd_26a4), melee exchanges (2a4a), per
           combatant action state machine (116e), redraw (14f7_0008)
     116e  action of combatant i by D_ca64[i]/D_ca57[i]: walk along the path (1624), hit
           reactions (199c), missiles (1ba0), spells/items (1f00, 2170), dying (2324)
     2a4a  melee: attack rate (18bf_0002), random roll per tick, attack (18bf_0102)
     38b6  set animation (0 stand, 2 attack, 5 walk...), loading the sprite sets (3d2c)
     4188  start an effect / level event at a position
   Battle result D_a893: -1 running, 0 enemies beaten, 2 fled through an exit, 3 party lost,
   4/6 left at once, 5 everybody down, 7 timed exit, 8 left the map without a fight.
     3166  player input of a tick (cursor, rubber band 35fc, portraits, edge scrolling, keys)
     3c86/3cc4/3d0c  order queue D_d9e6[i][3]
     3d2c  load a sprite file into a slot (catalogue, header, frame tables)
     4442  activation records: hidden encounters start when seen / approached
     45ae  endurance recovery outside battle
   status: 39 of 39 functions written, 13 match (2a04, 38b6, 3c86, 4694, 4780, 45ae, 27ac, 27cc, 2170, 0694, 3cc4, 465e, 0620).  0620:
   #pragma optimize("eglc", off), locals in struct {t, k} (the ICE form is avoided through t).  465e: register
   parameter under #pragma optimize("egl", off).  3cc4: indexed copy
   loop D_d9e6[i][k] = D_d9e6[i][k + 1] + #pragma optimize("a", on).  0694 (m4): vec_1b2b takes a
   long size, D_07d8 is a char, flags via D_d68d[i][0] (strength-reduced), and the one '(f & 1) == 1'
   test is an unsigned bit-field compare (6.00A turns the plain form into 'dec al').  45ae: `st` in
   `union { char st; int d; } s` gets its own slot (m4).  1158 is inline asm in the
   original.  DATA MATCHES (e8, 2026-10-09; stamped --wip): _DATA 1776..18c1 is defined here (facing
   tables before 0620, 17e2..18c1 as one packed struct because 6.00A word-aligns each definition);
   the 9 CONST slots need the floor image as ONE far struct (struct floor F_20a5_000, slot 7d36) plus
   a second name F_20a5_0 for the door cell lists (slot 7d3a). */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    int scrx;                   /* +00 screen x */
    int scry;                   /* +02 screen y */
    int x;                      /* +04 map position */
    int y;                      /* +06 */
    int tx;                     /* +08 destination */
    int ty;                     /* +0a */
    char padc[2];
    int *info;                  /* +0e animation info */
    int f10;                    /* +10 action: 0 idle, 1 walking, bit 1 melee, 0x80 missile, 0x100 spell */
    char pad12[2];
    char status;                /* +14 0 none, 1 active, 2 unconscious, 3 dead, 5 left */
    char f15;
    char kind;                  /* +16 animation slot */
    char pad17;
    char pose;                  /* +18 */
    char f19;                   /* +19 facing (path) */
    char f1a;                   /* +1a facing (sprite) */
    char f1b;                   /* +1b animation frame */
    char pad1c[4];
    char def;                   /* +20 */
    char atk;                   /* +21 */
    char weapon;                /* +22 */
    char pad23[0x27];
    char target;                /* +4a */
    char pad4b[6];
    char f51;                   /* +51 default weapon */
    char pad52[0xb];
    char endur;                 /* +5d */
    char str;                   /* +5e */
    char pad5f[5];
    char maxend;                /* +64 */
    char pad65[0x19];
    char nitems;                /* +7e */
    char pad7f;
};
struct trig { int x; int y; unsigned char w; unsigned char h;
              void (far *fn)(int, int, int, int); int a; int b; };      /* 14 bytes */
struct mod { char pad0[0xc]; unsigned t0; unsigned t1; unsigned t2; };                 /* D_a532[13], 0x12 */
struct efa { int x; int y; unsigned char f4; unsigned char kind; char f6; char level; unsigned t; };
struct efb { int x; int y; unsigned char f4; char f5; char level; char f7; unsigned t; };
struct dmg { char endur; char str; };
struct wtab { char v[63]; };

extern struct cbt combatants[];
/* the floor image 20a5:0000 (level%d.flr, 0x33fc bytes) is ONE far variable in the original
   source (every field is reached through CONST slot 7d36); the door cell lists (near offsets
   into the same segment) are read through a second name for 20a5:0000 (slot 7d3a) */
struct floor {
    unsigned char cell[40][40][4];      /* 0000 map cells (byte 3 bit 7: hidden) */
    char p1900[0x1f42 - 0x1900];
    int x1f42, x1f44, x1f46, x1f48, x1f4a;  /* 1f42 exit codes */
    int x1f4c, x1f4e;                   /* 1f4c arrival effect position */
    char x1f50;
    char x1f51;                         /* 1f51 door opened by the arrival */
    char p1f52[0x2184 - 0x1f52];
    char ntrig;                         /* 2184 */
    char p2185;
    struct trig trig[55];               /* 2186 */
    char p2488[2];
    unsigned doors[16];                 /* 248a cells hidden behind each door (offsets) */
};
extern struct floor far F_20a5_000;
extern int far F_20a5_0;
#define F_20a5_0000 (F_20a5_000.cell)
#define F_20a5_1f42 (F_20a5_000.x1f42)
#define F_20a5_1f46 (F_20a5_000.x1f46)
#define F_20a5_1f48 (F_20a5_000.x1f48)
#define F_20a5_1f4a (F_20a5_000.x1f4a)
#define F_20a5_1f4c (F_20a5_000.x1f4c)
#define F_20a5_1f4e (F_20a5_000.x1f4e)
#define F_20a5_1f50 (F_20a5_000.x1f50)
#define F_20a5_1f51 (F_20a5_000.x1f51)
#define F_20a5_2184 (F_20a5_000.ntrig)
#define F_20a5_2186 (F_20a5_000.trig)
#define DOORLIST(d) ((unsigned far *)((char far *)&F_20a5_0 + F_20a5_000.doors[d]))
extern unsigned long far doors_opened[];  /* doors opened, per level */
struct atv { int f0; int f2; char pad[26]; };  /* activation record, 30 bytes: x, y, ... */
struct atvs { struct atv a[64]; };
extern struct atvs far activations;     /* activation records (count activation_count) */
extern struct wtab far F_20a5_75ac;     /* weapon -> skill */
extern unsigned char combat_flags, D_9081;    /* combat start flags */
extern char floor_shown, party_floor;             /* shown floor, party floor */
extern char cbt_floor[];                   /* [13] floor */
extern int D_d661, D_d662;
extern char floor_dirty;                     /* floor changed: save it */
extern int D_c925, D_c978;              /* trigger that started the fight */
extern char D_c989, D_ca1b;
extern unsigned char activation_count;
extern unsigned combat_clock;                 /* combat clock (ticks) */
extern unsigned D_c2bb;
extern char D_d842;
extern char enemy_class[];
extern char cbt_half_w[], cbt_half_h[];         /* [13] half width / height */
extern char D_1182[], D_118e[];         /* per animation set */
extern int battle_result;                      /* battle result */
extern char D_ca30[13];
extern int D_c897[8];
extern char selected_member;
extern unsigned char D_e09c, D_e09b, D_e09a;
extern char leader;                     /* leader */
extern char D_d6f1;
extern char move_mode;                     /* movement mode */
extern char D_07d8;
extern int D_d9e4;
extern int D_c62a;
extern unsigned D_da4f, D_da49, D_da47, D_da45, D_da4d, D_da4b, D_da51, D_da53;
extern char D_c8b7;
extern char real_combat;                     /* 1 = real combat (enemies engaged) */
extern char combat_alarm;                     /* alarm */
extern char D_9920, D_9721, D_9681, D_9695, D_9631; /* sound / music states */
extern char D_07aa, D_07bc;
extern char msg_buf[];
extern char D_0768[];                   /* "Party will travel individually" */
extern char cbt_moving[];
extern char cbt_anim[], cbt_action[];
extern char effect_cloud_count, D_a395;
extern struct efa effect_clouds[];
extern struct efb effect_areas[];
extern unsigned D_d9ca[13], D_d9b0[13], cbt_engage_time[13], D_d97c[13], cbt_giveup_timeout[13], D_d948[13];
extern unsigned D_dabd[13], D_daa3[13], D_da89[13], D_da6f[13], D_da55[13];
extern struct mod D_a532[];
extern char D_07d2;
extern char D_e079;
extern char D_d6f0;
extern char D_d4e5[];
extern char D_a27a[];
extern char msg_file;
extern char D_a896[];
extern unsigned char D_d68d[][2];       /* [5] skill use flags */
extern char D_c651;
extern int battle_args;
extern char D_17b0;
extern char demo_mode, D_9084;
extern char D_d6eb;
extern char D_a3d8[13];
extern unsigned char cbt_flags[][2];
extern int los_x0, los_y0, los_x1, los_y1;
extern char D_e0a2;
extern char cbt_was_hit[];
extern struct dmg cbt_queued_damage[];
extern int D_e092;
void *memset(void *, int, unsigned);

void far party_panel_frames(int);
void far party_panel_rows(int);
void far show_message(char *, int);
void far word_fill(int *, int, int);
void far sub_root_047c_03da(int);
void far sub_root_047c_040b(int);
void far sub_root_047c_056f(void);
void far mouse_show(void);
int far sub_root_047c_1320(int, int);
int far _random_n(int);
int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
void far *far _fmemcpy(void far *, const void far *, unsigned);
int abs(int);
void far level_load_vec(void);                /* s020 05e6: load the level */
void far marching_order_rebuild_vec(int);                 /* s016 0da6: marching order */
void far vec_1c6b(int, int);            /* s024 0000: relocate the floor */
void far level_switch_vec(int);                 /* s024 059a: switch floor */
void far combat_state_clear_vec(void);                /* s020 06b6: place the party */
void far vec_1c4d(int, int);            /* s023 0aea: remove furniture */
void far vec_1c57(int, int);            /* s023 0b6e: remove wall */
void far enemy_turn_vec(void);                /* s018 1f43_0256: enemy turn */
void far vec_1c39(int);                 /* s016 0a2c: select member */
void far floor_view_draw_vec(int);                 /* s017 0000: redraw */
void far vec_1c25(void);                /* s018 1f43_104c */
void far vec_1c9d(int);                 /* s018 1f43_0fc2: enemy joins the fight */
void far vec_1c93(void);                /* s025 0cfc: reinforcements */
void far debug_mem_track_vec(void);                /* s015 0000 */
void far swap_save2_vec(char *, void far *, long);   /* s027 0d06: write a file */
void far vec_1ccf(int);                 /* s019 0a7c: end of modifier 0 */
void far vec_1cc5(int);                 /* s019 0b00: end of modifier 1 */
void far vec_1cbb(int);                 /* s019 0bf8: end of modifier 2 */
void far enemy_anim_free5_vec(void);                /* s025 0be4: free enemy animations */
void far skill_try_improve_vec(int, int, int);       /* s021 0000: skill improvement */
void far vec_1833(int, int, int, int);  /* trigger handlers (root 09c0) */
void far vec_1829(int, int, int, int);
int far cbt_collisions(struct cbt *);
int far cbt_anim_set(int);
void far combat_draw(void);
void far weapon_ready(int, int, int);
int far line_blocked();             /* line of sight (no prototype: called with 2 or 3 args) */
void far order_menu_reset(void);
void far loot_after_fight(void);
void far loot_pile_open(int);
void far sub_s014_0e76_0620(void);
void far sub_s014_0e76_0694(void);
int far combat_over(void);
void far combat_mode_leave(void);
void far combat_recover(void);
void far combat_tick(void);
void far combat_input(void);
void far cbt_set_anim(int, int);
void far order_queue_clear(int);
void far effect_start(int, int, int);
int far activations_check(void);
void far endurance_recover(void);
void far sub_s014_0e76_465e(int);

#define REBASE 0x4098                   /* = -49000: clock wrap-around */

/* combat entry and main loop */
void far combat_main(void)
{
    int i, k;
    unsigned long mask;
    unsigned *p;
    unsigned v;

    D_e092 = 0;
    level_load_vec();
    marching_order_rebuild_vec(0);
    if ((combat_flags & 0x10) == 0x10)
        vec_1c6b(D_d661, D_d662);
    if ((D_9081 & 1) == 0)
        for (i = 0; i < 5; i++)
            cbt_floor[i] = party_floor = floor_shown;
    level_switch_vec(floor_shown);
    if ((combat_flags & 0x20) == 0x20) {
        /* arrived through a door: open it and uncover what is behind */
        if (F_20a5_1f51 >= 0) {
            mask = 1L << F_20a5_1f51;
            doors_opened[floor_shown] |= mask;
            for (k = 0; (v = DOORLIST(F_20a5_1f51)[k]) != 0xffff; k++)
                F_20a5_0000[v >> 8][v & 0xff][3] &= 0x7f;
            floor_dirty = 1;
        }
        effect_start(F_20a5_1f4c, F_20a5_1f4e, F_20a5_1f50);
    }
    if ((D_9081 & 1) == 0)
        combat_state_clear_vec();
    if ((combat_flags & 4) == 4)
        /* the fight was started by a furniture / wall trigger: apply it */
        for (k = 0; k < F_20a5_2184; k++)
            if (F_20a5_2186[k].x == D_c925 && F_20a5_2186[k].y == D_c978
             && F_20a5_2186[k].w == D_c989 && F_20a5_2186[k].h == D_ca1b) {
                if (F_20a5_2186[k].fn == vec_1833)
                    vec_1c4d((char)F_20a5_2186[k].a, k);
                else if (F_20a5_2186[k].fn == vec_1829)
                    vec_1c57((char)F_20a5_2186[k].a, k);
                break;
            }
    D_c925 = -1;
    if ((combat_flags & 8) == 8)
        for (k = 0; k < activation_count; k++)
            if (activations.a[k].f0 < 0) {
                activations.a[k].f0 = 0 - activations.a[k].f0;
                break;
            }
    sub_s014_0e76_0620();
    if ((combat_flags & 0x40) == 0x40) {
        D_c2bb = combat_clock + 600;
        D_d842 = 1;
    }
    activations_check();
    order_menu_reset();
    for (i = 0; i < 13; i++) {
        order_queue_clear(i);
        if (combatants[i].status != 0 && (i < 5 || (i > 4 && enemy_class[i] == 0)))
            weapon_ready(i, combatants[i].f51, 0);
    }
    for (i = 0; i < 5; i++) {
        cbt_half_w[i] = D_1182[i];
        cbt_half_h[i] = D_118e[i];
        if (combatants[i].status != 0)
            cbt_set_anim(i, 5);
    }
    enemy_turn_vec();
    battle_result = -1;
    memset(D_ca30, 0xff, 13);
    word_fill(D_c897, -9999, 8);
    selected_member = -1;
    D_e09c = 0;
    D_e09b = 4;
    D_e09a = 0x10;
    vec_1c39(leader);
    D_d6f1 = 1;
    move_mode = 0;
    sub_root_047c_03da(1);
    sub_root_047c_03da(-2);
    sub_root_047c_03da(4);
    D_07d8 = 1;
    sub_root_047c_056f();
    floor_view_draw_vec(-1);
    combat_draw();
    mouse_show();
    D_d9e4 = combat_clock + 8;
    sub_root_047c_1320(6, 0);
    D_c62a = 0;
    battle_result = -1;
    D_da4f = combat_clock + 120;
    if (((combat_flags & 0x10) == 0x10 || (D_9081 & 1) == 1) && (combat_flags & 1) == 1 && D_c8b7 == 0)
        show_message("Battlefield save rules are in effect", 150);
    for (;;) {
        D_9920 = (combat_flags & 1) == 1 && D_c8b7 == 0 && real_combat == 0 ? 1 : 2;
        if (combat_over() == 1) {
            sub_s014_0e76_0694();
            return;
        }
        if (real_combat == 1 && D_da49 < combat_clock) {
            vec_1c25();
            D_da49 = combat_clock + 300;
        }
        if (combat_alarm == 1 && real_combat == 0 && D_da47 < combat_clock) {
            /* the alarm went off: real combat */
            combat_alarm = 'c';
            real_combat = 1;
            D_9721 = 1;
            D_da49 = combat_clock + 300;
            for (i = 0; i < 13; i++)
                vec_1c9d(i);
        }
        if (real_combat == 1 && move_mode != 0) {
            /* no formations in combat */
            move_mode = 0;
            strcpy(msg_buf, D_0768);
            D_07aa = 2;
            D_07bc = 2;
            for (i = 0; i < 5; i++) {
                combatants[i].target = -1;
                cbt_moving[i] = 0;
                combatants[i].f10 = 0;
                cbt_anim[i] = 0;
            }
            show_message(msg_buf, 60);
            D_9681 = 2;
            D_9695 = 2;
        }
        if (combat_clock > 50000) {
            combat_clock += REBASE;
            D_d9e4 += REBASE;
            D_da53 += REBASE;
            D_da51 += REBASE;
            D_da4f += REBASE;
            D_c2bb += REBASE;
            D_da4d += REBASE;
            D_da4b += REBASE;
            D_da45 += REBASE;
            D_da49 += REBASE;
            D_da47 += REBASE;
            for (k = 0; k < effect_cloud_count; k++)
                effect_clouds[k].t += REBASE;
            for (k = 0; k < D_a395; k++)
                effect_areas[k].t += REBASE;
            for (i = 0; i < 13; i++) {
                D_d9ca[i] += REBASE;
                D_d9b0[i] += REBASE;
                cbt_engage_time[i] += REBASE;
                D_d97c[i] += REBASE;
                cbt_giveup_timeout[i] += REBASE;
                D_d948[i] += REBASE;
                D_dabd[i] += REBASE;
                D_daa3[i] += REBASE;
                D_da89[i] += REBASE;
                D_da6f[i] += REBASE;
                D_da55[i] += REBASE;
                if (D_a532[i].t0 != -536)
                    D_a532[i].t0 += REBASE;
                if (D_a532[i].t1 != -536)
                    D_a532[i].t1 += REBASE;
                if (D_a532[i].t2 != -536)
                    D_a532[i].t2 += REBASE;
            }
        }
        D_07d2 = 0;
        D_e079 = 0;
        if (real_combat == 0)
            endurance_recover();
        combat_input();
        if (D_e079 == 0)
            for (i = 0; i < 13; i++)
                if (combatants[i].status == 1 && combatants[i].f1b != 0 && cbt_moving[i] == 0
                 && combatants[i].pose == 5)
                    combatants[i].f1b = 0;
        if (D_da4f < combat_clock) {
            if (activations_check() == 1)
                sub_s014_0e76_465e(0xe);
            enemy_turn_vec();
        }
        if (D_d6f0 == 0 && D_c2bb < combat_clock)
            vec_1c93();
        debug_mem_track_vec();
        combat_tick();
        for (i = 0; i < 13; i++)
            if (D_d4e5[i] != 0 && D_da6f[i] < combat_clock)
                D_d4e5[i] = 0;
    }
}

/* re-activate the first disabled activation record (copy it to the end) */
/* module data (DGROUP 179b..17bd) */
char D_179c[10] = {0, 1, 2, 3, 4, 5, 6, 7, 1, 5};      /* path facing -> sprite facing */
char D_17a6[10] = {4, 5, 6, 7, 0, 1, 2, 3, 5, 1};      /* push direction -> facing */
char D_17b0 = 0;
char D_17b1 = 0;
char D_17b2[12] = {2, 1, 0, 2, 3, 4, 6, 5, 4, 6, 7, 0};

#pragma optimize("eglc", off)
void far sub_s014_0e76_0620(void)
{
    struct { int t, k; } s;

    if ((combat_flags & 0x80) == 0x80)
        for (s.k = 0; s.k < activation_count; s.k++)
            if (activations.a[s.k].f0 < 0) {
                activations.a[activation_count] = activations.a[s.k];
                activation_count++;
                s.t = activations.a[s.k].f0;
                activations.a[s.k].f0 = -s.t;
                return;
            }
}
#pragma optimize("eglc", on)

/* after the battle: save the changed floor and activations, end temporary modifiers,
   skill improvement for the weapons/skills used (D_d68d flags) */
struct bit0 { unsigned b0:1; };

void far sub_s014_0e76_0694(void)
{
    int i;

    if (floor_dirty == 1) {
        sprintf(D_a27a, "level%d.flr", floor_shown);
        swap_save2_vec(D_a27a, (void far *)F_20a5_0000, 0x33fcL);
        sprintf(D_a27a, "level%d.atv", floor_shown);
        msg_file = activation_count;
        _fmemcpy(D_a896, &activations, activation_count * 30);
        swap_save2_vec(D_a27a, &msg_file, (unsigned long)activation_count * 30 + 1);
    }
    for (i = 0; i < 5; i++) {
        if (D_a532[i].t0 != -536)
            vec_1ccf(i);
        if (D_a532[i].t1 != -536)
            vec_1cc5(i);
        if (D_a532[i].t2 != -536)
            vec_1cbb(i);
    }
    sub_root_047c_040b(1);
    sub_root_047c_040b(2);
    sub_root_047c_040b(3);
    sub_root_047c_040b(4);
    D_07d8 = 0;
    floor_dirty = 0;
    enemy_anim_free5_vec();
    if (battle_result == 6)
        return;
    for (i = 0; i < 5; i++) {
        if (combatants[i].status != 1 && combatants[i].status != 2)
            continue;
        if ((D_d68d[i][0] & 8) == 8) {
            skill_try_improve_vec(i, F_20a5_75ac.v[combatants[i].f51], 1);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xf6;
        } else if (((struct bit0 *)D_d68d[i])->b0 == 1) {
            skill_try_improve_vec(i, F_20a5_75ac.v[combatants[i].f51], 0);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xfe;
        }
        if ((D_d68d[i][0] & 0x10) == 0x10) {
            skill_try_improve_vec(i, F_20a5_75ac.v[combatants[i].weapon], 1);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xed;
        } else if ((D_d68d[i][0] & 2) == 2) {
            skill_try_improve_vec(i, F_20a5_75ac.v[combatants[i].weapon], 0);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xfd;
        }
        if ((D_d68d[i][0] & 4) == 4) {
            skill_try_improve_vec(i, 4, 1);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xfb;
        }
        if ((D_d68d[i][0] & 0x20) == 0x20) {
            skill_try_improve_vec(i, 0xe, 1);
            party_panel_rows(i);
            D_d68d[i][0] &= 0x9f;
        } else if ((D_d68d[i][0] & 0x40) == 0x40) {
            skill_try_improve_vec(i, 0xe, 0);
            party_panel_rows(i);
            D_d68d[i][0] &= 0xbf;
        }
    }
}

/* end-of-battle test; 1 = leave the combat screen */
int far combat_over(void)
{
    int i, j, n, nolive, noenemy, nostanding;
    unsigned now;

    if (battle_result == 4 || battle_result == 6)
        return 1;
    if (battle_result == 7 && D_da4d < combat_clock)
        return 1;
    /* the floor where the enemies are */
    D_c651 = floor_shown;
    for (i = 5; i < 13; i++)
        if (combatants[i].status == 1) {
            D_c651 = cbt_floor[i];
            break;
        }
    /* every member that is still here has left through an exit (floor -99)? */
    nolive = 1;
    n = 0;
    for (i = 0; i < 5; i++)
        if (cbt_floor[i] == -99)
            n++;
        else if (combatants[i].status == 1 && cbt_floor[i] == D_c651)
            nolive = 0;
    if (nolive && n != 0) {
        if (real_combat == 1) {
            battle_result = 2;                 /* fled */
            battle_args = F_20a5_1f46;
        } else {
            battle_result = 8;
            battle_args = F_20a5_1f4a;
        }
        for (i = 0; i < 5; i++)
            if (cbt_floor[i] != -99 && (combatants[i].status == 1 || combatants[i].status == 2))
                combatants[i].status = 5;
        return 1;
    }
    if (real_combat == 1) {
        /* nobody standing on the enemies' floor but somebody elsewhere: go there */
        nolive = 1;
        n = 0;
        for (i = 0; i < 5; i++)
            if (cbt_floor[i] == D_c651 && combatants[i].status == 1)
                nolive = 0;
            else if (cbt_floor[i] != D_c651 && combatants[i].status == 1)
                n++;
        if (nolive && n != 0) {
            for (i = 0; i < 5; i++)
                if (cbt_floor[i] == D_c651 && (combatants[i].status == 1 || combatants[i].status == 2))
                    combatants[i].status = 5;
            loot_after_fight();
            for (i = 0; i <= 4; i++)
                if (combatants[i].status == 1)
                    break;
            if (i <= 4) {
                party_floor = cbt_floor[i];
                level_switch_vec(party_floor);
                for (j = 5; j < 13; j++)
                    combatants[j].status = 0;
                j = combatants[leader].status;
                if (j == 0 || j == 3 || j == 5)
                    for (i = 0; i < 5; i++)
                        if (combatants[i].status == 1 || combatants[i].status == 2) {
                            leader = D_17b0 = i;
                            break;
                        }
                combat_mode_leave();
                combat_recover();
                floor_view_draw_vec(i);
                combat_draw();
                return 0;
            }
        }
        nolive = 1;                     /* no member conscious on the enemies' floor */
        noenemy = 1;
        nostanding = 1;                 /* no member alive at all there */
        for (i = 0; i < 5; i++)
            if (combatants[i].status != 0 && cbt_floor[i] != -99 && cbt_floor[i] == D_c651) {
                if (combatants[i].status == 1) {
                    nolive = 0;
                    nostanding = 0;
                } else if (combatants[i].status == 2)
                    nostanding = 0;
            }
        for (i = 5; i < 13; i++)
            if (combatants[i].status != 0 && combatants[i].status == 1) {
                noenemy = 0;
                break;
            }
        if (nolive || noenemy || nostanding) {
            combat_mode_leave();
            if (noenemy) {
                battle_result = 0;
                D_da45 = combat_clock + 60;
            }
            if (nostanding) {
                for (i = 0; i < 5; i++)
                    if (combatants[i].status == 1 && cbt_floor[i] == -99)
                        battle_result = 3;
                if (battle_result != 3) {
                    battle_result = 5;
                    D_da45 = combat_clock + 240;
                }
            } else if (nolive) {
                battle_result = 3;
                D_da45 = combat_clock + 240;
            }
        }
    }
    now = combat_clock;
    if (battle_result == 2)
        battle_args = F_20a5_1f46;
    else if (battle_result == 3)
        battle_args = F_20a5_1f48;
    if ((battle_result == 3 || battle_result == 5) && combat_clock > D_da45)
        return 1;
    if (battle_result == 0 && D_da45 < combat_clock) {
        /* victory: clean up, revive the unconscious, loot */
        for (i = 5; i < 13; i++)
            combatants[i].status = 0;
        enemy_anim_free5_vec();
        if (demo_mode != 0)
            D_9084 = 1;
        for (i = 0; i < 5; i++)
            if (combatants[i].status != 0) {
                cbt_moving[i] = 0;
                combatants[i].f10 = 0;
                cbt_action[i] = 0;
                cbt_set_anim(i, 5);
            }
        if (demo_mode == 'I') {
            floor_view_draw_vec(-1);
            combat_draw();
        }
        j = combatants[leader].status;
        if (j == 0 || j == 3 || j == 5)
            for (i = 0; i < 5; i++)
                if (combatants[i].status == 1 || combatants[i].status == 2) {
                    leader = D_17b0 = i;
                    break;
                }
        loot_after_fight();
        combat_recover();
        loot_pile_open(leader);
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        battle_args = F_20a5_1f42;
        if (D_9084 == 1)
            return 1;
        if ((combat_flags & 1) == 1 && D_9084 == 0)
            battle_result = -1;
    }
    if (battle_result == -1 || (combat_flags & 1) != 0 || now <= D_da45)
        return 0;
    return 1;
}

/* leave battle mode */
#pragma optimize("a", on)
void far combat_mode_leave(void)
{
    int i, s;

    D_d6eb = 0;
    real_combat = 0;
    combat_alarm = 0;
    sub_root_047c_040b(3);
    D_9631 = 2;
    D_9721 = 2;
    for (i = 0; i < 13; i++) {
        D_a3d8[i] = 0;
        ((unsigned *)cbt_flags)[i] &= 0x9fff;
        s = cbt_anim_set(i);
        cbt_half_w[i] = D_1182[s];
        cbt_half_h[i] = D_118e[s];
    }
}
#pragma optimize("a", off)

/* after a fight: clear damage, restore 3/4 of the endurance, wake the unconscious and put
   them on a free spot near where they fell (or near a standing member) */
void far combat_recover(void)
{
    int i, r, tries, c, first, any, cx, cy;
    char t;

    any = 0;
    for (i = 0; i < 5; i++) {
        D_da55[i] = combat_clock;
        if (combatants[i].status == 1 || combatants[i].status == 2) {
            D_d4e5[i] = 0;
            cbt_was_hit[i] = 0;
            cbt_queued_damage[i].endur = 0;
            cbt_queued_damage[i].str = 0;
            t = combatants[i].maxend * 3 / 4;
            if (combatants[i].endur < t)
                combatants[i].endur = t;
        }
        if (combatants[i].status != 2)
            continue;
        combatants[i].status = 1;
        first = 1;
        los_x0 = cx = combatants[i].x;
        los_y0 = cy = combatants[i].y;
        tries = 0;
        D_e0a2 = 1;
        r = 16;
        c = i;
        for (;;) {
            combatants[i].x = _random_n(r) + los_x0;
            combatants[i].y = _random_n(r) + los_y0;
            if (cbt_collisions(&combatants[i]) == 0) {
                los_x1 = combatants[i].x;
                los_y1 = combatants[i].y;
                if (any || line_blocked(-1, 0, 0) == 0)
                    break;
            }
            if (++tries > 75) {
                tries = 0;
                r += 8;
                if (r > 60) {
                    if (first) {
                        c = 0;
                        first = 0;
                    }
                    if (c < 5) {
                        if (combatants[c].status == 1) {
                            los_x0 = combatants[c].x;
                            los_y0 = combatants[c].y;
                            c++;
                            r = 16;
                        } else
                            c++;
                    } else {
                        r = 30;
                        any = 1;
                    }
                }
            }
        }
        D_e0a2 = 0;
        party_panel_rows(i);
    }
}

extern char D_ca29;
extern unsigned char D_07d1;
extern char D_d610;
extern int D_e088, D_07d9;
extern char ai_paused;                     /* 1 = paused (orders being given) */
extern char D_11b1;                     /* follow the active member */
extern char D_d6ed;
extern char hotspot_count;
extern char hotspot_cbt[];
extern int D_d889[], D_d86f[];
extern int D_e09f, D_e09d;
extern char D_d505[][8], D_d5d3[];
extern char D_17b1;
extern char D_07d3;
extern unsigned char D_11de[8];         /* rotating colour pattern */
extern char cbt_special[];                   /* [13] special state (2 = stairs, 10 = ...) */
extern unsigned char D_c77e[];
extern char D_ca9a[];                   /* [13] push direction */
extern char D_caa7[];                   /* [13] pusher */
extern char D_17a6[];                   /* push direction -> facing */
extern struct cbt D_d73e;               /* scratch record */
extern int D_84da;
extern char D_d41b[], D_d1ce[];
extern char D_c681[][10], D_c6f4[][10]; /* step dx/dy per sprite set */
extern char D_d6fb[];
extern char cbt_damage_shown[];                   /* damage taken since the last display */
extern char D_e07b;
void far ai_tick(void);
void far party_locations(void);
void far order_mode_enter(void);
void far missiles_advance(void);
int far path_find(int);
void far path_give(int);
int far get_key(void);
void far quit_to_dos(void);
void far take_exit_vec(int, int);            /* s023 1618: take the stairs */
void far sub_s014_0e76_1158(void);
void far combat_actions(void);
void near cbt_step(int, int);
void near sub_s014_0e76_199c(int, int, int *);
void near cbt_missile_action(int, int);
void near sub_s014_0e76_1f00(int, int);
void near cbt_location_action(int);
void near cbt_dying(int, int);
void far cbt_cancel_action(int);
void far melee_exchanges(void);
void far order_queue(int, int);

/* one combat tick (every 8 clock ticks, or when paused every 8 calls) */
void far combat_tick(void)
{
    int i, j, k, c;

    if (D_ca29 == 1)
        return;
    if (combat_clock < (unsigned)D_d9e4 && (ai_paused != 1 || D_07d1 < 8))
        return;
    D_07d1 = 0;
    D_e092++;
    /* is the whole party on one floor (then the group movement modes are allowed) */
    D_d610 = real_combat != 1;
    for (i = 0; i < 5; i++)
        if (combatants[i].status == 1)
            break;
    if (i < 5)
        for (j = i + 1; j < 5; j++)
            if (combatants[j].status == 1 && cbt_floor[j] != cbt_floor[i]) {
                D_d610 = 0;
                break;
            }
    if (D_d610 == 1)
        D_9681 = D_9695 = 1;
    else if (move_mode == 1) {
        D_9681 = 1;
        D_9695 = 2;
    } else if (move_mode == 2) {
        D_9681 = 2;
        D_9695 = 1;
    } else
        D_9681 = D_9695 = 2;
    D_e088 = D_07d9;
    D_07d9 = 0;
    if (ai_paused == 0) {
        ai_tick();           /* AI */
        party_locations();           /* locations and triggers */
        melee_exchanges();           /* melee */
        sub_s014_0e76_1158();           /* colour cycling */
        combat_actions();           /* actions */
    }
    D_d9e4 = combat_clock + 8;
    if (D_ca29 == 1)
        return;
    if (D_11b1 == 1) {
        c = D_17b0;
        if (combatants[c].scry < 0x14 || combatants[c].scry > 0xa0
         || combatants[c].scrx < 0x14 || combatants[c].scrx > 0xdc) {
            floor_view_draw_vec(c);                /* scroll to keep the active member in view */
            D_d9e4 = combat_clock + 8;
        }
    }
    combat_draw();
    if (D_d6f1 == 1) {
        for (k = hotspot_count - 1; k >= 0; k--)
            if (hotspot_cbt[k] == D_d6ed)
                break;
        if (k >= 0) {
            selected_member = D_d6ed;
            D_e09f = D_d505[cbt_anim_set(D_d6ed)][combatants[D_d6ed].f1a] + D_d889[k] - 4;
            D_e09d = (D_d5d3[cbt_anim_set(selected_member)] >> 1) + D_d86f[k] - 13;
            order_mode_enter();
            D_d6f1 = 0;
        }
    }
    if (D_17b1 != 0) {
        D_07d3++;
        while (get_key() == 0x1000)
            quit_to_dos();
        D_07d3--;
    }
}

/* rotate the colour pattern (original: hand-written `ror`) */
void far sub_s014_0e76_1158(void)
{
    int k;

    for (k = 0; k < 8; k++)
        D_11de[k] = D_11de[k] >> 1 | D_11de[k] << 7;
}

/* action state machine of every combatant; a second pass handles those that were pushed
   by somebody processed later in the first pass */
void far combat_actions(void)
{
    int again[13];
    int pass, i, j, set, lo, hi, p;
    struct cbt *c;

    memset(again, 0, sizeof again);
    for (pass = 0; pass < 2; pass++) {
        for (i = 0; i < 13; i++) {
            c = &combatants[i];
            if (!(pass == 0 || (pass == 1 && again[i] == 1)) || c->status == 0)
                continue;
            set = cbt_anim_set(i);
            if (cbt_special[i] == 2 && floor_shown != party_floor) {
                take_exit_vec(D_c77e[i], i);  /* follow the party up/down the stairs */
                cbt_cancel_action(i);
                continue;
            }
            if (cbt_action[i] == 13 || cbt_special[i] == 10) {
                cbt_cancel_action(i);
                party_panel_rows(i);
                cbt_set_anim(i, 5);
                cbt_moving[i] = 0;
                cbt_action[i] = 0;
            }
            if ((c->f10 & 1) == 1)
                cbt_step(i, set);             /* walk */
            if ((c->f10 & 2) == 2 && cbt_action[i] == 1 && D_d9b0[i] <= combat_clock)
                sub_s014_0e76_199c(i, set, again);      /* melee animation step */
            if ((c->f10 & 0x80) == 0x80)
                cbt_missile_action(i, set);             /* missile weapon */
            if ((c->f10 & 0x100) == 0x100)
                sub_s014_0e76_1f00(i, set);             /* spell / potion */
            if (cbt_special[i] != 0)
                cbt_location_action(i);                  /* use an item / location */
            if (cbt_anim[i] == 3) {
                /* being pushed back one step by D_caa7[i] */
                cbt_set_anim(i, 3);
                c->f19 = D_ca9a[i];
                c->f1a = D_17a6[D_ca9a[i]];
                D_d73e = *c;
                D_84da = 0;
                c->x += D_c681[set][c->f19];
                c->y += D_c6f4[set][c->f19];
                D_e0a2 = 1;
                if (cbt_collisions(c) == 0) {
                    D_84da = 1;
                    p = D_caa7[i];
                    cbt_anim[p] = 4;
                    D_d41b[p]++;
                }
                D_e0a2 = 0;
                if (D_84da == 0)
                    *c = D_d73e;
                if (++c->f1b == 1) {
                    cbt_anim[i] = 0xe;
                    cbt_engage_time[i] += 60;
                    cbt_action[i] = 7;
                    p = D_caa7[i];
                    if (D_84da == 1 || cbt_anim[p] == 4) {
                        cbt_anim[p] = 5;          /* the pusher follows */
                        if (p < i)
                            again[p] = 1;
                    } else {
                        cbt_action[p] = 7;
                        cbt_anim[p] = 6;
                    }
                    if (i < 5) {
                        lo = 5;
                        hi = 13;
                    } else {
                        lo = 0;
                        hi = 5;
                    }
                    /* the attackers of i follow it */
                    for (j = lo; j < hi; j++)
                        if (combatants[j].target == i && c->target != j && cbt_moving[j] == 0
                         && (combatants[j].f10 & 2) == 2 && path_find(j) == 1) {
                            path_give(j);
                            cbt_moving[j] = 1;
                            D_d6fb[j] = 0;
                            order_queue_clear(j);
                            order_queue(j, combatants[j].f10);
                            combatants[j].f10 = 1;
                            combatants[j].pose = -1;
                            cbt_set_anim(j, 5);
                        }
                }
                c->f19 = c->f1a;
            }
            if (cbt_anim[i] == 5) {
                /* following up a push */
                cbt_set_anim(i, 5);
                D_d73e = *c;
                D_84da = 0;
                c->x += D_c681[set][c->f19];
                c->y += D_c6f4[set][c->f19];
                D_d1ce[i]++;
                D_e0a2 = 1;
                if (cbt_collisions(c) == 0) {
                    D_84da = 1;
                    D_d41b[i]--;
                    c->f1b++;
                    D_d97c[i] = combat_clock + 30;
                }
                D_e0a2 = 0;
                if (D_84da == 0)
                    *c = D_d73e;
                if (D_d1ce[i] == 1) {
                    cbt_anim[i] = 6;
                    cbt_action[i] = 7;
                }
            }
            if (c->status == 2 || c->status == 3)
                cbt_dying(i, set);             /* falling / dying */
        }
    }
    missiles_advance();                               /* missiles */
    for (i = 0; i < 13; i++)
        if (combatants[i].status == 1 && cbt_was_hit[i] == 1) {
            /* show the damage taken this tick for 150 ticks */
            D_d4e5[i] = cbt_damage_shown[i] > 42 ? 42 : cbt_damage_shown[i];
            D_da6f[i] = combat_clock + 150;
            cbt_damage_shown[i] = 0;
            cbt_was_hit[i] = 0;
        }
    if (D_e07b == 1) {
        floor_view_draw_vec(-1);
        D_e07b = 0;
    }
}

extern char D_ca30[];                   /* [13] next path step (dir | facing << 4), -1 none */
extern char marching_order[];                   /* marching order */
extern char marching_len;
extern struct win { char pad[0xc]; int f0c; } *D_0190;
extern char D_ca1c[];                   /* [13] steps to wait */
extern unsigned char D_c928[][2];       /* [13] */
extern char cbt_anim_len[];                   /* [13] animation sequence length */
extern int D_d5f6[];                    /* [13] what blocked the last step */
extern int D_d708;                      /* last collision result */
extern char D_179c[];                   /* path facing -> sprite facing */
extern char far cbt_paths[13][250];   /* paths */
extern char D_c799[];                   /* per animation set: can knock back */
extern char D_c21f;
extern unsigned char D_c26e;
extern char *D_117e;
extern char D_c97a[], D_ca0c[];
struct mod2 { char pad[9]; char held; char pad2[8]; };
void far member_status_letter(int, struct win *);
int far sub_s014_12f7_147c(int);
void far damage_apply(int);
int far push_direction(int);
void far sub_s014_0e76_2fa2(int);
int far sub_s014_0e76_469e(void *, int);

#define REC(i) (*(struct cbt *)&combatants[i])

/* take one step along the path */
void near cbt_step(int i, int set)
{
    unsigned now;
    int k, a, b, best, n;
    unsigned char st;
    int save;
    struct cbt *c;

    if (D_ca30[i] == -1)
        return;
    now = combat_clock;
    c = &combatants[i];
    if (((unsigned char *)c)[0x1e] + now < D_d9ca[i])
        D_d9ca[i] = now;
    if (D_d9ca[i] > now)
        return;
    if (((struct mod2 *)D_a532)[i].held == 1)
        goto wait;
    if (move_mode == 2 && marching_order[0] == i) {
        /* single file: each member follows the one in front */
        for (k = 1; k < marching_len; k++) {
            a = marching_order[k - 1];
            b = marching_order[k];
            combatants[b].tx = combatants[a].x;
            combatants[b].ty = combatants[a].y;
            combatants[b].target = a;
            if (path_find(b) == 1) {
                path_give(b);
                D_d6fb[b] = 0;
                D_d97c[b] = _random_n(60) + combat_clock + 30;
                cbt_moving[b] = 1;
                combatants[b].f10 = 1;
                D_ca30[b] = cbt_paths[b][(unsigned char)D_d6fb[b]];
            }
        }
        if (real_combat == 0) {
            save = D_0190->f0c;
            for (k = 0; k < 5; k++)
                if (combatants[k].status == 1) {
                    D_0190->f0c = leader == k ? 0xe : 0xf;
                    member_status_letter(k, D_0190);
                }
            D_0190->f0c = save;
        }
    }
    if (D_ca1c[i] != 0) {
        D_ca1c[i]--;
        goto wait;
    }
    D_d9ca[i] = ((unsigned char *)c)[0x1e] + combat_clock;
    cbt_set_anim(i, 5);
    if (D_a395 > 0) {
        /* slowing areas: only move on the ticks the rotating pattern allows */
        best = 0;
        for (k = 0; k < D_a395; k++)
            if ((D_c928[i][0] & 2) == 0 && sub_s014_0e76_469e(&effect_areas[k], i) == 1
             && best < (unsigned char)effect_areas[k].f5)
                best = (unsigned char)effect_areas[k].f5;
        if (best != 0 && (D_11de[best] & 1) == 0)
            return;
    }
    st = D_ca30[i];
    c->f19 = st & 0xf;
    c->f1a = D_179c[((char)st >> 4) & 0xf];
    D_d73e = *c;
    D_84da = 0;
    c->x += D_c681[set][c->f19];
    c->y += D_c6f4[set][c->f19];
    if (cbt_collisions(c) == 0) {
        D_84da = 1;
        n = cbt_anim_len[i] - 1;
        c->f1b++;
        c->f1b = (c->f1b - 1) % n + 1;          /* cycle the walk frames */
        if (cbt_moving[i] == 1) {
            D_d6fb[i]++;
            D_d97c[i] = _random_n(90) + combat_clock + 30;
            D_d5f6[i] = 0;
        }
    } else if (cbt_moving[i] == 1) {
        if (sub_s014_12f7_147c(c->target) != 0) {
            /* bumped into the target: fight */
            *c = D_d73e;
            D_84da = 1;
            sub_s014_0e76_2fa2(i);
            cbt_engage_time[i] = combat_clock;
            cbt_action[i] = 7;
        } else
            D_d5f6[i] = D_d708 & 0x1fff;
    }
    if (D_84da == 0)
        *c = D_d73e;
    D_ca30[i] = -1;
    return;
wait:
    D_d97c[i] = _random_n(90) + combat_clock + 30;
}

/* melee animation step of i; on the impact frame resolve the hit (knockback for heavy
   hits between two sets that allow it) */
#pragma optimize("a", on)
void near sub_s014_0e76_199c(int i, int set, int *again)
{
    int t;

    D_d9b0[i] = ((unsigned char *)&combatants[i])[0x1f] + combat_clock;
    cbt_set_anim(i, 2);
    combatants[i].f1b++;
    if (i < 5)
        D_d68d[i][0] |= 1;              /* weapon skill used */
    if (combatants[i].f1b - cbt_anim_len[i] != -1)
        return;
    if (demo_mode != 0 && i == 5 && D_c26e % D_c21f == 0 && battle_result == -1) {
        /* tutorial opponent switches to its special attack */
        if (demo_mode == 'I')
            strcpy(D_117e, "P7");
        else
            strcpy(D_117e, "PC");
        combatants[5].f10 = 0x100;
        D_ca0c[5] = 8;
        D_c97a[5] = 0x32;
    }
    t = combatants[i].target;
    if (cbt_anim[i] != 2 && cbt_anim[i] != 1) {
        sub_s014_0e76_465e(0x11);       /* miss */
        cbt_action[i] = 7;
        return;
    }
    if (cbt_anim[i] == 2 && combatants[t].target == i && D_c799[set] == 1
     && D_c799[cbt_anim_set(t)] == 1) {
        /* knock the target back */
        cbt_anim[t] = 3;
        D_caa7[t] = i;
        cbt_queued_damage[i].endur >>= 1;
        combatants[t].f1b = -1;
        D_ca9a[t] = push_direction(i);
        cbt_action[i] = 3;
        cbt_action[t] = 3;
        D_d41b[i] = 0;
        sub_s014_0e76_465e(0x10);
        if (t < i)
            again[t] = 1;
    } else
        cbt_action[i] = 7;
    if (i < 5)
        D_d68d[i][0] |= 8;
    if (cbt_queued_damage[t].endur > 0)
        cbt_was_hit[t] = 1;
    damage_apply(t);              /* apply the damage */
    if (combatants[t].status == 2 || combatants[t].status == 3) {
        cbt_anim[i] = 0;
        cbt_anim[t] = 0;
        if (demo_mode == 0 || i != 5)
            cbt_action[i] = 0;
        else
            cbt_action[i] = 7;
        party_panel_rows(i);
        party_panel_rows(t);
        if (t < i)
            again[t] = 1;
    }
}
#pragma optimize("", on)

extern char D_e05f[], D_e052[], D_e06c[];  /* [13] animation sub-state flags */
extern char cbt_anim_seq[][10];               /* [13] animation sequence */
extern char D_a3e7[][30];               /* sequences per animation set */
extern unsigned char D_c8a7[], D_c84a[];
extern struct wtab far F_20a5_7822;     /* weapon -> ammo */
extern char D_9f1f;
extern unsigned char D_c64c[], D_c7cd[];
extern char D_c784[], D_c8b2[];
extern char D_ca99;
extern char D_c83f[], D_c803[];         /* per animation set: corpse drawn / drops items */
extern char D_c1f3[];                   /* [13] killing it ends the battle */
struct pile { int ofs; char cnt; char level; int x; int y; int vis; char dx; char dy; };
extern struct pile far loot_piles[];
extern unsigned char loot_pile_count;
extern struct loot { int id; char a; char b; char cnt; char pad; } far *D_c654;
extern unsigned D_c67f;
extern struct loot *loot_list;   /* POINTER to the near copy (set by s002: &msg_file), not an array */
extern struct item6 { char b[6]; } far inventories[][64], far enemy_items[][16];   /* 'far' on EACH declarator: without it 0dbe was a near (DS) array -> garbage loot */
extern char D_d6ec;
extern char far F_2e38_0bfc[2];   /* reinforcements: [0] wave size, [1] remaining (2e38:0bfd); ONE far name = one CONST slot like the original (was 'char far F_2e38_0bfc, F_2e38_0bfd;': 0bfd lost its far and read DS:0bfd) */
extern int ems_frame_handle;
extern char D_d3fe[];
extern int far F_20a5_5890[0x8c];
extern struct dooreg { char pad[7]; char lock; char pad8[6]; } far F_20a5_1f54[];
extern struct regn { char pad[8]; char lock; char acid; char pad2[0x22]; } far F_20a5_300e[];
void far missile_launch_vec(int, int, int, int, int);   /* s018 1ef8_0000: launch a missile */
void far vec_1ce3(int, int);            /* s018 1ef8_03ae: drop / recover an item */
void far vec_1cf7(int, int, int);       /* s019 0dfe: spell effect */
void far door_toggle_vec(int, int);            /* s023 00b8: open / close a door */
int far lock_pick_vec(char far *);           /* s023 12c0: pick a lock */
void far chest_open_vec(int);                 /* s023 13d0: open a chest */
int far chest_disarm_vec(int);                  /* s023 153e: dissolve a lock */
int far enemies_place_vec(int, ...);              /* s025 0000: start an encounter (reinforcements / activation record) */
int far use_ammo(int);
void far consume_slot(int, int);
void far sub_s014_1db7_0418(int, int);
void far sub_s014_19cd_3258(char *, int *, int, int);
void far sub_s014_19cd_33c0(void far *, int);
int far sub_s014_19cd_3204(struct loot *, int);
void far ems_map_page(int, int, int);
int far index_of_word(int far *, int, int);
void far cbt_forget_target(int);
void far sub_s014_0e76_2802(void);

#define ANIMINFO(i) (combatants[i].info = (int *)(combatants[i].kind * 0x70 + 0xceae))
#define FRAMEDELAY(i) (((unsigned char *)&combatants[i])[0x1f])

/* missile weapon: 1 draw and loose, 9 reload, 10 aim, then 7 (ready) */
void near cbt_missile_action(int i, int set)
{
    unsigned now;
    int n, k;
    char a;
    struct cbt *c;

    now = combat_clock;
    c = &combatants[i];
    if (cbt_action[i] == 1) {
        if (D_d9b0[i] > now)
            return;
        D_d9b0[i] = FRAMEDELAY(i) + now;
        cbt_set_anim(i, 2);
        c->f1b++;
        if ((D_d68d[i][0] & 2) == 2 && (cbt_flags[i][1] & 2) == 2)
            cbt_flags[i][1] &= 0xfd;
        else {
            cbt_flags[i][1] &= 0xfd;
            if (cbt_anim_len[i] - c->f1b != 1)
                return;
            missile_launch_vec(i, c->target, 0, 0, 0);           /* loose */
            if (i < 5)
                D_d68d[i][0] |= 2;
            if (c->weapon > 0x16 && c->weapon < 0x1b)
                vec_1ce3(c->weapon, ((char *)c)[0x5a]);  /* thrown weapon leaves the hand */
            if (_random_n(10) < 7 && ((a = F_20a5_7822.v[c->weapon]) == '@' || a == 'A'))
                vec_1ce3(a, 0x19);      /* arrow/bolt can be picked up again */
            if (use_ammo(i) == -1) {
                cbt_action[i] = 0xd;        /* out of ammunition */
                return;
            }
        }
        cbt_action[i] = 9;
        D_d948[i] = _random_n(10) + combat_clock + 10;
        D_dabd[i] = ((unsigned char *)c)[0xc] * 4 + D_d948[i] + _random_n(10);
        D_daa3[i] = ((unsigned char *)c)[0x1d] + D_dabd[i] + _random_n(10);
        D_e05f[i] = 0;
        D_e06c[i] = 0;
        return;
    }
    if (cbt_action[i] == 9 && D_d948[i] <= combat_clock && combat_clock < D_dabd[i]) {
        /* reload */
        if (D_d9b0[i] <= combat_clock) {
            D_d9b0[i] = FRAMEDELAY(i) + combat_clock;
            if (D_e05f[i] == 0) {
                cbt_anim_seq[i][0] = 0;
                cbt_anim_seq[i][1] = 1;
                ANIMINFO(i);
                D_e05f[i] = 1;
                D_e052[i] = 1;
            }
            if (D_e052[i] == 1) {
                c->f1b = -1;
                D_e052[i] = 0;
            }
            if (++c->f1b == 1)
                D_e052[i] = 1;
        }
        return;
    }
    if ((cbt_action[i] == 10 && combat_clock < D_daa3[i]) || (cbt_action[i] == 9 && D_dabd[i] <= combat_clock)) {
        /* aim */
        cbt_action[i] = 10;
        if (D_d9b0[i] <= now) {
            D_d9b0[i] = FRAMEDELAY(i) + now;
            if (D_e06c[i] == 0) {
                D_e06c[i] = 1;
                n = D_c8a7[set] & 0x7f;
                for (k = 0; k < n; k++)
                    cbt_anim_seq[i][k] = D_a3e7[set][5 + k];
                ANIMINFO(i);
                cbt_anim_len[i] = n;
                D_e052[i] = 1;
            }
            if (D_e052[i] == 1) {
                c->f1b = -1;
                D_e052[i] = 0;
            }
            if (c->f1b < cbt_anim_len[i] - 1)
                c->f1b++;
            if ((D_c8a7[set] & 0x7f) - 1 <= c->f1b && (D_c8a7[set] & 0x80) != 0)
                D_e052[i] = 1;
        }
        return;
    }
    if (cbt_action[i] == 10 && D_daa3[i] <= combat_clock)
        cbt_action[i] = 7;                  /* ready to shoot again */
}

/* spell / potion: 1 start, 11 chant (random 90..130 ticks), 12 cast */
#pragma optimize("g", off)
void near sub_s014_0e76_1f00(int i, int set)
{
    int n, k;
    struct cbt *c;

    c = &combatants[i];
    if (cbt_action[i] == 1) {
        cbt_action[i] = 0xb;
        D_d948[i] = _random_n(40) + combat_clock + 90;
        D_e05f[i] = 0;
        if (demo_mode != 0 && i == 5) {
            cbt_action[5] = 0xc;
            ANIMINFO(5);
            combatants[5].f1b = 0;
        }
    }
    if (cbt_action[i] == 0xb && combat_clock < D_d948[i]) {
        if (D_d9b0[i] <= combat_clock) {
            D_d9b0[i] = FRAMEDELAY(i) + combat_clock;
            cbt_set_anim(i, 2);
            if (D_e05f[i] == 0) {
                cbt_anim_seq[i][0] = 0;
                cbt_anim_seq[i][1] = 1;
                ANIMINFO(i);
                D_e05f[i] = 1;
                D_e052[i] = 1;
            }
            if (D_e052[i] == 1) {
                c->f1b = -1;
                D_e052[i] = 0;
            }
            if (++c->f1b == 1)
                D_e052[i] = 1;
        }
        return;
    }
    if ((cbt_action[i] == 0xb && D_d948[i] <= combat_clock) || cbt_action[i] == 0xc) {
        if (cbt_action[i] != 0xc) {
            cbt_action[i] = 0xc;
            ANIMINFO(i);
            n = D_c84a[set] & 0x7f;
            cbt_anim_len[i] = n;
            for (k = 0; k < cbt_anim_len[i]; k++)
                cbt_anim_seq[i][k] = D_a3e7[set][k];
            c->f1b = -1;
        }
        if (D_d9b0[i] <= combat_clock) {
            D_d9b0[i] = FRAMEDELAY(i) + combat_clock;
            cbt_set_anim(i, 2);
            c->f1b++;
            if (cbt_anim_len[i] - c->f1b == 1) {
                missile_launch_vec(i, c->target, 6, D_ca0c[i], D_c97a[i]);
                vec_1cf7(i, D_ca0c[i], D_c97a[i]);
                cbt_action[i] = 0xd;
                if (demo_mode != 0 && i == 5) {
                    cbt_action[5] = 7;
                    combatants[5].f10 = 0x12;
                    D_9f1f = -1;
                    if (demo_mode == 'I')
                        strcpy(D_117e, "D7");
                    else
                        strcpy(D_117e, "CD");
                    return;
                }
                if (i < 5) {
                    D_d68d[i][0] |= 4;
                    return;
                }
                combatants[i].target = -1;
            }
        }
    }
}
#pragma optimize("g", on)

/* location actions ordered from the menu (cbt_special[i]) */
void near cbt_location_action(int i)
{
    if (cbt_special[i] == 1) {               /* use door */
        door_toggle_vec(D_c64c[i], i);
        D_e07b = 1;
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 3) {               /* pick the lock of a door */
        if (lock_pick_vec(&F_20a5_1f54[(unsigned char)D_c64c[i]].lock) == 1) {
            door_toggle_vec(D_c64c[i], i);
            D_e07b = 1;
        }
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 6) {               /* pick the lock of a chest */
        if (lock_pick_vec(&F_20a5_300e[(unsigned char)D_c784[i]].lock) == 1) {
            chest_open_vec(i);
            D_e07b = 1;
        }
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 4) {               /* dissolve a lock */
        if (chest_disarm_vec(i) == 1) {
            chest_open_vec(i);
            D_e07b = 1;
        }
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 7) {               /* force with a tool from the inventory */
        show_message((char *)0x590, 150);
        consume_slot(i, D_c7cd[i]);
        if (D_ca99 == 0)
            chest_open_vec(i);
        else
            door_toggle_vec(D_c64c[i], i);
        D_e07b = 1;
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 5) {               /* open chest */
        chest_open_vec(i);
        D_e07b = 1;
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 2) {               /* stairs */
        take_exit_vec(D_c77e[i], i);
        cbt_cancel_action(i);
    }
    if (cbt_special[i] == 8) {               /* leave the battlefield */
        cbt_floor[i] = -99;
        D_e07b = 1;
        cbt_flags[i][0] &= 0xdf;
        cbt_flags[i][0] |= 0x40;
        cbt_forget_target(i);
        cbt_cancel_action(i);
        party_panel_rows(i);
    }
    if (cbt_special[i] == 9) {               /* take loot */
        loot_pile_open(i);
        cbt_cancel_action(i);
    }
}

/* falling / dying; when the death animation ends the body becomes a loot pile */
void near cbt_dying(int i, int set)
{
    struct cbt *c;
    int n, k, pile, o;
    int ofs;
    unsigned cnt;
    struct item6 far *src;
    struct pile far *p;

    c = &combatants[i];
    cbt_set_anim(i, 4);
    c->f10 = 0;
    cbt_anim[i] = 0;
    cbt_action[i] = 0;
    if (c->f1b < cbt_anim_len[i] - 1)
        c->f1b++;
    if (D_c83f[set] == 2 || !(c->status == 3 || i > 4) || c->f1b - cbt_anim_len[i] != -1
     || (cbt_flags[i][1] & 1) == 1)
        return;
    if ((cbt_flags[i][0] & 0x80) == 0x80) {
        cbt_flags[i][1] |= 1;
        if (cbt_floor[i] != -99)
            D_e07b = 1;
        if (i > 4) {
            c->status = 0;
            sub_s014_1db7_0418(i, c->kind);
        }
        return;
    }
    cbt_flags[i][0] |= 0x80;
    sub_s014_0e76_465e(10);
    if (D_c1f3[i] == 1)
        D_9084 = 1;
    if (D_c803[set] == 1) {
        n = c->nitems;
        src = i < 5 ? inventories[i] : enemy_items[i - 5];
        pile = -1;
        for (k = 0; k < loot_pile_count; k++) {
            p = &loot_piles[k];
            if (p->level == floor_shown && c->x - 16 <= p->x && p->x < c->x + 16
             && c->y - 16 <= p->y && p->y < c->y + 16) {
                pile = k;
                break;
            }
        }
        if (pile == -1) {
            loot_piles[loot_pile_count].cnt = c->nitems;
            _fmemcpy(loot_list, src, n * 6);
            loot_piles[loot_pile_count].ofs = sub_s014_19cd_3204(loot_list, n);
        } else {
            /* merge with the pile already there */
            ofs = loot_piles[pile].ofs;
            cnt = (unsigned char)loot_piles[pile].cnt;
            D_c67f = 0;
            if (ofs >= 0)
                sub_s014_19cd_3258((char *)&cnt, &ofs, 1, 0);
            sub_s014_19cd_33c0(src, n);
            ems_map_page(3, 10, ems_frame_handle);
            _fmemcpy(loot_list, D_c654, D_c67f * 6);
            ems_map_page(3, 3, ems_frame_handle);
            o = sub_s014_19cd_3204(loot_list, D_c67f);
            loot_piles[loot_pile_count].ofs = o;
            loot_piles[loot_pile_count].cnt = D_c67f;
            for (k = 0; k < loot_pile_count; k++)
                if (loot_piles[k].ofs == -1) {
                    loot_piles[k].ofs = o;
                    loot_piles[k].cnt = D_c67f;
                }
        }
    } else {
        loot_piles[loot_pile_count].ofs = -2;
        loot_piles[loot_pile_count].cnt = 0;
    }
    loot_piles[loot_pile_count].level = floor_shown;
    loot_piles[loot_pile_count].x = c->x;
    loot_piles[loot_pile_count].y = c->y;
    sub_s014_0e76_2802();
    loot_piles[loot_pile_count].dx = 10;
    loot_piles[loot_pile_count].dy = 4;
    if (D_c83f[set] == 1)
        loot_piles[loot_pile_count].vis = 1;
    else if (D_c83f[set] == 0)
        loot_piles[loot_pile_count].vis = 0;
    loot_pile_count++;
    if (D_d6eb == 1 && i > 4 && ++D_d6ec == F_2e38_0bfc[0] && F_2e38_0bfc[1] > 0) {
        /* a wave of reinforcements for every F_2e38_0bfc enemies killed */
        D_d6ec = 0;
        if ((F_2e38_0bfc[1] -= F_2e38_0bfc[0]) < 0)
            F_2e38_0bfc[0] += F_2e38_0bfc[1];
        enemies_place_vec(1);
    }
    for (k = 0; k < 13; k++)
        if (combatants[k].status == 1 && combatants[k].target == i
         && (combatants[k].f10 == 0x80 || combatants[k].f10 == 0x100) && (demo_mode == 0 || k != 5)) {
            cbt_action[k] = 0xd;
            combatants[k].target = -1;
        }
}

/* cancel i's location action */
void far cbt_cancel_action(int i)
{
    *(volatile char *)&cbt_special[i] = 0;
    combatants[i].f10 = 0;
    cbt_anim[i] = 0;
}

/* enemies forget i as target */
void far cbt_forget_target(int i)
{
    int k;

    for (k = 5; k < 13; k++)
        if (combatants[k].target == i) {
            combatants[k].target = -1;
            combatants[k].f10 = 0;
            *(volatile char *)&cbt_anim[k] = 0;
            cbt_moving[k] = 0;
        }
}

/* move the newest loot pile so that it is drawn inside one screen tile */
void far sub_s014_0e76_2802(void)
{
    int dy[4], dx[4], t[4];
    int x, y, e1, e2, e3, e4, t0, k, n;
    struct pile far *p;

    dx[2] = -8;
    dx[0] = -8;
    dx[3] = 8;
    dx[1] = 8;
    dy[1] = -4;
    dy[0] = -4;
    dy[3] = 4;
    dy[2] = 4;
    p = &loot_piles[loot_pile_count];
    x = p->x;
    y = p->y;
    e1 = -(x % 32 - 8);
    e2 = -(y % 16 - 4);
    e3 = x % 32 - 24;
    e4 = y % 16 - 12;
    t0 = index_of_word(F_20a5_5890, (x & 0xffe0) * 8 + (y >> 4), 0x8c);
    for (n = 0, k = 0; k < 4; k++)
        if ((t[k] = index_of_word(F_20a5_5890, ((dx[k] + x) & 0xffe0) * 8 + ((dy[k] + y) >> 4),
                                       0x8c)) == t0)
            n++;
    if (n == 2) {
        if (t[0] == t0 && t[1] == t0)
            p->y -= e4;
        else if (t[0] == t0 && t[2] == t0)
            p->x -= e3;
        else if (t[2] == t0 && t0 == t[3])
            p->y += e2;
        else
            p->x += e1;
    } else if (n == 1) {
        if (t[0] == t0) {
            p->x -= e3;
            p->y -= e4;
        } else if (t[1] == t0) {
            p->x += e1;
            p->y -= e4;
        } else if (t0 == t[2]) {
            p->x -= e3;
            p->y += e2;
        } else {
            p->x += e1;
            p->y += e2;
        }
    }
}

/* push direction: the attacker's facing, randomly turned by one */
int far push_direction(int i)
{
    int f, r;

    f = D_d3fe[i];
    r = _random_n(10);
    if (r < 2)
        f++;
    else if (r > 7)
        f--;
    if (f > 7)
        f = 0;
    else if (f < 0)
        f = 7;
    return f;
}

extern int D_e096;
struct catn { char s[9]; };             /* sprite catalogue file name */
/* module data DGROUP 17e2..18c1 as one packed struct: the game's compiler did not word-align the
   odd-sized items ("wk", the 9-byte catalogue names, ".imc"); 6.00A aligns every definition */
struct dat17e2 {
    char t17e2[4][4];                   /* 17e2 [quadrant][slope] -> facing */
    char wk[3], cb[3], dy[3];           /* 17f2 sprite file kinds: walk, combat, dying */
    struct catn a00c, c00c, e00c, m00c, f60c, f01c;     /* 17fb catalogues */
    char imc[5];                        /* 1831 */
    int t1836[0x38];                    /* 1836 2-letter weapon animation codes */
    char t18a6[8], t18ae[8];            /* 18a6 view offset per facing */
    char t18b6[6], t18bc[6];            /* 18b6 effect area half sizes by size class */
} D_17e2_ = {
    {{2, 1, 0, 0}, {2, 3, 4, 5}, {6, 5, 0, 0}, {6, 7, 0, 1}},
    "wk", "cb", "dy",
    {"A00C.cat"}, {"C00C.cat"}, {"E00C.cat"}, {"M00C.cat"}, {"F60C.cat"}, {"F01C.cat"},
    ".imc",
    {
      'S' | '2' << 8, 'S' | 'W' << 8, 'C' | 'D' << 8, 'D' | 'R' << 8, 'A' | '2' << 8, 'A' | 'X' << 8, 'D' | 'X' << 8, 'H' | '2' << 8,
      'M' | '2' << 8, 'H' | 'M' << 8, 'M' | 'C' << 8, 'F' | '2' << 8, 'F' | 'L' << 8, 'S' | 'P' << 8, 'S' | 'Y' << 8, 'Q' | 'S' << 8,
      'S' | 'L' << 8, 'T' | 'A' << 8, 'D' | 'T' << 8, 'T' | 'K' << 8, 'B' | 'W' << 8, 'C' | 'B' << 8, 'H' | 'G' << 8, 'P' | 'T' << 8,
      'P' | 'Y' << 8, 'D' | 'P' << 8, 'V' | 'L' << 8, 'D' | '7' << 8, 'B' | 'E' << 8, 'G' | 'G' << 8, 'G' | 'F' << 8, 'H' | 'H' << 8,
      'T' | 'Z' << 8, 'W' | 'W' << 8, 'W' | 'F' << 8, 'S' | 'D' << 8, 'H' | 'P' << 8, 'C' | 'A' << 8, 'A' | 'U' << 8, 'U' | 'N' << 8,
      'B' | 'R' << 8, 'W' | 'V' << 8, 'G' | 'N' << 8, 'M' | 'N' << 8, 'M' | 'M' << 8, 'H' | 'B' << 8, 'C' | 'L' << 8, 'W' | 'H' << 8,
      'K' | 'B' << 8, 'S' | 'K' << 8, 'S' | 'C' << 8, 'O' | 'G' << 8, 'W' | 'S' << 8, 'D' | 'Z' << 8, 'P' | '7' << 8, 'P' | 'C' << 8
    },
    {0, 4, 12, 12, 0, -4, -12, -12}, {-16, -16, 0, 16, 16, 16, 0, -16},
    {0, 8, 16, 24, 32, 0}, {0, 4, 8, 12, 16, 0}
};
#define D_17e2 (D_17e2_.t17e2)
#define D_17f2 (D_17e2_.wk)
#define D_17f5 (D_17e2_.cb)
#define D_17f8 (D_17e2_.dy)
#define D_17fb (D_17e2_.a00c)
#define D_1804 (D_17e2_.c00c)
#define D_180d (D_17e2_.e00c)
#define D_1816 (D_17e2_.m00c)
#define D_181f (D_17e2_.f60c)
#define D_1828 (D_17e2_.f01c)
#define D_1831 (D_17e2_.imc)
#define D_1836 (D_17e2_.t1836)
#define D_18a6 (D_17e2_.t18a6)
#define D_18ae (D_17e2_.t18ae)
#define D_18b6 (D_17e2_.t18b6)
#define D_18bc (D_17e2_.t18bc)                /* [quadrant][slope] -> facing */
void far attack_rate(int);
int far cbt_attack(int, int);
int far cbt_hostile_pair(int, int);
void far sub_s014_0e76_2cca(int);
void far sub_s014_0e76_3cc4(int);

/* engagement tick: every ready combatant (cbt_action == 7) with an attack order starts an
   attack on its target (missiles need a line of fire, melee needs reach); movers approach;
   temporary modifiers expire */
#pragma optimize("a", on)
#pragma optimize("l", off)
void far melee_exchanges(void)
{
    int i, n, r, t;
    struct cbt *c;

    for (i = 0; i < 13; i++) {
        c = &combatants[i];
        if (c->status != 1)
            goto mods;
        if ((c->f10 & 0x1be) != 0 && cbt_moving[i] == 0 && cbt_engage_time[i] <= combat_clock && cbt_action[i] == 7) {
            D_a3d8[i] = 0;
            if (demo_mode != 0 && i == 5 && battle_result == -1) {
                /* tutorial opponent picks a target */
                do {
                    if (combatants[5].f10 == 0x100)
                        combatants[5].target = _random_n(5);
                    else {
                        combatants[5].target = -1;
                        for (n = 0; ; ) {
                            r = _random_n(5);
                            if (combatants[r].status == 1 && (combatants[r].f10 & 2) != 0) {
                                combatants[5].target = r;
                                D_c21f = 6;
                                break;
                            }
                            if (!(n++ < 10))
                                break;
                        }
                    }
                } while (combatants[5].target != -1 && combatants[combatants[5].target].status != 1);
                combatants[5].f1a = _random_n(4) + 1;
            }
            t = c->target;
            if ((c->f10 & 0x80) == 0x80) {
                los_x0 = c->x;
                los_y0 = c->y;
                los_x1 = combatants[t].x;
                los_y1 = combatants[t].y;
                if (line_blocked(i, t, 0x80) == 1) {
                    /* no line of fire */
                    if (c->pose != 2) {
                        cbt_set_anim(i, 2);
                        c->f1b = 0;
                    }
                    D_a3d8[i] = 1;
                    party_panel_rows(i);
                    if (i < 5)
                        cbt_engage_time[i] = _random_n(60) + combat_clock + 30;
                    else {
                        cbt_action[i] = 0xd;
                        c->target = -1;
                    }
                } else
                    party_panel_rows(i);
            }
            if (D_a3d8[i] == 0) {
                if ((c->f10 & 2) == 2 && cbt_hostile_pair(i, t) == 0) {
                    cbt_action[i] = 0xd;    /* target out of reach */
                    c->target = -1;
                    goto mods;
                }
                c->f1b = -1;
                cbt_action[i] = 1;
                D_d9b0[i] = combat_clock;
                attack_rate(i);
                cbt_engage_time[i] = _random_n(30) + combat_clock + 60;
                cbt_attack(i, t);       /* roll to hit and for damage now */
            }
        }
        if (cbt_moving[i] == 1)
            sub_s014_0e76_2cca(i);
mods:
        if (ai_paused == 0) {
            if (D_a532[i].t0 != 65000u && (unsigned)D_a532[i].t0 < combat_clock)
                vec_1ccf(i);
            if (D_a532[i].t1 != 65000u && (unsigned)D_a532[i].t1 < combat_clock)
                vec_1cc5(i);
            if (D_a532[i].t2 != 65000u && (unsigned)D_a532[i].t2 < combat_clock)
                vec_1cbb(i);
        }
    }
}
#pragma optimize("a", off)
#pragma optimize("l", on)

/* approach the target: re-path every 30..90 ticks */
void far sub_s014_0e76_2cca(int i)
{
    struct cbt *c;
    int sx, sy, k, blocked, n;

    c = &combatants[i];
    if (((struct mod2 *)D_a532)[i].held == 1) {
        c->f1b = 0;
        return;
    }
    if (cbt_paths[i][(unsigned char)D_d6fb[i]] == -1) {
        sub_s014_0e76_2fa2(i);          /* end of the path */
        return;
    }
    if (D_ca30[i] == -1)
        D_ca30[i] = cbt_paths[i][(unsigned char)D_d6fb[i]];
    if (D_d97c[i] > combat_clock)
        return;
    if (c->target != -1) {
        c->tx = combatants[c->target].x;
        c->ty = combatants[c->target].y;
    }
    if (path_find(i) == 1) {
        path_give(i);
        D_d6fb[i] = 0;
    } else {
        /* no path: is the destination occupied by somebody who is not moving? */
        sx = c->x;
        sy = c->y;
        c->x = c->tx;
        c->y = c->ty;
        if (cbt_collisions(c) != 0) {
            blocked = 0;
            for (k = 0; k < 13; k++)
                if ((D_d708 & 1 << k) != 0 && cbt_moving[k] == 0) {
                    blocked = 1;
                    break;
                }
            if (blocked) {
                if (abs(c->x - c->tx) < 16 && abs(c->y - c->ty) < 8) {
                    /* (always true: x was just set to tx) give up */
                    cbt_cancel_action(i);
                    cbt_action[i] = 0;
                    cbt_moving[i] = 0;
                    c->target = -1;
                    order_queue_clear(i);
                    if (i >= 5)
                        cbt_flags[i][1] |= 4;
                } else if (i >= 5) {
                    cbt_flags[i][1] |= 4;
                    cbt_cancel_action(i);
                    cbt_action[i] = 0;
                    cbt_moving[i] = 0;
                    c->target = -1;
                    order_queue_clear(i);
                } else {
                    /* look for a free visible spot near the destination */
                    sx = c->x;
                    sy = c->y;
                    los_x0 = c->tx;
                    los_y0 = c->ty;
                    n = 0;
                    D_e0a2 = 1;
                    for (;;) {
                        c->x = _random_n(32) + los_x0 - 16;
                        c->y = _random_n(20) + los_y0 - 10;
                        if (cbt_collisions(c) == 0)
                            los_y1 = los_x1 = sx;
                        if (line_blocked(-1, 0, 0) == 0)
                            break;
                        if (++n > 100) {
                            cbt_moving[i] = 0;
                            c->f10 = 0;
                            order_queue_clear(i);
                            break;
                        }
                    }
                    D_e0a2 = 0;
                    if (n <= 100) {
                        c->tx = c->x;
                        c->ty = c->y;
                        c->x = sx;
                        c->y = sy;
                        if (path_find(i) == 1) {
                            path_give(i);
                            D_d6fb[i] = 0;
                            cbt_moving[i] = 1;
                            D_d97c[i] = _random_n(60) + combat_clock + 30;
                        }
                    }
                }
            }
        }
        c->x = sx;
        c->y = sy;
    }
    D_d97c[i] = _random_n(60) + combat_clock + 30;
}

/* arrived / stopped: back to "ready" */
void far sub_s014_0e76_2fa2(int i)
{
    int was, save;
    struct win *w;

    if (combatants[i].f1b != 0)
        combatants[i].f1b = 0;
    cbt_moving[i] = 0;
    cbt_flags[i][1] &= 0xef;
    cbt_action[i] = 7;
    was = 0;
    if ((combatants[i].f10 & 0x1be) != 0) {
        cbt_engage_time[i] = combat_clock;
        was = 1;
    }
    sub_s014_0e76_3cc4(i);
    if ((combatants[i].f10 & 0x1be) != 0) {
        cbt_engage_time[i] = combat_clock;
        if (!was)
            D_d3fe[i] = combatants[i].f19;
    }
    if (combatants[i].target == -1 && combatants[i].f10 != 0)
        combatants[i].f10 = 0;
    if (i < 5) {
        w = D_0190;
        save = w->f0c;
        if (leader == i)
            w->f0c = 0xe;
        else
            w->f0c = 0xf;
        member_status_letter(i, w);
        D_0190->f0c = save;
    }
}

/* facing (0..7) of i towards its target, or its destination for a spell at a spot */
#pragma optimize("a", on)
#pragma optimize("g", off)
int far sub_s014_0e76_306a(int i)
{
    struct cbt *c;
    int dx, dy, ty, adx, ady, r1, r4, quad;

    c = &combatants[i];
    if (c->f10 != 0x100 || c->target != -1) {
        dx = combatants[c->target].x - c->x;
        ty = combatants[c->target].y;
    } else {
        dx = c->tx - c->x;
        ty = c->ty;
    }
    dy = ty - c->y;
    ady = abs(dy);
    adx = abs(dx);
    if (adx == 0) {
        r4 = 20;
        r1 = 20;
    } else {
        r1 = ady / adx;
        r4 = ady * 4 / adx;
    }
    if (dx >= 0 && dy < 0)
        quad = 0;
    else if (dx >= 0 && dy >= 0)
        quad = 1;
    else if (dx < 0 && dy >= 0)
        quad = 2;
    else
        quad = 3;
    if (quad == 0 || quad == 2)
        D_e096 = r1 != 0;
    else if (r4 == 0)
        D_e096 = 0;
    else if (r4 >= 1 && r4 <= 3)
        D_e096 = 1;
    else if (r4 >= 4 && r4 <= 9)
        D_e096 = 2;
    else
        D_e096 = 3;
    return D_17e2[quad][D_e096];
}
#pragma optimize("a", off)
#pragma optimize("g", on)

extern int mouse_x, mouse_y;              /* mouse x, y */
extern char D_d7fe;                     /* 1 = mouse drag (rubber band) active */
extern int *D_c4fe;                     /* mouse event record {?, count, y, x} */
extern char D_c866, D_c886;             /* view origin (scrolled by the edge arrows) */
extern char D_0880, D_0881;             /* key pending */
extern char cbt_floor[];
extern int cursor_default, cursor_current;              /* default cursor, current cursor */
extern int D_d72b, D_d72d, D_d72f, D_d731, D_d733, D_d735, D_d737, D_d739; /* scroll cursors */
void far menu_bar_run(int, int);
int far key_pressed(void);
void far mouse_hide(void);
void far mouse_set_cursor(int, int, int);
void far combat_mouse(void);
void far combat_char_sheet_vec(int);                 /* s022 0000: portrait click: select */
void far vec_1d5b(int);                 /* s019 0b9e: refresh the member */
void far vec_1d51(int);                 /* s022 04e4: portrait click: unconscious/absent */
void far member_activate_vec(int);                 /* s016 09bc: shift-click: make the leader */
void far vec_1d3d(void);                /* s016 0b92: scroll the view */
void far combat_keys_vec(void);                /* s016 0000: keyboard */
void far sub_s014_0e76_33a2(void);
void far cursor_keys(void);
void far cursor_normal(void);
#define KBFLAGS (*(unsigned char far *)0x00400017L)
#define MX mouse_x
#define MY mouse_y

/* player input of one tick: cursor shape, rubber band, portrait clicks, scroll arrows
   (shift + mouse at the screen edge), keyboard */
void far combat_input(void)
{
    int i;
    char *p;

    sub_s014_0e76_33a2();
    if (D_d7fe == 1)
        cursor_keys();
    combat_mouse();
    menu_bar_run(0x9419, 0);
    if (D_c62a == 0) {
        D_c4fe = (int *)sub_root_047c_1320(6, 0);
        D_c62a = D_c4fe[1];
    }
    if (D_c62a > 0 && D_c4fe[2] < 0x3c) {
        if ((KBFLAGS & 4) == 0) {
            i = D_c4fe[3] / 40;
            if (combatants[i].status == 1 && cbt_floor[i] != -99) {
                combat_char_sheet_vec(D_c4fe[3]);
                for (i = 0, p = &combatants[0].status; p < &combatants[5].status; i++, p += 0x80)
                    if (*p == 1)
                        vec_1d5b(i);
            } else
                vec_1d51(i);
        } else
            member_activate_vec(D_c4fe[3] / 40);
        D_c62a = 0;
    }
    if ((KBFLAGS & 2) != 0 || (KBFLAGS & 1) != 0) {
        if (MX > 0x7d && MX < 0xff && MY < 8) {
            D_c866--;
            goto up;
        }
        if (MX > 0x7d && MX < 0xff && MY > 0xbf) {
            D_c866++;
            goto down;
        }
        if (MX >= 0x3c && MX < 0x44 && MY > 0x32 && MY < 0x96) {
            D_c866--;
            goto scroll;
        }
        if (MX >= 0x137 && MY > 0x32 && MY < 0x96) {
            D_c866++;
            goto scroll;
        }
        if ((MX >= 0x3c && MX <= 0x7d && MY < 8) || (MX >= 0x3c && MX < 0x44 && MY <= 0x32)) {
            D_c866 -= 2;
            goto up;
        }
        if ((MX >= 0xff && MY < 8) || (MX >= 0x137 && MY <= 0x32)) {
up:         D_c886--;
            goto scroll;
        }
        if ((MX >= 0x3c && MX <= 0x7d && MY > 0xbf) || (MX >= 0x3c && MX < 0x44 && MY >= 0x96))
            goto down;
        if ((MX >= 0xff && MY > 0xbf) || (MX >= 0x137 && MY >= 0x96)) {
            D_c866 += 2;
down:       D_c886++;
scroll:     vec_1d3d();
        }
    }
    if (key_pressed() != 0 || D_0880 != 0 || D_0881 != 0)
        combat_keys_vec();
}

#define CURSOR(c, a, b) { if (cur == c) return; mouse_hide(); \
    mouse_set_cursor(a, b, c); mouse_show(); cursor_current = c; return; }

void far sub_s014_0e76_4694(void);       /* (declaration position matters for 38b6) */
/* mouse cursor shape: scroll arrows at the edges of the map window */
void far sub_s014_0e76_33a2(void)
{
    int cur;

    cur = cursor_current;
    if (MX >= 0x3c || cursor_default == cur) {
        if (MX > 0x7d && MX < 0xff && MY < 8)
            CURSOR(D_d72b, 3, 0)
        if (MX > 0x7d && MX < 0xff && MY > 0xbf)
            CURSOR(D_d733, 3, 7)
        if (MX >= 0x3c && MX < 0x44 && MY > 0x32 && MY < 0x96)
            CURSOR(D_d737, 0, 4)
        if (MX >= 0x137 && MY > 0x32 && MY < 0x96)
            CURSOR(D_d72f, 7, 4)
        if ((MX >= 0x3c && MX <= 0x7d && MY < 8) || (MX >= 0x3c && MX < 0x44 && MY <= 0x32))
            CURSOR(D_d739, 0, 0)
        if ((MX >= 0xff && MY < 8) || (MX >= 0x137 && MY <= 0x32))
            CURSOR(D_d72d, 7, 0)
        if ((MX >= 0x3c && MX <= 0x7d && MY > 0xbf) || (MX >= 0x3c && MX < 0x44 && MY >= 0x96))
            CURSOR(D_d735, 0, 7)
        if ((MX >= 0xff && MY > 0xbf) || (MX >= 0x137 && MY >= 0x96))
            CURSOR(D_d731, 7, 7)
        if (cursor_default == cur)
            return;
    }
    cursor_normal();
}

extern int D_e094;
extern char D_d728, D_d947, D_d73b;
extern int D_07d6, D_d945;              /* key held since */
void far mouse_move(int, int);

/* cursor keys move the mouse pointer (D_0880/D_0881 = key direction x/y); the
   longer the key is held, the faster it moves */
void far cursor_keys(void)
{
    int speed;
    int key;
    int a, b;
    int t;

    key = 0;
    if (D_0880 == 0 && D_0881 == 0) {
        D_d728 = 0;
        if (D_d947 == 1) {
            D_d73b = 1;
            D_d947 = 0;
        }
        D_07d6 = D_d945 = 0;
        return;
    }
    if (D_e094 != D_e092) {
        D_e094 = D_e092;
        D_d728 = 1;
        if (move_mode != 1)
            D_d947 = 1;
        if (D_0880 < 0 && D_0881 < 0)
            key = 0x4700;
        else if (D_0880 < 0 && D_0881 > 0)
            key = 0x4f00;
        else if (D_0880 > 0 && D_0881 < 0)
            key = 0x4900;
        else if (D_0880 > 0 && D_0881 > 0)
            key = 0x5100;
        else if (D_0880 < 0)
            key = 0x4b00;
        else if (D_0880 > 0)
            key = 0x4d00;
        else if (D_0881 < 0)
            key = 0x4800;
        else if (D_0881 > 0)
            key = 0x5000;
    }
    t = D_07d6 - D_d945;
    if (t <= 0x2d)
        speed = 1;
    else if (t <= 0x4b)
        speed = 3;
    else if (t <= 0x5a)
        speed = 5;
    else
        speed = 10;
    switch (key) {
    case 0x4800:                        /* up */
        a = MY - speed >= 0 ? MY - speed : 0;
        mouse_move(MX, a);
        break;
    case 0x5000:                        /* down */
        a = speed + MY > 0xc7 ? 0xc7 : speed + MY;
        mouse_move(MX, a);
        break;
    case 0x4b00:                        /* left */
        a = MX - speed >= 0x3d ? MX - speed : 0x3d;
        mouse_move(a, MY);
        break;
    case 0x4d00:                        /* right */
        a = speed + MX > 0x13f ? 0x13f : speed + MX;
        mouse_move(a, MY);
        break;
    case 0x4700:                        /* up left */
        a = MY - speed >= 0 ? MY - speed : 0;
        b = MX - speed >= 0x3d ? MX - speed : 0x3d;
        mouse_move(b, a);
        break;
    case 0x4f00:                        /* down left */
        b = speed + MY > 0xc7 ? 0xc7 : speed + MY;
        a = MX - speed >= 0x3d ? MX - speed : 0x3d;
        mouse_move(a, b);
        break;
    case 0x5100:                        /* down right */
        b = speed + MY > 0xc7 ? 0xc7 : speed + MY;
        a = speed + MX > 0x13f ? 0x13f : speed + MX;
        mouse_move(a, b);
        break;
    case 0x4900:                        /* up right */
        b = MY - speed >= 0 ? MY - speed : 0;
        a = speed + MX > 0x13f ? 0x13f : speed + MX;
        mouse_move(a, b);
        break;
    default:
        return;
    }
    if (key_pressed() != 0)
        get_key();
}

extern char *D_1166[], *anim_file[];       /* sprite file prefix / suffix per animation kind */
extern char *D_11b2[];                  /* sprite file loaded in each slot */
extern char D_cab4[][0x90];             /* walk sprite info per kind */
extern char D_ceae[][0x70];             /* combat sprite info */
extern char D_d1e4[][0x40];             /* dying sprite info */
extern int anim_buffers[], D_ca75[], D_ca8b[];    /* sprite memory per slot */
extern char D_c908[], D_c90c[];         /* walk / dying sequence length per kind */
extern char D_a3e7[][30];               /* attack frame sequence per slot */
extern char D_c3cc;
extern int D_d611[13];
int _stricmp(const char *, const char *);
char *strcat(char *, const char *);
void far sprite_file_load(int, char *, void *, int, int);

/* set the animation of combatant i: 5 walk, 2 attack, 3 hit, 4 dying.  The sprite file
   name is prefix(kind) + "wk"/"cb"/"dy" + suffix(kind); the file is loaded into the
   kind's slot unless it is already there */
void far cbt_set_anim(int i, int mode)
{
    int slot;
    int old;
    int kind;
    char name[10];
    int k;

    old = combatants[i].pose;
    slot = kind = combatants[i].kind;
    strcpy(name, D_1166[kind]);
    switch (mode) {
    case 2:
    case 3:
        strcat(name, D_17f5);
        if (kind >= 5)
            slot += 2;
        break;
    case 4:
        strcat(name, D_17f8);
        if (kind >= 5)
            slot += 4;
        break;
    case 5:
        strcat(name, D_17f2);
        break;
    }
    if (mode != 4)
        strcat(name, anim_file[kind]);
    if (old == mode && _stricmp(name, D_11b2[slot]) == 0)
        return;
    combatants[i].pose = mode;
    if (mode == 5) {
        if (_stricmp(name, D_11b2[slot]) != 0)
            sprite_file_load(i, name, D_cab4[kind], slot, anim_buffers[kind]);
        D_d1ce[i] = -1;
        cbt_anim_len[i] = D_c908[kind];
        combatants[i].info = (int *)D_cab4[kind];
        for (k = 0; k <= 9; k++)
            cbt_anim_seq[i][k] = k;
        if (D_c3cc == 0)
            combatants[i].f1b = 0;
    } else {
        if (mode == 2) {
            if (_stricmp(name, D_11b2[slot]) != 0)
                if (kind < 5)
                    sprite_file_load(i, name, D_ceae[kind], kind, anim_buffers[kind]);
                else
                    sprite_file_load(i, name, D_ceae[kind], kind + 2, D_ca75[kind]);
            cbt_anim_len[i] = D_c84a[slot] & 0x7f;
            for (k = 0; k < cbt_anim_len[i]; k++)
                cbt_anim_seq[i][k] = D_a3e7[slot][k];
            combatants[i].info = (int *)D_ceae[kind];
        } else if (mode == 3) {
            if (_stricmp(name, D_11b2[slot]) != 0)
                if (kind < 5)
                    sprite_file_load(i, name, D_ceae[kind], kind, anim_buffers[kind]);
                else
                    sprite_file_load(i, name, D_ceae[kind], kind + 2, D_ca75[kind]);
            combatants[i].info = (int *)D_ceae[kind];
            cbt_anim_len[i] = 2;
            for (k = 0; k <= 2; k++)
                cbt_anim_seq[i][k] = k;
        } else if (mode == 4) {
            if (_stricmp(name, D_11b2[slot]) != 0)
                if (kind < 5)
                    sprite_file_load(i, name, D_d1e4[kind], kind, anim_buffers[kind]);
                else
                    sprite_file_load(i, name, D_d1e4[kind], kind + 4, D_ca8b[kind]);
            if (kind >= 5)
                cbt_anim_len[i] = D_c90c[kind];
            combatants[i].info = (int *)D_d1e4[kind];
            for (k = 0; k <= cbt_anim_len[i]; k++)
                cbt_anim_seq[i][k] = k;
        } else
            goto done;
        if (D_c3cc == 0)
            combatants[i].f1b = -1;
    }
done:
    D_d611[i] = 100;
}

extern int order_queues[][3];                 /* [13] queued orders */

/* queue an order for combatant i */
void far order_queue(int i, int v)
{
    int k;

    for (k = 0; k < 3; k++)
        if (order_queues[i][k] == 0) {
            order_queues[i][k] = v;
            return;
        }
}

/* take the next queued order into the action word */
#pragma optimize("a", on)
void far sub_s014_0e76_3cc4(int i)
{
    int k;

    combatants[i].f10 = order_queues[i][0];
    for (k = 0; k < 2; k++)
        order_queues[i][k] = order_queues[i][k + 1];
    order_queues[i][2] = 0;
}
#pragma optimize("", on)

/* clear the order queue */
#pragma optimize("a", on)
void far order_queue_clear(int i)
{
    int k;

    for (k = 0; k < 3; k++)
        order_queues[i][k] = 0;
}
#pragma optimize("a", off)

struct anim { char seq[10]; char a; char b; char c; unsigned char flags; char e; };   /* 15 bytes */
extern struct anim far F_20a5_4276[];   /* attack animation per weapon code */
extern char far *load_buf;                /* load buffer */
extern char D_e0ab;                     /* full redraw request */
extern char D_119a[], D_11a6[];
extern char D_c763[], D_c799[], D_c7d2[], D_c803[], D_c83f[], D_c8ca[], D_d5de[];
extern char D_d55d[][8];                /* sprite width by set and facing */
extern unsigned D_c8b8, D_c8f5, D_c913;
extern char *D_c943[20];
extern char D_c86b[];
void far datafile_select(char *);    /* open a catalogue */
void far file_read_all(char *, char far *);    /* load a file */
void far sub_s014_1ec9_00da(int, int, int);         /* recolour the sprite set */

/* load the sprite file `name` into slot `slot` (sprite memory segment mem): read the
   header (hot spot, sizes, step tables, attack sequence by weapon code) and fill
   the frame offset table `frames` */
void far sprite_file_load(int i, char *name, int *frames, int slot, int mem)
{
    char c;
    int k, m, j;
    int n;
    int u;
    int far *q;
    struct catn cat;
    char fname[60];
    int far *p;

    c = *name;
    if (c > '_')
        c -= 0x20;
    switch (c) {
    case 'A':
        cat = D_17fb;
        break;
    case 'C':
        cat = D_1804;
        break;
    case 'E':
        cat = D_180d;
        break;
    case 'F':
        if (name[1] == '6')
            cat = D_181f;
        else
            cat = D_1828;
        break;
    case 'M':
        cat = D_1816;
        if (name[1] == '7' && name[2] == '3')
            demo_mode = 0x49;
        else if (name[1] == '9' && name[2] == '0')
            demo_mode = 0x5a;
        break;
    }
    datafile_select(cat.s);
    strcpy(fname, name);
    strcat(fname, D_1831);
    file_read_all(fname, load_buf);
    strcpy(D_11b2[slot], name);
    D_e0ab = 1;
    p = (int far *)load_buf;
    D_1182[slot] = p[0];
    D_118e[slot] = p[1];
    D_119a[slot] = D_1182[slot] + p[2];
    D_11a6[slot] = D_118e[slot] + p[3];
    D_c763[slot] = p[4];
    D_c799[slot] = p[5];
    D_c7d2[slot] = p[6];
    D_c803[slot] = p[7];
    D_c83f[slot] = p[8];
    D_c8ca[slot] = p[9];
    D_d5de[slot] = p[10];
    u = p[11];
    q = p + 13;
    if (p[12] != -1) {
        _fmemcpy(D_c681[slot], q, 10);
        _fmemcpy(D_c6f4[slot], q + 5, 10);
        q += 10;
    }
    D_d5d3[slot] = *q++;
    for (k = 0; k < 8; k++)
        D_d55d[slot][k] = *q++;
    for (k = 0; k < 8; k++)
        D_d505[slot][k] = (char)u - *q++;
    cbt_anim_len[i] = D_c908[slot] = *q++;
    if (name[5] != 0) {
        for (k = 0; k < 0x38; k++)
            if (D_1836[k] == *(int *)(name + 5)) {
                j = k;
                break;
            }
        _fmemcpy(D_a3e7[slot], F_20a5_4276[j].seq, 10);
        D_c8b8 = (unsigned char)F_20a5_4276[j].a;
        D_c8f5 = (unsigned char)F_20a5_4276[j].b;
        D_c913 = (unsigned char)F_20a5_4276[j].c;
        for (k = 0; k < 20; k++)
            D_a3e7[slot][10 + k] = *D_c943[k];
        D_c84a[slot] = F_20a5_4276[j].flags & 0x7f;
        D_c86b[slot] = F_20a5_4276[j].flags >> 7;
        D_c8a7[slot] = F_20a5_4276[j].e;
    }
    n = *q++;
    for (k = 0; k < D_c908[slot]; k++)
        for (m = 0; m < 8; m++)
            frames[k * 8 + m] = *q++ + mem;
    _fmemcpy((void far *)((unsigned long)mem << 16), q, n);
    if (slot < 5)
        sub_s014_1ec9_00da(combatants[i].kind, n - 4, mem);
}

         /* view offset per facing */

/* place the party around (x, y) facing dir (arrival through an exit/door): first
   in formation, then at random spots nearby that can see the point; after 500
   failures the 3x3 cells around the point are cleared once */
void far effect_start(int x, int y, int dir)
{
    int reset;
    int cy, cx;
    int i, k, c;
    int dx[5], dy[5];
    int ox, oy;
    int tries;

    if (x == -1 && y == -1)
        return;
    los_y0 = y;
    ox = (char)D_18a6[dir] / 4;
    oy = (char)D_18ae[dir] / 4;
    los_x0 = x;
    D_c866 = ox + (x >> 5);
    D_c886 = oy + (y >> 4);
    D_11b1 = 0;
    switch (dir) {
    case 0:
    case 1:
    case 7:
        dx[0] = -8; dx[1] = 8; dx[2] = -16; dx[3] = 16; dx[4] = 0;
        dy[0] = 0; dy[1] = 0; dy[2] = 8; dy[3] = 8; dy[4] = 8;
        break;
    case 2:
        dy[0] = 5; dy[1] = -5; dy[2] = 10; dy[3] = -10; dy[4] = 0;
        dx[0] = 0; dx[1] = 0; dx[2] = -16; dx[3] = -16; dx[4] = -16;
        break;
    case 3:
    case 4:
    case 5:
        dx[0] = -8; dx[1] = 8; dx[2] = -16; dx[3] = 16; dx[4] = 0;
        dy[0] = 0; dy[1] = 0; dy[2] = -8; dy[3] = -8; dy[4] = -8;
        break;
    case 6:
        dy[0] = 5; dy[1] = -5; dy[2] = 10; dy[3] = -10; dy[4] = 0;
        dx[0] = 0; dx[1] = 0; dx[2] = 16; dx[3] = 16; dx[4] = 16;
        break;
    }
    for (i = 0; i < 5; i++)
        if (combatants[i].status == 1) {
            combatants[i].x = 0;
            combatants[i].y = 0;
        }
    k = 0;
    for (i = 0; i < 5; i++) {
        c = marching_order[k];
        combatants[c].f1a = combatants[c].f19 = (char)dir;
        combatants[c].x = dx[i] + x;
        combatants[c].y = dy[i] + y;
        if (cbt_collisions(&combatants[c]) == 0)
            k++;
        if (marching_len <= k)
            break;
    }
    reset = 0;
    D_e0a2 = 1;
    for (; marching_len > k; k++) {
        c = marching_order[k];
again:
        tries = 0;
        do {
            combatants[c].x = _random_n(0x30) + x;
            combatants[c].y = _random_n(0x18) + y;
            if (cbt_collisions(&combatants[c]) == 0) {
                los_x1 = combatants[c].x;
                los_y1 = combatants[c].y;
                if (line_blocked(-1, 0, 0) == 0) {
                    combatants[c].f1a = combatants[c].f19 = (char)dir;
                    break;
                }
            }
            if (++tries > 500 && reset == 0) {
                cy = y >> 4;
                cx = x >> 5;
                F_20a5_0000[cx - 1][cy - 1][0] &= 0xf;
                F_20a5_0000[cx - 1][cy][0] &= 0xf;
                F_20a5_0000[cx - 1][cy + 1][0] &= 0xf;
                F_20a5_0000[cx][cy - 1][0] &= 0xf;
                F_20a5_0000[cx][cy][0] &= 0xf;
                F_20a5_0000[cx][cy + 1][0] &= 0xf;
                F_20a5_0000[cx + 1][cy - 1][0] &= 0xf;
                F_20a5_0000[cx + 1][cy][0] &= 0xf;
                F_20a5_0000[cx + 1][cy + 1][0] &= 0xf;
                reset = 1;
                goto again;
            }
        } while (tries <= 500);
    }
    D_e0a2 = 0;
}

extern int D_a691, D_a693;              /* cursors: orders / normal */
extern char opt_sound, opt_music;             /* sound options */
         /* effect area half sizes by size class */
extern char *D_21b2[], *D_21bc[];       /* effect area shapes */
extern char D_a5cc[];                   /* [13] side */
extern char D_e0ac[];                   /* [13] 1 = hostile */
int far distance3(int, int, int);  /* distance */
void far sound_play(int);       /* play a sound */
void far sub_s014_19cd_1d5e(int);       /* remove an activation record */
void far sub_s014_1db7_0a80(int, int);
#define CB(i, o) (((char *)&combatants[i])[o])

/* activation records (hidden encounters): when a party member can see one or comes
   within its radius, it is started (enemies appear) and removed.  Returns what the
   start returned. */
int far activations_check(void)
{
    int k, i;
    int next;
    int res;

    res = 0;
    if (floor_shown == party_floor && D_d6f0 == 1 && ai_paused == 0)
        for (k = 0; k < activation_count; ) {
            next = 1;
            if ((los_x0 = activations.a[k].f0) >= 0) {
                los_y0 = activations.a[k].f2;
                if ((*(unsigned far *)&F_20a5_0000[los_x0 >> 5][los_y0 >> 4][2] & 0x8000) == 0) {
                    for (i = 0; i < 5; i++)
                        if (combatants[i].status == 1) {
                            los_x1 = combatants[i].x;
                            los_y1 = combatants[i].y;
                            if (line_blocked(-1, 0, 0) == 0)
                                goto found;
                        }
                    if (activation_count != 0)
                        for (i = 0; i < 5; i++)
                            if (combatants[i].status == 1
                             && distance3((activations.a[k].f0 - combatants[i].x) / 2,
                                    activations.a[k].f2 - combatants[i].y, 0)
                                    < CB(i, 0x7b) + CB(i, 0x7a))
                                goto found;
                }
            }
            goto skip;
found:
            res = enemies_place_vec(0, (struct atv far *)&activations.a[k], i);
            sub_s014_19cd_1d5e(k);
            next = 0;
skip:
            if (next == 1)
                k++;
        }
    D_da4f = combat_clock + 0x78;
    return res;
}

/* out of battle: party members slowly recover endurance up to 3/4 of the maximum */
#pragma optimize("a", on)
#pragma optimize("g", off)
void far endurance_recover(void)
{
    int max, e, i;
    union { char st; int d; } s;

    for (i = 0; i < 5; i++) {
        s.st = combatants[i].status;
        if ((s.st == 1 || s.st == 2) && D_da55[i] < combat_clock) {
            max = combatants[i].maxend;
            e = combatants[i].endur;
            if ((max >> 2) + (max >> 1) <= e)
                D_da55[i] = combat_clock + 2700;
            else {
                combatants[i].endur++;
                if (max >> 3 > e)
                    D_da55[i] = combat_clock + 300;
                else if (max >> 2 > e)
                    D_da55[i] = combat_clock + 900;
                else if (max >> 1 > e)
                    D_da55[i] = combat_clock + 1800;
                else
                    D_da55[i] = combat_clock + 2700;
                party_panel_rows(i);
            }
        }
    }
}
#pragma optimize("", on)

/* play sound n if the sound options allow it */
#pragma optimize("egl", off)
void far sub_s014_0e76_465e(register int n)
{
    if ((opt_sound == 1 && n >= 10) || n == 0x12 || (opt_music == 1 && n >= 7 && n <= 9))
        sound_play(n);
}
#pragma optimize("egl", on)

void far sub_s014_0e76_4694(void)
{
    sub_s014_0e76_465e(0x12);
}

/* is combatant i inside the effect area e {x, y, size << 4} and in sight of its centre */
#pragma optimize("e", off)
int far sub_s014_0e76_469e(int *e, int i)
{
    unsigned s;
    int w, h;
    int lx, ly, hx, hy;
    int r, c;

    s = ((unsigned char *)e)[4] >> 4;
    w = D_18b6[s];
    lx = -(w - e[0]);
    h = D_18bc[s];
    ly = -(h - e[1]);
    hx = w + e[0];
    hy = h + e[1];
    los_x0 = e[0];
    los_y0 = e[1];
    if (lx < combatants[i].x && combatants[i].x < hx && ly < combatants[i].y && combatants[i].y < hy) {
        r = ((combatants[i].y - ly) * 4 >> 5) * s;
        c = (combatants[i].x - lx) * 4 >> 6;
        los_x1 = D_21b2[s][r + c] + e[0];
        los_y1 = D_21bc[s][r + c] + e[1];
        if (line_blocked(-1, -1) == 0)
            return 1;
    }
    return 0;
}
#pragma optimize("e", on)

/* restore the normal mouse cursor */
void far cursor_normal(void)
{
    mouse_hide();
    if (ai_paused == 0)
        mouse_set_cursor(0, 0, D_a693);
    else
        mouse_set_cursor(0, 0, D_a691);
    mouse_show();
    cursor_current = cursor_default;
}

/* are combatants i and j on the same side and (at least one) hostile */
int far cbt_hostile_pair(int i, int j)
{
    int r;

    r = 0;
    if (i == 5 && demo_mode != 0)
        return 1;
    if (j == -1)
        return 0;
    if (cbt_floor[j] != cbt_floor[i])
        return 0;
    sub_s014_1db7_0a80(i, 4);
    if (D_e0ac[j] == 1)
        r = 1;
    else {
        sub_s014_1db7_0a80(j, 4);
        if (D_e0ac[i] == 1)
            r = 1;
    }
    return r;
}
