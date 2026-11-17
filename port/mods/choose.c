/* choose.c - a box asking which of the party's things (ui/choose.cpp draws it): the native
 * screens' frame (mods/nscreen.h), a line clicked or its number pressed picks it, Esc or the way
 * out's button picks none. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "nscreen.h"
#include "choose.h"

ChooseModel choose_model;

int choose_run(const char *title, const char *note, const ChooseRow *rows, int n, const char *none)
{
    ChooseModel *c = &choose_model;
    if (n > CHOOSE_MAX) n = CHOOSE_MAX;
    snprintf(c->title, sizeof c->title, "%s", title);
    snprintf(c->note, sizeof c->note, "%s", note ? note : "");
    snprintf(c->none, sizeof c->none, "%s", none ? none : "None");
    memcpy(c->row, rows, (size_t)n * sizeof *rows);
    c->n = n; c->open = 1; c->seq++;
    ns_unwinding = 0;
    ns_enter(NS_CHOOSE);
    ns_clear();
    ns_input_begin();
    int member_rows = n && rows[0].name[0];
    int preferred = -1;
    for (int i = 0; i < n; i++) if (rows[i].recommended && !rows[i].disabled) preferred = i;
    int r = -2;
    while (r == -2) {
        if (!(member_rows ? ns_modal_round() : ns_round())) break;
        if (ns_call(GF_key_pressed, 0)) {
            int k = member_rows ? (u16)ns_call(GF_get_key, 0) : ns_get_key(), a = k & 0xff;
            if (k == 0x11b) r = -1;
            else if (a == 13 && preferred >= 0) r = preferred;
            else if (a >= '1' && a <= '9' && a - '1' < n && !rows[a - '1'].disabled) r = a - '1';
        }
        if (ns_unwinding) break;
        NsAction act;
        while (r == -2 && ns_pop(&act)) {
            if (act.type == CH_PICK && act.a >= 0 && act.a < n && !rows[act.a].disabled) r = act.a;
            else if (act.type == CH_NONE) r = -1;
        }
    }
    c->open = 0;
    ns_leave();
    if (ns_unwinding) return -1;
    fprintf(stderr, "choose: %s -> %d\n", c->title, r);
    return r;
}
