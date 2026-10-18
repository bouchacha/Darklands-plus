/* foesleft.c - the foes a place still holds that the party has not met yet (battle.foes_left,
 * on by default): shown under the battle margin's enemies, so a castle, dungeon or mine whose
 * fight "does not end" says where the rest still wait (the game ends it only when they are all
 * beaten, or the party walks out and counts as fled).
 *
 * The foes of a place wait in groups at activation spots (struct atv, 0x1e bytes: +0 x, +2 y,
 * +0b num[3] of each of three kinds; s024). The floor shown keeps its groups in `activations`
 * (seg at DS:7d3c, +0x47ee; their count DS:d6cc); a group seen by a member (activations_check,
 * s014 0e76:4442) steps out to fight and leaves the list. Every other floor's list is in the
 * game's swap store under "level<N>.atv" (a count byte, then the records): written for all
 * floors when the place is made (s004) or restored from a save (s003), and for a floor when
 * the party leaves it. Both copies of swap_save (s027 1f48:0a78 and s004 1262:0a7c) are
 * watched here and each floor's last list kept; a new place (battlefield_build, or
 * battle_restore from a saved game) clears them. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "foesleft.h"

#define ACTIVATIONS_SEG 0x7d3c                /* word: the segment of the floor's activation records */
#define ACTIVATIONS_OFF 0x47ee

static signed char known[FL_FLOORS];          /* this floor's list was seen */
static int foes[FL_FLOORS], groups[FL_FLOORS];

static int count_foes(u16 seg, u16 off, int n, int *ng)
{
    int sum = 0;
    if (n < 0 || n > 30) n = 0;
    for (int k = 0; k < n; k++)
        for (int j = 0; j < 3; j++) { int v = (s8)rb(seg, (u16)(off + k * 0x1e + 0x0b + j)); if (v > 0) sum += v; }
    if (ng) *ng = n;
    return sum;
}

static void saved(u16 seg, u16 off, u16 name)  /* a swap save: "level<N>.atv" is a floor's list */
{
    char s[24]; g_str(G_DGROUP, name, s, sizeof s);
    int f; char rest[8];
    if (sscanf(s, "level%d%7s", &f, rest) != 2 || strcmp(rest, ".atv") || f < 0 || f >= FL_FLOORS) return;
    int n = rb(seg, off);
    foes[f] = count_foes(seg, (u16)(off + 1), n, &groups[f]);
    known[f] = 1;
    if (getenv("DL_FOES_LOG")) fprintf(stderr, "foesleft: floor %d: %d groups, %d foes\n", f, groups[f], foes[f]);
}

static void save_hook(void)                   /* swap_save (long n, void far *buf, char *name) */
{
    saved(rw(SS, (u16)(SP + 10)), rw(SS, (u16)(SP + 8)), rw(SS, (u16)(SP + 12)));
    hook_call_original();
}

static void place_hook(void)                  /* battlefield_build / battle_restore: another place */
{
    memset(known, 0, sizeof known);
    if (getenv("DL_FOES_LOG")) fprintf(stderr, "foesleft: a new place\n");
    hook_call_original();
}

int foesleft_on(void) { return config_get_int("battle.foes_left", 1) && !config_get_int("original", 0); }

int foesleft_get(FoesLeft *out)
{
    memset(out, 0, sizeof *out);
    if (game_mode() != GM_BATTLE) return 0;
    int cur = (s8)g_rb(GV_floor_shown);
    if (cur < 0 || cur >= FL_FLOORS) return 0;
    out->floor = cur;
    for (int f = 0; f < FL_FLOORS; f++) {
        if (f == cur) {
            int n = g_rb(GV_activation_count);
            out->foes[f] = count_foes(g_rw(ACTIVATIONS_SEG), ACTIVATIONS_OFF, n, &out->groups[f]);
            out->known[f] = 1;
        } else if (known[f]) {
            out->foes[f] = foes[f]; out->groups[f] = groups[f]; out->known[f] = 1;
        }
        if (out->known[f]) out->floors = f + 1;
    }
    return 1;
}

void foesleft_init(void)
{
    int ok = hook_install(27, 0x1f48, 0x0a78, save_hook);
    ok = ok * 2 + hook_install(4, 0x1262, 0x0a7c, save_hook);
    ok = ok * 2 + hook_install(GF_battlefield_build, place_hook);
    ok = ok * 2 + hook_install(3, 0x0e76, 0x0ce8, place_hook);     /* battle_restore (a battle saved in a place, loaded) */
    if (getenv("DL_FOES_LOG")) fprintf(stderr, "foesleft: hooks %x\n", ok);
}
