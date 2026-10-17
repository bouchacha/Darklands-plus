/* levelwalk.c - a click on the level map (Tab, ui/levelmap.cpp) walks the party there: the
 * tactical view is scrolled so that the cell is in sight and the click the player would have
 * made on it is made for them (the game then walks the party, or the member given orders, as
 * for any click on the floor). assist.level_map_walk, on by default.
 *
 * The view's centre cell is DS:c866 / c886 (bytes); the original's scroll keys change them and
 * redraw with vec 1d3d (combat_input, s014 0e76:3166). A cell is 32 x 16 map pixels and the
 * view's slant puts map (mx, my) at screen (mx - my, my) from the view's origin (combat_mouse's
 * own reading of a click, turned round). Walking one by one, the click would be an order for one
 * member: the party is put to Travel As Group first (G). Done at the start of the next
 * combat_input, in the game's own time. */
#include <stdio.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"

#define VECSEG ((u16)(LOADSEG + 0x9c0))

int levelwalk_x = -1, levelwalk_y = -1, levelwalk_floor = -1;     /* the place, map pixels (32 x 16 a cell), and its floor */
void ui_notice(const char *msg);

/* where map pixel (mx, my) is on the screen now (game pixels), as combat_mouse (s014 19cd)
   reads a click back: map y = (y - F_20a5_41f4) + c293, map x = (y - 41f4) - F_20a5_4be0 + (x - 60) - 16 + c26c */
static void to_screen(int mx, int my, int *sx, int *sy)
{
    int oy = (s16)rw(G_FAR20A5, 0x41f4), ox = (s16)rw(G_FAR20A5, 0x4be0);
    int ry = my - (s16)g_rw(0xc293);
    *sy = ry + oy;
    *sx = mx - (s16)g_rw(0xc26c) + 16 + ox - ry + 60;
}
static int on_field(int x, int y) { return x >= 64 && x <= 316 && y >= 10 && y <= 190; }

static void input_hook(void)                 /* s014 0e76:3166 combat_input () */
{
    if (levelwalk_x < 0) { hook_call_original(); return; }
    int tx = levelwalk_x, ty = levelwalk_y, tfl = levelwalk_floor;
    levelwalk_x = levelwalk_y = levelwalk_floor = -1;
    if ((s8)g_rb(GV_floor_shown) != tfl) { hook_call_original(); return; }
    int mx = tx, my = ty;
    int sx, sy;
    to_screen(mx, my, &sx, &sy);
    int ocx = (s8)g_rb(0xc866), ocy = (s8)g_rb(0xc886);
    if (!on_field(sx, sy)) {                                   /* out of sight: the view centred on it */
        g_wb(0xc866, (u8)(mx / 32)); g_wb(0xc886, (u8)(my / 16));
        ns_call_ptr(VECSEG, 0x1d3d, 0);
        to_screen(mx, my, &sx, &sy);
    }
    if (!on_field(sx, sy)) {
        ui_notice("That place cannot be brought into view from here.");
        hook_call_original(); return;
    }
    if (g_rb(GV_move_mode) == 0) ns_call(16, 0x1ef8, 0x0bd6, 0);    /* walking one by one: Travel As Group (G), as the key does */
    /* the place itself, else the nearest the party can go to (by the game's own test of a group
       move, s014 19cd:11ec, which a click makes first: a cell under a wall or too tight for the
       party's formation fails) */
    {
        int lead = (s8)g_rb(GV_marching_order);
        if (lead < 0 || lead > 4) lead = 0;
        int best = 0;
        for (int r = 0; r <= 2 && !best; r++)
            for (int dy = -r; dy <= r && !best; dy++)
                for (int dx = -r; dx <= r && !best; dx++) {
                    if (r && dx > -r && dx < r && dy > -r && dy < r) continue;      /* (the ring at distance r) */
                    int px = mx + dx * 32, py = my + dy * 16, qx, qy;
                    to_screen(px, py, &qx, &qy);
                    if (!on_field(qx, qy)) continue;
                    if (ns_call(14, 0x19cd, 0x11ec, 3, lead, px, py) == 1) { mx = px; my = py; sx = qx; sy = qy; best = 1; }
                    if (ns_unwinding) return;
                }
        if (!best) {
            ui_notice("The party cannot get there from where it stands.");
            hook_call_original(); return;
        }
    }
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: map %d,%d -> screen %d,%d (view %d,%d -> %d,%d, move mode %d)\n", tx, ty, sx, sy, ocx, ocy, (s8)g_rb(0xc866), (s8)g_rb(0xc886), g_rb(GV_move_mode));
    void plat_click_game(int gx, int gy);
    plat_click_game(sx, sy);
    if (g_rb(GV_ai_paused) == 1) ui_notice("The party will walk there: Space resumes the battle.");
    hook_call_original();
}

void levelwalk_init(void)
{
    hook_install(14, 0x0e76, 0x3166, input_hook);
}
