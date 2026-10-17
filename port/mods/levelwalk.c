/* levelwalk.c - a click on the level map (Tab, ui/levelmap.cpp) walks the party there.
 * assist.level_map_walk, on by default.
 *
 * The party is put to Travel Single File (Q, as the key does: s016 1ef8:0cdc) and its leader is
 * given the walk a click on the floor gives in that mode (combat_mouse, s014 19cd:0006): the
 * leader's destination, path_find and path_give, the action "walk", the others following.
 * As a group (G) the four places of the formation must all be free and reachable round the
 * spot, which by a wall or in a passage they often are not, and the members stood still or
 * blocked each other half way. The order is given directly rather than as a click on the
 * battle view: with the battle paused the game reads a click's place only when it goes on,
 * from wherever the pointer is by then. Done at the start of the next combat_input, in the
 * game's own time.
 *
 * Combatants: DS:9c55 + 0x80 * i (+4 x, +6 y, +8 / +0a destination, +10 action (1 walk),
 * +14 status (1 up), +4a target); cbt_moving DS:a6d3[13]; D_d6fb[13] (the walk's step count). */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"

#define CBT(i) ((u16)(0x9c55 + (i) * 0x80))

int levelwalk_x = -1, levelwalk_y = -1, levelwalk_floor = -1;     /* the place, map pixels (32 x 16 a cell), and its floor */
void ui_notice(const char *msg);

static void input_hook(void)                 /* s014 0e76:3166 combat_input () */
{
    if (levelwalk_x < 0) { hook_call_original(); return; }
    int tx = levelwalk_x, ty = levelwalk_y, tfl = levelwalk_floor;
    levelwalk_x = levelwalk_y = levelwalk_floor = -1;
    if ((s8)g_rb(GV_floor_shown) != tfl) { hook_call_original(); return; }
    int was_mode = g_rb(GV_move_mode);
    if (was_mode != 2) { ns_call(16, 0x1ef8, 0x0cdc, 0); if (ns_unwinding) return; }
    int lead = (s8)g_rb(GV_marching_order);
    if (lead < 0 || lead > 4 || g_rb((u16)(CBT(lead) + 0x14)) != 1) { hook_call_original(); return; }
    /* the place itself, else the nearest the leader can reach (a cell under a wall, or one
       a door still shut keeps apart) */
    int otx = (s16)g_rw((u16)(CBT(lead) + 8)), oty = (s16)g_rw((u16)(CBT(lead) + 0xa));
    int mx = -1, my = -1;
    for (int r = 0; r <= 2 && mx < 0; r++)
        for (int dy = -r; dy <= r && mx < 0; dy++)
            for (int dx = -r; dx <= r && mx < 0; dx++) {
                if (r && dx > -r && dx < r && dy > -r && dy < r) continue;      /* (the ring at distance r) */
                int px = tx + dx * 32, py = ty + dy * 16;
                if (px < 0 || py < 0 || px >= 40 * 32 || py >= 40 * 16) continue;
                g_ww((u16)(CBT(lead) + 8), (u16)px); g_ww((u16)(CBT(lead) + 0xa), (u16)py);
                if (ns_call(GF_path_find, 1, lead) == 1) { mx = px; my = py; }
                if (ns_unwinding) return;
            }
    if (mx < 0) {
        g_ww((u16)(CBT(lead) + 8), (u16)otx); g_ww((u16)(CBT(lead) + 0xa), (u16)oty);
        ui_notice("The party cannot get there from where it stands.");
        hook_call_original(); return;
    }
    for (int i = 0; i < 5; i++) if (g_rb((u16)(CBT(i) + 0x14)) == 1) g_wb((u16)(GV_cbt_moving + i), 1);
    ns_call(GF_path_give, 1, lead);                             /* (the path found last: to mx, my) */
    if (ns_unwinding) return;
    g_ww((u16)(CBT(lead) + 0x10), 1);
    g_wb((u16)(CBT(lead) + 0x4a), 0xff);
    g_wb((u16)(0xd6fb + lead), 0);
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: map %d,%d -> %d,%d for %d (move mode %d, was %d)\n", tx, ty, mx, my, lead, g_rb(GV_move_mode), was_mode);
    if (g_rb(GV_ai_paused) == 1) ui_notice(was_mode != 2 ? "The party will walk there in single file (G: as a group again); Space resumes the battle."
                                                          : "The party will walk there: Space resumes the battle.");
    else if (was_mode != 2) ui_notice("The party walks there in single file (G: as a group again).");
    hook_call_original();
}

void levelwalk_init(void)
{
    hook_install(14, 0x0e76, 0x3166, input_hook);
}
