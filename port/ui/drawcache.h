/* drawcache.h - heavy ornament drawn once and kept (a plaited band is ~100,000 vertices a frame drawn
   afresh, 4 as a picture): by default a dense drawing is made into a picture by the processor (SDL's
   software renderer) and handed to the graphics card as an ordinary picture, a large sparse one is kept
   as its triangles (display.draw_cache, ui/drawcache.cpp) */
#pragma once
#include "imgui.h"
#include <functional>
/* draw `fn` (into the list it is handed, in window coordinates, inside lo..hi) as a kept picture.
   key: whatever else the drawing depends on (its size and place within lo..hi are added here).
   The drawing must be opaque where it isn't empty (its soft edges are blended onto nothing). Falls
   back to drawing it directly where the renderer can't draw into a picture (DL_NO_DRAWCACHE: always) */
void dc_draw(ImDrawList *dl, ImVec2 lo, ImVec2 hi, unsigned long long key, const std::function<void(ImDrawList *)> &fn);
void dc_frame_end(void);         /* (the pictures not used for a while let go) */
void dc_reset(void);             /* (the graphics card lost them: drawn again) */
static inline unsigned long long dc_key(unsigned long long h, double v) { long long q = (long long)(v * 64.0); return (h ^ (unsigned long long)q) * 1099511628211ull + 0x9e3779b97f4a7c15ull; }
