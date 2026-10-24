// settings_parts.h - the Settings page's sections kept in files of their own (ui/settings.cpp
// draws the page and calls them): what things look like (ui/settings_look.cpp) and the setup of
// the game on this computer (ui/settings_setup.cpp).
#pragma once
#include <functional>
#include "imgui.h"
/* a row of pictures to choose from: n cards, each drawn by draw(dl, a, b, value), its label under it; a click sets the config key */
void pic_choice(const char *id, const char *key, int def, const int *vals, const char *const *labels, int n, float cw, float ph,
                const std::function<void(ImDrawList *, ImVec2, ImVec2, int)> &draw);
void look_text(void);        /* Text: the original lettering or the new text, the typeface, the coins */
void look_ornaments(void);   /* Frames and clock: the band around the windows, the frames, the buttons */
void look_clock(void);       /*   the clock while hours pass */
void display_icons(void);    /*   the potions' grade marks */
void setup_screen(void);     /* Screen: the three ways to scale, the margins, sizes, the pointer, the window */
void setup_intro(void);      /* Intro */
void setup_sound(void);      /* Sound: the two previews, the devices, the volume, custom sound packs */
void controller_prefs(void); /* Controller: which one, its buttons, their jobs */
extern int settings_pack_preview;   /* a pack previewed (ui/ui.cpp lets the music through) */
