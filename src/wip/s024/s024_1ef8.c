/* place: s024 1ef8:0000 */
/* orig: size=0c92 CONST=7e02+0008 _DATA=23fc+0204 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 0/4, all functions written (functional drafts; 08f2 closest: one extra frame slot).
 * Combat level data (overlay s024, child of s014):
 * 0000 debug dump of every level's activation (.atv) and floor (.flr) records to tac.txt,
 *      then relocates the loaded floor of the current level (wall lists, trap handlers),
 * 059a switch to level n: save the current level's .flr/.atv if dirty (D_d4e4), load the new
 *      ones, mark the walls of opened doors (bit mask F_2e38_0bb6[level]),
 * 07a2 load "<name>.frc" (furniture/animation set) for the level,
 * 08f2 place combatant `who` around an activation spot (13 offsets per direction, then random
 *      tries, clearing the 3x3 floor cells after 500 tries).
 * Far data: segment 20a5 holds the current level's floor (.flr image, 0x33fc bytes at 20a5:0000)
 * and the activation records (0x1e bytes at 20a5:47ee); 2e38 holds per-level door masks. */
#include "darklands/card.h"
#include "darklands/game.h"

typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;

/* activation record (level%d.atv), 0x1e bytes */
struct atv {
    int strtx, strty, sprd;     /* +00 */
    char rm;                    /* +06 */
    char reenf;                 /* +07 AllowReenf */
    char type[3];               /* +08 */
    char num[3];                /* +0b */
    int f, g, pf;               /* +0e */
    char rtype, rsprd;          /* +14 */
    int rx, ry;                 /* +16 */
    char rrate, rmax, rdirn;    /* +1a */
    char pad;
};
struct atvtab { struct atv r[30]; };

struct item6 { char b[6]; };
struct trig { char n; unsigned char idx; char pad[7]; struct item6 it[5]; char pad2[5]; };   /* 0x2c */
struct xy10 { int x, y; char pad[6]; };
struct handler { union { int type; void (far *fn)(void); } u; char pad[10]; };    /* 0x0e */

/* level floor image (level%d.flr), lives at 20a5:0000 */
struct flr {
    char pad0[0x1f42];
    int v, s, r, d, e;          /* +1f42 */
    int f1f4c, f1f4e;           /* +1f4c */
    char f1f50;                 /* +1f50 */
    char door;                  /* +1f51 */
    char pad1f52[0x2184 - 0x1f52];
    char nhandler;              /* +2184 */
    char pad2185[7];
    struct handler h[50];       /* +218c */
    char pad[0x2488 - 0x218c - 50 * 0x0e];
    char nlist;                 /* +2488 */
    char pad2489;
    int list[64];               /* +248a */
    char pad250a[0x300c - 0x250a];
    char ntrig;                 /* +300c */
    char pad300d[0x0c];
    struct trig t[8];           /* +3019 */
    char pad30[0x3356 - 0x3019 - 8 * 0x2c];
    struct xy10 furn[5];        /* +3356 */
    struct xy10 wall[7];        /* +3388 */
    char pad33ce[3];
    char f33d1;                 /* +33d1 */
    char f33d2;                 /* +33d2 */
};
struct cell { char b0, b1, b2, b3; };
struct frc {
    int a[4];                   /* +00 */
    int b[4];                   /* +08 */
    int c[4];                   /* +10 */
    int d[4];                   /* +18 */
    char f20, f21;
    int size;                   /* +22 */
    int f24;                    /* +24 */
    int off;                    /* +26 */
};
struct cmb {
    char pad0[4];
    int x, y;                   /* +04 */
    char pad8[0x11];
    char f19, f1a;              /* +19 */
    char pad1b[0x65];
};
struct spot { char pad[8]; int x, y; unsigned char dir; };

extern struct atvtab far activations;
extern struct flr far floor_img;
extern struct { long m[16]; } far doors_opened;
extern struct { int w[1]; } far F_20a5_000;
extern struct cell far F_20a5_0[][40];
extern char far F_2e38_0c00[];
extern struct { char b[14]; } far F_20a5_1f60[];
extern char far F_20a5_33c8[];
extern char far floor_doors[];
extern char floor_shown;
extern unsigned char msg_file, activation_count;
extern char D_a896[];
extern char *loot_list;
extern int D_c7a4[];
extern char D_e230, D_07d3, floor_dirty;
extern int D_d6ee;
extern char far *load_buf;
extern int D_c8d5[];
extern char D_c12f[], D_c11d[], D_c907;
extern char D_d7be[][4], D_d802[][4], D_c13f[][4], D_c181[][4];
extern struct cmb combatants[];
extern int los_y0, los_x0, los_y1, los_x1;
extern int D_d645[], D_d664[];

FILE *fopen(const char *, const char *);
int fclose(FILE *);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);
void far *_fmemcpy(void far *, const void far *, unsigned);
void far *_fmemset(void far *, int, unsigned);
int far random_n(int);
void far swap_load2_vec(char *, char far *);
void far swap_save2_vec(char *, char far *, long);
void far level_enemies_load_vec(int);
void far secret_door_search_vec(void), vec_17f7(void), trap_spell_vec(void), trap_missile_vec(void);
void far vec_181f(void), vec_1815(void), vec_1833(void), vec_1829(void);
int far sub_s014_19cd_3204(char *, int);
void far effect_start(int, int, int);
void far datafile_select(char *);
void far file_read_all(char *, char far *);
int far dir_sub(int, int);
int far cbt_collisions(struct cmb *);
int far line_blocked(int, int, int);
void far sub_s024_1ef8_0000(int, int);
void far level_switch(int);
void far furniture_load(void);
int far place_around(int, struct spot *);

void far sub_s024_1ef8_0000(int n, int cur)
{
    char name[0x42];
    FILE *fp;
    char far *base;
    int lvl, i, j;
    struct atv far *a;
    struct trig far *t;

    base = (char far *)&floor_img + 0x24c6;
    floor_shown = cur;
    for (lvl = 0; lvl < n; lvl++) {
        fp = fopen("tac.txt", "a");
        fprintf(fp, "Enemy Activation Record for level %d\n", lvl);
        sprintf(name, "level%d.atv", lvl);
        swap_load2_vec(name, (char far *)&msg_file);
        _fmemcpy(&activations, D_a896, (activation_count = msg_file) * 30);
        fprintf(fp, "\tNumActivationSpots: %d\n", activation_count);
        for (i = 0; i < activation_count; i++) {
            fprintf(fp, "\tstrtx: %d\n", activations.r[i].strtx);
            fprintf(fp, "\tstrty: %d\n", activations.r[i].strty);
            fprintf(fp, "\tsprd: %d\n", activations.r[i].sprd);
            fprintf(fp, "\trm: %d\n", activations.r[i].rm);
            fprintf(fp, "\tAllowReenf: %d\n", activations.r[i].reenf);
            fprintf(fp, "\ttype[0]: %d\n", activations.r[i].type[0]);
            fprintf(fp, "\ttype[1]: %d\n", activations.r[i].type[1]);
            fprintf(fp, "\ttype[2]: %d\n", activations.r[i].type[2]);
            fprintf(fp, "\tnum/type[0]: %d\n", activations.r[i].num[0]);
            fprintf(fp, "\tnum/type[1]: %d\n", activations.r[i].num[1]);
            fprintf(fp, "\tnum/type[2]: %d\n", activations.r[i].num[2]);
            fprintf(fp, "\tF: %d\tG: %d Pf: %d\n", activations.r[i].f, activations.r[i].g,
                    activations.r[i].pf);
            if (activations.r[i].reenf) {
                fprintf(fp, "\t\tRtype: %d\n", activations.r[i].rtype);
                fprintf(fp, "\t\tRsprd: %d\n", activations.r[i].rsprd);
                fprintf(fp, "\t\tRx: %d\n", activations.r[i].rx);
                fprintf(fp, "\t\tRy: %d\n", activations.r[i].ry);
                fprintf(fp, "\t\tRrate: %d\n", activations.r[i].rrate);
                fprintf(fp, "\t\tRmax: %d\n", activations.r[i].rmax);
                fprintf(fp, "\t\tRdirn: %d\n", activations.r[i].rdirn);
            }
        }
        fclose(fp);
        sprintf(name, "level%d.flr", lvl);
        swap_load2_vec(name, (char far *)&floor_img);
        _fmemset(floor_doors, 0, 0x28);
        fp = fopen("tac.txt", "a");
        fprintf(fp, "Level Record:%d\n", lvl);
        fprintf(fp, "V:%d S:%d R:%d D%d E:%d\n", floor_img.v, floor_img.s, floor_img.r,
                floor_img.d, floor_img.e);
        fprintf(fp, "FurnRemove\n");
        for (i = 0; i < 5; i++)
            fprintf(fp, "x:%d y:%d\n", floor_img.furn[i].x, floor_img.furn[i].y);
        fprintf(fp, "WallRemove\n");
        for (i = 0; i < 7; i++)
            fprintf(fp, "x:%d y:%d\n", floor_img.wall[i].x, floor_img.wall[i].y);
        fclose(fp);
        level_enemies_load_vec(lvl);
        for (j = 0; j < floor_img.ntrig; j++) {
            t = &floor_img.t[j];
            if (t->n)
                for (i = 0; i < (unsigned char)t->n; i++)
                    ((struct item6 *)loot_list)[i] = floor_img.t[j].it[i];
            D_c7a4[floor_img.t[j].idx] = sub_s014_19cd_3204(loot_list, (unsigned char)t->n);
        }
        for (i = 0; i < floor_img.nlist; i++)
            floor_img.list[i] += (int)base;
        for (i = 0; i < floor_img.nhandler; i++)
            switch (floor_img.h[i].u.type) {
            case 0:
                floor_img.h[i].u.fn = secret_door_search_vec;
                break;
            case 1:
                floor_img.h[i].u.fn = vec_17f7;
                break;
            case 2:
                floor_img.h[i].u.fn = trap_spell_vec;
                break;
            case 4:
                floor_img.h[i].u.fn = trap_missile_vec;
                break;
            case 5:
                floor_img.h[i].u.fn = vec_181f;
                break;
            case 6:
                floor_img.h[i].u.fn = vec_1815;
                break;
            case 7:
                floor_img.h[i].u.fn = vec_1833;
                break;
            case 8:
                floor_img.h[i].u.fn = vec_1829;
                break;
            }
        swap_save2_vec(name, (char far *)&floor_img, 0x33fcL);
        if (lvl == cur) {
            char door = floor_img.door;
            if (door >= 0)
                doors_opened.m[floor_shown] |= 1L << door;
            effect_start(floor_img.f1f4c, floor_img.f1f4e, (unsigned char)floor_img.f1f50);
        }
    }
}

void far level_switch(int level)
{
    char name[0x42];
    long mask;
    int i, k, j;

    if (level == -1)
        return;
    D_e230 = 1;
    D_07d3++;
    if (floor_dirty == 1) {
        sprintf(name, "level%d.flr", floor_shown);
        swap_save2_vec(name, (char far *)&floor_img, 0x33fcL);
        sprintf(name, "level%d.atv", floor_shown);
        _fmemcpy(D_a896, &activations, (msg_file = activation_count) * 30);
        swap_save2_vec(name, (char far *)&msg_file, (unsigned long)activation_count * 30 + 1);
    }
    floor_dirty = 0;
    sprintf(name, "level%d.flr", level);
    swap_load2_vec(name, (char far *)&floor_img);
    _fmemset(F_2e38_0c00, 0, 0x28);
    for (i = 0; i < 40; i++)
        if (F_2e38_0c00[i] == 0 && F_20a5_1f60[i].b[0] != -1)
            F_2e38_0c00[F_20a5_1f60[i].b[0]] = 1;
    floor_shown = level;
    mask = doors_opened.m[level];
    for (k = 0; k < floor_img.nlist; k++) {
        if (!(mask & 1)) {
            j = 0;
            if (F_20a5_000.w[floor_img.list[k] / 2] != -1)
                for (j = 0; *(int far *)((char far *)&F_20a5_000 + floor_img.list[k] + j * 2) != -1; j++) {
                    int v = *(int far *)((char far *)&F_20a5_000 + floor_img.list[k] + j * 2);
                    F_20a5_0[v >> 8][v & 0xff].b3 |= 0x80;
                }
        }
        mask >>= 1;
    }
    sprintf(name, "level%d.atv", level);
    swap_load2_vec(name, (char far *)&msg_file);
    _fmemcpy(&activations, D_a896, (activation_count = msg_file) * 30);
    if (D_d6ee)
        furniture_load();
    D_07d3--;
}

void far furniture_load(void)
{
    char name[0x1c];
    struct frc far *f;
    union { char far *p; struct { int off, seg; } w; } dst;
    int k, t, j;

    f = (struct frc far *)load_buf;
    datafile_select("bc");
    sprintf(name, "%Fs.frc", F_20a5_33c8);
    file_read_all(name, load_buf);
    k = floor_img.f33d2;
    dst.w.seg = D_c8d5[floor_img.f33d1] = D_d6ee;
    dst.w.off = 0;
    _fmemcpy(dst.p, load_buf + f[k].off, f[k].size);
    t = floor_img.f33d1;
    D_c12f[t] = f[k].f20;
    D_c11d[t] = f[k].f21;
    if (f[k].f24 == 1)
        D_c907 = 1;
    for (j = 0; j < 4; j++) {
        D_d7be[t][j] = f[k].a[j];
        D_d802[t][j] = f[k].b[j];
        D_c13f[t][j] = f[k].c[j];
        D_c181[t][j] = f[k].d[j];
    }
}

int far place_around(int who, struct spot *a)
{
    int dx[13], dy[13];
    int ox, oy, i, done;
    register int x, y;

    ox = combatants[who].x;
    oy = combatants[who].y;
    combatants[who].f19 = combatants[who].f1a = dir_sub(a->dir, 4);
    switch (a->dir) {
    case 0:
        dy[3] = dy[4] = dy[5] = -8;
        dy[6] = dy[7] = dy[8] = -16;
        dy[9] = dy[10] = dy[11] = -24;
        dy[12] = -32;
        dx[0] = dx[3] = dx[6] = dx[9] = dy[0] = dy[1] = dy[2] = 0;
        dx[1] = dx[4] = dx[7] = dx[10] = -12;
        dx[2] = dx[5] = dx[8] = dx[11] = dx[12] = 12;
        break;
    case 2:
        dy[1] = dy[4] = dy[7] = dy[10] = -8;
        dy[2] = dy[5] = dy[8] = dy[11] = dy[12] = 8;
        dx[0] = dx[1] = dx[2] = dy[0] = dy[3] = dy[6] = dy[9] = 0;
        dx[3] = dx[4] = dx[5] = 12;
        dx[6] = dx[7] = dx[8] = 24;
        dx[9] = dx[10] = dx[11] = 36;
        dx[12] = 48;
        break;
    case 4:
        dy[3] = dy[4] = dy[5] = 8;
        dy[6] = dy[7] = dy[8] = 16;
        dy[9] = dy[10] = dy[11] = 24;
        dy[12] = 32;
        dx[0] = dx[3] = dx[6] = dx[9] = dy[0] = dy[1] = dy[2] = 0;
        dx[1] = dx[4] = dx[7] = dx[10] = 12;
        dx[2] = dx[5] = dx[8] = dx[11] = dx[12] = -12;
        break;
    case 6:
        dy[1] = dy[4] = dy[7] = dy[10] = 8;
        dy[2] = dy[5] = dy[8] = dy[11] = dy[12] = -8;
        dx[0] = dx[1] = dx[2] = dy[0] = dy[3] = dy[6] = dy[9] = 0;
        dx[3] = dx[4] = dx[5] = -12;
        dx[6] = dx[7] = dx[8] = -24;
        dx[9] = dx[10] = dx[11] = -36;
        dx[12] = -48;
        break;
    }
    if (who < 5) {
        for (i = 0; i < 13; i++) {
            combatants[who].x = dx[i] + a->x;
            combatants[who].y = dy[i] + a->y;
            if (cbt_collisions(&combatants[who]) == 0)
                goto ok;
        }
        los_x0 = a->x;
        los_y0 = a->y;
        done = i = 0;
        for (;;) {
            combatants[who].x = random_n(48) + los_x0;
            combatants[who].y = random_n(24) + los_y0;
            if (cbt_collisions(&combatants[who]) == 0) {
                los_x1 = combatants[who].x;
                los_y1 = combatants[who].y;
                if (line_blocked(-1, 0, 0) == 0)
                    goto ok;
            }
            if (++i > 500 && done == 0) {
                x = a->x >> 5;
                y = a->y >> 4;
                F_20a5_0[x - 1][y - 1].b0 &= 0x0f;
                F_20a5_0[x - 1][y].b0 &= 0x0f;
                F_20a5_0[x - 1][y + 1].b0 &= 0x0f;
                F_20a5_0[x][y - 1].b0 &= 0x0f;
                F_20a5_0[x][y].b0 &= 0x0f;
                F_20a5_0[x][y + 1].b0 &= 0x0f;
                F_20a5_0[x + 1][y - 1].b0 &= 0x0f;
                F_20a5_0[x + 1][y].b0 &= 0x0f;
                F_20a5_0[x + 1][y + 1].b0 &= 0x0f;
                done = 1;
                floor_dirty = 1;
                i = 0;
                continue;
            }
            if (i > 500)
                break;
        }
    } else {
        for (i = 12; i > 0; i--) {
            combatants[who].x = dx[i] + a->x;
            combatants[who].y = dy[i] + a->y;
            if (cbt_collisions(&combatants[who]) == 0)
                goto ok;
        }
    }
    combatants[who].x = ox;
    combatants[who].y = oy;
    return 0;
ok:
    D_d645[who] = combatants[who].x;
    D_d664[who] = combatants[who].y;
    return 1;
}
