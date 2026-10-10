#ifndef DL_UI_H
#define DL_UI_H
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { int aspect, smooth, integer; } UiSettings;
extern UiSettings ui_settings;
void ui_init(SDL_Window *win, SDL_Renderer *ren);
int ui_event(const SDL_Event *e);      /* 1: the event was for the interface */
int ui_is_open(void);
void ui_toggle(void);
void ui_draw(SDL_Renderer *ren);
void ui_select_tab(int t);
#ifdef __cplusplus
}
#endif
#endif
