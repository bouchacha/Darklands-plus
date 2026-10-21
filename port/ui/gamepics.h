/* gamepics.h - the game's own pictures (PICS\*.PIC, unpacked by mods/hdart.c) as textures for
 * the native screens: a party member's figure or herald, the paper doll put together from its
 * layers, a saint's statue. Colour 0 is see-through, as the game draws them. A picture's
 * colours are its own palette where it brings one, then a named picture's (the palette the
 * game sets for that screen), then the game's palette of the moment (the fixed interface
 * colours). Made once and kept; nothing of the game's files is written anywhere. */
#ifndef DL_GAMEPICS_H
#define DL_GAMEPICS_H
#include <SDL.h>
#include <string>
#include <vector>

struct GamePic { SDL_Texture *tex = nullptr; int w = 0, h = 0; };

void gamepics_set_renderer(SDL_Renderer *r);
/* one picture; palpic: the picture whose palette applies (nullptr: the game's) */
const GamePic *gamepic(const std::string &dosname, const char *palpic = nullptr);
/* several pictures drawn one over the other at (0, 0) (the first one sets the size) */
const GamePic *gamepic_layers(const std::vector<std::string> &dosnames, const char *palpic = nullptr);
/* the layers over the first with the first itself left out (the doll without its niche) */
const GamePic *gamepic_layers_off_back(const std::vector<std::string> &dosnames, const char *palpic = nullptr);
/* a part of a picture (x, y, w, h) */
const GamePic *gamepic_part(const std::string &dosname, int x, int y, int w, int h, const char *palpic = nullptr);
/* a part with its greys left out (see-through), for a word painted on grey stone: the colours
   whose channels differ by more than grey_tol stay, and the dark greys (its outline) */
const GamePic *gamepic_cutout(const std::string &dosname, int x, int y, int w, int h, int grey_tol);
/* forget them all (the game's palette changed for good: another game loaded) */
void gamepics_flush(void);

#endif
