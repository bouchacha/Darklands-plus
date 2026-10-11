/* fixes.c - fixes for bugs that are still in the last official version (483.07). Each can
 * be switched off (F12 > Helpers > Fixes); "play the original" switches them all off.
 *
 *  blessing  Saint blessings (and potions) that raise an attribute record how much they
 *            added before the attribute is capped (raise_attr_temp, s029 0e76:4c04: the cap
 *            is max + the LARGEST running boost, add_attr also clamps at 99). When they run
 *            out the full recorded amount is taken off again, so a second blessing on the
 *            same attribute, or one near 99, costs points for good (Int never comes back).
 *            Fixed by recording what was really added.
 *  fireward  St. Polycarp, St. Charity, Firewall ... set the high nibble of the member's
 *            fire-protection byte (party +0x41 = combatant +0x55). hit_damage (s014
 *            18bf:0778) adds that byte (& 0x7f) to the armour, so bits 4-6 turn into up to
 *            +64 armour that never wears off: the "invulnerable after Polycarp + Firewall"
 *            bug. Fixed by hiding those bits from the armour sum.
 *  fullpack  A member holds at most 64 item records; add_item_rec_impl silently drops what does
 *            not fit (quest items included). It now goes to another member with room.
 *  nullitem  Items with a code past the item list (>= 200) show as "(null)" (a NULL name
 *            pointer) and can be neither used nor sold; empty stacks (count 0) appear from
 *            the inn's ingredient refund. add_item_rec_impl (s031 18e7:01b4) now refuses both.
 *  castle    In a castle battle only the death of the boss ends the fight (D_9084), and that
 *            flag is not saved with the battle: after a save and reload mid-battle the knight
 *            is already dead and leaving always counts as fleeing ("battle not ending").
 *            combat_over (s014 0e76:08fe) now also looks at the boss's dead flag.
 *  bounty    A merchant's robber-knight bounty: $Merch03 (s092 188f:0222) hands $Raubi01
 *            (s080 1870:000c) the castle as the quest giver (saved_card2), so the reward event
 *            is not found; the card then reads and frees events[-1] (memory corruption) and the
 *            reward can be claimed again and again. The giver is now corrected on the way in.
 *  innsave   The lodging screen (s036 1462:0082) disables the Save and Load menu items (DS:9920,
 *            9934 = 2) and re-enables them on most exits but not all (a click on an empty
 *            party slot returns at once): Save/Load stayed grey until a character sheet was
 *            opened. They are enabled again on the way out.
 *  colours   The "neon hair" bug: characters' colours (and figures) change after encounters.
 *            They are now watched and put back (fixes_frame).
 *  copyprot  The manual check (s041 copy_protection, once when the party first leaves a town:
 *            pick the alchemical symbol for a substance) is skipped.
 *  alchemy   Brewing takes the ingredients with s033 1462:1630, which skips the stack that
 *            slides up when one is used up and forgets what it could not take: most batches
 *            cost only part of their ingredients. The remainder is now taken too. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "fixes.h"

Fix FIXES[] = {
    { "blessing", "Blessings no longer cost attribute points for good",
      "A second blessing on the same attribute, or one near 99, used to take points away when it wore off.", 1 },
    { "fireward", "No invulnerability from fire protection",
      "St. Polycarp, Firewall and the like no longer turn into permanent armour (+16 to +64).", 1 },
    { "nullitem", "No \"(null)\" or empty items",
      "Items past the item list and empty stacks are no longer added to an inventory.", 1 },
    { "castle", "Castle battles end when the knight dies",
      "Even after saving and loading during the battle.", 1 },
    { "bounty", "Robber-knight bounties from merchants pay out properly",
      "The original looked up the wrong quest giver: garbled names, memory corruption, rewards claimable again.", 1 },
    { "alchemy", "Brewing uses up all the ingredients",
      "The original took only part of them when an ingredient was spread over several stacks.", 1 },
    { "fullpack", "No more items lost to a full pack",
      "A character carries at most 64 kinds of things; what the game gave to a full pack simply vanished (quest items too). It now goes to someone with room.", 1 },
    { "innsave", "Saving and loading stay possible after the inn",
      "Leaving the inn's lodging screen in some ways (e.g. a click on an empty party slot) left Save and Load disabled until a character sheet was opened.", 1 },
    { "colours", "Characters keep their colours (the \"neon hair\" bug)",
      "Characters' colours and figures sometimes change to garish ones after an encounter. Their own colours are now kept and put back.", 1 },
    { "copyprot", "Skip the manual check",
      "The game no longer asks for an alchemical symbol from the printed manual when the party first sets out.", 1 },
};
int FIX_COUNT = (int)(sizeof FIXES / sizeof FIXES[0]);

/* ---------------------------------------------------------------- blessing */
#define EVSEG G_FAR2E38
static u32 ev_ptr(int i) { u16 o = rw(EVSEG, (u16)(0x1b2c + 4 * i)), s = rw(EVSEG, (u16)(0x1b2c + 4 * i + 2)); return s ? ((u32)s << 16) | o : 0; }

static void blessing_hook(void)            /* raise_attr_temp(who, k, d, h) */
{
    static u8 had[300];
    u8 before[5][7];
    for (int i = 0; i < 300; i++) had[i] = ev_ptr(i) != 0;
    for (int m = 0; m < 5; m++) for (int a = 0; a < 7; a++) before[m][a] = g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a));
    hook_call_original();
    for (int i = 0; i < 300; i++) {
        u32 e = ev_ptr(i);
        if (!e || had[i]) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x48) continue;
        int m = (s16)rw(s, (u16)(o + 0x1a)), a = (s16)rw(s, o);
        if (m < 0 || m >= 5 || a < 0 || a >= 7) continue;
        int added = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a)) - (s8)before[m][a];
        int rec = (s16)rw(s, (u16)(o + 0x1e));
        if (added < 0) added = 0;
        if (added != rec) {
            if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix blessing: member %d attr %d recorded %d, really added %d\n", m, a, rec, added);
            ww(s, (u16)(o + 0x1e), (u16)added);
        }
    }
}

/* ---------------------------------------------------------------- fireward */
static void fireward_hook(void)            /* hit_damage(a, t) */
{
    int a = (s16)rw(SS, (u16)(SP + 4)), t = (s16)rw(SS, (u16)(SP + 6));
    u16 ta = (u16)(0x9c55 + 0x80 * t + 0x55), aa = (u16)(0x9c55 + 0x80 * a + 0x55);
    int fix_t = t >= 0 && t < 5, fix_a = a >= 0 && a < 5;
    u8 st = fix_t ? g_rb(ta) : 0, sa = fix_a ? g_rb(aa) : 0;
    if (fix_t) g_wb(ta, (u8)(st & 0x8f));
    if (fix_a && a != t) g_wb(aa, (u8)(sa & 0x7f));
    hook_call_original();
    if (fix_a && a != t) g_wb(aa, sa);
    if (fix_t) g_wb(ta, st);
}

/* ---------------------------------------------------------------- nullitem */
static int fix_on(const char *id) { for (int i = 0; i < FIX_COUNT; i++) if (!strcmp(FIXES[i].id, id)) return FIXES[i].on; return 0; }

static int stacks(int m, u16 code, u8 type, u8 qual, int cnt)
{
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
    for (int j = 0; j < n && j < 64; j++) {
        u16 o = G_ITEM(m, j);
        if (rw(G_FAR2E38, o) == code && rb(G_FAR2E38, (u16)(o + 2)) == type && rb(G_FAR2E38, (u16)(o + 3)) == qual && rb(G_FAR2E38, (u16)(o + 4)) + cnt <= 0xff) return 1;
    }
    return 0;
}

static void additem_hook(void)             /* add_item_rec_impl(who, struct item {code, type, qual, cnt, weight}) */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    u16 code = rw(SS, (u16)(SP + 6)); u8 type = rb(SS, (u16)(SP + 8)), qual = rb(SS, (u16)(SP + 9));
    int cnt = rb(SS, (u16)(SP + 10));
    if (fix_on("nullitem") && ((code & 0xfff) >= 200 || cnt == 0)) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix nullitem: refused item %d x%d for %d\n", code & 0xfff, cnt, who);
        int m = who == -1 ? (s8)g_rb(G_LEADER) : who;
        AX = (m >= 0 && m < 5) ? g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)) : 0;
        hook_return_far(0);
        return;
    }
    if (fix_on("fullpack") && who != -2) {
        int m = who == -1 ? (s8)g_rb(G_LEADER) : who, size = (s16)g_rw(G_PARTY_SIZE);
        if (m >= 0 && m < 5 && g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)) >= 64 && !stacks(m, code, type, qual, cnt)) {
            int best = -1;
            for (int k = 0; k < size && k < 5; k++) {
                if (k == m || g_rb(G_MEMBER(k)) == 0) continue;
                if (stacks(k, code, type, qual, cnt)) { best = k; break; }
                if (g_rb((u16)(G_MEMBER(k) + G_M_NITEMS)) < 64 && (best < 0 || g_rb((u16)(G_MEMBER(k) + G_M_NITEMS)) < g_rb((u16)(G_MEMBER(best) + G_M_NITEMS)))) best = k;
            }
            if (best >= 0) {
                if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix fullpack: item %d for %d goes to %d\n", code & 0xfff, m, best);
                ww(SS, (u16)(SP + 4), (u16)best);
            } else fprintf(stderr, "fullpack: no room anywhere for item %d\n", code & 0xfff);
        }
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- castle */
static void castle_hook(void)              /* combat_over */
{
    if ((g_rb(0x9080) & 1) && g_rb(0x9084) != 1)
        for (int i = 5; i < 13; i++)
            if (g_rb((u16)(0xc1f3 + i)) == 1 && (g_rb((u16)(0xa696 + 2 * i)) & 0x80)) {
                if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix castle: boss %d is dead\n", i);
                g_wb(0x9084, 1);
                break;
            }
    hook_call_original();
}

/* ---------------------------------------------------------------- bounty */
static void bounty_hook(void)              /* $Raubi01 card, s080 1870:000c */
{
    int giver = (s16)g_rw(0xe896), loc = (s16)g_rw(G_CUR_LOC), match = 0, only = -1, n = 0;
    for (int i = 0; i < 300; i++) {
        u32 e = ev_ptr(i);
        if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x24 || (s16)rw(s, (u16)(o + 0x28)) != 3 || (s16)rw(s, (u16)(o + 0x1c)) != loc) continue;
        int owner = (s16)rw(s, (u16)(o + 0x1a));
        if (owner == giver) match = 1;
        only = owner; n++;
    }
    if (!match && n >= 1) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix bounty: giver %d -> %d\n", giver, only);
        g_ww(0xe896, (u16)only);
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- alchemy */
static int stock(int id)
{
    int n = 0, size = (s16)g_rw(G_PARTY_SIZE);
    for (int m = 0; m < 5 && m < size; m++) {
        int k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
        for (int j = 0; j < k && j < 64; j++)
            if ((rw(G_FAR2E38, G_ITEM(m, j)) & 0xfff) == id) n += rb(G_FAR2E38, (u16)(G_ITEM(m, j) + 4));
    }
    return n;
}

static void alchemy_hook(void)             /* s033 1462:1630 (first, id, n): take n of item id */
{
    int first = (s16)rw(SS, (u16)(SP + 4)), id = (s16)rw(SS, (u16)(SP + 6)), n = (s16)rw(SS, (u16)(SP + 8));
    int before = stock(id);
    hook_call_original();
    int left = n - (before - stock(id));
    void (*f)(void) = game_func(33, 0x1462, 0x1630);
    if (left > 0 && f) {
        hook_remove(33, 0x1462, 0x1630);
        for (int tries = 0; left > 0 && tries < 64; tries++) {
            int s0 = stock(id);
            if (!s0) break;
            u16 args[3] = { (u16)first, (u16)id, (u16)left };
            game_call_native(f, 3, args);
            int took = s0 - stock(id);
            if (took <= 0) break;
            left -= took;
        }
        hook_install(33, 0x1462, 0x1630, alchemy_hook);
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix alchemy: item %d: %d needed, %d still missing\n", id, n, left);
    }
}

/* ---------------------------------------------------------------- colours
 * member_palette (far 20a5:33fc, 24 bytes per party slot) and the figure codes (DS:a668, 4
 * per slot) only change at character creation (s038), when a game is loaded (s037, or before
 * the game proper) or when slots move (a member leaves). Anything else is the "neon hair"
 * corruption: the slot gets its last good colours back. */
#include "../runtime/dispatch.h"
int ui_is_open(void);
#define G_FAR20A5 ((u16)(LOADSEG + 0x20a5))
static u8 col_good[5][28]; static int col_have;
static void col_read(u8 cur[5][28])
{
    for (int m = 0; m < 5; m++) {
        for (int i = 0; i < 24; i++) cur[m][i] = rb(G_FAR20A5, (u16)(0x33fc + m * 24 + i));
        for (int i = 0; i < 4; i++) cur[m][24 + i] = g_rb((u16)(0xa668 + m * 4 + i));
    }
}
void fixes_frame(void)
{
    if (!fix_on("colours")) { col_have = 0; return; }
    u8 cur[5][28]; col_read(cur);
    if (!col_have || game_mode() == GM_OTHER || ovl_loaded[37] || ovl_loaded[38] || ui_is_open()) { memcpy(col_good, cur, sizeof cur); col_have = 1; return; }
    if (!memcmp(cur, col_good, sizeof cur)) return;
    int size = (s16)g_rw(G_PARTY_SIZE), moved = 1;
    for (int m = 0; m < size && m < 5; m++) {           /* slots that moved are fine */
        int found = 0;
        for (int k = 0; k < 5 && !found; k++) found = !memcmp(cur[m], col_good[k], 28);
        if (!found) moved = 0;
    }
    if (moved) { memcpy(col_good, cur, sizeof cur); return; }
    for (int m = 0; m < size && m < 5; m++) {
        if (!memcmp(cur[m], col_good[m], 28)) continue;
        if (col_good[m][24] < 'A' || col_good[m][24] > 'Z') continue;    /* the slot was empty: a new member */
        fprintf(stderr, "fix colours: member %d's colours/figure changed outside creation and loading (mode %d): put back\n", m, game_mode());
        if (getenv("DL_FIX_LOG")) {
            fprintf(stderr, "  was:"); for (int i = 0; i < 28; i++) fprintf(stderr, " %02x", col_good[m][i]);
            fprintf(stderr, "\n  now:"); for (int i = 0; i < 28; i++) fprintf(stderr, " %02x", cur[m][i]);
            fprintf(stderr, "\n  loaded:"); for (int i = 0; i < 140; i++) if (ovl_loaded[i]) fprintf(stderr, " %d", i);
            fprintf(stderr, "\n");
        }
        for (int i = 0; i < 24; i++) wb(G_FAR20A5, (u16)(0x33fc + m * 24 + i), col_good[m][i]);
        for (int i = 0; i < 4; i++) g_wb((u16)(0xa668 + m * 4 + i), col_good[m][24 + i]);
    }
    col_read(cur); memcpy(col_good, cur, sizeof cur);
}

/* ---------------------------------------------------------------- innsave */
static void innsave_hook(void)             /* the lodging screen, s036 1462:0082 */
{
    hook_call_original();
    if (g_rb(0x9920) == 2 || g_rb(0x9934) == 2) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix innsave: Save/Load enabled again\n");
        g_wb(0x9920, 1); g_wb(0x9934, 1);
    }
}

/* ---------------------------------------------------------------- copyprot */
static void copyprot_hook(void)            /* copy_protection(), s041 1462:0000: answered */
{
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix copyprot: skipped\n");
    hook_return_far(0);
}

/* ---------------------------------------------------------------- switching */
static const struct { int sect; unsigned seg, off; void (*fn)(void); } HOOKS[] = {
    { 29, 0x0e76, 0x4c04, blessing_hook }, { 14, 0x18bf, 0x0778, fireward_hook }, { 31, 0x18e7, 0x01b4, additem_hook },
    { 14, 0x0e76, 0x08fe, castle_hook }, { 80, 0x1870, 0x000c, bounty_hook }, { 33, 0x1462, 0x1630, alchemy_hook },
    { 31, 0x18e7, 0x01b4, additem_hook }, { 36, 0x1462, 0x0082, innsave_hook }, { -2, 0, 0, 0 }, { 41, 0x1462, 0x0000, copyprot_hook } };

void fix_enable(int i, int on)
{
    FIXES[i].on = on;
    char key[64]; snprintf(key, sizeof key, "fix.%s", FIXES[i].id);
    config_set_int(key, on);
    int shared = 0;                         /* another fix on the same function still on */
    for (int k = 0; k < FIX_COUNT; k++)
        if (k != i && FIXES[k].on && HOOKS[k].sect == HOOKS[i].sect && HOOKS[k].seg == HOOKS[i].seg && HOOKS[k].off == HOOKS[i].off) shared = 1;
    if (!HOOKS[i].fn) return;                /* checked every frame (fixes_frame), no hook */
    if (on) { if (!hook_install(HOOKS[i].sect, HOOKS[i].seg, HOOKS[i].off, HOOKS[i].fn)) fprintf(stderr, "fix %s: no hook point\n", FIXES[i].id); }
    else if (!shared) hook_remove(HOOKS[i].sect, HOOKS[i].seg, HOOKS[i].off);
}

void fixes_init(void)
{
    for (int i = 0; i < FIX_COUNT; i++) {
        char key[64]; snprintf(key, sizeof key, "fix.%s", FIXES[i].id);
        fix_enable(i, config_get_int(key, FIXES[i].on));
    }
}
