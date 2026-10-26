/* keep.c - things the player marks "keep": never sold (one by one, Sell spare gear) or dropped
 * by the screens of Darklands+, until unmarked; or "sell": sold with Sell spare gear though it is
 * no weapon, armor or shield (the potions the player never wants). Marked by kind and quality
 * (identical things share one record in the game), for each game (party seed) in
 * darklands_keep_<seed>.txt beside the journal (dlp_data_dir): "item quality", "item quality sell". */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "keep.h"

extern char dlp_data_dir[];
#define NK 256
static struct { short id, q, mark; } kept[NK];          /* mark: 1 keep, 2 sell */
static int nkept, kseed = -1;

static void path(char *b, size_t n, int seed) { snprintf(b, n, "%s/darklands_keep_%u.txt", dlp_data_dir, (unsigned)(u16)seed); }

static void load(int seed)
{
    nkept = 0; kseed = seed;
    char p[1200]; path(p, sizeof p, seed);
    FILE *f = fopen(p, "r");
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof line, f) && nkept < NK) {
        int id, q; char w[16] = "";
        if (line[0] == '#' || sscanf(line, "%d %d %15s", &id, &q, w) < 2) continue;
        kept[nkept].id = (short)id; kept[nkept].q = (short)q; kept[nkept].mark = strcmp(w, "sell") ? 1 : 2; nkept++;
    }
    fclose(f);
}
static void save(void)
{
    char p[1200]; path(p, sizeof p, kseed);
    if (!nkept) { remove(p); return; }
    FILE *f = fopen(p, "w");
    if (!f) return;
    fprintf(f, "# Darklands+: things kept from selling and dropping (item, quality), or marked to sell (item, quality, sell)\n");
    for (int k = 0; k < nkept; k++) fprintf(f, "%d %d%s\n", kept[k].id, kept[k].q, kept[k].mark == 2 ? " sell" : "");
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
