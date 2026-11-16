/* overland_zoom.h - the live travel view (overland_zoom.c) */
#ifndef DL_OVERLAND_ZOOM_H
#define DL_OVERLAND_ZOOM_H
#ifdef __cplusplus
extern "C" {
#endif
enum { OVERLAND_MAP_X = 60, OVERLAND_MAP_Y = 16 };
int overland_camera_at(int gx, int gy);            /* the live terrain can accept camera input here */
void overland_pan_by(float dx, float dy);          /* the pointer's movement in game pixels */
void overland_pan_recentre(void);
void overland_zoom_wheel(int gx, int gy, float notches);
int overland_zoom_click(int gx, int gy, int ctrl);
void overland_zoom_frame(int unobstructed);
void overland_zoom_draw(unsigned *rgb, const unsigned *palette);
int overland_zoom_active(void);                   /* the displayed terrain is zoomed or panned */
void overland_zoom_project(float *gx, float *gy);
void overland_zoom_view(int *col, int *row);       /* the left column and top row of the native view */
#ifdef __cplusplus
}
#endif
#endif
