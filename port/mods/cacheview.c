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
void plat_hold(int ms);
#include "gamemodel.h"
#include "cacheview.h"
#include "keep.h"
int sheetview_shared(void);

CacheModel cache_model;
static int cv_on = 1;
static int cv_arg;
u32 plat_ticks_ms(void);

#define F_STORE 0x37f4                       /* cache_items, far 2e38: { int id; char quality; char count; } */

int cacheview_is_on(void) { return cv_on; }
int cacheview_shared_pack(void) { return cache_model.shared; }
int cacheview_every_inn(void) { return cache_model.every_inn; }
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
    c->nstore = 0;
    for (int i = 0; i < store_n(); i++) {
        if (gm_ammo_hidden() && gm_is_ammo(store_id(i))) continue;
        CvEntry *e = &c->store[c->nstore++];
        e->id = store_id(i); e->qual = store_q(i); e->count = store_c(i); e->src = i;
        e->weight = gm_itemdef_weight(e->id);
    }
    c->shared = sheetview_shared();
    if (c->shared) {                                          /* every member's potions, then the party's pack: the spares of all, by kind and quality */
        c->nitems = 0; c->room = 0;
        for (int m = 0; m < 5; m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            for (int i = 0; i < gm_item_count(m) && c->nitems < 200; i++) {
                GmItem g; if (!gm_item(m, i, &g) || !gm_is_potion(g.id)) continue;
                CvItem *it = &c->item[c->nitems++];
                it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
                it->equipped = 0; it->kept = keep_is(g.id, g.qual); it->potion = 1; it->owner = m;
            }
        }
        int first = c->nitems;
        for (int m = 0; m < 5; m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            if (able(m) && gm_item_count(m) < 64) c->room = 1;
            for (int i = 0; i < gm_item_count(m); i++) {
                GmItem g; int spare = gm_item_spare(m, i);
                if (spare <= 0 || !gm_item(m, i, &g) || gm_is_potion(g.id) || (gm_ammo_hidden() && gm_is_ammo(g.id))) continue;
                int k = first;
                while (k < c->nitems && !(c->item[k].id == g.id && c->item[k].qual == g.qual)) k++;
                if (k < c->nitems) { c->item[k].count += spare; continue; }
                if (c->nitems >= 200) continue;
                CvItem *it = &c->item[c->nitems++];
                it->idx = -1; it->id = g.id; it->qual = g.qual; it->count = spare; it->weight = g.weight;
                it->equipped = 0; it->kept = keep_is(g.id, g.qual); it->potion = 0; it->owner = -1;
            }
        }
        for (int a = first + 1; a < c->nitems; a++)           /* by kind, the better first */
            for (int b = a; b > first; b--) {
                CvItem *x = &c->item[b - 1], *y = &c->item[b];
                if (x->id < y->id || (x->id == y->id && x->qual >= y->qual)) break;
                CvItem t = *x; *x = *y; *y = t;
            }
    } else {
        int m = c->who;
        c->nitems = 0;
        for (int i = 0; i < gm_item_count(m) && i < 64; i++) {
            GmItem g; gm_item(m, i, &g);
            if (gm_ammo_hidden() && gm_is_ammo(g.id)) continue;
            CvItem *it = &c->item[c->nitems++];
            it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
            it->equipped = gm_item_equipped(m, i); it->kept = 0; it->potion = 0; it->owner = m;
        }
        c->room = gm_item_count(m) < 64;
    }
    c->seq++;
}

static int key(int k) { return ns_call(GF_cache_key, 1, k); }

/* whom a thing from the store goes to in the shared pack: whoever carries some already (it
   stacks), else whoever has the most free places (the member shown, on a tie) */
static int pack_target(int id, int q)
{
    int best = -1, bm = -1, cur = who_now();
    for (int k = 0; k < 5; k++) {
        if (!able(k) || !gm_room_for(k, id, q)) continue;
        int stacks = 0;
        for (int j = 0; j < gm_item_count(k) && !stacks; j++) { GmItem g; if (gm_item(k, j, &g) && g.id == id && g.qual == q && g.count < 255) stacks = 1; }
        int score = stacks ? 1000 : 2 * (64 - gm_item_count(k)) + (k == cur);
        if (score > best) { best = score; bm = k; }
    }
    return bm;
}

static int take(int i, int all)
{
    int m = who_now(), done = 0, shown = m, shared = sheetview_shared();
    if (i < 0 || i >= store_n()) return 0;
    int id = store_id(i), q = store_q(i);
    for (int t = 0; t < (all ? 255 : 1); t++) {
        if (i >= store_n() || store_id(i) != id || store_q(i) != q) break;   /* (that entry is gone) */
        if (shared) { m = pack_target(id, q); if (m < 0) break; g_ww(GV_cache_who, (u16)m); }   /* (a potion too: someone's own) */
        else if (!gm_room_for(m, id, q)) break;
        g_ww((u16)(GV_cache_side + 2 * m), 1); g_ww((u16)(GV_cache_top + 2 * m), (u16)i); g_ww((u16)(GV_cache_sel + 2 * m), 0);
        g_ww(GV_cache_n_shown, (u16)store_n());
        int before = store_units();
        key(0x2247);
        if (ns_unwinding) return done;
        if (store_units() >= before) break;
        done++;
    }
    if (shared) g_ww(GV_cache_who, (u16)shown);
    char nm[32]; gm_itemdef_name(id, nm, sizeof nm);
    char t[160];
    if (shared) {
        if (done && gm_is_potion(id)) snprintf(t, sizeof t, "%d %s to %s (drag it onto another member on the character sheet).", done, nm, cache_model.names[m]);
        else if (done) snprintf(t, sizeof t, "%d %s into the party's pack.", done, nm);
        else snprintf(t, sizeof t, "No room: every member already carries 64 kinds of items.");
    }
    else if (done) snprintf(t, sizeof t, "%s takes %d %s.", cache_model.names[m], done, nm);
    else snprintf(t, sizeof t, "%s has no room for another kind of item (64 at most).", cache_model.names[m]);
    note(t);
    return done;
}

static int put(int m, int idx, int all, int quiet)
{
    int done = 0, shown = who_now();
    GmItem first; if (m < 0 || m >= 5 || !gm_item(m, idx, &first)) return 0;
    g_ww(GV_cache_who, (u16)m);                               /* (the game's key acts for the member chosen) */
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
    g_ww(GV_cache_who, (u16)shown);
    if (!quiet) {
        char nm[32]; gm_itemdef_name(first.id, nm, sizeof nm);
        char t[160];
        if (done) snprintf(t, sizeof t, "%s stores %d %s.", cache_model.names[m], done, nm);
        else snprintf(t, sizeof t, "The %s stays with %s.", nm, cache_model.names[m]);
        note(t);
    }
    return done;
}

/* the shared pack: store one (or all the party's spare ones) of a kind, from whoever has one spare */
static int pack_put(int id, int q, int all, int quiet)
{
    int shown = who_now(), done = 0;
    for (int k = 0; k < (all ? 255 * 5 : 1); k++) {
        int m, rec;
        if (!gm_pack_find(shown, id, q, &m, &rec)) break;
        g_ww(GV_cache_who, (u16)m);
        g_ww((u16)(GV_cache_side + 2 * m), 0); g_ww((u16)(GV_cache_inv_top + 2 * m), (u16)rec); g_ww((u16)(GV_cache_inv_sel + 2 * m), 0);
        g_ww((u16)(GV_cache_n_items + 2 * m), (u16)gm_item_count(m));
        int before = store_units();
        key(0x1950);
        if (ns_unwinding) return done;
        if (store_units() <= before) break;
        done++;
    }
    g_ww(GV_cache_who, (u16)shown);
    if (!quiet) {
        char nm[32], t[160]; gm_itemdef_name(id, nm, sizeof nm);
        if (done) snprintf(t, sizeof t, "%d %s stored.", done, nm);
        else snprintf(t, sizeof t, "The store room is full: the %s stays in the party's pack.", nm);
        note(t);
    }
    return done;
}

static void put_away(void)                   /* everything the member does not have in use */
{
    if (sheetview_shared()) {                /* (the shared pack: all the party's spares but the kept ones) */
        int kinds = 0, units = 0, kept = 0;
        for (int r = 0; r < cache_model.nitems && !ns_unwinding; r++) {
            CvItem it = cache_model.item[r];
            if (it.kept || it.idx >= 0) { kept += it.kept; continue; }   /* (not what is kept, nor the member's potions) */
            int d = pack_put(it.id, it.qual, 1, 1);
            if (d) { kinds++; units += d; }
        }
        char t[160];
        if (units) snprintf(t, sizeof t, "Stored %d item%s (%d kind%s); potions%s and equipped items are not stored.", units, units == 1 ? "" : "s", kinds, kinds == 1 ? "" : "s", kept ? ", items marked Keep" : "");
        else snprintf(t, sizeof t, kept ? "Nothing to store: the party's pack holds only items marked Keep." : "Nothing to store: the party's pack is empty.");
        note(t);
        return;
    }
    int m = who_now(), kinds = 0, units = 0;
    for (int i = gm_item_count(m) - 1; i >= 0 && !ns_unwinding; i--) {
        if (gm_item_equipped(m, i)) continue;
        int d = put(m, i, 1, 1);
        if (d) { kinds++; units += d; }
    }
    char t[160];
    if (units) snprintf(t, sizeof t, "%s stores %d item%s (%d kind%s); equipped items stay with them.", cache_model.names[m], units, units == 1 ? "" : "s", kinds, kinds == 1 ? "" : "s");
    else snprintf(t, sizeof t, "Nothing to store: everything %s carries is equipped.", cache_model.names[m]);
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
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
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
        if (r) g_str((u16)(r >> 16), (u16)((u16)r + 0x26), cache_model.place, sizeof cache_model.place);
        /* (shared: a place with no store yet gets the shared one with its first P; one left with its
           own old store, too full to join, says its town) */
        int sharedstore_places(int loc), sharedstore_on(void), np = cur >= 0 ? sharedstore_places(cur) : 1;
        cache_model.every_inn = np > 1 || (np == 0 && sharedstore_on()); }
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
            case CV_TAKE: if (a.a >= 0 && a.a < cache_model.nstore) take(cache_model.store[a.a].src, a.b == 255); break;
            case CV_PUT: if (cache_model.shared) { if (a.a >= 0 && a.a < cache_model.nitems) {
                             const CvItem *it = &cache_model.item[a.a];
                             if (it->idx >= 0) put(it->owner, it->idx, a.b == 255, 0);   /* (a member's own potion) */
                             else pack_put(it->id, it->qual, a.b == 255, 0); } }
                         else if (a.a >= 0 && a.a < cache_model.nitems) put(who_now(), cache_model.item[a.a].idx, a.b == 255, 0); break;
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
