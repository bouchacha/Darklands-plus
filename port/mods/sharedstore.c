/* sharedstore.c - one store room for every inn (store.shared, F12 > Helpers; on by default): what
 * is left with one innkeeper (a town's inn, a village's old woman) is there at every other, like
 * a chest that follows the party.
 *
 * The game keeps each store as a numbered cache in CACHE.TMP (saved with the game; s035 cache_load
 * / cache_save) and a location's record +0x18 says which (-1 none). Here every location points at
 * the same one:
 *   - when a store room opens (cache_load(handle), s035 1462:1350, from the screen's first draw),
 *     the caches of all places are merged into the one most places use (or this place's), the
 *     others emptied and their places pointed at it; this place too. A cache that would not fit
 *     (the open store holds 100 kinds, 127 of one thing a row: far 2e38:37f4) stays where it is,
 *     and its place keeps it until there is room;
 *   - when it closes (cache_close, 1462:04bc: written back, or the place marked as having none
 *     when it is empty), every place that pointed at the old cache follows this one's new
 *     number, so an emptied store is empty everywhere.
 * The game's own G and P, the screen and the saving are unchanged, but P is refused when the
 * store would hold more than 100 kinds or 127 of one thing (fixes.c storeroom, applied here too
 * when that fix is off: pooled in one store, the game's limits matter; its cache_save reads a
 * cache's count as signed, so from 128 kinds it would corrupt CACHE.TMP). A store already
 * holding more than 100 kinds (from before) is never merged nor written back by this. The towns
 * are pointed at the shared store once the location table is loaded (world_load, s043
 * 1462:0100) and when the first store is made. Turning the switch off keeps what is already
 * shared (the places point at one cache) but merges nothing more. */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "../runtime/hle.h"

#define F_STORE 0x37f4                       /* cache_items, far 2e38: { int id; char quality; char count; } */
#define STORE_MAX 100                        /* (the next table starts at 3984) */
#define LOC_CACHE 0x18

void ui_notice(const char *msg);
static int busy;
static void point_towns(void);

static int on(void) { return config_get_int("store.shared", 1) && !config_get_int("original", 0); }
static int loc_cache(int k) { u32 r = g_location(k); return r ? (s16)rw((u16)(r >> 16), (u16)((u16)r + LOC_CACHE)) : -1; }
static void loc_set_cache(int k, int h) { u32 r = g_location(k); if (r) ww((u16)(r >> 16), (u16)((u16)r + LOC_CACHE), (u16)h); }

typedef struct { u16 id; u8 q, n; } Ent;
static int store_read(Ent *e)                 /* the open store; -1 when it holds more than the table (never written back) */
{
    int n = (s16)g_rw(GV_cache_n); if (n < 0 || n > STORE_MAX) return -1;
    for (int i = 0; i < n; i++) {
        u16 o = (u16)(F_STORE + 4 * i);
        e[i].id = rw(G_FAR2E38, o); e[i].q = rb(G_FAR2E38, (u16)(o + 2)); e[i].n = rb(G_FAR2E38, (u16)(o + 3));
    }
    return n;
}
static void store_write(const Ent *e, int n)
{
    for (int i = 0; i < n; i++) {
        u16 o = (u16)(F_STORE + 4 * i);
        ww(G_FAR2E38, o, e[i].id); wb(G_FAR2E38, (u16)(o + 2), e[i].q); wb(G_FAR2E38, (u16)(o + 3), e[i].n);
    }
    g_ww(GV_cache_n, (u16)n); g_ww(GV_cache_n_shown, (u16)n);   /* (cache_save writes the "shown" count) */
}

/* cache h's things into the table t (n of them); 0 if they would not all fit */
static int add_into(Ent *t, int *n, const Ent *a, int na)
{
    for (int i = 0; i < na; i++) {
        int left = a[i].n;
        for (int j = 0; j < *n && left > 0; j++)
            if (t[j].id == a[i].id && t[j].q == a[i].q && t[j].n < 127) { int k = 127 - t[j].n < left ? 127 - t[j].n : left; t[j].n = (u8)(t[j].n + k); left -= k; }
        while (left > 0) {
            if (*n >= STORE_MAX) return 0;
            int k = left > 127 ? 127 : left;
            t[*n].id = a[i].id; t[*n].q = a[i].q; t[*n].n = (u8)k; (*n)++; left -= k;
        }
    }
    return 1;
}

/* the number of caches CACHE.TMP has (its byte 1), -1 unreadable */
static int caches_in_file(void)
{
    char path[1200]; dos_host_path("CACHE.TMP", path, sizeof path);
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    unsigned char b[2]; int n = fread(b, 1, 2, f) == 2 ? b[1] : -1;
    fclose(f);
    return n;
}

/* merge every place's cache into one; returns the one this place should open (-1 none) */
static int merge(int cur)
{
    static int refs[256];
    int nfile = caches_in_file();
    for (int h = 0; h < 256; h++) refs[h] = 0;
    for (int k = 0; k < G_NLOC; k++) { int h = loc_cache(k); if (h >= 1 && h < 256 && h <= nfile) refs[h]++; }
    int mine = loc_cache(cur), H = -1;
    for (int h = 1; h < 256; h++)
        if (refs[h] && (H < 0 || refs[h] > refs[H] || (refs[h] == refs[H] && h == mine))) H = h;
    if (H < 0) return mine;
    int stuck = 0;
    for (int h = 1; h < 256; h++) {
        if (!refs[h] || h == H) continue;
        static Ent a[STORE_MAX], t[STORE_MAX];
        ns_call(GF_cache_load, 1, h); if (ns_unwinding) return mine;
        int na = store_read(a);
        ns_call(GF_cache_load, 1, H); if (ns_unwinding) return mine;
        int nt = store_read(t);
        if (na < 0 || nt < 0) { if (h == mine) stuck = 1; fprintf(stderr, "store: cache %d or %d holds more than %d kinds: not merged\n", h, H, STORE_MAX); continue; }
        if (!add_into(t, &nt, a, na)) { if (h == mine) stuck = 1; fprintf(stderr, "store: cache %d (%d kinds) does not fit into the shared %d\n", h, na, H); continue; }
        store_write(t, nt);
        ns_call(GF_cache_save, 1, H); if (ns_unwinding) return mine;
        ns_call(GF_cache_load, 1, h); if (ns_unwinding) return mine;
        store_write(t, 0);
        ns_call(GF_cache_save, 1, h); if (ns_unwinding) return mine;      /* (emptied: no place points at it any more) */
        for (int k = 0; k < G_NLOC; k++) if (loc_cache(k) == h) loc_set_cache(k, H);
        fprintf(stderr, "store: cache %d (%d kinds) joined the shared %d (%d kinds)\n", h, na, H, nt);
    }
    if (stuck) { ui_notice("This inn's old store holds too many kinds of things to join the shared store room: it stays here until some are taken out."); return mine; }
    loc_set_cache(cur, H);
    return H;
}

static void load_hook(void)                   /* cache_load(handle), s035 1462:1350 */
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (busy || !on() || cur < 0 || cur >= G_NLOC) { hook_call_original(); return; }
    busy = 1;
    ns_unwinding = 0;                         /* (only merge's own calls may set it now) */
    int h = merge(cur);
    busy = 0;
    if (ns_unwinding) return;                 /* (the game unwinds: nothing more) */
    ww(SS, (u16)(SP + 4), (u16)h);
    hook_call_original();
}

static void close_hook(void)                  /* cache_close(r), s035 1462:04bc */
{
    int cur = (s16)g_rw(G_CUR_LOC);
    int before = cur >= 0 && cur < G_NLOC ? loc_cache(cur) : -1;
    hook_call_original();
    if (busy || cur < 0 || cur >= G_NLOC) return;
    int after = loc_cache(cur);
    if (before < 1) { if (after >= 1) point_towns(); return; }     /* (the first store: every town's) */
    if (after == before) return;
    u16 ax = AX;
    if (after < 1 && on()) {
        /* emptied: kept as an empty cache under its number (the game would number a new one the
           next time, and CACHE.TMP's table of them overruns its first cache from the 50th) */
        busy = 1;
        g_ww(GV_cache_n, 0); g_ww(GV_cache_n_shown, 0);
        ns_call(GF_cache_save, 1, before);
        busy = 0;
        if (!ns_unwinding) { loc_set_cache(cur, before); fprintf(stderr, "store: emptied (cache %d kept)\n", before); }
        AX = ax;
        return;
    }
    int n = 0;
    for (int k = 0; k < G_NLOC; k++) if (loc_cache(k) == before) { loc_set_cache(k, after); n++; }
    if (n) fprintf(stderr, "store: %d other place%s now with cache %d (was %d)\n", n, n == 1 ? "" : "s", after, before);
    AX = ax;
}

/* the cache most places point at (-1 none) */
static int shared_handle(void)
{
    static int refs[256];
    int H = -1;
    for (int h = 0; h < 256; h++) refs[h] = 0;
    for (int k = 0; k < G_NLOC; k++) { int h = loc_cache(k); if (h >= 1 && h < 256 && ++refs[h] > (H < 0 ? 0 : refs[H])) H = h; }
    return H;
}

/* every town's inn points at the shared store, so that "recover items stored here" is there at
   each: when the location table has been loaded (world_load: a new game, a load, back from a
   battle) and when the first store is made */
#define NCITY 92                              /* (the towns are the first 92 locations, DARKLAND.CTY) */
static void point_towns(void)
{
    if (!on()) return;
    int H = shared_handle();
    if (H < 1 || H > caches_in_file()) return;
    int n = 0;
    for (int k = 0; k < NCITY && k < G_NLOC; k++) if (loc_cache(k) < 1) { loc_set_cache(k, H); n++; }
    if (n) fprintf(stderr, "store: %d town%s now with the shared store (cache %d)\n", n, n == 1 ? "" : "s", H);
}
static void world_hook(void)                  /* world_load(locfile), s043 1462:0100 */
{
    hook_call_original();
    u16 ax = AX, dx = DX;
    point_towns();
    AX = ax; DX = dx;
}

/* P refused where it would overflow the pooled store, when fixes.c's storeroom fix is off */
const char *storeroom_put_refusal(int key);
int storeroom_fix_on(void);
void ui_notice(const char *msg);
static void key_hook(void)                    /* cache_key(key), s035 1462:0a46 */
{
    const char *why = on() && !storeroom_fix_on() ? storeroom_put_refusal(rw(SS, (u16)(SP + 4)) & 0xff) : 0;
    if (why) { ui_notice(why); hook_return_far(0); return; }
    hook_call_original();
}

int sharedstore_on(void) { return on(); }
/* how many places share loc's store (0: it has none) */
int sharedstore_places(int loc)
{
    int h = loc >= 0 && loc < G_NLOC ? loc_cache(loc) : -1, n = 0;
    if (h < 1) return 0;
    for (int k = 0; k < G_NLOC; k++) if (loc_cache(k) == h) n++;
    return n;
}

void sharedstore_init(void)
{
    hook_install(GF_cache_load, load_hook);
    hook_install(GF_cache_close, close_hook);
    hook_install(GF_world_load, world_hook);
    hook_install(GF_cache_key, key_hook);
}
