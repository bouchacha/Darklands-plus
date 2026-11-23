/* config.c - settings kept in darklands_port.ini next to the game (key = value lines) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#ifdef _WIN32
#include <windows.h>
#endif

/* (keys hold a mod's folder name - mods.<name> - and values a path: until 1.5.1-RC21 a key of 64 characters or more was
   kept cut and never found again, so such a mod went off at every start, and every set added it once more: the audit,
   2026-11-22) */
typedef struct { char key[200]; char val[520]; } Entry;
#define MAXE 1024
static Entry entries[MAXE];
static int n;
static char path[1100];

/* an index by key (the interface asks for settings some thousands of times a frame, most of them not in
   the file - the default - which a search along the list answered only after comparing every key) */
#define HSIZE 4096                                  /* (a power of two, well above MAXE) */
static short hidx[HSIZE];                           /* entry + 1; 0 empty */
static unsigned hkey(const char *k) { unsigned h = 2166136261u; while (*k) h = (h ^ (unsigned char)*k++) * 16777619u; return h; }
static void hput(int i) { unsigned h = hkey(entries[i].key) & (HSIZE - 1); while (hidx[h]) h = (h + 1) & (HSIZE - 1); hidx[h] = (short)(i + 1); }
static void hbuild(void) { memset(hidx, 0, sizeof hidx); for (int i = 0; i < n; i++) hput(i); }
static int hfind(const char *key)
{
    for (unsigned h = hkey(key) & (HSIZE - 1); hidx[h]; h = (h + 1) & (HSIZE - 1))
        if (!strcmp(entries[hidx[h] - 1].key, key)) return hidx[h] - 1;
    return -1;
}

void config_load(const char *dir)
{
    snprintf(path, sizeof path, "%s/darklands_port.ini", dir);
    FILE *f = fopen(path, "r");
    n = 0; memset(hidx, 0, sizeof hidx);
    if (!f) return;
    char line[800];
    while (fgets(line, sizeof line, f) && n < MAXE) {
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#' || line[0] == ';') continue;
        *eq = 0;
        char *k = line, *v = eq + 1;
        while (*k == ' ') k++;
        for (char *e = k + strlen(k); e > k && (e[-1] == ' ' || e[-1] == '\t'); ) *--e = 0;
        while (*v == ' ') v++;
        for (char *e = v + strlen(v); e > v && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' '); ) *--e = 0;
        snprintf(entries[n].key, sizeof entries[n].key, "%s", k);
        snprintf(entries[n].val, sizeof entries[n].val, "%s", v);
        if (hfind(entries[n].key) >= 0) continue;   /* (a key twice in the file: the first stands, as always) */
        hput(n);
        n++;
    }
    fclose(f);
}

/* written whole beside it and then put in its place, so a crash or a full disk mid-write leaves the old settings, not
   an empty file (every setting back at its default at the next start) */
void config_save(void)
{
    if (!path[0]) return;
    char tmp[1120]; snprintf(tmp, sizeof tmp, "%s.new", path);
    FILE *f = fopen(tmp, "w");
    if (!f) return;
    int ok = fprintf(f, "# Darklands native port settings (written by the game; edit while it is not running)\n") > 0;
    for (int i = 0; i < n && ok; i++) ok = fprintf(f, "%s = %s\n", entries[i].key, entries[i].val) > 0;
    if (fclose(f)) ok = 0;
    if (!ok) { remove(tmp); fprintf(stderr, "config: the settings could not be written\n"); return; }
#ifdef _WIN32
    if (!MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) { remove(tmp); fprintf(stderr, "config: the settings could not be replaced\n"); }
#else
    if (rename(tmp, path)) { remove(tmp); fprintf(stderr, "config: the settings could not be replaced\n"); }
#endif
}

const char *config_get(const char *key, const char *def)
{
    int i = hfind(key);
    return i >= 0 ? entries[i].val : def;
}

int config_get_int(const char *key, int def)
{
    const char *v = config_get(key, 0);
    return v ? atoi(v) : def;
}

void config_set(const char *key, const char *val)
{
    int i = hfind(key);
    if (i >= 0) { snprintf(entries[i].val, sizeof entries[i].val, "%s", val); return; }
    if (n < MAXE) { snprintf(entries[n].key, sizeof entries[n].key, "%s", key); snprintf(entries[n].val, sizeof entries[n].val, "%s", val); hput(n); n++; }
}

void config_unset(const char *key)
{
    int i = hfind(key);
    if (i >= 0) { entries[i] = entries[--n]; hbuild(); }
}
int config_count_prefix(const char *prefix, void (*each)(const char *key, const char *val))
{
    size_t l = strlen(prefix); int c = 0;
    for (int i = 0; i < n; i++) if (!strncmp(entries[i].key, prefix, l)) { if (each) each(entries[i].key, entries[i].val); c++; }
    return c;
}

void config_set_int(const char *key, int val)
{
    char b[32]; snprintf(b, sizeof b, "%d", val); config_set(key, b);
}
