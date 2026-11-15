#ifndef DL_PORTRAITS_H
#define DL_PORTRAITS_H
#include "gamepics.h"
#include "imgui.h"

constexpr float PORTRAIT_ASPECT = 0.45f;

void portraits_set_renderer(SDL_Renderer *renderer);
const GamePic *member_portrait(const char *short_name, const char *full_name);
void draw_portrait(ImDrawList *draw, const GamePic &pic, ImVec2 a, ImVec2 b, float rounding);

void portrait_picker_open(const char *short_name, const char *full_name);
bool portrait_picker_is_open(void);
void portrait_picker_window(void);

#endif
