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
 * few; saves of the player are never touched.
 *
 * Ironman: a game with one save, kept by the game itself. Chosen for the next new game
 * (saves.ironman, F12 > Settings > Preset); a running game is known for one by its save's comment,
 * "Ironman" and a number of its own. It is saved over on every card, on arriving at a place,
 * before each battle and when the player quits or closes the window (once the game can save:
 * not during a battle or in a shop); the game's Save, Ctrl+S and Save now save over it; loading
 * is off until the player is back at the start screen; when the party's journey ends its save is
 * deleted. A program killed during a battle comes back to before the battle (none is saved in one). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include "../runtime/cpu.h"
void ovl_changed(void);
#include "../runtime/hle.h"
#include "../runtime/hooks.h"
#include "../runtime/dispatch.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "saves.h"
#include "assist.h"
#include "nscreen.h"
#include "savesview.h"

int saves_on = 1;                /* quick keys and autosaves (off: the original's saving only) */
int saves_auto_arrive = 1;       /* autosave on arriving at a place */
int saves_auto_battle = 1;       /* autosave before a battle */
int saves_keep_quick = 3, saves_keep_auto = 3;

void ui_notice(const char *msg);
u32 plat_ticks_ms(void);

#define D_SAVE_OK  GV_font_flag_a        /* Save menu item: 1 enabled */
#define D_LOAD_OK  GV_font_flag_b        /* Load menu item: 1 enabled */
#define D_IN_BATTLE GV_in_battle       /* 1 during a battle */
#define D_SAVENAME 0xa680        /* "dksaveN.sav": the save last written or loaded */
/* that name as the save is on disk: past 99 the game writes "dksave100.sav", which is dksavea0.sav
   (dos_name_alias, runtime/dos.c), the name the listings give */
void saves_cur_name(char *out, size_t n)
{
    int dos_name_alias(const char *, char *, size_t);
    g_str(G_DGROUP, D_SAVENAME, out, n);
    char al[32];
    if (dos_name_alias(out, al, sizeof al)) snprintf(out, n, "%s", al);
}

static const char *answer;       /* text_input's answer while we save */
static int quick_due;
static char auto_due[24];
static char load_name[16];
static int load_stage;           /* 1 asked for, 2 load_game called, 3 confirmed, 4 read */
static u32 load_t, loaded_t;
static int in_save;
static char iron[16];            /* the running game's save comment when it is an Ironman game ("Ironman 3f2a"), else "" */
static int close_due;            /* Ironman: the window was closed: save at the next chance, then quit */
static int loaded_since_title, iron_check;   /* a save read since the start screen; a save read (is it an Ironman game's?) */
static const char *IRON_NO_LOAD = "An Ironman game cannot go back to a save. To load another game, quit to the start screen.";
int saves_ironman(void) { return iron[0] != 0; }

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
    si->florins = h[0x70] | h[0x71] << 8; si->groschen = h[0x72] | h[0x73] << 8; si->pfennigs = h[0x74] | h[0x75] << 8;   /* (the purse: three unsigned words) */
    si->fame = W(0x7a);
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
    char cur[16]; saves_cur_name(cur, sizeof cur);
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
    int asked = !strncmp(comment, "Quick", 5);
    if (iron[0]) comment = iron;                       /* (an Ironman game: always over its one save) */
    char before[16]; saves_cur_name(before, sizeof before);
    answer = comment; in_save = 1;
    int r = game_call_far(GF_save_game, 0, 0);
    answer = 0; in_save = 0;
    if (r < 0) { ui_notice("Could not save."); return 0; }
    char name[16]; saves_cur_name(name, sizeof name);
    if (getenv("DL_SAVES_LOG")) fprintf(stderr, "saves: %s -> %s\n", comment, name);
    if (iron[0]) { prune(iron, 1); if (asked) ui_notice("Saved (an Ironman game keeps one save, saved over as you go)"); return 1; }
    if (!strncmp(comment, "Quicksave", 9)) prune("Quicksave", saves_keep_quick);
    else if (!strncmp(comment, "Autosave", 8)) prune("Autosave", saves_keep_auto);
    char m[80]; snprintf(m, sizeof m, "%s (%s)", asked ? "Game saved" : "Autosaved", name);
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
    void (*f)(void) = game_func(GF_load_game);
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
        else ui_notice(ns_screen == NS_LODGE ? "The game does not allow saving while staying at an inn or camping: leave first (the days already spent are not given back)." :
                       g_rb(D_IN_BATTLE) == 1 ? "The game does not allow saving during a battle." :
                       "The game does not allow saving on this screen: leave it first (saving works on a card, in the overland view or in a town).");
    } else if (auto_due[0] && g_rb(D_SAVE_OK) == 1 && g_rb(D_IN_BATTLE) != 1) {
        char c[24]; snprintf(c, sizeof c, "%s", auto_due);
        auto_due[0] = 0;
        do_save(c);
    }
    if (close_due && iron[0] && g_rb(D_SAVE_OK) == 1 && g_rb(D_IN_BATTLE) != 1) {   /* (the window closed: saved, then gone) */
        void plat_quit(int code);
        do_save("Autosave");
        fprintf(stderr, "program exit: window closed (Ironman: saved first)\n");
        plat_quit(0);
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

/* a load under way (the game's load screen and its "Difficulty is now set to ..." box): the
   interface covers the view meanwhile (ui/ui.cpp) */
/* the cover over the game's load screen: while it opens and reads the save, and after, until that
   screen (s037) is gone and the place saved in has had a moment to be drawn (it showed for a few
   frames as the save was read: "Difficulty is now set to ...", the list) */
int saves_loading_cover(void)
{
    static u32 gone_t;
    if (load_stage == 2 || load_stage == 3) { gone_t = 0; return 1; }
    if (!loaded_t || plat_ticks_ms() - loaded_t > 4000) return 0;
    if (ovl_loaded[37]) { gone_t = 0; return 1; }
    if (!gone_t) gone_t = plat_ticks_ms();
    return plat_ticks_ms() - gone_t < 200;
}

static void askyesno_hook(void)            /* ask_yes_no(question) */
{
    if (load_stage == 2) {                  /* "Loading is irreversable. Proceed?" */
        load_stage = 3;
        AX = 1;
        hook_return_far(0);
        return;
    }
    hook_call_original();
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
    void (*hide)(void) = game_func(GF_screen_hold), (*show)(void) = game_func(GF_screen_release);
    void (*open)(void) = game_func(37, 0x1c7d, 0x024e), (*draw)(void) = game_func(37, 0x1c7d, 0x02ce);
    void (*key)(void) = game_func(37, 0x1c7d, 0x030a), (*close)(void) = game_func(37, 0x1c7d, 0x0218);
    if (!hide || !show || !open || !draw || !key || !close) { load_stage = 0; hook_call_original(); return; }
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));          /* the fonts, put back by 0218 */
    game_call_live(hide, 0, 0);
    game_call_live(open, 0, 0);
    game_call_live(draw, 0, 0);
    /* (the load screen and the game's "Difficulty is now set to ..." stay hidden: the screen is
       shown again only once the save is read, as the game draws the place it was saved in) */
    char first[16]; g_str(G_FAR2E38, 0x3b2e, first, sizeof first);
    int ok = 0;
    if (!strcasecmp(first, load_name) && (s16)g_rw(0x8c54) > 0) {
        g_ww(0x8c50, 0);
        u16 k = 0x264c;
        ok = (u16)game_call_live(key, 1, &k) == 0x11b;
    }
    game_call_live(show, 0, 0);
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
        char m[80]; snprintf(m, sizeof m, g_rb(D_IN_BATTLE) == 1 ? "Loaded %s: a battle, paused (Space resumes it)" : "Loaded %s", load_name);
        ui_notice(m);
    }
    load_stage = 0;
    loaded_t = plat_ticks_ms();
    auto_due[0] = 0;
    { void travel_stop(void); void encounters_after_load(void);   /* (what was under way is the old game's: a journey went on */
      travel_stop(); encounters_after_load(); }                   /*  on the loaded map, 1.4-RC2) */
    loaded_since_title = 1; iron_check = 1;          /* (an Ironman game's save, or not: saves_frame) */
}

/* the game's own Save and Load (the menu, Alt-S, Alt-L) as native dialogs (mods/savesview.c) */
static void save_hook(void)                /* save_game () */
{
    if (in_save) { hook_call_original(); return; }          /* (a quick- or autosave: do_save) */
    if (iron[0]) {                                          /* (an Ironman game: over its one save, no name to give) */
        answer = iron; in_save = 1;
        hook_call_original();
        answer = 0; in_save = 0;
        if (unw_active) return;
        prune(iron, 1);
        ui_notice("Saved (an Ironman game keeps one save, saved over as you go)");
        return;
    }
    char c[48], old[16], before[16];
    int r = savesview_save(c, sizeof c, old, sizeof old);
    if (r < 0) { hook_call_original(); return; }
    if (r == 0) { AX = 0; hook_return_far(0); return; }
    saves_cur_name(before, sizeof before);
    answer = c; in_save = 1;
    hook_call_original();
    answer = 0; in_save = 0;
    if (unw_active) return;
    char name[16]; saves_cur_name(name, sizeof name);
    SaveInfo now;
    int written = (s16)AX >= 0 && saves_read(name, &now) && strcasecmp(name, old) && (strcasecmp(name, before) || !before[0]);
    char m[128];
    if (old[0] && written && saves_delete(old)) snprintf(m, sizeof m, "Saved: %s (%s), in place of the old save", c, name);   /* (the save chosen to replace) */
    else snprintf(m, sizeof m, "Saved: %s (%s)", c, name);
    fprintf(stderr, "saves: %s\n", m);
    ui_notice(m);
}

static void load_hook(void)                /* load_game () */
{
    if (iron[0] && !load_stage) { ui_notice(IRON_NO_LOAD); hook_return_far(0); return; }
    if (load_stage || !saves_on) { hook_call_original(); return; }   /* (ours under way; or the original's list and question) */
    char f[16];
    int r = savesview_load(f, sizeof f);
    if (r < 0) { hook_call_original(); return; }
    if (r == 0) { hook_return_far(0); return; }
    snprintf(load_name, sizeof load_name, "%s", f);
    load_stage = 2; load_t = plat_ticks_ms();
    if (getenv("DL_SAVES_LOG")) fprintf(stderr, "saves: loading %s (the game's Load)\n", load_name);
    hook_call_original();                   /* (when it loads, the game unwinds through here) */
    if (unw_active) return;
    load_stage = 0;
}

/* the card's option switches (DS:ee76.., set by the card's script as its menu comes up) as they
   were while the menu was up. A save before a battle is made after the option chosen has run,
   which may have used them for something else (the boars' "use your horses" switch was left on);
   loaded, the card's menu comes up again from the saved switches without its script, and offered
   the horses to a party without any. The battle's autosave keeps the menu's switches instead */
static char menu_card[16]; static u16 menu_flags[10];
static void note_menu_flags(void)
{
    if (!assist_in_card_menu()) return;
    g_str(G_DGROUP, GV_card_name, menu_card, sizeof menu_card);
    for (int k = 0; k < 10; k++) menu_flags[k] = g_rw((u16)(GV_opt_flag_0 + 2 * k));
}
static void battle_hook(void)              /* run_battle (s029 0e76:2278) */
{
    if (getenv("DL_BATTLE_LOG")) fprintf(stderr, "run_battle: type %#x, place %d, in battle %d, floor %d\n", (s16)rw(SS, (u16)(SP + 4)), (s16)rw(SS, (u16)(SP + 6)), g_rb(D_IN_BATTLE), (s8)g_rb(GV_floor_shown));
    if (((saves_on && saves_auto_battle) || iron[0]) && g_rb(D_IN_BATTLE) != 1 && plat_ticks_ms() - loaded_t > 3000) {
        char cn[16]; g_str(G_DGROUP, GV_card_name, cn, sizeof cn);
        u16 now[10]; int swap = menu_card[0] && !strcmp(cn, menu_card);
        for (int k = 0; k < 10; k++) { now[k] = g_rw((u16)(GV_opt_flag_0 + 2 * k)); if (swap) g_ww((u16)(GV_opt_flag_0 + 2 * k), menu_flags[k]); }
        do_save("Autosave: battle");
        for (int k = 0; k < 10; k++) g_ww((u16)(GV_opt_flag_0 + 2 * k), now[k]);
    }
    hook_call_original();
}

static void hooks_set(int on);
/* a game under way is an Ironman game when its save (the one read last) is one's, or when it is new
   and the next new game was to be one: decided as the start screen is left (mods/titleview.c), and
   again whenever a save is read during play */
static void iron_from_save(void)
{
    char cur[16]; saves_cur_name(cur, sizeof cur);
    SaveInfo si;
    iron[0] = 0; close_due = 0;
    if (cur[0] && saves_read(cur, &si) && !strncmp(si.comment, "Ironman ", 8)) snprintf(iron, sizeof iron, "%.15s", si.comment);
    if (iron[0]) { fprintf(stderr, "saves: an Ironman game (%s)\n", iron); hooks_set(1); assist_watch_cards(); }
    else hooks_set(saves_on);
}
static int at_title;
void saves_title_reached(void) { at_title = 1; loaded_since_title = 0; iron[0] = 0; close_due = 0; hooks_set(saves_on); }
static void new_game_begins(void);
void saves_title_left(void)                      /* (the game begins: a save read there, or a new one) */
{
    if (!at_title) return;
    at_title = 0;
    if (loaded_since_title) { iron_check = 0; iron_from_save(); } else new_game_begins();
}
static void new_game_begins(void)
{
    iron[0] = 0; close_due = 0;
    if (!config_get_int("saves.ironman", 0)) return;
    snprintf(iron, sizeof iron, "Ironman %04x", (unsigned)((plat_ticks_ms() * 2654435761u) ^ (unsigned)time(0)) & 0xffff);
    snprintf(auto_due, sizeof auto_due, "Autosave");     /* (its save, made at once) */
    ui_notice("An Ironman game: it saves itself as you go, over one save. There is no going back.");
    fprintf(stderr, "saves: a new Ironman game (%s)\n", iron);
    hooks_set(1); assist_watch_cards();
}

static int last_loc = -2, was_menu;
static int card_due; static u32 card_saved;
void saves_frame(void)
{
    { void saves_note_game(void); saves_note_game(); }
    note_menu_flags();
    if (load_stage == 1 && plat_ticks_ms() - load_t > 30000) load_stage = 0;    /* never a moment to load */
    u32 now = plat_ticks_ms();
    if (iron_check && !at_title && game_mode() != GM_OTHER) { iron_check = 0; iron_from_save(); }
    if ((!saves_on && !iron[0]) || game_mode() == GM_OTHER) { last_loc = -2; return; }
    int loc = (s16)g_rw(G_CUR_LOC);
    if ((saves_auto_arrive || iron[0]) && last_loc == -1 && loc >= 0 && loc < G_NLOC && plat_ticks_ms() - loaded_t > 3000) snprintf(auto_due, sizeof auto_due, "Autosave");
    last_loc = loc;
    if (iron[0]) {                                     /* every card, once the last option's outcome has come: saved over (at most every 3 seconds) */
        int menu = assist_in_card_menu();
        if (menu && !was_menu) card_due = 1;
        was_menu = menu;
        if (card_due && now - loaded_t > 3000 && now - card_saved > 3000) { card_due = 0; card_saved = now; snprintf(auto_due, sizeof auto_due, "Autosave"); }
    }
}

/* the window closed (runtime/platform_sdl.c): an Ironman game is saved first, at the next moment the
   game allows it; 1 if the closing waits for that. Closed again, it goes at once */
int saves_close_requested(void)
{
    if (!iron[0] || game_mode() == GM_OTHER || close_due) return 0;
    close_due = 1;
    ui_notice(g_rb(D_IN_BATTLE) == 1 ? "Ironman: the game closes once it can save, after this battle. Close again to quit at once (the game then goes back to the last save)."
                                      : "Ironman: saving, then closing. Close again to quit at once (the game then goes back to the last save).");
    return 1;
}

/* the party's journey ended (the death of all, or retirement): an Ironman game's save goes with it */
void saves_journey_over(void)
{
    if (!iron[0]) return;
    SaveInfo *v; int n = saves_list(&v);
    for (int i = 0; i < n; i++) if (!strcmp(v[i].comment, iron)) {
        char b[20]; remove_ci(v[i].file); snprintf(b, sizeof b, "%s", v[i].file);
        char *dot = strrchr(b, '.'); if (dot) { strcpy(dot, ".BSV"); remove_ci(b); }
        fprintf(stderr, "saves: the Ironman game's save %s deleted (the journey is over)\n", v[i].file);
    }
    free(v);
    iron[0] = 0;
}

/* ---------------------------------------------------------------- requests */
void saves_quick_save(void)
{
    if (game_mode() == GM_OTHER) { ui_notice("Start or load a game first."); return; }
    quick_due = 1;
}

int saves_load(const char *name)
{
    if (iron[0]) { ui_notice(IRON_NO_LOAD); return 0; }
    if (game_mode() == GM_OTHER) { ui_notice("Loading from here works once a game is running; until then, load from the start screen."); return 0; }
    /* only a save that is there and can be read: once the game's load_game has torn the game down,
       a load that fails has nothing to go back to (the game stopped: "Buffer dealloc error") */
    SaveInfo si;
    if (!saves_read(name, &si)) { ui_notice("That save could not be read."); return 0; }
    snprintf(load_name, sizeof load_name, "%s", name);
    load_stage = 1; load_t = plat_ticks_ms();
    return 1;
}

/* the start screen's Continue and Load: its load screen (title_key 0x1454, s037 1c7d:0002), about
   to open, reads this save at once (loadscreen_hook, as for a load from the menu) */
void saves_title_load(const char *name)
{
    snprintf(load_name, sizeof load_name, "%s", name);
    load_stage = 3; load_t = plat_ticks_ms();
    fprintf(stderr, "saves: loading %s (the start screen)\n", load_name);
}

void saves_quick_load(void)
{
    if (iron[0]) { ui_notice(IRON_NO_LOAD); return; }
    SaveInfo *v; int n = saves_list(&v), found = -1;
    for (int i = 0; i < n && found < 0; i++) if (is_kind(&v[i], "Quicksave")) found = i;
    if (found < 0) ui_notice("No quicksave yet (Ctrl+S makes one).");
    else saves_load(v[found].file);
    free(v);
}

/* Ctrl+S quick save, F9 / Ctrl+L quick load; 1 if the key was taken */
int saves_key(int sym, int ctrl)
{
    if (!saves_on && !iron[0]) return 0;
    if (ctrl && (sym == 's' || sym == 'S')) { saves_quick_save(); return 1; }
    if ((ctrl && (sym == 'l' || sym == 'L')) || sym == 0x40000042 /* SDLK_F9 */) { saves_quick_load(); return 1; }
    return 0;
}

static struct { int sect; unsigned seg, off; void (*fn)(void); } HK[] = {
    { 37, 0x1462, 0x43e6, textinput_hook }, { -1, 0x0000, 0x0000, menubar_hook }, { -1, 0x0000, 0x2522, askyesno_hook },
    { 37, 0x1c7d, 0x06e6, loadlist_hook }, { 37, 0x1c7d, 0x0d24, loaded_hook }, { 37, 0x1c7d, 0x0002, loadscreen_hook }, { 29, 0x0e76, 0x2278, battle_hook } };

static void hooks_set(int on)
{
    static int installed;
    if (on == installed) return;
    installed = on;
    for (size_t i = 0; i < sizeof HK / sizeof HK[0]; i++) {
        if (on) { if (!hook_install(HK[i].sect, HK[i].seg, HK[i].off, HK[i].fn)) fprintf(stderr, "saves: no hook point %d %04x:%04x\n", HK[i].sect, HK[i].seg, HK[i].off); }
        else hook_remove(HK[i].sect, HK[i].seg, HK[i].off, HK[i].fn);
    }
}
void saves_enable(int on)
{
    saves_on = on;
    config_set_int("saves.quick", on);
    hooks_set(on || iron[0]);                          /* (an Ironman game saves itself whatever this says) */
}

/* the end of the party: the original leaves to DOS (after the death pictures); the port starts
 * the game again at its title instead, unless the player chose Quit (quit_to_dos, root
 * 0000:13a0) - so a saved game can be loaded at once */
int saves_restart = 1;
static int user_quit, game_seen;
static void quit_hook(void)
{
    fprintf(stderr, "quit_to_dos asked\n");
    if (iron[0] && g_rb(D_SAVE_OK) == 1 && g_rb(D_IN_BATTLE) != 1) do_save("Autosave");   /* (an Ironman game: saved as it is left) */
    user_quit = 1; hook_call_original(); user_quit = 0;
}
int saves_restart_on_exit(void)
{
    int headless = getenv("DL_HEADLESS") != 0;
    if (user_quit || !saves_restart || headless || !game_seen || config_get_int("original", 0)) return 0;
    fprintf(stderr, "the party's end: starting the game again\n");
    return 1;
}
void saves_note_game(void) { if (game_mode() != GM_OTHER) game_seen = 1; }
int saves_party_ended(void) { return !user_quit && game_seen; }     /* the program ends for the party's end, not a quit */

/* ---- battle saves made by Darklands+ up to 1.3: a castle's, mine's or dungeon's trigger areas came back
 * as pits. A battle save stores each level's trigger routines as numbers (s003 0e76:09b9..0a48: e->fn ==
 * vec_17f7 -> 1 ...) and the restore turns them back, any number it does not know into the pit's routine
 * (vec_181f, "Aaaahh!" once the whole party stands in the area). The translation compared the routines'
 * segment without the load segment, so nothing matched and the routines were saved as they stood in memory
 * (fixed in the translator, recomp16.py: a relocated immediate of cmp): after loading such a save every
 * trap, secret door, alarm and the mine's dwarves' iron door was a pit, and the door showed no riddle
 * (GitHub issue 2). When the restore takes a level out of the save (EXPLODE, s003 1016:0d07, from
 * 0e76:17d5), a routine stored as a far pointer is given its number first (the restore's own table). */
static void bsv_level_hook(void)
{
    u16 ret_off = rw(SS, SP), ret_seg = rw(SS, (u16)(SP + 2));
    hook_call_original();
    if (ret_off != 0x17da || ret_seg != (u16)(LOADSEG + 0x0e76)) return;
    static const u16 VEC[8] = { 0x17ed, 0x17f7, 0x1801, 0x180b, 0x181f, 0x1815, 0x1833, 0x1829 }, TYPE[8] = { 0, 1, 2, 4, 5, 6, 7, 8 };
    u16 bo = g_rw(GV_load_buf), bs = g_rw((u16)(GV_load_buf + 2));
    int n = rb(bs, (u16)(bo + 0x2184)), fixed = 0;
    for (int k = 0; k < n && k < 50; k++) {
        u16 r = (u16)(bo + 0x218c + 14 * k), off = rw(bs, r), seg = rw(bs, (u16)(r + 2));
        if (!seg) continue;                                     /* (a number, as the game saves it) */
        for (int t = 0; t < 8; t++) if (off == VEC[t]) { ww(bs, r, TYPE[t]); ww(bs, (u16)(r + 2), 0); fixed++; break; }
    }
    if (fixed) fprintf(stderr, "saves: %d trigger routines of a level put back (a battle save of an earlier Darklands+)\n", fixed);
}

void saves_init(void)
{
    hook_install(3, 0x1016, 0x0d07, bsv_level_hook);
    saves_restart = config_get_int("saves.restart_on_end", 1);
    hook_install(GF_quit_to_dos, quit_hook);
    hook_install(GF_save_game, save_hook);
    hook_install(GF_load_game, load_hook);
    savesview_init();
    saves_auto_arrive = config_get_int("saves.autosave_arrive", 1);
    saves_auto_battle = config_get_int("saves.autosave_battle", 1);
    saves_keep_quick = config_get_int("saves.keep_quick", 3);
    saves_keep_auto = config_get_int("saves.keep_auto", 3);
    saves_enable(config_get_int("saves.quick", 1));
}

/* ---- the first start of each version: a copy of the saved games before it writes any
 * ("Darklands+ save backups/<version> <date>/" in the player's Darklands+ folder: the game
 * folder, or the data folder when that cannot be written). saves.backup = 0 switches it off. */
void dos_mkdirs_for(const char *path);
void saves_backup_once(void)
{
    extern const char dlp_release[], dlp_version[]; extern char dlp_user_dir[];
    if (((getenv("DL_SCRIPT") || getenv("DL_CTL") || getenv("DL_HEADLESS")) && !getenv("DL_SAVES_BACKUP")) || !config_get_int("saves.backup", 1)) return;   /* (tests: only when asked) */
    const char *ver = dlp_release[0] ? dlp_release : dlp_version;
    if (!strcmp(config_get("saves.backed_up", ""), ver)) return;
    FindEnt *l = 0; int n = dos_list_dos("SAVES", "*.*", &l), done = 0, failed = 0;
    time_t t = time(0); struct tm *tm = localtime(&t);
    char dir[1400]; snprintf(dir, sizeof dir, "%s/Darklands+ save backups/%s, %04d-%02d-%02d", dlp_user_dir, ver, tm ? tm->tm_year + 1900 : 0, tm ? tm->tm_mon + 1 : 0, tm ? tm->tm_mday : 0);
    for (int i = 0; i < n; i++) {
        char dn[64], src[1100], dst[1700]; snprintf(dn, sizeof dn, "SAVES\\%s", l[i].name);
        dos_host_path(dn, src, sizeof src);
        snprintf(dst, sizeof dst, "%s/%s", dir, l[i].real);
        dos_mkdirs_for(dst);
        FILE *a = fopen(src, "rb"), *b = a ? fopen(dst, "wb") : 0;
        if (a && b) { char buf[16384]; size_t k; int bad = 0; while ((k = fread(buf, 1, sizeof buf, a)) > 0) if (fwrite(buf, 1, k, b) != k) bad = 1; if (bad) failed++; else done++; }
        else failed++;
        if (a) fclose(a);
        if (b) fclose(b);
    }
    free(l);
    fprintf(stderr, "saves: %d file%s backed up in %s%s\n", done, done == 1 ? "" : "s", dir, failed ? " (some could not be copied)" : "");
    if (!failed) { config_set("saves.backed_up", ver); config_save(); }
}
