/* keep.c - things the player marks "keep": never sold (one by one, Sell spare gear) or dropped
 * by the screens of Darklands+, until unmarked. Kept by kind and quality (identical things share
 * one record in the game), for each game (party seed) in darklands_keep_<seed>.txt beside the
 * journal (dlp_data_dir). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "keep.h"

extern char dlp_data_dir[];
#define NK 256
static struct { short id, q; } kept[NK];
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
        int id, q;
        if (line[0] == '#' || sscanf(line, "%d %d", &id, &q) != 2) continue;
        kept[nkept].id = (short)id; kept[nkept].q = (short)q; nkept++;
    }
    fclose(f);
}
static void save(void)
{
    char p[1200]; path(p, sizeof p, kseed);
    if (!nkept) { remove(p); return; }
    FILE *f = fopen(p, "w");
    if (!f) return;
    fprintf(f, "# Darklands+: things kept from selling and dropping (item, quality)\n");
    for (int k = 0; k < nkept; k++) fprintf(f, "%d %d\n", kept[k].id, kept[k].q);
    fclose(f);
}
static void sync(void) { int s = (s16)g_rw(G_PARTY_SEED); if (s != kseed) load(s); }

int keep_is(int id, int q)
{
    sync();
    for (int k = 0; k < nkept; k++) if (kept[k].id == id && kept[k].q == q) return 1;
    return 0;
}

void keep_set(int id, int q, int on)
{
    sync();
    for (int k = 0; k < nkept; k++)
        if (kept[k].id == id && kept[k].q == q) {
            if (!on) { kept[k] = kept[--nkept]; save(); }
            return;
        }
    if (on && nkept < NK) { kept[nkept].id = (short)id; kept[nkept].q = (short)q; nkept++; save(); }
}
