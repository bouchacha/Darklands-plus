// uiinternal.h - what the Darklands+ window's files share (ui.cpp: the window, its other pages and
// the frame; settings.cpp: the Mods, Helpers and Display pages; the pages with tabs of their own).
// C++ only.
#pragma once
#include <SDL.h>
#include "ui.h"
struct ImFont;

/* the modules' switches and the rest the pages read (declared here for both files) */
extern "C" int orders_click_attack;
extern "C" int idle_rest;                  /* runtime/bios.c: rest while the game only waits */
extern "C" { int marchview_is_on(void); void marchview_set(int on); }
extern "C" { int lodgeview_is_on(void); void lodgeview_set(int on); }
extern "C" { int enemyview_is_on(void); void enemyview_set(int on); }
extern "C" { int mapmsg_is_on(void); void mapmsg_set(int on); }
extern "C" { int infoview_is_on(void); void infoview_set(int on); }
extern "C" { int titleview_is_on(void); void titleview_set(int on); }
extern "C" { int rosterview_is_on(void); void rosterview_set(int on); }
extern "C" { int createview_is_on(void); void createview_set(int on); }
extern "C" { int prayview_is_on(void); void prayview_set(int on); }
extern "C" { int cacheview_is_on(void); void cacheview_set(int on); }
extern "C" { int savesview_is_on(void); void savesview_set(int on); }
extern "C" { int menuview_is_on(void); void menuview_set(int on); }
extern "C" { int alchemyview_showing(void); int alchemyview_is_on(void); void alchemyview_set(int on); }
extern "C" { int partypanel_is_on(void); void partypanel_set(int on); int partypanel_picture(void); void partypanel_set_picture(int v); void partypanel_draw(void); void partypanel_set_renderer(SDL_Renderer *r); void partypanel_init(void); }
extern "C" { int barterview_is_on(void); void barterview_set(int on); int barterview_showing(void); }
extern "C" { extern int battlescroll_on, battlescroll_rest; void battlescroll_set(int, int); extern int loot_return_thrown; extern int ammo_infinite; void ammo_set(int v); extern int thrown_infinite; void thrown_set(int v); }
extern int ui_spell_out;                   /* overlay.cpp: character sheet names in full */
extern "C" void rules_apply(void);
extern "C" void journal_set(int on);
extern "C" int journal_enabled(void);
extern "C" int plat_controller_connected(void);
extern "C" { int mpu_out_count(void); const char *mpu_out_name(int i); int mt32_families(void); int mt32_switch_pending(void); int mt32_switch(void); const char *mt32_status(void); void mt32_rescan(void); int mt32_dir_count(void); const char *mt32_dir(int); extern int sound_music_volume; }
extern ImFont *ui_font_game[UI_GAME_FONTS];
extern int ui_game_font, ui_text_edge;
extern const char *UI_GAME_FONT_NAMES[UI_GAME_FONTS];

extern int ui_dirty_config;              /* a setting changed: written when the window closes */
extern ImFont *ui_font_head;             /* the pages' headings */
extern ImFont *ui_font_bold;             /* the body's size, semibold: a switch's title */
extern int ui_sys_cursor;                /* display.system_cursor */
extern SDL_Window *ui_window;
void ui_overlay_font_update(void);       /* the typeface of the side panels and tooltips, after a change */
void ui_hint(const char *fmt, ...);      /* a line of explanation under a setting (greyed) */
/* a switch's words as one paragraph: the title in bold, a colon, what it does in grey, wrapped
   together; true when they are clicked */
bool ui_title_desc(const char *title, const char *desc, bool dim = false, const char *id = "##td");
int ui_original_mode(void);              /* "Play the original game" */
bool ui_modding(void);                   /* the editing tools shown (F12 > Settings > Helpers > Editor): the Editor tab, and changing weapons, items and foes in Records */
void ui_set_original(bool on);
void tab_mods(void);
void tab_helpers(void);
void tab_restored(void);
void tab_display(void);
bool ui_subtabs_begin(const char *id);   /* tabs inside a page: ImGui::BeginTabBar in the sub-tabs' face */
bool ui_subtab(const char *label, bool select = false);   /* ImGui::BeginTabItem in that face */

#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
