/* hdart.c - replacement art. A picture file PICS\NAME.PIC can be replaced by art\NAME.png
 * of any size (true colour, with transparency); it is drawn at window resolution wherever the
 * game shows that picture, with the game's own pixels on top wherever something else covers
 * it (text, the mouse pointer, other pictures without replacements).
 *
 * How pictures reach the screen (root 0617 and the graphics driver thunks in 047c):
 *  - pic_draw(page, x, y, name, mode) 0617:01b2 unpacks a PIC row by row onto a page
 *    (047c:054c (row, page, x, y, w)); backgrounds and card pictures go this way;
 *  - pic_load(name) 0617:045c unpacks it into a sprite (047c:05bc (w, h), 05b5 (row) per row,
 *    05ae) and returns its handle in AX; save_rect(page, x, y, w, h) 047c:04d5 makes a
 *    sprite of a page rectangle (handle in AX); gfx_draw_pic(win, x, y, handle) 047c:05a5
 *    draws a sprite (colour 0 transparent) into a window;
 *  - gfx_copy_rect(src, sx, sy, w, h, dst, dx, dy) 047c:0553 copies between windows.
 *    Window record (DGROUP): +0 page, +2/+4 origin, +6/+8 last x/y (relative).
 * Each page keeps a list of regions "picture P with its origin at (ox, oy)", carried along by
 * those calls. Page 0 is the screen: when a frame is shown, every region there is checked
 * pixel by pixel against the picture, and only the pixels that still match are replaced. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "hdart.h"
#include "hdtext.h"
#include "datamods.h"
extern u16 drvseg_mgraphic;

extern char game_dir[1024];
int hdart_on = 1;
int hdart_show_names;
static int logon;

/* ---------------------------------------------------------------- pictures */
typedef struct {
    char name[16];                  /* upper case, no path or extension */
    int w, h, sprite;
    u8 *px;
    u32 hist[256];
    u32 ref[256]; double refb;      /* brightest palette seen with it (or its own) */
    char *art;                      /* replacement file, if any */
    int baked;                      /* has words painted in (BAKED below) */
} Pic;
static Pic *pics; static int npics, cappics;

/* ---------------------------------------------------------------- replacement files */
typedef struct { char key[16]; char *path; } ArtFile;
static ArtFile *arts; static int narts, caparts;

static void art_add(const char *path, const char *file)
{
    const char *dot = strrchr(file, '.');
    if (!dot || (strcasecmp(dot, ".png") && strcasecmp(dot, ".jpg") && strcasecmp(dot, ".bmp") && strcasecmp(dot, ".tga"))) return;
    int n = (int)(dot - file); if (n <= 0 || n >= 16) return;
    char key[16]; for (int i = 0; i < n; i++) key[i] = (char)toupper((unsigned char)file[i]); key[n] = 0;
    for (int i = 0; i < narts; i++) if (!strcmp(arts[i].key, key)) return;
    if (narts == caparts) { caparts = caparts ? caparts * 2 : 64; arts = realloc(arts, sizeof *arts * caparts); }
    snprintf(arts[narts].key, 16, "%s", key);
    arts[narts].path = strdup(path);
    narts++;
}

static void scan_dir(const char *dir, int depth)
{
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    int nn = 0;
    char **mine = malloc(sizeof(char *) * 4096);
    if (!mine) { closedir(d); return; }
    while ((e = readdir(d)) && nn < 4096) if (e->d_name[0] != '.') mine[nn++] = strdup(e->d_name);
    closedir(d);
    for (int a = 1; a < nn; a++) for (int b = a; b > 0 && strcasecmp(mine[b - 1], mine[b]) > 0; b--) { char *t = mine[b]; mine[b] = mine[b - 1]; mine[b - 1] = t; }
    for (int i = 0; i < nn; i++) {
        char p[1400]; snprintf(p, sizeof p, "%s/%s", dir, mine[i]);
        struct stat st;
        if (!stat(p, &st)) {
            if (S_ISDIR(st.st_mode)) { if (depth < 3 && strcasecmp(mine[i], "original")) scan_dir(p, depth + 1); }
            else art_add(p, mine[i]);
        }
        free(mine[i]);
    }
    free(mine);
}

static const char *art_for(const char *key)
{
    for (int i = 0; i < narts; i++) if (!strcmp(arts[i].key, key)) return arts[i].path;
    return 0;
}

int hdart_count(void) { return narts; }
int hdart_generation;

void hdart_rescan(void)
{
    for (int i = 0; i < narts; i++) free(arts[i].path);
    narts = 0;
    static char md[DATAMODS_MAX][1200];
    int nm = datamods_dirs("art", md, DATAMODS_MAX);      /* the mods' art first: the first file of a name wins */
    for (int i = 0; i < nm; i++) scan_dir(md[i], 0);
    char p[1100]; snprintf(p, sizeof p, "%s/art", game_dir);
    scan_dir(p, 0);
    for (int i = 0; i < npics; i++) { free(pics[i].art); const char *a = art_for(pics[i].name); pics[i].art = a ? strdup(a) : 0; }
    hdart_generation++;
    if (logon) fprintf(stderr, "art: %d replacement pictures in %s\n", narts, p);
}

const char *hdart_file(int pic) { return pic >= 0 && pic < npics ? pics[pic].art : 0; }
const char *hdart_name(int pic) { return pic >= 0 && pic < npics ? pics[pic].name : ""; }
void hdart_size(int pic, int *w, int *h) { *w = pics[pic].w; *h = pics[pic].h; }

/* ---------------------------------------------------------------- PIC files */
static int read_file(const char *path, u8 **data, long *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
    *data = malloc((size_t)*n + 1);
    int ok = *data && fread(*data, 1, (size_t)*n, f) == (size_t)*n;
    fclose(f);
    if (!ok) { free(*data); *data = 0; }
    return ok;
}

/* LZW (LSB-first codes, 9 bits growing to max_bits, then a fresh dictionary) over RLE
 * (0x90 n: the previous byte n times in all; 0x90 0: a literal 0x90) */
typedef struct { u8 *img; long op, npx; int last, rle; } Out;
static void emit(Out *o, int v)
{
    if (o->rle) {
        o->rle = 0;
        if (v == 0) { if (o->op < o->npx) o->img[o->op++] = 0x90; o->last = 0x90; }
        else for (int k = 1; k < v && o->op < o->npx && o->last >= 0; k++) o->img[o->op++] = (u8)o->last;
    } else if (v == 0x90) o->rle = 1;
    else { if (o->op < o->npx) o->img[o->op++] = (u8)v; o->last = v; }
}

typedef struct { const u8 *s; long sn, si; u32 bitbuf; int nb, width, cnt, thr, maxb; } Bits;
static int next_code(Bits *b)
{
    while (b->nb < b->width && b->si < b->sn) { b->bitbuf |= (u32)b->s[b->si++] << b->nb; b->nb += 8; }
    if (b->nb < b->width) return -1;
    int c = (int)(b->bitbuf & ((1u << b->width) - 1));
    b->bitbuf >>= b->width; b->nb -= b->width;
    if (++b->cnt == b->thr) { b->cnt = 0; b->width++; b->thr <<= 1; if (b->width > b->maxb) { b->width = 9; b->thr = 256; } }
    return c;
}

static u8 *pic_decode(const u8 *d, long n, int *pw, int *ph, u32 *pal, u8 *palmask)
{
    long pos = 0;
    u8 *img = 0;
    if (palmask) memset(palmask, 0, 256);
    while (pos + 4 <= n) {
        int tag = d[pos] | d[pos + 1] << 8, len = d[pos + 2] | d[pos + 3] << 8;
        const u8 *b = d + pos + 4;
        if (pos + 4 + len > n) break;
        pos += 4 + len;
        if (tag == 0x304d && len >= 2 && pal) {
            int first = b[0], last = b[1];
            for (int i = first; i <= last && 2 + 3 * (i - first) + 2 < len; i++) {
                const u8 *c = b + 2 + 3 * (i - first);
                u32 r = c[0] & 63, g = c[1] & 63, bl = c[2] & 63;
                pal[i] = 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (bl << 2 | bl >> 4);
                palmask[i] = 1;
            }
        } else if (tag == 0x3058 && len >= 5 && !img) {
            int w = b[0] | b[1] << 8, h = b[2] | b[3] << 8, maxb = b[4];
            if (w <= 0 || h <= 0 || w > 2048 || h > 2048 || maxb < 9 || maxb > 12) return 0;
            static u16 prefix[4096]; static u8 suffix[4096], first[4096], stack[4096];
            Out o = { calloc((size_t)w * h, 1), 0, (long)w * h, -1, 0 };
            Bits bt = { b + 5, len - 5, 0, 0, 0, 9, 0, 256, maxb };
            int size = 1 << maxb;
            img = o.img;
            for (;;) {
                int c = next_code(&bt);
                if (c < 0 || c > 255) break;
                for (int k = 0; k < 256; k++) { suffix[k] = (u8)k; first[k] = (u8)k; }
                int tlen = 257, wcode = c, bad = 0;
                emit(&o, c);
                while (tlen < size) {
                    int k = next_code(&bt);
                    if (k < 0) break;
                    if (k > tlen || k == 256) { bad = 1; break; }
                    int code = k, sp = 0;
                    int fc = k < tlen ? first[k] : first[wcode];
                    if (k == tlen) { stack[sp++] = first[wcode]; code = wcode; }
                    while (code > 255 && sp < 4095) { stack[sp++] = suffix[code]; code = prefix[code]; }
                    stack[sp++] = (u8)code;
                    while (sp) emit(&o, stack[--sp]);
                    prefix[tlen] = (u16)wcode; suffix[tlen] = (u8)fc; first[tlen] = first[wcode]; tlen++;
                    wcode = k;
                }
                if (bad || tlen < size) break;
            }
            *pw = w; *ph = h;
        }
    }
    return img;
}

void dos_host_path(const char *dos, char *out, size_t outn);
void dos_host_path_read(const char *dos, char *out, size_t outn);

/* the palette a PIC file brings along (for colour fades: the art is drawn as bright as the
 * picture's colours are now, relative to these) */
static int file_palette(const char *dosname, u32 *pal, u8 *mask)
{
    char p[1200]; dos_host_path_read(dosname, p, sizeof p);
    u8 *d; long n;
    if (!read_file(p, &d, &n)) return 0;
    int w = 0, h = 0;
    u8 *img = pic_decode(d, n, &w, &h, pal, mask);
    free(img); free(d);
    for (int i = 0; i < 256; i++) if (mask[i]) return 1;
    return 0;
}

/* a PIC file of the game, unpacked (for other modules) */
u8 *hdart_decode_pic(const char *dosname, int *w, int *h, u32 *pal, u8 *mask)
{
    char p[1200]; dos_host_path_read(dosname, p, sizeof p);
    u8 *d; long n;
    if (!read_file(p, &d, &n)) return 0;
    u8 *img = pic_decode(d, n, w, h, pal, mask);
    free(d);
    return img;
}

static void base_name(const char *path, char *out)
{
    const char *b = path;
    for (const char *q = path; *q; q++) if (*q == '\\' || *q == '/' || *q == ':') b = q + 1;
    int n = 0;
    while (b[n] && b[n] != '.' && n < 15) { out[n] = (char)toupper((unsigned char)b[n]); n++; }
    out[n] = 0;
}

static double brightness(const Pic *P, const u32 *pal, int ch)
{
    double s = 0;
    for (int i = 0; i < 256; i++) {
        if (!P->hist[i]) continue;
        u32 c = pal[i];
        double v = ch < 0 ? (double)((c >> 16 & 255) + (c >> 8 & 255) + (c & 255)) : (double)(c >> (16 - 8 * ch) & 255);
        s += (double)P->hist[i] * v;
    }
    return s;
}

extern u8 vga_dac[768];
static void dac_lut(u32 *lut)
{
    for (int i = 0; i < 256; i++) {
        u32 r = vga_dac[i * 3] & 63, g = vga_dac[i * 3 + 1] & 63, b = vga_dac[i * 3 + 2] & 63;
        lut[i] = 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (b << 2 | b >> 4);
    }
}

/* a picture the game has just unpacked: find or add it */
static int pic_register(const char *dosname, int w, int h, const u8 *px, int sprite)
{
    char key[16];
    if (dosname && dosname[0]) base_name(dosname, key);
    else {                                          /* unnamed (unpacked from memory): by content */
        u32 hsh = 2166136261u;
        for (long i = 0; i < (long)w * h; i++) { hsh ^= px[i]; hsh *= 16777619u; }
        snprintf(key, sizeof key, "M%07X", hsh & 0xfffffff);
    }
    for (int i = 0; i < npics; i++) {
        Pic *P = &pics[i];
        if (!strcmp(P->name, key) && P->w == w && P->h == h && !memcmp(P->px, px, (size_t)w * h)) { P->sprite |= sprite; return i; }
    }
    if (npics == cappics) { cappics = cappics ? cappics * 2 : 256; pics = realloc(pics, sizeof *pics * cappics); }
    Pic *P = &pics[npics];
    memset(P, 0, sizeof *P);
    snprintf(P->name, sizeof P->name, "%s", key);
    P->w = w; P->h = h; P->sprite = sprite;
    P->px = malloc((size_t)w * h); memcpy(P->px, px, (size_t)w * h);
    for (long i = 0; i < (long)w * h; i++) P->hist[px[i]]++;
    const char *a = art_for(key); P->art = a ? strdup(a) : 0;
    { int has_baked(const char *); P->baked = has_baked(key); }
    u8 mask[256]; u32 fpal[256];
    dac_lut(P->ref);
    if (dosname && dosname[0] && file_palette(dosname, fpal, mask))
        for (int i = 0; i < 256; i++) if (mask[i]) P->ref[i] = fpal[i];
    P->refb = brightness(P, P->ref, -1);
    if (logon) fprintf(stderr, "art: picture %d %s %dx%d%s%s\n", npics, key, w, h, sprite ? " (sprite)" : "", P->art ? " - has art" : "");
    return npics++;
}

/* ---------------------------------------------------------------- new pictures
 * A picture the game asks for that is not among its files (PICS\SHIELDP.PIC, a crest beyond
 * the fifteen) is made from art\NAME.png: matched to the colours on screen, packed as a PIC
 * (RLE: 0x90 n repeats, LZW: 9-11 bit codes, LSB first, a fresh table every 1792 codes) and
 * kept in art\.generated. The art itself is then drawn at full resolution as usual. */
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *p);

typedef struct { u8 *b; long n, cap; u32 bits; int nb; } Buf;
static void put8(Buf *o, int v) { if (o->n == o->cap) { o->cap = o->cap ? o->cap * 2 : 4096; o->b = realloc(o->b, (size_t)o->cap); } o->b[o->n++] = (u8)v; }
static void put_code(Buf *o, int c, int width) { o->bits |= (u32)c << o->nb; o->nb += width; while (o->nb >= 8) { put8(o, (int)(o->bits & 255)); o->bits >>= 8; o->nb -= 8; } }

static long pic_encode(const u8 *px, int w, int h, u8 **out)
{
    Buf r = {0}, o = {0};
    long n = (long)w * h;
    for (long i = 0; i < n;) {                      /* RLE */
        long j = i; while (j < n && px[j] == px[i] && j - i < 255) j++;
        long run = j - i;
        if (run >= 4 && px[i] != 0x90) { put8(&r, px[i]); put8(&r, 0x90); put8(&r, (int)run); i = j; }
        else { if (px[i] == 0x90) { put8(&r, 0x90); put8(&r, 0); } else put8(&r, px[i]); i++; }
    }
    static short dict[2048][256];                   /* (code, next byte) -> code */
    int width = 9, cnt = 0, thr = 256;
    #define EMIT(c) do { put_code(&o, (c), width); if (++cnt == thr) { cnt = 0; width++; thr <<= 1; if (width > 11) { width = 9; thr = 256; } } } while (0)
    long i = 0;
    while (i < r.n) {
        for (int k = 0; k < 2048; k++) memset(dict[k], -1, sizeof dict[k]);
        int nxt = 257, w = r.b[i++], full = 0;
        while (i < r.n) {
            int c = r.b[i];
            if (dict[w][c] >= 0) { w = dict[w][c]; i++; continue; }
            EMIT(w);
            if (nxt >= 2048) { full = 1; break; }
            dict[w][c] = (short)nxt++;
            w = c; i++;
        }
        if (!full) EMIT(w);
    }
    #undef EMIT
    if (o.nb) put8(&o, (int)(o.bits & 255));
    Buf f = {0};                                    /* X0 chunk: w, h, max bits, data */
    long len = 5 + o.n;
    put8(&f, 0x58); put8(&f, 0x30); put8(&f, (int)(len & 255)); put8(&f, (int)(len >> 8));
    put8(&f, w & 255); put8(&f, w >> 8); put8(&f, h & 255); put8(&f, h >> 8); put8(&f, 11);
    for (long k = 0; k < o.n; k++) put8(&f, o.b[k]);
    free(r.b); free(o.b);
    *out = f.b;
    return len > 65535 ? -1 : f.n;
}

const char *hdart_virtual_file(const char *dosname)
{
    const char *dot = strrchr(dosname, '.');
    if (!dot || strcasecmp(dot, ".pic")) return 0;
    char key[16]; base_name(dosname, key);
    const char *art = art_for(key);
    if (!art) return 0;
    static char path[1300];
    char dir[1200]; snprintf(dir, sizeof dir, "%s/art/.generated", game_dir); MKDIR(dir);
    snprintf(path, sizeof path, "%s/%s.PIC", dir, key);
    struct stat sa, sg;
    if (!stat(path, &sg) && !stat(art, &sa) && sg.st_mtime >= sa.st_mtime) return path;   /* made already, art unchanged */
    FILE *f;
    int w, h, n;
    u8 *rgba = stbi_load(art, &w, &h, &n, 4);
    if (!rgba) return 0;
    /* the size of the game's picture: crests are 11x19 like SHIELDA; otherwise a twentieth */
    int tw = 11, th = 19;
    if (strncmp(key, "SHIELD", 6)) { tw = w / 5 > 0 ? w / 5 : 1; th = h / 5 > 0 ? h / 5 : 1; if (tw > 320) tw = 320; if (th > 200) th = 200; }
    u8 *px = malloc((size_t)tw * th);
    u32 pal[256]; dac_lut(pal);
    for (int y = 0; y < th; y++)
        for (int x = 0; x < tw; x++) {
            long r = 0, g = 0, b = 0, a = 0, c = 0;     /* average of the source area */
            for (int sy = y * h / th; sy < (y + 1) * h / th; sy++)
                for (int sx = x * w / tw; sx < (x + 1) * w / tw; sx++) {
                    const u8 *p = rgba + ((long)sy * w + sx) * 4;
                    r += p[0]; g += p[1]; b += p[2]; a += p[3]; c++;
                }
            if (!c || a / c < 128) { px[y * tw + x] = 0; continue; }
            r /= c; g /= c; b /= c;
            int best = 16; long bd = -1;
            for (int k = 16; k < 256; k++) {
                long dr = (long)(pal[k] >> 16 & 255) - r, dg = (long)(pal[k] >> 8 & 255) - g, db = (long)(pal[k] & 255) - b;
                long d = dr * dr * 3 + dg * dg * 4 + db * db * 2;
                if (bd < 0 || d < bd) { bd = d; best = k; }
            }
            px[y * tw + x] = (u8)best;
        }
    stbi_image_free(rgba);
    u8 *pic; long len = pic_encode(px, tw, th, &pic);
    free(px);
    if (len < 0) { free(pic); return 0; }
    f = fopen(path, "wb");
    if (!f) { free(pic); return 0; }
    fwrite(pic, 1, (size_t)len, f); fclose(f); free(pic);
    if (logon) fprintf(stderr, "art: made %s from %s (%dx%d)\n", path, art, tw, th);
    return path;
}

/* the crest chooser (character creation, s038): H steps the member's letter on and wraps
 * after 'O' (party record +1); when there are more crests (art\SHIELDP.png ...), the step
 * past 'O' goes to them instead */
static u8 crest_last[5];
static int crest_exists(int letter)
{
    char k[16]; snprintf(k, sizeof k, "SHIELD%c", letter);
    if (art_for(k)) return 1;
    char d[32], p[1200]; snprintf(d, sizeof d, "PICS\\SHIELD%c.PIC", letter); dos_host_path(d, p, sizeof p);
    FILE *f = fopen(p, "rb"); if (f) { fclose(f); return 1; }
    return 0;
}
static void crest_step(u16 name_off)
{
    char nm[64]; g_str(G_DGROUP, name_off, nm, sizeof nm);
    char key[16]; base_name(nm, key);
    if (strncmp(key, "SHIELD", 6) || !key[6] || key[7]) return;
    for (int m = 0; m < 5; m++) {
        u16 at = (u16)(G_MEMBER(m) + 1);
        u8 cur = g_rb(at), last = crest_last[m];
        if (cur == 'A' && last >= 'O' && last < 'Z' && crest_exists(last + 1) && (key[6] == 'A' || key[6] == 'a')) {
            g_wb(at, (u8)(last + 1));
            int p = (int)(strlen(nm) - 5);         /* "...shieldA.pic": the letter before ".pic" */
            if (p > 0) g_wb((u16)(name_off + p), (u8)(last + 1));
            cur = (u8)(last + 1);
            if (logon) fprintf(stderr, "art: crest of member %d: %c\n", m, cur);
        }
        crest_last[m] = cur;
    }
}

/* ---------------------------------------------------------------- regions */
typedef struct { int pic; s16 x, y, w, h, ox, oy; u8 opaque; } Reg;
#define NPAGES 8
#define MAXREG 2048
static Reg reg[NPAGES][MAXREG];
static int nreg[NPAGES];

static void reg_remove(int page, int i) { memmove(&reg[page][i], &reg[page][i + 1], sizeof(Reg) * (size_t)(nreg[page] - i - 1)); nreg[page]--; }

static void drop_covered(int page, int x, int y, int w, int h)
{
    for (int i = nreg[page] - 1; i >= 0; i--) {
        Reg *r = &reg[page][i];
        if (r->x >= x && r->y >= y && r->x + r->w <= x + w && r->y + r->h <= y + h) reg_remove(page, i);
    }
}

/* clip (x, y, w, h) to the box [cx0, cx1) x [cy0, cy1) */
static int clip(int *x, int *y, int *w, int *h, int cx0, int cy0, int cx1, int cy1)
{
    if (*x < cx0) { *w -= cx0 - *x; *x = cx0; }
    if (*y < cy0) { *h -= cy0 - *y; *y = cy0; }
    if (*x + *w > cx1) *w = cx1 - *x;
    if (*y + *h > cy1) *h = cy1 - *y;
    return *w > 0 && *h > 0;
}

static void reg_add(int page, Reg r, int cover)
{
    if (page < 0 || page >= NPAGES || r.pic < 0) return;
    int x = r.x, y = r.y, w = r.w, h = r.h;
    if (!clip(&x, &y, &w, &h, 0, 0, 320, 200)) return;
    if (!clip(&x, &y, &w, &h, r.ox, r.oy, r.ox + pics[r.pic].w, r.oy + pics[r.pic].h)) return;   /* never outside the picture */
    r.x = (s16)x; r.y = (s16)y; r.w = (s16)w; r.h = (s16)h;
    if (cover) drop_covered(page, x, y, w, h);
    for (int i = nreg[page] - 1; i >= 0; i--) {
        Reg *o = &reg[page][i];
        if (o->pic == r.pic && o->x == r.x && o->y == r.y && o->w == r.w && o->h == r.h && o->ox == r.ox && o->oy == r.oy) { reg_remove(page, i); break; }
    }
    if (nreg[page] == MAXREG) reg_remove(page, 0);
    reg[page][nreg[page]++] = r;
}

/* the regions of page within (x, y, w, h), moved by (dx, dy) */
static int reg_take(int page, int x, int y, int w, int h, int dx, int dy, Reg *out, int max)
{
    int n = 0;
    if (page < 0 || page >= NPAGES) return 0;
    for (int i = 0; i < nreg[page] && n < max; i++) {
        Reg r = reg[page][i];
        int rx = r.x, ry = r.y, rw = r.w, rh = r.h;
        if (!clip(&rx, &ry, &rw, &rh, x, y, x + w, y + h)) continue;
        r.x = (s16)(rx + dx); r.y = (s16)(ry + dy); r.w = (s16)rw; r.h = (s16)rh; r.ox = (s16)(r.ox + dx); r.oy = (s16)(r.oy + dy);
        out[n++] = r;
    }
    return n;
}

/* sprites: handle -> regions in sprite coordinates (save_rect, pic_load) */
#define SPRREG 24
typedef struct { u16 key; int n; Reg r[SPRREG]; } Spr;
#define MAXSPR 2048
static Spr spr[MAXSPR]; static int nspr, spr_next;
static short spr_index[65536];                  /* handle -> slot + 1 */
static Spr *spr_find(u16 key) { int i = spr_index[key]; return i ? &spr[i - 1] : 0; }
static Spr *spr_slot(u16 key)
{
    Spr *s = spr_find(key);
    if (s) return s;
    int i;
    if (nspr < MAXSPR) i = nspr++;
    else { i = spr_next; spr_next = (spr_next + 1) % MAXSPR; spr_index[spr[i].key] = 0; }
    spr[i].key = key; spr[i].n = 0;
    spr_index[key] = (short)(i + 1);
    return &spr[i];
}

/* battle sprites: frame segment -> IMC file and frame number (sprite_file_load, s014 0e76:3d2c,
 * fills frames[k*8+m] with the segments the frames were copied to); a frame at seg:0 is
 * u8 w, u8 h, then per row u8 count, u8 skip, u8 pixels[count] */
typedef struct { u16 seg; char name[12]; short n; int pic; u32 hash; } Frame;
#define MAXFRAME 8192
static Frame frame[MAXFRAME]; static int nframe, frame_next;
static short frame_index[65536];
static Frame *frame_find(u16 seg) { int i = frame_index[seg]; return i ? &frame[i - 1] : 0; }
static void frame_set(u16 seg, const char *name, int n)
{
    Frame *f = frame_find(seg);
    if (!f) {
        int i;
        if (nframe < MAXFRAME) i = nframe++;
        else { i = frame_next; frame_next = (frame_next + 1) % MAXFRAME; frame_index[frame[i].seg] = 0; }
        f = &frame[i]; f->seg = seg; frame_index[seg] = (short)(i + 1);
    }
    snprintf(f->name, sizeof f->name, "%s", name);
    f->n = (short)n; f->pic = -1; f->hash = 0;
}

/* ---------------------------------------------------------------- hooks */
#define ARG(k) rw(SS, (u16)(SP + 4 + 2 * (k)))
typedef struct { int on, page, x, y, w, h, rows, cap; char name[64]; u8 *px; } Cap;
static Cap cap;

static void cap_begin(u16 name_off)
{
    cap.on = 1; cap.page = -1; cap.x = cap.y = cap.w = cap.h = cap.rows = 0;
    cap.name[0] = 0;
    if (name_off) g_str(G_DGROUP, name_off, cap.name, sizeof cap.name);
}
static void cap_row(u16 buf, int w)
{
    if (!cap.on || w <= 0 || w > 2048) return;
    if (!cap.rows) cap.w = w;
    if (w != cap.w || cap.rows >= 2048) { cap.on = 0; return; }
    if ((long)(cap.rows + 1) * w > cap.cap) { cap.cap = (cap.rows + 64) * w; cap.px = realloc(cap.px, (size_t)cap.cap); }
    for (int i = 0; i < w; i++) cap.px[(long)cap.rows * w + i] = rb(G_DGROUP, (u16)(buf + i));
    cap.rows++;
}

static int win_page(u16 w) { return (s16)g_rw(w); }
static void win_box(u16 w, int *ox, int *oy, int *x1, int *y1)
{
    *ox = (s16)g_rw((u16)(w + 2)); *oy = (s16)g_rw((u16)(w + 4));
    *x1 = *ox + (s16)g_rw((u16)(w + 6)) + 1; *y1 = *oy + (s16)g_rw((u16)(w + 8)) + 1;
}

static void cap_to_page(void)
{
    if (cap.on && cap.rows && cap.page >= 0) {
        int id = pic_register(cap.name, cap.w, cap.rows, cap.px, 0);
        Reg r = { id, (s16)cap.x, (s16)cap.y, (s16)cap.w, (s16)cap.rows, (s16)cap.x, (s16)cap.y, 1 };
        reg_add(cap.page, r, 1);
        if (logon) fprintf(stderr, "art: %s on page %d at %d,%d\n", pics[id].name, cap.page, cap.x, cap.y);
        if (getenv("DL_PIC_HASH")) {
            u32 h1 = 2166136261u; for (long i = 0; i < (long)cap.w * cap.rows; i++) h1 = (h1 ^ cap.px[i]) * 16777619u;
            u32 h2 = 2166136261u; u16 seg = cap.page == 0 ? 0xa000 : rw(drvseg_mgraphic, (u16)(0x19b0 + 2 * cap.page));
            for (int y = 0; y < cap.rows && cap.y + y < 200; y++) for (int x = 0; x < cap.w && cap.x + x < 320; x++) h2 = (h2 ^ M[((u32)seg << 4) + (cap.y + y) * 320 + cap.x + x]) * 16777619u;
            u32 plat_ticks_ms(void);
            fprintf(stderr, "pichash %u %s %dx%d decoded %08x page %d %08x\n", plat_ticks_ms(), cap.name, cap.w, cap.rows, h1, cap.page, h2);
        }
    }
    cap.on = 0;
}
static void h_pic_draw(void) { cap_begin(ARG(3)); hook_call_original(); cap_to_page(); }   /* pic_draw(page, x, y, name, mode) */
static void h_pic_open(void) { cap_begin(ARG(0)); hook_call_original(); cap_to_page(); }   /* pic_draw_open(name, page) */
static void h_pic_mem(void) { cap_begin(0); hook_call_original(); cap_to_page(); }         /* pic_draw_mem(seg, page) */
static void h_row(void)                     /* 047c:054c (row, page, x, y, w) */
{
    hdtext_cover((s16)ARG(1), (s16)ARG(2), (s16)ARG(3), (s16)ARG(4), 1);
    if (cap.on) {
        if (!cap.rows) { cap.page = (s16)ARG(1); cap.x = (s16)ARG(2); cap.y = (s16)ARG(3); }
        cap_row(ARG(0), (s16)ARG(4));
    }
    hook_call_original();
}

static void h_pic_load(void)                /* pic_load(name) -> sprite handle */
{
    crest_step(ARG(0));
    cap_begin(ARG(0));
    hook_call_original();
    if (cap.on && cap.rows) {
        int id = pic_register(cap.name, cap.w, cap.rows, cap.px, 1);
        Spr *s = spr_slot(AX);
        s->n = 1; s->r[0] = (Reg){ id, 0, 0, (s16)cap.w, (s16)cap.rows, 0, 0, 1 };
    } else { Spr *s = spr_find(AX); if (s) s->n = 0; }
    cap.on = 0;
}
static void h_sprite_begin(void) { if (cap.on) { cap.w = (s16)ARG(0); cap.h = (s16)ARG(1); } hook_call_original(); }   /* 05bc (w, h) */
static void h_sprite_row(void) { if (cap.on) cap_row(ARG(0), cap.w ? cap.w : 1); hook_call_original(); }            /* 05b5 (row) */

static void h_save_rect(void)               /* save_rect(page, x, y, w, h) -> sprite handle */
{
    int page = (s16)ARG(0), x = (s16)ARG(1), y = (s16)ARG(2), w = (s16)ARG(3), h = (s16)ARG(4);
    hook_call_original();
    Spr *s = spr_slot(AX);
    s->n = reg_take(page, x, y, w, h, -x, -y, s->r, SPRREG);
}

static void h_draw_pic(void);
static int tracked(int pic) { return pic >= 0 && (pics[pic].art || hdart_show_names || pics[pic].baked); }

static void h_sprite_file(void)             /* sprite_file_load(i, name, frames, slot, mem) */
{
    char name[16]; g_str(G_DGROUP, ARG(1), name, sizeof name);
    u16 frames = ARG(2); int slot = (s16)ARG(3);
    hook_call_original();
    int k = (s8)g_rb((u16)(0xc908 + slot));
    char key[12]; int n = 0;
    while (name[n] && n < 9) { key[n] = (char)toupper((unsigned char)name[n]); n++; }
    key[n] = 0;
    for (int i = 0; i < k * 8 && i < 1024; i++) frame_set(g_rw((u16)(frames + 2 * i)), key, i);
    if (logon || getenv("DL_SPRITE_WATCH")) fprintf(stderr, "art: sprite set %s: %d frames (slot %d)\n", key, k * 8, slot);
}

/* the picture of a battle frame (decoded at seg:0; recoloured sets differ: one picture each) */
static int frame_pic(Frame *f)
{
    u16 seg = f->seg;
    int w = rb(seg, 0), h = rb(seg, 1);
    if (!w || !h) return -1;
    u32 hsh = 2166136261u; u16 o = 2;
    static u8 px[256 * 256];
    memset(px, 0, (size_t)w * h);
    for (int y = 0; y < h; y++) {
        int cnt = rb(seg, o), skip = rb(seg, (u16)(o + 1));
        hsh = (hsh ^ (u32)(cnt << 8 | skip)) * 16777619u;
        for (int i = 0; i < cnt; i++) {
            u8 v = rb(seg, (u16)(o + 2 + i));
            hsh = (hsh ^ v) * 16777619u;
            if (skip + i < w) px[y * w + skip + i] = v;
        }
        o = (u16)(o + 2 + cnt);
    }
    if (f->pic >= 0 && f->hash == hsh) return f->pic;
    char name[24]; snprintf(name, sizeof name, "%s_%d", f->name, f->n);
    f->pic = pic_register(name, w, h, px, 1);
    f->hash = hsh;
    return f->pic;
}

static void place_sprite(u16 win, int x, int y, Spr *s)
{
    int page = win_page(win);
    if (!s || !s->n || page < 0 || page >= NPAGES) return;
    int ox, oy, x1, y1; win_box(win, &ox, &oy, &x1, &y1);
    for (int i = 0; i < s->n; i++) {
        Reg r = s->r[i];
        if (!tracked(r.pic)) continue;
        int rx = r.x + ox + x, ry = r.y + oy + y, rw = r.w, rh = r.h;
        if (!clip(&rx, &ry, &rw, &rh, ox, oy, x1, y1)) continue;
        r.x = (s16)rx; r.y = (s16)ry; r.w = (s16)rw; r.h = (s16)rh; r.ox = (s16)(r.ox + ox + x); r.oy = (s16)(r.oy + oy + y);
        r.opaque = 0;
        reg_add(page, r, 0);
    }
}

static void sprite_watch(u16 key)            /* debug (DL_SPRITE_WATCH): a sprite's data changing after first use */
{
    static int on = -1; if (on < 0) on = getenv("DL_SPRITE_WATCH") != 0;
    if (!on) return;
    static u16 keys[4096]; static u32 hs[4096]; static int n;
    u32 h = 2166136261u; for (int i = 0; i < 256; i++) h = (h ^ rb(key, (u16)i)) * 16777619u;
    for (int i = 0; i < n; i++) if (keys[i] == key) {
        if (hs[i] != h) { u32 plat_ticks_ms(void); fprintf(stderr, "sprite %04x changed at %u ms (w %d h %d)\n", key, plat_ticks_ms(), rb(key, 0), rb(key, 1)); hs[i] = h; }
        return;
    }
    if (n < 4096) { keys[n] = key; hs[n] = h; n++; }
    { u32 plat_ticks_ms(void); fprintf(stderr, "sprite %04x first at %u ms: w %d h %d hash %08x\n", key, plat_ticks_ms(), rb(key, 0), rb(key, 1), h); }
}
static void h_draw_sprite(void)             /* gfx_draw_sprite(win, x, y, handle): battle frames, map tiles */
{
    u16 win = ARG(0); int x = (s16)ARG(1), y = (s16)ARG(2); u16 key = ARG(3);
    sprite_watch(key);
    hook_call_original();
    if (!narts && !hdart_show_names) return;
    Frame *f = game_mode() == GM_BATTLE ? frame_find(key) : 0;
    if (f) {
        int pic = frame_pic(f);
        if (!tracked(pic)) return;
        Spr one; one.key = key; one.n = 1; one.r[0] = (Reg){ pic, 0, 0, (s16)pics[pic].w, (s16)pics[pic].h, 0, 0, 0 };
        place_sprite(win, x, y, &one);
        return;
    }
    place_sprite(win, x, y, spr_find(key));
}

static void h_draw_pic(void)                /* gfx_draw_pic(win, x, y, handle) */
{
    u16 win = ARG(0); int x = (s16)ARG(1), y = (s16)ARG(2); u16 key = ARG(3);
    hook_call_original();
    place_sprite(win, x, y, spr_find(key));
}

static void h_copy(void)                    /* gfx_copy_rect(src, sx, sy, w, h, dst, dx, dy) */
{
    u16 sw = ARG(0), dw = ARG(5);
    int sx = (s16)ARG(1), sy = (s16)ARG(2), w = (s16)ARG(3), h = (s16)ARG(4), dx = (s16)ARG(6), dy = (s16)ARG(7);
    hook_call_original();
    int sp = win_page(sw), dp = win_page(dw);
    if (dp < 0 || dp >= NPAGES) return;
    int sox, soy, sx1, sy1, dox, doy, dx1, dy1;
    win_box(sw, &sox, &soy, &sx1, &sy1); win_box(dw, &dox, &doy, &dx1, &dy1);
    int ax = sox + sx, ay = soy + sy, bx = dox + dx, by = doy + dy;
    int cx = bx, cy = by, cw = w, ch = h;           /* the part that lands inside the destination */
    if (!clip(&cx, &cy, &cw, &ch, dox, doy, dx1, dy1) || !clip(&cx, &cy, &cw, &ch, 0, 0, 320, 200)) return;
    hdtext_cover(dp, cx, cy, cw, ch);
    hdtext_copy(sp, ax + (cx - bx), ay + (cy - by), cw, ch, dp, cx, cy);
    static Reg tmp[MAXREG];
    int n = reg_take(sp, ax + (cx - bx), ay + (cy - by), cw, ch, bx - ax, by - ay, tmp, MAXREG);
    drop_covered(dp, cx, cy, cw, ch);
    for (int i = 0; i < n; i++) reg_add(dp, tmp[i], 0);
}

/* ---------------------------------------------------------------- the frame */
HdDraw hdart_draws[HDART_MAXDRAWS];
int hdart_ndraws;

/* ---------------------------------------------------------------- words painted into pictures
 * The character sheet's buttons (Equipment, Formulae, Saints) are part of its picture, in the
 * old letters. With crisp text on, their letters are covered with the button's own grain (the
 * picture's row above or below the word) and the word joins the crisp text. */
typedef struct { const char *pic; int x0, y0, x1, y1; const char *text; } Baked;
static const Baked BAKED[] = {
    { "ARMBACK", 199, 152, 254, 159, "Equipment" }, { "ARMBACK", 199, 168, 254, 175, "Formulae" }, { "ARMBACK", 199, 184, 254, 191, "Saints" },
};
#define NBAKED ((int)(sizeof BAKED / sizeof BAKED[0]))
int has_baked(const char *name) { for (int k = 0; k < NBAKED; k++) if (!strcmp(BAKED[k].pic, name)) return 1; return 0; }
static int light(u8 v) { return v >= 0xb0 && v <= 0xdf; }

void hdart_labels(u32 *rgb, const u8 *idx, const u32 *lut)
{
    if (!hdtext_mode) return;
    for (int i = 0; i < nreg[0]; i++) {
        Reg *r = &reg[0][i]; Pic *P = &pics[r->pic];
        if (!P->baked) continue;
        for (int k = 0; k < NBAKED; k++) {
            const Baked *B = &BAKED[k];
            if (strcmp(B->pic, P->name) || B->y0 < 1 || B->y1 + 1 >= P->h || B->x1 >= P->w) continue;
            int sx0 = B->x0 + r->ox, sy0 = B->y0 + r->oy, sx1 = B->x1 + r->ox, sy1 = B->y1 + r->oy;
            if (sx0 < r->x || sy0 < r->y || sx1 >= r->x + r->w || sy1 >= r->y + r->h || sx1 >= 320 || sy1 >= 200 || sx0 < 0 || sy0 < 0) continue;
            long n = 0, same = 0;
            for (int y = sy0; y <= sy1; y++) for (int x = sx0; x <= sx1; x++) { n++; same += idx[y * 320 + x] == P->px[(y - r->oy) * P->w + (x - r->ox)]; }
            if (same * 10 < n * 9) continue;                      /* something covers the button */
            int bx0 = 999, bx1 = -1, by0 = 999, by1 = -1; u32 cnt[256]; memset(cnt, 0, sizeof cnt);
            u32 br = 0, bg = 0, bb = 0, bn = 0;
            for (int y = B->y0; y <= B->y1; y++)
                for (int x = B->x0; x <= B->x1; x++) {
                    u8 v = P->px[y * P->w + x];
                    int sp = (y + r->oy) * 320 + x + r->ox;
                    u8 g = light(v) ? P->px[(y <= (B->y0 + B->y1) / 2 ? B->y0 - 1 : B->y1 + 1) * P->w + x] : v;
                    if (light(g)) g = 0x88;
                    if (light(v)) { cnt[v]++; if (x < bx0) bx0 = x; if (x > bx1) bx1 = x; if (y < by0) by0 = y; if (y > by1) by1 = y; if (idx[sp] == v) rgb[sp] = lut[g]; }
                    u32 c = lut[g]; br += c >> 16 & 255; bg += c >> 8 & 255; bb += c & 255; bn++;
                }
            if (bx1 < 0 || hdtext_nruns >= HD_MAXRUNS) continue;
            int best = 0; for (int v = 1; v < 256; v++) if (cnt[v] > cnt[best]) best = v;
            HdRun *u = &hdtext_runs[hdtext_nruns++];
            snprintf(u->text, sizeof u->text, "%s", B->text);
            u->x = bx0 + r->ox; u->y = by0 + r->oy - 1; u->w = bx1 - bx0 + 1; u->h = by1 - by0 + 3; u->font = 1;
            u->rgb = lut[best]; u->bgrgb = bn ? (br / bn) << 16 | (bg / bn) << 8 | (bb / bn) : 0;
        }
    }
}

void hdart_prepare(u32 *rgb, const u8 *idx, const u32 *lut)
{
    hdart_ndraws = 0;
    if (!hdart_on || !narts || !nreg[0]) return;
    int any = 0;
    for (int i = 0; i < nreg[0]; i++) if (pics[reg[0][i].pic].art) { any = 1; break; }
    if (!any) return;
    static s16 owner[320 * 200];
    static u8 alive[MAXREG];
    int n = nreg[0];
    /* which regions are still on the screen: most of their own pixels are there */
    for (int i = 0; i < n; i++) {
        Reg *r = &reg[0][i]; Pic *P = &pics[r->pic];
        long match = 0, solid = 0;
        for (int y = r->y; y < r->y + r->h; y++) {
            const u8 *pp = P->px + (long)(y - r->oy) * P->w + (r->x - r->ox);
            const u8 *sp = idx + y * 320 + r->x;
            for (int x = 0; x < r->w; x++) {
                if (!pp[x] && !r->opaque) continue;
                solid++; match += pp[x] == sp[x];
            }
        }
        alive[i] = solid && match * 10 >= solid * 3;
    }
    for (int i = 0; i < 320 * 200; i++) owner[i] = -1;
    for (int i = 0; i < n; i++) {
        if (!alive[i]) continue;
        Reg *r = &reg[0][i]; Pic *P = &pics[r->pic];
        for (int y = r->y; y < r->y + r->h; y++) {
            const u8 *pp = P->px + (long)(y - r->oy) * P->w + (r->x - r->ox);
            int o = y * 320 + r->x;
            for (int x = 0; x < r->w; x++)
                if (pp[x] == idx[o + x] && (pp[x] || r->opaque)) owner[o + x] = (s16)i;
        }
    }
    for (int p = 0; p < 320 * 200; p++) if (owner[p] >= 0 && pics[reg[0][owner[p]].pic].art) rgb[p] &= 0x00ffffffu;
    /* draw list, oldest first, with the colour change of fades and night */
    for (int i = 0; i < n && hdart_ndraws < HDART_MAXDRAWS; i++) {
        if (!alive[i]) continue;
        Reg *r = &reg[0][i]; Pic *P = &pics[r->pic];
        if (!P->art) continue;
        double b = brightness(P, lut, -1);
        if (b > P->refb) { P->refb = b; memcpy(P->ref, lut, sizeof P->ref); }
        HdDraw *d = &hdart_draws[hdart_ndraws++];
        d->pic = r->pic;
        d->sx = r->x - r->ox; d->sy = r->y - r->oy; d->x = r->x; d->y = r->y; d->w = r->w; d->h = r->h;
        for (int c = 0; c < 3; c++) {
            double cur = brightness(P, lut, c), ref = brightness(P, P->ref, c);
            double f = ref > 0 ? cur / ref : 1.0;
            if (f > 1) f = 1;
            if (f < 0) f = 0;
            d->mod[c] = (u8)(f * 255 + 0.5);
        }
    }
}

/* the picture at screen pixel (x, y), for "show picture names": its whole placement */
int hdart_pick(int x, int y, int *px, int *py, int *pw, int *ph)
{
    if (x < 0 || y < 0 || x >= 320 || y >= 200) return -1;
    u8 v = M[0xa0000 + y * 320 + x];
    for (int i = nreg[0] - 1; i >= 0; i--) {
        Reg *r = &reg[0][i]; Pic *P = &pics[r->pic];
        if (x < r->x || y < r->y || x >= r->x + r->w || y >= r->y + r->h) continue;
        u8 c = P->px[(y - r->oy) * P->w + (x - r->ox)];
        if (c != v || (!c && !r->opaque)) continue;
        *px = r->ox; *py = r->oy; *pw = P->w; *ph = P->h;
        return r->pic;
    }
    return -1;
}

int hdart_has_art(int pic) { return pic >= 0 && pic < npics && pics[pic].art; }

/* ---------------------------------------------------------------- export */
int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
static int write_png(const char *path, int w, int h, const u8 *px, const u32 *pal, int transparent0)
{
    u8 *rgba = malloc((size_t)w * h * 4);
    if (!rgba) return 0;
    for (long i = 0; i < (long)w * h; i++) {
        u32 c = pal[px[i]];
        rgba[i * 4] = (u8)(c >> 16); rgba[i * 4 + 1] = (u8)(c >> 8); rgba[i * 4 + 2] = (u8)c;
        rgba[i * 4 + 3] = (transparent0 && !px[i]) ? 0 : 255;
    }
    int ok = stbi_write_png(path, w, h, 4, rgba, w * 4);
    free(rgba);
    return ok;
}

static int find_pic(const char *key) { for (int i = 0; i < npics; i++) if (!strcmp(pics[i].name, key)) return i; return -1; }

/* every picture of the game as PNG in art/original (for artists: same names, same colours,
 * colour 0 transparent where the game draws the picture as a sprite) */
int hdart_export(char *msg, int msgn)
{
    char dir[1200], p[1400];
    snprintf(dir, sizeof dir, "%s/art", game_dir); MKDIR(dir);
    snprintf(dir, sizeof dir, "%s/art/original", game_dir); MKDIR(dir);
    u32 base[256]; dac_lut(base);
    int done = 0, failed = 0;
    char pd[1200]; dos_host_path("PICS", pd, sizeof pd);
    DIR *d = opendir(pd);
    if (d) {
        struct dirent *e;
        while ((e = readdir(d))) {
            const char *dot = strrchr(e->d_name, '.');
            if (!dot || strcasecmp(dot, ".pic")) continue;
            snprintf(p, sizeof p, "%s/%s", pd, e->d_name);
            u8 *data; long n;
            if (!read_file(p, &data, &n)) { failed++; continue; }
            u32 pal[256]; u8 mask[256]; memcpy(pal, base, sizeof pal);
            int w = 0, h = 0;
            u8 *img = pic_decode(data, n, &w, &h, pal, mask);
            free(data);
            if (!img) continue;                     /* a palette only (CHARPALT) */
            char key[16]; base_name(e->d_name, key);
            int seen = find_pic(key), anypal = 0;
            for (int i = 0; i < 256; i++) anypal |= mask[i];
            if (seen >= 0) for (int i = 0; i < 256; i++) if (!mask[i]) pal[i] = pics[seen].ref[i];
            int sprite = seen >= 0 ? pics[seen].sprite : !anypal;
            snprintf(p, sizeof p, "%s/%s.png", dir, key);
            if (write_png(p, w, h, img, pal, sprite)) done++; else failed++;
            free(img);
        }
        closedir(d);
    }
    /* pictures seen in play that are not in PICS (enemy pictures from EINFO.CAT, ...) */
    for (int i = 0; i < npics; i++) {
        snprintf(p, sizeof p, "%s/%s.png", dir, pics[i].name);
        FILE *f = fopen(p, "rb");
        if (f) { fclose(f); continue; }
        if (write_png(p, pics[i].w, pics[i].h, pics[i].px, pics[i].ref, pics[i].sprite)) done++; else failed++;
    }
    snprintf(msg, msgn, "%d pictures written to %s%s", done, dir, failed ? " (some could not be written)" : "");
    if (logon) fprintf(stderr, "art: export: %s\n", msg);
    return done;
}

int hdart_seen(void) { return npics; }

/* ---------------------------------------------------------------- switching */
void hdart_set(int on) { hdart_on = on; config_set_int("display.hd_art", on); }

/* DL_THUNK_LOG: count the calls of the graphics driver thunks (which ones a screen uses) */
static const unsigned THUNKS[] = { 0x04c7, 0x04ce, 0x04dc, 0x04e3, 0x04ea, 0x04f8, 0x0504, 0x0514, 0x051b, 0x052e,
    0x053e, 0x0545, 0x055f, 0x0568, 0x056f, 0x0576, 0x0584, 0x058b, 0x0597, 0x059e, 0x05ae, 0x05ca, 0x05d1, 0x05d8,
    0x05df, 0x05e6, 0x05ed, 0x0744, 0x0767, 0x0783, 0x0791 };   /* not 0617/061e/073d (keyboard): polling */
static int thunk_idx[64]; static long thunk_n[64]; static int nthunk;
static void thunk_count(void)
{
    extern unsigned plat_ticks_ms(void);
    static unsigned last;
    for (int i = 0; i < nthunk; i++) if (thunk_idx[i] == hook_current) {
        thunk_n[i]++;
        if (THUNKS[i] == 0x04ef && thunk_n[i] % 500 == 1) fprintf(stderr, "thunk 04ef args %04x %d %d %04x %04x\n", ARG(0), (s16)ARG(1), (s16)ARG(2), ARG(3), ARG(4));
        break;
    }
    unsigned now = plat_ticks_ms();
    if (now - last > 10000) {
        last = now;
        fprintf(stderr, "thunks:");
        for (int i = 0; i < nthunk; i++) if (thunk_n[i]) fprintf(stderr, " %04x:%ld", THUNKS[i], thunk_n[i]);
        fprintf(stderr, "\n");
    }
    hook_call_original();
}
static void thunk_probe(void)
{
    extern const HookPoint HOOK_POINTS[]; extern const int HOOK_COUNT;
    for (unsigned i = 0; i < sizeof THUNKS / sizeof THUNKS[0]; i++) {
        unsigned lin = 0x47c0 + THUNKS[i];
        for (int k = 0; k < HOOK_COUNT; k++) if (HOOK_POINTS[k].lin == lin && HOOK_POINTS[k].sect == -1) {
            thunk_idx[nthunk] = HOOK_POINTS[k].idx;
            hook_install(-1, 0x047c, THUNKS[i], thunk_count);
            nthunk++;
            break;
        }
    }
}

void hdart_init(void)
{
    logon = getenv("DL_ART_LOG") != 0;
    hdart_on = config_get_int("display.hd_art", 1);
    hdart_show_names = config_get_int("display.art_names", 0);
    hdart_rescan();
    hook_install(-1, 0x0617, 0x01b2, h_pic_draw);
    hook_install(-1, 0x0617, 0x04e2, h_pic_open);
    hook_install(-1, 0x0617, 0x0546, h_pic_mem);
    hook_install(-1, 0x0617, 0x045c, h_pic_load);
    hook_install(-1, 0x047c, 0x054c, h_row);
    hook_install(-1, 0x047c, 0x05bc, h_sprite_begin);
    hook_install(-1, 0x047c, 0x05b5, h_sprite_row);
    hook_install(-1, 0x047c, 0x04d5, h_save_rect);
    hook_install(-1, 0x047c, 0x05a5, h_draw_pic);
    hook_install(-1, 0x047c, 0x0553, h_copy);
    hook_install(-1, 0x047c, 0x04ef, h_draw_sprite);
    hook_install(14, 0x0e76, 0x3d2c, h_sprite_file);
    if (getenv("DL_THUNK_LOG")) thunk_probe();
    if (getenv("DL_ART_MAKE")) { char d[64]; snprintf(d, sizeof d, "PICS\\%s.PIC", getenv("DL_ART_MAKE")); const char *r = hdart_virtual_file(d); fprintf(stderr, "art make: %s\n", r ? r : "(none)"); }
}
