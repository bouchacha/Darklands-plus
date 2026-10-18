/* fixes.c - fixes for bugs that are still in the last official version (483.07). Each can
 * be switched off (F12 > Settings > Helpers > Fixes); "play the original" switches them all off.
 *
 *  blessing  Saint blessings (and potions) that raise an attribute record how much they
 *            added before the attribute is capped (raise_attr_temp, s029 0e76:4c04: the cap
 *            is max + the LARGEST running boost, add_attr also clamps at 99). When they run
 *            out the full recorded amount is taken off again, so a second blessing on the
 *            same attribute, or one near 99, costs points for good (Int never comes back).
 *            Fixed by recording what was really added. And when a boost runs out (s029
 *            0e76:448a) it is taken off whatever the attribute is by then: endurance a night's
 *            rest has put back to its maximum, or strength wounds have taken, lost the boost
 *            again (a member at 3 strength whose +4 ran out died). Now only what is still above
 *            the maximum (and the other boosts and lowerings running) goes. Saints that lower an
 *            attribute for a while (Perception, Intellect, Agility, Charisma: a negative d) are
 *            recorded as negative and given back when they run out, never past the maximum.
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
 *            cost only part of their ingredients. The remainder is now taken too.
 *  potions   Deadly Blade (+3..5 damage), Strongedge (+3..5 penetration) and Greatpower
 *            (14/22/30 added to the whole byte) change the member's bonus byte, party +0x34
 *            (= combatant +0x48: high nibble damage, low nibble penetration), in use_potion
 *            (s031 1462:43b2), and record a kind 0x4a event of about a day. Its expiry (s029
 *            0e76:448a) only undoes kinds 0x48 and 0x49, so the bonus stays for good; the
 *            nibbles wrap (a fourth Deadly Blade gives +1 damage, Greatpower's 14 spills into
 *            the damage nibble), and hit_damage's penetration (byte + weapon) & 15 wraps too:
 *            "after Greatpower my bows only do 3-5 damage". The byte is now the sum of the
 *            potions still running, each nibble capped at 15, and penetration stops at 15.
 *  bestweapon member_best_skill (s029 0e76:01c0) sets best_skill for every skill it looks at
 *            (missing braces), so it always ends as 6 (Missile Devices); the practice of a
 *            card (s053 1838:074e: practice_skill(leader, best_skill ...)) then trained that.
 *            best_skill is set to the first of the highest weapon skills afterwards.
 *  offmap    One location of DARKLAND.LOC (record 373, a cave) lies at y 931, one row below the
 *            931-row map: quests that send the party there cannot be finished ("quests send
 *            you off the map"). It is moved onto the last row.
 *  itemcount When robbers take the party's things (lose_items, s031 18e7:0ab8: $Raubi02's
 *            surrender and others), the loop over a member's items starts at the count, one
 *            past the last item, and remove_item_rec (18e7:0406) lowers the count even for an
 *            item that is not there: one item too many is taken, and a member who had nothing
 *            left ends with -1 items (party +0x6a is a signed byte). When that member falls
 *            in battle, his pile is made (s014 0e76:2324) with _fmemcpy(DS:a895, items,
 *            count * 6): 65530 bytes over the whole data segment, stack included, and the
 *            game dies ("left through an unmatched return"). remove_item_rec and the battle's
 *            use_item_idx (s014 18bf:0fa4, the same flaw) now ignore an index past the
 *            items; counts are also kept within 0..64 (16 for enemies), so saves that carry
 *            -1 mend themselves.  *  testcard  Card 0 is the developers' test card ($Initi00), whose picture (pics\xhouse3.pic) the game
 *            does not have: a card that goes back to where it came from (prev_screen) went there
 *            when that was 0, and the game stopped. It now goes to the map or the town.
 *  teachers  The lodging screen (s036 1462:0082) lists the town's teachers (kind 0x28 events) into
 *            DS:8a38 with loops that walk every kind 0x28 event (event_next) but stop after as
 *            many as this town has (s029 0e76:37e2) and number them all: when another town's
 *            teachers come first in the table (two towns with teachers within their week), the
 *            list is empty and DS:8a38 stale, Train (T) finds no teacher (event_find = -1) and
 *            reads events[-1]: a cost of -4096 pfennigs ("Train or Study (-4096pfs)") and a
 *            total that adds 4096; the day itself (camp_spend_day) finds no teacher either, so
 *            nothing is learnt and the member is set back to relaxing. While that screen runs,
 *            event_next(0x28, i) now returns only this town's teachers that have begun.
 *  armourwear Identical things share one record (add_item_rec merges code + type + quality),
 *            so a worn armour and its spares are one stack. In battle, wear (acid, fire:
 *            s019 1ef8:0968) lowers the quality of that whole record, and when it is used up
 *            (1ef8:0914) consume_item is called until the record is gone: the spares carried
 *            lose quality with the worn one and are destroyed with it. Now one is taken off
 *            the stack and worn down alone, and only that one is thrown away.
 *  cardopts  Options some cards grey by mistake: $Selec01 (out over the city wall) tests the
 *            purse against the sewer-without-horses option instead of the bribe, so the bribe
 *            could be chosen without the money; $Revol14 (the crowd at the fortress) greys the
 *            climb with a rope when the party has a rope; $CityW02 (over the wall after the
 *            guards) counts only the last member's rope. Their opt_on[] put right before the
 *            option list is shown (card_options, and from the card view's own drawing of it).
 *  debtnote  A debt-note errand's papers lie in the office its giver names (below, debtnote).
 *  alchrisk  A failed brew's accident follows the formula's risk the right way round (below).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "fixes.h"

Fix FIXES[] = {
    { "blessing", "Blessings no longer cost attribute points for good",
      "A second blessing on the same attribute, or one near 99, used to take points away when it wore off; and a boost to endurance or strength that a night's rest or a wound had used up was taken off again when it ran out (a badly wounded member could die of it).", 1 },
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
    { "potions", "Weapon potions wear off and don't ruin weapons",
      "Deadly Blade, Strongedge and Greatpower never wore off, and their bonuses overflowed: after a few potions (or one Greatpower) weapons, bows above all, hit for almost nothing. They now last their day and add up properly.", 1 },
    { "offmap", "No quest destinations off the edge of the map",
      "One cave lies a row beyond the bottom of the map, so quests sent there could never be finished. It is moved onto the map.", 1 },
    { "bestweapon", "Weapon practice trains the weapon actually used best",
      "Where a card lets the leader practise with the weapon he or she is best at, the original always trained Missile Devices.", 1 },
    { "itemcount", "Robbers take what is there (and no crash after)",
      "When robbers took the party's things, the game removed one item too many; a character left with nothing ended up with -1 items, and if he or she later died in battle the game overwrote its own memory and stopped. Fixed; saved games with the fault mend themselves.", 1 },
    { "storeroom", "Nothing lost in store rooms",
      "A store room holds 100 kinds of things and its count of one thing ran over past 127 (shown as minus numbers, then lost). Putting more in is now refused with a word why.", 1 },
    { "testcard", "No stop on the developers' test card",
      "Card 0 is the developers' test card ($Initi00, \"Try it...you will like it!\"); a card that returns to the card it came from can go there when it came from none, and the game stops on its missing picture (pics\\xhouse3.pic). It now goes on to the map, or the town the party is in.", 1 },
    { "armourwear", "Only the armour worn wears out",
      "Identical pieces of armour are kept as one stack, so when acid or fire wore down the one being worn, every spare of the same kind and quality carried by that member lost quality too, and all were destroyed with it. Now only the worn one suffers.", 1 },
    { "teachers", "Training at the inn finds the town's teachers",
      "When another town's teachers were still about, the inn's Train or Study listed nobody and showed a cost like -4096 pfennigs (and a purse that grew); the day then passed with nothing learnt. The inn now lists this town's teachers and charges their fee.", 1 },
    { "cardopts", "Card options greyed by mistake",
      "Getting out over a city wall, the bribe could be offered without the money (the purse was checked against crawling out through the sewer instead); at a rebel crowd's fortress the climb with a rope was greyed when the party had a rope; climbing a wall after the guards counted only the last member's rope; and at the wall by an old sally port each line from the second on did the next line's deed (three \"abandon the horses\" lines the game never handles pushed them out of step).", 1 },
    { "patronsaint", "Asking a cathedral about its patron saint teaches the saint",
      "The answer was meant to teach the leader the saint, but the game wrote it for \"member -1\": nobody learnt the saint, and a bit of the fifth member's formulae was set instead.", 1 },
    { "debtnote", "Debt notes are where the merchant says",
      "A merchant who sends the party for debt notes names who holds them (the Medici or the Fugger), but the game hid them in another office above the marketplace, so breaking into the named one found nothing. They are now in the office named.", 1 },
    { "alchrisk", "A formula's risk the right way round",
      "Each formula is marked low, medium or high risk, and the risk decides how badly a failed batch at an inn, a village or a hideout goes: an explosion (the party hurt, damages to pay, no room at the inn for four months) or only a stink (one month). The game had it backwards: a failure of a low-risk formula exploded three times in four, a high-risk one's half the time. Now low risk explodes one time in four, medium one in three, high one in two.", 1 },
};
int FIX_COUNT = (int)(sizeof FIXES / sizeof FIXES[0]);

/* ---------------------------------------------------------------- blessing */
#define ev_ptr g_event

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
        if (rec >= 0) { if (added < 0) added = 0; if (added > rec) added = rec; }     /* a boost: what it really added */
        else { if (added > 0) added = 0; if (added < rec) added = rec; }              /* a saint's lowering (some lower Perception, Intellect ...): what it really took */
        if (added != rec) {
            if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix blessing: member %d attr %d recorded %d, really added %d\n", m, a, rec, added);
            ww(s, (u16)(o + 0x1e), (u16)added);
        }
    }
}

/* a boost runs out (s029 0e76:448a, the expiry of kinds 0x48 / 0x49 (event i): +0x1a the member,
   +0 the attribute, +0x1e the points): it takes off no more than what is still above what the
   attribute should be without it (the maximum and the other boosts and lowerings still running);
   a lowering that runs out gives back no more than up to that. (A lowering pulls the attribute
   below the maximum: a boost that ends while it runs takes its whole amount, as the lowering
   gives it back later.) */
static void blessing_end_hook(void)
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    u32 e = i >= 0 && i < 300 ? ev_ptr(i) : 0;
    if (!e) { hook_call_original(); return; }
    u16 s = (u16)(e >> 16), o = (u16)e;
    int m = (s16)rw(s, (u16)(o + 0x1a)), a = (s16)rw(s, o), rec = (s16)rw(s, (u16)(o + 0x1e));
    if ((s16)rw(s, (u16)(o + 0x22)) != 0x48 || m < 0 || m >= 5 || a < 0 || a >= 7 || rec == 0) { hook_call_original(); return; }
    int others = 0;                                          /* the member's other boosts and lowerings of this attribute */
    for (int j = 0; j < 300; j++) {
        u32 f = j != i ? ev_ptr(j) : 0;
        if (!f) continue;
        u16 fs = (u16)(f >> 16), fo = (u16)f;
        if ((s16)rw(fs, (u16)(fo + 0x22)) == 0x48 && (s16)rw(fs, (u16)(fo + 0x1a)) == m && (s16)rw(fs, fo) == a) others += (s16)rw(fs, (u16)(fo + 0x1e));
    }
    int cur = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a)), max = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a)), base = max + others;
    int now;                                                 /* what the expiry takes off (a lowering: negative, given back) */
    if (rec > 0) { now = cur - base; if (now < 0) now = 0; if (now > rec) now = rec; }
    else { now = cur - base; if (now > 0) now = 0; if (now < rec) now = rec; }
    if (now == rec) { hook_call_original(); return; }
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix blessing: member %d attr %d runs out: %d off, not %d (%d/%d, others %d)\n", m, a, now, rec, cur, max, others);
    ww(s, (u16)(o + 0x1e), (u16)now);
    hook_call_original();
    if (ev_ptr(i) == e && (s16)rw(s, (u16)(o + 0x1e)) == now) ww(s, (u16)(o + 0x1e), (u16)rec);
}

/* ---------------------------------------------------------------- fireward (+ potions) */
static int fix_on(const char *id);
static void hitdamage_hook(void)           /* hit_damage(a, t) */
{
    int a = (s16)rw(SS, (u16)(SP + 4)), t = (s16)rw(SS, (u16)(SP + 6));
    u16 ta = (u16)(0x9c55 + 0x80 * t + 0x55), aa = (u16)(0x9c55 + 0x80 * a + 0x55), ba = (u16)(0x9c55 + 0x80 * a + 0x48);
    int fire = fix_on("fireward");
    int fix_t = fire && t >= 0 && t < 5, fix_a = fire && a >= 0 && a < 5;
    u8 st = fix_t ? g_rb(ta) : 0, sa = fix_a ? g_rb(aa) : 0, sb = 0;
    int fix_b = 0;
    if (fix_t) g_wb(ta, (u8)(st & 0x8f));
    if (fix_a && a != t) g_wb(aa, (u8)(sa & 0x7f));
    if (fix_on("potions") && a >= 0 && a < 5) {   /* penetration = (bonus + weapon) & 15: stop at 15 */
        sb = g_rb(ba);
        int wp = g_rb((u16)(GV_combatants + 0x80 * a + 0x53)) & 15;
        if ((sb & 15) + wp > 15) {
            g_wb(ba, (u8)((sb & 0xf0) | (15 - wp))); fix_b = 1;
            if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix potions: %d hits %d, penetration %d + %d held at 15\n", a, t, sb & 15, wp);
        }
    }
    hook_call_original();
    if (fix_b) g_wb(ba, sb);
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
    if ((g_rb(GV_combat_flags) & 1) && g_rb(0x9084) != 1)
        for (int i = 5; i < 13; i++)
            if (g_rb((u16)(0xc1f3 + i)) == 1 && (g_rb((u16)(GV_cbt_flags + 2 * i)) & 0x80)) {
                if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix castle: boss %d is dead\n", i);
                g_wb(0x9084, 1);
                break;
            }
    hook_call_original();
}

/* ---------------------------------------------------------------- bounty */
static void bounty_hook(void)              /* $Raubi01 card, s080 1870:000c */
{
    int giver = (s16)g_rw(GV_saved_card2), loc = (s16)g_rw(G_CUR_LOC), match = 0, only = -1, n = 0;
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
        g_ww(GV_saved_card2, (u16)only);
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
        hook_remove(33, 0x1462, 0x1630, alchemy_hook);
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
        for (int i = 0; i < 4; i++) cur[m][24 + i] = g_rb((u16)(GV_member_shield + m * 4 + i));
    }
}
static void colours_frame(void)
{
    if (!fix_on("colours")) { col_have = 0; return; }
    u8 cur[5][28]; col_read(cur);
    extern unsigned dos_save_read_ms; u32 plat_ticks_ms(void);
    /* a game was loaded (quick load, autosave, the save browser, Alt-L, Quickstart): its colours
       come in over the next moments; counted in frames as well as time (a first start of the MT-32
       or a slow disk held the frames for longer than the time: the stale colours went back) */
    static unsigned last_read; static int grace;
    if (dos_save_read_ms != last_read) { last_read = dos_save_read_ms; grace = 300; }
    static int prev_mode = -1;                                   /* (the game proper just began: Quickstart, a new game) */
    if (prev_mode == GM_OTHER && game_mode() != GM_OTHER && grace < 300) grace = 300;
    prev_mode = game_mode();
    if (grace > 0) grace--;
    int loading = grace > 0 || (dos_save_read_ms && plat_ticks_ms() - dos_save_read_ms < 4000);
    if (!col_have || loading || game_mode() == GM_OTHER || ovl_loaded[37] || ovl_loaded[38] || ui_is_open()) { memcpy(col_good, cur, sizeof cur); col_have = 1; return; }
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
        { int z = 1; for (int i = 0; i < 24; i++) if (col_good[m][i]) z = 0; if (z) continue; }   /* (no colours kept yet: nothing to put back) */
        fprintf(stderr, "fix colours: member %d's colours/figure changed outside creation and loading (mode %d): put back\n", m, game_mode());
        if (getenv("DL_FIX_LOG")) {
            fprintf(stderr, "  was:"); for (int i = 0; i < 28; i++) fprintf(stderr, " %02x", col_good[m][i]);
            fprintf(stderr, "\n  now:"); for (int i = 0; i < 28; i++) fprintf(stderr, " %02x", cur[m][i]);
            fprintf(stderr, "\n  loaded:"); for (int i = 0; i < 140; i++) if (ovl_loaded[i]) fprintf(stderr, " %d", i);
            fprintf(stderr, "\n");
        }
        for (int i = 0; i < 24; i++) wb(G_FAR20A5, (u16)(0x33fc + m * 24 + i), col_good[m][i]);
        for (int i = 0; i < 4; i++) g_wb((u16)(GV_member_shield + m * 4 + i), col_good[m][24 + i]);
    }
    col_read(cur); memcpy(col_good, cur, sizeof cur);
}
static void potions_frame(void);
static void offmap_frame(void)                 /* locations below or right of the map onto its edge */
{
    static u32 n;
    if ((n++ & 63) || game_mode() == GM_OTHER) return;
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int x = (s16)rw(s, (u16)(o + 4)), y = (s16)rw(s, (u16)(o + 6));
        if (y > 930 && y < 940) { ww(s, (u16)(o + 6), 930); if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix offmap: location %d y %d -> 930\n", k, y); }
        if (x > 326 && x < 336) { ww(s, (u16)(o + 4), 326); if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix offmap: location %d x %d -> 326\n", k, x); }
    }
}
/* ---------------------------------------------------------------- itemcount */
static int clamp_items(int i)              /* combatant i's item count into 0..64 (enemies 0..16) */
{
    u16 a = (u16)(0x9c55 + 0x80 * i + 0x7e);  /* = party member i + 0x6a */
    int n = (s8)g_rb(a), max = i < 5 ? 64 : 16;
    if (n >= 0 && n <= max) return 0;
    fprintf(stderr, "fix itemcount: %s %d had %d items: %d\n", i < 5 ? "member" : "combatant", i, n, n < 0 ? 0 : max);
    g_wb(a, (u8)(n < 0 ? 0 : max));
    return 1;
}
static void itemcount_frame(void)
{
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    for (int m = 0; m < size; m++) clamp_items(m);
}
static void falls_hook(void)               /* s014 0e76:2324 (i, ...): a combatant falls; his items become a pile */
{
    int i = (s16)rw(SS, (u16)(SP + 2));      /* near call */
    if (i >= 0 && i < 13) clamp_items(i);
    hook_call_original();
}
static int idx_missing(int who, int idx)   /* who (0..12) has no item idx */
{
    if (who < 0 || who > 12) return 0;
    int n = (s8)g_rb((u16)(GV_combatants + 0x80 * who + 0x7e));
    if (idx >= 0 && idx < n) return 0;
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix itemcount: %d has no item %d (%d items): nothing removed\n", who, idx, n);
    return 1;
}
static void removeitem_hook(void)          /* remove_item_rec(who, idx), s031 18e7:0406 */
{
    if (idx_missing((s16)rw(SS, (u16)(SP + 4)), (s16)rw(SS, (u16)(SP + 6)))) { hook_return_far(0); return; }
    hook_call_original();
}
static void useitem_hook(void)             /* use_item_idx(who, idx): one of a stack used up, s014 18bf:0fa4 */
{
    if (idx_missing((s16)rw(SS, (u16)(SP + 4)), (s16)rw(SS, (u16)(SP + 6)))) { AX = 0xffff; hook_return_far(0); return; }
    hook_call_original();
}

void fixes_frame(void)
{
    colours_frame();
    if (fix_on("itemcount")) itemcount_frame();
    if (fix_on("potions")) potions_frame();
    if (fix_on("offmap")) offmap_frame();
}

/* ---------------------------------------------------------------- potions
 * The kind 0x4a events say which potions are still running: +0 0x69 for Deadly Blade, else the
 * potion's inventory index (the original passes idx for Strongedge and Greatpower), +0x1a the
 * member, +0x1e the amount (3..5 for the first two, 14/22/30 for Greatpower), +0x12 the end
 * (hour, day, month, year). The bonus byte is rebuilt from them every frame: it is nothing else
 * (only use_potion writes it), so saves spoilt by the old bug mend themselves. */
static int ev_running(u16 s, u16 o)
{
    static const u16 f[4] = { 0x18, 0x16, 0x14, 0x12 }, now[4] = { G_CUR_YEAR, G_CUR_MONTH, G_CUR_DAY, G_CUR_HOUR };
    for (int k = 0; k < 4; k++) {
        int e = (s16)rw(s, (u16)(o + f[k])), n = (s16)g_rw(now[k]);
        if (e != n) return e > n;
    }
    return 0;
}

void potion_bonus(int m, int *dmg, int *pen)
{
    *dmg = *pen = 0;
    for (int i = 0; i < 300; i++) {
        u32 e = ev_ptr(i);
        if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x4a || (s16)rw(s, (u16)(o + 0x1a)) != m || !ev_running(s, o)) continue;
        int type0 = (s16)rw(s, o), t = (s16)rw(s, (u16)(o + 0x1e));
        if (t < 0 || t > 0x7f) continue;
        if (type0 == 0x69) *dmg += t;                      /* Deadly Blade */
        else if (t <= 5) *pen += t;                         /* Strongedge */
        else { *dmg += t >> 4; *pen += t & 15; }            /* Greatpower, without the carry */
    }
    if (*dmg > 15) *dmg = 15;
    if (*pen > 15) *pen = 15;
}

static void potions_frame(void)
{
    if (game_mode() == GM_OTHER) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    for (int m = 0; m < size; m++) {
        int d, p; potion_bonus(m, &d, &p);
        u8 v = (u8)(d << 4 | p), was = g_rb((u16)(G_MEMBER(m) + 0x34));
        if (v == was) continue;
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix potions: member %d bonus %02x -> %02x\n", m, was, v);
        g_wb((u16)(G_MEMBER(m) + 0x34), v);
    }
}

/* ---------------------------------------------------------------- innsave */
static void innsave_hook(void)             /* the lodging screen, s036 1462:0082 */
{
    hook_call_original();
    if (g_rb(GV_font_flag_a) == 2 || g_rb(GV_font_flag_b) == 2) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix innsave: Save/Load enabled again\n");
        g_wb(GV_font_flag_a, 1); g_wb(GV_font_flag_b, 1);
    }
}

/* ---------------------------------------------------------------- copyprot */
static void copyprot_hook(void)            /* copy_protection(), s041 1462:0000: answered */
{
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix copyprot: skipped\n");
    hook_return_far(0);
}

/* ---------------------------------------------------------------- bestweapon */
static void bestweapon_hook(void)          /* member_best_skill(who), s029 0e76:01c0 */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    if (who == -1) who = (s8)g_rb(G_LEADER);
    if (who < 0 || who > 4) return;
    int best = 0, bv = 0;                      /* the first of the highest weapon skills, as intended */
    for (int i = 0; i < 7; i++) { int v = g_rb((u16)(G_MEMBER(who) + G_M_SKILLS + i)); if (v > bv) { bv = v; best = i; } }
    if (getenv("DL_FIX_LOG") && g_rb(GV_best_skill) != best) fprintf(stderr, "fix bestweapon: member %d best skill %d (the game said %d)\n", who, best, g_rb(GV_best_skill));
    g_wb(GV_best_skill, (u8)best);                    /* best_skill */
}

/* ---------------------------------------------------------------- switching */
/* ---------------------------------------------------------------- storeroom
 * The store rooms (s035): the open store is a table at far 2e38:37f4, 4 bytes an entry {item,
 * quality, number (signed char)}, room for 100 (the next table starts at 3984); DS:89f6 the
 * entries, DS:89f2 the member shown, DS:37c4[5] the list in use (0 the member's), DS:37ce[5]
 * + DS:37e2[5] the member's cursor. cache_add stacks without a limit and appends without
 * one: P (put) is refused when it would overflow either. */
void ui_notice(const char *msg);
/* why a P (put) with key `key` would overflow the open store, or 0 (also mods/sharedstore.c) */
const char *storeroom_put_refusal(int key)
{
    int m = (s16)g_rw(0x89f2);
    if ((key == 'p' || key == 'P') && m >= 0 && m < 5 && !g_rw((u16)(0x37c4 + 2 * m)) && g_rb((u16)(G_MEMBER(m) + G_M_NITEMS))) {
        int idx = (s16)g_rw((u16)(0x37e2 + 2 * m)) + (s16)g_rw((u16)(0x37ce + 2 * m));
        if (idx >= 0 && idx < 64) {
            u16 o = G_ITEM(m, idx);
            int id = rw(G_FAR2E38, o) & 0xfff, q = rb(G_FAR2E38, (u16)(o + 3));
            unsigned n = g_rw(0x89f6); int found = -1;
            for (unsigned i = 0; i < n && i < 100; i++) {
                u16 e = (u16)(0x37f4 + 4 * i);
                if ((rw(G_FAR2E38, e) & 0xfff) == id && rb(G_FAR2E38, (u16)(e + 2)) == q) { found = (int)i; break; }
            }
            return found >= 0 ? ((s8)rb(G_FAR2E38, (u16)(0x37f4 + 4 * found + 3)) >= 127 ? "The store room keeps at most 127 of one thing." : 0)
                              : (n >= 100 ? "The store room is full: it keeps 100 kinds of things." : 0);
        }
    }
    return 0;
}
int storeroom_fix_on(void) { return fix_on("storeroom"); }
static void storeroom_hook(void)           /* cache_key(key), s035 1462:0a46 */
{
    const char *why = storeroom_put_refusal(rw(SS, (u16)(SP + 4)) & 0xff);
    if (why) {
        ui_notice(why);
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix storeroom: %s\n", why);
        hook_return_far(0);
        return;
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- testcard
 * Card 0 (the card table's first entry, s044 1838:0000, $Initi00) is the developers' test card:
 * it sets the purse to 200 florins and loads pics\xhouse3.pic, which the game does not have, so
 * the game stops (exit 99). A card that ends by going back where it came from (next_card =
 * prev_screen: the town gate's challenge, s044 18d1, after its fight) goes there when prev_screen
 * is 0. Instead: the town's own card if the party is in one (its location record +0x0c, as the
 * map enters it), else the map (card 12). */
static void testcard_hook(void)            /* card 0 (), s044 1838:0000 */
{
    int loc = (s16)g_rw(GV_cur_loc), to = 12;
    if (loc >= 0 && loc < 92) { u32 r = g_location(loc); if (r) { int c = (s16)rw((u16)(r >> 16), (u16)((u16)r + 0x0c)); if (c > 0) to = c; } }
    fprintf(stderr, "fix testcard: card 0 asked for (prev_screen %d, cur_loc %d, mode %d): card %d instead\n",
            (s16)g_rw(GV_prev_screen), loc, game_mode(), to);
    g_ww(GV_next_card, (u16)to);
    AX = (u16)to;
    hook_return_far(0);
}

/* ---------------------------------------------------------------- teachers */
static int lodging_open;
void fixes_lodging(int open) { lodging_open = open; }      /* (the native lodging screen, which runs instead of the original) */
static void teachers_lodging_hook(void)    /* the lodging screen, s036 1462:0082 */
{
    lodging_open = 1;
    hook_call_original();
    lodging_open = 0;
}
static int ev_started(u32 e)
{
    u16 s = (u16)(e >> 16), o = (u16)e;
    int d[4][2] = { { (s16)rw(s, (u16)(o + 0x10)), (s16)g_rw(GV_cur_year) }, { (s16)rw(s, (u16)(o + 0x0e)), (s16)g_rw(GV_cur_month) },
                    { (s16)rw(s, (u16)(o + 0x0c)), (s16)g_rw(GV_cur_day) }, { (s16)rw(s, (u16)(o + 0x0a)), (s16)g_rw(GV_cur_hour) } };
    for (int k = 0; k < 3; k++) { if (d[k][0] > d[k][1]) return 0; if (d[k][0] < d[k][1]) return 1; }
    return d[3][0] <= d[3][1];
}
static void teachers_next_hook(void)       /* event_next(k, i), s029 0e76:324c */
{
    int k = (s16)rw(SS, (u16)(SP + 4)), i = (s16)rw(SS, (u16)(SP + 6));
    if (!lodging_open || k != 0x28 || i < 0) { hook_call_original(); return; }
    int loc = (s16)g_rw(GV_cur_loc), r = -1;
    for (; i < 300 && r < 0; i++) {
        u32 e = ev_ptr(i);
        if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) == 0x28 && (s16)rw(s, (u16)(o + 0x1c)) == loc && ev_started(e)) r = i;
    }
    if (r >= 0) g_ww(GV_last_event, (u16)r);
    AX = (u16)r;
    hook_return_far(0);
}

/* ---------------------------------------------------------------- armourwear */
#define CB_NITEMS(i) ((u16)(GV_combatants + (i) * 0x80 + 0x7e))
static int inv_find(int i, int type, int q)
{
    int n = (s8)g_rb(CB_NITEMS(i));
    for (int k = 0; k < n && k < 64; k++)
        if (rb(G_FAR2E38, (u16)(G_ITEM(i, k) + 2)) == type && rb(G_FAR2E38, (u16)(G_ITEM(i, k) + 3)) == q) return k;
    return -1;
}
static void armourwear_hook(void)          /* s019 1ef8:0968 (i, type, quality, amount): wear an armour down */
{
    int i = (s16)rw(SS, (u16)(SP + 4)), type = rw(SS, (u16)(SP + 6)) & 0xff, q = rw(SS, (u16)(SP + 8)) & 0xff, amt = (s16)rw(SS, (u16)(SP + 10));
    if (i < 0 || i >= 5) { hook_call_original(); return; }
    int k = inv_find(i, type, q), n = (s8)g_rb(CB_NITEMS(i));
    if (k < 0) { hook_call_original(); return; }
    u16 o = G_ITEM(i, k);
    int cnt = rb(G_FAR2E38, (u16)(o + 4));
    if (cnt <= 1) { hook_call_original(); return; }
    u8 nq = (u8)(q - amt);
    u16 id = rw(G_FAR2E38, o);
    int to = -1;
    for (int j = 0; j < n && j < 64; j++) {
        u16 p = G_ITEM(i, j);
        if (j != k && rw(G_FAR2E38, p) == id && rb(G_FAR2E38, (u16)(p + 2)) == type && rb(G_FAR2E38, (u16)(p + 3)) == nq && rb(G_FAR2E38, (u16)(p + 4)) < 255) { to = j; break; }
    }
    if (to >= 0) wb(G_FAR2E38, (u16)(G_ITEM(i, to) + 4), (u8)(rb(G_FAR2E38, (u16)(G_ITEM(i, to) + 4)) + 1));
    else if (n < 64) {
        u16 p = G_ITEM(i, n);
        for (int b = 0; b < 6; b++) wb(G_FAR2E38, (u16)(p + b), rb(G_FAR2E38, (u16)(o + b)));
        wb(G_FAR2E38, (u16)(p + 3), nq); wb(G_FAR2E38, (u16)(p + 4), 1);
        g_wb(CB_NITEMS(i), (u8)(n + 1));
    } else { hook_call_original(); return; }                 /* (a full pack: as the original) */
    wb(G_FAR2E38, (u16)(o + 4), (u8)(cnt - 1));
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix armourwear: member %d, one of %d (type %d, q%d) worn to q%d alone\n", i, cnt, type, q, (s8)nq);
    hook_return_far(0);
}
static void wornout_hook(void)             /* s019 1ef8:0914 (i): throw away worn-out armour */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    if (i < 0 || i >= 5) { hook_call_original(); return; }
    u16 cb = (u16)(GV_combatants + i * 0x80);
    for (int s = 0; s < 2; s++) {
        if ((s8)g_rb((u16)(cb + 0x4f + s)) > 0) continue;
        int k = inv_find(i, g_rb((u16)(cb + 0x4b + s)), g_rb((u16)(cb + 0x4f + s)));
        if (k >= 0) {                                       /* one of it (consume_item, s014 18bf:0db4) */
            u16 o = G_ITEM(i, k);
            int cnt = rb(G_FAR2E38, (u16)(o + 4)), n = (s8)g_rb(CB_NITEMS(i));
            if (cnt > 1) wb(G_FAR2E38, (u16)(o + 4), (u8)(cnt - 1));
            else {
                for (int j = k; j < n - 1; j++) for (int b = 0; b < 6; b++) wb(G_FAR2E38, (u16)(G_ITEM(i, j) + b), rb(G_FAR2E38, (u16)(G_ITEM(i, j + 1) + b)));
                g_wb(CB_NITEMS(i), (u8)(n - 1));
            }
            if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix armourwear: member %d's worn-out armour thrown away (%d left of it)\n", i, cnt - 1);
        }
        g_wb((u16)(cb + 0x4b + s), 0xff); g_wb((u16)(cb + 0x4d + s), 0); g_wb((u16)(cb + 0x4f + s), 0x19);
    }
    hook_return_far(0);
}

/* ---------------------------------------------------------------- cardopts */
int fix_is_on(const char *id) { for (int i = 0; i < FIX_COUNT; i++) if (!strcmp(FIXES[i].id, id)) return FIXES[i].on && !config_get_int("original", 0); return 0; }
static int gcall(int sect, u16 seg, u16 off, int n, u16 a0, u16 a1) { u16 a[2] = { a0, a1 }; return (s16)game_call_far(sect, seg, off, n, a); }
static void w04_options(void);
void fixes_card_opts(void)                  /* before a card's option list is shown: its opt_on[] put right */
{
    if (!fix_is_on("cardopts")) return;
    char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
    if (!strcasecmp(nm, "$CityW04") && (s16)rw(SS, (u16)(SP + 4)) == 0) { w04_options(); return; }     /* (its option list, block 0) */
    int c = !strcasecmp(nm, "$Selec01") ? 1 : !strcasecmp(nm, "$Revol14") ? 2 : !strcasecmp(nm, "$CityW02") ? 3 : 0;
    if (!c) return;
    int rope = gcall(GF_count_items, 2, (u16)-2, 0x3b) > 0;
    u8 o[8]; for (int i = 0; i < 8; i++) o[i] = g_rb((u16)(GV_opt_on + i));
    if (c == 1) {                           /* 1 abandon the horses and crawl out (greyed after a failed sewer, event 0x10), 2 the bribe */
        int loc = (s16)g_rw(G_CUR_LOC);
        int poor = gcall(GF_money_cmp, 2, G_MONEY, GV_var_money1) == -1;
        if (o[1] == 2 && !gcall(GF_event_at, 2, 0x10, (u16)loc)) g_wb((u16)(GV_opt_on + 1), 1);
        if (o[2] == 1 && poor) g_wb((u16)(GV_opt_on + 2), 2);
    } else if (c == 2) {                    /* 4 the climb with a rope (also greyed after waiting a day: prev_screen 0xbc) */
        if (o[4]) g_wb((u16)(GV_opt_on + 4), (rope && g_rw(GV_prev_screen) != 0xbc) ? 1 : 2);
    } else if (c == 3) {                    /* 0 the climber carries a rope for everyone */
        if (o[0]) g_wb((u16)(GV_opt_on + 0), rope ? 1 : 2);
    }
    if (getenv("DL_FIX_LOG")) { int ch = 0; for (int i = 0; i < 8; i++) ch |= o[i] != g_rb((u16)(GV_opt_on + i)); if (ch) fprintf(stderr, "fix cardopts: %s options put right\n", nm); }
}
/* $CityW04 (the wall by an old sally port, s073 1a7e): its script knows seven options (0 rope, 1
   climb, 2 sewer grate, 3 alchemy, 4 saint, 5 bribe, 6 sneak back) but its text has ten lines, with
   three "abandon the horses" variants the script never handles; the game numbers the lines in
   order, so from the second line on each line runs the next one's option (and lines 7-9 run
   nothing). Put right: the horse variants hidden, each line given its own option (shown, chance,
   choice), and the choice handed back to the script as its own number. */
static const int W04_LINE[10] = { 0, -1, 1, -1, 2, -1, 3, 4, 5, 6 };   /* line -> the script's option */
static u16 w04_fn[7][2]; static int w04_active;
static void w04_menu_hook(void)             /* card_menu (win, win2): the line chosen -> the script's option */
{
    hook_call_original();
    if (!w04_active) return;
    w04_active = 0;
    for (int i = 0; i < 7; i++) { g_ww((u16)(GV_opt_fn + 4 * i), w04_fn[i][0]); g_ww((u16)(GV_opt_fn + 4 * i + 2), w04_fn[i][1]); }
    int line = (s16)AX;
    if (line >= 0 && line < 10 && W04_LINE[line] >= 0) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix cardopts: $CityW04 line %d is option %d\n", line, W04_LINE[line]);
        AX = (u16)W04_LINE[line];
    }
}
static void w04_options(void)
{
    static int hooked;
    if (!hooked) { hook_install(GF_card_menu, w04_menu_hook); hooked = 1; }      /* (installed last: it runs first, round the card view's) */
    u8 on[7]; for (int i = 0; i < 7; i++) on[i] = g_rb((u16)(GV_opt_on + i));
    for (int i = 0; i < 7; i++) { w04_fn[i][0] = g_rw((u16)(GV_opt_fn + 4 * i)); w04_fn[i][1] = g_rw((u16)(GV_opt_fn + 4 * i + 2)); }
    for (int l = 0; l < 10; l++) {
        int o = W04_LINE[l];
        g_wb((u16)(GV_opt_on + l), o < 0 ? 0 : on[o]);
        if (o >= 0) { g_ww((u16)(GV_opt_fn + 4 * l), w04_fn[o][0]); g_ww((u16)(GV_opt_fn + 4 * l + 2), w04_fn[o][1]); }
    }
    w04_active = 1;
}

static void cardopts_hook(void)             /* card_options (block) */
{
    fixes_card_opts();
    hook_call_original();
}

/* $Cathe00's "ask about the patron saint" (s056 19c4:0952) calls set_member_bit (s029 0e76:1302,
   (who, bit): saints_known[who] |= ...) with who -1, which other routines take for the leader but
   this one does not */
static void patron_hook(void)
{
    if ((s16)rw(SS, (u16)(SP + 4)) == -1) {
        ww(SS, (u16)(SP + 4), (u16)g_rb(G_LEADER));
        fprintf(stderr, "fix patronsaint: saint %u taught to the leader\n", rw(SS, (u16)(SP + 6)));
    }
    hook_call_original();
}

/* ---------------------------------------------------------------- debtnote
 * A merchant's errand for papers in a town (kind 8, tag 10; s039 1462:1712, type0 6, 9, 10, 11) is
 * done at night: the town's marketplace ($Marke01), the offices upstairs ($Insid00, s108 1925): the
 * leading merchant's, the Fugger's, the Medici's, the Hanse's (GV_insid_office 0, 2, 1, 3 as chosen). The
 * papers are found when the event's +0x2a % 4 is the office (s108 1925:0438). The giver ($McGuf12,
 * s128 18f7) names the holder by the same +0x2a % 4 (goods merchant, Medici, Fugger, Hanse), but for
 * debt notes (type0 10) by +0x2a % 2 (Medici, Fugger): the office named never held them. Here the
 * office named holds them for the moment of the search (and no other does); +0x2a is put back. */
static int papers_event(int loc, u16 *seg, u16 *off)
{
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i);
        if (e && g_ev(e, G_EV_KIND) == 8 && g_ev(e, G_EV_LOC) == loc && g_ev(e, G_EV_TAG) == 10) { *seg = (u16)(e >> 16); *off = (u16)e; return i; }
    }
    return -1;
}
int fixes_debt_note_office(int aux) { return (aux % 2 == 0) ? 1 : 2; }     /* the office the giver names (Medici 1, Fugger 2) */
static void debtnote_hook(void)            /* the lock is open: search the office chosen */
{
    u16 sg, o;
    int loc = (s16)g_rw(G_CUR_LOC), i = papers_event(loc, &sg, &o);
    if (i < 0 || (s16)rw(sg, o) != 10) { hook_call_original(); return; }
    int aux = (s16)rw(sg, (u16)(o + 0x2a)), named = fixes_debt_note_office(aux), office = g_rb(GV_insid_office);
    int want = aux - aux % 4 + named;                        /* (the same +0x2a but for the office) */
    ww(sg, (u16)(o + 0x2a), (u16)want);
    if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix debtnote: office %d searched, the notes in office %d (game: %d)\n", office, named, aux % 4);
    hook_call_original();
    if (g_event(i) == (((u32)sg << 16) | o) && (s16)rw(sg, (u16)(o + 0x2a)) == want) ww(sg, (u16)(o + 0x2a), (u16)aux);   /* (still the same errand: as it was) */
}

/* ---------------------------------------------------------------- alchrisk
 * A brew's end (s029 0e76:3fb2, event i: +0x1e the chance, +0x2a where it was made, +0x2c the
 * formula's risk +0x52, 0 low .. 2 high as the alchemy page says): on a failure, half the time
 * and only at a place (cur_loc != -1), it starts an accident event 0x51 whose value is
 * rand_mod(4 - risk) != 0: 1 the explosion ($Alche02: hurt, damages, the inn barred four months),
 * 0 the stink (a month). That is 3/4 for low risk and 1/2 for high, the wrong way round; here the
 * roll counts when it is 0 (1/4 low, 1/3 medium, 1/2 high). rand_mod is hooked only for the
 * brew's end: its second roll, made only after a failure, is that one. */
static int alch_rolls, alch_risk, alch_chance, alch_failed;
static void alchrisk_rand_hook(void)
{
    int n = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    alch_rolls++;
    if (alch_rolls == 1) { alch_failed = alch_chance <= (s16)AX; return; }      /* (the chance against rand_mod(100)) */
    if (alch_rolls == 2 && alch_failed && n == 4 - alch_risk) {
        if (getenv("DL_FIX_LOG")) fprintf(stderr, "fix alchrisk: risk %d, roll %d of %d: %s\n", alch_risk, (s16)AX, n, AX == 0 ? "explosion" : "stink");
        AX = AX == 0 ? 1 : 0;
    }
}
void fixes_alchemy_blast_odds(int risk, int *num, int *den)    /* of a failure's accident, the share that is an explosion */
{
    if (risk < 0) risk = 0;
    if (risk > 2) risk = 2;
    *den = 4 - risk;
    *num = fix_is_on("alchrisk") ? 1 : 3 - risk;
}
static void alchrisk_hook(void)            /* the brew's end (i) */
{
    int i = (s16)rw(SS, (u16)(SP + 4));
    u32 e = i >= 0 && i < 300 ? ev_ptr(i) : 0;
    if (!e) { hook_call_original(); return; }
    u16 s = (u16)(e >> 16), o = (u16)e;
    alch_risk = (s16)rw(s, (u16)(o + 0x2c));
    if (alch_risk < 0 || alch_risk > 2) { hook_call_original(); return; }
    alch_chance = (s16)rw(s, (u16)(o + 0x1e));
    alch_rolls = 0; alch_failed = 0;
    if (!hook_install(GF_rand_mod, alchrisk_rand_hook)) { hook_call_original(); return; }
    hook_call_original();
    hook_remove(GF_rand_mod, alchrisk_rand_hook);
}

static const struct { int sect; unsigned seg, off; void (*fn)(void); } HOOKS[] = {
    { 29, 0x0e76, 0x4c04, blessing_hook }, { 14, 0x18bf, 0x0778, hitdamage_hook }, { 31, 0x18e7, 0x01b4, additem_hook },
    { 14, 0x0e76, 0x08fe, castle_hook }, { 80, 0x1870, 0x000c, bounty_hook }, { 33, 0x1462, 0x1630, alchemy_hook },
    { 31, 0x18e7, 0x01b4, additem_hook }, { 36, 0x1462, 0x0082, innsave_hook }, { -2, 0, 0, 0 }, { 41, 0x1462, 0x0000, copyprot_hook },
    { 14, 0x18bf, 0x0778, hitdamage_hook }, { -2, 0, 0, 0 }, { 29, 0x0e76, 0x01c0, bestweapon_hook }, { 31, 0x18e7, 0x0406, removeitem_hook },
    { 35, 0x1462, 0x0a46, storeroom_hook }, { 44, 0x1838, 0x0000, testcard_hook },
    { 19, 0x1ef8, 0x0968, armourwear_hook }, { 36, 0x1462, 0x0082, teachers_lodging_hook }, { GF_card_options, cardopts_hook },
    { GF_set_member_bit, patron_hook }, { 108, 0x1925, 0x0438, debtnote_hook }, { 29, 0x0e76, 0x3fb2, alchrisk_hook } };
/* further hooks that belong to a fix */
static const struct { const char *fix; int sect; unsigned seg, off; void (*fn)(void); } MORE[] = {
    { "itemcount", 14, 0x0e76, 0x2324, falls_hook }, { "itemcount", 14, 0x18bf, 0x0fa4, useitem_hook },
    { "teachers", 29, 0x0e76, 0x324c, teachers_next_hook }, { "armourwear", 19, 0x1ef8, 0x0914, wornout_hook },
    { "blessing", 29, 0x0e76, 0x448a, blessing_end_hook } };

void fix_enable(int i, int on)
{
    FIXES[i].on = on;
    char key[64]; snprintf(key, sizeof key, "fix.%s", FIXES[i].id);
    config_set_int(key, on);
    int shared = 0;                         /* another fix with the same hook still on (hooks of different fixes on one function chain) */
    for (int k = 0; k < FIX_COUNT; k++)
        if (k != i && FIXES[k].on && HOOKS[k].fn == HOOKS[i].fn && HOOKS[k].sect == HOOKS[i].sect && HOOKS[k].seg == HOOKS[i].seg && HOOKS[k].off == HOOKS[i].off) shared = 1;
    for (unsigned k = 0; k < sizeof MORE / sizeof MORE[0]; k++)
        if (!strcmp(MORE[k].fix, FIXES[i].id)) { if (on) hook_install(MORE[k].sect, MORE[k].seg, MORE[k].off, MORE[k].fn); else hook_remove(MORE[k].sect, MORE[k].seg, MORE[k].off, MORE[k].fn); }
    if (!HOOKS[i].fn) return;                /* checked every frame (fixes_frame), no hook */
    if (on) { if (!hook_install(HOOKS[i].sect, HOOKS[i].seg, HOOKS[i].off, HOOKS[i].fn)) fprintf(stderr, "fix %s: no hook point\n", FIXES[i].id); }
    else if (!shared) hook_remove(HOOKS[i].sect, HOOKS[i].seg, HOOKS[i].off, HOOKS[i].fn);
}

/* debugging (DL_CHECK_COPY): far copies (_fmemcpy, root 098c4) that would reach the stack */
static void copy_check_hook(void)
{
    u16 doff = rw(SS, (u16)(SP + 4)), dseg = rw(SS, (u16)(SP + 6)), soff = rw(SS, (u16)(SP + 8)), sseg = rw(SS, (u16)(SP + 10)), n = rw(SS, (u16)(SP + 12));
    if (dseg == SS && doff == 0xa895) fprintf(stderr, "pile copy: %u bytes (%u items) from %04x:%04x\n", n, n / 6, sseg, soff);
    if (dseg == SS && n && (u32)doff + n > (u32)SP - 0x200) {
        fprintf(stderr, "COPY INTO THE STACK: %04x:%04x <- %04x:%04x, %u bytes (SP %04x, return %04x:%04x)\n", dseg, doff, sseg, soff, n, SP, rw(SS, (u16)(SP + 2)), rw(SS, SP));
        ftrace_dump();
    }
    hook_call_original();
}

void fixes_init(void)
{
    if (getenv("DL_CHECK_COPY")) hook_install(-1, 0x06a1, 0x2eb4, copy_check_hook);
    for (int i = 0; i < FIX_COUNT; i++) {
        char key[64]; snprintf(key, sizeof key, "fix.%s", FIXES[i].id);
        fix_enable(i, config_get_int(key, FIXES[i].on));
    }
}
