/* place: s014 14f7:0008 */
/* orig: size=3c7a CONST=7d50+002e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: CONST=7d50 */
/* s014 14f7: COMBAT RENDERER.
   0008 draws one frame: lays out effect puffs (3682/3a46), positions combatants (31e6) and
   missiles (3404), erases last frame's logged sprites, queues map pieces that changed
   (1a6a), effects (07ec), combatants (098e), loot piles (118e), cursors and missiles,
   re-queues the dirty tiles (20d6), sorts list C (2f96), rebuilds the mouse hot spots,
   blits (0c92) and copies the dirty boxes to the screen (0e9e). Lists:
   through the root graphics library (047c_04ef/0553/0952), then effects, cursors and HUD.
     list A  D_e15e (14-byte entries, count D_e161 <= 100)    floor decals   (1fd4 adds)
     list B  D_e164 (0x26-byte entries, count D_e166 <= 0x8a) walls/furniture (2052 adds,
             304a quicksorts by depth keys +0e/+10/+12)
     list C  sprite records in segment 20a5 pointed to by F_20a5_34b4[] (count D_e162 < 157):
             combatants, missiles, effects (1f14 / 13ae add, 2f96 insertion-sorts)
   Dirty tiles: the 0x104 x 0xc8 view is cut into 14 x 10 tiles; D_e16a/D_e16c/D_e16e/D_e170
   (_based(D_d6f5)) hold each tile's dirty box (min x, min y, max x, max y); queued sprites
   get the box of their tile as clip window, 13ae grows the boxes of the tiles a sprite covers.
     098e   queue every visible combatant (body, wound flashes, status icons)
     07ec   queue effect clouds / area effects (s019 lists D_a2eb, D_a342, D_a396)
     0c92/0e9e  blit lists A/B/C and the effects, age the effect lists
     31e6   screen position of every combatant (oblique projection x - y + 0x88, y + 0x69)
     3404   screen position of the missiles
     343c   animation set of combatant i: kind, +2 (pose 2/3) or +4 (pose 4) for enemy slots
     347c/34d6/35ce/359a  clip-and-blit helpers (mouse cursor hidden around the blit)
     1e70   clip a rectangle to the 0..0x103 x 0..0xc7 view
   Sprite record +1c..+22 (e0..e3) hold the mouse hot spot box of a combatant sprite
   (x0, x1, y0, y1), or an effect kind >= 28000 (30000 member cursor, 31000 target cursor,
   31500 = 0x7b0c missiles/ground cursor, 32000 floor, 0x7d01.. walls).
   status: 27/27 written, 6 match (1e70 343c 347c 359a + 2). 3404 is hand-written asm in the
   original (`loop`), C equivalent here. */

struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
    int scrx;                   /* +00 screen x of the sprite */
    int scry;                   /* +02 screen y */
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[6];
    int *info;                  /* +0e animation info: sprite [frame * 8 + facing] */
    int f10;                    /* +10 action */
    char pad12[2];
    char status;                /* +14 */
    char f15;
    char kind;                  /* +16 animation slot */
    char pad17;
    char pose;                  /* +18 2/3 = down, 4 = dead? */
    char f19;                   /* +19 facing */
    char f1a;                   /* +1a facing (sprite) */
    char f1b;                   /* +1b animation frame */
    char pad1c[0x2e];
    char target;                /* +4a */
    char pad4b[0x35];
};
struct rect { int f0; int x; int y; int w; int h; };      /* blit window (D_01a4) */
struct drawa { int spr; int x; int y; int ox; int oy; int w; int h; };        /* 14 bytes */
struct drawb { int spr; int x; int y; int ox; int oy; int w; int h;
               int k0e; int k10; int k12; char pad[0x12]; };                  /* 0x26 bytes */
struct spr {                    /* queued sprite, 0x26 bytes (records in DGROUP) */
    int spr;                    /* +00 sprite data */
    int sx;                     /* +02 screen position */
    int sy;                     /* +04 */
    int cx;                     /* +06 clipped box */
    int cy;                     /* +08 */
    int w;                      /* +0a */
    int h;                      /* +0c */
    int x;                      /* +0e depth keys: map x, y - height, y + height */
    int k10;                    /* +10 */
    int k12;                    /* +12 */
    int sx2;                    /* +14 */
    int sy2;                    /* +16 */
    int pad18[2];
    int e0;                     /* +1c depth extents (or effect kind >= 28000) */
    int e1;                     /* +1e */
    int e2;                     /* +20 */
    int e3;                     /* +22 */
    char who;                   /* +24 combatant (-1) */
    char pad25;
};
struct dirty { int x; int y; int w; int h; };
struct rlog { int x; int y; int sx; int sy; int kind; char h; char w; char hh; char dx0;
              char b13; char dy0; char b14; char dw; char dh; char pad; };   /* 0x14 bytes */

extern _segment D_d6f5;
extern struct drawa _based(D_d6f5) *D_e15e;
extern struct drawb _based(D_d6f5) *D_e164;
extern struct drawa _based(D_d6f5) *D_e167;
extern int _based(D_d6f5) *D_e16a;      /* per sprite: x offset */
extern int _based(D_d6f5) *D_e16c;      /* y offset */
extern int _based(D_d6f5) *D_e16e;      /* width */
extern int _based(D_d6f5) *D_e170;      /* height */
extern unsigned char D_e161, D_e166, D_e169;
extern int D_e162;
extern unsigned far F_20a5_34b4[];      /* near pointers to the sprite records (list C) */
extern struct dirty _based(D_d6f5) *D_e172;  /* dirty boxes of this frame */
extern int D_e174;
extern int D_e176;
extern struct rlog far F_20a5_7354[];   /* replay log */
extern struct rect *D_0154;             /* view window */
/* per screen tile (0x8c = 14 x 10) */
extern char far F_20a5_36c8[];          /* map row of the tile */
extern char far F_20a5_363c[];          /* map column */
extern int far F_20a5_45be[];           /* screen x of the west wall */
extern int far F_20a5_4b4a[];           /* screen x */
extern int far F_20a5_46d6[];           /* screen y of the walls */
extern int far F_20a5_415e[];           /* screen y of the floor */
extern int far F_20a5_6eea[];           /* west wall sprites [n][2] */
extern int far F_20a5_6f2a[];           /* north wall sprites [n][2] */
extern int far F_20a5_3474[];           /* floor object sprites [n][2] */
extern char D_c76e[], D_c789[], D_c7de[], D_c7ee[];   /* west wall piece: dx, dy, w, h */
extern char D_c821[], D_c855[], D_c876[], D_c887[];   /* north wall piece */
extern char D_1ace[14], D_1adc[14];
struct tdesc {                          /* what is drawn in a screen tile, 0x12 bytes */
    int floor;                          /* floor sprite (-1: tile outside the map) */
    unsigned wwall;                     /* west wall piece */
    int nback;                          /* north wall back */
    unsigned nwall;                     /* north wall piece */
    int ewall;                          /* east wall */
    unsigned furn;                      /* furniture: id | orientation << 4 */
    int swall;                          /* south wall */
    int decal;                          /* floor decal (list A) */
    unsigned obj;                       /* floor object (list B) */
};
extern struct tdesc far F_20a5_4eb8[0x8c];
extern int far F_20a5_4046[], F_20a5_491a[], F_20a5_4a32[];
extern unsigned char D_c12e, D_c17f, D_c12d;     /* first piece/object ids that are drawn */
extern char D_c11d[];                   /* furniture height */
extern char D_d7be[], D_d802[];         /* furniture box offsets [f*4 + o] */
extern char D_c13f[], D_c181[];         /* furniture sprite offsets [f*4 + o] */
extern int D_c8d5[];                    /* furniture sprites */
/* effect lists of s019 */
struct efa { int x; int y; unsigned char f4; unsigned char kind; char f6; char level; unsigned t; };
struct efb { int x; int y; unsigned char f4; char f5; char level; char f7; unsigned t; };
struct efc { int x; int y; unsigned char f4; char f5; char level; char f7; };
extern struct efa D_a2eb[];
extern struct efb D_a342[];
extern struct efc D_a396[];
extern char D_a33d, D_a395, D_a531;
extern int far F_20a5_3bc0[8][16], F_20a5_3cc0[8][16];   /* cloud puffs on screen */
extern int far F_20a5_3754[8][16], F_20a5_3854[8][16];   /* area puffs */
extern int far F_20a5_59a8[8], F_20a5_59b8[8];           /* markers on screen */
extern int D_c897[], D_c8ba[], D_c8f7[], D_c915[];       /* last cloud boxes */
extern int D_8540[], D_855a[], D_85be[], D_85ae[], D_859e[], D_8574[];
extern int D_ca83[];                    /* area puff sprites [2][2] */
extern int far F_20a5_4db8[8][8], F_20a5_4e38[8][8];     /* cloud south edge on screen */
extern int far F_20a5_4c62[8][8], F_20a5_4ce2[8][8];     /* cloud east edge on screen */
extern unsigned far F_20a5_706a[8][8], F_20a5_70ea[8][8]; /* south edge on the map */
extern unsigned far F_20a5_6f6a[8][8], F_20a5_6fea[8][8]; /* east edge on the map */
extern long far F_20a5_0000[];          /* map cells */
extern char *D_21b2[], *D_21bc[];       /* puff offsets by cloud size */
extern int D_e0a5, D_e0a3, D_e0a9, D_e0a7;
int far sub_s014_1d69_000c(int, int, int);
extern int D_ca41[2], D_ca71[2], D_ca2c[2];  /* cloud sprites */
extern int D_cea4[];                    /* marker sprites */
extern struct rect *D_017c;             /* screen */
extern int D_c21d, D_c240;              /* visibility margins */
extern unsigned D_853e;                 /* combatants drawn this frame (bits) */
extern int far F_20a5_35ee[13][3];      /* hit flash: counter / sprites */
extern int D_d4c1;
extern char D_d4f2[], D_d4e5[];
extern unsigned D_da89[];
extern char D_c98a[][10];               /* animation sequence per combatant */
extern int far F_20a5_4d62[], F_20a5_3dc0[];  /* status icons party / enemies */
extern char D_a695;
void far sub_s014_1ec9_0070(int);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far vec_1d83(int);
void far vec_1d8d(int);
void far vec_1d97(int);
void far vec_1d79(void);
void far sub_s014_19cd_17f6(void);
int far sub_s014_14f7_13ae(int, int, int, int, int, int, int, int, int, int, int, int, char, char,
                           int, int, int, int);
void far sub_s014_14f7_24a4(void);
void far sub_s014_14f7_296e(unsigned);
void far sub_s014_14f7_2d32(void);
int far sub_s014_14f7_347c(int, int, int, int, struct rect *);
int far sub_s014_14f7_35ce(int, int, int, int, int, struct rect *, int, int);
void far sub_s014_14f7_07ec(unsigned);
void far sub_s014_14f7_098e(unsigned);
void far sub_s014_14f7_0c92(void);
void far sub_s014_14f7_0e9e(unsigned);
void far sub_s014_14f7_118e(void);
void far sub_s014_14f7_1a6a(int, int, int);
void far sub_s014_14f7_20d6(void);
void far sub_s014_14f7_2f96(int);
void far sub_s014_14f7_31e6(void);
void far sub_s014_14f7_3404(void);
void far sub_s014_14f7_3682(int);
void far sub_s014_14f7_3a46(int);
void far sub_root_0410_007e(int far *, int, int);
extern int D_1a66;                      /* animation phase 0..2 */
extern char D_e0ab;                     /* full redraw request */
extern unsigned D_07d4;                 /* combat clock */
extern char D_e15b, D_e15c, D_e15d;     /* changed map pieces this frame */
extern unsigned char D_e178[][2], D_e196[][2], D_e13d[][2];   /* (tile, piece) */
extern char D_d845;
extern int D_e1b4, D_e1b6, D_e1b8, D_e1ba, D_e1bc, D_e1be, D_e1c0, D_e1c2;  /* cursor */
extern int D_e09f, D_e09d, D_e1c6, D_e1c4;                    /* member cursor */
extern int D_c242, D_c2bd, D_c21b;
extern char D_a6b2;
struct msl { int sx; int sy; char pad4[4]; int x; int y; char pad0c[5]; char done; char facing;
             char pad13; char sprite; char pad15[5]; };      /* 0x1a bytes, D_944d */
extern struct msl D_944d[];
extern int D_d40b[], D_d1be[], D_d4b1[], D_d4d4[], D_d4a0[], D_d428[];  /* missile sprites */
extern int D_d5b7, D_d4ff, D_d501;
extern int D_d8f1[], D_d8d7[], D_d8bd[], D_d8a3[], D_d889[], D_d86f[];  /* hot spots */
extern unsigned char D_d862[], D_d855[];
extern char D_d848[];
extern char D_e0a1;
extern char D_1aea[];                   /* tile neighbourhood sizes by cloud size */
extern char *D_1b32[], *D_1b7e[];
extern char D_1aed;
extern char D_1b08[], D_1b54[];
/* screen tile of map point (x, y) without the 0x4b bias */
#define FDIV(v, s) ((v) < 0 ? -((((1 << (s)) - 1) - (v)) >> (s)) : (v) >> (s))
#define TILE(x, y) (D_1a5d[FDIV((y) - D_c293, 4)] + FDIV((y) - D_c293, 4) * 10 + FDIV((x) - D_c26c, 5))
#define GROW(t, d) { \
        if ((d)->x < D_e16a[t]) D_e16a[t] = (d)->x; \
        if ((d)->y < D_e16c[t]) D_e16c[t] = (d)->y; \
        if (D_e16e[t] < (d)->w + (d)->x - 1) D_e16e[t] = (d)->w + (d)->x - 1; \
        if (D_e170[t] < (d)->y + (d)->h - 1) D_e170[t] = (d)->y + (d)->h - 1; }
int far _random_n(int);
extern char D_c21a;
extern char D_c907;
extern char D_c763[];                   /* size class of an animation set */
extern char D_1a5d[];
extern char D_1a72[23], D_1a8a[23], D_1aa2[10], D_1aac[10], D_1ab6[8], D_1abe[8], D_1ac6[3], D_1aca[3];
#define SPR(k) ((struct spr far *)(struct spr *)F_20a5_34b4[k])

extern struct cbt D_9c55[];
extern int D_c26c, D_c293;              /* view scroll */
extern char D_c180;                     /* missiles */
extern int D_940b;
extern struct rect *D_01a4;
extern char D_a6b4[];                   /* [13] floor */
extern char D_11ca;                     /* floor shown */
extern char D_da36[], D_d3fe[];
extern int D_8650[13];                  /* screen x of the combatant's foot point */
extern int D_8584[13], D_85ce[13], D_85e8[13], D_8602[13], D_861c[13], D_8636[13]; /* dirty */
extern char D_e160;                     /* full redraw */
extern int D_d645[13], D_d664[13], D_d6b2[13], D_d6d0[13], D_d698[13]; /* last drawn */
extern int D_d611[13];
extern char D_d505[][8];                /* sprite x offset by set and facing */
extern char D_d55d[][8];                /* sprite width by set and facing */
extern char D_d5d3[];                   /* sprite height by set */
extern char D_c8ca[];
char far sub_s014_0e76_306a(int);
struct pile { int ofs; char cnt; char level; int x; int y; int vis; char dx; char dy; };
extern struct pile far F_20a5_59c8[];   /* loot piles */
extern char D_c80e;
extern int far F_20a5_5890[0x8c];       /* map cell code (col << 8 | row) of each screen tile */
extern char D_1a6a[4], D_1a6e[4];       /* corners of a pile */
extern int D_c869;                      /* loot pile sprite */
int far sub_root_0410_02e6(int far *, int, int);  /* index of a word, or n */

void far sub_root_05f8_0117(int, int, int);
void far sub_root_047c_04ef(struct rect *, int, int, int);
void far sub_root_047c_0553(int, int, int, int, int, struct rect *, int, int);
void far sub_root_047c_0952(struct rect *, int, int, int, int, int);
void far sub_s014_1ec9_0000(int);
int far sub_s014_14f7_359a(struct rect *, int, int, int);
void far sub_s014_14f7_1e70(int far *);
void far sub_s014_14f7_1f14(int, int, int, int, int, int, int, int);
void far sub_s014_14f7_1fd4(int, int, int, int);
void far sub_s014_14f7_2052(int, int, int, int);
int far sub_s014_14f7_343c(int);
int far sub_s014_14f7_34d6(struct rect *, int, int, int, int, int);

#define HIDE() sub_root_05f8_0117(0, 0, D_940b)
#define SHOW() sub_root_05f8_0117(0, 9, D_940b)

/* draw one combat frame */
void far sub_s014_14f7_0008(void)
{
    int k, n, sy, dx, w, h, spr;
    unsigned now;
    struct spr *s, *p;
    struct rlog far *l;
    struct msl *m;

    D_1a66 = (D_1a66 + 1) % 3;
    if (D_e0ab != 0) {
        /* full redraw */
        sub_root_047c_0553((int)D_017c, 0, 0, 0x104, 0xc8, D_0154, 0, 0);
        D_e0ab = 0;
        for (k = 0; k < D_a395; k++)
            D_a342[k].f4 |= 8;
    }
    sub_root_05f8_0117(0, 9, D_940b);
    for (k = 0; k < D_a33d; k++) {
        if (D_a2eb[k].f4 >> 4 != (D_a2eb[k].f4 & 0xf))
            D_c897[k] = -9999;          /* still growing: no erase box */
        sub_s014_14f7_3682(k);
    }
    for (k = 0; k < D_a395; k++)
        if ((D_a342[k].f4 & 0xf) >= 8)
            sub_s014_14f7_3a46(k);
    if (D_e160 == 1)
        D_e176 = 0;
    D_e169 = D_e166 = D_e161 = 0;
    D_e162 = 1;
    D_853e = 0;
    sub_s014_14f7_31e6();
    sub_s014_14f7_3404();
    D_e160 = 0;
    sub_root_0410_007e((int far *)D_e16a, 9999, 0x8c);
    sub_root_0410_007e((int far *)D_e16c, 9999, 0x8c);
    sub_root_0410_007e((int far *)D_e16e, -9999, 0x8c);
    sub_root_0410_007e((int far *)D_e170, -9999, 0x8c);
    D_e174 = 0;
    /* sprites of the last frame that must be erased (replay log) */
    for (k = 0; k < D_e176; k++) {
        l = &F_20a5_7354[k];
        sub_s014_14f7_13ae(-1, l->x, l->y, l->h, l->sx, l->sy, l->w, l->hh, l->kind, 0,
                           l->dx0, l->dy0, l->b13, l->b14, l->dw, l->dh, 0, 0);
    }
    D_e176 = 0;
    /* changed map pieces: floor objects, west walls, north walls */
    for (k = 0; k < D_e15d; k++)
        sub_s014_14f7_1a6a(D_e13d[k][0], D_e13d[k][1], 2);
    for (k = 0; k < D_e15b; k++)
        sub_s014_14f7_1a6a(D_e178[k][0], D_e178[k][1], 0);
    for (k = 0; k < D_e15c; k++)
        sub_s014_14f7_1a6a(D_e196[k][0], D_e196[k][1], 1);
    now = D_a695 == 0 ? D_07d4 : 0;
    sub_s014_14f7_07ec(now);
    sub_s014_14f7_098e(now);
    sub_s014_14f7_118e();
    if (D_d845 == -2 || D_d845 == -3) {
        /* ground cursor */
        sy = D_e1b6 - D_c293;
        D_e1c2 = D_e1b4 - D_c26c - sy + 0x88 - D_e1b8 / 2;
        D_e1be = sy - D_e1c0 + 0x69;
        sub_s014_14f7_13ae(-1, D_e1b4, D_e1b6, 0, D_e1c2, D_e1be, D_e1b8, D_e1ba, 0x7b0c, D_e1bc,
                           0, 0, 0, 0, 0, 0, 1, 0);
    }
    /* missiles */
    for (k = D_c180 - 1; k >= 0; k--) {
        m = &D_944d[k];
        if (m->done < 0 && (m->done & 7) == 0) {
            /* impact frame */
            switch (m->sprite) {
            case 0: spr = D_d40b[m->facing]; break;
            case 1: spr = D_d5b7; break;
            case 3: spr = D_d4b1[m->facing]; break;
            case 4: spr = D_d4d4[m->facing]; break;
            case 5: spr = D_d4a0[m->facing]; break;
            case 6: spr = D_d428[m->facing]; break;
            case 7:
            case 8: spr = D_d4ff; break;
            case 9: spr = D_d501; break;
            }
            m->x += 2;
            m->y += 2;
        } else {
            switch (m->sprite) {
            case 0: spr = D_d1be[m->facing]; break;
            case 1: spr = D_d5b7; break;
            case 3: spr = D_d4b1[m->facing]; break;
            case 4: spr = D_d4d4[m->facing]; break;
            case 5: spr = D_d4a0[m->facing]; break;
            case 6: spr = D_d428[m->facing]; break;
            case 7:
            case 8: spr = D_d4ff; break;
            case 9: spr = D_d501; break;
            }
        }
        switch (m->sprite) {
        case 0:
        case 2:
        case 3:
        case 5:
        case 6:
            dx = 3;
            w = h = 7;
            break;
        case 1:
            dx = 1;
            w = h = 2;
            break;
        case 4:
            dx = 7;
            w = 0xf;
            h = 8;
            break;
        case 7:
        case 8:
        case 9:
            dx = 0xa;
            w = 0x13;
            h = 0x10;
            break;
        }
        sy = m->y - D_c293;
        m->sx = m->x - D_c26c - sy - dx + 0x88;
        m->sy = sy + 0x55;
        sub_s014_14f7_13ae(-1, m->x, m->y, 0, m->sx, m->sy, w, h, 0x7b0c, spr,
                           0, 0, 0, 0, 0, 0, 1, 0);
        if (m->done < 0)
            vec_1d79(k);                /* finished: remove the missile */
    }
    sub_s014_14f7_20d6();
    /* cursor around the selected member */
    if (D_a6b2 >= 0 && (D_853e & 1 << D_a6b2) != 0) {
        for (k = 1; k < D_e162; k++) {
            s = (struct spr *)F_20a5_34b4[k];
            if (s->e0 < 28000 && s->who == D_a6b2)
                break;
        }
        if (D_e162 < 157) {
            s = (struct spr *)F_20a5_34b4[k];
            n = F_20a5_34b4[D_e162];
            ((struct spr *)n)->x = s->x + 1;
            ((struct spr *)n)->e0 = 30000;
            ((struct spr *)n)->k10 = s->k10 + 1;
            ((struct spr *)n)->k12 = s->k12 + 1;
            ((struct spr *)n)->spr = D_c21b;
            ((struct spr *)n)->w = D_e1c6;
            ((struct spr *)n)->h = D_e1c4;
            ((struct spr *)n)->cx = D_e09f;
            ((struct spr *)n)->cy = D_e09d;
            D_e162++;
        }
    }
    /* cursor around the target */
    if ((D_c242 == D_e1bc || D_c2bd == D_e1bc) && (D_853e & 1 << D_d845) != 0) {
        for (k = 1; k < D_e162; k++) {
            s = (struct spr *)F_20a5_34b4[k];
            if (s->e0 < 28000 && s->who == D_d845)
                break;
        }
        if (D_e162 < 157) {
            s = (struct spr *)F_20a5_34b4[k];
            n = F_20a5_34b4[D_e162];
            ((struct spr *)n)->x = s->x + 1;
            ((struct spr *)n)->e0 = 31000;
            ((struct spr *)n)->k10 = s->k10 + 1;
            ((struct spr *)n)->k12 = s->k12 + 1;
            ((struct spr *)n)->spr = D_e1bc;
            ((struct spr *)n)->w = D_e1b8;
            ((struct spr *)n)->h = D_e1ba;
            ((struct spr *)n)->cx = D_e1c2;
            ((struct spr *)n)->cy = D_e1be;
            D_e162++;
        }
    }
    sub_s014_14f7_2f96(D_e162);
    /* mouse hot spots of the combatants, front to back */
    D_e0a1 = 0;
    for (k = 1; k < D_e162; k++) {
        s = (struct spr *)F_20a5_34b4[k];
        if (s->e0 >= 28000)
            continue;
        n = D_e0a1;
        D_d8f1[n] = s->e0;
        D_d8d7[n] = s->e1;
        D_d8bd[n] = s->e2;
        D_d8a3[n] = s->e3;
        D_d889[n] = s->sx2;
        D_d86f[n] = s->sy2;
        D_d862[n] = s->w;
        D_d855[n] = s->h;
        D_d848[n] = s->who;
        D_e0a1++;
    }
    sub_s014_19cd_17f6();
    for (k = 1; k < D_e162; k++) {
        s = (struct spr *)F_20a5_34b4[k];
        if (s->e0 == 30000) {
            s->sx = s->cx = D_e09f;
            s->sy = s->cy = D_e09d;
            s->w = 8;
            s->h = 0x1a;
            sub_s014_14f7_1e70((int far *)s);
        } else if (s->e0 == 31000) {
            if (D_c242 == D_e1bc || D_c2bd == D_e1bc) {
                s->sx = s->cx = D_e1c2;
                s->sy = s->cy = D_e1be;
                s->h = s->w = 10;
                sub_s014_14f7_1e70((int far *)s);
            } else
                s->sx = -500;
        }
    }
    sub_s014_14f7_0c92();
    sub_s014_14f7_0e9e(now);
    sub_root_05f8_0117(0, 0, D_940b);
}

/* queue the effect clouds (edge sprites of every puff column/row) and the other effect lists */
void far sub_s014_14f7_07ec(unsigned now)
{
    int k, c, n, kind, s1, s2;

    if (D_a33d > 0) {
        sub_s014_14f7_24a4();
        for (k = 0; k < D_a33d; k++) {
            n = D_a2eb[k].f4 & 7;
            kind = D_a2eb[k].kind;
            if (now < D_a2eb[k].t) {
                s1 = D_ca41[_random_n(2)];
                s2 = D_ca71[_random_n(2)];
            } else {
                s2 = 0;
                s1 = 0;
            }
            for (c = 0; c < n; c++) {
                if (F_20a5_4db8[k][c] != -999)
                    sub_s014_14f7_13ae(-1, F_20a5_706a[k][c], F_20a5_70ea[k][c], 0,
                                       F_20a5_4db8[k][c], F_20a5_4e38[k][c], 0x12, 0x22,
                                       kind + 28000, s1, D_8574[k], D_859e[k], D_85ae[k],
                                       D_85be[k], D_855a[k], D_8540[k], 0, 0);
                if (F_20a5_4c62[k][c] != -999)
                    sub_s014_14f7_13ae(-1, F_20a5_6f6a[k][c], F_20a5_6fea[k][c], 2,
                                       F_20a5_4c62[k][c], F_20a5_4ce2[k][c], 8, 0x27,
                                       kind + 28000, s2, D_8574[k], D_859e[k], D_85ae[k],
                                       D_85be[k], D_855a[k], D_8540[k], 0, 0);
            }
        }
    }
    if (D_a395 > 0)
        sub_s014_14f7_296e(now);
    if (D_a531 > 0)
        sub_s014_14f7_2d32();
}

/* queue every visible combatant: body (frame D_c98a[i][f1b] of its animation), hit flashes
   (F_20a5_35ee[i]: negative = countdown, positive = flash sprites until D_da89[i]) and the
   status icon D_d4e5[i] */
void far sub_s014_14f7_098e(unsigned now)
{
    int i, set, w, h, sx, sy, spr;
    char st;
    struct spr *n, *p;
    int far *fx;

    for (i = 0; i < 13; i++) {
        st = D_9c55[i].status;
        if ((st != 1 && st != 3 && st != 2) || D_a6b4[i] != D_11ca)
            continue;
        set = sub_s014_14f7_343c(i);
        w = D_d55d[set][D_9c55[i].f1a];
        h = D_d5d3[set];
        if (sub_s014_14f7_347c(D_9c55[i].scrx - D_c21d, D_9c55[i].scry - D_c21d,
                               w + D_c240, h + D_c240, D_0154) == 0)
            continue;
        spr = D_9c55[i].info[D_c98a[i][D_9c55[i].f1b] * 8 + D_9c55[i].f1a];
        if (sub_s014_14f7_13ae(i, D_9c55[i].x, D_9c55[i].y, 0, D_9c55[i].scrx, D_9c55[i].scry,
                               w, h, 5, spr, D_85e8[i], D_8602[i], D_861c[i], D_8636[i],
                               D_85ce[i], D_8584[i], 0, st == 2 || st == 3) == 1)
            D_853e |= 1 << i;
        fx = F_20a5_35ee[i];
        if (fx[0] < 0 && fx[0] > -10) {
            if (++fx[0] == 0) {
                fx[0] = D_d4c1;
                D_d4f2[i] = 1;
            }
        } else if (fx[0] != 0 && now < D_da89[i] && D_e162 < 157) {
            /* hit flash on top of the body */
            n = (struct spr *)F_20a5_34b4[D_e162];
            p = (struct spr *)F_20a5_34b4[D_e162 - 1];
            *n = *p;
            n->spr = F_20a5_35ee[i][_random_n(3)];
            n->sx = D_d505[set][D_9c55[i].f1a] + D_9c55[i].scrx - 5;
            n->sy = D_d5d3[set] / 3 + D_9c55[i].scry - 5;
            n->e0 = p->e0;
            n->e1 = p->e1 + 1;
            n->e2 = p->e2 + 1;
            n->who = -1;
            n->e0 = 31999;
            D_e162++;
        }
        if (fx[0] != 0 && D_da89[i] <= now)
            fx[0] = 0;
        if (D_d4e5[i] != 0)
            sub_s014_14f7_13ae(i, D_9c55[i].x, D_9c55[i].y, 0,
                               D_d505[set][D_9c55[i].f1a] + D_9c55[i].scrx - 7,
                               D_d5d3[set] + D_9c55[i].scry - 0x27, 0x10, 6, 0x762a,
                               i < 5 ? F_20a5_4d62[D_d4e5[i]] : F_20a5_3dc0[D_d4e5[i]],
                               0, 0, 0, 0, 0, 0, 1, 0);
    }
}

/* blit lists A, B and C */
void far sub_s014_14f7_0c92(void)
{
    int k;
    struct drawa _based(D_d6f5) *a;
    struct drawb _based(D_d6f5) *b;
    struct spr far *s;

    for (k = 0; k < D_e161; k++) {
        a = D_e15e + k;
        D_01a4->x = a->ox + 60;
        D_01a4->y = a->oy;
        if (a->spr != 0 && a->w != 0 && a->h != 0) {
            D_01a4->w = a->w - 1;
            D_01a4->h = a->h - 1;
            sub_s014_14f7_359a(D_01a4, a->x - a->ox, a->y - a->oy, a->spr);
        }
    }
    for (k = 0; k < D_e166; k++) {
        b = D_e164 + k;
        D_01a4->x = b->ox + 60;
        D_01a4->y = b->oy;
        if (b->spr != 0 && b->w != 0 && b->h != 0) {
            D_01a4->w = b->w - 1;
            D_01a4->h = b->h - 1;
            sub_s014_14f7_359a(D_01a4, b->x - b->ox, b->y - b->oy, b->spr);
        }
    }
    for (k = 1; k < D_e162; k++) {
        s = SPR(k);
        D_01a4->x = s->cx + 60;
        D_01a4->y = s->cy;
        if ((s->e0 & 0xfff0) == 28000)
            sub_s014_1ec9_0000(s->e0 & 0xf);       /* team colours */
        if (s->spr != 0 && s->w != 0 && s->h != 0) {
            D_01a4->w = s->w - 1;
            D_01a4->h = s->h - 1;
            sub_s014_14f7_359a(D_01a4, s->sx - s->cx, s->sy - s->cy, s->spr);
        }
    }
}

/* blit the effects, age the effect lists, copy the dirty boxes to the screen */
void far sub_s014_14f7_0e9e(unsigned now)
{
    int k, j, n, spr;
    struct drawa _based(D_d6f5) *e;
    struct dirty _based(D_d6f5) *d;

    for (k = 0; k < D_e169; k++) {
        e = D_e167 + k;
        D_01a4->x = e->ox + 60;
        D_01a4->y = e->oy;
        n = (D_a2eb[k].f4 & 7) * (D_a2eb[k].f4 & 7);
        if (now < D_a2eb[k].t) {
            sub_s014_1ec9_0000(D_a2eb[k].kind);
            spr = D_ca2c[_random_n(2)];
        } else
            spr = 0;
        if (D_e167[k].w != 0 && D_e167[k].h != 0) {
            D_01a4->w = D_e167[k].w - 1;
            D_01a4->h = D_e167[k].h - 1;
            for (j = 0; n != 0; n--, j++)
                if (F_20a5_3bc0[k][j] != 0)
                    sub_s014_14f7_359a(D_01a4, F_20a5_3bc0[k][j] - D_e167[k].ox,
                                       F_20a5_3cc0[k][j] - D_e167[k].oy, spr);
        }
    }
    D_01a4->x = 60;
    D_01a4->w = 0x103;
    D_01a4->h = 199;
    D_01a4->y = 0;
    for (k = 0; k < D_a531; k++) {
        sub_s014_1ec9_0070(D_a396[k].f5);
        sub_s014_14f7_359a(D_01a4, F_20a5_59a8[k], F_20a5_59b8[k], D_cea4[D_a396[k].f4 & 7]);
    }
    if (D_a695 == 0) {
        for (k = D_a531 - 1; k >= 0; k--)
            if (++D_a396[k].f4 > 0x44)
                vec_1d97(k);            /* s019 02b4: marker expires */
        for (k = D_a395 - 1; k >= 0; k--)
            if (D_a342[k].t <= now)
                vec_1d8d(k);            /* s019 0260: area expires */
        for (k = 0; k < D_a395; k++)
            if ((D_a342[k].f4 & 0xf) < D_a342[k].f4 >> 4) {
                D_a342[k].f4++;
                D_a342[k].f4 |= 8;      /* grows: lay out again */
            }
        for (k = D_a33d - 1; k >= 0; k--)
            if (D_a2eb[k].t <= now)
                vec_1d83(k);            /* s019 01e2: cloud expires */
        for (k = 0; k < D_a33d; k++)
            if ((D_a2eb[k].f4 & 0xf) < D_a2eb[k].f4 >> 4)
                D_a2eb[k].f4++;
    }
    for (k = 0; k < D_e174; k++) {
        sub_root_047c_12c3();
        d = D_e172 + k;
        sub_s014_14f7_35ce((int)D_0154, d->x, d->y, d->w, d->h, D_017c, d->x, d->y);
        sub_root_047c_1293();
    }
}

/* queue the loot piles on the shown floor (list B), clipped to the dirty boxes of the
   screen tiles under them */
void far sub_s014_14f7_118e(void)
{
    int r[7];
    int k, c, t, found, minx, miny, maxx, maxy;
    struct pile far *p;
    struct drawb _based(D_d6f5) *b;

    for (k = 0; k < (unsigned char)D_c80e; k++) {
        p = &F_20a5_59c8[k];
        if (p->level != D_11ca)
            continue;
        r[1] = -p->dx - ((p->y - D_c293) - p->x) - D_c26c + 0x88;
        r[2] = (p->y - D_c293) - p->dy + 0x69;
        r[5] = 16;
        r[6] = 8;
        r[3] = r[1];
        r[4] = r[2];
        sub_s014_14f7_1e70((int far *)r);
        if (r[5] == 0 || r[6] == 0)
            continue;
        miny = 9999;
        minx = 9999;
        maxy = 0;
        maxx = 0;
        found = 0;
        for (c = 0; c < 4; c++) {
            t = sub_root_0410_02e6(F_20a5_5890,
                ((D_1a6e[c] + p->y) >> 4) + ((D_1a6a[c] + p->x) & 0xffe0) * 8, 0x8c);
            if (t != 0x8c) {
                found = 1;
                if (D_e16a[t] < minx)
                    minx = D_e16a[t];
                if (D_e16c[t] < miny)
                    miny = D_e16c[t];
                if (maxx < D_e16e[t])
                    maxx = D_e16e[t];
                if (maxy < D_e170[t])
                    maxy = D_e170[t];
            }
        }
        if (found == 1) {
            if (minx < 0)
                minx = 0;
            if (miny < 0)
                miny = 0;
            if (maxx > 0x103)
                maxx = 0x103;
            if (maxy > 199)
                maxy = 199;
            maxx += 1 - minx;
            if (D_e166 < 0x8a) {
                b = D_e164 + D_e166;
                b->x = r[1];
                b->y = r[2];
                b->w = maxx;
                b->h = maxy + 1 - miny;
                b->ox = minx;
                b->oy = miny;
                if (p->vis == 1) {
                    D_e164[D_e166].spr = D_c869;
                    D_e166++;
                }
            }
        }
    }
}

/* queue a sprite: list C (or list B when toB and not in the 'Z' demo), clip it to the view,
   remember the dirty box (D_e172), restore the background under it, optionally log it for
   the replay (F_20a5_7354), compute its depth extents from the size class of its animation
   set (D_c763) and grow the dirty boxes of the screen tiles it covers.
   who: combatant or -1; (x, y, h): map position and height; (sx, sy): screen position;
   (w, hh): size; kind >= 28000: team-coloured effect sprite. Returns 1 if queued. */
int far sub_s014_14f7_13ae(int who, int x, int y, int h, int sx, int sy, int w, int hh,
                           int kind, int spr, int dx0, int dy0, char b13, char b14,
                           int dw, int dh, int log, int toB)
{
    struct spr far *r;
    int tx, ty, tile, row, n, t, c;
    struct dirty _based(D_d6f5) *d;

    if (toB == 0 || D_c21a == 'Z') {
        if (D_e162 > 156)
            return 0;
        r = SPR(D_e162);
        D_e162++;
    } else {
        if (D_e166 > 0x89)
            return 0;
        r = (struct spr far *)(D_e164 + D_e166);
        D_e166++;
    }
    tx = x - D_c26c;
    tx = tx < 0 ? -((31 - tx) >> 5) : tx >> 5;
    ty = y - D_c293;
    ty = ty < 0 ? -((15 - ty) >> 4) : ty >> 4;
    tile = D_1a5d[ty] + ty * 10 + tx + 0x4b;
    row = (unsigned)tile / 10;
    r->x = x;
    r->k10 = y - h;
    r->k12 = h + y;
    r->sx = sx;
    r->cx = sx + dx0;
    r->sy = sy;
    r->cy = sy + dy0;
    r->who = who;
    r->w = w + dw;
    r->h = dh + hh;
    sub_s014_14f7_1e70((int far *)r);
    if (r->w == 0 || r->h == 0) {
        if (toB == 0)
            D_e162--;
        else
            D_e166--;
        return 0;
    }
    d = D_e172 + D_e174++;
    d->h = r->h;
    d->w = r->w;
    d->y = r->cy;
    d->x = r->cx;
    sub_s014_14f7_34d6(D_0154, d->x, d->y, d->w, d->h, 0);
    if (log == 1) {
        struct rlog far *l = &F_20a5_7354[D_e176++];
        l->x = x;
        l->y = y;
        l->h = h;
        l->sx = sx;
        l->sy = sy;
        l->w = w;
        l->hh = hh;
        l->kind = kind;
        l->dx0 = dx0;
        l->dy0 = dy0;
        l->b13 = b13;
        l->b14 = b14;
        l->dw = dw;
        l->dh = dh;
    }
    if (kind > 27999) {
        r->e0 = kind;
        goto done;
    }
    c = D_c763[sub_s014_14f7_343c(who)];
    if (c == 0) {
        r->e0 = D_8650[who] - 4;
        r->e1 = D_8650[who] + 3;
        hh = sy + hh;
        r->e2 = hh - 0x19;
    } else {
        hh >>= 1;
        if (c == 1) {
            r->e0 = D_8650[who] - 10;
            r->e1 = D_8650[who] + 10;
            r->e2 = hh + sy - 4;
            hh = hh + sy + 4;
        } else if (c == 2) {
            r->e0 = D_8650[who] - 5;
            r->e1 = D_8650[who] + 5;
            r->e2 = hh + sy - 15;
            hh = hh + sy + 15;
        } else if (c == 3) {
            r->e0 = D_8650[who] - 10;
            r->e1 = D_8650[who] + 10;
            r->e2 = hh + sy - 15;
            hh = hh + sy + 15;
        } else if (c == 4) {
            r->e0 = D_8650[who] - 25;
            r->e1 = D_8650[who] + 25;
            r->e2 = hh + sy - 25;
            hh = hh + sy + 25;
        } else
            goto done;
    }
    r->e3 = hh;
    r->sx2 = sx;
    r->sy2 = sy;
done:
    r->spr = spr;
#define GROW1(t) { \
        if (d->x < D_e16a[t]) D_e16a[t] = d->x; \
        if (d->y < D_e16c[t]) D_e16c[t] = d->y; \
        if (D_e16e[t] < d->w + d->x - 1) D_e16e[t] = d->w + d->x - 1; \
        if (D_e170[t] < d->y + d->h - 1) D_e170[t] = d->y + d->h - 1; }
    for (n = 0; n < 23; n++) {
        t = D_1a72[n] + tile;
        if (t >= 0 && t < 0x8c && t / 10 - D_1a8a[n] == row)
            GROW1(t);
    }
    if (who != -1 && (D_c763[sub_s014_14f7_343c(who)] & 1) != 0)
        for (n = 0; n < 10; n++) {
            t = D_1aa2[n] + tile;
            if (t >= 0 && t < 0x8c && t / 10 - D_1aac[n] == row)
                GROW1(t);
        }
    if (who != -1 && (D_c763[sub_s014_14f7_343c(who)] & 4) != 0)
        for (n = 0; n < 8; n++) {
            t = D_1ab6[n] + tile;
            if (t >= 0 && t < 0x8c && t / 10 - D_1abe[n] == row)
                GROW1(t);
        }
    if (who != -1 && (sub_s014_14f7_343c(who), D_c907 == 1))
        for (n = 0; n < 3; n++) {
            t = D_1ac6[n] + tile;
            if (t >= 0 && t < 0x8c && t / 10 - D_1aca[n] == row)
                GROW1(t);
        }
    return 1;
}

/* queue the wall/object piece n of screen tile `tile`: layer 0 = west wall, 1 = north wall
   (list C), 2 = floor object (list B) */
void far sub_s014_14f7_1a6a(int tile, int n, int layer)
{
    struct spr far *r;
    int row, col, k, t;
    struct dirty _based(D_d6f5) *d;

    if (layer == 2) {
        if (D_e166 > 0x89)
            return;
        r = (struct spr far *)(D_e164 + D_e166);
        D_e166++;
    } else {
        if (D_e162 > 156)
            return;
        r = SPR(D_e162);
        D_e162++;
    }
    row = F_20a5_36c8[tile] * 16;
    col = F_20a5_363c[tile] * 32;
    if (layer == 0) {
        r->x = col + 4;
        r->k10 = row;
        r->k12 = row;
        r->sx = r->cx = F_20a5_45be[tile] + D_c76e[n];
        r->sy = r->cy = D_c789[n] + F_20a5_46d6[tile];
        r->w = D_c7de[n];
        r->h = D_c7ee[n];
        r->spr = F_20a5_6eea[n * 2 + _random_n(2)];
        r->e0 = 0x7d01;
    } else if (layer == 1) {
        r->x = col + 1;
        r->k10 = row + 8;
        r->k12 = row + 8;
        r->sx = r->cx = F_20a5_4b4a[tile] + D_c821[n];
        r->sy = r->cy = D_c855[n] + F_20a5_46d6[tile];
        r->w = D_c876[n];
        r->h = D_c887[n];
        r->spr = F_20a5_6f2a[n * 2 + _random_n(2)];
        r->e0 = 0x7d04;
    } else {
        r->x = col;
        r->k10 = row;
        r->k12 = row;
        r->sx = r->cx = F_20a5_4b4a[tile];
        r->sy = r->cy = F_20a5_415e[tile];
        r->w = 0x30;
        r->h = 0x10;
        r->spr = F_20a5_3474[n * 2 + _random_n(2)];
        r->e0 = 0x7d1d;
    }
    sub_s014_14f7_1e70((int far *)r);
    if (r->w != 0 && r->h != 0) {
        d = D_e172 + D_e174++;
        d->h = r->h;
        d->w = r->w;
        d->y = r->cy;
        d->x = r->cx;
        sub_s014_14f7_34d6(D_0154, d->x, d->y, d->w, d->h, 0);
        for (k = 0; k < 14; k++) {
            t = D_1ace[k] + tile;
            if (t >= 0 && t < 0x8c && D_1adc[k] + tile / 10 == t / 10) {
                if (r->cx < D_e16a[t])
                    D_e16a[t] = r->cx;
                if (r->cy < D_e16c[t])
                    D_e16c[t] = r->cy;
                if (D_e16e[t] < r->cx + r->w - 1)
                    D_e16e[t] = r->cx + r->w - 1;
                if (D_e170[t] < r->h + r->cy - 1)
                    D_e170[t] = r->h + r->cy - 1;
            }
        }
        return;
    }
    if (layer != 0 && layer != 1)
        D_e166--;
    else
        D_e162--;
}

/* clip rectangle r (+6 x, +8 y, +0a w, +0c h) to the view */
void far sub_s014_14f7_1e70(int far *r)
{
    if (r[3] < 0) {
        if ((r[5] += r[3]) < 0)
            r[5] = 0;
        r[3] = 0;
    }
    if (r[3] > 0x103)
        r[5] = 0;
    if (r[3] + r[5] - 1 > 0x103 && (r[5] = 0x104 - r[3]) < 0)
        r[5] = 0;
    if (r[4] < 0) {
        if ((r[6] += r[4]) < 0)
            r[6] = 0;
        r[4] = 0;
    }
    if (r[4] > 199)
        r[6] = 0;
    if (r[4] + r[6] - 1 > 199 && (r[6] = 200 - r[4]) < 0)
        r[6] = 0;
}

/* add sprite `id` to list C */
void far sub_s014_14f7_1f14(int id, int flags, int k0e, int k10, int k12, int x, int y, int spr)
{
    struct spr far *s;

    if (D_e162 < 157) {
        s = SPR(D_e162);
        s->e0 = flags;
        s->x = k0e;
        s->k10 = k10;
        s->k12 = k12;
        s->sx = x;
        s->sy = y;
        SPR(D_e162)->w = D_e16e[id];
        SPR(D_e162)->h = D_e170[id];
        SPR(D_e162)->cx = D_e16a[id];
        SPR(D_e162)->cy = D_e16c[id];
        SPR(D_e162)->spr = spr;
        D_e162++;
    }
}

/* add sprite `id` to list A */
void far sub_s014_14f7_1fd4(int id, int x, int y, int spr)
{
    struct drawa _based(D_d6f5) *a;

    if (D_e161 < 100) {
        a = D_e15e + D_e161;
        a->x = x;
        a->y = y;
        a->w = D_e16e[id];
        a->h = D_e170[id];
        a->ox = D_e16a[id];
        a->oy = D_e16c[id];
        a->spr = spr;
        D_e161++;
    }
}

extern int D_3464[][2];                 /* wall sprites (20a5:3464 + 4*n) */

/* add wall/furniture sprite `id` to list B */
void far sub_s014_14f7_2052(int id, int x, int y, int n)
{
    struct drawb _based(D_d6f5) *b;

    if (D_e166 < 0x8a) {
        b = D_e164 + D_e166;
        b->x = x;
        b->y = y;
        b->w = D_e16e[id];
        b->h = D_e170[id];
        b->ox = D_e16a[id];
        b->oy = D_e16c[id];
        b->spr = D_3464[n][0];
        D_e166++;
    }
}

/* re-queue everything in the dirty screen tiles: floor, west/north walls and their pieces,
   furniture, decals and floor objects from the tile descriptions F_20a5_4eb8[tile] */
void far sub_s014_14f7_20d6(void)
{
    int t, row, col, f, hh, idx, y0, y1;
    unsigned v;
    struct tdesc far *td;

    for (t = 0; t < 0x8c; t++) {
        if (D_e16a[t] == 9999)
            continue;
        if (D_e16a[t] < 0)
            D_e16a[t] = 0;
        if (D_e16c[t] < 0)
            D_e16c[t] = 0;
        if (D_e16e[t] > 0x103)
            D_e16e[t] = 0x103;
        if (D_e170[t] > 199)
            D_e170[t] = 199;
        D_e16e[t] += 1 - D_e16a[t];
        D_e170[t] += 1 - D_e16c[t];
        td = &F_20a5_4eb8[t];
        if (td->floor == -1)
            continue;
        row = F_20a5_36c8[t] * 16;
        col = F_20a5_363c[t] * 32;
        if (td->floor != 0)
            sub_s014_14f7_1f14(t, 32000, col, row, row, F_20a5_45be[t], F_20a5_46d6[t], td->floor);
        if (td->wwall != 0 && D_c12e <= td->wwall) {
            v = td->wwall;
            sub_s014_14f7_1f14(t, 0x7d01, col + 4, row, row, D_c76e[v] + F_20a5_45be[t],
                               D_c789[v] + F_20a5_46d6[t], F_20a5_6eea[v * 2]);
        }
        if (td->nback != 0)
            sub_s014_14f7_1f14(t, 0x7d03, col, row + 1, row + 8, F_20a5_4b4a[t], F_20a5_46d6[t],
                               td->nback);
        if (td->nwall != 0 && D_c17f <= td->nwall) {
            v = td->nwall;
            sub_s014_14f7_1f14(t, 0x7d04, col + 1, row + 8, row + 8, D_c821[v] + F_20a5_4b4a[t],
                               D_c855[v] + F_20a5_46d6[t], F_20a5_6f2a[v * 2]);
        }
        if (td->ewall != 0)
            sub_s014_14f7_1f14(t, 0x7d05, col + 0x1f, row + 9, row + 0xf, F_20a5_4046[t],
                               F_20a5_46d6[t], td->ewall);
        if (td->furn != 0) {
            v = td->furn;
            f = v & 0xf;
            hh = D_c11d[f] >> 1;
            idx = f * 4 + ((v & 0xf0) >> 4);
            y0 = D_d802[idx] - hh + row;
            if (y0 < row + 1)
                y0 = row + 1;
            y1 = row + hh + D_d802[idx];
            if (y1 > row + 14)
                y1 = row + 14;
            sub_s014_14f7_1f14(t, 0x7d0a, D_d7be[idx] + col, y0, y1, D_c13f[idx] + F_20a5_4b4a[t],
                               D_c181[idx] + F_20a5_415e[t], D_c8d5[f]);
        }
        if (td->swall != 0)
            sub_s014_14f7_1f14(t, 0x7d14, col + 0x10, row + 0xf, row + 0xf, F_20a5_491a[t],
                               F_20a5_4a32[t], td->swall);
        if (td->decal != 0)
            sub_s014_14f7_1fd4(t, F_20a5_4b4a[t], F_20a5_415e[t], td->decal);
        if (td->obj != 0 && D_c12d <= td->obj)
            sub_s014_14f7_2052(t, F_20a5_4b4a[t], F_20a5_415e[t], td->obj);
    }
}

/* effect clouds (s019 list 1, D_a2eb): bounding box of the puffs (F_20a5_3bc0/3cc0),
   erase box against last frame (D_c897/D_c8ba/D_c8f7/D_c915), list-C entry, dirty tiles */
void far sub_s014_14f7_24a4(void)
{
    int k, n, j, minx, miny, maxx, maxy, w, h, ox, oxe, oy, oye, nxe, nye, tile, t;
    struct drawa _based(D_d6f5) *e;
    struct dirty _based(D_d6f5) *d;

    for (k = 0; k < D_a33d; k++) {
        if (D_a2eb[k].level != D_11ca) {
            D_c897[k] = -9999;
            D_c8ba[k] = -9999;
            continue;
        }
        n = D_a2eb[k].f4 & 7;
        miny = 9999;
        minx = 9999;
        maxy = -9999;
        maxx = -9999;
        for (j = 0; j < n * n; j++)
            if (F_20a5_3bc0[k][j] != 0 && F_20a5_3cc0[k][j] != 0) {
                if (F_20a5_3bc0[k][j] < minx)
                    minx = F_20a5_3bc0[k][j];
                if (maxx < F_20a5_3bc0[k][j])
                    maxx = F_20a5_3bc0[k][j];
                if (F_20a5_3cc0[k][j] < miny)
                    miny = F_20a5_3cc0[k][j];
                if (maxy < F_20a5_3cc0[k][j])
                    maxy = F_20a5_3cc0[k][j];
            }
        if (minx == 9999) {
            D_c897[k] = -9999;
            D_c8ba[k] = -9999;
            continue;
        }
        D_8540[k] = 0;
        D_855a[k] = 0;
        D_85be[k] = 0;
        D_85ae[k] = 0;
        D_859e[k] = 0;
        D_8574[k] = 0;
        w = maxx - minx + 25;
        h = maxy - miny + 8;
        if (D_c897[k] != -9999) {
            ox = D_c897[k];
            oxe = ox + D_c8f7[k];
            oy = D_c8ba[k];
            oye = D_c915[k] + oy;
            nxe = minx + w;
            nye = miny + h;
            if (minx < ox) {
                if (nxe < oxe) {
                    D_855a[k] = ox + (D_c8f7[k] - w) - minx;
                    D_85ae[k] = oxe - nxe;
                }
            } else {
                D_8574[k] = ox - minx;
                D_855a[k] = minx - ox;
                if (nxe < oxe) {
                    D_855a[k] = oxe + (minx - ox - nxe);
                    D_85ae[k] = oxe - nxe;
                }
            }
            if (miny < oy) {
                if (nye < oye) {
                    D_8540[k] = oy + (D_c915[k] - h) - miny;
                    D_85be[k] = oye - nye;
                }
            } else {
                D_859e[k] = oy - miny;
                D_8540[k] = miny - oy;
                if (nye < oye) {
                    D_8540[k] = oye + (miny - oy - nye);
                    D_85be[k] = oye - nye;
                }
            }
        }
        D_c897[k] = minx;
        D_c8ba[k] = miny;
        D_c8f7[k] = w;
        D_c915[k] = h;
        if (D_e169 > 0x7f)
            return;
        e = D_e167 + D_e169++;
        e->x = minx;
        e->ox = minx + D_8574[k];
        e->y = miny;
        e->oy = miny + D_859e[k];
        e->w = D_855a[k] - minx + maxx + 25;
        e->h = D_8540[k] - miny + maxy + 8;
        sub_s014_14f7_1e70((int far *)e);
        d = D_e172 + D_e174++;
        d->h = e->h;
        d->w = e->w;
        d->y = e->oy;
        d->x = e->ox;
        sub_s014_14f7_34d6(D_0154, d->x, d->y, d->w, d->h, 0);
        tile = TILE(D_a2eb[k].x, D_a2eb[k].y);
        if (d->x == 0 && (tile + 0x4b) % 10 == 9)
            tile++;
        tile += 0x4b;
        if (d->w + d->x > 0x102 && tile % 10 == 0)
            tile--;
        for (j = 0; j < D_1aea[n]; j++) {
            t = D_1b32[n][j] + tile;
            if (t >= 0 && t < 0x8c && t / 10 - D_1b7e[n][j] == tile / 10)
                GROW(t, d);
        }
    }
}

/* effect areas (s019 list 2, D_a342): puffs as list-B sprites, dirty tiles */
void far sub_s014_14f7_296e(unsigned now)
{
    int k, n, j, minx, miny, maxx, maxy, tile, t;
    unsigned tt;
    struct drawb _based(D_d6f5) *b;
    struct dirty _based(D_d6f5) *d;

    for (k = 0; k < D_a395; k++) {
        if (D_a342[k].level != D_11ca)
            continue;
        tt = D_a342[k].t;
        n = D_a342[k].f4 & 7;
        miny = 9999;
        minx = 9999;
        maxy = -9999;
        maxx = -9999;
        for (j = 0; j < n * n; j++)
            if (F_20a5_3754[k][j] != 0 && F_20a5_3854[k][j] != 0) {
                if (F_20a5_3754[k][j] < minx)
                    minx = F_20a5_3754[k][j];
                if (maxx < F_20a5_3754[k][j])
                    maxx = F_20a5_3754[k][j];
                if (F_20a5_3854[k][j] < miny)
                    miny = F_20a5_3854[k][j];
                if (maxy < F_20a5_3854[k][j])
                    maxy = F_20a5_3854[k][j];
            }
        if (minx == 9999)
            continue;
        b = D_e164 + D_e166;
        b->ox = minx;
        b->oy = miny;
        b->w = 0x17 - (minx - maxx);
        b->h = 8 - (miny - maxy);
        sub_s014_14f7_1e70((int far *)b);
        d = D_e172 + D_e174++;
        d->h = b->h;
        d->w = b->w;
        d->y = b->oy;
        d->x = b->ox;
        sub_s014_14f7_34d6(D_0154, d->x, d->y, d->w, d->h, 0);
        for (j = 0; j < n * n; j++) {
            b = D_e164 + D_e166++;
            b->x = b->ox = F_20a5_3754[k][j];
            b->y = b->oy = F_20a5_3854[k][j];
            b->w = 0x17;
            b->h = 8;
            b->spr = D_ca83[(tt <= now) * 2 + _random_n(2)];
            sub_s014_14f7_1e70((int far *)b);
        }
        tile = TILE(D_a342[k].x, D_a342[k].y);
        if (d->x == 0 && (tile + 0x4b) % 10 == 9)
            tile++;
        tile += 0x4b;
        if (d->w + d->x > 0x102 && tile % 10 == 0)
            tile--;
        for (j = 0; j < D_1aed; j++) {
            t = D_1b08[j] + tile;
            if (t >= 0 && t < 0x8c && t / 10 - D_1b54[j] == tile / 10)
                GROW(t, d);
        }
    }
}

/* effect markers (s019 list 3, D_a396): screen position (F_20a5_59a8/59b8), dirty tiles */
void far sub_s014_14f7_2d32(void)
{
    int r[7];
    int k, j, tile, t;
    struct dirty _based(D_d6f5) *d;

    for (k = 0; k < D_a531; k++) {
        if (D_a396[k].level != D_11ca)
            continue;
        r[2] = D_a396[k].y - D_c293;
        r[1] = -D_c26c - (r[2] - D_a396[k].x) + 0x5f;
        F_20a5_59a8[k] = r[1];
        r[2] += 0x44;
        F_20a5_59b8[k] = r[2];
        r[3] = 0x53;
        r[4] = 0x37;
        sub_s014_14f7_1e70((int far *)r - 2);
        d = D_e172 + D_e174++;
        d->h = r[4];
        d->w = r[3];
        d->y = r[2];
        d->x = r[1];
        sub_s014_14f7_34d6(D_0154, d->x, d->y, d->w, d->h, 0);
        tile = TILE(D_a396[k].x, D_a396[k].y);
        if (r[1] == 0 && (tile + 0x4b) % 10 == 9)
            tile++;
        tile += 0x4b;
        if (d->w + d->x > 0x102 && tile % 10 == 0)
            tile--;
        for (j = 0; j < D_1aed; j++) {
            t = D_1b08[j] + tile;
            if (t >= 0 && t < 0x8c && D_1b54[j] + tile / 10 == t / 10)
                GROW(t, d);
        }
    }
}

/* insertion sort of list C (entries 1..n-1) by depth */
void far sub_s014_14f7_2f96(int n)
{
    int k, j;
    unsigned cur, prev;
    int a, b, c;

    for (k = 2; k < n; k++) {
        cur = F_20a5_34b4[k];
        a = SPR(k)->x;
        b = SPR(k)->k10;
        c = SPR(k)->k12;
        SPR(0)->x = a;                /* sentinel */
        SPR(0)->k10 = b;
        SPR(0)->k12 = c;
        for (j = k; ; j--) {
            prev = F_20a5_34b4[j - 1];
            if (((struct spr *)prev)->k10 <= c
             && (((struct spr *)prev)->k12 < b
              || ((struct spr *)prev)->x <= a))
                break;
            F_20a5_34b4[j] = prev;
        }
        F_20a5_34b4[j] = cur;
    }
}

/* quicksort of list B (entries 0..n-1) by depth */
void far sub_s014_14f7_304a(int n)
{
    int hi[20], lo[20];
    struct drawb t;
    int sp, l, r, i, j, a, b, c;
    struct drawb _based(D_d6f5) *p;

    sp = 1;
    lo[1] = 0;
    hi[1] = n - 1;
    do {
        l = lo[sp];
        r = hi[sp];
        sp--;
        do {
            p = D_e164 + ((l + r) >> 1);
            a = p->k0e;
            b = p->k10;
            c = p->k12;
            i = l;
            j = r;
            do {
                while (D_e164[i].k12 < b || (D_e164[i].k10 <= c && D_e164[i].k0e < a))
                    i++;
                while (c < D_e164[j].k10 || (b <= D_e164[j].k12 && a < D_e164[j].k0e))
                    j--;
                if (i <= j) {
                    if (i < j) {
                        t = D_e164[i];
                        D_e164[i] = D_e164[j];
                        D_e164[j] = t;
                    }
                    i++;
                    j--;
                }
            } while (i <= j);
            if (j - l < r - i) {
                if (i < r) {
                    sp++;
                    lo[sp] = i;
                    hi[sp] = r;
                }
                r = j;
            } else {
                if (l < j) {
                    sp++;
                    lo[sp] = l;
                    hi[sp] = j;
                }
                l = i;
            }
        } while (l < r);
    } while (sp != 0);
}

/* screen position of every visible combatant and its dirty rectangle (old and new sprite
   box: D_8584/D_85ce/D_85e8/D_8602/D_861c/D_8636) */
void far sub_s014_14f7_31e6(void)
{
    int i, sx, sy, set, d, dy, a, b, lim, w, hi, lo;
    char st, f;

    for (i = 0; i < 13; i++) {
        st = D_9c55[i].status;
        if (st == 0 || D_a6b4[i] != D_11ca)
            continue;
        if ((D_9c55[i].f10 & 0x1be) != 0
         && (D_9c55[i].target != -1 || (D_9c55[i].target == -1 && D_9c55[i].f10 == 0x100))
         && st != 2 && st != 3) {
            /* turn towards the target */
            f = sub_s014_0e76_306a(i);
            D_9c55[i].f1a = f;
            D_9c55[i].f19 = f;
            if (D_da36[i] == 1) {
                D_d3fe[i] = f;
                D_da36[i] = 0;
            }
        }
        sx = D_9c55[i].x - D_c26c;
        sy = D_9c55[i].y - D_c293;
        set = sub_s014_14f7_343c(i);
        sx = sx - sy + 0x88;
        D_8650[i] = sx;
        D_9c55[i].scrx = sx - D_d505[set][D_9c55[i].f1a];
        D_9c55[i].scry = D_c8ca[set] - D_d5d3[set] + sy + 0x69;
        D_8584[i] = 0;
        D_85ce[i] = 0;
        D_8636[i] = 0;
        D_861c[i] = 0;
        D_8602[i] = 0;
        D_85e8[i] = 0;
        if (D_e160 == 0) {
            dy = D_9c55[i].y - D_d664[i];
            d = -D_d645[i] - (dy - D_9c55[i].x);
            if (d < 1) {
                D_85ce[i] = -d;
                D_861c[i] = -d;
            } else {
                D_85e8[i] = -d;
                D_85ce[i] = d;
            }
            if (dy < 1) {
                D_8584[i] = -dy;
                D_8636[i] = -dy;
            } else {
                D_8602[i] = -dy;
                D_8584[i] = dy;
            }
            f = D_9c55[i].f1a;
            if (D_d611[i] != f) {
                /* facing changed: the sprite box changed too */
                a = 300 - D_d505[set][f];
                b = 300 - D_d6b2[i];
                if (a - b > 0)
                    D_85e8[i] -= a - b;
                lim = D_d6d0[i] + b;
                w = D_d55d[set][f];
                hi = w + a;
                if (hi < lim)
                    hi = lim;
                lo = a < b ? a : b;
                if (w + a < lim)
                    D_861c[i] += lim - (w + a);
                if (w < hi - lo)
                    D_85ce[i] += hi - lo - w;
                d = sx - D_d698[i];
                D_8602[i] += d;
                D_8584[i] -= d;
            }
        }
        D_d645[i] = D_9c55[i].x;
        D_d664[i] = D_9c55[i].y;
        D_d611[i] = D_9c55[i].f1a;
        D_d6b2[i] = D_d505[set][D_9c55[i].f1a];
        D_d6d0[i] = D_d55d[set][D_9c55[i].f1a];
        D_d698[i] = sx;
    }
}

/* screen positions of the missiles (original is hand-written: `loop`) */
void far sub_s014_14f7_3404(void)
{
    int n;
    int *p;
    int dy;

    if (D_c180 > 0)
        for (n = D_c180, p = (int *)0x9457; n != 0; n--, p += 13) {
            dy = *p - D_c293;
            p[-5] = -(dy - p[-1]) - D_c26c + 0x85;
            p[-4] = dy + 0x55;
        }
}

/* animation set of combatant i */
int far sub_s014_14f7_343c(int i)
{
    int k;

    k = D_9c55[i].kind;
    switch (D_9c55[i].pose) {
    case 2:
    case 3:
        if (k >= 5)
            k += 2;
        break;
    case 4:
        if (k >= 5)
            k += 4;
        break;
    default:
        return k;
    }
    return k;
}

/* 1 if the box (x, y, w, h) is (partly) inside r */
int far sub_s014_14f7_347c(int x, int y, int w, int h, struct rect *r)
{
    if (x < 0) {
        w += x;
        x = 0;
        if (w <= 0)
            return 0;
    }
    if (w + x > r->w && r->w - x + 1 <= 0)
        return 0;
    if (y < 0) {
        h += y;
        y = 0;
        if (h <= 0)
            return 0;
    }
    if (h + y > r->h && r->h - y + 1 <= 0)
        return 0;
    return 1;
}

/* clip and blit (047c_0952) */
int far sub_s014_14f7_34d6(struct rect *r, int x, int y, int w, int h, int spr)
{
    if (w == 0 || h == 0)
        return;
    if (x < 0) {
        w += x;
        x = 0;
        if (w <= 0)
            return 0;
    }
    if (r->w < w + x && (w = r->w - x + 1) <= 0)
        return 0;
    if (y < 0) {
        h += y;
        y = 0;
        if (h <= 0)
            return 0;
    }
    if (r->h < h + y && (h = r->h - y + 1) <= 0)
        return 0;
    HIDE();
    sub_root_047c_0952(r, x, y, w, h, spr);
    SHOW();
    return 1;
}

/* blit (047c_04ef) */
int far sub_s014_14f7_359a(struct rect *r, int x, int y, int spr)
{
    HIDE();
    sub_root_047c_04ef(r, x, y, spr);
    SHOW();
}

/* clip and blit (047c_0553); sx/sy are reset when clipped */
int far sub_s014_14f7_35ce(int spr, int x, int y, int w, int h, struct rect *r, int sx, int sy)
{
    if (x < 0) {
        w += x;
        x = 0;
        sx = 0;
        if (w <= 0)
            return 0;
    }
    if (r->w < w + x && (w = r->w - x + 1) <= 0)
        return 0;
    if (y < 0) {
        h += y;
        y = 0;
        sy = 0;
        if (h <= 0)
            return 0;
    }
    if (r->h < h + y && (h = r->h - y + 1) <= 0)
        return 0;
    HIDE();
    sub_root_047c_0553(spr, x, y, w, h, r, sx, sy);
    SHOW();
    return 1;
}

/* lay out the n*n puffs of effect cloud k around its centre (random jitter), keep them off
   walls and out of sight-blocked spots, compute screen positions and the edge sprites */
void far sub_s014_14f7_3682(int k)
{
    unsigned px[16], py[16];
    int n, j, c, r, i, sx, sy, cx, cy;
    unsigned lo, hi, fx, fy, ex, ey;
    long far *cell;

    n = D_a2eb[k].f4 & 7;
    D_a2eb[k].f4 &= 0xf7;
    cx = D_e0a5 = D_a2eb[k].x;
    cy = D_e0a3 = D_a2eb[k].y;
    for (j = 0; j < n * n; j++) {
        px[j] = _random_n(3) + D_21b2[n][j] + cx;
        py[j] = _random_n(3) + D_21bc[n][j] + cy;
        cell = &F_20a5_0000[((int)px[j] >> 5) * 40 + ((int)py[j] >> 4)];
        lo = ((unsigned far *)cell)[0];
        hi = ((unsigned far *)cell)[1];
        fx = px[j] & 0x1f;
        fy = py[j] & 0xf;
        if ((hi & 0x8000) == 0x8000) {
            py[j] = 0;
            px[j] = 0;
        } else {
            if ((lo & 0xf) == 0) {
                if ((lo & 0xf00) != 0 && fx < 7)
                    px[j] = px[j] & 0xffe7 | 7;
            } else if (fx > 0x17)
                px[j] = px[j] & 0xfff7 | 0x17;
            if ((hi & 0xf) == 0) {
                if ((hi & 0xf00) != 0 && (int)fy > 10)
                    py[j] = py[j] & 0xfffa | 10;
            } else if ((int)fy < 4)
                py[j] = py[j] & 0xfff4 | 4;
            if ((int)fx > 0x16 && (((unsigned far *)(cell + 40))[1] & 0xf00) != 0 && (int)fy > 10)
                py[j] = py[j] & 0xfffa | 10;
        }
        D_e0a9 = px[j];
        D_e0a7 = py[j];
        if (sub_s014_1d69_000c(-1, -1, 0) == 1) {
            py[j] = 0;
            px[j] = 0;
        }
        if (px[j] == 0 || py[j] == 0) {
            F_20a5_3bc0[k][j] = 0;
            F_20a5_3cc0[k][j] = 0;
        } else {
            sy = py[j] - D_c293;
            sx = px[j] - D_c26c - sy + 0x7c;
            F_20a5_3bc0[k][j] = sx;
            F_20a5_3cc0[k][j] = sy + 0x45;
            if (sx > 0x103 || sy + 0x45 > 199) {
                py[j] = 0;
                px[j] = 0;
                F_20a5_3cc0[k][j] = 0;
                F_20a5_3bc0[k][j] = 0;
            }
        }
    }
    for (c = 0; c < n; c++) {
        /* southern edge of column c */
        for (r = 0; r < n; r++) {
            i = n * r + c;
            if (px[i] != 0) {
                ex = px[i];
                ey = py[i];
            }
        }
        if (ex == 0)
            F_20a5_4db8[k][c] = -999;
        else {
            F_20a5_706a[k][c] = ex;
            F_20a5_70ea[k][c] = ey + 4;
            sy = ey + 4 - D_c293;
            sx = ex - D_c26c - sy + 0x7d;
            F_20a5_4db8[k][c] = sx;
            F_20a5_4e38[k][c] = sy + 0x49;
            if (sx > 0x103 || sy + 0x49 > 199)
                F_20a5_4db8[k][c] = -999;
        }
        /* eastern edge of row c */
        for (r = 0; r < n; r++) {
            i = n * c + r;
            if (px[i] != 0) {
                ex = px[i];
                ey = py[i];
            }
        }
        if (ex == 0)
            F_20a5_4c62[k][c] = -999;
        else {
            F_20a5_6fea[k][c] = ey;
            F_20a5_6f6a[k][c] = ex + 7;
            sy = ey - D_c293;
            sx = ex + 7 - D_c26c - sy + 0x84;
            F_20a5_4c62[k][c] = sx;
            F_20a5_4ce2[k][c] = sy + 0x45;
            if (sx > 0x103 || sy + 0x45 > 199)
                F_20a5_4c62[k][c] = -999;
        }
    }
}

/* lay out the puffs of effect area k (no jitter) */
void far sub_s014_14f7_3a46(int k)
{
    unsigned px[16], py[16];
    int n, j, sy, sx, cx, cy;
    unsigned lo, hi, fx, fy;
    long far *cell;

    n = D_a342[k].f4 & 7;
    D_a342[k].f4 &= 0xf7;
    cx = D_e0a5 = D_a342[k].x;
    cy = D_e0a3 = D_a342[k].y;
    for (j = 0; j < n * n; j++) {
        py[j] = D_21bc[n][j] + cy + 1;
        px[j] = D_21b2[n][j] + cx + 1;
        cell = &F_20a5_0000[((int)px[j] >> 5) * 40 + ((int)py[j] >> 4)];
        lo = ((unsigned far *)cell)[0];
        hi = ((unsigned far *)cell)[1];
        fx = px[j] & 0x1f;
        fy = py[j] & 0xf;
        if ((hi & 0x8000) == 0x8000) {
            py[j] = 0;
            px[j] = 0;
        } else {
            if ((lo & 0xf) == 0) {
                if ((lo & 0xf00) != 0 && fx < 7)
                    px[j] = px[j] & 0xffe7 | 7;
            } else if (fx > 0x17)
                px[j] = px[j] & 0xfff7 | 0x17;
            if ((hi & 0xf) == 0) {
                if ((hi & 0xf00) != 0 && (int)fy > 10)
                    py[j] = py[j] & 0xfffa | 10;
            } else if ((int)fy < 4)
                py[j] = py[j] & 0xfff4 | 4;
            if (fx > 0x16 && (((unsigned far *)(cell + 40))[1] & 0xf00) != 0 && (int)fy > 10)
                py[j] = py[j] & 0xfffa | 10;
        }
        D_e0a9 = px[j];
        D_e0a7 = py[j];
        if (sub_s014_1d69_000c(-1, -1, 0) == 1) {
            py[j] = 0;
            px[j] = 0;
        }
        if (px[j] == 0 || py[j] == 0) {
            F_20a5_3754[k][j] = 0;
            F_20a5_3854[k][j] = 0;
        } else {
            sy = py[j] - D_c293;
            F_20a5_3854[k][j] = sy + 0x65;
            sx = -D_c26c - (sy - px[j]) + 0x7c;
            F_20a5_3754[k][j] = sx;
            if (sx > 0x103 || F_20a5_3854[k][j] > 199) {
                py[j] = 0;
                px[j] = 0;
                F_20a5_3854[k][j] = 0;
                F_20a5_3754[k][j] = 0;
            }
        }
    }
}
