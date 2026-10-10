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

int hdtext_mode = 1;                 /* 0 off, 1 card font, 2 all fonts */

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
#define MAXREC 256
#define MAXG 96
typedef struct {
    int used, font, x0, y0, w, h;      /* box on screen */
    u32 stamp;
    int n;                             /* glyphs */
    u8 ch[MAXG]; short gx[MAXG]; u8 gw[MAXG]; u8 col[MAXG];
    u8 *under;                         /* the picture under the box before the text */
} Rec;
static Rec rec[MAXREC];
static u32 stamp;

static u8 *page0(void) { return M + 0xa0000; }

static void drop_covered(int x0, int y0, int w, int h)
{
    for (int i = 0; i < MAXREC; i++) {
        Rec *r = &rec[i];
        if (r->used && r->x0 >= x0 && r->y0 >= y0 && r->x0 + r->w <= x0 + w && r->y0 + r->h <= y0 + h) { r->used = 0; free(r->under); r->under = 0; }
    }
}

static void text_hook(void)                 /* gfx_text(win, x, y, char *s) */
{
    if (!hdtext_mode || !nfnt) { hook_call_original(); return; }
    u16 w = rw(SS, (u16)(SP + 4)); int x = (s16)rw(SS, (u16)(SP + 6)), y = (s16)rw(SS, (u16)(SP + 8)); u16 sp = rw(SS, (u16)(SP + 10));
    int page = (s16)g_rw(w), font = (s16)g_rw((u16)(w + 0x10));
    if (page != 0 || font < 1 || font > nfnt || (hdtext_mode == 1 && font != 3)) { hook_call_original(); return; }
    const Fnt *F = &fnt[font - 1];
    int sx = (s16)g_rw((u16)(w + 2)) + x, sy = (s16)g_rw((u16)(w + 4)) + y;
    int col = g_rb((u16)(w + 0x0c));
    Rec r; memset(&r, 0, sizeof r);
    r.font = font; r.x0 = sx; r.y0 = sy; r.h = F->height;
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
    r.under = malloc((size_t)r.w * r.h);
    for (int yy = 0; yy < r.h; yy++) memcpy(r.under + yy * r.w, page0() + (sy + yy) * 320 + sx, (size_t)r.w);
    hook_call_original();
    drop_covered(r.x0, r.y0, r.w, r.h);
    int slot = 0; u32 oldest = 0xffffffffu;
    for (int i = 0; i < MAXREC; i++) { if (!rec[i].used) { slot = i; break; } if (rec[i].stamp < oldest) { oldest = rec[i].stamp; slot = i; } }
    free(rec[slot].under);
    r.used = 1; r.stamp = ++stamp;
    rec[slot] = r;
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
void hdtext_prepare(u32 *rgb, const u32 *lut)
{
    hdtext_nruns = 0;
    if (!hdtext_mode) return;
    const u8 *vga = page0();
    static u8 claimed[320 * 200];
    memset(claimed, 0, sizeof claimed);
    int mx, my, mw, mh; cursor_rect(&mx, &my, &mw, &mh);
    /* newest first */
    static int order[MAXREC]; int no = 0;
    for (int i = 0; i < MAXREC; i++) if (rec[i].used) order[no++] = i;
    for (int a = 1; a < no; a++) { int t = order[a], b = a; while (b > 0 && rec[order[b - 1]].stamp < rec[t].stamp) { order[b] = order[b - 1]; b--; } order[b] = t; }
    static int dbgn = -1; if (dbgn < 0) dbgn = getenv("DL_HD_LOG") ? 1 : 0;
    int dbg = 0;
    for (int oi = 0; oi < no; oi++) {
        Rec *r = &rec[order[oi]];
        dbg = dbgn && oi < 4;
        const Fnt *F = &fnt[r->font - 1];
        int ok[MAXG], gc[MAXG], alive = 0;
        for (int g = 0; g < r->n; g++) {
            ok[g] = 0;
            int gx = r->x0 + r->gx[g], gw = r->gw[g];
            if (mw && gx < mx + mw && gx + gw > mx && r->y0 < my + mh && r->y0 + r->h > my) { ok[g] = 4; continue; }   /* under the cursor: decided below */
            int c = -1, good = 1, any = 0;
            for (int yy = 0; yy < r->h && good; yy++)
                for (int xx = 0; xx < gw; xx++) {
                    int p = (r->y0 + yy) * 320 + gx + xx;
                    if (claimed[p]) { good = 0; break; }
                    u8 v = vga[p], u = r->under[yy * r->w + r->gx[g] + xx];
                    if (glyph_bit(F, r->ch[g], xx, yy)) { if (c < 0) c = v; else if (v != c) { good = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: glyph pixel %02x vs %02x (under %02x)\n", r->stamp, g, r->ch[g], xx, yy, v, c, u); break; } any = 1; }
                    else if (v != u) { good = 0; if (dbg) fprintf(stderr, "hd: rec %u glyph %d '%c' at %d,%d: background %02x was %02x\n", r->stamp, g, r->ch[g], xx, yy, v, u); break; }
                }
            if (!good) continue;
            if (!any) { ok[g] = 2; continue; }                 /* a space: kept with its neighbours */
            ok[g] = 1; gc[g] = c; alive++;
        }
        if (!alive) { if (stamp - r->stamp > 4000) { r->used = 0; free(r->under); r->under = 0; } continue; }
        /* glyphs under the mouse cursor: the line is still there, so they are too, in the
         * colour of their neighbours (only the pixels not covered by the cursor are wiped) */
        for (int g = 0; g < r->n; g++) {
            if (ok[g] != 4) continue;
            int c = -1;
            for (int k = g - 1; k >= 0 && c < 0; k--) if (ok[k] == 1) c = gc[k];
            for (int k = g + 1; k < r->n && c < 0; k++) if (ok[k] == 1) c = gc[k];
            if (c < 0) { ok[g] = 0; continue; }
            gc[g] = c; ok[g] = glyph_w(F, r->ch[g]) && r->ch[g] != ' ' ? 5 : 6;
        }
        /* wipe the verified glyphs and group them into runs of one colour */
        for (int g = 0; g < r->n; g++) {
            int under_cursor = ok[g] & 4;
            ok[g] &= 3;
            if (ok[g] != 1) continue;
            int gx = r->x0 + r->gx[g];
            for (int yy = 0; yy < r->h; yy++)
                for (int xx = 0; xx < r->gw[g]; xx++) {
                    int p = (r->y0 + yy) * 320 + gx + xx;
                    if (under_cursor && vga[p] != gc[g]) continue;           /* the cursor's own pixels stay */
                    claimed[p] = 1;
                    if (glyph_bit(F, r->ch[g], xx, yy)) rgb[p] = lut[r->under[yy * r->w + r->gx[g] + xx]];
                }
        }
        int g = 0;
        while (g < r->n) {
            if (ok[g] != 1) { g++; continue; }
            int s = g, c = gc[g], e = g;
            while (g < r->n && (ok[g] == 2 || (ok[g] == 1 && gc[g] == c))) { if (ok[g] == 1) e = g; g++; }
            g = e + 1;
            if (hdtext_nruns >= HD_MAXRUNS) return;
            HdRun *u = &hdtext_runs[hdtext_nruns++];
            int n = 0;
            for (int k = s; k < g && n < (int)sizeof u->text - 1; k++) u->text[n++] = (char)r->ch[k];
            u->text[n] = 0;
            u->x = r->x0 + r->gx[s];
            u->w = r->gx[g - 1] + r->gw[g - 1] - r->gx[s];
            u->y = r->y0; u->h = r->h; u->font = r->font;
            u->rgb = lut[c];
        }
    }
}

void hdtext_set(int mode) { hdtext_mode = mode; config_set_int("display.crisp_text", mode); }

void hdtext_init(void)
{
    load_fonts();
    hdtext_mode = config_get_int("display.crisp_text", 1);
    hook_install(-1, 0x047c, 0x055a, text_hook);
}
