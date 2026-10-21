/* battleend.c - the end of a battle said in words (battle.end_words, on by default; F12 >
 * Settings > Helpers > Battles).
 *
 * The game closes a battle (s002 0e76, after the loot) with show_message("BattleResults -
 * victory", 150): a box with the words of its programmers' log, held for 150 timer ticks
 * (about eight seconds) over the still battlefield - long enough to take for the game hanging,
 * and players clicked to get on. Here the box says how the battle ended ("Victory! The battle
 * is won." ...) and stays about three seconds. Before it, when the last foe falls, the game holds
 * the field still for 60 ticks of the battle clock (a defeat 240: some three and thirteen seconds)
 * before the loot; here a second (three for a defeat). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

#define END_TICKS 55                     /* (18.2 a second) */

static int on(void) { return config_get_int("battle.end_words", 1) && !config_get_int("original", 0); }

static void message_hook(void)          /* show_message (text, ticks) */
{
    u16 t = rw(SS, (u16)(SP + 4));
    char s[48]; g_str(G_DGROUP, t, s, sizeof s);
    static const char *const W[][2] = {
        { "victory", "Victory! The battle is won." }, { "retreat", "The party has retreated." },
        { "defeat", "The party is beaten." }, { "surrender", "The party has surrendered." },
        { "all dead", "The whole party has fallen." }, { "leave", "The party has left the field." },
    };
    if (on() && t == GV_msg_buf && !strncmp(s, "BattleResults - ", 16)) {
        for (unsigned k = 0; k < sizeof W / sizeof W[0]; k++) {
            if (strcmp(s + 16, W[k][0])) continue;
            const char *w = W[k][1];
            for (int i = 0; ; i++) { g_wb((u16)(t + i), (u8)w[i]); if (!w[i]) break; }
            if ((s16)rw(SS, (u16)(SP + 6)) > END_TICKS) ww(SS, (u16)(SP + 6), END_TICKS);
            fprintf(stderr, "battle end: \"%s\"\n", w);
            break;
        }
    }
    hook_call_original();
}

/* the field held still once the battle is decided (combat_over, s014 0e76:08fe: the last foe down
   holds it 60 ticks of the battle clock, a defeat 240) before the loot or the defeat: a second, three */
#define HOLD_WON 18
#define HOLD_LOST 55
static void over_hook(void)
{
    hook_call_original();
    if (!on() || AX) return;
    int res = (s16)g_rw(GV_battle_result);
    u16 now = g_rw(GV_combat_clock), until = g_rw(GV_battle_end_at);
    int hold = res == 0 ? HOLD_WON : res == 3 || res == 5 ? HOLD_LOST : -1;
    if (hold > 0 && (s16)(until - now) > hold) {
        g_ww(GV_battle_end_at, (u16)(now + hold));
        fprintf(stderr, "battle end: held %d ticks, not %d\n", hold, (s16)(until - now));
    }
    /* the battle clock stands while the battle is paused (for orders: the next member chosen, a
       fall) - decided, it would wait there for a click before the victory: it runs on */
    static u32 last_kick;
    if (hold > 0 && g_rb(GV_ai_paused) == 1) {
        u32 plat_ticks_ms(void); void bios_kbd_inject(u16);
        if (plat_ticks_ms() - last_kick > 400) { last_kick = plat_ticks_ms(); bios_kbd_inject(0x3920); fprintf(stderr, "battle end: decided while paused, resumed\n"); }
    }
}

void battleend_init(void)
{
    hook_install(GF_show_message, message_hook);
    hook_install(GF_combat_over, over_hook);
}
