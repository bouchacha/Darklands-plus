/* place: s014 1db7:0008 */
/* orig: size=1118 CONST=7db2+0006 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: CONST=7db2 */
/* s014 1db7: COMBAT AI (target selection and melee engagement).
     0008  AI tick: reach table (09ae), react to moved targets (026a); in real combat
           (D_a6c1 == 1) also engage idle neighbours (06cc), fight back (0840) and
           re-think the targets of two combatants per scheduler slot (0d36 -> 0e22).
     0028  path cost from i to every opponent lo..hi-1 (pathfinder 12f7_03ea on a scratch
           copy of the record); row F_20a5_787e[i*5..], best in D_e1f9[i]
     0158  which combatants are adjacent to the waypoints (F_20a5_72b4/7304), positions
           remembered in F_20a5_79a9[i][j]
     026a  follow targets that moved (re-path, or give up for 60 ticks: D_d5e9/D_d962)
     0418/047e/05c8  enemy animation slots 5/6 (two enemy sprite sets can be loaded):
           switch an enemy to another slot / release a slot (s025 vec_1dd3/vec_1ddd)
     09ae  F_20a5_7900[i][j] = 1 if i and j are within melee reach (symmetric)
     0a80  D_e0ac[k] = 1 for every combatant k inside i's weapon reach
     0ba8  stance for attacking t: 0x22 (t much more skilled), 0x12 or 10
     0c2a  set target t and start walking / fighting
     0cfa  F_20a5_7c4d[i] = endurance * 64 / max endurance (255 = out)
     0e22  re-evaluate the target of i: keep it, help a friend being attacked, or pick the
           weakest reachable opponent (endurance fraction + path cost / 5)
   status: 16/16 written, 8 match (06cc: #pragma optimize("a", on), m4; 09ae: pragma a + the status test written twice
   (outer guard and inside the inner loop), m4; 047e: pragma a, n/k home stores left; 0c2a now: AI tables are one far struct F_20a5_787e, CONST
   7db2/7db4/7db6 like the original, re-stamped e8 2026-10-09). 0028 (far) and the near functions 06cc 0840 09ae 0ba8 0c2a 0cfa
   0d36 0e22 as in the original.  m4: 0418 rewritten with a 'free' flag (f = 1 / f = 0; break), only the
   direction of cmp di,dx (i != j) is left; decl positions, renames, pragmas tried. */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    int tx;                     /* +08 destination */
    int ty;                     /* +0a */
    char padc[4];
    int f10;                    /* +10 action: 1 walking, 2 bit = melee, stance bits */
    char pad12[2];
    char status;                /* +14 1 active, 2 unconscious, 3 dead */
    char pad15;
    char kind;                  /* +16 animation slot (0..4 party, 5/6 enemy sets) */
    char pad17;
    char f18;                   /* +18 */
    char pad19[2];
    char f1b;                   /* +1b */
    char pad1c[0x2e];
    char target;                /* +4a target combatant (-1 none) */
    char pad4b[2];
    char f4d;                   /* +4d armour (location 0) */
    char pad4e[3];
    char f51;                   /* +51 default weapon */
    char pad52;
    unsigned char f53;          /* +53 weapon class | penetration */
    char pad54[5];
    char f59;                   /* +59 weapon skill */
    char pad5a[3];
    char endur;                 /* +5d endurance */
    char pad5e[6];
    char maxend;                /* +64 maximum endurance */
    char pad65[0x1b];
};
struct best { int cost; int who; };

char far *far _fstrcpy(char far *, const char far *);
void far *far _fmemset(void far *, int, unsigned);
char *strcpy(char *, const char *);
void *memset(void *, int, unsigned);
int abs(int);
void far party_panel_rows(int);
void far word_fill_far(int far *, int, int);   /* far word fill */
int far index_of_min_byte(unsigned char far *, int); /* index of the smallest byte */
int far index_of_min_int(int far *, int);         /* index of the smallest int */
unsigned far distance3(int, int, int);     /* distance */
void far cbt_set_anim(int, int);
void far order_queue(int, int);
void far order_queue_clear(int);
int far path_find(int);
int far path_trackers(struct cbt *);
void far path_give(int);
int far cbt_collisions(struct cbt *);
int far cbt_anim_set(int);
void far weapon_ready(int, int, int);
void far enemy_anim_free6_vec(int);                 /* s025: free enemy animation set */
void far enemy_anim_load_vec(int, int);            /* s025: load enemy animation set */
void far sub_s014_1db7_0028(int, int, int);
int far sub_s014_1db7_047e(int);
void far sub_s014_1db7_0418(int, int);
void far sub_s014_1db7_05c8(int);
void far sub_s014_1db7_0a80(int, int);
void near sub_s014_1db7_06cc(void);
void near sub_s014_1db7_0840(void);
void near sub_s014_1db7_09ae(void);
int near sub_s014_1db7_0ba8(int, int, int);
void near ai_set_target(int, int);
void near sub_s014_1db7_0cfa(void);
void near sub_s014_1db7_0d36(void);
void near ai_rethink(int, int);
void far sub_s014_1db7_026a(void);

extern struct cbt combatants[];
/* the AI tables 20a5:787e..7c59 are ONE far variable in the original (CONST slot 7db2) */
struct aitab {
    int cost[65];                       /* 787e path cost rows: [i*5 + j] (rows overlap!) */
    char reach[13][13];                 /* 7900 within melee reach */
    int pos[13][13][2];                 /* 79a9 remembered target positions */
    unsigned char pref[13];             /* 7c4d AI preference value */
};
extern struct aitab far F_20a5_787e;
#define F_20a5_7900 (F_20a5_787e.reach)
#define F_20a5_79a9 (F_20a5_787e.pos)
#define F_20a5_7c4d (F_20a5_787e.pref)
extern int far F_20a5_72b4[];           /* waypoint x */
extern int far F_20a5_7304[];           /* waypoint y */
extern char far anim_set_names[][20];      /* animation set names */
extern struct cbt D_d73e;               /* scratch record for the pathfinder */
extern int D_e116;                      /* path cost of the last search */
extern struct best D_e1f9[];            /* [13] */
extern int D_e11a;                      /* number of waypoints */
extern char D_1c36[];                   /* [10] direction dx */
extern char D_1c40[];                   /* [10] direction dy */
extern char D_c681[][10];               /* weapon reach x per animation set */
extern char D_c6f4[][10];               /* weapon reach y per animation set */
extern char D_e0ac[16];                 /* in reach */
extern int D_e11c[16];                  /* collision result per combatant */
extern char D_e13c;
extern char ai_paused;                     /* 1 = AI off */
extern char real_combat;                     /* 1 = real combat */
extern char demo_mode;                     /* demo mode (5 = player controlled) */
extern char D_e079;
extern char cbt_floor[];                   /* [13] floor */
extern char party_floor;                     /* floor of the party */
extern char cbt_moving[];                   /* [13] moving to target */
extern unsigned char cbt_flags[][2];       /* [13] flag words (byte 1 bit 4: AI disabled) */
extern char cbt_gave_up[];                   /* [13] gave up following */
extern unsigned cbt_giveup_timeout[];               /* [13] give-up timeout */
extern char D_d6fb[];                   /* [13] */
extern char leader;                     /* leader */
extern char move_mode;                     /* movement mode (2 = single file) */
extern unsigned combat_clock;                 /* combat clock */
extern unsigned char cbt_attack_rate[];          /* [13] attack rate */
extern char D_da36[];                   /* [13] */
extern unsigned cbt_engage_time[];               /* [13] time of engagement */
extern int D_d97c[];                    /* [13] */
extern char cbt_action[];                   /* [13] action */
extern char cbt_anim[];                   /* [13] current animation (3 dying, 5, 0xe) */
extern char enemy_class[];                   /* [13] */
extern int D_c200[];                    /* [13] animation id */
extern int enemy_anim_slot, D_d67e;              /* animation id loaded in slot 5 / 6 (-1 free) */
extern char D_d680[];                   /* [13] preferred slot */
extern int anim_buffers[];                    /* [7] animation buffers */
extern char *D_1166[];                  /* [7] animation file names */
extern char *anim_file[];
extern unsigned D_da53, D_da51;         /* scheduler times */
extern unsigned char D_e09c, D_e09b, D_e09a;

void far ai_tick(void)
{
    if (ai_paused != 1) {
        sub_s014_1db7_09ae();
        sub_s014_1db7_026a();
        if (real_combat == 1) {
            sub_s014_1db7_06cc();
            sub_s014_1db7_0840();
            sub_s014_1db7_0d36();
        }
    }
}

/* path cost from i to the opponents lo..hi-1 */
void far sub_s014_1db7_0028(int i, int lo, int hi)
{
    int best;
    int j, k, n;
    int far *row;

    best = 32000;
    if (combatants[i].status == 1) {
        row = &F_20a5_787e.cost[i * 5];
        word_fill_far(row, 32000, 13);
        for (j = lo; j < hi; j++) {
            if (combatants[j].status != 1)
                continue;
            if (i >= 5) {
                /* enemies do not crowd a target that already has 3 attackers */
                n = 0;
                for (k = 5; k < 13; k++)
                    if (combatants[k].target == j && combatants[k].status == 1
                     && ((combatants[k].f10 & 2) == 2 || combatants[k].f10 == 1))
                        n++;
                if (n >= 3)
                    continue;
            }
            D_d73e = combatants[i];
            D_d73e.tx = combatants[j].x;
            D_d73e.ty = combatants[j].y;
            D_d73e.target = j;
            if (path_trackers(&D_d73e) == 1) {
                F_20a5_787e.cost[i * 5 + j] = D_e116;
                if (D_e116 < best)
                    best = F_20a5_787e.cost[i * 5 + j];
            }
        }
    }
    D_e1f9[i].cost = best;
    D_e1f9[i].who = i;
}

/* combatants adjacent to the waypoints, as seen from i */
void far sub_s014_1db7_0158(int i)
{
    unsigned mask;
    int x, y, k, d, j;

    mask = 0;
    x = combatants[i].x;
    y = combatants[i].y;
    for (k = 0; k < D_e11a; k++)
        for (d = 0; d < 8; d++) {
            combatants[i].x = D_1c36[d] + F_20a5_72b4[k];
            combatants[i].y = D_1c40[d] + F_20a5_7304[k];
            D_e13c = 1;
            mask |= cbt_collisions(&combatants[i]);
            D_e13c = 0;
        }
    for (j = 0; j < 13; j++, mask >>= 1) {
        if ((mask & 1) == 1 && combatants[j].status == 1
         && (cbt_moving[j] == 0 || combatants[i].target == j)) {
            F_20a5_79a9[i][j][0] = combatants[j].x;
            F_20a5_79a9[i][j][1] = combatants[j].y;
        } else {
            F_20a5_79a9[i][j][0] = -1;
            F_20a5_79a9[i][j][1] = -1;
        }
    }
    combatants[i].x = x;
    combatants[i].y = y;
}

/* follow targets that moved away from where they were seen */
void far sub_s014_1db7_026a(void)
{
    int i, j;
    int far *q;

    for (i = 0; i < 13; i++) {
        if (combatants[i].status == 1 && cbt_moving[i] == 1 && cbt_gave_up[i] == 0) {
            for (j = 0; j < 13; j++) {
                q = F_20a5_79a9[i][j];
                if (q[0] == -1 || (q[0] == combatants[j].x && q[1] == combatants[j].y))
                    continue;
                if (distance3((q[0] - combatants[i].x) / 2, q[1] - combatants[i].y, 0) < 17) {
                    if (combatants[i].target == j) {
                        combatants[i].tx = combatants[j].x;
                        combatants[i].ty = combatants[j].y;
                    }
                } else {
                    if (combatants[i].target != j)
                        continue;
                    if ((int)distance3((q[0] - combatants[j].x) / 2, q[1] - combatants[j].y, 0) < 12
                     && (cbt_moving[leader] != 0 || move_mode != 2 || (j > 4 || i > 4) && (j < 5 || i < 5)))
                        continue;
                    combatants[i].tx = combatants[j].x;
                    combatants[i].ty = combatants[j].y;
                }
                if (path_find(i) == 1) {
                    path_give(i);
                    D_d6fb[i] = 0;
                } else {
                    cbt_gave_up[i] = 1;
                    cbt_giveup_timeout[i] = combat_clock + 60;
                }
                break;
            }
        } else if (cbt_gave_up[i] == 1 && cbt_giveup_timeout[i] < combat_clock)
            cbt_gave_up[i] = 0;
    }
}

/* enemy i left animation slot `kind`: free the slot when nobody else uses it */
void far sub_s014_1db7_0418(int i, int kind)
{
    int j, f;

    if (i >= 5) {
        f = 1;
        for (j = 5; j < 13; j++)
            if (i != j && combatants[j].status != 0 && combatants[j].kind == kind) {
                f = 0;
                break;
            }
        if (f == 1) {
            if (kind == 5) {
                enemy_anim_slot = -1;
                enemy_anim_free6_vec(kind);
            } else {
                D_d67e = -1;
                enemy_anim_free6_vec(6);
            }
        }
    }
}

/* move enemy i to the other enemy animation slot if it is free; returns the new slot or 0 */
#pragma optimize("a", on)
int far sub_s014_1db7_047e(int i)
{
    int n;
    int k, j;

    n = 0;
    k = combatants[i].kind;
    if (k == 5 && D_d67e == -1) {
        n = 6;
        D_d67e = D_c200[i];
    } else if (k == 6 && enemy_anim_slot == -1) {
        n = 5;
        enemy_anim_slot = D_c200[i];
    }
    if (n == 0)
        return 0;
    strcpy(D_1166[n], D_1166[k]);
    strcpy(anim_file[n], anim_file[k]);
    combatants[i].kind = n;
    _fstrcpy(anim_set_names[n], anim_set_names[k]);
    for (j = 5; j < 13; j++)
        if (combatants[j].status == 1 && combatants[j].kind == k)
            D_d680[j] = n;
    if (anim_buffers[n] == 0) {
        weapon_ready(i, combatants[i].f51, 0);
        enemy_anim_load_vec(n, i);
        combatants[i].f1b = 0;
    }
    return n;
}
#pragma optimize("a", off)

/* enemy i changes its animation set (weapon change) */
void far sub_s014_1db7_05c8(int i)
{
    int n, j;
    int k;
    char s;

    n = 0;
    for (j = 5; j < 13; j++)
        if (combatants[j].status != 0 && D_c200[i] == D_c200[j])
            n++;
    if (n == 1) {
        weapon_ready(i, combatants[i].f51, 0);
        *(volatile char *)&combatants[i].f1b = 0;
        *(volatile char *)&enemy_class[i] = 0;
        if (combatants[i].kind == 5)
            enemy_anim_slot = D_c200[i];
        else
            D_d67e = D_c200[i];
        return;
    }
    k = combatants[i].kind;
    if (D_d680[i] != -1) {
        combatants[i].kind = s = D_d680[i];
        if (anim_buffers[s] == 0) {
            weapon_ready(i, combatants[i].f51, 0);
            enemy_anim_load_vec(combatants[i].kind, i);
            combatants[i].f1b = 0;
        }
    } else
        sub_s014_1db7_047e(i);
    enemy_class[i] = 0;
    if (combatants[i].kind != k)
        sub_s014_1db7_0418(i, k);
}

/* idle combatants attack an adjacent opponent that is not fighting */
#pragma optimize("a", on)
void near sub_s014_1db7_06cc(void)
{
    int i, j, lo, hi;

    if (real_combat != 1)
        return;
    for (i = 0; i < 13; i++) {
        if ((demo_mode == 0 || i != 5) && (cbt_flags[i][1] & 0x10) == 0
         && combatants[i].status == 1 && (combatants[i].f10 & 2) == 0) {
            if (i < 5) {
                lo = 5;
                hi = 13;
            } else {
                lo = 0;
                hi = 5;
            }
            for (j = lo; j < hi; j++)
                if (F_20a5_7900[i][j] == 1 && combatants[j].target != i && combatants[i].target != j
                 && cbt_floor[j] == cbt_floor[i] && (combatants[j].f10 & 2) == 0) {
                    if (i >= 5 && enemy_class[i] != 0)
                        sub_s014_1db7_05c8(i);
                    weapon_ready(i, combatants[i].f51, 0);
                    combatants[i].f18 = -1;
                    cbt_set_anim(i, 2);
                    combatants[i].target = j;
                    cbt_attack_rate[i] = 0xff;
                    D_da36[i] = 1;
                    cbt_moving[i] = 0;
                    order_queue_clear(i);
                    combatants[i].f10 = sub_s014_1db7_0ba8(i, j, 0);
                    cbt_engage_time[i] = combat_clock;
                    cbt_action[i] = 7;
                    party_panel_rows(i);
                    break;
                }
        }
    }
}
#pragma optimize("a", off)

/* fight back against an adjacent attacker */
void near sub_s014_1db7_0840(void)
{
    int i, j;

    for (i = 0; i < 13; i++) {
        if ((demo_mode == 0 || i != 5) && combatants[i].status == 1 && D_e079 == 0
         && (cbt_flags[i][1] & 0x10) == 0) {
            for (j = 0; j < 13; j++)
                if (F_20a5_7900[i][j] == 1 && combatants[j].status == 1 && combatants[j].target == i
                 && cbt_floor[j] == cbt_floor[i] && (combatants[j].f10 & 2) != 0) {
                    if (((combatants[i].f10 & 2) == 0 || combatants[combatants[i].target].target != i)
                     && ((combatants[i].f10 & 2) == 0 || combatants[combatants[i].target].target != i)) {
                        cbt_moving[i] = 0;
                        if (i >= 5 && enemy_class[i] != 0)
                            sub_s014_1db7_05c8(i);
                        order_queue_clear(i);
                        D_da36[i] = 1;
                        combatants[i].f18 = -1;
                        weapon_ready(i, combatants[i].f51, 0);
                        cbt_set_anim(i, 2);
                        combatants[i].target = j;
                        combatants[i].f10 = sub_s014_1db7_0ba8(i, j, 0);
                        cbt_attack_rate[i] = 0xff;
                        cbt_engage_time[i] = combat_clock;
                        cbt_action[i] = 7;
                        party_panel_rows(i);
                    }
                    break;
                }
        }
    }
}

/* melee reach table */
#pragma optimize("a", on)
void near sub_s014_1db7_09ae(void)
{
    int i, j;

    _fmemset(F_20a5_7900, 0, 169);
    for (i = 0; i < 13; i++)
        if (combatants[i].status == 1) {
            sub_s014_1db7_0a80(i, 1);
            for (j = 0; j < 13; j++)
                F_20a5_7900[i][j] = D_e0ac[j];
        }
    for (i = 0; i < 13; i++)
        if (combatants[i].status == 1)
            for (j = 0; j < 13; j++)
                if (combatants[i].status == 1 && F_20a5_7900[i][j] != F_20a5_7900[j][i]) {
                    F_20a5_7900[j][i] = 1;
                    F_20a5_7900[i][j] = 1;
                }
}
#pragma optimize("a", off)

/* D_e0ac[k] = 1 for every combatant within i's weapon reach (scaled by step) */
void far sub_s014_1db7_0a80(int i, int step)
{
    int set;
    int x, y, d, k, rx, ry;

    set = cbt_anim_set(i);
    memset(D_e0ac, 0, 16);
    x = combatants[i].x;
    y = combatants[i].y;
    for (d = 0; d < 10; d++) {
        ry = D_c6f4[set][d];
        rx = D_c681[set][d];
        if (abs(rx) < abs(D_1c36[d]))
            rx = D_1c36[d];
        combatants[i].x = x + rx * step;
        if (abs(ry) < abs(D_1c40[d]))
            ry = D_1c40[d];
        combatants[i].y = y + ry * step;
        if (cbt_collisions(&combatants[i]) > 0)
            for (k = 0; k < 16; k++)
                if (D_e11c[k] != 0)
                    D_e0ac[k] = 1;
    }
    combatants[i].x = x;
    combatants[i].y = y;
}

/* stance of i attacking t */
int near sub_s014_1db7_0ba8(int i, int t, int force)
{
    int r;

    if (i < 5 && (combatants[i].f10 == 6 || combatants[i].f10 == 0x22))
        return combatants[i].f10;           /* party member keeps the ordered stance */
    if (combatants[t].f59 * 2 / 3 < combatants[i].f59 || force) {
        if (combatants[t].f4d < (combatants[i].f53 & 0xf))
            r = 0x12;
        else
            r = 10;
    } else
        r = 0x22;
    return r;
}

/* i targets t */
void near ai_set_target(int i, int t)
{
    combatants[i].target = t;
    combatants[i].tx = combatants[t].x;
    combatants[i].ty = combatants[t].y;
    if (F_20a5_787e.cost[i * 5 + t] == 0) {
        /* already adjacent */
        order_queue_clear(i);
        combatants[i].f10 = sub_s014_1db7_0ba8(i, t, 1);
        cbt_action[i] = 7;
        return;
    }
    if (path_find(i) == 1) {
        path_give(i);
        cbt_moving[i] = 1;
        D_d6fb[i] = 0;
        order_queue_clear(i);
        order_queue(i, sub_s014_1db7_0ba8(i, t, 1));
        combatants[i].f10 = 1;
        D_d97c[i] = combat_clock + 30;
    }
}

/* endurance fraction of everybody (target preference) */
void near sub_s014_1db7_0cfa(void)
{
    int i;

    for (i = 0; i < 13; i++)
        if (combatants[i].status == 1)
            F_20a5_7c4d[i] = (combatants[i].endur << 6) / combatants[i].maxend;
        else
            F_20a5_7c4d[i] = 0xff;
}

/* scheduler: every 5 ticks one party member, every D_e09a ticks a pair of enemies */
#pragma optimize("egl", off)
void near sub_s014_1db7_0d36(void)
{
    int list[4];
    int n;
    int *p;
    int k, v;

    if (real_combat == 0)
        return;
    n = 0;
    if (D_da53 < combat_clock) {
        list[n++] = D_e09c;
        D_da53 += 5;
        D_e09c = (D_e09c + 1) % 5;
    }
    if (D_da51 < combat_clock) {
        list[n] = D_e09b;
        list[n + 1] = D_e09b + 5 < 13 ? D_e09b + 5 : -1;
        n += 2;
        D_e09b = (D_e09b - 4) % 5 + 5;
        D_da51 += D_e09a;
    }
    for (p = list, k = n; k != 0; k--, p++) {
        v = *p;
        if (v != -1 && combatants[v].status == 1 && combatants[v].target != -1
         && cbt_floor[v] == party_floor && (combatants[v].f10 & 2) != 0)
            ai_rethink(v, combatants[v].target);
    }
}
#pragma optimize("egl", on)

/* re-think the target t of combatant i */
void near ai_rethink(int i, int t)
{
    int far *row;
    int j, lo, hi, w, sel, first;

    row = &F_20a5_787e.cost[i * 5];
    if (demo_mode != 0 && i == 5)
        return;
    if (combatants[t].status == 1 && F_20a5_7900[i][t] == 0 && cbt_anim[t] != 3
     && cbt_floor[t] == cbt_floor[i] && cbt_anim[i] != 5 && cbt_anim[i] != 3 && cbt_anim[i] != 0xe) {
        /* target not adjacent: walk to it */
        combatants[i].tx = combatants[t].x;
        combatants[i].ty = combatants[t].y;
        if (path_find(i) == 1) {
            path_give(i);
            cbt_moving[i] = 1;
            D_d6fb[i] = 0;
            order_queue_clear(i);
            order_queue(i, sub_s014_1db7_0ba8(i, t, 0));
            combatants[i].f10 = 1;
            D_d97c[i] = combat_clock + 30;
        }
    }
    if (combatants[t].status != 2 && (cbt_flags[t][0] & 1) != 1)
        return;
    /* target is down (or flagged): choose a new one */
    sub_s014_1db7_0028(i, i < 5 ? 5 : 0, i < 5 ? 13 : 5);
    first = index_of_min_int(row, 13);
    if (row[first] == 32000) {
        if (i < 5) {
            combatants[i].target = -1;
            combatants[i].f10 = 0;
            cbt_moving[i] = 0;
            cbt_set_anim(i, 5);
        } else
            cbt_action[i] = 0;
        return;
    }
    if (i < 5) {
        lo = 5;
        hi = 13;
    } else {
        lo = 0;
        hi = 5;
    }
    /* somebody is attacking i in melee: answer him */
    for (j = lo; j < hi; j++)
        if (combatants[j].status == 1 && combatants[j].target == i && (combatants[j].f10 & 2) != 0) {
            combatants[i].target = j;
            cbt_action[i] = 7;
            combatants[i].f10 = sub_s014_1db7_0ba8(i, j, 0);
            return;
        }
    sub_s014_1db7_0cfa();
    if (F_20a5_7c4d[index_of_min_byte(F_20a5_7c4d, 13)] <= 0x20) {
        /* weakest opponent, weighted by distance */
        for (j = 0; j < 13; j++)
            if (F_20a5_7c4d[j] != 0xff && (w = row[j]) != 32000)
                F_20a5_7c4d[j] += w / 5;
        sel = -1;
        for (;;) {
            w = index_of_min_byte(F_20a5_7c4d, 13);
            if (F_20a5_7c4d[w] == 0xff)
                break;
            sel = -1;
            for (j = lo; j < hi; j++)
                if (combatants[j].target == w && row[j] < 9999)
                    sel = j;
            if (sel != -1) {
                ai_set_target(i, sel);
                break;
            }
            F_20a5_7c4d[w] = 0xff;
        }
        if (sel != -1)
            return;
    }
    ai_set_target(i, first);
}
