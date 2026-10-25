/* margins_parts.h - what the margins' files share (ui/margins.cpp: the margins, the town and the
 * quests; ui/margins_battle.cpp: the battle's side, the order buttons) */
#pragma once
#include <string>
#include <functional>
#include "imgui.h"
std::string member_name(int m);
bool item_hovered(void);
void margin_tip(const char *text);
void heading(const char *t);
void name_line(const std::string &t, bool dim, const char *tail = nullptr);
void bar(float frac, ImU32 col, float w, float hgt);
void battle_panel(float width, float fs);         /* the right margin in a battle */
void order_bar(float x0, float x1, float y1, bool with_resume);   /* the order buttons at the foot of the field (Resume too when no right margin holds it) */
int margins_order_click(int x, int y);            /* a click on an order button: 1 taken */
void mclick(ImVec2 a, ImVec2 b, std::function<void()> f);   /* a place in a margin that takes a click */
extern "C" float ui_orderbar_top;                  /* the order strip's top while shown, else 0 */
extern "C" int partypanel_shown(void);
std::string battlelog_name(int i);
bool battlelog_met(int i);
