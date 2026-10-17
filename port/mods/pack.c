/* pack.c - the shared pack's housekeeping (inventory.shared, mods/sheetview.c): the party's
 * spare things are one pack in Darklands+'s screens, but the game still keeps each thing with a
 * member, and in a battle a bow, a crossbow or a gun shoots only what its bearer carries
 * (can_fire, s014 18bf:0c02: the ammunition of the weapon's kind, far 20a5:7822[type]). So
 * when a battle begins (run_battle, s029 0e76:2278) each member whose missile weapon needs
 * arrows, quarrels or balls and who has none is handed some from whoever carries them
 * (give_one / give_stack, s031 18e7): one when they never run out (assist.infinite_ammo), else
 * the stack of a member who does not shoot them, or half of a shooter's. The potions likewise:
 * each kind spread over the able members, since in a battle one drinks or throws only what one
 * carries. */
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

/* the potions, shared out: in a battle a member drinks or throws only what they carry (the
   orders' menu lists their own), so each kind of potion in the pack is spread over the able
   members, as evenly as it goes (the extra ones to those first in the marching order) */
static int is_potion(int id) { unsigned long c; return gm_itemdef_cats(id, &c) && (c & 0x800); }
static void spread_potions(void)
{
    int able[5], n = 0;
    for (int k = 0; k < 6 && n < 5; k++) {                 /* the marching order first, then the rest */
        int m = (s8)g_rb((u16)(GV_marching_order + k));
        if (m < 0 || m > 4) continue;
        int dup = 0; for (int j = 0; j < n; j++) if (able[j] == m) dup = 1;
        if (!dup && gm_member_present(m) && gm_member_status(m) == 1) able[n++] = m;
    }
    for (int m = 0; m < 5 && n < 5; m++) {
        int dup = 0; for (int j = 0; j < n; j++) if (able[j] == m) dup = 1;
        if (!dup && gm_member_present(m) && gm_member_status(m) == 1) able[n++] = m;
    }
    if (n < 2) return;
    struct { int id, q; } kinds[32]; int nk = 0;
    for (int j = 0; j < n; j++)
        for (int i = 0; i < gm_item_count(able[j]); i++) {
            GmItem g; if (!gm_item(able[j], i, &g) || !is_potion(g.id)) continue;
            int seen = 0; for (int k = 0; k < nk; k++) if (kinds[k].id == g.id && kinds[k].q == g.qual) seen = 1;
            if (!seen && nk < 32) { kinds[nk].id = g.id; kinds[nk].q = g.qual; nk++; }
        }
    for (int k = 0; k < nk && !ns_unwinding; k++) {
        int have[5], total = 0;
        for (int j = 0; j < n; j++) {
            have[j] = 0;
            for (int i = 0; i < gm_item_count(able[j]); i++) { GmItem g; if (gm_item(able[j], i, &g) && g.id == kinds[k].id && g.qual == kinds[k].q) have[j] += g.count; }
            total += have[j];
        }
        for (int j = 0; j < n && !ns_unwinding; j++) {
            int want = total / n + (j < total % n ? 1 : 0);
            while (have[j] < want && !ns_unwinding) {      /* from whoever has the most above their share */
                int from = -1, over = 0;
                for (int f = 0; f < n; f++) { int fw = total / n + (f < total % n ? 1 : 0); if (have[f] - fw > over) { over = have[f] - fw; from = f; } }
                if (from < 0 || !gm_room_for(able[j], kinds[k].id, kinds[k].q)) break;
                int rec = -1;
                for (int i = 0; i < gm_item_count(able[from]); i++) { GmItem g; if (gm_item(able[from], i, &g) && g.id == kinds[k].id && g.qual == kinds[k].q) rec = i; }
                if (rec < 0) break;
                ns_call(GF_give_one, 4, able[from], able[j], rec, 9);
                have[from]--; have[j]++;
            }
        }
    }
}

static void battle_hook(void)             /* run_battle (s029 0e76:2278) */
{
    if (sheetview_shared() && gm_party_size() > 1) {
        hand_out_ammo(); if (ns_unwinding) return;
        spread_potions(); if (ns_unwinding) return;
    }
    hook_call_original();
}

/* a member who leaves the party (party_remove, s037 1462:3b58: retiring at an inn, $Inn's card or
   the inn's party screen) would take their share of the pack along: the spares go to those who
   stay first. (Not while a new game's party is chosen, the roster screen: what each brings is
   their own.) */
static int choosing;
static void roster_hook(void) { choosing++; hook_call_original(); if (choosing > 0) choosing--; }
static void leave_hook(void)              /* party_remove (p) */
{
    int p = (s16)rw(SS, (u16)(SP + 4));
    if (!choosing && sheetview_shared() && p >= 0 && p < 5 && gm_member_present(p) && gm_party_size() > 1) {
        int moved = 0;
        for (int i = gm_item_count(p) - 1; i >= 0 && !ns_unwinding; i--) {
            GmItem g; if (!gm_item(p, i, &g)) continue;
            int eq = gm_item_equipped(p, i), spare = g.count - (eq ? 1 : 0);
            if (spare <= 0) continue;
            int to = -1;                                    /* whoever stays and has room: a stack of the same, or a free record */
            for (int m = 0; m < 5 && to < 0; m++) if (m != p && gm_member_present(m) && gm_member_status(m) != 5 && gm_room_for(m, g.id, g.qual)) to = m;
            if (to < 0) break;
            if (!eq) ns_call(GF_give_stack, 4, p, to, i, 9);
            else for (int k = 0; k < spare && !ns_unwinding; k++) ns_call(GF_give_one, 4, p, to, i, 9);
            moved += spare;
        }
        if (ns_unwinding) return;
        if (moved) fprintf(stderr, "pack: member %d leaves; %d spare things stay in the pack\n", p, moved);
    }
    hook_call_original();
}

void pack_init(void)
{
    hook_install(GF_run_battle, battle_hook);
    hook_install(GF_party_remove, leave_hook);
    hook_install(GF_roster_screen, roster_hook);
}
