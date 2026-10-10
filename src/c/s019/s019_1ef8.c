/* place: s019 1ef8:0000 */
/* orig: size=0e1c CONST=7de4+0006 _DATA=223a+0007 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s019: combat spell/effect areas (three effect lists, per-combatant temporary modifiers).
   status: all 17 functions written; FILE MATCH. 0300 058a 0aac 0b44 0c28 are MSC near functions
   (not hand-written asm). Notes: build/agent/e7/NOTES.md */
#include "darklands/card.h"
#include "darklands/game.h"
int far rand_1_n(int);     /* position matters: operand order in 0300 case 5 */

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[8];
    int f10;                    /* +10 condition bits */
    char pad12[2];
    char status;                /* +14 1 = active */
    char pad15[1];
    char kind;                  /* +16 animation slot */
    char pad17[3];
    unsigned char f1a;          /* +1a facing */
    char pad1b[7];
    char f22;                   /* +22 */
    char pad23[1];
    char f24;                   /* +24 */
    char pad25[0x26];
    char f4b[2];                /* +4b */
    char f4d[2];                /* +4d */
    char f4f[2];                /* +4f */
    char f51;                   /* +51 */
    char pad52[5];
    char f57;                   /* +57 */
    char pad58[7];
    char f5f;                   /* +5f */
    char f60;                   /* +60 */
    char pad61[3];
    char f64;                   /* +64 */
    char pad65[6];
    char f6b[7];                /* +6b */
    char pad72[0xc];
    char nitems;                /* +7e */
    char pad7f[1];
};
struct efa { int x; int y; unsigned char f4; unsigned char kind; char f6; char f7; int t; };  /* effect_clouds[8] */
struct efb { int x; int y; char f4; char f5; char f6; char f7; int t; };                    /* effect_areas[8] */
struct efc { int x; int y; char f4; char f5; char f6; char f7; };                           /* effect_markers[8] */
struct mod { char f0; char f1; char f2[7]; char f9; int fa; int fc; int fe; int f10; };     /* D_a532[13] */
struct item { unsigned id; unsigned char f2; unsigned char f3; unsigned char f4; char f5; };
struct fx { int f0; int f2; int f4; };
struct q16 { int w0; char f2; char name[4]; char f7; char f8; char f9; char pada[12]; };

extern struct cbt combatants[];
extern struct efa effect_clouds[];
extern struct efb effect_areas[];
extern struct efc effect_markers[];
extern struct mod D_a532[];
extern struct q16 D_9a47[];
extern char effect_cloud_count;
extern char D_a395;
extern char D_a531;
extern char D_d842;
extern char floor_shown;
extern int combat_clock;
extern int D_d4c3;
extern int D_d4c5;
extern char cbt_floor[];
extern int D_c927[];
extern int D_da89[];
extern char cbt_action[];
extern int D_c897[];
extern int D_c8ba[];
extern int D_c8f7[];
extern int D_c915[];
extern char *anim_file[];
extern int D_114a;
extern int D_114e[];
extern char D_97dd;
extern unsigned char D_21dc[];
extern unsigned char D_21e6[];
extern unsigned char D_2202[];
extern unsigned char D_221d[];
extern char D_2222[];
extern char D_2226[];
extern char D_222a[];
extern char D_222e[];
extern struct fx far F_20a5_35ee[];
extern struct item far inventories[][64];
extern struct item far enemy_items[][16];

int abs(int);
int far stricmp();
int far sprintf();
int far sub_root_0410_001a(int, int, int);
char far dir_add();
void far word_fill();
int far distance3();
void far sub_s014_0e76_465e();
int far sub_s014_0e76_469e();
void far damage_queue();
void far damage_apply();
void far weapon_ready();
int far consume_item();

void far sub_s019_1ef8_01e2(int n);
void near sub_s019_1ef8_0300(int n, int dur, int target, int side);
void near sub_s019_1ef8_058a(int n, int type, int p3, int dur, int target, int side);
void far sub_s019_1ef8_0914(int i);
void far sub_s019_1ef8_0968(int i, int code, unsigned char sub, int amt);
void far sub_s019_1ef8_0a7c(int i);
void near sub_s019_1ef8_0aac(int i, int pct, int idx);
void far sub_s019_1ef8_0b00(int i);
void near sub_s019_1ef8_0b44(int i, int pct, int idx);
void far sub_s019_1ef8_0b9e(int i);
void far sub_s019_1ef8_0bf8(int i);
void near sub_s019_1ef8_0c28(int i, int idx);

/* add an effect area at (x, y) */
#pragma optimize("a", on)
void far effect_spell(int x, int y, int p3, int type, int dur, int target, int side)
{
    int i;

    dur = sub_root_0410_001a(dur, 0x14, 0x31);
    D_d842 = 0;
    if (type == 9) {
        if (D_a395 >= 8)
            return;
        effect_areas[D_a395].x = x;
        effect_areas[D_a395].y = y;
        effect_areas[D_a395].f4 = 0x49;
        effect_areas[D_a395].f6 = floor_shown;
        switch (dur / 10) {
        case 2: effect_areas[D_a395].f5 = 4; break;
        case 3: effect_areas[D_a395].f5 = 5; break;
        case 4: effect_areas[D_a395].f5 = 6; break;
        }
        effect_areas[D_a395].t = D_221d[dur / 10] * 60 + combat_clock;
        D_a395++;
        sub_s014_0e76_465e(0xc);
    } else if (type >= 6 && type <= 8) {
        if (D_a531 >= 8)
            return;
        effect_markers[D_a531].x = x;
        effect_markers[D_a531].y = y;
        effect_markers[D_a531].f4 = 0x40;
        effect_markers[D_a531].f6 = floor_shown;
        effect_markers[D_a531].f5 = type - 6;
        sub_s019_1ef8_058a(++D_a531 - 1, type, p3, dur, target, side);
        sub_s014_0e76_465e(0xd);
    } else {
        if (effect_cloud_count < 8) {
            effect_clouds[effect_cloud_count].x = x;
            effect_clouds[effect_cloud_count].y = y;
            effect_clouds[effect_cloud_count].kind = type;
            effect_clouds[effect_cloud_count].f4 = D_21dc[type];
            effect_clouds[effect_cloud_count].f7 = floor_shown;
            effect_clouds[effect_cloud_count].t = D_2202[type * 3 + dur / 10] * 60 + combat_clock;
            sub_s019_1ef8_0300(++effect_cloud_count - 1, dur, target, side);
            sub_s014_0e76_465e(0xc);
            return;
        } else {
            for (i = 0; i < effect_cloud_count; i++)
                if (effect_clouds[i].kind != 2)
                    goto found;
            return;
found:
            sub_s019_1ef8_01e2(i);
            effect_clouds[effect_cloud_count].x = x;
            effect_clouds[effect_cloud_count].y = y;
            effect_clouds[effect_cloud_count].kind = type;
            effect_clouds[effect_cloud_count].f4 = D_21dc[type];
        }
        effect_clouds[effect_cloud_count].t = D_2202[type * 3 + dur / 10] * 60 + combat_clock;
        sub_s019_1ef8_0300(++effect_cloud_count - 1, dur, target, side);
        sub_s014_0e76_465e(0xc);
    }
}
#pragma optimize("", on)

/* remove entry n of the first effect list */
void far sub_s019_1ef8_01e2(int n)
{
    int i;

    if (effect_cloud_count != 0) {
        for (i = n; i < effect_cloud_count - 1; i++) {
            effect_clouds[i] = effect_clouds[i + 1];
            D_c8f7[i] = D_c8f7[i + 1];
            D_c915[i] = D_c915[i + 1];
            D_c897[i] = D_c897[i + 1];
            D_c8ba[i] = D_c8ba[i + 1];
        }
        effect_cloud_count--;
    }
}

/* remove entry n of the second effect list */
void far sub_s019_1ef8_0260(int n)
{
    int i;

    if (D_a395 != 0) {
        for (i = n; i < D_a395 - 1; i++)
            effect_areas[i] = effect_areas[i + 1];
        D_a395--;
    }
}

/* remove entry n of the third effect list */
void far sub_s019_1ef8_02b4(int n)
{
    int i;

    if (D_a531 != 0) {
        for (i = n; i < D_a531 - 1; i++)
            effect_markers[i] = effect_markers[i + 1];
        D_a531--;
    }
}

/* apply effect n of the first list to every combatant of the side inside it */
#pragma optimize("a", on)
void near sub_s019_1ef8_0300(int n, int dur, int target, int side)
{
    unsigned kind;
    int lvl;
    int t;
    int i;

    kind = effect_clouds[n].kind;
    for (i = 0; i < 13; i++) {
        if (combatants[i].status == 1 && cbt_floor[i] == side
            && sub_s014_0e76_469e(&effect_clouds[n], i) == 1
            && !(D_c927[i] & (1 << kind))) {
            lvl = dur / 10 - 2;
            t = dur / 10 + kind * 3 - 2;
            D_da89[i] = D_21e6[t] * 60 + combat_clock;
            F_20a5_35ee[i].f2 = D_d4c3;
            F_20a5_35ee[i].f4 = D_d4c5;
            F_20a5_35ee[i].f0 = -(D_21dc[kind] >> 4);
            switch (kind) {
            case 0:
                sub_s019_1ef8_0a7c(i);
                sub_s019_1ef8_0b00(i);
                if (target == i) {
                    sub_s019_1ef8_0aac(i, 0x32, t);
                    sub_s019_1ef8_0b44(i, 0x32, t);
                } else {
                    sub_s019_1ef8_0aac(i, 0x19, t);
                    sub_s019_1ef8_0b44(i, 0x19, t);
                }
                break;
            case 1:
                sub_s019_1ef8_0a7c(i);
                sub_s019_1ef8_0b00(i);
                if (target == i) {
                    sub_s019_1ef8_0bf8(i);
                    sub_s019_1ef8_0aac(i, 0x21, t);
                    sub_s019_1ef8_0b44(i, 0x21, t);
                    sub_s019_1ef8_0c28(i, t);
                } else {
                    sub_s019_1ef8_0aac(i, 0x19, t);
                    sub_s019_1ef8_0b44(i, 0x19, t);
                }
                break;
            case 3:
                sub_s019_1ef8_0b00(i);
                sub_s019_1ef8_0b44(i, (combatants[i].f4d[1] + combatants[i].f4d[0]) * 5, t);
                break;
            case 4:
                t = rand_1_n(8) + (lvl + 2) * 2;
                if (combatants[i].f4b[0] != -1) {
                    sub_s019_1ef8_0968(i, combatants[i].f4b[0], combatants[i].f4f[0], t);
                    combatants[i].f4f[0] -= t;
                }
                if (combatants[i].f4b[1] != -1) {
                    sub_s019_1ef8_0968(i, combatants[i].f4b[1], combatants[i].f4f[1], t);
                    combatants[i].f4f[1] -= t;
                }
                if (combatants[i].f4f[0] <= 0 || combatants[i].f4b[0] == -1) {
                    if (combatants[i].f4b[0] != -1)
                        t = -combatants[i].f4f[0];
                    damage_queue(t, i, 1, 0);
                    damage_apply(i);
                }
                sub_s019_1ef8_0914(i);
                break;
            case 5:
                t = (lvl + 2) * 2 + rand_1_n(8);
                damage_queue(((lvl + 2) * 10 - combatants[i].f64) / 10 + t, i,
                                   combatants[i].f4d[0] + 1, combatants[i].f4d[0]);
                damage_apply(i);
                break;
            }
        }
    }
}
#pragma optimize("", on)

#pragma optimize("a", on)
/* apply effect n of the third list (types 6..8) */
void near sub_s019_1ef8_058a(int n, int type, int p3, int dur, int target, int side)
{
    int lvl;
    int idx;
    int dx;
    int dy;
    int adx;
    int r;
    int s;
    int q;
    int th;
    int sk;
    int dmg;
    unsigned char c;
    unsigned char f;
    char d;
    int i;
    int ady;

    for (i = 0; i < 13; i++) {
        if (combatants[i].status == 1 && cbt_floor[i] == side && !(D_c927[i] & (1 << type))) {
            lvl = idx = dur / 10 - 2;
            if (idx > 2)
                idx = 2;
            idx += type * 3;
            if (type == 6) {
                if ((p3 < 5 && i >= 5) || (p3 >= 5 && i < 5)) {
                    dx = effect_markers[n].x - combatants[i].x;
                    dy = effect_markers[n].y - combatants[i].y;
                    adx = abs(dx);
                    ady = abs(dy);
                    r = adx ? ady * 4 / adx : 0x14;
                    if (r == 0)
                        s = 0;
                    else if (r >= 1 && r < 4)
                        s = 1;
                    else
                        s = 2;
                    if (dx >= 0 && dy < 0)
                        q = 0;
                    else if (dx >= 0 && dy >= 0)
                        q = 1;
                    else if (dx < 0 && dy >= 0)
                        q = 2;
                    else
                        q = 3;
                    c = D_222e[q * 3 + s];
                    d = dir_add(c = dir_add(c, 1),
                                           f = dir_add(combatants[i].f1a, 1));
                    if (d > 4)
                        d |= 0xf8;
                    d = abs(d);
                    if (d <= 2 || target == i) {
                        sub_s019_1ef8_0c28(i, idx);
                        D_da89[i] = D_21e6[idx] * 60 + combat_clock;
                        F_20a5_35ee[i].f2 = D_d4c3;
                        F_20a5_35ee[i].f4 = D_d4c5;
                        F_20a5_35ee[i].f0 = -2;
                    }
                }
            } else if (sub_s014_0e76_469e(&effect_markers[n], i) == 1) {
                r = distance3((effect_markers[n].x - combatants[i].x) / 2, effect_markers[n].x - combatants[i].x, 0);
                D_da89[i] = D_21e6[idx] * 60 + combat_clock;
                F_20a5_35ee[i].f2 = D_d4c3;
                F_20a5_35ee[i].f4 = D_d4c5;
                F_20a5_35ee[i].f0 = -4;
                switch (type) {
                case 7:
                    dmg = rand_1_n(8) + (lvl + 2) * 2;
                    if (target == i)
                        th = D_2222[lvl];
                    else if (r <= 0x10)
                        th = D_2226[lvl];
                    else
                        th = D_222a[lvl];
                    sk = combatants[i].f4d[0];
                    if (sk < th)
                        r = dmg;
                    else if (sk == th)
                        r = dmg / 3;
                    else
                        r = dmg / 8;
                    damage_queue(((lvl + 2) * 10 - combatants[i].f4f[0]) / 10 + r, i, th, sk);
                    damage_apply(i);
                    if (dmg >> 2 > 0) {
                        dmg >>= 2;
                        sub_s019_1ef8_0968(i, combatants[i].f4b[0], combatants[i].f4f[0], dmg);
                        sub_s019_1ef8_0968(i, combatants[i].f4b[1], combatants[i].f4f[1], dmg);
                        combatants[i].f4f[0] -= dmg;
                        combatants[i].f4f[1] -= dmg;
                        sub_s019_1ef8_0914(i);
                    }
                    break;
                case 8:
                    dmg = rand_1_n(6) + (lvl + 2) * 2;
                    if (r > 0x10)
                        dmg >>= 2;
                    damage_queue((combatants[i].f4f[0] + combatants[i].f24) / -10 + dmg, i,
                                       combatants[i].f4d[0] + 1, combatants[i].f4d[0]);
                    damage_apply(i);
                    if (dmg >> 2 > 0) {
                        dmg >>= 2;
                        sub_s019_1ef8_0968(i, combatants[i].f4b[0], combatants[i].f4f[0], dmg);
                        sub_s019_1ef8_0968(i, combatants[i].f4b[1], combatants[i].f4f[1], dmg);
                        combatants[i].f4f[0] -= dmg;
                        combatants[i].f4f[1] -= dmg;
                        sub_s019_1ef8_0914(i);
                    }
                    break;
                }
            }
        }
    }
}
#pragma optimize("", on)

/* drop worn-out items from both hands */
void far sub_s019_1ef8_0914(int i)
{
    int k;

    for (k = 0; k <= 1; k++) {
        if (combatants[i].f4f[k] <= 0) {
            while (consume_item(i, combatants[i].f4b[k], combatants[i].f4f[k]) != -1)
                ;
            combatants[i].f4b[k] = -1;
            combatants[i].f4d[k] = 0;
            combatants[i].f4f[k] = 0x19;
        }
    }
}

/* lower the quality of an inventory item of combatant i */
void far sub_s019_1ef8_0968(int i, int code, unsigned char sub, int amt)
{
    int found;
    int k;
    int pos;

    found = 0;
    if (i < 5) {
        for (k = 0; k < combatants[i].nitems; k++) {
            if (inventories[i][k].f2 == code && inventories[i][k].f3 == sub) {
                pos = k;
                found = 1;
                break;
            }
        }
        if (found == 1)
            inventories[i][pos].f3 -= amt;
    } else {
        for (k = 0; k < combatants[i].nitems; k++) {
            if (enemy_items[i - 5][k].f2 == code && enemy_items[i - 5][k].f3 == sub) {
                pos = k;
                found = 1;
                break;
            }
        }
        if (found == 1)
            enemy_items[i - 5][pos].f3 -= amt;
    }
}

/* restore the attribute modifiers held in D_a532[i] */
void far sub_s019_1ef8_0a7c(int i)
{
    combatants[i].f5f += D_a532[i].f0;
    D_a532[i].f0 = 0;
    combatants[i].f60 += D_a532[i].f1;
    D_a532[i].f1 = 0;
    D_a532[i].fc = -536;
}

void near sub_s019_1ef8_0aac(int i, int pct, int idx)
{
    D_a532[i].f0 = combatants[i].f5f * pct / 100;
    combatants[i].f5f -= D_a532[i].f0;
    D_a532[i].f1 = combatants[i].f60 * pct / 100;
    combatants[i].f60 -= D_a532[i].f1;
    D_a532[i].fc = D_21e6[idx] * 60 + combat_clock;
}

void far sub_s019_1ef8_0b00(int i)
{
    int k;

    for (k = 0; k < 7; k++) {
        combatants[i].f6b[k] += D_a532[i].f2[k];
        D_a532[i].f2[k] = 0;
    }
    sub_s019_1ef8_0b9e(i);
    D_a532[i].fe = -536;
}

void near sub_s019_1ef8_0b44(int i, int pct, int idx)
{
    int k;

    for (k = 0; k < 7; k++) {
        D_a532[i].f2[k] = combatants[i].f6b[k] * pct / 100;
        combatants[i].f6b[k] -= D_a532[i].f2[k];
    }
    sub_s019_1ef8_0b9e(i);
    D_a532[i].fe = D_21e6[idx] * 60 + combat_clock;
}

void far sub_s019_1ef8_0b9e(int i)
{
    if ((combatants[i].f57 & 0x80) == 0x80 && stricmp(anim_file[combatants[i].kind], "PT"))
        weapon_ready(i, combatants[i].f22, 0x80);
    else
        weapon_ready(i, combatants[i].f51, 0);
}

void far sub_s019_1ef8_0bf8(int i)
{
    D_a532[i].f9 = 0;
    D_a532[i].f10 = -536;
    if ((D_a532[i].fa & 2) == 2)
        combatants[i].f10 = D_a532[i].fa;
}

#pragma optimize("a", on)
void near sub_s019_1ef8_0c28(int i, int idx)
{
    D_a532[i].f9 = 1;
    D_a532[i].f10 = D_21e6[idx] * 60 + combat_clock;
    if (((D_a532[i].fa = combatants[i].f10) & 2) == 2)
        combatants[i].f10 = 0x22;
    if (combatants[i].f10 == 0x80 || combatants[i].f10 == 0x100)
        cbt_action[i] = 0xd;
}
#pragma optimize("", on)

/* build the list of potions (items 0x5f..0x68) of party member i */
void far sub_s019_1ef8_0c80(int i)
{
    struct { int id; int q; int c; } list[30];
    int cnt;
    int k;
    int n;
    int a;

    cnt = 0;
    if ((combatants[i].f10 & 2) == 2) {
        D_97dd = 2;
        return;
    }
    word_fill(list, -1, 0x5a);
    for (k = 0; k < combatants[i].nitems; k++) {
        if (inventories[i][k].id >= 0x5f && inventories[i][k].id <= 0x68
            && inventories[i][k].f3 >= 0x14 && inventories[i][k].f3 < 0x32
            && inventories[i][k].id != 0x61) {
            a = inventories[i][k].id - 0x5f;
            n = a * 3 + inventories[i][k].f3 / 10 - 2;
            list[n].id = a;
            list[n].q = inventories[i][k].f3;
            list[n].c = inventories[i][k].f4;
            cnt++;
        }
    }
    for (k = 1; cnt > 20 && k < 30; k += 3) {
        if (list[k].id != -1) {
            list[k].id = -1;
            cnt--;
        }
    }
    n = 0;
    for (k = 0; k < 30; k++) {
        if (list[k].id != -1) {
            D_9a47[n].w0 = D_114e[list[k].id];
            D_9a47[n].f9 = list[k].id;
            D_9a47[n].f7 = list[k].c;
            D_9a47[n].f8 = list[k].q;
            sprintf(D_9a47[n++].name, "q%d", list[k].q);
        }
    }
    D_9a47[n].w0 = D_114a;
    D_97dd = n ? 1 : 2;
}

void far sub_s019_1ef8_0dfe(int i, int a, int b)
{
    if (i < 5)
        consume_item(i, a + 0x84, b);
}
