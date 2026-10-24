/* overlay_parts.h - what the overlay's files share (ui/overlay.cpp: notices, the card and shop
 * helpers, the loot buttons; ui/overlay_text.cpp: crisp text, the chances, the pointer) */
#pragma once
#include "imgui.h"
ImDrawList *ovl_list(void);                       /* the overlay layer: over the picture, under windows */
void text_edged(ImDrawList *dl, ImFont *f, float size, ImVec2 p, ImU32 c, const char *t, int edge, int bg = -1);
double rel_lum(int r, int g, int b);
ImU32 contrast_fix(ImU32 c, int bg);
bool faint_text(ImU32 c, int bg);
ImVec2 g2w(float x, float y);                     /* game pixels to window pixels */
void draw_chances(void);                          /* the chances beside the original cards' options */
extern ImVec2 ptr_lo, ptr_hi; extern int ptr_frame;   /* where the pointer was drawn (window pixels), the frame */
