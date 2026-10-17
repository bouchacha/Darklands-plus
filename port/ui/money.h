// money.h - amounts of money as Darklands+ writes them (ui/money.cpp): florins, groschen and
// pfennigs (1 florin = 20 groschen = 240 pfennigs), each figure followed by a small gold, silver
// or copper coin (glyphs U+E000..E002 in every font, painted into the font atlas), or as
// "1fl 12gr 4pf" with display.coins off.
#pragma once
#include <string>
extern "C++" {                     /* (included from extern "C" blocks too) */

#define COIN_FL "\xee\x80\x80"      /* U+E000: a gold coin (florins) */
#define COIN_GR "\xee\x80\x81"      /* U+E001: silver (groschen) */
#define COIN_PF "\xee\x80\x82"      /* U+E002: copper (pfennigs) */

bool money_coins(void);                          // display.coins (on by default)
std::string money_str(long pf);                  // "1● 12● 4●" (coins) or "1fl 12gr 4pf"; 0: "0" + copper
std::string money_fgp(long fl, long gr, long pf); // the same for an amount kept in three parts (a purse)
std::string money_with_total(long pf);           // money_str, and "(105 pf)" after it from a groschen up
std::string money_total(long pf);                // "388 pfennigs in all" (for a tooltip or a strip)
void money_tooltip(long pf);                     // that, as a tooltip, when the last ImGui item is hovered and the amount has two parts or more

struct ImFontAtlas;
void money_glyphs_add(ImFontAtlas *atlas);       // before the atlas is built: room for the coins in every font
void money_glyphs_paint(ImFontAtlas *atlas);     // after: the coins painted, their place on the line set
}
