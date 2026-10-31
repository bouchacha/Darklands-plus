/* prayview.c - the prayer screen as a native screen (display.native_pray; the view is
 * ui/prayview.cpp).
 *
 * The original (saint_screen, s034 1462:022c) shows the saint's statue and legend and four lines:
 * more favor, less favor, pray now (for the party, or for a member picked from a box of names),
 * don't pray; it returns the chance of success (the saint's handler asked with what = 7) or -1.
 * Here the same: the same variables set as the original sets them (pray_df, pray_target,
 * pray_choose), the handler asked through its far pointer (far 290e:2937) for the chance, the
 * default favor (2), the flags (6), the statue (1) and the day (0); saint_pray then rolls and
 * calls on the saint as before. */
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
#include "prayview.h"
#include "relics.h"

PrayModel pray_model;
static int pv_on = 1;
#define ARG(k) (s16)rw(SS, (u16)(SP + 4 + 2 * (k)))
#define D_MAXDF(m) ((u16)(0x9c69 + 0x80 * (m) + 0x4f))   /* the member's divine favor (the screen's "DF remaining") */

int prayview_is_on(void) { return pv_on; }
void prayview_set(int on) { pv_on = on; config_set_int("display.native_pray", on); }

static int H(int saint, int a, int b, int what)          /* the saint's handler (who, favor, what) */
{
    u16 t = (u16)(LOADSEG + 0x290e), o = (u16)(0x2937 + 4 * saint);
    return ns_call_ptr(rw(t, (u16)(o + 2)), rw(t, o), 3, a, b, what);
}

static int clamp99(int v) { return v < 0 ? 0 : v > 99 ? 99 : v; }

static void build(void)
{
    PrayModel *p = &pray_model;
    p->df = (s16)g_rw(GV_pray_df);
    p->maxdf = (s8)g_rb(D_MAXDF(p->who));
    p->chance = clamp99(H(p->saint, p->who, p->df, 7));
    {   /* the least favor that gives the best chance this member can have (the bar's mark) */
        static int memo_who = -1, memo_saint = -1, memo_max = -1, memo = -1;
        if (memo_who != p->who || memo_saint != p->saint || memo_max != p->maxdf) {
            int best = -1, at = -1;
            for (int d = 0; d <= p->maxdf && !ns_unwinding; d++) { int c = clamp99(H(p->saint, p->who, d, 7)); if (c > best) { best = c; at = d; } if (c >= 99) break; }
            memo = best > 0 ? at : -1; memo_who = p->who; memo_saint = p->saint; memo_max = p->maxdf;
        }
        p->df99 = memo;
    }
    /* no chance: why, as the saints' common reckoning has it (s039 165c:0000: nothing unless the
       member's Virtue reaches the saint's minimum, its table's 4, and the favor its least, 2) */
    p->why = 0; p->virtue = (s8)g_rb((u16)(G_MEMBER(p->who) + G_M_SKILLS + 9));
    p->vmin = H(p->saint, p->who, 0, 4); p->xmin = H(p->saint, p->who, 0, 2);
    if (!p->chance && p->vmin > 0 && p->vmin < 100) { if (p->virtue < p->vmin) p->why = 1; else if (p->df < p->xmin) p->why = 2; }
    for (int m = 0; m < 5; m++) { p->present[m] = gm_member_present(m) && gm_member_status(m) != 0; gm_member_name(m, p->names[m], sizeof p->names[m]); }
    p->relic_df = relic_discount(p->who, p->saint, &p->relic);   /* (already in the least favor and the chance: the saint's function asks it) */
    p->seq++;
}

static void screen_hook(void)                 /* saint_screen(who, saint, party), s034 1462:022c */
{
    if (!pv_on || !config_get_int("display.native_pray", 1) || !ns_active()) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    PrayModel *p = &pray_model;
    p->who = ARG(0); p->saint = ARG(1); p->party = ARG(2);
    ns_unwinding = 0;
    /* the original's setup */
    g_ww(GV_pray_box, 0);
    g_ww(GV_pray_target, (u16)p->who);
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    g_ww(GV_pray_box_shown, 0); g_ww(0x8878, 0);
    int t = H(p->saint, p->who, 0, 6);
    p->choose = p->party ? (t & 2) : (t & 1);
    g_ww(GV_pray_choose, (u16)p->choose);
    int df = H(p->saint, p->who, 0, 2);
    if ((s8)g_rb(D_MAXDF(p->who)) < df) df = 0;
    g_ww(GV_pray_df, (u16)df);
    p->cls = H(p->saint, 0, 0, 1);
    {   int day = H(p->saint, p->who, 0, 0);
        int mon = ns_call(29, 0x1367, 0x0938, 1, day), dom = ns_call(29, 0x1367, 0x0982, 1, day);
        char mn[16] = "";
        if (mon >= 0 && mon < 12) { u16 t2 = (u16)(LOADSEG + 0x290e), o = (u16)(0x1d43 + 4 * mon); g_str(rw(t2, (u16)(o + 2)), rw(t2, o), mn, sizeof mn); }
        if (mn[0]) snprintf(p->day, sizeof p->day, "%d %s", dom, mn); else p->day[0] = 0; }
    if (ns_unwinding) return;
    build();
    p->open = 1;
    fprintf(stderr, "prayer: open (member %d, saint %d, chance %d)\n", p->who, p->saint, p->chance);
    ns_enter(NS_PRAY);
    ns_clear();
    ns_input_begin();
    int r = -2;
    while (r == -2) {
        if (!ns_round()) break;
        if (ns_call(GF_key_pressed, 0)) {
            int k = ns_get_key();
            if (k == 0x11b || k == 0x2044 || k == 0x2064) r = -1;
            else if (k == 0x1950 || k == 0x1970) { if (H(p->saint, p->who, (s16)g_rw(GV_pray_df), 7)) { g_ww(GV_pray_target, 0xffff); r = 1; } }
            else if (k == 0x0d2b || k == 0x0d3d || k == 0x324d || k == 0x326d || k == 0x4e2b || k == 0x4800) ns_push(PV_MORE, 0, 0, 0);
            else if (k == 0x0c2d || k == 0x264c || k == 0x266c || k == 0x4a2d || k == 0x5000) ns_push(PV_LESS, 0, 0, 0);
            else if (k == 0x1000) { ns_call(GF_saint_finish, 2, p->who, p->saint); ns_call(GF_quit_to_dos, 0); }
        }
        if (ns_unwinding) break;
        NsAction a;
        while (r == -2 && !ns_unwinding && ns_pop(&a)) {
            int cur = (s16)g_rw(GV_pray_df);
            switch (a.type) {
            case PV_MORE:
                if (H(p->saint, p->who, cur, 7) < 99) { cur++; if (cur > (s8)g_rb(D_MAXDF(p->who))) cur = (s8)g_rb(D_MAXDF(p->who)); g_ww(GV_pray_df, (u16)cur); }
                break;
            case PV_LESS: if (cur > 0) g_ww(GV_pray_df, (u16)(cur - 1)); break;
            case PV_SET: {                                   /* (no more than gives the best chance: the rest would be spent for nothing) */
                int v = a.a, mx = (s8)g_rb(D_MAXDF(p->who));
                if (p->df99 >= 0 && mx > p->df99) mx = p->df99;
                if (v < 0) v = 0; if (v > mx) v = mx; g_ww(GV_pray_df, (u16)v); break; }
            case PV_PRAY:
                if (!H(p->saint, p->who, cur, 7)) break;
                if (a.a >= 0 && (a.a >= gm_party_size() || !gm_member_present(a.a))) break;
                g_ww(GV_pray_target, (u16)a.a);
                r = 1;
                break;
            case PV_CANCEL: r = -1; break;
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    p->open = 0;
    ns_leave();
    if (ns_unwinding) return;
    if (r == 1) r = ns_call(GF_saint_finish, 2, p->who, p->saint);   /* (the fonts back; the chance) */
    else r = -1;                                                       /* (saint_pray puts the fonts back) */
    if (ns_unwinding) return;
    fprintf(stderr, "prayer: %s\n", r < 0 ? "not prayed" : "prayed");
    AX = (u16)r;
    hook_return_far(0);
}

void prayview_init(void)
{
    pv_on = config_get_int("display.native_pray", 1);
    hook_install(GF_saint_screen, screen_hook);
}
