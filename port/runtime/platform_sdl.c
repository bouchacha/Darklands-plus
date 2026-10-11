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
#include <SDL.h>
#include "cpu.h"
#include "hle.h"
#include "../ui/ui.h"
#include "../mods/hdart.h"
#include "../mods/hdtext.h"
#include "game.h"
#include "config.h"

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;
static u32 frame[320 * 200];
static int headless;
static int mouse_x = 160, mouse_y = 100, mouse_b;

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
static char pending_wshot[300];     /* whole-window screenshot (interface included) at the next present */
static int out_w = 960, out_h = 720, win_w = 960, win_h = 720;
static void layout(void)
{
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
        SDL_Rect r = { 0, 0, 640, 400 };
        SDL_UpdateTexture(own[t], &r, px + (ty * 400) * w + tx * 640, w * 4);
        tiles[t] = own[t]; tile_src[t] = r;
        tile_dst[t] = (SDL_FRect){ (float)tx / n, (float)ty / n, 1.0f / n, 1.0f / n };
    }
    return n * n;
}

void plat_present(const u8 *vga, const u8 *pal)
{
    { void fixes_frame(void); fixes_frame(); void saves_frame(void); saves_frame(); }   /* game-side checks, headless too */
    { static int slow = -1; if (slow < 0) slow = getenv("DL_SLOW_PRESENT") ? atoi(getenv("DL_SLOW_PRESENT")) : 0; if (slow) plat_sleep_ms(slow); }   /* test: a slow display */
    if (headless) return;
    static int ptime = -1; if (ptime < 0) ptime = getenv("DL_PRESENT_TIME") != 0;
    uint64_t pt0 = ptime ? plat_ticks_us() : 0;
    to_rgb(vga, pal);
    static u8 idx[320 * 200];
    memcpy(idx, vga, sizeof idx);
    { void hdtext_prepare(u32 *, const u32 *, u8 *); hdtext_prepare(frame, lut, idx); }   /* crisp text: wipe the glyphs it redraws */
    hdart_prepare(frame, idx, lut);                 /* replacement art: clear the pixels it covers */
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
        SDL_RenderCopyF(ren, tiles[t], &tile_src[t], &d);
    }
    uint64_t pt2 = ptime ? plat_ticks_us() : 0;
    ui_draw(ren);
    uint64_t pt3 = ptime ? plat_ticks_us() : 0;
    if (pending_wshot[0]) {
        SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, out_w, out_h, 32, SDL_PIXELFORMAT_ARGB8888);
        if (sf) { SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ARGB8888, sf->pixels, sf->pitch);
                  SDL_SaveBMP(sf, pending_wshot); SDL_FreeSurface(sf); fprintf(stderr, "saved %s\n", pending_wshot); }
        pending_wshot[0] = 0;
    }
    SDL_RenderPresent(ren);
    if (ptime) { static uint64_t acc, mx, cnt, a1, a2, a3; uint64_t t4 = plat_ticks_us(), d = t4 - pt0; acc += d; if (d > mx) mx = d;
                 a1 += pt1 - pt0; a2 += pt3 - pt2; a3 += t4 - pt3;
                 if (++cnt % 100 == 0) { fprintf(stderr, "present: %.2f ms average (prepare %.2f, ui %.2f, present %.2f), %.2f ms worst\n", acc / 1000.0 / 100, a1 / 1e5, a2 / 1e5, a3 / 1e5, mx / 1000.0); acc = mx = a1 = a2 = a3 = 0; } }
}

/* game pixel (gx, gy) -> window coordinates, where the interface draws (valid after the
 * first present) */
void plat_game_to_win(float gx, float gy, float *wx, float *wy)
{
    float ox = dest.x + gx * dest.w / 320.0f, oy = dest.y + gy * dest.h / 200.0f;
    *wx = out_w ? ox * win_w / out_w : ox;
    *wy = out_h ? oy * win_h / out_h : oy;
}

/* window coordinates -> game pixels */
static void to_game(int wx, int wy, int *gx, int *gy)
{
    int ox = win_w ? wx * out_w / win_w : wx, oy = win_h ? wy * out_h / win_h : wy;
    int x = dest.w ? (ox - dest.x) * 320 / dest.w : 0, y = dest.h ? (oy - dest.y) * 200 / dest.h : 0;
    if (x < 0) x = 0; if (x > 319) x = 319; if (y < 0) y = 0; if (y > 199) y = 199;
    *gx = x; *gy = y;
}

void plat_mouse_game(int *x, int *y) { *x = mouse_x; *y = mouse_y; }

int plat_menu_open(void) { return !headless && ui_is_open(); }

/* ---------------------------------------------------------------- events */
static void click_at(int x, int y);
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
            if (strcmp(s->cmd, "up")) key(sc, 0);
            if (!strcmp(s->cmd, "up")) key(sc, 1);
            else if (!strcmp(s->cmd, "key")) {      /* release a little later, like a real key press */
                Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
                r->t = now + 60; snprintf(r->cmd, sizeof r->cmd, "up"); snprintf(r->arg, sizeof r->arg, "%s", s->arg);
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
        else if (!strcmp(s->cmd, "shot")) save_bmp(s->arg);
        else if (!strcmp(s->cmd, "utab")) ui_select_tab(atoi(s->arg));
        else if (!strcmp(s->cmd, "travel")) { void travel_click(int, int); travel_click(x, y); }   /* as Ctrl+click at game x,y */
        else if (!strcmp(s->cmd, "travelto")) { int travel_to_loc(int, const char *); travel_to_loc(atoi(s->arg), "test"); }
        else if (!strcmp(s->cmd, "travelnear")) { int travel_nearest(int); travel_nearest(atoi(s->arg)); }   /* nearest location of a type */
        else if (!strcmp(s->cmd, "ugive")) { int a = 0, b = 0, c = 0; sscanf(s->arg, "%d,%d,%d", &a, &b, &c); void ui_give(int, int, int); ui_give(a, b, c); }
        else if (!strcmp(s->cmd, "uclick")) {            /* a click on the interface, window pixels */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            Uint32 wid = win ? SDL_GetWindowID(win) : 0;
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = wid; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
            memset(&ev, 0, sizeof ev); ev.type = SDL_MOUSEBUTTONDOWN; ev.button.windowID = wid; ev.button.button = SDL_BUTTON_LEFT; ev.button.state = SDL_PRESSED;
            ev.button.x = x; ev.button.y = y; ev.button.clicks = 1; SDL_PushEvent(&ev);
            ev.type = SDL_MOUSEBUTTONUP; ev.button.state = SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "uhover")) {            /* the pointer over the interface, window pixels */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = win ? SDL_GetWindowID(win) : 0; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "wshot")) snprintf(pending_wshot, sizeof pending_wshot, "%s", s->arg);
        else if (!strcmp(s->cmd, "artexport")) { char m[400]; hdart_export(m, sizeof m); fprintf(stderr, "%s\n", m); }
        else if (!strcmp(s->cmd, "artrescan")) hdart_rescan();
        else if (!strcmp(s->cmd, "ufight")) { extern int ufight_pending, ufight_seed; ufight_seed = -1; sscanf(s->arg, "%d,%d", &ufight_pending, &ufight_seed); if (!ufight_pending) ufight_pending = 10; }   /* test: a battle (type, seed) at the next card */
        else if (!strcmp(s->cmd, "upeek")) {               /* debug: words at link-time seg:off (+LOADSEG) */
            unsigned sg = 0, of = 0, n = 8; sscanf(s->arg, "%x,%x,%u", &sg, &of, &n);
            fprintf(stderr, "peek %04x:%04x:", sg, of);
            for (unsigned k = 0; k < n; k++) fprintf(stderr, " %04x", rw((u16)(LOADSEG + sg), (u16)(of + 2 * k)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(s->cmd, "upotion")) {             /* test: member m drinks potion code of quality q (use_potion, s031 1462:43b2) */
            int m = 0, code = 0x69, q = 30; sscanf(s->arg, "%d,%x,%d", &m, &code, &q);
            int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            if (m >= 0 && m < 5 && n < 64) {
                u16 o = G_ITEM(m, n);
                ww(G_FAR2E38, o, (u16)code); wb(G_FAR2E38, (u16)(o + 2), 0); wb(G_FAR2E38, (u16)(o + 3), (u8)q); wb(G_FAR2E38, (u16)(o + 4), 1); wb(G_FAR2E38, (u16)(o + 5), 0);
                g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)(n + 1));
                u16 a[2] = { (u16)m, (u16)n };
                game_call_far(31, 0x1462, 0x43b2, 2, a);
                g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)n);
                fprintf(stderr, "potion %x q%d for %d: bonus byte %02x\n", code, q, m, g_rb((u16)(G_MEMBER(m) + 0x34)));
            }
        }
        else if (!strcmp(s->cmd, "uhit")) {                /* test (in battle): hit_damage(a, t, r, loc), s014 18bf:0778 */
            int a = 0, t = 5, r = 1, loc = 0; sscanf(s->arg, "%d,%d,%d,%d", &a, &t, &r, &loc);
            u16 args[4] = { (u16)a, (u16)t, (u16)r, (u16)loc };
            int v = game_call_far(14, 0x18bf, 0x0778, 4, args);
            fprintf(stderr, "hit %d on %d: %d (AX %d)\n", a, t, v, (s16)AX);
        }
        else if (!strcmp(s->cmd, "uquicksave")) { void saves_quick_save(void); saves_quick_save(); }
        else if (!strcmp(s->cmd, "uquickload")) { void saves_quick_load(void); saves_quick_load(); }
        else if (!strcmp(s->cmd, "uload")) { int saves_load(const char *); saves_load(s->arg); }
        else if (!strcmp(s->cmd, "ubonus")) {              /* test: the members' bonus bytes (party +0x34) */
            fprintf(stderr, "bonus:");
            for (int m = 0; m < 5; m++) fprintf(stderr, " %02x", g_rb((u16)(G_MEMBER(m) + 0x34)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(s->cmd, "uclock")) {              /* test: the clock moves on h hours (no events run) */
            static const int D[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            int h = (s16)g_rw(G_CUR_HOUR) + atoi(s->arg), d = (s16)g_rw(G_CUR_DAY), mo = (s16)g_rw(G_CUR_MONTH), y = (s16)g_rw(G_CUR_YEAR);
            while (h >= 24) { h -= 24; if (++d > D[mo]) { d = 1; if (++mo >= 12) { mo = 0; y++; } } }
            g_ww(G_CUR_HOUR, (u16)h); g_ww(G_CUR_DAY, (u16)d); g_ww(G_CUR_MONTH, (u16)mo); g_ww(G_CUR_YEAR, (u16)y);
            fprintf(stderr, "clock %d:00 day %d month %d year %d\n", h, d, mo, y);
        }
        else if (!strcmp(s->cmd, "upoke")) {               /* debug: byte at link-time seg:off (+LOADSEG) */
            unsigned sg = 0, of = 0, v = 0; sscanf(s->arg, "%x,%x,%x", &sg, &of, &v);
            wb((u16)(LOADSEG + sg), (u16)of, (u8)v);
        }
        else if (!strcmp(s->cmd, "umode")) { int game_mode(void); fprintf(stderr, "mode %d\n", game_mode()); }   /* 1 card 2 map 3 screen 4 battle */
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
            fprintf(stderr, "text end\n");
        }
        else if (!strcmp(s->cmd, "ucard")) {               /* test: the card's name and its option lines (option id, text) */
            int n = (s8)g_rb(0xe96e);
            fprintf(stderr, "card %.12s n %d\n", (const char *)M + LIN(G_DGROUP, 0xe3d4), n);
            for (int i = 0; i < n && i < 20; i++) {
                int y = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i)), opt = (s8)g_rb((u16)(0xe9b0 + i));
                char t[256] = "";
                for (int k = 0; k < hdtext_nruns; k++)
                    if (hdtext_runs[k].y >= y - 1 && hdtext_runs[k].y < y + h - 1 && hdtext_runs[k].x >= 60 && strlen(t) + strlen(hdtext_runs[k].text) < 250) { if (*t) strcat(t, " | "); strcat(t, hdtext_runs[k].text); }
                fprintf(stderr, "  line %d opt %d y %d h %d on %d: %s\n", i, opt, y, h, opt >= 0 && opt < 20 ? g_rb((u16)(0xe99c + opt)) : -1, t);
            }
        }
        else if (!strcmp(s->cmd, "ugoto")) { void citynav_go(int); citynav_go(atoi(s->arg)); }   /* test: Go to place N in the city */
        else if (!strcmp(s->cmd, "ugotowin")) { void ui_open_goto(void); ui_open_goto(); }       /* test: the Go to window (G) */
        else if (!strcmp(s->cmd, "umap")) { void ui_toggle_map(void); ui_toggle_map(); }
        else if (!strcmp(s->cmd, "ucardkey")) { int gx, gy, assist_card_key(int, int *, int *); if (assist_card_key(atoi(s->arg), &gx, &gy)) click_at(gx, gy); else fprintf(stderr, "ucardkey %s: no option\n", s->arg); }
        else if (!strcmp(s->cmd, "quit")) plat_quit(0);
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

void plat_events(void)
{
    SDL_Event e;
    run_ctl();
    run_script();
    while (SDL_PollEvent(&e)) {
        if (!headless && ui_event(&e)) continue;
        switch (e.type) {
        case SDL_QUIT: plat_quit(0); break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED && !(SDL_GetWindowFlags(win) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MAXIMIZED))) {
                static u32 last_save; u32 t = plat_ticks_ms();
                config_set_int("display.window_w", e.window.data1); config_set_int("display.window_h", e.window.data2);
                if (t - last_save > 1000) { config_save(); last_save = t; }
            }
            break;
        case SDL_KEYDOWN:
            { void citynav_user_input(void); if (!e.key.repeat) citynav_user_input(); }
            if (e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)) {
                Uint32 fl = SDL_GetWindowFlags(win);
                SDL_SetWindowFullscreen(win, (fl & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                config_set_int("display.fullscreen", !(fl & SDL_WINDOW_FULLSCREEN_DESKTOP)); config_save();
                break;
            }
            if (e.key.keysym.sym == SDLK_F12 && (e.key.keysym.mod & KMOD_CTRL)) { save_bmp("screenshot.bmp"); break; }
            { int saves_key(int, int); if (!e.key.repeat && saves_key(e.key.keysym.sym, (e.key.keysym.mod & KMOD_CTRL) != 0)) break; }
            {   /* 1..9 on a card: that option (a click on its line) */
                SDL_Keycode k = e.key.keysym.sym; int d = k >= SDLK_1 && k <= SDLK_9 ? k - SDLK_0 : k >= SDLK_KP_1 && k <= SDLK_KP_9 ? k - SDLK_KP_1 + 1 : 0;
                int gx, gy, assist_card_key(int, int *, int *);
                if (d && !e.key.repeat && !(e.key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) && assist_card_key(d, &gx, &gy)) { click_at(gx, gy); break; }
            }
            key(e.key.keysym.scancode, 0);
            break;
        case SDL_KEYUP: key(e.key.keysym.scancode, 1); break;
        case SDL_MOUSEMOTION:
            to_game(e.motion.x, e.motion.y, &mouse_x, &mouse_y);
            send_mouse(); break;
        case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
            if (e.type == SDL_MOUSEBUTTONDOWN) { void citynav_user_input(void); citynav_user_input(); }
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
void plat_init(int scale)
{
    headless = getenv("DL_HEADLESS") != 0;
    if (headless) SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }
    t0 = SDL_GetPerformanceCounter(); freq = SDL_GetPerformanceFrequency();
    if (!scale) return;
    if (!headless) {
        int wp = getenv("DL_SCRIPT") ? 0 : SDL_WINDOWPOS_CENTERED;   /* scripted tests: a known window position */
        /* the window as it was left: size and full screen (display.window_w/h, display.fullscreen) */
        int ww = config_get_int("display.window_w", 960), wh = config_get_int("display.window_h", 720);
        if (ww < 320 || ww > 16384) ww = 960; if (wh < 200 || wh > 16384) wh = 720;
        win = SDL_CreateWindow("Darklands", wp, wp, ww, wh,
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (config_get_int("display.fullscreen", 0) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
        if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); exit(1); }
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        if (!ren) ren = SDL_CreateRenderer(win, -1, 0);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 200);
        SDL_ShowCursor(SDL_DISABLE);
        ui_init(win, ren);
    }
    SDL_AddTimer(1, tick_cb, 0);
    parse_script(getenv("DL_SCRIPT"));
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
             "Details are in darklands_port.log in the game folder.", msg);
    if (!headless) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", buf, win);
}
