/* place: s005 1432:0000 */
/* orig: size=03bc */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE, no DATA DIFFERS, stamped with stamp.py --wip. all 3 functions written from the asm (dec16 drafts were stubs).
 * Battlefield generator driver: sets up the work segment (struct gwork, see below) and
 * builds the levels D_db7d..D_db7b one after another through the vec_ thunks (s006/s007).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
void *memset(void *, int, unsigned);
void far *_fmemset(void far *, int, unsigned);

/* Generator work area: one far segment gen_work (= D_c105), addressed through based
 * pointers D_db3b.. that point at the fields below (offsets fixed by 005c).
 * Index 0 of every [5] array is the level (gen_level, 0..4). */
struct gexit { int x0, x2, x4, x6, x8, x0a; };      /* 12 bytes */
struct gslot { int a[4]; int b; };                  /* 10 bytes */
struct gwork {
    int room[5][30][4];                 /* 0000 D_db3b: room rects {x0, x1, y0, y1} */
    struct gslot slot[5][16];           /* 04b0 gen_strips */
    int r7d0[5][30];                    /* 07d0 D_db89 */
    unsigned char conn[5][31][31];      /* 08fc D_db8b: room connectivity bits */
    unsigned char m1bc1[5][40][40];     /* 1bc1 gen_cell_walls: per-cell wall bits */
    int m3b01[5][40][40];               /* 3b01 gen_cell_type: per-cell type (0/1 = solid) */
    int r7981[5][30];                   /* 7981 D_db91 */
    int m7aad[5][40][40];               /* 7aad gen_cell_door */
    struct gexit ex[5][4];              /* b92d gen_exits */
};

extern _segment gen_work;
extern _segment D_c105;
extern int D_db3b, keep_x0, D_db3f, D_db41, D_db43;
extern int gen_rooms[5];                   /* rooms per level */
extern int D_db4f[5];
extern int D_db59[5];
extern int D_db63;
extern int gen_strips;
extern int D_db67[5];
extern int gen_exit_count[5];                   /* exits per level (gen_exits) */
extern int gen_level_hi;                      /* highest level built */
extern int gen_level_lo;                      /* lowest level built */
extern int D_db7f;
extern int gen_level;                      /* current level */
extern int D_db83;                      /* number of levels - 1 */
extern char D_db85, D_db86;
extern char D_db87;
extern char D_db88;
extern int D_db89, D_db8b, gen_cell_walls, gen_cell_type, D_db91, gen_cell_door, gen_exits, D_db97, D_db99;

int far sub_s004_0e76_20dc(void);
int far roll_dice(int, int);
void far sub_s005_146d_08be(int, int);
void far vec_1bad(int);
void far vec_1bb7(int);
void far vec_1bc1(void);
void far vec_1bcb(void);
void far vec_1bd5(void);
void far vec_1bdf(void);
void far vec_1be9(void);
void far vec_1bf3(int);
void far vec_1bfd(int);
void far vec_1c07(void);
void far castle_build_vec(int *, int, int, int, int, int, int, int, int, int, int);
void far gen_clear(void);
void far sub_s005_1432_022a(int);

/* generate a battlefield; returns the number of levels */
int far gen_battlefield(int a, int *p8, int pa, int pc, int pe, int p10, int p12,
                           int p14, int p16, int p18, int p1a)
{
    int n;

    gen_clear();
    sub_s005_1432_022a(a);
    if (a)
        sub_s004_0e76_20dc();
    n = gen_level_hi - gen_level_lo + 1;
    castle_build_vec(p8, a, pa, pc, pe, p10, p12, p14, p16, p18, p1a);
    *p8 -= gen_level_lo;
    return n;
}

/* clear the work area */
void far gen_clear(void)
{
    int i, j, k;
    struct gwork _based(gen_work) *GW = 0;

    gen_work = D_c105;
    gen_strips = (int)GW->slot;
    D_db89 = (int)GW->r7d0;
    gen_cell_type = (int)GW->m3b01;
    D_db91 = (int)GW->r7981;
    D_db85 = D_db86 = 0;
    D_db97 = D_db99 = D_db3b = 0;
    gen_exits = (int)GW->ex;
    gen_cell_walls = (int)GW->m1bc1;
    gen_cell_door = (int)GW->m7aad;
    D_db8b = (int)GW->conn;
    memset(gen_exit_count, 0, sizeof gen_exit_count);
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 31; j++)
            _fmemset((char far *)GW->conn[i][j], 0, 31);
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 4; k++)
                GW->room[i][j][k] = -1;
            GW->r7981[i][j] = 0;
            GW->r7d0[i][j] = 0;
        }
        gen_rooms[i] = 0;
        D_db4f[i] = -1;
        D_db59[i] = -1;
        for (j = 0; j < 40; j++) {
            _fmemset((char far *)GW->m1bc1[i][j], 0, 40);
            _fmemset((char far *)GW->m7aad[i][j], 0, 80);
            _fmemset((char far *)GW->m3b01[i][j], 0, 80);
        }
        for (j = 0; j < 16; j++) {
            for (k = 0; k < 4; k++)
                GW->slot[i][j].a[k] = -1;
            GW->slot[i][j].b = 8;
        }
        D_db67[i] = 0;
        for (j = 0; j < 4; j++) {
            GW->ex[i][j].x8 = 0;
            GW->ex[i][j].x4 = GW->ex[i][j].x2 = GW->ex[i][j].x0 = -1;
        }
    }
    D_db41 = keep_x0 = 0x29;
    D_db3f = D_db43 = 0;
}

void far sub_s005_1432_022a(int a)
{
    int i;
    int t;

    gen_level_lo = 6;
    gen_level_hi = -1;
    D_db7f = D_db83 = roll_dice(1, 2) + 2;
    D_db87 = D_db88 = 1;
    if (!a) {
        D_db83 = 1;
        D_db7f = 2;
        D_db87 = D_db88 = 0;
    }
    D_db63 = -1;
    for (i = 0; i <= D_db83; i++) {
        gen_level = i + 1;
        if (gen_level > D_db83)
            gen_level = 0;
        if (gen_level == D_db7f && !D_db87)
            continue;
        if (gen_level == 0 && !D_db88)
            continue;
        gen_level_lo = gen_level > gen_level_lo ? gen_level_lo : gen_level;
        gen_level_hi = gen_level < gen_level_hi ? gen_level_hi : gen_level;
        sub_s005_146d_08be(0x1e, 0x1e);
        if (gen_level < D_db7f)
            vec_1c07();
        vec_1bfd(a);
        D_db4f[gen_level] = gen_rooms[gen_level] - 1;
        if (gen_level == 1) {
            vec_1bf3(a);
            D_db63 = gen_rooms[gen_level] - 1;
        } else
            vec_1be9();
        if (gen_level == D_db7f) {
            D_db59[gen_level] = gen_rooms[gen_level] - 1;
            vec_1bdf();
        } else {
            vec_1bd5();
            if (gen_level >= 1)
                vec_1bcb();
            if (gen_level == 1)
                vec_1bc1();
            t = gen_level;
            D_db59[t] = gen_rooms[t] - 1;
            if (t >= 1 && t < D_db83)
                vec_1bb7(a);
            vec_1bad(a);
            if (gen_level == 0)
                D_db88 = 0;
        }
    }
    vec_1bc1();
}
