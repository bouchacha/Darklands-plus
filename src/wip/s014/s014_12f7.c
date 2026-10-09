/* place: s014 12f7:000a */
/* orig: size=16b0 CONST=7d48+000e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s014 12f7: PATHFINDER.
   A "bug" algorithm with up to 10 parallel trackers (all state in a far work area addressed
   by _based(D_d6f7) pointers):
     tracker k: state D_e0f0[k] (0 dead, 1 heading straight for the goal, 2 following a wall,
     |0x80 = skip the current round), position D_e0f6/D_e0f8[k], previous D_e0f2/D_e0f4[k],
     goal D_e0fa/D_e0fc[k], facing D_e0fe[k], sprite D_e100[k], wall side D_e0ec[k]
     (0 = wall on one side, 1 = the other), turn balance D_e0ee[k], steps
     D_e0ea[k*250 + n] (direction 0..9 per step), and up to 40 waypoints (where it changed
     direction) D_e102/D_e104/D_e106/D_e112[k*40 + w], count D_e108[k], first own one D_e114[k].
   03ea runs all trackers one step per round (at most 250 rounds). A straight tracker that
   hits an obstacle (0ab4) probes the 4 neighbouring directions, turns into a wall follower
   and spawns a second follower for the other side (tables D_1900/D_19a0). A follower
   (03ea body) prefers to turn back towards the wall side, then straight, then away; it goes
   back to straight mode as soon as the direct step towards the goal is free. Trackers die
   when they loop (098e: revisiting a waypoint), make > 6 net turns or exceed 40 waypoints.
   The first tracker to reach the goal (1094) wins (0ffe): D_e116 = steps, D_84e6 = tracker.
   000a: path for combatant i to its destination (+8/+0a); enemies first open the closed
   doors (vec_1d6f), restored by 034a. The result goes to F_20a5_71ba (step directions,
   0xff end) with waypoints F_20a5_72b4/7304/716a; 1166 then shortcuts between waypoints.
   1554/15d0 copy a path to the combatant's own path F_20a5_6238[i] (161a adds the facing
   in the high nibble and removes single-step facing jitter).
   status: 18/18 written, 10 match. */

#pragma optimize("w", on)
struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    int tx;                     /* +08 destination */
    int ty;                     /* +0a */
    char padc[0xd];
    char facing;                /* +19 */
    char pad1a[2];
    char sprite;                /* +1c */
    char pad1d[0x2d];
    char target;                /* +4a */
    char pad4b[0x35];
};

extern _segment D_d6f7;                 /* segment of the pathfinder work area */
extern char _based(D_d6f7) *D_e0ea;     /* [10][250] steps */
extern char _based(D_d6f7) *D_e0ec;     /* [10] wall side */
extern char _based(D_d6f7) *D_e0ee;     /* [10] turn balance */
extern unsigned char _based(D_d6f7) *D_e0f0; /* [10] state */
extern int _based(D_d6f7) *D_e0f2;      /* [10] previous x */
extern int _based(D_d6f7) *D_e0f4;      /* [10] previous y */
extern int _based(D_d6f7) *D_e0f6;      /* [10] x */
extern int _based(D_d6f7) *D_e0f8;      /* [10] y */
extern int _based(D_d6f7) *D_e0fa;      /* [10] goal x */
extern int _based(D_d6f7) *D_e0fc;      /* [10] goal y */
extern char _based(D_d6f7) *D_e0fe;     /* [10] facing */
extern char _based(D_d6f7) *D_e100;     /* [10] sprite */
extern int _based(D_d6f7) *D_e102;      /* [10][40] waypoint x */
extern int _based(D_d6f7) *D_e104;      /* [10][40] waypoint y */
extern int _based(D_d6f7) *D_e106;      /* [10][40] waypoint step */
extern char _based(D_d6f7) *D_e108;     /* [10] waypoint count */
extern char _based(D_d6f7) *D_e10a;     /* best shortcut: steps */
extern int _based(D_d6f7) *D_e10c;      /* best shortcut: waypoint steps */
extern int _based(D_d6f7) *D_e10e;      /* best shortcut: waypoint x */
extern int _based(D_d6f7) *D_e110;      /* best shortcut: waypoint y */
extern unsigned char _based(D_d6f7) *D_e112; /* [10][40] waypoint wall side */
extern int _based(D_d6f7) *D_e114;      /* [10] first own waypoint */

extern char far F_20a5_71ba[250];       /* resulting path (steps, 0xff end) */
extern int far F_20a5_716a[40];         /* its waypoints: step */
extern int far F_20a5_72b4[40];         /* x */
extern int far F_20a5_7304[40];         /* y */
extern char far F_20a5_6238[13][250];   /* path of every combatant */
extern char far F_20a5_6331[];          /* alias: last byte of F_20a5_6238[0] */
extern char far F_20a5_33d3[40];        /* door states of the floor */
extern char far F_20a5_1f52;            /* number of doors */
extern struct door { int f0; char f2[7]; char closed; char pad[4]; } far F_20a5_1f54[]; /* 14 bytes */
extern char far F_2e38_0c00[40];        /* door locked flags */
extern char far F_2e38_0c64[250];       /* saved path */
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

void far sub_root_05f8_0117(int, int, int);    /* mouse cursor shape */
int far sub_root_0410_0038(int);               /* sign */
int far sub_root_0410_02ca(char far *, int, int); /* index of a byte */
void far *far _fmemcpy(void far *, const void far *, unsigned);
void far *far _fmemset(void far *, int, unsigned);
int abs(int);
void far movedata(unsigned, unsigned, unsigned, unsigned, unsigned);
int far sub_s014_1462_000a(struct cbt *);
int far sub_s014_14f7_343c(int);
void far sub_s014_1db7_0158(int);
void far vec_1d6f(int, int, int);       /* s023: set door state */
void far sub_s014_12f7_034a(int);
void far sub_s014_12f7_03b6(void);
void far sub_s014_12f7_03d2(void);
int far sub_s014_12f7_03ea(struct cbt *);
void near sub_s014_12f7_098e(int, int);
int far sub_s014_12f7_0ab4(int, struct cbt *, struct cbt *, int *, int, int, int *, int *, int);
int far sub_s014_12f7_0ffe(int, int *);
int far sub_s014_12f7_1094(int);
int far sub_s014_12f7_1166(struct cbt *);
int far sub_s014_12f7_147c(int);
void far sub_s014_12f7_1494(struct cbt *, int);
void far sub_s014_12f7_14f4(int, struct cbt *);
void near sub_s014_12f7_161a(int, int);
void far sub_s014_12f7_16a4(struct cbt *, struct cbt *);

extern struct cbt D_9c55[];
extern struct cbt *D_114c;              /* scratch record */
extern char D_a6b4[];                   /* [13] floor */
extern char D_e0a2;                     /* pathfinding active (formation collision rules) */
extern int D_e118;                      /* length of the last path */
extern int D_e116;                      /* steps of the winning tracker */
extern int D_e11a;                      /* number of waypoints */
extern int D_e0bc, D_e0be;              /* start position */
extern char D_d6cd;                     /* target of the searching combatant */
extern char D_d697;                     /* ignore non-target combatants */
extern char D_07d3;                     /* busy counter */
extern int D_18c2;                      /* cursor busy count */
extern int D_940b;
extern int D_84e6;                      /* winning tracker */
extern int D_84dc, D_84de;              /* its end position */
extern int D_84e0, D_84e2, D_84e4;      /* best shortcut: length, end x, end y */
extern char D_e0c0[40];                 /* saved door states */
extern int D_e11c[];                    /* collision result per combatant */
extern int D_da43;                      /* saved path length */
extern char D_c681[][10];               /* step dx per sprite set and direction */
extern char D_c6f4[][10];               /* step dy */
extern char D_c80f[];                   /* half width */
extern char D_c831[];                   /* half height */
extern char D_18c4[4][4];               /* [quadrant][slope] -> direction */
extern char D_18d4[][2];                /* [dir][side] turn away from the wall */
extern char D_18e2[][2];                /* [dir][side] turn towards the wall */
extern char D_18f0[];                   /* [dir] straight on */
extern char D_18f8[];                   /* [dir] reverse */
extern char D_1900[][16];               /* [dir][blocked mask] first follower direction */
extern char D_19a0[][16];               /* [dir][blocked mask] second follower direction */

#define RX(s, d) D_c681[s][d]
#define RY(s, d) D_c6f4[s][d]

/* path for combatant i to its destination: 1 found, 2 not */
int far sub_s014_12f7_000a(int i)
{
    struct cbt *c;
    int r;
    int k, m, n;

    D_e0a2 = 1;
    D_e118 = 9999;
    c = &D_9c55[i];
    sub_s014_12f7_16a4(c, D_114c);
    D_e0bc = D_114c->x;
    D_e0be = D_114c->y;
    D_114c->x = D_114c->tx;
    D_114c->y = D_114c->ty;
    D_07d3++;
    D_d6cd = D_9c55[i].target;
    if (D_d6cd != -1 && D_a6b4[D_d6cd] != D_a6b4[i]) {
        D_07d3--;
        D_e0a2 = 0;
        return 2;
    }
    sub_s014_12f7_03b6();
    D_d697 = 1;
    r = sub_s014_1462_000a(D_114c);
    D_d697 = 0;
    if (r != 0 && sub_s014_12f7_147c(D_9c55[i].target) == 0) {
        r = 2;                          /* destination blocked */
        goto done;
    }
    if (i >= 5) {
        /* enemies can walk through closed (unlocked) doors */
        _fmemcpy(D_e0c0, F_20a5_33d3, 40);
        for (k = 0; k < F_20a5_1f52; k++)
            if (F_2e38_0c00[k] == 0 && F_20a5_1f54[k].closed == 0 && F_20a5_1f54[k].f0 > 0)
                vec_1d6f(k, 1, 0);
    }
    sub_s014_12f7_16a4(c, D_114c);
    r = sub_s014_12f7_03ea(D_114c) & 0x7fff;
    if (D_e116 == 0) {
        D_e0a2 = 0;
        D_07d3--;
        D_e118 = 1;
        F_20a5_71ba[0] = -1;
        goto out;
    }
    if (r == 1) {
        _fmemcpy(F_20a5_71ba, D_e0ea + D_84e6 * 250, D_e116 + 1);
        _fmemcpy(F_20a5_72b4, D_e102 + D_84e6 * 40, D_e108[D_84e6] * 2);
        _fmemcpy(F_20a5_7304, D_e104 + D_84e6 * 40, D_e108[D_84e6] * 2);
        _fmemcpy(F_20a5_716a, D_e106 + D_84e6 * 40, D_e108[D_84e6] * 2);
        D_e118 = D_e116;
        D_e11a = D_e108[D_84e6];
        /* drop waypoints where the direction did not really change */
        for (k = 0; k < D_e11a - 1; k++)
            if (F_20a5_71ba[F_20a5_716a[k] + 1] == F_20a5_71ba[F_20a5_716a[k]]) {
                for (m = k, n = D_e11a - k - 1; k < D_e11a - 1 && n != 0; n--, m++) {
                    F_20a5_716a[m] = F_20a5_716a[m + 1];
                    F_20a5_72b4[m] = F_20a5_72b4[m + 1];
                    F_20a5_7304[m] = F_20a5_7304[m + 1];
                }
                D_e11a--;
            }
        sub_s014_1db7_0158(i);
        sub_s014_12f7_16a4(c, D_114c);
        r = sub_s014_12f7_1166(D_114c);
        if (r == 1) {
            D_e118 = sub_root_0410_02ca(F_20a5_71ba, -1, 250);
            F_20a5_71ba[249] = -1;
        }
    }
done:
    D_07d3--;
    D_e0a2 = 0;
out:
    sub_s014_12f7_034a(i);
    sub_root_05f8_0117(0, 0, D_940b);
    D_18c2 = 0;
    return r;
}

/* restore the doors opened for an enemy's path search */
void far sub_s014_12f7_034a(int i)
{
    int k;

    if (i > 4) {
        for (k = 0; k < F_20a5_1f52; k++)
            if (F_2e38_0c00[k] == 0 && F_20a5_1f54[k].f0 > 0)
                vec_1d6f(k, D_e0c0[k], 0);
        _fmemcpy(F_20a5_33d3, D_e0c0, 40);
    }
}

/* busy cursor on */
void far sub_s014_12f7_03b6(void)
{
    if (++D_18c2 == 1)
        sub_root_05f8_0117(0, 8, D_940b);
}

/* busy cursor off */
void far sub_s014_12f7_03d2(void)
{
    if (--D_18c2 == 0)
        sub_root_05f8_0117(0, 0, D_940b);
}

/* run the trackers from rec's position to rec->tx/ty: 1 found (D_e116, D_84e6), 2 not */
int far sub_s014_12f7_03ea(struct cbt *rec)
{
    struct cbt tmp;
    struct cbt save;
    int set;
    int side;
    int step;
    int nd;
    int t16, t250;
    int res;
    int dx, dy, adx, ady, ratio, slope, quad, dir;
    int k, k2, last, s, n;

    res = 2;
    sub_s014_12f7_03b6();
    sub_s014_12f7_1494(rec, 0);
    set = sub_s014_14f7_343c(rec->sprite);
    _fmemset((char far *)D_e108, 0, 10);
    _fmemset((char far *)D_e0f0, 0, 10);
    D_e0f0[0] = 1;
    D_e0f2[0] = D_e0f6[0];
    D_e0f4[0] = D_e0f8[0];
    D_84e6 = -1;
    D_e114[0] = 0;
    for (step = 0; step < 250; step++) {
        for (k = 0, t16 = 0, t250 = 0, k2 = 0; k2 < 20; k++, k2 += 2, t16 += 40, t250 += 250) {
            if (D_e0f0[k] == 0)
                continue;
            if ((D_e0f0[k] & 0x80) == 0x80) {
                D_e0f0[k] &= 0x7f;
                continue;
            }
            /* direction towards the goal: quadrant and slope class */
            dy = D_e0fc[k] - D_e0f8[k];
            dx = D_e0fa[k] - D_e0f6[k];
            adx = abs(dx);
            ady = abs(dy);
            ratio = adx == 0 ? 20 : ady * 4 / adx;
            if (ratio == 0)
                slope = 0;
            else if (ratio >= 1 && ratio <= 2)
                slope = 1;
            else if (ratio >= 3 && ratio <= 6)
                slope = 2;
            else
                slope = 3;
            if (dx >= 0 && dy < 0)
                quad = 0;
            else if (dx >= 0 && dy >= 0)
                quad = 1;
            else if (dx < 0 && dy >= 0)
                quad = 2;
            else
                quad = 3;
            dir = D_18c4[quad][slope];
            if (D_e0f0[k] == 1) {
                res = sub_s014_12f7_0ab4(k, &tmp, &save, &step, dir, rec->target, &side, &nd, set);
                if (res == -1)
                    res = 2;
                else if (res == 1)
                    break;
                continue;
            }
            /* wall follower */
            if (sub_s014_12f7_1094(k) == 1)
                goto found;
            last = D_e0ea[t250 + step - 1];
            s = D_e0ec[k];
            sub_s014_12f7_14f4(k, &tmp);
            if (D_e0ee[k] >= 1) {
                /* try the direct step towards the goal */
                tmp.x += RX(set, dir);
                tmp.y += RY(set, dir);
                tmp.facing = dir;
                if (sub_s014_1462_000a(&tmp) == 0) {
                    D_e0f0[k] = 1;
                    nd = dir;
                    goto moved;
                }
                if (sub_s014_12f7_147c(rec->target) != 0)
                    goto found;
            }
            if (D_e0ee[k] < 1) {
                /* turn towards the wall */
                sub_s014_12f7_14f4(k, &tmp);
                tmp.facing = nd = D_18e2[last][s];
                tmp.x += RX(set, nd);
                tmp.y += RY(set, nd);
                if (sub_s014_1462_000a(&tmp) == 0) {
                    D_e0ee[k]++;
                    goto moved;
                }
                if (sub_s014_12f7_147c(rec->target) != 0)
                    goto found;
            }
            /* straight on */
            sub_s014_12f7_14f4(k, &tmp);
            tmp.facing = nd = D_18f0[last];
            tmp.x += RX(set, nd);
            tmp.y += RY(set, nd);
            if (sub_s014_1462_000a(&tmp) != 0) {
                if (sub_s014_12f7_147c(rec->target) != 0)
                    goto found;
                /* turn away from the wall */
                sub_s014_12f7_14f4(k, &tmp);
                tmp.facing = nd = D_18d4[last][s];
                tmp.x += RX(set, nd);
                tmp.y += RY(set, nd);
                if (sub_s014_1462_000a(&tmp) == 0)
                    D_e0ee[k]--;
                else {
                    if (sub_s014_12f7_147c(rec->target) != 0)
                        goto found;
                    /* reverse */
                    sub_s014_12f7_14f4(k, &tmp);
                    tmp.facing = nd = D_18f8[last];
                    tmp.x += RX(set, nd);
                    tmp.y += RY(set, nd);
                    if (sub_s014_1462_000a(&tmp) == 0)
                        D_e0ee[k] -= 2;
                    else if (sub_s014_12f7_147c(rec->target) != 0)
                        goto found;
                }
            }
moved:
            D_e0f2[k] = D_e0f6[k];
            D_e0f4[k] = D_e0f8[k];
            if (nd != last) {
                /* direction changed: new waypoint */
                n = D_e108[k] + t16;
                D_e106[n] = step - 1;
                D_e112[n] = D_e0ec[k];
                D_e102[n] = D_e0f6[k];
                D_e104[n] = D_e0f8[k];
                D_e108[k]++;
                sub_s014_12f7_098e(k, D_e108[k] - 1);
                if (D_e108[k] >= 40) {
                    D_e0f0[k] = 0;
                    continue;
                }
            }
            D_e0ea[t250 + step] = nd;
            sub_s014_12f7_1494(&tmp, k);
            if (abs(D_e0ee[k]) >= 7)
                D_e0f0[k] = 0;          /* spiralling */
            continue;
found:
            res = sub_s014_12f7_0ffe(k, &step);
            break;
        }
    }
    D_d697 = 0;
    sub_s014_12f7_03d2();
    return res;
}

/* kill tracker k if its new waypoint w repeats a waypoint of any tracker (loop) */
void near sub_s014_12f7_098e(int k, int w)
{
    int x, y, m, j, lim;

    x = D_e102[k * 40 + w];
    y = D_e104[k * 40 + w];
    for (m = 0; m < 10; m++) {
        if (D_e0f0[m] == 0)
            continue;
        lim = m == k ? w - 1 : D_e108[m];
        for (j = D_e114[m]; j < lim; j++)
            if (D_e0ec[k] == D_e112[m * 40 + j]
             && x - 1 <= D_e102[m * 40 + j] && D_e102[m * 40 + j] <= x + 1
             && y - 1 <= D_e104[m * 40 + j] && D_e104[m * 40 + j] <= y + 1) {
                D_e108[k] = 42;
                return;
            }
    }
}

/* one step of straight tracker k in direction dir; on an obstacle split into two wall
   followers. Returns 2 (go on), 1 (goal reached) or -1 (out of trackers / waypoints). */
int far sub_s014_12f7_0ab4(int k, struct cbt *tmp, struct cbt *save, int *step, int dir,
                           int target, int *mask, int *nd, int set)
{
    int s, n, j, idx, w;

    if (sub_s014_12f7_1094(k) == 1)
        goto found;
    sub_s014_12f7_14f4(k, tmp);
    tmp->x += RX(set, dir);
    tmp->y += RY(set, dir);
    tmp->facing = dir;
    if (sub_s014_1462_000a(tmp) == 0) {
        D_e0f2[k] = D_e0f6[k];
        D_e0f4[k] = D_e0f8[k];
        D_e0ea[k * 250 + *step] = dir;
        sub_s014_12f7_1494(tmp, k);
        return 2;
    }
    if (sub_s014_12f7_147c(target) != 0)
        goto found;
    /* which of the directions 1, 2, 5, 6 are blocked */
    *mask = 0;
    *nd = 1;
    sub_s014_12f7_14f4(k, tmp);
    tmp->x += RX(set, 1);
    tmp->y += RY(set, 1);
    tmp->facing = 1;
    if (sub_s014_1462_000a(tmp) != 0) {
        if (sub_s014_12f7_147c(target) != 0)
            goto found;
        *mask = 1;
    }
    *nd = 2;
    sub_s014_12f7_14f4(k, tmp);
    tmp->x += RX(set, 2);
    tmp->y += RY(set, 2);
    tmp->facing = 2;
    if (sub_s014_1462_000a(tmp) != 0) {
        if (sub_s014_12f7_147c(target) != 0)
            goto found;
        *mask += 2;
    }
    *nd = 5;
    sub_s014_12f7_14f4(k, tmp);
    tmp->x += RX(set, 5);
    tmp->y += RY(set, 5);
    tmp->facing = 5;
    if (sub_s014_1462_000a(tmp) != 0) {
        if (sub_s014_12f7_147c(target) != 0)
            goto found;
        *mask += 4;
    }
    *nd = 6;
    sub_s014_12f7_14f4(k, tmp);
    tmp->x += RX(set, 6);
    tmp->y += RY(set, 6);
    tmp->facing = 6;
    if (sub_s014_1462_000a(tmp) != 0) {
        if (sub_s014_12f7_147c(target) != 0)
            goto found;
        *mask += 8;
    }
    idx = dir * 16 + *mask;
    *nd = D_1900[0][idx];
    if (*nd == -1)
        D_e0f0[k] = 0;
    else {
        /* this tracker follows the wall on side 0 */
        D_e0ec[k] = 0;
        D_e0ee[k] = 0;
        D_e0f0[k] = 2;
        s = *step;
        if (s != 0) {
            w = D_e108[k] + k * 40;
            D_e106[w] = s - 1;
            D_e112[w] = 0;
            D_e102[w] = D_e0f6[k];
            D_e104[w] = D_e0f8[k];
            D_e108[k]++;
            sub_s014_12f7_098e(k, D_e108[k] - 1);
            if (D_e108[k] > 39) {
                D_e0f0[k] = 0;
                return -1;
            }
        }
        sub_s014_12f7_14f4(k, save);
        D_e0f6[k] += RX(set, *nd);
        D_e0f8[k] += RY(set, *nd);
        D_e0ea[k * 250 + s] = *nd;
        D_e0f2[k] = D_e0f6[k];
        D_e0f4[k] = D_e0f8[k];
    }
    *nd = D_19a0[0][idx];
    if (*nd == -1)
        return 2;
    /* spawn a tracker following the wall on side 1 */
    for (n = 0; n < 10; n++)
        if (D_e0f0[n] == 0)
            break;
    if (n > 9)
        return -1;
    D_e0f0[n] = 2;
    if (k < n)
        D_e0f0[n] = 0x82;
    D_e0ec[n] = 1;
    s = *step;
    for (j = 0; j < s; j++)
        D_e0ea[n * 250 + j] = D_e0ea[k * 250 + j];
    for (j = 0; j < D_e108[k]; j++) {
        D_e102[n * 40 + j] = D_e102[k * 40 + j];
        D_e104[n * 40 + j] = D_e104[k * 40 + j];
        D_e106[n * 40 + j] = D_e106[k * 40 + j];
        D_e112[n * 40 + j] = D_e112[k * 40 + j];
    }
    D_e108[n] = D_e108[k];
    D_e114[n] = D_e108[n];
    D_e0ee[n] = 0;
    sub_s014_12f7_1494(save, n);
    D_e0f6[n] += RX(set, *nd);
    D_e0f8[n] += RY(set, *nd);
    D_e0ea[n * 250 + s] = *nd;
    D_e0f2[n] = D_e0f6[n];
    D_e0f4[n] = D_e0f8[n];
    return 2;
found:
    return sub_s014_12f7_0ffe(k, step);
}

/* tracker k reached the goal */
int far sub_s014_12f7_0ffe(int k, int *step)
{
    int w;

    D_e116 = *step;
    w = D_e108[k] + k * 40;
    D_e106[w] = D_e116;
    D_e0ea[k * 250 + D_e116] = -1;
    D_e102[w] = D_84dc = D_e0f6[k];
    D_e104[w] = D_84de = D_e0f8[k];
    D_e108[k]++;
    D_84e6 = k;
    *step = 260;                        /* ends the search loop */
    return 1;
}

/* 1 if tracker k is at (or has just passed) its goal */
int far sub_s014_12f7_1094(int k)
{
    int gx, px, gy, py, dx, dy, adx, ady;

    gx = D_e0fa[k];
    px = D_e0f2[k];
    gy = D_e0fc[k];
    py = D_e0f4[k];
    dx = gx - D_e0f6[k];
    dy = gy - D_e0f8[k];
    adx = abs(dx);
    ady = abs(dy);
    if (adx > 2 || ady > 2) {
        if (sub_root_0410_0038(dx) == sub_root_0410_0038(gx - px)
         && sub_root_0410_0038(dy) == sub_root_0410_0038(gy - py))
            return 0;
        if (adx >= D_c80f[k] || ady >= D_c831[k])
            return 0;
    }
    return 1;
}

/* shorten the path F_20a5_71ba by trying direct routes to later waypoints */
int far sub_s014_12f7_1166(struct cbt *rec)
{
    int len;
    int base;
    int a0;
    int a, m, d;
    int done;

    D_84e2 = -1;
    base = 0;
    done = 0;
    len = 0;
    a0 = 1;
    do {
        if (a0 >= D_e11a)
            break;
        for (a = a0; a < D_e11a; a++) {
            rec->tx = F_20a5_72b4[a];
            rec->ty = F_20a5_7304[a];
            if (sub_s014_12f7_03ea(rec) == 2)
                return 2;
            if (D_e116 + base < F_20a5_716a[a]) {
                /* direct route to waypoint a is shorter: remember it */
                _fmemcpy(D_e10a, D_e0ea + D_84e6 * 250, D_e116 + 1);
                _fmemcpy(D_e10e, D_e102 + D_84e6 * 40, D_e108[D_84e6] * 2);
                _fmemcpy(D_e110, D_e104 + D_84e6 * 40, D_e108[D_84e6] * 2);
                _fmemcpy(D_e10c, D_e106 + D_84e6 * 40, D_e108[D_84e6] * 2);
                D_84e2 = D_84dc;
                D_84e4 = D_84de;
                D_84e0 = D_e116;
                d = D_e116 - F_20a5_716a[a];
                for (m = a; m < D_e11a; m++)
                    F_20a5_716a[m] -= -base - d;
            } else if (D_84e2 != -1) {
                /* take the best shortcut found so far */
                _fmemcpy(F_20a5_71ba + len, D_e10a, D_84e0);
                len += D_84e0;
                rec->x = D_84e2;
                rec->y = D_84e4;
                base = F_20a5_716a[a - 1];
                a0 = a + 1;
                D_84e2 = -1;
                if (D_e11a == a)
                    done = 1;
                break;
            }
        }
    } while (a < D_e11a);
    if (!done) {
        if (a0 < 2) {
            if (sub_s014_12f7_03ea(rec) == 2 || D_e116 + len + 1 > 249)
                return 2;
            rec->tx = F_20a5_72b4[D_e11a - 1];
            rec->ty = F_20a5_7304[D_e11a - 1];
            _fmemcpy(F_20a5_71ba + len, D_e0ea + D_84e6 * 250, D_e116);
            len += D_e116;
        } else {
            for (a = a0 - 1; a < D_e11a; a++) {
                rec->tx = F_20a5_72b4[a];
                rec->ty = F_20a5_7304[a];
                if (sub_s014_12f7_03ea(rec) == 2)
                    return 2;
                if (D_e116 + len + 1 > 249)
                    return 2;
                _fmemcpy(F_20a5_71ba + len, D_e0ea + D_84e6 * 250, D_e116);
                len += D_e116;
                rec->x = D_84dc;
                rec->y = D_84de;
            }
        }
    }
    if (len != 0)
        F_20a5_71ba[len] = -1;
    return 1;
}

/* is combatant k (-1 none) among the colliding ones */
int far sub_s014_12f7_147c(int k)
{
    if (k == -1)
        return 0;
    return D_e11c[k];
}

/* tracker k := position/goal/facing/sprite of rec */
void far sub_s014_12f7_1494(struct cbt *rec, int k)
{
    D_e0f6[k] = rec->x;
    D_e0f8[k] = rec->y;
    D_e0fa[k] = rec->tx;
    D_e0fc[k] = rec->ty;
    D_e0fe[k] = rec->facing;
    D_e100[k] = rec->sprite;
}

/* rec := tracker k */
void far sub_s014_12f7_14f4(int k, struct cbt *rec)
{
    rec->x = D_e0f6[k];
    rec->y = D_e0f8[k];
    rec->tx = D_e0fa[k];
    rec->ty = D_e0fc[k];
    rec->facing = D_e0fe[k];
    rec->sprite = D_e100[k];
}

/* give combatant i the last found path */
void far sub_s014_12f7_1554(int i)
{
    char far *d;
    char far *s;

    d = F_20a5_6238[i];
    s = F_20a5_71ba;
    movedata(FP_SEG(s), FP_OFF(s), FP_SEG(d), FP_OFF(d), D_e118 + 1);
    sub_s014_12f7_161a(i, D_e118);
}

/* save the last found path */
void far sub_s014_12f7_159c(void)
{
    char far *d;
    char far *s;

    d = F_2e38_0c64;
    s = F_20a5_71ba;
    D_da43 = D_e118;
    movedata(FP_SEG(s), FP_OFF(s), FP_SEG(d), FP_OFF(d), D_e118 + 1);
}

/* give combatant i the saved path */
void far sub_s014_12f7_15d0(int i)
{
    int n;
    char far *d;
    char far *s;

    s = F_2e38_0c64;
    d = F_20a5_6238[i];
    n = D_da43;
    movedata(FP_SEG(s), FP_OFF(s), FP_SEG(d), FP_OFF(d), D_da43 + 1);
    sub_s014_12f7_161a(i, n);
}

/* step byte = direction | facing << 4; drop one-step facing changes */
void near sub_s014_12f7_161a(int i, int n)
{
    char far *p;
    int k;

    F_20a5_6331[i * 250] = -1;         /* = F_20a5_6238[i][249] */
    p = F_20a5_6238[i];
    for (k = 0; p[k] != -1; k++)
        p[k] |= p[k] << 4;
    if (n > 1)
        for (k = 0; k < 0xf8; k++) {
            if (p[k + 2] == -1)
                return;
            if (((p[k + 2] ^ p[k]) & 0xf0) == 0 && ((p[k + 1] ^ p[k]) & 0xf0) != 0)
                p[k + 1] = (p[k + 1] ^ p[k]) & 0xf ^ p[k];
        }
}

/* *dst = *src (combatant record copy) */
void far sub_s014_12f7_16a4(struct cbt *src, struct cbt *dst)
{
    *dst = *src;
}
