/* crt.c - the screen effect over the game's picture (display.crt, F12 > Settings > Screen):
 * 0 none, 1 scanlines, 2 CRT, 3 curved CRT. A player asked for DOSBox Staging's crt-auto look;
 * its shaders are GLSL, and the picture here is drawn through SDL's renderer (Direct3D on Windows), which
 * takes no shaders, so the look is built from what the renderer does draw:
 *   - scanlines: a picture the size of the game's on screen, laid over it multiplying (SDL_BLENDMODE_MOD):
 *     each of the game's 200 rows bright in its middle, darker towards its edges (from twice the game's
 *     size up; below that the rows are too thin for it);
 *   - CRT: the same, with an aperture grille (screen columns tinted red, green, blue in turn, from three
 *     times the game's size up) and a glow: the picture again, at half its size and smoothed, added faintly,
 *     which makes up the brightness the lines take away and lets bright pixels bleed a little;
 *   - curved CRT: all of it drawn into a picture of its own, then onto the screen as a mesh bent outwards
 *     towards the middle of each edge, its corners darker.
 * Only the game's own pixels: what Darklands+ draws over them (its screens, tooltips, the margins) stays
 * sharp. platform_sdl.c calls crt_begin before the picture and crt_end after it. */
#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "config.h"

static int mode_now(void) { int m = config_get_int("display.crt", 0); return m < 0 || m > 3 ? 0 : m; }

static SDL_Texture *overlay, *glow, *target;
static int ov_w, ov_h, ov_mode, tg_w, tg_h;

/* the multiplying picture: scanlines (and the grille) for a picture w x h on screen */
static void overlay_make(SDL_Renderer *ren, int w, int h, int mode)
{
    if (overlay && ov_w == w && ov_h == h && ov_mode == mode) return;
    if (overlay) { SDL_DestroyTexture(overlay); overlay = 0; }
    ov_w = w; ov_h = h; ov_mode = mode;
    if (w <= 0 || h <= 0) return;
    overlay = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (!overlay) return;
    u32 *px = malloc((size_t)w * h * 4);
    if (!px) { SDL_DestroyTexture(overlay); overlay = 0; return; }
    double rows = h / 200.0, depth = rows >= 2 ? (mode == 1 ? 0.45 : 0.40) : 0;
    int grille = mode >= 2 && w >= 960;
    const double M = 0.72;                                /* (the grille's two other colours in a column) */
    for (int y = 0; y < h; y++) {
        double p = (y + 0.5) * 200.0 / h, f = p - floor(p), d = fabs(f - 0.5) * 2;
        double s = 1 - depth * d * d;
        for (int x = 0; x < w; x++) {
            double r = s, g = s, b = s;
            if (grille) { int c = x % 3; r *= c == 0 ? 1 : M; g *= c == 1 ? 1 : M; b *= c == 2 ? 1 : M; }
            px[(size_t)y * w + x] = 0xff000000u | (u32)(r * 255 + 0.5) << 16 | (u32)(g * 255 + 0.5) << 8 | (u32)(b * 255 + 0.5);
        }
    }
    SDL_UpdateTexture(overlay, 0, px, w * 4);
    free(px);
    SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_MOD);
}

int crt_begin(SDL_Renderer *ren, SDL_Rect *dest)
{
    if (mode_now() != 3 || dest->w <= 0 || dest->h <= 0) return 0;
    if (!target || tg_w != dest->w || tg_h != dest->h) {
        if (target) SDL_DestroyTexture(target);
        target = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, dest->w, dest->h);
        tg_w = dest->w; tg_h = dest->h;
        if (!target) return 0;
    }
    if (SDL_SetRenderTarget(ren, target) != 0) return 0;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    dest->x = 0; dest->y = 0;
    return 1;
}

void crt_end(SDL_Renderer *ren, SDL_Rect *dest, SDL_Rect saved, const u32 *frame, int into_target)
{
    int mode = mode_now();
    if (!mode) return;
    SDL_Rect d = *dest;
    if (mode >= 2) {                                      /* the glow: half size, smoothed, added faintly */
        if (!glow) { glow = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 100);
                     if (glow) { SDL_SetTextureBlendMode(glow, SDL_BLENDMODE_ADD); SDL_SetTextureScaleMode(glow, SDL_ScaleModeLinear); } }
        if (glow) {
            static u32 half[160 * 100];
            for (int y = 0; y < 100; y++)
                for (int x = 0; x < 160; x++) {
                    const u32 *a = frame + (2 * y) * 320 + 2 * x, *b = a + 320;
                    u32 r = ((a[0] >> 16 & 255) + (a[1] >> 16 & 255) + (b[0] >> 16 & 255) + (b[1] >> 16 & 255)) / 4;
                    u32 g = ((a[0] >> 8 & 255) + (a[1] >> 8 & 255) + (b[0] >> 8 & 255) + (b[1] >> 8 & 255)) / 4;
                    u32 bl = ((a[0] & 255) + (a[1] & 255) + (b[0] & 255) + (b[1] & 255)) / 4;
                    half[y * 160 + x] = 0xff000000u | r << 16 | g << 8 | bl;
                }
            SDL_UpdateTexture(glow, 0, half, 160 * 4);
            SDL_SetTextureAlphaMod(glow, 80);
            SDL_RenderCopy(ren, glow, 0, &d);
        }
    }
    overlay_make(ren, d.w, d.h, mode);
    if (overlay) SDL_RenderCopy(ren, overlay, 0, &d);
    if (!into_target) return;
    /* the curved screen: the picture drawn as a mesh, each point pulled towards the middle by its distance
       from it squared (barrel: the corners most, so the edges bow outwards), darker towards the corners */
    SDL_SetRenderTarget(ren, 0);
    *dest = saved;
    enum { NX = 32, NY = 24 };
    static SDL_Vertex v[(NX + 1) * (NY + 1)];
    static int idx[NX * NY * 6];
    const float k = 0.06f, cx = saved.x + saved.w * 0.5f, cy = saved.y + saved.h * 0.5f;
    for (int j = 0; j <= NY; j++)
        for (int i = 0; i <= NX; i++) {
            float u = (float)i / NX, t = (float)j / NY, px = u * 2 - 1, py = t * 2 - 1, r2 = px * px + py * py;
            float s = (1 - k * r2) / (1 - k);
            SDL_Vertex *q = &v[j * (NX + 1) + i];
            q->position.x = cx + px * s * saved.w * 0.5f;
            q->position.y = cy + py * s * saved.h * 0.5f;
            q->tex_coord.x = u; q->tex_coord.y = t;
            float e = r2 * r2 / 4, lit = 1 - 0.28f * e;
            Uint8 c = (Uint8)(255 * (lit < 0 ? 0 : lit));
            q->color.r = q->color.g = q->color.b = c; q->color.a = 255;
        }
    int n = 0;
    for (int j = 0; j < NY; j++)
        for (int i = 0; i < NX; i++) {
            int a = j * (NX + 1) + i, b = a + 1, c = a + NX + 1, e = c + 1;
            idx[n++] = a; idx[n++] = b; idx[n++] = c; idx[n++] = b; idx[n++] = e; idx[n++] = c;
        }
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(target, SDL_ScaleModeLinear);
    SDL_RenderGeometry(ren, target, v, (NX + 1) * (NY + 1), idx, n);
}
