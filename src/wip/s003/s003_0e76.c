/* place: s003 0e76:0000 */
/* orig: size=1a09 _DATA=12c2+00c0 */
/* end: 10169 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Battle state save/restore (saves\<name>.bsv).
   status (e9, 2026-10-09): ALL 8 FUNCTIONS WRITTEN, 5 match (0040, 006a, 00a0, 19ca, 19e8 -
   fmatch reports 19e8 as DIFF only because the analysed file end (101e6) includes the
   PKWARE copyright/data block 10169..101e5 of s003_1016; for the same reason stamp.py --wip
   refuses the file ("stub" 19e8), and a stamp would cut that block out: NOT STAMPED, so
   --functional does not use it until srcfiles ends this file at 10169 (tool fix).
   - 0000: 6.00 keeps D_d919 in CX and a copy of n in DI for `D_d90f += n`; w4: eglc off +
     `register` n and a copy m gives the DI copy (13 -> 10 diff lines), the CX load is left.
   - 00dc: serialisation matches with `p += prev` written inside the next access
     (`*(p += 1) = D_x;`, `_fmemcpy((p += 0xcb2), ...)`) and pragma "c" off; the level
     loop differs (6.00 enregisters i / the entry pointer with DS = buffer segment, 6.00A
     does not in this big function), and local slots are not tuned.
   - 0ce8: read side generated the same way; slots/register details differ.
   Note: restore reads the word that save takes from D_d6ce into D_dad7 (original asymmetry).
   The file is a 0x2a-byte header D_d91b (word 0: packed size of the variable block,
   D_d91d[i]: packed size of level i, D_d93d/D_d93f and D_d941/D_d943: the two optional
   files D_1162/D_1164 (raw size, packed size)), followed by PKWARE DCL "implode" blocks
   (s003_1016, kept in assembly; implode = 1016:04c6, explode = 1016:0d07, both pascal):
   the battle variables serialised into the D_c103 buffer, then per level the .flr, .atv
   and .enm data, then the two files.  0000/006a are the memory read/write callbacks for
   (ex|im)plode, 0040/00a0 the file read/write callbacks (pascal, retf 8). */

/* pascal callbacks: the names come out upper case, which is fine (only referenced here) */
#include "darklands/card.h"
#include "darklands/game.h"
#define IMPLODE(r, w, b, t, s) ((unsigned (far pascal *)(void far *, void far *, char far *, unsigned far *, unsigned far *))sub_s003_1016_04c6)((void far *)(r), (void far *)(w), b, t, s)
#define EXPLODE(r, w, b) ((unsigned (far pascal *)(void far *, void far *, char far *))sub_s003_1016_0d07)((void far *)(r), (void far *)(w), b)

struct font { char x0[0xc]; int xc; int xe; };
struct ent { void (far *fn)(void); char x4[10]; };
struct lvl { char x0[0x2184]; char count; char x2185[7]; struct ent ent[1]; };
typedef struct _iobuf FILE;

extern struct font *card_window;
extern char far *load_buf;
extern int ems_frame_handle;
extern char floor_dirty;
extern char D_a27a[];
extern char msg_file[];
extern char D_a896[];
extern char activation_count;
extern char msg_buf[];
extern unsigned char combat_flags;
extern char *D_1162, *D_1164;
extern int D_d90b, D_d90d;
extern char far *D_d90f;
extern char far *D_d913;
extern unsigned D_d917, D_d919;
extern unsigned D_d91b;
extern unsigned D_d91d[16];
extern int D_d93d, D_d93f, D_d941, D_d943;
extern char far floor_img[];
extern char far activations[];
extern char D_07d8, floor_shown, D_907a, real_combat, D_c4fd, D_c600, party_floor, D_c651, loot_pile_count, D_c83e, D_c989, D_ca1b, D_ca29, D_ca99, D_d610, D_d661, D_d6f0, combat_alarm, D_d728, D_d73b, D_d7fe, D_d842, D_d947;
extern int D_dad7, combat_clock, D_07e6, D_160c, D_160e, cursor_current, battle_result, D_c1f0, D_c2bb, D_c925, D_c978, battle_args, D_ca3f, D_d662, D_d6ce, D_d945, D_d9e4, D_da34, D_da43, D_da45, D_da47, D_da49, D_da4b, D_da4d, D_da4f, D_da51, D_da53;
extern char cbt_flags[], cbt_floor[], cbt_special[], cbt_moving[], D_c1c1[], D_c3cd[], marching_order[], D_c64c[], D_c678[], D_c6ef[], D_c77e[], D_c784[], D_c7a4[], D_c7cd[], D_c7fe[], D_c8b2[], cbt_anim[], cbt_action[], D_d3fe[], cbt_damage_shown[], D_d4e5[], cbt_was_hit[], cbt_gave_up[], D_d5f6[], D_d62b[], D_d68d[], D_d6fb[], D_d948[], cbt_giveup_timeout[], D_d97c[], cbt_engage_time[], D_d9b0[], D_d9ca[], order_queues[], D_da36[], D_da55[], D_da6f[], D_da89[], D_daa3[], D_dabd[];
extern char far loot_piles[];
extern char far cbt_paths[];
extern char far doors_opened[];
extern char far F_2e38_0c64[];

void far secret_door_search_vec(void), vec_17f7(void), trap_spell_vec(void), trap_missile_vec(void);
void far vec_181f(void), vec_1815(void), vec_1833(void), vec_1829(void);
int far swap_save2_vec(char *, char far *, long);
int far swap_load2_vec(char *, char far *);
void far level_switch_vec(int);
int far sub_s003_1016_04c6();
int far sub_s003_1016_0d07();
void far *far _fmemcpy(void far *, const void far *, unsigned);
unsigned far _dos_read(int, void far *, unsigned, unsigned *);
unsigned far _dos_write(int, const void far *, unsigned, unsigned *);
unsigned far _dos_creat(const char *, unsigned, int *);
unsigned far _dos_close(int);
unsigned far sub_root_06a1_2c9e(const char *, unsigned, int *);
int far sprintf(char *, const char *, ...);
int far printf(const char *, ...);
void far exit_(int);
FILE *far fopen(const char *, const char *);
int far fclose(FILE *);
int far remove(const char *);
char *strcpy(char *, const char *);
unsigned strlen(const char *);
void far game_shutdown(void);
void far mouse_show(void);
void far mouse_hide(void);
void far ems_map_page(int, int, int);
int far file_load_far(char *, void far *);
int far file_save(char *, void far *, int);
void far sub_s003_0e76_19ca(char *);
void far sub_s003_0e76_19e8(char *, int);

/* explode: read from the D_d90f memory block (D_d919 bytes left) */
#pragma optimize("eglc", off)
unsigned far pascal sub_s003_0e76_0000(char far *buf, unsigned far *size)
{
    register unsigned n;
    register unsigned m;

    n = *size;
    if (n > D_d919)
        n = D_d919;
    D_d919 -= n;
    m = n;
    _fmemcpy(buf, D_d90f, n);
    D_d90f += m;
    return n;
}
#pragma optimize("", on)

/* implode/explode: read from the file D_d90b */
unsigned far pascal sub_s003_0e76_0040(char far *buf, unsigned far *size)
{
    unsigned nr;

    nr = 0;
    _dos_read(D_d90b, buf, *size, &nr);
    return nr;
}

/* explode: write to the D_d913 memory block */
#pragma optimize("a", on)
void far pascal sub_s003_0e76_006a(char far *buf, unsigned far *size)
{
    _fmemcpy(D_d913, buf, *size);
    D_d913 += *size;
    D_d917 += *size;
}
#pragma optimize("", on)

/* implode: write to the file D_d90d */
unsigned far pascal sub_s003_0e76_00a0(char far *buf, unsigned far *size)
{
    unsigned nw;

    nw = 0;
    _dos_write(D_d90d, buf, *size, &nw);
    D_d917 += *size;
    return nw;
}

/* save the battle to saves\<name>.bsv (name: "<name>.xxx") */
#pragma optimize("a", on)
#pragma optimize("c", off)
void far battle_save(char *name)
{
    struct lvl far *lv;
    struct ent far *e;
    char far *p;
    char far *work;
    char far *map;
    unsigned *hp;
    unsigned dsize, type, nw;
    int i, k;
    FILE *f;
    char buf[30];
    char path[30];
    char fn[30];

    ems_map_page(0, 4, ems_frame_handle);
    ems_map_page(1, 5, ems_frame_handle);
    ems_map_page(2, 6, ems_frame_handle);
    ems_map_page(3, 7, ems_frame_handle);
    work = load_buf + 0x4e20;
    if (floor_dirty == 1) {
        sprintf(D_a27a, "level%d.flr", floor_shown);
        swap_save2_vec(D_a27a, floor_img, 0x33fcL);
        sprintf(D_a27a, "level%d.atv", floor_shown);
        _fmemcpy(D_a896, activations, (unsigned char)(msg_file[0] = activation_count) * 30);
        swap_save2_vec(D_a27a, msg_file, (long)((unsigned char)activation_count * 30 + 1));
        floor_dirty = 0;
    }
    card_window->xc = 1;
    card_window->xe = 0xff;
    mouse_hide();
    strcpy(buf, name);
    i = strlen(buf);
    buf[i - 4] = 0;
    sprintf(path, "saves\\%s.bsv", buf);
    _dos_creat(path, 0, &D_d90d);
    *(p = load_buf) = D_907a;
    _fmemcpy((p += 1), cbt_paths, 0xcb2);
    _fmemcpy((p += 0xcb2), D_d6fb, 0xd);
    _fmemcpy((p += 0xd), D_c3cd, 0xe);
    *(p += 0xe) = D_c600;
    _fmemcpy((p += 1), D_c7a4, 0x28);
    _fmemcpy((p += 0x28), loot_piles, 0x870);
    *(p += 0x870) = loot_pile_count;
    *(p += 1) = D_c83e;
    *(p += 1) = D_d6f0;
    *(p += 1) = D_d728;
    *(p += 1) = D_d73b;
    *(p += 1) = D_d7fe;
    *(p += 1) = D_d947;
    *(int far *)(p += 1) = D_d945;
    *(p += 0x2) = D_d842;
    *(int far *)(p += 1) = D_c1f0;
    _fmemcpy((p += 0x2), marching_order, 0x6);
    _fmemcpy((p += 0x6), D_c64c, 0x5);
    _fmemcpy((p += 0x5), D_c678, 0x5);
    _fmemcpy((p += 0x5), D_c6ef, 0x5);
    _fmemcpy((p += 0x5), D_c77e, 0x5);
    _fmemcpy((p += 0x5), D_c784, 0x5);
    _fmemcpy((p += 0x5), D_c7cd, 0x5);
    _fmemcpy((p += 0x5), D_c7fe, 0x5);
    _fmemcpy((p += 0x5), D_c8b2, 0x5);
    *(p += 0x5) = D_d610;
    *(p += 1) = combat_alarm;
    *(int far *)(p += 1) = cursor_current;
    _fmemcpy((p += 0x2), cbt_moving, 0xd);
    _fmemcpy((p += 0xd), cbt_anim, 0xd);
    _fmemcpy((p += 0xd), cbt_action, 0xd);
    _fmemcpy((p += 0xd), D_d3fe, 0xd);
    _fmemcpy((p += 0xd), cbt_damage_shown, 0xd);
    _fmemcpy((p += 0xd), D_d4e5, 0xd);
    _fmemcpy((p += 0xd), cbt_was_hit, 0xd);
    _fmemcpy((p += 0xd), cbt_floor, 0xd);
    _fmemcpy((p += 0xd), cbt_gave_up, 0xd);
    _fmemcpy((p += 0xd), D_da36, 0xd);
    _fmemcpy((p += 0xd), D_d5f6, 0x1a);
    _fmemcpy((p += 0x1a), D_d62b, 0x1a);
    _fmemcpy((p += 0x1a), cbt_flags, 0x1a);
    _fmemcpy((p += 0x1a), cbt_special, 0xd);
    _fmemcpy((p += 0xd), D_c1c1, 0xd);
    _fmemcpy((p += 0xd), doors_opened, 0x40);
    *(p += 0x40) = real_combat;
    *(p += 1) = D_c4fd;
    *(p += 1) = floor_shown;
    *(p += 1) = party_floor;
    *(p += 1) = D_c651;
    *(int far *)(p += 1) = D_c925;
    *(int far *)(p += 0x2) = D_c978;
    *(p += 0x2) = D_c989;
    *(p += 1) = D_ca1b;
    *(p += 1) = D_ca29;
    *(int far *)(p += 1) = D_07e6;
    *(int far *)(p += 0x2) = D_ca3f;
    *(p += 0x2) = D_ca99;
    *(int far *)(p += 1) = battle_result;
    *(int far *)(p += 0x2) = battle_args;
    *(p += 0x2) = D_d661;
    *(int far *)(p += 1) = D_d662;
    _fmemcpy((p += 0x2), D_d68d, 0xa);
    *(int far *)(p += 0xa) = D_d6ce;
    *(int far *)(p += 0x2) = D_da34;
    _fmemcpy((p += 0x2), order_queues, 0x4e);
    *(int far *)(p += 0x4e) = D_d9e4;
    _fmemcpy((p += 0x2), D_d9ca, 0x1a);
    _fmemcpy((p += 0x1a), D_d9b0, 0x1a);
    _fmemcpy((p += 0x1a), cbt_engage_time, 0x1a);
    _fmemcpy((p += 0x1a), D_d97c, 0x1a);
    _fmemcpy((p += 0x1a), cbt_giveup_timeout, 0x1a);
    _fmemcpy((p += 0x1a), D_d948, 0x1a);
    _fmemcpy((p += 0x1a), D_dabd, 0x1a);
    _fmemcpy((p += 0x1a), D_daa3, 0x1a);
    _fmemcpy((p += 0x1a), D_da89, 0x1a);
    _fmemcpy((p += 0x1a), D_da6f, 0x1a);
    _fmemcpy((p += 0x1a), D_da55, 0x1a);
    *(int far *)(p += 0x1a) = D_da53;
    *(int far *)(p += 0x2) = D_da51;
    *(int far *)(p += 0x2) = D_da4f;
    *(int far *)(p += 0x2) = D_da4d;
    *(int far *)(p += 0x2) = D_da4b;
    *(int far *)(p += 0x2) = D_da49;
    *(int far *)(p += 0x2) = D_da47;
    *(int far *)(p += 0x2) = D_c2bb;
    *(int far *)(p += 0x2) = D_da45;
    *(int far *)(p += 0x2) = D_da43;
    *(int far *)(p += 0x2) = combat_clock;
    _fmemcpy((p += 0x2), F_2e38_0c64, 0xfa);
    *(p += 0xfa) = D_07d8;
    *(int far *)(p += 1) = D_160c;
    *(int far *)(p += 0x2) = D_160e;
    p += 2;
    _dos_write(D_d90d, &D_d91b, 0x2a, &nw);
    D_d90f = load_buf;
    D_d919 = (unsigned)p;
    D_d917 = type = 0;
    dsize = 0x800;
    IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
    D_d91b = D_d917;
    i = 0;
    if (D_d661 != 0) {
        hp = D_d91d;
        map = floor_img;
        do {
            sprintf(fn, "level%d.flr", i);
            p = load_buf;
            swap_load2_vec(fn, map);
            _fmemcpy(p, map, 0x33fc);
            lv = (struct lvl far *)load_buf;
            for (k = 0; k < lv->count; k++) {
                e = &lv->ent[k];
                if (e->fn == secret_door_search_vec)
                    e->fn = 0;
                else if (e->fn == vec_17f7)
                    *(long far *)&e->fn = 1;
                else if (e->fn == trap_spell_vec)
                    *(long far *)&e->fn = 2;
                else if (e->fn == trap_missile_vec)
                    *(long far *)&e->fn = 4;
                else if (e->fn == vec_181f)
                    *(long far *)&e->fn = 5;
                else if (e->fn == vec_1815)
                    *(long far *)&e->fn = 6;
                else if (e->fn == vec_1833)
                    *(long far *)&e->fn = 7;
                else if (e->fn == vec_1829)
                    *(long far *)&e->fn = 8;
            }
            p += 0x33fc;
            sprintf(fn, "level%d.atv", i);
            swap_load2_vec(fn, map);
            _fmemcpy(p, map, *map * 30 + 1);
            p += *p * 30 + 1;
            sprintf(fn, "level%d.enm", i);
            file_load_far(fn, p);
            p += *p * 0x122 + 1;
            D_d90f = load_buf;
            D_d919 = (unsigned)p;
            D_d917 = 0;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            *hp++ = D_d917;
        } while (++i < (unsigned char)D_d661);
    }
    D_d941 = -1;
    if ((f = fopen(D_1164, "r")) != 0) {
        fclose(f);
        if ((D_d941 = file_load_far(D_1164, load_buf)) != 0) {
            D_d90f = load_buf;
            D_d919 = D_d941;
            D_d917 = 0;
            dsize = 0x400;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            D_d943 = D_d917;
        }
    }
    D_d93d = -1;
    if ((f = fopen(D_1162, "r")) != 0) {
        fclose(f);
        if ((D_d93d = file_load_far(D_1162, load_buf)) != 0) {
            D_d90f = load_buf;
            D_d919 = D_d93d;
            D_d917 = 0;
            dsize = 0x400;
            IMPLODE(sub_s003_0e76_0000, sub_s003_0e76_00a0, work, &type, &dsize);
            D_d93f = D_d917;
        }
    }
    _dos_close(D_d90d);
    sub_root_06a1_2c9e(path, 2, &D_d90d);
    _dos_write(D_d90d, &D_d91b, 0x2a, &nw);
    _dos_close(D_d90d);
    ems_map_page(0, 0, ems_frame_handle);
    ems_map_page(1, 1, ems_frame_handle);
    ems_map_page(2, 2, ems_frame_handle);
    ems_map_page(3, 3, ems_frame_handle);
    level_switch_vec(floor_shown);
    mouse_show();
}
#pragma optimize("", on)

/* restore the battle from saves\<name>.bsv */
#pragma optimize("a", on)
void far battle_restore(char *name)
{
    struct lvl far *lv;
    char far *p;
    char far *work;
    unsigned nr;
    int i, k, n, r;
    char buf[30];
    char path[32];
    char fn[30];

    work = load_buf - 0x63c0;
    card_window->xc = 1;
    card_window->xe = 0xff;
    strcpy(buf, name);
    i = strlen(buf);
    buf[i - 4] = 0;
    sprintf(path, "saves\\%s.bsv", buf);
    if (sub_root_06a1_2c9e(path, 0, &D_d90b) != 0)
        return;
    _dos_read(D_d90b, &D_d91b, 0x2a, &nr);
    D_d917 = 0;
    _dos_read(D_d90b, D_d90f = load_buf + 0x4e20, D_d91b, &nr);
    if (D_d91b != nr)
        sub_s003_0e76_19ca("Vars");
    D_d919 = D_d91b;
    D_d90f = load_buf + 0x4e20;
    D_d913 = load_buf;
    if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
        sub_s003_0e76_19e8("Vars", r);
    p = load_buf;
    D_907a = *p;
    _fmemcpy(cbt_paths, (p += 1), 0xcb2);
    _fmemcpy(D_d6fb, (p += 0xcb2), 0xd);
    _fmemcpy(D_c3cd, (p += 0xd), 0xe);
    D_c600 = *(p += 0xe);
    _fmemcpy(D_c7a4, (p += 1), 0x28);
    _fmemcpy(loot_piles, (p += 0x28), 0x870);
    loot_pile_count = *(p += 0x870);
    D_c83e = *(p += 1);
    D_d6f0 = *(p += 1);
    D_d728 = *(p += 1);
    D_d73b = *(p += 1);
    D_d7fe = *(p += 1);
    D_d947 = *(p += 1);
    D_d945 = *(int far *)(p += 1);
    D_d842 = *(p += 0x2);
    D_c1f0 = *(int far *)(p += 1);
    _fmemcpy(marching_order, (p += 0x2), 0x6);
    _fmemcpy(D_c64c, (p += 0x6), 0x5);
    _fmemcpy(D_c678, (p += 0x5), 0x5);
    _fmemcpy(D_c6ef, (p += 0x5), 0x5);
    _fmemcpy(D_c77e, (p += 0x5), 0x5);
    _fmemcpy(D_c784, (p += 0x5), 0x5);
    _fmemcpy(D_c7cd, (p += 0x5), 0x5);
    _fmemcpy(D_c7fe, (p += 0x5), 0x5);
    _fmemcpy(D_c8b2, (p += 0x5), 0x5);
    D_d610 = *(p += 0x5);
    combat_alarm = *(p += 1);
    cursor_current = *(int far *)(p += 1);
    _fmemcpy(cbt_moving, (p += 0x2), 0xd);
    _fmemcpy(cbt_anim, (p += 0xd), 0xd);
    _fmemcpy(cbt_action, (p += 0xd), 0xd);
    _fmemcpy(D_d3fe, (p += 0xd), 0xd);
    _fmemcpy(cbt_damage_shown, (p += 0xd), 0xd);
    _fmemcpy(D_d4e5, (p += 0xd), 0xd);
    _fmemcpy(cbt_was_hit, (p += 0xd), 0xd);
    _fmemcpy(cbt_floor, (p += 0xd), 0xd);
    _fmemcpy(cbt_gave_up, (p += 0xd), 0xd);
    _fmemcpy(D_da36, (p += 0xd), 0xd);
    _fmemcpy(D_d5f6, (p += 0xd), 0x1a);
    _fmemcpy(D_d62b, (p += 0x1a), 0x1a);
    _fmemcpy(cbt_flags, (p += 0x1a), 0x1a);
    _fmemcpy(cbt_special, (p += 0x1a), 0xd);
    _fmemcpy(D_c1c1, (p += 0xd), 0xd);
    _fmemcpy(doors_opened, (p += 0xd), 0x40);
    real_combat = *(p += 0x40);
    D_c4fd = *(p += 1);
    floor_shown = *(p += 1);
    party_floor = *(p += 1);
    D_c651 = *(p += 1);
    D_c925 = *(int far *)(p += 1);
    D_c978 = *(int far *)(p += 0x2);
    D_c989 = *(p += 0x2);
    D_ca1b = *(p += 1);
    D_ca29 = *(p += 1);
    D_07e6 = *(int far *)(p += 1);
    D_ca3f = *(int far *)(p += 0x2);
    D_ca99 = *(p += 0x2);
    battle_result = *(int far *)(p += 1);
    battle_args = *(int far *)(p += 0x2);
    D_d661 = *(p += 0x2);
    D_d662 = *(int far *)(p += 1);
    _fmemcpy(D_d68d, (p += 0x2), 0xa);
    D_dad7 = *(int far *)(p += 0xa);
    D_da34 = *(int far *)(p += 0x2);
    _fmemcpy(order_queues, (p += 0x2), 0x4e);
    D_d9e4 = *(int far *)(p += 0x4e);
    _fmemcpy(D_d9ca, (p += 0x2), 0x1a);
    _fmemcpy(D_d9b0, (p += 0x1a), 0x1a);
    _fmemcpy(cbt_engage_time, (p += 0x1a), 0x1a);
    _fmemcpy(D_d97c, (p += 0x1a), 0x1a);
    _fmemcpy(cbt_giveup_timeout, (p += 0x1a), 0x1a);
    _fmemcpy(D_d948, (p += 0x1a), 0x1a);
    _fmemcpy(D_dabd, (p += 0x1a), 0x1a);
    _fmemcpy(D_daa3, (p += 0x1a), 0x1a);
    _fmemcpy(D_da89, (p += 0x1a), 0x1a);
    _fmemcpy(D_da6f, (p += 0x1a), 0x1a);
    _fmemcpy(D_da55, (p += 0x1a), 0x1a);
    D_da53 = *(int far *)(p += 0x1a);
    D_da51 = *(int far *)(p += 0x2);
    D_da4f = *(int far *)(p += 0x2);
    D_da4d = *(int far *)(p += 0x2);
    D_da4b = *(int far *)(p += 0x2);
    D_da49 = *(int far *)(p += 0x2);
    D_da47 = *(int far *)(p += 0x2);
    D_c2bb = *(int far *)(p += 0x2);
    D_da45 = *(int far *)(p += 0x2);
    D_da43 = *(int far *)(p += 0x2);
    combat_clock = *(int far *)(p += 0x2);
    _fmemcpy(F_2e38_0c64, (p += 0x2), 0xfa);
    D_07d8 = *(p += 0xfa);
    D_160c = *(int far *)(p += 1);
    D_160e = *(int far *)(p += 0x2);
    p += 2;
    for (i = 0; i < (unsigned char)D_d661; i++) {
        _dos_read(D_d90b, D_d90f = load_buf + 0x4e20, D_d91d[i], &nr);
        if (D_d91d[i] != nr)
            sub_s003_0e76_19ca("Lv");
        D_d919 = D_d91d[i];
        D_d90f = load_buf + 0x4e20;
        D_d913 = load_buf;
        r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work);
        sprintf(msg_buf, "Lv%d", i);
        if (r != 0)
            sub_s003_0e76_19e8(msg_buf, r);
        sprintf(fn, "level%d.flr", i);
        p = load_buf;
        lv = (struct lvl far *)load_buf;
        for (k = 0; k < lv->count; k++) {
            switch (*(int far *)&lv->ent[k].fn) {
            case 0:
                lv->ent[k].fn = secret_door_search_vec;
                break;
            case 1:
                lv->ent[k].fn = vec_17f7;
                break;
            case 2:
                lv->ent[k].fn = trap_spell_vec;
                break;
            case 4:
                lv->ent[k].fn = trap_missile_vec;
                break;
            default:
                lv->ent[k].fn = vec_181f;
                break;
            case 6:
                lv->ent[k].fn = vec_1815;
                break;
            case 7:
                lv->ent[k].fn = vec_1833;
                break;
            case 8:
                lv->ent[k].fn = vec_1829;
                break;
            }
        }
        _fmemcpy(floor_img, p, 0x33fc);
        swap_save2_vec(fn, floor_img, 0x33fcL);
        sprintf(fn, "level%d.atv", i);
        p += 0x33fc;
        n = *p;
        _fmemcpy(floor_img, p, n * 30 + 1);
        swap_save2_vec(fn, floor_img, (long)(n * 30 + 1));
        sprintf(fn, "level%d.enm", i);
        p += n * 30 + 1;
        n = *p;
        file_save(fn, p, n * 0x122 + 1);
    }
    if (D_d941 == -1 || D_d941 == 0)
        remove(D_1164);
    else {
        _dos_read(D_d90b, D_d90f = load_buf + 0x4e20, D_d943, &nr);
        if (D_d943 != nr)
            sub_s003_0e76_19ca("terrf");
        D_d919 = D_d943;
        D_d90f = load_buf + 0x4e20;
        D_d913 = load_buf;
        if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
            sub_s003_0e76_19e8("terrnf", r);
        file_save(D_1164, load_buf, D_d941);
    }
    if (D_d93d == -1 || D_d93d == 0)
        remove(D_1162);
    else {
        _dos_read(D_d90b, D_d90f = load_buf + 0x4e20, D_d93f, &nr);
        if (D_d93f != nr)
            sub_s003_0e76_19ca("cf");
        D_d919 = D_d93f;
        D_d90f = load_buf + 0x4e20;
        D_d913 = load_buf;
        if ((r = EXPLODE(sub_s003_0e76_0000, sub_s003_0e76_006a, work)) != 0)
            sub_s003_0e76_19e8("cache", r);
        file_save(D_1162, load_buf, D_d93d);
    }
    _dos_close(D_d90b);
    floor_dirty = 0;
    level_switch_vec(floor_shown);
    combat_flags &= 0x10;
}
#pragma optimize("", on)

/* unexpected end of a block */
void far sub_s003_0e76_19ca(char *what)
{
    game_shutdown();
    printf("EOF-%s", what);
    exit_(0);
}

/* explode error */
void far sub_s003_0e76_19e8(char *what, int err)
{
    game_shutdown();
    printf("Corrupt file-%s %d", what, err);
    exit_(0);
}
