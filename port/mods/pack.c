/* pack.c - the shared pack's housekeeping (inventory.shared, mods/sheetview.c): the party's
 * spare things are one pack in Darklands+'s screens, but the game still keeps each thing with a
 * member, and in a battle a bow, a crossbow or a gun shoots only what its bearer carries
 * (can_fire, s014 18bf:0c02: the ammunition of the weapon's kind, far 20a5:7822[type]). So
 * when a battle begins (run_battle, s029 0e76:2278) each member whose missile weapon needs
 * arrows, quarrels or balls and who has none is handed some from whoever carries them
 * (give_one / give_stack, s031 18e7): one when they never run out (assist.infinite_ammo), else
 * the stack of a member who does not shoot them, or half of a shooter's. */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "gamemodel.h"
#include "nscreen.h"

int sheetview_shared(void);
extern int ammo_infinite;

static int ammo_of(int m)                 /* the ammunition type the member's missile weapon shoots, 0 none */
{
    GmEquip e; gm_member_equip(m, &e);
    if (e.missile < 0 || e.missile > 0x3f) return 0;
    return rb(G_FAR20A5, (u16)(0x7822 + e.missile));
}
static int carried(int m, int type, int *rec)
{
    int n = 0;
    for (int i = 0; i < gm_item_count(m); i++) { GmItem g; if (gm_item(m, i, &g) && g.type == type) { n += g.count; if (rec) *rec = i; } }
    return n;
}

static void hand_out_ammo(void)
{
    for (int m = 0; m < 5; m++) {
        if (!gm_member_present(m) || gm_member_status(m) != 1) continue;
        int t = ammo_of(m);
        if (!t || carried(m, t, NULL)) continue;
        int best = -1, brec = -1, bn = 0, bshoots = 1;
        for (int h = 0; h < 5; h++) {
            if (h == m || !gm_member_present(h) || gm_member_status(h) == 5) continue;
            int rec = -1, n = carried(h, t, &rec);
            if (!n) continue;
            int shoots = ammo_of(h) == t;
            if (best < 0 || (bshoots && !shoots) || (shoots == bshoots && n > bn)) { best = h; brec = rec; bn = n; bshoots = shoots; }
        }
        if (best < 0) continue;
        if (gm_item_count(m) >= 64) continue;
        int give = ammo_infinite && !config_get_int("original", 0) ? 1 : bshoots ? bn / 2 : bn;
        if (give < 1) continue;
        if (!bshoots && give == bn) ns_call(GF_give_stack, 4, best, m, brec, 9);
        else for (int k = 0; k < give && !ns_unwinding; k++) {
            int rec = -1; if (!carried(best, t, &rec)) break;
            ns_call(GF_give_one, 4, best, m, rec, 9);
        }
        if (ns_unwinding) return;
        fprintf(stderr, "pack: member %d gets %d of ammunition %#x from member %d for the battle\n", m, give, t, best);
    }
}

static void battle_hook(void)             /* run_battle (s029 0e76:2278) */
{
    if (sheetview_shared() && gm_party_size() > 1) { hand_out_ammo(); if (ns_unwinding) return; }
    hook_call_original();
}

void pack_init(void)
{
    hook_install(GF_run_battle, battle_hook);
}
