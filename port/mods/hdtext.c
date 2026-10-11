/* hdtext.c - crisp text. Darklands draws its text with small bitmap fonts (FONTS.FNT; the
 * card font is a 7-pixel blackletter) that players find hard to read once scaled up. Here
 * every string the game draws on the visible page is noted (gfx_text, 047c:055a, the one
 * entry point for all game text: window, x, y, string), with the picture under it. When the
 * frame is shown, each glyph is checked against the screen: if its pixels are still there
 * (in one colour; a highlight may have recoloured it) and the background around it is
 * unchanged, the bitmap glyph is wiped from the picture and the interface draws the text
 * again with a real font at window resolution. Anything that does not check out (scrolled,
 * covered, half erased) simply stays as the game drew it.
 *
 * Window record (DGROUP): +0 page, +2/+4 origin, +6/+8 clip, +0a flags (bit 0 opaque),
 * +0c colour (low byte; high byte draw mode: 0 mov, 3 xor), +0e background, +10 font
 * (1-based into FONTS.FNT). String bytes >= 0x80 switch the colour (byte & 0x7f). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "hdtext.h"

extern char game_dir[1024];
extern u16 drvseg_mgraphic;

int hdtext_mode = 2;                 /* 0 off, 1 card font, 2 all fonts */

/* ---------------------------------------------------------------- fonts */
typedef struct { int first, last, bpr, height, cspace, lspace; const u8 *widths, *bitmap; } Fnt;
static Fnt fnt[4]; static int nfnt; static u8 *fntdata;

static void load_fonts(void)
{
    char p[1200]; snprintf(p, sizeof p, "%s/FONTS.FNT", game_dir);
    FILE *f = fopen(p, "rb");
    if (!f) { snprintf(p, sizeof p, "%s/fonts.fnt", game_dir); f = fopen(p, "rb"); }
    if (!f) return;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    fntdata = malloc((size_t)n);
    if (!fntdata || fread(fntdata, 1, (size_t)n, f) != (size_t)n) { fclose(f); return; }
    fclose(f);
    int cnt = fntdata[0] | fntdata[1] << 8;
    for (int i = 0; i < cnt && i < 4; i++) {
        int o = fntdata[2 + 2 * i] | fntdata[3 + 2 * i] << 8;
        if (o < 8 || o > n) break;
        const u8 *h = fntdata + o - 8;
        Fnt *F = &fnt[i];
        F->first = h[0]; F->last = h[1]; F->bpr = h[2]; F->height = h[4]; F->cspace = h[5]; F->lspace = h[6];
        F->widths = h - (F->last - F->first + 1); F->bitmap = fntdata + o;
        nfnt = i + 1;
    }
}

static int glyph_w(const Fnt *F, int c) { return (c < F->first || c > F->last) ? 0 : F->widths[c - F->first]; }
static int glyph_bit(const Fnt *F, int c, int x, int y)
{
    int cnt = F->last - F->first + 1, i = c - F->first;
    const u8 *row = F->bitmap + ((size_t)y * cnt + i) * F->bpr;
    return (row[x >> 3] >> (7 - (x & 7))) & 1;
}

/* ---------------------------------------------------------------- records */
#define MAXREC 512
#define MAXG 96
typedef struct {
    int used, page, font, x0, y0, w, h;   /* box on its page (0 the screen) */
    u32 stamp;
    int n;                             /* glyphs */
    int xr;                            /* drawn xor (option lists): a glyph pixel is the picture's ^ colour */
    int is_shadow;                     /* the same text drawn again a pixel off covers this one (engraved */
    int sh; u32 sh_stamp;              /* lettering); sh: slot + 1 of this record's shadow */
    u8 ch[MAXG]; short gx[MAXG]; u8 gw[MAXG]; u8 col[MAXG];
    u8 *under;                         /* the picture under the box before the text */
    u8 *bg;                            /* what the box shows now where there is no text (recolours applied) */
} Rec;
static Rec rec[MAXREC];
static u32 stamp;

static u8 *page0(void) { return M + 0xa0000; }
/* a page's pixels: page 0 is the screen; the others are 320x200 buffers whose segments the
 * graphics driver keeps in a table (MGRAPHIC cs:19b0, a word per page) */
static u8 *page_mem(int page)
{
    { static int done; if (!done && drvseg_mgraphic && getenv("DL_PAGE_INFO")) { done = 1; fprintf(stderr, "pages:");
        for (int p = 0; p < 8; p++) fprintf(stderr, " %d=%04x", p, rw(drvseg_mgraphic, (u16)(0x19b0 + 2 * p))); fprintf(stderr, "\n"); } }
    if (page == 0) return page0();
    if (page < 0 || page > 15 || !drvseg_mgraphic) return 0;
    u16 seg = rw(drvseg_mgraphic, (u16)(0x19b0 + 2 * page));
    if (!seg || seg == 0xa000) return 0;
    return M + ((u32)seg << 4);
}
static void rec_free(Rec *r) { r->used = 0; free(r->under); r->under = 0; free(r->bg); r->bg = 0; }

static void drop_covered(int page, int x0, int y0, int w, int h)
{
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (r->used && r->page == page && r->x0 >= x0 && r->y0 >= y0 && r->x0 + r->w <= x0 + w && r->y0 + r->h <= y0 + h) rec_free(r);
    }
}

/* a picture or a copied rectangle lands on the visible page: text it touches is gone (a
 * new picture over old text could otherwise pass the checks where it is plain enough) */
void hdtext_cover(int page, int x0, int y0, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (r->used && r->page == page && r->x0 < x0 + w && x0 < r->x0 + r->w && r->y0 < y0 + h && y0 < r->y0 + r->h) rec_free(r);
    }
}

static int rec_slot(void)
{
    int slot = 0; u32 oldest = 0xffffffffu;
    for (int i = 0; i < MAXREC; i++) { if (!rec[i].used) return i; if (rec[i].stamp < oldest) { oldest = rec[i].stamp; slot = i; } }
    rec_free(&rec[slot]);
    return slot;
}

/* engraved lettering (menus, the party list, help boxes): the text is drawn twice, a pixel
 * apart, dark then light. The later copy is the text; the earlier one its shadow, wiped with it */
static void link_shadow(int slot, int need_flag)
{
    Rec *r = &rec[slot];
    for (int i = 0; i < MAXREC; i++) {
        Rec *o = &rec[i];
        if (i == slot || !o->used || o->page != r->page || o->font != r->font || o->n != r->n || (need_flag && !o->is_shadow)) continue;
        int dx = r->x0 - o->x0, dy = r->y0 - o->y0;
        if ((!dx && !dy) || dx < -1 || dx > 1 || dy < -1 || dy > 1 || memcmp(o->ch, r->ch, (size_t)r->n)) continue;
        if (!need_flag && o->stamp > r->stamp) continue;
        o->is_shadow = 1; r->sh = i + 1; r->sh_stamp = o->stamp;
        return;
    }
}
static Rec *shadow_of(Rec *r)
{
    if (!r->sh) return 0;
    Rec *o = &rec[r->sh - 1];
    return o->used && o->stamp == r->sh_stamp ? o : 0;
}

/* a rectangle copied from one page to another (gfx_copy_rect; called by hdart.c after the
 * copy): the text inside it goes along. The party panel, the formula book and other screens
 * are drawn off screen and then copied up whole. */
void hdtext_copy(int sp, int ax, int ay, int w, int h, int dp, int bx, int by)
{
    if (!hdtext_mode || sp == dp || w <= 0 || h <= 0) return;
    int dx = bx - ax, dy = by - ay;
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (!r->used || r->page != sp || r->x0 < ax || r->y0 < ay || r->x0 >= ax + w || r->y0 + r->h > ay + h) continue;
        /* the glyphs that lie wholly inside the rectangle (a trailing space often does not) */
        int n = 0, nw = 0;
        while (n < r->n && r->x0 + r->gx[n] + r->gw[n] <= ax + w) { nw = r->gx[n] + r->gw[n]; n++; }
        if (!n) continue;
        int nx = r->x0 + dx, ny = r->y0 + dy;
        if (nx < 0 || ny < 0 || nx + nw > 320 || ny + r->h > 200) continue;
        int slot = rec_slot();
        if (slot == i || !r->used) continue;
        Rec c = *r;
        c.page = dp; c.x0 = nx; c.y0 = ny; c.n = n; c.w = nw; c.sh = 0;
        c.under = malloc((size_t)nw * r->h); c.bg = malloc((size_t)nw * r->h);
        for (int yy = 0; yy < r->h; yy++) { memcpy(c.under + yy * nw, r->under + yy * r->w, (size_t)nw); memcpy(c.bg + yy * nw, r->bg + yy * r->w, (size_t)nw); }
        c.stamp = ++stamp;
        rec[slot] = c;
    }
    for (int i = 0; i < MAXREC; i++)
        if (rec[i].used && rec[i].page == dp && !rec[i].is_shadow && rec[i].x0 >= bx && rec[i].y0 >= by && rec[i].x0 < bx + w && rec[i].y0 < by + h) link_shadow(i, 1);
}

/* recolours of the screen keep the records' backgrounds in step: gfx_recolor_rect (root
 * 047c:04dc: win, x, y, w, h, from, to), and the option lists' fade (s043 palette_fade,
 * 17e5:0008), after which the card code turns every pixel 0x20..0x7f into the fade colour */
static void recolor_box(int x0, int y0, int w, int h, int from, int to)
{
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (!r->used || !r->bg || r->page != 0) continue;
        for (int yy = 0; yy < r->h; yy++) {
            int y = r->y0 + yy; if (y < y0 || y >= y0 + h) continue;
            for (int xx = 0; xx < r->w; xx++) {
                int x = r->x0 + xx; if (x < x0 || x >= x0 + w) continue;
                u8 *b = &r->bg[yy * r->w + xx];
                if (from < 0 ? (*b >= 0x20 && *b < 0x80) : *b == from) *b = (u8)to;
            }
        }
    }
}
static void recolor_hook(void)
{
    u16 w = rw(SS, (u16)(SP + 4));
    int x = (s16)rw(SS, (u16)(SP + 6)), y = (s16)rw(SS, (u16)(SP + 8)), ww_ = (s16)rw(SS, (u16)(SP + 10)), h = (s16)rw(SS, (u16)(SP + 12));
    int from = rb(SS, (u16)(SP + 14)), to = rb(SS, (u16)(SP + 16));
    { static int lg = -1; if (lg < 0) lg = getenv("DL_TRACE_RECOLOR") != 0;
      if (lg) { u32 plat_ticks_ms(void); fprintf(stderr, "recolor %u: page %d at %d,%d + %d,%d %dx%d %02x->%02x\n", plat_ticks_ms(), (s16)g_rw(w), (s16)g_rw((u16)(w + 2)), (s16)g_rw((u16)(w + 4)), x, y, ww_, h, from, to); } }
    hook_call_original();
    if (!hdtext_mode || (s16)g_rw(w) != 0) return;
    recolor_box((s16)g_rw((u16)(w + 2)) + x, (s16)g_rw((u16)(w + 4)) + y, ww_, h, from, to);
}
static void fade_hook(void)
{
    int c = rb(SS, (u16)(SP + 4));
    hook_call_original();
    if (hdtext_mode) recolor_box(0, 0, 320, 200, -1, c);
}

static void text_hook(void)                 /* gfx_text(win, x, y, char *s) */
{
    if (!hdtext_mode || !nfnt) { hook_call_original(); return; }
    u16 w = rw(SS, (u16)(SP + 4)); int x = (s16)rw(SS, (u16)(SP + 6)), y = (s16)rw(SS, (u16)(SP + 8)); u16 sp = rw(SS, (u16)(SP + 10));
    int page = (s16)g_rw(w), font = (s16)g_rw((u16)(w + 0x10));
    u8 *pm = page_mem(page);
    if (!pm || font < 1 || font > nfnt || (hdtext_mode == 1 && (font != 3 || page != 0))) {
        { static int lg = -1; if (lg < 0) lg = getenv("DL_HD_LOG") != 0;
          if (lg) { char t[64]; int k; for (k = 0; k < 63 && rb(SS, (u16)(sp + k)); k++) { u8 c = rb(SS, (u16)(sp + k)); t[k] = c >= 32 && c < 127 ? (char)c : '?'; } t[k] = 0;
                    fprintf(stderr, "hd: skipped text page %d font %d at %d,%d: %s\n", page, font, x, y, t); } }
        hook_call_original(); return;
    }
    const Fnt *F = &fnt[font - 1];
    int sx = (s16)g_rw((u16)(w + 2)) + x, sy = (s16)g_rw((u16)(w + 4)) + y;
    int col = g_rb((u16)(w + 0x0c));
    Rec r; memset(&r, 0, sizeof r);
    r.page = page; r.font = font; r.x0 = sx; r.y0 = sy; r.h = F->height;
    int cx = sx;
    for (int k = 0; k < 200; k++) {
        u8 c = rb(SS, (u16)(sp + k));
        if (!c) break;
        if (c >= 0x80) { col = c == 0xff ? g_rb((u16)(w + 0x0c)) : (c & 0x7f); continue; }
        int gw = glyph_w(F, c);
        if (!gw) continue;
        if (r.n < MAXG) { r.ch[r.n] = c; r.gx[r.n] = (short)(cx - sx); r.gw[r.n] = (u8)gw; r.col[r.n] = (u8)col; r.n++; }
        cx += gw + F->cspace;
    }
    r.w = cx - sx;
    if (r.n == 0 || sx < 0 || sy < 0 || cx > 320 || sy + r.h > 200) { hook_call_original(); return; }
    r.xr = g_rb((u16)(w + 0x0d)) == 3;
    if (g_rb((u16)(w + 0x0d))) {            /* xor: drawing the same text again erases it (option lists) */
        for (int i = 0; i < MAXREC; i++) {
            Rec *o = &rec[i];
            if (o->used && o->page == page && o->x0 == sx && o->y0 == sy && o->font == font && o->n == r.n && !memcmp(o->ch, r.ch, (size_t)r.n)) {
                hook_call_original();
                hdtext_cover(page, sx, sy, r.w, r.h);
                if (getenv("DL_HD_LOG")) fprintf(stderr, "hd: xor text at %d,%d erased\n", sx, sy);
                return;
            }
        }
    }
    if (getenv("DL_HD_LOG")) { char t[100]; int k; for (k = 0; k < r.n && k < 99; k++) t[k] = (char)r.ch[k]; t[k] = 0;
        fprintf(stderr, "hd: text at %d,%d mode %d colour %02x font %d page %d win %04x: %s\n", sx, sy, g_rb((u16)(w + 0x0d)), g_rb((u16)(w + 0x0c)), font, page, w, t); }
    r.under = malloc((size_t)r.w * r.h);
    for (int yy = 0; yy < r.h; yy++) memcpy(r.under + yy * r.w, pm + (sy + yy) * 320 + sx, (size_t)r.w);
    r.bg = malloc((size_t)r.w * r.h); memcpy(r.bg, r.under, (size_t)r.w * r.h);
    hook_call_original();
    drop_covered(page, r.x0, r.y0, r.w, r.h);
    int slot = rec_slot();
    r.used = 1; r.stamp = ++stamp;
    rec[slot] = r;
    link_shadow(slot, 0);
}

/* the game's software mouse cursor (MGRAPHIC data: +0x20a x, +0x20c y, +0x20e/0x210 size,
 * +0x212 shown); glyphs under it keep their last verdict */
static void cursor_rect(int *x, int *y, int *w, int *h)
{
    *w = *h = 0;
    if (!drvseg_mgraphic) return;
    u16 ds = (u16)(drvseg_mgraphic + 0x140);
    if (!rw(ds, 0x212)) return;
    *x = (s16)rw(ds, 0x20a); *y = (s16)rw(ds, 0x20c); *w = (s16)rw(ds, 0x20e); *h = (s16)rw(ds, 0x210);
}

HdRun hdtext_runs[HD_MAXRUNS];
int hdtext_nruns;

/* called with the frame about to be shown: wipe the glyphs that are still on screen from the
 * RGB picture and list the runs of text to draw again */
void hdtext_prepare(u32 *rgb, const u32 *lut, u8 *idx)
{
    hdtext_nruns = 0;
    if (!hdtext_mode) return;
    const u8 *vga = page0();
    static u8 claimed[320 * 200];
    memset(claimed, 0, sizeof claimed);
    int mx, my, mw, mh; cursor_rect(&mx, &my, &mw, &mh);
    /* newest first */
    static int order[MAXREC]; int no = 0;
    for (int i = 0; i < MAXREC; i++) if (rec[i].used && rec[i].page == 0) order[no++] = i;
    for (int a = 1; a < no; a++) { int t = order[a], b = a; while (b > 0 && rec[order[b - 1]].stamp < rec[t].stamp) { order[b] = order[b - 1]; b--; } order[b] = t; }
    static int dbgn = -1; if (dbgn < 0) dbgn = getenv("DL_HD_LOG") ? 1 : 0;
    int dbg = 0;
    for (int oi = 0; oi < no; oi++) {
        Rec *r = &rec[order[oi]];
        if (r->is_shadow) continue;               /* goes with the text it shadows */
        dbg = dbgn && oi < 4;
        { static const char *flt = (const char *)-1; if (flt == (const char *)-1) flt = getenv("DL_HD_DEBUG");
          if (flt) { char tx[MAXG + 1]; int k; for (k = 0; k < r->n; k++) tx[k] = (char)r->ch[k]; tx[k] = 0; dbg = strstr(tx, flt) != 0; } }
        const Fnt *F = &fnt[r->font - 1];
        int ok[MAXG], gc[MAXG], alive = 0;
        /* xor glyphs (gc = -1): their colour is the mean of the glyph pixels that look different
         * from the picture, so a line shows whole and in its colour while the game fades the
         * list in and where the palette leaves some of its pixels invisible */
        u32 xs_r[MAXG], xs_g[MAXG], xs_b[MAXG], xs_n[MAXG];
        for (int g = 0; g < r->n; g++) {
            ok[g] = 0; xs_n[g] = 0;
            int gx = r->x0 + r->gx[g], gw = r->gw[g];
            if (mw && gx < mx + mw && gx + gw > mx && r->y0 < my + mh && r->y0 + r->h > my) { ok[g] = 4; continue; }   /* under the cursor: decided below */
            int c = -1, good = 1, any = 0, seen = 0, xgood = r->xr;
            u32 sr = 0, sg = 0, sb = 0, sn = 0;
            for (int yy = 0; yy < r->h && (good || xgood); yy++)
                for (int xx = 0; xx < gw; xx++) {
                    int p = (r->y0 + yy) * 320 + gx + xx;
                    if (claimed[p]) { good = xgood = 0; break; }
                    u8 v = vga[p], u = r->bg[yy * r->w + r->gx[g] + xx], u0 = r->under[yy * r->w + r->gx[g] + xx];
                    if (glyph_bit(F, r->ch[g], xx, yy)) {
                        any = 1;
                        if (lut[v] != lut[u]) { seen++; sr += lut[v] >> 16 & 255; sg += lut[v] >> 8 & 255; sb += lut[v] & 255; sn++; }
                        if (xgood && v != (u8)(u0 ^ r->col[g])) { xgood = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: xor pixel %02x, expected %02x (bg %02x)\n", r->stamp, g, r->ch[g], xx, yy, v, u0 ^ r->col[g], u); }
                        if (good) { if (c < 0) c = v; else if (v != c) { good = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: glyph pixel %02x vs %02x (under %02x)\n", r->stamp, g, r->ch[g], xx, yy, v, c, u); } }
                    } else if (v != u) { good = xgood = 0; if (dbg && r->xr) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: background %02x was %02x\n", r->stamp, g, r->ch[g], xx, yy, v, u); break; }
                }
            if (!good && !xgood) continue;
            if (!any) { ok[g] = 2; continue; }                 /* a space: kept with its neighbours */
            if (!seen && !xgood) continue;                     /* the glyph cannot be told from what was under it: gone */
            /* (an xor glyph that looks like the picture right now still belongs to its line: the
               line is drawn in the colour of its visible glyphs, or not at all while none is) */
            if (xgood) { ok[g] = 1; gc[g] = -1; xs_r[g] = sr; xs_g[g] = sg; xs_b[g] = sb; xs_n[g] = sn; }   /* xor first: one colour for the line */
            else if (good && seen) { ok[g] = 1; gc[g] = c; }
            else continue;
            alive++;
        }
        { static int lg = -1; static u32 lastt; u32 plat_ticks_ms(void); if (lg < 0) lg = getenv("DL_HD_LOG2") != 0;
          if (lg && plat_ticks_ms() - lastt > 300) {
              char st[MAXG + 1]; int k; for (k = 0; k < r->n; k++) st[k] = ok[k] == 1 ? (gc[k] < 0 ? 'x' : 'o') : ok[k] == 2 ? '_' : ok[k] >= 4 ? 'c' : '.'; st[k] = 0;
              char tx[MAXG + 1]; for (k = 0; k < r->n; k++) tx[k] = (char)r->ch[k]; tx[k] = 0;
              fprintf(stderr, "hd2 %u: rec %u at %d,%d xr %d: %s\n          %s\n", plat_ticks_ms(), r->stamp, r->x0, r->y0, r->xr, tx, st);
              if (oi == no - 1) lastt = plat_ticks_ms();
          } }
        if (!alive) { if (stamp - r->stamp > 4000) { r->used = 0; free(r->under); r->under = 0; free(r->bg); r->bg = 0; } continue; }
        /* glyphs under the mouse cursor: the line is still there, so they are too, in the
         * colour of their neighbours (only the pixels not covered by the cursor are wiped) */
        for (int g = 0; g < r->n; g++) {
            if (ok[g] != 4) continue;
            int c = -2;
            for (int k = g - 1; k >= 0 && c == -2; k--) if (ok[k] == 1) c = gc[k];
            for (int k = g + 1; k < r->n && c == -2; k++) if (ok[k] == 1) c = gc[k];
            if (c == -2) { ok[g] = 0; continue; }
            gc[g] = c; xs_n[g] = 0; ok[g] = glyph_w(F, r->ch[g]) && r->ch[g] != ' ' ? 5 : 6;
        }
        /* wipe the verified glyphs and group them into runs of one colour */
        Rec *S = shadow_of(r);
        for (int g = 0; g < r->n; g++) {
            int under_cursor = ok[g] & 4;
            ok[g] &= 3;
            if (ok[g] != 1) continue;
            int gx = r->x0 + r->gx[g];
            for (int yy = 0; yy < r->h; yy++)
                for (int xx = 0; xx < r->gw[g]; xx++) {
                    int p = (r->y0 + yy) * 320 + gx + xx;
                    if (under_cursor && (gc[g] >= 0 ? vga[p] != gc[g] : vga[p] != (u8)(r->under[yy * r->w + r->gx[g] + xx] ^ r->col[g]))) continue;   /* the cursor's own pixels stay */
                    claimed[p] = 1;
                    if (glyph_bit(F, r->ch[g], xx, yy)) {
                        u8 u = r->bg[yy * r->w + r->gx[g] + xx];
                        if (S) {                   /* the shadow's pixel under the text: the picture under both */
                            int sx = r->x0 + r->gx[g] + xx - S->x0, sy = r->y0 + yy - S->y0;
                            if (sx >= 0 && sy >= 0 && sx < S->w && sy < S->h) u = S->bg[sy * S->w + sx];
                        }
                        rgb[p] = lut[u]; if (idx) idx[p] = u;
                    }
                }
        }
        if (S) {                                   /* the shadow's own pixels, where they still show */
            for (int k = 0; k < S->n; k++)
                for (int yy = 0; yy < S->h; yy++)
                    for (int xx = 0; xx < S->gw[k]; xx++) {
                        if (!glyph_bit(F, S->ch[k], xx, yy)) continue;
                        int x = S->x0 + S->gx[k] + xx, y = S->y0 + yy, p = y * 320 + x;
                        if (vga[p] != S->col[k]) continue;
                        int mx = x - r->x0, my = y - r->y0;
                        int g = -1;
                        for (int q = 0; q < r->n; q++) if (mx >= r->gx[q] && mx < r->gx[q] + r->gw[q]) { g = q; break; }
                        if (g >= 0 && my >= 0 && my < r->h && ok[g] != 1) continue;   /* beside a glyph that stays */
                        if (g >= 0 && my >= 0 && my < r->h && glyph_bit(F, r->ch[g], mx - r->gx[g], my)) continue;
                        u8 u = S->bg[yy * S->w + S->gx[k] + xx];
                        rgb[p] = lut[u]; if (idx) idx[p] = u; claimed[p] = 1;
                    }
        }
        int g = 0;
        while (g < r->n) {
            if (ok[g] != 1) { g++; continue; }
            int s = g, c = gc[g], e = g;
            u32 tr = 0, tg = 0, tb = 0, tn = 0;
            while (g < r->n && (ok[g] == 2 || (ok[g] == 1 && gc[g] == c))) {
                if (ok[g] == 1) { e = g; tr += xs_r[g]; tg += xs_g[g]; tb += xs_b[g]; tn += xs_n[g]; }
                g++;
            }
            g = e + 1;
            u32 rgb;
            if (c >= 0) rgb = lut[c];
            else if (tn) rgb = 0xff000000u | (tr / tn) << 16 | (tg / tn) << 8 | (tb / tn);
            else continue;
            if (hdtext_nruns >= HD_MAXRUNS) return;
            HdRun *u = &hdtext_runs[hdtext_nruns++];
            int n = 0;
            for (int k = s; k < g && n < (int)sizeof u->text - 1; k++) u->text[n++] = (char)r->ch[k];
            u->text[n] = 0;
            u->x = r->x0 + r->gx[s];
            u->w = r->gx[g - 1] + r->gw[g - 1] - r->gx[s];
            u->y = r->y0; u->h = r->h; u->font = r->font;
            u->rgb = rgb;
            {   u32 br = 0, bgg = 0, bb = 0, bn = 0;
                for (int yy = 0; yy < r->h; yy++)
                    for (int xx = r->gx[s]; xx < r->gx[g - 1] + r->gw[g - 1]; xx++) { u32 v = lut[r->bg[yy * r->w + xx]]; br += v >> 16 & 255; bgg += v >> 8 & 255; bb += v & 255; bn++; }
                u->bgrgb = bn ? (br / bn) << 16 | (bgg / bn) << 8 | (bb / bn) : 0; }
        }
    }
}

void hdtext_set(int mode) { hdtext_mode = mode; config_set_int("display.crisp_text", mode); }

void hdtext_init(void)
{
    load_fonts();
    hdtext_mode = config_get_int("display.crisp_text", 2);
    hook_install(-1, 0x047c, 0x055a, text_hook);
    hook_install(-1, 0x047c, 0x04dc, recolor_hook);
    hook_install(43, 0x17e5, 0x0008, fade_hook);
}
