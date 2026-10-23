/* losaim.c - battle.los_line: while a member is about to shoot or throw at a foe (the pointer on
 * it, order mode), the line the game tests for a clear shot and what stands across it
 * (ui/battleoverlay.cpp draws them). The test is the game's own, line_blocked (s014 1d69:000c),
 * done again here box by box so the boxes in the way can be shown: every map cell (32 x 16 pixels)
 * in the rectangle the line spans, its four walls and the furniture standing in it when the
 * furniture's id is at least D_c1cf (on a forest map the trees, on a street the stalls...), then
 * every other combatant, its box shrunk by the shooter's height. Each box is clipped against the
 * line the game's way (one clip step, 1d69:0354), so a box counts exactly when it counts for the
 * game. With battle.shoot_past_allies on, the party's shots pass the people between (battlerules.c). */
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "losaim.h"

static int bx0, bx1, by0, by1;                 /* the box (D_e1d5, D_e1d7, D_e1d9, D_e1db) */
static int lx0, ly0, lx1, ly1;                 /* the line (los_x0 ...) */

static int outcode(int x, int y)               /* 1d69:04b2 */
{
    int c = 0;
    if (bx0 > x) c = 1;
    if (x > bx1) c |= 2;
    if (y < by0) c |= 8;
    if (y > by1) c |= 4;
    return c;
}
static int crosses(void)                       /* 1d69:0354, its 16-bit arithmetic */
{
    int c0 = outcode(lx0, ly0), c1 = outcode(lx1, ly1);
    if (c0 & c1) return 0;
    if (c0 == 0 || c1 == 0) return 1;
    if ((c1 | c0) == 12 || (c1 | c0) == 3) return 1;
    int px = 0, py = 0;
    if (c0 & 4) {
        if (ly1 == ly0) return 0;
        px = (s16)((long)(lx1 - lx0) * (by1 - ly1) / (ly1 - ly0) + lx1); py = by1;
    } else if (c0 & 8) {
        if (ly1 == ly0) return 0;
        px = (s16)((long)(by0 - ly1) * (lx1 - lx0) / (ly1 - ly0) + lx1); py = by0;
    }
    if (outcode(px, py)) {
        if (c0 & 2) {
            if (lx1 == lx0) return 0;
            py = (s16)((long)(bx1 - lx1) * (ly1 - ly0) / (lx1 - lx0) + ly1); px = bx1;
        } else if (c0 & 1) {
            if (lx1 == lx0) return 0;
            py = (s16)((long)(ly1 - ly0) * (bx0 - lx1) / (lx1 - lx0) + ly1); px = bx0;
        }
    }
    return outcode(px, py) == 0;
}

static int add(LosBox *out, int n, int max, int kind, int id)
{
    if (!crosses() || n >= max) return n;
    out[n].x0 = bx0; out[n].y0 = by0; out[n].x1 = bx1; out[n].y1 = by1; out[n].kind = kind; out[n].id = id;
    return n + 1;
}

int los_trace(int a, int b, int mode, LosBox *out, int max)
{
    if (a < 0 || a >= 13 || b < 0 || b >= 13) return 0;
    lx0 = (s16)g_rw((u16)(GV_combatants + a * 0x80 + 4)); ly0 = (s16)g_rw((u16)(GV_combatants + a * 0x80 + 6));
    lx1 = (s16)g_rw((u16)(GV_combatants + b * 0x80 + 4)); ly1 = (s16)g_rw((u16)(GV_combatants + b * 0x80 + 6));
    int n = 0;
    int y1 = ly1 / 16, y0 = ly0 / 16, x1 = lx1 / 32, x0 = lx0 / 32;
    int dy = abs(y0 - y1), dx = abs(x0 - x1), xmin = x1 > x0 ? x0 : x1, ymin = y1 > y0 ? y0 : y1;
    int first = (s8)g_rb(0xc1cf);                     /* D_c1cf */
    for (int col = xmin; col <= dx + xmin; col++) {
        int xp = col * 32 - 1;
        for (int row = ymin; row <= ymin + dy; row++) {
            int yp = row * 16 - 1;
            u16 k = (u16)((col * 40 + row) * 4);
            if (col < 0 || row < 0 || row >= 40 || col >= 100) continue;
            u8 b0 = rb(G_FAR20A5, k), b1 = rb(G_FAR20A5, (u16)(k + 1));
            u16 w2 = (u16)(rb(G_FAR20A5, (u16)(k + 2)) | rb(G_FAR20A5, (u16)(k + 3)) << 8);
            if ((w2 & 0xf) > 0 && (w2 & 0xf) < 12) { bx0 = xp; bx1 = xp + 0x21; by0 = yp; by1 = yp + 2; n = add(out, n, max, LOS_WALL, 0); }
            if ((w2 & 0xf00) > 0 && (w2 & 0xf00) < 0xc00) { by0 = yp + 0xf; bx0 = xp; bx1 = xp + 0x21; by1 = yp + 0x11; n = add(out, n, max, LOS_WALL, 0); }
            if ((b0 & 0xf) != 0 && (b0 & 0xf) < 12) { bx0 = xp + 0x1f; bx1 = xp + 0x21; by0 = yp; by1 = yp + 0x11; n = add(out, n, max, LOS_WALL, 0); }
            if ((b1 & 0xf) != 0 && (b1 & 0xf) < 12) { bx0 = xp; bx1 = xp + 2; by0 = yp; by1 = yp + 0x11; n = add(out, n, max, LOS_WALL, 0); }
            int f = (b0 >> 4) & 0xf;
            if (f >= first) {
                int o = (w2 >> 12) & 3;
                int ox = (s8)g_rb((u16)(0xd7be + f * 4 + o)), oy = (s8)g_rb((u16)(0xd802 + f * 4 + o));
                bx0 = ox + xp + 1; by0 = oy + yp + 1;
                bx1 = ox + (s8)g_rb((u16)(0xc12f + f)) + xp + 1; by1 = oy + (s8)g_rb((u16)(0xc11d + f)) + yp + 1;
                n = add(out, n, max, LOS_THING, f);
            }
        }
    }
    int h = mode == 0x100 ? (s8)g_rb((u16)(GV_combatants + a * 0x80 + 0x6f)) : (s8)g_rb((u16)(GV_combatants + a * 0x80 + 0x59));
    if (h >= 90) return n;
    /* battle.shoot_past_allies (mods/battlerules.c): a member's shot passes everyone but the target */
    if (a < 5 && config_get_int("battle.shoot_past_allies", 1) && !config_get_int("original", 0)) return n;
    for (int i = 0; i < 13; i++) {
        u16 c = (u16)(GV_combatants + i * 0x80);
        if (g_rb((u16)(c + 0x14)) != 1 || g_rb((u16)(GV_cbt_floor + i)) != g_rb(GV_floor_shown) || i == a || i == b) continue;
        int hw = (s8)g_rb((u16)(GV_cbt_half_w + i)), hh = (s8)g_rb((u16)(GV_cbt_half_h + i));
        int ax = hw + hw * h / -90, ay = hh + hh * h / -90;
        int cx = (s16)g_rw((u16)(c + 4)), cy = (s16)g_rw((u16)(c + 6));
        bx0 = cx - ax; by0 = cy - ay; bx1 = ax + cx; by1 = ay + cy;
        n = add(out, n, max, LOS_PERSON, i);
    }
    return n;
}
