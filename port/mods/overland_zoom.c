/* overland_zoom.c - zoom and pan the live terrain view. The original still draws
 * and walks its usual view. The frame contributes the terrain already on screen (including
 * the moving party), and the indexed world map supplies the land around it. */
#include <math.h>
#include <string.h>
#include "../runtime/config.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "overland_zoom.h"
#include "saves.h"
#include "travel.h"
#include "worldmap.h"

enum {
    VIEW_W = 320 - OVERLAND_MAP_X, VIEW_H = 200 - OVERLAND_MAP_Y,
    CENTRE_X = OVERLAND_MAP_X + VIEW_W / 2, CENTRE_Y = OVERLAND_MAP_Y + VIEW_H / 2,
    DEFAULT_LEVEL = 3
};
static const float scales[] = { 2.0f, 1.6f, 1.25f, 1.0f, 0.8f, 0.64f, 0.5f, 0.4f };
static float wheel_level = DEFAULT_LEVEL;
static int panned;
static float pan_x, pan_y;                              /* the fixed centre in world pixels */
static unsigned pan_save;
static int transformed;
static float drawn_scale = 1.0f;
static float source_centre_x = CENTRE_X, source_centre_y = CENTRE_Y;
static int view_col, view_row, ground;

static int in_view(int x, int y)
{
    return x >= OVERLAND_MAP_X && x < 320 && y >= OVERLAND_MAP_Y && y < 200;
}

static int available(void)
{
    return game_mode() == GM_MAP && travel_view_live() && !config_get_int("original", 0);
}

int overland_camera_at(int gx, int gy)
{
    return in_view(gx, gy) && available();
}

void overland_zoom_wheel(int gx, int gy, float notches)
{
    if (!overland_camera_at(gx, gy)) return;
    wheel_level = fminf(sizeof scales / sizeof scales[0] - 1, fmaxf(0, wheel_level - notches));
}

/* The native frame copies from (8,8) in its tile buffer to (60,16) on screen.
   The world's pixels for that buffer start at (16*view_col, 4*view_row). */
static int world_origin_x(void) { return view_col * 16 - 52; }
static int world_origin_y(void) { return view_row * 4 - 8; }

void overland_pan_by(float dx, float dy)
{
    if (!available()) return;
    if (!panned) {
        pan_x = world_origin_x() + source_centre_x;
        pan_y = world_origin_y() + source_centre_y;
        pan_save = saves_loaded_ms;
        panned = 1;
    }
    /* The terrain moves with the mouse, leaving the camera looking the other way. */
    pan_x -= dx / drawn_scale;
    pan_y -= dy / drawn_scale;
}

void overland_pan_recentre(void) { panned = 0; }

static float clamp_centre(float centre, int extent, float half_view)
{
    if (extent <= 2 * half_view) return extent * 0.5f;
    return fminf(extent - half_view, fmaxf(half_view, centre));
}

void overland_zoom_frame(int unobstructed)
{
    transformed = 0;
    drawn_scale = 1.0f;
    if (panned && (game_mode() != GM_MAP || saves_loaded_ms != pan_save || config_get_int("original", 0)))
        overland_pan_recentre();
    if (!unobstructed || !available()) return;

    view_col = (s16)g_rw(GV_map_view_col);
    int row = (s16)g_rw(GV_map_view_row);
    /* The native draw routine uses 1 when passed a zero column or buffer-relative row. */
    if (!view_col) view_col = 1;
    view_row = (s16)g_rw(GV_map_buf_row) + (row ? row : 1);
    source_centre_x = CENTRE_X;
    source_centre_y = CENTRE_Y;

    float scale = scales[(int)(wheel_level + 0.5f)];
    if (scale == 1.0f && !panned) return;
    ground = (u8)g_rw(GV_map_ground);
    if (!worldmap_prepare_terrain(ground)) {
        overland_pan_recentre();
        wheel_level = DEFAULT_LEVEL;
        return;
    }
    if (panned) {
        pan_x = clamp_centre(pan_x, worldmap.full_w, VIEW_W * 0.5f / scale);
        pan_y = clamp_centre(pan_y, worldmap.full_h, VIEW_H * 0.5f / scale);
        source_centre_x = pan_x - world_origin_x();
        source_centre_y = pan_y - world_origin_y();
    } else if (scale > 1.0f) {
        /* Follow the party's sprite in a close-up as it walks across the native view. */
        source_centre_x = (s16)g_rw(GV_map_marker_x) + 60;
        source_centre_y = (s16)g_rw(GV_map_marker_y) + 14;
    }
    drawn_scale = scale;
    transformed = 1;
}

int overland_zoom_active(void) { return transformed; }

void overland_zoom_project(float *gx, float *gy)
{
    if (!overland_zoom_active()) return;
    *gx = CENTRE_X + (*gx - source_centre_x) * drawn_scale;
    *gy = CENTRE_Y + (*gy - source_centre_y) * drawn_scale;
}

void overland_zoom_view(int *col, int *row)
{
    *col = overland_zoom_active() ? view_col : (s16)g_rw(GV_map_view_col);
    *row = overland_zoom_active() ? view_row : (s16)g_rw(GV_map_buf_row) + (s16)g_rw(GV_map_view_row);
}

static int source_pixel(int pixel, float view_centre, float source_centre)
{
    return (int)floorf(source_centre + ((pixel + 0.5f) - view_centre) / drawn_scale);
}

int overland_zoom_click(int gx, int gy, int ctrl)
{
    if (!overland_zoom_active() || !overland_camera_at(gx, gy)) return 0;
    int x = world_origin_x() + source_pixel(gx, CENTRE_X, source_centre_x);
    int y = world_origin_y() + source_pixel(gy, CENTRE_Y, source_centre_y);
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
    int origin_x = world_origin_x(), origin_y = world_origin_y();
    for (int x = 0; x < VIEW_W; x++) source_x[x] = source_pixel(x + OVERLAND_MAP_X, CENTRE_X, source_centre_x);

    for (int y = 0; y < VIEW_H; y++) {
        int sy = source_pixel(y + OVERLAND_MAP_Y, CENTRE_Y, source_centre_y);
        int wy = origin_y + sy;
        const u32 *native = sy >= OVERLAND_MAP_Y && sy < 200 ? rgb + sy * 320 : NULL;
        const u8 *terrain = wy >= 0 && wy < worldmap.full_h ? worldmap.idx + (size_t)wy * worldmap.full_w : NULL;
        for (int x = 0; x < VIEW_W; x++) {
            int sx = source_x[x], wx = origin_x + sx;
            if (native && sx >= OVERLAND_MAP_X && sx < 320) picture[y][x] = native[sx];
            else picture[y][x] = palette[terrain && wx >= 0 && wx < worldmap.full_w ? terrain[wx] : ground];
        }
    }
    for (int y = 0; y < VIEW_H; y++)
        memcpy(rgb + (y + OVERLAND_MAP_Y) * 320 + OVERLAND_MAP_X, picture[y], sizeof picture[y]);
}
