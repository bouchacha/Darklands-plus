/* savesview.c - the game's Save and Load as native dialogs (display.native_saves; the Full Original
 * preset keeps the game's). The view is ui/savesview.cpp.
 *
 * The original saves with a one-line comment prompt (save_game, s037 1462:2a68, into a new
 * saves\dksaveN.sav each time) and loads from a list of comments, nine at a time, after
 * "Loading is irreversable. Proceed?" (load_game, s037 1c7d:09c6). Here Save asks for the
 * comment on a page that also lists the saves there are (where, when in the game and in the
 * world, who); Load is that list to choose from. Both are still the game's own routines
 * (mods/saves.c: the comment handed to save_game's prompt; load_game led through its question
 * and list to the save chosen). Players long wished to save over an old save rather than pile up
 * new ones: a click on a save in the Save page's list makes the new save replace it (the game
 * still writes a new dksaveN.sav; the old one is deleted after), and the Load page deletes the
 * save chosen (two clicks; never the one last loaded or written, the game's own rule). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "savesview.h"
#include "gamemodel.h"

u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void dl_poll(void);

SavesModel saves_model;
static int sv_on = 1;

int savesview_is_on(void) { return sv_on; }
void savesview_set(int on) { sv_on = on; config_set_int("display.native_saves", on); }
void savesview_init(void) { sv_on = config_get_int("display.native_saves", 1); }

#define NQ 16
static NsAction q[NQ]; static int qh, qt;
void savesview_push(int type, int a) { int n = (qt + 1) % NQ; if (n == qh) return; q[qt].type = type; q[qt].a = a; q[qt].b = q[qt].c = 0; qt = n; }
static int pop(NsAction *o) { if (qh == qt) return 0; *o = q[qh]; qh = (qh + 1) % NQ; return 1; }


static void default_comment(char *out, int n)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    char place[24] = "";
    u32 r = cur >= 0 ? g_location(cur) : 0;
    if (r) g_str((u16)(r >> 16), (u16)((u16)r + 0x26), place, sizeof place);
    for (int i = 0; place[i]; i++) if ((unsigned char)place[i] < 32 || (unsigned char)place[i] > 126) place[i] = ' ';
    int day = (s16)g_rw(G_CUR_DAY), mon = (s16)g_rw(G_CUR_MONTH);
    char b[64]; snprintf(b, sizeof b, "%.14s %d %.3s", place[0] ? place : "Travelling", day, gm_month_name((unsigned)mon % 12));
    snprintf(out, (size_t)n, "%s", b);
}

/* the saves' comments are typed in the game's prompt: printable, at most SV_COMMENT_MAX letters */
static void take_comment(SavesModel *S, const SaveInfo *v)
{
    if (!strncmp(v->comment, "Quicksave", 9) || !strncmp(v->comment, "Autosave", 8)) { default_comment(S->comment, SV_COMMENT_MAX + 1); return; }   /* (those are told apart by it) */
    int k = 0;
    for (const char *c = v->comment; *c && k < SV_COMMENT_MAX; c++) S->comment[k++] = (*c >= 32 && *c < 127) ? *c : ' ';
    while (k && S->comment[k - 1] == ' ') k--;
    S->comment[k] = 0;
}

static void delete_save(SavesModel *S, int i)
{
    if (i < 0 || i >= S->nsaves || !strcasecmp(S->saves[i].file, S->cur_file)) return;    /* (not the save in play: the game's own rule) */
    u32 now = plat_ticks_ms();
    if (S->armed != i || now - S->armed_t > 4000) { S->armed = i; S->armed_t = now; return; }   /* (the first click: asks) */
    if (now - S->armed_t < 350) return;                     /* (not a double click) */
    S->armed = -1;
    char f[16]; snprintf(f, sizeof f, "%s", S->saves[i].file);
    if (!saves_delete(f)) { fprintf(stderr, "saves: %s not deleted (the save in play)\n", f); return; }
    fprintf(stderr, "saves: %s deleted\n", f);
    free(S->saves); S->saves = 0;
    S->nsaves = saves_list(&S->saves);
    if (S->sel >= S->nsaves) S->sel = S->nsaves - 1;
    S->seq++;
}

static int ok_or_cancel(int loading)
{
    SavesModel *S = &saves_model;
    qh = qt = 0;
    free(S->saves); S->saves = 0;
    S->nsaves = saves_list(&S->saves);
    g_str(G_DGROUP, 0xa680, S->cur_file, sizeof S->cur_file);     /* "dksaveN.sav": the save last written or read */
    S->loading = loading;
    S->sel = -1; S->armed = -1;
    if (loading) for (int i = 0; i < S->nsaves; i++) if (!strcasecmp(S->saves[i].file, S->cur_file)) { S->sel = i; break; }
    if (loading && S->sel < 0 && S->nsaves) S->sel = 0;
    S->open = 1; S->seq++; S->caret_t = plat_ticks_ms();
    fprintf(stderr, "saves: the %s page (%d saves)\n", loading ? "Load" : "Save", S->nsaves);
    ns_enter(NS_SAVES);
    ns_input_begin();
    int r = 0;
    while (!r) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        while (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (k == 0x011b) { r = -1; break; }
            if (k == 0x1c0d) { if (!loading || S->sel >= 0) { r = 1; break; } continue; }
            if (loading) {
                if (k == 0x4800 && S->sel > 0) S->sel--;
                else if (k == 0x5000 && S->sel + 1 < S->nsaves) S->sel++;
                else if (k == 0x5300) delete_save(S, S->sel);        /* (Del: as the row's Delete) */
                continue;
            }
            if (k == 0x4800 || k == 0x5000) { ns_in.wheel += k == 0x4800 ? 1 : -1; continue; }   /* (the mouse wheel comes as Up / Down: the list) */
            int c = k & 0xff;
            if (S->fresh && (k == 0x0e08 || (c >= 32 && c < 127))) S->comment[0] = 0;   /* (the first key replaces the suggestion) */
            S->fresh = 0;
            int len = (int)strlen(S->comment);
            if (k == 0x0e08) { if (len) S->comment[len - 1] = 0; }
            else if (c >= 32 && c < 127 && len < SV_COMMENT_MAX) { S->comment[len] = (char)c; S->comment[len + 1] = 0; }
            S->caret_t = plat_ticks_ms();
        }
        NsAction a;
        while (!r && pop(&a)) {
            if (a.type == SVV_CANCEL) r = -1;
            else if (a.type == SVV_SELECT && loading) { S->sel = a.a; if (S->armed != a.a) S->armed = -1; }
            else if (a.type == SVV_SELECT && a.a >= 0 && a.a < S->nsaves) {      /* Save: the one to replace, or none again */
                S->sel = S->sel == a.a ? -1 : a.a;
                if (S->sel >= 0 && S->fresh) take_comment(S, &S->saves[S->sel]);
                if (S->sel < 0 && S->fresh) default_comment(S->comment, SV_COMMENT_MAX + 1);
            }
            else if (a.type == SVV_DELETE && loading) delete_save(S, a.a);
            else if (a.type == SVV_OK) { if (loading) { if (a.a >= 0 && a.a < S->nsaves) { S->sel = a.a; r = 1; } } else r = 1; }
        }
        if (unw_active) break;
    }
    S->open = 0;
    {   /* the key that closed it let go before the game goes on (its release came after the game's
           loop had found a key waiting that was no longer there: the travel loop waited) */
        u32 t0 = plat_ticks_ms();
        while (plat_ticks_ms() - t0 < 160) { dl_poll(); plat_sleep_ms(3); }
        { void bios_kbd_drop_pending(void); bios_kbd_drop_pending(); }   /* (a key still on its way to the game: not given to it) */
        ns_call(GF_flush_keys, 0);
    }
    fprintf(stderr, "saves: the page closed (%s)\n", r == 1 ? "chosen" : "cancelled");
    ns_leave();
    ns_input_begin();
    return r == 1;
}

int savesview_save(char *comment, int n, char *replace, int rn)
{
    if (rn > 0) replace[0] = 0;
    if (!sv_on || !ns_active()) return -1;                       /* (-1: the original's way) */
    default_comment(saves_model.comment, SV_COMMENT_MAX + 1);
    saves_model.fresh = 1;
    if (!ok_or_cancel(0)) return 0;
    char *c = saves_model.comment;
    while (*c == ' ') c++;
    snprintf(comment, (size_t)n, "%s", *c ? c : "Darklands");
    if (rn > 0 && saves_model.sel >= 0 && saves_model.sel < saves_model.nsaves) snprintf(replace, (size_t)rn, "%s", saves_model.saves[saves_model.sel].file);
    return 1;
}

int savesview_load(char *file, int n)
{
    if (!sv_on || !ns_active()) return -1;

    if (!ok_or_cancel(1)) return 0;
    if (saves_model.sel < 0 || saves_model.sel >= saves_model.nsaves) return 0;
    snprintf(file, (size_t)n, "%s", saves_model.saves[saves_model.sel].file);
    return 1;
}
