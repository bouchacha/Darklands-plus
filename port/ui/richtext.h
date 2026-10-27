// richtext.h - text with the attributes' and skills' shields in it, wherever Darklands+ writes about one
// (richtext.cpp). In a text:
//   {Strength}             Strength's shield alone, in place of the word (its name under the pointer)
//   {Strength:30 of 35}    the shield, then the value: where a value is given, changed or asked for
//   {Edged}{Impact} +15    shields side by side share the value after them, a bracket under the group
//   {@2} {@2:54}           a party member's head (2: the member's place in the party, 0..4), its value after it;
//                          the member's name under the pointer (rich_member writes one)
// With display.coins on, an amount of money in words ("3 groschen", "12pfs", "2 florins") is written with its
// coin instead of the word (money_words).
// A name is the character sheet's (ui_key_of: "Edged Weapons" or "Edged", "Speak Latin" or "Latin",
// "Reputation", "Fame", "Age"); anything else between braces is written as it stands. Every text the native
// screens (NsDraw's text, para: the name under the pointer), their tooltips (ns_tip), the margins'
// (margin_tip) and ui_rich_text write takes them; tools/check_docs.py checks the names.
#pragma once
#include "imgui.h"
#include <string>

bool rich_has(const char *s);                                   /* the text holds a shield, a head or (coins on) an amount in words */
enum { RICH_MEMBER = 64 };                                      /* a head's key: RICH_MEMBER + the member's place */
std::string rich_member(int m, const std::string &value = "");  /* "{@m}" or "{@m:value}" */
std::string rich_key_name(int key);                             /* the shield's name or the member's */
/* member m's head (the figure beside a battle: its head and shoulders), sz tall (as wide as the game's pixels
   make it: sz / 1.2) with its top left at p0 (ui/partypanel.cpp) */
bool ui_member_head(ImDrawList *dl, int m, ImVec2 p0, float sz, ImU32 tint = IM_COL32_WHITE);
std::string ui_member_name(int m);                              /* the member's short name (UTF-8) */
/* the size it takes in font f at size fs (pixels), wrapped at wrap pixels (0: only at '\n') */
ImVec2 rich_size(ImFont *f, float fs, float wrap, const char *s);
/* drawn at pos (its top left) in colour col; returns its size */
ImVec2 rich_draw(ImDrawList *dl, ImFont *f, float fs, ImVec2 pos, float wrap, ImU32 col, const char *s);
int rich_key_at(ImVec2 m);                                      /* the shield at m (pixels) of the last rich_draw, -1 none */
/* a shield's name for the pointer on it, shown at the frame's end (ui_rich_tip_flush) unless another tooltip
   was opened in the meantime (ui_rich_tip_taken: ns_tip, margin_tip) */
void ui_rich_tip_later(int key);
void ui_rich_tip_taken(void);
void ui_rich_tip_flush(void);
std::string rich_plain(const char *s);                          /* the words alone ({Strength:30} -> "Strength 30", {Strength} -> "Strength"), for a log */
/* a text written without braces with each attribute's and skill's name (capitalised, a whole word: "Speak Common",
   "Edged", "Endurance") given its shield, "{Name}": for the accounts written as a list of names (mods/saintinfo.c) */
std::string rich_auto(const char *s);
/* in the current ImGui window, as ImGui::TextUnformatted (wrapped where its text wrap position says) */
void ui_rich_text(const char *s);
void ui_rich_textf(const char *fmt, ...) IM_FMTARGS(1);
void ui_rich_textf_c(const ImVec4 &col, const char *fmt, ...) IM_FMTARGS(2);   /* in that colour (as ImGui::TextColored) */
void ui_rich_textf_dim(const char *fmt, ...) IM_FMTARGS(1);                    /* greyed (as ImGui::TextDisabled) */
