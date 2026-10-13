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
#ifndef DL_RTSPLIT
extern u16 drvseg_mgraphic;
#endif

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

/* new text over old: the old record goes only when it is the same line drawn again (same
 * place, font and length). Other covered text is kept: the newer record claims the pixels
 * first, so the old one simply fails its checks while covered, and comes back when the game
 * puts the old picture back (the character sheet's skill box, opened wider over the narrow
 * one and closed again by restoring a saved image: the rows the wide names had covered
 * showed in the bitmap font) */
static void drop_covered(int page, int x0, int y0, int w, int h)
{
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (r->used && r->page == page && r->x0 == x0 && r->y0 == y0 && r->w == w && r->h == h) rec_free(r);
    }
}

/* a picture or a copied rectangle lands on the visible page: text it touches is gone (a
 * new picture over old text could otherwise pass the checks where it is plain enough) */
void hdtext_cover(int page, int x0, int y0, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    { static int lg = -1; if (lg < 0) lg = getenv("DL_HD_BLIT") != 0; if (lg && h > 1) fprintf(stderr, "hd: cover page %d %d,%d %dx%d\n", page, x0, y0, w, h); }
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
    { static int lg = -1; if (lg < 0) lg = getenv("DL_HD_BLIT") != 0; if (lg) fprintf(stderr, "hd: copy page %d %d,%d %dx%d -> page %d %d,%d\n", sp, ax, ay, w, h, dp, bx, by); }
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
    if ((s16)g_rw(w) != 0) return;
    int x0 = (s16)g_rw((u16)(w + 2)) + x, y0 = (s16)g_rw((u16)(w + 4)) + y;
    /* the picture the game's pointer hides (MGRAPHIC DS:0216, a box at DS:020a / 020c of DS:020e x
       DS:0210) is recoloured too: a highlight drawn while the pointer stood on the text left that
       copy in the old colours, and the pointer the interface draws shows the picture without the
       sprite, so the old letters appeared there (an orange "Qu", a dark "g ( 4)" in a blue line) */
    if (drvseg_mgraphic) {
        u16 ds = (u16)(drvseg_mgraphic + 0x140);
        int bx = (s16)rw(ds, 0x20a), by = (s16)rw(ds, 0x20c), bw = (s16)rw(ds, 0x20e), bh = (s16)rw(ds, 0x210);
        if (rw(ds, 0x212) && rw(ds, 0x214) == 0 && bw > 0 && bh > 0 && bw * bh <= 0x200)
            for (int r = 0; r < bh; r++) for (int c = 0; c < bw; c++) {
                int px = bx + c, py = by + r;
                if (px < x0 || px >= x0 + ww_ || py < y0 || py >= y0 + h) continue;
                u16 o = (u16)(0x216 + r * bw + c);
                if (rb(ds, o) == from) wb(ds, o, (u8)to);
            }
    }
    if (!hdtext_mode) return;
    recolor_box(x0, y0, ww_, h, from, to);
}
int hdtext_fast_fades;                     /* cards' fades at once: while "go to" walks (ui/citynav.cpp), or always (assist.fast_fades) */
static void fade_hook(void)                 /* palette_fade(a, b, c, d, duration) */
{
    int c = rb(SS, (u16)(SP + 4));
    if (hdtext_fast_fades || (config_get_int("assist.fast_fades", 1) && !config_get_int("original", 0))) ww(SS, (u16)(SP + 12), 0);   /* one step */
    hook_call_original();
    if (hdtext_mode) recolor_box(0, 0, 320, 200, -1, c);
}

/* text drawn again where the game drew text before (a menu line redrawn in another colour,
 * or the same): the old glyphs are not the new line's background; what was behind them is.
 * (Only at the same place: engraved lettering is the same text again a pixel off, and there
 * the first glyphs are part of the look.) */
static void inherit_bg(Rec *r)
{
    /* oldest first: a later record's background is the cleaner one where both have letters */
    static int ord[MAXREC]; int no = 0;
    for (int i = 0; i < MAXREC; i++) if (rec[i].used && rec[i].bg && rec[i].page == r->page) ord[no++] = i;
    {   /* none of the same line on this page: the same line on another (the off-screen page the
           game copies rows up from: the recruitment list after Add to the Party), its background */
        int have = 0, best = -1;
        for (int k = 0; k < no; k++) { const Rec *o = &rec[ord[k]]; if (o->x0 == r->x0 && o->y0 == r->y0 && o->font == r->font) have = 1; }
        if (!have)
            for (int i = 0; i < MAXREC; i++) {
                const Rec *o = &rec[i];
                if (!o->used || !o->bg || o->page == r->page || o->x0 != r->x0 || o->y0 != r->y0 || o->font != r->font || o->w != r->w || o->n != r->n || memcmp(o->ch, r->ch, (size_t)r->n)) continue;
                if (best < 0 || o->stamp > rec[best].stamp) best = i;
            }
        if (best >= 0) memcpy(r->bg, rec[best].bg, (size_t)r->w * r->h);
    }
    for (int a = 1; a < no; a++) { int t = ord[a], b = a; while (b > 0 && rec[ord[b - 1]].stamp > rec[t].stamp) { ord[b] = ord[b - 1]; b--; } ord[b] = t; }
    for (int k = 0; k < no; k++) {
        const Rec *o = &rec[ord[k]];
        int ax = r->x0 > o->x0 ? r->x0 : o->x0, ay = r->y0 > o->y0 ? r->y0 : o->y0;
        int bx = r->x0 + r->w < o->x0 + o->w ? r->x0 + r->w : o->x0 + o->w, by = r->y0 + r->h < o->y0 + o->h ? r->y0 + r->h : o->y0 + o->h;
        if (ax >= bx || ay >= by) continue;
        /* the same line drawn again: its background as it is; other text the new one is drawn
           over without the picture cleared first (the recruitment list redrawn after Add to the
           Party, over its engraved shadow a pixel off): where its letters still show */
        int same = o->x0 == r->x0 && o->y0 == r->y0 && o->font == r->font;
        if (!same) continue;            /* (engraved lettering: the same text a pixel off is part of the look) */
        const Fnt *F = &fnt[o->font - 1];
        for (int g = 0; g < o->n; g++) {
            int gx0 = o->x0 + o->gx[g];
            for (int x = gx0 > ax ? gx0 : ax; x < gx0 + o->gw[g] && x < bx; x++)
                for (int y = ay; y < by; y++)
                    if (glyph_bit(F, o->ch[g], x - gx0, y - o->y0)) {
                        u8 *d = &r->bg[(y - r->y0) * r->w + (x - r->x0)];
                        u8 ob = o->bg[(y - o->y0) * o->w + (x - o->x0)];
                        u8 letter = o->xr ? (u8)(o->under[(y - o->y0) * o->w + (x - o->x0)] ^ o->col[g]) : o->col[g];
                        if (same || *d == letter) *d = ob;
                    }
        }
    }
}

/* the glyphs of string sp in font F from sx (colour codes 0x80.. as the game's): the record's
 * ch/gx/gw/col; its width */
static int layout(const Fnt *F, u16 sp, int sx, int col, int defcol, Rec *r)
{
    int cx = sx; r->n = 0;
    for (int k = 0; k < 200; k++) {
        u8 c = rb(SS, (u16)(sp + k));
        if (!c) break;
        if (c >= 0x80) { col = c == 0xff ? defcol : (c & 0x7f); continue; }
        int gw = glyph_w(F, c);
        if (!gw) continue;
        if (r->n < MAXG) { r->ch[r->n] = c; r->gx[r->n] = (short)(cx - sx); r->gw[r->n] = (u8)gw; r->col[r->n] = (u8)col; r->n++; }
        cx += gw + F->cspace;
    }
    return cx - sx;
}
/* the text the game just drew is what font F gives: every pixel of the box (bw x bh, before:
 * under) changed exactly where F's glyphs are (to the glyph's colour, or xor) and nowhere else */
static int drawn_in(const Fnt *F, const Rec *r, const u8 *pm, const u8 *under, int sx, int sy, int bw, int bh, int xr)
{
    static u8 want[320 * 200]; static u8 is[320 * 200];
    memset(is, 0, (size_t)bw * bh);
    for (int g = 0; g < r->n; g++)
        for (int yy = 0; yy < F->height && yy < bh; yy++)
            for (int xx = 0; xx < r->gw[g]; xx++) {
                int x = r->gx[g] + xx;
                if (x >= bw || !glyph_bit(F, r->ch[g], xx, yy)) continue;
                is[yy * bw + x] = 1; want[yy * bw + x] = xr ? (u8)(under[yy * bw + x] ^ r->col[g]) : r->col[g];
            }
    for (int yy = 0; yy < bh; yy++)
        for (int x = 0; x < bw; x++) {
            u8 now = pm[(sy + yy) * 320 + sx + x], was = under[yy * bw + x];
            if (is[yy * bw + x]) { if (now != want[yy * bw + x]) return 0; }
            else if (now != was) return 0;
        }
    return 1;           /* (the same text drawn again over itself changes nothing and still fits) */
}

/* the same letters were under the new ones already (the line drawn before in another colour,
 * brought back by a copy from a page where nothing tells what was behind them: the recruitment
 * list after Add to the Party, whose old highlight came back blue from the off-screen page):
 * under the glyphs, the box's own background from the nearest pixel beside them, so a wipe
 * does not bring the old letters back around the crisp ones */
static void clean_old_letters(Rec *r, const Fnt *F)
{
    static u8 isg[320 * 16];
    if (r->w * r->h > (int)sizeof isg) return;
    memset(isg, 0, (size_t)r->w * r->h);
    int cnt[256] = { 0 }, ng = 0;
    for (int g = 0; g < r->n; g++)
        for (int yy = 0; yy < r->h; yy++)
            for (int xx = 0; xx < r->gw[g]; xx++)
                if (glyph_bit(F, r->ch[g], xx, yy)) { int p = yy * r->w + r->gx[g] + xx; if (!isg[p]) { isg[p] = 1; cnt[r->under[p]]++; ng++; } }
    if (ng < 4) return;
    int c = 0; for (int k = 1; k < 256; k++) if (cnt[k] > cnt[c]) c = k;
    int other = 0, nother = 0;
    for (int p = 0; p < r->w * r->h; p++) if (!isg[p]) { nother++; if (r->under[p] == c) other++; }
    if (cnt[c] * 10 < ng * 7 || other * 10 > nother) return;           /* not letters of this shape */
    for (int yy = 0; yy < r->h; yy++)
        for (int xx = 0; xx < r->w; xx++) {
            int p = yy * r->w + xx;
            if (!isg[p] || r->bg[p] != c) continue;
            int v = -1;
            for (int d = 1; d < r->w && v < 0; d++) {
                if (xx - d >= 0 && !isg[p - d] && r->under[p - d] != c) v = r->bg[p - d];
                else if (xx + d < r->w && !isg[p + d] && r->under[p + d] != c) v = r->bg[p + d];
            }
            if (v >= 0) r->bg[p] = (u8)v;
        }
    if (getenv("DL_HD_LOG")) fprintf(stderr, "hd: text at %d,%d over the same letters (colour %02x): their background guessed\n", r->x0, r->y0, c);
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
    int sx = (s16)g_rw((u16)(w + 2)) + x, sy = (s16)g_rw((u16)(w + 4)) + y;
    int defcol = g_rb((u16)(w + 0x0c));
    int mode = g_rb((u16)(w + 0x0d));
    /* the box every font would need: the game draws with the font its driver has selected, which
       is not always the window's (the recruitment list after Add to the Party: the panels' font) */
    static Rec lay[4]; int lw[4], bw = 0, bh = 0;
    memset(lay, 0, sizeof lay);
    for (int f = 0; f < nfnt; f++) { lw[f] = layout(&fnt[f], sp, sx, defcol, defcol, &lay[f]); if (lw[f] > bw) bw = lw[f]; if (fnt[f].height > bh) bh = fnt[f].height; }
    if (lay[font - 1].n == 0 || sx < 0 || sy < 0) { hook_call_original(); return; }
    if (sx + bw > 320) bw = 320 - sx;
    if (sy + bh > 200) bh = 200 - sy;
    if (bw <= 0 || bh <= 0) { hook_call_original(); return; }
    int xr = mode == 3;
    if (mode) {                                /* xor: drawing the same text again erases it (option lists) */
        const Fnt *F = &fnt[font - 1]; int fw = lw[font - 1];
        for (int i = 0; i < MAXREC; i++) {
            Rec *o = &rec[i];
            if (o->used && o->page == page && o->x0 == sx && o->y0 == sy && o->font == font && o->n == lay[font - 1].n && !memcmp(o->ch, lay[font - 1].ch, (size_t)o->n)) {
                hook_call_original();
                hdtext_cover(page, sx, sy, fw, F->height);
                if (getenv("DL_HD_LOG")) fprintf(stderr, "hd: xor text at %d,%d erased\n", sx, sy);
                return;
            }
        }
    }
    u8 *big = malloc((size_t)bw * bh);
    for (int yy = 0; yy < bh; yy++) memcpy(big + yy * bw, pm + (sy + yy) * 320 + sx, (size_t)bw);
    hook_call_original();
    /* which font it was: the window's, else whichever the pixels show; none (clipped, drawn over
       something moving): no record, the game's own letters stay */
    int used = -1;
    if (drawn_in(&fnt[font - 1], &lay[font - 1], pm, big, sx, sy, bw, bh, xr)) used = font - 1;
    else for (int f = 0; f < nfnt && used < 0; f++) if (f != font - 1 && sx + lw[f] <= 320 && drawn_in(&fnt[f], &lay[f], pm, big, sx, sy, bw, bh, xr)) used = f;
    if (getenv("DL_HD_LOG")) { char t[100]; int k; for (k = 0; k < lay[font - 1].n && k < 99; k++) t[k] = (char)lay[font - 1].ch[k]; t[k] = 0;
        fprintf(stderr, "hd: text at %d,%d mode %d colour %02x font %d (drawn in %d) page %d win %04x: %s\n", sx, sy, mode, defcol, font, used + 1, page, w, t); }
    if (used < 0) { drop_covered(page, sx, sy, lw[font - 1], fnt[font - 1].height); free(big); return; }
    const Fnt *F = &fnt[used];
    Rec r = lay[used];
    r.page = page; r.font = used + 1; r.x0 = sx; r.y0 = sy; r.h = F->height; r.w = lw[used];
    r.xr = xr;
    if (sx + r.w > 320 || sy + r.h > 200) { free(big); return; }
    r.under = malloc((size_t)r.w * r.h);
    for (int yy = 0; yy < r.h; yy++) memcpy(r.under + yy * r.w, big + yy * bw, (size_t)r.w);
    free(big);
    r.bg = malloc((size_t)r.w * r.h); memcpy(r.bg, r.under, (size_t)r.w * r.h);
    inherit_bg(&r);
    if (!xr) clean_old_letters(&r, F);
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

/* where the game's own mouse pointer is drawn (1), or 0 when it is hidden */
int hdtext_game_cursor(int *x, int *y, int *w, int *h) { cursor_rect(x, y, w, h); return *w > 0; }

/* the pointer as the driver drew it on the screen (MGRAPHIC 0000:029a / 033c): its sprite
 * (DS:0200 the sprite's segment: width, height, then each row a count, a start and that many
 * pixels, 0 clear) at DS:0202, DS:0204, on page DS:0214; the picture it hides saved at
 * DS:0216, rows of DS:020e at DS:020a, DS:020c, DS:0210 of them. The text checks look at the
 * screen without it (so the letters under it are wiped and drawn again like the rest), and
 * the interface draws it again over the crisp text (hdtext_cursor) */
#define CUR_MAX 48
static int cur_on, cur_x, cur_y, cur_w, cur_h;
static u8 cur_px[CUR_MAX * CUR_MAX];
static u32 cur_rgb[CUR_MAX * CUR_MAX];
static u8 clean[320 * 200];
static void cursor_read(const u32 *lut)
{
    cur_on = 0;
    memcpy(clean, page0(), sizeof clean);
    if (!drvseg_mgraphic) return;
    u16 ds = (u16)(drvseg_mgraphic + 0x140);
    { static int lg = -1; if (lg < 0) lg = getenv("DL_CUR_LOG") != 0;
      if (lg) { static char last[200]; char b[200]; snprintf(b, sizeof b, "cursor: shown %d page %d save %d,%d %dx%d at %d,%d", rw(ds, 0x212), rw(ds, 0x214), (s16)rw(ds, 0x20a), (s16)rw(ds, 0x20c), (s16)rw(ds, 0x20e), (s16)rw(ds, 0x210), (s16)rw(ds, 0x202), (s16)rw(ds, 0x204));
                if (strcmp(b, last)) { strcpy(last, b); fprintf(stderr, "%s\n", b); } } }
    if (!rw(ds, 0x212) || rw(ds, 0x214) != 0) return;
    int bx = (s16)rw(ds, 0x20a), by = (s16)rw(ds, 0x20c), bw = (s16)rw(ds, 0x20e), bh = (s16)rw(ds, 0x210);
    if (bx >= 0 && by >= 0 && bw > 0 && bh > 0 && bx + bw <= 320 && by + bh <= 200 && bw * bh <= 0x200)
        for (int r = 0; r < bh; r++) for (int c = 0; c < bw; c++) clean[(by + r) * 320 + bx + c] = rb(ds, (u16)(0x216 + r * bw + c));
    u16 seg = rw(ds, 0x200);
    if (!seg) return;
    int w = rb(seg, 0), h = rb(seg, 1);
    if (!w || !h || w > CUR_MAX || h > CUR_MAX) return;
    memset(cur_px, 0, sizeof cur_px);
    u16 o = 2;
    for (int r = 0; r < h; r++) {
        int n = rb(seg, o), st = rb(seg, (u16)(o + 1)); o = (u16)(o + 2);
        for (int i = 0; i < n; i++) { u8 c = rb(seg, (u16)(o + i)); if (c && st + i < CUR_MAX) cur_px[r * CUR_MAX + st + i] = c; }
        o = (u16)(o + n);
    }
    cur_x = (s16)rw(ds, 0x202); cur_y = (s16)rw(ds, 0x204); cur_w = w; cur_h = h;
    for (int i = 0; i < CUR_MAX * CUR_MAX; i++) cur_rgb[i] = cur_px[i] ? (lut[cur_px[i]] | 0xff000000u) : 0;
    cur_on = 1;
}
int hdtext_live_pointer;
static int on_cursor(int x, int y)
{
    if (hdtext_live_pointer) return 0;      /* its sprite is taken out of the picture (pointer_out): the letters under it are wiped like the rest */
    x -= cur_x; y -= cur_y;
    return cur_on && x >= 0 && y >= 0 && x < cur_w && y < cur_h && cur_px[y * CUR_MAX + x];
}
/* the pointer's pixels for the frame just prepared (0xffRRGGBB, 0 clear; CUR_MAX a row) */
const unsigned *hdtext_cursor(int *x, int *y, int *w, int *h, int *stride)
{
    if (!cur_on) return 0;
    *x = cur_x; *y = cur_y; *w = cur_w; *h = cur_h; *stride = CUR_MAX;
    return cur_rgb;
}

/* The interface draws the pointer itself, where the mouse is now, rather than where the
 * game last drew it: the game moves its pointer only when it gets round to it (a frame or
 * two late, in whole game pixels - 3 or 4 screen pixels a step), which felt sluggish. Its
 * sprite is taken out of the picture (hdtext_live_pointer, set by the interface while it
 * draws the pointer); its offset from the mouse position (the hot spot) is learnt while
 * the mouse stands still. */
static int hot_x, hot_y; static u16 hot_seg; static int hot_w, hot_h;
void plat_mouse_game(int *x, int *y);
void hdtext_pointer_hotspot(int *hx, int *hy) { *hx = hot_x; *hy = hot_y; }
static void pointer_out(u32 *rgb, const u32 *lut, u8 *idx)
{
    if (!cur_on) return;
    {   static int lmx = -1, lmy = -1, still; int mx, my; plat_mouse_game(&mx, &my);
        u16 seg = drvseg_mgraphic ? rw((u16)(drvseg_mgraphic + 0x140), 0x200) : 0;
        if (seg != hot_seg || cur_w != hot_w || cur_h != hot_h) { hot_seg = seg; hot_w = cur_w; hot_h = cur_h; hot_x = hot_y = 0; still = 0; }
        if (mx == lmx && my == lmy) still++; else still = 0;
        lmx = mx; lmy = my;
        if (still >= 3) { int hx = cur_x - mx, hy = cur_y - my; if (hx > -CUR_MAX && hx <= 0 && hy > -CUR_MAX && hy <= 0) { hot_x = hx; hot_y = hy; } }
    }
    if (!hdtext_live_pointer) return;
    for (int r = 0; r < cur_h; r++)
        for (int c = 0; c < cur_w; c++) {
            int x = cur_x + c, y = cur_y + r;
            if (!cur_px[r * CUR_MAX + c] || x < 0 || y < 0 || x >= 320 || y >= 200) continue;
            int p = y * 320 + x;
            rgb[p] = lut[clean[p]]; if (idx) idx[p] = clean[p];
        }
}

/* one glyph of a record, rows y0..y1: still on the screen as drawn (good: its pixels in one
 * colour, a highlight may have recoloured it; xgood: drawn xor over the picture), the
 * picture around it unchanged? */
typedef struct { int good, xgood, any, seen, c; u32 sr, sg, sb, sn; } Chk;
static void glyph_check(const Rec *r, const Fnt *F, int g, int y0, int y1, const u8 *vga, const u8 *claimed, const u32 *lut, int dbg, Chk *q)
{
    int gx = r->x0 + r->gx[g], gw = r->gw[g];
    int c = -1, good = 1, any = 0, seen = 0, xgood = r->xr;
    u32 sr = 0, sg = 0, sb = 0, sn = 0;
    for (int yy = y0; yy < y1 && (good || xgood); yy++)
        for (int xx = 0; xx < gw; xx++) {
            int p = (r->y0 + yy) * 320 + gx + xx;
            if (claimed[p]) { good = xgood = 0; break; }
            u8 v = vga[p], u = r->bg[yy * r->w + r->gx[g] + xx], u0 = r->under[yy * r->w + r->gx[g] + xx];
            if (glyph_bit(F, r->ch[g], xx, yy)) {
                any = 1;
                if (lut[v] != lut[u]) { seen++; sr += lut[v] >> 16 & 255; sg += lut[v] >> 8 & 255; sb += lut[v] & 255; sn++; }
                if (xgood && v != (u8)(u0 ^ r->col[g])) { xgood = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: xor pixel %02x, expected %02x (bg %02x)\n", r->stamp, g, r->ch[g], xx, yy, v, u0 ^ r->col[g], u); }
                /* an option line after its fade: the card code recoloured the xor pixels
                   0x20..0x7f to the line's colour; xor pixels outside that range (over
                   some picture colours) kept theirs, holes the original shows too */
                if (good && r->xr && v == (u8)(u0 ^ r->col[g]) && (v < 0x20 || v >= 0x80)) { }
                else if (good) { if (c < 0) c = v; else if (v != c) { good = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: glyph pixel %02x vs %02x (under %02x)\n", r->stamp, g, r->ch[g], xx, yy, v, c, u); } }
            } else if (v != u) { good = xgood = 0; if (dbg && r->xr) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: background %02x was %02x\n", r->stamp, g, r->ch[g], xx, yy, v, u); break; }
        }
    q->good = good; q->xgood = xgood; q->any = any; q->seen = seen; q->c = c; q->sr = sr; q->sg = sg; q->sb = sb; q->sn = sn;
}

HdRun hdtext_runs[HD_MAXRUNS];
int hdtext_nruns;
int hdtext_dump;                             /* Ctrl+F12: the next frame's verdicts into the log */

/* called with the frame about to be shown: wipe the glyphs that are still on screen from the
 * RGB picture and list the runs of text to draw again */
static void prepare_runs(u32 *rgb, const u32 *lut, u8 *idx);
static void merge_runs(void);
void hdtext_prepare(u32 *rgb, const u32 *lut, u8 *idx) { prepare_runs(rgb, lut, idx); merge_runs(); }
static void prepare_runs(u32 *rgb, const u32 *lut, u8 *idx)
{
    hdtext_nruns = 0;
    cursor_read(lut);
    pointer_out(rgb, lut, idx);
    if (!hdtext_mode) return;
    const u8 *vga = clean;                       /* the screen without the pointer */
    static u8 claimed[320 * 200];
    memset(claimed, 0, sizeof claimed);
    /* newest first */
    static int order[MAXREC]; int no = 0;
    for (int i = 0; i < MAXREC; i++) if (rec[i].used && rec[i].page == 0) order[no++] = i;
    for (int a = 1; a < no; a++) { int t = order[a], b = a; while (b > 0 && rec[order[b - 1]].stamp < rec[t].stamp) { order[b] = order[b - 1]; b--; } order[b] = t; }
    static int dbgn = -1; if (dbgn < 0) dbgn = getenv("DL_HD_LOG") ? 1 : 0;
    int dbg = 0, dump = hdtext_dump; hdtext_dump = 0;
    if (dump) {
        int cx, cy, cw, ch; cursor_rect(&cx, &cy, &cw, &ch);
        fprintf(stderr, "text dump: %d records on the screen, crisp text %d, the game's pointer %s at %d,%d %dx%d (sprite at %d,%d), drawn by the interface %d\n",
                no, hdtext_mode, cw ? "shown" : "hidden", cx, cy, cw, ch, cur_x, cur_y, hdtext_live_pointer);
    }
    for (int oi = 0; oi < no; oi++) {
        Rec *r = &rec[order[oi]];
        if (r->is_shadow) continue;               /* goes with the text it shadows */
        dbg = (dbgn && oi < 4) || dump;
        { static const char *flt = (const char *)-1; if (flt == (const char *)-1) flt = getenv("DL_HD_DEBUG");
          if (flt) { char tx[MAXG + 1]; int k; for (k = 0; k < r->n; k++) tx[k] = (char)r->ch[k]; tx[k] = 0; dbg = strstr(tx, flt) != 0; } }
        const Fnt *F = &fnt[r->font - 1];
        int ok[MAXG], gc[MAXG], alive = 0;
        /* xor glyphs (gc = -1): their colour is the mean of the glyph pixels that look different
         * from the picture, so a line shows whole and in its colour while the game fades the
         * list in and where the palette leaves some of its pixels invisible */
        u32 xs_r[MAXG], xs_g[MAXG], xs_b[MAXG], xs_n[MAXG];
        int gb0[MAXG], gb1[MAXG];                  /* the rows of each glyph still to be seen */
        for (int g = 0; g < r->n; g++) {
            ok[g] = 0; xs_n[g] = 0; gb0[g] = 0; gb1[g] = r->h;
            Chk q; glyph_check(r, F, g, 0, r->h, vga, claimed, lut, dbg, &q);
            if (!q.good && !q.xgood) {
                /* covered in part from above or below (a menu's edge, a message box): the rows
                   that still check out, if they are most of the glyph, and the text is drawn
                   there only */
                int rowok[64], h = r->h < 64 ? r->h : 64;
                for (int yy = 0; yy < h; yy++) { Chk t; glyph_check(r, F, g, yy, yy + 1, vga, claimed, lut, 0, &t); rowok[yy] = t.good || t.xgood; }
                int a = 0; while (a < h && rowok[a]) a++;
                int b = h; while (b > 0 && rowok[b - 1]) b--;
                int lo = 0, hi = 0;
                if (a >= h - b) { lo = 0; hi = a; } else { lo = b; hi = h; }
                if (dbg) { fprintf(stderr, "hd: rec %u glyph %d '%c' rows ", r->stamp, g, r->ch[g]); for (int yy = 0; yy < h; yy++) fputc(rowok[yy] ? '1' : '0', stderr); fprintf(stderr, " band %d-%d\n", lo, hi); }
                if (hi - lo >= (h + 1) / 2 && hi - lo < h) {
                    Chk t; glyph_check(r, F, g, lo, hi, vga, claimed, lut, 0, &t);
                    if (t.good || t.xgood) { q = t; gb0[g] = lo; gb1[g] = hi; }
                }
            }
            if (!q.good && !q.xgood) continue;
            if (!q.any) { ok[g] = 2; continue; }               /* a space: kept with its neighbours */
            if (!q.seen && !q.xgood) continue;                 /* the glyph cannot be told from what was under it: gone */
            /* (an xor glyph that looks like the picture right now still belongs to its line: the
               line is drawn in the colour of its visible glyphs, or not at all while none is) */
            if (q.xgood) { ok[g] = 1; gc[g] = -1; xs_r[g] = q.sr; xs_g[g] = q.sg; xs_b[g] = q.sb; xs_n[g] = q.sn; }   /* xor first: one colour for the line */
            else if (q.good && q.seen && q.c >= 0) { ok[g] = 1; gc[g] = q.c; }
            else continue;
            alive++;
        }
        { static int lg = -1; static u32 lastt; u32 plat_ticks_ms(void); if (lg < 0) lg = getenv("DL_HD_LOG2") != 0;
          if (dump || (lg && plat_ticks_ms() - lastt > 300)) {
              char st[MAXG + 1]; int k; for (k = 0; k < r->n; k++) st[k] = ok[k] == 1 ? (gc[k] < 0 ? 'x' : 'o') : ok[k] == 2 ? '_' : ok[k] >= 4 ? 'c' : '.'; st[k] = 0;
              char tx[MAXG + 1]; for (k = 0; k < r->n; k++) tx[k] = (char)r->ch[k]; tx[k] = 0;
              fprintf(stderr, "hd2 %u: rec %u at %d,%d xr %d: %s\n          %s\n", plat_ticks_ms(), r->stamp, r->x0, r->y0, r->xr, tx, st);
              if (oi == no - 1) lastt = plat_ticks_ms();
          } }
        if (!alive) { if (stamp - r->stamp > 4000) { r->used = 0; free(r->under); r->under = 0; free(r->bg); r->bg = 0; } continue; }
        /* wipe the verified glyphs and group them into runs of one colour */
        Rec *S = shadow_of(r);
        for (int g = 0; g < r->n; g++) {
            if (ok[g] != 1) continue;
            int gx = r->x0 + r->gx[g];
            for (int yy = gb0[g]; yy < gb1[g]; yy++)
                for (int xx = 0; xx < r->gw[g]; xx++) {
                    int p = (r->y0 + yy) * 320 + gx + xx;
                    claimed[p] = 1;
                    if (on_cursor(gx + xx, r->y0 + yy)) continue;   /* the pointer's own pixels stay */
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
                        if (vga[p] != S->col[k] || on_cursor(x, y)) continue;
                        int mx = x - r->x0, my = y - r->y0;
                        int g = -1;
                        for (int q = 0; q < r->n; q++) if (mx >= r->gx[q] && mx < r->gx[q] + r->gw[q]) { g = q; break; }
                        if (g >= 0 && my >= 0 && my < r->h && ok[g] != 1) continue;   /* beside a glyph that stays */
                        if (g >= 0 && my >= 0 && my < r->h && glyph_bit(F, r->ch[g], mx - r->gx[g], my)) continue;
                        u8 u = S->bg[yy * S->w + S->gx[k] + xx];
                        rgb[p] = lut[u]; if (idx) idx[p] = u; claimed[p] = 1;
                    }
        }
        int g = 0, last_e = -2;
        while (g < r->n) {
            if (ok[g] != 1) { g++; continue; }
            int s = g, c = gc[g], e = g, b0 = gb0[g], b1 = gb1[g];
            u32 tr = 0, tg = 0, tb = 0, tn = 0;
            while (g < r->n && (ok[g] == 2 || (ok[g] == 1 && gc[g] == c && gb0[g] == b0 && gb1[g] == b1))) {
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
            u->y = r->y0; u->h = r->h; u->font = r->font; u->center = 0;
            u->clip0 = b0; u->clip1 = b1;
            u->cont = s == last_e + 1; last_e = e;     /* the same line going on in another colour or band */
            u->rgb = rgb;
            {   u32 br = 0, bgg = 0, bb = 0, bn = 0, same = 0; u8 first = r->bg[r->gx[s]];
                for (int yy = 0; yy < r->h; yy++)
                    for (int xx = r->gx[s]; xx < r->gx[g - 1] + r->gw[g - 1]; xx++) {
                        u8 q = r->bg[yy * r->w + xx]; u32 v = lut[q]; br += v >> 16 & 255; bgg += v >> 8 & 255; bb += v & 255; bn++; same += q == first; }
                u->bgrgb = bn ? (br / bn) << 16 | (bgg / bn) << 8 | (bb / bn) : 0;
                u->flatbg = bn && same * 10 >= bn * 9; }     /* a plain box behind it, not a picture */
        }
    }
}

/* pieces of one line drawn separately side by side (the party panel's endurance, whose digits
 * the game draws one at a time when it changes: "3" "7" became "3 7" in the narrower crisp
 * type): one run, so the line is set as a whole */
static void merge_runs(void)
{
    for (int again = 1; again; ) {
        again = 0;
        for (int i = 0; i < hdtext_nruns; i++) {
            HdRun *a = &hdtext_runs[i];
            if (!a->text[0] || a->center) continue;
            const Fnt *F = a->font >= 1 && a->font <= nfnt ? &fnt[a->font - 1] : 0;
            int gap = F ? F->cspace : 1;
            for (int j = 0; j < hdtext_nruns; j++) {
                HdRun *b = &hdtext_runs[j];
                if (j == i || !b->text[0] || b->center || b->y != a->y || b->h != a->h || b->font != a->font || b->rgb != a->rgb
                    || b->clip0 != a->clip0 || b->clip1 != a->clip1) continue;
                int d = b->x - (a->x + a->w);
                if (d < 0 || d > gap + 1) continue;
                size_t la = strlen(a->text), lb = strlen(b->text);
                if (la + lb + 1 > sizeof a->text) continue;
                memcpy(a->text + la, b->text, lb + 1);
                a->w = b->x + b->w - a->x;
                b->text[0] = 0;
                again = 1;
            }
        }
    }
    int n = 0;
    for (int i = 0; i < hdtext_nruns; i++) if (hdtext_runs[i].text[0]) hdtext_runs[n++] = hdtext_runs[i];
    hdtext_nruns = n;
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
