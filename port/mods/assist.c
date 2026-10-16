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
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"
#include "cardview.h"
#include <stdio.h>
#include <stdlib.h>

u32 plat_ticks_ms(void);

CardChances card_chances;
int assist_chances = 2;            /* 0 off, 1 as the difficulty level allows, 2 always numbers */

static int cm_depth, cm_dirty;
static u32 last_eval;
static int last_leader = -1;

int ufight_pending, ufight_seed = -1;   /* test command ufight: a battle at the next card menu */
int uscreen_pending, uscreen_a, uscreen_b; /* test command ualchemy: the alchemy screen from the card menu up */
/* (from the native card menu's loop, mods/cardview.c: a test screen opened as a card would open it,
 * s029 0e76:2204 (who, hours), and the card's picture drawn again after it) */
int assist_test_screens(void)
{
    if (!uscreen_pending) return 0;
    uscreen_pending = 0;
    void (*f)(void) = game_func(29, 0x0e76, 0x2204);
    u16 args[2] = { (u16)uscreen_a, (u16)uscreen_b };
    if (f) game_call_live(f, 2, args);
    return 1;
}
static void card_menu_hook(void)        /* s043 150b:1142 card_menu(win, win2) */
{
    if (ufight_pending) {               /* run_battle(battlefield, seed, 3 x (enemy type, quality, number)), s029 0e76:2278 */
        int bf = ufight_pending; ufight_pending = 0;
        void (*f)(void) = game_func(GF_run_battle);
        u16 args[11] = { (u16)bf, (u16)ufight_seed, 3, 2, 5, 0, 1, 1, 0xffff, 0, 0 };
        if (f) game_call_live(f, 11, args);
    }
    cm_depth++; cm_dirty = 1;
    void chronicle_capture(void), chronicle_choice(int);
    chronicle_capture();                 /* the text on show (ui/journal.cpp) */
    if (cardview_active()) {             /* the card screen drawn by Darklands+ (mods/cardview.c) */
        int sel = cardview_menu();
        int cardview_unwinding(void);
        if (cardview_unwinding()) { cm_depth--; if (!cm_depth) card_chances.active = 0; return; }
        chronicle_choice(sel);
        cm_depth--;
        if (!cm_depth) card_chances.active = 0;
        AX = (u16)sel;
        hook_return_far(0);
        return;
    }
    hook_call_original();
    chronicle_choice((s16)AX);           /* and the option chosen */
    cm_depth--;
    if (!cm_depth) card_chances.active = 0;
}

/* handlers that wait for the player (they show text): never asked again */
static u32 bad[64]; static int nbad;
static int is_bad(u32 a) { for (int k = 0; k < nbad; k++) if (bad[k] == a) return 1; return 0; }

extern int dos_no_writes;
static void evaluate(void)
{
    int n = (s8)g_rb(0xe96e);
    if (n < 0 || n > 20) n = 0;
    void *snap = eval_snapshot();
    dos_no_writes = 1;                   /* the answers are thrown away: so is any change to the disk */
    CardChances c;
    memset(&c, 0, sizeof c);
    c.n = n;
    for (int i = 0; i < n; i++) {
        int opt = (s8)g_rb((u16)(0xe9b0 + i)), kind = (s16)g_rw((u16)(0xe9c4 + 2 * i));
        if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: line %d/%d y %d opt %d kind %d on %d\n", i, n,
                                             g_rb((u16)(0xe974 + i)), opt, kind, opt >= 0 && opt < 20 ? g_rb((u16)(GV_opt_on + opt)) : -1);
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
        if (getenv("DL_ASSIST_LOG")) { int ovl_section_at(u32); fprintf(stderr, "assist: card %.12s line %d opt %d kind %d fn %04x:%04x (s%03d) -> %d%% member %d\n",
                                             (const char *)M + LIN(G_DGROUP, 0xe3d4), i, opt, kind, seg, off, ovl_section_at(LIN(seg, off)), pct, who); }
        if (pct >= 0) { c.line[i].show = 1; c.line[i].pct = pct > 100 ? 100 : pct; c.line[i].who = who; }
    }
    eval_free(snap);
    dos_no_writes = 0;
    c.active = 1;
    c.seq = card_chances.seq + 1;
    card_chances = c;
}

/* every poll of a card menu: the chances asked again when something may have changed */
void assist_menu_poll(void)
{
    if (assist_chances && cm_depth && !g_rb(0xee42)) {
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
    if (assist_chances || assist_card_help || assist_card_keys || assist_watching) { hook_install(GF_card_menu, card_menu_hook); hook_install(GF_card_mouse, card_mouse_hook); }
    else { hook_remove(GF_card_menu, card_menu_hook); hook_remove(GF_card_mouse, card_mouse_hook); }
    if (!assist_chances) card_chances.active = 0;
}

void assist_watch_cards(void) { if (!assist_watching) { assist_watching = 1; card_hooks(); } }

void assist_set_chances(int mode)
{
    assist_chances = mode;
    config_set_int("assist.chances", mode);
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
             (unsigned)(plat_ticks_ms() - card_chances.polled), g_rb(0xee41), g_rb(0xee42), (s8)g_rb(0xe96e));
    (void)digit;
    return b;
}

int assist_card_key(int digit, int *gx, int *gy)
{
    /* D_ee42: the pointer is in the box of an option that picks a member, saint or item, and
       the menu is that box's (its lines are not numbered) */
    if (!assist_card_keys || !assist_in_card_menu() || g_rb(0xee42)) return 0;
    int n = (s8)g_rb(0xe96e), i = digit - 1;
    if (i < 0 || i >= n || n > 20) return 0;
    int x0 = 60 + (s16)g_rw(0xe972), y0 = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i));
    if (h <= 0) return 0;
    *gx = x0 + 10; *gy = y0 + (h > 8 ? 4 : h / 2);
    if (g_rb(0xee41)) {     /* such a box is shown (the pointer rests on its option): fine unless it covers the line */
        int bx = (s16)g_rw(0xee0d) + 0x3c, by = g_rb(0xee0f), bw = (s16)g_rw(0xee11), bh = g_rb(0xee10);
        if (*gx >= bx && *gx <= bx + bw && *gy >= by && *gy <= by + bh) return 0;
    }
    return 1;
}

void assist_init(void)
{
    assist_card_help = config_get_int("assist.card_help", 1);
    assist_card_keys = config_get_int("assist.card_keys", 1);
    assist_set_chances(config_get_int("assist.chances", 2));
}
