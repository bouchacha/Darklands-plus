/* worldmap_draw.h - the world map's pictures and its places, shared by the map window
 * (ui/worldmap.cpp), the travel map's labels and the test sheet (ui/worldmap_draw.cpp). */
#pragma once
#include <string>
#include <vector>
#include "imgui.h"

struct Place { int loc, kind, x, y; std::string name; };
enum IconKind { IK_TOWN, IK_VILLAGE, IK_CASTLE, IK_MONASTERY, IK_SHRINE, IK_CAVE, IK_MINE, IK_WATER, IK_TOMB, IK_STONES, IK_RUIN, IK_OTHER, IK_N };
extern const char *IK_LABEL[IK_N];

std::vector<Place> places(void);                       /* every place the map shows */
const char *loc_kind_label(int k);                     /* a location's type (its record's +0) in words */
const char *kind_label(int k);
std::string kind_line(const std::vector<Place> &P, const Place &p);
int icon_kind(int k);                                  /* a location type's picture (IconKind) */
int kind_group(int k);                                 /* 0 town 1 village 2 castle 3 holy 4 other */
int kind_group_of_icon(int ik);
ImU32 kind_colour(int g);
void draw_icon_painted(ImDrawList *dl, ImVec2 c, float s, int ik);
void draw_icon_seal(ImDrawList *dl, ImVec2 c, float s, int ik);
void draw_icon(ImDrawList *dl, ImVec2 c, float s, int ik);   /* a place's picture (the seals) */
void quest_glow(ImDrawList *dl, ImVec2 c, float is);   /* the halo behind a quest's picture */
std::string hhmm(int minutes);                         /* "40 minutes", "22 hours", "1.4 days" */
float aspect(void);
void tile_px(int x, int y, float *px, float *py);      /* a tile's middle, in travel-screen pixels */
float map_border_px(void);
void compass_rose(ImDrawList *dl, ImVec2 c, float r);
void draw_map_frame(ImDrawList *dl, ImVec2 a, ImVec2 b);
void boat_way_draw(ImDrawList *dl, const std::vector<ImVec2> &pts, float lw, bool lit, int al, float t, bool both_ways = false);
void layer_glyph(ImDrawList *dl, ImVec2 c, float s, int which, ImU32 ink);
void layer_struck(ImDrawList *dl, ImVec2 c, float s);
