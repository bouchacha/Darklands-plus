/* platform_sdl.c - window, screen, input and time on SDL2.
 *
 * Test hooks (environment variables), used by the automated play tests:
 *   DL_SCRIPT="t:cmd;t:cmd;..."  t in ms since start; cmd is one of
 *        key:NAME        press+release an SDL key name (e.g. Return, Escape, a, F1)
 *        down:NAME / up:NAME
 *        move:X,Y        mouse position in game pixels (0..319, 0..199)
 *        click:X,Y       left click at X,Y   rclick:X,Y   right click
 *        shot:FILE.bmp   save the screen
 *        quit
 *   DL_HEADLESS=1        no window (SDL dummy video driver)
 *   DL_FAST=1            don't throttle presentation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include "cpu.h"
#include "hle.h"
#include "../ui/ui.h"
#include "../mods/hdart.h"
#include "../mods/hdtext.h"
#include "game.h"
#include "gamevars.h"
#include "config.h"

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;
static u32 frame[320 * 200];
static int headless;
static int mouse_x = 160, mouse_y = 100, mouse_b;
static int pad_test_axis[4];                   /* (game controllers, further down) */

/* ---------------------------------------------------------------- time */
static Uint64 t0, freq;
u32 plat_ticks_ms(void) { return (u32)((SDL_GetPerformanceCounter() - t0) * 1000 / freq); }
uint64_t plat_ticks_us(void) { return (uint64_t)((SDL_GetPerformanceCounter() - t0) * 1000000 / freq); }
void plat_sleep_ms(int ms) { SDL_Delay((Uint32)ms); }

static Uint32 tick_cb(Uint32 iv, void *p) { (void)p; poll_flag = 1; return iv; }

/* ---------------------------------------------------------------- script */
typedef struct { u32 t; char cmd[16]; char arg[200]; } Step;
static Step steps[512];
static int nsteps, step_i;

static void parse_script(const char *s)
{
    while (s && *s && nsteps < 512) {
        const char *e = strchr(s, ';');
        size_t n = e ? (size_t)(e - s) : strlen(s);
        char buf[256];
        if (n >= sizeof buf) n = sizeof buf - 1;
        memcpy(buf, s, n); buf[n] = 0;
        char *c1 = strchr(buf, ':');
        if (c1) {
            *c1 = 0;
            Step *st = &steps[nsteps++];
            st->t = (u32)atoi(buf);
            char *c2 = strchr(c1 + 1, ':');
            if (c2) { *c2 = 0; snprintf(st->arg, sizeof st->arg, "%s", c2 + 1); }
            snprintf(st->cmd, sizeof st->cmd, "%s", c1 + 1);
        }
        s = e ? e + 1 : 0;
    }
}

/* ---------------------------------------------------------------- keyboard map (XT set 1) */
static int xt_code(SDL_Scancode sc, int *ext)
{
    *ext = 0;
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        static const u8 t[26] = {0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,
                                 0x31,0x18,0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c};
        return t[sc - SDL_SCANCODE_A];
    }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) return 2 + (sc - SDL_SCANCODE_1);
    if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F10) return 0x3b + (sc - SDL_SCANCODE_F1);
    switch (sc) {
    case SDL_SCANCODE_RETURN: return 0x1c;
    case SDL_SCANCODE_ESCAPE: return 0x01;
    case SDL_SCANCODE_BACKSPACE: return 0x0e;
    case SDL_SCANCODE_TAB: return 0x0f;
    case SDL_SCANCODE_SPACE: return 0x39;
    case SDL_SCANCODE_MINUS: return 0x0c;
    case SDL_SCANCODE_EQUALS: return 0x0d;
    case SDL_SCANCODE_LEFTBRACKET: return 0x1a;
    case SDL_SCANCODE_RIGHTBRACKET: return 0x1b;
    case SDL_SCANCODE_BACKSLASH: return 0x2b;
    case SDL_SCANCODE_SEMICOLON: return 0x27;
    case SDL_SCANCODE_APOSTROPHE: return 0x28;
    case SDL_SCANCODE_GRAVE: return 0x29;
    case SDL_SCANCODE_COMMA: return 0x33;
    case SDL_SCANCODE_PERIOD: return 0x34;
    case SDL_SCANCODE_SLASH: return 0x35;
    case SDL_SCANCODE_F11: return 0x57;
    case SDL_SCANCODE_F12: return 0x58;
    case SDL_SCANCODE_LSHIFT: return 0x2a;
    case SDL_SCANCODE_RSHIFT: return 0x36;
    case SDL_SCANCODE_LCTRL: return 0x1d;
    case SDL_SCANCODE_LALT: return 0x38;
    case SDL_SCANCODE_RCTRL: *ext = 1; return 0x1d;
    case SDL_SCANCODE_RALT: *ext = 1; return 0x38;
    case SDL_SCANCODE_CAPSLOCK: return 0x3a;
    case SDL_SCANCODE_KP_7: return 0x47;
    case SDL_SCANCODE_KP_8: return 0x48;
    case SDL_SCANCODE_KP_9: return 0x49;
    case SDL_SCANCODE_KP_MINUS: return 0x4a;
    case SDL_SCANCODE_KP_4: return 0x4b;
    case SDL_SCANCODE_KP_5: return 0x4c;
    case SDL_SCANCODE_KP_6: return 0x4d;
    case SDL_SCANCODE_KP_PLUS: return 0x4e;
    case SDL_SCANCODE_KP_1: return 0x4f;
    case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_KP_3: return 0x51;
    case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_KP_PERIOD: return 0x53;
    case SDL_SCANCODE_KP_ENTER: *ext = 1; return 0x1c;
    case SDL_SCANCODE_KP_DIVIDE: *ext = 1; return 0x35;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;
    /* the game predates the separate cursor block: send the keypad codes */
    case SDL_SCANCODE_HOME: return 0x47;
    case SDL_SCANCODE_UP: return 0x48;
    case SDL_SCANCODE_PAGEUP: return 0x49;
    case SDL_SCANCODE_LEFT: return 0x4b;
    case SDL_SCANCODE_RIGHT: return 0x4d;
    case SDL_SCANCODE_END: return 0x4f;
    case SDL_SCANCODE_DOWN: return 0x50;
    case SDL_SCANCODE_PAGEDOWN: return 0x51;
    case SDL_SCANCODE_INSERT: return 0x52;
    case SDL_SCANCODE_DELETE: return 0x53;
    default: return -1;
    }
}

static void key(SDL_Scancode sc, int release)
{
    int ext, c = xt_code(sc, &ext);
    if (c < 0) return;
    if (ext) kbd_push_scancode(0xe0, 0);
    kbd_push_scancode((u8)c, release);
}

static void send_mouse(void)
{
    { static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_MOUSE") != 0; if (tr) fprintf(stderr, "mouse %u: %d,%d buttons %d\n", plat_ticks_ms(), mouse_x, mouse_y, mouse_b); }
    mouse_update(mouse_x * 2, mouse_y, mouse_b);
}

/* ---------------------------------------------------------------- screen */
static u32 lut[256];
static void to_rgb(const u8 *vga, const u8 *pal)
{
    for (int i = 0; i < 256; i++) {
        u32 r = pal[i * 3] & 63, g = pal[i * 3 + 1] & 63, b = pal[i * 3 + 2] & 63;
        lut[i] = 0xff000000u | (((r << 2) | (r >> 4)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 2) | (b >> 4));
    }
    for (int i = 0; i < 320 * 200; i++) frame[i] = lut[vga[i]];
}

static void save_bmp(const char *path)
{
    to_rgb(M + 0xa0000, vga_dac);
    FILE *f = fopen(path, "wb");
    if (!f) return;
    u8 h[54] = {'B', 'M'};
    u32 sz = 54 + 320 * 200 * 3;
    memcpy(h + 2, &sz, 4); h[10] = 54; h[14] = 40;
    s32 w = 320, hh = -200; memcpy(h + 18, &w, 4); memcpy(h + 22, &hh, 4);
    h[26] = 1; h[28] = 24;
    fwrite(h, 1, 54, f);
    for (int i = 0; i < 320 * 200; i++) { u8 px[3] = {(u8)frame[i], (u8)(frame[i] >> 8), (u8)(frame[i] >> 16)}; fwrite(px, 1, 3, f); }
    fclose(f);
    { int nz = 0, ps = 0; for (int i = 0; i < 64000; i++) nz += M[0xa0000 + i] != 0; for (int i = 0; i < 768; i++) ps += vga_dac[i];
      fprintf(stderr, "saved %s (mode %02x, %d non-zero pixels, palette sum %d)\n", path, video_mode, nz, ps); }
}

/* where the 320x200 picture goes in the window (renderer output pixels) */
static SDL_Rect dest;
static char pending_small[1300]; static int pending_small_w;   /* plat_capture_small: the window, small, as a PNG */
typedef struct { unsigned char *px; int w, h, max_w; char path[1300]; } SmallJob;
static int small_job(void *arg)                  /* each pixel the average of f x f (a whole factor), then the PNG */
{
    SmallJob *j = arg;
    int f = (j->w + j->max_w - 1) / j->max_w; if (f < 1) f = 1;
    int w = j->w / f, h = j->h / f;
    unsigned char *sm = malloc((size_t)w * h * 4);
    if (sm) {
        for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) for (int c = 0; c < 3; c++) {
            unsigned sum = 0;
            for (int jj = 0; jj < f; jj++) for (int i = 0; i < f; i++) sum += j->px[((size_t)(y * f + jj) * j->w + x * f + i) * 4 + c];
            sm[((size_t)y * w + x) * 4 + c] = (unsigned char)(sum / (f * f));
            sm[((size_t)y * w + x) * 4 + 3] = 255;
        }
        int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
        if (stbi_write_png(j->path, w, h, 4, sm, w * 4)) fprintf(stderr, "screen picture kept: %s\n", j->path);
        free(sm);
    }
    free(j->px); free(j);
    return 0;
}
void plat_capture_small(const char *path, int max_w) { snprintf(pending_small, sizeof pending_small, "%s", path); pending_small_w = max_w; }
static char pending_wshot[300];     /* whole-window screenshot (interface included) at the next present */
static int pending_shot;             /* Ctrl+F12: the window as the player sees it, a PNG in screenshots/ */
#ifdef _WIN32
#include <direct.h>
#define PATH_SEP_STR "\\"
static void shot_mkdir(const char *d) { _mkdir(d); }
#else
#include <sys/stat.h>
#define PATH_SEP_STR "/"
static void shot_mkdir(const char *d) { mkdir(d, 0755); }
#endif
static int out_w = 960, out_h = 720, win_w = 960, win_h = 720;
static void layout(void)
{
    if (!ren || !win) return;
    SDL_GetRendererOutputSize(ren, &out_w, &out_h);
    SDL_GetWindowSize(win, &win_w, &win_h);
    int ph = ui_settings.aspect ? 240 : 200;      /* picture height in 320-wide units */
    int w, h;
    if (ui_settings.integer && out_w >= 320 && out_h >= ph) {
        int k = out_w / 320; if (out_h / ph < k) k = out_h / ph;
        w = 320 * k; h = ph * k;
    } else {
        w = out_w; h = w * ph / 320;
        if (h > out_h) { h = out_h; w = h * 320 / ph; }
    }
    dest.x = (out_w - w) / 2; dest.y = (out_h - h) / 2; dest.w = w; dest.h = h;
}

/* ---------------------------------------------------------------- replacement art */
/* textures of the replacement pictures (mods/hdart.c decides what goes where), loaded when
 * first shown and dropped again when unused while over the memory budget */
typedef struct { SDL_Texture *t; int w, h, tried, gen; u32 used; } ArtTex;
static ArtTex *arttex; static int nartt;
static long long art_bytes;
static u32 art_frame;
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *retval_from_stbi_load);

static SDL_Texture *art_texture(int pic, int *w, int *h)
{
    if (pic >= nartt) {
        int n = pic + 64;
        arttex = realloc(arttex, sizeof *arttex * n);
        memset(arttex + nartt, 0, sizeof *arttex * (n - nartt));
        nartt = n;
    }
    ArtTex *a = &arttex[pic];
    if (a->gen != hdart_generation) {             /* the art folder was scanned again */
        if (a->t) { SDL_DestroyTexture(a->t); art_bytes -= (long long)a->w * a->h * 4; }
        a->t = 0; a->tried = 0; a->gen = hdart_generation;
    }
    a->used = art_frame;
    if (!a->t && !a->tried) {
        a->tried = 1;
        const char *path = hdart_file(pic);
        int n;
        unsigned char *px = path ? stbi_load(path, &a->w, &a->h, &n, 4) : 0;
        if (px) {
            a->t = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, a->w, a->h);
            if (a->t) {
                SDL_UpdateTexture(a->t, 0, px, a->w * 4);
                SDL_SetTextureBlendMode(a->t, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(a->t, SDL_ScaleModeLinear);
                art_bytes += (long long)a->w * a->h * 4;
            }
            stbi_image_free(px);
        } else if (path) fprintf(stderr, "art: cannot read %s\n", path);
        /* over budget: drop what has not been shown for a while */
        for (int i = 0; i < nartt && art_bytes > 768ll << 20; i++)
            if (arttex[i].t && art_frame - arttex[i].used > 120) {
                SDL_DestroyTexture(arttex[i].t); arttex[i].t = 0; arttex[i].tried = 0;
                art_bytes -= (long long)arttex[i].w * arttex[i].h * 4;
            }
    }
    *w = a->w; *h = a->h;
    return a->t;
}

static void art_draw(void)
{
    art_frame++;
    if (!hdart_ndraws) return;
    float kx = dest.w / 320.0f, ky = dest.h / 200.0f;
    for (int i = 0; i < hdart_ndraws; i++) {
        HdDraw *d = &hdart_draws[i];
        int tw, th, pw, ph;
        SDL_Texture *t = art_texture(d->pic, &tw, &th);
        if (!t) continue;
        hdart_size(d->pic, &pw, &ph);
        float fx = (float)tw / pw, fy = (float)th / ph;
        SDL_Rect src = { (int)(d->sx * fx + 0.5f), (int)(d->sy * fy + 0.5f), (int)(d->w * fx + 0.5f), (int)(d->h * fy + 0.5f) };
        SDL_FRect dst = { dest.x + d->x * kx, dest.y + d->y * ky, d->w * kx, d->h * ky };
        SDL_SetTextureColorMod(t, d->mod[0], d->mod[1], d->mod[2]);
        SDL_RenderCopyF(ren, t, &src, &dst);
    }
}

/* EPX / Scale2x: each pixel P becomes four; a corner takes the colour of the two neighbours
 * beside it when they agree (and the other two do not), which rounds off diagonal steps */
static void scale2x(const u32 *src, int w, int h, u32 *dst)
{
    for (int y = 0; y < h; y++) {
        const u32 *up = src + (y > 0 ? y - 1 : y) * w, *row = src + y * w, *dn = src + (y < h - 1 ? y + 1 : y) * w;
        u32 *o0 = dst + (2 * y) * 2 * w, *o1 = o0 + 2 * w;
        for (int x = 0; x < w; x++) {
            u32 P = row[x], A = up[x], D = dn[x], C = row[x > 0 ? x - 1 : x], B = row[x < w - 1 ? x + 1 : x];
            u32 e0 = P, e1 = P, e2 = P, e3 = P;
            if (A != D && C != B) {
                if (C == A) e0 = A;
                if (A == B) e1 = B;
                if (D == C) e2 = C;
                if (B == D) e3 = D;
            }
            o0[2 * x] = e0; o0[2 * x + 1] = e1; o1[2 * x] = e2; o1[2 * x + 1] = e3;
        }
    }
}

void plat_scale2x(const unsigned *src, int w, int h, unsigned *dst) { scale2x(src, w, h, dst); }
/* a piece of the game's picture (cw x ch, 0xAARRGGBB), for the settings' preview of the scaling:
   of nine places, the one with the most colours (most edges to show) */
int plat_frame_crop(unsigned *out, int cw, int ch)
{
    int best = -1, bx = 0, by = 0;
    for (int j = 0; j < 3; j++) for (int i = 0; i < 3; i++) {
        int x0 = 20 + i * (280 - cw) / 2, y0 = 20 + j * (160 - ch) / 2;
        int n = 0; u32 seen[64]; int ns = 0;
        for (int y = y0; y < y0 + ch; y += 2) for (int x = x0; x < x0 + cw; x += 2) {
            u32 c = frame[y * 320 + x]; int k = 0; while (k < ns && seen[k] != c) k++;
            if (k == ns && ns < 64) seen[ns++] = c;
        }
        n = ns;
        if (n > best) { best = n; bx = x0; by = y0; }
    }
    for (int y = 0; y < ch; y++) for (int x = 0; x < cw; x++) out[y * cw + x] = frame[(by + y) * 320 + bx + x] | 0xff000000u;
    return best;
}

/* the frame as textures, smoothed when asked. The smoothed picture goes up in tiles of
 * 640x400 (each in a 1024x512 texture): Direct3D 9 under Wine shows nothing from wider
 * textures, and tiles cost nothing elsewhere. Returns the number of tiles (1 unsmoothed). */
static SDL_Texture *tiles[4];
static SDL_Rect tile_src[4]; static SDL_FRect tile_dst[4];     /* dst: fractions of the picture */
static int frame_textures(void)
{
    int sc = ui_settings.scaler;
    if (sc <= 0 || sc > 2) {
        SDL_UpdateTexture(tex, 0, frame, 320 * 4);
        tiles[0] = tex; tile_src[0] = (SDL_Rect){ 0, 0, 320, 200 }; tile_dst[0] = (SDL_FRect){ 0, 0, 1, 1 };
        return 1;
    }
    static SDL_Texture *own[4];
    static u32 b2[640 * 400], b4[1280 * 800], last[320 * 200]; static int last_sc = -1;
    int n0 = sc == 2 ? 2 : 1;
    if (last_sc == sc && own[0] && !memcmp(last, frame, sizeof last)) {      /* unchanged: the tiles stand */
        for (int t = 0; t < n0 * n0; t++) tiles[t] = own[t];
        return n0 * n0;
    }
    memcpy(last, frame, sizeof last); last_sc = sc;
    { static int tm = -1; if (tm < 0) tm = getenv("DL_SCALER_TIME") != 0; if (tm) { static uint64_t acc, cnt; uint64_t t0 = plat_ticks_us(); scale2x(frame, 320, 200, b2); if (sc == 2) scale2x(b2, 640, 400, b4); acc += plat_ticks_us() - t0; if (++cnt % 50 == 0) fprintf(stderr, "scaler: %.2f ms per frame\n", acc / 1000.0 / cnt); } }
    scale2x(frame, 320, 200, b2);
    int n = sc == 2 ? 2 : 1;                    /* tiles per side */
    const u32 *px = b2; int w = 640;
    if (sc == 2) { scale2x(b2, 640, 400, b4); px = b4; w = 1280; }
    for (int t = 0; t < n * n; t++) {
        if (!own[t]) own[t] = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 512);
        if (!own[t]) { fprintf(stderr, "scaler: no texture: %s\n", SDL_GetError()); ui_settings.scaler = 0; return frame_textures(); }
        int tx = t % n, ty = t / n;
        /* each tile carries a pixel of its neighbours round its inner edges: smoothed (linear)
           scaling reads past a tile's own pixels, and without them the seams showed as lines */
        int l = tx > 0, tp = ty > 0, rt = tx < n - 1, bt = ty < n - 1;
        SDL_Rect up = { 0, 0, 640 + l + rt, 400 + tp + bt };
        SDL_UpdateTexture(own[t], &up, px + (ty * 400 - tp) * w + tx * 640 - l, w * 4);
        SDL_Rect r = { l, tp, 640, 400 };
        tiles[t] = own[t]; tile_src[t] = r;
        tile_dst[t] = (SDL_FRect){ (float)tx / n, (float)ty / n, 1.0f / n, 1.0f / n };
    }
    return n * n;
}

/* battle.smooth_scroll: a step of the battle view (its origin, DS:c866 across and c886 down: a step
 * across moves the picture 32 pixels, one down 16 aside and 16 up) slides the field (x 60..319 of
 * the picture) to its new place over a moment instead of jumping. The frame before the step and
 * the new one are drawn offset so that they meet: what leaves is the old one's, what comes in the
 * new one's. A jump of more than two steps (the view sent to a member) is not slid. */
static int slide_ms(void) { static int ms = -1; if (ms < 0) { const char *e = getenv("DL_SLIDE_MS"); ms = e ? atoi(e) : 120; if (ms < 1) ms = 1; } return ms; }   /* (test: DL_SLIDE_MS) */
void plat_mouse_game(int *x, int *y);
static u32 slide_prev[320 * 200];           /* the last frame shown in a battle */
static SDL_Texture *slide_old, *slide_new;
static u32 slide_t0; static float slide_sx, slide_sy; static int sliding, slide_h = -999, slide_v;
static float slide_t(void) { float t = sliding ? (float)(plat_ticks_ms() - slide_t0) / slide_ms() : 1.0f; return t < 0 ? 0 : t > 1 ? 1 : t; }
/* where the new frame stands now, in game pixels (the battle's overlays move with it) */
void plat_field_slide(float *x, float *y)
{
    float t = slide_t();
    if (!sliding || t >= 1) { *x = *y = 0; return; }
    float e = 1 - (1 - t) * (1 - t);           /* (easing out) */
    *x = -slide_sx * (1 - e); *y = -slide_sy * (1 - e);
}
static int slide_watch(void)                /* 1: the field slides this frame */
{
    int on = game_mode() == GM_BATTLE && config_get_int("battle.smooth_scroll", 1) && !config_get_int("original", 0);
    if (!on) { slide_h = -999; sliding = 0; return 0; }
    int h = (s8)g_rb(0xc866), v = (s8)g_rb(0xc886), dh = h - slide_h, dv = v - slide_v, start = 0;
    if (slide_h != -999 && (dh || dv) && abs(dh) <= 2 && abs(dv) <= 2) {
        if (!slide_old) slide_old = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 200);
        if (!slide_new) slide_new = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 200);
        if (slide_old && slide_new) {
            SDL_UpdateTexture(slide_old, 0, slide_prev, 320 * 4);
            slide_sx = (float)(-32 * dh + 16 * dv); slide_sy = (float)(-16 * dv);
            slide_t0 = plat_ticks_ms(); sliding = start = 1;
        }
    } else if (slide_h != -999 && (dh || dv)) sliding = 0;   /* (a jump: no slide) */
    slide_h = h; slide_v = v;
    memcpy(slide_prev, frame, sizeof slide_prev);
    if (sliding && !start && slide_t() >= 1) sliding = 0;
    return sliding;
}
static void slide_draw(void)                /* the field: the old frame leaving, the new one coming */
{
    float t = slide_t(), e = 1 - (1 - t) * (1 - t), kx = dest.w / 320.0f, ky = dest.h / 200.0f;
    SDL_UpdateTexture(slide_new, 0, frame, 320 * 4);
    SDL_Rect clip = { dest.x + (int)(60 * kx + 0.5f), dest.y, dest.w - (int)(60 * kx + 0.5f), dest.h };
    SDL_RenderSetClipRect(ren, &clip);
    SDL_Rect src = { 60, 0, 260, 200 };
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderFillRect(ren, &clip);
    SDL_Texture *tx[2] = { slide_old, slide_new };
    float ox[2] = { slide_sx * e, -slide_sx * (1 - e) }, oy[2] = { slide_sy * e, -slide_sy * (1 - e) };
    for (int k = 0; k < 2; k++) {
        SDL_SetTextureScaleMode(tx[k], ui_settings.smooth ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(tx[k], SDL_BLENDMODE_NONE);
        SDL_FRect d = { dest.x + (60 + ox[k]) * kx, dest.y + oy[k] * ky, 260 * kx, 200 * ky };
        SDL_RenderCopyF(ren, tx[k], &src, &d);
    }
    /* the game draws its pointer into the picture: the new frame's, carried along with it, is
       covered with the old frame's ground at that place (the window's own pointer stays put) */
    int px, py; plat_mouse_game(&px, &py);
    SDL_Rect c = { px - 2, py - 4, 20, 18 };    /* (the hand: its tip at the pointer, the rest right and below) */
    SDL_Rect cs = { c.x - (int)slide_sx, c.y - (int)slide_sy, c.w, c.h };
    if (cs.x >= 0 && cs.y >= 0 && cs.x + cs.w <= 320 && cs.y + cs.h <= 200) {
        SDL_FRect d = { dest.x + (c.x + ox[1]) * kx, dest.y + (c.y + oy[1]) * ky, c.w * kx, c.h * ky };
        SDL_RenderCopyF(ren, slide_old, &cs, &d);
    }
    SDL_RenderSetClipRect(ren, 0);
}

int plat_rec_frames;
/* a native screen being set up (the original's own screen drawn first, unseen): the last frame
   stays on screen until the new view is up (ns_enter releases it), at most ms */
static u32 hold_until;
void plat_hold(int ms) { hold_until = ms > 0 ? SDL_GetTicks() + (u32)ms : 0; }
void plat_present(const u8 *vga, const u8 *pal)
{
    { void fixes_frame(void); fixes_frame(); void saves_frame(void); saves_frame(); void rules_frame(void); rules_frame(); void autopause_frame(void); autopause_frame(); void battlelog_frame(void); battlelog_frame(); void memberlooks_frame(void); memberlooks_frame(); void losses_frame(void); losses_frame(); void battleend_frame(void); battleend_frame(); }   /* game-side checks, headless too */
    { static int slow = -1; if (slow < 0) slow = getenv("DL_SLOW_PRESENT") ? atoi(getenv("DL_SLOW_PRESENT")) : 0; if (slow) plat_sleep_ms(slow); }   /* test: a slow display */
    if (headless) return;
    if (hold_until && (s32)(hold_until - SDL_GetTicks()) > 0) { SDL_PumpEvents(); return; }
    hold_until = 0;
    static int ptime = -1; if (ptime < 0) ptime = getenv("DL_PRESENT_TIME") != 0;
    uint64_t pt0 = ptime ? plat_ticks_us() : 0;
    to_rgb(vga, pal);
    static u8 idx[320 * 200];
    memcpy(idx, vga, sizeof idx);
    { void hdtext_prepare(u32 *, const u32 *, u8 *); hdtext_prepare(frame, lut, idx); }   /* crisp text: wipe the glyphs it redraws */
    hdart_prepare(frame, idx, lut);                 /* replacement art: clear the pixels it covers */
    { void hdart_labels(u32 *, const u8 *, const u32 *); hdart_labels(frame, idx, lut); }   /* words painted into pictures */
    { void worldmap_watch_palette(void); worldmap_watch_palette(); }
    uint64_t pt1 = ptime ? plat_ticks_us() : 0;
    int nt = frame_textures();
    layout();
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    art_draw();
    for (int t = 0; t < nt; t++) {
        SDL_SetTextureScaleMode(tiles[t], ui_settings.smooth ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(tiles[t], hdart_ndraws ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
        SDL_FRect d = { dest.x + tile_dst[t].x * dest.w, dest.y + tile_dst[t].y * dest.h, tile_dst[t].w * dest.w, tile_dst[t].h * dest.h };
        if (nt > 1) {                            /* (tiles: edges on whole pixels, shared by neighbours: no gap or overlap between them) */
            float x0 = roundf(d.x), y0 = roundf(d.y), x1 = roundf(d.x + d.w), y1 = roundf(d.y + d.h);
            d = (SDL_FRect){ x0, y0, x1 - x0, y1 - y0 };
        }
        SDL_RenderCopyF(ren, tiles[t], &tile_src[t], &d);
    }
    if (slide_watch()) slide_draw();
    uint64_t pt2 = ptime ? plat_ticks_us() : 0;
    ui_draw(ren);
    uint64_t pt3 = ptime ? plat_ticks_us() : 0;
    extern unsigned long long ui_build_us;
    uint64_t ptb = ptime && ui_build_us ? (uint64_t)((ui_build_us - t0) * 1000000 / freq) : pt3;
    if (pending_shot) {
        pending_shot = 0;
        unsigned char *px = malloc((size_t)out_w * out_h * 4);
        if (px && SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ABGR8888, px, out_w * 4) == 0) {   /* bytes R G B A */
            extern char dlp_data_dir[];
            char dir[1200], path[1300], name[64];
            snprintf(dir, sizeof dir, "%s/screenshots", dlp_data_dir);
            shot_mkdir(dir);
            time_t t = time(0); struct tm *tm = localtime(&t);
            strftime(name, sizeof name, "darklands-%Y-%m-%d-%H%M%S.png", tm);
            snprintf(path, sizeof path, "%s/%s", dir, name);
            for (int i = 0; i < out_w * out_h; i++) px[i * 4 + 3] = 255;
            int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
            char msg[200];
            if (stbi_write_png(path, out_w, out_h, 4, px, out_w * 4)) snprintf(msg, sizeof msg, "Screenshot saved: screenshots%s%s", PATH_SEP_STR, name);
            else snprintf(msg, sizeof msg, "The screenshot could not be saved (%s)", dir);
            fprintf(stderr, "%s\n", msg);
            void ui_notice(const char *); ui_notice(msg);
        }
        free(px);
    }
    {   /* test: DL_REC=dir, ucmd urec:N - the next N frames each saved (to catch a frame's flash) */
        extern int plat_rec_frames; static int recn;
        if (plat_rec_frames > 0 && getenv("DL_REC")) {
            SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, out_w, out_h, 32, SDL_PIXELFORMAT_ARGB8888);
            if (sf) { char fp[600]; snprintf(fp, sizeof fp, "%s/f%04d.bmp", getenv("DL_REC"), recn++);
                      SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ARGB8888, sf->pixels, sf->pitch);
                      int sm = getenv("DL_REC_SMALL") ? atoi(getenv("DL_REC_SMALL")) : 1;   /* (1/N the size: long recordings) */
                      if (sm > 1) { SDL_Surface *s2 = SDL_CreateRGBSurfaceWithFormat(0, out_w / sm, out_h / sm, 32, SDL_PIXELFORMAT_ARGB8888);
                                    if (s2) { SDL_BlitScaled(sf, NULL, s2, NULL); SDL_SaveBMP(s2, fp); SDL_FreeSurface(s2); } }
                      else SDL_SaveBMP(sf, fp);
                      SDL_FreeSurface(sf); }
            plat_rec_frames--;
        }
    }
    if (pending_small[0]) {                     /* (ui/screenpics.cpp: the settings' pictures of the new screens) */
        unsigned char *px = malloc((size_t)out_w * out_h * 4);
        if (px && SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ABGR8888, px, out_w * 4) == 0) {
            SmallJob *j = malloc(sizeof *j);              /* made small and written on a thread of its own: no hitch */
            if (j) {
                j->px = px; j->w = out_w; j->h = out_h; j->max_w = pending_small_w; snprintf(j->path, sizeof j->path, "%s", pending_small);
                SDL_Thread *t = SDL_CreateThread(small_job, "screenpic", j);
                if (t) { SDL_DetachThread(t); px = 0; } else free(j);
            }
        }
        free(px);
        pending_small[0] = 0;
    }
    if (pending_wshot[0]) {
        SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, out_w, out_h, 32, SDL_PIXELFORMAT_ARGB8888);
        if (sf) { SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ARGB8888, sf->pixels, sf->pitch);
                  SDL_SaveBMP(sf, pending_wshot); SDL_FreeSurface(sf); fprintf(stderr, "saved %s\n", pending_wshot); }
        pending_wshot[0] = 0;
    }
    SDL_RenderPresent(ren);
    if (ptime) { static uint64_t acc, mx, cnt, a1, a2, a3, ab, mb, vx, vm, cm; extern int ui_vtx, ui_cmds; vx += ui_vtx; if ((uint64_t)ui_vtx > vm) vm = ui_vtx; if ((uint64_t)ui_cmds > cm) cm = ui_cmds; uint64_t t4 = plat_ticks_us(), d = t4 - pt0; acc += d; if (d > mx) mx = d;
                 a1 += pt1 - pt0; a2 += pt3 - pt2; a3 += t4 - pt3; { uint64_t b = ptb > pt2 && ptb <= pt3 ? ptb - pt2 : pt3 - pt2; ab += b; if (b > mb) mb = b; }
                 static int pn; if (!pn) { pn = atoi(getenv("DL_PRESENT_TIME")); if (pn < 10) pn = 100; }
                 if (++cnt % pn == 0) { fprintf(stderr, "present: %.2f ms average (prepare %.2f, ui %.2f, build %.2f, present %.2f), %.2f ms worst, build worst %.2f, vertices %llu (max %llu), draw calls max %llu\n", acc / 1000.0 / pn, a1 / 1e3 / pn, a2 / 1e3 / pn, ab / 1e3 / pn, a3 / 1e3 / pn, mx / 1000.0, mb / 1000.0, (unsigned long long)(vx / pn), (unsigned long long)vm, (unsigned long long)cm); acc = mx = a1 = a2 = a3 = ab = mb = vx = vm = cm = 0; } }
}

/* game pixel (gx, gy) -> window coordinates, where the interface draws (valid after the
 * first present) */
void plat_game_to_win(float gx, float gy, float *wx, float *wy)
{
    float ox = dest.x + gx * dest.w / 320.0f, oy = dest.y + gy * dest.h / 200.0f;
    *wx = out_w ? ox * win_w / out_w : ox;
    *wy = out_h ? oy * win_h / out_h : oy;
}

/* The drawing surface made to match the window again. A Windows renderer can keep its surface at
 * an old size when the window changed while it was in the background (switching windows during
 * the opening while the window was being maximized): the picture is then shown stretched and the
 * mouse lands at half its place. The renderer resizes its surface when it sees a "size changed"
 * window event, so one is sent with the window's present size (nothing changes if it was right). */
static int mouse_far_x, mouse_far_y;        /* the furthest mouse position seen (for reports) */
static void resync_surface(const char *why)
{
    int pw, ph, ow, oh; SDL_GetWindowSizeInPixels(win, &pw, &ph); SDL_GetRendererOutputSize(ren, &ow, &oh);
    int ww, wh; SDL_GetWindowSize(win, &ww, &wh);
    if (pw > 0 && ph > 0 && !(SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED) && (ow != pw || oh != ph || ww != win_w || wh != win_h)) {   /* (only when the sizes differ: renewing the surface at each return to the window was a hitch) */
        SDL_Event ev; memset(&ev, 0, sizeof ev);
        ev.type = SDL_WINDOWEVENT; ev.window.event = SDL_WINDOWEVENT_SIZE_CHANGED; ev.window.windowID = SDL_GetWindowID(win);
        ev.window.data1 = ww; ev.window.data2 = wh; ev.window.timestamp = SDL_GetTicks();
        SDL_PushEvent(&ev);     /* the renderer's event watch sees it now; the game loop sees it next */
    }
    layout();
    if (ow != pw || oh != ph || ww != win_w || wh != win_h)
        fprintf(stderr, "window: %s, the drawing surface was %dx%d for a window of %dx%d pixels: renewed\n", why, ow, oh, pw, ph);
}

/* window coordinates -> game pixels */
static void to_game(int wx, int wy, int *gx, int *gy)
{
    if (ren && win) {   /* the sizes as they are now (a size can change between frames) */
        int a, b, c, d; SDL_GetWindowSize(win, &a, &b); SDL_GetRendererOutputSize(ren, &c, &d);
        if (a != win_w || b != win_h || c != out_w || d != out_h) layout();
    }
    int ox = win_w ? wx * out_w / win_w : wx, oy = win_h ? wy * out_h / win_h : wy;
    int x = dest.w ? (ox - dest.x) * 320 / dest.w : 0, y = dest.h ? (oy - dest.y) * 200 / dest.h : 0;
    if (x < 0) x = 0; if (x > 319) x = 319; if (y < 0) y = 0; if (y > 199) y = 199;
    *gx = x; *gy = y;
}

void plat_mouse_game(int *x, int *y) { *x = mouse_x; *y = mouse_y; }
/* the system pointer is beside the picture (over the margins): the game's own pointer waits at
   the picture's edge meanwhile (a battle must not scroll, a click there is not the game's) */
static int ptr_out;
int plat_pointer_outside(void) { return ptr_out; }
int plat_win_outside(int wx, int wy)
{
    int ox = win_w ? wx * out_w / win_w : wx, oy = win_h ? wy * out_h / win_h : wy;
    return ox < dest.x || ox >= dest.x + dest.w || oy < dest.y || oy >= dest.y + dest.h;
}

/* dragging the battle field (battle.drag_pan): the right button (or the middle one) held and moved
 * pulls the view with the pointer, a step of the view's origin at a time (mods/battlescroll.c:
 * a step across moves the picture 32 pixels sideways, one down the grid 16 aside and 16 up). A
 * right press is held back until it is a drag or not: not, it is played as the click it was (the
 * menus open on its release); a drag swallows it. */
static int drag_state, drag_btn, drag_x0, drag_y0, drag_lx, drag_ly, drag_replay;
static float drag_ax, drag_ay;
static SDL_Event drag_down;
static int drag_ok(int wx, int wy)
{
    int ui_is_open(void), ns_showing(void);
    return game_mode() == GM_BATTLE && !ui_is_open() && !ns_showing() && !plat_win_outside(wx, wy)
           && config_get_int("battle.drag_pan", 1) && !config_get_int("original", 0);
}
/* a click that started something in the interface (a double-click on the world map that sets off
   travelling and closes it): its release, and any press before it, kept from the game, which
   would take it for a walk to where the pointer is (and that ends the travelling) */
static int swallow_click;
void plat_swallow_click(void) { if (SDL_GetMouseState(0, 0) & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) swallow_click = 1; }
static int drag_filter(SDL_Event *e)
{
    void battlescroll_pan(int, int);
    if (e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
        int b = e->button.button;
        if (b != SDL_BUTTON_RIGHT && b != SDL_BUTTON_MIDDLE) return 0;
        if (drag_replay > 0 && b == SDL_BUTTON_RIGHT) { drag_replay--; return 0; }          /* (a held-back click played again) */
        if (e->type == SDL_MOUSEBUTTONDOWN) {
            if (drag_state || !drag_ok(e->button.x, e->button.y)) return 0;
            drag_state = b == SDL_BUTTON_MIDDLE ? 2 : 1; drag_btn = b;
            drag_x0 = drag_lx = e->button.x; drag_y0 = drag_ly = e->button.y; drag_ax = drag_ay = 0;
            drag_down = *e;
            return 1;
        }
        if (!drag_state || b != drag_btn) return 0;
        int was = drag_state; drag_state = 0;
        if (was == 1) {                                     /* no drag: the click it was */
            SDL_Event up = *e;
            drag_replay = 2;
            SDL_PushEvent(&drag_down); SDL_PushEvent(&up);
        }
        return 1;
    }
    if (e->type == SDL_MOUSEMOTION && drag_state) {
        if (drag_state == 1 && abs(e->motion.x - drag_x0) + abs(e->motion.y - drag_y0) > 6) drag_state = 2;
        if (drag_state == 2) {
            float ax, ay, bx, by; plat_game_to_win(0, 0, &ax, &ay); plat_game_to_win(320, 200, &bx, &by);
            float kx = (bx - ax) / 320.0f, ky = (by - ay) / 200.0f;
            if (kx > 0 && ky > 0) { drag_ax += (e->motion.x - drag_lx) / kx; drag_ay += (e->motion.y - drag_ly) / ky; }
            drag_lx = e->motion.x; drag_ly = e->motion.y;
            int v = (int)(-drag_ay / 16.0f);                 /* the picture follows the pointer */
            drag_ay += 16.0f * v;
            int h = (int)((16.0f * v - drag_ax) / 32.0f);
            drag_ax -= -32.0f * h + 16.0f * v;
            if (h || v) battlescroll_pan(h, v);
        }
    }
    return 0;
}

static float dpi_k = 1.0f;      /* Windows' display scale (1.5 at 150 %): window sizes are saved divided by it */
/* the game stands still while the menu is open, and while the window is in the background
 * (display.pause_unfocused, on by default: a battle does not go on behind another window) */
static int bg_paused;           /* the window lost the focus and the game waits */
static u32 bg_resumed;          /* when it came back: the click that brought it back is not the game's */
int plat_headless(void) { return headless; }
int plat_menu_open(void) { return !headless && (ui_is_open() || bg_paused); }
int plat_in_background(void) { return bg_paused; }
/* the window can't be seen (minimized): the waiting loops may rest long; behind another window it is
   still seen, and the interface over it is drawn at a steady pace */
int plat_bg_sleep_ms(void) { return !bg_paused ? 15 : (win && (SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED)) ? 100 : 33; }

/* ---------------------------------------------------------------- events */
static void click_at(int x, int y);
void plat_press_key(int scancode);
int testcmd_run(const char *cmd, const char *arg);   /* testcmds.c */
static void run_script(void)
{
    u32 now = plat_ticks_ms();
    if (step_i && step_i == nsteps) step_i = nsteps = 0;
    while (step_i < nsteps && steps[step_i].t <= now) {
        Step *s = &steps[step_i++];
        int x = 0, y = 0;
        sscanf(s->arg, "%d,%d", &x, &y);
        if (!strcmp(s->cmd, "key") || !strcmp(s->cmd, "down") || !strcmp(s->cmd, "up")) {
            SDL_Scancode sc = SDL_GetScancodeFromName(s->arg);
            if (sc == SDL_SCANCODE_UNKNOWN) { fprintf(stderr, "script: unknown key %s\n", s->arg); continue; }
            if (getenv("DL_TRACE_INPUT")) fprintf(stderr, "script %u ms: %s %s (sc %d)\n", now, s->cmd, s->arg, (int)sc);
            if (sc == SDL_SCANCODE_F12 && !headless) { ui_toggle(); continue; }
            if (!strcmp(s->cmd, "key") && sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) {   /* as a player's key: a card's option (0 the tenth) */
                int gx, gy, assist_card_key(int, int *, int *);
                if (assist_card_key(sc - SDL_SCANCODE_1 + 1, &gx, &gy)) { click_at(gx, gy); continue; }
            }
            if (strcmp(s->cmd, "up")) key(sc, 0);
            if (!strcmp(s->cmd, "up")) key(sc, 1);
            else if (!strcmp(s->cmd, "key")) {      /* release a little later, like a real key press */
                char a[sizeof s->arg]; memcpy(a, s->arg, sizeof a);       /* (r may be s when the list is full) */
                Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
                r->t = now + 60; snprintf(r->cmd, sizeof r->cmd, "up"); snprintf(r->arg, sizeof r->arg, "%s", a);
                for (int i = nsteps - 1; i > step_i && steps[i].t < steps[i - 1].t; i--) { Step t = steps[i]; steps[i] = steps[i - 1]; steps[i - 1] = t; }
            }
        } else if (!strcmp(s->cmd, "move")) { mouse_x = x; mouse_y = y; send_mouse(); }
        else if (!strcmp(s->cmd, "click") || !strcmp(s->cmd, "rclick")) {
            int b = s->cmd[0] == 'r' ? 2 : 1;
            mouse_x = x; mouse_y = y; send_mouse();
            mouse_b |= b; send_mouse();
            /* release on a later poll so the game sees the press */
            Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
            r->t = now + 80; snprintf(r->cmd, sizeof r->cmd, "release"); snprintf(r->arg, sizeof r->arg, "%d", b);
            /* keep the list sorted by time */
            for (int i = nsteps - 1; i > step_i && steps[i].t < steps[i - 1].t; i--) { Step t = steps[i]; steps[i] = steps[i - 1]; steps[i - 1] = t; }
        } else if (!strcmp(s->cmd, "release")) { mouse_b &= ~atoi(s->arg); send_mouse(); }
        else if (!strcmp(s->cmd, "press")) { mouse_b |= atoi(s->arg); send_mouse(); }       /* test: a button held */
        else if (!strcmp(s->cmd, "shot")) save_bmp(s->arg);
        else if (!strcmp(s->cmd, "utab")) ui_select_tab(atoi(s->arg));
        else if (!strcmp(s->cmd, "travel")) { void travel_click(int, int); travel_click(x, y); }   /* as Ctrl+click at game x,y */
        else if (!strcmp(s->cmd, "travelto")) { int travel_to_loc(int, const char *); travel_to_loc(atoi(s->arg), "test"); }
        else if (!strcmp(s->cmd, "travelnear")) { int travel_nearest(int); travel_nearest(atoi(s->arg)); }   /* nearest location of a type */
        else if (!strcmp(s->cmd, "uclick") || !strcmp(s->cmd, "urclick")) {   /* a click (urclick: right) as from the system, window pixels */
            int btn = s->cmd[1] == 'r' ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            Uint32 wid = win ? SDL_GetWindowID(win) : 0;
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = wid; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
            memset(&ev, 0, sizeof ev); ev.type = SDL_MOUSEBUTTONDOWN; ev.button.windowID = wid; ev.button.button = (Uint8)btn; ev.button.state = SDL_PRESSED;
            ev.button.x = x; ev.button.y = y; ev.button.clicks = 1; SDL_PushEvent(&ev);
            ev.type = SDL_MOUSEBUTTONUP; ev.button.state = SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "udown") || !strcmp(s->cmd, "uup")) {   /* test: the left button pressed or let go there (window pixels): a drag between */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            Uint32 wid = win ? SDL_GetWindowID(win) : 0;
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = wid; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
            memset(&ev, 0, sizeof ev); ev.type = s->cmd[1] == 'd' ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP; ev.button.windowID = wid; ev.button.button = SDL_BUTTON_LEFT;
            ev.button.state = s->cmd[1] == 'd' ? SDL_PRESSED : SDL_RELEASED; ev.button.x = x; ev.button.y = y; ev.button.clicks = 1; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "ukey")) {               /* test: a key as from the system (SDL events), by SDL name */
            SDL_Scancode sc = SDL_GetScancodeFromName(s->arg);
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            ev.type = SDL_KEYDOWN; ev.key.windowID = win ? SDL_GetWindowID(win) : 0; ev.key.state = SDL_PRESSED;
            ev.key.keysym.scancode = sc; ev.key.keysym.sym = SDL_GetKeyFromScancode(sc); SDL_PushEvent(&ev);
            ev.type = SDL_KEYUP; ev.key.state = SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "uhover")) {            /* the pointer over the interface, window pixels */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = win ? SDL_GetWindowID(win) : 0; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "uwheel")) {             /* test: the mouse wheel turned by n notches (up > 0) */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            ev.type = SDL_MOUSEWHEEL; ev.wheel.windowID = win ? SDL_GetWindowID(win) : 0; ev.wheel.y = atoi(s->arg);
#if SDL_VERSION_ATLEAST(2, 0, 18)
            ev.wheel.preciseY = (float)ev.wheel.y;       /* what ImGui's SDL backend reads */
#endif
            SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "utype")) {               /* test: text typed into the interface (SDL text input) */
            for (const char *t = s->arg; *t; ) {          /* in pieces: an event holds 31 bytes */
                SDL_Event ev; memset(&ev, 0, sizeof ev); ev.type = SDL_TEXTINPUT; ev.text.windowID = win ? SDL_GetWindowID(win) : 0;
                size_t k = strlen(t); if (k > 24) k = 24;
                memcpy(ev.text.text, t, k); ev.text.text[k] = 0; SDL_PushEvent(&ev); t += k;
            }
        }
        else if (!strcmp(s->cmd, "upadbtn")) {             /* test: a controller button (name,1 down / 0 up) */
            char nm[32] = ""; int dn = 1; sscanf(s->arg, "%31[^,],%d", nm, &dn);
            SDL_Event ev; memset(&ev, 0, sizeof ev); ev.type = dn ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
            ev.cbutton.button = (Uint8)SDL_GameControllerGetButtonFromString(nm); ev.cbutton.state = dn ? SDL_PRESSED : SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "upadaxis")) {            /* test: a stick held (0 left x, 1 left y, 2 right y, 3 left trigger; value, or 'off') */
            int a = 0, v = 0; if (sscanf(s->arg, "%d,%d", &a, &v) == 2 && a >= 0 && a < 4) pad_test_axis[a] = v;
            else if (a >= 0 && a < 4) pad_test_axis[a] = 0x7fffffff;
        }
        else if (!strcmp(s->cmd, "wshot")) snprintf(pending_wshot, sizeof pending_wshot, "%s", s->arg);
        else if (!strcmp(s->cmd, "usdlkey")) {             /* test: a key as an SDL event (through Darklands+'s own keys, ui_event): usdlkey:z */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            ev.type = SDL_KEYDOWN; ev.key.state = SDL_PRESSED; ev.key.keysym.scancode = SDL_GetScancodeFromName(s->arg);
            ev.key.keysym.sym = SDL_GetKeyFromScancode(ev.key.keysym.scancode);
            SDL_PushEvent(&ev); ev.type = SDL_KEYUP; ev.key.state = SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "uopt")) {                /* test: click the line of crisp text containing arg */
            
            int found = 0;
            for (int i = 0; i < hdtext_nruns && !found; i++)
                if (strstr(hdtext_runs[i].text, s->arg)) {
                    mouse_x = hdtext_runs[i].x + 6; mouse_y = hdtext_runs[i].y + 3; send_mouse();
                    mouse_b |= 1; send_mouse();
                    Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
                    r->t = now + 80; snprintf(r->cmd, sizeof r->cmd, "release"); snprintf(r->arg, sizeof r->arg, "1");
                    for (int k = nsteps - 1; k > step_i && steps[k].t < steps[k - 1].t; k--) { Step t = steps[k]; steps[k] = steps[k - 1]; steps[k - 1] = t; }
                    found = 1;
                }
            fprintf(stderr, "uopt %s: %s\n", s->arg, found ? "clicked" : "not found");
        }
        else if (!strcmp(s->cmd, "utext")) {               /* the text on screen (crisp text runs), for scripts */
            
            for (int i = 0; i < hdtext_nruns; i++) fprintf(stderr, "text %d %d %d %d %06x %s\n", hdtext_runs[i].x, hdtext_runs[i].y, hdtext_runs[i].w, hdtext_runs[i].h, hdtext_runs[i].rgb & 0xffffff, hdtext_runs[i].text);
            { int cx, cy, cw, ch; if (hdtext_game_cursor(&cx, &cy, &cw, &ch)) fprintf(stderr, "cursor %d %d %d %d mouse %d %d\n", cx, cy, cw, ch, mouse_x, mouse_y); }
            fprintf(stderr, "text end\n");
        }
        else if (!strcmp(s->cmd, "ucardkey")) { int gx, gy, assist_card_key(int, int *, int *); if (assist_card_key(atoi(s->arg), &gx, &gy)) click_at(gx, gy); else { const char *assist_card_key_why(int); fprintf(stderr, "ucardkey %s: no option (%s)\n", s->arg, assist_card_key_why(atoi(s->arg))); } }
        else if (!strcmp(s->cmd, "quit")) plat_quit(0);
        else if (testcmd_run(s->cmd, s->arg)) {}                 /* the game-state test commands (testcmds.c) */
    }
}

/* DL_CTL=<file>: an external driver appends script steps ("cmd:arg", one per line); they run
 * at once. Used for interactive automated play (port/tools/nctl.py). */
static void run_ctl(void)
{
    static const char *path; static long pos; static int init;
    if (!init) { init = 1; path = getenv("DL_CTL"); }
    if (!path) return;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fseek(f, pos, SEEK_SET);
    char line[300];
    while (fgets(line, sizeof line, f)) {
        size_t n = strlen(line);
        if (!n || line[n - 1] != '\n') break;          /* incomplete line: later */
        pos += (long)n;
        line[n - 1] = 0;
        char *c = strchr(line, ':');
        if (!c || nsteps >= 512) continue;
        *c = 0;
        /* insert as an immediate step */
        Step st; st.t = 0; snprintf(st.cmd, sizeof st.cmd, "%s", line); snprintf(st.arg, sizeof st.arg, "%s", c + 1);
        memmove(&steps[step_i + 1], &steps[step_i], (size_t)(nsteps - step_i) * sizeof(Step));
        steps[step_i] = st; nsteps++;
    }
    fclose(f);
}

/* the menu bar with a click: the game shows its menus only while the right button is held and
 * takes the entry under the pointer when it is let go, so a right click only flashed them.
 * A short right click (the bar up, DS:a2e3) now leaves them open, held for the game, and the
 * next click chooses: the button is let go there (any key: away from the menus, as the game
 * takes no keys while the button is held) */
static int menu_sticky;                    /* 1 open after a short click; 2 the choosing click's release to swallow */
static u32 rdown_t, rup_wait;
static void later(u32 ms, const char *cmd, const char *arg)
{
    Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
    r->t = plat_ticks_ms() + ms; snprintf(r->cmd, sizeof r->cmd, "%s", cmd); snprintf(r->arg, sizeof r->arg, "%s", arg);
    for (int k = nsteps - 1; k > step_i && steps[k].t < steps[k - 1].t; k--) { Step t = steps[k]; steps[k] = steps[k - 1]; steps[k - 1] = t; }
}
static int menu_bar_up(void) { return game_mode() != GM_OTHER && (s16)g_rw(0xa2e3) == 1; }
static int menu_click(const SDL_Event *e)
{
    int right = e->button.button == SDL_BUTTON_RIGHT;
    if (menu_sticky == 2) { if (e->type == SDL_MOUSEBUTTONUP) menu_sticky = 0; return 1; }
    if (menu_sticky == 1) {
        if (e->type != SDL_MOUSEBUTTONDOWN) return 1;
        if (!menu_bar_up()) { menu_sticky = 0; mouse_b &= ~2; send_mouse(); return 0; }
        to_game(e->button.x, e->button.y, &mouse_x, &mouse_y);
        mouse_b &= ~2; send_mouse();                 /* chosen where it was clicked */
        menu_sticky = 2;
        return 1;
    }
    int menuview_is_on(void);
    if (!right || config_get_int("original", 0) || menuview_is_on()) return 0;   /* (the menus drawn by Darklands+ stay open by themselves) */
    if (e->type == SDL_MOUSEBUTTONDOWN) { rdown_t = plat_ticks_ms(); rup_wait = 0; return 0; }
    if (plat_ticks_ms() - rdown_t < 350) {          /* short: held a little longer, until the game has its bar up */
        rup_wait = plat_ticks_ms();
        return 1;
    }
    return 0;
}
/* each poll: a short right click becomes an open menu once the game shows its bar (or is let go) */
static void menu_click_poll(void)
{
    if (!rup_wait) return;
    if (menu_bar_up()) {
        rup_wait = 0; menu_sticky = 1;
        static int told;                            /* the game takes no keys while its menus are open: say how they close */
        if (told++ < 2 && !getenv("DL_CTL")) { void ui_notice(const char *); ui_notice("The menu stays open (Darklands+): click an entry to choose it; click anywhere else, or press any key, to close it."); }
        return;
    }
    if (plat_ticks_ms() - rup_wait > 200) { rup_wait = 0; mouse_b &= ~2; send_mouse(); }
}
/* Esc with the menus open: let go away from them */
static int menu_escape(void)
{
    if (menu_sticky != 1) return 0;
    char a[32]; snprintf(a, sizeof a, "%d,%d", mouse_x, mouse_y);
    mouse_y = 199; send_mouse();
    later(60, "release", "2"); later(120, "move", a);
    menu_sticky = 0;
    return 1;
}

static SDL_Scancode swallow_key; static int swallow_button;   /* the key or click that stopped "go to" */
/* ---------------------------------------------------------------- game controllers
 * A controller (Xbox / PlayStation / Switch pads, the Steam Deck in gamepad mode) plays the game
 * as a mouse and a few keys, made into the same events, so the game, the menus and everything of
 * Darklands+ answer as they do to the mouse and keyboard:
 *   left stick   the pointer (left trigger held: slowly)   right stick  the wheel (lists)
 *   A            click                                      B            right click (the menu bar)
 *   X            Enter                                      Y            Esc
 *   d-pad        arrow keys                                 LB / RB      Tab (level map) / Space (pause)
 *   Start        F12 (Darklands+ menu)                      Back         M (world map)
 *   right trigger held: Ctrl (Ctrl+click on the map travels there)   left stick pressed: F1 (keys)
 * Always on; F12 > Settings > Controller shows the buttons and remaps them. */
static SDL_GameController *pads[4];
static int pad_test_axis[4] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };   /* test: axis values set by ucmd upadaxis */
static double pad_fx, pad_fy, pad_wheel; static u32 pad_last; static int pad_ctrl;
/* button prompts (plat_prompts): the controller drawn on screen while it is the one in use - the
   last input came from it, not from the keyboard or the mouse - in the look of its kind */
static int input_pad, pad_style = 1;           /* (1 Xbox, 2 PlayStation, 3 Nintendo) */
static int pad_injected;                       /* key and button events the controller pushed, not yet taken */
static int pad_warp_x = -1, pad_warp_y = -1;   /* where the stick last put the pointer */
static int style_of(SDL_GameController *g)
{
#if SDL_VERSION_ATLEAST(2, 0, 12)
    switch (SDL_GameControllerGetType(g)) {
    case SDL_CONTROLLER_TYPE_PS3: case SDL_CONTROLLER_TYPE_PS4:
#if SDL_VERSION_ATLEAST(2, 0, 14)
    case SDL_CONTROLLER_TYPE_PS5:
#endif
        return 2;
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
#if SDL_VERSION_ATLEAST(2, 24, 0)
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT: case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT: case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
#endif
        return 3;
    default: return 1;
    }
#else
    (void)g; return 1;
#endif
}
/* 0: keys and mouse; 1 Xbox, 2 PlayStation, 3 Nintendo (input.prompts: 0 as the input in use, 1 keys
   and mouse always, 2..4 a controller's always - a Steam Deck's trackpads count as a mouse) */
int plat_prompts(void)
{
    int m = config_get_int("input.prompts", 0);
    if (m >= 2 && m <= 4) return m - 1;
    if (m == 1) return 0;
    return input_pad ? pad_style : 0;
}
static void pad_used(SDL_JoystickID id)
{
    input_pad = 1;
    for (int i = 0; i < 4; i++) if (pads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i])) == id) pad_style = style_of(pads[i]);
}

static int pad_enabled(void) { return !headless; }      /* (always; input.gamepad retired) */
static void pad_key(SDL_Scancode sc, int up)
{
    SDL_Event ev; memset(&ev, 0, sizeof ev);
    ev.type = up ? SDL_KEYUP : SDL_KEYDOWN; ev.key.windowID = win ? SDL_GetWindowID(win) : 0;
    ev.key.state = up ? SDL_RELEASED : SDL_PRESSED;
    ev.key.keysym.scancode = sc; ev.key.keysym.sym = SDL_GetKeyFromScancode(sc); ev.key.keysym.mod = (Uint16)SDL_GetModState();
    if (SDL_PushEvent(&ev) == 1) pad_injected++;
}
static void pad_button(int b, int up)
{
    int x, y; SDL_GetMouseState(&x, &y);
    SDL_Event ev; memset(&ev, 0, sizeof ev);
    ev.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN; ev.button.windowID = win ? SDL_GetWindowID(win) : 0;
    ev.button.button = (Uint8)b; ev.button.state = up ? SDL_RELEASED : SDL_PRESSED; ev.button.clicks = 1; ev.button.x = x; ev.button.y = y;
    if (SDL_PushEvent(&ev) == 1) pad_injected++;
}
static int pad_axis(int a)
{
    if (pad_test_axis[a] != 0x7fffffff) return pad_test_axis[a];
    static const SDL_GameControllerAxis ax[4] = { SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTY, SDL_CONTROLLER_AXIS_RIGHTY, SDL_CONTROLLER_AXIS_TRIGGERLEFT };
    int best = 0;
    for (int i = 0; i < 4; i++) if (pads[i]) { int v = SDL_GameControllerGetAxis(pads[i], ax[a]); if (abs(v) > abs(best)) best = v; }
    return best;
}
static double pad_curve(int v)                 /* -1..1 with a dead zone and a gentle start */
{
    double x = v / 32767.0, m = fabs(x);
    if (m < 0.15) return 0;
    m = (m - 0.15) / 0.85; m = m * m * 0.7 + m * 0.3;
    return x < 0 ? -m : m;
}
/* every pass: the sticks move the pointer and turn the wheel */
static void pad_tick(void)
{
    u32 now = plat_ticks_ms(); double dt = pad_last ? (now - pad_last) / 1000.0 : 0; pad_last = now;
    if (dt > 0.1) dt = 0.1;
    if (!pad_enabled() || !win) return;
    if (!(SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS) && !getenv("DL_CTL")) return;   /* another window has the focus */
    int any = 0; for (int i = 0; i < 4; i++) any |= pads[i] != 0;
    if (!any && pad_test_axis[0] == 0x7fffffff && pad_test_axis[1] == 0x7fffffff && pad_test_axis[2] == 0x7fffffff) return;
    double sx = pad_curve(pad_axis(0)), sy = pad_curve(pad_axis(1)), wy = pad_curve(pad_axis(2));
    int lt = pad_axis(3) > 12000;
    int w, h; SDL_GetWindowSize(win, &w, &h);
    double speed = h * 0.85 * (lt ? 0.3 : 1.0);   /* window heights per second at full tilt */
    if (sx || sy) {
        int x, y; SDL_GetMouseState(&x, &y);
        pad_fx += sx * speed * dt; pad_fy += sy * speed * dt;
        int dx = (int)pad_fx, dy = (int)pad_fy;
        if (dx || dy) {
            pad_fx -= dx; pad_fy -= dy;
            x += dx; y += dy; if (x < 0) x = 0; if (y < 0) y = 0; if (x >= w) x = w - 1; if (y >= h) y = h - 1;
            SDL_WarpMouseInWindow(win, x, y);
            pad_warp_x = x; pad_warp_y = y; input_pad = 1;
            if (getenv("DL_CTL")) {                  /* tests (a virtual display may not report the warp) */
                SDL_Event ev; memset(&ev, 0, sizeof ev); ev.type = SDL_MOUSEMOTION; ev.motion.windowID = SDL_GetWindowID(win); ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
            }
        }
    } else pad_fx = pad_fy = 0;
    if (wy) {
        pad_wheel += -wy * 8 * dt;                   /* up to 8 notches a second */
        int n = (int)pad_wheel;
        if (n) {
            pad_wheel -= n;
            SDL_Event ev; memset(&ev, 0, sizeof ev); ev.type = SDL_MOUSEWHEEL; ev.wheel.windowID = SDL_GetWindowID(win); ev.wheel.y = n; SDL_PushEvent(&ev);
        }
    } else pad_wheel = 0;
}
/* controller events become mouse and key events; 1 if the event was a controller's */
/* the buttons the player may give other jobs (F12 > Settings > Controller):
   A, B, X, Y, LB, RB, Start, Back, the left stick pressed (in the order of ui/padglyphs.h's PB_A ...
   PB_LS); each does one of the same nine jobs, its own unless changed (pad.bind.N). The d-pad, the
   sticks, the right trigger and the right stick pressed keep theirs. */
#define PAD_BINDS 9
static const int PAD_PHYS[PAD_BINDS] = { SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_LEFTSTICK };
int plat_pad_bind(int k)
{
    char key[16]; snprintf(key, sizeof key, "pad.bind.%d", k);
    int a = config_get_int(key, k);
    return a >= 0 && a < PAD_BINDS ? a : k;
}
void plat_pad_bind_set(int k, int a) { char key[16]; snprintf(key, sizeof key, "pad.bind.%d", k); config_set_int(key, a); }
static void pad_do(int a, int up)
{
    switch (a) {
    case 0: pad_button(SDL_BUTTON_LEFT, up); break;
    case 1: pad_button(SDL_BUTTON_RIGHT, up); break;
    case 2: pad_key(SDL_SCANCODE_RETURN, up); break;
    case 3: pad_key(SDL_SCANCODE_ESCAPE, up); break;
    case 4: pad_key(SDL_SCANCODE_TAB, up); break;
    case 5: pad_key(SDL_SCANCODE_SPACE, up); break;
    case 6: pad_key(SDL_SCANCODE_F12, up); break;
    case 7: pad_key(SDL_SCANCODE_M, up); break;
    case 8: pad_key(SDL_SCANCODE_F11, up); break;   /* (the keys of the moment: F1 ... F5 are the game's) */
    }
}
/* the controller's name and kind (1 Xbox, 2 PlayStation, 3 Nintendo), 0 when none is connected */
int plat_controller_kind(const char **name)
{
    for (int i = 0; i < 4; i++) if (pads[i]) { if (name) *name = SDL_GameControllerName(pads[i]); return style_of(pads[i]); }
    if (name) *name = "";
    return 0;
}
static int pad_event(const SDL_Event *e)
{
    if (e->type == SDL_CONTROLLERDEVICEADDED) {
        for (int i = 0; i < 4; i++) if (!pads[i]) { pads[i] = SDL_GameControllerOpen(e->cdevice.which);
            if (pads[i]) {
                fprintf(stderr, "controller: %s\n", SDL_GameControllerName(pads[i]));
                void pad_hello(void); pad_hello();
            }
            break; }
        return 1;
    }
    if (e->type == SDL_CONTROLLERDEVICEREMOVED) {
        for (int i = 0; i < 4; i++) if (pads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i])) == e->cdevice.which) { SDL_GameControllerClose(pads[i]); pads[i] = 0; }
        return 1;
    }
    if (e->type == SDL_CONTROLLERAXISMOTION) {
        if (pad_enabled() && (e->caxis.value > 12000 || e->caxis.value < -12000)) pad_used(e->caxis.which);
        if (e->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT && pad_enabled()) {
            int on = e->caxis.value > 12000;
            if (on != pad_ctrl) { pad_ctrl = on; SDL_SetModState(on ? (SDL_Keymod)(SDL_GetModState() | KMOD_LCTRL) : (SDL_Keymod)(SDL_GetModState() & ~KMOD_CTRL)); }
        }
        return 1;
    }
    if (e->type != SDL_CONTROLLERBUTTONDOWN && e->type != SDL_CONTROLLERBUTTONUP) return 0;
    if (!pad_enabled()) return 1;
    int up = e->type == SDL_CONTROLLERBUTTONUP;
    if (!up) pad_used(e->cbutton.which);
    for (int k = 0; k < PAD_BINDS; k++) if (e->cbutton.button == PAD_PHYS[k]) { pad_do(plat_pad_bind(k), up); return 1; }
    switch (e->cbutton.button) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP: pad_key(SDL_SCANCODE_UP, up); break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: pad_key(SDL_SCANCODE_DOWN, up); break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: pad_key(SDL_SCANCODE_LEFT, up); break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: pad_key(SDL_SCANCODE_RIGHT, up); break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: pad_key(SDL_SCANCODE_Z, up); break;   /* (Z: fast forward in a battle) */
    default: break;
    }
    return 1;
}
/* once, a few seconds after the start (or when one is plugged in): which controller, if any */
void ui_pad_hello(int kind, const char *name);   /* (ui/overlay.cpp) */
static int hello_done;
void pad_hello(void)
{
    if (hello_done || headless) return;
    const char *nm; int k = plat_controller_kind(&nm);
    if (!k && plat_ticks_ms() < 4000) return;
    hello_done = 1;
    ui_pad_hello(k, nm);
}
int plat_controller_connected(void) { for (int i = 0; i < 4; i++) if (pads[i]) return 1; return 0; }

void plat_events(void)
{
    SDL_Event e;
    run_ctl();
    run_script();
    menu_click_poll();
    if (!hello_done) pad_hello();
    pad_tick();
    while (SDL_PollEvent(&e)) {
        if (pad_event(&e)) continue;
        /* the keyboard or the mouse in use again (not what the controller pushed or the stick did) */
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            if (pad_injected > 0) pad_injected--;
            else if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) input_pad = 0;
        } else if (e.type == SDL_MOUSEMOTION && input_pad && (abs(e.motion.x - pad_warp_x) > 2 || abs(e.motion.y - pad_warp_y) > 2)
                   && (abs(e.motion.xrel) > 3 || abs(e.motion.yrel) > 3)) input_pad = 0;
        if (!headless && ui_event(&e)) { if (e.type == SDL_MOUSEBUTTONUP) swallow_click = 0; continue; }   /* (a release taken by the interface ends the swallowing too) */
        if (swallow_click && (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)) { if (e.type == SDL_MOUSEBUTTONUP) swallow_click = 0; continue; }
        if (drag_filter(&e)) continue;
        switch (e.type) {
        case SDL_QUIT: fprintf(stderr, "program exit: window closed\n"); plat_quit(0); break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST && !getenv("DL_SCRIPT") && !getenv("DL_CTL") && config_get_int("display.pause_unfocused", 1)) {
                bg_paused = 1; fprintf(stderr, "window in the background: the game waits\n");
                menu_escape();                      /* a menu left open by a click closes, choosing nothing */
            }
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED && bg_paused) { bg_paused = 0; bg_resumed = plat_ticks_ms(); mouse_b = 0; }
            if ((e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED || e.window.event == SDL_WINDOWEVENT_SHOWN || e.window.event == SDL_WINDOWEVENT_RESTORED
                 || e.window.event == SDL_WINDOWEVENT_MAXIMIZED) && ren && win)
                resync_surface(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED ? "focus" : e.window.event == SDL_WINDOWEVENT_SHOWN ? "shown" :
                               e.window.event == SDL_WINDOWEVENT_RESTORED ? "restored" : "maximized");
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_MAXIMIZED || e.window.event == SDL_WINDOWEVENT_RESTORED
                || e.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED) {
                layout();                           /* the mouse is placed with the new sizes at once, not from the next frame */
                fprintf(stderr, "window: %s, window %dx%d, drawing surface %dx%d, picture %d,%d %dx%d\n",
                        e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ? "size" : e.window.event == SDL_WINDOWEVENT_MAXIMIZED ? "maximized" :
                        e.window.event == SDL_WINDOWEVENT_RESTORED ? "restored" : "display", win_w, win_h, out_w, out_h, dest.x, dest.y, dest.w, dest.h);
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED || e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    config_set_int("display.maximized", e.window.event == SDL_WINDOWEVENT_MAXIMIZED); config_save();
                }
            }
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED && !(SDL_GetWindowFlags(win) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MAXIMIZED))) {
                static u32 last_save; u32 t = plat_ticks_ms();
                config_set_int("display.window_w", (int)(e.window.data1 / dpi_k + 0.5f)); config_set_int("display.window_h", (int)(e.window.data2 / dpi_k + 0.5f));
                if (t - last_save > 1000) { config_save(); last_save = t; }
            }
            break;
        case SDL_KEYDOWN:
            {   /* a key while "go to" walks only stops it (the game does not see it: it could pick an option) */
                int citynav_busy(void); void citynav_user_input(void);
                if (citynav_busy()) { if (!e.key.repeat) citynav_user_input(); swallow_key = e.key.keysym.scancode; break; }
            }
            if (e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)) {
                Uint32 fl = SDL_GetWindowFlags(win);
                SDL_SetWindowFullscreen(win, (fl & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                config_set_int("display.fullscreen", !(fl & SDL_WINDOW_FULLSCREEN_DESKTOP)); config_save();
                break;
            }
            if (e.key.keysym.sym == SDLK_F12 && (e.key.keysym.mod & KMOD_CTRL)) {
                extern int hdtext_dump; hdtext_dump = 1;   /* and what the crisp text made of the screen, into the log (for reports) */
                fprintf(stderr, "screenshot: window %dx%d, drawing surface %dx%d, picture %d,%d %dx%d\n", win_w, win_h, out_w, out_h, dest.x, dest.y, dest.w, dest.h);
                {   int pw = 0, ph = 0, mx, my; float dd = 0; if (win) SDL_GetWindowSizeInPixels(win, &pw, &ph); SDL_GetMouseState(&mx, &my);
                    if (win) SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(win), &dd, 0, 0);
                    fprintf(stderr, "screenshot: window in pixels %dx%d, display dpi %.0f, mouse at %d,%d (furthest seen %d,%d) -> game %d,%d, flags %#x\n",
                            pw, ph, dd, mx, my, mouse_far_x, mouse_far_y, mouse_x, mouse_y, win ? (unsigned)SDL_GetWindowFlags(win) : 0u); }
                if (ren) pending_shot = 1; else save_bmp("screenshot.bmp");
                break;
            }
            if (!e.key.repeat && menu_escape()) break;     /* any key closes menus a right click left open (the game would ignore it) */
            { int saves_key(int, int); if (!e.key.repeat && saves_key(e.key.keysym.sym, (e.key.keysym.mod & KMOD_CTRL) != 0)) break; }
            {   /* 1..9 on a card: that option (a click on its line); 0 the tenth */
                SDL_Keycode k = e.key.keysym.sym; int d = k >= SDLK_1 && k <= SDLK_9 ? k - SDLK_0 : k >= SDLK_KP_1 && k <= SDLK_KP_9 ? k - SDLK_KP_1 + 1 : k == SDLK_0 || k == SDLK_KP_0 ? 10 : 0;
                int gx, gy, assist_card_key(int, int *, int *);
                if (d && !e.key.repeat && !(e.key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) && assist_card_key(d, &gx, &gy)) { click_at(gx, gy); break; }
            }
            key(e.key.keysym.scancode, 0);
            break;
        case SDL_KEYUP: if (swallow_key && e.key.keysym.scancode == swallow_key) { swallow_key = 0; break; } key(e.key.keysym.scancode, 1); break;
        case SDL_MOUSEWHEEL:
            if (menu_escape()) break;
            /* the wheel in the game's lists (the character sheet's equipment, formulae and saints,
               the shops' wares): Up / Down, the keys those lists answer; only on such screens, not
               on cards, the map or in battles, where the arrow keys do other things */
            if (e.wheel.y && game_mode() == GM_SCREEN && !config_get_int("original", 0)) {
                int n = e.wheel.y > 0 ? e.wheel.y : -e.wheel.y; if (n > 3) n = 3;
                for (int i = 0; i < n; i++) plat_press_key(e.wheel.y > 0 ? SDL_SCANCODE_UP : SDL_SCANCODE_DOWN);
            }
            break;
        case SDL_MOUSEMOTION:
            if (e.motion.x > mouse_far_x) mouse_far_x = e.motion.x; if (e.motion.y > mouse_far_y) mouse_far_y = e.motion.y;
            to_game(e.motion.x, e.motion.y, &mouse_x, &mouse_y);
            ptr_out = plat_win_outside(e.motion.x, e.motion.y);
            send_mouse(); break;
        case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
            if (bg_resumed && plat_ticks_ms() - bg_resumed < 400) {       /* the click that brought the window back */
                if (e.type == SDL_MOUSEBUTTONUP) bg_resumed = 0;
                break;
            }
            bg_resumed = 0;
            {   extern int opening_playing;                         /* a click ends the opening, as a key (mods/opening.c) */
                if (opening_playing) { if (e.type == SDL_MOUSEBUTTONDOWN) bios_kbd_inject(0x3920); break; } }
            if (menu_click(&e)) break;
            {   /* a click while "go to" walks only stops it (and its release goes nowhere) */
                int citynav_busy(void); void citynav_user_input(void);
                if (e.type == SDL_MOUSEBUTTONDOWN && citynav_busy()) { citynav_user_input(); swallow_button |= 1 << e.button.button; break; }
                if (e.type == SDL_MOUSEBUTTONUP && (swallow_button & (1 << e.button.button))) { swallow_button &= ~(1 << e.button.button); break; }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && (SDL_GetModState() & KMOD_CTRL)) {
                int gx, gy; to_game(e.button.x, e.button.y, &gx, &gy);
                void travel_click(int, int); travel_click(gx, gy);       /* Ctrl+click on the map: travel there */
                break;
            }
            int b = e.button.button == SDL_BUTTON_LEFT ? 1 : e.button.button == SDL_BUTTON_RIGHT ? 2 : 4;
            if (e.type == SDL_MOUSEBUTTONDOWN) mouse_b |= b; else mouse_b &= ~b;
            send_mouse(); break; }
        default: break;
        }
    }
}

void plat_click_game(int gx, int gy) { click_at(gx, gy); }

/* the game set its pointer (int 33h, 4: a card's option chosen with the arrow keys, the start of
 * a battle, the saints' list): the system pointer is
 * moved there too, so the two never part (it is hidden over the picture, but the next move of
 * the mouse would make the game's pointer jump back to it) */
void plat_mouse_warp(int gx, int gy)
{
    if (headless || !win) return;
    mouse_x = gx; mouse_y = gy;
    if (ui_is_open() || SDL_GetMouseFocus() != win || (SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS) == 0) return;
    float wx, wy; plat_game_to_win(gx + 0.5f, gy + 0.5f, &wx, &wy);
    SDL_WarpMouseInWindow(win, (int)wx, (int)wy);
}
/* a key press for the game (released a little later) */
void plat_press_key(int scancode)
{
    key((SDL_Scancode)scancode, 0);
    Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
    r->t = plat_ticks_ms() + 60; snprintf(r->cmd, sizeof r->cmd, "up"); snprintf(r->arg, sizeof r->arg, "%s", SDL_GetScancodeName((SDL_Scancode)scancode));
    for (int k = nsteps - 1; k > step_i && steps[k].t < steps[k - 1].t; k--) { Step t = steps[k]; steps[k] = steps[k - 1]; steps[k - 1] = t; }
}

/* a left click at game pixel (x, y), released a little later so the game sees it */
static void click_at(int x, int y)
{
    mouse_x = x; mouse_y = y; send_mouse();
    mouse_b |= 1; send_mouse();
    u32 now = plat_ticks_ms();
    Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
    r->t = now + 80; snprintf(r->cmd, sizeof r->cmd, "release"); snprintf(r->arg, sizeof r->arg, "1");
    for (int k = nsteps - 1; k > step_i && steps[k].t < steps[k - 1].t; k--) { Step t = steps[k]; steps[k] = steps[k - 1]; steps[k - 1] = t; }
}

/* ---------------------------------------------------------------- init / quit */
SDL_Renderer *plat_renderer(void) { return ren; }
void plat_init(int scale)
{
    headless = getenv("DL_HEADLESS") != 0;
    if (headless) SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
#ifdef _WIN32
    /* the window in the screen's own pixels, so Windows' 125 % / 150 % scaling does not stretch
       (and blur) the picture and the crisp text; the menus grow by that scale instead (ui.cpp) */
    if (config_get_int("display.dpi_aware", 1)) SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }
    t0 = SDL_GetPerformanceCounter(); freq = SDL_GetPerformanceFrequency();
    if (!headless && SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) fprintf(stderr, "no game controllers: %s\n", SDL_GetError());
    if (!scale) return;
    if (!headless) {
        int wp = getenv("DL_SCRIPT") ? 0 : SDL_WINDOWPOS_CENTERED;   /* scripted tests: a known window position */
        /* the window as it was left: size and full screen (display.window_w/h, display.fullscreen) */
        /* the size is kept in 96-dpi units (as earlier versions, which Windows scaled, saved it) */
#ifdef _WIN32
        { float ddpi = 0; if (config_get_int("display.dpi_aware", 1) && SDL_GetDisplayDPI(0, &ddpi, 0, 0) == 0 && ddpi > 96) dpi_k = ddpi / 96.0f; }
#endif
        int ww = (int)(config_get_int("display.window_w", 0) * dpi_k + 0.5f), wh = (int)(config_get_int("display.window_h", 0) * dpi_k + 0.5f);
        if (ww <= 0 || wh <= 0) {   /* the first start: 4:3, four fifths of the screen's height */
            SDL_DisplayMode dm; ww = 960; wh = 720;
            if (!getenv("DL_SCRIPT") && !getenv("DL_CTL") && SDL_GetDesktopDisplayMode(0, &dm) == 0 && dm.h >= 600) {   /* (tests: 960 x 720) */ wh = dm.h * 4 / 5; ww = wh * 4 / 3; if (ww > dm.w * 9 / 10) { ww = dm.w * 9 / 10; wh = ww * 3 / 4; } }
        }
        if (ww < 320 || ww > 16384) ww = 960; if (wh < 200 || wh > 16384) wh = 720;
        /* maximized unless the player made the window smaller (display.maximized; the side margins
           show from the start); tests keep a known size */
        int maxi = config_get_int("display.maximized", 1) && !getenv("DL_SCRIPT") && !getenv("DL_CTL");
        win = SDL_CreateWindow("Darklands", wp, wp, ww, wh,
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (maxi ? SDL_WINDOW_MAXIMIZED : 0) |
                               (config_get_int("display.fullscreen", 0) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
        if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); exit(1); }
#ifdef _WIN32
        /* Direct3D 11: SDL's default on Windows is Direct3D 9, whose drawing surface could stay at
           the old size when the window was maximized (the picture then shown stretched while the
           mouse was measured in window pixels: the pointer at half its place). display.renderer
           names another (direct3d, opengl ...) */
        {   const char *want = config_get("display.renderer", "direct3d11");
            for (int i = 0; want[0] && i < SDL_GetNumRenderDrivers(); i++) {
                SDL_RendererInfo ri; if (SDL_GetRenderDriverInfo(i, &ri) == 0 && !strcmp(ri.name, want)) {
                    ren = SDL_CreateRenderer(win, i, SDL_RENDERER_ACCELERATED);
                    if (!ren) fprintf(stderr, "renderer %s did not start (%s): SDL's choice instead\n", want, SDL_GetError());
                    break;
                }
            }
        }
#endif
        if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        if (!ren) ren = SDL_CreateRenderer(win, -1, 0);
        {   SDL_RendererInfo ri; int a, b, c, d; SDL_GetRendererInfo(ren, &ri); SDL_GetWindowSize(win, &a, &b); SDL_GetRendererOutputSize(ren, &c, &d);
            fprintf(stderr, "renderer %s, window %dx%d, drawing surface %dx%d\n", ri.name, a, b, c, d); }
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 200);
        SDL_ShowCursor(SDL_DISABLE);
        ui_init(win, ren);
    }
    SDL_AddTimer(1, tick_cb, 0);
    parse_script(getenv("DL_SCRIPT"));
}

/* start the game again (a new process in this one's place): after the party's end the original
 * leaves to DOS; here the title comes back, where a saved game can be loaded */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
void plat_setenv(const char *k, const char *v) { SDL_setenv(k, v, 1); }
void plat_restart(void)
{
    extern int main_argc; extern char **main_argv;
    fflush(stdout); fflush(stderr);
    SDL_Quit();
#ifdef _WIN32
    SetEnvironmentVariableA("DL_RESTARTED", "1");
    STARTUPINFOW si; PROCESS_INFORMATION pi; memset(&si, 0, sizeof si); si.cb = sizeof si;
    if (CreateProcessW(NULL, GetCommandLineW(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) { CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
    exit(0);
#else
    setenv("DL_RESTARTED", "1", 1);
    (void)main_argc;
    execv("/proc/self/exe", main_argv);
    execvp(main_argv[0], main_argv);
    exit(0);
#endif
}

void plat_quit(int code)
{
    fflush(stdout); fflush(stderr);
    SDL_Quit();
    exit(code);
}

void plat_error_box(const char *msg)
{
    char buf[700];
    snprintf(buf, sizeof buf, "The game stopped because of an internal error:\n\n%s\n\n"
             "Details are in darklands_port.log, in the game folder (or, if Darklands+ may not write there, in its own data folder).", msg);
    if (!headless) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", buf, win);
}
