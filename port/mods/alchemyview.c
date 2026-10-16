/* alchemyview.c - the alchemy screen as a native screen (nscreen.h; the view is ui/alchemyview.cpp).
 *
 * The original (alchemy_screen, s033 1462:0000) lists the formulas the alchemist knows; U opens a
 * formula's page with one batch, M / F add and take batches (their ingredients reserved as they
 * go), S starts them, Esc goes back (the reservation undone). Here:
 *   - the setup is the original's: fonts kept, DARKLAND.ALC loaded with the game's own mem_alloc
 *     and file_load_far into the same table, then alchemy_setup (the rows, the party's ingredients,
 *     a batch under way taken back to be set again) and the original's picture, unseen;
 *   - choosing a formula is the original's Esc on the page shown (if any), the row set, then U;
 *   - more / fewer are the page's own M and F; Start is alchemy_brew, as the original's S;
 *   - leaving: Esc's undoing on a page, alchemy_resume, alchemy_close.
 * The chance of success is reckoned as the original's page shows it. */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"
#include "alchemyview.h"

AlchemyModel alchemy_model;
static int av_on = 1;
u32 plat_ticks_ms(void);
void dos_host_path_read(const char *dos, char *out, size_t outn);

int alchemyview_is_on(void) { return av_on; }
void alchemyview_set(int on) { av_on = on; config_set_int("display.native_alchemy", on); }

static void note(const char *t) { snprintf(alchemy_model.note, sizeof alchemy_model.note, "%s", t); alchemy_model.note_t = plat_ticks_ms(); fprintf(stderr, "alchemy: %s\n", t); }

#define ROWS_SEG ((u16)(LOADSEG + 0x28f5))           /* the rows: {formula, count, count under way} */
#define S_ALC_MEM 0x321d                             /* the original's strings (s033 _DATA): "alc mem" */
#define S_ALC_FILE 0x3225                            /* "darkland.alc" */
static int row_formula(int r) { return (s16)rw(ROWS_SEG, (u16)(6 * r)); }
static int row_count(int r) { return (s16)rw(ROWS_SEG, (u16)(6 * r + 2)); }
static int row_resumed(int r) { return (s16)rw(ROWS_SEG, (u16)(6 * r + 4)); }
static void set_row_count(int r, int n) { ww(ROWS_SEG, (u16)(6 * r + 2), (u16)n); }
static u32 formula_ptr(int f) { return ((u32)rw(G_FAR2E38, (u16)(0x35e4 + 4 * f + 2)) << 16) | rw(G_FAR2E38, (u16)(0x35e4 + 4 * f)); }
static int have(int item) { return (s16)g_rw((u16)(GV_alchemy_have + 2 * item)); }

static void build(void)
{
    AlchemyModel *a = &alchemy_model;
    int who = (s16)g_rw(GV_alchemy_who);
    a->who = who;
    gm_member_name(who, a->name, sizeof a->name);
    a->nrows = (s16)g_rw(GV_alchemy_nrows); if (a->nrows < 0) a->nrows = 0; if (a->nrows > 66) a->nrows = 66;
    a->sel = (s16)g_rw(GV_alchemy_row);
    a->page = (s16)g_rw(GV_alchemy_page) == 1;
    for (int r = 0; r < a->nrows; r++) {
        AvRow *w = &a->row[r];
        w->formula = row_formula(r); w->count = row_count(r);
        u32 p = formula_ptr(w->formula); w->can = p != 0;
        for (int i = 0; p && i < 5; i++) {
            int c = (s8)rb((u16)(p >> 16), (u16)((u16)p + 0x54 + 4 * i)), it = (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x56 + 4 * i));
            if (!c) break;
            if (have(it) < c) w->can = 0;
        }
    }
    a->ningr = 0; a->count = 0;
    if (a->sel >= 0 && a->sel < a->nrows) {
        int f = row_formula(a->sel);
        u32 p = formula_ptr(f);
        a->count = row_count(a->sel);
        u16 b = G_MEMBER(who);
        a->intel = (s8)g_rb((u16)(b + G_M_ATTR_CUR + 4));
        a->alch = (s8)g_rb((u16)(b + G_M_SKILLS + 7));
        a->stone = (s16)g_rw(GV_philosophers_stone);
        a->mystic = p ? (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x50)) : 0;
        a->risk = p ? (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x52)) : 0;
        int n = a->count, m = n > 2 ? 100 / (n >> 1) : 100;        /* (as alchemy_draw_page and alchemy_brew) */
        a->base = m;
        int i = a->intel + (a->alch - a->mystic) + a->stone + m;
        a->pct = i < 0 || n == 0 ? 0 : i > 99 ? 99 : i;
        int i1 = a->intel + (a->alch - a->mystic) + a->stone + 100;
        a->pct1 = i1 < 0 ? 0 : i1 > 99 ? 99 : i1;
        a->can_more = 1;
        for (int k = 0; p && k < 5; k++) {
            int c = (s8)rb((u16)(p >> 16), (u16)((u16)p + 0x54 + 4 * k)), it = (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x56 + 4 * k));
            if (!c) break;
            a->ingr[a->ningr].item = it; a->ingr[a->ningr].need = c; a->ingr[a->ningr].have = have(it);
            if (have(it) < c) a->can_more = 0;
            a->ningr++;
        }
    }
    a->seq++;
}

/* the original's Esc on a formula's page: the batches back to what was under way */
static void page_back(void)
{
    if ((s16)g_rw(GV_alchemy_page) != 1) return;
    int r = (s16)g_rw(GV_alchemy_row), f = row_formula(r), k = row_count(r) - row_resumed(r);
    for (; k > 0 && !ns_unwinding; k--) ns_call(GF_alchemy_unreserve, 1, f);
    for (; k < 0 && !ns_unwinding; k++) ns_call(GF_alchemy_reserve, 1, f);
    set_row_count(r, row_resumed(r));
    g_ww(GV_alchemy_page, 0);
}

static void choose(int r)
{
    if (r < 0 || r >= (s16)g_rw(GV_alchemy_nrows)) return;
    page_back();
    g_ww(GV_alchemy_row, (u16)r);
    g_ww(GV_alchemy_top, (u16)(r < 17 ? r : 16));
    ns_call(GF_alchemy_list_key, 1, 0x1655);                 /* U: its page, one batch if the ingredients are there */
}

static int prev_screen;
static void screen_hook(void)                /* alchemy_screen(who, hours), s033 1462:0000 */
{
    int who = (s16)rw(SS, (u16)(SP + 4)), hours = (s16)rw(SS, (u16)(SP + 6));
    char host[512]; struct stat st;
    dos_host_path_read("darkland.alc", host, sizeof host);
    if (!av_on || !ns_active() || who < 0 || who > 4 || stat(host, &st) != 0 || st.st_size < 1 || st.st_size > 0x8000) { hook_call_original(); return; }
    /* the original's setup */
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    g_wb(GV_screen_kind, 4);
    g_ww(GV_alchemy_who, (u16)who);
    g_ww(GV_alchemy_row, 0); g_ww(GV_alchemy_top, 0);
    u16 seg = (u16)ns_call(GF_mem_alloc, 3, (int)(st.st_size & 0xffff), (int)(st.st_size >> 16), S_ALC_MEM);
    if (ns_unwinding) return;
    g_ww(GV_alchemy_file, 0); g_ww(GV_alchemy_file + 2, seg);
    ns_call(GF_file_load_far, 3, S_ALC_FILE, 0, seg);
    int n = rb(seg, 0);
    for (int i = 0; i < n && i < 66; i++) { ww(G_FAR2E38, (u16)(0x35e4 + 4 * i), (u16)(1 + i * 0x68)); ww(G_FAR2E38, (u16)(0x35e4 + 4 * i + 2), seg); }
    ns_call(GF_screen_hold, 0);
    ns_call(GF_alchemy_setup, 1, who);
    ns_call(GF_alchemy_draw_list, 1, who);
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) return;
    memset(alchemy_model.note, 0, sizeof alchemy_model.note);
    if ((s16)g_rw(GV_alchemy_nrows) > 0) choose(0);
    build();
    alchemy_model.open = 1;
    fprintf(stderr, "alchemy: open (member %d, %d formulae)\n", who, alchemy_model.nrows);
    prev_screen = ns_screen;
    ns_screen = NS_ALCHEMY;
    ns_clear();
    ns_input_begin();
    int leave = 0, started = 0;
    while (!leave) {
        if (!ns_round()) break;
        if (g_rw(GV_party_icons_dirty)) {
            g_ww(GV_party_icons_dirty, 0);
            ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
            build();
        }
        if (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (k == 0x11b || k == 0x2044 || k == 0x2064) leave = 1;
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;
            else if (k == 0x1000) { ns_call(GF_alchemy_close, 1, -1); ns_call(GF_quit_to_dos, 0); }
        }
        if (ns_unwinding) break;
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            switch (a.type) {
            case AV_SELECT: choose(a.a); break;
            case AV_MORE:
                if ((s16)g_rw(GV_alchemy_page) != 1) choose((s16)g_rw(GV_alchemy_row));
                else ns_call(GF_alchemy_page_key, 1, 0x324d);
                break;
            case AV_FEWER: if ((s16)g_rw(GV_alchemy_page) == 1) ns_call(GF_alchemy_page_key, 1, 0x2146); break;
            case AV_START: {
                int r = (s16)g_rw(GV_alchemy_row);
                if ((s16)g_rw(GV_alchemy_page) != 1 || row_count(r) <= 0) { note("Choose a formula and at least one batch first."); break; }
                char nm[32]; gm_member_name(who, nm, sizeof nm);
                char t[120]; snprintf(t, sizeof t, "%s starts %d batches.", nm, row_count(r));
                ns_call(GF_alchemy_brew, 1, hours);
                if (!ns_unwinding) { note(t); started = 1; leave = 1; }
                break; }
            case AV_LEAVE: leave = 1; break;
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    alchemy_model.open = 0;
    ns_screen = prev_screen;
    fprintf(stderr, "alchemy: closed\n");
    if (ns_unwinding) return;
    if (!started) { page_back(); ns_call(GF_alchemy_resume, 0); }
    AX = (u16)ns_call(GF_alchemy_close, 1, -1);
    hook_return_far(0);
}

void alchemyview_init(void)
{
    av_on = config_get_int("display.native_alchemy", 1);
    hook_install(33, 0x1462, 0x0000, screen_hook);
}
