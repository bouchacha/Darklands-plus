/* barterview.c - the shop screen as a native screen (nscreen.h).
 *
 * The original (s032 1462:0000 barter_screen, src/wip/s032/barter.c) shows four of the
 * merchant's goods and four of the member's things at a time, sells and buys one at a time
 * with P and S, and draws everything into the picture. Here:
 *   - setup and the merchant's stock are the original's own (the same variables set, then
 *     078e: the stock for today, seeded as the game seeds it; it also draws, unseen);
 *   - the model (mods/barterview.h) is read from the game's variables, with the prices from
 *     the game's own buy_price (192e) / sell_price (1b26), asked side-effect free;
 *   - buying and selling are the original's key handler (0f60) with its cursor variables set
 *     to the chosen line: P (0x1950) buys one, S (0x1f53) sells one; a stack is S repeated;
 *   - another member: D_884e and the original's redraw (0b30, unseen);
 *   - leaving: the original's 076c (fonts, leader back).
 * The barter variables (DGROUP): 884e the member bartering, 884c the leader on entry,
 * 869c far name, 8868 long categories, 8836 shop, 8834 quality, 86a2[] wares, 885c their
 * number, 86a0 first ware shown, 8832 ware cursor, 8842[5] 1 = cursor on the wares,
 * 8852[5] first own item shown, 885e[5] own cursor, 8838[5] own count. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"
#include "barterview.h"
#include "keep.h"

BarterModel barter_model;
static int bv_on = 1;
u32 plat_ticks_ms(void);

int barterview_on(void) { return bv_on && ns_active(); }
int barterview_is_on(void) { return bv_on; }
void barterview_set(int on) { bv_on = on; config_set_int("display.native_shops", on); }


static int g_side(int m) { return (s16)g_rw((u16)(GV_barter_side + 2 * m)); }
static void s_int(u16 a, int v) { g_ww(a, (u16)v); }

void money_cstr(long pf, char *out, size_t n);       /* ui/money.cpp */
static void note(const char *t) { snprintf(barter_model.note, sizeof barter_model.note, "%s", t); barter_model.note_t = plat_ticks_ms(); fprintf(stderr, "shop: %s\n", t); }

/* the model, from the game's variables; prices asked side-effect free (buy_price may draw a
 * random number in some places, as the original asks it at every redraw) */
static void build(void)
{
    BarterModel *b = &barter_model;
    b->shop = (s16)g_rw(GV_barter_shop);
    b->quality = (s16)g_rw(GV_barter_quality);
    { u16 o = g_rw(GV_barter_name), s = g_rw(GV_barter_name + 2); if (s) g_str(s, o, b->shopname, sizeof b->shopname); else b->shopname[0] = 0; }
    b->who = (s16)g_rw(GV_barter_who);
    b->payer = gm_leader();
    b->npar = gm_party_size();
    for (int m = 0; m < 5; m++) { b->present[m] = gm_member_present(m); gm_member_name(m, b->names[m], sizeof b->names[m]); }
    gm_money(&b->fl, &b->gr, &b->pf);
    b->total_pf = (long)b->fl * 240 + (long)b->gr * 12 + b->pf;
    int n = (s16)g_rw(GV_barter_nwares); if (n < 0) n = 0; if (n > 200) n = 200;
    b->nwares = n;
    int m = b->who, ni = gm_item_count(m);
    int sheetview_shared(void);
    b->shared = sheetview_shared();
    void *snap = eval_snapshot();
    for (int i = 0; i < n; i++) {
        BvWare *w = &b->ware[i];
        w->id = (s16)g_rw((u16)(GV_barter_wares + 2 * i));
        u16 a[2] = { (u16)w->id, 0 };
        w->price = (u16)game_call_link(GF_buy_price, 1, a);
        eval_restore(snap);
        w->qual = (s16)game_call_link(GF_ware_quality, 1, a);
        eval_restore(snap);
        w->weight = gm_itemdef_weight(w->id);
    }
    b->nitems = 0;
    for (int pass = 0; pass < (b->shared ? 2 : 1); pass++)   /* the member's things (shared: those in use, then their own potions) */
    for (int i = 0; i < ni; i++) {
        GmItem g; gm_item(m, i, &g);
        int eq = b->shared ? (gm_item_equipped(m, i) ? 1 : gm_item_riding(m, i) ? 2 : 0) : gm_item_equipped(m, i);
        if (b->shared && (pass == 0 ? !eq : (eq || !gm_is_potion(g.id)))) continue;
        BvItem *it = &b->item[b->nitems++];
        it->idx = i; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = b->shared && eq ? 1 : g.count; it->weight = g.weight;
        it->equipped = eq; it->holder = m; it->kept = keep_is(g.id, g.qual);
    }
    if (b->shared) {                                      /* then the party's pack: every member's spares by kind and quality */
        int first = b->nitems;
        for (int h = 0; h < 5; h++) {
            if (!gm_member_present(h) || gm_member_status(h) == 5) continue;
            for (int i = 0; i < gm_item_count(h); i++) {
                GmItem g; if (!gm_item(h, i, &g) || gm_is_potion(g.id)) continue;   /* (potions: each member's own) */
                int spare = gm_item_spare(h, i);
                if (spare <= 0) continue;
                int k = first;
                while (k < b->nitems && !(b->item[k].id == g.id && b->item[k].qual == g.qual)) k++;
                if (k == b->nitems) {
                    if (b->nitems >= 200) continue;
                    BvItem *it = &b->item[b->nitems++];
                    it->idx = -1; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = 0; it->weight = g.weight;
                    it->equipped = 0; it->holder = -1; it->kept = keep_is(g.id, g.qual);
                }
                b->item[k].count += spare;
            }
        }
        for (int a = first + 1; a < b->nitems; a++) {
            BvItem t = b->item[a]; int c = a - 1;
            while (c >= first && (b->item[c].id > t.id || (b->item[c].id == t.id && b->item[c].qual < t.qual))) { b->item[c + 1] = b->item[c]; c--; }
            b->item[c + 1] = t;
        }
    }
    for (int k = 0; k < b->nitems; k++) {
        BvItem *it = &b->item[k];
        if (gm_itemdef_no_trade(it->id)) it->price = 0;
        else { u16 a[2] = { (u16)it->id, (u16)it->qual }; it->price = (u16)game_call_link(GF_sell_price, 2, a); eval_restore(snap); }
    }
    eval_free(snap);
    b->seq++;
}

static long purse(void) { int f, g, p; gm_money(&f, &g, &p); return (long)f * 240 + g * 12 + p; }
static long batch_purse; static int batch_items, batch;

static int key(int k) { return ns_call(GF_barter_key, 1, k); }

static void buy(int i, int times)
{
    int who = (s16)g_rw(GV_barter_who), m = who, done = 0;
    int sheetview_shared(void);
    for (int t = 0; t < times; t++) {
        if (i < 0 || i >= (s16)g_rw(GV_barter_nwares)) break;
        if (sheetview_shared() && i < barter_model.nwares) {      /* the pack: whoever has room for it, the member of the tab first */
            int id = (s16)g_rw((u16)(GV_barter_wares + 2 * i)), q = barter_model.ware[i].qual;
            if (!gm_room_for(m, id, q)) for (int h = 0; h < 5; h++) if (gm_member_present(h) && gm_member_status(h) != 5 && gm_room_for(h, id, q)) { m = h; break; }
            s_int(GV_barter_who, m);
        }
        s_int((u16)(GV_barter_side + 2 * m), 1); s_int(GV_barter_ware_top, i); s_int(GV_barter_ware_cur, 0);
        long before = purse(); int nb = gm_item_count(m);
        key(0x1950);
        s_int(GV_barter_who, who);
        if (ns_unwinding) return;
        if (purse() == before && gm_item_count(m) == nb) break;
        done++;
    }
    char nm[32]; gm_itemdef_name((s16)g_rw((u16)(GV_barter_wares + 2 * i)), nm, sizeof nm);
    char t[120];
    if (!done) snprintf(t, sizeof t, i < barter_model.nwares && barter_model.ware[i].price > purse() ? "Not bought: the purse holds too little." : "Not bought: %s has no room for another kind of thing.", barter_model.names[m]);
    else snprintf(t, sizeof t, "Bought %d %s.", done, nm);
    note(t);
}

static void sell(int idx, int all)
{
    int m = (s16)g_rw(GV_barter_who), done = 0;
    GmItem first; if (!gm_item(m, idx, &first)) return;
    if (gm_itemdef_no_trade(first.id)) { note("The merchants of the Empire will not buy that."); return; }
    if (keep_is(first.id, first.qual)) { note("Kept: not sold (unmark it on the character sheet first)."); return; }
    for (int t = 0; t < (all ? 255 : 1); t++) {
        GmItem g;
        if (!gm_item(m, idx, &g) || g.id != first.id || g.qual != first.qual) break;   /* the stack is gone */
        s_int((u16)(GV_barter_side + 2 * m), 0); s_int((u16)(GV_barter_item_top + 2 * m), idx); s_int((u16)(GV_barter_item_cur + 2 * m), 0);
        s_int((u16)(GV_barter_item_count + 2 * m), gm_item_count(m));
        long before = purse();
        key(0x1f53);
        if (ns_unwinding) return;
        if (purse() == before) break;
        done++;
    }
    char nm[32]; gm_itemdef_name(first.id, nm, sizeof nm);
    char t[120];
    if (!done) snprintf(t, sizeof t, "Not sold: this merchant pays nothing for it.");
    else snprintf(t, sizeof t, "Sold %d %s.", done, nm);
    note(t);
}

/* the pack's (id, q): sold from whoever carries a spare, one at a time, as that member's own sale */
static void psell(int id, int q, int all)
{
    if (gm_itemdef_no_trade(id)) { note("The merchants of the Empire will not buy that."); return; }
    if (keep_is(id, q)) { note("Kept: not sold (unmark it on the character sheet first)."); return; }
    int who = (s16)g_rw(GV_barter_who), done = 0, h, i;
    for (int t = 0; t < (all ? 999 : 1); t++) {
        if (!gm_pack_find(who, id, q, &h, &i)) break;
        s_int(GV_barter_who, h);
        s_int((u16)(GV_barter_side + 2 * h), 0); s_int((u16)(GV_barter_item_top + 2 * h), i); s_int((u16)(GV_barter_item_cur + 2 * h), 0);
        s_int((u16)(GV_barter_item_count + 2 * h), gm_item_count(h));
        long before = purse();
        key(0x1f53);
        s_int(GV_barter_who, who);
        if (ns_unwinding) return;
        if (purse() == before) break;
        done++;
    }
    char nm[32]; gm_itemdef_name(id, nm, sizeof nm);
    char t[120];
    if (!done) snprintf(t, sizeof t, "Not sold: this merchant pays nothing for it.");
    else snprintf(t, sizeof t, "Sold %d %s.", done, nm);
    note(t);
}

static void set_who(int m)
{
    if (!gm_member_present(m)) return;
    s_int(GV_barter_who, m);
    ns_call(GF_barter_redraw, 5, m, g_rw(GV_barter_name), g_rw(GV_barter_name + 2), g_rw(GV_barter_cats), g_rw(GV_barter_cats + 2));   /* (the original's redraw, unseen) */
}

static void set_payer(int m)
{
    if (!gm_member_present(m) || gm_member_status(m) != 1) return;
    g_wb(G_LEADER, (u8)m);
    ns_call(GF_party_panel_rows, 1, 0x100);                    /* party_panel_rows */
}

/* barter_screen(shop, char far *name, long cats), natively: called from mods/loot.c's hook */
void barterview_run(void)
{
    int shop = (s16)rw(SS, (u16)(SP + 4));
    u16 name_o = rw(SS, (u16)(SP + 6)), name_s = rw(SS, (u16)(SP + 8));
    u16 cats_lo = rw(SS, (u16)(SP + 10)), cats_hi = rw(SS, (u16)(SP + 12));
    ns_unwinding = 0;
    /* the original's setup (barter_screen's first lines) */
    g_wb(GV_font_flag_a, 2); g_wb(GV_font_flag_b, 2);
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));   /* back/screen window fonts kept */
    g_ww(GV_back_win_font, 1); g_ww(GV_screen_win_font, 1);
    g_wb(GV_screen_kind, 5);
    int lead = gm_leader();
    s_int(GV_barter_leader, lead); s_int(GV_barter_who, lead);
    g_ww(GV_screen_win_colour, 8);                                          /* screen_win_colour */
    if (!name_s) { u16 ts = g_rw(GV_shop_names_seg); name_o = rw(ts, (u16)(0x207f + 4 * shop)); name_s = rw(ts, (u16)(0x207f + 4 * shop + 2)); }   /* F_290e_207f[shop] */
    g_ww(GV_barter_name, name_o); g_ww(GV_barter_name + 2, name_s);
    g_ww(GV_barter_cats, cats_lo); g_ww(GV_barter_cats + 2, cats_hi);
    s_int(GV_barter_shop, shop);
    if (shop < 0 || shop >= 9) s_int(GV_barter_quality, 25);
    else { u16 co = g_rw(GV_city_rec), cs = g_rw(0x9787);                /* city_rec */ s_int(GV_barter_quality, cs ? (s8)rb(cs, (u16)(co + 0x62 + shop)) : 25); }
    if (shop == -2) s_int(GV_barter_quality, 10);
    for (int r = 0; r < 5; r++) { s_int((u16)(GV_barter_side + 2 * r), 1); s_int((u16)(GV_barter_item_count + 2 * r), 0); s_int((u16)(GV_barter_item_top + 2 * r), 0); s_int((u16)(GV_barter_item_cur + 2 * r), 0); }
    s_int(GV_barter_nwares, 0); s_int(GV_barter_ware_top, 0); s_int(GV_barter_ware_cur, 0);
    ns_call(GF_screen_hold, 0);
    ns_call(GF_barter_draw, 5, lead, name_o, name_s, cats_lo, cats_hi);   /* today's stock (and the original's picture, unseen) */
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) return;
    memset(barter_model.note, 0, sizeof barter_model.note);
    build();
    barter_model.open = 1;
    fprintf(stderr, "shop: open (shop %d, %d wares)\n", shop, barter_model.nwares);
    ns_enter(NS_BARTER);
    ns_clear();
    ns_input_begin();
    int leave = 0;
    while (!leave) {
        if (!ns_round()) break;
        int cx, cy;
        if (ns_in.click && ns_in.cx < 0x3c) {                 /* a portrait: the member's sheet, as the original */
            ns_take_click(&cx, &cy);
            ns_nested = 1;
            ns_call(GF_member_sheet, 1, cy / 40);
            ns_nested = 0;
            if (ns_unwinding) break;
            g_ww(GV_party_icons_dirty, 1);
            ns_input_begin();
        }
        if (g_rw(GV_party_icons_dirty)) {                                   /* party_icons_dirty */                                   /* the party changed (a member's sheet ...) */
            g_ww(GV_party_icons_dirty, 0);
            ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
            build();
        }
        if (ns_call(GF_key_pressed, 0)) {                 /* keys: Esc / L leave, B the next member, Alt-Q */
            int k = ns_get_key();
            if (k == 0x11b || k == 0x264c || k == 0x266c) leave = 1;
            else if (k == 0x3042 || k == 0x3062) { int m = (s16)g_rw(GV_barter_who); for (int t = 1; t <= 5; t++) if (gm_member_present((m + t) % 5)) { set_who((m + t) % 5); break; } build(); }
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;   /* (the mouse wheel comes as Up / Down) */
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
            else if (k == 0x1000) { ns_call(GF_quit_to_dos, 0); }
            else if (k >= 0x5e00 && k <= 0x6200 && !(k & 0xff)) { set_payer((k >> 8) - 0x5e); build(); }
        }
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            switch (a.type) {
            case BV_BUY: buy(a.a, a.b > 0 ? a.b : 1); break;
            case BV_SELL: sell(a.a, a.b == 255); break;
            case BV_PSELL: psell(a.a, a.b, a.c == 255); break;
            case BV_WHO: set_who(a.a); break;
            case BV_PAYER: set_payer(a.a); break;
            case BV_LEAVE: leave = 1; break;
            case BV_BATCH_START: batch_purse = purse(); batch_items = 0; for (int h = 0; h < 5; h++) if (gm_member_present(h)) batch_items += gm_item_count(h); batch = 1; break;
            case BV_BATCH_END: {
                batch = 0;
                long got = purse() - batch_purse;
                char t[120];
                if (got <= 0) snprintf(t, sizeof t, "Nothing sold: this merchant pays nothing for those things.");
                else {
                    int now = 0; for (int h = 0; h < 5; h++) if (gm_member_present(h)) now += gm_item_count(h);
                    char mo[64]; money_cstr(got, mo, sizeof mo);
                    snprintf(t, sizeof t, "Sold the spare gear (%d kinds of things) for %s.", batch_items - now, mo);
                }
                note(t);
                break;
            }
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    barter_model.open = 0;
    fprintf(stderr, "shop: closed\n");
    ns_leave();
    if (ns_unwinding) return;
    AX = (u16)ns_call(GF_barter_close, 2, -1, shop);       /* fonts and leader back */
    hook_return_far(0);
}

void barterview_init(void) { bv_on = config_get_int("display.native_shops", 1); }
