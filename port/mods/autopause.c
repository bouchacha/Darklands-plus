/* autopause.c - battles pause themselves (as if Space were pressed) when a member of the party
 * falls, or is badly hurt, or when foes waiting in a castle, dungeon or mine come out to fight (a
 * group of the shown floor's activations steps out: their count, DS:d6cc, falls while the floor
 * stays the same; see mods/foesleft.c), so the player can give new orders in time. Off: the
 * original's battles, which run on until the player pauses them. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL_scancode.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"

int autopause_down = 1;          /* a member falls (endurance or strength gone) */
int autopause_hurt = 0;          /* a member's endurance drops below a quarter */
int autopause_foes = 1;          /* foes come out to fight (castles, dungeons, mines) */

void plat_press_key(int scancode);
void ui_notice(const char *msg);
u32 plat_ticks_ms(void);

static int state[5], in_battle, last_floor, last_groups;
static u32 foes_seen, floor_changed;      /* a group stepped out (0 none); the floor shown last changed */
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
    int floor = (s8)g_rb(GV_floor_shown), groups = g_rb(GV_activation_count);
    if (!in_battle) {                            /* the battle begins: who is hurt already does not count */
        in_battle = 1;
        for (int m = 0; m < 5; m++) { int s = m < size ? member_state(m) : 0; state[m] = s < 0 ? 1 : s; }
        last_floor = floor; last_groups = groups; foes_seen = 0; floor_changed = 0;   /* (the place is laid out in the battle's first frames: its groups appear, a fall soon after is real) */
        return;
    }
    if ((floor != last_floor || groups != last_groups) && getenv("DL_FOES_LOG"))
        fprintf(stderr, "autopause: floor %d -> %d, groups %d -> %d (pause depth %d, ai paused %d)\n", last_floor, floor, last_groups, groups, g_rb(GV_pause_depth), g_rb(0xa695));
    /* a group steps out: the count falls while the floor stays. Going up or down, the floor shown and
       the count of its groups change a frame or more apart, so a fall counts only well away from a
       change of floor (and the pause comes half a second after it, the floor still the same) */
    u32 now = plat_ticks_ms();
    if (floor != last_floor) { floor_changed = now; foes_seen = 0; }
    else if (groups < last_groups && now - floor_changed > 1500) foes_seen = now ? now : 1;
    if (foes_seen && now - foes_seen >= 500) {
        foes_seen = 0;
        if (autopause_foes && !config_get_int("original", 0) && !g_rb(GV_pause_depth) && now - last_pause >= 1500 && now - floor_changed > 2000) {
            last_pause = now;
            plat_press_key(SDL_SCANCODE_SPACE);
            ui_notice("Paused: foes come out to fight");
        }
    }
    last_floor = floor; last_groups = groups;
    for (int m = 0; m < size; m++) {
        int s = member_state(m);
        if (s < 0) continue;
        int was = state[m]; state[m] = s;
        if (s <= was) continue;
        if (!(s == 2 ? autopause_down : autopause_hurt)) continue;
        if (g_rb(GV_pause_depth)) continue;                                      /* paused already */
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
void autopause_set_foes(int on) { autopause_foes = on; config_set_int("battle.pause_foes", on); }
void autopause_init(void)
{
    autopause_foes = config_get_int("battle.pause_foes", 1);
    autopause_down = config_get_int("battle.pause_down", 1);
    autopause_hurt = config_get_int("battle.pause_hurt", 0);
}
