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
#include "../runtime/config.h"
#include "assist.h"
#include <stdio.h>
#include <stdlib.h>

u32 plat_ticks_ms(void);

CardChances card_chances;
int assist_chances = 2;            /* 0 off, 1 as the difficulty level allows, 2 always numbers */

static int cm_depth, cm_dirty;
static u32 last_eval;
static int last_leader = -1;

int ufight_pending, ufight_seed = -1;   /* test command ufight: a battle at the next card menu */
static void card_menu_hook(void)        /* s043 150b:1142 card_menu(win, win2) */
{
    if (ufight_pending) {               /* run_battle(battlefield, seed, 3 x (enemy type, quality, number)), s029 0e76:2278 */
        int bf = ufight_pending; ufight_pending = 0;
        void (*f)(void) = game_func(29, 0x0e76, 0x2278);
        u16 args[11] = { (u16)bf, (u16)ufight_seed, 3, 2, 5, 0, 1, 1, 0xffff, 0, 0 };
        if (f) game_call_live(f, 11, args);
    }
    cm_depth++; cm_dirty = 1;
    hook_call_original();
    cm_depth--;
    if (!cm_depth) card_chances.active = 0;
}

/* handlers that wait for the player (they show text): never asked again */
static u32 bad[64]; static int nbad;
static int is_bad(u32 a) { for (int k = 0; k < nbad; k++) if (bad[k] == a) return 1; return 0; }

static void evaluate(void)
{
    int n = (s8)g_rb(0xe96e);
    if (n < 0 || n > 20) n = 0;
    void *snap = eval_snapshot();
    CardChances c;
    memset(&c, 0, sizeof c);
    c.n = n;
    for (int i = 0; i < n; i++) {
        int opt = (s8)g_rb((u16)(0xe9b0 + i)), kind = (s16)g_rw((u16)(0xe9c4 + 2 * i));
        if (getenv("DL_ASSIST_LOG")) fprintf(stderr, "assist: line %d/%d y %d opt %d kind %d on %d\n", i, n,
                                             g_rb((u16)(0xe974 + i)), opt, kind, opt >= 0 && opt < 20 ? g_rb((u16)(0xe99c + opt)) : -1);
        if (opt < 0 || opt >= 20 || kind > 0) continue;
        if (g_rb((u16)(0xe99c + opt)) != 1) continue;               /* option disabled */
        u16 off = g_rw((u16)(0xea14 + 4 * opt)), seg = g_rw((u16)(0xea16 + 4 * opt));
        u32 key = 0;
        if (seg) { int ovl_section_at(u32); u32 lin = LIN(seg, off); key = ((u32)(ovl_section_at(lin - ((u32)LOADSEG << 4)) & 0xff) << 20) | lin; }
        if (!seg || is_bad(key)) continue;
        g_ww(0xee6c, (u16)(opt / 4)); g_ww(0xee6a, (u16)(opt % 4));  /* menu_member/slot, as the engine sets them */
        u32 r = game_call_ptr(seg, off, 0, 0, 0x5a);
        eval_restore(snap);
        if (game_call_timed_out) { if (nbad < 64) bad[nbad++] = key; continue; }
        g_ww(0xee6c, (u16)(opt / 4)); g_ww(0xee6a, (u16)(opt % 4));
        u32 r2 = game_call_ptr(seg, off, 0, 0, 0xa5);
        eval_restore(snap);
        if (r2 != r) r = 0xffffffffu;                                  /* depends on stack garbage: no chance */
        int pct = (s16)(r & 0xffff), who = (s16)(r >> 16);
        if (getenv("DL_ASSIST_LOG")) { int ovl_section_at(u32); fprintf(stderr, "assist: card %.12s line %d opt %d kind %d fn %04x:%04x (s%03d) -> %d%% member %d\n",
                                             (const char *)M + LIN(G_DGROUP, 0xe3d4), i, opt, kind, seg, off, ovl_section_at(LIN(seg, off)), pct, who); }
        if (pct >= 0) { c.line[i].show = 1; c.line[i].pct = pct > 100 ? 100 : pct; c.line[i].who = who; }
    }
    eval_free(snap);
    c.active = 1;
    c.seq = card_chances.seq + 1;
    card_chances = c;
}

static void card_mouse_hook(void)       /* s043 150b:1744 card_mouse(win, win2): every menu poll */
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
    hook_call_original();
}

void assist_set_chances(int mode)
{
    assist_chances = mode;
    config_set_int("assist.chances", mode);
    if (mode) { hook_install(43, 0x150b, 0x1142, card_menu_hook); hook_install(43, 0x150b, 0x1744, card_mouse_hook); }
    else { hook_remove(43, 0x150b, 0x1142); hook_remove(43, 0x150b, 0x1744); card_chances.active = 0; }
}

void assist_init(void)
{
    assist_set_chances(config_get_int("assist.chances", 2));
}
