/* pathfix.c - better pathfinding in battles and dungeons.
 *
 * The game plans every move with path_find (s014 12f7:000a): up to ten "bug" trackers that
 * head straight for the goal and follow walls when blocked, with hard limits (250 steps,
 * 40 turns, 10 trackers). A tracker leaves a wall as soon as the direct step is free, so in
 * a pocket that opens towards the goal it walks straight back in; long or winding routes
 * (mines!) simply fail, and the member stands still or stops in a dead end.
 *
 * Here the party's moves are planned with A* over the same pixel positions and the same
 * step vectors the walker uses, testing every step with the game's own collision function
 * (cbt_collisions, s014 1462:000a, with the walking rules), so a planned path is a path the
 * member can walk. Routes longer than the game's 250-step path buffer are cut; the game
 * re-plans moving combatants every 30-90 ticks, which carries them on. If the search finds
 * nothing within its budget, the original planner runs. Enemies keep the original planner
 * unless "enemies too" is on.
 *
 * Combatant records: DS:9c55 + 0x80*i (+4 x, +6 y, +8 tx, +0a ty, +19 facing, +1c own
 * index, +4a target); floors DS:a6b4[13]; step deltas D_c681/D_c6f4[set*10 + dir] (signed
 * bytes, set from 14f7:343c); path result far 20a5:716a {int wstep[40]; char path[250];
 * int wx[40]; int wy[40]}; D_e118/D_e116 length, D_e11a waypoints. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"

uint64_t plat_ticks_us(void);

int pathfix_party = 1, pathfix_enemies = 0;

#define CBT(i)   ((u16)(0x9c55 + (i) * 0x80))
#define PATHSEG  ((u16)(LOADSEG + 0x20a5))
#define MAPW 1280
#define MAPH 640
#define MAXEXP 60000

static void (*f_collide)(void), (*f_set)(void), (*f_waypoints)(void), (*f_cursor)(void);

typedef struct { u32 stamp; u16 g; u8 dir; u8 coll; } Node;   /* coll: 0 unknown, 1 free, 2 blocked, 3 goal (bumps the target) */
static Node *nodes;
static u32 gen;
typedef struct { u32 f; u32 idx; } HeapE;
static HeapE *heap; static int hn, hcap;

static void hpush(u32 f, u32 idx)
{
    if (hn == hcap) { hcap = hcap ? hcap * 2 : 65536; heap = realloc(heap, (size_t)hcap * sizeof *heap); }
    int i = hn++;
    while (i > 0) { int p = (i - 1) / 2; if (heap[p].f <= f) break; heap[i] = heap[p]; i = p; }
    heap[i].f = f; heap[i].idx = idx;
}
static HeapE hpop(void)
{
    HeapE top = heap[0], last = heap[--hn];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= hn) break;
        if (c + 1 < hn && heap[c + 1].f < heap[c].f) c++;
        if (heap[c].f >= last.f) break;
        heap[i] = heap[c]; i = c;
    }
    if (hn) heap[i] = last;
    return top;
}

static int self_i, target_i;
static int ncoll;

/* the game's collision test for combatant self_i standing at (x, y) */
static int collide(int x, int y)
{
    u16 c = CBT(self_i);
    u16 sx = g_rw((u16)(c + 4)), sy = g_rw((u16)(c + 6));
    g_ww((u16)(c + 4), (u16)x); g_ww((u16)(c + 6), (u16)y);
    u16 arg = c;
    int r = (s16)game_call_native(f_collide, 1, &arg);
    g_ww((u16)(c + 4), sx); g_ww((u16)(c + 6), sy);
    ncoll++;
    return r;
}

static int node_coll(u32 idx, int x, int y)
{
    Node *n = &nodes[idx];
    if (n->stamp != gen) { n->stamp = gen; n->g = 0xffff; n->dir = 0xff; n->coll = 0; }
    if (!n->coll) {
        int r = collide(x, y);
        if (r == 0) n->coll = 1;
        else if (target_i >= 0 && r == (1 << target_i)) n->coll = 3;
        else n->coll = 2;
    }
    return n->coll;
}

static int sdx[10], sdy[10], maxdx = 1, maxdy = 1;

/* ---- the floor's cells (32 x 16 pixels, 40 x 40, column-major u32 at far 20a5:0000):
 * bits 0-3 east wall, 8-11 west, 16-19 north, 24-27 south (1..11 blocks; doors read 12..15
 * when open), bit 31 void. A breadth-first search over the cells from the destination gives
 * every cell its distance in cells; it steers the pixel search around dead ends and tells
 * at once when the destination cannot be reached at all. */
#define FLOORSEG ((u16)(LOADSEG + 0x20a5))
static u32 cellv[1600];
static short celld[1600];
static int wall(u32 v, int shift) { int t = (int)(v >> shift) & 15; return t >= 1 && t <= 11; }
static int cell_open(int cx, int cy, int dx, int dy)
{
    int nx = cx + dx, ny = cy + dy;
    if (nx < 0 || nx >= 40 || ny < 0 || ny >= 40) return 0;
    u32 a = cellv[cx * 40 + cy], b = cellv[nx * 40 + ny];
    if ((a | b) & 0x80000000u) return 0;
    if (dx == 1) return !wall(a, 0) && !wall(b, 8);
    if (dx == -1) return !wall(a, 8) && !wall(b, 0);
    if (dy == 1) return !wall(a, 24) && !wall(b, 16);
    return !wall(a, 16) && !wall(b, 24);
}
static void cell_bfs(int tx, int ty)
{
    for (int k = 0; k < 1600; k++) { cellv[k] = (u32)rw(FLOORSEG, (u16)(4 * k)) | ((u32)rw(FLOORSEG, (u16)(4 * k + 2)) << 16); celld[k] = -1; }
    static short q[1600];
    int qh = 0, qt = 0, c0 = (tx >> 5) * 40 + (ty >> 4);
    if (c0 < 0 || c0 >= 1600) return;
    celld[c0] = 0; q[qt++] = (short)c0;
    while (qh < qt) {
        int c = q[qh++], cx = c / 40, cy = c % 40;
        for (int d = 0; d < 8; d++) {
            static const int NBX[8] = { 1, -1, 0, 0, 1, 1, -1, -1 }, NBY[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
            int dx = NBX[d], dy = NBY[d], ok;
            if (!dx || !dy) ok = cell_open(cx, cy, dx, dy);
            else ok = (cell_open(cx, cy, dx, 0) && cell_open(cx + dx, cy, 0, dy)) || (cell_open(cx, cy, 0, dy) && cell_open(cx, cy + dy, dx, 0));
            if (!ok) continue;
            int n = (cx + dx) * 40 + cy + dy;
            if (celld[n] >= 0) continue;
            celld[n] = (short)(celld[c] + 1); q[qt++] = (short)n;
        }
    }
}

static u32 heur(int x, int y, int tx, int ty)
{
    int ax = abs(tx - x), ay = abs(ty - y);
    int hx = (ax + maxdx - 1) / maxdx, hy = (ay + maxdy - 1) / maxdy;
    int h = hx > hy ? hx : hy;
    int c = (x >> 5) * 40 + (y >> 4);
    if (c >= 0 && c < 1600 && celld[c] > 1) { int hc = (celld[c] - 1) * 10; if (hc > h) h = hc; }
    return (u32)h;
}

static char path[4096];

/* the step vectors of combatant i's animation set */
static void load_steps(int i)
{
    u16 sp = g_rb((u16)(CBT(i) + 0x1c));
    int set = (s16)game_call_native(f_set, 1, &sp);
    maxdx = maxdy = 1;
    for (int d = 0; d < 10; d++) {
        sdx[d] = (s8)g_rb((u16)(0xc681 + set * 10 + d)); sdy[d] = (s8)g_rb((u16)(0xc6f4 + set * 10 + d));
        if (abs(sdx[d]) > maxdx) maxdx = abs(sdx[d]);
        if (abs(sdy[d]) > maxdy) maxdy = abs(sdy[d]);
    }
}

/* A* from (x0, y0) to (tx, ty); returns the number of steps (directions in path[]), -1 if none */
static int search(int x0, int y0, int tx, int ty, int *expanded)
{
    if (!nodes) { nodes = calloc((size_t)MAPW * MAPH, sizeof *nodes); if (!nodes) return -1; }
    if (++gen == 0) { memset(nodes, 0, (size_t)MAPW * MAPH * sizeof *nodes); gen = 1; }
    hn = 0;
    if (x0 < 0 || x0 >= MAPW || y0 < 0 || y0 >= MAPH) return -1;
    u32 s = (u32)x0 * MAPH + (u32)y0;
    nodes[s].stamp = gen; nodes[s].g = 0; nodes[s].dir = 0xff; nodes[s].coll = 1;
    hpush(heur(x0, y0, tx, ty) * 3, s);
    int exp = 0;
    u32 goal = 0xffffffffu;
    uint64_t t_end = plat_ticks_us() + 40000;            /* at most 40 ms, then the game's own planner */
    while (hn && exp < MAXEXP) {
        if (!(exp & 511) && plat_ticks_us() > t_end) break;
        HeapE e = hpop();
        Node *n = &nodes[e.idx];
        int x = (int)(e.idx / MAPH), y = (int)(e.idx % MAPH);
        u32 hcur = heur(x, y, tx, ty);
        if (e.f != (u32)n->g + hcur * 3) continue;            /* stale entry */
        exp++;
        if (abs(tx - x) <= 2 && abs(ty - y) <= 2) { goal = e.idx; break; }
        if (n->coll == 3) { goal = e.idx; break; }               /* in contact with the target */
        for (int d = 0; d < 10; d++) {
            if (!sdx[d] && !sdy[d]) continue;
            int nx = x + sdx[d], ny = y + sdy[d];
            if (nx < 0 || nx >= MAPW || ny < 0 || ny >= MAPH) continue;
            u32 ni = (u32)nx * MAPH + (u32)ny;
            int c = node_coll(ni, nx, ny);
            if (c == 2) continue;
            Node *m = &nodes[ni];
            /* a small extra cost for changing direction keeps the walk smooth */
            u32 g = (u32)n->g + 2 + (n->dir != 0xff && n->dir != d ? 1 : 0);
            if (g >= m->g || g > 0xfff0) continue;
            m->g = (u16)g; m->dir = (u8)d;
            hpush(g + heur(nx, ny, tx, ty) * 3, ni);             /* weighted (1.5 x, g counted twice) */
        }
    }
    *expanded = exp;
    if (goal == 0xffffffffu) return -1;
    int len = 0;
    for (u32 idx = goal; nodes[idx].dir != 0xff && len < (int)sizeof path; ) {
        int d = nodes[idx].dir;
        path[len++] = (char)d;
        int x = (int)(idx / MAPH) - sdx[d], y = (int)(idx % MAPH) - sdy[d];
        idx = (u32)x * MAPH + (u32)y;
    }
    for (int a = 0, b = len - 1; a < b; a++, b--) { char t = path[a]; path[a] = path[b]; path[b] = t; }
    return len;
}

static void sweep(int i, int n);
static void path_find_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    { static int done; const char *e = getenv("DL_PATH_SWEEP"); if (!e) e = getenv("DL_PATH_SWEEP_FAR"); if (e && !done && i < 5) { done = 1; sweep(i, atoi(e)); } }
    if (i < 0 || i >= 13 || !(i < 5 ? pathfix_party : pathfix_enemies) || !f_collide || !f_set) { hook_call_original(); return; }
    u16 c = CBT(i);
    int x = (s16)g_rw((u16)(c + 4)), y = (s16)g_rw((u16)(c + 6));
    int tx = (s16)g_rw((u16)(c + 8)), ty = (s16)g_rw((u16)(c + 0x0a));
    int target = (s8)g_rb((u16)(c + 0x4a));
    if (target >= 13) target = -1;
    if (target >= 0 && g_rb((u16)(GV_cbt_floor + target)) != g_rb((u16)(GV_cbt_floor + i))) { hook_call_original(); return; }

    load_steps(i);

    /* collision rules while planning: walking walls (D_e0a2 0), map and combatants tested,
     * moving combatants other than the target do not block (D_d697 1, D_d6cd target) */
    u8 s_e0a2 = g_rb(GV_path_formation), s_d697 = g_rb(GV_path_target_only), s_e13c = g_rb(0xe13c), s_c600 = g_rb(0xc600);
    u16 s_d708 = g_rw(0xd708);
    u8 s_e11c[32]; for (int k = 0; k < 32; k++) s_e11c[k] = g_rb((u16)(0xe11c + k));
    g_wb(GV_path_formation, 0); g_wb(GV_path_target_only, 1); g_wb(0xe13c, 0); g_wb(0xc600, 0); g_wb(0xd6cd, (u8)target);
    self_i = i; target_i = target; ncoll = 0;

    uint64_t t0 = plat_ticks_us();
    int exp = 0, len = -1;
    int start = collide(x, y);
    int dest = collide(tx, ty);
    int reach = 0;
    if (start != -1 && !(dest != 0 && (target < 0 || (dest & ~(1 << target))))) {
        cell_bfs(tx, ty);
        int c0 = (x >> 5) * 40 + (y >> 4);
        reach = c0 >= 0 && c0 < 1600 && celld[c0] >= 0;
        if (reach) len = search(x, y, tx, ty, &exp);
    }
    uint64_t t1 = plat_ticks_us();

    g_wb(GV_path_formation, s_e0a2); g_wb(GV_path_target_only, s_d697); g_wb(0xe13c, s_e13c); g_wb(0xc600, s_c600); g_ww(0xd708, s_d708);
    for (int k = 0; k < 32; k++) g_wb((u16)(0xe11c + k), s_e11c[k]);
    if (getenv("DL_PATH_LOG"))
        fprintf(stderr, "pathfix: %d (%d,%d)->(%d,%d) target %d: start %d dest %d: %d steps, %d expanded, %d tests, %u us\n",
                i, x, y, tx, ty, target, start, dest, len, exp, ncoll, (unsigned)(t1 - t0));
    if (len < 0) { hook_call_original(); return; }       /* nothing found: the game's own planner decides */

    /* write the result the way path_find leaves it */
    int n = len < 249 ? len : 249;
    for (int k = 0; k < n; k++) wb(PATHSEG, (u16)(0x71ba + k), (u8)path[k]);
    wb(PATHSEG, (u16)(0x71ba + n), 0xff);
    wb(PATHSEG, (u16)(0x71ba + 249), 0xff);
    int nw = 0, px = x, py = y;
    for (int k = 0; k < n; k++) {
        px += sdx[(int)path[k]]; py += sdy[(int)path[k]];
        if ((k + 1 < n && path[k + 1] != path[k]) || k + 1 == n) {
            if (nw < 40) { ww(PATHSEG, (u16)(0x716a + 2 * nw), (u16)k); ww(PATHSEG, (u16)(0x72b4 + 2 * nw), (u16)px); ww(PATHSEG, (u16)(0x7304 + 2 * nw), (u16)py); nw++; }
        }
    }
    g_ww(0xe118, (u16)n); g_ww(0xe116, (u16)n); g_ww(0xe11a, (u16)nw);
    g_ww(0xe0bc, (u16)x); g_ww(0xe0be, (u16)y);
    if (f_waypoints) { u16 a = (u16)i; game_call_native(f_waypoints, 1, &a); }   /* 1db7:0158: who stands near the route */
    g_wb(GV_path_formation, 0);
    if (f_cursor) { u16 a[3] = { 0, 0, g_rw(GV_ems_frame_handle) }; game_call_native(f_cursor, 3, a); }   /* normal cursor, as path_find leaves it */
    g_ww(0x18c2, 0);
    AX = 1;
    hook_return_far(0);
}

/* debugging (DL_PATH_SWEEP=n): on the first planned move of a battle, plan n random moves
 * for that member with both planners and count who finds a path */
static void sweep(int i, int n)
{
    u16 c = CBT(i);
    int x = (s16)g_rw((u16)(c + 4)), y = (s16)g_rw((u16)(c + 6));
    u16 stx = g_rw((u16)(c + 8)), sty = g_rw((u16)(c + 0x0a)); u8 star = g_rb((u16)(c + 0x4a));
    int valid = 0, nat = 0, orig = 0, onlyn = 0, onlyo = 0; uint64_t tn = 0, to = 0;
    long steps_n = 0, steps_o = 0; int both = 0;
    load_steps(i);
    fprintf(stderr, "pathfix sweep: steps");
    for (int d = 0; d < 10; d++) fprintf(stderr, " (%d,%d)", sdx[d], sdy[d]);
    fprintf(stderr, "\n");
    srand(1234);
    /* DL_PATH_SWEEP_FAR: destinations are the floor cells reachable from the member, far ones
     * first (a mine's long winding routes); stats by distance in cells */
    static int dlx[1600], dly[1600], dld[1600]; int nd = 0, far = getenv("DL_PATH_SWEEP_FAR") != 0;
    int bn[4] = {0}, bo[4] = {0}, bv[4] = {0};
    if (far) {
        cell_bfs(x, y);
        for (int c = 0; c < 1600; c++) if (celld[c] > 0) { dlx[nd] = (c / 40) * 32 + 16; dly[nd] = (c % 40) * 16 + 8; dld[nd] = celld[c]; nd++; }
        for (int a = 1; a < nd; a++) for (int b = a; b > 0 && dld[b - 1] < dld[b]; b--) {
            int t = dlx[b]; dlx[b] = dlx[b - 1]; dlx[b - 1] = t; t = dly[b]; dly[b] = dly[b - 1]; dly[b - 1] = t; t = dld[b]; dld[b] = dld[b - 1]; dld[b - 1] = t; }
        if (n > nd) n = nd;
        fprintf(stderr, "pathfix sweep: %d floor cells reachable, farthest %d cells away\n", nd, nd ? dld[0] : 0);
    }
    for (int k = 0; k < n; k++) {
        int tx = far ? dlx[k * nd / n] : x + rand() % 800 - 400, ty = far ? dly[k * nd / n] : y + rand() % 400 - 200;
        int bucket = far ? (dld[k * nd / n] <= 5 ? 0 : dld[k * nd / n] <= 10 ? 1 : dld[k * nd / n] <= 20 ? 2 : 3) : 0;
        if (tx < 16 || tx > MAPW - 16 || ty < 16 || ty > MAPH - 16) continue;
        g_ww((u16)(c + 8), (u16)tx); g_ww((u16)(c + 0x0a), (u16)ty); g_wb((u16)(c + 0x4a), 0xff);
        self_i = i; target_i = -1;
        u8 s_e0a2 = g_rb(GV_path_formation), s_d697 = g_rb(GV_path_target_only);
        g_wb(GV_path_formation, 0); g_wb(GV_path_target_only, 1); g_wb(0xd6cd, 0xff);
        int dest = collide(tx, ty);
        int e = 0; uint64_t t0 = plat_ticks_us();
        int ln = -1, unreach = 0;
        if (dest == 0) { cell_bfs(tx, ty); int c0 = (x >> 5) * 40 + (y >> 4); if (celld[c0] >= 0) ln = search(x, y, tx, ty, &e); else unreach = 1; }
        tn += plat_ticks_us() - t0;
        g_wb(GV_path_formation, s_e0a2); g_wb(GV_path_target_only, s_d697);
        if (dest != 0) continue;
        valid++;
        void *snap = eval_snapshot();
        hook_remove(GF_path_find, path_find_hook);
        u16 a = (u16)i; t0 = plat_ticks_us();
        int ro = (s16)(game_call_link(GF_path_find, 1, &a) & 0xffff);
        to += plat_ticks_us() - t0;
        int lo = (s16)g_rw(0xe118);
        eval_restore(snap); eval_free(snap);
        hook_install(GF_path_find, path_find_hook);
        nat += ln >= 0; orig += ro == 1; onlyn += ln >= 0 && ro != 1; onlyo += ln < 0 && ro == 1;
        bv[bucket]++; bn[bucket] += ln >= 0; bo[bucket] += ro == 1;
        if (ln < 0 && ro == 1) fprintf(stderr, "pathfix sweep: native failed (%d,%d)->(%d,%d): %s, %d expanded, original %d steps\n",
                                       x, y, tx, ty, unreach ? "cells unreachable" : "search gave up", e, lo);
        if (ln >= 0 && ro == 1) { both++; steps_n += ln; steps_o += lo; }
    }
    g_ww((u16)(c + 8), stx); g_ww((u16)(c + 0x0a), sty); g_wb((u16)(c + 0x4a), star);
    if (far) {
        static const char *BN[4] = { "1-5 cells", "6-10 cells", "11-20 cells", "over 20 cells" };
        for (int b = 0; b < 4; b++) if (bv[b]) fprintf(stderr, "pathfix sweep: %s away: %d destinations, new planner %d, original %d\n", BN[b], bv[b], bn[b], bo[b]);
    }
    fprintf(stderr, "pathfix sweep: %d reachable-looking destinations: native %d, original %d, only native %d, only original %d; "
            "time native %u us, original %u us; where both: %ld vs %ld steps\n", valid, nat, orig, onlyn, onlyo, (unsigned)tn, (unsigned)to,
            steps_n, steps_o);
}

void pathfix_set(int party, int enemies)
{
    pathfix_party = party; pathfix_enemies = enemies;
    config_set_int("fix.pathfinding", party); config_set_int("fix.pathfinding_enemies", enemies);
    if (party || enemies) hook_install(GF_path_find, path_find_hook);
    else hook_remove(GF_path_find, path_find_hook);
}

/* ---------------------------------------------------------------- battle speed
 * Battles run on combat_clock (DS:07d4), which the game's timer routine (root 047c:000a)
 * counts up on every timer tick unless the battle is paused (DS:07d3); combat ticks,
 * walking, animation frames and actions are all timed against it. At a higher speed the
 * routine counts it up faster; the rest of the timer routine (sound, other clocks) runs as
 * before. */
int battle_speed = 100;                  /* percent */
static int clock_acc;
static void timer_hook(void)
{
    hook_call_original();
    { static int n; if (getenv("DL_SPEED_LOG") && !(n++ % 50)) fprintf(stderr, "timer: speed %d paused %d mode %d clock %u\n", battle_speed, g_rb(GV_pause_depth), game_mode(), g_rw(GV_combat_clock)); }
    if (battle_speed <= 100 || g_rb(GV_pause_depth) || game_mode() != GM_BATTLE) { clock_acc = 0; return; }
    clock_acc += battle_speed - 100;
    while (clock_acc >= 100) { clock_acc -= 100; g_ww(GV_combat_clock, (u16)(g_rw(GV_combat_clock) + 1)); }
}

void battle_speed_set(int pct)
{
    if (pct < 100) pct = 100;
    if (pct > 400) pct = 400;
    battle_speed = pct;
    config_set_int("battle.speed", pct);
    if (pct > 100) hook_install(-1, 0x047c, 0x000a, timer_hook);
    else hook_remove(-1, 0x047c, 0x000a, timer_hook);
}

void pathfix_init(void)
{
    battle_speed_set(config_get_int("battle.speed", 100));
    f_collide = game_func(GF_cbt_collisions);
    f_set = game_func(GF_cbt_anim_set);
    f_waypoints = game_func(14, 0x1db7, 0x0158);
    f_cursor = game_func(GF_ems_map_page);
    if (!f_collide || !f_set) fprintf(stderr, "pathfix: game functions not found\n");
    pathfix_set(config_get_int("fix.pathfinding", 1), config_get_int("fix.pathfinding_enemies", 0));
}
