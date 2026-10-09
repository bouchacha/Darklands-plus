/* place: s014 19cd:0006 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
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
   status: 34 of 34 functions written, 2 match (see NOTES). */

struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
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
extern int far F_20a5_41f4;             /* view origin y */
extern int far F_20a5_4be0;             /* view origin x */
extern char far F_20a5_1f52;            /* doors */
extern struct rect14 far F_20a5_1f54[];
extern char far F_20a5_2184;            /* trigger areas */
extern struct trig far F_20a5_2186[];
extern char far F_20a5_2fb6;            /* stairs areas */
extern struct rect14 far F_20a5_2fb8[];
extern char far F_20a5_300c;            /* special regions */
extern struct region far F_20a5_300e[];
extern char far F_20a5_3326;            /* exit zones */
extern struct zone far F_20a5_3328[];
extern char far F_20a5_33d3[40];        /* door states */
extern struct pile far F_20a5_59c8[];   /* loot piles (count D_c80e): offset in the loot file */
extern struct loot far F_20a5_3954[];   /* items of the fallen (count D_c783) */
extern char far F_20a5_6238[13][250];   /* paths */
extern char far F_20a5_7900[13][13];    /* melee reach */
extern long far F_20a5_0000[];          /* map cells */
extern unsigned char far F_20a5_1900[]; /* map height/flag bytes [40][40] */
extern struct { int a, b, c; } far F_2e38_0bb0;
extern unsigned char far F_2e38_0003[][0x180]; /* party inventories (+0x3e..) */

extern struct cbt D_9c55[];
extern int D_e118;                      /* path length */
extern int D_d65f, D_d67e;              /* enemy animation slots */
extern struct cbt D_d73e;               /* scratch record */
extern char D_d848[];                   /* hot spot -> combatant */
extern int D_d8f1[], D_d8d7[], D_d8bd[], D_d8a3[]; /* hot spot rects */
extern int D_d889[], D_d86f[];          /* cursor anchor */
extern unsigned char D_d862[], D_d855[];/* cursor size */
extern char D_e0a1;                     /* hot spot count */
extern int D_6eec, D_6eee;              /* mouse x, y */
extern unsigned char D_6ef0;            /* mouse buttons */
extern char D_d728;                     /* button pressed */
#define KBFLAGS (*(unsigned char far *)0x00400017L)  /* BIOS keyboard flags (4 = Ctrl) */
extern int D_9409;
extern char D_c102;                     /* movement mode */
extern unsigned D_da4b;                 /* group-cursor timeout */
extern unsigned D_07d4;                 /* clock */
extern char D_d845;                     /* cursor target */
extern int D_c941, D_c987, D_ca19, D_ca2a;   /* group cursor colours */
extern int D_e1c0;
extern char D_c624[];                   /* marching order */
extern char D_c629;                     /* its length */
extern char D_a6b3, D_a6b2;             /* order mode, selected member */
extern int D_a6b0;                      /* chosen action */
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
extern char D_a6d3[];
extern char D_d6fb[];
extern char D_c623, D_11ca;             /* party floor, shown floor */
extern int D_e0a5, D_e0a3, D_e0a9, D_e0a7;
extern int D_c2bd, D_c242, D_c244, D_c28f, D_c4db, D_c621;   /* cursor colours */
extern int D_e1c2, D_e1be, D_e1b8, D_e1ba, D_e1bc;
extern char D_c600;
extern int D_c652, D_c67d;              /* order destination */
extern char D_a6c1;
extern char D_17b0;
extern char D_ca64[], D_ca57[], D_ca30[];
extern int D_d97c[];
extern int D_c867;
extern char D_a6c4[];                   /* [13] */
extern unsigned char D_d6ea;
extern unsigned char D_a696[][2];       /* [13] location flags */
extern char *D_1174[];
extern char D_c1ce, D_c1f2, D_ca0c[], D_c97a[];
extern int D_d996[];
extern char D_9064[];                   /* [13] selected flag */
extern char D_a695;
extern char D_07d3;
extern char D_9609;
extern char D_6f9c_[];
extern char D_c80f[], D_c831[];
extern char D_d505[][8];                /* sprite x offsets */
extern char D_d5d3[], D_d5de[];
extern char D_1b9a[8], D_1ba2[8], D_1baa[8], D_1bb2[8], D_1bba[8], D_1bc2[8]; /* formation */
extern char D_179c[];
extern char D_17b2[][3];

void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_0000_24bc(char *, int);      /* message box */
int far sub_root_047c_1320(int, int);
unsigned far sub_root_047c_1120(int, int, int);
int far sub_root_0410_004c(int, int);          /* direction - n (mod 8) */
int far sub_root_0410_005a(int, int);          /* direction + n (mod 8) */
void far sub_s014_0e76_3c86(int, int);
void far sub_s014_0e76_4188(int, int, int);
void far sub_s014_0e76_4694(void);
void far sub_s014_0e76_4780(void);
int far sub_s014_12f7_000a(int);
void far sub_s014_12f7_1554(int);
void far sub_s014_12f7_159c(void);
void far sub_s014_12f7_15d0(int);
int far sub_s014_1462_000a(struct cbt *);
int far sub_s014_14f7_343c(int);
void far sub_s014_14f7_0008(void);
void far sub_s014_18bf_0a40(int, int, int);
int far sub_s014_18bf_0c02(int);
int far sub_s014_1d69_000c(int, int, int);
void far vec_1d47(int);                 /* s016 09bc: Ctrl-click on a member */
void far vec_1c39(int);                 /* s016 0a2c: click on a member */
void far vec_1c2f(int);                 /* s017 0000: redraw (combatant or -1) */
void far vec_1d6f(int, int, int);       /* s023 04ac: door state */
void far vec_1c4d(int, int);            /* s023 0aea: remove furniture */
void far vec_1c57(int, int);            /* s023 0b6e: remove wall */
void far vec_1da1(int);                 /* s019 0c80: spell/effect menu of a member */
void far vec_1cb1(void);                /* s025 0be4: free enemy animations */
void far vec_1dab(unsigned, unsigned, unsigned char *, unsigned, unsigned, int); /* s040: loot screen */
void far vec_1ae5(void);                /* s002 0e76_0c8c: restore combat screen */
void far vec_1dbf(void);                /* s024 07a2: reload furniture */
void far vec_1db5(int);                 /* s025 0a84: reload an enemy animation set */

void far sub_s014_19cd_069c(void);
void far sub_s014_19cd_0738(void);
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
void far sub_s014_19cd_28a2(void);
void far sub_s014_19cd_29b8(void);

/* m := combatant of the front-most hot spot under (x, y), or 99 */
#define TOPMOST(m, x, y) { int k_, by_ = 0; m = 99; \
    for (k_ = D_e0a1 - 1; k_ >= 0; k_--) \
        if (HIT(k_, x, y) && by_ < D_9c55[(unsigned char)D_d848[k_]].y) { \
            m = (unsigned char)D_d848[k_]; by_ = D_9c55[m].y; } }
#define MAPY(my) ((my) - F_20a5_41f4)
#define HIT(k, x, y) (D_d8f1[k] <= (x) && (x) <= D_d8d7[k] && D_d8bd[k] <= (y) && (y) <= D_d8a3[k])

/* member cursor around the selected member, using hot spot k */
#define MEMBER_CURSOR(k) \
    D_e09f = D_d505[sub_s014_14f7_343c(D_a6b2)][D_9c55[D_a6b2].f1a] + D_d889[k] - 4; \
    D_e09d = (D_d5d3[sub_s014_14f7_343c(D_a6b2)] >> 1) + D_d86f[k] - 13

/* mouse handler */
void far sub_s014_19cd_0006(void)
{
    int mx, my;
    int k, i, n, r;
    int c1, c2, w1, h1, w2, h2, dx1, dy1, dx2, dy2, col1, col2, lead;
    int ex, ey, ry, rx;

    my = D_6eee;
    mx = D_6eec - 60;
    if (D_9409 != 0)
        return;
    if (D_c102 != 0 && D_da4b < D_07d4)
        D_d845 = -1;
    if (D_c102 == 1) {
        col1 = D_c941; dx1 = 11; dx2 = 11; dy1 = 25; D_e1c0 = 25; dy2 = 25;
        w1 = 0x16; w2 = 0x16; h1 = 0x1d; h2 = 0x1d; col2 = D_c987;
        lead = D_c624[0];
    } else if (D_c102 == 2) {
        col1 = D_ca19; dx1 = 7; dx2 = 7; dy1 = 28; D_e1c0 = 28; dy2 = 28;
        w1 = 0xe; w2 = 0xe; h1 = 0x1f; h2 = 0x1f; col2 = D_ca2a;
        lead = D_c624[0];
    }
    /* hover */
    if ((D_d728 == 1 || (D_6ef0 & 1) != 0) && mx >= 0 && mx < 0x104 && (KBFLAGS & 4) == 0) {
        if (D_c102 == 0) {
            if (D_a6b3 == 0) {
                for (k = D_e0a1 - 1; k >= 0; k--)
                    if (D_d848[k] < 5 && HIT(k, mx, my)) {
                        D_a6b2 = D_d848[k];
                        MEMBER_CURSOR(k);
                        D_e1c6 = 8;
                        D_e1c4 = 0x1a;
                        D_a6b0 = -1;
                        D_c7cc = 0;
                        sub_s014_19cd_19ee();
                        break;
                    }
                if (k < 0)
                    D_a6b2 = -1;
            } else
                sub_s014_19cd_0ad4(mx, my);
        } else {
            /* group cursor */
            D_d73e = D_9c55[D_c624[0]];
            ry = MAPY(my);
            D_d742 = ry - F_20a5_4be0 + mx - 16 + D_c26c;
            D_d744 = ry + D_c293;
            D_d788 = -1;
            D_d757 = 0x14;
            D_da4b = D_d7fe == 0 ? D_07d4 + 60 : D_07d4 + 1800;
            D_e1b4 = D_d742;
            D_e1b6 = D_d744;
            if (sub_s014_1462_000a(&D_d73e) == 0) {
                D_d845 = -2;
                sub_s014_19cd_1764(mx - dx1, my - dy1, w1, h1, col1, -1, 0);
            } else {
                D_d845 = -3;
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
        if (D_c102 != 0)
            D_da4b = D_07d4 + 60;
        D_d73b = 0;
        ey = D_6eee;
        ex = D_6eec;
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
        for (k = D_e0a1 - 1; k >= 0; k--)
            if (D_d848[k] < 5 && HIT(k, ex, ey)) {
                vec_1d47(D_d848[k]);
                return;
            }
        return;
    }
    if (D_c102 == 0) {
        if (D_a6b3 == 0) {
            D_a6b2 = -1;
            for (k = D_e0a1 - 1; k >= 0; k--)
                if (D_d848[k] < 5 && HIT(k, ex, ey)) {
                    D_a6b2 = D_d848[k];
                    MEMBER_CURSOR(k);
                    sub_s014_19cd_069c();
                    return;
                }
        } else
            sub_s014_19cd_0dcc(ex, ey);
        return;
    }
    /* group move */
    ry = MAPY(ey);
    rx = ry - F_20a5_4be0 + ex - 16;
    D_da4b = D_07d4 + 60;
    for (i = 0; i < 5; i++)
        if (D_9c55[i].status == 1)
            D_a6d3[i] = 1;
    if (sub_s014_19cd_11ec(lead, rx + D_c26c, ry + D_c293) != 1) {
        for (i = 0; i < 5; i++)
            if (D_9c55[i].status == 1)
                D_a6d3[i] = 0;
        D_d845 = -3;
        sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
        sub_s014_0e76_4694();
        return;
    }
    if (D_c102 != 1) {
        /* single file: only the leader paths, the others follow */
        D_9c55[D_c624[0]].tx = rx + D_c26c;
        D_9c55[D_c624[0]].ty = ry + D_c293;
        if (sub_s014_12f7_000a(D_c624[0]) == 1) {
            sub_s014_12f7_1554(D_c624[0]);
            D_d845 = -2;
            sub_s014_19cd_1764(ex - dx1, ey - dy1, w1, h1, col1, -1, 0);
            D_9c55[D_c624[0]].f10 = 1;
            D_9c55[D_c624[0]].target = -1;
            D_d6fb[D_c624[0]] = 0;
            return;
        }
        D_d845 = -3;
        sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
        sub_s014_0e76_4694();
        D_a6d3[D_c624[0]] = 0;
        return;
    }
    for (k = 0; k < D_c629; k++) {
        n = D_c624[k];
        if (D_9c55[n].status != 1)
            continue;
        if (sub_s014_12f7_000a(n) != 1) {
            D_d845 = -3;
            sub_s014_19cd_1764(ex - dx2, ey - dy2, w2, h2, col2, -1, 0);
            sub_s014_0e76_4694();
            for (i = 0; i < 5; i++)
                if (D_9c55[i].status == 1)
                    D_a6d3[i] = 0;
            return;
        }
        sub_s014_12f7_1554(n);
        D_d845 = -2;
        sub_s014_19cd_1764(ex - dx1, ey - dy1, w1, h1, col1, -1, 0);
        D_9c55[n].f10 = 1;
        D_d6fb[n] = 0;
    }
}

/* enter order mode for D_a6b2 */
void far sub_s014_19cd_069c(void)
{
    D_e1c6 = 8;
    D_e1c4 = 0x1a;
    D_a6b3 = 1;
    D_9064[D_a6b2] = 1;
    sub_root_0000_1588(D_a6b2);
    sub_root_0000_1716(0x100);
    D_d7fe = 1;
    D_d6ea = D_a696[D_a6b2][1] >> 4 & 1;
    D_a6b0 = -1;
    D_a6d1 = -1;
    D_c7cc = 0;
    D_e1bc = 0;
    D_c652 = 0;
    sub_s014_19cd_19ee();
    if (D_a6c1 == 0)
        sub_s014_19cd_2232(D_a6b2);
    sub_s014_19cd_29b8();
    if (D_a695 == 0) {
        D_a695 = 1;                     /* pause the AI while giving orders */
        sub_s014_0e76_4780();
        D_c867 = D_07d4;
        D_07d3++;
        D_9609 = 1;
    }
}

/* leave order mode and apply the order */
void far sub_s014_19cd_0738(void)
{
    int dx, dy, i;
    char *n;

    dy = D_c67d;
    dx = D_c652;
    D_d7fe = 0;
    if (D_a6b0 == -1 && D_c7cc == 0 && D_c652 == 0)
        goto out;
    i = D_a6b2;
    if (D_a6b0 == 0x400) {
        D_9c55[i].f10 = 0;
        D_a6c4[i] = D_a6d1;
        D_a696[i][0] &= 0xef;
        goto out;
    }
    if (D_d845 == -3 || D_e1bc == D_c2bd)
        goto out;
    if (D_c652 != 0) {
        D_9c55[i].tx = D_e1b4 = D_c652;
        D_9c55[i].ty = D_e1b6 = dy;
    }
    if (D_a6b0 == -1)
        D_a6b0 = D_9c55[i].f10;
    if ((D_a6b0 & 0x1be) == 0x1be)
        D_a696[i][0] &= 0xef;
    if (D_d845 < 1) {
        if (D_d845 != -2)
            goto out;
        if (D_a6b0 != 0x100) {
            if (D_a6c1 == 1 && D_c623 != D_11ca)
                sub_root_0000_24bc((char *)0x64a, 0x96);   /* "USE STAIRS-your only option..." */
            else {
                if (D_c7cc == 1 || dx != 0) {
                    sub_s014_12f7_15d0(i);
                    D_d6fb[i] = 0;
                }
                D_17b0 = i;
                D_9c55[i].tx = D_c652;
                D_9c55[i].ty = D_c67d;
                D_9c55[i].target = -1;
                D_a6d3[i] = 1;
                D_9c55[i].f10 = 1;
                D_ca64[i] = 0;
                D_ca57[i] = 0;
                D_ca30[i] = -1;
                D_d97c[i] = D_c867 + 30;
            }
            goto out;
        }
        /* cast / use at a spot */
        D_a6d3[i] = 0;
        n = D_1174[D_9c55[i].kind];
        n[0] = 'P';
        n[1] = 'T';
        D_ca0c[i] = D_c1ce;
        D_c97a[i] = D_c1f2;
        D_9c55[i].f10 = 0x100;
        D_9c55[i].tx = dx;
        D_9c55[i].ty = dy;
        D_9c55[i].target = -1;
        D_d996[i] = D_c867;
    } else if (D_a6b0 == 0x80 || D_a6b0 == 0x100) {
        if (D_a6b0 == 0x80) {
            /* fire a missile at D_d845 */
            D_a6d3[i] = 0;
            sub_s014_18bf_0a40(i, D_9c55[i].weapon, 0x80);
            D_9c55[i].target = D_d845;
            D_9c55[i].f10 = 0x80;
            D_a696[i][0] |= 2;
            D_d996[i] = D_c867;
        } else {
            D_a6d3[i] = 0;
            n = D_1174[D_9c55[i].kind];
            n[0] = 'P';
            n[1] = 'T';
            D_ca0c[i] = D_c1ce;
            D_c97a[i] = D_c1f2;
            D_9c55[i].f10 = 0x100;
            if (D_d845 == -2) {
                D_9c55[i].tx = dx;
                D_9c55[i].ty = dy;
                D_9c55[i].target = -1;
            } else
                D_9c55[i].target = D_d845;
            D_d996[i] = D_c867;
        }
    } else {
        /* melee attack on D_d845 */
        if ((D_c7cc == 1 || dx != 0) && D_c762 == 0)
            sub_s014_12f7_15d0(i);
        sub_s014_18bf_0a40(i, D_9c55[i].f51, 0);
        D_9c55[i].target = D_d845;
        if (D_c7cc == 0) {
            if ((D_a6b0 & 2) != 2)
                goto out;
            D_9c55[i].f10 = D_a6b0;
        } else if (D_c762 != 1) {
            D_a6d3[i] = 1;
            D_d6fb[i] = 0;
            D_9c55[i].f10 = 1;
            D_17b0 = i;
            if ((D_a6b0 & 2) == 2)
                sub_s014_0e76_3c86(i, D_a6b0);
            goto out;
        } else
            D_9c55[i].f10 = D_a6b0;
    }
    D_ca64[i] = 7;
out:
    D_9064[D_a6b2] = 0;
    D_a6b3 = 0;
    sub_root_0000_1588(D_a6b2);
    sub_root_0000_1716(0x100);
    D_a6b0 = -1;
    D_e1bc = 0;
    D_a6b2 = -1;
    D_d845 = -1;
    sub_s014_19cd_28a2();
}

/* order mode: hover feedback at (x, y) */
void far sub_s014_19cd_0ad4(int x, int y)
{
    int k, f, col, h, w, m;

    TOPMOST(m, x, y);
    if (m < 5)
        return;                         /* over a party member */
    D_d845 = -1;
    f = D_9c55[D_a6b2].f10;
    k = D_e0a1 - 1;
    if (D_a6b0 == 1 || (D_a6b0 == -1 && (f == 1 || f == 0)))
        k = -1;
    else
        for (; k >= 0; k--)
            if ((unsigned char)D_d848[k] > 4 && HIT(k, x, y)) {
                D_d845 = D_d848[k];
                col = D_c242;
                if (D_a6b0 == 0x80 || D_a6b0 == 0x100
                 || (D_a6b0 == -1 && (f == 0x80 || f == 0x100))) {
                    D_e0a5 = D_9c55[D_a6b2].x;
                    D_e0a3 = D_9c55[D_a6b2].y;
                    D_e0a9 = D_9c55[D_d845].x;
                    D_e0a7 = D_9c55[D_d845].y;
                    if (sub_s014_1d69_000c(D_a6b2, D_d845, 0x80) != 0)
                        col = D_c2bd;   /* no line of fire */
                }
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], col, 0, D_d845);
                break;
            }
    if (k < 0 && ((D_a6b0 == -1 && f != 0x1082) || D_a6b0 == 1 || (D_a6b0 == 0x100 && D_e1c8 == 0))) {
        D_d73e = D_9c55[D_a6b2];
        h = MAPY(y);
        D_d742 = h - F_20a5_4be0 + x + D_c26c - 16;
        D_d744 = h + D_c293;
        D_d788 = -1;
        D_d757 = 0x14;
        if (D_a6b0 == 0x100)
            D_c600 = 1;
        D_e1b4 = D_d742;
        D_e1b6 = D_d744;
        if (sub_s014_1462_000a(&D_d73e) == 0) {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c28f;
            D_d845 = -2;
            if (D_a6b0 == 0x100 && D_e1c8 == 0) {
                D_e0a5 = D_9c55[D_a6b2].x;
                D_e0a3 = D_9c55[D_a6b2].y;
                D_e0a9 = D_d742;
                D_e0a7 = D_d744;
                h = 0xf;
                D_e1c0 = 0xf;
                w = 0x15;
                col = D_c4db;
                if (sub_s014_1d69_000c(D_a6b2, D_a6b2, 0x100) == 1) {
                    col = D_c621;
                    D_d845 = -3;
                }
            }
        } else {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c244;
            D_d845 = -3;
        }
        sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
        if (D_a6b0 == 0x100)
            D_c600 = 0;
    }
    if (D_d845 == -1)
        D_e1bc = 0;
}

/* order mode: click at (x, y) */
void far sub_s014_19cd_0dcc(int x, int y)
{
    int k, f, col, h, w, m, i;

    TOPMOST(m, x, y);
    if (m < 5) {
        if (D_a6b2 != -1)
            sub_s014_19cd_0738();
        vec_1c39(m);
        return;
    }
    D_d845 = -1;
    D_e1bc = 0;
    if (D_c623 != D_11ca) {
        sub_root_0000_24bc((char *)0x64a, 0x96);
        return;
    }
    D_c762 = 0;
    i = D_a6b2;
    f = D_9c55[i].f10;
    k = D_e0a1 - 1;
    if (D_a6b0 == 1 || (D_a6b0 == -1 && (f == 1 || f == 0)))
        k = -1;
    else
        for (; k >= 0; k--)
            if ((unsigned char)D_d848[k] >= 5 && HIT(k, x, y)) {
                D_d73e = D_9c55[i];
                D_d845 = D_d848[k];
                D_9c55[i].tx = D_e1b4 = D_9c55[D_d845].x;
                D_9c55[i].ty = D_e1b6 = D_9c55[D_d845].y;
                D_9c55[i].target = D_d845;
                if (D_a6b0 == 0x100 || D_a6b0 == 0x80) {
                    D_e0a5 = D_9c55[i].x;
                    D_e0a3 = D_9c55[i].y;
                    D_e0a9 = D_9c55[i].tx;
                    D_e0a7 = D_9c55[i].ty;
                    if (sub_s014_1d69_000c(i, D_d845, 0x100) == 1) {
                        sub_s014_0e76_4694();
                        col = D_c2bd;
                    } else {
                        D_c7cc = 1;
                        col = D_c242;
                    }
                } else {
                    if (F_20a5_7900[i][D_d845] == 0) {
                        if (sub_s014_12f7_000a(i) != 1) {
                            sub_s014_0e76_4694();
                            col = D_c2bd;
                            goto show;
                        }
                        sub_s014_12f7_159c();
                    } else
                        D_c762 = 1;
                    D_c7cc = 1;
                    col = D_c242;
                }
show:
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], col, 0, D_d845);
                D_9c55[D_a6b2] = D_d73e;
                break;
            }
    if (D_d845 == -3 || k >= 0)
        return;
    if (!((D_a6b0 == -1 && f != 0x1082) || D_a6b0 == 1 || (D_a6b0 == 0x100 && D_e1c8 == 0)))
        return;
    /* click on the ground */
    D_d73e = D_9c55[i];
    D_e1b6 = MAPY(y);
    D_9c55[i].tx = D_e1b4 = D_e1b6 - F_20a5_4be0 + x + D_c26c - 16;
    D_9c55[i].ty = D_e1b6 = D_e1b6 + D_c293;
    D_9c55[i].target = -1;
    D_9c55[i].f19 = 0x14;
    if (D_a6b0 == 0x100 && (D_c600 = 1, D_e1c8 == 0)) {
        D_e0a5 = D_9c55[i].x;
        D_e0a3 = D_9c55[i].y;
        D_e0a9 = D_9c55[i].tx;
        D_e0a7 = D_9c55[i].ty;
        h = 0xf;
        D_e1c0 = 0xf;
        w = 0x15;
        col = D_c4db;
        if (sub_s014_1d69_000c(i, i, 0x100) == 1) {
            col = D_c621;
            goto blocked;
        }
    } else {
        if (sub_s014_12f7_000a(i) != 1) {
            h = 0x1a;
            D_e1c0 = 0x1a;
            w = 0x1b;
            col = D_c244;
blocked:
            D_d845 = -3;
            sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
            sub_s014_0e76_4694();
            goto restore;
        }
        sub_s014_12f7_159c();
        h = 0x1a;
        D_e1c0 = 0x1a;
        w = 0x1b;
        col = D_c28f;
    }
    D_d845 = -2;
    sub_s014_19cd_1764(x - 5, y - h, 0xb, w, col, -1, 0);
    D_c652 = D_9c55[D_a6b2].tx;
    D_c67d = D_9c55[D_a6b2].ty;
restore:
    if (D_a6b0 == 0x100)
        D_c600 = 0;
    D_9c55[D_a6b2] = D_d73e;
}

extern struct item6 { int id; unsigned char f2; unsigned char f3; unsigned char cnt; char f5; }
    far F_2e38_003e[][64];              /* party inventories */
extern struct { char f; int a; int b; char pad[5]; } far F_20a5_3355[];  /* furniture removal */
extern char far F_20a5_3350[];          /* [n*10] removal applies at once */
extern struct { char pad[4]; int a; int b; char c; char pad2; } far F_20a5_3382[]; /* walls */
extern struct { unsigned char col[4]; unsigned char row[4]; unsigned char f[4]; }
    far F_20a5_3236[];                  /* trap map patches */
extern char far F_20a5_47ee[][30];      /* activation records */
extern unsigned char D_d6cc;            /* their count */
extern char D_c6ef[], D_c678[], D_c64c[], D_c7cd[], D_c77e[], D_c8b2[], D_c784[], D_c7fe[];
extern char D_a6b4[];                   /* [13] floor */
extern char D_c81c[];                   /* [13] loot pile under the combatant */
extern char D_c80e;                     /* loot piles */
extern char D_c865;
extern char D_907b;                     /* leader */
extern char D_96a9, D_96bd, D_96d1, D_96e5, D_96f9, D_970d, D_9735, D_9749; /* menu enables */
extern int D_970b;                      /* menu text */
extern char D_ca99;
extern char D_d4e4, D_ca29, D_a893;
extern int D_ca3d, D_da4d, D_d9e4;
extern int D_c925, D_c978;
extern char D_c989, D_ca1b;
extern char D_0570[];                   /* " discovers a trap!" */
void far vec_1801(int, int, int, int);  /* trap handlers in root 09c0 */
void far vec_180b(int, int, int, int);
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
        sx[k] = D_9c55[k].x;
        sy[k] = D_9c55[k].y;
    }
    D_9c55[lead].tx = px;
    D_9c55[lead].ty = py;
    if (sub_s014_12f7_000a(lead) != 1)
        return 0;
    r = 1;
    if (D_c102 == 1) {
        /* formation: facing from the direction of the move */
        if ((int)sub_root_047c_1120((px - D_9c55[lead].x) / 2, py - D_9c55[lead].y, 0) < 49) {
            d = (px - D_9c55[lead].x) / 2;
            k = py - D_9c55[lead].y;
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
            sub_s014_12f7_1554(lead);
            path = F_20a5_6238[lead];
            if (path[D_e118 - 1] == -1)
                D_e118--;
            last = D_179c[(unsigned char)path[D_e118 - 1] >> 4];
            a1 = sub_root_0410_005a(D_179c[(unsigned char)path[D_e118 - 2] >> 4], last) & 0xff;
            a2 = sub_root_0410_005a(D_179c[(unsigned char)path[D_e118 - 3] >> 4], last) & 0xff;
            dir = sub_root_0410_004c(last, (a1 * 2 + 1) / 3) & 0xff;
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
        D_e0a3 = py;
        oy += py;
        D_e0a5 = px;
        ox += px;
        r = sub_s014_19cd_164a(ox, oy, dir, lo, hi);
        if (r == 0)
            r = sub_s014_19cd_164a(ox, oy, sub_root_0410_005a(dir, 1) & 0xff, lo, hi);
        if (r == 0)
            r = sub_s014_19cd_164a(ox, oy, sub_root_0410_004c(dir, 1) & 0xff, lo, hi);
    }
    /* the placed positions become destinations */
    for (k = lo; k < hi; k++) {
        D_9c55[k].tx = D_9c55[k].x;
        D_9c55[k].ty = D_9c55[k].y;
        D_9c55[k].x = sx[k];
        D_9c55[k].y = sy[k];
    }
    if (r == 0)
        for (k = lo; k < hi; k++)
            D_a6d3[k] = 0;
    return r;
}

/* place the marching order in formation `dir` around (px, py); 1 if every spot is free */
int far sub_s014_19cd_164a(int px, int py, int dir, int lo, int hi)
{
    int k, row, m;

    for (k = 0; k < D_c629; k++) {
        row = (k - lo) >> 1;
        m = D_c624[k];
        D_9c55[m].x = D_1b9a[dir] + row * D_1baa[dir] + px + ((k & 1) == 1 ? D_1ba2[dir] : 0);
        D_9c55[m].y = D_1bb2[dir] + row * D_1bc2[dir] + ((k & 1) == 1 ? D_1bba[dir] : 0) + py;
        if (sub_s014_19cd_1714(m) == 0)
            return 0;
    }
    return 1;
}

/* 1 if combatant i's (trial) position is free and visible from the clicked point */
int far sub_s014_19cd_1714(int i)
{
    if (sub_s014_1462_000a(&D_9c55[i]) == 0) {
        D_e0a9 = D_9c55[i].x;
        D_e0a7 = D_9c55[i].y;
        if (sub_s014_1d69_000c(-1, 0, 0) == 0)
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
    } else {
        D_e1c2 = D_d505[sub_s014_14f7_343c(who)][D_9c55[who].f1a] + x - 5;
        D_e1be = D_d5d3[sub_s014_14f7_343c(who)] - D_d5de[sub_s014_14f7_343c(who)] + y - 5;
        h = 10;
        D_e1b8 = 10;
    }
    D_e1ba = h;
    D_e1bc = col;
}

/* keep the cursors on the moving combatants */
void far sub_s014_19cd_17f6(void)
{
    int k, f, col;

    if (D_a6b3 == 1) {
        for (k = D_e0a1 - 1; k >= 0; k--)
            if (D_d848[k] == D_a6b2) {
                MEMBER_CURSOR(k);
                D_e1c6 = 8;
                D_e1c4 = 0x1a;
                break;
            }
    }
    if (D_d845 >= 0) {
        for (k = D_e0a1 - 1; k >= 0; k--)
            if (D_d848[k] == D_d845)
                break;
        if (k >= 0) {
            col = D_e1bc;
            f = D_9c55[D_a6b2].f10;
            if (D_a6b0 == 0x80 || D_a6b0 == 0x100 || (D_a6b0 == -1 && (f == 0x80 || f == 0x100))) {
                D_e0a5 = D_9c55[D_a6b2].x;
                D_e0a3 = D_9c55[D_a6b2].y;
                D_e0a9 = D_9c55[D_d845].x;
                D_e0a7 = D_9c55[D_d845].y;
                col = sub_s014_1d69_000c(D_a6b2, D_d845, 0x80) == 0 ? D_c242 : D_c2bd;
            }
            D_e1be = D_d86f[k];
            sub_s014_19cd_1764(D_d889[k], D_e1be, D_d862[k], D_d855[k], col, 0, D_d845);
        } else
            D_e1bc = 0;
    }
}

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
    if (w + x - 1 > 0x103 && (D_e1c6 = 0x104 - x) < 0)
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

    c = &D_9c55[D_a6b2];
    if (c->target == -1) {
        if (D_a6d3[D_a6b2] != 1 && c->f10 != 0x100 && c->f10 != 1) {
            D_d845 = -1;
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
        D_d845 = -2;
        sub_s014_19cd_1764(0, 0, 0xb, w, col, -1, 0);
    } else {
        for (k = 0; k < D_e0a1; k++)
            if (c->target == (unsigned char)D_d848[k]) {
                D_d845 = D_d848[k];
                sub_s014_19cd_1764(D_d889[k], D_d86f[k], D_d862[k], D_d855[k], D_c242, 0, D_d845);
                return;
            }
    }
}

/* best lockpick (item 99) of member i -> D_c7cd[i] */
#define FIND_PICK(i) { int n_; unsigned char b_ = 100; D_c7cd[i] = -1; \
    for (n_ = 0; n_ < D_9c55[i].nitems; n_++) \
        if (F_2e38_003e[i][n_].id == 99 && F_2e38_003e[i][n_].f3 < b_) { \
            D_c7cd[i] = n_; b_ = F_2e38_003e[i][n_].f3; } }

/* door under combatant i (flag 2) and the leader's door menu entries */
void near sub_s014_19cd_1ade(int i)
{
    int k;
    unsigned lo, hi, v;
    long far *cell;
    char lock;

    for (k = 0; k < F_20a5_1f52; k++) {
        if (F_20a5_1f54[k].x <= 0)
            continue;
        if (INSIDE(F_20a5_1f54[k], D_9c55[i].x, D_9c55[i].y)) {
            cell = &F_20a5_0000[(F_20a5_1f54[k].f8 & 0x3f) * 40 + (F_20a5_1f54[k].f9 & 0x3f)];
            lo = ((unsigned far *)cell)[0];
            hi = ((unsigned far *)cell)[1];
            if ((F_20a5_1f54[k].f8 & 0x80) == 0x80)
                D_c6ef[i] = 0;
            else if ((F_20a5_1f54[k].f8 & 0x40) == 0x40)
                D_c6ef[i] = 1;
            else if ((F_20a5_1f54[k].f9 & 0x80) == 0x80)
                D_c6ef[i] = 2;
            else if ((F_20a5_1f54[k].f9 & 0x40) == 0x40)
                D_c6ef[i] = 3;
            switch (D_c6ef[i]) {        /* wall of the door cell on that side */
            case 0: v = hi & 0xff; break;
            case 1: v = lo & 0xff; break;
            case 2: v = hi >> 8; break;
            case 3: v = lo >> 8; break;
            }
            v &= 0xf;
            D_a696[i][0] |= 2;
            D_c678[i] = (int)v < 8 ? 1 : 0;   /* door open */
            D_c64c[i] = k;
            break;
        }
        D_a696[i][0] &= 0xfd;
    }
    if (D_907b != i || D_a695 != 0)
        return;
    if ((D_a696[i][0] & 2) == 2) {
        lock = F_20a5_1f54[D_c64c[i]].f7;
        if (lock != 0 && D_a6c1 == 0) {
            D_96a9 = 2;
            D_96f9 = 2;
            D_96e5 = 1;
            D_ca99 = 1;
            FIND_PICK(i);
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
        if ((D_a696[i][0] & 4) != 0)
            return;
    }
    D_96f9 = 2;
    D_96e5 = 2;
}

/* remove activation record k */
void far sub_s014_19cd_1d5e(int k)
{
    int n;
    char far *p;

    if (D_d6cc != 0) {
        if (k < D_d6cc - 1)
            for (p = F_20a5_47ee[k], n = D_d6cc - k - 1; n != 0; n--, p += 30)
                _fmemcpy(p, p + 30, 30);
        D_d6cc--;
    }
}

/* stairs under combatant i (flag 1) */
void near sub_s014_19cd_1db8(int i)
{
    int k;

    for (k = 0; k < F_20a5_2fb6; k++) {
        if (INSIDE(F_20a5_2fb8[k], D_9c55[i].x, D_9c55[i].y)) {
            D_c77e[i] = k;
            D_a696[i][0] |= 1;
            break;
        }
        D_a696[i][0] &= 0xfe;
    }
    if (D_907b == i && D_a695 == 0) {
        if (D_c623 != D_11ca) {
            D_96bd = 1;
            D_c77e[i] = D_c8b2[i];
            return;
        }
        D_96bd = D_a696[i][0] & 1;
        if (D_96bd != 1)
            D_96bd = 2;
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
        if (INSIDE(F_20a5_300e[k], D_9c55[i].x, D_9c55[i].y)) {
            D_c784[i] = k;
            D_a696[i][0] |= 4;
            break;
        }
        D_a696[i][0] &= 0xfb;
    }
    if (D_907b != i || D_a695 != 0)
        return;
    if ((D_a696[i][0] & 4) == 4) {
        D_96d1 = 2;
        D_96e5 = 2;
        D_96f9 = 2;
        D_970d = 2;
        r = &F_20a5_300e[(unsigned char)D_c784[i]];
        if ((r->lock & 0x7f) == 0 || D_a6c1 != 0) {
            if (r->acid == 0 || D_a6c1 != 0) {
                if ((r->lock & 0x7f) == 0 && r->acid == 0 && D_a6c1 == 0)
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
        if ((D_a696[i][0] & 2) == 0) {
            D_96e5 = 2;
            D_96f9 = 2;
            D_970d = 2;
        }
    }
}

/* exit zone under combatant i (flag 0x20) */
void near sub_s014_19cd_209a(int i)
{
    int k;

    for (k = 0; k < F_20a5_3326; k++) {
        if (F_20a5_3328[k].x <= 0)
            continue;
        if (D_a6b4[i] == D_11ca && F_20a5_3328[k].x <= D_9c55[i].x
         && D_9c55[i].x < F_20a5_3328[k].w + F_20a5_3328[k].x
         && F_20a5_3328[k].y <= D_9c55[i].y && D_9c55[i].y < F_20a5_3328[k].y + F_20a5_3328[k].h) {
            D_c7fe[i] = k;
            D_a696[i][0] |= 0x20;
            break;
        }
        D_a696[i][0] &= 0xdf;
    }
    if (D_907b == i && D_a695 == 0)
        D_9749 = (D_a696[i][0] & 0x20) == 0x20 ? 1 : 2;
}

/* trigger areas: call the handler of every area combatant i stands in
   (enemies only spring the two trap handlers vec_180b / vec_1801) */
void near sub_s014_19cd_216a(int i)
{
    int k;

    for (k = 0; k < F_20a5_2184; k++)
        if (F_20a5_2186[k].x > 0 && INSIDE(F_20a5_2186[k], D_9c55[i].x, D_9c55[i].y)
         && (i < 5 || F_20a5_2186[k].fn == vec_180b || F_20a5_2186[k].fn == vec_1801))
            (*F_20a5_2186[k].fn)(i, k, F_20a5_2186[k].a, F_20a5_2186[k].b);
}

/* loot pile next to combatant i */
void near sub_s014_19cd_2232(int i)
{
    int x, y, k;

    D_c81c[i] = -1;
    x = D_9c55[i].x;
    y = D_9c55[i].y;
    for (k = 0; k < (unsigned char)D_c80e; k++)
        if (F_20a5_59c8[k].level == D_11ca && F_20a5_59c8[k].cnt != 0
         && x - 16 <= F_20a5_59c8[k].x && F_20a5_59c8[k].x < x + 16
         && y - 16 <= F_20a5_59c8[k].y && F_20a5_59c8[k].y < y + 16) {
            D_c81c[i] = k;
            break;
        }
    D_c865 = D_c81c[i] != -1;
    if (D_907b == i && D_a695 == 0)
        D_9735 = D_c865 && D_a6c1 == 0 ? 1 : 2;
}

/* trigger: pit / trapdoor - fires when the whole party stands in area k */
void far sub_s014_19cd_2312(int i, int k, int n, int b)
{
    int m;

    for (m = 0; m < 5; m++)
        if (D_9c55[m].status == 1 && !INSIDE(F_20a5_2186[k], D_9c55[m].x, D_9c55[m].y))
            return;
    F_20a5_2186[k].x = -1;
    D_d4e4 = 1;
    sub_s014_0e76_4188(F_20a5_3355[n].a, F_20a5_3355[n].b, F_20a5_3355[n].f);
    D_ca29 = 1;
    D_a893 = 6;
    D_ca3d = b;
    sub_root_0000_24bc("Aaaahh!", 60);
}

/* trigger: remove furniture n (alarm) */
void far sub_s014_19cd_23f6(int i, int k, unsigned char n, int b)
{
    if (D_a6c1 != 1) {
        D_ca3d = b;
        if (F_20a5_3350[n * 10] == 1)
            vec_1c4d(n, k);
        D_a893 = 7;
        if (D_ca29 == 0) {
            D_da4d = D_07d4;
            D_d9e4 = D_07d4 + 1000;
            D_ca29 = 1;
        }
        sub_s014_0e76_4188(F_20a5_3355[n].a, F_20a5_3355[n].b, F_20a5_3355[n].f);
        D_c925 = F_20a5_2186[k].x;
        D_c978 = F_20a5_2186[k].y;
        D_c989 = F_20a5_2186[k].w;
        D_ca1b = F_20a5_2186[k].h;
    }
}

/* trigger: remove wall n */
void far sub_s014_19cd_24a8(int i, int k, unsigned char n, int b)
{
    if (D_a6c1 != 1) {
        D_ca3d = b;
        if (F_20a5_3382[n].pad[0] == 1)
            vec_1c57(n, k);
        D_a893 = 7;
        if (D_ca29 == 0) {
            D_da4d = D_07d4;
            D_d9e4 = D_07d4 + 1000;
            D_ca29 = 1;
        }
        sub_s014_0e76_4188(F_20a5_3382[n].a, F_20a5_3382[n].b, F_20a5_3382[n].c);
        D_c925 = F_20a5_2186[k].x;
        D_c978 = F_20a5_2186[k].y;
        D_c989 = F_20a5_2186[k].w;
        D_ca1b = F_20a5_2186[k].h;
    }
}

/* trigger: hidden trap - member i with perception above `skill` discovers it and the trap
   is drawn into the map (4 patches of trap n) */
void far sub_s014_19cd_255a(int i, int k, int n, int skill)
{
    char msg[60];
    int m, sh;
    unsigned v, f;
    long far *cell;

    if (D_9c55[i].f60 <= skill)
        return;
    F_20a5_2186[k].x = -F_20a5_2186[k].x;
    for (m = 0; m < 4; m++) {
        v = F_20a5_3236[n].col[m];
        if (v == 0)
            continue;
        f = F_20a5_3236[n].f[m];
        if (f < 0x80)
            F_20a5_1900[F_20a5_3236[n].row[m] + v * 40] |= f;
        else {
            sh = (v & 0x80) == 0 ? 0xc : 0x14;
            cell = &F_20a5_0000[F_20a5_3236[n].row[m] + (v & 0x3f) * 40];
            *cell |= (long)(f & 0x7f) << sh;
        }
    }
    strcpy(msg, D_9c55[i].name);
    strcat(msg, D_0570);
    sub_root_0000_24bc(msg, 150);
    vec_1c2f(-1);
    memset(D_a6d3, 0, 5);
}

/* per tick: locations of the party (door / stairs / region / exit flags) and triggers */
void far sub_s014_19cd_26a4(void)
{
    int i, k, m;
    unsigned char old, was;

    for (i = 0; i < 5; i++) {
        if (D_9c55[i].status != 1)
            continue;
        was = (D_a696[i][0] & 2) >> 1;
        sub_s014_19cd_1ade(i);
        if (was == 0 && (D_a696[i][0] & 2) == 2) {
            D_a696[i][0] &= 0xda;
            goto entered;
        }
        if (was == 1 && (D_a696[i][0] & 2) == 0) {
            D_a696[i][0] &= 0xda;
            goto left;
        }
        old = D_a696[i][0];
        sub_s014_19cd_1db8(i);
        if ((old & 1) == 0 && (D_a696[i][0] & 1) == 1) {
            D_a696[i][0] &= 0xd9;
            goto entered;
        }
        if ((old & 1) == 1 && (D_a696[i][0] & 1) == 0) {
            D_a696[i][0] &= 0xd9;
            goto left;
        }
        was = (D_a696[i][0] & 4) >> 2;
        sub_s014_19cd_1e92(i);
        if (was == 0 && (D_a696[i][0] & 4) == 4) {
            D_a696[i][0] &= 0xdc;
            goto entered;
        }
        if (was == 1 && (D_a696[i][0] & 4) == 0) {
            D_a696[i][0] &= 0xdc;
            goto left;
        }
        was = (D_a696[i][0] & 0x20) >> 5;
        sub_s014_19cd_209a(i);
        if (was == 0 && (D_a696[i][0] & 0x20) == 0x20) {
            D_a696[i][0] &= 0xf8;
            goto entered;
        }
        if (was == 1 && (D_a696[i][0] & 0x20) == 0) {
            D_a696[i][0] &= 0xf8;
            goto left;
        }
        sub_s014_19cd_216a(i);
        D_a696[i][0] &= 0xf7;
        continue;
entered:
        D_a696[i][0] |= 8;
left:
        sub_root_0000_1716(i);
    }
    for (i = 5; i < 13; i++)
        if (D_9c55[i].status == 1)
            sub_s014_19cd_216a(i);
    /* enemies open closed, unlocked doors they walk into */
    for (k = 0; k < F_20a5_1f52; k++)
        if (F_20a5_33d3[k] == 0 && F_20a5_1f54[k].f7 == 0 && F_20a5_1f54[k].x > 0)
            for (m = 5; m < 13; m++)
                if (D_9c55[m].status == 1 && INSIDE(F_20a5_1f54[k], D_9c55[m].x, D_9c55[m].y)) {
                    vec_1d6f(k, 1, 1);
                    vec_1c2f(-1);
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
extern char far *D_c103;                /* loot file image */
extern struct loot D_c291[];            /* near copy of the list */
extern int D_c7a4[20];
extern char D_c96b[13];
extern char D_9085[];
extern char D_9429, D_943d, D_9893;
extern int D_d6ee, D_d6f2, D_d6f9, D_d729, D_d70a, D_d73c, D_d726, D_d800;  /* level buffers */
extern int D_07d8;
extern char D_9069, D_9c4c;
extern int D_ca55, D_ca53, D_9c42, D_a68f, D_a691, D_a693;
extern int D_9920, D_9934;
extern char *D_11bc[6];
extern int D_ca45[];
extern int D_a2d7[5];                   /* shield pictures of the party */
extern int D_e1cb[5];
struct i5 { int v[5]; };
struct b13 { char b[13]; };
int far _alloc_buffer(unsigned, int, char *);
void far _free_buffer(int, char *);
void far sub_root_0410_031e(int, int);  /* save palette */
void far sub_root_0410_0364(int, int);  /* restore palette */
void far sub_root_05f8_0117(int, int, int);
extern int D_940b;
void far sub_root_047c_12c3(void);
void far sub_root_047c_124f(int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_04ea(int, int);
void far sub_root_047c_1293(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_0545(void);
void far sub_root_047c_04f8(void);
int far sub_root_0617_011e(char *, char far *);      /* load a file, returns its size */
void far sub_root_0617_016a(char *, char far *, int); /* write a file */
int far sub_root_0617_045c(char *);                   /* load a picture */
void far sub_root_06a1_3108(char far *, char far *, int); /* far memmove */
void far sub_s014_0e76_38b6(int, int);
void far sub_s014_19cd_2e50(struct loot far *, unsigned *, void far *, int);
int far sub_s014_19cd_3204(struct loot *, int);
void far sub_s014_19cd_3258(char *, int *, int, int);
void far sub_s014_19cd_33c0(struct loot far *, int);
void far sub_s014_19cd_36f0(void);
void far sub_s014_19cd_3766(void);
typedef struct _iobuf FILE;
FILE *fopen(const char *, const char *);
int fseek(FILE *, long, int);
long ftell(FILE *);
unsigned fwrite(const void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
#define CURSOR_BUSY() sub_root_05f8_0117(3, 10, D_940b)
#define CURSOR_IDLE() sub_root_05f8_0117(3, 3, D_940b)

/* reset the order menu */
void far sub_s014_19cd_28a2(void)
{
    struct menu *m;

    D_9433 = 2;
    for (m = D_97dd_; m < D_97dd_ + 6; m++)
        m->on = 2;
    D_9609 = 2;
    D_961d = 2;
    D_9645 = D_c623 == D_11ca ? 1 : 2;
    strcpy(D_06ab + 8, D_073e);
    D_9659 = 2;
    D_96a9 = 2;
    D_96bd = 2;
    strcpy(D_06bb + 10, D_c102 == 0 ? D_073e : D_0744);
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
    strcpy(D_0727 + 16, D_c102 == 0 ? D_073e : D_0744);
    D_975d = 2;
}

/* build the order menu of the selected member */
void far sub_s014_19cd_29b8(void)
{
    struct menu *m;
    int a;
    unsigned char fl, lock;
    struct region far *r;

    sub_s014_19cd_28a2();
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
    a = D_a6b2;
    if (D_c623 != D_11ca) {
        D_96bd = 1;
        D_9645 = 2;
        D_9659 = 2;
        D_966d = 2;
        D_c77e[a] = D_c8b2[a];
        return;
    }
    D_9645 = 1;
    strcpy(D_c107, "\x02");
    strcat(D_c107, D_9c55[a].name);
    strcat(D_c107, D_11f2);
    D_961d = 1;
    D_9735 = D_c865 == 1 && D_a6c1 == 0 ? 1 : 2;
    if (D_a6c1 == 1) {
        D_9433 = 1;
        for (m = D_97dd_; m < D_97dd_ + 6; m++)
            m->on = 1;
        D_d7ff = 1;
        vec_1da1(a);
        a = D_a6b2;
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
        if (D_9c55[a].weapon == -1 || sub_s014_18bf_0c02(a) == 0)
            D_9841 = 2;                 /* no missile weapon / ammo: no "fire" */
    }
    a = D_a6b2;
    fl = D_a696[a][0];
    if ((fl & 2) == 2) {
        lock = F_20a5_1f54[(unsigned char)D_c64c[a]].f7;
        if (lock == 0 || D_a6c1 != 0)
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
    if ((fl & 4) == 4 && D_a6c1 == 0) {
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
void far sub_s014_19cd_2d08(int i)
{
    int ofs[2];
    char cnt;
    int k;

    if (D_1b98 != 1)
        return;
    if (D_c81c[i] < 0) {
        cnt = -D_c81c[i];
        ofs[0] = D_d846;
    } else {
        ofs[0] = F_20a5_59c8[D_c81c[i]].ofs;
        cnt = F_20a5_59c8[D_c81c[i]].cnt;
    }
    if (cnt == 0)
        return;
    D_07d3++;
    sub_s014_19cd_3258(&cnt, ofs, 1, 0);
    sub_s014_19cd_2e50(D_c654, &D_c67f, &F_2e38_0bb0, i);
    F_2e38_0bb0.a = 0;
    F_2e38_0bb0.b = 0;
    F_2e38_0bb0.c = 0;
    if (D_c80e != 0) {
        if (D_c67f != 0) {
            CURSOR_BUSY();
            _fmemcpy(D_c291, D_c654, D_c67f * 6);
            CURSOR_IDLE();
            ofs[0] = sub_s014_19cd_3204(D_c291, D_c67f);
        }
        ofs[1] = 0;
        for (k = 0; k < (unsigned char)D_c80e; k++)
            if (F_20a5_59c8[k].ofs == -1) {
                F_20a5_59c8[k].cnt = D_c67f;
                F_20a5_59c8[k].ofs = ofs[0];
            }
    }
    vec_1c2f(D_17b0);
    sub_s014_14f7_0008();
    D_07d3--;
}

/* loot screen: free the combat buffers, run the s040 screen, restore everything */
void far sub_s014_19cd_2e50(struct loot far *list, unsigned *cnt, void far *x, int who)
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
    seq = *(struct b13 *)D_c96b;
    for (k = 0; k < 13; k++)
        kinds[k] = D_9c55[k].f1b;
    strcpy(D_9085, "Seq dealloc");
    s9429 = D_9429;
    s9433 = D_9433;
    s943d = D_943d;
    s9893 = D_9893;
    D_9429 = 2;
    D_9433 = 2;
    D_943d = 2;
    D_9893 = 2;
    save65f = D_d65f;
    save67e = D_d67e;
    vec_1cb1();
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
        sub_root_0410_031e(0, D_ca55);
    }
    seg = _alloc_buffer(6000, 0, "loot");
    tmp = (struct loot far *)((unsigned long)seg << 16);
    CURSOR_BUSY();
    _fmemcpy(tmp, list, *(unsigned char *)cnt * 6);
    CURSOR_IDLE();
    if (D_9c4c == 1) {
        sub_root_047c_12c3();
        sub_root_047c_124f(0, 0, D_9c42);
        D_a68f = D_9c42;
    }
    sub_s014_19cd_36f0();
    sub_root_047c_0568();
    sub_root_0000_1716(0x100);
    D_9920 = 2;
    D_9934 = 2;
    vec_1dab(0, seg, (unsigned char *)cnt, (unsigned)(unsigned long)x, (unsigned)((unsigned long)x >> 16), who);
    D_9934 = 1;
    sub_root_047c_1320(6, 0);
    D_c62a = 0;
    if (D_9069 == 0) {
        sub_root_0410_0364(D_ca53, D_ca55);
        _free_buffer(D_ca55, "Pal Buf");
    }
    CURSOR_BUSY();
    _fmemcpy(list, tmp, *(unsigned char *)cnt * 6);
    CURSOR_IDLE();
    _free_buffer(seg, "loot");
    sub_s014_19cd_3766();
    vec_1ae5();
    if (reload == 1)
        vec_1dbf();
    if (D_9c4c == 1) {
        sub_root_047c_12c3();
        sub_root_047c_124f(0, 0, D_a695 == 0 ? D_a693 : D_a691);
        D_a68f = D_9c42;
        sub_root_047c_04ea(0, 0);
        sub_root_047c_1293();
    }
    D_07d8 = 1;
    for (k = 0; k < 6; k++)
        strcpy(D_11bc[k], "");
    for (k = 5; k < 13; k++)
        if (D_9c55[k].status != 0) {
            if (D_ca45[D_9c55[k].kind] == 0)
                vec_1db5(D_9c55[k].kind);
            sub_s014_0e76_38b6(k, ((char *)&D_9c55[k])[0x18]);
        }
    D_9429 = s9429;
    D_9433 = s9433;
    D_943d = s943d;
    D_9893 = s9893;
    D_d65f = save65f;
    D_d67e = save67e;
    *(struct b13 *)D_c96b = seq;
    for (k = 0; k < 13; k++)
        D_9c55[k].f1b = kinds[k];
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
    size = sub_root_0617_011e(D_1162, D_c103);
    for (k = 0; k < n; k++) {
        if (ofs[k] == -1)
            continue;
        p = D_c103 + ofs[k];
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
        for (j = 0; j < (unsigned char)D_c80e; j++) {
            if (F_20a5_59c8[j].ofs == ofs[k])
                F_20a5_59c8[j].ofs = -1;
            if (ofs[k] < F_20a5_59c8[j].ofs)
                F_20a5_59c8[j].ofs -= c * 6;
        }
        size -= c * 6;
    }
    sub_root_0617_016a(D_1162, D_c103, size);
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
void far sub_s014_19cd_34a0(void)
{
    int ofs[180];
    char cnt[180];
    unsigned char idx[180];
    int n, a, b, k, o;
    struct loot far *d;
    struct loot far *s;

    n = 0;
    for (k = D_c83e; k < (unsigned char)D_c80e; k++)
        if (F_20a5_59c8[k].ofs != -2) {
            ofs[n] = F_20a5_59c8[k].ofs;
            idx[n] = k;
            cnt[n] = F_20a5_59c8[k].cnt;
            n++;
        }
    for (a = 0; a < n - 1; a++)
        for (b = a + 1; b < n; b++)
            if (ofs[a] == ofs[b]) {
                F_20a5_59c8[idx[b]].ofs = -1;
                ofs[b] = -1;
            }
    CURSOR_BUSY();
    for (k = 0, s = F_20a5_3954, d = D_c654; k < D_c783; k++, s++, d++)
        *d = *s;
    CURSOR_IDLE();
    D_c67f = D_c783;
    if (n != 0)
        sub_s014_19cd_3258(cnt, ofs, n, D_c67f);
    D_c81c[D_907b] = 0;
    if (n < 1 && D_c783 == 0)
        D_1b98 = 0;
    else {
        D_1b98 = 1;
        CURSOR_BUSY();
        _fmemcpy(D_c291, D_c654, D_c67f * 6);
        CURSOR_IDLE();
        o = sub_s014_19cd_3204(D_c291, D_c67f);
        for (k = 0; k < (unsigned char)D_c80e; k++)
            if (F_20a5_59c8[k].ofs == -1) {
                F_20a5_59c8[k].cnt = D_c67f;
                F_20a5_59c8[k].ofs = o;
                D_c81c[D_907b] = k;
                break;
            }
        if ((unsigned char)D_c80e == k) {
            D_c81c[D_907b] = -D_c783;
            D_d846 = o;
        }
    }
    D_c83e = D_c80e;
    D_c783 = 0;
}

/* load the shield pictures of the party ("pics\shielda.pic", letter = record +0x15) */
void far sub_s014_19cd_36f0(void)
{
    char name[32];
    char *c;
    int *p;

    *(struct i5 *)D_e1cb = *(struct i5 *)D_a2d7;
    sub_root_047c_0545();
    for (c = &D_9c55[0].status, p = D_a2d7; p < D_a2d7 + 5; c += 0x80, p++) {
        *p = 0;
        if (*c != 0) {
            strcpy(name, "pics\\shielda.pic");
            name[11] = c[1];
            *p = sub_root_0617_045c(name);
        }
    }
    sub_root_047c_04f8();
}

/* free the shield pictures */
void far sub_s014_19cd_3766(void)
{
    int k;

    for (k = 0; k < 5; k++)
        if (D_a2d7[k] != 0) {
            _free_buffer(D_a2d7[k], "RSM");
            break;
        }
    *(struct i5 *)D_a2d7 = *(struct i5 *)D_e1cb;
}
