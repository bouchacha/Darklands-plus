/* place: s018 1f43:0006 */
/* orig: size=1088 CONST=7dde+0006 _DATA=216e+0006 */
/* data: CONST=7dde */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 2/6 (0fc2, 104c match); ALL functions written.
 * 0256: only 2 spots left: the ranged branch reloads `mov bx,e` in the original, and the
 *   spell pick loop computes D_d37c[j + e*10] as bx=j; cx=e*10; add bx,cx (6.00A: [bx+si]).
 *   Locals use hash-ordered names (slot order = name hash buckets, see PORTING_NOTES).
 * 0f02 (m4): scalars in one struct {p2, d, best, y, x} give the original frame/slots; only the
 *   home store of the first loop pointer (mov [bp-2],9C69h; mov si,[bp-2]) is missing.
 *   0c8a: #pragma optimize("a", on) (m4): only the home stores of k's initial values
 *   (mov [bp-2],0 / mov si,[bp-2] and mov [bp-2],cx) are missing.  0006: slot differences.
 * Enemy AI helpers (overlay s018, child of s014):
 * 0006 which party members are seen by an enemy (fills seen[5], returns count),
 * 0256 enemy turn AI (once per tick: drop lost targets, sort enemies by distance to the
 * party, then per enemy class D_c2bf: 0 melee walks to the least-engaged visible party member,
 * 1 archer shoots (>= 32) or closes in, 2 caster picks a spell from D_d37c[e][10] (counts) /
 * D_d411 (levels), spell 6 = targeted, others area spells aimed to hit the fewest own enemies), 0c8a pick a free tile next to a target (random offsets
 * D_214e/D_215e) or fall back to party/enemy positions, 0f02 distance table from enemy n to
 * each visible party member (F_20a5 dist[n][5]) and the nearest one into D_e1f9[n],
 * 0fc2 refresh a party member's animation frame counts, 104c do that for flagged members. */
#include "darklands/card.h"
#include "darklands/game.h"

struct cmb {
    char pad0[4];
    int x;          /* +04 */
    int y;          /* +06 */
    int f08;        /* +08 destination x */
    int f0a;        /* +0a destination y */
    char padc[4];
    int f10;        /* +10 */
    char pad12[2];
    char status;    /* +14 1 = active */
    char pad15;
    char kind;      /* +16 animation slot */
    char pad17;
    char f18;       /* +18 */
    char pad19[2];
    char f1b;       /* +1b */
    char pad1c[6];
    char f22;       /* +22 weapon */
    char pad23[0x27];
    char f4a;       /* +4a target index (-1 none) */
    char pad4b[6];
    char f51;       /* +51 */
    char pad52[0x2e];
};

struct tgt { int dist; int f2; int idx; };      /* 6-byte target list entries */
struct near2 { int dist; int who; };

/* far data segment 20a5 (one big far struct; base 0 so offsets are absolute) */
struct far20a5 {
    char pad0[0x624c];
    struct { char f0; char pad[0xf9]; } rec[13];   /* +624c, 0xfa bytes each */
    char pad[0x787e - 0x624c - 13 * 0xfa];
    int dist[13][5];                              /* +787e */
};
extern struct far20a5 far floor_img;
/* the same far data under the names each function uses (one CONST slot per variable) */
extern struct { int d[13][5]; } far F_20a5_787e;            /* distance enemy -> party member */
extern struct { struct { char f0; char pad[0xf9]; } r[13]; } far F_20a5_624c;  /* +0 = -1: no path */

extern struct cmb combatants[];
extern int los_y0, los_x0, los_y1, los_x1;
extern char floor_shown;
extern unsigned char combat_alarm;
extern char cbt_floor[];
extern char D_c4fd;
extern struct { char f0; char pad[0x11]; } D_a53b[];
extern char D_17b2[4][3];
extern char D_d5c6[];
extern int D_20ee[][6];
extern char D_214e[], D_215e[];
extern char D_d6fb[], cbt_moving[];
extern int D_d97c[];
extern unsigned D_c2bb;
extern int combat_clock;
extern struct near2 D_e1f9[];
extern int cbt_flags[];
extern char cbt_half_w[], cbt_half_h[];
extern char D_119a[], D_11a6[], D_1182[], D_118e[];
extern char D_e052[], D_e05f[], D_e06c[];
extern char ai_paused, D_d842, demo_mode, D_ca69, real_combat, D_9721, D_7d3;
extern unsigned char D_e237;
extern int D_9ee5, D_d9a0, D_da49;
extern char *D_117e;
extern char cbt_moving[], cbt_anim[], D_d1d7[], D_ca0c[], D_c97a[], cbt_action[], D_21dc[];
extern unsigned char enemy_class[];
extern char D_d37c[];             /* [13][10] spell/ammo counts */
extern unsigned char D_d411[];    /* [13][10] spell levels */
extern char *anim_file[];
extern int cbt_engage_time[];
struct spot { int x; int y; char c; char pad[5]; };
struct cand { unsigned char who; unsigned char cnt; int dist; };
void far order_queue_clear(int);
void far cbt_cancel_action(int);
void far sub_s014_1db7_05c8(int);
void far weapon_ready(int, int, int);
void far cbt_set_anim(int, int);
void far order_queue(int, int);
int far sub_s014_0e76_469e(struct spot *, int);

void *memset(void *, int, unsigned);
char *strcpy(char *, const char *);
int abs(int);
int far random_n(int);
int far line_blocked();
int far path_find(int);
void far path_give(int);
int far distance3(int, int, int);
int far cbt_anim_set(int);
int far cbt_collisions(struct cmb *);
int far enemy_sees(int *);
void far enemy_turn(void);
void far sub_s018_1f43_0c8a(int, struct tgt *);
void far sub_s018_1f43_0f02(int, int *);
void far sub_s018_1f43_0fc2(int);
void far sub_s018_1f43_104c(void);

int far enemy_sees(int *seen)
{
    int a, b, c, dy, e, p, k, d, ok, cnt, alive, n, i;

    memset(seen, 0, 10);
    cnt = 0;
    alive = 0;
    for (i = 0; i < 5; i++)
        if (combatants[i].status == 1)
            alive++;
    if (combat_alarm == 0x63) {
        n = 0;
        for (i = 0; i < 5; i++)
            if (combatants[i].status == 1) {
                seen[i] = 1;
                n++;
            }
        combat_alarm = 0;
        return n;
    }
    for (e = 5; e < 13; e++) {
        if (combatants[e].status == 1 && D_a53b[e].f0 == 0) {
            for (p = 0; p < 5; p++) {
                if (seen[p] == 0 && combatants[p].status == 1 && cbt_floor[p] == floor_shown) {
                    c = (combatants[p].x - combatants[e].x) / 2;
                    dy = combatants[p].y - combatants[e].y;
                    a = abs(c);
                    b = a ? abs(dy) * 4 / a : 20;
                    if (b == 0)
                        a = 0;
                    else if (b >= 2 && b < 8)
                        a = 1;
                    else
                        a = 2;
                    if (c >= 0 && dy < 0)
                        d = 0;
                    else if (c >= 0 && dy >= 0)
                        d = 1;
                    else if (c < 0 && dy >= 0)
                        d = 2;
                    else
                        d = 3;
                    b = D_17b2[d][a];
                    c = 0;
                    if (D_d5c6[e] == 7)
                        ok = 1;
                    else {
                        ok = c;
                        for (k = 0; k < D_d5c6[e]; k++)
                            if (D_20ee[combatants[e].kind][k] == b) {
                                ok = 1;
                                break;
                            }
                    }
                    if (ok == 1) {
                        los_x0 = combatants[e].x;
                        los_y0 = combatants[e].y;
                        los_x1 = combatants[p].x;
                        los_y1 = combatants[p].y;
                        if (line_blocked(-1, 0, 0) == 0) {
                            D_c4fd = 1;
                            seen[p] = 1;
                            cnt++;
                        }
                    }
                    if (alive == cnt)
                        break;
                }
            }
        }
    }
    return cnt;
}

void far enemy_turn(void)
{
    int e_k;
    int i_f;
    int who_;
    int type_l;
    unsigned char turn_d;
    int j_b;
    int n_m;
    int seen_[5];
    int t_f;
    int k_n;
    int m_l;
    int flag_n;
    int cnt_b;
    struct spot sp_c;
    struct cand ctmp_b;
    int cnts_m[5];
    int mn_i;
    struct near2 ntmp_d;
    int v_m;
    struct tgt list_f[5];
    struct tgt tmp_;
    struct cand cs_k[5];

    if (ai_paused == 1)
        return;
    if (D_d842 == 1 && combat_clock < D_c2bb)
        return;
    if (demo_mode != 0 && !(D_9ee5 & 0x1be)) {
        D_9ee5 = 0x12;
        D_d9a0 = combat_clock;
        D_ca69 = 7;
        if (demo_mode == 'I')
            strcpy(D_117e, "D7");
        else
            strcpy(D_117e, "CD");
        real_combat = 1;
        D_9721 = 1;
        D_da49 = combat_clock + 300;
        for (i_f = 0; i_f < 13; i_f++)
            sub_s018_1f43_0fc2(i_f);
    } else if (demo_mode == 0) {
        D_d842 = 0;
        D_7d3++;
        D_e237++;
        D_e237 &= 3;
        turn_d = D_e237 + 5;
        cnt_b = enemy_sees(seen_);
        if (cnt_b > 0 && real_combat == 0) {
            real_combat = 1;
            D_da49 = combat_clock + 300;
            for (i_f = 0; i_f < 13; i_f++)
                sub_s018_1f43_0fc2(i_f);
        }
        if (cnt_b > 0 && real_combat == 1)
            D_9721 = 1;
        /* drop targets that left the fight; when combat has started, melee/ranged enemies
         * see the whole party */
        for (i_f = 5; i_f < 13; i_f++) {
            if (combatants[i_f].status == 1 && combatants[i_f].f4a != -1 && seen_[combatants[i_f].f4a] != 1) {
drop:
                if (combatants[combatants[i_f].f4a].status != 1 || cbt_floor[combatants[i_f].f4a] != floor_shown) {
                    order_queue_clear(i_f);
                    cbt_cancel_action(i_f);
                    cbt_moving[i_f] = 0;
                    *(volatile char *)&cbt_anim[i_f] = 0;
                    combatants[i_f].f4a = -1;
                }
                if (real_combat == 1 && (enemy_class[i_f] == 1 || enemy_class[i_f] == 2)) {
                    sub_s014_1db7_05c8(i_f);
                    weapon_ready(i_f, combatants[i_f].f51, 0);
                    combatants[i_f].f18 = -1;
                    cbt_set_anim(i_f, 2);
                    order_queue_clear(i_f);
                    cbt_cancel_action(i_f);
                    combatants[i_f].f1b = 0;
                    cnt_b = 0;
                    for (m_l = 0; m_l < 5; m_l++)
                        if (combatants[m_l].status == 1) {
                            seen_[m_l] = 1;
                            cnt_b++;
                        }
                    combatants[i_f].f4a = -1;
                }
            }
        }
        if (cnt_b > 0) {
            for (m_l = 5; m_l < 13; m_l++)
                sub_s018_1f43_0f02(m_l, seen_);
            /* enemies nearest to the party act first */
            for (m_l = 5; m_l < 12; m_l++) {
                mn_i = m_l;
                ntmp_d = D_e1f9[m_l];
                for (k_n = m_l + 1; k_n < 13; k_n++)
                    if (D_e1f9[k_n].dist < ntmp_d.dist) {
                        mn_i = k_n;
                        ntmp_d = D_e1f9[k_n];
                    }
                D_e1f9[mn_i] = D_e1f9[m_l];
                D_e1f9[m_l] = ntmp_d;
            }
            for (m_l = 5; m_l < 13; m_l++) {
                e_k = D_e1f9[m_l].who;
                if (((cbt_flags[e_k] & 0x400) != 0x400 || e_k == turn_d || turn_d - e_k == -4)
                        && combatants[e_k].status == 1 && combatants[e_k].f4a == -1 && combatants[e_k].f10 == 0
                        && D_a53b[e_k].f0 == 0) {
                /* party members by number of enemies already on them, then distance */
                for (j_b = 0; j_b < 5; j_b++) {
                    cnts_m[j_b] = 0;
                    if (combatants[j_b].status == 1)
                        for (k_n = 5; k_n < 13; k_n++)
                            if (combatants[k_n].f4a == j_b)
                                cnts_m[combatants[k_n].f4a]++;
                    list_f[j_b].idx = j_b;
                    list_f[j_b].f2 = cnts_m[j_b];
                    list_f[j_b].dist = F_20a5_787e.d[e_k][j_b];
                }
                for (j_b = 0; j_b < 4; j_b++) {
                    mn_i = j_b;
                    tmp_ = list_f[j_b];
                    for (k_n = j_b + 1; k_n < 5; k_n++)
                        if (list_f[k_n].f2 < tmp_.f2 || (list_f[k_n].f2 == tmp_.f2 && list_f[k_n].dist < tmp_.dist)) {
                            mn_i = k_n;
                            tmp_ = list_f[k_n];
                        }
                    list_f[mn_i] = list_f[j_b];
                    list_f[j_b] = tmp_;
                }
                if (enemy_class[e_k] != 2) {
melee:
                    /* class 0 walks to the target, class 1 shoots from 32+ or closes in */
                    for (k_n = 0; k_n < 5; k_n++) {
                        t_f = list_f[k_n].idx;
                        if (combatants[t_f].status == 1 && seen_[t_f] == 1) {
                            switch (enemy_class[e_k]) {
                            default: break;
                            case 1:
                                if (list_f[k_n].dist < 0x20) {
                                    sub_s014_1db7_05c8(e_k);
                                    weapon_ready(e_k, combatants[e_k].f51, 0);
                                    combatants[e_k].f18 = -1;
                                    cbt_set_anim(e_k, 2);
                                    combatants[e_k].f1b = 0;
                            case 0:
                                    combatants[e_k].f08 = combatants[t_f].x;
                                    combatants[e_k].f0a = combatants[t_f].y;
                                    combatants[e_k].f4a = t_f;
                                    if (path_find(e_k) == 1) {
                                        path_give(e_k);
                                        combatants[e_k].f10 = 1;
                                        D_d6fb[e_k] = 0;
                                        cbt_moving[e_k] = 1;
                                        order_queue(e_k, 0x12);
                                        D_d97c[e_k] = random_n(90) + combat_clock + 30;
                                        cbt_flags[e_k] &= ~0x400;
                                        k_n = 100;
                                    } else
                                        combatants[e_k].f4a = -1;
                                } else {
ranged:
                                    los_x0 = combatants[e_k].x;
                                    los_y0 = combatants[e_k].y;
                                    los_x1 = combatants[t_f].x;
                                    los_y1 = combatants[t_f].y;
                                    if (line_blocked(e_k, t_f, 0x80) == 0) {
                                        weapon_ready(e_k, combatants[e_k].f22, 0x80);
                                        combatants[e_k].f4a = t_f;
                                        combatants[e_k].f10 = 0x80;
                                        cbt_engage_time[e_k] = combat_clock;
                                        cbt_action[e_k] = 7;
                                        k_n = 100;
                                    }
                                }
                            }
                        }
                    }
                    if (k_n < 100) {
                        if (enemy_class[e_k] == 1)
                            sub_s018_1f43_0c8a(e_k, list_f);
                        else
                            cbt_flags[e_k] |= 0x400;
                    }
                } else if (random_n(3) == 0) {
                    /* class 2: spell caster */
                    if (D_d1d7[e_k] == 0) {
stop:
                        sub_s014_1db7_05c8(e_k);
                        weapon_ready(e_k, combatants[e_k].f51, 0);
                        combatants[e_k].f18 = -1;
                        cbt_set_anim(e_k, 2);
                        combatants[e_k].f1b = 0;
                        goto melee;
                    }
                    if (D_d1d7[e_k] != 0) {
                        do
                            j_b = random_n(10);
                        while (j_b == 2 || D_d37c[j_b + e_k * 10] == 0);
                        type_l = j_b;
                        v_m = D_d411[j_b + e_k * 10];
                        D_d37c[j_b + e_k * 10]--;
                        D_d1d7[e_k]--;
                    }
                    los_x0 = combatants[e_k].x;
                    los_y0 = combatants[e_k].y;
                    if (type_l == 6) {
                        for (k_n = 0; k_n < 5; k_n++) {
                            t_f = list_f[k_n].idx;
                            los_x1 = combatants[t_f].x;
                            los_y1 = combatants[t_f].y;
                            if (combatants[t_f].status == 1 && seen_[t_f] == 1 && line_blocked(-1) == 0) {
                                if (list_f[k_n].dist < 0x20)
                                    goto stop;
                                anim_file[combatants[e_k].kind][0] = 'P';
                                anim_file[combatants[e_k].kind][1] = 'T';
                                D_ca0c[e_k] = type_l;
                                D_c97a[e_k] = v_m;
                                combatants[e_k].f10 = 0x100;
                                combatants[e_k].f08 = (combatants[e_k].x + combatants[t_f].x) >> 1;
                                combatants[e_k].f0a = (combatants[e_k].y + combatants[t_f].y) >> 1;
                                combatants[e_k].f4a = -1;
                                cbt_engage_time[e_k] = combat_clock;
                                cbt_action[e_k] = 7;
                                break;
                            }
                        }
                    } else {
                        /* area spell: rate each visible party member by how many enemies
                         * the spell would also hit */
                        for (j_b = 0; j_b < 5; j_b++) {
                            los_x1 = combatants[j_b].x;
                            los_y1 = combatants[j_b].y;
                            if (combatants[j_b].status == 1 && seen_[j_b] == 1 && line_blocked(-1) == 0) {
                                sp_c.x = combatants[j_b].x;
                                sp_c.y = combatants[j_b].y;
                                sp_c.c = D_21dc[type_l];
                                cs_k[j_b].who = j_b;
                                cs_k[j_b].cnt = 0;
                                for (k_n = 0; k_n < 5; k_n++)
                                    if (list_f[k_n].idx == j_b) {
                                        cs_k[j_b].dist = list_f[k_n].dist;
                                        break;
                                    }
                                if (cs_k[j_b].dist >= 0x20) {
                                    for (k_n = 5; k_n < 13; k_n++)
                                        if (combatants[k_n].status == 1 && sub_s014_0e76_469e(&sp_c, k_n) == 1)
                                            cs_k[j_b].cnt++;
                                    continue;
                                }
                            }
                            cs_k[j_b].cnt = cs_k[j_b].who = 0xff;
                        }
                        for (j_b = 0; j_b < 4; j_b++) {
                            mn_i = j_b;
                            ctmp_b = cs_k[j_b];
                            for (k_n = j_b + 1; k_n < 5; k_n++)
                                if (cs_k[k_n].cnt < ctmp_b.cnt) {
                                    mn_i = k_n;
                                    ctmp_b = cs_k[k_n];
                                }
                            cs_k[mn_i] = cs_k[j_b];
                            cs_k[j_b] = ctmp_b;
                        }
                        if (cs_k[0].who != 0xff) {
                            for (j_b = 1; j_b < 5; j_b++)
                                if (cs_k[j_b].cnt != cs_k[0].cnt) {
                                    n_m = j_b;
                                    break;
                                }
                            who_ = cs_k[random_n(n_m)].who;
                            anim_file[combatants[e_k].kind][0] = 'P';
                            anim_file[combatants[e_k].kind][1] = 'T';
                            D_ca0c[e_k] = type_l;
                            D_c97a[e_k] = v_m;
                            combatants[e_k].f10 = 0x100;
                            combatants[e_k].f08 = combatants[who_].x;
                            combatants[e_k].f0a = combatants[who_].y;
                            combatants[e_k].f4a = who_;
                            cbt_engage_time[e_k] = combat_clock;
                            cbt_action[e_k] = 7;
                        } else {
                            flag_n = 1;
                            for (j_b = 0; j_b < 5; j_b++)
                                if (cs_k[j_b].dist > 0x20 && combatants[cs_k[j_b].who].status == 1) {
                                    flag_n = 0;
                                    break;
                                }
                            if (flag_n == 1)
                                goto stop;
                            sub_s018_1f43_0c8a(e_k, list_f);
                        }
                    }
                }
                }
            }
        }
        D_7d3--;
    }
}

#pragma optimize("a", on)
void far sub_s018_1f43_0c8a(int who, struct tgt *list)
{
    int k, found, tries, t;

    found = 0;
    for (tries = 0; tries < 50; tries++) {
        k = random_n(16);
        los_x0 = D_214e[k] + combatants[who].x;
        los_y0 = D_215e[k] + combatants[who].y;
        for (k = 0; k < 5; k++)
            if (list[k].dist != 0x7d00) {
                t = list[k].idx;
                los_x1 = combatants[t].x;
                los_y1 = combatants[t].y;
                if (line_blocked(who, t, 0x80) == 0)
                    k = 100;
            }
        if (k >= 100) {
            combatants[who].f08 = los_x0;
            combatants[who].f0a = los_y0;
            if (path_find(who) == 1) {
                path_give(who);
                combatants[who].f10 = 1;
                D_d6fb[who] = 0;
                cbt_moving[who] = 1;
                D_d97c[who] = random_n(90) + combat_clock + 30;
                found = 1;
                tries = 100;
            }
        }
    }
    if (found == 0) {
        for (k = 0; k < 5; k++)
            if (list[k].dist != 0x7d00) {
                t = list[k].idx;
                combatants[who].f08 = combatants[t].x;
                combatants[who].f0a = combatants[t].y;
                combatants[who].f4a = k;
                if (path_find(who) == 1) {
                    path_give(who);
                    F_20a5_624c.r[who].f0 = -1;
                    combatants[who].f10 = 1;
                    D_d6fb[who] = 0;
                    cbt_moving[who] = 1;
                    D_d97c[who] = random_n(90) + combat_clock + 30;
                    found = 1;
                    combatants[who].f4a = -1;
                    break;
                }
            }
    }
    if (found == 0) {
        for (k = 5; k < 13; k++)
            if (combatants[k].status == 1 && who != k) {
                combatants[who].f08 = combatants[k].x;
                combatants[who].f0a = combatants[k].y;
                combatants[who].f4a = k;
                if (path_find(who) == 1) {
                    path_give(who);
                    F_20a5_624c.r[who].f0 = -1;
                    combatants[who].f10 = 1;
                    D_d6fb[who] = 0;
                    cbt_moving[who] = 1;
                    D_d97c[who] = random_n(90) + combat_clock + 30;
                    found = 1;
                    combatants[who].f4a = -1;
                    break;
                }
            }
    }
    if (found == 0)
        combatants[who].f4a = -1;
}
#pragma optimize("a", off)

void far sub_s018_1f43_0f02(int n, int *flags)
{
    int i;
    struct { int p2, d, best, y, x; } s;

    s.best = 0x7d00;
    if (combatants[n].status == 1 && combatants[n].f4a == -1) {
        s.x = combatants[n].x;
        s.y = combatants[n].y;
        for (i = 0; i < 5; i++)
            if (combatants[i].status == 1 && flags[i] == 1) {
                s.d = floor_img.dist[n][i] = distance3((combatants[i].x - s.x) / 2, combatants[i].y - s.y, 0);
                if (s.d < s.best)
                    s.best = s.d;
            } else
                floor_img.dist[n][i] = 0x7d00;
    }
    D_e1f9[n].dist = s.best;
    D_e1f9[n].who = n;
}

void far sub_s018_1f43_0fc2(int i)
{
    int k;

    if (combatants[i].status == 1) {
        cbt_flags[i] |= 0x2000;
        k = cbt_anim_set(i);
        cbt_half_w[i] = D_119a[k];
        cbt_half_h[i] = D_11a6[k];
        D_e06c[i] = D_e05f[i] = D_e052[i] = 0;
        if (cbt_collisions(&combatants[i])) {
            cbt_half_w[i] = D_1182[k];
            cbt_half_h[i] = D_118e[k];
            cbt_flags[i] &= ~0x4000;
        } else
            cbt_flags[i] |= 0x4000;
    }
}

void far sub_s018_1f43_104c(void)
{
    int i;

    for (i = 0; i < 13; i++)
        if (combatants[i].status == 1 && (cbt_flags[i] & 0x2000) == 0x2000 && !(cbt_flags[i] & 0x4000))
            sub_s018_1f43_0fc2(i);
}
