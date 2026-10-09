/* place: s023 1ef8:0000 */
/* orig: size=1acc CONST=7df4+000e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 2/16 (0d1a, 0d88), ALL 16 functions written (functional drafts). 03ca/0734: the original
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

extern struct flr far F_20a5_0000;
extern struct { long m[16]; } far F_2e38_0bb6;
extern struct { int w[1]; } far F_20a5_000;
extern struct cell far F_20a5_0[][40];
extern int far F_20a5_248a[];
extern int far F_20a5_00[];
extern struct trap far F_20a5_2184[];
extern char far F_20a5_33d3[];
extern struct sdoor far F_20a5_3350[];
extern struct mtrap far F_20a5_316e[];
extern struct chest far F_20a5_3014[];
extern struct stair far F_20a5_2fb8[];
extern struct wtab far F_20a5_76e7;     /* weapon damage */
extern struct wtab far F_20a5_76a8;     /* weapon class | penetration */
extern struct item far F_2e38_003e[][64];
extern struct { struct item it[5][64]; } far F_2e38_03e;
extern struct { struct atv r[30]; } far F_20a5_47ee;
extern char far F_2e38_0bb0[];
extern char far F_20a5_47f4[];

extern struct cmb D_9c55[];
extern char D_11ca, D_d4e4, D_d6f0, D_d6f4, D_e1ca, D_07d3, D_a6c1, D_c623, D_d842, D_d6eb;
extern unsigned char D_d6cc;
extern unsigned D_d68d[];
extern char D_c678[];       /* door/lock action: 1 = open */
extern unsigned char D_c6ef[];  /* facing per combatant */
extern unsigned char D_c784[];  /* chest index per combatant */
extern int D_c7a4[];
extern int D_e0a3, D_e0a5, D_e0a7, D_e0a9;
extern int D_07d4, D_c2bb, D_c19b;
extern int D_c67f, D_940b;
extern unsigned D_c654, D_c656;
extern char *D_c291;
extern char D_17b0;
extern char D_c80f[], D_c831[], D_119a[], D_11a6[], D_a6b4[], D_c8b2[];
extern char D_23e5[][3], D_23ef[][3];
extern char D_c1f3[];
extern int D_c23e[];
extern int D_c28d[];
extern char D_c3cd[];
extern char D_04c8[], D_04f8[], D_0518[], D_0540[], D_055a[], D_0582[], D_0644[];

int far random_n(int);
int far sub_root_0000_0c8c(int);
void far sub_root_0000_24bc(char *, int);
int far sub_root_047c_1120(int, int, int);
int far sub_root_0410_001a(int, int, int);
void far sub_root_05f8_0117(int, int, int);
void far *_fmemcpy(void far *, const void far *, unsigned);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int far sub_s014_1d69_000c(int, int, int);
void far sub_s014_18bf_08ca(int, int, int, int);
void far sub_s014_0e76_27cc(int);
void far sub_s014_19cd_3258(char *, int *, int, int);
void far sub_s014_19cd_2e50(unsigned, unsigned, int *, char far *, int);
int far sub_s014_19cd_3204(char *, int);
void far sub_s014_19cd_1d5e(int);
void far sub_s014_19cd_34a0(void);
void far sub_s014_14f7_0008(void);
void far vec_1c2f(int);
void far vec_1b3f(int);
void far vec_1cb1(void);
void far vec_1e05(int, char *);
int far vec_1d29(int, struct atv far *, int);
void far vec_1dc9(int, int, int, int, int, int, int);
void far vec_1ced(int, int, int, int, int);

void far sub_s023_1ef8_0000(int, int, int);
void far sub_s023_1ef8_00b8(int, int);
void near sub_s023_1ef8_03ca(int, int, int, int);
void far sub_s023_1ef8_04ac(int, int, int);
void near sub_s023_1ef8_0734(int, int, int, int);
void far sub_s023_1ef8_0812(int, int, unsigned, int);
void far sub_s023_1ef8_0aea(int, int);
void far sub_s023_1ef8_0b6e(int, int);
void near sub_s023_1ef8_0d1a(int, int, int);
void far sub_s023_1ef8_0d88(int, int, int);
void far sub_s023_1ef8_0e30(int, int, int, int);
int far sub_s023_1ef8_12c0(unsigned char far *);
void far sub_s023_1ef8_13d0(int);
int far sub_s023_1ef8_153e(int);
void far sub_s023_1ef8_1618(int, int);
void far sub_s023_1ef8_17d2(int, char *);

/* reveal wall group n of the current level: remember it in the level mask and clear the
 * hidden bit of every cell in its list (inline in the original, one copy per caller) */
#define REVEAL(n, u, j) \
    F_2e38_0bb6.m[D_11ca] |= 1L << (n); \
    u.w = *(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[n]); \
    if (u.w != -1) { \
        j = 0; \
        do { \
            F_20a5_0000.c[u.b.hi][u.b.lo].b3 &= 0x7f; \
            j += 2; \
            u.w = *(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[n] + j); \
        } while (u.w != -1); \
    }

union cw { int w; struct { unsigned char lo, hi; } b; };

void far sub_s023_1ef8_0000(int who, int h, int n)
{
    union cw u;
    int j;

    D_d4e4 = 1;
    if (n >= 0) {
        REVEAL(n, u, j)
    }
    F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
    vec_1c2f(-1);
}

void far sub_s023_1ef8_00b8(int n, int who)
{
    union cw u;
    int j, x, y, w, k;

    D_d4e4 = 1;
    x = 0;
    w = F_20a5_0000.d[n].wall;
    if (w >= 0) {
        REVEAL(w, u, j)
    }
    if (D_c678[who] == 1)
        F_20a5_0000.open[n] = 1;
    else
        F_20a5_0000.open[n] = 0;
    F_20a5_0000.d[n].state = 0;
    if (F_20a5_0000.d[n].link != -1) {
        F_20a5_0000.d[F_20a5_0000.d[n].link].state = 0;
        if (D_c678[who] == 1)
            F_20a5_0000.open[F_20a5_0000.d[n].link] = 1;
        else
            F_20a5_0000.open[F_20a5_0000.d[n].link] = 0;
    }
    x = F_20a5_0000.d[n].x & 0x3f;
    y = F_20a5_0000.d[n].y & 0x3f;
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
    x = F_20a5_0000.d[n].x2 & 0x3f;
    if (x) {
        y = F_20a5_0000.d[n].y2 & 0x3f;
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
        for (k = 0; k < D_d6cc; k++)
            if (F_20a5_47ee.r[k].rm == w && F_20a5_47ee.r[k].sprd > 0) {
                if (vec_1d29(0, &F_20a5_47ee.r[k], who) == 1) {
                    D_d6f4 = 1;
                    D_c19b = random_n(300) + D_07d4 + 300;
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
        case 0: F_20a5_0000.c[x][y].b2 |= 8; return;
        case 1: F_20a5_0000.c[x][y].b0 |= 8; return;
        case 2: F_20a5_0000.c[x][y].b3 |= 8; return;
        case 3: F_20a5_0000.c[x][y].b1 |= 8; return;
        }
    else
        switch (dir) {
        case 0: F_20a5_0000.c[x][y].b2 &= ~8; return;
        case 1: F_20a5_0000.c[x][y].b0 &= ~8; return;
        case 2: F_20a5_0000.c[x][y].b3 &= ~8; return;
        case 3: F_20a5_0000.c[x][y].b1 &= ~8; return;
        }
}

void far sub_s023_1ef8_04ac(int n, int open, int reveal)
{
    union cw u;
    int j, x, y, w, dir;
    unsigned char t;

    if ((F_20a5_0000.d[n].x & 0x80) == 0x80)
        dir = 0;
    else if ((F_20a5_0000.d[n].x & 0x40) == 0x40)
        dir = 1;
    else if (((t = F_20a5_0000.d[n].y) & 0x80) == 0x80)
        dir = 2;
    else if ((t & 0x40) == 0x40)
        dir = 3;
    if (open == 1)
        F_20a5_0000.open[n] = 1;
    else
        F_20a5_0000.open[n] = 0;
    if (reveal == 1) {
        D_d4e4 = 1;
        w = F_20a5_0000.d[n].wall;
        if (w >= 0) {
            REVEAL(w, u, j)
        }
    }
    if (F_20a5_0000.d[n].link != -1) {
        if (open == 1)
            F_20a5_0000.open[F_20a5_0000.d[n].link] = 1;
        else
            F_20a5_0000.open[F_20a5_0000.d[n].link] = 0;
    }
    x = F_20a5_0000.d[n].x & 0x3f;
    y = F_20a5_0000.d[n].y & 0x3f;
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
    x = F_20a5_0000.d[n].x2 & 0x3f;
    if (x) {
        y = F_20a5_0000.d[n].y2 & 0x3f;
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
        case 0: F_20a5_0000.c[x][y].b2 |= 8; return;
        case 1: F_20a5_0000.c[x][y].b0 |= 8; return;
        case 2: F_20a5_0000.c[x][y].b3 |= 8; return;
        case 3: F_20a5_0000.c[x][y].b1 |= 8; return;
        }
    else
        switch (dir) {
        case 0: F_20a5_0000.c[x][y].b2 &= ~8; return;
        case 1: F_20a5_0000.c[x][y].b0 &= ~8; return;
        case 2: F_20a5_0000.c[x][y].b3 &= ~8; return;
        case 3: F_20a5_0000.c[x][y].b1 &= ~8; return;
        }
}

/* who searches: when skill check `roll` beats the record, the secret door is found */
void far sub_s023_1ef8_0812(int who, int h, unsigned rec, int roll)
{
    char msg[60];
    union cw u;
    int j, w, n, h2, x, y;

    n = rec & 0xff;
    h2 = rec >> 8;
    if (D_9c55[who].f60 > roll) {
        F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
        if (h2 != 0xff)
            F_20a5_0000.t[h2].on = -F_20a5_0000.t[h2].on;
        if ((F_20a5_0000.d[n].x & 0x80) == 0x80)
            D_c6ef[who] = 0;
        else if ((F_20a5_0000.d[n].x & 0x40) == 0x40)
            D_c6ef[who] = 1;
        else if ((F_20a5_0000.d[n].y & 0x80) == 0x80)
            D_c6ef[who] = 2;
        else if ((F_20a5_0000.d[n].y & 0x40) == 0x40)
            D_c6ef[who] = 3;
        x = F_20a5_0000.d[n].x & 0x3f;
        y = F_20a5_0000.d[n].y & 0x3f;
        switch (D_c6ef[who]) {
        case 0:
            F_20a5_0000.c[x][y].b2 &= 0xf0;
            F_20a5_0000.c[x][y - 1].b3 &= 0xf0;
            break;
        case 1:
            F_20a5_0000.c[x][y].b0 &= 0xf0;
            F_20a5_0000.c[x + 1][y].b1 &= 0xf0;
            break;
        case 2:
            F_20a5_0000.c[x][y].b3 &= 0xf0;
            F_20a5_0000.c[x][y + 1].b2 &= 0xf0;
            break;
        case 3:
            F_20a5_0000.c[x][y].b1 &= 0xf0;
            F_20a5_0000.c[x - 1][y].b0 &= 0xf0;
            break;
        }
        strcpy(msg, D_9c55[who].name);
        strcat(msg, D_055a);
        sub_root_0000_24bc(msg, 150);
        D_d4e4 = 1;
        x = 0;
        w = F_20a5_0000.d[n].wall;
        if (w >= 0) {
            REVEAL(w, u, j)
        }
        vec_1c2f(-1);
    }
}

void far sub_s023_1ef8_0aea(int n, int h)
{
    F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
    D_d4e4 = 1;
    if (F_20a5_0000.sd[n].x)
        F_20a5_0000.c[F_20a5_0000.sd[n].x][F_20a5_0000.sd[n].y].b0 &= 0x0f;
    if (F_20a5_0000.sd[n].x2)
        F_20a5_0000.c[F_20a5_0000.sd[n].x2][F_20a5_0000.sd[n].y2].b0 &= 0x0f;
}

void far sub_s023_1ef8_0b6e(int n, int h)
{
    union cw u;
    int j, x, y, dir;
    struct wrec far *q;

    F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
    D_d4e4 = 1;
    dir = 0;
    q = &F_20a5_0000.wr[n];
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
    case 0: F_20a5_0000.c[x][y].b2 = 0; break;
    case 1: F_20a5_0000.c[x][y].b0 &= 0xf0; break;
    case 2: F_20a5_0000.c[x][y].b3 &= 0xf0; break;
    case 3: F_20a5_0000.c[x][y].b1 = 0; break;
    }
}

/* spell trap t fires at who */
void far sub_s023_1ef8_0d88(int who, int h, int t)
{
    sub_root_0000_24bc(D_0644, 60);
    vec_1dc9(D_9c55[who].x, D_9c55[who].y, who, F_20a5_0000.m[t].u.c.a,
             F_20a5_0000.m[t].u.c.b, who, D_11ca);
    F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
    if (F_20a5_0000.t[F_20a5_0000.m[t].link].on >= 0)
        F_20a5_0000.t[F_20a5_0000.m[t].link].on = -F_20a5_0000.t[F_20a5_0000.m[t].link].on;
}

/* missile trap t (and its twin t2, 0xff none) fires its darts along the wall line */
void far sub_s023_1ef8_0e30(int who, int h, int t, int t2)
{
    int sv1[5], sv2[5];
    int i, k, cur, dir, cnt, x, y, cx, cy, tgt, dmg, cls, arm, d;

    sub_root_0000_24bc(D_0644, 60);
    F_20a5_0000.t[h].on = -F_20a5_0000.t[h].on;
    cur = t;
    if (F_20a5_0000.t[F_20a5_0000.m[cur].link].on >= 0)
        F_20a5_0000.t[F_20a5_0000.m[cur].link].on = -F_20a5_0000.t[F_20a5_0000.m[cur].link].on;
    for (i = 0; i < 5; i++) {
        sv1[i] = D_c80f[i];
        sv2[i] = D_c831[i];
        D_c80f[i] = D_119a[i];
        D_c831[i] = D_11a6[i];
    }
again:
    dir = F_20a5_0000.m[cur].dir;
    cnt = F_20a5_0000.m[cur].cnt;
    x = F_20a5_0000.m[cur].u.x;
    cx = x >> 5;
    y = F_20a5_0000.m[cur].y;
    cy = y >> 4;
    for (i = 0; i < cnt; i++) {
        switch (dir) {
        case 0:
            D_e0a5 = D_23e5[cnt][i] + x;
            D_e0a3 = y - 3;
            for (k = 0; k < 16; k++)
                if (F_20a5_0000.c[cx][cy - k].b2 & 0x0f) {
                    D_e0a9 = D_e0a5;
                    D_e0a7 = ((cy - k) << 4) + 4;
                    break;
                }
            if (k == 16) {
                D_e0a9 = D_e0a5;
                D_e0a7 = (cy << 4) - 0xec;
            }
            break;
        case 2:
            D_e0a5 = x + 3;
            D_e0a3 = D_23ef[cnt][i] + y;
            for (k = 0; k < 16; k++)
                if (F_20a5_0000.c[cx + k][cy].b0 & 0x0f) {
                    D_e0a9 = ((cx + k) << 5) + 0x1a;
                    D_e0a7 = D_e0a3;
                    break;
                }
            if (k == 16) {
                D_e0a9 = (cx << 5) + 0x1fa;
                D_e0a7 = D_e0a3;
            }
            break;
        case 4:
            D_e0a5 = D_23e5[cnt][i] + x;
            D_e0a3 = y + 3;
            for (k = 0; k < 16; k++)
                if (*(unsigned far *)&F_20a5_0000.c[cx][cy + k].b2 & 0xf00) {
                    D_e0a9 = D_e0a5;
                    D_e0a7 = ((cy + k) << 4) + 0x0b;
                    break;
                }
            if (k == 16) {
                D_e0a9 = D_e0a5;
                D_e0a7 = (cy << 4) + 0xfb;
            }
            break;
        case 6:
            D_e0a5 = x - 3;
            D_e0a3 = D_23ef[cnt][i] + y;
            for (k = 0; k < 16; k++)
                if (F_20a5_0000.c[cx - k][cy].b1 & 0x0f) {
                    D_e0a9 = ((cx - k) << 5) + 6;
                    D_e0a7 = D_e0a3;
                    break;
                }
            if (k == 16) {
                D_e0a9 = (cx << 5) - 0x1da;
                D_e0a7 = D_e0a3;
            }
            break;
        }
        tgt = sub_s014_1d69_000c(-2, -2, 0);
        if (tgt < 50)
            vec_1ced(-2, -1, 0, dir, F_20a5_0000.m[cur].wpn);
        else {
            tgt -= 50;
            if (D_9c55[tgt].f5f >= random_n(100))
                vec_1ced(-2, tgt, 0, dir, F_20a5_0000.m[cur].wpn);
            else {
                dmg = random_n(6) + F_20a5_76e7.v[F_20a5_0000.m[cur].wpn] - 2;
                cls = F_20a5_76a8.v[F_20a5_0000.m[cur].wpn] & 0x0f;
                arm = D_9c55[tgt].f4d;
                if (cls > arm)
                    d = dmg;
                else if (cls == arm)
                    d = dmg / 2;
                else
                    d = dmg / 8;
                d += (F_20a5_0000.m[cur].skill - D_9c55[tgt].f4f) / 10;
                sub_s014_18bf_08ca(d, tgt, cls, arm);
                vec_1ced(-1, tgt, 0, dir, F_20a5_0000.m[cur].wpn);
            }
        }
    }
    if (t == cur && t2 != 0xff) {
        cur = t2;
        goto again;
    }
    for (i = 0; i < 5; i++) {
        D_c80f[i] = sv1[i];
        D_c831[i] = sv2[i];
    }
}

/* the selected member (D_e1ca) picks a lock: needs lock picks; returns 1 when it opens */
int far sub_s023_1ef8_12c0(unsigned char far *lock)
{
    int k, found, ok, q, s;

    k = 0;
    found = 0;
    ok = 0;
    if (D_9c55[D_e1ca].f7e > 0) {
        for (; F_2e38_003e[D_e1ca][k].id != 0x40; )
            if (++k >= D_9c55[D_e1ca].f7e)
                goto none;
        found = 1;
    }
none:
    q = F_2e38_03e.it[D_e1ca][k].f3;
    if (found == 0)
        sub_root_0000_24bc(D_04c8, 150);
    else {
        s = D_9c55[D_e1ca].f79 + q - 25;
        if ((*lock & 0x7f) < sub_root_0000_0c8c(10) + s) {
            sub_root_0000_24bc(D_04f8, 150);
            ok = 1;
            *lock = 0;
            D_d68d[D_e1ca] |= 0x20;
        } else {
            if ((*lock & 0x7f) < s + 10)
                sub_root_0000_24bc(D_0540, 150);
            else
                sub_root_0000_24bc(D_0518, 150);
            D_d68d[D_e1ca] |= 0x40;
        }
    }
    return ok;
}

/* who opens his chest: let the party take the contents */
void far sub_s023_1ef8_13d0(int who)
{
    char n;
    int h;
    struct chest far *c;

    D_07d3++;
    c = &F_20a5_0000.ch[D_c784[who]];
    if ((F_20a5_0000.c[c->x][c->y].b0 & 0xf0) == 0x50)
        *(long far *)&F_20a5_0000.c[F_20a5_0000.ch[D_c784[who]].x][F_20a5_0000.ch[D_c784[who]].y] += 0x10;
    h = D_c7a4[F_20a5_0000.ch[D_c784[who]].idx];
    n = F_20a5_0000.ch[D_c784[who]].n;
    sub_s014_19cd_3258(&n, &h, 1, 0);
    sub_s014_19cd_2e50(D_c654, D_c656, &D_c67f, (char far *)F_20a5_0000.ch[D_c784[who]].it, who);
    D_d4e4 = 1;
    F_20a5_0000.ch[D_c784[who]].it[0] = 0;
    F_20a5_0000.ch[D_c784[who]].it[1] = 0;
    F_20a5_0000.ch[D_c784[who]].it[2] = 0;
    F_20a5_0000.ch[D_c784[who]].n = D_c67f;
    if (D_c67f != 0) {
        sub_root_05f8_0117(3, 10, D_940b);
        _fmemcpy((char far *)D_c291, (char far *)(((long)D_c656 << 16) | D_c654), D_c67f * 6);
        sub_root_05f8_0117(3, 3, D_940b);
        D_c7a4[F_20a5_0000.ch[D_c784[who]].idx] = sub_s014_19cd_3204(D_c291, D_c67f);
    }
    vec_1c2f(D_17b0);
    sub_s014_14f7_0008();
    D_07d3--;
}

/* who disarms the trap of his chest; on failure it goes off */
int far sub_s023_1ef8_153e(int who)
{
    int s, r, ok;

    ok = 0;
    s = D_9c55[who].f79 - 10;
    r = sub_root_0000_0c8c(20);
    if (F_20a5_0000.ch[D_c784[who]].lock < s + r) {
        sub_root_0000_24bc(D_0582, 150);
        D_d68d[who] |= 0x20;
        F_20a5_0000.ch[D_c784[who]].lock = 0;
        return 1;
    }
    sub_root_0000_24bc(D_0644, 60);
    D_d68d[who] |= 0x40;
    vec_1dc9(D_9c55[who].x, D_9c55[who].y, who, F_20a5_0000.ch[D_c784[who]].trap & 0x0f,
             (F_20a5_0000.ch[D_c784[who]].trap >> 4) * 10, who, D_11ca);
    return ok;
}

/* who takes stairs/exit n: move to the other level */
void far sub_s023_1ef8_1618(int n, int who)
{
    struct stair s;
    int j, wall, lvl, old, alone, i;
    struct cmb *p;
    unsigned char far *c;

    old = D_11ca;
    s = F_20a5_0000.s[n];
    wall = F_20a5_0000.s[n].wall;
    F_20a5_0000.s[n].wall = -1;
    lvl = F_20a5_0000.s[n].lvl;
    D_a6b4[who] = lvl;
    alone = 1;
    for (i = 0, p = D_9c55; p < D_9c55 + 5; p++, i++)
        if (p->status == 1 && D_a6b4[i] != lvl)
            alone = 0;
    if (alone == 1) {
        D_c623 = lvl;
        sub_s023_1ef8_17d2(who, (char *)&s);
        D_a6c1 = 0;
    }
    vec_1b3f(lvl);
    if (wall != -1) {
        F_2e38_0bb6.m[lvl] |= 1L << wall;
        j = 0;
        if (*(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[wall]) != -1)
            do {
                c = (unsigned char far *)&F_20a5_000 + F_20a5_0000.list[wall] + j;
                F_20a5_0000.c[c[1]][c[0]].b3 &= 0x7f;
                j += 2;
            } while (*(int far *)((char far *)&F_20a5_000 + F_20a5_0000.list[wall] + j) != -1);
        D_d4e4 = 1;
    }
    vec_1e05(who, (char *)&s);
    if (D_c623 == lvl)
        vec_1c2f(who);
    else {
        vec_1b3f(old);
        vec_1c2f(-1);
    }
    D_9c55[who].f10 = 0;
    sub_s014_0e76_27cc(who);
    D_c8b2[who] = F_20a5_0000.s[n].f0d;
}

/* the party left the level through exit e (14-byte stairs record): if enemy groups that saw
 * the party follow (D_c23e[kind] bit per level) they pursue after a delay; otherwise the
 * weakest enemies are dropped and the rest re-form an activation record at the exit */
void far sub_s023_1ef8_17d2(int who, char *e)
{
    char seen[7];
    int typ[7], num[7];
    int i, k, d, cnt, any, best, idx, csum, wsum, kill, nmin, r;
    struct cmb *p;

    seen[4] = seen[5] = seen[6] = 0;
    cnt = 0;
    any = 0;
    for (p = D_9c55 + 5; p < D_9c55 + 13; p++)
        if (p->status == 1) {
            cnt++;
            if (D_c23e[p->kind] & (1 << D_c623)) {
                seen[p->kind] = 1;
                any = 1;
            }
        }
    if (cnt <= 0)
        return;
    if (any == 1 && D_a6c1 == 1) {
        for (i = 0; i < 3; i++)
            if (seen[i + 4] == 0)
                for (p = D_9c55 + 5; p < D_9c55 + 13; p++)
                    if (p->kind - i == 4)
                        p->status = 0;
        best = 9999;
        for (p = D_9c55 + 5; p < D_9c55 + 13; p++)
            if (p->status == 1) {
                d = sub_root_047c_1120((D_9c55[who].x - p->x) / 2, D_9c55[who].y - p->y, 0);
                if (d < best)
                    best = d;
            }
        *(struct stair *)D_c3cd = *(struct stair *)e;
        D_c2bb = sub_root_0410_001a(best / 32, 1, 4) * 60 + D_07d4;
        D_d6eb = 0;
        D_d842 = 1;
        D_d6f0 = 0;
        sub_s014_19cd_34a0();
        D_a6c1 = 0;
        return;
    }
    num[4] = num[5] = num[6] = 0;
    wsum = csum = 0;
    best = 9999;
    for (i = 5, p = D_9c55 + 5; p < D_9c55 + 13; p++, i++)
        if (p->status == 1) {
            csum += p->f5d;
            wsum += p->f64;
            d = sub_root_047c_1120((D_9c55[who].x - p->x) / 2, D_9c55[who].y - p->y, 0);
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
            for (i = 5, p = D_9c55 + 5; p < D_9c55 + 13; p++, i++)
                if (p->status == 1) {
                    d = (char)(100 * p->f5d) / p->f64;
                    if (d < r && D_c1f3[i] == 0) {
                        r = d;
                        nmin = i;
                    }
                }
            D_9c55[nmin].status = 0;
        } while (--kill);
    }
    if (cnt <= 0)
        return;
    for (p = D_9c55 + 5; p < D_9c55 + 13; p++)
        if (p->status == 1) {
            typ[p->kind] = D_c28d[p->kind];
            num[p->kind]++;
        }
    F_20a5_47ee.r[D_d6cc].x = D_9c55[idx].x;
    F_20a5_47ee.r[D_d6cc].y = D_9c55[idx].y;
    F_20a5_47ee.r[D_d6cc].sprd = 0x10;
    F_20a5_47ee.r[D_d6cc].reenf = 0;
    *(struct six far *)F_20a5_47ee.r[D_d6cc].fgp = *(struct six far *)F_2e38_0bb0;
    for (i = 0; i < 3; i++) {
        F_20a5_47ee.r[D_d6cc].type[i] = typ[i + 4];
        F_20a5_47ee.r[D_d6cc].num[i] = num[i + 4];
    }
    F_20a5_47ee.r[D_d6cc].rm = -1;
    D_d6cc++;
    D_d6eb = 0;
    D_d4e4 = 1;
    for (p = D_9c55 + 5; p < D_9c55 + 13; p++)
        p->status = 0;
    vec_1cb1();
}
