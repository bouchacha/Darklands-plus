/* ui_decls.h - the other parts of the interface and of the game side that ui/ui.cpp and
 * ui/ui_windows.cpp call (declarations only) */
#pragma once
extern "C" int saves_loading_cover(void);   /* mods/saves.c */
void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);   /* ui/hourglass.cpp */
extern "C" void banner_draw(void);                                   /* ui/banner.cpp */
extern "C" void worldmap_icon_sheet_draw(void); extern "C" void orn_test_sheet_draw(void); extern "C" void niche_test_sheet_draw(void); extern "C" void clock_test_sheet_draw(void);                      /* ui/worldmap.cpp (a test) */
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[], FONT_GRENZE[], FONT_COMIC[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE, FONT_COMIC_SIZE;
extern "C" { int mt32_available(void); int mt32_is_open(void); void plat_restart(void); }
void welcome_window(void);
extern "C" { void tips_init(void); int tips_open(void); void ui_open_tab(int t); }
void tips_frame(void);
bool tips_never_box(bool *never);       /* ui/tips.cpp: "Never show again", from the fourth start */
extern "C" int orders_click_attack;
void help_window(void);
void tab_about(void); void mt32_window(void);
extern "C" int citynav_window(void);
extern "C" int citynav_available(void);
extern "C" void citynav_draw_cover(void);
extern "C" void ui_icon_styles_draw(void);   /* ui/skillicons.cpp (test) */   /* ui/citynav.cpp: Go to under way, the streets passed not shown */
extern "C" void citynav_tick(void);
extern "C" void citynav_go(int dest);
extern "C" int worldmap_open(void);
extern "C" void worldmap_draw(void);
extern "C" int worldmap_find_escape(int typing);
void worldmap_set_renderer(SDL_Renderer *r);
extern int ui_want_subtab;
extern "C" int idle_rest;                  /* runtime/bios.c: rest while the game only waits */
extern "C" int halloffame_is_open(void);
extern "C" void halloffame_open(void);
extern "C" void halloffame_draw(void);
extern "C" void cardview_draw(void);
extern "C" void barterview_draw(void);
extern "C" void sheetview_draw(void);
extern "C" void lootview_draw(void);
extern "C" void menuview_draw(void);
extern "C" void savesview_draw(void);
extern "C" void cacheview_draw(void);
extern "C" void prayview_draw(void);
extern "C" void createview_draw(void);
extern "C" void rosterview_draw(void);
extern "C" void infoview_draw(void);
extern "C" void titleview_draw(void);
extern "C" void marchview_draw(void);
extern "C" { int marchview_is_on(void); void marchview_set(int on); }
extern "C" void lodgeview_draw(void);
extern "C" int lodgeview_showing(void), lodgeview_camping(void);
extern "C" int savesview_showing(int *loading);
extern "C" { int lodgeview_is_on(void); void lodgeview_set(int on); }
extern "C" void enemyview_draw(void);
extern "C" { int enemyview_is_on(void); void enemyview_set(int on); }
extern "C" void mapmsg_draw(void);
extern "C" void choose_draw(void);
extern "C" { int mapmsg_is_on(void); void mapmsg_set(int on); }
extern "C" { int infoview_is_on(void); void infoview_set(int on); }
extern "C" { int rosterview_is_on(void); void rosterview_set(int on); }
extern "C" { int createview_is_on(void); void createview_set(int on); }
extern "C" { int prayview_is_on(void); void prayview_set(int on); }
extern "C" { int cacheview_is_on(void); void cacheview_set(int on); }
extern "C" { int savesview_is_on(void); void savesview_set(int on); }
extern "C" { int menuview_is_on(void); void menuview_set(int on); }
extern "C" void ns_defer_frame_end(int game_held);
extern "C" void alchemyview_draw(void);
extern "C" { int alchemyview_showing(void); int alchemyview_is_on(void); void alchemyview_set(int on); }
extern "C" { int partypanel_is_on(void); void partypanel_set(int on); int partypanel_picture(void); void partypanel_set_picture(int v); void partypanel_draw(void); void partypanel_set_renderer(SDL_Renderer *r); void partypanel_init(void); }
void gamepics_set_renderer(SDL_Renderer *r);
extern "C" { int lootview_showing(void); int lootview_is_on(void); void lootview_set(int on); }
extern "C" { int sheetview_showing(void); int sheetview_is_on(void); void sheetview_set(int on); }
extern "C" { int barterview_is_on(void); void barterview_set(int on); int barterview_showing(void); }
extern "C" { extern int battlescroll_on, battlescroll_rest; void battlescroll_set(int, int); extern int loot_return_thrown; extern int ammo_infinite; void ammo_set(int v); extern int thrown_infinite; void thrown_set(int v); }
extern "C" void halloffame_close(void);
extern "C" int halloffame_record(void);
void halloffame_set_renderer(SDL_Renderer *r);
void figures_set_renderer(SDL_Renderer *r);
extern "C" int figpick_is_open(void);
extern "C" void figpick_window(void);
extern int ui_spell_out;                   /* overlay.cpp: character sheet names in full */
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
extern "C" void ui_overlay(void);
extern "C" void hdtext_draw(void);
extern "C" void hdtext_draw_cursor(void);
extern "C" void hdtext_pointer_off(void);
extern "C" void hdtext_pointer_frame_end(void);
void ui_guide_tab(void);
extern "C" void tab_journal(void);
extern "C" void tab_saves(void);
extern "C" void ui_draw_notice(void);
extern "C" { int cacheview_showing(void); int cacheview_shared_pack(void); int cacheview_every_inn(void); }
extern "C" void ui_notice(const char *msg);
extern "C" void rules_apply(void);
extern "C" void ui_notice_for(const char *msg, unsigned ms);
extern "C" void tab_editor(void);
extern "C" void journal_init(void);
extern "C" void journal_set(int on);
extern "C" int journal_enabled(void);
extern "C" void partypanel_init(void);
extern "C" int levelmap_is_open(void);
extern "C" int levelmap_available(void);
extern "C" void levelmap_toggle(void);
extern "C" void levelmap_frame(void);
extern "C" void levelmap_draw(void);
extern "C" int ui_alert_open(void);
void ui_alert_draw(void);
extern "C" int overlay_click(int x, int y), overlay_click_up(int x, int y);
extern "C" int ns_showing(void);
extern "C" int margins_click(int x, int y);           /* ui/margins.cpp: the battle's order buttons */
extern "C" int partypanel_click(int x, int y);        /* ui/partypanel.cpp: shoot or melee on a member's box; in a battle the box chooses them */
extern "C" int cardview_chip_click(int x, int y);    /* ui/cardview.cpp: who speaks for the party, on an option where the player picks */
extern "C" int partypanel_rclick(int x, int y);       /* ui/partypanel.cpp: in a battle, the sheet */
extern "C" int plat_win_outside(int wx, int wy);
extern "C" { extern int loot_active, barter_active; int loot_enabled(void); }
extern "C" int plat_controller_connected(void);
void rules_tab(void);
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
extern "C" { extern int hdtext_live_pointer; }
extern "C" unsigned long long ui_build_us;   /* (DL_PRESENT_TIME: when the interface was built, before it is drawn) */
