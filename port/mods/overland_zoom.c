/* overland_zoom.c - zoom the live terrain view around its centre. The original still draws
 * and walks its usual view. The frame contributes the terrain already on screen (including
 * the moving party), and the indexed world map supplies the land around it. */
#include <math.h>
#include <string.h>
#include "../runtime/config.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "overland_zoom.h"
#include "travel.h"
#include "worldmap.h"

enum { VIEW_W = 320 - OVERLAND_MAP_X, VIEW_H = 200 - OVERLAND_MAP_Y };
static const float centre_x = OVERLAND_MAP_X + VIEW_W * 0.5f;
static const float centre_y = OVERLAND_MAP_Y + VIEW_H * 0.5f;
static const float scales[] = { 1.0f, 0.8f, 0.64f, 0.5f };
static float wheel_level;
static float drawn_scale = 1.0f;
static int view_col, view_row, ground;

static int in_view(int x, int y)
{
    return x >= OVERLAND_MAP_X && x < 320 && y >= OVERLAND_MAP_Y && y < 200;
}

static int available(void)
{
    return travel_view_live() && !config_get_int("original", 0);
}

void overland_zoom_wheel(int gx, int gy, float notches)
{
    if (!in_view(gx, gy) || !available()) return;
    wheel_level = fminf(sizeof scales / sizeof scales[0] - 1, fmaxf(0, wheel_level - notches));
}

void overland_zoom_frame(int unobstructed)
{
    drawn_scale = 1.0f;
    int level = (int)(wheel_level + 0.5f);
    if (!level || !unobstructed || !available()) return;

    ground = (u8)g_rw(GV_map_ground);
    if (!worldmap_prepare_terrain(ground)) { wheel_level = 0; return; }
    view_col = (s16)g_rw(GV_map_view_col);
    int row = (s16)g_rw(GV_map_view_row);
    /* The native draw routine uses 1 when passed a zero column or buffer-relative row. */
    if (!view_col) view_col = 1;
    view_row = (s16)g_rw(GV_map_buf_row) + (row ? row : 1);
    drawn_scale = scales[level];
}

int overland_zoom_active(void) { return drawn_scale != 1.0f; }

void overland_zoom_project(float *gx, float *gy)
{
    if (!overland_zoom_active()) return;
    *gx = centre_x + (*gx - centre_x) * drawn_scale;
    *gy = centre_y + (*gy - centre_y) * drawn_scale;
}

void overland_zoom_view(int *col, int *row)
{
    *col = overland_zoom_active() ? view_col : (s16)g_rw(GV_map_view_col);
    *row = overland_zoom_active() ? view_row : (s16)g_rw(GV_map_buf_row) + (s16)g_rw(GV_map_view_row);
}

static int source_pixel(int pixel, float centre)
{
    return (int)floorf(centre + ((pixel + 0.5f) - centre) / drawn_scale);
}

/* The native frame copies from (8,8) in its tile buffer to (60,16) on screen.
   The world's pixels for that buffer start at (16*view_col, 4*view_row). */
static int world_x(int gx) { return gx + view_col * 16 - 52; }
static int world_y(int gy) { return gy + view_row * 4 - 8; }

int overland_zoom_click(int gx, int gy, int ctrl)
{
    if (!overland_zoom_active() || !in_view(gx, gy) || !available()) return 0;
    int x = world_x(source_pixel(gx, centre_x)), y = world_y(source_pixel(gy, centre_y));
    if (x >= 0 && x < worldmap.full_w && y >= 0 && y < worldmap.full_h) {
        if (ctrl) travel_route_to_pixel(x, y);
        else travel_walk_to_pixel(x, y);
    }
    return 1;
}

void overland_zoom_draw(u32 *rgb, const u32 *palette)
{
    if (!overland_zoom_active()) return;

    static u32 picture[VIEW_H][VIEW_W];
    int source_x[VIEW_W];
    for (int x = 0; x < VIEW_W; x++) source_x[x] = source_pixel(x + OVERLAND_MAP_X, centre_x);

    for (int y = 0; y < VIEW_H; y++) {
        int sy = source_pixel(y + OVERLAND_MAP_Y, centre_y);
        int wy = world_y(sy);
        const u32 *native = sy >= OVERLAND_MAP_Y && sy < 200 ? rgb + sy * 320 : NULL;
        const u8 *terrain = wy >= 0 && wy < worldmap.full_h ? worldmap.idx + (size_t)wy * worldmap.full_w : NULL;
        for (int x = 0; x < VIEW_W; x++) {
            int sx = source_x[x], wx = world_x(sx);
            if (native && sx >= OVERLAND_MAP_X && sx < 320) picture[y][x] = native[sx];
            else picture[y][x] = palette[terrain && wx >= 0 && wx < worldmap.full_w ? terrain[wx] : ground];
        }
    }
    for (int y = 0; y < VIEW_H; y++)
        memcpy(rgb + (y + OVERLAND_MAP_Y) * 320 + OVERLAND_MAP_X, picture[y], sizeof picture[y]);
}
