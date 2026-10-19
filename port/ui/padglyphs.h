// padglyphs.h - a game controller's buttons drawn on screen (button prompts): while the controller
// is the input in use (runtime/platform_sdl.c plat_prompts), the hints and the buttons of the
// native screens show the button that does it, in the look of the controller's kind (Xbox,
// PlayStation, Nintendo). Drawn with ImGui's shapes: no pictures from anyone's trademark.
#ifndef DL_PADGLYPHS_H
#define DL_PADGLYPHS_H
#include "imgui.h"
#include <string>

/* the controller's buttons as Darklands+ maps them (runtime/platform_sdl.c pad_event) */
enum PadButton {
    PB_A,        /* a click (left button) */
    PB_B,        /* a right click (the menus) */
    PB_X,        /* Enter */
    PB_Y,        /* Esc */
    PB_LB,       /* Tab (the level map) */
    PB_RB,       /* Space (pause) */
    PB_START,    /* F12 (the Darklands+ window) */
    PB_BACK,     /* M (the world map) */
    PB_LS,       /* the left stick pressed: F1 */
    PB_RT,       /* the right trigger held: Ctrl */
    PB_LSTICK,   /* the left stick: the pointer */
    PB_RSTICK,   /* the right stick: the wheel (pressed: Z, fast forward in a battle) */
    PB_DPAD,     /* the arrow keys */
    PB_COUNT
};

extern "C" int plat_prompts(void);            /* 0 keys and mouse; 1 Xbox, 2 PlayStation, 3 Nintendo */
inline bool ui_pad_prompts() { return plat_prompts() != 0; }

/* the button b, about h pixels tall, centred on c; returns its width */
float ui_pad_glyph(ImDrawList *dl, ImVec2 c, float h, int b);
float ui_pad_glyph_width(float h, int b);
/* the button's name in words ("A", "Cross", "Menu" ...), for tooltips and plain text */
const char *ui_pad_name(int b);
/* ImGui: the button as an item on the line (for SameLine with text after it) */
void ui_pad_inline(int b);
/* ImGui: text in which "{A}", "{B}", "{X}", "{Y}", "{LB}", "{RB}", "{START}", "{BACK}", "{LS}",
   "{RT}", "{LSTICK}", "{RSTICK}", "{DPAD}" are drawn as the buttons, wrapped at the window's
   wrap width; disabled: in the dim colour */
void ui_pad_text(const char *text, bool disabled = false);

#endif
