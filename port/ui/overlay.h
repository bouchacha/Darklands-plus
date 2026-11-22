// overlay.h - what the overlay's files share (overlay.cpp draws the layer over the running game and
// calls the others; battleoverlay.cpp: the battle's bars, the pointer's info, the fast-forward badge).
#pragma once
#include "imgui.h"
#include <string>
#include <vector>

#include "layers.h"                       /* (where each thing is drawn: ui_tier_list) */
ImVec2 ui_g2w(float gx, float gy);        /* a game pixel (320x200) in window pixels */
void ui_draw_battle_bars(void);
void ui_draw_battle_info(void);
void ui_draw_battle_fast(void);

/* the stock to keep: a town sells what the party keeps a stock of and has fewer of (ui/journal.cpp), shown as
   the learned saint's box (overlay.cpp) */
struct StockShort { int id, have, want; std::string shop; };
void ui_stock_popup(const std::string &town, const std::vector<StockShort> &rows);
