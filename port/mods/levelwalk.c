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
/* the leader's walk to (tx, ty) - or a spot in its cell, or in the cells round it, that the leader can reach (a
   cell under a wall, or one a door still shut keeps apart): its destination, path_find and path_give, the action
   "walk", the others following (single file). 1 given, 0 no way */
void pathfix_long_search(int on);
static int walk_to(int lead, int tx, int ty, int *gx, int *gy)
{
    int otx = (s16)g_rw(CBF(lead, CB_TO_X)), oty = (s16)g_rw(CBF(lead, CB_TO_Y));
    int mx = -1, my = -1;
    pathfix_long_search(1);
    /* first the cell itself: its middle, else the spot in it nearest the middle a member can stand on (beside a
       wall the middle can be too near it for the member's breadth, and the walk went to the next cell: 1.4,
       "only works when you pick a tile that isn't next to a wall"; a spot the game refuses is told at once) */
    static int off[49][2], noff;
    if (!noff) {
        for (int dy = -6; dy <= 6; dy += 2) for (int dx = -12; dx <= 12; dx += 4) { off[noff][0] = dx; off[noff][1] = dy; noff++; }
        for (int i = 1; i < noff; i++) for (int j = i; j > 0 && off[j][0] * off[j][0] + 4 * off[j][1] * off[j][1] < off[j - 1][0] * off[j - 1][0] + 4 * off[j - 1][1] * off[j - 1][1]; j--) {
            int t0 = off[j][0], t1 = off[j][1]; off[j][0] = off[j - 1][0]; off[j][1] = off[j - 1][1]; off[j - 1][0] = t0; off[j - 1][1] = t1; }
    }
    for (int k = 0; k < noff && mx < 0; k++) {
        int px = tx + off[k][0], py = ty + off[k][1];
        if (px < 0 || py < 0 || px >= 40 * 32 || py >= 40 * 16 || px / 32 != tx / 32 || py / 16 != ty / 16) continue;
        g_ww(CBF(lead, CB_TO_X), (u16)px); g_ww(CBF(lead, CB_TO_Y), (u16)py);
        if (ns_call(GF_path_find, 1, lead) == 1) { mx = px; my = py; }
        if (ns_unwinding) { pathfix_long_search(0); return 0; }
    }
    for (int r = 1; r <= 2 && mx < 0; r++)
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
static int door_pending = -1, door_floor = -1, door_wx, door_wy, door_wait;   /* a door clicked on the field: used on arrival (levelwalk_door) */
#define DOOR_REC(n) ((u16)(0x1f54 + 14 * (n)))      /* doors: count 20a5:1f52, 14-byte records (x, y, w, h map pixels; +12 the door paired with it, -1 none) */
static int door_partner(int n) { return (s8)rb(G_FAR20A5, (u16)(DOOR_REC(n) + 12)); }
/* the door whose doorway member i stands in, as the game has it (s014 19cd:1ade, each round: D_a696[i] bit 2, the door D_c64c[i];
   with it what Use Door will do, D_c678[i] open or shut, and from which side, D_c6ef[i]), or -1 */
static int in_doorway(int i) { return (g_rb((u16)(0xa696 + 2 * i)) & 2) ? (s8)g_rb((u16)(0xc64c + i)) : -1; }
static void goal_drop(void) { levelwalk_goal_x = levelwalk_goal_y = levelwalk_goal_floor = -1; goal_lead = -1; door_pending = -1; }
static void goal_check(void)
{
    if (goal_lead < 0) return;
    int lead = goal_lead;
    if (g_rb(GV_real_combat) || g_rb(CBF(lead, CB_STATUS)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + lead)) != levelwalk_goal_floor) { goal_drop(); return; }
    int x = (s16)g_rw(CBF(lead, CB_X)), y = (s16)g_rw(CBF(lead, CB_Y));
    if (door_pending >= 0) {                                       /* (a door clicked: used as Use Door, once the game has the leader in its doorway) */
        int gd = in_doorway(lead), d = door_pending;
        if (gd >= 0 && (gd == d || gd == door_partner(d)) && (s8)g_rb(GV_floor_shown) == door_floor) {
            goal_drop();
            ns_call(GF_door_toggle, 2, gd, lead);                   /* (then as the game's Use Door, s016: the view drawn anew - the room */
            if (!ns_unwinding) ns_call(GF_floor_view_draw, 1, -1);   /*  beyond an opened door shows at once - and the order ended) */
            if (!ns_unwinding) ns_call(GF_cbt_cancel_action, 1, lead);
            if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: door %d used by %d\n", gd, lead);
            return;
        }
    }
    if (abs(x - levelwalk_goal_x) <= 20 && abs(y - levelwalk_goal_y) <= 12) {                         /* (there) */
        if (door_pending >= 0 && !g_rb((u16)(GV_cbt_moving + lead)) && ++door_wait > 20) {           /* (stopped short of the doorway) */
            goal_drop(); ui_notice("The party could not get into the doorway.");
        }
        if (door_pending < 0) goal_drop();
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
    extern unsigned dos_save_read_ms; static unsigned last_read;
    if (dos_save_read_ms != last_read) {         /* (a game loaded: a walk, stairs or door asked for before it are not its) */
        last_read = dos_save_read_ms;
        goal_drop(); stairs_pending = -1; levelwalk_x = levelwalk_y = levelwalk_floor = -1;
        if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: a game loaded, walks forgotten\n");
    }
    if (stairs_pending >= 0 && levelwalk_x < 0) { stairs_arrive(); if (ns_unwinding) return; }
    if (levelwalk_x < 0) { goal_check(); if (ns_unwinding) return; hook_call_original(); return; }
    /* (a battle's start - a save loaded too - chooses its leader and pauses for orders once the leader is on the
       screen, D_d6f1 till then: a walk given before that pause stood still in it. It waits for it, a moment) */
    static u32 defer_t;
    if (g_rb(0xd6f1) == 1) {
        if (!defer_t) { defer_t = plat_ticks_ms() | 1; if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: the walk waits for the battle's first pause\n"); }
        if (plat_ticks_ms() - defer_t < 10000) { hook_call_original(); return; }
    }
    defer_t = 0;
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
    goal_lead = lead; levelwalk_goal_x = mx; levelwalk_goal_y = my; levelwalk_goal_floor = tfl; goal_legs = 0; door_pending = door; door_wait = 0;
    goal_fx = (s16)g_rw(CBF(lead, CB_X)); goal_fy = (s16)g_rw(CBF(lead, CB_Y));
    if (getenv("DL_LEVELWALK_LOG")) fprintf(stderr, "levelwalk: map %d,%d -> %d,%d for %d (move mode %d, was %d; paused %d)\n", tx, ty, mx, my, lead, g_rb(GV_move_mode), was_mode, g_rb(GV_ai_paused));
    /* (word that the click was taken: the map closes on it, and the party is seen to go. Out of a fight the
       battle paused goes on at once, as for the stairs; in a fight it waits for Space) */
    const char *file = was_mode != 2 ? " in single file (press G to move as a group again)" : "";
    char m[200];
    if (door_pending >= 0) {                                       /* (a door clicked: told so at the click) */
        if (was_mode != 2) ui_notice("The party goes in single file (press G to move as a group again).");
        hook_call_original(); return;
    }
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
static int by_door(int i)                  /* member i within a few paces of a door (U is then Use Door) */
{
    int n = (s8)rb(G_FAR20A5, 0x1f52), x = (s16)g_rw(CBF(i, CB_X)), y = (s16)g_rw(CBF(i, CB_Y));
    for (int k = 0; k < n && k < 40; k++) {
        u16 o = (u16)(0x1f54 + 14 * k);
        int rx = (s16)rw(G_FAR20A5, o), ry = (s16)rw(G_FAR20A5, (u16)(o + 2)), w = rb(G_FAR20A5, (u16)(o + 4)), h = rb(G_FAR20A5, (u16)(o + 5));
        if (x >= rx - 24 && x < rx + w + 24 && y >= ry - 12 && y < ry + h + 12) return 1;
    }
    return 0;
}
static void keys_hook(void)                /* combat_keys (): U with nobody on the stairs */
{
    extern int bios_kbd_avail(void); extern u16 bios_kbd_get(int);
    if (!bios_kbd_avail() || g_rb(GV_real_combat) || !!config_get_int("original", 0)) { hook_call_original(); return; }
    u16 k = bios_kbd_get(0);
    if (k != 0x1655 && k != 0x1675) { hook_call_original(); return; }
    int fl = (s8)g_rb(GV_floor_shown), lead = (s8)g_rb(GV_marching_order);
    for (int i = 0; i < 5; i++)
        if (g_rb(CBF(i, CB_STATUS)) == 1 && (s8)g_rb((u16)(GV_cbt_floor + i)) == fl && (on_stairs(i) || by_door(i))) { hook_call_original(); return; }   /* (U there is the game's: the stairs, or Use Door) */
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
 * combatants' boxes (D_d8f1 ...) against their places; a door's picture stands on the edge between its
 * doorway and its pair's and rises some 40 pixels above it: a click is for the door on that face only (a
 * box round it took in floor and the dark beside it: 1.4-RC, "the whole area registers a click to the
 * door"), and only for a door seen (its doorway or its pair's revealed). Doors: count 20a5:1f52, 14-byte
 * records at 1f54 (x, y, w, h map pixels; +12 its pair), open 20a5:33d3[k]. */
static s32 lw_cell(int x, int y) { u16 o = (u16)((x * 40 + y) * 4); return (s32)((u32)rw(G_FAR20A5, o) | (u32)rw(G_FAR20A5, (u16)(o + 2)) << 16); }
/* a wall on side d of cell (x, y) (d 0 east, 1 west, 2 south, 3 north: the low nibble of the cell's bytes 0, 1,
   3, 2 - 1..11 a wall, 12..15 a door open) */
static int lw_wall(int x, int y, int d) { static const int SH[4] = { 0, 8, 24, 16 }; int t = (int)((u32)lw_cell(x, y) >> SH[d]) & 15; return t >= 1 && t <= 11; }
/* the cells the party reaches from the leader's without going through a shut door's doorway or a wall (floor
   cells, by their sides): dist (cells, -1 not), and which cells are such doorways */
static signed char rdist[40][40];
static unsigned char rdoor[40][40];
static int reach(int lead)
{
    int nd = (s8)rb(G_FAR20A5, 0x1f52);
    if (lead < 0 || lead > 4 || nd < 0 || nd > 40) return 0;
    memset(rdoor, 0, sizeof rdoor);
    for (int k = 0; k < nd; k++) {
        if (rb(G_FAR20A5, (u16)(0x33d3 + k))) continue;                   /* (an open door is no wall) */
        u16 q = DOOR_REC(k);
        int qx = (s16)rw(G_FAR20A5, q), qy = (s16)rw(G_FAR20A5, (u16)(q + 2)), qw = rb(G_FAR20A5, (u16)(q + 4)), qh = rb(G_FAR20A5, (u16)(q + 5));
        if (qx < 0 || qy < 0) continue;
        for (int cx = qx / 32; cx <= (qx + qw - 1) / 32; cx++) for (int cy = qy / 16; cy <= (qy + qh - 1) / 16; cy++)
            if (cx >= 0 && cx < 40 && cy >= 0 && cy < 40) rdoor[cx][cy] = 1;
    }
    memset(rdist, -1, sizeof rdist);
    static short qx_[1600], qy_[1600]; int qh_ = 0, qt_ = 0;
    int sx = (s16)g_rw(CBF(lead, CB_X)) / 32, sy = (s16)g_rw(CBF(lead, CB_Y)) / 16;
    if (sx < 0 || sx >= 40 || sy < 0 || sy >= 40) return 0;
    rdist[sx][sy] = 0; qx_[qt_] = (short)sx; qy_[qt_++] = (short)sy;
    while (qh_ < qt_) {
        int cx = qx_[qh_], cy = qy_[qh_++];
        static const int D[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
        for (int k = 0; k < 4; k++) {
            int nx = cx + D[k][0], ny = cy + D[k][1];
            if (nx < 0 || nx >= 40 || ny < 0 || ny >= 40 || rdist[nx][ny] >= 0 || rdoor[nx][ny] || lw_cell(nx, ny) < 0) continue;
            if (lw_wall(cx, cy, k) || lw_wall(nx, ny, k ^ 1)) continue;
            rdist[nx][ny] = (signed char)(rdist[cx][cy] < 120 ? rdist[cx][cy] + 1 : 120); qx_[qt_] = (short)nx; qy_[qt_++] = (short)ny;
        }
    }
    return 1;
}
int levelwalk_door_open(int n);
int levelwalk_door_at(int gx, int gy)          /* the door under the game's pointer (gx, gy), or -1 */
{
    if (game_mode() != GM_BATTLE || config_get_int("original", 0) || g_rb(GV_real_combat)) return -1;
    int menuview_open(void), ui_is_open(void); extern int ns_screen;
    if (menuview_open() || ui_is_open() || ns_screen != NS_NONE) return -1;    /* (a menu, a window or a screen over the field: the click is theirs) */
    int fx = gx - 60;
    if (fx < 0 || fx >= 0x104) return -1;
    int ox = (s8)g_rb(0xc866) * 32, oy = (s8)g_rb(0xc886) * 16;
    int n = (s8)rb(G_FAR20A5, 0x1f52), best = -1, bd = 1 << 30;
    if (n < 0 || n > 40) return -1;
    int nh = (s8)g_rb(GV_hotspot_count);                                  /* (on a figure: the figure's, a member chosen or a foe) */
    for (int k = 0; k < nh && k < 40; k++)
        if (fx >= (s16)g_rw((u16)(0xd8f1 + 2 * k)) && fx <= (s16)g_rw((u16)(0xd8d7 + 2 * k)) && gy >= (s16)g_rw((u16)(0xd8bd + 2 * k)) && gy <= (s16)g_rw((u16)(0xd8a3 + 2 * k))) return -1;
    for (int k = 0; k < n; k++) {
        u16 r0 = DOOR_REC(k);
        if (levelwalk_door_open(k)) continue;                              /* (an open door: its doorway is a way through, walked as the game walks) */
        int p = door_partner(k);
        if (p < 0 || p >= n) continue;
        int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
        int px = (s16)rw(G_FAR20A5, DOOR_REC(p)), py = (s16)rw(G_FAR20A5, (u16)(DOOR_REC(p) + 2));
        if (x < 0 || y < 0 || px < 0 || py < 0) continue;
        if ((x / 32 >= 40 || y / 16 >= 40 || lw_cell(x / 32, y / 16) < 0) && (px / 32 >= 40 || py / 16 >= 40 || lw_cell(px / 32, py / 16) < 0)) continue;   /* (in the dark: not seen) */
        /* the door's face: on the edge between its doorway and its pair's, rising H pixels (a point of it: its
           place along the edge, and its height above the floor; M pixels' grace round it) */
        const int H = 40, M = 3;
        int along, up, len;
        if (py == y && px != x) {                                          /* (the pair east or west: the edge runs north-south, down and to the left on the screen) */
            int ex = (x > px ? x : px) - ox, u = ex + 135 - fx;            /* (u: the edge's point under the pointer's column, map y - oy) */
            along = u + oy - y; len = h; up = u + 109 - gy;
        } else if (px == x && py != y) {                                   /* (north or south: the edge runs across the screen) */
            int ey = (y > py ? y : py) - oy, mx = fx - 135 + ox + ey;
            along = mx - x; len = w; up = ey + 109 - gy;
        } else continue;
        if (along < -M || along > len + M || up < -M || up > H + M) continue;
        int d = abs(along - len / 2) + abs(up - H / 2);
        if (d < bd) { bd = d; best = k; }
    }
    if (best < 0) return -1;
    /* the box round a door's picture takes in the floor before it: a point on floor the party can walk to
       (on its side of every doorway) is a walk there, the game's, not the door */
    int my = gy - 109 + oy, mx = fx - 135 + ox + (my - oy);
    int cx = mx >= 0 ? mx / 32 : -1, cy = my >= 0 ? my / 16 : -1;
    if (cx >= 0 && cx < 40 && cy >= 0 && cy < 40 && lw_cell(cx, cy) >= 0 && reach((s8)g_rb(GV_marching_order)) && !rdoor[cx][cy] && rdist[cx][cy] >= 0) return -1;
    return best;
}
int levelwalk_door_open(int n) { return n >= 0 && n < 40 && rb(G_FAR20A5, (u16)(0x33d3 + n)) != 0; }
/* the party into door n's doorway, from the side it is on (the game's Use Door is for a member standing in
   it), the door used there (goal_check); a doorway it cannot step into: the nearest floor before it */
int levelwalk_door(int n)
{
    int nd = (s8)rb(G_FAR20A5, 0x1f52), lead = (s8)g_rb(GV_marching_order);
    if (n < 0 || n >= nd || lead < 0 || lead > 4 || !reach(lead)) return 0;
    u16 r0 = DOOR_REC(n);
    int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
    float dx = x + w / 2.0f, dy = y + h / 2.0f;
    int bx = -1, by = -1, bdist = 1 << 30;
    for (int s = 0; s < 2; s++) {                                          /* the doorway's cells beside one the party reaches (and its pair's: */
        int dn = s ? door_partner(n) : n;                                  /*  of a double door one leaf may stand over the wall) */
        if (dn < 0 || dn >= nd) continue;
        u16 q = DOOR_REC(dn);
        int qx = (s16)rw(G_FAR20A5, q), qy = (s16)rw(G_FAR20A5, (u16)(q + 2)), qw = rb(G_FAR20A5, (u16)(q + 4)), qh = rb(G_FAR20A5, (u16)(q + 5));
        for (int cx = qx / 32; cx <= (qx + qw - 1) / 32; cx++)
            for (int cy = qy / 16; cy <= (qy + qh - 1) / 16; cy++) {
                if (qx < 0 || qy < 0 || cx < 0 || cx >= 40 || cy < 0 || cy >= 40 || lw_cell(cx, cy) < 0) continue;
                static const int D[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
                for (int k = 0; k < 4; k++) {
                    int nx = cx + D[k][0], ny = cy + D[k][1];
                    if (nx < 0 || nx >= 40 || ny < 0 || ny >= 40 || rdist[nx][ny] < 0 || rdist[nx][ny] >= bdist) continue;
                    bdist = rdist[nx][ny]; bx = cx; by = cy;
                }
            }
    }
    if (bx < 0) {                                                          /* (none: the floor before it) */
        float bd = 1e9f;
        for (int cx = (int)(dx / 32) - 2; cx <= (int)(dx / 32) + 2; cx++)
            for (int cy = (int)(dy / 16) - 2; cy <= (int)(dy / 16) + 2; cy++) {
                if (cx < 0 || cx >= 40 || cy < 0 || cy >= 40 || rdist[cx][cy] < 0) continue;
                float px = cx * 32 + 16.0f, py = cy * 16 + 8.0f;
                float d = (px - dx) * (px - dx) + 4 * (py - dy) * (py - dy);
                if (d < bd) { bd = d; bx = cx; by = cy; }
            }
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
