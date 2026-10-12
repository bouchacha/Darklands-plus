#ifndef DL_UI_H
#define DL_UI_H
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { int aspect, smooth, integer, scaler; } UiSettings;   /* scaler: 0 none, 1 2x, 2 4x (edge smoothing) */
extern UiSettings ui_settings;
void ui_init(SDL_Window *win, SDL_Renderer *ren);
int ui_event(const SDL_Event *e);      /* 1: the event was for the interface */
int ui_is_open(void);
void ui_toggle(void);
void ui_draw(SDL_Renderer *ren);
void ui_select_tab(int t);
#define UI_GAME_FONTS 7
extern float ui_scale;                  /* the menus' size: 1 at 96 dpi (display.ui_scale, or the system's scale on Windows) */
#define UIS(x) ((x) * ui_scale)
float ui_text_px(void);                 /* text size beside and over the picture (side panels, tooltips, map names) */
#ifdef __cplusplus
}
#endif
#endif
