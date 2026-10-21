// gamepics.cpp - the game's pictures as textures (see gamepics.h).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/hle.h"
u8 *hdart_decode_pic(const char *dosname, int *w, int *h, u32 *pal, u8 *mask);
}

static SDL_Renderer *gp_ren;
static std::map<std::string, GamePic> cache;

void gamepics_set_renderer(SDL_Renderer *r) { gp_ren = r; }
void gamepics_flush(void)
{
    for (auto &e : cache) if (e.second.tex) SDL_DestroyTexture(e.second.tex);
    cache.clear();
}

struct Img { std::vector<u8> px; int w = 0, h = 0; u32 pal[256]; u8 mask[256]; };
static bool load(const std::string &name, Img &im)
{
    memset(im.mask, 0, sizeof im.mask);
    u8 *p = hdart_decode_pic(name.c_str(), &im.w, &im.h, im.pal, im.mask);
    if (!p) return false;
    im.px.assign(p, p + (size_t)im.w * im.h);
    free(p);
    return true;
}

/* the colours: the picture's own, then palpic's, then the game's now */
static void palette(const Img &im, const char *palpic, u32 *out)
{
    Img pp; bool have_pp = palpic && load(palpic, pp);
    for (int i = 0; i < 256; i++) {
        if (im.mask[i]) out[i] = im.pal[i];
        else if (have_pp && pp.mask[i]) out[i] = pp.pal[i];
        else {
            u32 r = vga_dac[i * 3] & 63, g = vga_dac[i * 3 + 1] & 63, b = vga_dac[i * 3 + 2] & 63;
            out[i] = 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (b << 2 | b >> 4);
        }
    }
}

static const GamePic *upload(const std::string &key, const std::vector<u32> &rgba, int w, int h)
{
    GamePic g; g.w = w; g.h = h;
    if (gp_ren && w > 0 && h > 0) {
        g.tex = SDL_CreateTexture(gp_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
        if (g.tex) {
            SDL_UpdateTexture(g.tex, 0, rgba.data(), w * 4);
            SDL_SetTextureBlendMode(g.tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(g.tex, SDL_ScaleModeNearest);
        }
    }
    return &(cache[key] = g);
}

const GamePic *gamepic_layers(const std::vector<std::string> &names, const char *palpic)
{
    if (names.empty()) return nullptr;
    std::string key = palpic ? std::string(palpic) + "|" : std::string("|");
    for (auto &n : names) key += n + "+";
    auto it = cache.find(key);
    if (it != cache.end()) return it->second.tex ? &it->second : nullptr;
    Img base;
    if (!load(names[0], base)) { cache[key] = GamePic(); return nullptr; }
    u32 pal[256]; palette(base, palpic, pal);
    std::vector<u32> rgba((size_t)base.w * base.h);
    for (size_t i = 0; i < rgba.size(); i++) rgba[i] = base.px[i] ? pal[base.px[i]] : 0;
    for (size_t k = 1; k < names.size(); k++) {
        Img l;
        if (!load(names[k], l)) continue;
        u32 lp[256]; palette(l, palpic, lp);
        for (int y = 0; y < l.h && y < base.h; y++)
            for (int x = 0; x < l.w && x < base.w; x++) { u8 v = l.px[(size_t)y * l.w + x]; if (v) rgba[(size_t)y * base.w + x] = lp[v]; }
    }
    const GamePic *g = upload(key, rgba, base.w, base.h);
    return g->tex ? g : nullptr;
}

const GamePic *gamepic(const std::string &name, const char *palpic) { return gamepic_layers({ name }, palpic); }

const GamePic *gamepic_part(const std::string &name, int x0, int y0, int w, int h, const char *palpic)
{
    char kb[64]; snprintf(kb, sizeof kb, "@%d,%d,%d,%d", x0, y0, w, h);
    std::string key = (palpic ? std::string(palpic) : std::string()) + "|" + name + kb;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second.tex ? &it->second : nullptr;
    Img im;
    if (!load(name, im) || w <= 0 || h <= 0) { cache[key] = GamePic(); return nullptr; }
    u32 pal[256]; palette(im, palpic, pal);
    std::vector<u32> rgba((size_t)w * h, 0);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int sx = x0 + x, sy = y0 + y;
            if (sx < 0 || sy < 0 || sx >= im.w || sy >= im.h) continue;
            u8 v = im.px[(size_t)sy * im.w + sx];
            rgba[(size_t)y * w + x] = pal[v];                 /* (a part is opaque: a statue's niche) */
        }
    const GamePic *g = upload(key, rgba, w, h);
    return g->tex ? g : nullptr;
}

/* a part with its grey left out (see-through): a word painted on grey stone, cut out */
const GamePic *gamepic_cutout(const std::string &name, int x0, int y0, int w, int h, int grey_tol)
{
    char kb[64]; snprintf(kb, sizeof kb, "#cut%d@%d,%d,%d,%d", grey_tol, x0, y0, w, h);
    std::string key = name + kb;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second.tex ? &it->second : nullptr;
    Img im;
    if (!load(name, im) || w <= 0 || h <= 0) { cache[key] = GamePic(); return nullptr; }
    u32 pal[256]; palette(im, nullptr, pal);
    std::vector<u32> rgba((size_t)w * h, 0);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int sx = x0 + x, sy = y0 + y;
            if (sx < 0 || sy < 0 || sx >= im.w || sy >= im.h) continue;
            u32 c = pal[im.px[(size_t)sy * im.w + sx]];
            int r = c >> 16 & 255, g = c >> 8 & 255, b = c & 255;
            int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
            if (mx - mn > grey_tol || mx < 72) rgba[(size_t)y * w + x] = c | 0xff000000u;   /* (the colours, and the dark outlines) */
        }
    const GamePic *g = upload(key, rgba, w, h);
    return g->tex ? g : nullptr;
}

/* test: a picture written as a PNG in the colours the game shows it in now (ucmd upicpng:NAME,SCALE) */
extern "C" int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
extern "C" int gamepics_write_png(const char *name, const char *out, int scale)
{
    Img im; if (!load(name, im)) return 0;
    u32 pal[256]; palette(im, nullptr, pal);
    if (scale < 1) scale = 1;
    int W = im.w * scale, H = im.h * scale;
    std::vector<u8> rgba((size_t)W * H * 4);
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
        u8 c = im.px[(size_t)(y / scale) * im.w + x / scale]; u32 v = pal[c];
        u8 *d = &rgba[((size_t)y * W + x) * 4];
        d[0] = (u8)(v >> 16); d[1] = (u8)(v >> 8); d[2] = (u8)v; d[3] = c ? 255 : 0;
    }
    return stbi_write_png(out, W, H, 4, rgba.data(), W * 4);
}
