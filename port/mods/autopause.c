/* autopause.c - battles pause themselves (as if Space were pressed) when a member of the party
 * falls, or is badly hurt, so the player can give new orders in time. Off: the original's
 * battles, which run on until the player pauses them. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL_scancode.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "assist.h"

int autopause_down = 1;          /* a member falls (endurance or strength gone) */
int autopause_hurt = 0;          /* a member's endurance drops below a quarter */

void plat_press_key(int scancode);
void ui_notice(const char *msg);
u32 plat_ticks_ms(void);

static int state[5], in_battle;
static u32 last_pause;

static int member_state(int m)
{
    u16 b = G_MEMBER(m);
    int end = (s8)g_rb((u16)(b + G_M_ATTR_CUR)), endm = (s8)g_rb((u16)(b + G_M_ATTR_MAX)), str = (s8)g_rb((u16)(b + G_M_ATTR_CUR + 1));
    if (endm <= 0) return 0;
    if (end <= 0 || str <= 0) return 2;
    if (end * 4 <= endm) return 1;
    if (end * 10 > endm * 4) return 0;
    return -1;                                   /* between a quarter and 40%: as it was */
}

void autopause_frame(void)
{
    if (getenv("DL_BLOW_LOG") && game_mode() == GM_BATTLE) {      /* debugging: every change of endurance, strength, status */
        static int pe[13], ps[13], pt[13];
        for (int i = 0; i < 13; i++) {
            u16 b = (u16)(0x9c55 + 0x80 * i);
            int e = (s8)g_rb((u16)(b + 0x5d)), s = (s8)g_rb((u16)(b + 0x5e)), t = g_rb((u16)(b + 0x14));
            if (e != pe[i] || s != ps[i] || t != pt[i]) fprintf(stderr, "frame %u: %d end %d -> %d str %d -> %d status %d -> %d\n", plat_ticks_ms(), i, pe[i], e, ps[i], s, pt[i], t);
            pe[i] = e; ps[i] = s; pt[i] = t;
        }
    }
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 0 || size > 5) size = 0;
    if (game_mode() != GM_BATTLE) { in_battle = 0; return; }
    if (!in_battle) {                            /* the battle begins: who is hurt already does not count */
        in_battle = 1;
        for (int m = 0; m < 5; m++) { int s = m < size ? member_state(m) : 0; state[m] = s < 0 ? 1 : s; }
        return;
    }
    for (int m = 0; m < size; m++) {
        int s = member_state(m);
        if (s < 0) continue;
        int was = state[m]; state[m] = s;
        if (s <= was) continue;
        if (!(s == 2 ? autopause_down : autopause_hurt)) continue;
        if (g_rb(0x07d3)) continue;                                      /* paused already */
        if (plat_ticks_ms() - last_pause < 1500) continue;
        last_pause = plat_ticks_ms();
        plat_press_key(SDL_SCANCODE_SPACE);
        char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
        char msg[80]; snprintf(msg, sizeof msg, s == 2 ? "Paused: %s has fallen" : "Paused: %s is badly hurt", n);
        ui_notice(msg);
    }
}

void autopause_set(int down, int hurt)
{
    autopause_down = down; autopause_hurt = hurt;
    config_set_int("battle.pause_down", down); config_set_int("battle.pause_hurt", hurt);
}
void autopause_init(void)
{
    autopause_down = config_get_int("battle.pause_down", 1);
    autopause_hurt = config_get_int("battle.pause_hurt", 0);
}
