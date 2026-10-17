/* cacheview.c - the store room as a native screen (nscreen.h; the view is ui/cacheview.cpp).
 *
 * The original (item_cache_screen, s035 1462:0000) shows four stored things and four of the
 * member's at a time and moves one thing per G or P. Here the setup and the leaving are the
 * original's (the same variables; cache_first_draw reads the store, cache_close writes it back)
 * and every move is the original's key handler (cache_key) with its cursors set to the row:
 * G (0x2247) takes one from the store, P (0x1950) puts one in; a stack is G or P repeated. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"
#include "cacheview.h"

CacheModel cache_model;
static int cv_on = 1;
static int cv_arg;
u32 plat_ticks_ms(void);

#define F_STORE 0x37f4                       /* cache_items, far 2e38: { int id; char quality; char count; } */

int cacheview_is_on(void) { return cv_on; }
void cacheview_set(int on) { cv_on = on; config_set_int("display.native_cache", on); }

static void note(const char *t) { snprintf(cache_model.note, sizeof cache_model.note, "%s", t); cache_model.note_t = plat_ticks_ms(); fprintf(stderr, "store room: %s\n", t); }

static int store_n(void) { int n = g_rw(GV_cache_n); return n < 0 ? 0 : n > 256 ? 256 : n; }
static int store_id(int i) { return (s16)rw(G_FAR2E38, (u16)(F_STORE + 4 * i)) & 0xfff; }
static int store_q(int i) { return rb(G_FAR2E38, (u16)(F_STORE + 4 * i + 2)); }
static int store_c(int i) { return rb(G_FAR2E38, (u16)(F_STORE + 4 * i + 3)); }
static int store_units(void) { int t = 0; for (int i = 0; i < store_n(); i++) t += store_c(i); return t; }
static int who_now(void) { return (s16)g_rw(GV_cache_who); }
static int able(int m) { int st = gm_member_status(m); return m < gm_party_size() && gm_member_present(m) && st != 0; }

static void build(void)
{
    CacheModel *c = &cache_model;
    c->who = who_now();
    c->npar = gm_party_size();
    for (int m = 0; m < 5; m++) { c->present[m] = gm_member_present(m); c->able[m] = able(m); gm_member_name(m, c->names[m], sizeof c->names[m]); }
    c->nstore = store_n();
    for (int i = 0; i < c->nstore; i++) {
        c->store[i].id = store_id(i); c->store[i].qual = store_q(i); c->store[i].count = store_c(i);
        c->store[i].weight = gm_itemdef_weight(c->store[i].id);
    }
    int m = c->who;
    c->nitems = gm_item_count(m);
    for (int i = 0; i < c->nitems && i < 64; i++) {
        GmItem g; gm_item(m, i, &g);
        CvItem *it = &c->item[i];
        it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
        it->equipped = gm_item_equipped(m, i);
    }
    c->room = c->nitems < 64;
    c->seq++;
}

static int key(int k) { return ns_call(GF_cache_key, 1, k); }

static int take(int i, int all)
{
    int m = who_now(), done = 0;
    if (i < 0 || i >= store_n()) return 0;
    int id = store_id(i), q = store_q(i);
    for (int t = 0; t < (all ? 255 : 1); t++) {
        if (i >= store_n() || store_id(i) != id || store_q(i) != q) break;   /* (that entry is gone) */
        if (!gm_room_for(m, id, q)) break;
        g_ww((u16)(GV_cache_side + 2 * m), 1); g_ww((u16)(GV_cache_top + 2 * m), (u16)i); g_ww((u16)(GV_cache_sel + 2 * m), 0);
        g_ww(GV_cache_n_shown, (u16)store_n());
        int before = store_units();
        key(0x2247);
        if (ns_unwinding) return done;
        if (store_units() >= before) break;
        done++;
    }
    char nm[32]; gm_itemdef_name(id, nm, sizeof nm);
    char t[160];
    if (done) snprintf(t, sizeof t, "%s takes %d %s.", cache_model.names[m], done, nm);
    else snprintf(t, sizeof t, "%s has no room for another kind of thing (64 at most).", cache_model.names[m]);
    note(t);
    return done;
}

static int put(int idx, int all, int quiet)
{
    int m = who_now(), done = 0;
    GmItem first; if (!gm_item(m, idx, &first)) return 0;
    for (int t = 0; t < (all ? 255 : 1); t++) {
        GmItem g;
        if (!gm_item(m, idx, &g) || g.id != first.id || g.qual != first.qual) break;
        g_ww((u16)(GV_cache_side + 2 * m), 0); g_ww((u16)(GV_cache_inv_top + 2 * m), (u16)idx); g_ww((u16)(GV_cache_inv_sel + 2 * m), 0);
        g_ww((u16)(GV_cache_n_items + 2 * m), (u16)gm_item_count(m));
        int before = store_units();
        key(0x1950);
        if (ns_unwinding) return done;
        if (store_units() <= before) break;
        done++;
    }
    if (!quiet) {
        char nm[32]; gm_itemdef_name(first.id, nm, sizeof nm);
        char t[160];
        if (done) snprintf(t, sizeof t, "%s stores %d %s.", cache_model.names[m], done, nm);
        else snprintf(t, sizeof t, "The %s stays with %s.", nm, cache_model.names[m]);
        note(t);
    }
    return done;
}

static void put_away(void)                   /* everything the member does not have in use */
{
    int m = who_now(), kinds = 0, units = 0;
    for (int i = gm_item_count(m) - 1; i >= 0 && !ns_unwinding; i--) {
        if (gm_item_equipped(m, i)) continue;
        int d = put(i, 1, 1);
        if (d) { kinds++; units += d; }
    }
    char t[160];
    if (units) snprintf(t, sizeof t, "%s stores %d things (%d kinds); what is in use stays.", cache_model.names[m], units, kinds);
    else snprintf(t, sizeof t, "%s has nothing that is not in use.", cache_model.names[m]);
    note(t);
}

static void set_who(int m)
{
    if (m < 0 || m >= 5 || !able(m)) return;
    g_ww(GV_cache_who, (u16)m);
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    ns_call(GF_cache_redraw, 2, m, cv_arg);                   /* (the original's picture, unseen) */
}

static void screen_hook(void)                 /* item_cache_screen(arg), s035 1462:0000 */
{
    if (!cv_on || !ns_active()) { hook_call_original(); return; }
    cv_arg = (s16)rw(SS, (u16)(SP + 4));
    ns_unwinding = 0;
    /* the original's setup (cache_screen's first lines) */
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    g_wb(GV_screen_kind, 5);
    int lead = gm_leader();
    g_ww(GV_cache_leader, (u16)lead); g_ww(GV_cache_who, (u16)lead);
    g_ww(GV_screen_win_colour, 8);
    g_ww(GV_cache_arg, (u16)cv_arg);
    ns_call(GF_screen_hold, 0);
    ns_call(GF_cache_first_draw, 2, lead, cv_arg);          /* (the store read) */
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) return;
    memset(cache_model.note, 0, sizeof cache_model.note);
    {   int cur = (s16)g_rw(G_CUR_LOC); u32 r = cur >= 0 ? g_location(cur) : 0;
        cache_model.place[0] = 0;
        if (r) g_str((u16)(r >> 16), (u16)((u16)r + 0x26), cache_model.place, sizeof cache_model.place); }
    build();
    cache_model.open = 1;
    fprintf(stderr, "store room: open (%d kinds stored)\n", cache_model.nstore);
    ns_enter(NS_CACHE);
    ns_clear();
    ns_input_begin();
    int leave = 0;
    while (!leave) {
        if (!ns_round()) break;
        int cx, cy;
        if (ns_in.click && ns_in.cx < 0x3c) { ns_take_click(&cx, &cy); set_who(cy / 40); build(); }
        if (ns_call(GF_key_pressed, 0)) {
            int k = ns_get_key();
            if (k == 0x11b || k == 0x264c || k == 0x266c) leave = 1;
            else if (k == 0x2e43 || k == 0x2e63) { int m = who_now(); for (int t = 1; t <= 5; t++) if (able((m + t) % 5)) { set_who((m + t) % 5); break; } build(); }
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
            else if (k == 0x1000) { ns_call(GF_cache_close, 1, -1); ns_call(GF_quit_to_dos, 0); }
        }
        if (ns_unwinding) break;
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            switch (a.type) {
            case CV_TAKE: take(a.a, a.b == 255); break;
            case CV_PUT: put(a.a, a.b == 255, 0); break;
            case CV_WHO: set_who(a.a); break;
            case CV_PUTAWAY: put_away(); break;
            case CV_LEAVE: leave = 1; break;
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    cache_model.open = 0;
    ns_leave();
    fprintf(stderr, "store room: closed (%d kinds stored)\n", store_n());
    if (ns_unwinding) return;
    ns_call(GF_cache_close, 1, -1);
    hook_return_far(0);
}

void cacheview_init(void)
{
    cv_on = config_get_int("display.native_cache", 1);
    hook_install(GF_item_cache_screen, screen_hook);
}
