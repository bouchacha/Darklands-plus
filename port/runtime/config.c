/* config.c - settings kept in darklands_port.ini next to the game (key = value lines) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

typedef struct { char key[64]; char val[192]; } Entry;
static Entry entries[256];
static int n;
static char path[1100];

void config_load(const char *dir)
{
    snprintf(path, sizeof path, "%s/darklands_port.ini", dir);
    FILE *f = fopen(path, "r");
    n = 0;
    if (!f) return;
    char line[300];
    while (fgets(line, sizeof line, f) && n < 256) {
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
        n++;
    }
    fclose(f);
}

void config_save(void)
{
    if (!path[0]) return;
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "# Darklands native port settings (written by the game; edit while it is not running)\n");
    for (int i = 0; i < n; i++) fprintf(f, "%s = %s\n", entries[i].key, entries[i].val);
    fclose(f);
}

const char *config_get(const char *key, const char *def)
{
    for (int i = 0; i < n; i++) if (!strcmp(entries[i].key, key)) return entries[i].val;
    return def;
}

int config_get_int(const char *key, int def)
{
    const char *v = config_get(key, 0);
    return v ? atoi(v) : def;
}

void config_set(const char *key, const char *val)
{
    for (int i = 0; i < n; i++) if (!strcmp(entries[i].key, key)) { snprintf(entries[i].val, sizeof entries[i].val, "%s", val); return; }
    if (n < 256) { snprintf(entries[n].key, sizeof entries[n].key, "%s", key); snprintf(entries[n].val, sizeof entries[n].val, "%s", val); n++; }
}

void config_set_int(const char *key, int val)
{
    char b[32]; snprintf(b, sizeof b, "%d", val); config_set(key, b);
}
