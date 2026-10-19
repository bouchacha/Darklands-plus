/* worldmap.c - the whole overland map in one picture (for the world map: M on the travel map or a card).
 * Drawn the way the travel screen draws its view (s030 1462:20d2), for every tile at once:
 * DARKLAND.MAP terrain, tiles of 16x12 from MAPICONS.PIC rows 1-15 (terrain 1-15) and
 * MAPICON2.PIC rows 0-15 (terrain 16-31) at (column*16 + 8 on odd rows, row*4), colour 0
 * transparent, the column chosen by which of the four diagonal neighbours continue the
 * terrain (1 up-left, 2 up-right, 4 down-left, 8 down-right; water, rivers, fords and bridges
 * count as one, so do the four marsh/geest kinds, roads join fords, bridges and sites).
 * Locations (records 92-413, except hidden ones: menu 0x62 or kind 15) replace their tile
 * by their icon (s030 1462:1c76). Empty tiles show the season's ground colour (D_2860). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "worldmap.h"

const unsigned char *travel_map(int *w, int *h);
u8 *hdart_decode_pic(const char *dosname, int *w, int *h, u32 *pal, u8 *mask);
extern u8 vga_dac[768];

WorldMap worldmap;
static u8 *sheet[2]; static int sw[2], sh[2];
static u32 sheetpal[256]; static u8 sheetmask[256];
static u32 mappal[256]; static int have_mappal;

static int load_sheets(void)
{
    if (sheet[0] && sheet[1]) return 1;
    memset(sheetmask, 0, sizeof sheetmask);
    for (int b = 0; b < 2; b++) {
        u32 pal[256]; u8 mask[256];
        free(sheet[b]);
        sheet[b] = hdart_decode_pic(b ? "PICS\\MAPICON2.PIC" : "PICS\\MAPICONS.PIC", &sw[b], &sh[b], pal, mask);
        if (!sheet[b]) return 0;
        for (int i = 0; i < 256; i++) if (mask[i] && !sheetmask[i]) { sheetpal[i] = pal[i]; sheetmask[i] = 1; }
    }
    return 1;
}

static u32 dac_rgb(int i)
{
    u32 r = vga_dac[i * 3] & 63, g = vga_dac[i * 3 + 1] & 63, b = vga_dac[i * 3 + 2] & 63;
    return 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (b << 2 | b >> 4);
}

/* the travel screen's colours (the seasons change some, s030 1462:2dd6): what the screen
 * shows when the world map is opened from the travel map */
static void take_palette(void)
{
    for (int i = 0; i < 256; i++) mappal[i] = dac_rgb(i);
    have_mappal = 1;
}
void worldmap_watch_palette(void) { }

static int loc_icon(int kind)
{
    switch (kind) {
    case 3: return 0x33;
    case 5: case 15: case 19: case 24: return 0x38;
    case 6: return 0x37;
    case 8: case 10: return 0x1d;
    case 9: return 0x3e;
    case 4: case 13: return 0x39;
    case 16: return 0x36;
    case 17: return 0x3b;
    case 18: return 0x35;
    case 20: return 0x3c;
    case 21: return 0x3d;
    case 22: return 0x3f;
    case 23: return 0x40;
    case 26: return 0x3a;
    default: return 0x1c;
    }
}

static int water(int t) { return t == 1 || t == 2 || t == 3 || t == 25 || t == 27; }
static int marsh(int t) { return t >= 4 && t <= 7; }

/* tile picture: ICONS[row][col] = sheet bank, its top-left corner */
static const u8 *icon(int lin, int *stride)
{
    int r = lin / 16, c = lin % 16;
    int b = r < 15 ? 0 : 1, sr = r < 15 ? r + 1 : r - 15;
    if (sr * 12 + 12 > sh[b] || c * 16 + 16 > sw[b]) return 0;
    *stride = sw[b];
    return sheet[b] + sr * 12 * sw[b] + c * 16;
}

int worldmap_build(void)
{
    int mw, mh;
    const unsigned char *terr = travel_map(&mw, &mh);
    if (!terr || !load_sheets()) return 0;
    u8 *g = malloc((size_t)mw * mh);
    memcpy(g, terr, (size_t)mw * mh);
    for (int k = 92; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int kind = (s16)rw(s, o), x = (s16)rw(s, (u16)(o + 4)), y = (s16)rw(s, (u16)(o + 6)), menu = (s16)rw(s, (u16)(o + 0x0c));
        if (menu == 0x62 || kind == 15 || x < 0 || y < 0 || x >= mw || y >= mh) continue;
        g[y * mw + x] = (u8)loc_icon(kind);
    }
    int W = mw * 16 + 8, H = (mh - 1) * 4 + 12;
    int month = (s16)g_rw(G_CUR_MONTH);
    if (game_mode() == GM_MAP) take_palette();
    int ground = (month <= 2 || month >= 10) ? 0x0f : (month == 3 || month == 8 || month == 9) ? 6 : 0x68;
    u8 *c = malloc((size_t)W * H);
    memset(c, ground, (size_t)W * H);
    #define G(yy, xx) (((yy) < 0 || (yy) >= mh || (xx) < 0 || (xx) >= mw) ? 0 : g[(yy) * mw + (xx)])
    for (int y = 0; y < mh; y++) {
        int odd = y & 1;
        for (int x = 0; x < mw; x++) {
            int te = g[y * mw + x], m = 0, lin;
            if (!te) continue;
            int a = G(y - 1, x - 1 + odd), b = G(y - 1, x + odd), d = G(y + 1, x - 1 + odd), e = G(y + 1, x + odd);
            if (water(te)) m = water(a) | water(b) << 1 | water(d) << 2 | water(e) << 3;
            else if (marsh(te)) m = marsh(a) | marsh(b) << 1 | marsh(d) << 2 | marsh(e) << 3;
            else if (te == 24) m = (a == 24 || a >= 25) | (b == 24 || b >= 25) << 1 | (d == 24 || d >= 25) << 2 | (e == 24 || e >= 25) << 3;
            else if (te < 50) m = (a == te) | (b == te) << 1 | (d == te) << 2 | (e == te) << 3;
            lin = te >= 50 ? te + 382 : (te - 1) * 16 + m;
            int stride; const u8 *p = icon(lin, &stride);
            if (!p) continue;
            int px = x * 16 + (odd ? 8 : 0), py = y * 4;
            for (int yy = 0; yy < 12; yy++) {
                u8 *q = c + (py + yy) * W + px;
                const u8 *s = p + yy * stride;
                for (int xx = 0; xx < 16; xx++) if (s[xx]) q[xx] = s[xx];
            }
        }
    }
    #undef G
    free(g);
    /* colours: the travel screen's own when it has been seen in daylight, else the sheets' */
    u32 pal[256];
    for (int i = 0; i < 256; i++) pal[i] = have_mappal ? mappal[i] : sheetmask[i] ? sheetpal[i] : dac_rgb(i);
    /* level 0 is half size (2x2 averaged), each further level half again */
    for (int l = 0; l < WM_LEVELS; l++) { free(worldmap.px[l]); worldmap.px[l] = 0; }
    int w = (W + 1) / 2, h = (H + 1) / 2;
    u32 *L = malloc(sizeof(u32) * (size_t)w * h);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            unsigned r = 0, gg = 0, b = 0, n = 0;
            for (int dy = 0; dy < 2; dy++) for (int dx = 0; dx < 2; dx++) {
                int sx = 2 * x + dx, sy = 2 * y + dy;
                if (sx >= W || sy >= H) continue;
                u32 v = pal[c[sy * W + sx]];
                r += v >> 16 & 255; gg += v >> 8 & 255; b += v & 255; n++;
            }
            L[y * w + x] = 0xff000000u | (r / n) << 16 | (gg / n) << 8 | (b / n);
        }
    free(worldmap.idx); worldmap.idx = c;              /* kept for the full-scale view */
    memcpy(worldmap.pal, pal, sizeof pal);
    worldmap.px[0] = L; worldmap.w[0] = w; worldmap.h[0] = h;
    for (int l = 1; l < WM_LEVELS; l++) {
        int pw = worldmap.w[l - 1], ph = worldmap.h[l - 1], nw = (pw + 1) / 2, nh = (ph + 1) / 2;
        u32 *P = worldmap.px[l - 1], *N = malloc(sizeof(u32) * (size_t)nw * nh);
        for (int y = 0; y < nh; y++)
            for (int x = 0; x < nw; x++) {
                unsigned r = 0, gg = 0, b = 0, n = 0;
                for (int dy = 0; dy < 2; dy++) for (int dx = 0; dx < 2; dx++) {
                    int sx = 2 * x + dx, sy = 2 * y + dy;
                    if (sx >= pw || sy >= ph) continue;
                    u32 v = P[sy * pw + sx];
                    r += v >> 16 & 255; gg += v >> 8 & 255; b += v & 255; n++;
                }
                N[y * nw + x] = 0xff000000u | (r / n) << 16 | (gg / n) << 8 | (b / n);
            }
        worldmap.px[l] = N; worldmap.w[l] = nw; worldmap.h[l] = nh;
    }
    worldmap.full_w = W; worldmap.full_h = H; worldmap.tiles_w = mw; worldmap.tiles_h = mh;
    worldmap.month = month;
    worldmap.stale = 0;
    worldmap.generation++;
    if (getenv("DL_WM_DUMP")) {
        int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
        u8 *rgb = malloc((size_t)w * h * 3);
        for (long i = 0; i < (long)w * h; i++) { u32 v = L[i]; rgb[i * 3] = (u8)(v >> 16); rgb[i * 3 + 1] = (u8)(v >> 8); rgb[i * 3 + 2] = (u8)v; }
        stbi_write_png(getenv("DL_WM_DUMP"), w, h, 3, rgb, w * 3);
        free(rgb);
        fprintf(stderr, "worldmap: %dx%d, palette %s, month %d, ground %d\n", w, h, have_mappal ? "from the travel screen" : "from the sheets", month, ground);
    }
    return 1;
}
