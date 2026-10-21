/* ornament.h - the period ornaments of the interface (display.ornate), drawn after the original's
 * own: the plaited copper band round its cards, gilt plaques with rivets (the Hall of Fame, the
 * party sheet's name plates), the illuminated capitals' red and gold, parchment. */
#pragma once
#include "imgui.h"

bool orn_on(void);                                         /* display.ornate */
int orn_style(void);                                       /* 0 off, 1 plaited copper, 2 walnut and brass, 3 illuminated, 4 gothic stone */
const char *orn_style_name(int s);
/* the chosen style's band round a..b, w wide inside it */
void orn_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w);
void orn_frame_style(ImDrawList *dl, ImVec2 a, ImVec2 b, float w, int style);
/* a small window in style s (a heading, lines, a button), for the settings' choice */
void orn_preview(ImDrawList *dl, ImVec2 a, ImVec2 b, int style, ImFont *f);
/* a plaited band (two copper strands, over and under) round the rectangle a..b, w wide inside it;
   a gilt lozenge at each corner */
void orn_braid_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float w);
/* a double gilt rule round a..b (s: its scale, about the line spacing in pixels), corner lozenges */
void orn_gilt_frame(ImDrawList *dl, ImVec2 a, ImVec2 b, float s);
/* a straight gilt rule from x0 to x1 at y: two lines, a lozenge in the middle, dots at the ends */
void orn_rule(ImDrawList *dl, float x0, float x1, float y, float s);
/* a gilt rivet (a boss with a highlight) */
void orn_rivet(ImDrawList *dl, ImVec2 c, float r);
/* a plaque: brass by default, else `base` lit like metal; hover brightens it. Rivets at its ends */
void orn_plaque(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets = true);
/* parchment shading: the edges of a..b darkened and foxed, as old paper (drawn over a plain fill) */
void orn_parchment_edges(ImDrawList *dl, ImVec2 a, ImVec2 b, float w);
/* ImGui::SeparatorText dressed: a lozenge before the words, a double gilt rule after */
void orn_septext(const char *label);
/* a hairline rule from a to b (across or down): two gilt lines g apart; across, a lozenge at its end */
void orn_hairline(ImDrawList *dl, ImVec2 a, ImVec2 b, float g, bool across);
/* under the chosen tab: a gilt line with a small lozenge in the middle (k: pixels per game row) */
void orn_tab_mark(ImDrawList *dl, ImVec2 a, ImVec2 b, float k);
/* a frame or a button in the given style (display.frame_style, display.button_style; see ornament.cpp) */
void orn_frame_style_box(ImDrawList *dl, ImVec2 a, ImVec2 b, float s, int style);
void orn_button_style(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 base, bool hover, bool rivets, int style);
int orn_button_lettered(void);                             /* buttons as words: their labels in ink, not light */
/* the interface's windows: a plaited band round each (tooltips a gilt rule); before ImGui::Render */
void orn_windows(void);
