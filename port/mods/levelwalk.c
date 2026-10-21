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
#include "../runtime/hle.h"

#define CBT(i) ((u16)(0x9c55 + (i) * 0x80))

int levelwalk_x = -1, levelwalk_y = -1, levelwalk_floor = -1;     /* the place, map pixels (32 x 16 a cell), and its floor */
void ui_notice(const char *msg);

static int stairs_pending = -1, stairs_floor; static u32 stairs_t;   /* U: the stairs walked to, taken on arrival */
static int stairs_under(int i);
static void stairs_arrive(void)
{
    int lead = (s8)g_rb(GV_marching_order);
    if (lead < 0 || lead > 4 || (s8)g_rb(GV_floor_shown) != stairs_floor || plat_ticks_ms() - stairs_t > 30000 || g_rb(GV_real_combat)) { stairs_pending = -1; return; }
    if (stairs_under(lead) != stairs_pending) return;
    int n = stairs_pending; stairs_pending = -1;
    ns_call(GF_take_exit, 2, n, lead);                     /* (the others follow: take_exit_hook) */
    if (!ns_unwinding) ns_call(GF_cbt_cancel_action, 1, lead);
}
static void input_hook(void)                 /* s014 0e76:3166 combat_input () */
{
    if (stairs_pending >= 0 && levelwalk_x < 0) { stairs_arrive(); if (ns_unwinding) return; }
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
    if (g_rb(GV_ai_paused) == 1) ui_notice(was_mode != 2 ? "The party will walk there in single file (press G to move as a group again); Space resumes the battle."
                                                          : "The party will walk there: Space resumes the battle.");
    else if (was_mode != 2) ui_notice("The party walks there in single file (press G to move as a group again).");
    hook_call_original();
}

/* ---------------------------------------------------------------- stairs
 * In the game a member takes the stairs by standing on them: U (or walking onto them) then takes
 * the leader, or in a group or single file everyone (combat_keys, s016 1ef8:0000; take_exit
 * s023 1ef8:1618 (n, who)). Two things left parties stuck in a castle or dungeon:
 * - U with nobody on the stairs did nothing (a party placed beside them at a castle's start):
 *   now, outside a fight, the party walks to the nearest stairs of the floor and takes them;
 * - the leader walking onto the stairs went alone, the others stayed below: outside a fight
 *   (real_combat 0) the members still on the floor the leader left follow at once. */
#define FLR_NSTAIRS 0x2fb6
#define FLR_STAIRS  0x2fb8                 /* rect14[6]: x, y (map pixels), w, h (bytes) */
static int following;
static void take_exit_hook(void)           /* take_exit (n, who) */
{
    int n = (s16)rw(SS, (u16)(SP + 4)), who = (s16)rw(SS, (u16)(SP + 6));
    int from = (s8)g_rb((u16)(GV_cbt_floor + (who >= 0 && who < 13 ? who : 0)));
    int explore = !g_rb(GV_real_combat) && who >= 0 && who < 5 && !following && config_get_int("assist.stairs_together", 1);
    hook_call_original();
    if (ns_unwinding || !explore) return;
    following = 1;
    for (int i = 0; i < 5 && !ns_unwinding; i++) {
        if (i == who || g_rb((u16)(CBT(i) + 0x14)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + i)) != from) continue;
        ns_call(GF_take_exit, 2, n, i);
        if (!ns_unwinding) ns_call(GF_cbt_cancel_action, 1, i);
    }
    following = 0;
}
static int stairs_under(int i)
{
    int ns = (s8)rb(G_FAR20A5, FLR_NSTAIRS), x = (s16)g_rw((u16)(CBT(i) + 4)), y = (s16)g_rw((u16)(CBT(i) + 6));
    for (int k = 0; k < ns && k < 6; k++) {
        u16 o = (u16)(FLR_STAIRS + 14 * k);
        int rx = (s16)rw(G_FAR20A5, o), ry = (s16)rw(G_FAR20A5, (u16)(o + 2)), w = rb(G_FAR20A5, (u16)(o + 4)), h = rb(G_FAR20A5, (u16)(o + 5));
        if (x >= rx && x < rx + w && y >= ry && y < ry + h) return k;
    }
    return -1;
}
static int on_stairs(int i) { return stairs_under(i) >= 0; }
static void keys_hook(void)                /* combat_keys (): U with nobody on the stairs */
{
    extern int bios_kbd_avail(void); extern u16 bios_kbd_get(int);
    if (!bios_kbd_avail() || g_rb(GV_real_combat) || !config_get_int("assist.stairs_together", 1)) { hook_call_original(); return; }
    u16 k = bios_kbd_get(0);
    if (k != 0x1655 && k != 0x1675) { hook_call_original(); return; }
    int fl = (s8)g_rb(GV_floor_shown), lead = (s8)g_rb(GV_marching_order);
    for (int i = 0; i < 5; i++)
        if (g_rb((u16)(CBT(i) + 0x14)) == 1 && (s8)g_rb((u16)(GV_cbt_floor + i)) == fl && on_stairs(i)) { hook_call_original(); return; }
    if (lead < 0 || lead > 4 || g_rb((u16)(CBT(lead) + 0x14)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + lead)) != fl) { hook_call_original(); return; }
    int ns = (s8)rb(G_FAR20A5, FLR_NSTAIRS), best = -1, bd = 0x7fffffff, bx = 0, by = 0;
    int x = (s16)g_rw((u16)(CBT(lead) + 4)), y = (s16)g_rw((u16)(CBT(lead) + 6));
    for (int s = 0; s < ns && s < 6; s++) {
        u16 o = (u16)(FLR_STAIRS + 14 * s);
        int cx = (s16)rw(G_FAR20A5, o) + rb(G_FAR20A5, (u16)(o + 4)) / 2, cy = (s16)rw(G_FAR20A5, (u16)(o + 2)) + rb(G_FAR20A5, (u16)(o + 5)) / 2;
        int d = (cx - x) * (cx - x) + 4 * (cy - y) * (cy - y);
        if (d < bd) { bd = d; best = s; bx = cx; by = cy; }
    }
    bios_kbd_get(1);                                         /* (the key is ours) */
    if (best < 0) { ui_notice("There are no stairs on this floor that the party has found."); return; }
    levelwalk_x = bx; levelwalk_y = by; levelwalk_floor = fl;
    stairs_pending = best; stairs_floor = fl; stairs_t = plat_ticks_ms();
    ui_notice("The party walks to the stairs and takes them.");
    if (g_rb(GV_ai_paused) == 1) { extern void bios_kbd_inject(u16); bios_kbd_inject(0x3920); }   /* (on its way at once: Space) */
}

void levelwalk_init(void)
{
    hook_install(14, 0x0e76, 0x3166, input_hook);
    hook_install(GF_take_exit, take_exit_hook);
    hook_install(GF_combat_keys, keys_hook);
}
