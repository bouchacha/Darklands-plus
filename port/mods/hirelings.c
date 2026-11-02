/* hirelings.c - Companions stay (rules.hirelings_stay: experimental, off in every preset).
 *
 * A companion the party takes on through a card (add_member, s029 0e76:4650 (0, name, short name,
 * picture, hours, group)) comes with a contract: an event of kind 0x43 (type0 the party slot, +0x1e
 * the group) lasting those hours - a village's Schulz 2160 (90 days: s077 1996, s080 1838), the
 * Hanse's man 8640 (360 days: s080 1838), a knight 9999 (s082 1922), which the game reads as for ever
 * (to the last hour of 1499). When a contract ends the game notes that the companion left (kind 0x47,
 * event_expire_msg, s029 0e76:4442) and a card sends him off ($NPCEx00, s104 190e).
 *
 * With the switch on every contract runs for ever, as a knight's does: one being made (the hours
 * given to add_member), and those the party holds when a game is loaded or the switch is turned on.
 * A quest that ends with its companion going home (the Hanse's man delivered at a Hanse office:
 * s061 1a5f closes every contract of group 7) is left as it is. Turned off again, a contract made
 * for ever stays so. */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "saves.h"

static int on(void) { return config_get_int("rules.hirelings_stay", 0) && !config_get_int("original", 0); }

static void add_member_hook(void)          /* add_member (0, name, short name, picture, hours, group) */
{
    if (on() && (s16)rw(SS, (u16)(SP + 12)) != 9999) {
        fprintf(stderr, "hirelings: a companion's contract of %d hours made for ever\n", (s16)rw(SS, (u16)(SP + 12)));
        ww(SS, (u16)(SP + 12), 9999);
    }
    hook_call_original();
}

/* every contract the party holds: to the game's "for ever" (23:00, 31 December 1499; months from 0, as the
   game's own for-ever events have it) */
static void contracts_for_ever(void)
{
    int n = 0;
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i);
        if (!e || g_ev(e, G_EV_KIND) != 0x43 || g_ev(e, G_EV_END + 6) == 1499) continue;
        g_ev_set(e, G_EV_END, 23); g_ev_set(e, G_EV_END + 2, 31); g_ev_set(e, G_EV_END + 4, 12); g_ev_set(e, G_EV_END + 6, 1499);
        n++;
    }
    if (n) fprintf(stderr, "hirelings: %d contract%s made for ever\n", n, n == 1 ? "" : "s");
}
static void after_load(void) { if (on()) contracts_for_ever(); }

void hirelings_apply(int v) { if (v && !config_get_int("original", 0)) contracts_for_ever(); }   /* (the switch turned on: options.c) */

void hirelings_init(void)
{
    hook_install(GF_add_member, add_member_hook);
    saves_on_loaded(after_load);
}
