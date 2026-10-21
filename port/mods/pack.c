/* pack.c - the shared pack's housekeeping (inventory.shared, mods/sheetview.c): the party's
 * spare things are one pack in Darklands+'s screens, but the game still keeps each thing with a
 * member, and in a battle a bow, a crossbow or a gun shoots only what its bearer carries
 * (can_fire, s014 18bf:0c02: the ammunition of the weapon's kind, far 20a5:7822[type]). So
 * when a battle begins (run_battle, s029 0e76:2278) each member whose missile weapon needs
 * arrows, quarrels or balls and who has none is handed some from whoever carries them
 * (give_one / give_stack, s031 18e7): one when they never run out (assist.infinite_ammo), else
 * the stack of a member who does not shoot them, or half of a shooter's. The potions are handed
 * over as a member's orders open (below). */
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

/* ---------------------------------------------------------------- handing things over
 * The game still keeps every thing with some member, and in places it asks one member for one:
 * then Darklands+ hands it over first (give_one, s031 18e7:0000, the game's own), with the
 * battle's bookkeeping kept: give_one may reset best_skill (DS 0x99eb, a byte: item_unequip ->
 * equip_weight when a stack empties), a member's Eater-Water for a lock is a record number
 * (DS 0xc7cd, char[5], 0xff none) that shifts when an earlier record empties, the last one of a
 * thing in use stays, and so does the last potion a member is ordered to throw (the kind
 * code - 0x5f at DS 0xca0c, the quality at 0xc97a, char[13]; pending while the fighter's +0x10
 * word has 0x100 and its action byte, DS 0xca64, is not yet 0xd). */
#define D_BEST_SKILL  0x99eb
#define D_EATER_SLOT  0xc7cd
#define D_THROW_KIND  0xca0c
#define D_THROW_Q     0xc97a
#define D_CBT_ACTION  0xca64
#define D_ACTOR       0xe1ca              /* byte: who picks a lock */
#define ID_LOCKPICK   0x40
#define ID_EATERWATER 0x63

static int giver_ok(int h) { int st = gm_member_status(h); return h >= 0 && h < 5 && h < gm_party_size() && st != 0 && st != 3 && st != 5; }
static int carried_id(int m, int id, int q)          /* how many of id (of quality q; -1 any) member m carries */
{
    int n = 0;
    for (int i = 0; i < gm_item_count(m); i++) { GmItem g; if (gm_item(m, i, &g) && g.id == id && (q < 0 || g.qual == q)) n += g.count; }
    return n;
}
static int throw_pending(int d, int id, int q)
{
    if (d < 0 || d >= 5) return 0;
    return (g_rw((u16)(0x9c65 + 0x80 * d)) & 0x100) && g_rb((u16)(D_CBT_ACTION + d)) != 0xd
        && g_rb((u16)(D_THROW_KIND + d)) == (u8)(id - 0x5f) && g_rb((u16)(D_THROW_Q + d)) == (u8)q;
}
static int room_by_records(int m, int id, int q)
{
    if (gm_item_count(m) < 64) return 1;
    for (int j = 0; j < gm_item_count(m); j++) { GmItem x; if (gm_item(m, j, &x) && x.id == id && x.qual == q && x.count < 255) return 1; }
    return 0;
}
/* (when the giver's stack empties, give_one goes through item_unequip -> equip_weight, which in a battle
   reads the item definitions that are not in memory and writes a wrong load into the giver's +0x35 (the
   fighter's load, its pace of blows): a thing not in use never changes the load, so both loads are kept.
   The Eater-Water record and a pending throw are the battle engine's (a dungeon too); elsewhere they
   are left over from the last battle) */
static int engine_on(void) { return game_mode() == GM_BATTLE; }
static int give_safe(int from, int to, int rec)
{
    GmItem g; if (!gm_item(from, rec, &g) || !room_by_records(to, g.id, g.qual)) return 0;
    int empties = g.count == 1, eng = engine_on();
    u8 eat = from < 5 && eng ? g_rb((u16)(D_EATER_SLOT + from)) : 0xff;
    if (eat != 0xff) { GmItem e; if (!gm_item(from, eat, &e) || e.id != ID_EATERWATER) eat = 0xff; }
    if (empties && (gm_item_in_use(from, rec) || (eat != 0xff && eat == rec) || (eng && throw_pending(from, g.id, g.qual)))) return 0;
    u8 bs = g_rb(D_BEST_SKILL), lf = g_rb((u16)(G_MEMBER(from) + 0x35)), lt = g_rb((u16)(G_MEMBER(to) + 0x35));
    ns_call(GF_give_one, 4, from, to, rec, 9);
    if (ns_unwinding) return 0;
    g_wb(D_BEST_SKILL, bs); g_wb((u16)(G_MEMBER(from) + 0x35), lf); g_wb((u16)(G_MEMBER(to) + 0x35), lt);
    if (empties && eat != 0xff && eat > rec) g_wb((u16)(D_EATER_SLOT + from), (u8)(eat - 1));
    return 1;
}
/* one of id (the best quality, or exactly q) to member a from whoever else has one to spare; 1 if handed */
static int fetch(int a, int id, int q)
{
    int bh = -1, bi = -1, bq = -1, bn = 0;
    for (int h = 0; h < 5; h++) {
        if (h == a || !giver_ok(h)) continue;
        for (int i = 0; i < gm_item_count(h); i++) {
            GmItem g; if (!gm_item(h, i, &g) || g.id != id || (q >= 0 && g.qual != q) || gm_item_spare(h, i) <= 0) continue;
            if (g.count == 1 && engine_on() && throw_pending(h, id, g.qual)) continue;
            if (g.qual > bq || (g.qual == bq && g.count > bn)) { bh = h; bi = i; bq = g.qual; bn = g.count; }
        }
    }
    return bh >= 0 && give_safe(bh, a, bi);
}

/* lock picks: outside a battle (a dungeon, a castle) a member's orders offer a lock only to a member
   with lock picks of their own (potions, Eater-Water too, stay each member's own) */
static void orders_hook(void)             /* order_menu_build (), s014 19cd:29b8: the selected member's orders */
{
    if (sheetview_shared() && gm_party_size() > 1 && g_rb(GV_real_combat) != 1) {
        int a = (s8)g_rb(GV_selected_member);
        if (a >= 0 && a < 5 && gm_member_status(a) == 1 && !carried_id(a, ID_LOCKPICK, -1)) fetch(a, ID_LOCKPICK, -1);
        if (ns_unwinding) return;
    }
    hook_call_original();
}
/* a lock is picked by the member walking first or ordered to (DS 0xe1ca), with their own lock
   picks (lock_pick, s023 1ef8:12c0, the first record of item 0x40) */
static void lockpick_hook(void)
{
    int a = g_rb(D_ACTOR);
    if (sheetview_shared() && a < 5 && gm_member_present(a) && !carried_id(a, ID_LOCKPICK, -1)) {
        if (fetch(a, ID_LOCKPICK, -1)) fprintf(stderr, "pack: member %d is handed lock picks\n", a);
        if (ns_unwinding) return;
    }
    hook_call_original();
}

/* ammunition and thrown weapons that run out (with "never run out" off): a shot uses one of the
   member's own (use_ammo, s014 18bf:0c48 (i): consume_item of the weapon's ammunition, or of the
   thrown weapon itself), and on the last one the member changes weapon. So the member's stack is
   topped up from the pack first, while there is more. */
extern int thrown_infinite;
static void use_ammo_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    if (sheetview_shared() && i >= 0 && i < 5 && gm_party_size() > 1 && !config_get_int("original", 0)) {
        u16 cb = (u16)(0x9c55 + 0x80 * i);
        int wt = (s8)g_rb((u16)(cb + 0x22));
        int at = wt >= 0 && wt < 0x40 ? rb(G_FAR20A5, (u16)(0x7822 + wt)) : 0;
        int t = at ? at : wt, q = at ? -1 : g_rb((u16)(cb + 0x5a));
        int infinite = at ? ammo_infinite : thrown_infinite;
        if (t > 0 && !infinite) {
            int have = 0;
            for (int k = 0; k < gm_item_count(i); k++) { GmItem g; if (gm_item(i, k, &g) && g.type == t && (q < 0 || g.qual == q)) have += g.count; }
            for (int n = 0; have <= 1 && n < 10 && !ns_unwinding; n++) {     /* (ten at a time) */
                int bh = -1, bi = -1, bn = 0;
                for (int h = 0; h < 5; h++) {
                    if (h == i || !giver_ok(h)) continue;
                    for (int k = 0; k < gm_item_count(h); k++) {
                        GmItem g; if (!gm_item(h, k, &g) || g.type != t || (q >= 0 && g.qual != q) || gm_item_spare(h, k) <= 0) continue;
                        if (g.count > bn) { bh = h; bi = k; bn = g.count; }
                    }
                }
                if (bh < 0) break;
                for (int k = 0; k < 10 && !ns_unwinding; k++) {
                    GmItem g; if (!gm_item(bh, bi, &g) || g.type != t || gm_item_spare(bh, bi) <= 0) break;
                    if (!give_safe(bh, i, bi)) break;
                    have++;
                }
                break;
            }
            if (ns_unwinding) return;
        }
    }
    hook_call_original();
}

/* arrows, quarrels and balls that never run out are not shown (gm_ammo_hidden), but a bow, crossbow
   or gun shoots only while its bearer carries one: each shooter is given one of the party's, or a
   new one if the party has none */
static void ensure_ammo(void)
{
    for (int m = 0; m < 5 && !ns_unwinding; m++) {
        if (!gm_member_present(m) || gm_member_status(m) != 1) continue;
        int t = ammo_of(m);
        if (t < 0x40 || t > 0x42 || carried(m, t, NULL)) continue;
        int from = -1, rec = -1;
        for (int h = 0; h < 5 && from < 0; h++) {
            if (h == m || !gm_member_present(h) || gm_member_status(h) == 5) continue;
            int r = -1; if (carried(h, t, &r) > (ammo_of(h) == t ? 1 : 0)) { from = h; rec = r; }
        }
        if (from >= 0) ns_call(GF_give_one, 4, from, m, rec, 9);
        else ns_call(GF_add_item_q, 3, m, 33 + (t - 0x40), 25);
        if (!ns_unwinding) fprintf(stderr, "pack: member %d carries the ammunition %#x the game wants to see\n", m, t);
    }
}

static int in_battle;                     /* run_battle running */
static void battle_hook(void)             /* run_battle (s029 0e76:2278) */
{
    if (gm_ammo_hidden()) { ensure_ammo(); if (ns_unwinding) return; }
    else if (sheetview_shared() && gm_party_size() > 1) {
        hand_out_ammo(); if (ns_unwinding) return;
    }
    in_battle++;
    hook_call_original();
    if (in_battle > 0) in_battle--;
}

/* the horses: the game counts each member's own. The party is mounted only when every member
   carries a horse or mule (party_get(5), s029 0e76:1326: travel and some 18 cards), and a chase
   or escape adds the member's own best mount's quality (party_max/min_eff_attr2_bonus, 0e76:072e /
   088e, through best_item_quality (who, 0x2000), 0f54; the joust likewise). So before any of these
   asks, a member without a mount is handed the best spare one (each keeps the best they have:
   the one they ride, gm_item_riding). Mounts are item type 108. */
static int has_mount(int m)
{
    for (int i = 0; i < gm_item_count(m); i++) { GmItem g; if (gm_item(m, i, &g) && g.type == 108) return 1; }
    return 0;
}
static void horses_even(void)
{
    if (!sheetview_shared() || gm_party_size() < 2 || in_battle) return;
    for (int m = 0; m < 5 && !ns_unwinding; m++) {
        if (!gm_member_present(m) || gm_member_status(m) != 1 || has_mount(m) || gm_item_count(m) >= 64) continue;
        int from = -1, rec = -1, q = -1;
        for (int h = 0; h < 5; h++) {                         /* the best spare mount (not one someone rides) */
            if (h == m || !gm_member_present(h) || gm_member_status(h) == 0 || gm_member_status(h) == 3) continue;
            for (int i = 0; i < gm_item_count(h); i++) {
                GmItem g; if (!gm_item(h, i, &g) || g.type != 108 || gm_item_spare(h, i) <= 0) continue;
                if (g.qual > q) { q = g.qual; from = h; rec = i; }
            }
        }
        if (from < 0) return;                                 /* (no spare mounts) */
        ns_call(GF_give_one, 4, from, m, rec, 9);
        fprintf(stderr, "pack: member %d is given a mount (quality %d) from member %d\n", m, q, from);
    }
}
static void mount_query_hook(void) { horses_even(); if (ns_unwinding) return; hook_call_original(); }
static void party_get_hook(void)          /* party_get (what): 5 = everyone mounted */
{
    int what = (s16)rw(SS, (u16)(SP + 4));
    if (what == 5) { horses_even(); if (ns_unwinding) return; }
    hook_call_original();
    if (what == 5 && getenv("DL_PACK_LOG")) fprintf(stderr, "pack: party_get(5) = %d\n", (s16)AX);
}
static void best_quality_hook(void)       /* best_item_quality (who, long mask) */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    u32 mask = rw(SS, (u16)(SP + 6)) | ((u32)rw(SS, (u16)(SP + 8)) << 16);
    if (mask == 0x2000 && who >= -2 && who < 5) { horses_even(); if (ns_unwinding) return; }
    hook_call_original();
}

/* a member who leaves the party (party_remove, s037 1462:3b58: retiring at an inn, $Inn's card or
   the inn's party screen), or dies or is lost on a card (party_delete, 1462:3f30, from some 56 card
   places and when one stays away too long) would take their share of the pack along: the spares go
   to those who stay first. Not while a new game's party is chosen (the roster screen: what each
   brings is their own), and not in a battle (a fallen member's things are already in the loot). */
static int choosing;
static void roster_hook(void) { choosing++; hook_call_original(); if (choosing > 0) choosing--; }
static void spares_off(int p)
{
    /* (most callers of party_delete have already set the member's status to 0: their records are still there) */
    if (choosing || in_battle || !sheetview_shared() || p < 0 || p >= 5 || !gm_item_count(p)) return;
    int moved = 0;
    for (int i = gm_item_count(p) - 1; i >= 0 && !ns_unwinding; i--) {
        GmItem g; if (!gm_item(p, i, &g)) continue;
        int eq = gm_item_in_use(p, i), spare = g.count - (eq ? 1 : 0);   /* (they keep what they use, their mount too) */
        if (spare <= 0) continue;
        int to = -1;                                    /* whoever stays and has room: a stack of the same, or a free record */
        for (int m = 0; m < 5 && to < 0; m++) {
            int st = gm_member_status(m);
            if (m != p && gm_member_present(m) && st != 0 && st != 3 && st != 5 && gm_room_for(m, g.id, g.qual)) to = m;
        }
        if (to < 0) break;
        if (!eq) ns_call(GF_give_stack, 4, p, to, i, 9);
        else for (int k = 0; k < spare && !ns_unwinding; k++) ns_call(GF_give_one, 4, p, to, i, 9);
        moved += spare;
    }
    if (moved && !ns_unwinding) fprintf(stderr, "pack: member %d leaves; %d spare things stay in the pack\n", p, moved);
}
static void leave_hook(void)              /* party_remove (p) */
{
    spares_off((s16)rw(SS, (u16)(SP + 4)));
    if (ns_unwinding) return;
    hook_call_original();
}
static void delete_hook(void)             /* party_delete (p) */
{
    spares_off((s16)rw(SS, (u16)(SP + 4)));
    if (ns_unwinding) return;
    hook_call_original();
}

void pack_init(void)
{
    hook_install(GF_run_battle, battle_hook);
    hook_install(GF_party_remove, leave_hook);
    hook_install(GF_party_delete, delete_hook);
    hook_install(GF_party_get, party_get_hook);
    hook_install(GF_party_max_eff_attr2_bonus, mount_query_hook);
    hook_install(GF_party_min_eff_attr2_bonus, mount_query_hook);
    hook_install(GF_best_item_quality, best_quality_hook);
    hook_install(GF_roster_screen, roster_hook);
    hook_install(GF_order_menu_build, orders_hook);
    hook_install(GF_lock_pick, lockpick_hook);
    hook_install(GF_use_ammo, use_ammo_hook);
}
