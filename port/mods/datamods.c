/* datamods.c - mod folders: game files replaced without touching the game's own.
 *
 * <game>/mods/<Mod name>/ holds files laid out like the game folder; while the mod is switched
 * on (F12 > Mods) the game reads them instead of its own:
 *   mods/My mod/DARKLAND.LST           the whole item list
 *   mods/My mod/PICS/CASTLE1.PIC       one picture
 *   mods/My mod/MSGFILES/$MILCR00.MSG  one file inside a catalogue (MSGFILES, *.CAT, BC, LCASTLE);
 *                                      new names are added to it
 *   mods/My mod/MSGFILES/$MILCR00.TXT  a card file as text (F12 > Mods > "Export the card texts")
 *   mods/My mod/art/, sounds/          replacement art and sounds, as in the game folder
 *   mods/My mod/music/                 recordings in place of the game's music (musicpack.c)
 *   mods/My mod/mod.txt                first line: the name shown; the rest: a description
 * Mods are applied in the order of the list in the settings (dragged there; at first alphabetical,
 * a mod new to the list at its end): where two change the same file, the later one wins. Only reading is redirected: saves and settings stay in the game folder. */
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
#include "../runtime/config.h"
#include "datamods.h"

extern char game_dir[1024];

DataMod datamods[DATAMODS_MAX];
int datamods_n;
static int generation = 1;          /* changes whenever the set of mods in use changes */

/* ---------------------------------------------------------------- paths */
static int is_dir(const char *p) { struct stat st; return !stat(p, &st) && S_ISDIR(st.st_mode); }
static int is_file(const char *p) { struct stat st; return !stat(p, &st) && !S_ISDIR(st.st_mode); }

/* dir/name with name matched without regard to case; 1 if it exists */
static int find_ci(const char *dir, const char *name, char *out, size_t outn)
{
    DIR *d = opendir(dir);
    int found = 0;
    if (d) {
        struct dirent *e;
        while ((e = readdir(d))) if (!strcasecmp(e->d_name, name)) { snprintf(out, outn, "%s/%s", dir, e->d_name); found = 1; break; }
        closedir(d);
    }
    if (!found) snprintf(out, outn, "%s/%s", dir, name);
    return found;
}
/* base/rel (rel with / or \ separators), each component without regard to case */
static int resolve_ci(const char *base, const char *rel, char *out, size_t outn)
{
    char tmp[1200], r[512];
    snprintf(tmp, sizeof tmp, "%s", base); snprintf(r, sizeof r, "%s", rel);
    for (char *q = r; *q; q++) if (*q == '\\') *q = '/';
    int ok = 1;
    for (char *tok = r; *tok;) {
        char *sl = strchr(tok, '/');
        if (sl) *sl = 0;
        if (*tok && strcmp(tok, ".")) { char nxt[1200]; if (!find_ci(tmp, tok, nxt, sizeof nxt)) ok = 0; snprintf(tmp, sizeof tmp, "%s", nxt); }
        if (!sl) break;
        tok = sl + 1;
    }
    snprintf(out, outn, "%s", tmp);
    return ok;
}
extern char dlp_user_dir[];
static void mods_dir(char *out, size_t n) { find_ci(dlp_user_dir[0] ? dlp_user_dir : game_dir, "mods", out, n); }
void datamods_folder(char *out, size_t n) { mods_dir(out, n); }

/* ---------------------------------------------------------------- the list */
/* its place in the order (datamod_order.<folder>), or past the end if never placed */
static int mod_place(const DataMod *m) { char key[200]; snprintf(key, sizeof key, "datamod_order.%s", m->folder); return config_get_int(key, 1 << 20); }
static int cmp_mod(const void *a, const void *b)
{
    int pa = mod_place(a), pb = mod_place(b);
    if (pa != pb) return pa < pb ? -1 : 1;
    return strcasecmp(((const DataMod *)a)->folder, ((const DataMod *)b)->folder);
}
static void mods_changed(void)
{
    generation++;
    void hdart_rescan(void); hdart_rescan();
    void rules_apply(void); rules_apply();
    void sfx_reload(void); sfx_reload();             /* a mod's sounds and music come and go at once */
    void musicpack_reload(void); musicpack_reload();
}
/* mod i moved to place j of the list (the later wins); the order kept */
void datamods_move(int i, int j)
{
    if (i < 0 || i >= datamods_n || j < 0 || j >= datamods_n || i == j) return;
    DataMod t = datamods[i];
    if (i < j) memmove(&datamods[i], &datamods[i + 1], (size_t)(j - i) * sizeof t);
    else memmove(&datamods[j + 1], &datamods[j], (size_t)(i - j) * sizeof t);
    datamods[j] = t;
    for (int k = 0; k < datamods_n; k++) { char key[200]; snprintf(key, sizeof key, "datamod_order.%s", datamods[k].folder); config_set_int(key, k); }
    mods_changed();
}

void datamods_scan(void)
{
    datamods_n = 0;
    /* the player's mods folder, and the game folder's when that is another (a game folder that
       cannot be written: mods put there by hand still count; a folder of the same name in the
       player's wins) */
    char roots[3][1100]; int nr = 0;
    mods_dir(roots[nr++], sizeof roots[0]);
    if (dlp_user_dir[0] && strcmp(dlp_user_dir, game_dir)) find_ci(game_dir, "mods", roots[nr++], sizeof roots[0]);
    {   /* and the mods that come with Darklands+ (in its own folder when that is not the game's) */
        extern char dlp_prog_dir[];
        char pm[1100];
        if (dlp_prog_dir[0] && find_ci(dlp_prog_dir, "mods", pm, sizeof pm)) {
            int dup = 0; for (int k = 0; k < nr; k++) if (!strcmp(roots[k], pm)) dup = 1;
            if (!dup) snprintf(roots[nr++], sizeof roots[0], "%s", pm);
        }
    }
    for (int ri = 0; ri < nr; ri++) {
    const char *md = roots[ri];
    DIR *d = opendir(md);
    if (!d) continue;
    struct dirent *e;
    while ((e = readdir(d)) && datamods_n < DATAMODS_MAX) {
        if (e->d_name[0] == '.' || e->d_name[0] == '_') continue;
        char p[1400]; snprintf(p, sizeof p, "%s/%s", md, e->d_name);
        if (!is_dir(p)) continue;
        { int dup = 0; for (int k = 0; k < datamods_n; k++) if (!strcasecmp(datamods[k].folder, e->d_name)) dup = 1; if (dup) continue; }
        DataMod *m = &datamods[datamods_n++];
        memset(m, 0, sizeof *m);
        snprintf(m->folder, sizeof m->folder, "%s", e->d_name);
        snprintf(m->path, sizeof m->path, "%s", p);
        snprintf(m->title, sizeof m->title, "%s", e->d_name);
        char t[1500];
        if (find_ci(p, "mod.txt", t, sizeof t)) {
            FILE *f = fopen(t, "rb");
            if (f) {
                char line[400]; int first = 1; size_t dn = 0;
                while (fgets(line, sizeof line, f)) {
                    size_t l = strlen(line); while (l && (line[l - 1] == '\n' || line[l - 1] == '\r')) line[--l] = 0;
                    if (first) { if (!l) continue; snprintf(m->title, sizeof m->title, "%s", line); first = 0; continue; }
                    if (dn + l + 2 < sizeof m->desc) { if (dn) m->desc[dn++] = '\n'; memcpy(m->desc + dn, line, l); dn += l; m->desc[dn] = 0; }
                }
                fclose(f);
            }
        }
        char key[200]; snprintf(key, sizeof key, "datamod.%s", m->folder);
        m->on = config_get_int(key, 0);
    }
    closedir(d);
    }
    qsort(datamods, (size_t)datamods_n, sizeof datamods[0], cmp_mod);
    generation++;
}

void datamods_set(int i, int on)
{
    if (i < 0 || i >= datamods_n) return;
    datamods[i].on = on;
    char key[200]; snprintf(key, sizeof key, "datamod.%s", datamods[i].folder);
    config_set_int(key, on);
    mods_changed();
}
int datamods_generation(void) { return generation; }
/* "Play the original game" (F12 > Settings) leaves the mods out too */
int datamods_enabled(void) { return !config_get_int("original", 0); }
int datamods_active(void) { if (!datamods_enabled()) return 0; for (int i = 0; i < datamods_n; i++) if (datamods[i].on) return 1; return 0; }

/* the art / sounds folders of the mods in use, the one that wins first (for hdart, sfx) */
int datamods_dirs(const char *sub, char out[][1200], int max)
{
    int n = 0;
    if (!datamods_enabled()) return 0;
    for (int i = datamods_n - 1; i >= 0 && n < max; i--) {
        if (!datamods[i].on) continue;
        char p[1200];
        if (find_ci(datamods[i].path, sub, p, sizeof p) && is_dir(p)) snprintf(out[n++], 1200, "%s", p);
    }
    return n;
}

/* ---------------------------------------------------------------- catalogues */
static const char *CATS[] = { "MSGFILES", "A00C.CAT", "C00C.CAT", "E00C.CAT", "F01C.CAT", "F60C.CAT", "M00C.CAT", "EINFO.CAT", "IMAPS.CAT", "BC", "LCASTLE" };

static unsigned char *read_all(const char *path, long *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *b = malloc((size_t)(len > 0 ? len : 1));
    if (b && len > 0 && fread(b, 1, (size_t)len, f) != (size_t)len) { free(b); b = 0; }
    fclose(f);
    *n = len;
    return b;
}

/* a card file as text (datamods_export_cards) back to the game's form */
static unsigned char *card_text_to_msg(const unsigned char *t, long n, long *outn)
{
    unsigned char *o = malloc((size_t)n + 16);
    long on = 1, i = 0; int cards = 0;
    o[0] = 0;
    int in_card = 0;
    while (i < n) {
        long e = i; while (e < n && t[e] != '\n') e++;
        long l = e; if (l > i && t[l - 1] == '\r') l--;
        if (l > i && t[i] == '#') { i = e + 1; continue; }        /* a comment ('#' in the text itself is written \23) */
        if (l - i >= 8 && !memcmp(t + i, "=== card", 8)) {
            if (in_card) { if (on > 0 && o[on - 1] == '\n') on--; o[on++] = 0; }   /* the text ended with the line before */
            int v[5] = { 0, 0, 0, 0, 0 }, k = 0;
            const unsigned char *p = memchr(t + i, ':', (size_t)(l - i));
            if (p) { p++; for (; k < 5 && p < t + l; k++) { while (p < t + l && *p == ' ') p++; v[k] = atoi((const char *)p); while (p < t + l && isdigit(*p)) p++; } }
            for (k = 0; k < 5; k++) o[on++] = (unsigned char)v[k];
            in_card = 1; cards++;
        } else if (in_card) {
            for (long k = i; k < l; k++) {
                if (t[k] == '\\' && k + 1 < l && t[k + 1] == '\\') { o[on++] = '\\'; k++; }
                else if (t[k] == '\\' && k + 2 < l && isxdigit(t[k + 1]) && isxdigit(t[k + 2])) { char h[3] = { (char)t[k + 1], (char)t[k + 2], 0 }; o[on++] = (unsigned char)strtol(h, 0, 16); k += 2; }
                else o[on++] = t[k];
            }
            o[on++] = '\n';
        }
        i = e + 1;
    }
    if (in_card) { if (on > 0 && o[on - 1] == '\n') on--; o[on++] = 0; }
    o[0] = (unsigned char)cards;
    *outn = on;
    return o;
}

typedef struct { char name[13]; unsigned ts; unsigned char *data; long len; int owned; } Ent;

/* the catalogue with the mods' files put in: a file in the cache, or 0 if no mod touches it */
static const char *merged_catalog(const char *base_path, const char *catname)
{
    char dirs[DATAMODS_MAX][1200]; int nd = 0;
    for (int i = 0; i < datamods_n; i++) {                       /* lowest first: later ones overwrite */
        if (!datamods[i].on) continue;
        char p[1200];
        if (find_ci(datamods[i].path, catname, p, sizeof p) && is_dir(p)) snprintf(dirs[nd++], 1200, "%s", p);
    }
    if (!nd) return 0;
    static char cached[16][1300]; static int cached_gen[16];
    int ci = 0; for (int k = 0; k < (int)(sizeof CATS / sizeof CATS[0]); k++) if (!strcasecmp(CATS[k], catname)) ci = k;
    if (cached_gen[ci] == generation && is_file(cached[ci])) return cached[ci];
    long n; unsigned char *b = read_all(base_path, &n);
    if (!b || n < 2) { free(b); return 0; }
    int cnt = b[0] | b[1] << 8;
    Ent *e = calloc((size_t)cnt + 512, sizeof *e); int ne = 0;
    for (int i = 0; i < cnt && 2 + (i + 1) * 24 <= n; i++) {
        const unsigned char *x = b + 2 + i * 24;
        Ent *t = &e[ne++];
        memcpy(t->name, x, 12); t->name[12] = 0;
        t->ts = x[12] | x[13] << 8 | x[14] << 16 | (unsigned)x[15] << 24;
        long len = x[16] | x[17] << 8 | x[18] << 16 | (long)x[19] << 24, off = x[20] | x[21] << 8 | x[22] << 16 | (long)x[23] << 24;
        if (off < 0 || len < 0 || off + len > n) { ne--; continue; }
        t->data = b + off; t->len = len;
    }
    for (int d = 0; d < nd; d++) {
        DIR *dd = opendir(dirs[d]);
        if (!dd) continue;
        struct dirent *de;
        while ((de = readdir(dd))) {
            if (de->d_name[0] == '.') continue;
            char p[1500]; snprintf(p, sizeof p, "%s/%s", dirs[d], de->d_name);
            if (!is_file(p)) continue;
            char nm[64]; snprintf(nm, sizeof nm, "%s", de->d_name);
            for (char *q = nm; *q; q++) *q = (char)toupper((unsigned char)*q);
            long len; unsigned char *data = read_all(p, &len);
            if (!data) continue;
            size_t L = strlen(nm);
            if (L > 4 && !strcmp(nm + L - 4, ".TXT")) {               /* card text: to the game's form, as NAME.MSG */
                if (!strcasecmp(catname, "MSGFILES")) { long on; unsigned char *m = card_text_to_msg(data, len, &on); free(data); data = m; len = on; strcpy(nm + L - 4, ".MSG"); }
                else { free(data); continue; }
            }
            if (strlen(nm) > 12) { free(data); continue; }
            int k; for (k = 0; k < ne; k++) if (!strcasecmp(e[k].name, nm)) break;
            if (k == ne) { if (ne >= cnt + 512) { free(data); continue; } memset(&e[ne], 0, sizeof e[ne]); snprintf(e[ne].name, sizeof e[ne].name, "%s", nm); ne++; }
            if (e[k].owned) free(e[k].data);
            e[k].data = data; e[k].len = len; e[k].owned = 1;
        }
        closedir(dd);
    }
    char md[1100], cache[1200]; mods_dir(md, sizeof md);
    snprintf(cache, sizeof cache, "%s/.cache", md); MKDIR(cache);
    snprintf(cached[ci], sizeof cached[ci], "%s/%s", cache, CATS[ci]);
    FILE *f = fopen(cached[ci], "wb");
    if (f) {
        unsigned char h[2] = { (unsigned char)ne, (unsigned char)(ne >> 8) }; fwrite(h, 1, 2, f);
        long off = 2 + 24L * ne;
        for (int k = 0; k < ne; k++) {
            unsigned char x[24]; memset(x, 0, sizeof x);
            memcpy(x, e[k].name, strlen(e[k].name) < 12 ? strlen(e[k].name) : 12);
            unsigned v[3] = { e[k].ts, (unsigned)e[k].len, (unsigned)off };
            for (int j = 0; j < 3; j++) { x[12 + 4 * j] = (unsigned char)v[j]; x[13 + 4 * j] = (unsigned char)(v[j] >> 8); x[14 + 4 * j] = (unsigned char)(v[j] >> 16); x[15 + 4 * j] = (unsigned char)(v[j] >> 24); }
            fwrite(x, 1, 24, f);
            off += e[k].len;
        }
        for (int k = 0; k < ne; k++) fwrite(e[k].data, 1, (size_t)e[k].len, f);
        fclose(f);
        cached_gen[ci] = generation;
    }
    for (int k = 0; k < ne; k++) if (e[k].owned) free(e[k].data);
    free(e); free(b);
    return cached_gen[ci] == generation ? cached[ci] : 0;
}

/* ---------------------------------------------------------------- the game's reads */
/* a file the game opens for reading (DOS name relative to the game folder, e.g. "PICS\X.PIC"):
 * the host path to read instead, or 0 to read the game's own */
const char *datamods_redirect(const char *rel, const char *game_path)
{
    if (!datamods_active()) return 0;
    static char out[1300];
    const char *r = rel;
    if (r[0] && r[1] == ':') r += 2;
    while (*r == '\\' || *r == '/') r++;
    const char *found = 0;
    for (int i = datamods_n - 1; i >= 0 && !found; i--) {
        if (!datamods[i].on) continue;
        char p[1300];
        if (resolve_ci(datamods[i].path, r, p, sizeof p) && is_file(p)) { snprintf(out, sizeof out, "%s", p); found = out; }
    }
    const char *b = r; for (const char *q = r; *q; q++) if (*q == '\\' || *q == '/') b = q + 1;
    for (int k = 0; k < (int)(sizeof CATS / sizeof CATS[0]); k++)
        if (!strcasecmp(b, CATS[k]) && b == r) {
            char base[1300]; snprintf(base, sizeof base, "%s", found ? found : game_path);
            const char *m = merged_catalog(base, CATS[k]);
            if (m) { snprintf(out, sizeof out, "%s", m); return out; }
        }
    return found;
}

/* ---------------------------------------------------------------- for modders */
/* every card file of MSGFILES as text, into mods/_export/MSGFILES/; returns the number written */
int datamods_export_cards(char *where, size_t wn)
{
    char src[1300]; resolve_ci(game_dir, "MSGFILES", src, sizeof src);
    long n; unsigned char *b = read_all(src, &n);
    if (!b || n < 2) { free(b); return -1; }
    char md[1100], dst[1300]; mods_dir(md, sizeof md); MKDIR(md);
    snprintf(dst, sizeof dst, "%s/_export", md); MKDIR(dst);
    snprintf(dst, sizeof dst, "%s/_export/MSGFILES", md); MKDIR(dst);
    snprintf(where, wn, "%s", dst);
    int cnt = b[0] | b[1] << 8, done = 0;
    for (int i = 0; i < cnt && 2 + (i + 1) * 24 <= n; i++) {
        const unsigned char *x = b + 2 + i * 24;
        char nm[13]; memcpy(nm, x, 12); nm[12] = 0;
        long len = x[16] | x[17] << 8 | x[18] << 16 | (long)x[19] << 24, off = x[20] | x[21] << 8 | x[22] << 16 | (long)x[23] << 24;
        if (off + len > n || len < 1) continue;
        char *dot = strrchr(nm, '.'); if (dot) *dot = 0;
        char p[1500]; snprintf(p, sizeof p, "%s/%s.TXT", dst, nm);
        FILE *f = fopen(p, "wb");
        if (!f) continue;
        const unsigned char *m = b + off, *end = m + len;
        int nb = *m++;
        fprintf(f, "# %s: %d card%s. Each starts with a line \"=== card N: y x 0 max_x 0 ===\" (text box top and left,\n"
                   "# right edge). Bytes the game uses for layout are written \\xx (hex): \\14 new paragraph, \\15 an option,\n"
                   "# \\1d after the option's \"...\", \\16 an option opening the saints, \\10 the potions, \\06 a battle,\n"
                   "# \\01 ... \\13 a part shown or hidden by the game. { is o-umlaut, | u-umlaut, \\1f a-umlaut.\n"
                   "# Put the edited file into mods/<your mod>/MSGFILES/ (keep the name); lines starting with # are ignored.\n",
                nm, nb, nb == 1 ? "" : "s");
        for (int c = 0; c < nb && m + 5 <= end; c++) {
            fprintf(f, "=== card %d: %d %d %d %d %d ===\n", c, m[0], m[1], m[2], m[3], m[4]);
            m += 5;
            while (m < end && *m) {
                unsigned char ch = *m++;
                if (ch == '\n') fputc('\n', f);
                else if (ch == '\\') fputs("\\\\", f);
                else if (ch < 0x20 || ch >= 0x7f || ch == '#') fprintf(f, "\\%02x", ch);
                else fputc(ch, f);
            }
            fputc('\n', f);
            if (m < end) m++;
        }
        fclose(f);
        done++;
    }
    free(b);
    return done;
}

/* a new, empty mod with a mod.txt that explains the layout */
int datamods_new(const char *name, char *where, size_t wn)
{
    char md[1100], p[1300]; mods_dir(md, sizeof md); MKDIR(md);
    snprintf(p, sizeof p, "%s/%s", md, name);
    if (is_dir(p)) { snprintf(where, wn, "%s", p); return 0; }
    if (MKDIR(p)) return -1;
    snprintf(where, wn, "%s", p);
    char t[1400]; snprintf(t, sizeof t, "%s/mod.txt", p);
    FILE *f = fopen(t, "wb");
    if (f) {
        fprintf(f, "%s\n"
                   "Describe your mod here (these lines show in F12 > Mods).\n"
                   "\n"
                   "Put files into this folder laid out like the game folder; while the mod is on, the game reads them\n"
                   "instead of its own: DARKLAND.LST, DARKLAND.ENM, PICS/NAME.PIC, a folder MSGFILES/ with single card\n"
                   "files (.MSG, or .TXT as written by \"Export the card texts\"), art/ and sounds/ as in the game folder,\n"
                   "music/ with recordings named by the piece's number (10.ogg, \"11 market.ogg\"; see F12 > Settings).\n", name);
        fclose(f);
    }
    datamods_scan();
    return 1;
}

void datamods_init(void) { datamods_scan(); }
