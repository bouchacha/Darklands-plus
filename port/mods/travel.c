/* travel.c - travelling on the overland map by itself, along roads and over bridges.
 *
 * The original walks in a straight line towards a clicked point: into rivers and around
 * nothing, and it forgets the point when the view scrolls. Here a destination (a location,
 * or a map tile Ctrl+clicked) gets a route: Dijkstra over the map's tiles with the game's
 * own travel times (s030 1462:2ac8: minutes per step by terrain; roads are fastest), never
 * through ocean or major rivers except at bridges and fords, and around other locations
 * (stepping on one would enter it). The route is then walked by feeding the overland loop's
 * one-step function (s030 1462:0c44) the keypad direction towards the next tile, exactly as
 * if the player held the key, so time, encounters and arrival work as in the original.
 * A random encounter only interrupts: back on the map the party carries on. A key or a
 * click of the player's ends it.
 *
 * Map: DARKLAND.MAP (big-endian width 327, height 931, row offsets, RLE rows: count<<5 |
 * terrain). Odd rows are shifted half a tile to the right, so a tile has six neighbours:
 * east/west (16 one-pixel steps) and four diagonals (8 steps). Marker (D_2866, D_2868),
 * view origin column D_286a, buffer row D_286e, view row D_286c; tile of the marker:
 * row (D_2868+11)/4 + D_286e + D_286c - 1, column D_286a + (D_2866 + (odd row ? 16 : 24))/16 - 1. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "travel.h"

extern char game_dir[1024];

static unsigned char *terr;
static int mw, mh;
TravelState travel;
int travel_enabled = 1, travel_fast = 1;          /* fast: 0 normal, 1 4x, 2 instant */

/* minutes per one-pixel step by terrain (s030 1462:2ac8) */
static int minutes(int t)
{
    switch (t) {
    case 24: return 6;
    case 10: return 10; case 9: return 11; case 11: return 12; case 7: return 13; case 12: return 14;
    case 13: return 16; case 6: return 17; case 14: return 18; case 18: return 20; case 15: return 27;
    case 3: case 19: return 30; case 1: case 2: case 16: return 40; case 20: return 45;
    case 4: case 5: return 50; case 17: return 60; case 21: return 67; case 22: return 98; case 23: return 197;
    default: return 9;                                /* plains, farmland, fords, bridges, sites */
    }
}

static int load_map(void)
{
    if (terr) return 1;
    char p[1200]; snprintf(p, sizeof p, "%s/DARKLAND.MAP", game_dir);
    FILE *f = fopen(p, "rb");
    if (!f) { snprintf(p, sizeof p, "%s/darkland.map", game_dir); f = fopen(p, "rb"); }
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *d = malloc((size_t)n);
    if (!d || fread(d, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(d); return 0; }
    fclose(f);
    mw = d[0] << 8 | d[1]; mh = d[2] << 8 | d[3];
    if (mw <= 0 || mh <= 0 || mw > 1000 || mh > 2000 || 4 + 4 * mh > n) { free(d); return 0; }
    terr = calloc((size_t)mw * mh, 1);
    for (int y = 0; y < mh; y++) {
        long o = d[4 + 4 * y] | d[5 + 4 * y] << 8 | d[6 + 4 * y] << 16 | (long)d[7 + 4 * y] << 24;
        int x = 0;
        while (x < mw && o < n) {
            int b = d[o++], c = b >> 5;
            if (!c) break;
            while (c-- && x < mw) terr[y * mw + x++] = (unsigned char)(b & 31);
        }
    }
    free(d);
    return 1;
}

int travel_terrain(int x, int y) { return terr && x >= 0 && y >= 0 && x < mw && y < mh ? terr[y * mw + x] : -1; }

/* ---------------------------------------------------------------- routes */
static int *dist, *prev;
static unsigned char *site;           /* tiles of locations (stepping on one enters it) */
static int field_x = -1, field_y = -1, field_seq;

static void mark_sites(void)
{
    if (!site) site = calloc((size_t)mw * mh, 1);
    memset(site, 0, (size_t)mw * mh);
    for (int i = 0; i < G_NLOC; i++) {
        u32 r = g_location(i);
        if (!r) continue;
        int x = (s16)rw((u16)(r >> 16), (u16)(r + 4)), y = (s16)rw((u16)(r >> 16), (u16)(r + 6));
        if (x >= 0 && y >= 0 && x < mw && y < mh) site[y * mw + x] = 1;
    }
}

static int blocked(int t) { return t == 1 || t == 2; }                 /* ocean, major river */
static int is_site(int idx) { int t = terr[idx]; return site[idx] || t == 26 || t == 28 || t == 29 || t == 30 || t == 31; }

typedef struct { int d, i; } HE;
static HE *hp; static int hn, hc;
static void hpush(int d, int i)
{
    if (hn == hc) { hc = hc ? hc * 2 : 65536; hp = realloc(hp, (size_t)hc * sizeof *hp); }
    int k = hn++;
    while (k) { int p = (k - 1) / 2; if (hp[p].d <= d) break; hp[k] = hp[p]; k = p; }
    hp[k].d = d; hp[k].i = i;
}
static HE hpop(void)
{
    HE t = hp[0], l = hp[--hn]; int k = 0;
    for (;;) { int c = 2 * k + 1; if (c >= hn) break; if (c + 1 < hn && hp[c + 1].d < hp[c].d) c++; if (hp[c].d >= l.d) break; hp[k] = hp[c]; k = c; }
    if (hn) hp[k] = l;
    return t;
}

static int nb(int x, int y, int k, int *nx, int *ny, int *steps)
{
    int odd = y & 1;
    static const int DXE[2] = { 1, -1 };
    if (k < 2) { *nx = x + DXE[k]; *ny = y; *steps = 16; }
    else {
        int dy = k < 4 ? -1 : 1, right = k & 1;
        *nx = x - 1 + odd + right; *ny = y + dy; *steps = 8;
    }
    return *nx >= 0 && *ny >= 0 && *nx < mw && *ny < mh;
}

/* travel times (minutes) from (x, y) to every tile; locations block except as destinations */
static void field(int x0, int y0)
{
    size_t n = (size_t)mw * mh;
    if (!dist) { dist = malloc(n * sizeof *dist); prev = malloc(n * sizeof *prev); }
    for (size_t k = 0; k < n; k++) { dist[k] = 0x7fffffff; prev[k] = -1; }
    mark_sites();
    int s = y0 * mw + x0;
    dist[s] = 0; hn = 0; hpush(0, s);
    while (hn) {
        HE e = hpop();
        if (e.d != dist[e.i]) continue;
        int x = e.i % mw, y = e.i / mw;
        if (e.i != s && is_site(e.i)) continue;           /* one can arrive at a location, not pass it */
        for (int k = 0; k < 6; k++) {
            int nx, ny, st;
            if (!nb(x, y, k, &nx, &ny, &st)) continue;
            int ni = ny * mw + nx, t = terr[ni];
            if (blocked(t)) continue;
            int d = e.d + st * (minutes(terr[e.i]) + minutes(t)) / 2;
            if (d < dist[ni]) { dist[ni] = d; prev[ni] = e.i; hpush(d, ni); }
        }
    }
    field_x = x0; field_y = y0; field_seq++;
}

void travel_party_tile(int *x, int *y)
{
    int rowk = ((s16)g_rw(0x2868) + 11) / 4;
    int py = rowk + (s16)g_rw(0x286e) + (s16)g_rw(0x286c) - 1;
    int cold = (((py & 1) ? (s16)g_rw(0x2866) + 16 : (s16)g_rw(0x2866) + 24)) / 16;
    *x = (s16)g_rw(0x286a) + cold - 1; *y = py;
}

static int map_ready(void) { return game_mode() == GM_MAP && load_map(); }

/* minutes from the party to tile (x, y), -1 if unreachable; fields are cached per party tile */
int travel_minutes(int x, int y)
{
    if (!map_ready()) return -2;
    int px, py; travel_party_tile(&px, &py);
    if (px < 0 || py < 0 || px >= mw || py >= mh) return -1;
    if (px != field_x || py != field_y) field(px, py);
    if (x < 0 || y < 0 || x >= mw || y >= mh) return -1;
    int d = dist[y * mw + x];
    return d == 0x7fffffff ? -1 : d;
}

static int plan(int tx, int ty)
{
    int px, py; travel_party_tile(&px, &py);
    if (px != field_x || py != field_y) field(px, py);
    int g = ty * mw + tx;
    if (dist[g] == 0x7fffffff) return 0;
    int n = 0;
    for (int i = g; i >= 0 && n < TRAVEL_MAXROUTE; i = prev[i]) n++;
    travel.n = n; travel.i = 1;                        /* route[0] is where the party stands */
    for (int i = g, k = n - 1; i >= 0 && k >= 0; i = prev[i], k--) { travel.rx[k] = i % mw; travel.ry[k] = i / mw; }
    travel.minutes = dist[g];
    return 1;
}

int travel_to_tile(int x, int y, int loc, const char *name)
{
    if (!map_ready()) return 0;
    if (x < 0 || y < 0 || x >= mw || y >= mh) return 0;
    int px, py; travel_party_tile(&px, &py);
    field(px, py);
    if (!plan(x, y)) return 0;
    travel.active = 1; travel.loc = loc; travel.tx = x; travel.ty = y;
    snprintf(travel.name, sizeof travel.name, "%s", name ? name : "");
    travel.replan = 0;
    if (getenv("DL_TRAVEL_LOG")) fprintf(stderr, "travel: to %s (%d,%d) from (%d,%d): %d tiles, %d minutes\n", travel.name, x, y, px, py, travel.n, travel.minutes);
    return 1;
}

void travel_stop(void) { travel.active = 0; }

/* a location is entered by stepping on any of its tiles (a city covers several): the goal is
 * the nearest reachable tile of it, or its own tile */
int travel_minutes_loc(int loc, int *gx, int *gy)
{
    u32 r = g_location(loc);
    if (!r || travel_minutes(0, 0) == -2) return -1;     /* also refreshes the field */
    int lx = (s16)rw((u16)(r >> 16), (u16)(r + 4)), ly = (s16)rw((u16)(r >> 16), (u16)(r + 6));
    int best = -1;
    for (int y = ly - 4; y <= ly + 5; y++)
        for (int x = lx - 4; x <= lx + 5; x++) {
            if (x < 0 || y < 0 || x >= mw || y >= mh) continue;
            int t = terr[y * mw + x];
            if (!(x == lx && y == ly) && t != 29 && t != 28) continue;
            if (t == 29 && (abs(x - lx) > 4 || abs(y - ly) > 4)) continue;
            int d = travel_minutes(x, y);
            if (d >= 0 && (best < 0 || d < best)) { best = d; if (gx) *gx = x; if (gy) *gy = y; }
        }
    return best;
}

int travel_to_loc(int loc, const char *name)
{
    int gx = -1, gy = -1;
    if (travel_minutes_loc(loc, &gx, &gy) < 0) return 0;
    return travel_to_tile(gx, gy, loc, name);
}

/* ---------------------------------------------------------------- walking the route */
static void tile_of(int mx, int my, int *tx, int *ty)
{
    int rowk = (my + 11) / 4;
    int py = rowk + (s16)g_rw(0x286e) + (s16)g_rw(0x286c) - 1;
    *ty = py;
    *tx = (s16)g_rw(0x286a) + ((py & 1) ? mx + 16 : mx + 24) / 16 - 1;
}

static int marker_target(int tx, int ty, int *mx, int *my)
{
    int vx = (s16)g_rw(0x286a), vr = (s16)g_rw(0x286e) + (s16)g_rw(0x286c);
    *my = 4 * (ty - vr) - 6;
    *mx = 16 * (tx - vx) + ((ty & 1) ? 7 : -1);
    return 1;
}

static void step_hook(void)                 /* s030 1462:0c44 (key): one step of the overland loop */
{
    if (travel.active) {
        u16 key = rw(SS, (u16)(SP + 4));
        if (key != 0x4c35) { travel.active = 0; if (getenv("DL_TRAVEL_LOG")) fprintf(stderr, "travel: stopped by key %04x\n", key); }   /* the player took over */
        else {
            int px, py; travel_party_tile(&px, &py);
            if (travel.replan) { travel.replan = 0; field(px, py); if (!plan(travel.tx, travel.ty)) travel.active = 0; }
            /* where on the route are we? (after an encounter the party may stand elsewhere) */
            int found = -1;
            for (int k = travel.i - 1; k < travel.n && k < travel.i + 3; k++) if (k >= 0 && travel.rx[k] == px && travel.ry[k] == py) found = k;
            if (found < 0) { field(px, py); if (!plan(travel.tx, travel.ty)) travel.active = 0; found = 0; }
            if (travel.active) {
                travel.i = found + 1;
                if (travel.i >= travel.n) { travel.active = 0; if (getenv("DL_TRAVEL_LOG")) fprintf(stderr, "travel: arrived at (%d,%d)\n", px, py); }   /* arrived (a location is entered by the game) */
            }
            if (travel.active) {
                int mx, my; marker_target(travel.rx[travel.i], travel.ry[travel.i], &mx, &my);
                int x = (s16)g_rw(0x2866), y = (s16)g_rw(0x2868);
                /* the marker's tile is a rectangular approximation of the hex grid: a straight
                 * line between two tiles' centres can graze a third tile (a town would then be
                 * entered). Of the eight keys take the one that gets closest to the next tile's
                 * centre while every position it can lead to stays on this tile or the next. */
                static const u16 KEYS[8] = { 0x4700, 0x4800, 0x4900, 0x4b00, 0x4d00, 0x4f00, 0x5000, 0x5100 };
                static const int KX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 }, KY[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
                u16 k = 0x4c35; long bestd = -1;
                int nx = travel.rx[travel.i], ny = travel.ry[travel.i];
                for (int c = 0; c < 8; c++) {
                    int ok = 1;
                    for (int v = 0; v < 2 && ok; v++) {               /* vertical steps happen every other call */
                        int qx = x + KX[c], qy = y + (v ? KY[c] : 0), tx, ty;
                        tile_of(qx, qy, &tx, &ty);
                        if (!((tx == px && ty == py) || (tx == nx && ty == ny))) ok = 0;
                    }
                    if (!ok) continue;
                    long ddx = mx - (x + KX[c]), ddy = my - (y + KY[c]);
                    long d = ddx * ddx + 4 * ddy * ddy;
                    if (bestd < 0 || d < bestd) { bestd = d; k = KEYS[c]; }
                }
                if (bestd < 0) {                                       /* no safe key: head straight for it */
                    if (x > mx && y > my) k = 0x4700; else if (x > mx && y < my) k = 0x4f00;
                    else if (x < mx && y > my) k = 0x4900; else if (x < mx && y < my) k = 0x5100;
                    else if (x > mx) k = 0x4b00; else if (x < mx) k = 0x4d00;
                    else if (y > my) k = 0x4800; else if (y < my) k = 0x5000;
                }
                ww(SS, (u16)(SP + 4), k);
                int left = 0, from = py * mw + px;
                (void)from;
                if (field_x >= 0) { int d = dist[travel.ty * mw + travel.tx], here = dist[py * mw + px]; left = d - (here == 0x7fffffff ? 0 : here); }
                travel.left = left;
            }
        }
    }
    hook_call_original();
    int r = (s16)AX;
    if (travel.active && r >= 0 && getenv("DL_TRAVEL_LOG")) { int px, py; travel_party_tile(&px, &py); fprintf(stderr, "travel: step result %d at (%d,%d), route %d/%d\n", r, px, py, travel.i, travel.n); }
    if (travel.active && r >= 0) {
        if (r == 9999) travel.replan = 1;                     /* water in the way: plan again */
        else if (r < G_NLOC) travel.active = 0;               /* entered a location (the destination or not) */
        /* 1000 / 2000: an encounter; the walk goes on when the map is back */
    }
}

static void delay_hook(void)                /* s030 1767:0046: the pause between steps */
{
    static int n;
    /* travel_fast: 0 the game's pace, 1 four times as fast, 2 at once */
    if (travel.active && (travel_fast >= 2 || (travel_fast == 1 && (n++ & 3)))) { AX = 0; hook_return_far(0); return; }
    hook_call_original();
}

void travel_set(int on, int fast)
{
    travel_enabled = on; travel_fast = fast;
    config_set_int("travel.auto", on); config_set_int("travel.fast", fast);
    if (on) { hook_install(30, 0x1462, 0x0c44, step_hook); hook_install(30, 0x1767, 0x0046, delay_hook); }
    else { hook_remove(30, 0x1462, 0x0c44); hook_remove(30, 0x1767, 0x0046); travel.active = 0; }
}

void travel_init(void)
{
    travel_set(config_get_int("travel.auto", 1), config_get_int("travel.fast", 1));
}

/* Ctrl+click at game pixel (gx, gy) on the travel map */
void travel_click(int gx, int gy)
{
    if (!travel_enabled || !map_ready() || gx < 62) return;
    int mx = gx - 0x41, my = gy - 0x15;                       /* marker coordinates, as the game takes a click */
    int rowk = (my + 11) / 4;
    int ty = rowk + (s16)g_rw(0x286e) + (s16)g_rw(0x286c) - 1;
    int tx = (s16)g_rw(0x286a) + ((ty & 1) ? mx + 16 : mx + 24) / 16 - 1;
    /* a location there? then travel to it */
    for (int i = 0; i < G_NLOC; i++) {
        u32 r = g_location(i);
        if (!r) continue;
        int lx = (s16)rw((u16)(r >> 16), (u16)(r + 4)), ly = (s16)rw((u16)(r >> 16), (u16)(r + 6));
        if (abs(lx - tx) <= 1 && abs(ly - ty) <= 1) {
            char b[24]; g_str((u16)(r >> 16), (u16)(r + 0x26), b, sizeof b);
            if (travel_to_loc(i, b)) return;
        }
    }
    if (blocked(travel_terrain(tx, ty))) return;
    travel_to_tile(tx, ty, -1, "the marked place");
}

/* tests: travel to the nearest location of a type (6 = mine ...) */
int travel_nearest(int type)
{
    int best = -1, bm = 0;
    for (int i = 0; i < G_NLOC; i++) {
        u32 r = g_location(i);
        if (!r || rw((u16)(r >> 16), (u16)r) != type) continue;
        int m = travel_minutes_loc(i, 0, 0);
        if (m >= 0 && (best < 0 || m < bm)) { best = i; bm = m; }
    }
    if (best < 0) return 0;
    if (getenv("DL_TRAVEL_LOG")) fprintf(stderr, "travel: nearest of type %d is %d (%d minutes)\n", type, best, bm);
    return travel_to_loc(best, "nearest");
}
