/* levelwalk.c - a click on the level map (Tab, ui/levelmap.cpp) walks the party there.
 * Always with the map (but in the original game); the party also takes the stairs together.
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
#include <string.h>
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
void levelwalk_stairs(int s);
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
/* the leader's walk to (tx, ty), or the nearest spot round it the leader can reach (a cell under a wall,
   or one a door still shut keeps apart): its destination, path_find and path_give, the action "walk",
   the others following (single file). 1 given, 0 no way */
void pathfix_long_search(int on);
static int walk_to(int lead, int tx, int ty, int *gx, int *gy)
{
    int otx = (s16)g_rw(CBF(lead, CB_TO_X)), oty = (s16)g_rw(CBF(lead, CB_TO_Y));
    int mx = -1, my = -1;
    pathfix_long_search(1);
    for (int r = 0; r <= 2 && mx < 0; r++)
        for (int dy = -r; dy <= r && mx < 0; dy++)
            for (int dx = -r; dx <= r && mx < 0; dx++) {
                if (r && dx > -r && dx < r && dy > -r && dy < r) continue;      /* (the ring at distance r) */
                int px = tx + dx * 32, py = ty + dy * 16;
                if (px < 0 || py < 0 || px >= 40 * 32 || py >= 40 * 16) continue;
                g_ww(CBF(lead, CB_TO_X), (u16)px); g_ww(CBF(lead, CB_TO_Y), (u16)py);
                if (ns_call(GF_path_find, 1, lead) == 1) { mx = px; my = py; }
                if (ns_unwinding) { pathfix_long_search(0); return 0; }
            }
    pathfix_long_search(0);
    if (mx < 0) { g_ww(CBF(lead, CB_TO_X), (u16)otx); g_ww(CBF(lead, CB_TO_Y), (u16)oty); return 0; }
    for (int i = 0; i < 5; i++) if (g_rb(CBF(i, CB_STATUS)) == 1) g_wb((u16)(GV_cbt_moving + i), 1);
    ns_call(GF_path_give, 1, lead);                             /* (the path found last: to mx, my) */
    if (ns_unwinding) return 0;
    g_ww(CBF(lead, CB_ORDER), 1);
    g_wb(CBF(lead, CB_TARGET), 0xff);
    g_wb((u16)(0xd6fb + lead), 0);
    *gx = mx; *gy = my;
    return 1;
}
/* a walk given from the level map, carried on: the game's path holds 250 steps, and a longer route (from
   one end of a mine to the other) ended there, the party standing half way as if the click had not been
   taken. When the leader stops short of the place, having got on since the last leg, the next leg is
   given, up to a dozen; a fight (real_combat) or another order ends it */
int levelwalk_goal_x = -1, levelwalk_goal_y = -1, levelwalk_goal_floor = -1;   /* (ui/levelmap.cpp marks it) */
static int goal_lead = -1, goal_legs, goal_fx, goal_fy;
static int door_pending = -1, door_floor = -1, door_wx, door_wy;   /* a door clicked on the field: used on arrival (levelwalk_door) */
static void goal_drop(void) { levelwalk_goal_x = levelwalk_goal_y = levelwalk_goal_floor = -1; goal_lead = -1; door_pending = -1; }
static void goal_check(void)
{
    if (goal_lead < 0) return;
    int lead = goal_lead;
    if (g_rb(GV_real_combat) || g_rb(CBF(lead, CB_STATUS)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + lead)) != levelwalk_goal_floor) { goal_drop(); return; }
    int x = (s16)g_rw(CBF(lead, CB_X)), y = (s16)g_rw(CBF(lead, CB_Y));
    if (abs(x - levelwalk_goal_x) <= 20 && abs(y - levelwalk_goal_y) <= 12) {                         /* (there) */
        int d = door_pending; goal_drop();
        if (d >= 0 && (s8)g_rb(GV_floor_shown) == door_floor && !g_rb(GV_real_combat)) {               /* (a door clicked: it is used, as Use Door) */
            ns_call(GF_door_toggle, 2, d, lead);
            if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: door %d used by %d\n", d, lead);
        }
        return;
    }
    if (g_rb((u16)(GV_cbt_moving + lead)) || g_rw(CBF(lead, CB_ORDER)) == 1) return;                 /* (on the way) */
    if ((s16)g_rw(CBF(lead, CB_TO_X)) != levelwalk_goal_x && abs((s16)g_rw(CBF(lead, CB_TO_X)) - levelwalk_goal_x) > 64) { goal_drop(); return; }   /* (another order since) */
    if (goal_legs >= 12 || (abs(x - goal_fx) < 8 && abs(y - goal_fy) < 8)) {                           /* (no further) */
        goal_drop(); ui_notice("The party could get no nearer to the place chosen on the level map."); return;
    }
    int gx, gy;
    goal_fx = x; goal_fy = y; goal_legs++;
    if (!walk_to(lead, levelwalk_goal_x, levelwalk_goal_y, &gx, &gy)) { if (!ns_unwinding) goal_drop(); return; }
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: leg %d from %d,%d to %d,%d\n", goal_legs, x, y, gx, gy);
}
static void input_hook(void)                 /* s014 0e76:3166 combat_input () */
{
    if (stairs_pending >= 0 && levelwalk_x < 0) { stairs_arrive(); if (ns_unwinding) return; }
    if (levelwalk_x < 0) { goal_check(); if (ns_unwinding) return; hook_call_original(); return; }
    int tx = levelwalk_x, ty = levelwalk_y, tfl = levelwalk_floor;
    levelwalk_x = levelwalk_y = levelwalk_floor = -1;
    int door = door_pending >= 0 && tx == door_wx && ty == door_wy ? door_pending : -1;   /* (another walk asked since: no door) */
    door_pending = -1;
    if ((s8)g_rb(GV_floor_shown) != tfl) { hook_call_original(); return; }
    int was_mode = g_rb(GV_move_mode);
    if (was_mode != 2) { ns_call(16, 0x1ef8, 0x0cdc, 0); if (ns_unwinding) return; }
    int lead = (s8)g_rb(GV_marching_order);
    if (lead < 0 || lead > 4 || g_rb(CBF(lead, CB_STATUS)) != 1) { hook_call_original(); return; }
    int mx, my;
    if (!walk_to(lead, tx, ty, &mx, &my)) {
        if (ns_unwinding) return;
        goal_drop();
        ui_notice("The party cannot get there from where it stands.");
        hook_call_original(); return;
    }
    goal_lead = lead; levelwalk_goal_x = mx; levelwalk_goal_y = my; levelwalk_goal_floor = tfl; goal_legs = 0; door_pending = door;
    goal_fx = (s16)g_rw(CBF(lead, CB_X)); goal_fy = (s16)g_rw(CBF(lead, CB_Y));
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: map %d,%d -> %d,%d for %d (move mode %d, was %d)\n", tx, ty, mx, my, lead, g_rb(GV_move_mode), was_mode);
    /* (word that the click was taken: the map closes on it, and the party is seen to go. Out of a fight the
       battle paused goes on at once, as for the stairs; in a fight it waits for Space) */
    const char *file = was_mode != 2 ? " in single file (press G to move as a group again)" : "";
    char m[200];
    if (door_pending >= 0) { hook_call_original(); return; }       /* (a door clicked: told so at the click) */
    if (g_rb(GV_ai_paused) == 1 && !g_rb(GV_real_combat)) { extern void bios_kbd_inject(u16); bios_kbd_inject(0x3920); snprintf(m, sizeof m, "The party walks there%s.", file); }
    else if (g_rb(GV_ai_paused) == 1) snprintf(m, sizeof m, "The party will walk there%s: Space resumes the battle.", file);
    else snprintf(m, sizeof m, "The party walks there%s.", file);
    ui_notice(m);
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
    int explore = !g_rb(GV_real_combat) && who >= 0 && who < 5 && !following && !config_get_int("original", 0);
    hook_call_original();
    if (ns_unwinding || !explore) return;
    following = 1;
    for (int i = 0; i < 5 && !ns_unwinding; i++) {
        if (i == who || g_rb(CBF(i, CB_STATUS)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + i)) != from) continue;
        ns_call(GF_take_exit, 2, n, i);
        if (!ns_unwinding) ns_call(GF_cbt_cancel_action, 1, i);
    }
    following = 0;
}
static int stairs_under(int i)
{
    int ns = (s8)rb(G_FAR20A5, FLR_NSTAIRS), x = (s16)g_rw(CBF(i, CB_X)), y = (s16)g_rw(CBF(i, CB_Y));
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
    if (!bios_kbd_avail() || g_rb(GV_real_combat) || !!config_get_int("original", 0)) { hook_call_original(); return; }
    u16 k = bios_kbd_get(0);
    if (k != 0x1655 && k != 0x1675) { hook_call_original(); return; }
    int fl = (s8)g_rb(GV_floor_shown), lead = (s8)g_rb(GV_marching_order);
    for (int i = 0; i < 5; i++)
        if (g_rb(CBF(i, CB_STATUS)) == 1 && (s8)g_rb((u16)(GV_cbt_floor + i)) == fl && on_stairs(i)) { hook_call_original(); return; }
    if (lead < 0 || lead > 4 || g_rb(CBF(lead, CB_STATUS)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + lead)) != fl) { hook_call_original(); return; }
    int ns = (s8)rb(G_FAR20A5, FLR_NSTAIRS), best = -1, bd = 0x7fffffff;
    int x = (s16)g_rw(CBF(lead, CB_X)), y = (s16)g_rw(CBF(lead, CB_Y));
    for (int s = 0; s < ns && s < 6; s++) {
        u16 o = (u16)(FLR_STAIRS + 14 * s);
        int cx = (s16)rw(G_FAR20A5, o) + rb(G_FAR20A5, (u16)(o + 4)) / 2, cy = (s16)rw(G_FAR20A5, (u16)(o + 2)) + rb(G_FAR20A5, (u16)(o + 5)) / 2;
        int d = (cx - x) * (cx - x) + 4 * (cy - y) * (cy - y);
        if (d < bd) { bd = d; best = s; }
    }
    bios_kbd_get(1);                                         /* (the key is ours) */
    if (best < 0) { ui_notice("There are no stairs on this floor that the party has found."); return; }
    levelwalk_stairs(best);
}
/* the party walks to stairs s of the floor shown and, outside a fight, takes them on arrival (U above;
   a click on a floor of the level map's chart, ui/levelmap.cpp) */
void levelwalk_stairs(int s)
{
    int fl = (s8)g_rb(GV_floor_shown), ns = (s8)rb(G_FAR20A5, FLR_NSTAIRS);
    if (s < 0 || s >= ns || s >= 6) return;
    u16 o = (u16)(FLR_STAIRS + 14 * s);
    levelwalk_x = (s16)rw(G_FAR20A5, o) + rb(G_FAR20A5, (u16)(o + 4)) / 2; levelwalk_y = (s16)rw(G_FAR20A5, (u16)(o + 2)) + rb(G_FAR20A5, (u16)(o + 5)) / 2; levelwalk_floor = fl;
    int take = !g_rb(GV_real_combat);
    if (take) { stairs_pending = s; stairs_floor = fl; stairs_t = plat_ticks_ms(); }
    ui_notice(take ? "The party walks to the stairs and takes them." : "The party walks to the stairs.");
    if (g_rb(GV_ai_paused) == 1) { extern void bios_kbd_inject(u16); bios_kbd_inject(0x3920); }   /* (on its way at once: Space) */
}

void levelwalk_battle_begins(void) { goal_drop(); stairs_pending = -1; }

/* ---------------------------------------------------------------- doors on the field
 * Out of a fight a door is used by walking beside it and choosing Use Door, and nothing on the field told
 * that a door could be used: a click on one said "can't walk there" (a door stands in the wall). Now the
 * pointer over a door says so (ui/battleoverlay.cpp), and a click on it walks the party to the floor
 * before it and uses it on arrival (door_toggle, s023 1ef8:00b8, as the order does: a lock still holds).
 * The view: a ground point (map x, y) is drawn at field x = (x - 32 ox) - (y - 16 oy) + 135, y = y - 16 oy
 * + 109 (ox, oy the view's origin, D_c866 / c886; field x = the game's x - 60), measured from the
 * combatants' boxes (D_d8f1 ...) against their places; a door's picture stands on an edge of its doorway
 * and rises some 40 pixels above it. Doors: count 20a5:1f52, 14-byte records at 1f54 (x, y, w, h map pixels), open 20a5:33d3[k]. */
static s32 lw_cell(int x, int y) { u16 o = (u16)((x * 40 + y) * 4); return (s32)((u32)rw(G_FAR20A5, o) | (u32)rw(G_FAR20A5, (u16)(o + 2)) << 16); }
int levelwalk_door_at(int gx, int gy)          /* the door under the game's pointer (gx, gy), or -1 */
{
    if (game_mode() != GM_BATTLE || config_get_int("original", 0) || g_rb(GV_real_combat)) return -1;
    int fx = gx - 60;
    if (fx < 0 || fx >= 0x104) return -1;
    int ox = (s8)g_rb(0xc866) * 32, oy = (s8)g_rb(0xc886) * 16;
    int n = (s8)rb(G_FAR20A5, 0x1f52), best = -1, bd = 1 << 30;
    if (n < 0 || n > 40) return -1;
    for (int k = 0; k < n; k++) {
        u16 r0 = (u16)(0x1f54 + 14 * k);
        int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
        int sx0 = (x - ox) - (y + h - oy) + 135 - 12, sx1 = (x + w - ox) - (y - oy) + 135 + 2;   /* (the picture: along the doorway's edge, rising above it) */
        int sy1 = (y + h - oy) + 109, sy0 = (y - oy) + 109 - 44;
        if (fx < sx0 || fx > sx1 || gy < sy0 || gy > sy1) continue;
        int cx = (sx0 + sx1) / 2, d = abs(fx - cx) + abs(gy - (sy0 + sy1) / 2);
        if (d < bd) { bd = d; best = k; }
    }
    return best;
}
int levelwalk_door_open(int n) { return n >= 0 && n < 40 && rb(G_FAR20A5, (u16)(0x33d3 + n)) != 0; }
/* the party to the floor before door n (the nearest floor cell to it, on the side the party is), the door used there */
int levelwalk_door(int n)
{
    int nd = (s8)rb(G_FAR20A5, 0x1f52), lead = (s8)g_rb(GV_marching_order);
    if (n < 0 || n >= nd || lead < 0 || lead > 4) return 0;
    u16 r0 = (u16)(0x1f54 + 14 * n);
    int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
    float dx = x + w / 2.0f, dy = y + h / 2.0f;
    int lx = (s16)g_rw(CBF(lead, CB_X)), ly = (s16)g_rw(CBF(lead, CB_Y));
    /* the cells the party reaches without going through a doorway (floor cells, by their sides), from the
       leader's: of those round the door, the nearest to it is where they stop (on their own side of it) */
    static signed char dist[40][40];
    static unsigned char doorway[40][40];
    memset(doorway, 0, sizeof doorway);
    for (int k = 0; k < nd && k < 40; k++) {
        u16 q = (u16)(0x1f54 + 14 * k);
        int qx = (s16)rw(G_FAR20A5, q), qy = (s16)rw(G_FAR20A5, (u16)(q + 2)), qw = rb(G_FAR20A5, (u16)(q + 4)), qh = rb(G_FAR20A5, (u16)(q + 5));
        for (int cx = qx / 32; cx <= (qx + qw - 1) / 32; cx++) for (int cy = qy / 16; cy <= (qy + qh - 1) / 16; cy++)
            if (cx >= 0 && cx < 40 && cy >= 0 && cy < 40) doorway[cx][cy] = 1;
    }
    memset(dist, -1, sizeof dist);
    static short qx_[1600], qy_[1600]; int qh_ = 0, qt_ = 0;
    int sx = lx / 32, sy = ly / 16;
    if (sx >= 0 && sx < 40 && sy >= 0 && sy < 40) { dist[sx][sy] = 0; qx_[qt_] = (short)sx; qy_[qt_++] = (short)sy; }
    while (qh_ < qt_) {
        int cx = qx_[qh_], cy = qy_[qh_++];
        static const int D[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
        for (int k = 0; k < 4; k++) {
            int nx = cx + D[k][0], ny = cy + D[k][1];
            if (nx < 0 || nx >= 40 || ny < 0 || ny >= 40 || dist[nx][ny] >= 0 || doorway[nx][ny] || lw_cell(nx, ny) < 0) continue;
            dist[nx][ny] = (signed char)(dist[cx][cy] < 120 ? dist[cx][cy] + 1 : 120); qx_[qt_] = (short)nx; qy_[qt_++] = (short)ny;
        }
    }
    float bd = 1e9f; int bx = -1, by = -1;
    for (int cx = (int)(dx / 32) - 2; cx <= (int)(dx / 32) + 2; cx++)
        for (int cy = (int)(dy / 16) - 2; cy <= (int)(dy / 16) + 2; cy++) {
            if (cx < 0 || cx >= 40 || cy < 0 || cy >= 40 || dist[cx][cy] < 0) continue;
            float px = cx * 32 + 16.0f, py = cy * 16 + 8.0f;
            float d = (px - dx) * (px - dx) + 4 * (py - dy) * (py - dy);
            if (d < bd) { bd = d; bx = cx; by = cy; }
        }
    if (bx < 0) return 0;
    levelwalk_x = bx * 32 + 16; levelwalk_y = by * 16 + 8; levelwalk_floor = (s8)g_rb(GV_floor_shown);
    door_pending = n; door_floor = levelwalk_floor; door_wx = levelwalk_x; door_wy = levelwalk_y;
    return 1;
}
/* a click on the field (ui/ui.cpp, before the game has it): on a door, out of a fight, the walk to it; 1 taken */
int levelwalk_door_click(void)
{
    int ui_pointer_on_cover(void), plat_pointer_outside(void);
    void plat_mouse_game(int *x, int *y);
    if (ui_pointer_on_cover() || plat_pointer_outside()) return 0;
    int gx, gy; plat_mouse_game(&gx, &gy);
    int d = levelwalk_door_at(gx, gy);
    if (d < 0 || !levelwalk_door(d)) return 0;
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: click on door %d\n", d);
    if (g_rb(GV_ai_paused) == 1) { extern void bios_kbd_inject(u16); bios_kbd_inject(0x3920); }   /* (on its way at once, as from the level map) */
    ui_notice(levelwalk_door_open(d) ? "The party walks to the door to close it." : "The party walks to the door to open it.");
    return 1;
}
void levelwalk_init(void)
{
    hook_install(14, 0x0e76, 0x3166, input_hook);
    hook_install(GF_take_exit, take_exit_hook);
    hook_install(GF_combat_keys, keys_hook);
}
