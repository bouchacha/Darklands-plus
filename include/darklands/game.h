/* darklands/game.h - readable names for the engine (root, s000..s043).
 *
 * Macros ONLY, like darklands/card.h: every name expands to the raw linker name (D_xxxx =
 * DGROUP offset, F_ssss_xxxx = far data, sub_<img>_<frame>_<off> = original function,
 * vec_xxxx = RTLink overlay call vector), so the compiled code is unchanged.  Engine files
 * include BOTH headers (card.h first): the names of card.h (party, leader, cur_loc, the s029
 * rules, the card engine API ...) are not repeated here.  Each source keeps its own
 * extern/prototype lines (their order matters to MSC 6.00A), written with these names.
 *
 * "?" at the start of a comment = meaning not fully certain.
 * A function and its overlay call vector (callers in other overlays go through the vector)
 * share a name: the vector is <name>_vec, or - when card.h already names the vector - the
 * function is <name>_impl.
 * Some functions also have a symbolic linker name used by a few sources (alloc_buffer,
 * random_n, copy_rect_saved, party_info ...); those names are left alone (a macro would
 * rename the symbol) and the raw name gets a different readable name here.
 *
 * tools/gamenames.py converts an engine file to these names; docs/ENGINE.md is the map.
 */

/* ---- root 05ba / 044a / 0410 / 03ec: DOS memory, random numbers, small helpers ----------- */
#define mem_alloc                sub_root_05ba_0004   /* (bytes, name): DOS block, its segment; aborts when out of memory (the symbol alloc_buffer) */
#define mem_free                 sub_root_05ba_00c2   /* (seg, name): free a DOS block, aborts on error (free_buffer) */
#define mem_realloc              sub_root_05ba_0144   /* (seg, bytes, name): resize a DOS block (realloc_buffer) */
#define dos_print                sub_root_044a_00f4   /* (msg): DOS 09h, print a '$'-terminated string */
#define dos_alloc                sub_root_044a_014e   /* (paras): DOS 48h allocate */
#define dos_free                 sub_root_044a_0164   /* (seg): DOS 49h free */
#define dos_resize               sub_root_044a_0181   /* (seg, paras): DOS 4Ah resize */
#define rand_n                   sub_root_0410_0008   /* (n): 0..n-1 from rand()'s 15 bits scaled by n (the symbol random_n) */
#define sign_of                  sub_root_0410_0038   /* (v): -1, 0, 1 */
#define dir_sub                  sub_root_0410_004c   /* (dir, n): direction - n (mod 8) */
#define dir_add                  sub_root_0410_005a   /* (dir, n): direction + n (mod 8) */
#define word_fill                sub_root_0410_0068   /* (int *p, v, nwords) */
#define word_fill_far            sub_root_0410_007e   /* (int far *p, v, nwords) */
#define index_of_min_byte        sub_root_0410_01dc   /* (unsigned char far *p, n) */
#define index_of_min_int         sub_root_0410_02a8   /* (int far *p, n) */
#define index_of_byte            sub_root_0410_02ca   /* (char far *p, v, n): index of byte v in p[0..n-1] */
#define index_of_word            sub_root_0410_02e6   /* (int far *p, v, n): index of word v, or n */
#define palette_save             sub_root_0410_031e   /* (a, b) */
#define palette_restore          sub_root_0410_0364   /* (a, b) */
#define rect_copy_paged          sub_root_03ec_000e   /* (src, sx, sy, w, h, dst, dx, dy): copy_rect via saved DOS copies (the symbol copy_rect_saved) */
#define saved_rects              D_8462               /* struct {x, y, w, h, seg}[12]: rectangles kept in DOS memory by rect_copy_paged */

/* ---- root 0617: files, catalogues (.CAT), pictures, fonts -------------------------------- */
#define file_open                sub_root_0617_000e   /* (name, mode): from the open catalogue when it holds the file */
#define file_create              sub_root_0617_0080   /* (name) */
#define file_close               sub_root_0617_00ae   /* (h) */
#define file_load                sub_root_0617_00d4   /* (name, char *buf) */
#define file_load_far            sub_root_0617_011e   /* (name, char far *buf): whole file, returns its size */
#define file_save                sub_root_0617_016a   /* (name, char far *buf, len) */
#define pic_draw                 sub_root_0617_01b2   /* (x, y, z, name, mode): unpack a picture file and draw it line by line */
#define pic_draw_1               sub_root_0617_0230   /* (x, name) */
#define pic_draw_pal             sub_root_0617_024a   /* (x, name, char *palette): draw a picture, its palette into palette */
#define pic_draw_0               sub_root_0617_0264   /* (x, name) */
#define pic_load_1               sub_root_0617_027c   /* (name) */
#define pic_load_m               sub_root_0617_0298   /* (name, mode) */
#define file_unpack              sub_root_0617_02b2   /* (char far *dst, name): unpack a whole file to memory */
#define font_load                sub_root_0617_02f6   /* (slot, idx, name): font idx of a font file into font slot */
#define pic_load                 sub_root_0617_045c   /* (name): load a picture into DOS memory, returns its handle (segment) */
#define pic_draw_1r              sub_root_0617_04d0   /* (name, x) */
#define pic_draw_open            sub_root_0617_04e2   /* (name, x): leaves the file open */
#define pic_draw_mem             sub_root_0617_0546   /* (seg, x): unpack a picture held in memory at seg:0000 */
#define mem_unpack               sub_root_0617_05b0   /* (char far *src, char far *dst) */
#define stream_restart_file      sub_root_0617_0644   /* (h) */
#define catalog_open             sub_root_0617_06d8   /* (name) */
#define catalog_close            sub_root_0617_070e   /* () */
#define catalog_find             sub_root_0617_0736   /* (name, struct catentry *): NULL name = next entry */
#define fab_open                 sub_root_05d9_002a   /* ? FAB stream (PAN player): open */
#define fab_close                sub_root_05d9_0059   /* FAB stream: close */
#define fab_eof                  sub_root_05d9_006a   /* FAB stream: end of file */
#define fab_read                 sub_root_05d9_0070   /* FAB stream: read to a linear address */
#define file_read_all            sub_root_05d9_019a   /* (name, char far *buf): load a file */
#define ems_map_page             sub_root_05f8_0117   /* (phys, log, handle): map logical EMS page log of handle into physical page phys (root 05f8: EMS driver interface) */

/* ---- root 0265: text output (struct gfx / textwin: +0c colour, +0e background, +10 font) --- */
#define draw_text_far            sub_root_0265_0004   /* (win, x, y, char far *s) */
#define draw_text                sub_root_0265_006e   /* (win, x, y, char *s) */
#define draw_text_centered       sub_root_0265_00c4   /* (win, y, s) */
#define draw_text_wrapped_far    sub_root_0265_00f0   /* (win, char far *text, ...) */
#define draw_text_wrapped        sub_root_0265_0134   /* (win, text, width, x, y, ...): word-wrapped text with control codes and hot spots */
#define measure_text_wrapped_far sub_root_0265_0614   /* (win, char far *text, ...) */
#define measure_text_wrapped     sub_root_0265_0654   /* (win, text, width, x, y, ...): (y_end - 1) << 16 | last line width */
#define text_pixel_width         sub_root_0265_084e   /* (win, s): width of a string in the window's font */
#define ltoa_commas              sub_root_0265_0882   /* (long v, buf): "12,345" */
#define itoa_commas              sub_root_0265_09b2   /* (int v, buf) */

/* ---- root 0000: game shell (menu bar, hot keys, options, message boxes, party panel) ----- */
#define menu_bar_run             sub_root_0000_0000   /* (menus, once): menu bar loop, calls the chosen item's handler */
#define menu_bar_track           sub_root_0000_01d4   /* (menus) */
#define menu_bar_draw            sub_root_0000_06c2   /* (menu) */
#define menu_dropdown_open       sub_root_0000_07d4   /* (items, x, y) */
#define menu_submenu_open        sub_root_0000_0984   /* (subs, x, y) */
#define menu_dropdown_close      sub_root_0000_0bc2   /* () */
#define menu_submenu_close       sub_root_0000_0c2c   /* () */
#define rand_1_n                 sub_root_0000_0c8c   /* (n): 1..n (n < 2 counts as 2) */
#define hot_keys                 sub_root_0000_0caa   /* (): Alt-D difficulty, Alt-F sound, Alt-C saintly changes, Alt-M music, F10 menu */
#define toggle_saint_msgs        sub_root_0000_1154   /* (): "Show saintly changes" on/off */
#define toggle_music             sub_root_0000_11f2   /* () */
#define toggle_sound             sub_root_0000_129a   /* () */
#define exit_if_requested        sub_root_0000_1338   /* (): exit to DOS when D_07f4 is set */
#define game_shutdown            sub_root_0000_1354   /* (): stop sound/timer, back to text mode 3 */
#define flush_keys               sub_root_0000_1384   /* (): empty the key buffer */
#define quit_to_dos              sub_root_0000_13a0   /* (): "Quit to DOS? y/n" */
#define set_difficulty_basic     sub_root_0000_1408   /* () */
#define set_difficulty_standard  sub_root_0000_1418   /* () */
#define set_difficulty_expert    sub_root_0000_1428   /* () */
#define show_difficulty          sub_root_0000_1438   /* (show): tick the menu entry, report "Difficulty is now set to ..." */
#define copy_rect_clipped        sub_root_0000_14f6   /* (src, x, y, w, h, dst, dx, dy) */
#define party_panel_frames       sub_root_0000_1588   /* (which): portrait frames, normal (0) or highlighted (1) */
#define party_panel_rows         sub_root_0000_1716   /* (which): the party panel rows */
#define member_status_letter     sub_root_0000_18fa   /* (i, win) */
#define member_bars              sub_root_0000_1a9c   /* (i): endurance/strength bars of a member */
#define draw_frame_box           sub_root_0000_2286   /* (win, x, y, cw, ch, bg): bevelled box with a cw x ch interior */
#define text_box                 sub_root_0000_23e6   /* (win, y, save, text, dst, ...): framed text box, returns the saved rectangle */
#define show_message             sub_root_0000_24bc   /* (text, ticks): message box for a while */
#define ask_yes_no               sub_root_0000_2522   /* (question) */
#define datafile_select          sub_root_0000_25f2   /* (name): select data file name (kept in data_file) unless current */
#define datafile_forget          sub_root_0000_262e   /* (): data_file = "" */
#define game_difficulty          D_906a               /* 0 basic, 1 standard, 2 expert (chances shown as numbers / words / nothing); bit 7 other use */
#define opt_saint_msgs           D_01bd               /* 1: show saintly changes */
#define opt_sound                D_01be               /* 1: sound effects on */
#define opt_music                D_01bf               /* 1: music on */
#define exit_requested           D_07f4               /* set: exit_if_requested leaves to DOS */
#define data_file                F_2e38_0000          /* name of the current data file (datafile_select) */

/* ---- root 047c: graphics driver interface (MGRAPHIC thunks), mouse, keyboard, timer (assembly) --- */
#define gfx_copy_rect            sub_root_047c_0553   /* (src, sx, sy, w, h, dst, dx, dy): copy a rectangle between pages */
#define screen_hold              sub_root_047c_0568   /* (): before a whole screen is drawn (the graphics driver hides it meanwhile) */
#define screen_release           sub_root_047c_056f   /* (): after: the screen shown */
#define gfx_text                 sub_root_047c_055a   /* (win, x, y, char *s): draw a string */
#define gfx_draw_pic             sub_root_047c_05a5   /* (win, x, y, seg): draw a picture held in memory (pic_load handle) */
#define gfx_recolor_rect         sub_root_047c_04dc   /* (win, x, y, w, h, from, to): replace colour from by to (highlights) */
#define gfx_fill_rect            sub_root_047c_0952   /* (win, x, y, w, h, colour) */
#define gfx_line                 sub_root_047c_0d62   /* (win, x0, y0, x1, y1, colour) */
#define gfx_draw_sprite          sub_root_047c_04ef   /* (buf, x, y, sprite) */
#define gfx_save_rect            sub_root_047c_04d5   /* (0, x, y, w, h): copy a screen rectangle to DOS memory, returns its segment */
#define gfx_page_segment         sub_root_047c_053e   /* ? (page): segment of a graphics page */
#define char_width               sub_root_047c_04e3   /* (font, ch): width of a character in pixels */
#define font_height              sub_root_047c_051b   /* ? (font) */
#define set_palette              sub_root_047c_058b   /* (char *pal): load a 256-colour palette (palette) */
#define key_pressed              sub_root_047c_073d   /* (): 1 when a key is waiting */
#define get_key                  sub_root_047c_0744   /* (): next key (scan code << 8 | ASCII) */
#define sound_play               sub_root_047c_078a   /* (n): play a sound (1: stop the music) */
#define distance3                sub_root_047c_1120   /* (dx, dy, dz): integer approximation of sqrt(dx*dx + dy*dy + dz*dz) */
#define mouse_set_cursor         sub_root_047c_124f   /* (x, y, cursor): hot spot and pointer */
#define mouse_move               sub_root_047c_1266   /* ? (x, y) */
#define mouse_x                  D_6eec               /* mouse position */
#define mouse_y                  D_6eee
#define mouse_buttons            D_6ef0               /* bit 0 left, bit 1 right */
#define cur_palette              D_e48c               /* the current 256-colour palette (768 bytes) */
#define screen_win               D_27f4               /* struct gfx of the visible screen page */
#define screen_win_colour        D_2800               /* its text colour (screen_win + 0x0c) */
#define screen_win_font          D_2804               /* its font (screen_win + 0x10) */
#define back_win                 D_2806               /* struct gfx of the off-screen page (drawn, then copied to screen_win) */
#define back_win_colour          D_2812               /* its text colour */
#define back_win_font            D_2816               /* its font */
#define cursor_default           D_9c42               /* mouse cursor shapes: default */
#define cursor_current           D_a68f               /* the cursor set last */
#define party_icons_dirty        D_9c46               /* party changed: reload the icons */
#define menu_bar_active          D_a2e3               /* a menu of the menu bar is open */
#define font_flag_a              D_9920               /* 2 while a screen with lists is up (barter, sheet), else 1 */
#define font_flag_b              D_9934               /* (set with font_flag_a) */
#define screen_kind              D_907c               /* the screen up: 2 a sheet, 5 a shop ... */
#define saved_back_font          D_e7a2               /* back_win_font kept by the shop and the sheet */
#define saved_screen_font        D_e7a0               /* screen_win_font kept by the shop and the sheet */
#define in_battle                D_907a               /* 1 during a battle */
#define shop_names_seg           D_7e66               /* segment of F_290e_207f, the shops' names (char far *[9]) */

/* ---- s032: the shop (barter_screen) ------------------------------------------------------ */
#define barter_who               D_884e               /* the member bartering (whose things are listed) */
#define barter_leader            D_884c               /* the leader when the shop opened */
#define barter_name              D_869c               /* char far *: the shop's name */
#define barter_cats              D_8868               /* long: the item categories it sells */
#define barter_shop              D_8836               /* the shop (city_rec quality index; -1, -2 others) */
#define barter_quality           D_8834               /* the quality of its wares */
#define barter_wares             D_86a2               /* int[]: the item ids for sale today */
#define barter_nwares            D_885c               /* their number */
#define barter_ware_top          D_86a0               /* the first ware shown */
#define barter_ware_cur          D_8832               /* the ware cursor (row on screen) */
#define barter_side              D_8842               /* int[5]: 1 the cursor is in the wares, 0 in the member's things */
#define barter_item_top          D_8852               /* int[5]: the member's first thing shown */
#define barter_item_cur          D_885e               /* int[5]: the cursor in the member's things */
#define barter_item_count        D_8838               /* int[5]: the member's things, counted */

/* ---- s031: the character sheet (member_sheet) -------------------------------------------- */
#define sheet_box                D_867c               /* char: the box chosen (9: equipment) */
#define sheet_who                D_867d               /* char: the member shown */
#define sheet_list_open          D_867e               /* char: the chosen box's list is open */

/* ---- s000: start-up (main, config.drk, drivers) ------------------------------------------ */
#define startup_error            sub_s000_0e76_0000   /* (msg): print "<program>: msg" */
#define bios_set_cursor          sub_s000_0e76_0092   /* (row, col) */
#define print_banner             sub_s000_0e76_00b8   /* () */
#define startup_config           sub_s000_0e76_0114   /* (argc, argv, envp): title screen, config.drk (sound driver), command line */
#define battle_menus_enable      sub_s000_0e76_0f04   /* (): Orders/Attack menu entries at the start of a battle */
#define video_detect             sub_s000_0fb2_0008   /* (): BIOS display combination, remember the text mode */
#define video_restore            sub_s000_0fb2_00be   /* (): back to the remembered text mode */
#define video_restore_vec        vec_1865             /* overlay call vector of video_restore */

/* ---- s001: default party ----------------------------------------------------------------- */
#define party_default            sub_s001_0e76_0000   /* (): the four pre-generated members with equipment */
#define party_default_vec        vec_1adb             /* overlay call vector of party_default */

/* ---- s002 / s003: battle entry and battle save files ------------------------------------- */
#define battle_run               sub_s002_0e76_0000   /* (mode, bfld, rseed, 3 x (type, quality, number)): set up, fight, clean up */
#define battle_run_vec           vec_18bf             /* overlay call vector of battle_run */
#define battle_portraits         sub_s002_0e76_0806   /* (): short portraits of the party on the battle screen */
#define battle_tiles_load        sub_s002_0e76_0c8c   /* (): tile sets of the battlefield (.flc .ffc .nwc ...) */
#define battle_tiles_load_vec    vec_1ae5             /* overlay call vector of battle_tiles_load */
#define party_anim_alloc         sub_s002_0e76_168a   /* (): animation buffers of the party (tacanim.db sizes) */
#define party_records_load       sub_s002_0e76_1762   /* (): party records from darkland.lst, default names */
#define battle_save              sub_s003_0e76_00dc   /* ? (): serialise the battle into saves\<name>.bsv (PKWARE implode) */
#define battle_save_vec          vec_1b21             /* overlay call vector of battle_save */
#define battle_restore           sub_s003_0e76_0ce8   /* ? (): read it back (explode) */
#define battle_restore_vec       vec_1b17             /* overlay call vector of battle_restore */
#define battle_args              D_ca3d               /* battle parameters passed to battle_run */

/* ---- s004..s013: battlefield construction (generator work segment D_db39, see s005_1432.c) --- */
#define battlefield_build        sub_s004_0e76_0000   /* (type, ...): load darkland.enm/.lst, run the generator, write terrain.fil, level%d.trn/.atv */
#define battlefield_build_vec    vec_1aef             /* overlay call vector of battlefield_build */
#define monster_type_map         sub_s004_0e76_06aa   /* ? : map group monster types (used by s013) */
#define monsters_clear           sub_s004_0e76_04cc   /* (): clear the monster slots, default monster record */
#define groups_clear             sub_s004_0e76_1060   /* (): clear the groups, default group record */
#define monsters_fixup           sub_s004_0e76_122c   /* (): per level: variants from the seed, merge duplicates, group bonuses */
#define groups_fixup             sub_s004_0e76_1d90   /* (): drop empty groups, fix leaders, roll group values */
#define roll_dice                sub_s004_0e76_20f6   /* (n, d): n dice of d sides */
#define dir_between              sub_s004_0e76_2160   /* (x0, y0, x1, y1): direction 0/2/4/6 */
#define map_turn                 sub_s004_1091_000a   /* (r): turn / mirror a loaded tactical map (r & 3 quarter turns, r >= 4 mirrored) */
#define facing_turn              sub_s004_1091_17e0   /* (f, r): turn a facing 0..7 by r */
#define gen_battlefield          sub_s005_1432_0000   /* (): generate a battlefield, returns the number of levels */
#define gen_battlefield_vec      vec_1b8f             /* overlay call vector of gen_battlefield */
#define gen_clear                sub_s005_1432_005c   /* (): clear the work area */
#define gen_add_room             sub_s006_1a57_000c   /* (kind, ...): kind 2 keep, 3 tower, 4 gate house, 5 building, 6 gate path */
#define castle_build             sub_s007_15af_0000   /* (): build and save all levels of a castle/keep battlefield */
#define castle_build_vec         vec_1c11             /* overlay call vector of castle_build */
#define wild_build               sub_s008_1432_1388   /* ? (): wilderness battlefield */
#define empty_battlefield        sub_s008_1432_2d62   /* (): empty map in the floor image, saved as level%d.flr */
#define empty_battlefield_vec    vec_1b99             /* overlay call vector of empty_battlefield */
#define mine_build               sub_s009_1432_0000   /* (t): mine battle for location type t, returns the number of levels */
#define mine_build_vec           vec_1b85             /* overlay call vector of mine_build */
#define city_build               sub_s010_1432_0000   /* (p): city battle, returns the number of levels */
#define city_build_vec           vec_1b7b             /* overlay call vector of city_build */
#define city_battle_type         sub_s010_1432_0102   /* (p): battlefield type 1..9 for a city location type */
#define level_save               sub_s010_1432_2de0   /* (k): save level buffer k as level%d.flr */
#define templar_build            sub_s011_1432_0000   /* (): Templar fortress, 16 rooms into 4 level buffers */
#define templar_build_vec        vec_1b5d             /* overlay call vector of templar_build */
#define gen_work                 D_db39               /* segment of the generator work area (rooms, connections, cells) */
#define gen_level                D_db81               /* current level of the generator */
#define gen_rooms                D_db45               /* rooms per level */
#define gen_level_lo             D_db7d               /* lowest level built */
#define gen_level_hi             D_db7b               /* highest level built */
#define keep_x0                  D_db3d               /* keep rectangle x0 (then x1, y0, y1) */
#define gen_strips               D_db65               /* the 4 free strips around the keep {x0, x1, y0, y1, side} */
#define gen_cell_type            D_db8f               /* cell types (0/1 solid, 2..8 floor kinds), [(lvl * 40 + x) * 40 + y] */
#define gen_cell_walls           D_db8d               /* wall bits 1/2/4/8 for sides 0..3 */
#define gen_cell_door            D_db93               /* wall/door kinds 3..6, 11 stairs */
#define gen_exits                D_db95               /* exits (stairs) per level */
#define gen_exit_count           D_db71               /* exits per level (gen_exits) */
#define gate_side                D_db25               /* gate side / party side of the battlefield */
#define enemy_seg                D_dadd               /* segment of the enemy buffer (darkland.enm, 0xf5c8 bytes) */
#define mon_template             D_db29               /* int[16][12] template per monster slot */
#define bf_monsters              D_db2b               /* struct mon[16][12] (0xd6 bytes) */
#define bf_groups                D_db2d               /* struct group[16][10] (0x1e bytes) */
#define mon_templates            D_db2f               /* monster templates (0xcc bytes) */
#define enemy_types              D_db31               /* struct etype[0x52] (0x18 bytes) */
#define tile_desc                D_db33               /* battlefield tile description record */
#define bf_items                 D_db35               /* item records (0x2e bytes) */
#define mon_count                D_dae5               /* monsters per level [16] */
#define group_count              D_db05               /* groups per level [16] */

/* ---- s014..s025: tactical combat --------------------------------------------------------- */
#define combat_main              sub_s014_0e76_0000   /* (): combat entry and main loop */
#define combat_main_vec          vec_1b03             /* overlay call vector of combat_main */
#define combat_over              sub_s014_0e76_08fe   /* (): end-of-battle test, 1 = leave */
#define combat_mode_leave        sub_s014_0e76_0d86   /* (): leave battle mode */
#define combat_recover           sub_s014_0e76_0dee   /* (): after a fight: clear damage, endurance 3/4, wake the unconscious */
#define combat_recover_vec       vec_1af9             /* overlay call vector of combat_recover */
#define combat_tick              sub_s014_0e76_0f58   /* (): one tick: AI, locations, melee, actions, redraw */
#define combat_actions           sub_s014_0e76_116e   /* (): action state machine of every combatant */
#define cbt_step                 sub_s014_0e76_1624   /* (i): one step along the path */
#define cbt_missile_action       sub_s014_0e76_1ba0   /* (i): draw/loose, reload, aim */
#define cbt_location_action      sub_s014_0e76_2170   /* (i): door/lock/chest actions ordered from the menu */
#define cbt_dying                sub_s014_0e76_2324   /* (i): falling / dying, the body becomes a loot pile */
#define cbt_cancel_action        sub_s014_0e76_27ac   /* (i) */
#define cbt_forget_target        sub_s014_0e76_27cc   /* (i): enemies forget i as target */
#define push_direction           sub_s014_0e76_2a04   /* (): attacker's facing, randomly turned by one */
#define melee_exchanges          sub_s014_0e76_2a4a   /* (): melee rolls of a tick */
#define cbt_ready                sub_s014_0e76_2fa2   /* (i): arrived or stopped: action 7 "ready"; one with an attack order engages at once */
#define combat_input             sub_s014_0e76_3166   /* (): player input of one tick */
#define cursor_keys              sub_s014_0e76_35fc   /* (): cursor keys move the mouse pointer */
#define cbt_set_anim             sub_s014_0e76_38b6   /* (i, anim): set the animation, loading the sprite set */
#define order_queue              sub_s014_0e76_3c86   /* (i, ...): queue an order */
#define order_queue_clear        sub_s014_0e76_3d0c   /* () */
#define sprite_file_load         sub_s014_0e76_3d2c   /* (slot, ...): load a sprite file into a slot */
#define effect_start             sub_s014_0e76_4188   /* (...): start an effect / level event at a position */
#define activations_check        sub_s014_0e76_4442   /* (): hidden encounters start when seen / approached */
#define endurance_recover        sub_s014_0e76_45ae   /* (): recovery outside battle */
#define cursor_normal            sub_s014_0e76_4780   /* (): restore the normal mouse cursor */
#define cbt_hostile_pair         sub_s014_0e76_47ae   /* (i, j) */
#define path_find                sub_s014_12f7_000a   /* (i): path to the destination, 1 found, 2 not */
#define path_ems_on              sub_s014_12f7_03b6   /* (): map EMS page 8 for the path search (nested) */
#define path_ems_off             sub_s014_12f7_03d2   /* () */
#define path_trackers            sub_s014_12f7_03ea   /* (rec): run the bug-algorithm trackers */
#define path_give                sub_s014_12f7_1554   /* (i): give combatant i the last found path */
#define path_save                sub_s014_12f7_159c   /* () */
#define path_give_saved          sub_s014_12f7_15d0   /* (i) */
#define cbt_collisions           sub_s014_1462_000a   /* (c): movement collision test at the current position */
#define combat_draw              sub_s014_14f7_0008   /* (): draw one combat frame */
#define cbt_screen_pos           sub_s014_14f7_31e6   /* (): screen position of every visible combatant */
#define cbt_anim_set             sub_s014_14f7_343c   /* (i): animation set of combatant i */
#define attack_rate              sub_s014_18bf_0002   /* (i) */
#define cbt_attack               sub_s014_18bf_0102   /* (a, t): one attack, returns the to-hit result */
#define to_hit_roll              sub_s014_18bf_02a8   /* (a, t, *crit): 0 miss, 1 parried/dodged, 2 hit, 3 good hit */
#define missile_to_hit           sub_s014_18bf_0592   /* (...) */
#define hit_damage               sub_s014_18bf_0778   /* (a, t, loc, r) */
#define damage_queue             sub_s014_18bf_08ca   /* (t, d, ...) */
#define damage_apply             sub_s014_18bf_098e   /* (): endurance 0 unconscious, strength 0 dead */
#define weapon_ready             sub_s014_18bf_0a40   /* (i, w, alt) */
#define can_fire                 sub_s014_18bf_0c02   /* (i): has ammunition */
#define use_ammo                 sub_s014_18bf_0c48   /* (i) */
#define consume_item             sub_s014_18bf_0db4   /* (i, id, q) */
#define consume_slot             sub_s014_18bf_0fa4   /* (i, k) */
#define combat_mouse             sub_s014_19cd_0006   /* (): mouse handler */
#define order_mode_enter         sub_s014_19cd_069c   /* () */
#define order_mode_leave         sub_s014_19cd_0738   /* (): apply the order */
#define order_mode_leave_vec     vec_1a59             /* overlay call vector of order_mode_leave */
#define party_locations          sub_s014_19cd_26a4   /* (): door/stairs/region/exit flags, triggers */
#define order_menu_reset         sub_s014_19cd_28a2   /* () */
#define order_menu_reset_vec     vec_18d3             /* overlay call vector of order_menu_reset */
#define order_menu_build         sub_s014_19cd_29b8   /* () */
#define loot_pile_open           sub_s014_19cd_2d08   /* (i) */
#define loot_screen              sub_s014_19cd_2e50   /* (): run the s040 loot screen from combat */
#define loot_after_fight         sub_s014_19cd_34a0   /* (): items of the fallen and new piles to the loot file */
#define shields_load             sub_s014_19cd_36f0   /* (): party shield pictures */
#define shields_free             sub_s014_19cd_3766   /* () */
#define missiles_advance         sub_s014_1d47_0000   /* (): every missile one tick */
#define line_blocked             sub_s014_1d69_000c   /* (a, b): line of sight / fire blocked by a wall, furniture or combatant */
#define ai_tick                  sub_s014_1db7_0008   /* () */
#define ai_costs                 sub_s014_1db7_0028   /* (i, lo, hi): walking costs from i to lo..hi-1, far 20a5:787e[i*5+j] */
#define ai_set_target            sub_s014_1db7_0c2a   /* (i, t) */
#define ai_rethink               sub_s014_1db7_0e22   /* (i) */
#define cloud_blocks_line        sub_s014_1ec9_0142   /* (a, b): line passes through an effect cloud */
#define debug_mem_track          sub_s015_1ef8_0000   /* (): smallest free DOS memory seen */
#define debug_mem_track_vec      vec_1c89             /* overlay call vector of debug_mem_track */
#define combat_keys              sub_s016_1ef8_0000   /* (): combat keyboard handler */
#define combat_keys_vec          vec_1d33             /* overlay call vector of combat_keys */
#define member_activate          sub_s016_1ef8_09bc   /* (i): make party member i the active one */
#define member_activate_vec      vec_1d47             /* overlay call vector of member_activate */
#define marching_order_rebuild   sub_s016_1ef8_0da6   /* () */
#define marching_order_rebuild_vec vec_1c75             /* overlay call vector of marching_order_rebuild */
#define single_file              sub_s016_1ef8_0e7e   /* () */
#define single_file_vec          vec_1de7             /* overlay call vector of single_file */
#define floor_view_draw          sub_s017_1ef8_0000   /* (who): redraw the 14 x 10 cell window around who */
#define floor_view_draw_vec      vec_1c2f             /* overlay call vector of floor_view_draw */
#define missile_launch           sub_s018_1ef8_0000   /* (i, target, ...) */
#define missile_spent            sub_s018_1ef8_03ae   /* (kind, quality): a thrown weapon (always) or an arrow / bolt (7 in 10) to be found after the battle */
#define missile_launch_vec       vec_1ced             /* overlay call vector of missile_launch */
#define missile_remove           sub_s018_1ef8_027e   /* (idx) */
#define missile_remove_vec       vec_1d79             /* overlay call vector of missile_remove */
#define enemy_sees               sub_s018_1f43_0006   /* (e, seen[5]): visible party members */
#define enemy_turn               sub_s018_1f43_0256   /* (): enemy AI, once per tick */
#define enemy_turn_vec           vec_1c43             /* overlay call vector of enemy_turn */
#define effect_spell             sub_s019_1ef8_0000   /* ? (...): impact effect of a spell/missile */
#define effect_spell_vec         vec_1dc9             /* overlay call vector of effect_spell */
#define level_enemies_load       sub_s020_1ef8_0000   /* (): level%d.trn into the enemy type table */
#define level_enemies_load_vec   vec_1df1             /* overlay call vector of level_enemies_load */
#define level_load               sub_s020_1ef8_05e6   /* () */
#define level_load_vec           vec_1c7f             /* overlay call vector of level_load */
#define combat_state_clear       sub_s020_1ef8_06b6   /* () */
#define combat_state_clear_vec   vec_1c61             /* overlay call vector of combat_state_clear */
#define skill_try_improve        sub_s021_1ef8_0000   /* (member, skill, mode): skill improvement in combat */
#define skill_try_improve_vec    vec_1ca7             /* overlay call vector of skill_try_improve */
#define skill_change_msg         sub_s021_1ef8_0080   /* (member, skill, amount): "<skill> changed from X to Y" */
#define enemy_info               sub_s021_1ef8_02ce   /* (): enemy information display (einfo.cat) */
#define enemy_info_vec           vec_1a4f             /* overlay call vector of enemy_info */
#define combat_char_sheet        sub_s022_1ef8_0000   /* () */
#define combat_char_sheet_vec    vec_1d65             /* overlay call vector of combat_char_sheet */
#define do_surrender             sub_s022_1ef8_03e4   /* () */
#define member_away_msg          sub_s022_1ef8_04e4   /* (who): "... is on another level" (the sheet in battle, for a member out of reach) */
#define do_surrender_vec         vec_19eb             /* overlay call vector of do_surrender */
#define choose_party_order       sub_s022_1ef8_051c   /* () */
#define choose_party_order_vec   vec_183d             /* overlay call vector of choose_party_order */
#define door_toggle              sub_s023_1ef8_00b8   /* (n, who): open/close a door */
#define door_toggle_vec          vec_1d0b             /* overlay call vector of door_toggle */
#define door_set                 sub_s023_1ef8_04ac   /* (n, state) */
#define door_set_vec             vec_1d6f             /* overlay call vector of door_set */
#define secret_door_search       sub_s023_1ef8_0812   /* (who, roll) */
#define secret_door_search_vec   vec_17ed             /* overlay call vector of secret_door_search */
#define trap_spell               sub_s023_1ef8_0d88   /* (t, who) */
#define trap_spell_vec           vec_1801             /* overlay call vector of trap_spell */
#define trap_missile             sub_s023_1ef8_0e30   /* (t, t2) */
#define trap_missile_vec         vec_180b             /* overlay call vector of trap_missile */
#define lock_pick                sub_s023_1ef8_12c0   /* (): selected member picks a lock (lock picks = item 0x40) */
#define lock_pick_vec            vec_1d1f             /* overlay call vector of lock_pick */
#define chest_open               sub_s023_1ef8_13d0   /* (who) */
#define chest_open_vec           vec_1d01             /* overlay call vector of chest_open */
#define chest_disarm             sub_s023_1ef8_153e   /* (who) */
#define chest_disarm_vec         vec_1d15             /* overlay call vector of chest_disarm */
#define take_exit                sub_s023_1ef8_1618   /* (who, n): stairs / exit to another level */
#define take_exit_vec            vec_1cd9             /* overlay call vector of take_exit */
#define leave_level              sub_s023_1ef8_17d2   /* (e) */
#define level_switch             sub_s024_1ef8_059a   /* (n): save the current .flr/.atv, load level n */
#define level_switch_vec         vec_1b3f             /* overlay call vector of level_switch */
#define furniture_load           sub_s024_1ef8_07a2   /* (): <name>.frc of the level */
#define furniture_load_vec       vec_1dbf             /* overlay call vector of furniture_load */
#define place_around             sub_s024_1ef8_08f2   /* (who, ...): place a combatant around an activation spot */
#define place_around_vec         vec_1e05             /* overlay call vector of place_around */
#define enemies_place            sub_s025_1ef8_0000   /* (): enemies of level%d.enm into combatants 5..12 */
#define enemies_place_vec        vec_1d29             /* overlay call vector of enemies_place */
#define enemy_anim_load          sub_s025_1ef8_09a6   /* (kind, who) */
#define enemy_anim_load_vec      vec_1ddd             /* overlay call vector of enemy_anim_load */
#define enemy_anim_alloc         sub_s025_1ef8_0a84   /* (kind) */
#define enemy_anim_alloc_vec     vec_1db5             /* overlay call vector of enemy_anim_alloc */
#define enemy_anim_free5         sub_s025_1ef8_0be4   /* () */
#define enemy_anim_free5_vec     vec_1cb1             /* overlay call vector of enemy_anim_free5 */
#define enemy_anim_free6         sub_s025_1ef8_0c02   /* () */
#define enemy_anim_free6_vec     vec_1dd3             /* overlay call vector of enemy_anim_free6 */
#define combatants               D_9c55               /* struct cbt[13] (0x80 bytes): 0..4 party (= party - 0x14), 5..12 enemies */
#define cbt_floor                D_a6b4               /* [13] floor (level) of each combatant */
#define cbt_ends_battle          D_c1f3               /* [13] its fall ends the battle (the master of a place; monster record +aa) */
#define cbt_action               D_ca64               /* [13] current action */
#define cbt_anim                 D_ca57               /* [13] current animation (3 dying ...) */
#define cbt_moving               D_a6d3               /* [13] moving to a target */
#define cbt_flags                D_a696               /* [13] location flag words (byte 1 bit 4: AI disabled) */
#define cbt_special              D_a6c4               /* [13] location action (2 stairs, ...) */
#define cbt_attack_rate          D_d493               /* [13] (255 = recompute) */
#define cbt_queued_damage        D_d70c               /* [13] */
#define cbt_damage_shown         D_d4c7               /* [13] damage taken since the last display */
#define cbt_was_hit              D_d4f2               /* [13] */
#define cbt_engage_time          D_d996               /* [13] time of engagement */
#define cbt_gave_up              D_d5e9               /* [13] gave up following */
#define cbt_giveup_timeout       D_d962               /* [13] */
#define cbt_half_w               D_c80f               /* [13] half width of the bounding box */
#define cbt_half_h               D_c831               /* [13] half height */
#define cbt_anim_seq             D_c98a               /* [13] animation / sound sequence */
#define cbt_anim_len             D_c96b               /* [13] its length */
#define enemy_class              D_c2bf               /* [13] 0 melee, 1 archer, 2 caster */
#define cbt_selected             D_9064               /* [13] chosen flags */
#define real_combat              D_a6c1               /* 1 = enemies engaged (battle mode) */
#define combat_clock             D_07d4               /* ticks */
#define combat_alarm             D_d6f4               /* alarm: real combat starts */
#define combat_flags             D_9080               /* combat start flags */
#define ai_paused                D_a695               /* 1 = paused while orders are given */
#define selected_member          D_a6b2               /* member being given orders */
#define order_mode               D_a6b3               /* 1 while a member is selected */
#define order_action             D_a6b0               /* chosen action (1 walk, 0x80 missile, 0x100 spell/item ...) */
#define order_special            D_a6d1               /* the action at a place being ordered (with order_action 0x400) */
#define order_dest_x             D_c652               /* int: a spot on the ground clicked in this order (0 none): where order_mode_leave sends the member */
#define order_dest_y             D_c67d               /* int: its y */
#define cbt_state                D_a3d8               /* [5] a member's special state (1 'b', 2 'e' on the panel) */
#define insid_office             D_eeb4               /* $Insid00 (s108 1925): the office chosen above the marketplace (0 the leading merchant, 1 the Medici, 2 the Fugger, 3 the Hanse) */
#define cbt_path_step            D_d6fb               /* char[13]: the step a walk has reached in its path */
#define cbt_mods                 D_a532               /* [13] of 0x12 bytes: +9 held (1: told to hold), +0c/+0e/+10 the modifiers' ends */
#define cursor_target            D_d845               /* what the cursor points at (>= 0 combatant, -2 ground, -3 blocked) */
#define hotspot_cbt              D_d848               /* hot spot -> combatant */
#define hotspot_count            D_e0a1
#define marching_order           D_c624               /* char[6] */
#define marching_len             D_c629
#define move_mode                D_c102               /* movement mode (2 = single file) */
#define party_floor              D_c623               /* floor of the party */
#define floor_shown              D_11ca               /* floor currently shown */
#define floor_dirty              D_d4e4               /* floor changed: save it */
#define demo_mode                D_c21a               /* demo / tutorial mode */
#define loot_pile_count          D_c80e
#define activation_count         D_d6cc
#define effect_clouds            D_a2eb               /* effect list 1 (10 bytes each) */
#define effect_cloud_count       D_a33d
#define effect_areas             D_a342               /* effect list 2 */
#define effect_markers           D_a396               /* effect list 3 */
#define missile_recs             D_944d               /* struct [16] (0x1a bytes) */
#define missile_count            D_c180
#define enemy_anim_slot          D_d65f               /* animation id loaded in slot 5 / 6 (-1 free) */
#define anim_buffers             D_ca45               /* [7] sprite memory per slot */
#define anim_file                D_1174               /* [7] animation file name per kind slot */
#define los_x0                   D_e0a5               /* line of sight segment (los_x0, los_y0)-(los_x1, los_y1), pixels */
#define los_y0                   D_e0a3
#define los_x1                   D_e0a9
#define los_y1                   D_e0a7
#define order_queues             D_d9e6               /* [13][3] */
#define load_buf                 D_c103               /* far load buffer (level files, loot file image) */
#define loot_list                D_c291               /* pointer to the near loot copy (set by s002) */
#define floor_img                F_20a5_0000          /* current level floor image (0x33fc bytes, level%d.flr): cells, heights, objects, exits */
#define floor_doors              F_20a5_33d3          /* door open flags of the floor */
#define member_palette           F_20a5_33fc          /* palette per party slot (24 bytes) */
#define activations              F_20a5_47ee          /* activation records (0x1e bytes, activation_count) */
#define loot_piles               F_20a5_59c8          /* loot piles (loot_pile_count): offsets in the loot file */
#define cbt_paths                F_20a5_6238          /* path of every combatant */
#define path_result              F_20a5_71ba          /* step directions of the last path (0xff end) */
#define doors_opened             F_2e38_0bb6          /* long per level: wall groups / doors opened */
#define enemy_items              F_2e38_0dbe          /* enemy inventories ([i - 5]) */
#define anim_set_names           F_2e38_0bd8

/* ---- s029 1367 / 0e76: rules (more; card.h has the rest) --------------------------------- */
#define bell_name                sub_s029_1367_0846   /* (i, char far *dst): copy name i of the time-of-day table */
#define date_copy                sub_s029_1367_088a   /* (d, s) */
#define date_set                 sub_s029_1367_08ae   /* (d, year, month, day, hour) */
#define month_of_yday            sub_s029_1367_0938   /* (d) */
#define mday_of_yday             sub_s029_1367_0982   /* (d) */
#define yday_now                 sub_s029_1367_09cc   /* () */
#define date_add_hours           sub_s029_1367_09ea   /* (date far *d, hours) */
#define day_number_now           sub_s029_1367_0a6c   /* (): absolute day number of now */
#define days_since               sub_s029_1367_0a96   /* (date far *d): 0 if in the future */
#define days_until               sub_s029_1367_0b1c   /* (date far *d): 0 if past */
#define days_between             sub_s029_1367_0ba4   /* (a, b): 0..32000 */
#define member_fatigue           sub_s029_0e76_00c6   /* (who): member value +0x35 */
#define set_member_fatigue       sub_s029_0e76_00e4   /* (who, v) */
#define equip_bonuses            sub_s029_0e76_0792   /* (who, v): attribute bonuses of the equipped items */
#define heal_attr_pct            sub_s029_0e76_08f2   /* (who, k, pct): heal pct % of an attribute's damage */
#define party_item_total         sub_s029_0e76_1128   /* (code): quantity carried by the present members */
#define knows_item_type          sub_s029_0e76_11fa   /* (who, k): 2e38:075f knowledge table */
#define member_fatigue_state     sub_s029_0e76_18f2   /* (who): 0 fresh .. 0x13, -1 exhausted */
#define hours_pass               sub_s029_0e76_255a   /* (hours): hourly party update; nobody left -> death screen */
#define event_slot_free          sub_s029_0e76_3144   /* (): first free event slot (fatal when all 300 are used) */
#define event_has_started        sub_s029_0e76_3180   /* (i) */
#define event_has_ended          sub_s029_0e76_31e6   /* (i) */
#define event_next               sub_s029_0e76_324c   /* (k, i): next slot >= i of kind k */
#define event_expire_msg         sub_s029_0e76_4442   /* (i) */
#define event_expire_msg_vec     vec_1e5f             /* overlay call vector of event_expire_msg */
#define event_48_max             sub_s029_0e76_4e0c   /* (who, a) */
#define days_per_month           D_2776               /* int[12] */
#define party_fame               D_9082
#define standing_fugger          D_4bb6               /* int: the party's standing with the Fugger (their cards add and take; under 0, with the town's regard, no tasks) - not in a saved game */
#define standing_medici          D_4bb8               /* int: with the Medici, likewise */
#define standing_hanse           D_4bba               /* int: with the Hanse, likewise */
#define party_money_gr           D_906d               /* groschen of party_money */
#define party_money_pf           D_906f               /* pfennigs of party_money */
#define inventories              F_2e38_003e          /* struct item6 [5][64] (6-byte records, 0x180 bytes per member) */
#define saints_known             F_2e38_082c          /* per member 0x14 bytes (160 bits) */
#define formulas_known           F_2e38_07be          /* per member 0x16 bytes (bits 0-2 = formulas 3k..3k+2) */
#define virtue_names             F_2e38_1736          /* 3 per entry of formulas_known */
#define saint_names              F_2e38_183e          /* 8 per byte of saints_known */
#define skill_names              F_290e_2137
#define bell_names               F_290e_20c3          /* time-of-day names */
#define saint_fns                F_290e_2937          /* int (far *[])(who, x, what): one function per saint (s039 165c) */
#define event_expiry_fns         F_290e_290f          /* event expiry handlers */
#define member_attrs             D_9cb2               /* = party[i] + 0x49: attributes */

/* ---- s030: overland map ------------------------------------------------------------------ */
#define overland_map             sub_s030_1462_0000   /* (): overlay entry: place the party, run the map */
#define overland_map_vec         vec_1f9f             /* overlay call vector of overland_map */
#define overland_open            sub_s030_1462_00a8   /* (x, y): open DARKLAND.MAP, centre the view, run the loop */
#define overland_loop            sub_s030_1462_02c0   /* (): returns the next card / menu id */
#define map_message              sub_s030_1462_2bb0   /* (msg) */

/* ---- s031..s041: strategic screens ------------------------------------------------------- */
#define member_sheet             sub_s031_1462_0124   /* ? (who): inventory / character screen of a member (F1..F5) */
#define member_sheet_vec         vec_1fdb             /* overlay call vector of member_sheet */
#define sheet_leave              sub_s031_1462_08f6   /* (r): fonts, cursor, buffers back; returns r */
#define sheet_open               sub_s031_1462_09b8   /* (who): fonts kept, the whole sheet drawn */
#define sheet_member             sub_s031_1462_0a82   /* (who): the sheet drawn for another member */
#define sheet_doll               sub_s031_1462_0b22   /* (who): the paper doll drawn into back_win */
#define sheet_portrait_frame     sub_s031_1462_2518   /* (who, colour): the frame of a portrait */
#define sheet_box_draw           sub_s031_1462_39a0   /* (box, who, struct gfx win): one box's text (0 name bar, 2 load ...) */
#define use_potion               sub_s031_1462_43b2   /* (who, idx): drink */
#define inventory_leave          sub_s031_1462_0000   /* (): save locs.tmp / events.tmp, free the world data */
#define inventory_leave_vec      vec_1dfb             /* overlay call vector of inventory_leave */
#define give_one                 sub_s031_18e7_0000   /* (from, to, idx, box): one of the stack (box 9: the equipment list) */
#define give_stack               sub_s031_18e7_007c   /* (from, to, idx, box): the whole stack */
#define add_item_q               sub_s031_18e7_016e   /* (who, code, qual): one item of quality qual */
#define add_item_q_vec           vec_1f8b             /* overlay call vector of add_item_q */
#define item_delete              sub_s031_18e7_0406   /* (who, idx) */
#define item_take_one            sub_s031_18e7_0474   /* (who, idx): one item off stack idx */
#define item_take_one_vec        vec_2003             /* overlay call vector of item_take_one */
#define item_remove_stack        sub_s031_18e7_07d4   /* (who, idx) */
#define item_move                sub_s031_18e7_10d2   /* (who, from, to) */
#define item_unequip             sub_s031_18e7_12e8   /* (who, idx, box): put away (box 9) */
#define item_ready               sub_s031_18e7_11b2   /* (who, idx): take up / wear (the sheet's A) */
#define equip_weight             sub_s031_18e7_140a   /* (who): recompute the weight of the equipped items */
#define equip_weight_vec         vec_2049             /* overlay call vector of equip_weight */
#define equipped_type            sub_s031_18e7_1460   /* (who, slot) */
#define item_entry_quality       sub_s031_18e7_15d6   /* (who, idx) */
#define random_item_flags        sub_s031_18e7_1894   /* (flags) */
#define random_item_flags_vec    vec_207b             /* overlay call vector of random_item_flags */
#define barter_screen            sub_s032_1462_0000   /* (shop, char far *name, long cats): barter screen */
#define barter_close             sub_s032_1462_076c   /* (ret, shop): fonts and the leader back */
#define barter_draw              sub_s032_1462_078e   /* (who, name far, cats long): today's stock (seeded), the whole screen */
#define barter_redraw            sub_s032_1462_0b30   /* (who, name far, cats long): the screen for another member */
#define ware_quality             sub_s032_1462_0efa   /* (k): the quality of ware k here */
#define barter_key               sub_s032_1462_0f60   /* (key): P buy, S sell, B next member ... at the cursors */
#define barter_screen_vec        vec_1eff             /* overlay call vector of barter_screen */
#define buy_price                sub_s032_1462_192e   /* (k): pfennigs the shop asks */
#define sell_price               sub_s032_1462_1b26   /* (k, qual): pfennigs the shop pays */
#define shop_has_item            sub_s032_1462_1d18   /* (k) */
#define alchemy_screen           sub_s033_1462_0000   /* (who): alchemy screen */
#define alchemy_screen_vec       vec_1ef5             /* overlay call vector of alchemy_screen */
#define alc_view                 sub_s033_15d1_000a   /* (): formula viewer */
#define alc_view_vec             vec_2017             /* overlay call vector of alc_view */
#define alchemy_close            sub_s033_1462_0628   /* (r): fonts, the formula file freed, the cursor; returns r */
#define alchemy_setup            sub_s033_1462_0658   /* (who): the formulas who knows into alchemy_rows, the party's ingredients counted */
#define alchemy_draw_list        sub_s033_1462_0796   /* (who): the batch list screen */
#define alchemy_draw_page        sub_s033_1462_094a   /* (who): the page of the chosen formula */
#define alchemy_list_key         sub_s033_1462_0d54   /* (key): U use the formula (the page, one batch), Up / Down */
#define alchemy_page_key         sub_s033_1462_0f6e   /* (key): M one batch more, F one fewer */
#define alchemy_reserve          sub_s033_1462_125c   /* (f): take formula f's ingredients from alchemy_have; 0 if short */
#define alchemy_unreserve        sub_s033_1462_12ec   /* (f): give them back */
#define alchemy_has_all          sub_s033_1462_1334   /* (f): the party has formula f's ingredients */
#define alchemy_brew             sub_s033_1462_1390   /* (hours): start the chosen batches (an event; the ingredients used) */
#define alchemy_resume           sub_s033_1462_14e6   /* (): a batch of this member under way: its ingredients back, its count into the rows */
#define alchemy_who              D_886c               /* the alchemist */
#define alchemy_row              D_e7d4               /* the chosen row of alchemy_rows */
#define alchemy_top              D_3248               /* the chosen row's place on screen */
#define alchemy_nrows            D_3244               /* the formulas the alchemist knows */
#define alchemy_page             D_324e               /* 0 the list, 1 the page of a formula */
#define alchemy_can_more         D_8872               /* one batch more can be made */
#define alchemy_have             D_e716               /* int[]: the party's ingredients (by item id), less what is reserved */
#define alchemy_file             D_9c2b               /* char far *: DARKLAND.ALC in memory */
#define philosophers_stone       D_9078               /* the Philosopher's Stone's bonus to alchemy (party info "PhStone") */
#define party_notes              D_9072               /* florins in notes (party info "NOTES") */
#define saint_pray_party         sub_s034_1462_0000   /* (who, saint): pray for the whole party; 1 heard, 0 not, -1 backed out */
#define saint_pray               sub_s034_1462_0102   /* (who, saint): pray (the prayer screen, then the roll) */
#define saint_screen             sub_s034_1462_022c   /* (who, saint, party): the prayer screen; the chance of success, -1 backed out */
#define saint_finish             sub_s034_1462_06e2   /* (who, saint): the fonts back; the chance (the saint's handler, 7) */
#define pray_box                 D_887a               /* unsigned: the names box's saved picture (0 none) */
#define pray_df                  D_887c               /* int: the divine favor to be spent */
#define pray_box_shown           D_887e               /* int: the names box is up */
#define pray_target              D_89ea               /* int: the member prayed for (-1 the one praying / the party) */
#define pray_choose              D_89ec               /* int: the saint helps one member: who is chosen */
#define party_info_screen        sub_s034_1550_0006   /* (): party information screen (the symbol party_info) */
#define party_info_screen_vec    vec_185b             /* overlay call vector of party_info_screen */
#define item_cache_screen        sub_s035_1462_0000   /* (arg): item cache screen (the symbol cache_screen) */
#define item_cache_screen_vec    vec_1ed7             /* overlay call vector of item_cache_screen */
#define cache_close              sub_s035_1462_04bc   /* (r): the store written back (or none marked), the leader back; returns r */
#define cache_first_draw         sub_s035_1462_0510   /* (who, arg): the whole screen; the store read (cache_load) */
#define cache_redraw             sub_s035_1462_0782   /* (who, arg): the screen for another member */
#define cache_key                sub_s035_1462_0a46   /* (key): G get one, P put one, C another member, arrows ... at the cursors */
#define cache_add                sub_s035_1462_10d2   /* (id, who, quality): one more in the store */
#define cache_take               sub_s035_1462_114a   /* (i): one fewer of entry i */
#define cache_load               sub_s035_1462_1350   /* (handle): store number handle of cache.tmp into cache_items */
#define cache_save               sub_s035_1462_1468   /* (handle): cache_items back into cache.tmp (-1 a new store) */
#define cache_items              F_2e38_37f4          /* struct { int id; char quality; char count; } []: the store shown */
#define cache_n                  D_89f6               /* unsigned: entries in cache_items */
#define cache_n_shown            D_89f0               /* int: the same, as the screen last listed */
#define cache_who                D_89f2               /* int: the member whose things are shown */
#define cache_leader             D_89ee               /* int: the leader when the screen opened */
#define cache_arg                D_89f8               /* int: the screen's argument */
#define cache_n_items            D_37ba               /* int[5]: the member's count of things */
#define cache_side               D_37c4               /* int[5]: 1 the cursor is in the store, 0 in the member's things */
#define cache_inv_top            D_37ce               /* int[5]: the member's list, first shown */
#define cache_top                D_37d8               /* int[5]: the store's list, first shown */
#define cache_inv_sel            D_37e2               /* int[5]: the member's list, the cursor's row */
#define cache_sel                D_37ec               /* int[5]: the store's list, the cursor's row */
#define camp_spend_day           sub_s036_1462_147a   /* (): spend one day */
#define camp_activity            D_8a18               /* int[5]: each member's day at the inn or camp (0 relax, 1 regain strength, 2 pray, 3 alchemy, 4 earn, 5 guard, 6 train) */
#define camp_activity_arg        D_89fe               /* int[5]: its number (6: the skill taught; 4: the wage) */
#define save_game                sub_s037_1462_2a68   /* (): "Save game" */
#define save_game_vec            vec_1847             /* overlay call vector of save_game */
#define char_in_slot             sub_s037_1462_3a8a   /* (p): character index in party slot p */
#define char_in_slot_vec         vec_1ecd             /* overlay call vector of char_in_slot */
#define slot_of_char             sub_s037_1462_3ae2   /* (c): party slot, -1 if none */
#define slot_of_char_vec         vec_20ad             /* overlay call vector of slot_of_char */
#define party_remove             sub_s037_1462_3b58   /* (p) */
#define party_remove_vec         vec_1ec3             /* overlay call vector of party_remove */
#define party_add                sub_s037_1462_3cd6   /* (c) */
#define party_add_vec            vec_20a3             /* overlay call vector of party_add */
#define party_delete             sub_s037_1462_3f30   /* (p) */
#define party_delete_vec         vec_18b5             /* overlay call vector of party_delete */
#define load_saved_game          sub_s037_1462_4184   /* (name) */
#define text_input               sub_s037_1462_43e6   /* (buf, max, prompt) */
#define text_input_vec           vec_20b7             /* overlay call vector of text_input */
#define roster_screen            sub_s037_1462_0000   /* (): the party of a new game (pics\cretscrn.pic): 1 begin, 0 back to the title */
#define title_screen             sub_s037_1e26_0008   /* (): the start screen (pics\startscr.pic): Quickstart, Create a New World, The Story Continues */
#define title_close              sub_s037_1e26_00a6   /* (): its buttons' pictures freed, the fonts back */
#define title_setup              sub_s037_1e26_00dc   /* (): pics\startscr.pic and its palette, the buttons' pictures */
#define title_redraw             sub_s037_1e26_014a   /* (): the start screen copied to the screen */
#define title_key                sub_s037_1e26_0172   /* (key): 0x1051 Quickstart, 0x2e43 create, 0x1454 load, 0x1000 quit; 0x11b: the game begins */
#define roster_leave             sub_s037_1462_0348   /* (r): the portrait and the buttons freed, the fonts back; returns r */
#define roster_load_pics         sub_s037_1462_0390   /* (): cretscrn.pic, buttnCR2.pic, the fonts */
#define roster_draw              sub_s037_1462_03d4   /* (): the menu, the list, the party's boxes */
#define roster_key               sub_s037_1462_0444   /* (key): C new, A add, D drop, K delete, H herald, S figure, 1/2/3 colours, B begin (0x11b) ... at roster_sel */
#define roster_list              sub_s037_1462_2760   /* (win): the characters of charactr.tmp ('+' in the party), roster_n counted */
#define roster_figures           sub_s037_1462_4562   /* (): figure_list from pics\F??stat.PIC, A??, C?? */
#define roster_sel               D_e7da               /* int: the character chosen (charactr.tmp index; -1 none) */
#define roster_n                 D_e7dc               /* int: characters in charactr.tmp */
#define roster_portrait          D_e88e               /* seg: the chosen member's portrait (pics\<figure>small.pic) */
#define roster_buttons           D_e88a               /* seg: pics\buttnCR2.pic */
#define figure_count             D_e7de               /* int: figures in figure_list */
#define figure_cur               D_e7e0               /* int: the figure Select Image gave last */
#define load_game                sub_s037_1c7d_09c6   /* (): confirm, tear down, reload, longjmp to the main loop */
#define load_game_vec            vec_1851             /* overlay call vector of load_game */
#define create_character         sub_s037_18c8_0004   /* () */
#define create_character_vec     vec_20cb             /* overlay call vector of create_character */
#define create_close             sub_s037_18c8_047c   /* (): pictures freed, the fonts back */
#define create_load_pics         sub_s037_18c8_04b2   /* (): chargen.pic, the buttons and the bubbles */
#define create_draw              sub_s037_18c8_053e   /* (): the whole screen */
#define create_key               sub_s037_18c8_058e   /* (key): the key / click handler (1..15 a list row); 0x11b: the character is done or given up */
#define create_reset             sub_s037_18c8_20fa   /* (): a new character (a name drawn, the attributes of the gender) */
#define create_state             D_8c1e               /* int: 0 name and gender, 1 childhood, 2 attribute EPs, 3 profession, 4 skill EPs */
#define create_row               D_8bfc               /* int: the attribute / skill row chosen (skills: rows 7 and 14 are gaps) */
#define create_hover             D_43ee               /* int: the list row under the pointer */
#define create_nlist             D_8c00               /* int: rows in the list */
#define create_list              D_8bd2               /* int[0x14]: the professions offered (-1 none) */
#define create_ep                D_8bfa               /* int: experience points left this step */
#define create_ep_step           D_8c48               /* int: the step's EPs */
#define create_age               D_e81c               /* unsigned: age */
#define create_female            D_e821               /* char: 1 a woman */
#define create_attr              D_e86e               /* char[7]: End Str Agl Per Int Chr, DF */
#define create_skill             D_e875               /* char[0x13]: the skills */
#define create_attr_spent        D_8c06               /* int[6]: EPs put into each attribute this step */
#define create_skill_spent       D_8c22               /* int[0x13]: EPs put into each skill this step */
#define create_name              D_e82f               /* char[]: the full name (22 letters) */
#define create_nick              D_e848               /* char[]: the short name (9 letters) */
#define create_random_name       D_8bfe               /* int: the name was drawn (a new gender draws another) */
#define create_prof              D_4428               /* int: the last profession */
#define create_prev              D_442a               /* int: the profession before the last */
#define create_child             D_8c02               /* int: the childhood */
#define create_taken             D_4402               /* char[0x25]: bit 0 a profession taken */
#define loot_distribute          sub_s040_1462_0000   /* (items far, unsigned char *count, int far *cash): loot screen entry */
#define loot_distribute_vec      vec_1dab             /* overlay call vector of loot_distribute */
#define loot_loop                sub_s040_1462_01c8   /* (items far, count near, cash far): the loot screen's loop */
#define loot_close               sub_s040_1462_06c8   /* (r): fonts and the leader back; returns r */
#define loot_first_draw          sub_s040_1462_06e2   /* (who): the whole screen */
#define loot_redraw              sub_s040_1462_0952   /* (who): the screen for another member */
#define loot_key                 sub_s040_1462_0c22   /* (key): G get, P put, D next member ... at the cursors */
#define loot_who                 D_8dc4               /* the member receiving */
#define loot_leader              D_8dba               /* the leader when the screen opened */
#define loot_n                   D_8dbc               /* entries in the pile */
#define loot_pile                D_8db6               /* struct invitem far *: the pile (6 bytes: item, type, quality, count, weight) */
#define loot_count_ptr           D_8dc8               /* unsigned char far *: the caller's count of the pile */
#define loot_cash                D_8dcc               /* int far *: the cash found (fl, gr, pf) */
#define loot_n_items             D_4c06               /* int[5]: each member's records, as the screen counted them */
#define loot_side                D_4c10               /* int[5]: 1 the cursor is in the pile, 0 in the member's things */
#define loot_inv_top             D_4c1a               /* int[5] */
#define loot_pile_top            D_4c24               /* int[5] */
#define loot_inv_sel             D_4c2e               /* int[5] */
#define loot_pile_sel            D_4c38               /* int[5] */
#define copyprot_run             sub_s041_1462_0000   /* (): copy-protection screen (the symbol copy_protection) */
#define copyprot_run_vec         vec_1fd1             /* overlay call vector of copyprot_run */
#define member_shield            D_a668               /* figure code per party slot ("F01", ...: PICS\<code>SHORT.PIC, the sprites); the herald is party byte +1 */
#define shield_pics              D_a2d7               /* shield picture handles per member */
#define lst_buf                  D_a889               /* darkland.lst buffer */
#define loc_buf                  D_a885               /* location file buffer */

/* ---- s043: card engine (more; card.h has the script API) --------------------------------- */
#define world_load               sub_s043_1462_0100   /* (locfile): world data for the current city */
#define world_load_vec           vec_18e7             /* overlay call vector of world_load */
#define cards_shutdown           sub_s043_1462_054e   /* (): charactr.tmp names, locs.tmp, events.tmp, free */
#define cards_shutdown_vec       vec_18c9             /* overlay call vector of cards_shutdown */
#define cards_startup            sub_s043_1462_078e   /* (): charactr.tmp, shields, common pictures, world */
#define cards_startup_vec        vec_1919             /* overlay call vector of cards_startup */
#define sound_file_load          sub_s043_1462_09c8   /* (name): whole file into a huge buffer */
#define sound_file_load_vec      vec_18fb             /* overlay call vector of sound_file_load */
#define palette_fade             sub_s043_17e5_0008   /* (a, b, c, d, e) */
#define msg_load                 sub_s043_150b_020e   /* (name): msgfiles\<name>.msg into msg_file, index the blocks */
#define text_expand              sub_s043_150b_06fe   /* (idx, out): text block with $variables expanded */
#define var_expand               sub_s043_150b_0886   /* (name): value of a $variable */
#define var_index                sub_s043_150b_10fc   /* (name): index in var_names, -1 */
#define highlight_rect           sub_s043_150b_1374   /* (win, x, y, w, h, c1, c2) */
#define card_mouse               sub_s043_150b_1744   /* (win, win2): mouse in the option menu */
#define wait_continue            sub_s043_150b_2826   /* (): Enter/Space/click, returns the key */
#define card_global_keys         sub_s043_150b_2936   /* (key): F1..F5 sheets, Shift-F1..F5 leader */
#define pass_hours_hourglass     sub_s043_150b_2a04   /* (hours) */
#define pass_hours_hourglass_vec vec_1f95             /* overlay call vector of pass_hours_hourglass */
#define hourglass_frame          sub_s043_150b_2d3a   /* (k) */
#define msg_file                 D_a895               /* loaded .msg card text (< 6000 bytes) */
#define msg_blocks               D_e8a5               /* number of text blocks */
#define msg_index                D_e8a7               /* text block positions {y[], x[], w[], ofs[]} */
#define menu_highlight           D_e96f               /* highlighted line (-1 none) */
#define side_win                 D_0104               /* ? text window of the card screen (s043: "side window"; also cleared as 320x200) */
#define screen_ptr               D_0118               /* ? screen window */
#define var_names                F_290e_21ff          /* char far *[73]: $variable names */

/* ---- s026..s028: swap stores (XMS, EMS, temporary files) --------------------------------- */
#define xms_read                 sub_s026_1ef8_0000   /* (len, buf, handle, off) */
#define xms_write                sub_s026_1ef8_008e   /* (len, buf, handle, off) */
#define swapfile_write           sub_s026_1f09_000c   /* (len, buf, fd, off) */
#define swapfile_read            sub_s026_1f09_00a6   /* (len, buf, fd, off) */
#define ems_write                sub_s026_1f1d_0000   /* (len, buf, handle, off) */
#define ems_read                 sub_s026_1f1d_013e   /* (len, buf, handle, off) */
#define ems_map_save             sub_s026_1f1d_027c   /* (h) */
#define ems_map_restore          sub_s026_1f1d_0296   /* (h) */
#define swap_hash                sub_s027_1f48_0262   /* (name) */
#define swap_create              sub_s027_1f48_0298   /* (size, name) */
#define swap_save                sub_s027_1f48_0a78   /* (n, buf, name): save n bytes under name */
#define swap_load_n              sub_s027_1f48_0b04   /* (n, buf, name) */
#define swap_load                sub_s027_1f48_0ba8   /* (buf, name): whole block, its size */
#define swap_load_free           sub_s027_1f48_0c4a   /* (buf, name): load and delete */
#define swap_save2               sub_s027_1f48_0d06   /* (name, buf, n): swap_save with the arguments reordered */
#define swap_save2_vec           vec_1b2b             /* overlay call vector of swap_save2 */
#define swap_load2               sub_s027_1f48_0d30   /* ? (name, buf) */
#define swap_load2_vec           vec_1b35             /* overlay call vector of swap_load2 */
#define swap_init                sub_s028_1f48_0000   /* (): choose the first usable swap store */
#define swap_init_vec            vec_1937             /* overlay call vector of swap_init */
#define ems_present              sub_s028_1f48_0aee   /* () */
#define xms_present              sub_s028_1f48_0cb2   /* () */
#define xms_entry                D_c0f9               /* XMS driver entry point */
#define xms_version              D_08c2               /* 0 = no XMS */
#define xms_error                D_08c8               /* last XMS error */
#define ems_version              D_08be               /* BCD */
#define ems_error                D_08c0               /* last EMS error */
#define ems_frame                D_c097               /* EMS page frame segment */
#define ems_frame_handle         D_940b               /* EMS handle of the page frame */
#define swap_allowed             D_08d6               /* low byte: stores allowed, 0xff00 = initialisation failed */
#define swap_dir_store           D_08d4               /* store holding the directory */
#define swap_stores              D_c09b               /* store table [type] {handle, size, base} */

/* ---- functions behind the overlay vectors of card.h -------------------------------------- */
#define party_lore_level_impl    sub_s039_1462_0470   /* the function behind party_lore_level (card.h) */
#define equipped_item_impl       sub_s031_18e7_14b6   /* the function behind equipped_item (card.h) */
#define lose_items_impl          sub_s031_18e7_0854   /* the function behind lose_items (card.h) */
#define give_reward_impl         sub_s039_1462_1e06   /* the function behind give_reward (card.h) */
#define side2_percent_impl       sub_s039_1462_0eaa   /* the function behind side2_percent (card.h) */
#define side1_percent_impl       sub_s039_1462_0e96   /* the function behind side1_percent (card.h) */
#define loc_side_set_impl        sub_s039_1462_0a7e   /* the function behind loc_side_set (card.h) */
#define loc_side_get_impl        sub_s039_1462_0a34   /* the function behind loc_side_get (card.h) */
#define practice_skill_impl      sub_s039_1462_0132   /* the function behind practice_skill (card.h) */
#define add_item_rec_impl        sub_s031_18e7_01b4   /* the function behind add_item_rec (card.h) */
#define nearest_loc_type_impl    sub_s030_1462_2842   /* the function behind nearest_loc_type (card.h) */
#define nearest_city_impl        sub_s030_1462_271a   /* the function behind nearest_city (card.h) */
#define keep_items_pct_impl      sub_s031_18e7_0ab8   /* the function behind keep_items_pct (card.h) */
#define remove_items_flags_impl  sub_s031_18e7_0c4a   /* the function behind remove_items_flags (card.h) */
#define add_items_impl           sub_s031_18e7_00e0   /* the function behind add_items (card.h) */
#define item_quality_impl        sub_s031_18e7_15f8   /* the function behind item_quality (card.h) */
#define add_item_impl            sub_s031_18e7_0128   /* the function behind add_item (card.h) */
#define item_quality_flags_impl  sub_s031_18e7_171c   /* the function behind item_quality_flags (card.h) */
#define take_item_impl           sub_s031_18e7_04d4   /* the function behind take_item (card.h) */
#define remove_item_stack_impl   sub_s031_18e7_0668   /* the function behind remove_item_stack (card.h) */
#define loc_hostile_impl         sub_s039_1462_00ba   /* the function behind loc_hostile (card.h) */
#define side1_strength_impl      sub_s039_1462_0d46   /* the function behind side1_strength (card.h) */
#define event_loc_of_impl        sub_s039_1462_1a68   /* the function behind event_loc_of (card.h) */
#define event_f2e_of_impl        sub_s039_1462_1a80   /* the function behind event_f2e_of (card.h) */
#define event_type_of_impl       sub_s039_1462_1a98   /* the function behind event_type_of (card.h) */
#define injure_impl              sub_s039_1462_026a   /* the function behind injure (card.h) */
#define party_present_count_impl sub_s039_1462_1de6   /* the function behind party_present_count (card.h) */
#define conflict_outcome_impl    sub_s039_1462_0ebe   /* the function behind conflict_outcome (card.h) */
#define side2_strength_impl      sub_s039_1462_0bf6   /* the function behind side2_strength (card.h) */
/* overland map (s030) */
#define map_marker_x             D_2866               /* int: the party marker's x on the map view (pixels) */
#define map_marker_y             D_2868               /* int: its y */
#define map_view_col             D_286a               /* int: the map view's left column (tiles) */
#define map_view_row             D_286c               /* int: the party's row in the view buffer */
#define map_buf_row              D_286e               /* int: the view buffer's top row on the map */
/* the card menu's layout (s043 struct layout, 0xf6 bytes) */
#define card_layout              D_e96e               /* char: lines (the struct's first field) */
#define card_layout_w            D_e970               /* int: width */
#define card_layout_x            D_e972               /* int: x */
#define card_layout_y            D_e974               /* unsigned char[20]: each line's y */
#define card_layout_h            D_e988               /* unsigned char[20]: each line's height */
#define card_layout_opt          D_e9b0               /* char[20]: the option number of each line */
#define card_layout_kind         D_e9c4               /* int[20]: 1/2 an option with a member/saint/item box (2252) */
#define card_box_shown           D_ee41               /* char: such a box is shown */
#define card_box_pointer         D_ee42               /* char: the pointer is in that box (its own option list) */
/* battles */
#define path_formation           D_e0a2               /* char: pathfinding for a formation (its collision rules) */
#define path_target_only         D_d697               /* char: ignore combatants other than the target */
#define pause_depth              D_07d3               /* char: pauses in force (orders being given ...) */
#define battle_view_col          D_c866               /* char: the battle view's origin, column (a step: 32 pixels sideways; vec_1d3d clamps it to 1..0x27) */
#define battle_view_row          D_c886               /* char: the battle view's origin, row (a step: 16 pixels across and 16 up) */
#define battle_follow            D_11b1               /* char: the view follows the member being given orders (1), jumping near an edge */
#define battle_end_at            D_da45               /* unsigned: the combat clock's tick at which a decided battle ends */
#define battle_tick              D_e092               /* unsigned: the battle's own clock, back at 0 in a new battle */
