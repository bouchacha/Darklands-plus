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
 * re-plans moving combatants every 30-90 ticks, which carries them on. The party and the
 * enemies alike (one switch, fix.pathfinding).
 *
 * In a melee crowd (stress-tested with traces of whole battles, DL_BATTLE_TRACE below):
 * - the collision test is the game's for the floor, asked once per spot and kept, and done here
 *   for the fighters (collide below): a search costs a few ms at most, not 40;
 * - the goal is any step that touches the target, as for the walker and the game's planner, and
 *   a foe with no free side is known at once (target_open), not after combing the map;
 * - a fighter whose last step was blocked plans around the one in its way, asks again soon
 *   (crowd_step_hook), starts a new route from its first step, and follows a foe that moved
 *   (crowd_approach_hook); an enemy takes no long way round a crowd, and one that cannot get on
 *   from a spot is told so (the game then waits, or picks another foe).
 * If the search runs out of time, or the target's cells are out of reach (an enemy opens
 * closed doors, which only the original does), the original planner runs.
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
#include "nscreen.h"

uint64_t plat_ticks_us(void);

int pathfix_party = 1, pathfix_enemies = 1;

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

static int self_i, target_i, bounded, search_cut;
static unsigned search_budget = 40000;   /* us */
#define D_REPLAN 0xd97c                  /* int[13]: re-plan time */
#define D_BLOCKER 0xd5f6                 /* int[13]: who blocked the last step */
#define D_NEXTSTEP 0xca30                /* char[13]: the step lined up, -1 none */
#define FLOORSEG_ ((u16)(LOADSEG + 0x20a5))   /* the floor's cells, far 20a5:0000 (below) */
static struct { s16 x, y; u8 n; u16 clk; s16 tx, ty; } stall[13];
static u8 stall_resets[13];                                 /* (the party: times a member blocked only by friends kept asking) */   /* where each combatant last asked for a route, and how often since from there */
static int ncoll;

/* the game's collision test for combatant self_i standing at (x, y); others: 0 leaves the
 * combatants out (D_c600 1), the floor alone */
static int collide_game(int x, int y, int others)
{
    u16 c = CBT(self_i);
    u16 sx = g_rw((u16)(c + CB_X)), sy = g_rw((u16)(c + CB_Y));
    u8 s_c600 = g_rb(0xc600);
    g_ww((u16)(c + CB_X), (u16)x); g_ww((u16)(c + CB_Y), (u16)y);
    if (!others) g_wb(0xc600, 1);
    u16 arg = c;
    int r = (s16)game_call_native(f_collide, 1, &arg);
    g_ww((u16)(c + CB_X), sx); g_ww((u16)(c + CB_Y), sy); g_wb(0xc600, s_c600);
    return r;
}

/* That test costs a couple of microseconds, and a search makes tens of thousands: a long one took
 * 40 ms, a frame and more. Its two parts are taken apart here. The floor (walls, furniture, the
 * void: 1462:01f0, 083e) depends on nothing but the spot, the walker's size and the floor's cells;
 * it is asked of the game once per spot and size and kept (until a cell changes: a door, another
 * floor, another battle). The others in the way are tested here as the game tests them
 * (1462:0740: active, on the walker's floor, and with D_d697 only the target and those standing;
 * boxes that overlap, but for the two edge cases it lets pass). DL_PATH_CHECK=1 compares every
 * test with the game's own and tells of any difference. */
#define NSLOT 4
static u8 *smap[NSLOT]; static u16 skey[NSLOT]; static u32 suse[NSLOT], sclock;
static u32 scells[1600]; static u8 *scur;
static int ob_n; static struct { int x, y, w, h, bit; } ob[13];
static int self_k, self_w, self_h, pcheck = -1, pcheck_bad;
static u16 also_block;                   /* those who blocked the walker's last step: in the way though on the move */
static void others_setup(void)           /* who blocks, by the current D_d697 / D_d6cd */
{
    int d697 = g_rb(GV_path_target_only), d6cd = (s8)g_rb(0xd6cd), fl = g_rb((u16)(GV_cbt_floor + self_k));
    ob_n = 0;
    for (int i = 0; i < 13; i++) {
        u16 c = CBT(i);
        if (i == self_k || g_rb((u16)(c + CB_STATUS)) != 1 || g_rb((u16)(GV_cbt_floor + i)) != fl) continue;
        if (d697 && d6cd != i && g_rb((u16)(GV_cbt_moving + i)) && !(also_block >> i & 1)) continue;
        ob[ob_n].x = (s16)g_rw((u16)(c + CB_X)); ob[ob_n].y = (s16)g_rw((u16)(c + CB_Y));
        ob[ob_n].w = (s8)g_rb((u16)(GV_cbt_half_w + i)); ob[ob_n].h = (s8)g_rb((u16)(GV_cbt_half_h + i));
        ob[ob_n].bit = (s16)g_rw((u16)(0x01e4 + 2 * i));                   /* D_01e4[i]: 1 << i */
        ob_n++;
    }
}
static void collide_setup(void)          /* at each planning, once the collision rules are set */
{
    static u32 cells[1600];
    for (int k = 0; k < 1600; k++) cells[k] = (u32)rw(FLOORSEG_, (u16)(4 * k)) | ((u32)rw(FLOORSEG_, (u16)(4 * k + 2)) << 16);
    if (memcmp(cells, scells, sizeof cells)) {
        if (getenv("DL_PATH_LOG")) { int n = 0, f = -1; for (int k = 0; k < 1600; k++) if (cells[k] != scells[k]) { n++; if (f < 0) f = k; }
            fprintf(stderr, "pathfix: floor cache cleared: %d cells changed (first %d: %08x -> %08x)\n", n, f, scells[f], cells[f]); }
        memcpy(scells, cells, sizeof cells);
        for (int k = 0; k < NSLOT; k++) if (smap[k]) memset(smap[k], 0, (size_t)MAPW * MAPH);
    }
    if (pcheck < 0) pcheck = getenv("DL_PATH_CHECK") != 0;
    u16 c = CBT(self_i);
    self_k = g_rb((u16)(c + 0x1c));
    self_w = (s8)g_rb((u16)(GV_cbt_half_w + self_k)); self_h = (s8)g_rb((u16)(GV_cbt_half_h + self_k));
    u16 key = (u16)((self_w & 0xff) | (self_h & 0xff) << 8);
    int k = -1;
    for (int j = 0; j < NSLOT; j++) if (smap[j] && skey[j] == key) k = j;
    if (k < 0) {
        for (int j = 0; j < NSLOT && k < 0; j++) if (!smap[j]) k = j;
        if (k < 0) { k = 0; for (int j = 1; j < NSLOT; j++) if (suse[j] < suse[k]) k = j; }
        if (!smap[k]) smap[k] = malloc((size_t)MAPW * MAPH);
        if (smap[k]) memset(smap[k], 0, (size_t)MAPW * MAPH);
        skey[k] = key;
    }
    suse[k] = ++sclock;
    scur = smap[k];
    others_setup();
}
static int collide(int x, int y)
{
    ncoll++;
    if (!scur || x < 0 || x >= MAPW || y < 0 || y >= MAPH) return collide_game(x, y, 1);
    u8 *p = &scur[(u32)x * MAPH + (u32)y];
    if (!*p) { int f = collide_game(x, y, 0); *p = f == -1 ? 0xff : f == 0 ? 1 : (u8)(0x80 | ((f >> 8) & 0x7f)); }
    int r = *p == 0xff ? -1 : *p == 1 ? 0 : (*p & 0x7f) << 8;
    if (r != -1) {
        int R = x + self_w - 1, L = x - self_w + 1, B = y + self_h - 1, T = y - self_h + 1;
        for (int k = 0; k < ob_n; k++) {
            int ox = ob[k].x, oy = ob[k].y, ow = ob[k].w, oh = ob[k].h;
            if (abs(ox - x) < ow + self_w && abs(oy - y) < oh + self_h) {
                int sr = ow + ox, sb = oh + oy, st = oy - oh;
                if ((ox - ow != R || (B != st && T != sb)) && (L != sr || (B != st && sb != T))) r |= ob[k].bit;
            }
        }
    }
    if (pcheck) {
        int g = collide_game(x, y, 1);
        int h = r;
        if (h != -1 && g != -1) { g &= ~also_block; h &= ~also_block; }   /* (those the game lets pass on the move) */
        if (g != h && pcheck_bad++ < 20) fprintf(stderr, "pathfix check: %d at %d,%d: %#x here, %#x by the game\n", self_i, x, y, h & 0xffff, g & 0xffff);
    }
    return r;
}

/* a step that touches the target ends the walk in a fight, whoever else it touches (cbt_step,
 * s014 0e76:1624, and the game's planner test D_e11c[target] alone): such a spot is a goal */
static int node_coll(u32 idx, int x, int y)
{
    Node *n = &nodes[idx];
    if (n->stamp != gen) { n->stamp = gen; n->g = 0xffff; n->dir = 0xff; n->coll = 0; }
    if (!n->coll) {
        int r = collide(x, y);
        if (r == 0) n->coll = 1;
        else if (target_i >= 0 && r != -1 && (r & (1 << target_i))) n->coll = 3;
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

/* is there a free spot one step from the target, from where a step reaches it? A foe whose every
 * side is taken (a melee blob) cannot be reached, and the search would comb the map for a way
 * until its time ran out: tens of milliseconds, for every enemy at the back, many times over */
static int target_open(int x0, int y0)
{
    u16 cs = CBT(self_i), ct = CBT(target_i);
    int tx = (s16)g_rw((u16)(ct + 4)), ty = (s16)g_rw((u16)(ct + 6));
    int W = g_rb((u16)(GV_cbt_half_w + g_rb((u16)(cs + 0x1c)))) + g_rb((u16)(GV_cbt_half_w + g_rb((u16)(ct + 0x1c))));
    int H = g_rb((u16)(GV_cbt_half_h + g_rb((u16)(cs + 0x1c)))) + g_rb((u16)(GV_cbt_half_h + g_rb((u16)(ct + 0x1c))));
    int bw = W + maxdx, bh = H + maxdy;
    if (abs(x0 - tx) <= bw && abs(y0 - ty) <= bh) return 1;    /* (beside it already) */
    for (int dy = -bh; dy <= bh; dy++)
        for (int dx = -bw; dx <= bw; dx++) {
            if (abs(dx) < W - 1 && abs(dy) < H - 1) continue;   /* (on the target) */
            int x = tx + dx, y = ty + dy;
            if (x < 0 || x >= MAPW || y < 0 || y >= MAPH) continue;
            if (node_coll((u32)x * MAPH + (u32)y, x, y) == 1) return 1;
        }
    return 0;
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
    *expanded = 0;
    if (target_i >= 0 && !target_open(x0, y0)) return -1;
    u32 h0 = heur(x0, y0, tx, ty);
    hpush(h0 * 3, s);
    /* no long way round a crowd. An enemy goes for a foe by at most half again the way the floor
       allows (the cells' distance, in the heuristic) and 30 steps more; anyone planning around
       everyone (stuck) by at most twice it and 40 more. A melee blob that closes the near side of
       a foe sent the ones at the back on walks of 150 or 200 steps, round the houses, for a foe
       they would reach when the fight had moved on; now they look for another foe or wait (the
       game's own choice when it finds no way: 2cca, enemy_turn). The bound is on g + h (the
       shortest route through a spot), so a search for a foe out of reach stays near the line to it */
    u32 maxg = 0xfff0, m;
    if (bounded & 1) { m = (h0 * 3 / 2 + 30) * 5 / 2; if (m < maxg) maxg = m; }   /* (g: 2 a step, 1 more a turn) */
    if (bounded & 2) { m = (h0 * 2 + 40) * 5 / 2; if (m < maxg) maxg = m; }
    if (bounded & 4) { m = 250 * 5 / 2; if (m < maxg) maxg = m; }              /* (a cost: the trackers' 250 steps) */
    int exp = 0;
    u32 goal = 0xffffffffu;
    uint64_t t_end = plat_ticks_us() + search_budget;    /* at most 40 ms (a route) or 5 (a cost), then the game's own planner */
    search_cut = 0;
    while (hn) {
        if (exp >= MAXEXP || (!(exp & 511) && plat_ticks_us() > t_end)) { search_cut = 1; break; }
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
            if (g >= m->g) continue;
            u32 h = heur(nx, ny, tx, ty);
            if (g + 2 * h > maxg) continue;
            m->g = (u16)g; m->dir = (u8)d;
            hpush(g + h * 3, ni);                                 /* weighted (1.5 x, g counted twice) */
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
static FILE *trace; static FILE *trace_file(void);
/* path_find's answer "no way" (2), the state left as it leaves it */
static void no_way(void)
{
    g_ww(0xe118, 9999);
    g_wb(GV_path_formation, 0); g_wb(GV_path_target_only, 0);
    if (f_cursor) { u16 a[3] = { 0, 0, g_rw(GV_ems_frame_handle) }; game_call_native(f_cursor, 3, a); }
    g_ww(0x18c2, 0);
    AX = 2;
    hook_return_far(0);
}
static u16 plan_clock[13]; static u8 plan_count[13];  /* when each combatant last had a route planned */
/* a new battle (mods/ways.c orders_frame): what this file remembers of the last one forgotten - its
   clock restarts, so a time kept from before would mean nothing */
void pathfix_battle_begins(void)
{
    memset(stall, 0, sizeof stall); memset(stall_resets, 0, sizeof stall_resets);
    memset(plan_clock, 0, sizeof plan_clock); memset(plan_count, 0, sizeof plan_count);
}
static void path_find_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    if (i >= 0 && i < 13) { plan_clock[i] = g_rw(GV_combat_clock); plan_count[i]++; }
    { static int done; const char *e = getenv("DL_PATH_SWEEP"); if (!e) e = getenv("DL_PATH_SWEEP_FAR"); if (e && !done && i < 5) { done = 1; sweep(i, atoi(e)); } }
    if (i < 0 || i >= 13 || !(i < 5 ? pathfix_party : pathfix_enemies) || !f_collide || !f_set) { hook_call_original(); return; }
    u16 c = CBT(i);
    int x = (s16)g_rw((u16)(c + CB_X)), y = (s16)g_rw((u16)(c + CB_Y));
    int tx = (s16)g_rw((u16)(c + CB_TO_X)), ty = (s16)g_rw((u16)(c + CB_TO_Y));
    int target = (s8)g_rb((u16)(c + CB_TARGET));
    if (target >= 13) target = -1;
    if (target >= 0 && g_rb((u16)(GV_cbt_floor + target)) != g_rb((u16)(GV_cbt_floor + i))) { hook_call_original(); return; }

    /* asking from one spot again and again. A combatant that asks again from where it stood the last time
       has not got on: the route went through friends on the move (who do not block a plan), and they stand
       in the way; it then plans around everyone. (Asks within a few ticks are one: the enemies' choice of
       a foe asks for each foe in turn.) The eighth ask from one spot is told there is no way, and the game
       gives the foe up, or the member looks for a spot near it (2cca), instead of asking for ever. Who that
       holds for:
       - enemies, and the party in a fight going for a foe: as said;
       - the party in a fight walking somewhere, with only friends in the way (two in a doorway, paths
         crossing): four rounds more, asking again at different moments (crowd_step_hook), so that one gets
         round while the other stands;
       - the party out of a fight, or following a friend (single file): never. It waits behind the one in
         front and goes on when that one does. Told so for good, a follower that stood in the line while the
         leader went round a corner never moved again, and one such member made every order of a group fail
         ("can't walk there": s014 19cd stops them all when one has no way), until the next fight.
       A member of the party asking for another goal (a new order; the one it follows gone on) starts afresh. */
    int stuck = 0;
    u16 now = g_rw(GV_combat_clock);
    if (i < 5 && abs(tx - stall[i].tx) + abs(ty - stall[i].ty) > 24) stall[i].n = 0;
    stall[i].tx = (s16)tx; stall[i].ty = (s16)ty;
    if (abs(x - stall[i].x) <= 3 && abs(y - stall[i].y) <= 3) {
        if ((u16)(now - stall[i].clk) >= 6 && stall[i].n < 250) { stall[i].n++; stall[i].clk = now; }
        stuck = stall[i].n;
    } else { stall[i].x = (s16)x; stall[i].y = (s16)y; stall[i].n = 0; stall[i].clk = now; }
    if (stuck >= 8 && i < 5 && ((target >= 0 && target < 5) || g_rb(GV_real_combat) != 1)) stuck = 7;
    else if (stuck >= 8 && i < 5 && target < 0 && stall_resets[i] < 4) {
        u16 blk = g_rw((u16)(D_BLOCKER + 2 * i)) & 0x1fff;
        if (blk && !(blk & ~0x1fu)) { stall[i].n = 3; stuck = 3; stall_resets[i]++; }
    }
    if (stuck == 0) stall_resets[i] = 0;
    if (stuck >= 8) {
        if (trace_file()) fprintf(trace, "P %u %d %d %d %d %d %d %d %d %d %d %u\n", now, i, x, y, tx, ty, target, stuck, -2, 0, 0, 0u);
        no_way();
        return;
    }

    load_steps(i);

    /* collision rules while planning: walking walls (D_e0a2 0), map and combatants tested,
     * moving combatants other than the target do not block (D_d697 1, D_d6cd target) */
    u8 s_e0a2 = g_rb(GV_path_formation), s_d697 = g_rb(GV_path_target_only), s_e13c = g_rb(0xe13c), s_c600 = g_rb(0xc600);
    u16 s_d708 = g_rw(0xd708);
    u8 s_e11c[32]; for (int k = 0; k < 32; k++) s_e11c[k] = g_rb((u16)(0xe11c + k));
    g_wb(GV_path_formation, 0); g_wb(GV_path_target_only, 1); g_wb(0xe13c, 0); g_wb(0xc600, 0); g_wb(0xd6cd, (u8)target);
    self_i = i; target_i = target; ncoll = 0; bounded = i >= 5 && target >= 0;
    also_block = g_rw((u16)(D_BLOCKER + 2 * i)) & 0x1fff;  /* (D_d5f6, set by a blocked step, cleared by the next good one) */
    collide_setup();

    uint64_t t0 = plat_ticks_us();
    int exp = 0, len = -1;
    int start = collide(x, y);
    int dest = collide(tx, ty);
    int reach = 0;
    /* (the destination as path_find tests it: free, or touching the target, whoever else is there) */
    if (start != -1 && (dest == 0 || (target >= 0 && dest != -1 && (dest & (1 << target))))) {
        if (stuck) { g_wb(GV_path_target_only, 0); others_setup(); bounded |= 2; }   /* everyone blocks: around them */
        cell_bfs(tx, ty);
        int c0 = (x >> 5) * 40 + (y >> 4);
        reach = c0 >= 0 && c0 < 1600 && celld[c0] >= 0;
        if (reach) len = search(x, y, tx, ty, &exp);
    }
    uint64_t t1 = plat_ticks_us();

    g_wb(GV_path_formation, s_e0a2); g_wb(GV_path_target_only, s_d697); g_wb(0xe13c, s_e13c); g_wb(0xc600, s_c600); g_ww(0xd708, s_d708);
    for (int k = 0; k < 32; k++) g_wb((u16)(0xe11c + k), s_e11c[k]);
    if (getenv("DL_PATH_LOG"))
        fprintf(stderr, "pathfix: %d (%d,%d)->(%d,%d) target %d: start %d dest %d%s: %d steps, %d expanded, %d tests, %u us\n",
                i, x, y, tx, ty, target, start, dest, stuck ? " (stuck: around everyone)" : "", len, exp, ncoll, (unsigned)(t1 - t0));
    if (trace_file()) fprintf(trace, "P %u %d %d %d %d %d %d %d %d %d %d %u\n", g_rw(GV_combat_clock), i, x, y, tx, ty, target, stuck, len, exp, ncoll, (unsigned)(t1 - t0));
    /* nothing found. The search looked everywhere it may (no way, or none short enough for an
       enemy, or no free side of the foe): there is none, and the game's planner, with the same
       steps and tests, would only take its time to say so (tens of ms in a crowd). Else (cells
       out of reach: an enemy opens closed doors, which only the game's planner does; or out of
       time) the game's planner decides */
    if (len < 0 && reach && !search_cut) { no_way(); return; }
    if (len < 0) {
        hook_call_original();
        if (trace) fprintf(trace, "O %u %d %d %u\n", g_rw(GV_combat_clock), i, (s16)AX, (unsigned)(plat_ticks_us() - t1));
        return;
    }

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

/* ---- walking in a crowd
 * A walker re-plans when its re-plan time comes (D_d97c[i]): 30..120 ticks after its last good
 * step (cbt_step), or after its last plan (the approach, s014 0e76:2cca). Two things made a melee
 * blob stand still and jerk:
 * - a walker whose step was blocked by somebody stood until that time came, up to two and a half
 *   seconds, though the one in its way had stopped to fight. It now asks again after 12..24 ticks
 *   (from the same spot the plan goes around everyone, see stall[] above);
 * - on a new plan it still took the step of the old route that it had lined up (D_ca30[i]) and
 *   then went on along the new one from its second step: a step off the route, into somebody.
 *   The new route now starts from its first step.
 * And a walker going for a foe re-plans when the foe has moved on from where the route ends
 * (it walked to the old spot first, stopped, and only then turned), at most every 30 ticks. */
static int crowd_on(int i) { return i >= 0 && i < 13 && (i < 5 ? pathfix_party : pathfix_enemies); }
static void crowd_step_hook(void)        /* cbt_step (i, set), s014 0e76:1624 (a near call) */
{
    int i = (s16)rw(SS, (u16)(SP + 2));
    if (!crowd_on(i)) { hook_call_original(); return; }
    u16 c = CBT(i);
    u8 next = g_rb((u16)(D_NEXTSTEP + i));
    u16 x = g_rw((u16)(c + CB_X)), y = g_rw((u16)(c + CB_Y));
    hook_call_original();
    if (ns_unwinding || next == 0xff || g_rb((u16)(D_NEXTSTEP + i)) != 0xff || !g_rb((u16)(GV_cbt_moving + i))) return;
    if (g_rw((u16)(c + CB_X)) != x || g_rw((u16)(c + CB_Y)) != y || !g_rw((u16)(D_BLOCKER + 2 * i))) return;
    u16 due = (u16)(g_rw(GV_combat_clock) + 12 + rand() % 13);   /* blocked by somebody: ask again soon */
    if (g_rw((u16)(D_REPLAN + 2 * i)) > due) g_ww((u16)(D_REPLAN + 2 * i), due);
}
static void crowd_approach_hook(void)    /* the approach (i), s014 0e76:2cca: every tick while walking */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    if (!crowd_on(i)) { hook_call_original(); return; }
    u16 c = CBT(i), now = g_rw(GV_combat_clock);
    int t = (s8)g_rb((u16)(c + CB_TARGET));
    if (t >= 0 && t < 13 && g_rb(CBF(t, CB_STATUS)) == 1 && g_rb((u16)(GV_cbt_floor + t)) == g_rb((u16)(GV_cbt_floor + i))
        && (u16)(now - plan_clock[i]) >= 30 && g_rw((u16)(D_REPLAN + 2 * i)) > now
        && (abs((s16)g_rw(CBF(t, CB_X)) - (s16)g_rw((u16)(c + CB_TO_X))) > 12 || abs((s16)g_rw(CBF(t, CB_Y)) - (s16)g_rw((u16)(c + CB_TO_Y))) > 8))
        g_ww((u16)(D_REPLAN + 2 * i), now);               /* the foe has moved on: a new route now */
    u8 n0 = plan_count[i];
    hook_call_original();
    if (ns_unwinding) return;
    if (plan_count[i] != n0 && g_rb((u16)(GV_cbt_path_step + i)) == 0) g_wb((u16)(D_NEXTSTEP + i), 0xff);   /* a new route: from its first step */
}

/* ---- the AI's walking costs
 * When a member's foe falls (ai_rethink), ai_costs (s014 1db7:0028) measures the way on foot to
 * each foe with the game's route trackers (12f7:03ea), everyone else in the way: five to eight
 * runs of up to ten trackers by 250 steps, the game's collision test at every step, 10 to 40 ms
 * in one tick (a hitch, at each kill). Here each is measured with the search above (the same
 * steps, tests and goal: a step touching the foe; at most 250 steps; 5 ms each, else the
 * trackers). The cost is the route's steps, as the trackers' (D_e116); none, 32000 as before. */
static int costs_who = -1;
static void costs_hook(void)             /* ai_costs (i, lo, hi) */
{
    int was = costs_who;
    costs_who = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    costs_who = was;
}
static void trackers_hook(void)          /* path_trackers (rec), a copy of the combatant with tx, ty, target set */
{
    int i = costs_who;
    if (i < 0 || i >= 13 || !(i < 5 ? pathfix_party : pathfix_enemies) || !f_collide || !f_set) { hook_call_original(); return; }
    u16 rec = rw(SS, (u16)(SP + 4));
    int x = (s16)g_rw((u16)(rec + 4)), y = (s16)g_rw((u16)(rec + 6)), tx = (s16)g_rw((u16)(rec + 8)), ty = (s16)g_rw((u16)(rec + 0xa));
    int target = (s8)g_rb((u16)(rec + 0x4a));
    if (target < 0 || target >= 13 || x != (s16)g_rw(CBF(i, CB_X)) || y != (s16)g_rw(CBF(i, CB_Y))) { hook_call_original(); return; }
    load_steps(i);
    u8 s_e0a2 = g_rb(GV_path_formation), s_e13c = g_rb(0xe13c), s_c600 = g_rb(0xc600), s_d6cd = g_rb(0xd6cd); u16 s_d708 = g_rw(0xd708);
    u8 s_e11c[32]; for (int k = 0; k < 32; k++) s_e11c[k] = g_rb((u16)(0xe11c + k));
    g_wb(GV_path_formation, 0); g_wb(0xe13c, 0); g_wb(0xc600, 0); g_wb(0xd6cd, (u8)target);   /* (walking rules; D_d697 as the caller left it: everyone blocks) */
    self_i = i; target_i = target; ncoll = 0; bounded = 4; also_block = 0;
    collide_setup();
    uint64_t t0 = plat_ticks_us();
    int exp = 0, len = -1, reach = 0;
    if (collide(x, y) != -1) {
        cell_bfs(tx, ty);
        int c0 = (x >> 5) * 40 + (y >> 4);
        reach = c0 >= 0 && c0 < 1600 && celld[c0] >= 0;
        if (reach) { search_budget = 5000; len = search(x, y, tx, ty, &exp); search_budget = 40000; }
    }
    g_wb(GV_path_formation, s_e0a2); g_wb(0xe13c, s_e13c); g_wb(0xc600, s_c600); g_wb(0xd6cd, s_d6cd); g_ww(0xd708, s_d708);
    for (int k = 0; k < 32; k++) g_wb((u16)(0xe11c + k), s_e11c[k]);
    if (trace_file()) fprintf(trace, "C %u %d %d %d %d %u\n", g_rw(GV_combat_clock), i, target, len, exp, (unsigned)(plat_ticks_us() - t0));
    if (len < 0 && (!reach || search_cut)) { hook_call_original(); return; }   /* (doors for an enemy, or out of time: the trackers) */
    if (len > 249) len = -1;
    g_wb(GV_path_target_only, 0);
    if (len >= 0) g_ww(0xe116, (u16)len);
    AX = len >= 0 ? 1 : 2;
    hook_return_far(0);
}

/* debugging (DL_PATH_SWEEP=n): on the first planned move of a battle, plan n random moves
 * for that member with both planners and count who finds a path */
static void sweep(int i, int n)
{
    u16 c = CBT(i);
    int x = (s16)g_rw((u16)(c + CB_X)), y = (s16)g_rw((u16)(c + CB_Y));
    u16 stx = g_rw((u16)(c + CB_TO_X)), sty = g_rw((u16)(c + CB_TO_Y)); u8 star = g_rb((u16)(c + CB_TARGET));
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
        g_ww((u16)(c + CB_TO_X), (u16)tx); g_ww((u16)(c + CB_TO_Y), (u16)ty); g_wb((u16)(c + CB_TARGET), 0xff);
        self_i = i; target_i = -1; bounded = 0;
        u8 s_e0a2 = g_rb(GV_path_formation), s_d697 = g_rb(GV_path_target_only);
        g_wb(GV_path_formation, 0); g_wb(GV_path_target_only, 1); g_wb(0xd6cd, 0xff);
        also_block = 0; collide_setup();
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
    g_ww((u16)(c + CB_TO_X), stx); g_ww((u16)(c + CB_TO_Y), sty); g_wb((u16)(c + CB_TARGET), star);
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
    config_set_int("fix.pathfinding", party || enemies);     /* (one switch for both sides; apart only for tests: upath) */
    if (party || enemies) {
        hook_install(GF_path_find, path_find_hook);
        hook_install(GF_cbt_step, crowd_step_hook);
        hook_install(14, 0x0e76, 0x2cca, crowd_approach_hook);
        hook_install(GF_ai_costs, costs_hook);
        hook_install(GF_path_trackers, trackers_hook);
    } else {
        hook_remove(GF_path_find, path_find_hook);
        hook_remove(GF_cbt_step, crowd_step_hook);
        hook_remove(14, 0x0e76, 0x2cca, crowd_approach_hook);
        hook_remove(GF_ai_costs, costs_hook);
        hook_remove(GF_path_trackers, trackers_hook);
    }
}

/* ---------------------------------------------------------------- battle speed
 * Battles run on combat_clock (DS:07d4), which the game's timer routine (root 047c:000a)
 * counts up on every timer tick unless the battle is paused (DS:07d3); combat ticks,
 * walking, animation frames and actions are all timed against it. At a higher speed the
 * routine counts it up faster; the rest of the timer routine (sound, other clocks) runs as
 * before. */
int battle_speed = 100;                  /* percent */
int battle_fast;                         /* fast forward (Z in a battle, battle.fast_key): BATTLE_FAST_PCT while on */
static int clock_acc;
static void timer_hook(void)
{
    hook_call_original();
    int sp = battle_fast ? BATTLE_FAST_PCT : battle_speed;
    { static int n; if (getenv("DL_SPEED_LOG") && !(n++ % 50)) fprintf(stderr, "timer: speed %d paused %d mode %d clock %u\n", sp, g_rb(GV_pause_depth), game_mode(), g_rw(GV_combat_clock)); }
    if (game_mode() != GM_BATTLE) battle_fast = 0;              /* (a battle's fast forward ends with it) */
    if (sp <= 100 || g_rb(GV_pause_depth) || game_mode() != GM_BATTLE) { clock_acc = 0; return; }
    clock_acc += sp - 100;
    while (clock_acc >= 100) { clock_acc -= 100; g_ww(GV_combat_clock, (u16)(g_rw(GV_combat_clock) + 1)); }
}
static void timer_hook_set(void)
{
    if (battle_speed > 100 || battle_fast) hook_install(-1, 0x047c, 0x000a, timer_hook);
    else hook_remove(-1, 0x047c, 0x000a, timer_hook);
}

void battle_speed_set(int pct)
{
    if (pct < 100) pct = 100;
    if (pct > 400) pct = 400;
    battle_speed = pct;
    config_set_int("battle.speed", pct);
    timer_hook_set();
}
void battle_fast_set(int on)
{
    battle_fast = on && game_mode() == GM_BATTLE;
    fprintf(stderr, "battle: fast forward %s\n", battle_fast ? "on" : "off");
    timer_hook_set();
}

/* ---------------------------------------------------------------- tests
 * DL_BATTLE_TRACE=file: after every combat tick (s014 0e76:0f58), one line per active combatant:
 * clock, index, x, y, action (+0x10), moving, target, cbt_action, anim, tx, ty, wall-clock us, what blocked the
 * last step (D_d5f6), the re-plan time (D_d97c), the path step reached. And
 * DL_PATH_TRACE: each planning (the hook above) with its cost goes there too (P lines). */
static FILE *trace_file(void)
{
    static int tried;
    if (!tried) { tried = 1; const char *f = getenv("DL_BATTLE_TRACE"); if (f) trace = fopen(f, "w"); }
    return trace;
}
static uint64_t costs_us, trackers_us; static int costs_n, trackers_n;
static void costs_time_hook(void) { uint64_t t = plat_ticks_us(); hook_call_original(); costs_us += plat_ticks_us() - t; costs_n++; }
static void trackers_time_hook(void) { uint64_t t = plat_ticks_us(); hook_call_original(); trackers_us += plat_ticks_us() - t; trackers_n++; }
static void tick_trace_hook(void)
{
    uint64_t t0 = plat_ticks_us();
    costs_us = trackers_us = 0; costs_n = trackers_n = 0;
    hook_call_original();
    FILE *f = trace_file();
    if (!f || ns_unwinding) return;
    {   /* K: a combat tick that took long (us in all, in ai_costs, in the game's trackers) */
        uint64_t dt = plat_ticks_us() - t0;
        if (dt > 4000) fprintf(f, "K %u %u %u %d %u %d\n", g_rw(GV_combat_clock), (unsigned)dt, (unsigned)costs_us, costs_n, (unsigned)trackers_us, trackers_n);
    }
    unsigned clk = g_rw(GV_combat_clock);
    static unsigned last = 0xffffffffu;
    if (clk == last) return;                                 /* (the tick runs every frame: once a clock step) */
    last = clk;
    unsigned long long us = (unsigned long long)plat_ticks_us();
    for (int i = 0; i < 13; i++) {
        u16 c = CBT(i);
        if (g_rb((u16)(c + CB_STATUS)) != 1) continue;
        fprintf(f, "T %u %d %d %d %d %d %d %d %d %d %d %llu %u %u %d\n", clk, i, (s16)g_rw((u16)(c + CB_X)), (s16)g_rw((u16)(c + CB_Y)),
                (s16)g_rw((u16)(c + CB_ORDER)), g_rb((u16)(GV_cbt_moving + i)), (s8)g_rb((u16)(c + CB_TARGET)), (s8)g_rb((u16)(GV_cbt_action + i)),
                (s8)g_rb((u16)(GV_cbt_anim + i)), (s16)g_rw((u16)(c + CB_TO_X)), (s16)g_rw((u16)(c + CB_TO_Y)), us,
                g_rw((u16)(0xd5f6 + 2 * i)), g_rw((u16)(0xd97c + 2 * i)), g_rb((u16)(GV_cbt_path_step + i)));
    }
    fflush(f);
}

/* test command uwalk:i,x,y: combatant i walks to x, y (map pixels) as a click on the floor sends it */
void pathfix_test_walk(int i, int x, int y)
{
    if (i < 0 || i >= 13) return;
    u16 c = CBT(i);
    g_ww((u16)(c + CB_TO_X), (u16)x); g_ww((u16)(c + CB_TO_Y), (u16)y); g_wb((u16)(c + CB_TARGET), 0xff);
    int r = ns_call(GF_path_find, 1, i);
    if (ns_unwinding) return;
    if (r != 1) { fprintf(stderr, "walk: %d finds no way to %d,%d\n", i, x, y); return; }
    ns_call(GF_order_queue_clear, 1, i);
    if (ns_unwinding) return;
    ns_call(GF_path_give, 1, i);
    if (ns_unwinding) return;
    g_wb((u16)(GV_cbt_moving + i), 1);
    g_ww((u16)(c + CB_ORDER), 1);
    g_wb((u16)(GV_cbt_path_step + i), 0);
    g_ww((u16)(D_REPLAN + 2 * i), (u16)(g_rw(GV_combat_clock) + 30));
    fprintf(stderr, "walk: %d from %d,%d to %d,%d\n", i, (s16)g_rw((u16)(c + CB_X)), (s16)g_rw((u16)(c + CB_Y)), x, y);
}

void pathfix_init(void)
{
    if (getenv("DL_BATTLE_TRACE")) { hook_install(GF_combat_tick, tick_trace_hook); hook_install(GF_ai_costs, costs_time_hook); hook_install(GF_path_trackers, trackers_time_hook); }
    battle_speed_set(config_get_int("battle.speed", 100));
    f_collide = game_func(GF_cbt_collisions);
    f_set = game_func(GF_cbt_anim_set);
    f_waypoints = game_func(14, 0x1db7, 0x0158);
    f_cursor = game_func(GF_ems_map_page);
    if (!f_collide || !f_set) fprintf(stderr, "pathfix: game functions not found\n");
    int on = config_get_int("fix.pathfinding", 1);          /* (one switch for the party and the foes since 1.3; fix.pathfinding_enemies is no more) */
    pathfix_set(on, on);
}
