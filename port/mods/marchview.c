/* marchview.c - Change Marching Order as a native dialog (display.native_march; the view is
 * ui/marchview.cpp).
 *
 * The original (choose_party_order, s022 1ef8:051c) asks "Select first", "Select second" ... and
 * takes a click on a member's box (or F1..F5, or the member's figure in a battle) for each, the
 * last one following by itself; then marching_order[] holds them and marching_len their number,
 * and in single file (move_mode 2) the line is formed again (single_file). Here the able members
 * are listed in their present order (those not in it after) and moved up and down; Done writes
 * the same two variables and forms the line as the original does, Cancel (Esc) keeps the order
 * as it was. */
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
#include "marchview.h"

void plat_sleep_ms(int ms);
void dl_poll(void);

MarchModel march_model;
static int mv_on = 1;

int marchview_is_on(void) { return mv_on; }
void marchview_set(int on) { mv_on = on; config_set_int("display.native_march", on); }

static void build_names(void)
{
    MarchModel *m = &march_model;
    for (int i = 0; i < 5; i++) {
        gm_member_name(i, m->name[i], sizeof m->name[i]);
        gm_member_figure_code(i, m->figure[i], sizeof m->figure[i]);
        gm_member_palette(i, m->pal[i]);
    }
    m->seq++;
}

static void screen_hook(void)                 /* choose_party_order (), s022 1ef8:051c */
{
    if (!mv_on || !ns_active()) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    MarchModel *m = &march_model;
    ns_unwinding = 0;
    /* the able members: in the present order first, then the others */
    int able[5] = { 0 }, used[5] = { 0 };
    for (int i = 0; i < 5; i++) able[i] = g_rb(G_MEMBER(i)) == 1;
    m->n = 0;
    int len = g_rb(GV_marching_len);
    for (int k = 0; k < len && k < 5; k++) {
        int i = (s8)g_rb((u16)(GV_marching_order + k));
        if (i >= 0 && i < 5 && able[i] && !used[i]) { m->order[m->n++] = i; used[i] = 1; }
    }
    for (int i = 0; i < 5; i++) if (able[i] && !used[i]) m->order[m->n++] = i;
    if (m->n < 2) { hook_call_original(); return; }
    build_names();
    m->open = 1;
    fprintf(stderr, "marching order: open (%d members)\n", m->n);
    ns_enter(NS_MARCH);
    ns_clear();
    ns_input_begin();
    int done = 0;                             /* 1 done, 2 cancelled */
    while (!done) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        ns_call(GF_exit_if_requested, 0);
        if (ns_unwinding) break;
        while (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (k == 0x011b) done = 2;
            else if (k == 0x1c0d) done = 1;
        }
        NsAction a;
        while (!done && ns_pop(&a)) {
            int k = a.a;
            if (a.type == MV_UP && k > 0 && k < m->n) { int t = m->order[k - 1]; m->order[k - 1] = m->order[k]; m->order[k] = t; m->seq++; }
            else if (a.type == MV_DOWN && k >= 0 && k + 1 < m->n) { int t = m->order[k + 1]; m->order[k + 1] = m->order[k]; m->order[k] = t; m->seq++; }
            else if (a.type == MV_DONE) done = 1;
            else if (a.type == MV_CANCEL) done = 2;
        }
    }
    m->open = 0;
    ns_leave();
    if (ns_unwinding) { fprintf(stderr, "marching order: closed (unwound)\n"); return; }
    if (done == 1) {
        for (int k = 0; k < m->n; k++) g_wb((u16)(GV_marching_order + k), (u8)m->order[k]);
        g_wb(GV_marching_len, (u8)m->n);
        if (g_rb(GV_move_mode) == 2) ns_call(GF_single_file, 0);
    }
    fprintf(stderr, "marching order: closed (%s)\n", done == 1 ? "set" : "kept");
    hook_return_far(0);
}

void marchview_init(void)
{
    mv_on = config_get_int("display.native_march", 1);
    hook_install(GF_choose_party_order, screen_hook);
}
