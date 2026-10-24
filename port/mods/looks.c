/* looks.c - more figures for the party.
 *
 * The original offers four figures (Select Character Image in the creation menus: F01, F60,
 * C00, A00 - found by the files pics\F??stat.PIC, A??, C??) and three colour keys for each.
 * The human enemies have figures of their own with every animation a party member needs
 * (walking, fighting, dying: E00C.CAT), so they can be the party's too: a sergeant, a monk,
 * a knight, the three "ex-PC" figures (grey-haired, brown-haired, blond) ...
 *
 * What the game needs for a figure "XXX" and how each is met here:
 *   - battle sprites XXX{WK,CB}<weapon>.IMC, XXXDY.IMC from the catalogue of its first letter
 *     (s014 0e76:3d2c): E00C.CAT has them, but only for the weapons that enemy carries.
 *     cbt_set_anim (s014 0e76:38b6) builds the name from the weapon's suffix (anim_file[kind]);
 *     for a weapon the figure has no animation for, the hook puts the nearest one it has in
 *     for the call (a monk with a sword swings his staff) and puts the real suffix back
 *     (the suffix also means "throwing a potion" when it is PT, s019 0b9e).
 *   - its colours: enemy sprites take 16 colours of the battle palette (ENEMYPAL.DAT ranges at
 *     32/48/64, shared with the enemies in the battle); a party member has 8 of its own
 *     ((slot + 10) * 8, from its 24-byte palette at far 20a5:33fc, what the colour keys
 *     change). After the sprite file is loaded (hdart's hook on 3d2c calls looks_sprite_loaded)
 *     the figure's enemy colours are put onto the member's 8: the main ones (clothes, hair) as
 *     up to 8 shades in up to three parts, skin, shadow and small details onto the battle's
 *     common colours (0x80..0x9f, the same in the creation screens).
 *   - TACANIM.DB sizes: the enemies' records are there already.
 *   - pics\XXXsmall.pic (the portrait of the creation menus: front and back) and
 *     pics\XXXshort.pic (the little figure beside the battle): made from the sprites on first
 *     use (looks_virtual_file, when the game's own file does not exist), with 0xeb..0xf2 for
 *     the member's colours as the game's own pictures have.
 *   - the colour keys 1/2/3 (s037 0444 / s038 02f8 have tables for the four figures only): the
 *     hook gives the figure's parts a cycle of dyes (red, blue, green, gold ...).
 *   - Select Character Image: the hook makes the game's own handler take the next figure after
 *     the member's current one, in the game's list followed by these (the original cycles one
 *     global position through its list of ten at most, DS:e7e2).
 *
 * The figure code is kept where the game keeps it (charactr.tmp, the save's party figures), so
 * saves stay ordinary; a party with these figures needs Darklands+ to show them (the original
 * would stop at the missing pictures). Offering them is switchable (party.more_figures; off in
 * "Play the original game"); a party that has them keeps working either way. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
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
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "hdart.h"
#include "looks.h"

extern char dlp_user_dir[];
#define USER_DIR (dlp_user_dir[0] ? dlp_user_dir : game_dir)
void dos_host_path_read(const char *dos, char *out, size_t outn);

/* DGROUP */
#define D_FIGLIST   0xe7e2      /* char [10][4]: the figures Select Character Image cycles through */
#define D_FIGCOUNT  0xe7de
#define D_FIGCUR    0xe7e0
#define D_SELCHAR   0xe7da      /* the character under the cursor (creation menus) */
#define D_CHARIDS   0x8d06      /* s038: character index per list row */
#define D_FIGURE    GV_member_shield      /* char [5][4]: each party slot's figure */
#define D_PORTRAIT  0xe753      /* the portrait's colours = palette entries 0xeb..0xf2 (8 RGB) */
#define D_GROUPS    0x1166      /* near char *[7]: sprite group per combatant kind */
#define D_WEAPONS   GV_anim_file      /* near char *[7]: weapon suffix per kind */
#define D_CBT       GV_combatants      /* combatants [13] x 0x80; +0x16 kind */
#define D_FRAMES    0xc908      /* s8 [slot]: frames per direction of the loaded sprite file */
#define F_PALETTE   0x33fc      /* far 20a5: member palettes [5][24] */
#define G_FAR20A5   ((u16)(LOADSEG + 0x20a5))
#define ARG(k) rw(SS, (u16)(SP + 4 + 2 * (k)))

static int logon;
#define LOG(...) do { if (logon) fprintf(stderr, __VA_ARGS__); } while (0)

/* ------------------------------------------------------------------ the figures */
static const struct { const char *code, *title; } KNOWN[] = {
    { "E05", "Gray-haired fighter" }, { "E07", "Brown-haired woman" }, { "E12", "Blond fighter" },
    { "E00", "Sergeant" }, { "E02", "Town guard" }, { "E04", "Mercenary" }, { "E03", "Bandit" },
    { "E10", "Knight" }, { "E11", "Knight in plate" }, { "E13", "Templar" }, { "E09", "Monk" },
    { "E06", "Alchemist" }, { "E08", "Witch" }, { "E17", "Cultist" },
};
#define NKNOWN ((int)(sizeof KNOWN / sizeof KNOWN[0]))
#define MAXLOOK 32

typedef struct {
    char code[4]; char title[40];
    int state;                  /* 0 not looked at, 1 usable, -1 not */
    int S;                      /* first battle palette index of its enemy colours */
    u8 rgb[16][3];              /* those colours (the first enemy type's first ENEMYPAL record) */
    long cnt[16];               /* pixels of each in all its sprites */
    s8 slot[16];                /* -> member colour 0..7, or -1: */
    u8 fix[16];                 /*    a common colour instead */
    int nslot; u8 def[8][3];    /* the member colours as the enemy has them */
    int npart; s8 part[3][8]; int partn[3]; int neutral[3];
    char weap[24][3]; int nweap;
    char pic_file[16];          /* the walking sprites the pictures are made from */
    u8 made[2];                 /* its portrait / battle figure written this run */
} Look;
static Look looks[MAXLOOK];
static int nlooks = -1;

/* ------------------------------------------------------------------ game files */
static u8 *slurp(const char *dos, long *n)
{
    char p[1300]; dos_host_path_read(dos, p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    u8 *b = malloc((size_t)(len > 0 ? len : 1));
    if (b && fread(b, 1, (size_t)len, f) != (size_t)len) { free(b); b = 0; }
    fclose(f);
    if (b) *n = len;
    return b;
}

typedef struct { char name[13]; const u8 *data; long len; } CatEnt;
typedef struct { const char *file; u8 *buf; long len; CatEnt *e; int n, tried; } Cat;
static Cat cats[5] = { { "E00C.CAT" }, { "A00C.CAT" }, { "C00C.CAT" }, { "F01C.CAT" }, { "F60C.CAT" } };
static Cat *cat_open(int k)
{
    Cat *c = &cats[k];
    if (c->tried) return c->n ? c : 0;
    c->tried = 1;
    c->buf = slurp(c->file, &c->len);
    if (!c->buf || c->len < 2) return 0;
    int n = c->buf[0] | c->buf[1] << 8;
    c->e = calloc((size_t)n + 1, sizeof *c->e);
    for (int i = 0; i < n && 2 + (i + 1) * 24 <= c->len; i++) {
        const u8 *x = c->buf + 2 + i * 24;
        long len = x[16] | x[17] << 8 | (long)x[18] << 16 | (long)x[19] << 24, off = x[20] | x[21] << 8 | (long)x[22] << 16 | (long)x[23] << 24;
        if (off < 0 || len < 0 || off + len > c->len) continue;
        CatEnt *e = &c->e[c->n++];
        memcpy(e->name, x, 12); e->name[12] = 0;
        for (char *q = e->name; *q; q++) *q = (char)toupper((unsigned char)*q);
        e->data = c->buf + off; e->len = len;
    }
    return c->n ? c : 0;
}
#define cat (cats[0].e)
#define ncat (cats[0].n)
static int cat_load(void) { return cat_open(0) != 0; }      /* the enemies' catalogue (listed) */
/* a sprite file from the catalogue the game takes it from (s014 sprite_file_load: by its first letter) */
static const CatEnt *cat_find(const char *name)
{
    int k = -1;
    switch (toupper((unsigned char)name[0])) {
    case 'E': k = 0; break;
    case 'A': k = 1; break;
    case 'C': k = 2; break;
    case 'F': k = name[1] == '6' ? 4 : 3; break;
    }
    Cat *c = k >= 0 ? cat_open(k) : 0;
    if (!c) return 0;
    for (int i = 0; i < c->n; i++) if (!strcasecmp(c->e[i].name, name)) return &c->e[i];
    return 0;
}

/* MicroProse LZ (formats/dlformats/lzss.py); the length of the output, -1 broken */
static long lz_unpack(const u8 *d, long n, u8 *out, long cap)
{
    if (n < 2) return -1;
    long pos = 2, o = 0; unsigned ctrl = d[0] | d[1] << 8; int left = 16;
    #define BIT(v) do { v = ctrl & 1; ctrl >>= 1; if (--left == 0) { if (pos + 2 > n) return -1; ctrl = d[pos] | d[pos + 1] << 8; pos += 2; left = 16; } } while (0)
    for (;;) {
        int b; BIT(b);
        if (b) { if (pos >= n || o >= cap) return -1; out[o++] = d[pos++]; continue; }
        long len, dist; BIT(b);
        if (b) {
            if (pos + 2 > n) return -1;
            unsigned w = d[pos] | d[pos + 1] << 8; pos += 2;
            len = (w >> 8) & 7; dist = 0x2000 - (long)(((w >> 11) << 8) | (w & 0xff));
            if (!len) {
                if (pos >= n) return -1;
                int c = d[pos++];
                if (c == 0) break;
                if (c == 1) continue;
                len = c + 1;
            } else len += 2;
        } else {
            int a, c; BIT(a); BIT(c);
            len = (a << 1 | c) + 2;
            if (pos >= n) return -1;
            dist = 256 - d[pos++];
        }
        if (dist > o || o + len > cap) return -1;
        for (long i = 0; i < len; i++, o++) out[o] = out[o - dist];
    }
    #undef BIT
    return o;
}

/* an IMC sprite file: frames by direction (8) x frame_count */
typedef struct { u8 *raw; long n; int fc; const u8 *offs; const u8 *data; long dlen; } Imc;
static int imc_open(const char *name, Imc *m)
{
    memset(m, 0, sizeof *m);
    const CatEnt *e = cat_find(name);
    if (!e) return 0;
    m->raw = malloc(65536);
    m->n = lz_unpack(e->data, e->len, m->raw, 65536);
    if (m->n < 0x30) { free(m->raw); m->raw = 0; return 0; }
    int flag = m->raw[0x18] | m->raw[0x19] << 8;
    int base = flag == 0xffff ? 0x1a : 0x2e;
    m->fc = m->raw[base + 34] | m->raw[base + 35] << 8;
    long ds = base + 38 + 16L * m->fc;
    if (m->fc <= 0 || m->fc > 64 || ds > m->n) { free(m->raw); m->raw = 0; return 0; }
    m->offs = m->raw + base + 38; m->data = m->raw + ds; m->dlen = m->n - ds;
    return 1;
}
static void imc_close(Imc *m) { free(m->raw); m->raw = 0; }
/* frame k as pixels (w x h, 0 = transparent); 0 if broken */
static int imc_frame(const Imc *m, int k, u8 *px, int *w, int *h)
{
    if (k < 0 || k >= 8 * m->fc) return 0;
    long p = 16L * (m->offs[2 * k] | m->offs[2 * k + 1] << 8);
    if (p + 2 > m->dlen) return 0;
    *w = m->data[p]; *h = m->data[p + 1]; p += 2;
    if (!*w || !*h) return 0;
    memset(px, 0, (size_t)*w * *h);
    for (int y = 0; y < *h; y++) {
        if (p + 2 > m->dlen) return 0;
        int cnt = m->data[p], skip = m->data[p + 1];
        if (p + 2 + cnt > m->dlen || skip + cnt > *w) return 0;
        memcpy(px + y * *w + skip, m->data + p + 2, (size_t)cnt);
        p += 2 + cnt;
    }
    return 1;
}
/* every pixel of every frame (each stored frame once) */
static void imc_count(const Imc *m, long cnt[256])
{
    long seen[512]; int ns = 0;
    for (int k = 0; k < 8 * m->fc; k++) {
        long p = 16L * (m->offs[2 * k] | m->offs[2 * k + 1] << 8);
        int dup = 0; for (int i = 0; i < ns; i++) if (seen[i] == p) dup = 1;
        if (dup) continue;
        if (ns < 512) seen[ns++] = p;
        if (p + 2 > m->dlen) continue;
        int h = m->data[p + 1]; p += 2;
        for (int y = 0; y < h && p + 2 <= m->dlen; y++) {
            int c = m->data[p];
            for (int i = 0; i < c && p + 2 + i < m->dlen; i++) cnt[m->data[p + 2 + i]]++;
            p += 2 + c;
        }
    }
}

/* ------------------------------------------------------------------ colours */
static u8 common[256][3]; static u8 is_target[256]; static int common_ok;
static void common_load(void)
{
    if (common_ok) return;
    common_ok = 1;
    long n; u8 *c = slurp("COMNCLRS.DAT", &n);
    static const u8 ega[4][4] = { { 5, 0, 0, 0 }, { 7, 42, 42, 42 }, { 8, 21, 21, 21 }, { 15, 63, 63, 63 } };
    for (int i = 0; i < 4; i++) { u8 k = ega[i][0]; memcpy(common[k], ega[i] + 1, 3); is_target[k] = 1; }
    if (c && n >= 216) {
        for (int k = 0; k < 16; k++) memcpy(common[0x10 + k], c + 3 * k, 3);
        for (int k = 0; k < 44; k++) memcpy(common[0x78 + k], c + 3 * (16 + k), 3);
        for (int k = 0x80; k < 0xa0; k++) is_target[k] = 1;    /* the same in battles and in the creation screens */
    }
    free(c);
}
static long cdist(const u8 *a, const u8 *b)
{
    long dr = a[0] - b[0], dg = a[1] - b[1], db = a[2] - b[2];
    return dr * dr * 3 + dg * dg * 4 + db * db * 2;
}
static u8 nearest_common(const u8 *rgb)
{
    int best = 5; long bd = -1;
    for (int k = 0; k < 256; k++) if (is_target[k]) { long d = cdist(common[k], rgb); if (bd < 0 || d < bd) { bd = d; best = k; } }
    return (u8)best;
}
static void hsv(const u8 *c, float *h, float *s, float *v)
{
    float r = c[0] / 63.f, g = c[1] / 63.f, b = c[2] / 63.f;
    float mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b), d = mx - mn;
    *v = mx; *s = mx > 0 ? d / mx : 0;
    if (d <= 0) *h = 0;
    else if (mx == r) *h = fmodf((g - b) / d / 6.f + 1.f, 1.f);
    else if (mx == g) *h = (b - r) / d / 6.f + 1.f / 3;
    else *h = (r - g) / d / 6.f + 2.f / 3;
}
static void rgb_of(float h, float s, float v, u8 *out)
{
    h = fmodf(h + 1.f, 1.f) * 6; int i = (int)h; float f = h - i, p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
    float r, g, b;
    switch (i % 6) { case 0: r = v; g = t; b = p; break; case 1: r = q; g = v; b = p; break; case 2: r = p; g = v; b = t; break;
                     case 3: r = p; g = q; b = v; break; case 4: r = t; g = p; b = v; break; default: r = v; g = p; b = q; }
    float c[3] = { r, g, b };
    for (int k = 0; k < 3; k++) { int x = (int)(c[k] * 63 + 0.5f); out[k] = (u8)(x < 0 ? 0 : x > 63 ? 63 : x); }
}
static int neutral_c(const u8 *c) { float h, s, v; hsv(c, &h, &s, &v); return s < 0.2f || v < 0.15f; }
static float fam_dist(const u8 *a, const u8 *b)
{
    float ha, sa, va, hb, sb, vb; hsv(a, &ha, &sa, &va); hsv(b, &hb, &sb, &vb);
    int na = sa < 0.2f || va < 0.15f, nb = sb < 0.2f || vb < 0.15f;
    if (na && nb) return fabsf(va - vb) * 0.6f;
    if (na != nb) return 1.f + fabsf(va - vb);
    float dh = fabsf(ha - hb); if (dh > 0.5f) dh = 1 - dh;
    return dh * 2.5f + fabsf(va - vb) * 0.6f + fabsf(sa - sb) * 0.3f;
}

/* the dyes of the colour keys */
typedef struct { const char *name; float h, s; int mode; } Dye;     /* mode 0 the figure's own, 1 hue, 2 dark, 3 light */
static const Dye DYE_C[] = { { "default", 0, 0, 0 }, { "red", 0.0f, 0.85f, 1 }, { "blue", 0.61f, 0.75f, 1 }, { "green", 0.33f, 0.6f, 1 },
    { "gold", 0.12f, 0.85f, 1 }, { "purple", 0.79f, 0.6f, 1 }, { "brown", 0.08f, 0.6f, 1 }, { "teal", 0.5f, 0.6f, 1 },
    { "black", 0, 0, 2 }, { "white", 0, 0, 3 } };
static const Dye DYE_N[] = { { "default", 0, 0, 0 }, { "dark", 0, 0, 2 }, { "light", 0, 0, 3 }, { "brown", 0.08f, 0.45f, 1 },
    { "blue", 0.6f, 0.35f, 1 }, { "green", 0.3f, 0.3f, 1 }, { "red", 0.0f, 0.45f, 1 }, { "gold", 0.12f, 0.5f, 1 } };
#define NDYE_C ((int)(sizeof DYE_C / sizeof DYE_C[0]))
#define NDYE_N ((int)(sizeof DYE_N / sizeof DYE_N[0]))
static void dye(const u8 *in, const Dye *d, int neutral, u8 *out)
{
    float h, s, v; hsv(in, &h, &s, &v);
    switch (d->mode) {
    case 0: memcpy(out, in, 3); return;
    case 1: s = neutral ? d->s * 0.8f : d->s * (0.55f + 0.45f * (s / 0.6f > 1 ? 1 : s / 0.6f)); h = d->h;
            v = 0.2f + 0.8f * v;                     /* (dark cloth would stay nearly black in any colour) */
            break;
    case 2: s *= 0.3f; v *= 0.5f; break;
    default: s *= 0.25f; v = 0.42f + 0.58f * v; break;
    }
    rgb_of(h, s, v, out);
}

/* ------------------------------------------------------------------ one figure */
static void analyse(Look *L)
{
    L->state = -1;
    if (!cat_load()) return;
    common_load();
    /* its colours: the first enemy type of this group, its first palette */
    long nenm, npal; u8 *enm = slurp("DARKLAND.ENM", &nenm), *pal = slurp("ENEMYPAL.DAT", &npal);
    int rec = -1;
    for (int t = 0; enm && t < 71 && (t + 1) * 204 <= nenm; t++)
        if (!strncasecmp((const char *)enm + t * 204, L->code, 3)) { rec = enm[t * 204 + 17]; break; }
    if (rec < 0 || !pal || (rec + 1) * 53 > npal) { free(enm); free(pal); LOG("figures: %s: no enemy palette\n", L->code); return; }
    L->S = pal[rec * 53] / 3;
    for (int i = 0; i < 16; i++) for (int c = 0; c < 3; c++) L->rgb[i][c] = pal[rec * 53 + 1 + 3 * i + c] & 63;
    free(enm); free(pal);
    if (L->S < 16 || L->S + 16 > 0x50) { LOG("figures: %s: palette at %d\n", L->code, L->S); return; }
    /* its sprites: the weapons, the pixels, the file for the pictures */
    static long cnt[256]; memset(cnt, 0, sizeof cnt);
    int have_dy = 0, best_w = 999;
    for (int i = 0; i < ncat; i++) {
        const char *n = cat[i].name;
        if (strncasecmp(n, L->code, 3)) continue;
        Imc m; if (!imc_open(n, &m)) continue;
        imc_count(&m, cnt);
        if (!strncasecmp(n + 3, "DY", 2)) have_dy = 1;
        if (!strncasecmp(n + 3, "WK", 2) && L->nweap < 24) {
            char w[3] = { n[5], n[6], 0 };
            int has_cb = 0; char cb[16]; snprintf(cb, sizeof cb, "%.3sCB%s.IMC", L->code, w);
            has_cb = cat_find(cb) != 0;
            if (has_cb) { memcpy(L->weap[L->nweap++], w, 3); }
            static u8 px[256 * 256]; int fw, fh;              /* the narrowest front view for the pictures */
            if (imc_frame(&m, 4 * m.fc, px, &fw, &fh)) {
                int x0 = fw, x1 = -1;
                for (int y = 0; y < fh - 4; y++) for (int x = 0; x < fw; x++) if (px[y * fw + x]) { if (x < x0) x0 = x; if (x > x1) x1 = x; }
                if (x1 >= x0 && x1 - x0 < best_w) { best_w = x1 - x0; snprintf(L->pic_file, sizeof L->pic_file, "%s", n); }
            }
        }
        imc_close(&m);
    }
    if (!L->nweap || !have_dy || !L->pic_file[0]) { LOG("figures: %s: sprites missing\n", L->code); return; }
    {   /* its buffer sizes (s025 enemy_anim_alloc reads them for party members too) */
        long nt; u8 *t = slurp("TACANIM.DB", &nt); int found = 0;
        for (long o = 0; t && o + 14 <= nt; o += 14) if (!strncasecmp((const char *)t + o, L->code, 3) && !t[o + 3]) found = 1;
        free(t);
        if (!found) { LOG("figures: %s: not in TACANIM.DB\n", L->code); return; }
    }
    long total = 0;
    for (int i = 0; i < 16; i++) { L->cnt[i] = cnt[L->S + i]; total += L->cnt[i]; }
    /* where each colour sits in the front view: hair is at the top (and is left as it is) */
    float top[16]; long fcnt[16]; for (int i = 0; i < 16; i++) { top[i] = 0; fcnt[i] = 0; }
    {
        Imc m;
        if (imc_open(L->pic_file, &m)) {
            static u8 px[256 * 256]; int fw, fh;
            if (imc_frame(&m, 4 * m.fc, px, &fw, &fh)) {
                int y0 = fh, y1 = -1;
                for (int y = 0; y < fh; y++) for (int x = 0; x < fw; x++) if (px[y * fw + x]) { if (y < y0) y0 = y; if (y > y1) y1 = y; }
                long n[16] = { 0 }, up[16] = { 0 };
                for (int y = y0; y <= y1; y++) for (int x = 0; x < fw; x++) {
                    int c = px[y * fw + x] - L->S;
                    if (c < 0 || c >= 16) continue;
                    n[c]++; if ((y - y0) * 5 < (y1 - y0 + 1)) up[c]++;      /* the top fifth: the head */
                }
                for (int i = 0; i < 16; i++) { top[i] = n[i] ? (float)up[i] / n[i] : 0; fcnt[i] = n[i]; }
            }
            imc_close(&m);
        }
    }
    /* which colours are its own (member colours) and which the common ones do */
    int cand[16], nc = 0;
    for (int i = 0; i < 16; i++) {
        L->slot[i] = -1; L->fix[i] = nearest_common(L->rgb[i]);
        if (!L->cnt[i]) continue;
        float h, s, v; hsv(L->rgb[i], &h, &s, &v);
        int mx = L->rgb[i][0] > L->rgb[i][1] ? L->rgb[i][0] : L->rgb[i][1]; if (L->rgb[i][2] > mx) mx = L->rgb[i][2];
        if (mx <= 6) continue;                                              /* shadow, black */
        if (h >= 0.02f && h <= 0.125f && s >= 0.25f && s <= 0.78f && v >= 0.74f) continue;   /* skin */
        if (L->cnt[i] * 100 < total) continue;                              /* small details */
        cand[nc++] = i;
    }
    /* families of shades (agglomerative, average linkage) */
    int fam[16]; for (int k = 0; k < nc; k++) fam[k] = k;
    for (;;) {
        float best = 1e9f; int fa = -1, fb = -1;
        for (int a = 0; a < nc; a++) for (int b = 0; b < nc; b++) {
            if (fam[a] != a || fam[b] != b || a >= b) continue;
            float sum = 0; int n = 0;
            for (int x = 0; x < nc; x++) if (fam[x] == a) for (int y = 0; y < nc; y++) if (fam[y] == b) { sum += fam_dist(L->rgb[cand[x]], L->rgb[cand[y]]); n++; }
            if (n && sum / n < best) { best = sum / n; fa = a; fb = b; }
        }
        if (fa < 0 || best > 0.30f) break;
        for (int x = 0; x < nc; x++) if (fam[x] == fb) fam[x] = fa;
    }
    int roots[16], nr = 0; long rc[16];
    for (int k = 0; k < nc; k++) if (fam[k] == k) {
        /* a family that sits at the top of the figure in a hair colour (blond, brown, grey) is hair:
           left as it is (no green hair) */
        long n = 0, up = 0; float hs = 0, ss = 0;
        for (int x = 0; x < nc; x++) if (fam[x] == k) { float h, sa, v; hsv(L->rgb[cand[x]], &h, &sa, &v); n += fcnt[cand[x]]; up += (long)(top[cand[x]] * fcnt[cand[x]]); hs += h * fcnt[cand[x]]; ss += sa * fcnt[cand[x]]; }
        if (n && up * 10 > n * 7) {
            float h = hs / n, sa = ss / n;
            if ((h >= 0.02f && h <= 0.18f && sa >= 0.25f) || sa < 0.2f) {
                if (logon) fprintf(stderr, "figures: %s: a family of %d colors is hair (%ld %% at the top)\n", L->code, (int)n, up * 100 / n);
                continue;
            }
        }
        rc[nr] = 0; for (int x = 0; x < nc; x++) if (fam[x] == k) rc[nr] += L->cnt[cand[x]]; roots[nr++] = k;
    }
    for (int a = 0; a < nr; a++) for (int b = a + 1; b < nr; b++) if (rc[b] > rc[a]) { long t = rc[a]; rc[a] = rc[b]; rc[b] = t; int r = roots[a]; roots[a] = roots[b]; roots[b] = r; }
    if (nr > 3) nr = 3;                                                     /* the rest: common colours */
    int size[3] = { 0 }, quota[3] = { 0 }, n = 0;
    for (int f = 0; f < nr; f++) { for (int x = 0; x < nc; x++) if (fam[x] == roots[f]) size[f]++; n += size[f]; }
    for (int f = 0; f < nr; f++) quota[f] = n <= 8 ? size[f] : (8 * size[f] / n > 0 ? 8 * size[f] / n : 1);
    for (;;) { int s = 0, big = -1; for (int f = 0; f < nr; f++) { s += quota[f]; if (quota[f] > 1 && (big < 0 || quota[f] > quota[big])) big = f; } if (s <= 8 || big < 0) break; quota[big]--; }
    for (;;) { int s = 0, need = -1; for (int f = 0; f < nr; f++) { s += quota[f]; if (quota[f] < size[f] && (need < 0 || size[f] - quota[f] > size[need] - quota[need])) need = f; } if (s >= 8 || need < 0) break; quota[need]++; }
    /* each family: shades by brightness, the closest neighbours merged down to its quota */
    L->nslot = 0; L->npart = 0;
    for (int f = 0; f < nr; f++) {
        int mem[16], m = 0;
        for (int x = 0; x < nc; x++) if (fam[x] == roots[f]) mem[m++] = cand[x];
        float lum[16];
        for (int a = 0; a < m; a++) lum[a] = (L->rgb[mem[a]][0] * 3 + L->rgb[mem[a]][1] * 4 + L->rgb[mem[a]][2] * 2) / 9.f;
        for (int a = 0; a < m; a++) for (int b = a + 1; b < m; b++) if (lum[b] < lum[a]) { float t = lum[a]; lum[a] = lum[b]; lum[b] = t; int q = mem[a]; mem[a] = mem[b]; mem[b] = q; }
        int grp[16], ng = m; for (int a = 0; a < m; a++) grp[a] = a;      /* group = index of its first member */
        while (ng > quota[f]) {
            int best = -1; float bd = 1e9f;
            for (int a = 1; a < m; a++) if (grp[a] != grp[a - 1] && lum[a] - lum[a - 1] < bd) { bd = lum[a] - lum[a - 1]; best = a; }
            if (best < 0) break;
            int from = grp[best], to = grp[best - 1];
            for (int a = 0; a < m; a++) if (grp[a] == from) grp[a] = to;
            ng--;
        }
        L->partn[L->npart] = 0; L->neutral[L->npart] = 1;
        for (int a = 0; a < m; a++) {
            if (grp[a] != a) continue;
            if (L->nslot >= 8) break;
            long w = 0, r = 0, g = 0, b = 0;
            for (int x = 0; x < m; x++) if (grp[x] == a) { long c = L->cnt[mem[x]]; w += c; r += c * L->rgb[mem[x]][0]; g += c * L->rgb[mem[x]][1]; b += c * L->rgb[mem[x]][2];
                                                          L->slot[mem[x]] = (s8)L->nslot; if (!neutral_c(L->rgb[mem[x]])) L->neutral[L->npart] = 0; }
            if (!w) w = 1;
            L->def[L->nslot][0] = (u8)((r + w / 2) / w); L->def[L->nslot][1] = (u8)((g + w / 2) / w); L->def[L->nslot][2] = (u8)((b + w / 2) / w);
            L->part[L->npart][L->partn[L->npart]++] = (s8)L->nslot;
            L->nslot++;
        }
        if (L->partn[L->npart]) L->npart++;
    }
    {   /* parts that hardly show (the bandit's own colours are a few pixels of its belt: its clothes
           are drawn in the common colours): no colour keys for them */
        long ftot = 0, fpart[3] = { 0 };
        for (int i = 0; i < 16; i++) ftot += fcnt[i];
        for (int i = 0; i < 16; i++) if (L->slot[i] >= 0)
            for (int pp = 0; pp < L->npart; pp++) for (int q = 0; q < L->partn[pp]; q++) if (L->part[pp][q] == L->slot[i]) fpart[pp] += fcnt[i];
        Imc m; long fall = 0;
        if (imc_open(L->pic_file, &m)) {
            static u8 px[256 * 256]; int fw, fh;
            if (imc_frame(&m, 4 * m.fc, px, &fw, &fh)) for (int k = 0; k < fw * fh; k++) if (px[k]) fall++;
            imc_close(&m);
        }
        int keep = 0;
        for (int pp = 0; pp < L->npart; pp++) {
            if (fall && fpart[pp] * 100 < fall * 4) { if (logon) fprintf(stderr, "figures: %s: part %d hardly shows (%ld of %ld pixels): no color key\n", L->code, pp, fpart[pp], fall); continue; }
            if (keep != pp) { memcpy(L->part[keep], L->part[pp], sizeof L->part[0]); L->partn[keep] = L->partn[pp]; L->neutral[keep] = L->neutral[pp]; }
            keep++;
        }
        L->npart = keep;
        (void)ftot;
    }
    L->state = 1;
    {   /* debug: DL_LOOKS_DUMP=dir writes each figure's front view: its own colours, and the parts tinted */
        const char *dd = getenv("DL_LOOKS_DUMP");
        Imc m;
        if (dd && imc_open(L->pic_file, &m)) {
            static u8 px[256 * 256]; int fw, fh;
            if (imc_frame(&m, 4 * m.fc, px, &fw, &fh) && fw > 0 && fh > 0) {
                int K = 4, NP = 5, W = (fw * K + K) * NP, H = fh * K;
                u8 *img = calloc((size_t)W * H * 3, 1);
                static const u8 TINT[3][3] = { { 255, 60, 60 }, { 60, 220, 60 }, { 70, 110, 255 } };
                for (int y = 0; y < fh; y++) for (int x = 0; x < fw; x++) {
                    int c = px[y * fw + x]; if (!c) continue;
                    u8 rgb[3], t[3];
                    if (c >= L->S && c < L->S + 16) memcpy(rgb, L->rgb[c - L->S], 3); else memcpy(rgb, common[c], 3);
                    for (int k = 0; k < 3; k++) rgb[k] = (u8)(rgb[k] << 2 | rgb[k] >> 4);
                    int part = -1;
                    if (c >= L->S && c < L->S + 16 && L->slot[c - L->S] >= 0)
                        for (int pp = 0; pp < L->npart; pp++) for (int q = 0; q < L->partn[pp]; q++) if (L->part[pp][q] == L->slot[c - L->S]) part = pp;
                    int lum = (rgb[0] * 3 + rgb[1] * 4 + rgb[2] * 2) / 9;
                    for (int k = 0; k < 3; k++) t[k] = part >= 0 ? (u8)(TINT[part][k] * (64 + lum) / 320) : (u8)(lum / 2);
                    u8 dyed[3][3];                       /* part 0 red, part 1 blue, part 2 green (or part 0 in each) */
                    for (int v = 0; v < 3; v++) {
                        static const int DY[3] = { 1, 2, 3 };
                        int pp = L->npart > 1 ? v % L->npart : 0;
                        u8 o6[3];
                        if (c >= L->S && c < L->S + 16) memcpy(o6, L->rgb[c - L->S], 3); else memcpy(o6, common[c], 3);
                        u8 d6[3];
                        if (part == pp) dye(o6, &DYE_C[DY[v]], L->neutral[pp], d6); else memcpy(d6, o6, 3);
                        for (int k = 0; k < 3; k++) dyed[v][k] = (u8)(d6[k] << 2 | d6[k] >> 4);
                    }
                    for (int dy = 0; dy < K; dy++) for (int dx = 0; dx < K; dx++) {
                        size_t row = (size_t)(y * K + dy) * W;
                        memcpy(img + (row + x * K + dx) * 3, rgb, 3);
                        memcpy(img + (row + (fw * K + K) + x * K + dx) * 3, t, 3);
                        for (int v = 0; v < 3; v++) memcpy(img + (row + (fw * K + K) * (2 + v) + x * K + dx) * 3, dyed[v], 3);
                    }
                }
                char path[1200]; snprintf(path, sizeof path, "%s/%s.png", dd, L->code);
                int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
                stbi_write_png(path, W, H, 3, img, W * 3);
                free(img);
            }
            imc_close(&m);
        }
    }
    if (logon) {
        fprintf(stderr, "figures: %s (%s): colors %d..%d, %d own shades in %d parts, weapons", L->code, L->title, L->S, L->S + 15, L->nslot, L->npart);
        for (int i = 0; i < L->nweap; i++) fprintf(stderr, " %s", L->weap[i]);
        fprintf(stderr, ", pictures from %s\n  ", L->pic_file);
        for (int i = 0; i < 16; i++) if (L->cnt[i]) fprintf(stderr, " %d:%s%d(%ld/%ld)", i, L->slot[i] >= 0 ? "slot" : "fix", L->slot[i] >= 0 ? L->slot[i] : L->fix[i], fcnt[i], L->cnt[i]);
        fprintf(stderr, "\n");
    }
}

static void list_init(void)
{
    if (nlooks >= 0) return;
    logon = getenv("DL_LOOKS_LOG") != 0;
    nlooks = 0;
    for (int i = 0; i < NKNOWN && nlooks < MAXLOOK; i++) {
        Look *L = &looks[nlooks++];
        memset(L, 0, sizeof *L);
        snprintf(L->code, sizeof L->code, "%s", KNOWN[i].code);
        snprintf(L->title, sizeof L->title, "%s", KNOWN[i].title);
    }
    /* other human groups a mod brings (E??DY.IMC in E00C.CAT and an enemy type of that group):
       named after the enemy type */
    if (!cat_load()) return;
    long nenm; u8 *enm = slurp("DARKLAND.ENM", &nenm);
    for (int i = 0; i < ncat && nlooks < MAXLOOK; i++) {
        const char *n = cat[i].name;
        if (toupper((unsigned char)n[0]) != 'E' || strcasecmp(n + 3, "DY.IMC")) continue;
        int dup = 0; for (int k = 0; k < nlooks; k++) if (!strncasecmp(looks[k].code, n, 3)) dup = 1;
        if (dup) continue;
        for (int t = 0; enm && t < 71 && (t + 1) * 204 <= nenm; t++) {
            if (strncasecmp((const char *)enm + t * 204, n, 3)) continue;
            Look *L = &looks[nlooks++];
            memset(L, 0, sizeof *L);
            snprintf(L->code, sizeof L->code, "%.3s", n);
            char tn[11]; memcpy(tn, enm + t * 204 + 4, 10); tn[10] = 0;
            for (int k = (int)strlen(tn) - 1; k > 0 && isdigit((unsigned char)tn[k]); k--) tn[k] = 0;
            snprintf(L->title, sizeof L->title, "%s", tn);
            break;
        }
    }
    free(enm);
}
static Look *look_find(const char *code)
{
    list_init();
    if (!code || toupper((unsigned char)code[0]) != 'E') return 0;
    for (int i = 0; i < nlooks; i++)
        if (!strncasecmp(looks[i].code, code, 3)) {
            Look *L = &looks[i];
            if (!L->state) analyse(L);
            return L->state > 0 ? L : 0;
        }
    return 0;
}

/* ------------------------------------------------------------------ the public side */
int looks_offered(void) { return config_get_int("party.more_figures", 1) && !config_get_int("original", 0); }
int looks_count(void) { list_init(); return nlooks; }
const char *looks_code(int i) { list_init(); return i >= 0 && i < nlooks ? looks[i].code : ""; }
const char *looks_title(int i) { list_init(); return i >= 0 && i < nlooks ? looks[i].title : ""; }
int looks_usable(int i) { list_init(); return i >= 0 && i < nlooks && look_find(looks[i].code) != 0; }
/* the weapons figure code has its own animations for (the suffixes of its XXXWK??.IMC with an
   XXXCB??.IMC): -1 for the game's own figures, which have them all */
int looks_weapons(const char *code, char out[][3], int max)
{
    Look *L = look_find(code);
    if (!L) return -1;
    int n = 0;
    for (int i = 0; i < L->nweap && n < max; i++) { out[n][0] = L->weap[i][0]; out[n][1] = L->weap[i][1]; out[n][2] = 0; n++; }
    return n;
}
int looks_is_extra(const char *code) { return look_find(code) != 0; }
const char *looks_title_of(const char *code)
{
    static const struct { const char *c, *t; } ORIG[] = { { "F01", "Tall man" }, { "F60", "Woman" }, { "C00", "Stout priest" }, { "A00", "Hooded man" } };
    for (int i = 0; i < 4; i++) if (!strncasecmp(code, ORIG[i].c, 3)) return ORIG[i].t;
    Look *L = look_find(code);
    return L ? L->title : "";
}

static const u8 DEFAULT_PAL[24] = { 0x39, 0x36, 0, 0x3d, 0x38, 0x15, 0, 0x18, 0x28, 0, 0x11, 0x25, 0, 0x15, 0x25, 0x1c, 0, 0, 0x17, 0, 0, 0x11, 0, 0 };
static u8 saved_pal[5][24]; static int saved_have[5];
static s8 dye_at[5][3];

static void portrait_show(int m)                /* the creation screen's portrait shows member m */
{
    for (int i = 0; i < 24; i++) {
        u8 v = rb(G_FAR20A5, (u16)(F_PALETTE + m * 24 + i));
        g_wb((u16)(D_PORTRAIT + i), v);
        vga_dac[0xeb * 3 + i] = v & 63;
    }
}
static void palette_set(int m, const u8 *p) { for (int i = 0; i < 24; i++) wb(G_FAR20A5, (u16)(F_PALETTE + m * 24 + i), p[i]); }
static void palette_get(int m, u8 *p) { for (int i = 0; i < 24; i++) p[i] = rb(G_FAR20A5, (u16)(F_PALETTE + m * 24 + i)); }

/* member m's figure changes from `from` to `to`: its colours go with it */
static void figure_changed(int m, const char *from, const char *to, int show)
{
    if (m < 0 || m >= 5 || !strncasecmp(from, to, 3)) return;
    Look *a = look_find(from), *b = look_find(to);
    if (!a && !b) return;
    u8 p[24];
    if (!a) { palette_get(m, saved_pal[m]); saved_have[m] = 1; }        /* the original figure's colours, for coming back */
    if (b) {
        memset(p, 0, sizeof p);
        for (int k = 0; k < b->nslot; k++) memcpy(p + 3 * k, b->def[k], 3);
    } else memcpy(p, saved_have[m] ? saved_pal[m] : DEFAULT_PAL, 24);
    palette_set(m, p);
    memset(dye_at[m], 0, sizeof dye_at[m]);
    if (show) portrait_show(m);
    LOG("figures: member %d: %.3s -> %.3s\n", m, from, to);
}

void dos_host_path_write(const char *dos, char *out, size_t outn);
void looks_set_figure(int m, const char *code)      /* (the editor) */
{
    if (m < 0 || m >= 5) return;
    char old[4]; for (int i = 0; i < 4; i++) old[i] = (char)g_rb((u16)(D_FIGURE + 4 * m + i));
    old[3] = 0;
    u8 fig[4] = { (u8)code[0], (u8)code[1], (u8)code[2], 0 };
    for (int i = 0; i < 4; i++) g_wb((u16)(D_FIGURE + 4 * m + i), fig[i]);
    /* and in charactr.tmp, where the battles take it from (s043 cards_shutdown reads it back) */
    char p[1300]; dos_host_path_write("charactr.tmp", p, sizeof p);
    FILE *f = fopen(p, "r+b");
    if (f) { if (!fseek(f, 12 + 4 * m, SEEK_SET)) fwrite(fig, 1, 4, f); fclose(f); }
    figure_changed(m, old, code, 0);
}

/* the next dye of part `part` of member m's figure; its name, 0 if that figure has no such part */
static const char *dye_next(int m, int part, int show)
{
    char code[4]; for (int i = 0; i < 3; i++) code[i] = (char)g_rb((u16)(D_FIGURE + 4 * m + i));
    code[3] = 0;
    Look *L = look_find(code);
    if (!L || part >= L->npart) return 0;
    int nd = L->neutral[part] ? NDYE_N : NDYE_C;
    const Dye *dy = L->neutral[part] ? DYE_N : DYE_C;
    int d = (dye_at[m][part] + 1) % nd;
    dye_at[m][part] = (s8)d;
    for (int j = 0; j < L->partn[part]; j++) {
        int k = L->part[part][j]; u8 c[3];
        dye(L->def[k], &dy[d], L->neutral[part], c);
        for (int i = 0; i < 3; i++) wb(G_FAR20A5, (u16)(F_PALETTE + m * 24 + 3 * k + i), c[i]);
    }
    if (show) portrait_show(m);
    LOG("figures: member %d (%s) part %d: %s\n", m, code, part, dy[d].name);
    return dy[d].name;
}

/* ------------------------------------------------------------------ the picker (ui/figures.cpp) */
int figpick_open(int member, int count);                /* (ui) opens it: 1 */
static int view_file(const char *file, const Look *L, int dir, int maxw, int maxh, u8 *out, int *w, int *h);
static char pending[4]; static int pending_on;
int looks_picker_on(void) { return looks_offered() && config_get_int("party.figure_picker", 1); }

/* the figures Select Character Image offers, in its order: the game's list, then these */
int looks_list(char out[][4], int max)
{
    int cnt = (s16)g_rw(D_FIGCOUNT), n = 0;
    if (cnt < 0 || cnt > 10) cnt = 0;
    for (int i = 0; i < cnt && n < max; i++, n++) { for (int k = 0; k < 3; k++) out[n][k] = (char)g_rb((u16)(D_FIGLIST + 4 * i + k)); out[n][3] = 0; }
    list_init();
    if (looks_offered()) for (int i = 0; i < nlooks && n < max; i++) if (look_find(looks[i].code)) snprintf(out[n++], 4, "%s", looks[i].code);
    return n;
}
void looks_member_figure(int m, char *code) { for (int k = 0; k < 3; k++) code[k] = (char)g_rb((u16)(D_FIGURE + 4 * m + k)); code[3] = 0; }

/* the picker's choice: Select Character Image again (S), which takes it */
void looks_pick(const char *code)
{
    snprintf(pending, sizeof pending, "%s", code); pending_on = 1;
    kbd_push_scancode(0x1f, 0); kbd_push_scancode(0x1f, 1);
}
/* the figure the next Select Character Image (S) takes, the caller giving the S itself */
void looks_pick_next(const char *code) { snprintf(pending, sizeof pending, "%s", code); pending_on = 1; }

static u8 rgb8(u8 v) { v &= 63; return (u8)(v << 2 | v >> 4); }
/* member m in figure `code`, seen from the front: RGBA (w x h at most maxw x maxh); 0 if none */
int looks_thumb(const char *code, int m, u8 *rgba, int maxw, int maxh, int *w, int *h)
{
    list_init(); common_load();
    Look *L = look_find(code);
    char file[16] = "";
    if (L) snprintf(file, sizeof file, "%s", L->pic_file);
    else {
        char f[16]; snprintf(f, sizeof f, "%.3sWKSW.IMC", code);
        if (cat_find(f)) snprintf(file, sizeof file, "%s", f);
        else for (int k = 1; k < 5 && !file[0]; k++) { Cat *c = cat_open(k); for (int i = 0; c && i < c->n; i++) if (!strncasecmp(c->e[i].name, code, 3) && !strncasecmp(c->e[i].name + 3, "WK", 2)) { snprintf(file, sizeof file, "%s", c->e[i].name); break; } }
    }
    if (!file[0]) return 0;
    static u8 px[256 * 256];
    if (!view_file(file, L, 4, maxw, maxh, px, w, h)) return 0;
    char cur[4]; looks_member_figure(m, cur);
    int own = !strncasecmp(cur, code, 3);
    u8 mp[24];                                          /* the member colours it would have */
    if (own || (!L && !look_find(cur))) palette_get(m, mp);
    else if (L) { memset(mp, 0, sizeof mp); for (int k = 0; k < L->nslot; k++) memcpy(mp + 3 * k, L->def[k], 3); }
    else memcpy(mp, saved_have[m] ? saved_pal[m] : DEFAULT_PAL, 24);
    for (int i = 0; i < *w * *h; i++) {
        u8 p = px[i], *o = rgba + 4 * i;
        const u8 *c;
        u8 tmp[3];
        if (!p) { o[0] = o[1] = o[2] = o[3] = 0; continue; }
        if (L && p >= L->S && p < L->S + 16) {
            int sl = L->slot[p - L->S];
            c = sl >= 0 ? mp + 3 * sl : common[L->fix[p - L->S]];
        } else if (p >= 0xeb && p <= 0xf2) c = mp + 3 * (p - 0xeb);
        else if ((p >= 0x10 && p < 0x20) || (p >= 0x78 && p < 0xa4) || is_target[p]) c = common[p];
        else { tmp[0] = vga_dac[3 * p]; tmp[1] = vga_dac[3 * p + 1]; tmp[2] = vga_dac[3 * p + 2]; c = tmp; }
        o[0] = rgb8(c[0]); o[1] = rgb8(c[1]); o[2] = rgb8(c[2]); o[3] = 255;
    }
    return 1;
}

/* ------------------------------------------------------------------ the creation menus */
void ui_notice(const char *msg);
static void figure_notice(const char *msg) { if (!config_get_int("original", 0)) ui_notice(msg); }

static void read_party(int ids[5], char fig[5][4])
{
    char p[1300]; dos_host_path_read("charactr.tmp", p, sizeof p);
    FILE *f = fopen(p, "rb");
    u8 b[32]; memset(b, 0xff, sizeof b);
    if (f) { if (fread(b, 1, 32, f) != 32) memset(b, 0xff, sizeof b); fclose(f); }
    for (int i = 0; i < 5; i++) { ids[i] = (s16)(b[2 + 2 * i] | b[3 + 2 * i] << 8); memcpy(fig[i], b + 12 + 4 * i, 4); fig[i][3] = 0; }
}
static int slot_of(int id) { int ids[5]; char fig[5][4]; read_party(ids, fig); for (int i = 0; i < 5; i++) if (ids[i] == id) return i; return -1; }

static void key_hook(int ovl)
{
    int key = ARG(0);
    int is_s = key == 0x1f53 || key == 0x1f73;
    int part = key == 0x231 || key == 0x4f31 ? 0 : key == 0x332 || key == 0x5032 ? 1 : key == 0x433 || key == 0x5133 ? 2 : -1;
    if (is_s) {
        int sel = (s16)g_rw(D_SELCHAR);
        int id = ovl == 38 ? (s16)g_rw((u16)(D_CHARIDS + 2 * sel)) : sel;
        int ids[5]; char before[5][4]; read_party(ids, before);
        int m = -1; for (int i = 0; i < 5; i++) if (ids[i] == id) m = i;
        u8 keep[44]; for (int i = 0; i < 40; i++) keep[i] = g_rb((u16)(D_FIGLIST + i));
        int cnt = (s16)g_rw(D_FIGCOUNT), cur = (s16)g_rw(D_FIGCUR);
        int patched = 0;
        if (m >= 0 && cnt > 0 && cnt <= 10) {
            /* the full list: the game's own, then these */
            char all[10 + MAXLOOK][4]; int n = 0, at = -1;
            for (int i = 0; i < cnt; i++) { for (int k = 0; k < 4; k++) all[n][k] = (char)keep[4 * i + k]; all[n][3] = 0; n++; }
            list_init();
            if (looks_offered()) for (int i = 0; i < nlooks; i++) if (look_find(looks[i].code)) snprintf(all[n++], 4, "%s", looks[i].code);
            for (int i = 0; i < n; i++) if (!strncasecmp(all[i], before[m], 3)) at = i;
            if (at < 0 && look_find(before[m])) at = cnt - 1;              /* an extra one while they are not offered: back to the first */
            if (!pending_on && looks_picker_on() && !config_get_int("original", 0)) {
                if (figpick_open(m, n)) { AX = 0; hook_return_far(0); return; }       /* the picker sends S again with the choice */
            }
            const char *next = all[at < 0 ? (cur + 1) % cnt : (at + 1) % n];
            if (pending_on) { for (int i = 0; i < n; i++) if (!strncasecmp(all[i], pending, 3)) next = all[i]; pending_on = 0; }
            for (int k = 0; k < 4; k++) g_wb((u16)(D_FIGLIST + 4 + k), (u8)(k < 3 ? next[k] : 0));
            g_ww(D_FIGCUR, 0); if (cnt < 2) g_ww(D_FIGCOUNT, 2);
            patched = 1;
        }
        hook_call_original();
        u16 ax = AX;
        if (patched) {
            char now[4]; for (int k = 0; k < 4; k++) now[k] = (char)g_rb((u16)(D_FIGURE + 4 * m + k));
            now[3] = 0;
            for (int i = 0; i < 40; i++) g_wb((u16)(D_FIGLIST + i), keep[i]);
            g_ww(D_FIGCOUNT, (u16)cnt);
            int pos = 0; for (int i = 0; i < cnt; i++) if (!strncasecmp((const char *)keep + 4 * i, now, 3)) pos = i;
            g_ww(D_FIGCUR, (u16)pos);
            figure_changed(m, before[m], now, 1);
            if (looks_offered() && strncasecmp(before[m], now, 3)) {
                int k = 0, n = cnt; for (int i = 0; i < cnt; i++) if (!strncasecmp((const char *)keep + 4 * i, now, 3)) k = i + 1;
                for (int i = 0; i < nlooks; i++) if (look_find(looks[i].code)) { n++; if (!strncasecmp(looks[i].code, now, 3)) k = n; }
                Look *L = look_find(now);
                char msg[160];
                if (L) snprintf(msg, sizeof msg, "Sprite %d of %d: %s (an enemy's sprite%s)", k, n, L->title,
                                L->npart > 2 ? "; the 1st, 2nd and 3rd color buttons recolour it" : L->npart > 1 ? "; the 1st and 2nd color buttons recolour it" :
                                L->npart ? "; the 1st color button recolours it" : "");
                else snprintf(msg, sizeof msg, "Sprite %d of %d: %s", k, n, looks_title_of(now)[0] ? looks_title_of(now) : now);
                figure_notice(msg);
            }
        }
        AX = ax;
        return;
    }
    int slots_move = key == 0x1e41 || key == 0x1e61 || key == 0x2044 || key == 0x2064 || key == 0x254b || key == 0x256b;   /* A, D, K */
    hook_call_original();
    if (slots_move) { memset(saved_have, 0, sizeof saved_have); memset(dye_at, 0, sizeof dye_at); }   /* (members may have moved up) */
    if (part >= 0) {
        u16 ax = AX;
        int m = slot_of((s16)g_rw(D_SELCHAR));
        if (m >= 0) {
            const char *d = dye_next(m, part, 1);
            if (d) { char msg[80]; snprintf(msg, sizeof msg, "%s color: %s", part == 0 ? "1st" : part == 1 ? "2nd" : "3rd", d); figure_notice(msg); }
            else {
                char code[4]; looks_member_figure(m, code);
                if (look_find(code)) figure_notice(part ? "This sprite has no more colors that can be changed." : "This sprite's colors are fixed: they cannot be changed.");
            }
        }
        AX = ax;
    }
}
static void key37_hook(void) { key_hook(37); }
static void key38_hook(void) { key_hook(38); }

/* ------------------------------------------------------------------ battles */
static const char *FALLBACK[][7] = {
    { "SW", "S2", "AX", "MC", "DR", "CL", 0 }, { "S2", "SW", "A2", "HB", "AX", 0 }, { "DR", "SW", "CL", "MC", "AX", 0 },
    { "AX", "A2", "MC", "HM", "SW", 0 }, { "A2", "AX", "HB", "S2", "SW", 0 }, { "MC", "HM", "CL", "AX", "FL", "SW", 0 },
    { "M2", "MC", "HM", "A2", "S2", 0 }, { "HM", "MC", "CL", "AX", 0 }, { "H2", "HM", "MC", "A2", "S2", 0 },
    { "FL", "MC", "HM", "CL", 0 }, { "F2", "FL", "MC", "A2", 0 }, { "CL", "MC", "HM", "QS", "SW", 0 },
    { "QS", "SP", "PT", "HB", "CL", 0 }, { "SP", "HB", "QS", "PT", 0 }, { "HB", "SP", "A2", "QS", 0 },
    { "PT", "QS", "DR", "SW", 0 }, { "BW", "CB", "PT", 0 }, { "CB", "BW", "PT", 0 }, { "HG", "CB", "BW", "PT", 0 },
    { "TA", "TK", "DT", "PT", "AX", 0 }, { "TK", "DT", "TA", "PT", "DR", 0 }, { "DT", "TK", "TA", "PT", 0 },
};
static int has_weapon(const Look *L, const char *w) { for (int i = 0; i < L->nweap; i++) if (!strncasecmp(L->weap[i], w, 2)) return 1; return 0; }
static const char *weapon_for(const Look *L, const char *w)
{
    if (has_weapon(L, w)) return 0;
    for (int r = 0; r < (int)(sizeof FALLBACK / sizeof FALLBACK[0]); r++)
        if (!strncasecmp(FALLBACK[r][0], w, 2))
            for (int k = 1; FALLBACK[r][k]; k++) if (has_weapon(L, FALLBACK[r][k])) return FALLBACK[r][k];
    static const char *MISSILE = "PT BW CB HG TA TK DT";
    if (has_weapon(L, "SW")) return "SW";
    for (int i = 0; i < L->nweap; i++) if (!strstr(MISSILE, L->weap[i])) return L->weap[i];  /* a hand weapon before a throw */
    return L->weap[0];
}

/* cbt_set_anim(i, mode): a party member with one of these figures and a weapon it has no
 * animation for gets the nearest one for this call */
static void set_anim_hook(void)
{
    int i = (s16)ARG(0), mode = (s16)ARG(1);
    int kind = i >= 0 && i < 13 ? (s8)g_rb((u16)(D_CBT + 0x16 + 0x80 * i)) : -1;
    if (kind >= 0 && kind < 5 && mode != 4) {
        u16 gp = g_rw((u16)(D_GROUPS + 2 * kind)), wp = g_rw((u16)(D_WEAPONS + 2 * kind));
        char grp[4] = { (char)g_rb(gp), (char)g_rb((u16)(gp + 1)), (char)g_rb((u16)(gp + 2)), 0 };
        Look *L = look_find(grp);
        if (L) {
            char w[3] = { (char)g_rb(wp), (char)g_rb((u16)(wp + 1)), 0 };
            const char *f = weapon_for(L, w);
            if (f) {
                g_wb(wp, (u8)f[0]); g_wb((u16)(wp + 1), (u8)f[1]);
                hook_call_original();
                g_wb(wp, (u8)w[0]); g_wb((u16)(wp + 1), (u8)w[1]);
                return;
            }
        }
    }
    hook_call_original();
}

/* (from hdart's hook on sprite_file_load) a party slot's sprites were loaded: the figure's
 * enemy colours onto the member's own */
void looks_sprite_loaded(const char *name, u16 frames, int slot)
{
    if (slot < 0 || slot >= 5) return;
    Look *L = look_find(name);
    if (!L) return;
    u8 lut[256]; for (int i = 0; i < 256; i++) lut[i] = (u8)i;
    for (int i = 0; i < 16; i++) lut[L->S + i] = L->slot[i] >= 0 ? (u8)((slot + 10) * 8 + L->slot[i]) : L->fix[i];
    int n = (s8)g_rb((u16)(D_FRAMES + slot)) * 8;
    u16 seen[512]; int ns = 0, changed = 0;
    for (int k = 0; k < n && k < 512; k++) {
        u16 seg = g_rw((u16)(frames + 2 * k));
        int dup = 0; for (int j = 0; j < ns; j++) if (seen[j] == seg) dup = 1;
        if (dup || !seg) continue;
        seen[ns++] = seg;
        int h = rb(seg, 1); u16 o = 2;
        for (int y = 0; y < h; y++) {
            int c = rb(seg, o);
            for (int x = 0; x < c; x++) { u16 a = (u16)(o + 2 + x); u8 v = rb(seg, a); if (lut[v] != v) { wb(seg, a, lut[v]); changed++; } }
            o = (u16)(o + 2 + c);
        }
    }
    LOG("figures: %s in slot %d: %d frames, %d pixels to the member's colors\n", name, slot, ns, changed);
}

/* ------------------------------------------------------------------ pictures */
static void shadow_off(u8 *px, int w, int h, const Look *L)
{
    u8 dark[256]; memset(dark, 0, sizeof dark); dark[5] = 1;
    for (int i = 0; L && i < 16; i++) { int mx = L->rgb[i][0] > L->rgb[i][1] ? L->rgb[i][0] : L->rgb[i][1]; if (L->rgb[i][2] > mx) mx = L->rgb[i][2]; if (mx <= 8) dark[L->S + i] = 1; }
    static int qx[65536], qy[65536]; int qn = 0;
    static u8 seen[256 * 256]; memset(seen, 0, (size_t)w * h);
    for (int y = h - 1; y >= 0 && y >= h - 3; y--) for (int x = 0; x < w; x++) if (dark[px[y * w + x]]) { qx[qn] = x; qy[qn] = y; qn++; seen[y * w + x] = 1; }
    while (qn) {
        qn--; int x = qx[qn], y = qy[qn];
        px[y * w + x] = 0;
        static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
        for (int k = 0; k < 4; k++) {
            int nx = x + d[k][0], ny = y + d[k][1];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h || ny < h - 6 || seen[ny * w + nx] || !dark[px[ny * w + nx]]) continue;
            seen[ny * w + nx] = 1; qx[qn] = nx; qy[qn] = ny; qn++;
        }
    }
}
static int bbox(const u8 *px, int w, int h, int *x0, int *y0, int *x1, int *y1)
{
    *x0 = w; *y0 = h; *x1 = -1; *y1 = -1;
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) if (px[y * w + x]) { if (x < *x0) *x0 = x; if (x > *x1) *x1 = x; if (y < *y0) *y0 = y; if (y > *y1) *y1 = y; }
    return *x1 >= 0;
}
static u8 pic_colour(const Look *L, u8 p, int creation)
{
    if (p >= L->S && p < L->S + 16) return L->slot[p - L->S] >= 0 ? (u8)(0xeb + L->slot[p - L->S]) : L->fix[p - L->S];
    if (creation && ((p >= 0x10 && p < 0x20) || (p >= 0x78 && p < 0x80) || (p >= 0xa0 && p < 0xa4))) return nearest_common(common[p]);
    return p;
}
/* the figure's front (dir 4) or back (dir 0) view, shadow off, cropped to the figure: at most
   maxw wide around the body (a staff or a raised sword is cut) and maxh high from the feet */
static int view_file(const char *file, const Look *L, int dir, int maxw, int maxh, u8 *out, int *w, int *h);
static int view(const Look *L, int dir, int maxw, int maxh, u8 *out, int *w, int *h) { return view_file(L->pic_file, L, dir, maxw, maxh, out, w, h); }
static int view_file(const char *file, const Look *L, int dir, int maxw, int maxh, u8 *out, int *w, int *h)
{
    Imc m; if (!imc_open(file, &m)) return 0;
    static u8 px[256 * 256]; int fw, fh, ok = imc_frame(&m, dir * m.fc, px, &fw, &fh);
    imc_close(&m);
    if (!ok) return 0;
    shadow_off(px, fw, fh, L);
    int x0, y0, x1, y1; if (!bbox(px, fw, fh, &x0, &y0, &x1, &y1)) return 0;
    if (x1 - x0 + 1 > maxw) {                       /* the body: the widest maxw columns' worth of pixels */
        int col[256] = { 0 }, best = x0, bs = -1;
        for (int y = y0; y <= y1; y++) for (int x = x0; x <= x1; x++) if (px[y * fw + x]) col[x]++;
        for (int a = x0; a + maxw - 1 <= x1; a++) { int sum = 0; for (int x = a; x < a + maxw; x++) sum += col[x]; if (sum > bs) { bs = sum; best = a; } }
        x0 = best; x1 = best + maxw - 1;
        for (int y = 0; y < fh; y++) for (int x = 0; x < fw; x++) if (x < x0 || x > x1) px[y * fw + x] = 0;
        if (!bbox(px, fw, fh, &x0, &y0, &x1, &y1)) return 0;
    }
    if (y1 - y0 + 1 > maxh) y0 = y1 - maxh + 1;
    *w = x1 - x0 + 1; *h = y1 - y0 + 1;
    for (int y = 0; y < *h; y++) memcpy(out + y * *w, px + (y0 + y) * fw + x0, (size_t)*w);
    return 1;
}
/* the portrait of the creation menus: front and back. The menus clear 36 x 36 pixels from one
   column left and one row below where they draw it (s037 0444 / s038 02f8), so everything stays
   inside 34 x 35 from its second row (the game's own are 35..41 wide with the figures inside) */
static long make_small(const Look *L, u8 **file)
{
    enum { W = 34, H = 34 };
    u8 out[W * H]; memset(out, 0, sizeof out);
    static u8 v[2][256 * 256]; int vw[2], vh[2];
    if (!view(L, 4, 15, 30, v[0], &vw[0], &vh[0]) || !view(L, 0, 15, 30, v[1], &vw[1], &vh[1])) return -1;
    int x = (W - (vw[0] + 2 + vw[1])) / 2; if (x < 0) x = 0;
    for (int f = 0; f < 2; f++) {
        int y0 = 31 - vh[f]; if (y0 < 1) y0 = 1;
        for (int y = 0; y < vh[f]; y++) for (int xx = 0; xx < vw[f]; xx++) {
            u8 p = v[f][y * vw[f] + xx];
            if (p && x + xx < W && y0 + y < H) out[(y0 + y) * W + x + xx] = pic_colour(L, p, 1);
        }
        x += vw[f] + 2;
    }
    return hdart_pic_encode(out, W, H, file);
}
static long make_short(const Look *L, u8 **file)
{
    enum { W = 11, H = 19 };
    u8 out[W * H]; memset(out, 0, sizeof out);
    static u8 v[256 * 256]; int fw, fh;
    if (!view(L, 4, 15, 40, v, &fw, &fh)) return -1;
    float sc = (float)W / fw < (float)H / fh ? (float)W / fw : (float)H / fh;
    int tw = (int)(fw * sc + 0.5f), th = (int)(fh * sc + 0.5f); if (tw < 1) tw = 1; if (th < 1) th = 1;
    int ox = (W - tw) / 2, oy = H - th;
    for (int ty = 0; ty < th; ty++) for (int tx = 0; tx < tw; tx++) {
        int sx0 = (int)(tx / sc), sx1 = (int)((tx + 1) / sc), sy0 = (int)(ty / sc), sy1 = (int)((ty + 1) / sc);
        if (sx1 <= sx0) sx1 = sx0 + 1;
        if (sy1 <= sy0) sy1 = sy0 + 1;
        int hist[256]; memset(hist, 0, sizeof hist); int tot = 0, on = 0, best = 0;
        for (int sy = sy0; sy < sy1 && sy < fh; sy++) for (int sx = sx0; sx < sx1 && sx < fw; sx++) {
            tot++; u8 p = v[sy * fw + sx];
            if (p) { on++; if (++hist[p] > hist[best] || !best) best = p; }
        }
        if (on && on * 2 >= tot) out[(oy + ty) * W + ox + tx] = pic_colour(L, (u8)best, 0);
    }
    return hdart_pic_encode(out, W, H, file);
}

/* a picture the game opens that does not exist: one made for these figures, or 0 */
const char *looks_virtual_file(const char *dosname)
{
    const char *b = dosname; for (const char *q = dosname; *q; q++) if (*q == '\\' || *q == '/') b = q + 1;
    if (strlen(b) != 12) return 0;
    int small = !strcasecmp(b + 3, "SMALL.PIC"), shrt = !strcasecmp(b + 3, "SHORT.PIC");
    if (!small && !shrt) return 0;
    char code[4] = { b[0], b[1], b[2], 0 };
    Look *L = look_find(code);
    if (!L) return 0;
    static char path[1300];
    char dir[1200]; snprintf(dir, sizeof dir, "%s/.cache", USER_DIR); MKDIR(dir);
    snprintf(dir, sizeof dir, "%s/.cache/figures", USER_DIR); MKDIR(dir);
    char up[13]; for (int i = 0; i < 13; i++) up[i] = (char)toupper((unsigned char)b[i]);
    snprintf(path, sizeof path, "%s/%s", dir, up);
    struct stat st;
    if (L->made[small] && !stat(path, &st)) return path;
    u8 *file = 0; long n = small ? make_small(L, &file) : make_short(L, &file);
    if (n <= 0) { free(file); LOG("figures: could not make %s\n", up); return 0; }
    FILE *f = fopen(path, "wb");
    if (!f) { free(file); return 0; }
    fwrite(file, 1, (size_t)n, f); fclose(f); free(file);
    L->made[small] = 1;
    LOG("figures: made %s (%ld bytes)\n", path, n);
    return path;
}

/* ------------------------------------------------------------------ */
void looks_init(void)
{
    logon = getenv("DL_LOOKS_LOG") != 0;                /* (the list itself: on first use, when the game's files are set up) */
    hook_install(37, 0x1462, 0x0444, key37_hook);
    hook_install(38, 0x1462, 0x02f8, key38_hook);
    hook_install(GF_cbt_set_anim, set_anim_hook);
    if (getenv("DL_LOOKS_DUMP"))
        for (int i = 0; i < nlooks; i++) {
            if (!look_find(looks[i].code)) continue;
            char n[32]; snprintf(n, sizeof n, "PICS\\%sSMALL.PIC", looks[i].code); looks_virtual_file(n);
            snprintf(n, sizeof n, "PICS\\%sSHORT.PIC", looks[i].code); looks_virtual_file(n);
        }
}
