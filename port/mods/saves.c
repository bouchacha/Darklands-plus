/* saves.c - quick save and quick load, autosaves, and loading any save from the Darklands+ menu.
 *
 * The original saves only through Alt-S (or the menu) with a comment to type, into a new
 * saves\dksaveN.sav every time, and loads through Alt-L and a list of nine. Everything here
 * goes through the game's own routines, at the moment the game itself would run them:
 *  - saving: save_game (s037 1462:2a68), whose comment prompt (text_input, 1462:43e6) is
 *    answered with "Quicksave" / "Autosave ..."; from the menu bar loop (menu_bar_run, root
 *    0000:0000, which calls hot_keys, where Alt-S runs it) when the game allows saving (Save menu item enabled, DS:9920 == 1), or before a
 *    battle (run_battle, s029 0e76:2278).
 *  - loading: load_game (s037 1c7d:09c6) is called where Alt-L would call it (menu_bar_run);
 *    its "Loading is irreversable. Proceed?" (ask_yes_no, root 0000:2522) is answered yes, and
 *    its load screen (1c7d:0002) does what it does when L is pressed on the save (the list,
 *    1c7d:06e6, is started at it). load_game then tears the running game down and returns to
 *    the main loop (longjmp) as usual.
 * Older quick- and autosaves (told apart by their comment) are deleted, keeping the newest
 * few; saves of the player are never touched. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include "../runtime/cpu.h"
void ovl_changed(void);
#include "../runtime/hle.h"
#include "../runtime/hooks.h"
#include "../runtime/dispatch.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "saves.h"

int saves_on = 1;                /* quick keys and autosaves (off: the original's saving only) */
int saves_auto_arrive = 1;       /* autosave on arriving at a place */
int saves_auto_battle = 1;       /* autosave before a battle */
int saves_keep_quick = 3, saves_keep_auto = 3;

void ui_notice(const char *msg);
u32 plat_ticks_ms(void);

#define D_SAVE_OK  0x9920        /* Save menu item: 1 enabled */
#define D_LOAD_OK  0x9934        /* Load menu item: 1 enabled */
#define D_IN_BATTLE 0x907a       /* 1 during a battle */
#define D_SAVENAME 0xa680        /* "dksaveN.sav": the save last written or loaded */

static const char *answer;       /* text_input's answer while we save */
static int quick_due;
static char auto_due[24];
static char load_name[16];
static int load_stage;           /* 1 asked for, 2 load_game called, 3 confirmed, 4 read */
static u32 load_t, loaded_t;
static int in_save;

/* ---------------------------------------------------------------- files */

/* the file on disk of a save's DOS name, as the game reads it (any case; the shadow folder's
 * copy when the game folder cannot be written, runtime/dos.c) */
int dos_list_dos(const char *dosdir, const char *pat, FindEnt **out);
static int real_path(const char *name, char *p, size_t n)
{
    char dn[64]; snprintf(dn, sizeof dn, "SAVES\\%s", name);
    dos_host_path(dn, p, n);
    struct stat st;
    return stat(p, &st) == 0;
}

int saves_read(const char *name, SaveInfo *si)
{
    char p[1200];
    FILE *f = real_path(name, p, sizeof p) ? fopen(p, "rb") : 0;
    if (!f) return 0;
    u8 h[0x189 + 5 * 0x22a];
    memset(h, 0, sizeof h);
    size_t got = fread(h, 1, sizeof h, f);
    fclose(f);
    if (got < 0x189) return 0;
    memset(si, 0, sizeof *si);
    snprintf(si->file, sizeof si->file, "%s", name);
    memcpy(si->place, h, 0x15); si->place[0x15] = 0;
    memcpy(si->comment, h + 0x15, 0x4f); si->comment[0x4f] = 0;
#define W(o) ((int)(s16)(h[o] | h[(o) + 1] << 8))
    si->year = W(0x68); si->month = W(0x6a); si->day = W(0x6c); si->hour = W(0x6e);
    si->florins = W(0x70); si->fame = W(0x7a);
    int nd = W(0xf1), np = 0;
    for (int m = 0; m < 5; m++) {
        int id = W(0xf3 + 2 * m);
        if (id < 0 || id >= nd || 0x189 + (id + 1) * 0x22a > (int)got) continue;
        const u8 *r = h + 0x189 + id * 0x22a;
        memcpy(si->members[np], r + 0x3e, 11); si->members[np][11] = 0;
        np++;
    }
    si->nmembers = np;
#undef W
    struct stat st;
    si->mtime = stat(p, &st) ? 0 : (long long)st.st_mtime;
    si->battle = (h[0x96] & 0x80) != 0;                 /* the difficulty byte: bit 7 while in battle */
    return 1;
}

int saves_list(SaveInfo **out)
{
    FindEnt *l; int n = dos_list_dos("SAVES", "*.SAV", &l);
    SaveInfo *v = calloc((size_t)(n ? n : 1), sizeof *v);
    int k = 0;
    for (int i = 0; i < n; i++) if (saves_read(l[i].name, &v[k])) k++;
    free(l);
    *out = v;
    return k;
}

static int is_kind(const SaveInfo *s, const char *prefix) { return !strncmp(s->comment, prefix, strlen(prefix)); }

static void remove_ci(const char *name)
{
    int dos_delete(const char *dos);
    char dn[64]; snprintf(dn, sizeof dn, "SAVES\\%s", name);
    dos_delete(dn);                      /* any case; in the shadow folder too (runtime/dos.c) */
}

int saves_delete(const char *name)
{
    char cur[16]; g_str(G_DGROUP, D_SAVENAME, cur, sizeof cur);
    if (!strcasecmp(cur, name)) return 0;                  /* the game's own rule: not the loaded one */
    char b[20];
    remove_ci(name);
    snprintf(b, sizeof b, "%s", name);
    char *dot = strrchr(b, '.');
    if (dot) { strcpy(dot, ".BSV"); remove_ci(b); }
    return 1;
}

/* keep the newest few quick- and autosaves */
static void prune(const char *prefix, int keep)
{
    SaveInfo *v; int n = saves_list(&v), seen = 0;
    for (int i = 0; i < n; i++) {                            /* newest first */
        if (!is_kind(&v[i], prefix)) continue;
        if (++seen > keep) saves_delete(v[i].file);
    }
    free(v);
}

/* ---------------------------------------------------------------- saving */
static void textinput_hook(void)           /* text_input(char far *buf, char far *prompt, int max) */
{
    if (!answer) { hook_call_original(); return; }
    u16 off = rw(SS, (u16)(SP + 4)), seg = rw(SS, (u16)(SP + 6));
    int max = (s16)rw(SS, (u16)(SP + 12)), i = 0;
    if (max < 2 || max > 0x4f) max = 0x17;
    for (; answer[i] && i < max - 1; i++) wb(seg, (u16)(off + i), (u8)answer[i]);
    wb(seg, (u16)(off + i), 0);
    answer = 0;
    hook_return_far(0);
}

static int do_save(const char *comment)
{
    if (g_rb(D_SAVE_OK) != 1) return 0;
    char before[16]; g_str(G_DGROUP, D_SAVENAME, before, sizeof before);
    answer = comment; in_save = 1;
    int r = game_call_far(37, 0x1462, 0x2a68, 0, 0);
    answer = 0; in_save = 0;
    if (r < 0) { ui_notice("Could not save."); return 0; }
    char name[16]; g_str(G_DGROUP, D_SAVENAME, name, sizeof name);
    if (getenv("DL_SAVES_LOG")) fprintf(stderr, "saves: %s -> %s\n", comment, name);
    if (!strncmp(comment, "Quicksave", 9)) prune("Quicksave", saves_keep_quick);
    else if (!strncmp(comment, "Autosave", 8)) prune("Autosave", saves_keep_auto);
    char m[80]; snprintf(m, sizeof m, "%s saved (%s)", !strncmp(comment, "Quick", 5) ? "Game" : "Autosave", name);
    ui_notice(m);
    return 1;
}

/* ---------------------------------------------------------------- the moments */
/* load_game (s037 1c7d:09c6), called the way Alt-L calls it: when it loads, it ends in a
 * longjmp to the main loop, which unwinds through here (1); if it does not (the player said
 * no, nothing to load), the overlays are put back as the vector would and 0 is returned */
void ovl_area(u32 *lin_start, u32 *lin_end);
static int call_load_game(void)
{
    void (*f)(void) = game_func(37, 0x1c7d, 0x09c6);
    if (!f) return 0;
    u32 a0, a1;
    ovl_area(&a0, &a1);
    a0 += (u32)LOADSEG << 4; a1 += (u32)LOADSEG << 4;
    u8 *snap = malloc(a1 - a0);
    int loaded[256];
    memcpy(snap, M + a0, a1 - a0);
    memcpy(loaded, ovl_loaded, sizeof loaded);
    u16 sp = SP, ds = DS, es = ES;
    DS = ES = G_DGROUP;
    push16(HLE_SEG); push16(0xfff0);
    ovl_enter(37);
    f();
    if (unw_active) { free(snap); return 1; }
    SP = sp; DS = ds; ES = es;
    memcpy(M + a0, snap, a1 - a0);
    memcpy(ovl_loaded, loaded, sizeof loaded); ovl_changed();
    free(snap);
    return 0;
}

static void menubar_hook(void)             /* menu_bar_run(menu, once): with once 0 it calls hot_keys, which reads Alt-S, Alt-L ... */
{
    if ((s16)rw(SS, (u16)(SP + 6)) != 0) { hook_call_original(); return; }
    if (quick_due) {
        quick_due = 0;
        if (g_rb(D_SAVE_OK) == 1) do_save("Quicksave");
        else ui_notice("The game does not allow saving here.");
    } else if (auto_due[0] && g_rb(D_SAVE_OK) == 1 && g_rb(D_IN_BATTLE) != 1) {
        char c[24]; snprintf(c, sizeof c, "%s", auto_due);
        auto_due[0] = 0;
        do_save(c);
    }
    if (load_stage == 1) {
        if (g_rb(D_LOAD_OK) == 1) {
            load_stage = 2; load_t = plat_ticks_ms();
            if (getenv("DL_SAVES_LOG")) fprintf(stderr, "saves: loading %s\n", load_name);
            if (call_load_game()) return;                  /* back in the main loop with the save */
            load_stage = 0;
        } else { load_stage = 0; ui_notice("The game does not allow loading here."); }
    }
    hook_call_original();
}

static void askyesno_hook(void)            /* ask_yes_no(question) */
{
    if (load_stage == 2) {                  /* "Loading is irreversable. Proceed?" */
        load_stage = 3;
        AX = 1;
        hook_return_far(0);
        return;
    }
    extern int cardview_dialog;
    cardview_dialog++;                      /* (the native card steps aside: mods/cardview.c) */
    hook_call_original();
    cardview_dialog--;
}

static void loadlist_hook(void)            /* the load screen's list (s037 1c7d:06e6): from our save on */
{
    if (load_stage == 3) {
        FindEnt *l; int n = dos_list_dos("SAVES", "*.SAV", &l), at = -1;
        for (int i = 0; i < n; i++) if (!strcasecmp(l[i].name, load_name)) { at = i; break; }
        free(l);
        if (at >= 0) g_ww(0x8c52, (u16)at);
    }
    hook_call_original();
}

/* the load screen (s037 1c7d:0002), what it does when L is pressed on our save: drawn (and the
 * list read), then 030a(L) reads the save, then 0218 closes the screen */
static void loadscreen_hook(void)
{
    if (load_stage != 3) { hook_call_original(); return; }
    void (*hide)(void) = game_func(-1, 0x047c, 0x0568), (*show)(void) = game_func(-1, 0x047c, 0x056f);
    void (*open)(void) = game_func(37, 0x1c7d, 0x024e), (*draw)(void) = game_func(37, 0x1c7d, 0x02ce);
    void (*key)(void) = game_func(37, 0x1c7d, 0x030a), (*close)(void) = game_func(37, 0x1c7d, 0x0218);
    if (!hide || !show || !open || !draw || !key || !close) { load_stage = 0; hook_call_original(); return; }
    g_ww(0xe7a2, g_rw(0x2816)); g_ww(0xe7a0, g_rw(0x2804));          /* the fonts, put back by 0218 */
    game_call_live(hide, 0, 0);
    game_call_live(open, 0, 0);
    game_call_live(draw, 0, 0);
    game_call_live(show, 0, 0);
    char first[16]; g_str(G_FAR2E38, 0x3b2e, first, sizeof first);
    int ok = 0;
    if (!strcasecmp(first, load_name) && (s16)g_rw(0x8c54) > 0) {
        g_ww(0x8c50, 0);
        u16 k = 0x264c;
        ok = (u16)game_call_live(key, 1, &k) == 0x11b;
    }
    if (!ok) ui_notice("That save could not be read.");
    load_stage = 0;
    u16 a = (u16)ok;
    AX = (u16)game_call_live(close, 1, &a);
    hook_return_far(0);
}

static void loaded_hook(void)              /* the save is read (s037 1c7d:0d24) */
{
    if (getenv("DL_SAVES_LOG")) {
        char f[16]; g_str(rw(SS, (u16)(SP + 6)), rw(SS, (u16)(SP + 4)), f, sizeof f);
        fprintf(stderr, "saves: the game reads %s (stage %d)\n", f, load_stage);
    }
    hook_call_original();
    if (load_name[0] && load_stage) {
        char m[80]; snprintf(m, sizeof m, g_rb(D_IN_BATTLE) == 1 ? "Loaded %s: a battle, paused - Space goes on" : "Loaded %s", load_name);
        ui_notice(m);
    }
    load_stage = 0;
    loaded_t = plat_ticks_ms();
    auto_due[0] = 0;
}

static void battle_hook(void)              /* run_battle (s029 0e76:2278) */
{
    if (saves_on && saves_auto_battle && g_rb(D_IN_BATTLE) != 1 && plat_ticks_ms() - loaded_t > 3000) do_save("Autosave: battle");
    void loot_battle_begin(void), loot_battle_end(void);
    loot_battle_begin();                    /* (mods/loot.c: the thrown weapons each member carries) */
    hook_call_original();
    loot_battle_end();
}

static int last_loc = -2;
void saves_frame(void)
{
    { void saves_note_game(void); saves_note_game(); }
    if (load_stage == 1 && plat_ticks_ms() - load_t > 30000) load_stage = 0;    /* never a moment to load */
    if (!saves_on || game_mode() == GM_OTHER) { last_loc = -2; return; }
    int loc = (s16)g_rw(G_CUR_LOC);
    if (saves_auto_arrive && last_loc == -1 && loc >= 0 && loc < G_NLOC && plat_ticks_ms() - loaded_t > 3000) snprintf(auto_due, sizeof auto_due, "Autosave");
    last_loc = loc;
}

/* ---------------------------------------------------------------- requests */
void saves_quick_save(void)
{
    if (game_mode() == GM_OTHER) { ui_notice("Start or load a game first."); return; }
    quick_due = 1;
}

int saves_load(const char *name)
{
    if (game_mode() == GM_OTHER) { ui_notice("Loading from here works once a game is running; use the game's own Load before that."); return 0; }
    snprintf(load_name, sizeof load_name, "%s", name);
    load_stage = 1; load_t = plat_ticks_ms();
    return 1;
}

void saves_quick_load(void)
{
    SaveInfo *v; int n = saves_list(&v), found = -1;
    for (int i = 0; i < n && found < 0; i++) if (is_kind(&v[i], "Quicksave")) found = i;
    if (found < 0) ui_notice("No quicksave yet (Ctrl+S makes one).");
    else saves_load(v[found].file);
    free(v);
}

/* Ctrl+S quick save, F9 / Ctrl+L quick load; 1 if the key was taken */
int saves_key(int sym, int ctrl)
{
    if (!saves_on) return 0;
    if (ctrl && (sym == 's' || sym == 'S')) { saves_quick_save(); return 1; }
    if ((ctrl && (sym == 'l' || sym == 'L')) || sym == 0x40000042 /* SDLK_F9 */) { saves_quick_load(); return 1; }
    return 0;
}

static struct { int sect; unsigned seg, off; void (*fn)(void); } HK[] = {
    { 37, 0x1462, 0x43e6, textinput_hook }, { -1, 0x0000, 0x0000, menubar_hook }, { -1, 0x0000, 0x2522, askyesno_hook },
    { 37, 0x1c7d, 0x06e6, loadlist_hook }, { 37, 0x1c7d, 0x0d24, loaded_hook }, { 37, 0x1c7d, 0x0002, loadscreen_hook }, { 29, 0x0e76, 0x2278, battle_hook } };

void saves_enable(int on)
{
    saves_on = on;
    config_set_int("saves.quick", on);
    for (size_t i = 0; i < sizeof HK / sizeof HK[0]; i++) {
        if (on) { if (!hook_install(HK[i].sect, HK[i].seg, HK[i].off, HK[i].fn)) fprintf(stderr, "saves: no hook point %d %04x:%04x\n", HK[i].sect, HK[i].seg, HK[i].off); }
        else hook_remove(HK[i].sect, HK[i].seg, HK[i].off);
    }
    /* these two do other work too (the native card steps aside for a question; thrown weapons
       are noted at a battle's start): always in place, their saving parts check saves_on */
    hook_install(-1, 0x0000, 0x2522, askyesno_hook);
    hook_install(29, 0x0e76, 0x2278, battle_hook);
}

/* the end of the party: the original leaves to DOS (after the death pictures); the port starts
 * the game again at its title instead, unless the player chose Quit (quit_to_dos, root
 * 0000:13a0) - so a saved game can be loaded at once */
int saves_restart = 1;
static int user_quit, game_seen;
static void quit_hook(void) { fprintf(stderr, "quit_to_dos asked\n"); user_quit = 1; hook_call_original(); user_quit = 0; }
int saves_restart_on_exit(void)
{
    int headless = getenv("DL_HEADLESS") != 0;
    if (user_quit || !saves_restart || headless || !game_seen || config_get_int("original", 0)) return 0;
    fprintf(stderr, "the party's end: starting the game again\n");
    return 1;
}
void saves_note_game(void) { if (game_mode() != GM_OTHER) game_seen = 1; }
int saves_party_ended(void) { return !user_quit && game_seen; }     /* the program ends for the party's end, not a quit */

void saves_init(void)
{
    saves_restart = config_get_int("saves.restart_on_end", 1);
    hook_install(-1, 0x0000, 0x13a0, quit_hook);
    saves_auto_arrive = config_get_int("saves.autosave_arrive", 1);
    saves_auto_battle = config_get_int("saves.autosave_battle", 1);
    saves_keep_quick = config_get_int("saves.keep_quick", 3);
    saves_keep_auto = config_get_int("saves.keep_auto", 3);
    saves_enable(config_get_int("saves.quick", 1));
}
