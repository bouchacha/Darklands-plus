/* place: s025 1ef8:0000 */
/* orig: size=0ddc CONST=7e0a+000a _DATA=2600+0073 */
/* data: CONST=7e0a */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* QA2 fix (2026-10-09): F_20a5_763b/F_20a5_763c were two far names -> 6 CONST slots instead of the original 5, so data-search stamped CONST at 7d1c (s001's and s002's slots!) and the --mod build overwrote their segment words: battle started garbled (s002) and Quickstart gave 'Hand Axe 00q' (s001). Now one far array F_20a5_763b[2]; a data: CONST=7e0a hint line; re-stamped CONST=7e0a+000a (tools/playtests/constcheck.py ok). */
/* status: wip 6/7. 0000 decoded fully but blocked: the original keeps SI live across inline
 * block copies with push si/push di (6.00); 6.00A splits the register variable instead. Combat overlay s025 (child of s014): enemy placement and animation buffers.
 * 0000 places the enemies of the level (level%d.enm) into the combatant records 5..12;
 * 09a6 (kind, who) makes sure the animation buffers of an enemy kind are loaded (frees and
 * reloads all when a bigger one is needed); 0a84 (kind) allocates the "W"/"C"/"D" buffers with
 * the sizes from tacanim.db (14-byte records: name, W, C, D sizes, 4 frame counts);
 * 0be4/0c02 free the buffers of kinds 5 and 6; 0cfc enemy side setup; 0d3c reads the
 * four frame counts of a combatant's kind. */
#include "darklands/card.h"
#include "darklands/game.h"

typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;
struct anim { char name[4]; unsigned w, c, d; char n[4]; };
struct mem { char status; char pad1; char kind; char pad3; char f04; char pad5[0x7b]; };
struct s6 { char b[6]; };
struct s6i { int a; char p[4]; };
struct grp { char type; char n; int x; int y; char count; char pad7; char facing; char pad9; };
struct enc { int x; int y; int n; char pad6; char flag; char types[3]; char counts[3];
    int fl; int gr; int pf; struct grp grp; };
struct pal { unsigned char off; char rgb[0x30]; unsigned char a, b, c, d; };
struct money { int fl; int gr; int pf; };
struct bits { unsigned lo : 9; unsigned mid : 4; unsigned hi : 2; unsigned top : 1; };
struct ab { char a; char b; };
struct cbt {
    char pad0[4];
    int x;   /* +04 */
    int y;   /* +06 */
    char pad8[6];
    char * info;   /* +0e */
    int w10;   /* +10 */
    char pad12[2];
    char status;   /* +14 */
    char pad15[1];
    char kind;   /* +16 */
    char pad17[1];
    char f18;   /* +18 */
    char f19;   /* +19 */
    char f1a;   /* +1a */
    char f1b;   /* +1b */
    char idx;   /* +1c */
    char pad1d[5];
    char f22;   /* +22 */
    char pad23[39];
    char f4a;   /* +4a */
    char pad4b[6];
    char f51;   /* +51 */
    char pad52[44];
    char nitems;   /* +7e */
    char pad7f[1];
};
struct etype {
    struct cbt head;
    char name[4];   /* +80 */
    char name2[0x17];   /* +84 */
    char f9b;   /* +9b */
    char pad9c[2];
    int f9e;   /* +9e */
    int fa0;   /* +a0 */
    int fa2;   /* +a2 */
    int fa4;   /* +a4 */
    char fa6;   /* +a6 */
    unsigned char fa7[3];   /* +a7 */
    char faa;   /* +aa */
    char fab;   /* +ab */
    int fac;   /* +ac */
    char fae[10];   /* +ae */
    char fb8[10];   /* +b8 */
    struct s6 fc2[16];   /* +c2 */
};

#define MK_FP(s, o) ((void far *)(((unsigned long)(s) << 16) | (unsigned)(o)))

extern struct cbt combatants[];
extern struct etype D_b899;
extern int D_e0ea, D_d6f7;
extern char D_17b2[];
extern int ems_frame_handle;
extern char D_9069;
extern unsigned D_ca53, D_ca55;
extern char msg_buf[];
extern int D_c28d[], D_c23e[], D_c927[], D_d645[], D_d664[];
extern char enemy_class[], D_c1f3[], D_c1c1[], D_d1d7[], D_d680[];
extern char D_d37c[][10], D_d411[][10], cbt_anim_seq[][10];
extern struct s6 far F_2e38_0bde[][16];
extern char far anim_set_names[][0x14];
extern struct grp far F_2e38_0bf6;
extern struct money far F_2e38_0bb0;
extern struct s6i far F_20a5_35ee[];
extern char far F_20a5_763b[];   /* 2 chars (763b, 763c): one far name -> one CONST slot */
extern int los_x0, los_y0, los_x1, los_y1;
extern struct bits cbt_flags[];
extern char cbt_action[], D_d4e5[], cbt_was_hit[], cbt_gave_up[], cbt_damage_shown[], cbt_moving[], cbt_special[];
extern struct ab cbt_queued_damage[];
extern char cbt_anim[], D_d5c6[];
extern char D_1182[], D_118e[], D_119a[], D_11a6[];
extern char cbt_half_w[], cbt_half_h[];
extern char D_d6eb;
extern char demo_mode;
extern char D_c908[], cbt_anim_len[];
extern char D_cab4[][0x90];
extern char D_9631, floor_dirty;
extern int D_da34;
extern char *anim_file[];
extern char D_261c[], D_2628[];

extern struct mem party[];
extern unsigned anim_buffers[];
extern int D_ca75[], D_ca8b[];
extern int D_ca4d;
extern char *D_1166[];
extern char *D_11b2[], *D_11b6[], *D_11ba[];
extern int D_c200[];
extern int enemy_anim_slot, D_d67e;
extern char D_c3cc;
extern char D_d6f0;
extern char floor_shown;
extern char cbt_floor[];
extern int D_c3cd;
extern int combat_clock, D_da4f;

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
FILE *fopen(const char *, const char *);
unsigned fread(void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
int stricmp(const char *, const char *);
int far alloc_buffer(unsigned, int, char *);
void far free_buffer(int, char *);
void far cbt_set_anim(int, int);
int far place_around_vec(int, int *);
int abs(int);
int fseek(FILE *, long, int);
void far *_fmemcpy(void far *, const void far *, unsigned);
int far random_n(int);
void far ems_map_page(int, int, int);
void far file_load_far(char *, char far *);
void far palette_save(unsigned, unsigned);
void far palette_restore(unsigned, unsigned);
int far cbt_collisions(struct cbt *);
int far line_blocked(int, int, int);
void far weapon_ready(int, int, int);
void far sub_root_047c_0362(int, int, int, int);
void far sub_root_047c_03da(int);
int far enemies_place(int, struct enc far *, int);
void far enemy_anim_load(int, int);
void far enemy_anim_alloc(int);
void far enemy_anim_free5(void);
void far enemy_anim_free6(int);
void far sub_s025_1ef8_0cfc(void);
void far sub_s025_1ef8_0d3c(int, char *, char *, char *, char *);

int far enemies_place(int mode, struct enc far *enc, int who)
{
    int x0, y0, n, ngroups, facing, dir, quad, dx, dy, adx, ady;
    char far *types;
    char far *counts;
    struct pal far *pals;
    struct etype *rec;
    FILE *fp;
    int g, j, k, slot, si, tries, ret, placed, w, p, pi;
    int w2, w1, basex, basey, rx, ry;
    char a, b;
    unsigned char palidx[3], pb[3], pc[3], pd[3];

    ret = 0;
    placed = 0;
    rec = &D_b899;
    pals = (struct pal far *)MK_FP(D_d6f7, D_e0ea + 0x31b);
    switch (mode) {
    case 0:
        x0 = enc->x;
        y0 = enc->y;
        n = enc->n;
        types = enc->types;
        counts = enc->counts;
        ngroups = 3;
        dx = (combatants[who].x - x0) / 2;
        dy = combatants[who].y - y0;
        adx = abs(dx);
        ady = abs(dy);
        if (adx != 0)
            facing = ady * 4 / adx;
        else
            facing = 20;
        if (facing == 0)
            dir = 0;
        else if (facing >= 2 && facing < 8)
            dir = 1;
        else
            dir = 2;
        if (dx >= 0 && dy < 0)
            quad = 0;
        else if (dx >= 0 && dy >= 0)
            quad = 1;
        else if (dx < 0 && dy >= 0)
            quad = 2;
        else
            quad = 3;
        facing = D_17b2[quad * 3 + dir];
        break;
    case 1:
        x0 = F_2e38_0bf6.x;
        y0 = F_2e38_0bf6.y;
        n = F_2e38_0bf6.n;
        ngroups = 1;
        types = &F_2e38_0bf6.type;
        counts = &F_2e38_0bf6.count;
        facing = F_2e38_0bf6.facing;
        break;
    }
    ems_map_page(0, 8, ems_frame_handle);
    file_load_far("enemypal.dat", (char far *)pals);
    if (D_9069 == 0) {
        D_ca55 = D_d6f7;
        D_ca53 = 0;
        palette_save(D_ca53, D_ca55);
    }
    sprintf(msg_buf, "level%d.enm", floor_shown);
    fp = fopen(msg_buf, "rb");
    for (g = 0; g < ngroups; g++) {
        if (types[g] == -1)
            continue;
        fseek(fp, (long)types[g] * 0x122 + 1, 0);
        fread(&D_b899, 0x122, 1, fp);
        for (j = 0; j < counts[g]; j++) {
            si = -1;
            for (k = 5; k <= 12; k++)
                if (combatants[k].status == 0) {
                    si = k;
                    break;
                }
            if (si != -1) {
                if (rec->fa0 == enemy_anim_slot)
                    slot = 5;
                else if (rec->fa0 == D_d67e)
                    slot = 6;
                else if (enemy_anim_slot == -1) {
                    slot = 5;
                    enemy_anim_slot = rec->fa0;
                } else if (D_d67e == -1) {
                    slot = 6;
                    D_d67e = rec->fa0;
                } else
                    si = -1;
            }
            if (si == -1)
                continue;
            cbt_floor[si] = floor_shown;
            D_c200[si] = rec->fa2;
            combatants[si] = rec->head;
            combatants[si].f19 = combatants[si].f1a = facing;
            combatants[si].kind = slot;
            combatants[si].idx = si;
            combatants[si].f4a = -1;
            D_c28d[slot] = types[g];
            if (mode == 0)
                enemy_class[si] = rec->fab;
            else
                enemy_class[si] = 0;
            D_c23e[slot] = rec->fa4;
            D_c1f3[si] = rec->faa;
            D_c1c1[si] = rec->fa6;
            strcpy(D_1166[slot], rec->name);
            _fstrcpy(anim_set_names[slot], rec->name2);
            for (k = 0; k < combatants[si].nitems; k++)
                F_2e38_0bde[si][k] = rec->fc2[k];
            D_c927[si] = rec->fac;
            D_d1d7[si] = 0;
            for (k = 0; k < 10; k++) {
                if (k != 2)
                    D_d1d7[si] += rec->fae[k];
                D_d37c[si][k] = rec->fae[k];
                D_d411[si][k] = rec->fb8[k];
            }
            if (rec->f9e == 0)
                w = combatants[si].f51;
            else {
                w = combatants[si].f22;
                D_d680[si] = -1;
                if (combatants[si].kind == 5) {
                    if (rec->fa2 == D_d67e)
                        D_d680[si] = 6;
                } else if (combatants[si].kind == 6 && rec->fa2 == enemy_anim_slot)
                    D_d680[si] = 5;
            }
            if (rec->fab != 2)
                weapon_ready(si, w, rec->f9e);
            else {
                anim_file[combatants[si].kind][0] = 'P';
                anim_file[combatants[si].kind][1] = 'T';
            }
            combatants[si].status = 1;
            los_x0 = x0;
            los_y0 = y0;
            w2 = n * 2;
            w1 = n;
            p = rec->f9b + facing / 2 * 3;
            a = D_261c[p];
            b = D_2628[p];
            basex = los_x0 - (a < 0 ? w2 : 0);
            basey = los_y0 - (b < 0 ? w1 : 0);
            rx = w2 << (a & 1);
            ry = w1 << (b & 1);
            cbt_flags[si].lo = 0;
            cbt_special[si] = cbt_moving[si] = cbt_queued_damage[si].b = cbt_queued_damage[si].a = cbt_damage_shown[si] = cbt_gave_up[si] =
                cbt_was_hit[si] = D_d4e5[si] = cbt_action[si] = 0;
            combatants[si].w10 = 0;
            F_20a5_35ee[si].a = 0;
            D_d5c6[si] = cbt_anim[si] = 7;
            sub_s025_1ef8_0d3c(si, &D_1182[slot], &D_118e[slot], &D_119a[slot], &D_11a6[slot]);
            cbt_half_w[si] = D_119a[slot];
            cbt_half_h[si] = D_11a6[slot];
            cbt_flags[si].hi = 3;
            for (tries = 0; tries <= 500; tries++) {
                if (tries == 0) {
                    combatants[si].x = basex;
                    combatants[si].y = basey;
                } else {
                    combatants[si].x = random_n(rx) + basex;
                    combatants[si].y = random_n(ry) + basey;
                }
                if (cbt_collisions(&combatants[si]) == 0) {
                    los_x1 = combatants[si].x;
                    los_y1 = combatants[si].y;
                    if (line_blocked(-1, 0, 0) == 0)
                        break;
                }
            }
            if (tries > 500)
                combatants[si].status = 0;
            else {
                placed = 1;
                if (D_9069 == 0)
                    for (p = 0; p < 3; p++) {
                        pi = rec->fa7[p];
                        if (pi == 0xff) {
                            palidx[p] = -1;
                            continue;
                        }
                        _fmemcpy(*(char far **)&D_ca53 + pals[pi].off, pals[pi].rgb, 0x30);
                        palidx[p] = pals[pi].a;
                        pb[p] = pals[pi].b;
                        pc[p] = pals[pi].c;
                        pd[p] = pals[pi].d;
                    }
                if (mode == 0 && enc->flag == 1) {
                    D_d6eb = 1;
                    F_2e38_0bf6 = enc->grp;
                }
                combatants[si].f18 = 5;
                if (anim_buffers[slot] == 0) {
                    ems_map_page(0, 4, ems_frame_handle);
                    ems_map_page(0, 5, ems_frame_handle);
                    ems_map_page(0, 6, ems_frame_handle);
                    ems_map_page(0, 7, ems_frame_handle);
                    enemy_anim_load(slot, si);
                    ems_map_page(0, 8, ems_frame_handle);
                    ems_map_page(1, 1, ems_frame_handle);
                    ems_map_page(2, 2, ems_frame_handle);
                    ems_map_page(3, 3, ems_frame_handle);
                }
                if (demo_mode && si == 5) {
                    if (demo_mode == 'I') {
                        F_20a5_763b[0] = 'D';
                        F_20a5_763b[1] = '7';
                    } else {
                        F_20a5_763b[0] = 'C';
                        F_20a5_763b[1] = 'D';
                    }
                }
                cbt_anim_len[si] = D_c908[slot];
                for (p = 0; p <= 9; p++)
                    cbt_anim_seq[si][p] = p;
                combatants[si].info = D_cab4[combatants[si].kind];
                combatants[si].f1b = 0;
            }
            D_d645[si] = combatants[si].x;
            D_d664[si] = combatants[si].y;
        }
    }
    fclose(fp);
    if (placed == 1) {
        ret = 1;
        D_9631 = 1;
        floor_dirty = 1;
        D_da34 |= 1 << floor_shown;
        F_2e38_0bb0.pf += enc->pf;
        if (F_2e38_0bb0.pf >= 12) {
            F_2e38_0bb0.gr += F_2e38_0bb0.pf / 12;
            F_2e38_0bb0.pf %= 12;
        }
        F_2e38_0bb0.gr += enc->gr;
        if (F_2e38_0bb0.gr >= 20) {
            F_2e38_0bb0.fl += F_2e38_0bb0.gr / 20;
            F_2e38_0bb0.gr %= 20;
        }
        F_2e38_0bb0.fl += enc->fl;
        if (D_9069 == 0) {
            palette_restore(D_ca53, D_ca55);
            for (g = 0; g < 3; g++)
                if (palidx[g] != 0xff) {
                    sub_root_047c_0362(pd[g], pc[g], palidx[g], pb[g]);
                    sub_root_047c_03da(pd[g]);
                }
        }
    }
    ems_map_page(0, 0, ems_frame_handle);
    return ret;
}

char D_261c[12] = { -15, -15, -15, -15, 0, -16, -15, -15, -15, -15, -16, 0 };
char D_2628[12] = { -15, -16, 0, -15, -15, -15, -15, 0, -16, -15, -15, -15 };

#pragma optimize("a", on)
void far enemy_anim_load(int kind, int who)
{
    int need, k, t;

    need = 0;
    for (k = kind + 1; k <= 6; k++)
        if (anim_buffers[k] != 0 && anim_buffers[kind] < anim_buffers[k])
            need = 1;
    if (need == 0) {
        enemy_anim_alloc(kind);
        cbt_set_anim(who, 5);
        return;
    }
    enemy_anim_free5();
    for (k = 5; k < 13; k++) {
        if (party[k].status) {
            t = party[k].kind;
            if (anim_buffers[t] == 0) {
                enemy_anim_alloc(t);
                if (t == 5)
                    enemy_anim_slot = D_c200[k];
                else if (t == 6)
                    D_d67e = D_c200[k];
            }
            D_c3cc = 1;
            cbt_set_anim(k, party[k].f04);
            D_c3cc = 0;
        }
    }
}

#pragma optimize("", on)
void far enemy_anim_alloc(int kind)
{
    char name[6];
    struct anim rec;
    FILE *fp;

    strcpy(name, D_1166[kind]);
    fp = fopen("tacanim.db", "rb");
    while (!(fp->_flag & 0x10)) {
        fread(&rec, 14, 1, fp);
        if (stricmp(rec.name, name) == 0)
            break;
    }
    fclose(fp);
    if (kind >= 5) {
        sprintf(name, "W %s", D_1166[kind]);
        anim_buffers[kind] = alloc_buffer(rec.w, 0, name);
        sprintf(name, "C %s", D_1166[kind]);
        D_ca75[kind] = alloc_buffer(rec.c, 0, name);
        sprintf(name, "D %s", D_1166[kind]);
        D_ca8b[kind] = alloc_buffer(rec.d, 0, name);
    } else if (rec.w < rec.c) {
        sprintf(name, "C %s", D_1166[kind]);
        D_ca4d = alloc_buffer(rec.c, 0, name);
    } else {
        sprintf(name, "W %s", D_1166[kind]);
        D_ca4d = alloc_buffer(rec.w, 0, name);
    }
}

void far enemy_anim_free5(void)
{
    int k;

    for (k = 5; k <= 6; k++)
        enemy_anim_free6(k);
    enemy_anim_slot = -1;
    D_d67e = -1;
}

void far enemy_anim_free6(int kind)
{
    char tag[8];

    strcpy(tag, "EB");
    if (anim_buffers[kind]) {
        free_buffer(anim_buffers[kind], tag);
        strcpy(D_11b2[kind], "");
        anim_buffers[kind] = 0;
        if (D_ca75[kind]) {
            strcat(tag, "t");
            free_buffer(D_ca75[kind], tag);
            D_ca75[kind] = 0;
            strcpy(D_11b6[kind], "");
        }
        if (D_ca8b[kind]) {
            strcat(tag, "d");
            free_buffer(D_ca8b[kind], tag);
            D_ca8b[kind] = 0;
            strcpy(D_11ba[kind], "");
        }
    }
}

void far sub_s025_1ef8_0cfc(void)
{
    int i;

    D_d6f0 = 1;
    for (i = 5; i < 13; i++)
        if (party[i].status) {
            cbt_floor[i] = floor_shown;
            if (!place_around_vec(i, &D_c3cd))
                party[i].status = 0;
        }
    D_da4f = combat_clock;
}

void far sub_s025_1ef8_0d3c(int who, char *a, char *b, char *c, char *d)
{
    char name[4];
    struct anim rec;
    FILE *fp;

    strcpy(name, D_1166[party[who].kind]);
    fp = fopen("tacanim.db", "rb");
    while (!(fp->_flag & 0x10)) {
        fread(&rec, 14, 1, fp);
        if (stricmp(rec.name, name) == 0)
            break;
    }
    fclose(fp);
    *a = rec.n[0];
    *b = rec.n[1];
    *c = rec.n[2];
    *d = rec.n[3];
}
