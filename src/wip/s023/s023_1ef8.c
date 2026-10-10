/* place: s023 1ef8:0000 */
/* orig: size=1acc CONST=7df4+000e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 3/16 (0d1a, 0d88, 153e: if/else with `ok = 1` and one `return ok`, m4).
 * m4: REVEAL indexes the cell as c[(unsigned)u.w >> 8][u.w & 0xff] (orig: mul byte hi*0xa0, lo
 * and 0ffh) and 0000's u is a 4-byte union (frame 4): 0000 is down to one jl-vs-jge/jmp jump size,
 * 0b6e to the far-address CSE (see e8), 04ac to slot order (u -0e, t -0c, n*14 temp -0a),
 * 13d0 to 6.00's word ops on the cell byte ('and ax,0F0h' - 6.00A narrows); the loot buffer is read
 * as one far pointer (LOOTBUF over D_c654/D_c656) and the cell indexed without a pointer local, ALL 16 functions written (functional drafts). 03ca/0734: the original
 * repeats `leave; ret` after every case (6.00A jumps to one epilogue). DATA MATCHES (e8, stamped --wip):
 * the module owns 7 CONST slots 7df4..7e00 (7e02.. are s024's): 2e38:0bb6, the floor struct
 * F_20a5_0000, F_20a5_000 (cell-list reads), 76e7, 76a8, F_2e38_03e, 47ee. Every floor access goes
 * through F_20a5_0000 (6.00A gives a far name a slot when one access in a function is not CSE'd).
 * Combat dungeon features (overlay s023, child of s014): doors, locks, secret doors and traps
 * of the current level's floor image (20a5:0000, level%d.flr):
 * 0000 open a wall group (handler h, wall list w): reveal its walls, toggle handler h,
 * 00b8 open/close door n for combatant who (door walls, both door cells, linked door, then
 *      wake the enemies of an activation spot behind it),
 * 03ca/0734 set or clear the door bit (8) of the wall on side dir of cell (x, y),
 * 04ac open/close door n (state 1/0), optionally revealing its wall group,
 * 0812 skill check that finds secret door n (record low byte = door, high byte = 2nd handler),
 * 0aea open a secret passage (clear the wall nibbles of two cells),
 * 0b6e reveal walls + remove the door walls of secret door record n,
 * 0d1a clear the wall on side dir of cell (x, y),
 * 0d88 spring a spell/gas trap (vec_1dc9), 0e30 spring a missile trap (dart volleys from a
 *      wall line, hits rolled against armour), 12c0 pick a lock (needs lock picks, item 0x40),
 * 13d0 open chest of combatant who (trap-door cell art, item transfer through the list UI),
 * 153e disarm a chest trap, 1618 change level through stairs/exit record n,
 * 17d2 leave the level: all party members gone -> end of combat, else enemies that have to
 *      stay behind re-form an activation record at the exit.
 * Far data (20a5 = level floor image):
 *   +0000 cells 40 x 40 x 4 bytes (byte 0..3 = walls N/E/S/W? low nibble = wall type, bit 3 =
 *         door closed, byte 3 bit 7 = hidden wall),
 *   +1f5a door records (14 bytes), +2184 trap/handler records (14 bytes, +2 int on/off sign),
 *   +248a wall group lists (offsets of -1 terminated cell lists), +2fb8 stairs/exit records
 *   (14 bytes), +3014 chest records (0x2c bytes), +316e missile trap records (10 bytes),
 *   +3350 secret door records (10 bytes), +33d3[40] door open flags.
 *   2e38:0bb6 long per level: wall groups already revealed (bit n = group n). */
#include "darklands/card.h"
#include "darklands/game.h"

struct cell { unsigned char b0, b1, b2, b3; };
struct door {               /* 14 bytes at 20a5:1f5a */
    char wall;              /* +0 wall group to reveal (-1 none) */
    char state;             /* +1 */
    unsigned char x, y;     /* +2 cell 1 (bits 7/6: side flags) */
    unsigned char x2, y2;   /* +4 cell 2 (0 = none) */
    char link;              /* +6 linked door (-1 none) */
    char pad[7];
};
struct trap { char pad0[2]; int on; char pad4[10]; };     /* 14 bytes at 20a5:2184 */
struct stair { char pad0[6]; char lvl; char wall; char pad8[5]; char f0d; };   /* 14, 2fb8 */
struct chest {              /* 0x2c bytes at 20a5:3014 */
    unsigned char x, y;     /* +0 */
    char f2;
    unsigned char lock;     /* +3 lock / trap difficulty */
    unsigned char trap;     /* +4 trap: low nibble kind, high nibble level */
    char n;                 /* +5 item count */
    unsigned char idx;      /* +6 inventory handle index */
    char f7;
    int it[3];              /* +8 */
    char pad[0x2c - 0x0e];
};
struct mtrap {              /* 10 bytes at 20a5:316e */
    int link;               /* +0 handler of the twin trap */
    union { int x; struct { unsigned char a, b; } c; } u;   /* +2 */
    int y;                  /* +4 */
    char cnt;               /* +6 darts per volley */
    unsigned char dir;      /* +7 0/2/4/6 */
    unsigned char wpn;      /* +8 */
    unsigned char skill;    /* +9 */
};
struct sdoor { char f0; unsigned char x, y, x2, y2; char pad[5]; };   /* 10, 3350 furniture removal */
struct wrec { char f0; unsigned char x, y; char wall; char pad[6]; };  /* 10, 3382 wall removal */

struct flr {
    struct cell c[40][40];              /* +0000 */
    char pad1900[0x1f5a - 0x1900];
    struct door d[39];                  /* +1f5a */
    char pad[0x2184 - 0x1f5a - 39 * 14];
    struct trap t[50];                  /* +2184 */
    char pad2[0x248a - 0x2184 - 50 * 14];
    int list[64];                       /* +248a */
    char pad3[0x2fb8 - 0x250a];
    struct stair s[6];                  /* +2fb8 */
    char pad4[0x3014 - 0x2fb8 - 6 * 14];
    struct chest ch[7];                 /* +3014 */
    char pad5[0x316e - 0x3014 - 7 * 0x2c];
    struct mtrap m[40];                 /* +316e */
    char pad6[0x3350 - 0x316e - 40 * 10];
    struct sdoor sd[5];                 /* +3350 */
    struct wrec wr[7];                  /* +3382 */
    char pad7[0x33d3 - 0x3382 - 7 * 10];
    char open[40];                      /* +33d3 */
};

struct cmb {
    char pad0[4];
    int x, y;               /* +04 */
    char pad8[8];
    int f10;                /* +10 state */
    char pad12[2];
    char status;            /* +14 */
    char pad15;
    char kind;              /* +16 */
    char pad17[4];
    char f1b;               /* +1b */
    char pad1c[0x22];
    char name[0x0f];        /* +3e */
    char f4d;               /* +4d armour */
    char f4e;
    char f4f;               /* +4f */
    char pad50[0x0d];
    char f5d;               /* +5d */
    char f5e;
    char f5f;               /* +5f dodge */
    char f60;               /* +60 */
    char pad61[3];
    char f64;               /* +64 */
    char pad65[0x14];
    char f79;               /* +79 lock / trap skill */
    char pad7a[4];
    char f7e;               /* +7e item count */
    char pad7f;
};
struct item { unsigned id; unsigned char f2; unsigned char f3; unsigned char f4; char f5; };
struct atv {                /* activation record, 0x1e bytes at 20a5:47ee */
    int x, y, sprd;
    char rm, reenf;
    char type[3], num[3];
    char fgp[6];            /* +0e F, G, Pf */
    char pad[0x1e - 0x14];
};
struct six { char b[6]; };
struct wtab { char v[63]; };

extern struct flr far floor_img;
extern struct { long m[16]; } far doors_opened;
extern struct { int w[1]; } far F_20a5_000;
extern struct cell far F_20a5_0[][40];
extern int far F_20a5_248a[];
extern int far F_20a5_00[];
extern struct trap far F_20a5_2184[];
extern char far floor_doors[];
extern struct sdoor far F_20a5_3350[];
extern struct mtrap far F_20a5_316e[];
extern struct chest far F_20a5_3014[];
extern struct stair far F_20a5_2fb8[];
extern struct wtab far F_20a5_76e7;     /* weapon damage */
extern struct wtab far F_20a5_76a8;     /* weapon class | penetration */
extern struct item far inventories[][64];
extern struct { struct item it[5][64]; } far F_2e38_03e;
extern struct { struct atv r[30]; } far activations;
extern char far F_2e38_0bb0[];
extern char far F_20a5_47f4[];

extern struct cmb combatants[];
extern char floor_shown, floor_dirty, D_d6f0, combat_alarm, D_e1ca, D_07d3, real_combat, party_floor, D_d842, D_d6eb;
extern unsigned char activation_count;
extern unsigned D_d68d[];
extern char D_c678[];       /* door/lock action: 1 = open */
extern unsigned char D_c6ef[];  /* facing per combatant */
extern unsigned char D_c784[];  /* chest index per combatant */
extern int D_c7a4[];
extern int los_y0, los_x0, los_y1, los_x1;
extern int combat_clock, D_c2bb, D_c19b;
extern int D_c67f, ems_frame_handle;
extern unsigned D_c654, D_c656;
#define LOOTBUF (*(char far * *)&D_c654)
extern char *loot_list;
extern char D_17b0;
extern char cbt_half_w[], cbt_half_h[], D_119a[], D_11a6[], cbt_floor[], D_c8b2[];
extern char D_23e5[][3], D_23ef[][3];
extern char D_c1f3[];
extern int D_c23e[];
extern int D_c28d[];
extern char D_c3cd[];
extern char D_04c8[], D_04f8[], D_0518[], D_0540[], D_055a[], D_0582[], D_0644[];

int far random_n(int);
int far rand_1_n(int);
void far show_message(char *, int);
int far distance3(int, int, int);
int far sub_root_0410_001a(int, int, int);
void far ems_map_page(int, int, int);
void far *_fmemcpy(void far *, const void far *, unsigned);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int far line_blocked(int, int, int);
void far damage_queue(int, int, int, int);
void far cbt_forget_target(int);
void far sub_s014_19cd_3258(char *, int *, int, int);
void far loot_screen(unsigned, unsigned, int *, char far *, int);
int far sub_s014_19cd_3204(char *, int);
void far sub_s014_19cd_1d5e(int);
void far loot_after_fight(void);
void far combat_draw(void);
void far floor_view_draw_vec(int);
void far level_switch_vec(int);
void far enemy_anim_free5_vec(void);
void far place_around_vec(int, char *);
int far enemies_place_vec(int, struct atv far *, int);
void far effect_spell_vec(int, int, int, int, int, int, int);
void far missile_launch_vec(int, int, int, int, int);

void far sub_s023_1ef8_0000(int, int, int);
void far door_toggle(int, int);
void near sub_s023_1ef8_03ca(int, int, int, int);
void far door_set(int, int, int);
void near sub_s023_1ef8_0734(int, int, int, int);
void far secret_door_search(int, int, unsigned, int);
void far sub_s023_1ef8_0aea(int, int);
void far sub_s023_1ef8_0b6e(int, int);
void near sub_s023_1ef8_0d1a(int, int, int);
void far trap_spell(int, int, int);
void far trap_missile(int, int, int, int);
int far lock_pick(unsigned char far *);
void far chest_open(int);
int far chest_disarm(int);
void far take_exit(int, int);
void far leave_level(int, char *);

/* reveal wall group n of the current level: remember it in the level mask and clear the
 * hidden bit of every cell in its list (inline in the original, one copy per caller) */
#define REVEAL(n, u, j) \
    doors_opened.m[floor_shown] |= 1L << (n); \
    u.w = *(int far *)((char far *)&F_20a5_000 + floor_img.list[n]); \
    if (u.w != -1) { \
        j = 0; \
        do { \
            floor_img.c[(unsigned)u.w >> 8][u.w & 0xff].b3 &= 0x7f; \
            j += 2; \
            u.w = *(int far *)((char far *)&F_20a5_000 + floor_img.list[n] + j); \
        } while (u.w != -1); \
    }

union cw { int w; struct { unsigned char lo, hi; } b; };

void far sub_s023_1ef8_0000(int who, int h, int n)
{
    union { int w; long l; } u;
    int j;

    floor_dirty = 1;
    if (n >= 0) {
        REVEAL(n, u, j)
    }
    floor_img.t[h].on = -floor_img.t[h].on;
    floor_view_draw_vec(-1);
}

void far door_toggle(int n, int who)
{
    union cw u;
    int j, x, y, w, k;

    floor_dirty = 1;
    x = 0;
    w = floor_img.d[n].wall;
    if (w >= 0) {
        REVEAL(w, u, j)
    }
    if (D_c678[who] == 1)
        floor_img.open[n] = 1;
    else
        floor_img.open[n] = 0;
    floor_img.d[n].state = 0;
    if (floor_img.d[n].link != -1) {
        floor_img.d[floor_img.d[n].link].state = 0;
        if (D_c678[who] == 1)
            floor_img.open[floor_img.d[n].link] = 1;
        else
            floor_img.open[floor_img.d[n].link] = 0;
    }
    x = floor_img.d[n].x & 0x3f;
    y = floor_img.d[n].y & 0x3f;
    switch (D_c6ef[who]) {
    case 0:
        sub_s023_1ef8_03ca(who, x, y, 0);
        sub_s023_1ef8_03ca(who, x, y - 1, 2);
        break;
    case 1:
        sub_s023_1ef8_03ca(who, x, y, 1);
        sub_s023_1ef8_03ca(who, x + 1, y, 3);
        break;
    case 2:
        sub_s023_1ef8_03ca(who, x, y, 2);
        sub_s023_1ef8_03ca(who, x, y + 1, 0);
        break;
    case 3:
        sub_s023_1ef8_03ca(who, x, y, 3);
        sub_s023_1ef8_03ca(who, x - 1, y, 1);
        break;
    }
    x = floor_img.d[n].x2 & 0x3f;
    if (x) {
        y = floor_img.d[n].y2 & 0x3f;
        switch (D_c6ef[who]) {
        case 0:
            sub_s023_1ef8_03ca(who, x, y, 0);
            sub_s023_1ef8_03ca(who, x, y - 1, 2);
            break;
        case 1:
            sub_s023_1ef8_03ca(who, x, y, 1);
            sub_s023_1ef8_03ca(who, x + 1, y, 3);
            break;
        case 2:
            sub_s023_1ef8_03ca(who, x, y, 2);
            sub_s023_1ef8_03ca(who, x, y + 1, 0);
            break;
        case 3:
            sub_s023_1ef8_03ca(who, x, y, 3);
            sub_s023_1ef8_03ca(who, x - 1, y, 1);
            break;
        }
    }
    /* wake the activation spot that waits behind this wall group */
    if (w != -1 && D_d6f0 == 1)
        for (k = 0; k < activation_count; k++)
            if (activations.r[k].rm == w && activations.r[k].sprd > 0) {
                if (enemies_place_vec(0, &activations.r[k], who) == 1) {
                    combat_alarm = 1;
                    D_c19b = random_n(300) + combat_clock + 300;
                }
                sub_s014_19cd_1d5e(k);
                break;
            }
}

/* door bit of the wall on side dir of cell (x, y): set when who opens, else clear */
void near sub_s023_1ef8_03ca(int who, int x, int y, int dir)
{
    if (D_c678[who] == 1)
        switch (dir) {
        case 0: floor_img.c[x][y].b2 |= 8; return;
        case 1: floor_img.c[x][y].b0 |= 8; return;
        case 2: floor_img.c[x][y].b3 |= 8; return;
        case 3: floor_img.c[x][y].b1 |= 8; return;
        }
    else
        switch (dir) {
        case 0: floor_img.c[x][y].b2 &= ~8; return;
        case 1: floor_img.c[x][y].b0 &= ~8; return;
        case 2: floor_img.c[x][y].b3 &= ~8; return;
        case 3: floor_img.c[x][y].b1 &= ~8; return;
        }
}

void far door_set(int n, int open, int reveal)
{
    union cw u;
    int j, x, y, w, dir;
    unsigned char t;

    if ((floor_img.d[n].x & 0x80) == 0x80)
        dir = 0;
    else if ((floor_img.d[n].x & 0x40) == 0x40)
        dir = 1;
    else if (((t = floor_img.d[n].y) & 0x80) == 0x80)
        dir = 2;
    else if ((t & 0x40) == 0x40)
        dir = 3;
    if (open == 1)
        floor_img.open[n] = 1;
    else
        floor_img.open[n] = 0;
    if (reveal == 1) {
        floor_dirty = 1;
        w = floor_img.d[n].wall;
        if (w >= 0) {
            REVEAL(w, u, j)
        }
    }
    if (floor_img.d[n].link != -1) {
        if (open == 1)
            floor_img.open[floor_img.d[n].link] = 1;
        else
            floor_img.open[floor_img.d[n].link] = 0;
    }
    x = floor_img.d[n].x & 0x3f;
    y = floor_img.d[n].y & 0x3f;
    switch (dir) {
    case 0:
        sub_s023_1ef8_0734(x, y, 0, open);
        sub_s023_1ef8_0734(x, y - 1, 2, open);
        break;
    case 1:
        sub_s023_1ef8_0734(x, y, 1, open);
        sub_s023_1ef8_0734(x + 1, y, 3, open);
        break;
    case 2:
        sub_s023_1ef8_0734(x, y, 2, open);
        sub_s023_1ef8_0734(x, y + 1, 0, open);
        break;
    case 3:
        sub_s023_1ef8_0734(x, y, 3, open);
        sub_s023_1ef8_0734(x - 1, y, 1, open);
        break;
    }
    x = floor_img.d[n].x2 & 0x3f;
    if (x) {
        y = floor_img.d[n].y2 & 0x3f;
        switch (dir) {
        case 0:
            sub_s023_1ef8_0734(x, y, 0, open);
            sub_s023_1ef8_0734(x, y - 1, 2, open);
            break;
        case 1:
            sub_s023_1ef8_0734(x, y, 1, open);
            sub_s023_1ef8_0734(x + 1, y, 3, open);
            break;
        case 2:
            sub_s023_1ef8_0734(x, y, 2, open);
            sub_s023_1ef8_0734(x, y + 1, 0, open);
            break;
        case 3:
            sub_s023_1ef8_0734(x, y, 3, open);
            sub_s023_1ef8_0734(x - 1, y, 1, open);
            break;
        }
    }
}

void near sub_s023_1ef8_0734(int x, int y, int dir, int open)
{
    if (open == 1)
        switch (dir) {
        case 0: floor_img.c[x][y].b2 |= 8; return;
        case 1: floor_img.c[x][y].b0 |= 8; return;
        case 2: floor_img.c[x][y].b3 |= 8; return;
        case 3: floor_img.c[x][y].b1 |= 8; return;
        }
    else
        switch (dir) {
        case 0: floor_img.c[x][y].b2 &= ~8; return;
        case 1: floor_img.c[x][y].b0 &= ~8; return;
        case 2: floor_img.c[x][y].b3 &= ~8; return;
        case 3: floor_img.c[x][y].b1 &= ~8; return;
        }
}

/* who searches: when skill check `roll` beats the record, the secret door is found */
void far secret_door_search(int who, int h, unsigned rec, int roll)
{
    char msg[60];
    union cw u;
    int j, w, n, h2, x, y;

    n = rec & 0xff;
    h2 = rec >> 8;
    if (combatants[who].f60 > roll) {
        floor_img.t[h].on = -floor_img.t[h].on;
        if (h2 != 0xff)
            floor_img.t[h2].on = -floor_img.t[h2].on;
        if ((floor_img.d[n].x & 0x80) == 0x80)
            D_c6ef[who] = 0;
        else if ((floor_img.d[n].x & 0x40) == 0x40)
            D_c6ef[who] = 1;
        else if ((floor_img.d[n].y & 0x80) == 0x80)
            D_c6ef[who] = 2;
        else if ((floor_img.d[n].y & 0x40) == 0x40)
            D_c6ef[who] = 3;
        x = floor_img.d[n].x & 0x3f;
        y = floor_img.d[n].y & 0x3f;
        switch (D_c6ef[who]) {
        case 0:
            floor_img.c[x][y].b2 &= 0xf0;
            floor_img.c[x][y - 1].b3 &= 0xf0;
            break;
        case 1:
            floor_img.c[x][y].b0 &= 0xf0;
            floor_img.c[x + 1][y].b1 &= 0xf0;
            break;
        case 2:
            floor_img.c[x][y].b3 &= 0xf0;
            floor_img.c[x][y + 1].b2 &= 0xf0;
            break;
        case 3:
            floor_img.c[x][y].b1 &= 0xf0;
            floor_img.c[x - 1][y].b0 &= 0xf0;
            break;
        }
        strcpy(msg, combatants[who].name);
        strcat(msg, D_055a);
        show_message(msg, 150);
        floor_dirty = 1;
        x = 0;
        w = floor_img.d[n].wall;
        if (w >= 0) {
            REVEAL(w, u, j)
        }
        floor_view_draw_vec(-1);
    }
}

void far sub_s023_1ef8_0aea(int n, int h)
{
    floor_img.t[h].on = -floor_img.t[h].on;
    floor_dirty = 1;
    if (floor_img.sd[n].x)
        floor_img.c[floor_img.sd[n].x][floor_img.sd[n].y].b0 &= 0x0f;
    if (floor_img.sd[n].x2)
        floor_img.c[floor_img.sd[n].x2][floor_img.sd[n].y2].b0 &= 0x0f;
}

void far sub_s023_1ef8_0b6e(int n, int h)
{
    union cw u;
    int j, x, y, dir;
    struct wrec far *q;

    floor_img.t[h].on = -floor_img.t[h].on;
    floor_dirty = 1;
    dir = 0;
    q = &floor_img.wr[n];
    if (q->wall > 0) {
        REVEAL(q->wall, u, j)
    }
    if ((q->x & 0x80) == 0x80)
        dir = 0;
    else if ((q->x & 0x40) == 0x40)
        dir = 1;
    else if ((q->y & 0x80) == 0x80)
        dir = 2;
    else if ((q->y & 0x40) == 0x40)
        dir = 3;
    x = q->x & 0x3f;
    y = q->y & 0x3f;
    switch (dir) {
    case 0:
        sub_s023_1ef8_0d1a(x, y, 0);
        sub_s023_1ef8_0d1a(x, y - 1, 2);
        break;
    case 1:
        sub_s023_1ef8_0d1a(x, y, 1);
        sub_s023_1ef8_0d1a(x + 1, y, 3);
        break;
    case 2:
        sub_s023_1ef8_0d1a(x, y, 2);
        sub_s023_1ef8_0d1a(x, y + 1, 0);
        break;
    case 3:
        sub_s023_1ef8_0d1a(x, y, 3);
        sub_s023_1ef8_0d1a(x - 1, y, 1);
        break;
    }
}

void near sub_s023_1ef8_0d1a(int x, int y, int dir)
{
    switch (dir) {
    case 0: floor_img.c[x][y].b2 = 0; break;
    case 1: floor_img.c[x][y].b0 &= 0xf0; break;
    case 2: floor_img.c[x][y].b3 &= 0xf0; break;
    case 3: floor_img.c[x][y].b1 = 0; break;
    }
}

/* spell trap t fires at who */
void far trap_spell(int who, int h, int t)
{
    show_message(D_0644, 60);
    effect_spell_vec(combatants[who].x, combatants[who].y, who, floor_img.m[t].u.c.a,
             floor_img.m[t].u.c.b, who, floor_shown);
    floor_img.t[h].on = -floor_img.t[h].on;
    if (floor_img.t[floor_img.m[t].link].on >= 0)
        floor_img.t[floor_img.m[t].link].on = -floor_img.t[floor_img.m[t].link].on;
}

/* missile trap t (and its twin t2, 0xff none) fires its darts along the wall line */
void far trap_missile(int who, int h, int t, int t2)
{
    int sv1[5], sv2[5];
    int i, k, cur, dir, cnt, x, y, cx, cy, tgt, dmg, cls, arm, d;

    show_message(D_0644, 60);
    floor_img.t[h].on = -floor_img.t[h].on;
    cur = t;
    if (floor_img.t[floor_img.m[cur].link].on >= 0)
        floor_img.t[floor_img.m[cur].link].on = -floor_img.t[floor_img.m[cur].link].on;
    for (i = 0; i < 5; i++) {
        sv1[i] = cbt_half_w[i];
        sv2[i] = cbt_half_h[i];
        cbt_half_w[i] = D_119a[i];
        cbt_half_h[i] = D_11a6[i];
    }
again:
    dir = floor_img.m[cur].dir;
    cnt = floor_img.m[cur].cnt;
    x = floor_img.m[cur].u.x;
    cx = x >> 5;
    y = floor_img.m[cur].y;
    cy = y >> 4;
    for (i = 0; i < cnt; i++) {
        switch (dir) {
        case 0:
            los_x0 = D_23e5[cnt][i] + x;
            los_y0 = y - 3;
            for (k = 0; k < 16; k++)
                if (floor_img.c[cx][cy - k].b2 & 0x0f) {
                    los_x1 = los_x0;
                    los_y1 = ((cy - k) << 4) + 4;
                    break;
                }
            if (k == 16) {
                los_x1 = los_x0;
                los_y1 = (cy << 4) - 0xec;
            }
            break;
        case 2:
            los_x0 = x + 3;
            los_y0 = D_23ef[cnt][i] + y;
            for (k = 0; k < 16; k++)
                if (floor_img.c[cx + k][cy].b0 & 0x0f) {
                    los_x1 = ((cx + k) << 5) + 0x1a;
                    los_y1 = los_y0;
                    break;
                }
            if (k == 16) {
                los_x1 = (cx << 5) + 0x1fa;
                los_y1 = los_y0;
            }
            break;
        case 4:
            los_x0 = D_23e5[cnt][i] + x;
            los_y0 = y + 3;
            for (k = 0; k < 16; k++)
                if (*(unsigned far *)&floor_img.c[cx][cy + k].b2 & 0xf00) {
                    los_x1 = los_x0;
                    los_y1 = ((cy + k) << 4) + 0x0b;
                    break;
                }
            if (k == 16) {
                los_x1 = los_x0;
                los_y1 = (cy << 4) + 0xfb;
            }
            break;
        case 6:
            los_x0 = x - 3;
            los_y0 = D_23ef[cnt][i] + y;
            for (k = 0; k < 16; k++)
                if (floor_img.c[cx - k][cy].b1 & 0x0f) {
                    los_x1 = ((cx - k) << 5) + 6;
                    los_y1 = los_y0;
                    break;
                }
            if (k == 16) {
                los_x1 = (cx << 5) - 0x1da;
                los_y1 = los_y0;
            }
            break;
        }
        tgt = line_blocked(-2, -2, 0);
        if (tgt < 50)
            missile_launch_vec(-2, -1, 0, dir, floor_img.m[cur].wpn);
        else {
            tgt -= 50;
            if (combatants[tgt].f5f >= random_n(100))
                missile_launch_vec(-2, tgt, 0, dir, floor_img.m[cur].wpn);
            else {
                dmg = random_n(6) + F_20a5_76e7.v[floor_img.m[cur].wpn] - 2;
                cls = F_20a5_76a8.v[floor_img.m[cur].wpn] & 0x0f;
                arm = combatants[tgt].f4d;
                if (cls > arm)
                    d = dmg;
                else if (cls == arm)
                    d = dmg / 2;
                else
                    d = dmg / 8;
                d += (floor_img.m[cur].skill - combatants[tgt].f4f) / 10;
                damage_queue(d, tgt, cls, arm);
                missile_launch_vec(-1, tgt, 0, dir, floor_img.m[cur].wpn);
            }
        }
    }
    if (t == cur && t2 != 0xff) {
        cur = t2;
        goto again;
    }
    for (i = 0; i < 5; i++) {
        cbt_half_w[i] = sv1[i];
        cbt_half_h[i] = sv2[i];
    }
}

/* the selected member (D_e1ca) picks a lock: needs lock picks; returns 1 when it opens */
int far lock_pick(unsigned char far *lock)
{
    int k, found, ok, q, s;

    k = 0;
    found = 0;
    ok = 0;
    if (combatants[D_e1ca].f7e > 0) {
        for (; inventories[D_e1ca][k].id != 0x40; )
            if (++k >= combatants[D_e1ca].f7e)
                goto none;
        found = 1;
    }
none:
    q = F_2e38_03e.it[D_e1ca][k].f3;
    if (found == 0)
        show_message(D_04c8, 150);
    else {
        s = combatants[D_e1ca].f79 + q - 25;
        if ((*lock & 0x7f) < rand_1_n(10) + s) {
            show_message(D_04f8, 150);
            ok = 1;
            *lock = 0;
            D_d68d[D_e1ca] |= 0x20;
        } else {
            if ((*lock & 0x7f) < s + 10)
                show_message(D_0540, 150);
            else
                show_message(D_0518, 150);
            D_d68d[D_e1ca] |= 0x40;
        }
    }
    return ok;
}

/* who opens his chest: let the party take the contents */
void far chest_open(int who)
{
    char n;
    int h;

    D_07d3++;
    if ((floor_img.c[floor_img.ch[D_c784[who]].x][floor_img.ch[D_c784[who]].y].b0 & 0xf0) == 0x50)
        *(long far *)&floor_img.c[floor_img.ch[D_c784[who]].x][floor_img.ch[D_c784[who]].y] += 0x10;
    h = D_c7a4[floor_img.ch[D_c784[who]].idx];
    n = floor_img.ch[D_c784[who]].n;
    sub_s014_19cd_3258(&n, &h, 1, 0);
    loot_screen(D_c654, D_c656, &D_c67f, (char far *)floor_img.ch[D_c784[who]].it, who);
    floor_dirty = 1;
    floor_img.ch[D_c784[who]].it[0] = 0;
    floor_img.ch[D_c784[who]].it[1] = 0;
    floor_img.ch[D_c784[who]].it[2] = 0;
    floor_img.ch[D_c784[who]].n = D_c67f;
    if (D_c67f != 0) {
        ems_map_page(3, 10, ems_frame_handle);
        _fmemcpy((char far *)loot_list, LOOTBUF, D_c67f * 6);
        ems_map_page(3, 3, ems_frame_handle);
        D_c7a4[floor_img.ch[D_c784[who]].idx] = sub_s014_19cd_3204(loot_list, D_c67f);
    }
    floor_view_draw_vec(D_17b0);
    combat_draw();
    D_07d3--;
}

/* who disarms the trap of his chest; on failure it goes off */
int far chest_disarm(int who)
{
    int s, r, ok;

    ok = 0;
    s = combatants[who].f79 - 10;
    r = rand_1_n(20);
    if (floor_img.ch[D_c784[who]].lock < s + r) {
        show_message(D_0582, 150);
        ok = 1;
        D_d68d[who] |= 0x20;
        floor_img.ch[D_c784[who]].lock = 0;
    } else {
        show_message(D_0644, 60);
        D_d68d[who] |= 0x40;
        effect_spell_vec(combatants[who].x, combatants[who].y, who, floor_img.ch[D_c784[who]].trap & 0x0f,
             (floor_img.ch[D_c784[who]].trap >> 4) * 10, who, floor_shown);
    }
    return ok;
}

/* who takes stairs/exit n: move to the other level */
void far take_exit(int n, int who)
{
    struct stair s;
    int j, wall, lvl, old, alone, i;
    struct cmb *p;
    unsigned char far *c;

    old = floor_shown;
    s = floor_img.s[n];
    wall = floor_img.s[n].wall;
    floor_img.s[n].wall = -1;
    lvl = floor_img.s[n].lvl;
    cbt_floor[who] = lvl;
    alone = 1;
    for (i = 0, p = combatants; p < combatants + 5; p++, i++)
        if (p->status == 1 && cbt_floor[i] != lvl)
            alone = 0;
    if (alone == 1) {
        party_floor = lvl;
        leave_level(who, (char *)&s);
        real_combat = 0;
    }
    level_switch_vec(lvl);
    if (wall != -1) {
        doors_opened.m[lvl] |= 1L << wall;
        j = 0;
        if (*(int far *)((char far *)&F_20a5_000 + floor_img.list[wall]) != -1)
            do {
                c = (unsigned char far *)&F_20a5_000 + floor_img.list[wall] + j;
                floor_img.c[c[1]][c[0]].b3 &= 0x7f;
                j += 2;
            } while (*(int far *)((char far *)&F_20a5_000 + floor_img.list[wall] + j) != -1);
        floor_dirty = 1;
    }
    place_around_vec(who, (char *)&s);
    if (party_floor == lvl)
        floor_view_draw_vec(who);
    else {
        level_switch_vec(old);
        floor_view_draw_vec(-1);
    }
    combatants[who].f10 = 0;
    cbt_forget_target(who);
    D_c8b2[who] = floor_img.s[n].f0d;
}

/* the party left the level through exit e (14-byte stairs record): if enemy groups that saw
 * the party follow (D_c23e[kind] bit per level) they pursue after a delay; otherwise the
 * weakest enemies are dropped and the rest re-form an activation record at the exit */
void far leave_level(int who, char *e)
{
    char seen[7];
    int typ[7], num[7];
    int i, k, d, cnt, any, best, idx, csum, wsum, kill, nmin, r;
    struct cmb *p;

    seen[4] = seen[5] = seen[6] = 0;
    cnt = 0;
    any = 0;
    for (p = combatants + 5; p < combatants + 13; p++)
        if (p->status == 1) {
            cnt++;
            if (D_c23e[p->kind] & (1 << party_floor)) {
                seen[p->kind] = 1;
                any = 1;
            }
        }
    if (cnt <= 0)
        return;
    if (any == 1 && real_combat == 1) {
        for (i = 0; i < 3; i++)
            if (seen[i + 4] == 0)
                for (p = combatants + 5; p < combatants + 13; p++)
                    if (p->kind - i == 4)
                        p->status = 0;
        best = 9999;
        for (p = combatants + 5; p < combatants + 13; p++)
            if (p->status == 1) {
                d = distance3((combatants[who].x - p->x) / 2, combatants[who].y - p->y, 0);
                if (d < best)
                    best = d;
            }
        *(struct stair *)D_c3cd = *(struct stair *)e;
        D_c2bb = sub_root_0410_001a(best / 32, 1, 4) * 60 + combat_clock;
        D_d6eb = 0;
        D_d842 = 1;
        D_d6f0 = 0;
        loot_after_fight();
        real_combat = 0;
        return;
    }
    num[4] = num[5] = num[6] = 0;
    wsum = csum = 0;
    best = 9999;
    for (i = 5, p = combatants + 5; p < combatants + 13; p++, i++)
        if (p->status == 1) {
            csum += p->f5d;
            wsum += p->f64;
            d = distance3((combatants[who].x - p->x) / 2, combatants[who].y - p->y, 0);
            if (d < best) {
                best = d;
                idx = i;
            }
        }
    k = wsum / cnt;
    kill = cnt - (k / 2 + csum) / k;
    nmin = 5;
    if (kill > 0) {
        cnt -= kill;
        do {
            r = 999;
            for (i = 5, p = combatants + 5; p < combatants + 13; p++, i++)
                if (p->status == 1) {
                    d = (char)(100 * p->f5d) / p->f64;
                    if (d < r && D_c1f3[i] == 0) {
                        r = d;
                        nmin = i;
                    }
                }
            combatants[nmin].status = 0;
        } while (--kill);
    }
    if (cnt <= 0)
        return;
    for (p = combatants + 5; p < combatants + 13; p++)
        if (p->status == 1) {
            typ[p->kind] = D_c28d[p->kind];
            num[p->kind]++;
        }
    activations.r[activation_count].x = combatants[idx].x;
    activations.r[activation_count].y = combatants[idx].y;
    activations.r[activation_count].sprd = 0x10;
    activations.r[activation_count].reenf = 0;
    *(struct six far *)activations.r[activation_count].fgp = *(struct six far *)F_2e38_0bb0;
    for (i = 0; i < 3; i++) {
        activations.r[activation_count].type[i] = typ[i + 4];
        activations.r[activation_count].num[i] = num[i + 4];
    }
    activations.r[activation_count].rm = -1;
    activation_count++;
    D_d6eb = 0;
    floor_dirty = 1;
    for (p = combatants + 5; p < combatants + 13; p++)
        p->status = 0;
    enemy_anim_free5_vec();
}
