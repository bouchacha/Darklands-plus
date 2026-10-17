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
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"
#include "lootview.h"

LootModel loot_model;
static int lv_on = 1;
u32 plat_ticks_ms(void);
int loot_enabled(void);

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
    l->npile = pile_n();
    for (int i = 0; i < l->npile; i++) {
        u16 o = pile_off(i);
        l->pile[i].id = rw(pile_seg(), o) & 0xfff;
        l->pile[i].qual = rb(pile_seg(), (u16)(o + 3));
        l->pile[i].count = rb(pile_seg(), (u16)(o + 4));
        l->pile[i].weight = gm_itemdef_weight(l->pile[i].id);
    }
    int m = l->who;
    l->nitems = gm_item_count(m);
    for (int i = 0; i < l->nitems; i++) {
        GmItem g; gm_item(m, i, &g);
        LvItem *it = &l->item[i];
        it->idx = i; it->id = g.id; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
        it->equipped = gm_item_equipped(m, i);
    }
    l->room = l->nitems < 64;
    l->assist = loot_enabled();
    l->seq++;
}

static int key(int k) { return ns_call(GF_loot_key, 1, k); }

static void take(int i, int all)
{
    int m = who_now(), done = 0;
    if (i < 0 || i >= pile_n()) return;
    u16 o = pile_off(i);
    int id = rw(pile_seg(), o) & 0xfff, q = rb(pile_seg(), (u16)(o + 3));
    char nm[32]; gm_itemdef_name(id, nm, sizeof nm);
    for (int t = 0; t < (all ? 255 : 1); t++) {
        if (i >= pile_n()) break;
        o = pile_off(i);
        if ((rw(pile_seg(), o) & 0xfff) != id || rb(pile_seg(), (u16)(o + 3)) != q) break;   /* (that entry is gone) */
        if (!gm_room_for(m, rw(pile_seg(), o), q)) break;
        g_ww((u16)(GV_loot_side + 2 * m), 1); g_ww((u16)(GV_loot_pile_top + 2 * m), (u16)i); g_ww((u16)(GV_loot_pile_sel + 2 * m), 0);
        int before = pile_units();
        key(0x2247);
        if (ns_unwinding) return;
        if (pile_units() >= before) break;
        done++;
    }
    char t[160];
    if (done) snprintf(t, sizeof t, "%s takes %d %s.", loot_model.names[m], done, nm);
    else snprintf(t, sizeof t, "%s has no room for another kind of thing (64 at most).", loot_model.names[m]);
    note(t);
}

static void put(int idx, int all)
{
    int m = who_now(), done = 0;
    GmItem first; if (!gm_item(m, idx, &first)) return;
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
            case LV_TAKE: take(a.a, a.b == 255); break;
            case LV_PUT: put(a.a, a.b == 255); break;
            case LV_WHO: set_who(a.a); break;
            case LV_TAKEALL: key(0x1e61); if (!ns_unwinding) note(pile_n() ? "Took what fits." : "Took everything."); break;
            case LV_SHARE: key(0x1f73); if (!ns_unwinding) note(pile_n() ? "Shared out what fits." : "Shared out everything."); break;
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
