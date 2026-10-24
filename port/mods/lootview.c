/* lootview.c - the loot screen as a native screen (nscreen.h; the view is ui/lootview.cpp).
 *
 * The original (loot_loop, s040 1462:01c8) shows four things of the pile and four of the member
 * at a time, and moves one thing per G or P. Here the setup and the leaving are the original's
 * (the same variables, the cash added to the purse as it adds it, loot_first_draw, loot_close),
 * and every move is the original's key handler (loot_key) with its cursors set to the row:
 * G (0x2247) takes one from the pile, P (0x1950) puts one back; a stack is G or P repeated.
 * Take all and Share out are mods/loot.c's A and S, through the same key handler (they work
 * through G too). The thrown weapons go back to their throwers on the first draw (mods/loot.c). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
void plat_hold(int ms);
#include "gamemodel.h"
#include "lootview.h"
#include "keep.h"

LootModel loot_model;
static int lv_on = 1;
u32 plat_ticks_ms(void);
int loot_enabled(void);
int sheetview_shared(void);

int lootview_is_on(void) { return lv_on; }
void lootview_set(int on) { lv_on = on; config_set_int("display.native_loot", on); }

static void note(const char *t) { snprintf(loot_model.note, sizeof loot_model.note, "%s", t); loot_model.note_t = plat_ticks_ms(); fprintf(stderr, "loot screen: %s\n", t); }

static u16 pile_seg(void) { return g_rw(GV_loot_pile + 2); }
static u16 pile_off(int i) { return (u16)(g_rw(GV_loot_pile) + 6 * i); }
static int pile_n(void) { int n = (s16)g_rw(GV_loot_n); return n < 0 ? 0 : n > 200 ? 200 : n; }
static int pile_units(void) { int t = 0; for (int i = 0; i < pile_n(); i++) t += rb(pile_seg(), (u16)(pile_off(i) + 4)); return t; }
static int who_now(void) { return (s16)g_rw(GV_loot_who); }
static int able(int m) { int st = gm_member_status(m); return m < gm_party_size() && st != 0 && st != 3 && (s8)g_rb((u16)(GV_cbt_floor + m)) != -99; }

static void build(void)
{
    LootModel *l = &loot_model;
    l->who = who_now();
    l->npar = gm_party_size();
    for (int m = 0; m < 5; m++) { l->present[m] = gm_member_present(m); l->able[m] = able(m); gm_member_name(m, l->names[m], sizeof l->names[m]); }
    l->npile = 0;
    for (int i = 0; i < pile_n(); i++) {
        u16 o = pile_off(i);
        int id = rw(pile_seg(), o) & 0xfff;
        if (gm_ammo_hidden() && gm_is_ammo(id)) continue;
        LvPile *p = &l->pile[l->npile++];
        p->id = id; p->src = i;
        p->qual = rb(pile_seg(), (u16)(o + 3));
        p->count = rb(pile_seg(), (u16)(o + 4));
        p->weight = gm_itemdef_weight(id);
    }
    l->shared = sheetview_shared();
    if (l->shared) {                                          /* every member's potions, then the party's pack: the spares of all, by kind and quality */
        l->nitems = 0; l->room = 0;
        for (int m = 0; m < 5; m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            for (int i = 0; i < gm_item_count(m) && l->nitems < 200; i++) {
                GmItem g; if (!gm_item(m, i, &g) || !gm_is_potion(g.id)) continue;
                LvItem *it = &l->item[l->nitems++];
                it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
                it->equipped = 0; it->kept = keep_is(g.id, g.qual); it->potion = 1; it->owner = m;
            }
        }
        int first = l->nitems;
        for (int m = 0; m < 5; m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            if (able(m) && gm_item_count(m) < 64) l->room = 1;
            for (int i = 0; i < gm_item_count(m); i++) {
                GmItem g; int spare = gm_item_spare(m, i);
                if (spare <= 0 || !gm_item(m, i, &g) || gm_is_potion(g.id) || (gm_ammo_hidden() && gm_is_ammo(g.id))) continue;
                int k = first;
                while (k < l->nitems && !(l->item[k].id == g.id && l->item[k].qual == g.qual)) k++;
                if (k < l->nitems) { l->item[k].count += spare; continue; }
                if (l->nitems >= 200) continue;
                LvItem *it = &l->item[l->nitems++];
                it->idx = -1; it->id = g.id; it->qual = g.qual; it->count = spare; it->weight = g.weight;
                it->equipped = 0; it->kept = keep_is(g.id, g.qual); it->potion = 0; it->owner = -1;
            }
        }
        for (int a = first + 1; a < l->nitems; a++)           /* by kind, the better first */
            for (int b = a; b > first; b--) {
                LvItem *x = &l->item[b - 1], *y = &l->item[b];
                if (x->id < y->id || (x->id == y->id && x->qual >= y->qual)) break;
                LvItem t = *x; *x = *y; *y = t;
            }
    } else {
        int m = l->who;
        l->nitems = 0;
        for (int i = 0; i < gm_item_count(m); i++) {
            GmItem g; gm_item(m, i, &g);
            if (gm_ammo_hidden() && gm_is_ammo(g.id)) continue;
            LvItem *it = &l->item[l->nitems++];
            it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
            it->equipped = gm_item_equipped(m, i); it->kept = 0; it->potion = 0; it->owner = m;
        }
        l->room = gm_item_count(m) < 64;
    }
    l->assist = loot_enabled();
    l->seq++;
}

static int key(int k) { return ns_call(GF_loot_key, 1, k); }

/* whom a thing from the pile goes to in the shared pack: whoever carries some already (it stacks),
   else whoever has the most free places (the member shown, on a tie); as Share out (mods/loot.c) */
static int pack_target(int id, int q)
{
    int best = -1, bm = -1, cur = who_now();
    for (int k = 0; k < 5; k++) {
        if (!gm_member_present(k) || !able(k) || !gm_room_for(k, id, q)) continue;
        int stacks = 0;
        for (int j = 0; j < gm_item_count(k) && !stacks; j++) { GmItem g; if (gm_item(k, j, &g) && g.id == (id & 0xfff) && g.qual == q && g.count < 255) stacks = 1; }
        int score = stacks ? 1000 : 2 * (64 - gm_item_count(k)) + (k == cur);
        if (score > best) { best = score; bm = k; }
    }
    return bm;
}

static void take(int i, int all)
{
    int m = who_now(), done = 0, shown = m, shared = sheetview_shared();
    if (i < 0 || i >= pile_n()) return;
    u16 o = pile_off(i);
    int id = rw(pile_seg(), o) & 0xfff, q = rb(pile_seg(), (u16)(o + 3));
    char nm[32]; gm_itemdef_name(id, nm, sizeof nm);
    for (int t = 0; t < (all ? 255 : 1); t++) {
        if (i >= pile_n()) break;
        o = pile_off(i);
        if ((rw(pile_seg(), o) & 0xfff) != id || rb(pile_seg(), (u16)(o + 3)) != q) break;   /* (that entry is gone) */
        if (shared) { m = pack_target(rw(pile_seg(), o), q); if (m < 0) break; g_ww(GV_loot_who, (u16)m); }   /* (a potion too: someone's own) */
        else if (!gm_room_for(m, rw(pile_seg(), o), q)) break;
        g_ww((u16)(GV_loot_side + 2 * m), 1); g_ww((u16)(GV_loot_pile_top + 2 * m), (u16)i); g_ww((u16)(GV_loot_pile_sel + 2 * m), 0);
        int before = pile_units();
        key(0x2247);
        if (ns_unwinding) return;
        if (pile_units() >= before) break;
        done++;
    }
    if (shared) g_ww(GV_loot_who, (u16)shown);
    char t[160];
    if (shared) {
        if (done && gm_is_potion(id)) snprintf(t, sizeof t, "%d %s to %s (drag it onto another member on the character sheet).", done, nm, loot_model.names[m]);
        else if (done) snprintf(t, sizeof t, "%d %s into the party's pack.", done, nm);
        else snprintf(t, sizeof t, "No room: every member already carries 64 kinds of items.");
    }
    else if (done) snprintf(t, sizeof t, "%s takes %d %s.", loot_model.names[m], done, nm);
    else snprintf(t, sizeof t, "%s has no room for another kind of item (64 at most).", loot_model.names[m]);
    note(t);
}

/* the shared pack: put back one (or all the party's spare ones) of pack row r, from whoever has one spare */
static void pack_put(int r, int all)
{
    if (r < 0 || r >= loot_model.nitems) return;
    int id = loot_model.item[r].id, q = loot_model.item[r].qual, shown = who_now(), done = 0;
    char nm[32], t[160]; gm_itemdef_name(id, nm, sizeof nm);
    if (keep_is(id, q)) { snprintf(t, sizeof t, "The %s is marked Keep on the character sheet: it stays in the party's pack.", nm); note(t); return; }
    for (int k = 0; k < (all ? 255 * 5 : 1); k++) {
        int m, rec;
        if (!gm_pack_find(shown, id, q, &m, &rec)) break;
        g_ww(GV_loot_who, (u16)m);
        g_ww((u16)(GV_loot_side + 2 * m), 0); g_ww((u16)(GV_loot_inv_top + 2 * m), (u16)rec); g_ww((u16)(GV_loot_inv_sel + 2 * m), 0);
        g_ww((u16)(GV_loot_n_items + 2 * m), (u16)gm_item_count(m));
        int before = pile_units();
        key(0x1950);
        if (ns_unwinding) return;
        if (pile_units() <= before) break;
        done++;
    }
    g_ww(GV_loot_who, (u16)shown);
    if (done) snprintf(t, sizeof t, "%d %s back into the pile.", done, nm);
    else snprintf(t, sizeof t, "The %s stays in the party's pack.", nm);
    note(t);
}

static void put(int m, int idx, int all)
{
    int done = 0, shown = who_now();
    GmItem first; if (m < 0 || m >= 5 || !gm_item(m, idx, &first)) return;
    g_ww(GV_loot_who, (u16)m);                                /* (the game's key acts for the member chosen) */
    char nm[32]; gm_itemdef_name(first.id, nm, sizeof nm);
    for (int t = 0; t < (all ? 255 : 1); t++) {
        GmItem g;
        if (!gm_item(m, idx, &g) || g.id != first.id || g.qual != first.qual) break;
        g_ww((u16)(GV_loot_side + 2 * m), 0); g_ww((u16)(GV_loot_inv_top + 2 * m), (u16)idx); g_ww((u16)(GV_loot_inv_sel + 2 * m), 0);
        g_ww((u16)(GV_loot_n_items + 2 * m), (u16)gm_item_count(m));
        int before = pile_units();
        key(0x1950);
        if (ns_unwinding) return;
        if (pile_units() <= before) break;
        done++;
    }
    g_ww(GV_loot_who, (u16)shown);
    char t[160];
    if (done) snprintf(t, sizeof t, "%s puts %d %s into the pile.", loot_model.names[m], done, nm);
    else snprintf(t, sizeof t, "The %s stays with %s.", nm, loot_model.names[m]);
    note(t);
}

static void set_who(int m)
{
    if (m < 0 || m >= 5 || !able(m)) return;
    g_ww(GV_loot_who, (u16)m);
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    ns_call(GF_loot_redraw, 1, m);                            /* (the original's picture, unseen) */
}

static void loop_hook(void)                  /* loot_loop(items far, count near, cash far), s040 1462:01c8 */
{
    if (!lv_on || !ns_active()) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    u16 items_o = rw(SS, (u16)(SP + 4)), items_s = rw(SS, (u16)(SP + 6)), count = rw(SS, (u16)(SP + 8));
    u16 cash_o = rw(SS, (u16)(SP + 10)), cash_s = rw(SS, (u16)(SP + 12));
    ns_unwinding = 0;
    /* the original's setup (loot_loop's first lines) */
    g_ww(GV_loot_n, 0);
    for (int i = 0; i < 5; i++) {
        g_ww((u16)(GV_loot_n_items + 2 * i), 0); g_ww((u16)(GV_loot_side + 2 * i), 1);
        g_ww((u16)(GV_loot_inv_top + 2 * i), 0); g_ww((u16)(GV_loot_pile_top + 2 * i), 0);
        g_ww((u16)(GV_loot_inv_sel + 2 * i), 0); g_ww((u16)(GV_loot_pile_sel + 2 * i), 0);
    }
    ww(cash_s, cash_o, 0);                                    /* (the original drops the florins) */
    for (int k = 1; k <= 2; k++) { int v = (s16)rw(cash_s, (u16)(cash_o + 2 * k)); ww(cash_s, (u16)(cash_o + 2 * k), (u16)(v < 0 ? 0 : v > 10 ? 10 : v)); }
    g_ww(GV_loot_pile, items_o); g_ww(GV_loot_pile + 2, items_s);
    g_ww(GV_loot_count_ptr, count); g_ww(GV_loot_count_ptr + 2, G_DGROUP);   /* (a near pointer made far) */
    g_ww(GV_loot_cash, cash_o); g_ww(GV_loot_cash + 2, cash_s);
    int lead = gm_leader();
    g_ww(GV_loot_who, (u16)lead); g_ww(GV_loot_leader, (u16)lead);
    g_ww(GV_screen_win_colour, 8);
    loot_model.cash_fl = (s16)rw(cash_s, cash_o); loot_model.cash_gr = (s16)rw(cash_s, (u16)(cash_o + 2)); loot_model.cash_pf = (s16)rw(cash_s, (u16)(cash_o + 4));
    ns_call(GF_money_add_fgp, 4, GV_party_money, loot_model.cash_fl, loot_model.cash_gr, loot_model.cash_pf);
    ns_call(GF_screen_hold, 0);
    ns_call(GF_loot_first_draw, 1, lead);                     /* (and the thrown weapons back: mods/loot.c) */
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) return;
    memset(loot_model.note, 0, sizeof loot_model.note);
    build();
    loot_model.open = 1;
    fprintf(stderr, "loot screen: open (%d in the pile)\n", loot_model.npile);
    if (getenv("DL_LOOT_LOG")) {                              /* the pile's records as the game gave them (item, type, quality, count, +5) */
        int n = (s16)g_rw(count);
        for (int k = 0; k < n && k < 200; k++) { u16 o = (u16)(items_o + 6 * k);
            fprintf(stderr, "loot pile %d: item %#06x %02x q%d x%d %02x\n", k, rw(items_s, o), rb(items_s, (u16)(o + 2)), rb(items_s, (u16)(o + 3)), rb(items_s, (u16)(o + 4)), rb(items_s, (u16)(o + 5))); }
    }
    ns_enter(NS_LOOT);
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
            else if (k == 0x2044 || k == 0x2064) { int m = who_now(); for (int t = 1; t <= 5; t++) if (able((m + t) % 5)) { set_who((m + t) % 5); break; } build(); }
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
            else if (k == 0x1000) { ns_call(GF_loot_close, 1, -1); ns_call(GF_quit_to_dos, 0); }
            else if ((k & 0xff) == 'a' || (k & 0xff) == 'A' || (k & 0xff) == 's' || (k & 0xff) == 'S') { key(k); build(); }
        }
        if (ns_unwinding) break;
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            switch (a.type) {
            case LV_TAKE: if (a.a >= 0 && a.a < loot_model.npile) take(loot_model.pile[a.a].src, a.b == 255); break;
            case LV_PUT:                                      /* (shared: a potion's row is its owner's to put back) */
                if (a.a < 0 || a.a >= loot_model.nitems) break;
                if (loot_model.item[a.a].idx < 0) pack_put(a.a, a.b == 255);
                else put(loot_model.shared ? loot_model.item[a.a].owner : who_now(), loot_model.item[a.a].idx, a.b == 255);
                break;
            case LV_WHO: set_who(a.a); break;
            case LV_TAKEALL: key(0x1e61); if (!ns_unwinding) note(pile_n() ? "Took what fits." : "Took everything."); break;
            case LV_SHARE:
                if (loot_model.shared) {                      /* everything into the pack (a potion to whoever carries the same, or has the most room) */
                    for (int i = 0; i < pile_n() && !ns_unwinding; ) { int n0 = pile_n(), u0 = pile_units(); take(i, 1); if (pile_n() >= n0 && pile_units() >= u0) i++; }
                    if (!ns_unwinding) note(pile_n() ? "Took what fits." : "Took everything.");
                } else { key(0x1f73); if (!ns_unwinding) note(pile_n() ? "Shared out what fits." : "Shared out everything."); }
                break;
            case LV_LEAVE: leave = 1; break;
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    loot_model.open = 0;
    ns_leave();
    fprintf(stderr, "loot screen: closed (%d left)\n", pile_n());
    if (ns_unwinding) return;
    ns_call(GF_loot_close, 1, -1);
    hook_return_far(0);
}

void lootview_init(void)
{
    lv_on = config_get_int("display.native_loot", 1);
    hook_install(40, 0x1462, 0x01c8, loop_hook);
}
