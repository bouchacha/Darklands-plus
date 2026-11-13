/* keep.c - the player's marks, for each game (party seed) in darklands_keep_<seed>.txt beside the
 * journal (dlp_data_dir):
 *   - things marked "keep": never sold (one by one, Sell spare gear) or dropped by the screens of
 *     Darklands+, until unmarked; or "sell": sold with Sell spare gear though they are no weapon,
 *     armor or shield (the potions the player never wants). Marked by kind and quality (identical
 *     things share one record in the game): "item quality", "item quality sell";
 *   - the alchemy screen's potions starred (first in its list) or put aside (last, still there):
 *     "potion k first", "potion k last" (until 1.4.6-RC12 the list had a row a formula, marked as
 *     "formula f first|last": read as its potion's mark, a star before a mark aside);
 *   - an ingredient's stock to keep (his ask, 1.4.7): in a town that sells it, a notice while the
 *     party has fewer: "stock item n". */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "keep.h"

extern char dlp_data_dir[];
#define NK 256
static struct { short id, q, mark; } kept[NK];          /* mark: 1 keep, 2 sell */
static int nkept, kseed = -1;
static unsigned char pmark[22];                           /* a potion's: 0 none, 1 first, 2 last */
static unsigned char stock[23];                           /* an ingredient's (item 72 + i) stock to keep, 0 none */

static void path(char *b, size_t n, int seed) { snprintf(b, n, "%s/darklands_keep_%u.txt", dlp_data_dir, (unsigned)(u16)seed); }

static void load(int seed)
{
    nkept = 0; kseed = seed; memset(pmark, 0, sizeof pmark); memset(stock, 0, sizeof stock);
    char p[1200]; path(p, sizeof p, seed);
    FILE *f = fopen(p, "r");
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof line, f) && nkept < NK) {
        int id, q; char w[16] = "";
        if (sscanf(line, "stock %d %d", &id, &q) == 2) { if (id >= 72 && id < 95) stock[id - 72] = (unsigned char)(q < 0 ? 0 : q > 255 ? 255 : q); continue; }
        if (sscanf(line, "potion %d %15s", &id, w) == 2) { if (id >= 0 && id < 22) pmark[id] = !strcmp(w, "first") ? 1 : !strcmp(w, "last") ? 2 : 0; continue; }
        if (sscanf(line, "formula %d %15s", &id, w) == 2) {          /* (1.4.6-RC12 and before) */
            int m = !strcmp(w, "first") ? 1 : !strcmp(w, "last") ? 2 : 0;
            if (id >= 0 && id < 66 && m && pmark[id / 3] != 1) pmark[id / 3] = (unsigned char)m;
            continue;
        }
        if (line[0] == '#' || sscanf(line, "%d %d %15s", &id, &q, w) < 2) continue;
        kept[nkept].id = (short)id; kept[nkept].q = (short)q; kept[nkept].mark = strcmp(w, "sell") ? 1 : 2; nkept++;
    }
    fclose(f);
}
static void save(void)
{
    char p[1200]; path(p, sizeof p, kseed);
    int nf = 0; for (int i = 0; i < 22; i++) nf += pmark[i] != 0;
    for (int i = 0; i < 23; i++) nf += stock[i] != 0;
    if (!nkept && !nf) { remove(p); return; }
    FILE *f = fopen(p, "w");
    if (!f) return;
    fprintf(f, "# Darklands+: things kept from selling and dropping (item, quality), or marked to sell (item, quality, sell);\n"
               "# potions first or last in the alchemy screen's list (potion k first|last); an ingredient's stock to keep (stock item n)\n");
    for (int k = 0; k < nkept; k++) fprintf(f, "%d %d%s\n", kept[k].id, kept[k].q, kept[k].mark == 2 ? " sell" : "");
    for (int i = 0; i < 22; i++) if (pmark[i]) fprintf(f, "potion %d %s\n", i, pmark[i] == 1 ? "first" : "last");
    for (int i = 0; i < 23; i++) if (stock[i]) fprintf(f, "stock %d %d\n", 72 + i, stock[i]);
    fclose(f);
}
static void sync(void) { int s = (s16)g_rw(G_PARTY_SEED); if (s != kseed) load(s); }

static int mark_of(int id, int q)
{
    sync();
    for (int k = 0; k < nkept; k++) if (kept[k].id == id && kept[k].q == q) return kept[k].mark;
    return 0;
}
int keep_is(int id, int q) { return mark_of(id, q) == 1; }
int sell_is(int id, int q) { return mark_of(id, q) == 2; }

/* the mark of (id, q): 0 none, 1 keep, 2 sell */
void keep_mark(int id, int q, int mark)
{
    sync();
    for (int k = 0; k < nkept; k++)
        if (kept[k].id == id && kept[k].q == q) {
            if (!mark) kept[k] = kept[--nkept]; else kept[k].mark = (short)mark;
            save();
            return;
        }
    if (mark && nkept < NK) { kept[nkept].id = (short)id; kept[nkept].q = (short)q; kept[nkept].mark = (short)mark; nkept++; save(); }
}
void keep_set(int id, int q, int on) { if (on) keep_mark(id, q, 1); else if (keep_is(id, q)) keep_mark(id, q, 0); }

/* a potion's place in the alchemy screen's list: 0 as the game has it, 1 first (starred), 2 last (put aside) */
int potion_mark(int k) { sync(); return k >= 0 && k < 22 ? pmark[k] : 0; }
void potion_mark_set(int k, int mark) { sync(); if (k < 0 || k >= 22 || pmark[k] == mark) return; pmark[k] = (unsigned char)mark; save(); }

/* an ingredient's stock to keep (0 none); a click steps it: none, 5, 10, 20, 40, none */
int stock_want(int id) { sync(); return id >= 72 && id < 95 ? stock[id - 72] : 0; }
void stock_want_set(int id, int n) { sync(); if (id < 72 || id >= 95 || stock[id - 72] == n) return; stock[id - 72] = (unsigned char)(n < 0 ? 0 : n > 255 ? 255 : n); save(); }
int stock_next(int n) { return n <= 0 ? 20 : n < 50 ? 50 : n < 100 ? 100 : n < 200 ? 200 : 0; }   /* (his steps, 1.4.7) */
