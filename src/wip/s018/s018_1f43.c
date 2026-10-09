/* place: s018 1f43:0006 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 2/6 (0fc2, 104c match); 0256 NOT written yet (stub).
 * 0f02: frame one slot smaller than the original; 0c8a/0006: home-store/slot differences.
 * Enemy AI helpers (overlay s018, child of s014):
 * 0006 which party members are seen by an enemy (fills seen[5], returns count),
 * 0256 enemy turn decision, 0c8a pick a free tile next to a target (random offsets
 * D_214e/D_215e) or fall back to party/enemy positions, 0f02 distance table from enemy n to
 * each visible party member (F_20a5 dist[n][5]) and the nearest one into D_e1f9[n],
 * 0fc2 refresh a party member's animation frame counts, 104c do that for flagged members. */

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
    char pad17[0x33];
    char f4a;       /* +4a target index (-1 none) */
    char pad4b[0x35];
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
extern struct far20a5 far F_20a5_0000;

extern struct cmb D_9c55[];
extern int D_e0a3, D_e0a5, D_e0a7, D_e0a9;
extern char D_11ca;
extern unsigned char D_d6f4;
extern char D_a6b4[];
extern char D_c4fd;
extern struct { int f0; char pad[0x10]; } D_a53b[];
extern char D_17b2[4][3];
extern char D_d5c6[];
extern int D_20ee[][6];
extern char D_214e[], D_215e[];
extern char D_d6fb[], D_a6d3[];
extern int D_d97c[];
extern int D_07d4;
extern struct near2 D_e1f9[];
extern int D_a696[];
extern char D_c80f[], D_c831[];
extern char D_119a[], D_11a6[], D_1182[], D_118e[];
extern char D_e052[], D_e05f[], D_e06c[];

void *memset(void *, int, unsigned);
int abs(int);
int far random_n(int);
int far sub_s014_1d69_000c(int, int, int);
int far sub_s014_12f7_000a(int);
void far sub_s014_12f7_1554(int);
int far sub_root_047c_1120(int, int, int);
int far sub_s014_14f7_343c(int);
int far sub_s014_1462_000a(struct cmb *);
int far sub_s018_1f43_0006(int *);
void far sub_s018_1f43_0256(void);
void far sub_s018_1f43_0c8a(int, struct tgt *);
void far sub_s018_1f43_0f02(int, int *);
void far sub_s018_1f43_0fc2(int);
void far sub_s018_1f43_104c(void);

int far sub_s018_1f43_0006(int *seen)
{
    int a, b, c, dy, e, p, k, d, ok, cnt, alive, n, i;

    memset(seen, 0, 10);
    cnt = 0;
    alive = 0;
    for (i = 0; i < 5; i++)
        if (D_9c55[i].status == 1)
            alive++;
    if (D_d6f4 == 0x63) {
        n = 0;
        for (i = 0; i < 5; i++)
            if (D_9c55[i].status == 1) {
                seen[i] = 1;
                n++;
            }
        D_d6f4 = 0;
        return n;
    }
    for (e = 5; e < 13; e++) {
        if (D_9c55[e].status == 1 && D_a53b[e].f0 == 0) {
            for (p = 0; p < 5; p++) {
                if (seen[p] == 0 && D_9c55[p].status == 1 && D_a6b4[p] == D_11ca) {
                    c = (D_9c55[p].x - D_9c55[e].x) / 2;
                    dy = D_9c55[p].y - D_9c55[e].y;
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
                            if (D_20ee[D_9c55[e].kind][k] == b) {
                                ok = 1;
                                break;
                            }
                    }
                    if (ok == 1) {
                        D_e0a5 = D_9c55[e].x;
                        D_e0a3 = D_9c55[e].y;
                        D_e0a9 = D_9c55[p].x;
                        D_e0a7 = D_9c55[p].y;
                        if (sub_s014_1d69_000c(-1, 0, 0) == 0) {
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

void far sub_s018_1f43_0256(void)
{
}

void far sub_s018_1f43_0c8a(int who, struct tgt *list)
{
    int k, found, tries, t;

    found = 0;
    for (tries = 0; tries < 50; tries++) {
        k = random_n(16);
        D_e0a5 = D_214e[k] + D_9c55[who].x;
        D_e0a3 = D_215e[k] + D_9c55[who].y;
        for (k = 0; k < 5; k++)
            if (list[k].dist != 0x7d00) {
                t = list[k].idx;
                D_e0a9 = D_9c55[t].x;
                D_e0a7 = D_9c55[t].y;
                if (sub_s014_1d69_000c(who, t, 0x80) == 0)
                    k = 100;
            }
        if (k >= 100) {
            D_9c55[who].f08 = D_e0a5;
            D_9c55[who].f0a = D_e0a3;
            if (sub_s014_12f7_000a(who) == 1) {
                sub_s014_12f7_1554(who);
                D_9c55[who].f10 = 1;
                D_d6fb[who] = 0;
                D_a6d3[who] = 1;
                D_d97c[who] = random_n(90) + D_07d4 + 30;
                found = 1;
                tries = 100;
            }
        }
    }
    if (found == 0) {
        for (k = 0; k < 5; k++)
            if (list[k].dist != 0x7d00) {
                t = list[k].idx;
                D_9c55[who].f08 = D_9c55[t].x;
                D_9c55[who].f0a = D_9c55[t].y;
                D_9c55[who].f4a = k;
                if (sub_s014_12f7_000a(who) == 1) {
                    sub_s014_12f7_1554(who);
                    F_20a5_0000.rec[who].f0 = -1;
                    D_9c55[who].f10 = 1;
                    D_d6fb[who] = 0;
                    D_a6d3[who] = 1;
                    D_d97c[who] = random_n(90) + D_07d4 + 30;
                    found = 1;
                    D_9c55[who].f4a = -1;
                    break;
                }
            }
    }
    if (found == 0) {
        for (k = 5; k < 13; k++)
            if (D_9c55[k].status == 1 && who != k) {
                D_9c55[who].f08 = D_9c55[k].x;
                D_9c55[who].f0a = D_9c55[k].y;
                D_9c55[who].f4a = k;
                if (sub_s014_12f7_000a(who) == 1) {
                    sub_s014_12f7_1554(who);
                    F_20a5_0000.rec[who].f0 = -1;
                    D_9c55[who].f10 = 1;
                    D_d6fb[who] = 0;
                    D_a6d3[who] = 1;
                    D_d97c[who] = random_n(90) + D_07d4 + 30;
                    found = 1;
                    D_9c55[who].f4a = -1;
                    break;
                }
            }
    }
    if (found == 0)
        D_9c55[who].f4a = -1;
}

void far sub_s018_1f43_0f02(int n, int *flags)
{
    int i, d, x, y, best;

    best = 0x7d00;
    if (D_9c55[n].status == 1 && D_9c55[n].f4a == -1) {
        x = D_9c55[n].x;
        y = D_9c55[n].y;
        for (i = 0; i < 5; i++)
            if (D_9c55[i].status == 1 && flags[i] == 1) {
                d = F_20a5_0000.dist[n][i] = sub_root_047c_1120((D_9c55[i].x - x) / 2, D_9c55[i].y - y, 0);
                if (d < best)
                    best = d;
            } else
                F_20a5_0000.dist[n][i] = 0x7d00;
    }
    D_e1f9[n].dist = best;
    D_e1f9[n].who = n;
}

void far sub_s018_1f43_0fc2(int i)
{
    int k;

    if (D_9c55[i].status == 1) {
        D_a696[i] |= 0x2000;
        k = sub_s014_14f7_343c(i);
        D_c80f[i] = D_119a[k];
        D_c831[i] = D_11a6[k];
        D_e06c[i] = D_e05f[i] = D_e052[i] = 0;
        if (sub_s014_1462_000a(&D_9c55[i])) {
            D_c80f[i] = D_1182[k];
            D_c831[i] = D_118e[k];
            D_a696[i] &= ~0x4000;
        } else
            D_a696[i] |= 0x4000;
    }
}

void far sub_s018_1f43_104c(void)
{
    int i;

    for (i = 0; i < 13; i++)
        if (D_9c55[i].status == 1 && (D_a696[i] & 0x2000) == 0x2000 && !(D_a696[i] & 0x4000))
            sub_s018_1f43_0fc2(i);
}
