/* place: s004 0e76:0000 */
/* orig: size=21ba CONST=7d28+0002 _DATA=1382+0075 */
/* data: CONST=7d28 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE (11/11 written), 4/11 match (1134, 20f6, 213c, 2160); stamped.
 * Written from the asm (dec16 drafts were stubs, Ghidra output unusable). Battlefield loader: 0000
 * loads darkland.enm (enemy types D_db31, monster templates D_db2f, tile descriptions D_db33) and
 * darkland.lst (items D_db35), builds the battlefield of type t through the generator overlays
 * (vec_1b49..vec_1b99), fixes up the monster records (122c, 1d90) and writes terrain.fil and
 * level%d.trn / level%d.atv. Enemy buffer (D_dadd, 0xf5c8 bytes): D_db29 int[16][12] template per
 * monster slot, D_db2b struct mon[16][12] (0xd6 bytes), D_db2d struct group[16][10] (0x1e bytes),
 * D_db2f struct mtpl[] (0xcc), D_db31 struct etype[0x52] (0x18), D_db33 tile descriptions.
 * Counts: D_dae5[16] monsters, D_db05[16] groups per level.
 * 1134 and 2160 match only through symbol-table effects (the unused `extern int D_ffe0;` position,
 * the declarations of 06aa/122c): re-check them after adding or renaming any symbol above them.
 * Near misses: 1d90 (2: 6.00A merges the x0e zero store), 20dc (bx vs dx), 0000 (31: D_c103 CSE
 * temp), 122c (163: jump table addresses, cross-block CSE), 06aa (~170, 6.00A C4203 "too large for global
 * optimizations"). 04cc/1060: memset of D_dae5/D_db05 comes out as `push ss` (DS is switched in these
 * functions, cport.compat600 only rewrites the `push ds` form) and far memsets (6.00A limit).
 * Locals of 06aa/122c/1d90 are renamed to 6.00A's name-hash slot order (build/agent/e6/hord.py).
 */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };

struct item {                           /* 6 bytes, bf_items index + data */
    int id;
    unsigned char b2, b3, b4;
    char b5;
};
struct mon {                            /* 0xd6 bytes */
    unsigned x0;                        /* 00 */
    unsigned x2;                        /* 02 */
    unsigned x4;                        /* 04: level bits */
    int x6;                             /* 06 */
    char x8;                            /* 08 */
    char x9[3];                         /* 09 */
    char x0c;
    unsigned char x0d;                  /* 0d: 0/1/2 */
    char x0e[4];                        /* 0e: template name */
    char x12[0x14];                     /* 12: type name */
    char x26, x27;
    char x28, x29, x2a, x2b, x2c, x2d, x2e;
    char x2f[2], x31[2];                /* 2f, 31 */
    char x33, x34, x35;
    char x36[7], x3d[7];                /* 36, 3d */
    char x44[19];                       /* 44 */
    char x57;
    char x58[2], x5a[2], x5c[2];
    int x5e;
    char x60[10], x6a[10];
    struct item x74[16];                /* 74 */
    unsigned char xd4;                  /* d4: items */
    char xd5;
};
struct group {                          /* 0x1e bytes */
    int x0, x2, x4;
    char x6, x7;
    char type[3];                       /* 08 */
    char cnt[3];                        /* 0b */
    int x0e, x10, x12;
    char x14, x15;
    int x16, x18;
    char x1a, x1b, x1c, x1d;
};
struct dice { char a, b, c; };
struct mtpl {                           /* 0xcc bytes, monster template */
    char x0[0x0e];                      /* 00: name */
    char x0e, x0f, x10, x11, x12, x13;
    char x14[7];                        /* 14 */
    char x1b[19];                       /* 1b */
    char x2e, x2f;
    int x30;                            /* 30 */
    struct dice x32[0x16];              /* 32 */
    struct dice x74[10];                /* 74 */
    char x92[2], x94[2];                /* 92, 94 */
    char x96, x97, x98, x99, x9a, x9b, x9c, x9d, x9e;
    unsigned char x9f;
    char xa0[6], xa6[6];                /* a0, a6 */
    char xac;
    struct { char a, b; } xad[2];       /* ad */
    char xb1, xb2, xb3, xb4, xb5, xb6;
    char pb7[2];
    char xb9;
    char xba[2], xbc[2], xbe[2];        /* ba */
    int xc0, xc2, xc4, xc6, xc8, xca;
};
struct etype {                          /* 0x18 bytes */
    int x0;                             /* template */
    char x2[0x14];                      /* name */
    unsigned char x16;                  /* bits 0-5 weapon slots, 0x40, 0x80 */
    char x17;
};
struct itype { char p[0x25]; unsigned char x25; char p26[0x2e - 0x26]; };
struct tdesc { char b[0xc4]; };
struct find_t { char reserved[21]; char attrib; unsigned wr_time, wr_date; long size; char name[13]; };

extern int cur_loc;
extern char combat_flags;
extern int D_ffe0;                      /* unused: symbol-table position (2160 compare direction) */
extern int D_da34;
extern int D_db27;
extern unsigned D_dadb;
extern _segment enemy_seg;
extern unsigned D_dae1;
extern _segment D_dae3;
extern int mon_count[16];
extern int group_count[16];
extern int _based(enemy_seg) *mon_template;
extern struct mon _based(enemy_seg) *bf_monsters;
extern struct group _based(enemy_seg) *bf_groups;
extern struct mtpl _based(enemy_seg) *mon_templates;
extern struct etype _based(enemy_seg) *enemy_types;
extern struct tdesc _based(enemy_seg) *tile_desc;
extern struct itype _based(D_dae3) *bf_items;
extern char far *load_buf;
extern char far F_20a5_75ac[];

#define CL(a) ((a) > 0x7f ? 0x7f : (a))

unsigned char tab_13d0[32] = {     /* unreferenced here */
    0x00, 0x0e, 0x0e, 0x0e, 0xfe, 0xff, 0x0e, 0x0e, 0x54, 0x5f, 0x04, 0x04, 0x54, 0x5f, 0x04, 0x04,
    0x54, 0x5f, 0x5f, 0x5f, 0xff, 0xff, 0x5f, 0x5f, 0x54, 0x5f, 0x04, 0x04, 0x54, 0x5f, 0x04, 0x04
};

unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
int far _dos_findfirst(char *, unsigned, struct find_t *);
int far rand(void);
int far sprintf();
void far srand(unsigned);
char far *far _fstrcpy(char far *, char far *);
void far *far _fmemcpy(void far *, void far *, unsigned);
void far *far _fmemset(void far *, int, unsigned);
int far datafile_select(char *);
void *memset(void *, int, unsigned);
int abs(int);
void far file_load_far(char *, char far *);
void far file_save(char *, char far *, int);
int far sub_s004_1262_0d0a(char *, void far *, long);
int far vec_1b49();
int far vec_1b53();
int far templar_build_vec();
int far vec_1b67();
int far vec_1b71();
int far city_build_vec();
int far mine_build_vec();
int far gen_battlefield_vec();
int far empty_battlefield_vec();
void far monsters_clear(void);
int far monster_type_map(int, int, int);
void far groups_clear(void);
void far sub_s004_0e76_1134(void);
void far monsters_fixup(int, int);
void far groups_fixup(int);
void far sub_s004_0e76_20dc(void);
int far roll_dice(int, int);
int far sub_s004_0e76_213c(int, int, int, int);
int far dir_between(int, int, int, int);

/* build the battlefield of type t; returns the number of levels written */
#pragma optimize("a", on)
int far battlefield_build(int t, int seed, int *res, int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9)
{
    char *nm1 = "Sc";
    char *nm2 = "II";
    int r;
    int n;
    int i;
    char cnt;
    char name[30];
    struct find_t ff;

    enemy_seg = (_segment)(D_dadb = alloc_buffer(0xf5c8L, nm1));
    mon_template = 0;
    bf_monsters = (struct mon _based(enemy_seg) *)0x180;
    bf_groups = (struct group _based(enemy_seg) *)0xa200;
    mon_templates = (struct mtpl _based(enemy_seg) *)0xb4c0;
    enemy_types = (struct etype _based(enemy_seg) *)0xed54;
    tile_desc = (struct tdesc _based(enemy_seg) *)0xf504;
    file_load_far("darkland.enm", (char far *)mon_templates);
    sprintf(name, "darkland.lst");
    _dos_findfirst(name, 0, &ff);
    D_dae3 = (_segment)(D_dae1 = alloc_buffer(ff.size, nm2));
    bf_items = 0;
    file_load_far(name, (char far *)bf_items);
    bf_items = (struct itype _based(D_dae3) *)3;
    datafile_select("imaps.cat");
    srand(cur_loc);
    r = rand();
    srand(seed);
    monsters_clear();
    groups_clear();
    sub_s004_0e76_1134();
    switch (t) {
    default:
        n = empty_battlefield_vec(t, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
    case 0x27: case 0x28:
    case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e:
        n = gen_battlefield_vec(t, res, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        break;
    case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: case 0x10:
    case 0x66: case 0x67: case 0x68: case 0x69: case 0x6c: case 0x6d:
    case 0x7f: case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86:
    case 0x87: case 0x88: case 0x89:
        n = mine_build_vec(t, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
    case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: case 0x20:
    case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26:
    case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d:
    case 0x56: case 0x6a: case 0x6b:
        n = city_build_vec(t, res, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        break;
    case 0x2e: case 0x2f: case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
    case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d:
    case 0x3e: case 0x3f: case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
    case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d:
    case 0x4e: case 0x4f: case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55:
    case 0x61: case 0x62: case 0x63:
    case 0x6f: case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76:
    case 0x77: case 0x78: case 0x79:
        n = vec_1b71(t, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x6e:
        n = vec_1b67(t, res, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    case 0x5e:
        n = templar_build_vec(t, res, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        break;
    case 0x5f:
        n = vec_1b53(t, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    case 0x60: case 0x64: case 0x65:
        n = vec_1b49(t, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        *res = 0;
        break;
    }
    monsters_fixup(n, r);
    groups_fixup(n);
    sprintf(name, "terrain.fil");
    file_save(name, (char far *)tile_desc, sizeof(struct tdesc));
    for (i = 0; i < n; i++) {
        sprintf(name, "level%d.trn", i);
        cnt = mon_count[i];
        *load_buf = cnt;
        _fmemcpy(load_buf + 1, bf_monsters + i * 12, cnt * sizeof(struct mon));
        sub_s004_1262_0d0a(name, load_buf, cnt * sizeof(struct mon) + 1);
        sprintf(name, "level%d.atv", i);
        cnt = group_count[i];
        *load_buf = cnt;
        _fmemcpy(load_buf + 1, bf_groups + i * 10, cnt * 30);
        sub_s004_1262_0d0a(name, load_buf, (long)(30 * cnt + 1));
    }
    free_buffer(D_dadb, nm1);
    free_buffer(D_dae1, nm2);
    return n;
}
#pragma optimize("", on)
#pragma optimize("", on)
#pragma optimize("", on)

/* clear the monster slots and set up the default monster record */
void far monsters_clear(void)
{
    struct mon far *m;
    struct item far *it;
    int i, k, o, o2;

    memset(mon_count, 0, sizeof(mon_count));
    for (i = 0; i < 16; i++)
        _fmemset(mon_template + i * 12, -1, 24);
    m = bf_monsters;
    m->x0 = -1;
    m->x2 = -1;
    m->x4 = 0;
    m->x6 = 0;
    m->x8 = 0;
    _fmemset(m->x9, -1, 3);
    m->x28 = -1;
    m->x2a = -1;
    m->x29 = 0x19;
    m->x2b = 0x19;
    m->x0d = 0;
    m->x26 = 0;
    m->x2d = 0;
    m->x0c = 0;
    m->x0e[0] = 0;
    m->x12[0] = 0;
    m->x27 = 0;
    m->x2c = 0;
    m->x2e = 0;
    for (i = 0; i < 2; i++) {
        m->x2f[i] = -1;
        m->x31[i] = 0x19;
    }
    m->x33 = 0;
    m->x34 = 0x19;
    m->x35 = -1;
    for (i = 0; i < 7; i++) {
        m->x3d[i] = 0x14;
        m->x36[i] = m->x3d[i];
    }
    _fmemset(m->x44, 0x14, 19);
    m->xd4 = 0;
    m->x57 = 0;
    for (i = 0; i < 2; i++) {
        m->x58[i] = 0;
        m->x5a[i] = 0;
        m->x5c[i] = 0;
    }
    for (i = 0; i < 10; i++) {
        m->x60[i] = 0;
        m->x6a[i] = 0x14;
    }
    m->x5e = 0;
    m->x74[0].id = -1;
    it = &m->x74[0];
    it->b2 = 0xff;
    it->b3 = 0x19;
    it->b4 = 0;
    it->b5 = 0;
    for (k = 1; k < 16; k++)
        m->x74[k] = m->x74[0];
    for (o = 1; o < 12; o++)
        bf_monsters[o] = bf_monsters[0];
    o2 = 12;
    for (k = 15; k; k--)
        for (i = 12; i; i--)
            bf_monsters[o2++] = bf_monsters[0];
}

/* add a monster of enemy type t (at most n variants) to level l; returns its slot */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
int far monster_type_map(int l, int t, int n)
{
    int unm;
    struct etype far *el;
    char mask2d;
    struct mon far *mg;
    int k0j;
    int kk;
    struct mtpl far *tpd;
    struct item far *itl;
    int ia;
    char mask1o;
    int vg;
    int jd;

    if (t < 0 || t >= 0x52)
        return -1;
    el = enemy_types + t;
    if (!(el->x16 & 0x3f)) {
        goto ec;
    eb:
        if (el->x16 & 0x3f)
            goto ex;
        el--;
    ec:
        if (el > (struct etype far *)enemy_types)
            goto eb;
    }
ex:
    kk = el->x0;
    goto kc;
kb:
    if (kk <= 0)
        goto kx;
    kk--;
kc:
    if (mon_templates[kk].x0e < 0)
        goto kb;
kx:
    n = max(min(mon_templates[kk].x0e, n), 1);
    k0j = kk;
    kk += *(volatile int *)&n - 1;
    for (ia = 0; ia < mon_count[l]; ia++)
        if (mon_template[l * 12 + ia] == kk)
            return ia;
    tpd = mon_templates + kk;
    mg = bf_monsters + (l * 12 + mon_count[l]);
    mg->x2 = k0j << 8;
    mg->x0 = k0j << 8;
    mg->x4 = 1 << l;
    mg->x8 = tpd->x2e;
    ia = roll_dice(1, tpd->x0f);
    if (t == 0x4d)
        ia = 3;
    jd = tpd->x11 + (ia - 1) * tpd->x10;
    for (ia = 0; ia < tpd->x10; ia++)
        mg->x9[ia] = (char)jd + (char)ia;
    _fstrcpy(mg->x0e, tpd->x0);
    _fstrcpy(mg->x12, el->x2);
    mg->x26 = tpd->x13;
    mg->x27 = tpd->x12;
    ia = 0;
    mask2d = mask1o = 0;
    for (; ia < 6 && tpd->xa0[ia] >= 0; ia++) {
        if (el->x16 & (1 << ia)) {
            mask2d |= 1 << ia;
            for (jd = 0; jd < mon_count[l]; jd++) {
                vg = mon_template[l * 12 + jd];
                if (vg >= k0j && vg <= mon_templates[k0j].x0e + k0j - 1
                    && bf_monsters[l * 12 + jd].x28 != tpd->xa0[ia])
                    mask1o |= 1 << ia;
            }
        }
    }
    if (!mask1o)
        mask1o = mask2d;
    if (!mask2d)
        jd = 0;
    else
        do {
            jd = roll_dice(1, ia) - 1;
        } while (!((1 << jd) & (unsigned char)mask1o));
    mg->x28 = tpd->xa0[jd];
    if (el->x16 & 0x80) {
        mg->x0 |= 0x84;
        mg->x0d = 2;
        mg->x6 = 0x80;
        mg->x27 = 2;
    } else if (el->x16 & 0x40) {
        mg->x0 |= (unsigned char)tpd->x9a;
        mg->x0d = 1;
        mg->x6 = 0x80;
        mg->x27 = 2;
    } else {
        mg->x0 |= (unsigned char)mg->x28;
        mg->x0d = 0;
    }
    mg->x2 |= (unsigned char)mg->x28;
    mg->x29 = tpd->xac;
    if ((ia = tpd->xb3) != -1) {
        itl = &mg->x74[mg->xd4];
        if ((vg = ia >= 0 ? ia : mg->x28) >= 0) {
            itl->b2 = vg;
            itl->id = tpd->xa6[jd];
            itl->b3 = (ia = tpd->xb4) >= 0 ? ia : tpd->xac;
            itl->b4 = 1;
            itl->b5 = bf_items[itl->id].x25;
            mg->x2e += itl->b5;
            mg->xd4++;
        }
    }
    if (mg->x0d == 1) {
        mg->x2a = tpd->x9a;
        mg->x2b = tpd->x9c;
        if ((ia = tpd->xb5) != -1) {
            itl = &mg->x74[mg->xd4];
            if ((vg = ia >= 0 ? ia : mg->x2a) >= 0) {
                itl->b2 = vg;
                itl->id = tpd->x9b;
                itl->b3 = (ia = tpd->xb6) >= 0 ? ia : tpd->x9c;
                itl->b4 = 1;
                itl->b5 = bf_items[itl->id].x25;
                mg->x2e += itl->b5;
                mg->xd4++;
            }
        }
    }
    if (tpd->x9d >= 0 && tpd->x9e >= 0) {
        itl = &mg->x74[mg->xd4];
        itl->b2 = tpd->x9d;
        itl->b3 = 0x19;
        itl->id = tpd->x9e;
        itl->b4 = tpd->x9f;
        itl->b5 = bf_items[itl->id].x25 * itl->b4;
        mg->x2e += itl->b5;
        mg->xd4++;
    }
    for (ia = 0; ia < 2; ia++) {
        mg->x2f[ia] = tpd->x92[ia];
        mg->x31[ia] = tpd->x96;
        if ((jd = tpd->xad[ia].a) == -1)
            continue;
        itl = &mg->x74[mg->xd4];
        if ((vg = jd >= 0 ? jd : mg->x2f[ia]) < 0)
            continue;
        itl->b2 = vg;
        itl->id = tpd->x94[ia];
        itl->b3 = (jd = tpd->xad[ia].b) >= 0 ? jd : tpd->x96;
        itl->b4 = 1;
        if (ia == 0 && itl->b2 <= 0x4b || ia == 1 && itl->b2 <= 0x54) {
            itl->b5 = bf_items[itl->id].x25;
            mg->x2e += itl->b5;
        }
        mg->xd4++;
    }
    mg->x35 = tpd->x97;
    mg->x34 = tpd->x99;
    if ((ia = tpd->xb1) != -1 && tpd->x99 != -1) {
        itl = &mg->x74[mg->xd4];
        if ((vg = ia >= 0 ? ia : mg->x35) >= 0) {
            itl->b2 = vg;
            itl->id = tpd->x98;
            itl->b3 = (ia = tpd->xb2) >= 0 ? ia : tpd->x99;
            itl->b4 = 1;
            itl->b5 = bf_items[itl->id].x25;
            mg->x2e += itl->b5;
            mg->xd4++;
        }
    }
    for (ia = 0; ia < 7; ia++) {
        mg->x3d[ia] = tpd->x14[ia];
        mg->x36[ia] = mg->x3d[ia];
    }
    for (ia = 0; ia < 19; ia++)
        mg->x44[ia] = tpd->x1b[ia];
    mg->x57 = tpd->xb9;
    for (ia = 0; ia < 2; ia++) {
        mg->x58[ia] = tpd->xba[ia];
        mg->x5a[ia] = tpd->xbc[ia];
        mg->x5c[ia] = tpd->xbe[ia];
    }
    mg->x5e = tpd->x30;
    for (ia = 0; ia < 10; ia++) {
        if ((jd = roll_dice(1, tpd->x74[ia].a) + tpd->x74[ia].b) > 0) {
            mg->x60[ia] = jd;
            mg->x6a[ia] = tpd->x74[ia].c;
        }
    }
    for (ia = 0; mg->xd4 < 16; ia++) {
        if (ia >= 0x16)
            break;
        if ((jd = roll_dice(1, tpd->x32[ia].a) + tpd->x32[ia].b) > 0) {
            itl = &mg->x74[mg->xd4];
            itl->b2 = ia - 0x7c;
            itl->b4 = jd;
            itl->b3 = tpd->x32[ia].c;
            itl->id = ia + 0x5f;
            itl->b5 = bf_items[itl->id].x25;
            mg->x2e += (char)jd * itl->b5;
            mg->xd4++;
        }
    }
    if (k0j == 0x3d) {
        itl = &mg->x74[mg->xd4];
        itl->b2 = 0x98;
        itl->b4 = 1;
        itl->b3 = 0x28;
        itl->id = 0x73;
        itl->b5 = bf_items[itl->id].x25;
        mg->x2e += itl->b5;
        mg->xd4++;
    }
    mon_template[l * 12 + (jd = mon_count[l]++)] = kk;
    return jd;
}

/* clear the groups and set up the default group record */
void far groups_clear(void)
{
    struct group far *g;
    int i, k, o;

    memset(group_count, 0, sizeof(group_count));
    g = bf_groups;
    g->x0 = 0;
    g->x6 = -1;
    g->x7 = 0;
    g->x2 = 0;
    g->x4 = 0;
    for (i = 0; i < 3; i++) {
        g->type[i] = -1;
        g->cnt[i] = 0;
    }
    g->x0e = 0;
    g->x10 = 0;
    g->x12 = 0;
    g->x16 = 0;
    g->x18 = 0;
    g->x15 = 0;
    g->x1a = 0;
    g->x1b = 0;
    g->x14 = -1;
    g->x1c = -1;
    for (i = 1; i < 10; i++)
        bf_groups[i] = bf_groups[0];
    o = 10;
    for (k = 15; k; k--)
        for (i = 10; i; i--)
            bf_groups[o++] = bf_groups[0];
}

/* default tile descriptions */
#pragma optimize("a", on)
void far sub_s004_0e76_1134(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        tile_desc->b[i + 0xa4] = tile_desc->b[i + 0x8a] = tile_desc->b[i + 0x70] = tile_desc->b[i + 0x57] =
            tile_desc->b[i + 0x3d] = tile_desc->b[i + 0x24] = tile_desc->b[i + 0x0a] = -1;
    }
    *(char volatile _based(enemy_seg) *)&tile_desc->b[0] = 0;
    tile_desc->b[0x23] = 2;
    tile_desc->b[0xa3] = tile_desc->b[0x89] = tile_desc->b[0x56] = 8;
    tile_desc->b[0xb4] = 0;
    _fstrcpy((char far *)&tile_desc->b[1], "");
    _fstrcpy((char far *)&tile_desc->b[0x1a], "");
    _fstrcpy((char far *)&tile_desc->b[0x34], "");
    _fstrcpy((char far *)&tile_desc->b[0x4d], "");
    _fstrcpy((char far *)&tile_desc->b[0x67], "");
    _fstrcpy((char far *)&tile_desc->b[0x80], "");
    _fstrcpy((char far *)&tile_desc->b[0x9a], "");
}

/* per level: monster variants from the seed, merge duplicates, group bonuses */
void far monsters_fixup(int nlev, int seed)
{
    int xl;
    int a1ck;
    int slotn[12];
    int ndiffi;
    int ih;
    int mx3i;
    char flagh;
    struct group far *gk;
    int totl[0x47];
    int a28h;
    int ab8i;
    int kj;
    int lvc;
    unsigned c0c;
    struct mon far *mi;
    int kindsn[0x47];
    int tc;
    int numh[12];
    struct mtpl far *tpd;
    struct mon far *m2i;
    int a178h;
    int baseb;
    int jo;
    int rh;
    int a17a;
    int a180a;
    int a17em;
    int a184n;
    int mx2e;
    int mx1g;
    int a18ca;
    unsigned c2i;
    int a190d;
    int a18e;

    for (lvc = 0; lvc < nlev; lvc++) {
        for (ih = ndiffi = 0; ih < 12; ih++) {
            slotn[ih] = -1;
            numh[ih] = 0;
        }
        for (jo = 0; jo < 0x47; jo++)
            kindsn[jo] = totl[jo] = 0;
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            mi = &bf_monsters[lvc * 12 + ih];
            tpd = &mon_templates[mon_template[lvc * 12 + ih]];
            rh = seed % tpd->x0f + 1;
            baseb = tpd->x11 + (rh - 1) * tpd->x10;
            for (jo = 0; jo < tpd->x10; jo++)
                mi->x9[jo] = (char)jo + (char)baseb;
        }
        for (kj = 0; kj < group_count[lvc]; kj++) {
            gk = &bf_groups[lvc * 10 + kj];
            for (jo = 0; jo < 3; jo++)
                if ((ih = gk->type[jo]) >= 0 && ih < mon_count[lvc])
                    numh[ih] += gk->cnt[jo] < 0 ? 0 : gk->cnt[jo];
        }
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            tc = mon_template[lvc * 12 + ih];
            goto tt;
        td:
            tc--;
        tt:
            if (tc > 0 && mon_templates[tc].x0e < 0)
                goto td;
            for (xl = 0; xl < ih; xl++)
                if (slotn[xl] == tc)
                    break;
            if (xl >= ih) {
                ndiffi++;
                kindsn[tc]++;
            }
            slotn[ih] = tc;
            totl[tc] += numh[ih];
        }
        flagh = 0;
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            mi = &bf_monsters[lvc * 12 + ih];
            rh = mi->x0d;
            if (rh == 1 || rh == 2) {
                flagh = 1;
                if (ndiffi > 1 && totl[slotn[ih]] > 1) {
                    switch (slotn[ih]) {
                    default:
                        mi->x0d = 0;
                        mi->x6 = 0;
                        mi->x27 = mon_templates[mon_template[lvc * 12 + ih]].x12;
                        mi->x0 &= 0xff00;
                        mi->x0 |= (unsigned char)mi->x28;
                        break;
                    case 0x15: case 0x16: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d:
                    case 0x23: case 0x2d: case 0x33: case 0x35:
                        for (kj = 0; kj < group_count[lvc]; kj++) {
                            gk = &bf_groups[lvc * 10 + kj];
                            for (jo = 0; jo < 3; jo++)
                                if (gk->type[jo] == ih)
                                    gk->cnt[jo] = gk->cnt[jo] > 1 ? 1 : gk->cnt[jo];
                        }
                        break;
                    }
                }
            }
        }
        ih = 0;
        baseb = mon_count[lvc];
        goto it;
    ii:
        ih++;
    it:
        if (ih >= mon_count[lvc])
            goto idone;
        tc = slotn[ih];
        xl = 0;
        goto xt;
    ji:
        jo++;
    jt:
        if (jo >= mi->xd4)
            goto cp;
        if (mi->x74[jo].b2 != mi->x28)
            goto ji;
        mi->x74[jo].b2 = m2i->x28;
        mi->x74[jo].b3 = m2i->x29;
    cp:
        mi->x28 = m2i->x28;
        mi->x0 = m2i->x0;
        mi->x2 = m2i->x2;
        mi->x0d = m2i->x0d;
        mi->x6 = m2i->x6;
        baseb--;
    x9:
        for (jo = 0; jo < 3; jo++)
            mi->x9[jo] = m2i->x9[jo];
        goto ii;
    xi:
        xl++;
    xt:
        if (xl >= ih)
            goto ii;
        if (slotn[xl] != tc)
            goto xi;
        mi = &bf_monsters[lvc * 12 + ih];
        m2i = &bf_monsters[lvc * 12 + xl];
        if (baseb > 2 || flagh) {
            jo = 0;
            goto jt;
        }
        goto x9;
    idone:
        kj = mx1g = mx2e = mx3i = 0;
        for (; kj < group_count[lvc]; kj++) {
            gk = &bf_groups[lvc * 10 + kj];
            for (jo = 0; jo < 3; jo++) {
                if ((ih = gk->type[jo]) < 0 || ih >= mon_count[lvc])
                    continue;
                tpd = &mon_templates[mon_template[lvc * 12 + ih]];
                if (tpd->xba[0] > 0 || tpd->xba[1] > 0 || tpd->xbc[0] > 0 || tpd->xbc[1] > 0
                    || tpd->xbe[0] > 0 || tpd->xbe[1] > 0) {
                    for (rh = 0; rh < gk->cnt[jo]; rh++) {
                        baseb = roll_dice(1, tpd->xba[0]) + tpd->xba[1];
                        mx1g = baseb >= mx1g ? baseb : mx1g;
                        baseb = roll_dice(1, tpd->xbc[0]) + tpd->xbc[1];
                        mx2e = baseb >= mx2e ? baseb : mx2e;
                        baseb = roll_dice(1, tpd->xbe[0]) + tpd->xbe[1];
                        mx3i = baseb >= mx3i ? baseb : mx3i;
                    }
                }
            }
        }
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            mi = &bf_monsters[lvc * 12 + ih];
            mi->x36[1] = CL(mi->x36[1] + mx1g);
            mi->x36[0] = CL(mi->x36[0] + mx2e);
            mi->x44[0] = CL(mi->x44[0] + mx3i);
            mi->x44[1] = CL(mi->x44[1] + mx3i);
            mi->x44[2] = CL(mi->x44[2] + mx3i);
            mi->x44[3] = CL(mi->x44[3] + mx3i);
        }
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            tc = mon_template[lvc * 12 + ih];
            switch (tc) {
            case 0x15:
                a184n = 3;
                a1ck = 0x1e;
                a190d = 0x16;
                ab8i = 4;
                a178h = 1;
                a18ca = a18e = a17a = 2;
                a17em = a180a = a28h = 0;
                break;
            case 0x16:
                a184n = 4;
                a1ck = 0x28;
                a190d = 0x1e;
                ab8i = 5;
                a18ca = a18e = a17a = 3;
                a28h = a178h = 2;
                a180a = 1;
                a17em = 0xb;
                break;
            case 0x1b:
                a1ck = 0x14;
                a190d = 0xe;
                a18e = a17a = a184n = 1;
                ab8i = 3;
                a18ca = a178h = 0;
                a17em = a180a = a28h = 0;
                break;
            case 0x1c:
                a1ck = 0x14;
                a190d = 0xe;
                a18e = a17a = a184n = 2;
                ab8i = 3;
                a18ca = a178h = 0;
                a17em = a180a = a28h = 0;
                break;
            case 0x1d:
                a1ck = 0x14;
                a190d = 0xe;
                ab8i = a18e = a17a = a184n = 3;
                a18ca = a178h = 0;
                a17em = a180a = a28h = 0;
                break;
            case 0x2d:
            case 0x35:
                a190d = a17a = a1ck = a184n = 0;
                a28h = a178h = a18e = 2;
                a180a = a18ca = ab8i = 1;
                a17em = 9;
                break;
            default:
                continue;
            }
            for (xl = 0; xl < mon_count[lvc]; xl++) {
                mi = &bf_monsters[lvc * 12 + xl];
                if (mi->x0d == 1 && roll_dice(1, 5) <= a184n) {
                    mi->x44[4] += a1ck;
                    mi->x44[5] += a1ck;
                    mi->x44[6] += a1ck;
                }
                baseb = F_20a5_75ac[mi->x28];
                switch (baseb) {
                case 1: case 2: case 6:
                    if (roll_dice(1, 5) <= a17a) {
                        if (baseb == 6)
                            mi->x2b += a190d;
                        else
                            mi->x29 += a190d;
                    }
                    break;
                case 0: case 3:
                    if (roll_dice(1, 5) <= a178h)
                        mi->x2d |= a18ca & 0x0f;
                default:
                    if (roll_dice(1, 5) <= a18e)
                        mi->x2d |= ab8i << 4;
                    break;
                }
                if (roll_dice(1, 5) <= a28h) {
                    mi->x33 = a180a;
                    mi->x31[0] += a17em;
                    mi->x31[1] += a17em;
                }
            }
        }
    }
    for (lvc = 0; lvc < nlev; lvc++) {
        for (ih = 0; ih < mon_count[lvc]; ih++) {
            mi = &bf_monsters[lvc * 12 + ih];
            c0c = mi->x0;
            c2i = mi->x2;
            for (jo = 0; jo < nlev; jo++)
                for (xl = 0; xl < mon_count[jo]; xl++) {
                    m2i = &bf_monsters[jo * 12 + xl];
                    if (m2i->x0 == c0c && m2i->x2 == c2i)
                        mi->x4 |= 1 << jo;
                }
        }
    }
}

/* drop empty group entries and groups, fix the group leaders, roll the group values */
void far groups_fixup(int nlev)
{
    int ig, xrg, xsg, lva;
    struct mon far *mg;
    int kj;
    int cnta[10];
    struct group far *g;
    struct mtpl far *tpd;
    char far *ed;
    int ung;
    int ja, vf;

    for (lva = 0; lva < nlev; lva++) {
        for (kj = 0; kj < 10; kj++)
            cnta[kj] = 0;
        for (kj = 0; kj < group_count[lva]; kj++) {
            g = &bf_groups[lva * 10 + kj];
            for (ja = 0; ja < 3; ja++) {
                if (g->type[ja] >= 0 && g->type[ja] < mon_count[lva] && g->cnt[ja] > 0)
                    cnta[kj]++;
                else {
                    g->type[ja] = -1;
                    g->cnt[ja] = 0;
                }
            }
        }
        for (kj = 0; kj < group_count[lva]; ) {
            if (cnta[kj] > 0) {
                kj++;
                continue;
            }
            for (xsg = kj + 1; xsg < group_count[lva]; xsg++)
                bf_groups[lva * 10 + xsg - 1] = bf_groups[lva * 10 + xsg];
            group_count[lva]--;
        }
        for (kj = 0; kj < group_count[lva]; kj++) {
            g = &bf_groups[lva * 10 + kj];
            if (!g->x7)
                continue;
            ed = &g->x14;
            if ((ig = *ed) < 0 || ig >= mon_count[lva])
                goto clr;
            mg = &bf_monsters[lva * 12 + ig];
            if (mg->x0d == 2)
                goto clr;
            if (mg->x0d == 1) {
                mg->x0d = 0;
                mg->x6 = 0;
                mg->x27 = mon_templates[mon_template[lva * 12 + *ed]].x12;
                mg->x0 &= 0xff00;
                mg->x0 |= (unsigned char)mg->x28;
            }
            continue;
        clr:
            g->x7 = 0;
        }
        for (kj = 0; kj < group_count[lva]; kj++) {
            g = &bf_groups[lva * 10 + kj];
            g->x0e = 0;
            for (ja = g->x12 = g->x10 = 0; ja < 3; ja++) {
                if (g->type[ja] < 0)
                    continue;
                tpd = &mon_templates[mon_template[lva * 12 + g->type[ja]]];
                for (xrg = 0; xrg < g->cnt[ja]; xrg++) {
                    if ((vf = roll_dice(1, tpd->xc0) + tpd->xc6) > 0)
                        g->x0e += vf;
                    if ((vf = roll_dice(1, tpd->xc2) + tpd->xc8) > 0)
                        g->x10 += vf;
                    if ((vf = roll_dice(1, tpd->xc4) + tpd->xca) > 0)
                        g->x12 += vf;
                }
            }
        }
    }
}
#pragma optimize("a", on)
void far sub_s004_0e76_20dc(void)
{
    combat_flags |= 1;
    if (D_db27 != cur_loc) {
        D_da34 = 0;
        D_db27 = cur_loc;
    }
}

/* n dice of d sides */
int far roll_dice(int n, int d)
{
    int s, i;

    if (d >= 1 && n >= 1) {
        if (d == 1)
            return n;
        s = 0;
        for (i = 0; i < n; i++)
            s += rand() % d + 1;
        return s;
    }
    return 0;
}

int far sub_s004_0e76_213c(int x0, int y0, int x1, int y1)
{
    int a, b;

    a = abs(x1 - x0);
    b = abs(y1 - y0);
    return b < a ? a : b;
}

/* direction 0/2/4/6 from (x0, y0) to (x1, y1) */
int far dir_between(int x0, int y0, int x1, int y1)
{
    int dx, dy;

    dx = x1 - x0;
    dy = y1 - y0;
    if (dy < dx && -dy > dx)
        return 0;
    if (dx > dy && -dy < dx)
        return 2;
    if (dx < dy && -dy < dx)
        return 4;
    if (dx < dy && dx < -dy)
        return 6;
    if (dy < 0)
        return 0;
    return 4;
}
