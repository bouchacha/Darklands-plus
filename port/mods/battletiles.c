/* The common loader copies the selected records to different segments every battle.
 * Bind those segments to source names, then paint at the game's clipped sprite call,
 * where floors, doors and fighters are already in depth order. */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "battletiles.h"
#include "hdart.h"
#include "hdtext.h"
#include "nscreen.h"
#include "../runtime/config.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/hooks.h"

#ifndef DL_RTSPLIT
extern u16 drvseg_mgraphic;
#endif
extern u8 vga_dac[768];
extern u8 *stbi_load(const char *, int *, int *, int *, int);
extern void stbi_image_free(void *);

#define ARG(k) rw(SS, (u16)(SP + 4 + 2 * (k)))
#define TILE_MAX 256    /* 15 slots x (1+2+4+2+4+2+1) frames, plus variable furniture */
#define PAGES 8
typedef struct {
    char name[40];
    u16 seg;
    int w, h, pic, generation;
    u8 *original, *indexed;
    u32 *rgb;
} Tile;
static Tile tiles[TILE_MAX + 1];
static u16 byseg[65536];
static int count, town, active, original, names, generation, enabled, variable_slot, tracking;
static int tile_log;

static void tracking_update(void)
{
    tracking = active && !original && names;
    if (!active || original || !enabled) return;
    for (int i = 1; i <= count && !tracking; i++)
        if (hdart_art_file(tiles[i].name)) tracking = 1;
}
/* One entry per game pixel. High word = tile slot (zero = other drawing),
 * low word = source pixel, so clipped redraws and copies keep their actual owner. */
static u32 owner[PAGES][320 * 200];
static u32 whole_owner[320 * 200];

void battletiles_snapshot(void) { if (tracking) memcpy(whole_owner, owner[0], sizeof whole_owner); }

static void clear_images(Tile *t)
{
    free(t->indexed); free(t->rgb);
    t->indexed = NULL; t->rgb = NULL;
}
static void clear_tiles(void)
{
    if (!count) return;
    for (int i = 1; i <= count; i++) { free(tiles[i].original); clear_images(&tiles[i]); }
    memset(tiles, 0, sizeof tiles);
    memset(byseg, 0, sizeof byseg);
    memset(owner, 0, sizeof owner);
    memset(whole_owner, 0, sizeof whole_owner);
    count = active = tracking = variable_slot = 0;
}

static void bind_tile(u16 seg, const char *set, const char *ext, int record, int variant, unsigned bytes)
{
    if (!seg || byseg[seg] || count == TILE_MAX || bytes < 2) return;
    int w = rb(seg, 0), h = rb(seg, 1);
    if (!w || !h) return;
    u8 *px = calloc((size_t)w * h, 1);
    if (!px) return;
    unsigned off = 2;
    for (int y = 0; y < h; y++) {
        if (off + 2 > bytes) { free(px); return; }
        int len = rb(seg, off), skip = rb(seg, off + 1);
        if (off + 2 + len > bytes) { free(px); return; }
        int n = len < w - skip ? len : w - skip;
        if (n > 0) memcpy(px + y * w + skip, M + LIN(seg, off + 2), n);
        off += 2 + len;
    }
    Tile *t = &tiles[++count];
    byseg[seg] = (u16)count;
    snprintf(t->name, sizeof t->name, "BT_%s%s_%s_%02X_%d", town ? "TOWN_" : "", set, ext, record, variant);
    t->seg = seg; t->w = w; t->h = h; t->original = px;
    t->pic = -1; t->generation = -1;
}

/* Offsets within the terrain descriptor at DS:9085 and the resulting segment tables
 * from battle_tiles_load (s002 0e76:0c8c). Effects use interleaved far-data pairs. */
static const struct { const char *ext; u8 set, slots, variants, paired; u16 tbl[4]; } groups[] = {
    {"FLC", 0x01, 0x0a, 1, 0, {0xc1d0}},
    {"FFC", 0x1a, 0x24, 2, 1, {0x3474}},
    {"NWC", 0x34, 0x3d, 4, 0, {0xc220, 0xc4dd, 0xc601, 0xc24c}},
    {"NFC", 0x4d, 0x57, 2, 1, {0x6eea}},
    {"WWC", 0x67, 0x70, 4, 0, {0xc29b, 0xc62c, 0xc658, 0xc26f}},
    {"WFC", 0x80, 0x8a, 2, 1, {0x6f2a}},
    {"FRC", 0x9a, 0xa4, 1, 0, {0xc8d5}}
};
#define GROUPS (sizeof groups / sizeof groups[0])
static u16 frame_bytes[GROUPS][16][4], furniture_bytes;
static int loading, loading_furniture;

/* The loader leaves each record's byte count in its scratch buffer only until the
 * next bank. Remember the bounds there: a few effect records point outside their copy. */
static void h_read_bank(void)
{
    if (!loading && !loading_furniture) { hook_call_original(); return; }
    char name[20]; g_str(G_DGROUP, ARG(0), name, sizeof name);
    u16 buf = ARG(1), seg = ARG(2);
    hook_call_original();
    const char *ext = strrchr(name, '.');
    if (!ext || unw_active) return;
    for (unsigned g = 0; g < GROUPS; g++) {
        if (strcasecmp(ext+1, groups[g].ext)) continue;
        int furniture = !strcmp(groups[g].ext, "FRC");
        int stride = furniture ? 40 : g == 0 ? 4 : g == 1 ? 6 : 10;
        for (int slot = 1; slot < (loading_furniture ? 2 : 16); slot++) {
            int record = loading_furniture ? rb(G_FAR20A5, 0x33d2) : (s8)g_rb(0x9085 + groups[g].slots + slot);
            if (record < 0) continue;
            u16 entry = buf + record * stride;
            unsigned size = rw(seg, entry + (furniture ? 34 : 0));
            unsigned base = rw(seg, entry + (furniture ? 38 : 2));
            if (loading_furniture) { furniture_bytes = size; return; }
            for (int v = 0; v < groups[g].variants; v++) {
                unsigned offset = v ? rw(seg, entry + 2 + v*2) : base;
                if (offset >= base && offset < base+size)
                    frame_bytes[g][slot][v] = (u16)(base+size-offset);
            }
        }
        break;
    }
}

static void h_tiles_load(void)
{
    clear_tiles();
    memset(frame_bytes, 0, sizeof frame_bytes);
    loading = 1;
    hook_call_original();
    loading = 0;
    if (unw_active) return;
    int kind = g_rw(GV_battlefield_kind);
    town = (kind >= 0x11 && kind <= 0x26) || (kind >= 0x29 && kind <= 0x2d) ||
           kind == 0x56 || kind == 0x6a || kind == 0x6b;
    for (int variant = 0; variant < 4; variant++) {
        for (unsigned g = 0; g < GROUPS; g++) {
            if (variant >= groups[g].variants) continue;
            char set[10];
            g_str(G_DGROUP, 0x9085 + groups[g].set, set, sizeof set);
            if (!set[0]) continue;
            for (char *p = set; *p; p++) *p = (char)toupper((unsigned char)*p);
            for (int slot = 1; slot < 16; slot++) {
                int record = (s8)g_rb(0x9085 + groups[g].slots + slot);
                if (record < 0) continue;
                u16 seg = groups[g].paired
                    ? rw(G_FAR20A5, groups[g].tbl[0] + slot * 4 + variant * 2)
                    : g_rw(groups[g].tbl[variant] + slot * 2);
                bind_tile(seg, set, groups[g].ext, record, variant, frame_bytes[g][slot][variant]);
            }
        }
    }
    active = count > 0;
    enabled = hdart_on; original = config_get_int("original", 0); names = hdart_show_names;
    generation = hdart_generation; tracking_update();
    if (tile_log) fprintf(stderr, "battle art: battlefield %#x, %d tile segments%s\n", kind, count, town ? " (town)" : "");
}

static void h_furniture_load(void)
{
    loading_furniture = 1; furniture_bytes = 0;
    hook_call_original();
    loading_furniture = 0;
    if (!active || unw_active) return;
    u16 seg = g_rw(0xd6ee);
    if (!seg) return;
    if (variable_slot) {
        Tile *old = &tiles[variable_slot];
        byseg[old->seg] = 0;
        free(old->original); clear_images(old); memset(old, 0, sizeof *old);
        count--; variable_slot = 0;
    }
    if (byseg[seg]) return;
    char set[10]; g_str(G_FAR20A5, 0x33c8, set, sizeof set);
    for (char *p = set; *p; p++) *p = (char)toupper((unsigned char)*p);
    bind_tile(seg, set, "FRC", rb(G_FAR20A5, 0x33d2), 0, furniture_bytes);
    variable_slot = byseg[seg];
    tracking_update();
}

static void h_input(void)
{
    int orig = config_get_int("original", 0);
    if (active && (enabled != hdart_on || original != orig || names != hdart_show_names || generation != hdart_generation)) {
        int previously_tracking = tracking;
        enabled = hdart_on; original = orig; names = hdart_show_names; generation = hdart_generation;
        tracking_update();
        if (tracking || previously_tracking) {
            ns_call(GF_floor_view_draw, 1, -1);
            if (ns_unwinding) return;
            void plat_hold_until_drawn(void); plat_hold_until_drawn();
        }
    }
    hook_call_original();
}

static void h_battle(void)
{
    clear_tiles();
    hook_call_original();
    clear_tiles();
}

static int tile_pic(Tile *t)
{
    if (t->pic < 0) t->pic = hdart_register_tile(t->name, t->w, t->h, t->original);
    return t->pic;
}
void battletiles_templates(void)
{
    if (active) for (int i = 1; i <= count; i++) tile_pic(&tiles[i]);
}

static void load_art(Tile *t)
{
    if (t->generation == hdart_generation) return;
    clear_images(t); t->generation = hdart_generation;
    const char *path = hdart_art_file(t->name);
    if (!path) return;
    int w, h, comp;
    u8 *rgba = stbi_load(path, &w, &h, &comp, 4);
    if (!rgba) return;
    t->indexed = calloc((size_t)t->w * t->h, 1);
    t->rgb = calloc((size_t)t->w * t->h, sizeof *t->rgb);
    if (!t->indexed || !t->rgb) { clear_images(t); stbi_image_free(rgba); return; }
    for (int y = 0; y < t->h; y++) for (int x = 0; x < t->w; x++) {
        int off = y * t->w + x;
        const u8 *p = rgba + 4 * ((size_t)((long long)y * h / t->h) * w + (long long)x * w / t->w);
        if (p[3] < 128) continue;
        /* Also paint the nearest VGA colour into the actual page: the game copies it,
         * snapshots it, and may show it under other screens or effects. */
        int best = 1, distance = 0x7fffffff;
        for (int i = 1; i < 256; i++) {
            int r = (p[0] >> 2) - (vga_dac[3*i] & 63);
            int g = (p[1] >> 2) - (vga_dac[3*i+1] & 63);
            int b = (p[2] >> 2) - (vga_dac[3*i+2] & 63);
            int d = 3*r*r + 4*g*g + 2*b*b;
            if (d < distance) { distance = d; best = i; }
        }
        t->indexed[off] = best;
        t->rgb[off] = 0xff000000u | ((u32)p[0] << 16) | ((u32)p[1] << 8) | p[2];
    }
    stbi_image_free(rgba);
    if (tile_log) fprintf(stderr, "battle art: %s %dx%d <- %s\n", t->name, t->w, t->h, path);
}

/* Driver windows have an origin and inclusive right/bottom relative to that origin. */
static int bounds(u16 win, int *x, int *y, int *w, int *h, int *sx, int *sy)
{
    int ox = (s16)g_rw(win+2), oy = (s16)g_rw(win+4);
    int right = ox + (s16)g_rw(win+6) + 1, bottom = oy + (s16)g_rw(win+8) + 1;
    *x += ox; *y += oy;
    if (ox < 0) ox = 0; if (oy < 0) oy = 0;
    if (right > 320) right = 320; if (bottom > 200) bottom = 200;
    if (*x < ox) { *sx = ox - *x; *w -= *sx; *x = ox; }
    if (*y < oy) { *sy = oy - *y; *h -= *sy; *y = oy; }
    if (*x + *w > right) *w = right - *x;
    if (*y + *h > bottom) *h = bottom - *y;
    return *w > 0 && *h > 0;
}

int battletiles_sprite(u16 win, int x, int y, u16 seg)
{
    if (!tracking || !seg) return 0;
    int page = (s16)g_rw(win);
    if (page < 0 || page >= PAGES) return 0;
    int slot = byseg[seg];
    Tile *t = &tiles[slot];
    if (slot && enabled) load_art(t);
    int replaced = slot && enabled && t->indexed;
    int w = rb(seg, 0), h = rb(seg, 1), sx = 0, sy = 0;
    if (slot && (w != t->w || h != t->h)) return 0;
    if (!bounds(win, &x, &y, &w, &h, &sx, &sy)) return replaced;
    u16 page_seg = page ? rw(drvseg_mgraphic, 0x19b0 + 2 * page) : 0xa000;
    if (!page_seg || LIN(page_seg, 0) + 64000 > MEM_SIZE) return 0;
    u8 *mem = M + LIN(page_seg, 0);
    if (slot) {
        const u8 *px = replaced ? t->indexed : t->original;
        for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) {
            int p = (sy+j) * t->w + sx+i;
            if (!px[p]) continue;
            int dst = (y+j)*320 + x+i;
            owner[page][dst] = ((u32)slot << 16) | (u16)p;
            if (replaced) mem[dst] = px[p];
        }
    } else { /* Fighter/loot sprites cover earlier scenery even when their colours coincide. */
        unsigned o = 2;
        for (int j = 0; j < sy+h; j++) {
            int n = rb(seg, o), start = rb(seg, o+1);
            if (j >= sy) {
                int begin = start < sx ? sx : start, end = start+n < sx+w ? start+n : sx+w;
                for (int i = begin; i < end; i++) if (rb(seg, o+2+i-start))
                    owner[page][(y+j-sy)*320+x+i-sx] = 0;
            }
            o += 2+n;
        }
    }
    return replaced;
}

/* MGRAPHIC slot 31 presents a whole offscreen page with a direct 64,000-byte
 * copy. Tile draws after this only repair the dirty rectangles around moving actors. */
static void h_show_page(void)
{
    int page = ARG(0);
    hook_call_original();
    if (tracking && page > 0 && page < PAGES) memcpy(owner[0], owner[page], sizeof owner[0]);
}

static void h_fill(void)
{
    if (tracking) {
        u16 win = ARG(0); int page = (s16)g_rw(win);
        int x = (s16)ARG(1), y = (s16)ARG(2), w = (s16)ARG(3), h = (s16)ARG(4), sx = 0, sy = 0;
        if (page >= 0 && page < PAGES && bounds(win, &x, &y, &w, &h, &sx, &sy))
            for (int j = 0; j < h; j++) memset(&owner[page][(y+j)*320+x], 0, w*sizeof(u32));
    }
    hook_call_original();
}

void battletiles_copy(int src, int sx, int sy, int dst, int dx, int dy, int w, int h)
{
    if (!tracking || src < 0 || src >= PAGES || dst < 0 || dst >= PAGES) return;
    if (sx < 0) { dx -= sx; w += sx; sx = 0; }
    if (sy < 0) { dy -= sy; h += sy; sy = 0; }
    if (sx+w > 320) w = 320-sx; if (sy+h > 200) h = 200-sy;
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++) {
        int row = src == dst && dy > sy ? h-1-j : j;
        memmove(&owner[dst][(dy+row)*320+dx], &owner[src][(sy+row)*320+sx], w*sizeof(u32));
    }
}

int battletiles_pick(int x, int y, int *px, int *py, int *pw, int *ph)
{
    if (!tracking) return -1;
    u32 tag = owner[0][y*320+x]; int slot = tag >> 16, offset = (u16)tag;
    if (!slot || slot > count) return -1;
    Tile *t = &tiles[slot]; const u8 *pxs = enabled && t->indexed ? t->indexed : t->original;
    int bx, by, bw, bh;
    u16 driver_ds = (u16)(drvseg_mgraphic + 0x140);
    u8 colour = rb(0xa000, y*320+x);
    /* The native pointer may still be painted at its hotspot. MGRAPHIC saves the
     * pixels under it at DS:0216; pick the visible tile through the pointer. */
    if (hdtext_game_cursor(&bx, &by, &bw, &bh) && rw(driver_ds, 0x214) == 0 &&
        bw * bh <= 0x200 && x >= bx && x < bx+bw && y >= by && y < by+bh)
        colour = rb(driver_ds, 0x216 + (y-by)*bw + x-bx);
    if (offset >= t->w*t->h || pxs[offset] != colour) return -1;
    *px = x-offset%t->w; *py = y-offset/t->w; *pw = t->w; *ph = t->h;
    return tile_pic(t);
}
void battletiles_colour(u32 *rgb, const u8 *idx)
{
    if (!tracking || !enabled) return;
    for (int i = 0; i < 320*200; i++) {
        u32 tag = whole_owner[i]; int slot = tag >> 16, offset = (u16)tag;
        if (!slot || slot > count) continue;
        Tile *t = &tiles[slot];
        if (t->rgb && offset < t->w*t->h && t->indexed[offset] == idx[i]) rgb[i] = t->rgb[offset];
    }
}

void battletiles_init(void)
{
    tile_log = getenv("DL_ART_LOG") != NULL;
    hook_install(GF_battle_run, h_battle);
    hook_install(GF_file_read_all, h_read_bank);
    hook_install(GF_battle_tiles_load, h_tiles_load);
    hook_install(GF_furniture_load, h_furniture_load);
    hook_install(-1, 0x047c, 0x059e, h_show_page);
    hook_install(GF_gfx_fill_rect, h_fill);
    hook_install(GF_combat_input, h_input);
}
