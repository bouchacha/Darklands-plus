/* assist.c - playing aids of the native port that watch the game through hooks:
 *
 *  - chances of success on card options. The card scripts already know them: every option
 *    has a handler (opt_fn[k], returning struct res {chance, member}) that the original
 *    only calls while the player holds Shift over the option. Here all of them are asked
 *    once per menu (and again every second, as the leader or equipment may change), in a
 *    side-effect-free evaluation (gamecall.c), and the interface draws the results next
 *    to the options (ui/overlay.cpp).
 *
 * Card engine variables (s043 150b, DGROUP): layout of the option text D_e96e {n, w e970,
 * x e972, y[20] e974, h[20] e988, opt_on[20] e99c, opt[20] e9b0, kind[20] e9c4, opt_fn[20]
 * ea14}; D_ee41 = a skill-check box is shown, D_ee42 = the mouse is in that box. */
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/hle.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"
#include "nscreen.h"
#include "cardview.h"
#include <stdio.h>
#include <stdlib.h>

u32 plat_ticks_ms(void);

CardChances card_chances;
int assist_chances = 2;            /* 0 off, 1 as the difficulty level allows, 2 always numbers */

static int cm_depth, cm_dirty;

/* ---- the best member speaks for the party (assist.best_leader, on by default). Many cards test
 * the leader alone (member_skill(-1, ...), member_attr(-1, ...): the charisma and Speak Common of
 * every talk, a perception, a lore ...), and shops price by the leader's charisma, so the
 * original had the player change the leader for each. Here each option's chance is asked of
 * every able member as leader (the evaluation below); the option chosen is played with the
 * member who does best as leader, and the player's own leader is back at the next card or on
 * the map. Prices are asked with the best charisma as leader (buy_price, sell_price). */
int assist_best_leader = 1;
static int saved_leader = -1;           /* the player's leader, while another speaks */
static int leader_able(int m)
{
    if (m < 0 || m >= 5 || m >= (s16)g_rw(G_PARTY_SIZE)) return 0;
    return g_rb((u16)(G_MEMBER(m) + G_M_STATUS)) == 1;
}
int assist_best_at_attr(int k)
{
    int cur = (s8)g_rb(G_LEADER), best = leader_able(cur) ? cur : -1, bv = best >= 0 ? (s8)g_rb((u16)(G_MEMBER(cur) + G_M_ATTR_CUR + k)) : -1000;
    for (int m = 0; m < 5; m++) {
        if (!leader_able(m)) continue;
        int v = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + k));
        if (v > bv) { bv = v; best = m; }
    }
    return best >= 0 ? best : cur;
}
int assist_bargainer(void) { return assist_best_leader && !config_get_int("original", 0) ? assist_best_at_attr(5) : (s8)g_rb(G_LEADER); }
void assist_leader_restore(void)
{
    if (saved_leader < 0) return;
    if (leader_able(saved_leader)) g_wb(G_LEADER, (u8)saved_leader);
    if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: leader %d again\n", saved_leader);
    saved_leader = -1;
}
static void leader_for_choice(int sel)
{
    if (!assist_best_leader || !card_chances.active || config_get_int("original", 0)) return;
    for (int i = 0; i < card_chances.n && i < 20; i++)
        if (card_chances.line[i].opt == sel && card_chances.line[i].lead >= 0) {
            int m = card_chances.line[i].lead;
            if (!leader_able(m)) return;
            if (saved_leader < 0) saved_leader = (s8)g_rb(G_LEADER);
            g_wb(G_LEADER, (u8)m);
            if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: option %d: %d speaks for the party (leader %d)\n", sel, m, saved_leader);
            return;
        }
}
static void price_hook(void)            /* buy_price (k) / sell_price (k, qual), s032: by the best charisma */
{
    if (!assist_best_leader || config_get_int("original", 0)) { hook_call_original(); return; }
    int cur = (s8)g_rb(G_LEADER), m = assist_best_at_attr(5);
    if (m == cur) { hook_call_original(); return; }
    g_wb(G_LEADER, (u8)m);
    hook_call_original();
    if (!unw_active) g_wb(G_LEADER, (u8)cur);
}
/* two cards roll the leader as they open, before any option is shown: $Raubi02 (s080 18a5:000c)
   Woodwise (skill 0x12) against d100 to see the ambush coming, $Revol02 (s073 1838:0000) Charisma
   - 20 and the town's mood to calm a crowd. There too the one best at it speaks for the party (the
   player's leader is back at the card's menu, as after a chosen option) */
static void entry_best(int skill, int attr)
{
    if (!assist_best_leader || config_get_int("original", 0)) return;
    int cur = (s8)g_rb(G_LEADER), best = cur, bv = -1000;
    for (int m = 0; m < 5; m++) {
        if (!leader_able(m)) continue;
        int v = skill >= 0 ? (s16)ns_call(GF_member_skill, 2, m, skill) : (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + attr));
        if (ns_unwinding) return;
        if (v > bv || (v == bv && m == cur)) { bv = v; best = m; }
    }
    if (best == cur || !leader_able(best)) return;
    if (saved_leader < 0) saved_leader = cur;
    g_wb(G_LEADER, (u8)best);
    if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: a card's opening roll: %d speaks for the party (leader %d)\n", best, saved_leader);
}
static void raubi02_hook(void) { entry_best(0x12, -1); if (ns_unwinding) return; hook_call_original(); }
static void revol02_hook(void) { entry_best(-1, 5); if (ns_unwinding) return; hook_call_original(); }
/* and two options roll the leader inside their action, with no chance shown beside them:
   $Villa07's prayer at a village shrine (s078 191c:020c: divine favour for all of Speak Latin / 40 +
   Religion / 60 + 1) and $Drago03's (s082 1845:0454: Virtue, then the leader is hurt) */
static void villa07_hook(void)
{
    if (assist_best_leader && !config_get_int("original", 0)) {
        int cur = (s8)g_rb(G_LEADER), best = cur, bv = -1;
        for (int m = 0; m < 5; m++) {
            if (!leader_able(m)) continue;
            int v = (s16)ns_call(GF_member_skill, 2, m, 0xb) / 40 + (s16)ns_call(GF_member_skill, 2, m, 8) / 60;
            if (ns_unwinding) return;
            if (v > bv || (v == bv && m == cur)) { bv = v; best = m; }
        }
        if (best != cur && leader_able(best)) { if (saved_leader < 0) saved_leader = cur; g_wb(G_LEADER, (u8)best); }
    }
    hook_call_original();
}
static void drago03_hook(void) { entry_best(9, -1); if (ns_unwinding) return; hook_call_original(); }

static u32 last_eval;
static int last_leader = -1;

int ufight_pending, ufight_seed = -1;
int ufight_groups[9] = { 3, 2, 5, 0, 1, 1, -1, 0, 0 };   /* (enemy type, quality, number) x 3; ufight:bf,seed,t,q,n,... */   /* test command ufight: a battle at the next card menu */
int uscreen_pending, uscreen_a, uscreen_b; /* test command ualchemy: the alchemy screen from the card menu up */
/* (from the native card menu's loop, mods/cardview.c: a test screen opened as a card would open it,
 * s029 0e76:2204 (who, hours), and the card's picture drawn again after it) */
int assist_test_screens(void)
{
    if (!uscreen_pending) return 0;
    int kind = uscreen_pending;
    uscreen_pending = 0;
    if (kind == 3) {                    /* the store room (its overlay vector, as the town's card calls it) */
        void native_go(u16 seg, u16 off);
        u16 sp = SP, ds = DS, es = ES;
        DS = ES = G_DGROUP;
        push16(0); push16(HLE_SEG); push16(0xfff0);
        native_go((u16)(LOADSEG + 0x9c0), 0x1ed7);
        if (!unw_active) { SP = sp; DS = ds; ES = es; }
        return 1;
    }
    void (*f)(void) = kind == 2 ? game_func(34, 0x1462, 0x0102) : game_func(29, 0x0e76, 0x2204);   /* 2: saint_pray (who, saint) */
    u16 args[2] = { (u16)uscreen_a, (u16)uscreen_b };
    if (f) game_call_live(f, 2, args);
    return 1;
}
static void card_menu_hook(void)        /* s043 150b:1142 card_menu(win, win2) */
{
    int encounters_card_active(void);
    if (ufight_pending && !encounters_card_active()) {   /* run_battle(battlefield, seed, 3 x (enemy type, quality, number)), s029 0e76:2278 (test; not inside a new encounter, whose script would lose its place) */
        int bf = ufight_pending; ufight_pending = 0;
        void (*f)(void) = game_func(GF_run_battle);
        u16 args[11] = { (u16)bf, (u16)ufight_seed };
        for (int k = 0; k < 9; k++) args[2 + k] = (u16)ufight_groups[k];
        if (f) game_call_live(f, 11, args);
    }
    assist_leader_restore();            /* (a new card: the player's leader again; asked anew below) */
    cm_depth++; cm_dirty = 1;
    void chronicle_capture(void), chronicle_choice(int);
    chronicle_capture();                 /* the text on show (ui/journal.cpp) */
    if (cardview_active()) {             /* the card screen drawn by Darklands+ (mods/cardview.c) */
        int sel = cardview_menu();
        int cardview_unwinding(void);
        if (cardview_unwinding()) { cm_depth--; if (!cm_depth) card_chances.active = 0; return; }
        chronicle_choice(sel);
        leader_for_choice(sel);
        cm_depth--;
        if (!cm_depth) card_chances.active = 0;
        AX = (u16)sel;
        hook_return_far(0);
        return;
    }
    hook_call_original();
    chronicle_choice((s16)AX);           /* and the option chosen */
    leader_for_choice((s16)AX);
    cm_depth--;
    if (!cm_depth) card_chances.active = 0;
}

/* handlers that wait for the player (they show text): never asked again */
static u32 bad[64]; static int nbad;
static int is_bad(u32 a) { for (int k = 0; k < nbad; k++) if (bad[k] == a) return 1; return 0; }

/* what an option's chance reads (CardChances reads): the party's numbers changed for every member,
   the option asked again; a group at a time, halved until the ones that matter are left - raised
   (skills 99, attributes 60, the town's opinion and fame high), or lowered when the chance is near
   certain and cannot rise. A handful of calls for an option that reads one or two things */
enum { F_ATTR = 19, F_REP = 26, F_FAME = 27 };
static int ask_with(u16 seg, u16 off, int opt, void *snap, const int *fs, int nf, int high)
{
    for (int q = 0; q < nf; q++) {
        int f = fs[q];
        if (f < F_ATTR) { for (int m = 0; m < 5; m++) g_wb((u16)(G_MEMBER(m) + G_M_SKILLS + f), (u8)(high ? 99 : 1)); }
        else if (f < F_REP) { for (int m = 0; m < 5; m++) { g_wb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + f - F_ATTR), (u8)(high ? 60 : 1)); g_wb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + f - F_ATTR), (u8)(high ? 60 : 1)); } }
        else if (f == F_REP) {
            s16 v = high ? 100 : -100; int loc = (s16)g_rw(G_CUR_LOC);
            g_ww(GV_cur_rep, (u16)v);
            if (loc >= 0) { u32 r = g_location(loc); if (r) ww((u16)(r >> 16), (u16)((u16)r + 0x12), (u16)v); }
        } else g_ww(GV_party_fame, (u16)(high ? 30000 : 0));
    }
    g_ww(GV_menu_member, (u16)(opt / 4)); g_ww(GV_menu_slot, (u16)(opt % 4));
    u32 r = game_call_ptr(seg, off, 0, 0, 0x5a);
    eval_restore(snap);
    return game_call_timed_out ? -1000 : (s16)(r & 0xffff);
}
static void find_reads(u16 seg, u16 off, int opt, void *snap, int p0, const int *fs, int nf, int high, u32 *reads, int *calls)
{
    if (nf <= 0 || *calls > 40) return;
    (*calls)++;
    int p = ask_with(seg, off, opt, snap, fs, nf, high);
    if (p == -1000 || p == p0) return;
    if (nf == 1) { *reads |= 1u << fs[0]; return; }
    find_reads(seg, off, opt, snap, p0, fs, nf / 2, high, reads, calls);
    find_reads(seg, off, opt, snap, p0, fs + nf / 2, nf - nf / 2, high, reads, calls);
}
static struct { u32 key; u32 reads; } reads_cache[24]; static int nreads_cache; static char reads_card[13];
static u32 option_reads(u16 seg, u16 off, int opt, void *snap, int p0)
{
    /* asked once for each option of a card and its leader: the chances are asked again every second,
       what they read does not change */
    char card[13]; memcpy(card, (const char *)M + LIN(G_DGROUP, 0xe3d4), 12); card[12] = 0;
    if (strcmp(card, reads_card)) { strcpy(reads_card, card); nreads_cache = 0; }
    u32 key = ((u32)seg << 16 | off) ^ ((u32)(u8)g_rb(G_LEADER) << 28) ^ (u32)opt << 24;
    for (int k = 0; k < nreads_cache; k++) if (reads_cache[k].key == key) return reads_cache[k].reads;
    static const int SK[19] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    static const int AT[7] = { 19, 20, 21, 22, 23, 24, 25 }, RP[1] = { F_REP }, FM[1] = { F_FAME };
    u32 reads = 0; int calls = 0, high = p0 < 95;
    find_reads(seg, off, opt, snap, p0, SK, 19, high, &reads, &calls);
    find_reads(seg, off, opt, snap, p0, AT, 7, high, &reads, &calls);
    find_reads(seg, off, opt, snap, p0, RP, 1, high, &reads, &calls);
    find_reads(seg, off, opt, snap, p0, FM, 1, high, &reads, &calls);
    if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: opt %d reads %#x (%d calls)\n", opt, reads, calls);
    if (nreads_cache < 24) { reads_cache[nreads_cache].key = key; reads_cache[nreads_cache].reads = reads; nreads_cache++; }
    return reads;
}

extern int dos_no_writes;
static void evaluate(void)
{
    int n = (s8)g_rb(GV_card_layout);
    if (n < 0 || n > 20) n = 0;
    u32 t0 = plat_ticks_ms();
    void *snap = eval_snapshot();
    dos_no_writes = 1;                   /* the answers are thrown away: so is any change to the disk */
    CardChances c;
    memset(&c, 0, sizeof c);
    c.n = n;
    for (int i = 0; i < 20; i++) { c.line[i].opt = -1; c.line[i].lead = -1; }
    for (int i = 0; i < n; i++) {
        int opt = (s8)g_rb((u16)(GV_card_layout_opt + i)), kind = (s16)g_rw((u16)(GV_card_layout_kind + 2 * i));
        if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: line %d/%d y %d opt %d kind %d on %d\n", i, n,
                                             g_rb((u16)(GV_card_layout_y + i)), opt, kind, opt >= 0 && opt < 20 ? g_rb((u16)(GV_opt_on + opt)) : -1);
        if (opt < 0 || opt >= 20 || kind > 0) continue;
        if (g_rb((u16)(GV_opt_on + opt)) != 1) continue;               /* option disabled */
        u16 off = g_rw((u16)(GV_opt_fn + 4 * opt)), seg = g_rw((u16)(0xea16 + 4 * opt));
        u32 key = 0;
        if (seg) { int ovl_section_at(u32); u32 lin = LIN(seg, off); key = ((u32)(ovl_section_at(lin - ((u32)LOADSEG << 4)) & 0xff) << 20) | lin; }
        if (!seg || is_bad(key)) continue;
        g_ww(GV_menu_member, (u16)(opt / 4)); g_ww(GV_menu_slot, (u16)(opt % 4));  /* menu_member/slot, as the engine sets them */
        u32 r = game_call_ptr(seg, off, 0, 0, 0x5a);
        eval_restore(snap);
        if (game_call_timed_out) { if (nbad < 64) bad[nbad++] = key; continue; }
        g_ww(GV_menu_member, (u16)(opt / 4)); g_ww(GV_menu_slot, (u16)(opt % 4));
        u32 r2 = game_call_ptr(seg, off, 0, 0, 0xa5);
        eval_restore(snap);
        if (game_call_timed_out) { if (nbad < 64) bad[nbad++] = key; continue; }   /* waits for the player: never again (each try costs 100 ms) */
        if (r2 != r) r = 0xffffffffu;                                  /* depends on stack garbage: no chance */
        int pct = (s16)(r & 0xffff), who = (s16)(r >> 16);
        if (pct >= 0 && r != 0xffffffffu) {
            /* asked again with the whole party at 1 in every attribute and skill: a chance that
               does not move is no skill check (the handlers of plain options answer too: leaving
               town "100 %", a roll of the dice "0 %" or "100 %" from one second to the next) */
            g_ww(GV_menu_member, (u16)(opt / 4)); g_ww(GV_menu_slot, (u16)(opt % 4));
            for (int m = 0; m < 5; m++) {
                u16 b = G_MEMBER(m);
                for (int a = 0; a < 7; a++) { g_wb((u16)(b + G_M_ATTR_CUR + a), 1); g_wb((u16)(b + G_M_ATTR_MAX + a), 1); }
                for (int k = 0; k < 19; k++) g_wb((u16)(b + G_M_SKILLS + k), 1);
            }
            u32 r3 = game_call_ptr(seg, off, 0, 0, 0x5a);
            eval_restore(snap);
            if (game_call_timed_out) { if (nbad < 64) bad[nbad++] = key; continue; }
            if ((s16)(r3 & 0xffff) == pct) {
                if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: line %d opt %d: %d%% whatever the party: no skill check\n", i, opt, pct);
                continue;
            }
        }
        int lead = -1;
        if (assist_best_leader && pct >= 0 && r != 0xffffffffu && !config_get_int("original", 0)) {   /* each able member as leader */
            int cur = (s8)g_rb(G_LEADER), bestp = pct, dep = 0;
            for (int m = 0; m < 5; m++) {
                if (m == cur || !leader_able(m)) continue;
                g_wb(G_LEADER, (u8)m);
                g_ww(GV_menu_member, (u16)(opt / 4)); g_ww(GV_menu_slot, (u16)(opt % 4));
                u32 rm = game_call_ptr(seg, off, 0, 0, 0x5a);
                eval_restore(snap);
                if (game_call_timed_out) break;
                int pm = (s16)(rm & 0xffff);
                if (pm != pct) dep = 1;                                 /* (the leader's own check) */
                if (pm > bestp) { bestp = pm; lead = m; }
            }
            if (dep) who = lead >= 0 ? lead : cur;                     /* the chance is that member's: their name beside it, not "all" */
            if (lead >= 0) { if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: line %d opt %d: %d%% with %d as leader (%d%% with %d)\n", i, opt, bestp, lead, pct, cur); pct = bestp; }
        }
        if (getenv("DL_ASSIST_LOG")) { int ovl_section_at(u32); fprintf(stderr, "assist: card %.12s line %d opt %d kind %d fn %04x:%04x (s%03d) -> %d%% member %d\n",
                                             (const char *)M + LIN(G_DGROUP, 0xe3d4), i, opt, kind, seg, off, ovl_section_at(LIN(seg, off)), pct, who); }
        c.line[i].opt = opt; c.line[i].lead = lead;
        if (pct >= 0) { c.line[i].show = 1; c.line[i].pct = pct > 100 ? 100 : pct; c.line[i].who = who; }
        if (pct >= 0 && r != 0xffffffffu) {                            /* what it reads, with the leader who would do it */
            int cur = (s8)g_rb(G_LEADER);
            if (lead >= 0) g_wb(G_LEADER, (u8)lead);
            void *s2 = lead >= 0 ? eval_snapshot() : snap;
            c.line[i].reads = option_reads(seg, off, opt, s2, pct);
            if (lead >= 0) { eval_free(s2); g_wb(G_LEADER, (u8)cur); }
        }
    }
    eval_free(snap);
    dos_no_writes = 0;
    c.active = 1;
    c.seq = card_chances.seq + 1;
    card_chances = c;
    { u32 dt = plat_ticks_ms() - t0; if (getenv("DL_ASSIST_LOG") || dt > 60) fprintf(stderr, "assist: the card's chances took %u ms (%d lines)\n", dt, n); }
}

/* every poll of a card menu: the chances asked again when something may have changed */
void assist_menu_poll(void)
{
    if ((assist_chances || assist_best_leader) && cm_depth && !g_rb(GV_card_box_pointer)) {
        u32 now = plat_ticks_ms();
        int lead = (s8)g_rb(G_LEADER);
        if (cm_dirty || lead != last_leader || now - last_eval > 1000) {
            cm_dirty = 0; last_leader = lead; last_eval = now;
            evaluate();
        }
    }
    card_chances.polled = plat_ticks_ms();
}
static void card_mouse_hook(void)       /* s043 150b:1744 card_mouse(win, win2): every menu poll */
{
    assist_menu_poll();
    hook_call_original();
}

int assist_card_help = 1;          /* show the saints and items that work on a card */
int assist_card_keys = 1;          /* number keys pick a card's options */
static int assist_watching;        /* ui/citynav.cpp walks through card menus */
static void card_hooks(void)
{
    if (assist_chances || assist_card_help || assist_card_keys || assist_watching || assist_best_leader) { hook_install(GF_card_menu, card_menu_hook); hook_install(GF_card_mouse, card_mouse_hook); }
    else { hook_remove(GF_card_menu, card_menu_hook); hook_remove(GF_card_mouse, card_mouse_hook); }
    if (assist_best_leader) { hook_install(GF_buy_price, price_hook); hook_install(GF_sell_price, price_hook);
                              hook_install(80, 0x18a5, 0x000c, raubi02_hook); hook_install(73, 0x1838, 0x0000, revol02_hook);
                              hook_install(78, 0x191c, 0x020c, villa07_hook); hook_install(82, 0x1845, 0x0454, drago03_hook); }
    else { hook_remove(GF_buy_price, price_hook); hook_remove(GF_sell_price, price_hook); assist_leader_restore();
           hook_remove(80, 0x18a5, 0x000c, raubi02_hook); hook_remove(73, 0x1838, 0x0000, revol02_hook);
           hook_remove(78, 0x191c, 0x020c, villa07_hook); hook_remove(82, 0x1845, 0x0454, drago03_hook); }
    if (!assist_chances && !assist_best_leader) card_chances.active = 0;
}

void assist_watch_cards(void) { if (!assist_watching) { assist_watching = 1; card_hooks(); } }

void assist_set_chances(int mode)
{
    assist_chances = mode;
    config_set_int("assist.chances", mode);
    card_hooks();
}

void assist_set_best_leader(int on)
{
    assist_best_leader = on;
    config_set_int("assist.best_leader", on);
    card_hooks();
}

void assist_set_card_help(int on)
{
    assist_card_help = on;
    config_set_int("assist.card_help", on);
    card_hooks();
}

/* the card's option menu is up (polled in the last 300 ms) */
int assist_in_card_menu(void) { return cm_depth > 0 && plat_ticks_ms() - card_chances.polled < 300; }

void assist_set_card_keys(int on)
{
    assist_card_keys = on;
    config_set_int("assist.card_keys", on);
    card_hooks();
}

/* number key 1..9 on a card: where to click for that option (game pixels); 0 if none */
/* (tests) why a number key does nothing now */
const char *assist_card_key_why(int digit)
{
    static char b[120];
    snprintf(b, sizeof b, "keys %d depth %d polled %u ms ago ee41 %d ee42 %d options %d", assist_card_keys, cm_depth,
             (unsigned)(plat_ticks_ms() - card_chances.polled), g_rb(GV_card_box_shown), g_rb(GV_card_box_pointer), (s8)g_rb(GV_card_layout));
    (void)digit;
    return b;
}

int assist_card_key(int digit, int *gx, int *gy)
{
    /* D_ee42: the pointer is in the box of an option that picks a member, saint or item, and
       the menu is that box's (its lines are not numbered) */
    if (!assist_card_keys || !assist_in_card_menu() || g_rb(GV_card_box_pointer)) return 0;
    int n = (s8)g_rb(GV_card_layout), i = digit - 1;
    if (i < 0 || i >= n || n > 20) return 0;
    int x0 = 60 + (s16)g_rw(GV_card_layout_x), y0 = g_rb((u16)(GV_card_layout_y + i)), h = g_rb((u16)(GV_card_layout_h + i));
    if (h <= 0) return 0;
    *gx = x0 + 10; *gy = y0 + (h > 8 ? 4 : h / 2);
    if (g_rb(GV_card_box_shown)) {     /* such a box is shown (the pointer rests on its option): fine unless it covers the line */
        int bx = (s16)g_rw(0xee0d) + 0x3c, by = g_rb(0xee0f), bw = (s16)g_rw(0xee11), bh = g_rb(0xee10);
        if (*gx >= bx && *gx <= bx + bw && *gy >= by && *gy <= by + bh) return 0;
    }
    return 1;
}

void assist_init(void)
{
    assist_card_help = config_get_int("assist.card_help", 1);
    assist_card_keys = config_get_int("assist.card_keys", 1);
    assist_best_leader = config_get_int("assist.best_leader", 1);
    assist_set_chances(config_get_int("assist.chances", 2));
}
