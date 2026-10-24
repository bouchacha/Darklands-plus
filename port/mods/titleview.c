/* titleview.c - the start screen as a native page (display.native_title; the view is
 * ui/titleview.cpp).
 *
 * The original (title_screen, s037 1e26:0008) shows pics\startscr.pic with three buttons -
 * Quickstart, Create a New World, The Story Continues - and passes their keys (0x1051, 0x2e43,
 * 0x1454; 0x1000 quits) to its handler (title_key, 1e26:0172) until that answers 0x11b: the game
 * begins. Here the same handler does every choice, with the setup and the leaving the
 * original's. Continue and Load are the handler's load screen (0x1454), told beforehand which
 * save to read (the newest; or the one chosen on the native Load page, mods/savesview.c), so the
 * game's own load runs (mods/saves.c: saves_title_load). */
#include "../ui/uitabs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "savesview.h"
#include "titleview.h"

void plat_sleep_ms(int ms);
void dl_poll(void);
void ui_open_tab(int t);

TitleModel title_model;
static int tv_on = 1;

int titleview_is_on(void) { return tv_on; }
void titleview_set(int on) { tv_on = on; config_set_int("display.native_title", on); }

static void build(void)
{
    TitleModel *m = &title_model;
    SaveInfo *v = 0;
    m->nsaves = saves_list(&v);
    if (m->nsaves > 0) m->last = v[0]; else memset(&m->last, 0, sizeof m->last);
    free(v);
    m->can_continue = m->nsaves > 0 && savesview_is_on() && saves_on;
    m->seq++;
}

static int choose(int key)                   /* the original's handler: 0x11b when the game begins */
{
    TitleModel *m = &title_model;
    m->busy = 1;
    int r = (u16)ns_call(GF_title_key, 1, key);
    m->busy = 0;
    if (!ns_unwinding) build();
    return r;
}

void tips_title_reached(void);               /* ui/tips.cpp: the New Player Guide's note (the welcome, the first time) */
static void screen_hook(void)                 /* title_screen (), s037 1e26:0008 */
{
    tips_title_reached();
    if (!tv_on || !ns_active()) { hook_call_original(); return; }
    TitleModel *m = &title_model;
    ns_unwinding = 0;
    ns_call(-1, 0x047c, 0x0568, 0);                                  /* (the original's setup) */
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    ns_call(GF_title_setup, 0);
    ns_call(GF_title_redraw, 0);
    ns_call(-1, 0x047c, 0x056f, 0);
    if (ns_unwinding) return;
    build();
    m->open = 1;
    fprintf(stderr, "title: open (%d saves)\n", m->nsaves);
    ns_enter(NS_TITLE);
    ns_clear();
    ns_input_begin();
    int begin = 0;
    while (!begin) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        if (ns_unwinding) break;
        while (!begin && ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0), c = k & 0xff;
            if (c == 13 && m->can_continue) ns_push(TV_CONTINUE, 0, 0, 0);
            else if (c == 13 && m->nsaves == 0) ns_push(TV_NEW, 0, 0, 0);          /* (no save yet: a new party is the way in) */
            else if (c == 'l' || c == 'L' || c == 't' || c == 'T') ns_push(TV_LOAD, 0, 0, 0);
            else if (c == 'c' || c == 'C' || c == 'n' || c == 'N') ns_push(TV_NEW, 0, 0, 0);
            else if (c == 'q' || c == 'Q') ns_push(TV_QUICK, 0, 0, 0);
            else if (k == 0x1000) ns_push(TV_QUIT, 0, 0, 0);
        }
        NsAction a;
        while (!begin && !ns_unwinding && ns_pop(&a)) {
            int r = 0;
            switch (a.type) {
            case TV_CONTINUE:
                if (!m->can_continue) break;
                fprintf(stderr, "title: continue with %s\n", m->last.file);
                saves_title_load(m->last.file);
                r = choose(0x1454);
                break;
            case TV_LOAD: {
                char f[16];
                if (m->nsaves == 0) break;                                              /* (nothing to load: the choice is greyed) */
                int k = m->nsaves > 0 && saves_on ? savesview_load(f, sizeof f) : -1;    /* (the native list: -1 off) */
                if (ns_unwinding) break;
                if (k == 1) { saves_title_load(f); r = choose(0x1454); }
                else if (k < 0) r = choose(0x1454);
                else build();                                                           /* (saves may have been deleted there) */
                break; }
            case TV_NEW: r = choose(0x2e43); break;
            case TV_QUICK: r = choose(0x1051); break;
            case TV_SETTINGS: ui_open_tab(UI_PAGE_HELPERS); break;
            case TV_QUIT: choose(0x1000); break;
            }
            if (r == 0x11b) begin = 1;
        }
    }
    m->open = 0;
    ns_leave();
    fprintf(stderr, "title: closed%s\n", begin ? " (the game begins)" : "");
    if (ns_unwinding) return;
    ns_call(GF_title_close, 0);
    hook_return_far(0);
}

void titleview_init(void)
{
    tv_on = config_get_int("display.native_title", 1);
    hook_install(GF_title_screen, screen_hook);
}
