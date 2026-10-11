/* worldmap.h - the whole overland map as one picture (worldmap.c) */
#ifndef DL_WORLDMAP_H
#define DL_WORLDMAP_H
#ifdef __cplusplus
extern "C" {
#endif
#define WM_LEVELS 4
typedef struct {
    unsigned *px[WM_LEVELS];          /* ARGB; level 0 is half the travel screen's scale */
    int w[WM_LEVELS], h[WM_LEVELS];
    int full_w, full_h;               /* travel-screen pixels: tile (x, y) at x*16 (+8 odd rows), y*4, 16x12 */
    int tiles_w, tiles_h, month, stale, generation;
    unsigned char *idx;               /* full scale: palette indices, full_w x full_h */
    unsigned pal[256];                /* ARGB */
} WorldMap;
extern WorldMap worldmap;
int worldmap_build(void);
void worldmap_watch_palette(void);
#ifdef __cplusplus
}
#endif
#endif
